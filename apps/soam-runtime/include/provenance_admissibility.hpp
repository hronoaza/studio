#pragma once

#include "production_provenance_envelope.hpp"
#include "retained_source_evidence.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace AdaptiveMesh {

namespace detail {
struct ProvenanceAdmissibilityTestAccess;
}

class PolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    PolicySnapshotId(const PolicySnapshotId&) noexcept = default;
    PolicySnapshotId& operator=(const PolicySnapshotId&) noexcept = default;
    PolicySnapshotId(PolicySnapshotId&&) noexcept = default;
    PolicySnapshotId& operator=(PolicySnapshotId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const PolicySnapshotId&,
        const PolicySnapshotId&) noexcept = default;

private:
    explicit PolicySnapshotId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionProvenanceAdmissibilityPolicyProvider;
    friend struct detail::ProvenanceAdmissibilityTestAccess;
};

class AdmissibilityDecisionId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    AdmissibilityDecisionId(const AdmissibilityDecisionId&) noexcept = default;
    AdmissibilityDecisionId& operator=(const AdmissibilityDecisionId&) noexcept = default;
    AdmissibilityDecisionId(AdmissibilityDecisionId&&) noexcept = default;
    AdmissibilityDecisionId& operator=(AdmissibilityDecisionId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const AdmissibilityDecisionId&,
        const AdmissibilityDecisionId&) noexcept = default;

private:
    explicit AdmissibilityDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProvenanceAdmissibilityEvaluator;
};

struct AdmissibilityPolicyDescriptor final {
    std::array<std::uint8_t, 16> policyId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
};

enum class ProducerLifecycleStatus : std::uint8_t {
    Recognized,
    Retired,
    Prohibited
};

struct ProducerPolicyEntry final {
    ProvenanceMetadataView::Id128 producerId;
    std::uint16_t producerMajor;
    std::uint16_t producerMinor;
    std::uint8_t implementationRevisionKind;
    ProvenanceMetadataView::Digest256 implementationRevision;
    ProducerLifecycleStatus status;
};

enum class SchemaLifecycleStatus : std::uint8_t {
    Compatible,
    Retired,
    Prohibited,
    Incompatible
};

struct SchemaPolicyEntry final {
    ProvenanceMetadataView::Id128 schemaId;
    std::uint16_t schemaMajor;
    std::uint16_t schemaMinor;
    std::uint16_t canonicalEncodingVersion;
    SchemaLifecycleStatus status;
};

enum class DependencySemanticCategory : std::uint8_t {
    SourceCaptureContract,
    CanonicalEncoding,
    DigestProfile,
    RuntimeContract,
    InterpretationPolicy,
    Other
};

enum class DependencyLifecycleStatus : std::uint8_t {
    Active,
    Retired,
    Prohibited,
    Incompatible
};

struct DependencyPolicyEntry final {
    std::uint8_t dependencyKind;
    ProvenanceMetadataView::Id128 dependencyId;
    std::uint16_t versionMajor;
    std::uint16_t versionMinor;
    std::uint8_t revisionKind;
    ProvenanceMetadataView::Digest256 revisionDigest;
    DependencySemanticCategory category;
    DependencyLifecycleStatus status;
};

class ProvenanceAdmissibilityPolicySnapshot final {
public:
    ProvenanceAdmissibilityPolicySnapshot(
        const ProvenanceAdmissibilityPolicySnapshot&) = default;
    ProvenanceAdmissibilityPolicySnapshot& operator=(
        const ProvenanceAdmissibilityPolicySnapshot&) = default;
    ProvenanceAdmissibilityPolicySnapshot(
        ProvenanceAdmissibilityPolicySnapshot&&) noexcept = default;
    ProvenanceAdmissibilityPolicySnapshot& operator=(
        ProvenanceAdmissibilityPolicySnapshot&&) noexcept = default;

    [[nodiscard]] const PolicySnapshotId& policySnapshotId() const noexcept {
        return policySnapshotId_;
    }
    [[nodiscard]] const AdmissibilityPolicyDescriptor& descriptor() const noexcept {
        return descriptor_;
    }
    [[nodiscard]] const std::vector<ProducerPolicyEntry>& producers() const noexcept {
        return producers_;
    }
    [[nodiscard]] const std::vector<SchemaPolicyEntry>& schemas() const noexcept {
        return schemas_;
    }
    [[nodiscard]] const std::vector<DependencyPolicyEntry>& dependencies() const noexcept {
        return dependencies_;
    }
    [[nodiscard]] const std::vector<std::uint8_t>& requiredDependencyKinds() const noexcept {
        return requiredDependencyKinds_;
    }

private:
    ProvenanceAdmissibilityPolicySnapshot(
        PolicySnapshotId policySnapshotId,
        AdmissibilityPolicyDescriptor descriptor,
        std::vector<ProducerPolicyEntry> producers,
        std::vector<SchemaPolicyEntry> schemas,
        std::vector<DependencyPolicyEntry> dependencies,
        std::vector<std::uint8_t> requiredDependencyKinds) noexcept
        : policySnapshotId_(std::move(policySnapshotId)),
          descriptor_(descriptor),
          producers_(std::move(producers)),
          schemas_(std::move(schemas)),
          dependencies_(std::move(dependencies)),
          requiredDependencyKinds_(std::move(requiredDependencyKinds)) {}

    PolicySnapshotId policySnapshotId_;
    AdmissibilityPolicyDescriptor descriptor_;
    std::vector<ProducerPolicyEntry> producers_;
    std::vector<SchemaPolicyEntry> schemas_;
    std::vector<DependencyPolicyEntry> dependencies_;
    std::vector<std::uint8_t> requiredDependencyKinds_;

    friend class ProductionProvenanceAdmissibilityPolicyProvider;
    friend struct detail::ProvenanceAdmissibilityTestAccess;
};

class ProductionProvenanceAdmissibilityPolicyProvider final {
public:
    [[nodiscard]] static std::optional<ProvenanceAdmissibilityPolicySnapshot>
    createCurrent();
};

enum class ProvenanceAdmissibilityReason : std::uint8_t {
    PolicyUnavailable,
    MetadataInconsistent,
    CanonicalDigestMismatch,
    ProducerUnknown,
    ProducerRetired,
    ProducerProhibited,
    ImplementationRevisionUnrecognized,
    SchemaUnknown,
    SchemaIncompatible,
    SchemaRetired,
    CanonicalEncodingUnsupported,
    RequiredDependencyMissing,
    DependencyUnknown,
    DependencyRetired,
    DependencyProhibited,
    DependencyIncompatible,
    LegacyInterpretationDependency,
    SourceResolverFailure,
    SourceEvidenceIntegrityConflict,
    SourceRecordUnavailable,
    SourceRecordMismatch
};

struct SourceVerificationSummary final {
    SourceCaptureId sourceCaptureId;
    bool bitExactMatch;
};

class AdmissibleProductionProvenance final {
public:
    AdmissibleProductionProvenance(
        const AdmissibleProductionProvenance&) = default;
    AdmissibleProductionProvenance& operator=(
        const AdmissibleProductionProvenance&) = default;
    AdmissibleProductionProvenance(
        AdmissibleProductionProvenance&&) noexcept = default;
    AdmissibleProductionProvenance& operator=(
        AdmissibleProductionProvenance&&) noexcept = default;

    [[nodiscard]] const AdmissibilityDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const PolicySnapshotId& policySnapshotId() const noexcept {
        return policySnapshotId_;
    }
    [[nodiscard]] const AdmissibilityPolicyDescriptor& policyDescriptor() const noexcept {
        return policyDescriptor_;
    }
    [[nodiscard]] const ProductionProvenanceEnvelope& envelope() const noexcept {
        return envelope_;
    }
    [[nodiscard]] const SourceVerificationSummary& sourceVerification() const noexcept {
        return sourceVerification_;
    }

private:
    AdmissibleProductionProvenance(
        AdmissibilityDecisionId decisionId,
        PolicySnapshotId policySnapshotId,
        AdmissibilityPolicyDescriptor policyDescriptor,
        ProductionProvenanceEnvelope envelope,
        SourceVerificationSummary sourceVerification) noexcept
        : decisionId_(std::move(decisionId)),
          policySnapshotId_(std::move(policySnapshotId)),
          policyDescriptor_(policyDescriptor),
          envelope_(std::move(envelope)),
          sourceVerification_(std::move(sourceVerification)) {}

    AdmissibilityDecisionId decisionId_;
    PolicySnapshotId policySnapshotId_;
    AdmissibilityPolicyDescriptor policyDescriptor_;
    ProductionProvenanceEnvelope envelope_;
    SourceVerificationSummary sourceVerification_;

    friend class ProvenanceAdmissibilityEvaluator;
};

class ProvenanceAdmissibilityRejection final {
public:
    ProvenanceAdmissibilityRejection(
        const ProvenanceAdmissibilityRejection&) = default;
    ProvenanceAdmissibilityRejection& operator=(
        const ProvenanceAdmissibilityRejection&) = default;
    ProvenanceAdmissibilityRejection(
        ProvenanceAdmissibilityRejection&&) noexcept = default;
    ProvenanceAdmissibilityRejection& operator=(
        ProvenanceAdmissibilityRejection&&) noexcept = default;

    [[nodiscard]] const AdmissibilityDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const PolicySnapshotId& policySnapshotId() const noexcept {
        return policySnapshotId_;
    }
    [[nodiscard]] const ProvenanceItemId& provenanceItemId() const noexcept {
        return provenanceItemId_;
    }
    [[nodiscard]] ProvenanceAdmissibilityReason primaryReason() const noexcept {
        return primaryReason_;
    }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept {
        return reasonFlags_;
    }

private:
    ProvenanceAdmissibilityRejection(
        AdmissibilityDecisionId decisionId,
        PolicySnapshotId policySnapshotId,
        ProvenanceItemId provenanceItemId,
        ProvenanceAdmissibilityReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          policySnapshotId_(std::move(policySnapshotId)),
          provenanceItemId_(std::move(provenanceItemId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    AdmissibilityDecisionId decisionId_;
    PolicySnapshotId policySnapshotId_;
    ProvenanceItemId provenanceItemId_;
    ProvenanceAdmissibilityReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class ProvenanceAdmissibilityEvaluator;
};

using ProvenanceAdmissibilityResult =
    std::variant<
        AdmissibleProductionProvenance,
        ProvenanceAdmissibilityRejection>;

class ProvenanceAdmissibilityEvaluator final {
public:
    [[nodiscard]] std::optional<ProvenanceAdmissibilityResult> evaluate(
        const ProductionProvenanceEnvelope& envelope,
        const ProvenanceAdmissibilityPolicySnapshot& policy,
        const SourceEvidenceResolver& resolver) const;
};

} // namespace AdaptiveMesh
