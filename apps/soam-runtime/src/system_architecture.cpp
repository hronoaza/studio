#include "system_architecture.hpp"
#include "production_transition_evaluator.hpp"
#include "detail/production_transition_evaluation_binding.hpp"
#include "detail/policy_persistence_binding.hpp"
#include "policy_persistence.hpp"
#include "production_transition_live_snapshot.hpp"
#include "detail/production_transition_live_snapshot_binding.hpp"
#include "detail/source_capture_id_internal.hpp"

#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <exception>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace AdaptiveMesh {

void requireFinite(double value, const char* name) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument(std::string(name) + " must be finite");
    }
}

void Vector3D::validate() const {
    requireFinite(x, "x");
    requireFinite(y, "y");
    requireFinite(z, "z");
}

double Vector3D::distanceTo(const Vector3D& other) const {
    validate();
    other.validate();
    const double result = std::hypot(std::hypot(x - other.x, y - other.y), z - other.z);
    requireFinite(result, "distance");
    return result;
}

double Vector3D::orientationFactorTo(const Vector3D& target) const {
    const double distance = distanceTo(target);
    if (distance < 1e-6) return 1.0;
    return 0.5 * (1.0 + (target.z - z) / distance);
}

void IdentityInvariant::validate() const {
    requireFinite(baseline, "baseline");
    requireFinite(maxEpsilon, "maxEpsilon");
    if (maxEpsilon <= 0.0) {
        throw std::invalid_argument("maxEpsilon must be positive");
    }
}

bool IdentityInvariant::isWithinSafetyBound(double stateValue) const {
    validate();
    requireFinite(stateValue, "state");
    return std::abs(stateValue - baseline) <= maxEpsilon;
}

SignalCategory MetaEvaluator::evaluate(
    double candidateState,
    double healthIndex,
    const IdentityInvariant& omega) const
{
    requireFinite(candidateState, "candidateState");
    requireFinite(healthIndex, "healthIndex");

    if (!omega.isWithinSafetyBound(candidateState)) {
        return SignalCategory::DESTRUCTIVE_DRIFT;
    }

    if (healthIndex > 0.7 &&
        std::abs(candidateState - omega.baseline) > 1.2) {
        return SignalCategory::CREATIVE_SIGNAL;
    }

    return SignalCategory::NOISE;
}

void SpatialBridge::updateBridgeState(SignalCategory category) {
    switch (category) {
    case SignalCategory::NOISE:
        capacity = std::max(0.01, capacity * 0.85);
        status = BridgeStatus::DAMPING;
        break;
    case SignalCategory::CREATIVE_SIGNAL:
        capacity = std::min(1.0, capacity + 0.15);
        status = capacity >= 0.9
            ? BridgeStatus::NORMAL
            : BridgeStatus::RECOVERY;
        break;
    case SignalCategory::DESTRUCTIVE_DRIFT:
        capacity = 0.0;
        status = BridgeStatus::ISOLATED;
        break;
    }
}

double SpatialBridge::getEffectiveCoupling() const {
    requireFinite(distance, "bridge distance");
    requireFinite(orientationWeight, "bridge orientationWeight");
    requireFinite(capacity, "bridge capacity");

    if (distance < 0.0 ||
        orientationWeight < 0.0 || orientationWeight > 1.0 ||
        capacity < 0.0 || capacity > 1.0) {
        throw std::invalid_argument("invalid bridge geometry/state");
    }

    return capacity *
           (1.0 / (1.0 + 0.1 * distance)) *
           orientationWeight;
}

double SpatialBridge::getEffectiveTransmission() const {
    return getEffectiveCoupling();
}

AutopoieticNode::AutopoieticNode(
    std::size_t nodeId,
    Vector3D pos,
    double initialBaseline)
    : id(nodeId),
      position(pos),
      state(initialBaseline)
{
    position.validate();
    requireFinite(initialBaseline, "baseline");
    invariant.baseline = initialBaseline;
}

AutopoieticNode::AutopoieticNode(const AutopoieticNode& other)
    : id(other.id),
      position(other.position),
      state(other.state.load()),
      healthIndex(other.healthIndex.load()),
      invariant(other.invariant),
      metaEvaluator(other.metaEvaluator),
      bridges(other.bridges)
{
}

void AutopoieticNode::updateHealth() {
    invariant.validate();
    const double currentState = state.load();
    requireFinite(currentState, "state");

    healthIndex.store(std::max(
        0.0,
        1.0 - (std::abs(currentState - invariant.baseline) /
               invariant.maxEpsilon)));
}

double AutopoieticNode::applyLocalReflexFilter(double rawInput) const {
    requireFinite(rawInput, "rawInput");

    const double currentState = state.load();
    requireFinite(currentState, "state");

    constexpr double maxAllowedReflexStep = 3.5;
    const double delta = rawInput - currentState;

    if (std::abs(delta) > maxAllowedReflexStep) {
        return currentState +
            (delta > 0.0 ? maxAllowedReflexStep : -maxAllowedReflexStep);
    }

    return rawInput;
}

struct SpatialAdaptiveMesh::Impl {
    using EdgeKey = std::pair<std::size_t, std::size_t>;

    struct EdgeKeyHash {
        std::size_t operator()(const EdgeKey& key) const noexcept {
            const std::size_t h1 = std::hash<std::size_t>{}(key.first);
            const std::size_t h2 = std::hash<std::size_t>{}(key.second);
            return h1 ^ (h2 + static_cast<std::size_t>(0x9e3779b9U) +
                         (h1 << 6) + (h1 >> 2));
        }
    };

    struct WorkerSlot {
        std::size_t assignedGeneration = 0;
        std::size_t completedGeneration = 0;
    };

    std::vector<AutopoieticNode> nodes;
    double alpha = 0.15;
    std::size_t workerLimit = 0;

    mutable std::shared_mutex topologyMutex;

    std::vector<std::jthread> workers;
    std::vector<std::unique_ptr<WorkerSlot>> workerSlots;
    std::mutex workMutex;
    std::condition_variable workAvailable;
    std::condition_variable workCompleted;

    bool stoppingWorkers = false;
    std::size_t workGeneration = 0;
    std::size_t activeNodeCount = 0;
    std::size_t activeWorkerCount = 0;
    std::size_t remainingWorkerCount = 0;
    std::exception_ptr workerException;

    std::vector<double>* dispatchedStates = nullptr;
    std::vector<std::vector<double>>* dispatchedBridgeCapacities = nullptr;
    std::vector<std::vector<BridgeStatus>>* dispatchedBridgeStatuses = nullptr;

    std::vector<double> computedStates;
    std::vector<std::vector<double>> pendingBridgeCapacities;
    std::vector<std::vector<BridgeStatus>> pendingBridgeStatuses;

    bool simulationBufferShapeDirty = true;
    std::uint64_t nextRelationshipGeneration = 0;
    std::uint64_t transitionStateVersion = 1;

    explicit Impl(std::size_t maxWorkers)
        : workerLimit(maxWorkers)
    {
    }

    ~Impl() {
        stopWorkerPool();
    }

    void validateNodeIndex(int nodeId) const {
        if (nodeId < 0 || nodeId >= static_cast<int>(nodes.size())) {
            throw std::out_of_range("node index is out of range");
        }
    }

    void enforceStabilityConditionUnlocked() noexcept {
        std::size_t maxDegree = 0;
        for (const auto& node : nodes) {
            maxDegree = std::max(maxDegree, node.bridges.size());
        }

        alpha = maxDegree == 0
            ? 0.15
            : std::min(0.2, 0.8 / static_cast<double>(maxDegree));
    }

    void validateTopologyUnlocked() const {
        std::unordered_set<EdgeKey, EdgeKeyHash> directedEdges;

        for (std::size_t sourceNodeId = 0;
             sourceNodeId < nodes.size();
             ++sourceNodeId) {
            const auto& node = nodes[sourceNodeId];
            node.position.validate();
            node.invariant.validate();

            for (const auto& bridge : node.bridges) {
                if (bridge.targetNodeId < 0 ||
                    bridge.targetNodeId >= static_cast<int>(nodes.size()) ||
                    bridge.targetNodeId == static_cast<int>(sourceNodeId)) {
                    throw std::runtime_error(
                        "bridge target violates topology invariant");
                }

                static_cast<void>(bridge.getEffectiveCoupling());

                if (!directedEdges.emplace(
                        sourceNodeId,
                        static_cast<std::size_t>(bridge.targetNodeId))
                         .second) {
                    throw std::runtime_error("duplicate directed bridge");
                }
            }
        }

        for (const auto& edge : directedEdges) {
            if (!directedEdges.contains({edge.second, edge.first})) {
                throw std::runtime_error(
                    "bridge symmetry invariant violated");
            }
        }
    }

    void connectPairsUnlocked(
        const std::vector<std::pair<int, int>>& pairs)
    {
        struct PendingEdge {
            std::size_t source;
            SpatialBridge bridge;
        };

        std::vector<PendingEdge> pending;
        pending.reserve(pairs.size() * 2);

        std::vector<std::size_t> additions(nodes.size());

        for (const auto& [nodeA, nodeB] : pairs) {
            const auto source = static_cast<std::size_t>(nodeA);
            const auto target = static_cast<std::size_t>(nodeB);

            const double distance =
                nodes[source].position.distanceTo(nodes[target].position);
            const double forward =
                nodes[source].position.orientationFactorTo(
                    nodes[target].position);
            const double reverse =
                nodes[target].position.orientationFactorTo(
                    nodes[source].position);
            const std::uint64_t relationshipGeneration =
                ++nextRelationshipGeneration;

            pending.push_back({
                source,
                {nodeB, distance, forward, 1.0, BridgeStatus::NORMAL,
                 relationshipGeneration}
            });
            pending.push_back({
                target,
                {nodeA, distance, reverse, 1.0, BridgeStatus::NORMAL,
                 relationshipGeneration}
            });

            ++additions[source];
            ++additions[target];
        }

        // All potentially throwing preparation completes before publication.
        for (std::size_t nodeId = 0; nodeId < nodes.size(); ++nodeId) {
            if (additions[nodeId] == 0) continue;
            auto& bridges = nodes[nodeId].bridges;
            bridges.reserve(bridges.size() + additions[nodeId]);
        }

        static_assert(std::is_nothrow_copy_constructible_v<SpatialBridge>);

        for (const auto& edge : pending) {
            nodes[edge.source].bridges.push_back(edge.bridge);
        }

        if (!pairs.empty()) {
            enforceStabilityConditionUnlocked();
            simulationBufferShapeDirty = true;
            ++transitionStateVersion;
        }
    }

    [[nodiscard]] std::size_t resolveWorkerCountUnlocked() const noexcept {
        if (nodes.empty()) return 0;

        const std::size_t hardwareWorkers = std::max<std::size_t>(
            1,
            static_cast<std::size_t>(std::thread::hardware_concurrency()));

        const std::size_t requestedWorkers =
            workerLimit == 0 ? hardwareWorkers : workerLimit;

        return std::min(requestedWorkers, nodes.size());
    }

    [[nodiscard]] static double computeEffectiveCouplingUnchecked(
        const SpatialBridge& bridge) noexcept
    {
        return bridge.capacity *
               (1.0 / (1.0 + 0.1 * bridge.distance)) *
               bridge.orientationWeight;
    }

    static void updateHealthUnchecked(
        AutopoieticNode& node,
        double currentState) noexcept
    {
        node.healthIndex.store(std::max(
            0.0,
            1.0 - (std::abs(currentState - node.invariant.baseline) /
                   node.invariant.maxEpsilon)));
    }

    [[nodiscard]] static SignalCategory evaluateSignalUncheckedInvariant(
        double candidateState,
        double healthIndex,
        const IdentityInvariant& invariant)
    {
        requireFinite(candidateState, "candidateState");

        const double deltaFromBase =
            std::abs(candidateState - invariant.baseline);

        if (deltaFromBase > invariant.maxEpsilon) {
            return SignalCategory::DESTRUCTIVE_DRIFT;
        }

        if (healthIndex > 0.7 && deltaFromBase > 1.2) {
            return SignalCategory::CREATIVE_SIGNAL;
        }

        return SignalCategory::NOISE;
    }

    void runNodeRange(
        std::size_t firstNode,
        std::size_t lastNode,
        std::vector<double>& outputStates,
        std::vector<std::vector<double>>& bridgeCapacityOutput,
        std::vector<std::vector<BridgeStatus>>& bridgeStatusOutput)
    {
        for (std::size_t nodeId = firstNode;
             nodeId < lastNode;
             ++nodeId) {
            auto& node = nodes[nodeId];
            double diffusionSum = 0.0;
            const double currentState = node.state.load();

            for (std::size_t bridgeIndex = 0;
                 bridgeIndex < node.bridges.size();
                 ++bridgeIndex) {
                SpatialBridge nextBridge = node.bridges[bridgeIndex];

                const auto neighborId =
                    static_cast<std::size_t>(nextBridge.targetNodeId);
                const double neighborState =
                    nodes[neighborId].state.load();
                const double deltaState =
                    neighborState - currentState;

                nextBridge.updateBridgeState(
                    evaluateSignalUncheckedInvariant(
                        currentState + deltaState,
                        node.healthIndex.load(),
                        node.invariant));

                if (!std::isfinite(nextBridge.capacity)) {
                    throw std::runtime_error(
                        "simulation produced non-finite bridge capacity");
                }

                bridgeCapacityOutput[nodeId][bridgeIndex] =
                    nextBridge.capacity;
                bridgeStatusOutput[nodeId][bridgeIndex] =
                    nextBridge.status;

                diffusionSum +=
                    computeEffectiveCouplingUnchecked(nextBridge) *
                    deltaState;
            }

            outputStates[nodeId] =
                currentState + alpha * diffusionSum;
        }
    }

    void workerLoop(std::size_t workerId, WorkerSlot& slot) {
        std::unique_lock lock(workMutex);

        for (;;) {
            workAvailable.wait(lock, [this, &slot] {
                return stoppingWorkers ||
                       slot.assignedGeneration >
                           slot.completedGeneration;
            });

            if (stoppingWorkers &&
                slot.assignedGeneration <= slot.completedGeneration) {
                return;
            }

            const std::size_t observedGeneration =
                slot.assignedGeneration;
            const std::size_t workerCount =
                activeWorkerCount;
            const std::size_t nodeCount =
                activeNodeCount;

            auto* outputStates = dispatchedStates;
            auto* bridgeCapacities =
                dispatchedBridgeCapacities;
            auto* bridgeStatuses =
                dispatchedBridgeStatuses;

            const std::size_t firstNode =
                workerId * nodeCount / workerCount;
            const std::size_t lastNode =
                (workerId + 1) * nodeCount / workerCount;

            lock.unlock();

            std::exception_ptr error;
            try {
                runNodeRange(
                    firstNode,
                    lastNode,
                    *outputStates,
                    *bridgeCapacities,
                    *bridgeStatuses);
            } catch (...) {
                error = std::current_exception();
            }

            lock.lock();

            if (error && !workerException) {
                workerException = error;
            }

            if (observedGeneration == workGeneration &&
                workerId < activeWorkerCount &&
                slot.completedGeneration < observedGeneration) {
                slot.completedGeneration = observedGeneration;
                if (remainingWorkerCount > 0 &&
                    --remainingWorkerCount == 0) {
                    workCompleted.notify_one();
                }
            }
        }
    }

    void ensureWorkerPoolUnlocked() {
        const std::size_t requiredWorkerCount =
            resolveWorkerCountUnlocked();

        workers.reserve(requiredWorkerCount);
        workerSlots.reserve(requiredWorkerCount);

        for (std::size_t workerId = workers.size();
             workerId < requiredWorkerCount;
             ++workerId) {
            auto slot = std::make_unique<WorkerSlot>();
            slot->assignedGeneration = workGeneration;
            slot->completedGeneration = workGeneration;

            WorkerSlot* const slotAddress = slot.get();
            workerSlots.push_back(std::move(slot));

            try {
                workers.emplace_back(
                    [this, workerId, slotAddress] {
                        workerLoop(workerId, *slotAddress);
                    });
            } catch (...) {
                workerSlots.pop_back();
                throw;
            }
        }
    }

    void stopWorkerPool() noexcept {
        {
            std::lock_guard lock(workMutex);
            stoppingWorkers = true;
        }
        workAvailable.notify_all();
        workers.clear();
        workerSlots.clear();
    }

    void addNode(
        std::size_t id,
        Vector3D position,
        double baseline)
    {
        position.validate();
        requireFinite(baseline, "baseline");

        std::unique_lock lock(topologyMutex);

        if (id != nodes.size()) {
            throw std::invalid_argument(
                "node ID must match insertion index");
        }

        nodes.emplace_back(id, position, baseline);
        simulationBufferShapeDirty = true;
        ++transitionStateVersion;
    }

    void connectNodes(int nodeA, int nodeB) {
        std::unique_lock lock(topologyMutex);

        validateNodeIndex(nodeA);
        validateNodeIndex(nodeB);

        if (nodeA == nodeB) {
            throw std::invalid_argument(
                "self-connections are not allowed");
        }

        const auto& bridges =
            nodes[static_cast<std::size_t>(nodeA)].bridges;

        const bool alreadyConnected =
            std::any_of(
                bridges.begin(),
                bridges.end(),
                [nodeB](const SpatialBridge& bridge) {
                    return bridge.targetNodeId == nodeB;
                });

        if (alreadyConnected) {
            throw std::invalid_argument(
                "bridge pair already exists");
        }

        connectPairsUnlocked({{nodeA, nodeB}});
    }

    void connectNodePairs(
        const std::vector<std::pair<int, int>>& connections)
    {
        std::unique_lock lock(topologyMutex);

        std::unordered_set<EdgeKey, EdgeKeyHash> batchPairs;
        batchPairs.reserve(connections.size());

        for (const auto& [nodeA, nodeB] : connections) {
            validateNodeIndex(nodeA);
            validateNodeIndex(nodeB);

            if (nodeA == nodeB) {
                throw std::invalid_argument(
                    "self-connections are not allowed");
            }

            const std::size_t first =
                static_cast<std::size_t>(
                    std::min(nodeA, nodeB));
            const std::size_t second =
                static_cast<std::size_t>(
                    std::max(nodeA, nodeB));

            if (!batchPairs.emplace(first, second).second) {
                throw std::invalid_argument(
                    "duplicate bridge pair in batch");
            }

            const bool alreadyConnected =
                std::any_of(
                    nodes[first].bridges.begin(),
                    nodes[first].bridges.end(),
                    [second](const SpatialBridge& bridge) {
                        return bridge.targetNodeId ==
                               static_cast<int>(second);
                    });

            if (alreadyConnected) {
                throw std::invalid_argument(
                    "bridge pair already exists");
            }
        }

        connectPairsUnlocked(connections);
    }

    void pruneIsolatedBridges(double threshold) {
        requireFinite(threshold, "minCapacityThreshold");
        if (threshold < 0.0) {
            throw std::invalid_argument(
                "minCapacityThreshold must not be negative");
        }

        std::unique_lock lock(topologyMutex);

        std::unordered_set<EdgeKey, EdgeKeyHash> invalidPairs;

        for (std::size_t sourceNodeId = 0;
             sourceNodeId < nodes.size();
             ++sourceNodeId) {
            for (const auto& bridge :
                 nodes[sourceNodeId].bridges) {
                if (bridge.targetNodeId < 0 ||
                    bridge.targetNodeId >=
                        static_cast<int>(nodes.size())) {
                    continue;
                }

                if (bridge.capacity < threshold ||
                    bridge.status == BridgeStatus::ISOLATED) {
                    const auto targetNodeId =
                        static_cast<std::size_t>(
                            bridge.targetNodeId);
                    invalidPairs.emplace(
                        std::min(sourceNodeId, targetNodeId),
                        std::max(sourceNodeId, targetNodeId));
                }
            }
        }

        bool changed = false;

        for (std::size_t sourceNodeId = 0;
             sourceNodeId < nodes.size();
             ++sourceNodeId) {
            auto& bridges =
                nodes[sourceNodeId].bridges;
            const std::size_t oldSize =
                bridges.size();

            std::erase_if(
                bridges,
                [&invalidPairs, sourceNodeId](
                    const SpatialBridge& bridge) {
                    const auto targetNodeId =
                        static_cast<std::size_t>(
                            bridge.targetNodeId);
                    return invalidPairs.contains({
                        std::min(sourceNodeId, targetNodeId),
                        std::max(sourceNodeId, targetNodeId)
                    });
                });

            changed = changed ||
                      bridges.size() != oldSize;
        }

        enforceStabilityConditionUnlocked();
        simulationBufferShapeDirty =
            simulationBufferShapeDirty || changed;
        if (changed) {
            ++transitionStateVersion;
        }
    }

    void autoConnectNearbyNodes(double radius) {
        requireFinite(radius, "radius");
        if (radius < 0.0) {
            throw std::invalid_argument(
                "radius must not be negative");
        }

        std::unique_lock lock(topologyMutex);

        std::vector<std::pair<int, int>> connections;

        for (std::size_t first = 0;
             first < nodes.size();
             ++first) {
            for (std::size_t second = first + 1;
                 second < nodes.size();
                 ++second) {
                const bool alreadyConnected =
                    std::any_of(
                        nodes[first].bridges.begin(),
                        nodes[first].bridges.end(),
                        [second](const SpatialBridge& bridge) {
                            return bridge.targetNodeId ==
                                static_cast<int>(second);
                        });

                if (!alreadyConnected &&
                    nodes[first].position.distanceTo(
                        nodes[second].position) <= radius) {
                    connections.emplace_back(
                        static_cast<int>(first),
                        static_cast<int>(second));
                }
            }
        }

        connectPairsUnlocked(connections);
    }

    void injectExternalShock(
        int targetNodeId,
        double shockMagnitude)
    {
        requireFinite(shockMagnitude, "shockMagnitude");

        std::unique_lock lock(topologyMutex);
        validateNodeIndex(targetNodeId);

        auto& node =
            nodes[static_cast<std::size_t>(targetNodeId)];

        node.state.store(
            node.applyLocalReflexFilter(
                node.state.load() + shockMagnitude));
        node.updateHealth();
        ++transitionStateVersion;
    }

    void simulationStep() {
        std::unique_lock lock(topologyMutex);

        validateTopologyUnlocked();
        ensureWorkerPoolUnlocked();

        if (simulationBufferShapeDirty) {
            computedStates.resize(nodes.size());
            pendingBridgeCapacities.resize(nodes.size());
            pendingBridgeStatuses.resize(nodes.size());

            for (std::size_t nodeId = 0;
                 nodeId < nodes.size();
                 ++nodeId) {
                pendingBridgeCapacities[nodeId].resize(
                    nodes[nodeId].bridges.size());
                pendingBridgeStatuses[nodeId].resize(
                    nodes[nodeId].bridges.size());
            }

            simulationBufferShapeDirty = false;
        }

        if (nodes.empty()) return;

        if (workers.size() <= 1) {
            runNodeRange(
                0,
                nodes.size(),
                computedStates,
                pendingBridgeCapacities,
                pendingBridgeStatuses);
        } else {
            {
                std::lock_guard workLock(workMutex);

                activeNodeCount = nodes.size();
                activeWorkerCount = workers.size();
                remainingWorkerCount = activeWorkerCount;
                workerException = nullptr;

                dispatchedStates = &computedStates;
                dispatchedBridgeCapacities =
                    &pendingBridgeCapacities;
                dispatchedBridgeStatuses =
                    &pendingBridgeStatuses;

                ++workGeneration;

                for (std::size_t workerId = 0;
                     workerId < activeWorkerCount;
                     ++workerId) {
                    workerSlots[workerId]
                        ->assignedGeneration = workGeneration;
                }
            }

            workAvailable.notify_all();

            std::unique_lock workLock(workMutex);
            workCompleted.wait(
                workLock,
                [this] {
                    return remainingWorkerCount == 0;
                });

            dispatchedStates = nullptr;
            dispatchedBridgeCapacities = nullptr;
            dispatchedBridgeStatuses = nullptr;

            std::exception_ptr error =
                workerException;
            workerException = nullptr;

            workLock.unlock();

            if (error) {
                std::rethrow_exception(error);
            }
        }

        for (double stateValue : computedStates) {
            if (!std::isfinite(stateValue)) {
                throw std::runtime_error(
                    "simulation produced non-finite state");
            }
        }

        // Domain publication occurs only after successful computation/validation.
        for (std::size_t nodeId = 0;
             nodeId < nodes.size();
             ++nodeId) {
            nodes[nodeId].state.store(
                computedStates[nodeId]);

            updateHealthUnchecked(
                nodes[nodeId],
                computedStates[nodeId]);

            for (std::size_t bridgeIndex = 0;
                 bridgeIndex <
                     nodes[nodeId].bridges.size();
                 ++bridgeIndex) {
                nodes[nodeId]
                    .bridges[bridgeIndex]
                    .capacity =
                    pendingBridgeCapacities
                        [nodeId][bridgeIndex];

                nodes[nodeId]
                    .bridges[bridgeIndex]
                    .status =
                    pendingBridgeStatuses
                        [nodeId][bridgeIndex];
            }
        }

        ++transitionStateVersion;
    }
};

SpatialAdaptiveMesh::SpatialAdaptiveMesh(std::size_t maxWorkers)
    : impl_(std::make_unique<Impl>(maxWorkers)),
      transitionEvaluationBinding_(
          detail::makeProductionTransitionEvaluationBinding(this)),
      persistenceRegistryState_(
          detail::makeProductionPersistenceRegistryState(this)),
      transitionLiveSnapshotBinding_(
          detail::makeProductionTransitionLiveSnapshotBinding(this))
{
}

SpatialAdaptiveMesh::~SpatialAdaptiveMesh() {
    detail::invalidateProductionPersistenceRegistryState(
        persistenceRegistryState_);
    detail::invalidateProductionTransitionLiveSnapshotBinding(
        transitionLiveSnapshotBinding_);
    detail::invalidateProductionTransitionEvaluationBinding(
        transitionEvaluationBinding_);
}

void SpatialAdaptiveMesh::addNode(
    std::size_t id,
    Vector3D pos,
    double baseline)
{
    impl_->addNode(id, pos, baseline);
}

void SpatialAdaptiveMesh::connectNodes(int nodeA, int nodeB) {
    impl_->connectNodes(nodeA, nodeB);
}

void SpatialAdaptiveMesh::connectNodePairs(
    const std::vector<std::pair<int, int>>& connections)
{
    impl_->connectNodePairs(connections);
}

void SpatialAdaptiveMesh::enforceStabilityCondition() noexcept {
    std::unique_lock lock(impl_->topologyMutex);
    impl_->enforceStabilityConditionUnlocked();
}

void SpatialAdaptiveMesh::pruneIsolatedBridges(
    double minCapacityThreshold)
{
    impl_->pruneIsolatedBridges(minCapacityThreshold);
}

void SpatialAdaptiveMesh::autoConnectNearbyNodes(double radius) {
    impl_->autoConnectNearbyNodes(radius);
}

void SpatialAdaptiveMesh::injectExternalShock(
    int targetNodeId,
    double shockMagnitude)
{
    impl_->injectExternalShock(
        targetNodeId,
        shockMagnitude);
}

void SpatialAdaptiveMesh::simulationStep() {
    impl_->simulationStep();
}

void SpatialAdaptiveMesh::simulationStepAsync() {
    // Retained as a blocking compatibility wrapper in this phase.
    impl_->simulationStep();
}

double SpatialAdaptiveMesh::getNodeState(std::size_t id) const {
    std::shared_lock lock(impl_->topologyMutex);
    return impl_->nodes.at(id).state.load();
}

double SpatialAdaptiveMesh::getNodeHealth(std::size_t id) const {
    std::shared_lock lock(impl_->topologyMutex);
    return impl_->nodes.at(id).healthIndex.load();
}

std::size_t SpatialAdaptiveMesh::getNodeBridgesCount(
    std::size_t id) const
{
    std::shared_lock lock(impl_->topologyMutex);
    return impl_->nodes.at(id).bridges.size();
}

std::optional<ProductionRelationshipSourceSnapshot>
SpatialAdaptiveMesh::captureProductionRelationshipSourceSnapshot(
    const ProductionRelationshipLocator& locator) const
{
    const auto captureIdBytes =
        detail::tryGenerateSourceCaptureIdBytes();
    if (!captureIdBytes.has_value()) {
        return std::nullopt;
    }

    SourceCaptureId captureId{*captureIdBytes};

    std::shared_lock lock(impl_->topologyMutex);

    if (locator.sourceNodeId >= impl_->nodes.size() ||
        locator.targetNodeId >= impl_->nodes.size() ||
        locator.sourceNodeId == locator.targetNodeId) {
        return std::nullopt;
    }

    const auto& source = impl_->nodes[locator.sourceNodeId];
    const auto& target = impl_->nodes[locator.targetNodeId];

    const auto found = std::find_if(
        source.bridges.begin(),
        source.bridges.end(),
        [&locator](const SpatialBridge& bridge) {
            return bridge.targetNodeId ==
                static_cast<int>(locator.targetNodeId);
        });

    if (found == source.bridges.end()) {
        return std::nullopt;
    }

    return ProductionRelationshipSourceSnapshot{
        std::move(captureId),
        locator.sourceNodeId,
        locator.targetNodeId,
        found->generation,
        impl_->transitionStateVersion,
        found->distance,
        found->orientationWeight,
        found->capacity,
        found->status,
        source.state.load(),
        target.state.load(),
        source.healthIndex.load(),
        target.healthIndex.load()
    };
}

ProductionTransitionEvaluator
SpatialAdaptiveMesh::productionTransitionEvaluator() const noexcept {
    return ProductionTransitionEvaluator{transitionEvaluationBinding_};
}

ProductionBridgePersistenceRegistry
SpatialAdaptiveMesh::productionBridgePersistenceRegistry() const noexcept {
    return ProductionBridgePersistenceRegistry{persistenceRegistryState_};
}

ProductionTransitionLiveSnapshotSource
SpatialAdaptiveMesh::productionTransitionLiveSnapshotSource() const noexcept {
    return ProductionTransitionLiveSnapshotSource{
        transitionLiveSnapshotBinding_};
}

std::optional<SpatialAdaptiveMesh::PersistenceCreationSnapshot>
SpatialAdaptiveMesh::capturePersistenceCreationSnapshot(
    std::size_t sourceNodeId,
    std::size_t targetNodeId) const
{
    std::shared_lock lock(impl_->topologyMutex);

    if (sourceNodeId >= impl_->nodes.size() ||
        targetNodeId >= impl_->nodes.size() ||
        sourceNodeId == targetNodeId) {
        return std::nullopt;
    }

    const auto& bridges = impl_->nodes[sourceNodeId].bridges;
    const auto found = std::find_if(
        bridges.begin(),
        bridges.end(),
        [targetNodeId](const SpatialBridge& bridge) {
            return bridge.targetNodeId ==
                static_cast<int>(targetNodeId);
        });

    if (found == bridges.end()) {
        return std::nullopt;
    }

    return PersistenceCreationSnapshot{
        sourceNodeId,
        targetNodeId,
        found->generation,
        impl_->transitionStateVersion
    };
}


bool SpatialAdaptiveMesh::transitionLocatorIsValid(
    std::size_t sourceNodeId,
    std::size_t targetNodeId) const noexcept
{
    std::shared_lock lock(impl_->topologyMutex);
    return sourceNodeId < impl_->nodes.size() &&
           targetNodeId < impl_->nodes.size() &&
           sourceNodeId != targetNodeId;
}

std::optional<SpatialAdaptiveMesh::TransitionSnapshot>
SpatialAdaptiveMesh::captureTransitionSnapshot(
    std::size_t sourceNodeId,
    std::size_t targetNodeId) const
{
    std::shared_lock lock(impl_->topologyMutex);

    if (sourceNodeId >= impl_->nodes.size() ||
        targetNodeId >= impl_->nodes.size() ||
        sourceNodeId == targetNodeId) {
        return std::nullopt;
    }

    const auto& bridges = impl_->nodes[sourceNodeId].bridges;
    const auto found = std::find_if(
        bridges.begin(),
        bridges.end(),
        [targetNodeId](const SpatialBridge& bridge) {
            return bridge.targetNodeId ==
                static_cast<int>(targetNodeId);
        });

    if (found == bridges.end()) {
        return std::nullopt;
    }

    return TransitionSnapshot{
        sourceNodeId,
        targetNodeId,
        found->generation,
        impl_->transitionStateVersion
    };
}

bool SpatialAdaptiveMesh::revalidateTransitionSnapshot(
    const TransitionSnapshot& snapshot) const
{
    std::shared_lock lock(impl_->topologyMutex);

    if (snapshot.sourceNodeId >= impl_->nodes.size() ||
        snapshot.targetNodeId >= impl_->nodes.size() ||
        impl_->transitionStateVersion != snapshot.stateVersion) {
        return false;
    }

    const auto& bridges = impl_->nodes[snapshot.sourceNodeId].bridges;
    const auto found = std::find_if(
        bridges.begin(),
        bridges.end(),
        [&snapshot](const SpatialBridge& bridge) {
            return bridge.targetNodeId ==
                       static_cast<int>(snapshot.targetNodeId) &&
                   bridge.generation ==
                       snapshot.relationshipGeneration;
        });

    return found != bridges.end();
}

} // namespace AdaptiveMesh


std::optional<AdaptiveMesh::ProductionTransitionLiveSnapshot>
AdaptiveMesh::SpatialAdaptiveMesh::captureTransitionLiveValiditySnapshot(
    std::size_t sourceNodeId,
    std::size_t targetNodeId) const
{
    std::shared_lock lock(impl_->topologyMutex);

    if (sourceNodeId >= impl_->nodes.size() ||
        targetNodeId >= impl_->nodes.size() ||
        sourceNodeId == targetNodeId) {
        return std::nullopt;
    }

    const auto& bridges = impl_->nodes[sourceNodeId].bridges;
    const auto found = std::find_if(
        bridges.begin(),
        bridges.end(),
        [targetNodeId](const SpatialBridge& bridge) {
            return bridge.targetNodeId ==
                static_cast<int>(targetNodeId);
        });

    if (found == bridges.end()) {
        return AdaptiveMesh::ProductionTransitionLiveSnapshot{
            sourceNodeId,
            targetNodeId,
            impl_->transitionStateVersion,
            false,
            std::nullopt
        };
    }

    return AdaptiveMesh::ProductionTransitionLiveSnapshot{
        sourceNodeId,
        targetNodeId,
        impl_->transitionStateVersion,
        true,
        found->generation
    };
}
