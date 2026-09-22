#include "policy_persistence.hpp"
#include "detail/policy_persistence_internal.hpp"
#include "d9_implementation_revision.hpp"

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

namespace AdaptiveMesh {
namespace detail {
namespace {

constexpr std::size_t kMaxAttempts = 8;
constexpr std::size_t kRecentCount = 128;

std::atomic<D9OpaqueIdFillFunction> testFillFunction{nullptr};
std::atomic_flag recentIdsLock = ATOMIC_FLAG_INIT;
std::array<std::array<std::uint8_t,16>, kRecentCount> recentIds{};
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
    for (std::size_t i = 0; i < recentIdsSize; ++i) {
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
            out.data() + offset,
            out.size() - offset,
            0);
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
tryGenerateD9OpaqueIdBytes() noexcept
{
    for (std::size_t attempt = 0; attempt < kMaxAttempts; ++attempt) {
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

void setD9OpaqueIdFillFunctionForTesting(
    D9OpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction, std::memory_order_release);
}

void resetD9OpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr, std::memory_order_release);
    RecentIdsGuard guard;
    recentIds = {};
    recentIdsSize = 0;
    nextRecentId = 0;
}

} // namespace detail

namespace {

constexpr PersistenceProfileId::Bytes kPersistenceProfileId{
    0x4d,0x8a,0x2f,0x71,0x39,0x56,0x43,0x9c,
    0xa5,0x12,0xc7,0x3e,0x90,0x2b,0x61,0xdd
};

constexpr double kActivationThreshold = 0.50;
constexpr double kReleaseThreshold = 0.25;
constexpr std::size_t kActivationSamples = 2;
constexpr std::size_t kReleaseSamples = 2;

[[nodiscard]] constexpr std::uint8_t hexNibble(char value) {
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
implementationRevisionDigest()
{
    constexpr const char* hex = SOAM_D9_IMPLEMENTATION_REVISION_SHA;
    std::array<std::uint8_t,32> digest{};
    for (std::size_t i = 0; i < digest.size(); ++i) {
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
    ProductionPolicyEvidenceReason reason) noexcept
{
    return std::uint64_t{1}
        << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool upstreamLineageIsConsistent(
    const VersionedProductionInterpretation& interpretation) noexcept
{
    const auto& provenance = interpretation.admissibleProvenance();
    const auto& envelope = provenance.envelope();

    return
        provenance.sourceVerification().bitExactMatch &&
        provenance.sourceVerification().sourceCaptureId ==
            envelope.sourceCaptureId() &&
        nonZero(envelope.sourceCaptureId().bytes()) &&
        nonZero(envelope.provenanceItemId().bytes()) &&
        nonZero(provenance.decisionId().bytes()) &&
        nonZero(provenance.policySnapshotId().bytes()) &&
        nonZero(interpretation.decisionId().bytes()) &&
        nonZero(interpretation.interpretationPolicySnapshotId().bytes());
}

} // namespace

std::optional<ProductionPolicyEvidenceResult>
ProductionBridgePolicyEvidenceEvaluator::evaluate(
    const VersionedProductionInterpretation& interpretation) const
{
    const auto decisionBytes = detail::tryGenerateD9OpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;

    PolicyEvidenceDecisionId decisionId{*decisionBytes};

    const auto reject = [&](
        ProductionPolicyEvidenceReason reason)
        -> std::optional<ProductionPolicyEvidenceResult>
    {
        return ProductionPolicyEvidenceResult{
            ProductionPolicyEvidenceRejection{
                decisionId,
                interpretation.decisionId(),
                reason,
                reasonFlag(reason)
            }
        };
    };

    if (!upstreamLineageIsConsistent(interpretation)) {
        return reject(
            ProductionPolicyEvidenceReason::UpstreamLineageInconsistent);
    }

    AdaptiveBridgePolicy policy;
    const auto evidence = policy.evaluate(
        interpretation.observation(),
        interpretation.confidence());

    if (!std::isfinite(evidence.value()) ||
        evidence.value() < -1.0 ||
        evidence.value() > 1.0) {
        return reject(
            ProductionPolicyEvidenceReason::EvidenceInvariantViolation);
    }

    try {
        return ProductionPolicyEvidenceResult{
            ProductionBridgePolicyEvidence{
                std::move(decisionId),
                evidence,
                interpretation
            }
        };
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<PersistencePolicySnapshot>
ProductionPersistencePolicyProvider::createCurrent()
{
    const auto snapshotBytes = detail::tryGenerateD9OpaqueIdBytes();
    if (!snapshotBytes.has_value()) return std::nullopt;

    return PersistencePolicySnapshot{
        PersistencePolicySnapshotId{*snapshotBytes},
        PersistencePolicyDescriptor{
            PersistenceProfileId{kPersistenceProfileId},
            1U,
            0U,
            1U,
            kImplementationRevision
        },
        kActivationThreshold,
        kReleaseThreshold,
        kActivationSamples,
        kReleaseSamples
    };
}

} // namespace AdaptiveMesh
