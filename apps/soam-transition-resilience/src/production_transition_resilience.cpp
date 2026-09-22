#include "production_transition_resilience.hpp"
#include "detail/production_transition_resilience_internal.hpp"
#include "transition_resilience_implementation_revision.hpp"
#include "system_architecture.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#elif defined(__linux__)
#include <sys/random.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <cstdlib>
#endif

namespace AdaptiveMesh::detail {
namespace {

constexpr std::size_t kMaxAttempts = 8;
constexpr std::size_t kRecentCount = 128;

std::atomic<ResilienceOpaqueIdFillFunction> testFillFunction{nullptr};
std::atomic_flag recentIdsLock = ATOMIC_FLAG_INIT;
std::array<std::array<std::uint8_t,16>,kRecentCount> recentIds{};
std::size_t recentIdsSize = 0;
std::size_t nextRecentId = 0;

class RecentIdsGuard final {
public:
    RecentIdsGuard() noexcept {
        while (recentIdsLock.test_and_set(std::memory_order_acquire)) {}
    }
    ~RecentIdsGuard() {
        recentIdsLock.clear(std::memory_order_release);
    }
private:
    RecentIdsGuard(const RecentIdsGuard&) = delete;
    RecentIdsGuard& operator=(const RecentIdsGuard&) = delete;
};

[[nodiscard]] bool allZero(const std::array<std::uint8_t,16>& bytes) noexcept {
    for (const auto byte : bytes) if (byte != 0U) return false;
    return true;
}

[[nodiscard]] bool reserveIfNotRecent(
    const std::array<std::uint8_t,16>& bytes) noexcept
{
    RecentIdsGuard guard;
    for (std::size_t i=0; i<recentIdsSize; ++i) {
        if (recentIds[i] == bytes) return false;
    }
    if (recentIdsSize < kRecentCount) {
        recentIds[recentIdsSize++] = bytes;
    } else {
        recentIds[nextRecentId] = bytes;
        nextRecentId = (nextRecentId + 1U) % kRecentCount;
    }
    return true;
}

[[nodiscard]] bool fillFromOperatingSystem(
    std::array<std::uint8_t,16>& out) noexcept
{
#if defined(_WIN32)
    const NTSTATUS status = BCryptGenRandom(
        nullptr,reinterpret_cast<PUCHAR>(out.data()),
        static_cast<ULONG>(out.size()),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status >= 0;
#elif defined(__linux__)
    std::size_t offset = 0;
    while (offset < out.size()) {
        const auto result = ::getrandom(out.data()+offset,out.size()-offset,0);
        if (result > 0) {
            offset += static_cast<std::size_t>(result);
            continue;
        }
        if (result < 0 && errno == EINTR) continue;
        return false;
    }
    return true;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    ::arc4random_buf(out.data(),out.size());
    return true;
#else
    static_cast<void>(out);
    return false;
#endif
}

} // namespace

std::optional<std::array<std::uint8_t,16>>
tryGenerateResilienceOpaqueIdBytes() noexcept
{
    for (std::size_t attempt=0; attempt<kMaxAttempts; ++attempt) {
        std::array<std::uint8_t,16> candidate{};
        const auto fill=testFillFunction.load(std::memory_order_acquire);
        if (fill != nullptr) {
            if (!fill(candidate)) return std::nullopt;
        } else if (!fillFromOperatingSystem(candidate)) {
            return std::nullopt;
        }
        if (allZero(candidate)) continue;
        if (!reserveIfNotRecent(candidate)) continue;
        return candidate;
    }
    return std::nullopt;
}

void setResilienceOpaqueIdFillFunctionForTesting(
    ResilienceOpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction,std::memory_order_release);
}

void resetResilienceOpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr,std::memory_order_release);
    RecentIdsGuard guard;
    recentIds={};
    recentIdsSize=0;
    nextRecentId=0;
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {
namespace {

constexpr ResiliencePolicyId::Bytes kPolicyId{
    0x52,0x45,0x53,0x49,0x4c,0x56,0x31,0x01,
    0x93,0x34,0xb1,0x72,0x5d,0x28,0xe0,0x44
};

[[nodiscard]] constexpr std::uint8_t hexNibble(char value) noexcept {
    if (value >= '0' && value <= '9') return static_cast<std::uint8_t>(value-'0');
    if (value >= 'a' && value <= 'f') return static_cast<std::uint8_t>(value-'a'+10);
    if (value >= 'A' && value <= 'F') return static_cast<std::uint8_t>(value-'A'+10);
    return 0xffU;
}

[[nodiscard]] constexpr std::array<std::uint8_t,32>
implementationRevisionDigest() noexcept
{
    constexpr const char* hex =
        SOAM_TRANSITION_RESILIENCE_IMPLEMENTATION_REVISION_SHA;
    std::array<std::uint8_t,32> digest{};
    for (std::size_t i=0; i<digest.size(); ++i) {
        const auto high=hexNibble(hex[i*2U]);
        const auto low=hexNibble(hex[i*2U+1U]);
        digest[i]=static_cast<std::uint8_t>((high<<4U)|low);
    }
    return digest;
}

constexpr auto kImplementationRevision=implementationRevisionDigest();

template<std::size_t N>
[[nodiscard]] bool nonZero(const std::array<std::uint8_t,N>& bytes) noexcept {
    for (const auto byte:bytes) if (byte != 0U) return true;
    return false;
}

[[nodiscard]] std::uint64_t reasonFlag(TransitionResilienceReason reason) noexcept {
    return std::uint64_t{1} << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool requestLineageIsConsistent(
    const ProductionDerivedTransitionRequest& request) noexcept
{
    const auto& descriptor=request.policyDescriptor();
    const auto& relation=request.binding().relationship();
    const auto& key=request.persistenceStreamKey();

    if (!nonZero(request.decisionId().bytes()) ||
        !nonZero(request.persistenceObservationDecisionId().bytes()) ||
        !nonZero(request.persistenceStreamInstanceId().bytes()) ||
        !nonZero(request.policyEvidenceDecisionId().bytes()) ||
        !nonZero(request.sourceCaptureId().bytes()) ||
        !nonZero(descriptor.policyId.bytes()) ||
        !nonZero(descriptor.implementationRevision) ||
        request.stateVersion()==0U ||
        descriptor.majorVersion!=1U ||
        descriptor.minorVersion!=0U ||
        descriptor.implementationRevisionKind!=1U) {
        return false;
    }

    if (relation.sourceNodeId()!=key.sourceNodeId() ||
        relation.targetNodeId()!=key.targetNodeId() ||
        relation.generation()!=key.relationshipGeneration()) {
        return false;
    }

    switch (request.sourceRecommendation()) {
        case PersistentBridgeRecommendation::SUPPORT:
            return request.binding().direction()==RequestedTransitionDirection::support;
        case PersistentBridgeRecommendation::CONSTRAIN:
            return request.binding().direction()==RequestedTransitionDirection::constrain;
        case PersistentBridgeRecommendation::PRESERVE:
            return false;
    }
    return false;
}

[[nodiscard]] bool recognizedPolicy(
    const TransitionResiliencePolicySnapshot& policy) noexcept
{
    const auto& d=policy.descriptor();
    return d.policyId.bytes()==kPolicyId &&
        d.majorVersion==1U &&
        d.minorVersion==0U &&
        d.implementationRevisionKind==1U &&
        d.implementationRevision==kImplementationRevision &&
        policy.criterion()==
            ResilienceCriterion::single_request_relationship_loss_survivability &&
        policy.transitionClass()==
            detail::ProductionTransitionResiliencePolicyAccess::
                bridgeCouplingAdjustmentV1();
}

[[nodiscard]] bool finiteUnit(double value) noexcept {
    return std::isfinite(value) && value>=0.0 && value<=1.0;
}

[[nodiscard]] bool recognizedStatus(BridgeStatus status) noexcept {
    return status==BridgeStatus::NORMAL ||
        status==BridgeStatus::DAMPING ||
        status==BridgeStatus::RECOVERY ||
        status==BridgeStatus::ISOLATED;
}

struct PairValidation final {
    bool structuralValid=false;
    bool operational=false;
    std::uint64_t generation=0;
};

[[nodiscard]] PairValidation validatePair(
    const ProductionTransitionResilienceRelationshipSnapshot& pair) noexcept
{
    if (pair.nodeA()>=pair.nodeB() ||
        pair.aToB().size()!=1U ||
        pair.bToA().size()!=1U) {
        return {};
    }

    const auto& f=pair.aToB().front();
    const auto& r=pair.bToA().front();

    if (f.sourceNodeId()!=pair.nodeA() ||
        f.targetNodeId()!=pair.nodeB() ||
        r.sourceNodeId()!=pair.nodeB() ||
        r.targetNodeId()!=pair.nodeA() ||
        f.generation()==0U ||
        f.generation()!=r.generation() ||
        f.distance()!=r.distance() ||
        !std::isfinite(f.distance()) ||
        f.distance()<0.0 ||
        !finiteUnit(f.capacity()) ||
        !finiteUnit(r.capacity()) ||
        !finiteUnit(f.orientationWeight()) ||
        !finiteUnit(r.orientationWeight()) ||
        !recognizedStatus(f.status()) ||
        !recognizedStatus(r.status())) {
        return {};
    }

    const bool operational=
        f.status()!=BridgeStatus::ISOLATED &&
        r.status()!=BridgeStatus::ISOLATED &&
        f.capacity()>0.0 &&
        r.capacity()>0.0;

    return {true,operational,f.generation()};
}

[[nodiscard]] bool containsNode(
    const std::unordered_set<std::size_t>& nodes,
    std::size_t id) noexcept
{
    return nodes.find(id)!=nodes.end();
}

} // namespace

std::optional<TransitionResiliencePolicySnapshot>
ProductionTransitionResiliencePolicyProvider::createCurrent()
{
    const auto bytes=detail::tryGenerateResilienceOpaqueIdBytes();
    if (!bytes.has_value()) return std::nullopt;

    return TransitionResiliencePolicySnapshot{
        ResiliencePolicySnapshotId{*bytes},
        ResiliencePolicyDescriptor{
            ResiliencePolicyId{kPolicyId},
            1U,0U,1U,kImplementationRevision
        },
        ResilienceCriterion::single_request_relationship_loss_survivability,
        detail::ProductionTransitionResiliencePolicyAccess::
            bridgeCouplingAdjustmentV1()
    };
}

std::optional<ProductionTransitionResilienceResult>
ProductionTransitionResilienceEvaluator::evaluate(
    const ProductionDerivedTransitionRequest& request,
    const TransitionResiliencePolicySnapshot& policy) const
{
    const auto decisionBytes=detail::tryGenerateResilienceOpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;

    ResilienceDecisionId decisionId{*decisionBytes};

    const auto reject=[&](TransitionResilienceReason reason)
        -> std::optional<ProductionTransitionResilienceResult>
    {
        return ProductionTransitionResilienceResult{
            ProductionTransitionResilienceRejection{
                decisionId,request.decisionId(),reason,reasonFlag(reason)
            }
        };
    };

    if (!requestLineageIsConsistent(request)) {
        return reject(TransitionResilienceReason::RequestLineageInconsistent);
    }

    if (request.binding().transitionClass()!=
        detail::ProductionTransitionResiliencePolicyAccess::
            bridgeCouplingAdjustmentV1()) {
        return reject(TransitionResilienceReason::TransitionClassUnsupported);
    }

    if (!recognizedPolicy(policy)) {
        return reject(TransitionResilienceReason::PolicyRevisionUnrecognized);
    }

    if (request.binding().transitionClass()!=policy.transitionClass()) {
        return reject(TransitionResilienceReason::TransitionClassUnsupported);
    }

    const auto snapshot=source_.capture();
    if (!snapshot.has_value()) {
        return reject(TransitionResilienceReason::SnapshotUnavailable);
    }

    if (snapshot->stateVersion()!=request.stateVersion()) {
        return reject(TransitionResilienceReason::RequestStateVersionMismatch);
    }

    std::unordered_set<std::size_t> nodes;
    nodes.reserve(snapshot->nodeIds().size());
    for (const auto id:snapshot->nodeIds()) {
        if (!nodes.insert(id).second) {
            return reject(TransitionResilienceReason::TopologySnapshotInvalid);
        }
    }

    const auto& requested=request.binding().relationship();
    if (!containsNode(nodes,requested.sourceNodeId()) ||
        !containsNode(nodes,requested.targetNodeId()) ||
        requested.sourceNodeId()==requested.targetNodeId()) {
        return reject(TransitionResilienceReason::RelationshipIdentityMismatch);
    }

    const auto requestA=std::min(
        requested.sourceNodeId(),requested.targetNodeId());
    const auto requestB=std::max(
        requested.sourceNodeId(),requested.targetNodeId());

    const ProductionTransitionResilienceRelationshipSnapshot* requestPair=nullptr;
    std::unordered_set<std::uint64_t> pairKeys;
    pairKeys.reserve(snapshot->relationships().size());

    auto pairKey=[](std::size_t a,std::size_t b) noexcept {
        return (static_cast<std::uint64_t>(a)<<32U) ^
            static_cast<std::uint64_t>(b);
    };

    for (const auto& pair:snapshot->relationships()) {
        if (!containsNode(nodes,pair.nodeA()) ||
            !containsNode(nodes,pair.nodeB()) ||
            pair.nodeA()>=pair.nodeB()) {
            return reject(TransitionResilienceReason::TopologySnapshotInvalid);
        }
        if (!pairKeys.insert(pairKey(pair.nodeA(),pair.nodeB())).second) {
            return reject(TransitionResilienceReason::TopologySnapshotInvalid);
        }
        if (pair.nodeA()==requestA && pair.nodeB()==requestB) {
            requestPair=&pair;
        }
    }

    if (requestPair==nullptr ||
        requestPair->aToB().size()!=1U ||
        requestPair->bToA().size()!=1U) {
        return reject(TransitionResilienceReason::RelationshipIdentityMismatch);
    }

    const auto requestValidation=validatePair(*requestPair);
    if (!requestValidation.structuralValid) {
        const auto& f=requestPair->aToB().front();
        const auto& r=requestPair->bToA().front();
        if (f.generation()!=requested.generation() ||
            r.generation()!=requested.generation()) {
            return reject(TransitionResilienceReason::RelationshipIdentityMismatch);
        }
        return reject(TransitionResilienceReason::TopologySnapshotInvalid);
    }

    if (requestValidation.generation!=requested.generation()) {
        return reject(TransitionResilienceReason::RelationshipIdentityMismatch);
    }

    if (!requestValidation.operational) {
        return reject(TransitionResilienceReason::TopologySnapshotInvalid);
    }

    std::unordered_map<std::size_t,std::vector<std::size_t>> adjacency;
    adjacency.reserve(nodes.size());
    for (const auto id:nodes) adjacency.emplace(id,std::vector<std::size_t>{});

    for (const auto& pair:snapshot->relationships()) {
        const auto validation=validatePair(pair);
        if (!validation.structuralValid) {
            return reject(TransitionResilienceReason::TopologySnapshotInvalid);
        }

        if (pair.nodeA()==requestA && pair.nodeB()==requestB) {
            continue;
        }

        if (!validation.operational) continue;

        adjacency[pair.nodeA()].push_back(pair.nodeB());
        adjacency[pair.nodeB()].push_back(pair.nodeA());
    }

    for (auto& [id,neighbors]:adjacency) {
        static_cast<void>(id);
        std::sort(neighbors.begin(),neighbors.end());
    }

    std::queue<std::pair<std::size_t,std::size_t>> queue;
    std::unordered_set<std::size_t> visited;
    visited.reserve(nodes.size());
    queue.push({requested.sourceNodeId(),0U});
    visited.insert(requested.sourceNodeId());

    std::size_t hopCount=0U;
    bool found=false;

    while (!queue.empty()) {
        const auto [node,hops]=queue.front();
        queue.pop();

        if (node==requested.targetNodeId()) {
            found=true;
            hopCount=hops;
            break;
        }

        const auto it=adjacency.find(node);
        if (it==adjacency.end()) continue;

        for (const auto next:it->second) {
            if (visited.insert(next).second) {
                queue.push({next,hops+1U});
            }
        }
    }

    if (!found) hopCount=0U;

    return ProductionTransitionResilienceResult{
        ProductionResiliencePrerequisiteRecord{
            std::move(decisionId),
            request.binding(),
            policy.snapshotId(),
            found,
            hopCount
        }
    };
}

} // namespace AdaptiveMesh
