#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <cstdint>
#include <utility>
#include <vector>

namespace AdaptiveMesh {

class ProductionTransitionEvaluator;
namespace detail { class ProductionTransitionEvaluationBindingState; }

void requireFinite(double value, const char* name);

struct Vector3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    void validate() const;
    [[nodiscard]] double distanceTo(const Vector3D& other) const;
    [[nodiscard]] double orientationFactorTo(const Vector3D& target) const;
};

struct IdentityInvariant {
    double baseline = 1.6180339887;
    double maxEpsilon = 10.0;

    void validate() const;
    [[nodiscard]] bool isWithinSafetyBound(double state) const;
};

enum class SignalCategory { NOISE, CREATIVE_SIGNAL, DESTRUCTIVE_DRIFT };
enum class BridgeStatus { NORMAL, DAMPING, RECOVERY, ISOLATED };

class MetaEvaluator {
public:
    [[nodiscard]] SignalCategory evaluate(
        double candidateState,
        double healthIndex,
        const IdentityInvariant& omega) const;
};

struct SpatialBridge {
    int targetNodeId;
    double distance;
    double orientationWeight;
    double capacity = 1.0;
    BridgeStatus status = BridgeStatus::NORMAL;
    std::uint64_t generation = 0;

    void updateBridgeState(SignalCategory category);
    [[nodiscard]] double getEffectiveCoupling() const;
    [[nodiscard]] double getEffectiveTransmission() const;
};

class AutopoieticNode {
public:
    std::size_t id;
    Vector3D position;
    std::atomic<double> state;
    std::atomic<double> healthIndex{1.0};
    IdentityInvariant invariant;
    MetaEvaluator metaEvaluator;
    std::vector<SpatialBridge> bridges;
    mutable std::mutex nodeMutex;

    AutopoieticNode(std::size_t nodeId, Vector3D pos, double initialBaseline);
    AutopoieticNode(const AutopoieticNode& other);

    void updateHealth();
    [[nodiscard]] double applyLocalReflexFilter(double rawInput) const;
};

class SpatialAdaptiveMesh {
public:
    explicit SpatialAdaptiveMesh(std::size_t maxWorkers = 0);
    ~SpatialAdaptiveMesh();

    SpatialAdaptiveMesh(const SpatialAdaptiveMesh&) = delete;
    SpatialAdaptiveMesh& operator=(const SpatialAdaptiveMesh&) = delete;
    SpatialAdaptiveMesh(SpatialAdaptiveMesh&&) = delete;
    SpatialAdaptiveMesh& operator=(SpatialAdaptiveMesh&&) = delete;

    void addNode(std::size_t id, Vector3D pos, double baseline);
    void connectNodes(int nodeA, int nodeB);
    void connectNodePairs(const std::vector<std::pair<int, int>>& connections);
    void enforceStabilityCondition() noexcept;
    void pruneIsolatedBridges(double minCapacityThreshold = 0.05);
    void autoConnectNearbyNodes(double radius);
    void injectExternalShock(int targetNodeId, double shockMagnitude);

    void simulationStep();

    // Compatibility name retained from 1.1. This call is blocking in Phase B.
    void simulationStepAsync();

    [[nodiscard]] double getNodeState(std::size_t id) const;
    [[nodiscard]] double getNodeHealth(std::size_t id) const;
    [[nodiscard]] std::size_t getNodeBridgesCount(std::size_t id) const;

    [[nodiscard]] ProductionTransitionEvaluator
    productionTransitionEvaluator() const noexcept;

    [[nodiscard]] std::optional<ProductionRelationshipProvenance>
    captureProductionRelationshipProvenance(
        const ProductionTransitionEvaluationLocator& locator) const;

private:
    struct TransitionSnapshot {
        std::size_t sourceNodeId;
        std::size_t targetNodeId;
        std::uint64_t relationshipGeneration;
        std::uint64_t stateVersion;
    };

    [[nodiscard]] bool transitionLocatorIsValid(
        std::size_t sourceNodeId,
        std::size_t targetNodeId) const noexcept;

    [[nodiscard]] std::optional<TransitionSnapshot> captureTransitionSnapshot(
        std::size_t sourceNodeId,
        std::size_t targetNodeId) const;

    [[nodiscard]] bool revalidateTransitionSnapshot(
        const TransitionSnapshot& snapshot) const;

    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::shared_ptr<detail::ProductionTransitionEvaluationBindingState>
        transitionEvaluationBinding_;

    friend class ProductionTransitionEvaluator;
};

} // namespace AdaptiveMesh
