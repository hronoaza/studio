#include "provenance_admissibility.hpp"
#include "detail/provenance_admissibility_internal.hpp"
#include "detail/provenance_envelope_internal.hpp"
#include "d8b_implementation_revision.hpp"

#include <array>
#include <atomic>
#include <bit>
#include <cerrno>
#include <cstddef>
#include <cstdint>
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

constexpr std::size_t kMaxAttempts = 8;
constexpr std::size_t kRecentCount = 128;

std::atomic<D8COpaqueIdFillFunction> testFillFunction{nullptr};
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
    std::size_t offset=0;
    while (offset < out.size()) {
        const auto result=::getrandom(out.data()+offset,out.size()-offset,0);
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
tryGenerateD8COpaqueIdBytes() noexcept
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

void setD8COpaqueIdFillFunctionForTesting(
    D8COpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction,std::memory_order_release);
}

void resetD8COpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr,std::memory_order_release);
    RecentIdsGuard guard;
    recentIds={};
    recentIdsSize=0;
    nextRecentId=0;
}

ProvenanceAdmissibilityPolicySnapshot
ProvenanceAdmissibilityTestAccess::makePolicy(
    PolicySnapshotId::Bytes snapshotId,
    AdmissibilityPolicyDescriptor descriptor,
    std::vector<ProducerPolicyEntry> producers,
    std::vector<SchemaPolicyEntry> schemas,
    std::vector<DependencyPolicyEntry> dependencies,
    std::vector<std::uint8_t> requiredDependencyKinds)
{
    return ProvenanceAdmissibilityPolicySnapshot{
        PolicySnapshotId{snapshotId},
        descriptor,
        std::move(producers),
        std::move(schemas),
        std::move(dependencies),
        std::move(requiredDependencyKinds)
    };
}

} // namespace detail

namespace {

constexpr ProvenanceMetadataView::Id128 kSchemaId{
    0x3c,0xb7,0x9d,0x20,0xa6,0x2b,0x87,0x80,
    0x96,0x0c,0x1a,0xeb,0x70,0xb4,0x2e,0xf4
};
constexpr ProvenanceMetadataView::Id128 kProducerId{
    0x20,0x1a,0x17,0xbf,0x2d,0xc6,0x1a,0x8a,
    0x72,0xaa,0x76,0x63,0x78,0x00,0xe5,0x35
};
constexpr ProvenanceMetadataView::Id128 kDepSourceCapture{
    0xeb,0xe0,0x30,0x70,0x61,0x1d,0x3e,0x46,
    0x0a,0x72,0xd1,0x12,0xb9,0xa1,0x81,0xad
};
constexpr ProvenanceMetadataView::Id128 kDepCanonicalEncoding{
    0x11,0x6a,0xe0,0x37,0xcd,0x62,0x7d,0xff,
    0x94,0x96,0xdd,0x58,0x12,0xac,0x71,0x24
};
constexpr ProvenanceMetadataView::Id128 kDepDigestProfile{
    0xda,0x5d,0x36,0x58,0x65,0x07,0x6c,0xc4,
    0x15,0xc1,0xac,0xd8,0x42,0xab,0xf0,0xd0
};
constexpr std::array<std::uint8_t,16> kPolicyId{
    0x7d,0x38,0x6a,0xc2,0x15,0x4f,0x47,0x92,
    0xa1,0xc8,0x30,0xd7,0x2e,0x65,0x89,0xb4
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

[[nodiscard]] constexpr ProvenanceMetadataView::Digest256
implementationRevisionDigest()
{
    constexpr const char* hex = SOAM_D8B_IMPLEMENTATION_REVISION_SHA;
    ProvenanceMetadataView::Digest256 digest{};
    for (std::size_t i=0; i<digest.size(); ++i) {
        const auto high=hexNibble(hex[i*2U]);
        const auto low=hexNibble(hex[i*2U+1U]);
        digest[i]=static_cast<std::uint8_t>((high<<4U)|low);
    }
    return digest;
}

constexpr auto kImplementationRevision=implementationRevisionDigest();

template <std::size_t N>
[[nodiscard]] bool nonZero(const std::array<std::uint8_t,N>& value) noexcept {
    for (const auto byte:value) {
        if (byte != 0U) return true;
    }
    return false;
}

[[nodiscard]] bool sameBits(double lhs,double rhs) noexcept {
    return std::bit_cast<std::uint64_t>(lhs) ==
        std::bit_cast<std::uint64_t>(rhs);
}

[[nodiscard]] std::uint64_t reasonFlag(
    ProvenanceAdmissibilityReason reason) noexcept
{
    return std::uint64_t{1} << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool sourceMatches(
    const RetainedSourceEvidenceRecord& record,
    const ProductionProvenanceEnvelope& envelope) noexcept
{
    return
        record.sourceCaptureId()==envelope.sourceCaptureId() &&
        record.sourceNodeId()==envelope.sourceNodeId() &&
        record.targetNodeId()==envelope.targetNodeId() &&
        record.relationshipGeneration()==envelope.relationshipGeneration() &&
        record.stateVersion()==envelope.stateVersion() &&
        sameBits(record.distance(),envelope.distance()) &&
        sameBits(record.orientationWeight(),envelope.orientationWeight()) &&
        sameBits(record.capacity(),envelope.capacity()) &&
        record.bridgeStatus()==envelope.bridgeStatus() &&
        sameBits(record.sourceState(),envelope.sourceState()) &&
        sameBits(record.targetState(),envelope.targetState()) &&
        sameBits(record.sourceHealth(),envelope.sourceHealth()) &&
        sameBits(record.targetHealth(),envelope.targetHealth());
}

[[nodiscard]] const ProducerPolicyEntry* findProducer(
    const ProvenanceMetadataView& metadata,
    const ProvenanceAdmissibilityPolicySnapshot& policy,
    bool& familyKnown) noexcept
{
    familyKnown=false;
    for (const auto& entry:policy.producers()) {
        if (entry.producerId==metadata.producerId() &&
            entry.producerMajor==metadata.producerMajor() &&
            entry.producerMinor==metadata.producerMinor()) {
            familyKnown=true;
            if (entry.implementationRevisionKind==
                    metadata.implementationRevisionKind() &&
                entry.implementationRevision==
                    metadata.implementationRevision()) {
                return &entry;
            }
        }
    }
    return nullptr;
}

[[nodiscard]] const SchemaPolicyEntry* findSchema(
    const ProvenanceMetadataView& metadata,
    const ProvenanceAdmissibilityPolicySnapshot& policy,
    bool& schemaIdKnown,
    bool& schemaVersionKnown) noexcept
{
    schemaIdKnown=false;
    schemaVersionKnown=false;
    for (const auto& entry:policy.schemas()) {
        if (entry.schemaId==metadata.schemaId()) {
            schemaIdKnown=true;
            if (entry.schemaMajor==metadata.schemaMajor() &&
                entry.schemaMinor==metadata.schemaMinor()) {
                schemaVersionKnown=true;
                if (entry.canonicalEncodingVersion==
                    metadata.canonicalEncodingVersion()) {
                    return &entry;
                }
            }
        }
    }
    return nullptr;
}

[[nodiscard]] const DependencyPolicyEntry* findDependency(
    const ProvenanceDependencyDescriptor& dependency,
    const ProvenanceAdmissibilityPolicySnapshot& policy,
    bool& familyKnown) noexcept
{
    familyKnown=false;
    for (const auto& entry:policy.dependencies()) {
        if (entry.dependencyKind==dependency.kind &&
            entry.dependencyId==dependency.dependencyId) {
            familyKnown=true;
            if (entry.versionMajor==dependency.versionMajor &&
                entry.versionMinor==dependency.versionMinor &&
                entry.revisionKind==dependency.revisionKind &&
                entry.revisionDigest==dependency.revisionDigest) {
                return &entry;
            }
        }
    }
    return nullptr;
}

} // namespace

std::optional<ProvenanceAdmissibilityPolicySnapshot>
ProductionProvenanceAdmissibilityPolicyProvider::createCurrent()
{
    const auto id=detail::tryGenerateD8COpaqueIdBytes();
    if (!id.has_value()) return std::nullopt;

    const ProvenanceMetadataView::Digest256 zero{};

    std::vector<ProducerPolicyEntry> producers{
        {kProducerId,1,0,1,kImplementationRevision,
         ProducerLifecycleStatus::Recognized}
    };
    std::vector<SchemaPolicyEntry> schemas{
        {kSchemaId,1,0,1,SchemaLifecycleStatus::Compatible}
    };
    std::vector<DependencyPolicyEntry> dependencies{
        {1,kDepSourceCapture,1,0,0,zero,
         DependencySemanticCategory::SourceCaptureContract,
         DependencyLifecycleStatus::Active},
        {2,kDepCanonicalEncoding,1,0,0,zero,
         DependencySemanticCategory::CanonicalEncoding,
         DependencyLifecycleStatus::Active},
        {3,kDepDigestProfile,1,0,0,zero,
         DependencySemanticCategory::DigestProfile,
         DependencyLifecycleStatus::Active}
    };

    return ProvenanceAdmissibilityPolicySnapshot{
        PolicySnapshotId{*id},
        AdmissibilityPolicyDescriptor{kPolicyId,1,0},
        std::move(producers),
        std::move(schemas),
        std::move(dependencies),
        {1U,2U,3U}
    };
}

std::optional<ProvenanceAdmissibilityResult>
ProvenanceAdmissibilityEvaluator::evaluate(
    const ProductionProvenanceEnvelope& envelope,
    const ProvenanceAdmissibilityPolicySnapshot& policy,
    const SourceEvidenceResolver& resolver) const
{
    const auto decisionBytes=detail::tryGenerateD8COpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;
    AdmissibilityDecisionId decisionId{*decisionBytes};

    const auto reject=[&](
        ProvenanceAdmissibilityReason reason)
        -> std::optional<ProvenanceAdmissibilityResult>
    {
        return ProvenanceAdmissibilityResult{
            ProvenanceAdmissibilityRejection{
                decisionId,
                policy.policySnapshotId(),
                envelope.provenanceItemId(),
                reason,
                reasonFlag(reason)
            }
        };
    };

    const auto recomputed=detail::sha256D8BDomainSeparated(
        envelope.canonicalBytes());
    if (recomputed != envelope.canonicalDigest().bytes()) {
        return reject(ProvenanceAdmissibilityReason::CanonicalDigestMismatch);
    }

    const auto& metadata=envelope.metadata();
    if (!nonZero(metadata.schemaId()) ||
        !nonZero(metadata.producerId()) ||
        metadata.implementationRevisionKind()!=1U ||
        !nonZero(metadata.implementationRevision())) {
        return reject(ProvenanceAdmissibilityReason::MetadataInconsistent);
    }

    bool producerFamily=false;
    const auto* producer=findProducer(metadata,policy,producerFamily);
    if (producer==nullptr) {
        return reject(producerFamily
            ? ProvenanceAdmissibilityReason::ImplementationRevisionUnrecognized
            : ProvenanceAdmissibilityReason::ProducerUnknown);
    }
    if (producer->status==ProducerLifecycleStatus::Retired) {
        return reject(ProvenanceAdmissibilityReason::ProducerRetired);
    }
    if (producer->status==ProducerLifecycleStatus::Prohibited) {
        return reject(ProvenanceAdmissibilityReason::ProducerProhibited);
    }

    bool schemaIdKnown=false;
    bool schemaVersionKnown=false;
    const auto* schema=findSchema(
        metadata,policy,schemaIdKnown,schemaVersionKnown);
    if (schema==nullptr) {
        if (!schemaIdKnown) {
            return reject(ProvenanceAdmissibilityReason::SchemaUnknown);
        }
        if (schemaVersionKnown) {
            return reject(
                ProvenanceAdmissibilityReason::CanonicalEncodingUnsupported);
        }
        return reject(ProvenanceAdmissibilityReason::SchemaIncompatible);
    }
    if (schema->status==SchemaLifecycleStatus::Retired) {
        return reject(ProvenanceAdmissibilityReason::SchemaRetired);
    }
    if (schema->status!=SchemaLifecycleStatus::Compatible) {
        return reject(ProvenanceAdmissibilityReason::SchemaIncompatible);
    }

    for (const auto requiredKind:policy.requiredDependencyKinds()) {
        bool present=false;
        for (const auto& dependency:metadata.dependencies()) {
            if (dependency.kind==requiredKind) {
                present=true;
                break;
            }
        }
        if (!present) {
            return reject(
                ProvenanceAdmissibilityReason::RequiredDependencyMissing);
        }
    }

    for (const auto& dependency:metadata.dependencies()) {
        bool familyKnown=false;
        const auto* exact=findDependency(dependency,policy,familyKnown);
        if (exact==nullptr) {
            return reject(familyKnown
                ? ProvenanceAdmissibilityReason::DependencyIncompatible
                : ProvenanceAdmissibilityReason::DependencyUnknown);
        }
        if (exact->category==DependencySemanticCategory::InterpretationPolicy) {
            return reject(
                ProvenanceAdmissibilityReason::LegacyInterpretationDependency);
        }
        switch (exact->status) {
        case DependencyLifecycleStatus::Active:
            break;
        case DependencyLifecycleStatus::Retired:
            return reject(ProvenanceAdmissibilityReason::DependencyRetired);
        case DependencyLifecycleStatus::Prohibited:
            return reject(ProvenanceAdmissibilityReason::DependencyProhibited);
        case DependencyLifecycleStatus::Incompatible:
            return reject(ProvenanceAdmissibilityReason::DependencyIncompatible);
        }
    }

    const auto resolution=resolver.resolve(envelope.sourceCaptureId());
    switch (resolution.status) {
    case SourceResolutionStatus::FOUND:
        if (!resolution.record.has_value()) {
            return reject(ProvenanceAdmissibilityReason::SourceResolverFailure);
        }
        break;
    case SourceResolutionStatus::NOT_FOUND:
        return reject(ProvenanceAdmissibilityReason::SourceRecordUnavailable);
    case SourceResolutionStatus::RESOLVER_FAILURE:
        return reject(ProvenanceAdmissibilityReason::SourceResolverFailure);
    case SourceResolutionStatus::INTEGRITY_CONFLICT:
        return reject(
            ProvenanceAdmissibilityReason::SourceEvidenceIntegrityConflict);
    }

    if (!sourceMatches(*resolution.record,envelope)) {
        return reject(ProvenanceAdmissibilityReason::SourceRecordMismatch);
    }

    return ProvenanceAdmissibilityResult{
        AdmissibleProductionProvenance{
            std::move(decisionId),
            policy.policySnapshotId(),
            policy.descriptor(),
            envelope,
            SourceVerificationSummary{envelope.sourceCaptureId(),true}
        }
    };
}

} // namespace AdaptiveMesh
