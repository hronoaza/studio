#include "production_transition_request.hpp"
#include "detail/production_transition_request_internal.hpp"
#include "transition_request_implementation_revision.hpp"

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

std::atomic<TransitionRequestOpaqueIdFillFunction> testFillFunction{nullptr};
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

[[nodiscard]] bool allZero(
    const std::array<std::uint8_t,16>& bytes) noexcept
{
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
            out.data() + offset, out.size() - offset, 0);
        if (result > 0) {
            offset += static_cast<std::size_t>(result);
            continue;
        }
        if (result < 0 && errno == EINTR) continue;
        return false;
    }
    return true;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    ::arc4random_buf(out.data(), out.size());
    return true;
#else
    static_cast<void>(out);
    return false;
#endif
}

} // namespace

std::optional<std::array<std::uint8_t,16>>
tryGenerateTransitionRequestOpaqueIdBytes() noexcept
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

void setTransitionRequestOpaqueIdFillFunctionForTesting(
    TransitionRequestOpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction, std::memory_order_release);
}

void resetTransitionRequestOpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr, std::memory_order_release);
    RecentIdsGuard guard;
    recentIds = {};
    recentIdsSize = 0;
    nextRecentId = 0;
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {
namespace {

constexpr TransitionRequestPolicyId::Bytes kPolicyId{
    0x54,0x52,0x51,0x56,0x31,0x9d,0x47,0x2a,
    0xa8,0x14,0x3f,0x62,0xc1,0x75,0x20,0x0d
};

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
        SOAM_TRANSITION_REQUEST_IMPLEMENTATION_REVISION_SHA;
    std::array<std::uint8_t,32> digest{};
    for (std::size_t i=0; i<digest.size(); ++i) {
        const auto high = hexNibble(hex[i * 2U]);
        const auto low = hexNibble(hex[i * 2U + 1U]);
        digest[i] = static_cast<std::uint8_t>((high << 4U) | low);
    }
    return digest;
}

constexpr auto kImplementationRevision = implementationRevisionDigest();

template<std::size_t N>
[[nodiscard]] bool nonZero(
    const std::array<std::uint8_t,N>& bytes) noexcept
{
    for (const auto byte : bytes) {
        if (byte != 0U) return true;
    }
    return false;
}

[[nodiscard]] std::uint64_t reasonFlag(
    TransitionRequestDerivationReason reason) noexcept
{
    return std::uint64_t{1}
        << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool recognizedPolicy(
    const TransitionRequestPolicySnapshot& policy) noexcept
{
    const auto& descriptor = policy.descriptor();
    return
        descriptor.policyId.bytes() == kPolicyId &&
        descriptor.majorVersion == 1U &&
        descriptor.minorVersion == 0U &&
        descriptor.implementationRevisionKind == 1U &&
        descriptor.implementationRevision == kImplementationRevision &&
        policy.bridgeCouplingAdjustmentClass() ==
            detail::ProductionTransitionRequestDerivationAccess::
                bridgeCouplingAdjustmentV1();
}

[[nodiscard]] bool validLineage(
    const ProductionPersistentBridgeRecommendation& recommendation) noexcept
{
    return
        nonZero(recommendation.decisionId().bytes()) &&
        nonZero(recommendation.streamInstanceId().bytes()) &&
        nonZero(recommendation.policyEvidenceDecisionId().bytes()) &&
        nonZero(recommendation.sourceCaptureId().bytes()) &&
        recommendation.stateVersion() > 0U;
}

} // namespace

std::optional<TransitionRequestPolicySnapshot>
ProductionTransitionRequestPolicyProvider::createCurrent()
{
    const auto snapshotBytes =
        detail::tryGenerateTransitionRequestOpaqueIdBytes();
    if (!snapshotBytes.has_value()) return std::nullopt;

    return TransitionRequestPolicySnapshot{
        TransitionRequestPolicySnapshotId{*snapshotBytes},
        TransitionRequestPolicyDescriptor{
            TransitionRequestPolicyId{kPolicyId},
            1U,
            0U,
            1U,
            kImplementationRevision
        },
        detail::ProductionTransitionRequestDerivationAccess::
            bridgeCouplingAdjustmentV1()
    };
}

std::optional<ProductionTransitionRequestDerivationResult>
ProductionTransitionRequestDeriver::derive(
    const ProductionPersistentBridgeRecommendation& recommendation,
    const TransitionRequestPolicySnapshot& policy) const
{
    const auto decisionBytes =
        detail::tryGenerateTransitionRequestOpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;

    TransitionRequestDecisionId decisionId{*decisionBytes};

    const auto reject = [&](
        TransitionRequestDerivationReason reason)
        -> std::optional<ProductionTransitionRequestDerivationResult>
    {
        return ProductionTransitionRequestDerivationResult{
            ProductionTransitionRequestDerivationRejection{
                decisionId,
                recommendation.decisionId(),
                reason,
                reasonFlag(reason)
            }
        };
    };

    if (!recognizedPolicy(policy)) {
        return reject(
            TransitionRequestDerivationReason::PolicyRevisionUnrecognized);
    }

    if (!validLineage(recommendation)) {
        return reject(
            TransitionRequestDerivationReason::
                RecommendationLineageInconsistent);
    }

    if (recommendation.recommendation() ==
        PersistentBridgeRecommendation::PRESERVE) {
        return reject(
            TransitionRequestDerivationReason::PreserveRecommendation);
    }

    RequestedTransitionDirection direction;
    switch (recommendation.recommendation()) {
        case PersistentBridgeRecommendation::SUPPORT:
            direction = RequestedTransitionDirection::support;
            break;
        case PersistentBridgeRecommendation::CONSTRAIN:
            direction = RequestedTransitionDirection::constrain;
            break;
        case PersistentBridgeRecommendation::PRESERVE:
            return reject(
                TransitionRequestDerivationReason::PreserveRecommendation);
        default:
            return reject(
                TransitionRequestDerivationReason::
                    RecommendationLineageInconsistent);
    }

    const auto& key = recommendation.streamKey();

    const auto relationship =
        detail::ProductionTransitionRequestDerivationAccess::relationship(
            key.sourceNodeId(),
            key.targetNodeId(),
            key.relationshipGeneration());

    const auto stateVersion =
        detail::ProductionTransitionRequestDerivationAccess::stateVersion(
            recommendation.stateVersion());

    const auto transitionClass =
        detail::ProductionTransitionRequestDerivationAccess::
            bridgeCouplingAdjustmentV1();

    const auto binding =
        detail::ProductionTransitionRequestDerivationAccess::binding(
            relationship,
            direction,
            transitionClass,
            stateVersion);

    try {
        return ProductionTransitionRequestDerivationResult{
            ProductionDerivedTransitionRequest{
                std::move(decisionId),
                binding,
                policy.snapshotId(),
                policy.descriptor(),
                recommendation.decisionId(),
                recommendation.streamInstanceId(),
                recommendation.streamKey(),
                recommendation.policyEvidenceDecisionId(),
                recommendation.sourceCaptureId(),
                recommendation.recommendation(),
                recommendation.stateVersion()
            }
        };
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace AdaptiveMesh
