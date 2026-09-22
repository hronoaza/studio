#include "production_provenance_envelope.hpp"
#include "detail/provenance_envelope_internal.hpp"
#include "system_architecture.hpp"
#include "d8b_implementation_revision.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
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

namespace AdaptiveMesh {
namespace detail {
namespace {

constexpr std::size_t kMaxIdAttempts = 8;
constexpr std::size_t kRecentIdCount = 64;

constexpr Id128 kSchemaId{
    0x3c,0xb7,0x9d,0x20,0xa6,0x2b,0x87,0x80,
    0x96,0x0c,0x1a,0xeb,0x70,0xb4,0x2e,0xf4
};
constexpr Id128 kProducerId{
    0x20,0x1a,0x17,0xbf,0x2d,0xc6,0x1a,0x8a,
    0x72,0xaa,0x76,0x63,0x78,0x00,0xe5,0x35
};
constexpr Id128 kDepSourceCapture{
    0xeb,0xe0,0x30,0x70,0x61,0x1d,0x3e,0x46,
    0x0a,0x72,0xd1,0x12,0xb9,0xa1,0x81,0xad
};
constexpr Id128 kDepCanonicalEncoding{
    0x11,0x6a,0xe0,0x37,0xcd,0x62,0x7d,0xff,
    0x94,0x96,0xdd,0x58,0x12,0xac,0x71,0x24
};
constexpr Id128 kDepDigestProfile{
    0xda,0x5d,0x36,0x58,0x65,0x07,0x6c,0xc4,
    0x15,0xc1,0xac,0xd8,0x42,0xab,0xf0,0xd0
};

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

[[nodiscard]] constexpr Digest256 implementationRevisionDigest() {
    constexpr const char* hex = SOAM_D8B_IMPLEMENTATION_REVISION_SHA;
    Digest256 digest{};
    for (std::size_t i = 0; i < digest.size(); ++i) {
        const std::uint8_t high = hexNibble(hex[i * 2U]);
        const std::uint8_t low = hexNibble(hex[i * 2U + 1U]);
        digest[i] = static_cast<std::uint8_t>((high << 4U) | low);
    }
    return digest;
}

constexpr Digest256 kImplementationRevision =
    implementationRevisionDigest();

std::atomic<ProvenanceItemIdFillFunction> testFillFunction{nullptr};
std::atomic_bool forceCanonicalFailure{false};
std::atomic_bool forceDigestFailure{false};
std::atomic_bool forceManifestFailure{false};
std::atomic_flag recentIdsLock = ATOMIC_FLAG_INIT;
std::array<ProvenanceItemId::Bytes, kRecentIdCount> recentIds{};
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

[[nodiscard]] bool allZero(const ProvenanceItemId::Bytes& bytes) noexcept {
    for (const auto byte : bytes) {
        if (byte != 0U) return false;
    }
    return true;
}

[[nodiscard]] bool reserveIfNotRecent(
    const ProvenanceItemId::Bytes& bytes) noexcept
{
    RecentIdsGuard guard;
    for (std::size_t i = 0; i < recentIdsSize; ++i) {
        if (recentIds[i] == bytes) return false;
    }
    if (recentIdsSize < kRecentIdCount) {
        recentIds[recentIdsSize++] = bytes;
    } else {
        recentIds[nextRecentId] = bytes;
        nextRecentId = (nextRecentId + 1U) % kRecentIdCount;
    }
    return true;
}

[[nodiscard]] bool fillFromOperatingSystem(
    ProvenanceItemId::Bytes& out) noexcept
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
        const auto result = ::getrandom(out.data() + offset, out.size() - offset, 0);
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

[[nodiscard]] bool fillCandidate(ProvenanceItemId::Bytes& out) noexcept {
    if (const auto overrideFill =
            testFillFunction.load(std::memory_order_acquire)) {
        return overrideFill(out);
    }
    return fillFromOperatingSystem(out);
}

void appendU16(std::vector<std::uint8_t>& out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xffU));
    out.push_back(static_cast<std::uint8_t>(value & 0xffU));
}

void appendU64(std::vector<std::uint8_t>& out, std::uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8) {
        out.push_back(static_cast<std::uint8_t>(
            (value >> static_cast<unsigned>(shift)) & 0xffU));
    }
}

template <std::size_t N>
void appendArray(
    std::vector<std::uint8_t>& out,
    const std::array<std::uint8_t, N>& bytes)
{
    out.insert(out.end(), bytes.begin(), bytes.end());
}

void appendF64(std::vector<std::uint8_t>& out, double value) {
    appendU64(out, std::bit_cast<std::uint64_t>(value));
}

[[nodiscard]] std::array<std::uint8_t, 54> encodeDependency(
    const CanonicalDependencyInput& dependency)
{
    std::array<std::uint8_t, 54> bytes{};
    std::size_t offset = 0;
    bytes[offset++] = dependency.kind;
    for (const auto byte : dependency.id) bytes[offset++] = byte;
    bytes[offset++] = static_cast<std::uint8_t>((dependency.major >> 8U) & 0xffU);
    bytes[offset++] = static_cast<std::uint8_t>(dependency.major & 0xffU);
    bytes[offset++] = static_cast<std::uint8_t>((dependency.minor >> 8U) & 0xffU);
    bytes[offset++] = static_cast<std::uint8_t>(dependency.minor & 0xffU);
    bytes[offset++] = dependency.revisionKind;
    for (const auto byte : dependency.revisionDigest) bytes[offset++] = byte;
    return bytes;
}

[[nodiscard]] std::uint8_t bridgeStatusTag(BridgeStatus status) {
    switch (status) {
    case BridgeStatus::NORMAL: return 0;
    case BridgeStatus::DAMPING: return 1;
    case BridgeStatus::RECOVERY: return 2;
    case BridgeStatus::ISOLATED: return 3;
    }
    return 255;
}

[[nodiscard]] std::vector<CanonicalDependencyInput>
productionDependencies()
{
    if (forceManifestFailure.load(std::memory_order_acquire)) return {};

    const Digest256 zero{};
    return {
        {1, kDepSourceCapture, 1, 0, 0, zero},
        {2, kDepCanonicalEncoding, 1, 0, 0, zero},
        {3, kDepDigestProfile, 1, 0, 0, zero}
    };
}

// Minimal SHA-256, used only for deterministic canonical content binding.
constexpr std::array<std::uint32_t, 64> kSha256{
    0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,
    0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
    0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,
    0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
    0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,
    0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
    0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,
    0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
    0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,
    0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
    0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,
    0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
    0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,
    0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
    0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,
    0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

[[nodiscard]] constexpr std::uint32_t rotr(std::uint32_t value, unsigned bits) {
    return (value >> bits) | (value << (32U - bits));
}

[[nodiscard]] Digest256 sha256Raw(const std::vector<std::uint8_t>& input) noexcept {
    std::vector<std::uint8_t> data = input;
    const std::uint64_t bitLength =
        static_cast<std::uint64_t>(data.size()) * 8U;
    data.push_back(0x80U);
    while ((data.size() % 64U) != 56U) data.push_back(0U);
    for (int shift = 56; shift >= 0; shift -= 8) {
        data.push_back(static_cast<std::uint8_t>(
            (bitLength >> static_cast<unsigned>(shift)) & 0xffU));
    }

    std::array<std::uint32_t, 8> h{
        0x6a09e667U,0xbb67ae85U,0x3c6ef372U,0xa54ff53aU,
        0x510e527fU,0x9b05688cU,0x1f83d9abU,0x5be0cd19U
    };

    for (std::size_t chunk = 0; chunk < data.size(); chunk += 64U) {
        std::array<std::uint32_t, 64> w{};
        for (std::size_t i = 0; i < 16U; ++i) {
            const std::size_t p = chunk + i * 4U;
            w[i] =
                (static_cast<std::uint32_t>(data[p]) << 24U) |
                (static_cast<std::uint32_t>(data[p + 1U]) << 16U) |
                (static_cast<std::uint32_t>(data[p + 2U]) << 8U) |
                static_cast<std::uint32_t>(data[p + 3U]);
        }
        for (std::size_t i = 16U; i < 64U; ++i) {
            const std::uint32_t s0 =
                rotr(w[i - 15U], 7U) ^ rotr(w[i - 15U], 18U) ^
                (w[i - 15U] >> 3U);
            const std::uint32_t s1 =
                rotr(w[i - 2U], 17U) ^ rotr(w[i - 2U], 19U) ^
                (w[i - 2U] >> 10U);
            w[i] = w[i - 16U] + s0 + w[i - 7U] + s1;
        }

        std::uint32_t a=h[0],b=h[1],c=h[2],d=h[3];
        std::uint32_t e=h[4],f=h[5],g=h[6],hh=h[7];
        for (std::size_t i = 0; i < 64U; ++i) {
            const std::uint32_t s1 = rotr(e,6U)^rotr(e,11U)^rotr(e,25U);
            const std::uint32_t ch = (e & f) ^ ((~e) & g);
            const std::uint32_t temp1 = hh + s1 + ch + kSha256[i] + w[i];
            const std::uint32_t s0 = rotr(a,2U)^rotr(a,13U)^rotr(a,22U);
            const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t temp2 = s0 + maj;
            hh=g; g=f; f=e; e=d+temp1; d=c; c=b; b=a; a=temp1+temp2;
        }
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d;
        h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }

    Digest256 digest{};
    std::size_t offset=0;
    for (const auto value : h) {
        digest[offset++] = static_cast<std::uint8_t>((value >> 24U) & 0xffU);
        digest[offset++] = static_cast<std::uint8_t>((value >> 16U) & 0xffU);
        digest[offset++] = static_cast<std::uint8_t>((value >> 8U) & 0xffU);
        digest[offset++] = static_cast<std::uint8_t>(value & 0xffU);
    }
    return digest;
}

} // namespace

std::optional<ProvenanceItemId::Bytes>
tryGenerateProvenanceItemIdBytes() noexcept
{
    for (std::size_t attempt = 0; attempt < kMaxIdAttempts; ++attempt) {
        ProvenanceItemId::Bytes candidate{};
        if (!fillCandidate(candidate)) return std::nullopt;
        if (allZero(candidate)) continue;
        if (!reserveIfNotRecent(candidate)) continue;
        return candidate;
    }
    return std::nullopt;
}

std::optional<std::vector<std::uint8_t>>
encodeCanonicalEnvelopeV1(const CanonicalEnvelopeInput& input)
{
    if (forceCanonicalFailure.load(std::memory_order_acquire)) {
        return std::nullopt;
    }
    if (input.bridgeStatusTag > 3U) return std::nullopt;
    if (input.dependencies.size() >
        static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())) {
        return std::nullopt;
    }

    std::vector<std::array<std::uint8_t,54>> encodedDependencies;
    encodedDependencies.reserve(input.dependencies.size());
    for (const auto& dependency : input.dependencies) {
        if (dependency.kind == 0U || dependency.kind == 255U) return std::nullopt;
        if (dependency.revisionKind > 1U) return std::nullopt;
        if (dependency.revisionKind == 0U) {
            for (const auto byte : dependency.revisionDigest) {
                if (byte != 0U) return std::nullopt;
            }
        }
        encodedDependencies.push_back(encodeDependency(dependency));
    }
    std::sort(encodedDependencies.begin(), encodedDependencies.end());
    if (std::adjacent_find(
            encodedDependencies.begin(),
            encodedDependencies.end()) != encodedDependencies.end()) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> out;
    out.reserve(207U + encodedDependencies.size() * 54U);
    constexpr std::array<std::uint8_t,8> magic{
        'S','O','A','M','D','8','B','1'
    };
    appendArray(out, magic);
    appendArray(out, input.schemaId);
    appendU16(out, input.schemaMajor);
    appendU16(out, input.schemaMinor);
    appendU16(out, input.canonicalEncodingVersion);
    appendArray(out, input.producerId);
    appendU16(out, input.producerMajor);
    appendU16(out, input.producerMinor);
    out.push_back(input.implementationRevisionKind);
    if (input.implementationRevisionKind > 1U) return std::nullopt;
    if (input.implementationRevisionKind == 0U) {
        for (const auto byte : input.implementationRevision) {
            if (byte != 0U) return std::nullopt;
        }
    }
    appendArray(out, input.implementationRevision);
    appendArray(out, input.provenanceItemId);
    appendArray(out, input.sourceCaptureId);
    appendU64(out, input.sourceNodeId);
    appendU64(out, input.targetNodeId);
    appendU64(out, input.relationshipGeneration);
    appendU64(out, input.stateVersion);
    appendF64(out, input.distance);
    appendF64(out, input.orientationWeight);
    appendF64(out, input.capacity);
    out.push_back(input.bridgeStatusTag);
    appendF64(out, input.sourceState);
    appendF64(out, input.targetState);
    appendF64(out, input.sourceHealth);
    appendF64(out, input.targetHealth);
    appendU16(out, static_cast<std::uint16_t>(encodedDependencies.size()));
    for (const auto& dependency : encodedDependencies) {
        appendArray(out, dependency);
    }
    out.push_back(0U);
    return out;
}

Digest256 sha256D8BDomainSeparated(
    const std::vector<std::uint8_t>& canonicalBytes) noexcept
{
    constexpr std::array<std::uint8_t,31> domain{
        'S','O','A','M',':','D','8','B',':','P','R','O','V','E','N','A',
        'N','C','E','-','E','N','V','E','L','O','P','E',':','V','1'
    };
    std::vector<std::uint8_t> preimage;
    preimage.reserve(domain.size() + canonicalBytes.size());
    preimage.insert(preimage.end(), domain.begin(), domain.end());
    preimage.insert(preimage.end(), canonicalBytes.begin(), canonicalBytes.end());
    return sha256Raw(preimage);
}

void setProvenanceItemIdFillFunctionForTesting(
    ProvenanceItemIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction, std::memory_order_release);
}

void setD8BCanonicalEncodingFailureForTesting(bool enabled) noexcept {
    forceCanonicalFailure.store(enabled, std::memory_order_release);
}
void setD8BDigestFailureForTesting(bool enabled) noexcept {
    forceDigestFailure.store(enabled, std::memory_order_release);
}
void setD8BManifestFailureForTesting(bool enabled) noexcept {
    forceManifestFailure.store(enabled, std::memory_order_release);
}
bool d8bDigestFailureForTesting() noexcept {
    return forceDigestFailure.load(std::memory_order_acquire);
}
bool d8bManifestFailureForTesting() noexcept {
    return forceManifestFailure.load(std::memory_order_acquire);
}

void resetD8BTestSeams() noexcept {
    testFillFunction.store(nullptr, std::memory_order_release);
    forceCanonicalFailure.store(false, std::memory_order_release);
    forceDigestFailure.store(false, std::memory_order_release);
    forceManifestFailure.store(false, std::memory_order_release);
    RecentIdsGuard guard;
    recentIds = {};
    recentIdsSize = 0;
    nextRecentId = 0;
}

} // namespace detail

ProductionProvenanceEnvelope::ProductionProvenanceEnvelope(
    ProvenanceItemId provenanceItemId,
    SourceCaptureId sourceCaptureId,
    std::size_t sourceNodeId,
    std::size_t targetNodeId,
    std::uint64_t relationshipGeneration,
    std::uint64_t stateVersion,
    double distance,
    double orientationWeight,
    double capacity,
    BridgeStatus bridgeStatus,
    double sourceState,
    double targetState,
    double sourceHealth,
    double targetHealth,
    ProvenanceMetadataView metadata,
    CanonicalDigest canonicalDigest,
    std::vector<std::uint8_t> canonicalBytes) noexcept
    : provenanceItemId_(std::move(provenanceItemId)),
      sourceCaptureId_(std::move(sourceCaptureId)),
      sourceNodeId_(sourceNodeId),
      targetNodeId_(targetNodeId),
      relationshipGeneration_(relationshipGeneration),
      stateVersion_(stateVersion),
      distance_(distance),
      orientationWeight_(orientationWeight),
      capacity_(capacity),
      bridgeStatus_(bridgeStatus),
      sourceState_(sourceState),
      targetState_(targetState),
      sourceHealth_(sourceHealth),
      targetHealth_(targetHealth),
      metadata_(std::move(metadata)),
      canonicalDigest_(std::move(canonicalDigest)),
      canonicalBytes_(std::move(canonicalBytes))
{
}

std::optional<ProductionProvenanceEnvelope>
ProductionProvenanceEnvelopeProducer::produce(
    const ProductionRelationshipSourceSnapshot& snapshot) const
{
    const auto itemBytes = detail::tryGenerateProvenanceItemIdBytes();
    if (!itemBytes.has_value()) return std::nullopt;

    if (detail::d8bManifestFailureForTesting()) return std::nullopt;
    auto dependencies = detail::productionDependencies();
    if (dependencies.empty()) return std::nullopt;

    if constexpr (sizeof(std::size_t) > sizeof(std::uint64_t)) {
        if (snapshot.sourceNodeId() >
                static_cast<std::size_t>(std::numeric_limits<std::uint64_t>::max()) ||
            snapshot.targetNodeId() >
                static_cast<std::size_t>(std::numeric_limits<std::uint64_t>::max())) {
            return std::nullopt;
        }
    }

    const std::uint8_t statusTag = detail::bridgeStatusTag(snapshot.bridgeStatus());
    if (statusTag > 3U) return std::nullopt;

    std::vector<ProvenanceDependencyDescriptor> metadataDependencies;
    metadataDependencies.reserve(dependencies.size());
    for (const auto& dependency : dependencies) {
        metadataDependencies.push_back(ProvenanceDependencyDescriptor{
            dependency.kind,
            dependency.id,
            dependency.major,
            dependency.minor,
            dependency.revisionKind,
            dependency.revisionDigest
        });
    }

    ProvenanceMetadataView metadata{
        detail::kSchemaId, 1, 0, 1,
        detail::kProducerId, 1, 0, 1, detail::kImplementationRevision,
        std::move(metadataDependencies)
    };

    std::vector<detail::CanonicalDependencyInput> canonicalDependencies;
    canonicalDependencies.reserve(metadata.dependencies().size());
    for (const auto& dependency : metadata.dependencies()) {
        canonicalDependencies.push_back(detail::CanonicalDependencyInput{
            dependency.kind,
            dependency.dependencyId,
            dependency.versionMajor,
            dependency.versionMinor,
            dependency.revisionKind,
            dependency.revisionDigest
        });
    }

    detail::CanonicalEnvelopeInput input{
        metadata.schemaId(),
        metadata.schemaMajor(),
        metadata.schemaMinor(),
        metadata.canonicalEncodingVersion(),
        metadata.producerId(),
        metadata.producerMajor(),
        metadata.producerMinor(),
        metadata.implementationRevisionKind(),
        metadata.implementationRevision(),
        *itemBytes,
        snapshot.sourceCaptureId().bytes(),
        static_cast<std::uint64_t>(snapshot.sourceNodeId()),
        static_cast<std::uint64_t>(snapshot.targetNodeId()),
        snapshot.relationshipGeneration(),
        snapshot.stateVersion(),
        snapshot.distance(),
        snapshot.orientationWeight(),
        snapshot.capacity(),
        statusTag,
        snapshot.sourceState(),
        snapshot.targetState(),
        snapshot.sourceHealth(),
        snapshot.targetHealth(),
        std::move(canonicalDependencies)
    };

    auto canonicalBytes = detail::encodeCanonicalEnvelopeV1(input);
    if (!canonicalBytes.has_value()) return std::nullopt;
    if (detail::d8bDigestFailureForTesting()) return std::nullopt;

    const auto digestBytes =
        detail::sha256D8BDomainSeparated(*canonicalBytes);

    return ProductionProvenanceEnvelope{
        ProvenanceItemId{*itemBytes},
        snapshot.sourceCaptureId(),
        snapshot.sourceNodeId(),
        snapshot.targetNodeId(),
        snapshot.relationshipGeneration(),
        snapshot.stateVersion(),
        snapshot.distance(),
        snapshot.orientationWeight(),
        snapshot.capacity(),
        snapshot.bridgeStatus(),
        snapshot.sourceState(),
        snapshot.targetState(),
        snapshot.sourceHealth(),
        snapshot.targetHealth(),
        std::move(metadata),
        CanonicalDigest{digestBytes},
        std::move(*canonicalBytes)
    };
}

} // namespace AdaptiveMesh
