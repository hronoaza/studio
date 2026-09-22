#include "production_transition_invariant.hpp"
#include "detail/production_transition_invariant_internal.hpp"
#include "transition_invariant_implementation_revision.hpp"
#include "system_architecture.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

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

std::atomic<InvariantOpaqueIdFillFunction> testFillFunction{nullptr};
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
    for (const auto byte : bytes) {
        if (byte != 0U) return false;
    }
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
        nullptr,
        reinterpret_cast<PUCHAR>(out.data()),
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
tryGenerateInvariantOpaqueIdBytes() noexcept
{
    for (std::size_t attempt=0; attempt<kMaxAttempts; ++attempt) {
        std::array<std::uint8_t,16> candidate{};
        const auto fill = testFillFunction.load(std::memory_order_acquire);
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

void setInvariantOpaqueIdFillFunctionForTesting(
    InvariantOpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction,std::memory_order_release);
}

void resetInvariantOpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr,std::memory_order_release);
    RecentIdsGuard guard;
    recentIds = {};
    recentIdsSize = 0;
    nextRecentId = 0;
}

double projectInvariantCapacityForTesting(
    double current,
    RequestedTransitionDirection direction) noexcept
{
    constexpr double fraction = 0.125;
    return direction == RequestedTransitionDirection::support
        ? current + (1.0-current)*fraction
        : current*(1.0-fraction);
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {
namespace {

constexpr InvariantPolicyId::Bytes kPolicyId{
    0x49,0x4e,0x56,0x50,0x52,0x4f,0x4a,0x31,
    0xa2,0x10,0x7c,0x33,0x91,0x56,0xd4,0x08
};

constexpr double kAdjustmentFraction = 0.125;

[[nodiscard]] constexpr std::uint8_t hexNibble(char value) noexcept {
    if (value >= '0' && value <= '9') {
        return static_cast<std::uint8_t>(value - '0');
    }
    if (value >= 'a' && value <= 'f') {
        return static_cast<std::uint8_t>(value - 'a' + 10);
    }
    if (value >= 'A' && value <= 'F') {
        return static_cast<std::uint8_t>(value - 'A' + 10);
    }
    return 0xffU;
}

[[nodiscard]] constexpr std::array<std::uint8_t,32>
implementationRevisionDigest() noexcept
{
    constexpr const char* hex =
        SOAM_TRANSITION_INVARIANT_IMPLEMENTATION_REVISION_SHA;
    std::array<std::uint8_t,32> digest{};
    for (std::size_t i=0; i<digest.size(); ++i) {
        const auto high = hexNibble(hex[i*2U]);
        const auto low = hexNibble(hex[i*2U+1U]);
        digest[i] = static_cast<std::uint8_t>((high << 4U) | low);
    }
    return digest;
}

constexpr auto kImplementationRevision = implementationRevisionDigest();

template<std::size_t N>
[[nodiscard]] bool nonZero(const std::array<std::uint8_t,N>& bytes) noexcept {
    for (const auto byte : bytes) {
        if (byte != 0U) return true;
    }
    return false;
}

[[nodiscard]] std::uint64_t reasonFlag(TransitionInvariantReason reason) noexcept {
    return std::uint64_t{1} << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool requestLineageIsConsistent(
    const ProductionDerivedTransitionRequest& request) noexcept
{
    const auto& descriptor = request.policyDescriptor();
    const auto& relation = request.binding().relationship();
    const auto& key = request.persistenceStreamKey();

    if (!nonZero(request.decisionId().bytes()) ||
        !nonZero(request.persistenceObservationDecisionId().bytes()) ||
        !nonZero(request.persistenceStreamInstanceId().bytes()) ||
        !nonZero(request.policyEvidenceDecisionId().bytes()) ||
        !nonZero(request.sourceCaptureId().bytes()) ||
        !nonZero(descriptor.policyId.bytes()) ||
        !nonZero(descriptor.implementationRevision) ||
        request.stateVersion() == 0U ||
        descriptor.majorVersion != 1U ||
        descriptor.minorVersion != 0U ||
        descriptor.implementationRevisionKind != 1U) {
        return false;
    }

    if (relation.sourceNodeId() != key.sourceNodeId() ||
        relation.targetNodeId() != key.targetNodeId() ||
        relation.generation() != key.relationshipGeneration()) {
        return false;
    }

    switch (request.sourceRecommendation()) {
        case PersistentBridgeRecommendation::SUPPORT:
            return request.binding().direction() ==
                RequestedTransitionDirection::support;
        case PersistentBridgeRecommendation::CONSTRAIN:
            return request.binding().direction() ==
                RequestedTransitionDirection::constrain;
        case PersistentBridgeRecommendation::PRESERVE:
            return false;
    }
    return false;
}

[[nodiscard]] bool recognizedPolicy(
    const TransitionInvariantPolicySnapshot& policy) noexcept
{
    const auto& d = policy.descriptor();
    return d.policyId.bytes() == kPolicyId &&
        d.majorVersion == 1U &&
        d.minorVersion == 0U &&
        d.implementationRevisionKind == 1U &&
        d.implementationRevision == kImplementationRevision &&
        policy.adjustmentFraction() == kAdjustmentFraction &&
        policy.transitionClass() ==
            detail::ProductionTransitionInvariantPolicyAccess::
                bridgeCouplingAdjustmentV1();
}

[[nodiscard]] bool finiteUnit(double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

[[nodiscard]] bool validBridgeInput(
    const ProductionTransitionInvariantBridgeSnapshot& bridge) noexcept
{
    return finiteUnit(bridge.capacity()) &&
        std::isfinite(bridge.distance()) &&
        bridge.distance() >= 0.0 &&
        finiteUnit(bridge.orientationWeight());
}

[[nodiscard]] double projectCapacity(
    double current,
    RequestedTransitionDirection direction) noexcept
{
    if (direction == RequestedTransitionDirection::support) {
        return current + (1.0-current)*kAdjustmentFraction;
    }
    return current*(1.0-kAdjustmentFraction);
}

[[nodiscard]] bool endpointInvariantSatisfied(
    double state,
    double baseline,
    double maxEpsilon) noexcept
{
    return std::isfinite(state) &&
        std::isfinite(baseline) &&
        std::isfinite(maxEpsilon) &&
        maxEpsilon > 0.0 &&
        std::abs(state-baseline) <= maxEpsilon;
}

[[nodiscard]] bool effectiveCouplingValid(
    double projectedCapacity,
    double distance,
    double orientationWeight) noexcept
{
    const double value =
        projectedCapacity *
        (1.0/(1.0+0.1*distance)) *
        orientationWeight;
    return finiteUnit(value);
}

} // namespace

std::optional<TransitionInvariantPolicySnapshot>
ProductionTransitionInvariantPolicyProvider::createCurrent()
{
    const auto bytes = detail::tryGenerateInvariantOpaqueIdBytes();
    if (!bytes.has_value()) return std::nullopt;

    return TransitionInvariantPolicySnapshot{
        InvariantPolicySnapshotId{*bytes},
        InvariantPolicyDescriptor{
            InvariantPolicyId{kPolicyId},
            1U,0U,1U,kImplementationRevision
        },
        kAdjustmentFraction,
        detail::ProductionTransitionInvariantPolicyAccess::
            bridgeCouplingAdjustmentV1()
    };
}

std::optional<ProductionTransitionInvariantResult>
ProductionTransitionInvariantEvaluator::evaluate(
    const ProductionDerivedTransitionRequest& request,
    const TransitionInvariantPolicySnapshot& policy) const
{
    const auto decisionBytes = detail::tryGenerateInvariantOpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;

    InvariantDecisionId decisionId{*decisionBytes};

    const auto reject = [&](TransitionInvariantReason reason)
        -> std::optional<ProductionTransitionInvariantResult>
    {
        return ProductionTransitionInvariantResult{
            ProductionTransitionInvariantRejection{
                decisionId,
                request.decisionId(),
                reason,
                reasonFlag(reason)
            }
        };
    };

    if (!requestLineageIsConsistent(request)) {
        return reject(TransitionInvariantReason::RequestLineageInconsistent);
    }

    if (!recognizedPolicy(policy)) {
        return reject(TransitionInvariantReason::PolicyRevisionUnrecognized);
    }

    if (request.binding().transitionClass() != policy.transitionClass()) {
        return reject(TransitionInvariantReason::TransitionClassUnsupported);
    }

    const auto& relation = request.binding().relationship();
    const auto snapshot = source_.capture(
        relation.sourceNodeId(), relation.targetNodeId());

    if (!snapshot.has_value()) {
        return reject(TransitionInvariantReason::SnapshotUnavailable);
    }

    if (snapshot->stateVersion() != request.stateVersion()) {
        return reject(TransitionInvariantReason::RequestStateVersionMismatch);
    }

    const auto& forward = snapshot->forwardBridge();
    const auto& reverse = snapshot->reverseBridge();

    if (!forward.has_value() || !reverse.has_value() ||
        snapshot->sourceNodeId() != relation.sourceNodeId() ||
        snapshot->targetNodeId() != relation.targetNodeId() ||
        forward->targetNodeId() != relation.targetNodeId() ||
        reverse->targetNodeId() != relation.sourceNodeId() ||
        forward->generation() != relation.generation() ||
        reverse->generation() != relation.generation() ||
        forward->distance() != reverse->distance()) {
        return reject(TransitionInvariantReason::RelationshipIdentityMismatch);
    }

    if (forward->status() == BridgeStatus::ISOLATED ||
        reverse->status() == BridgeStatus::ISOLATED ||
        !validBridgeInput(*forward) ||
        !validBridgeInput(*reverse)) {
        return reject(TransitionInvariantReason::ProjectionUnavailable);
    }

    const double forwardProjected =
        projectCapacity(forward->capacity(),request.binding().direction());
    const double reverseProjected =
        projectCapacity(reverse->capacity(),request.binding().direction());

    if (!finiteUnit(forwardProjected) ||
        !finiteUnit(reverseProjected)) {
        return reject(TransitionInvariantReason::ProjectionUnavailable);
    }

    const bool monotonic =
        request.binding().direction() == RequestedTransitionDirection::support
        ? (forwardProjected >= forward->capacity() &&
           reverseProjected >= reverse->capacity())
        : (forwardProjected <= forward->capacity() &&
           reverseProjected <= reverse->capacity());

    const bool forwardCouplingValid = effectiveCouplingValid(
        forwardProjected,forward->distance(),forward->orientationWeight());
    const bool reverseCouplingValid = effectiveCouplingValid(
        reverseProjected,reverse->distance(),reverse->orientationWeight());

    const bool sourceInvariant = endpointInvariantSatisfied(
        snapshot->sourceState(),
        snapshot->sourceInvariantBaseline(),
        snapshot->sourceInvariantMaxEpsilon());
    const bool targetInvariant = endpointInvariantSatisfied(
        snapshot->targetState(),
        snapshot->targetInvariantBaseline(),
        snapshot->targetInvariantMaxEpsilon());

    const bool structural = true;
    const bool satisfied =
        structural &&
        monotonic &&
        forwardCouplingValid &&
        reverseCouplingValid &&
        sourceInvariant &&
        targetInvariant;

    return ProductionTransitionInvariantResult{
        ProductionInvariantPrerequisiteRecord{
            std::move(decisionId),
            request.binding(),
            policy.snapshotId(),
            satisfied,
            sourceInvariant,
            targetInvariant,
            structural,
            forwardProjected,
            reverseProjected,
            forwardCouplingValid,
            reverseCouplingValid
        }
    };
}

} // namespace AdaptiveMesh
