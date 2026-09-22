#include "provenance_admissibility.hpp"
#include "detail/provenance_admissibility_internal.hpp"
#include "detail/provenance_envelope_internal.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
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

} // namespace detail

namespace {

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

[[nodiscard]] bool sameProducerKey(
    const ProducerPolicyEntry& a,
    const ProducerPolicyEntry& b) noexcept
{
    return a.producerId==b.producerId &&
        a.producerMajor==b.producerMajor &&
        a.producerMinor==b.producerMinor &&
        a.implementationRevisionKind==b.implementationRevisionKind &&
        a.implementationRevision==b.implementationRevision;
}

[[nodiscard]] bool sameSchemaKey(
    const SchemaPolicyEntry& a,
    const SchemaPolicyEntry& b) noexcept
{
    return a.schemaId==b.schemaId &&
        a.schemaMajor==b.schemaMajor &&
        a.schemaMinor==b.schemaMinor &&
        a.canonicalEncodingVersion==b.canonicalEncodingVersion;
}

[[nodiscard]] bool sameDependencyKey(
    const DependencyPolicyEntry& a,
    const DependencyPolicyEntry& b) noexcept
{
    return a.dependencyKind==b.dependencyKind &&
        a.dependencyId==b.dependencyId &&
        a.versionMajor==b.versionMajor &&
        a.versionMinor==b.versionMinor &&
        a.revisionKind==b.revisionKind &&
        a.revisionDigest==b.revisionDigest;
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

} // namespace

std::optional<ProvenanceAdmissibilityPolicySnapshot>
ProvenanceAdmissibilityPolicyPublisher::publish(
    AdmissibilityPolicyDescriptor descriptor,
    std::vector<ProducerPolicyEntry> producers,
    std::vector<SchemaPolicyEntry> schemas,
    std::vector<DependencyPolicyEntry> dependencies) const
{
    if (!nonZero(descriptor.policyId)) return std::nullopt;

    for (std::size_t i=0;i<producers.size();++i) {
        if (!nonZero(producers[i].producerId)) return std::nullopt;
        for (std::size_t j=i+1;j<producers.size();++j) {
            if (sameProducerKey(producers[i],producers[j])) return std::nullopt;
        }
    }
    for (std::size_t i=0;i<schemas.size();++i) {
        if (!nonZero(schemas[i].schemaId)) return std::nullopt;
        for (std::size_t j=i+1;j<schemas.size();++j) {
            if (sameSchemaKey(schemas[i],schemas[j])) return std::nullopt;
        }
    }
    for (std::size_t i=0;i<dependencies.size();++i) {
        if (!nonZero(dependencies[i].dependencyId)) return std::nullopt;
        for (std::size_t j=i+1;j<dependencies.size();++j) {
            if (sameDependencyKey(dependencies[i],dependencies[j])) return std::nullopt;
        }
    }

    const auto id=detail::tryGenerateD8COpaqueIdBytes();
    if (!id.has_value()) return std::nullopt;

    return ProvenanceAdmissibilityPolicySnapshot{
        PolicySnapshotId{*id},
        descriptor,
        std::move(producers),
        std::move(schemas),
        std::move(dependencies)
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
        return reject(ProvenanceAdmissibilityReason::CANONICAL_DIGEST_MISMATCH);
    }

    const auto& metadata=envelope.metadata();
    if (!nonZero(metadata.schemaId()) ||
        !nonZero(metadata.producerId()) ||
        metadata.implementationRevisionKind()>1U) {
        return reject(ProvenanceAdmissibilityReason::METADATA_INCONSISTENT);
    }

    const ProducerPolicyEntry* producerExact=nullptr;
    bool producerFamily=false;
    for (const auto& entry:policy.producers()) {
        if (entry.producerId==metadata.producerId() &&
            entry.producerMajor==metadata.producerMajor() &&
            entry.producerMinor==metadata.producerMinor()) {
            producerFamily=true;
            if (entry.implementationRevisionKind==
                    metadata.implementationRevisionKind() &&
                entry.implementationRevision==
                    metadata.implementationRevision()) {
                producerExact=&entry;
                break;
            }
        }
    }
    if (producerExact==nullptr) {
        return reject(producerFamily
            ? ProvenanceAdmissibilityReason::IMPLEMENTATION_REVISION_UNRECOGNIZED
            : ProvenanceAdmissibilityReason::PRODUCER_UNKNOWN);
    }
    if (producerExact->lifecycleStatus==ProducerLifecycleStatus::RETIRED) {
        return reject(ProvenanceAdmissibilityReason::PRODUCER_RETIRED);
    }
    if (producerExact->lifecycleStatus==ProducerLifecycleStatus::PROHIBITED) {
        return reject(ProvenanceAdmissibilityReason::PRODUCER_PROHIBITED);
    }

    const SchemaPolicyEntry* schemaExact=nullptr;
    bool schemaIdKnown=false;
    bool schemaVersionKnown=false;
    for (const auto& entry:policy.schemas()) {
        if (entry.schemaId==metadata.schemaId()) {
            schemaIdKnown=true;
            if (entry.schemaMajor==metadata.schemaMajor() &&
                entry.schemaMinor==metadata.schemaMinor()) {
                schemaVersionKnown=true;
                if (entry.canonicalEncodingVersion==
                    metadata.canonicalEncodingVersion()) {
                    schemaExact=&entry;
                    break;
                }
            }
        }
    }
    if (schemaExact==nullptr) {
        if (!schemaIdKnown) {
            return reject(ProvenanceAdmissibilityReason::SCHEMA_UNKNOWN);
        }
        if (schemaVersionKnown) {
            return reject(ProvenanceAdmissibilityReason::CANONICAL_ENCODING_UNSUPPORTED);
        }
        return reject(ProvenanceAdmissibilityReason::SCHEMA_INCOMPATIBLE);
    }
    if (schemaExact->lifecycleStatus==SchemaLifecycleStatus::RETIRED) {
        return reject(ProvenanceAdmissibilityReason::SCHEMA_RETIRED);
    }
    if (schemaExact->lifecycleStatus!=SchemaLifecycleStatus::COMPATIBLE) {
        return reject(ProvenanceAdmissibilityReason::SCHEMA_INCOMPATIBLE);
    }

    bool required1=false,required2=false,required3=false;
    for (const auto& dependency:metadata.dependencies()) {
        if (dependency.kind==1U) required1=true;
        if (dependency.kind==2U) required2=true;
        if (dependency.kind==3U) required3=true;

        const DependencyPolicyEntry* exact=nullptr;
        bool familyKnown=false;
        for (const auto& entry:policy.dependencies()) {
            if (entry.dependencyKind==dependency.kind &&
                entry.dependencyId==dependency.dependencyId) {
                familyKnown=true;
                if (entry.versionMajor==dependency.versionMajor &&
                    entry.versionMinor==dependency.versionMinor &&
                    entry.revisionKind==dependency.revisionKind &&
                    entry.revisionDigest==dependency.revisionDigest) {
                    exact=&entry;
                    break;
                }
            }
        }
        if (exact==nullptr) {
            return reject(familyKnown
                ? ProvenanceAdmissibilityReason::DEPENDENCY_INCOMPATIBLE
                : ProvenanceAdmissibilityReason::DEPENDENCY_UNKNOWN);
        }
        if (exact->semanticCategory==
            DependencySemanticCategory::INTERPRETATION_POLICY) {
            return reject(
                ProvenanceAdmissibilityReason::LEGACY_INTERPRETATION_DEPENDENCY);
        }
        switch (exact->lifecycleStatus) {
        case DependencyLifecycleStatus::ACTIVE:
            break;
        case DependencyLifecycleStatus::RETIRED:
            return reject(ProvenanceAdmissibilityReason::DEPENDENCY_RETIRED);
        case DependencyLifecycleStatus::PROHIBITED:
            return reject(ProvenanceAdmissibilityReason::DEPENDENCY_PROHIBITED);
        case DependencyLifecycleStatus::INCOMPATIBLE:
            return reject(ProvenanceAdmissibilityReason::DEPENDENCY_INCOMPATIBLE);
        }
    }

    if (!required1 || !required2 || !required3) {
        return reject(ProvenanceAdmissibilityReason::REQUIRED_DEPENDENCY_MISSING);
    }

    const auto resolution=resolver.resolve(envelope.sourceCaptureId());
    switch (resolution.status) {
    case SourceResolutionStatus::FOUND:
        if (!resolution.record.has_value()) {
            return reject(ProvenanceAdmissibilityReason::SOURCE_RESOLVER_FAILURE);
        }
        break;
    case SourceResolutionStatus::NOT_FOUND:
        return reject(ProvenanceAdmissibilityReason::SOURCE_RECORD_UNAVAILABLE);
    case SourceResolutionStatus::RESOLVER_FAILURE:
        return reject(ProvenanceAdmissibilityReason::SOURCE_RESOLVER_FAILURE);
    case SourceResolutionStatus::INTEGRITY_CONFLICT:
        return reject(
            ProvenanceAdmissibilityReason::SOURCE_EVIDENCE_INTEGRITY_CONFLICT);
    }

    if (!sourceMatches(*resolution.record,envelope)) {
        return reject(ProvenanceAdmissibilityReason::SOURCE_RECORD_MISMATCH);
    }

    SourceVerificationSummary verification{
        envelope.sourceCaptureId(),
        true
    };

    return ProvenanceAdmissibilityResult{
        AdmissibleProductionProvenance{
            std::move(decisionId),
            policy.policySnapshotId(),
            policy.descriptor(),
            envelope,
            std::move(verification)
        }
    };
}

} // namespace AdaptiveMesh
