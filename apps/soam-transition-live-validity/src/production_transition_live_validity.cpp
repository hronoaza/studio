#include "production_transition_live_validity.hpp"
#include "detail/production_transition_live_validity_internal.hpp"

#include <array>
#include <atomic>
#include <cerrno>
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

std::atomic<LiveValidityOpaqueIdFillFunction> testFillFunction{nullptr};
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
        const auto result = ::getrandom(
            out.data() + offset,out.size() - offset,0);
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
tryGenerateLiveValidityOpaqueIdBytes() noexcept
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

void setLiveValidityOpaqueIdFillFunctionForTesting(
    LiveValidityOpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction,std::memory_order_release);
}

void resetLiveValidityOpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr,std::memory_order_release);
    RecentIdsGuard guard;
    recentIds = {};
    recentIdsSize = 0;
    nextRecentId = 0;
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {
namespace {

template<std::size_t N>
[[nodiscard]] bool nonZero(const std::array<std::uint8_t,N>& bytes) noexcept {
    for (const auto byte : bytes) {
        if (byte != 0U) return true;
    }
    return false;
}

[[nodiscard]] std::uint64_t reasonFlag(
    TransitionLiveValidityReason reason) noexcept
{
    return std::uint64_t{1}
        << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool requestLineageIsConsistent(
    const ProductionDerivedTransitionRequest& request) noexcept
{
    const auto& descriptor = request.policyDescriptor();
    const auto& binding = request.binding();
    const auto& relation = binding.relationship();
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
            return binding.direction() == RequestedTransitionDirection::support;
        case PersistentBridgeRecommendation::CONSTRAIN:
            return binding.direction() == RequestedTransitionDirection::constrain;
        case PersistentBridgeRecommendation::PRESERVE:
            return false;
    }

    return false;
}

} // namespace

std::optional<ProductionTransitionLiveValidityResult>
ProductionTransitionLiveValidityEvaluator::evaluate(
    const ProductionDerivedTransitionRequest& request) const
{
    const auto parentBytes = detail::tryGenerateLiveValidityOpaqueIdBytes();
    if (!parentBytes.has_value()) return std::nullopt;

    LiveValidityDecisionId parentId{*parentBytes};

    const auto reject = [&](TransitionLiveValidityReason reason)
        -> std::optional<ProductionTransitionLiveValidityResult>
    {
        return ProductionTransitionLiveValidityResult{
            ProductionTransitionLiveValidityRejection{
                parentId,
                request.decisionId(),
                reason,
                reasonFlag(reason)
            }
        };
    };

    if (!requestLineageIsConsistent(request)) {
        return reject(
            TransitionLiveValidityReason::RequestLineageInconsistent);
    }

    const auto& relation = request.binding().relationship();
    if (relation.sourceNodeId() == relation.targetNodeId()) {
        return reject(
            TransitionLiveValidityReason::InvalidRelationshipIdentity);
    }

    const auto freshnessBytes =
        detail::tryGenerateLiveValidityOpaqueIdBytes();
    const auto revalidationBytes =
        detail::tryGenerateLiveValidityOpaqueIdBytes();

    if (!freshnessBytes.has_value() || !revalidationBytes.has_value()) {
        return reject(
            TransitionLiveValidityReason::InternalDecisionFailure);
    }

    const auto snapshot = source_.capture(
        relation.sourceNodeId(),
        relation.targetNodeId());

    if (!snapshot.has_value()) {
        return reject(
            TransitionLiveValidityReason::SnapshotUnavailable);
    }

    if (snapshot->sourceNodeId() != relation.sourceNodeId() ||
        snapshot->targetNodeId() != relation.targetNodeId()) {
        return reject(
            TransitionLiveValidityReason::SnapshotUnavailable);
    }

    const bool fresh =
        snapshot->stateVersion() == request.stateVersion();

    const bool revalidated =
        snapshot->relationshipPresent() &&
        snapshot->relationshipGeneration().has_value() &&
        *snapshot->relationshipGeneration() == relation.generation();

    ProductionFreshnessPrerequisiteRecord freshness{
        FreshnessDecisionId{*freshnessBytes},
        request.binding(),
        snapshot->stateVersion(),
        fresh
    };

    ProductionRevalidationPrerequisiteRecord revalidation{
        RevalidationDecisionId{*revalidationBytes},
        request.binding(),
        snapshot->relationshipPresent(),
        snapshot->relationshipGeneration(),
        revalidated
    };

    try {
        return ProductionTransitionLiveValidityResult{
            ProductionTransitionLiveValidityEvidence{
                std::move(parentId),
                request,
                std::move(freshness),
                std::move(revalidation)
            }
        };
    } catch (...) {
        return reject(
            TransitionLiveValidityReason::InternalDecisionFailure);
    }
}

} // namespace AdaptiveMesh
