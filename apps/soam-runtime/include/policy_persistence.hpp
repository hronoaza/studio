#pragma once

#include "adaptive_bridge_policy.hpp"
#include "versioned_interpretation.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>

namespace AdaptiveMesh {

class PolicyEvidenceDecisionId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    PolicyEvidenceDecisionId(const PolicyEvidenceDecisionId&) noexcept = default;
    PolicyEvidenceDecisionId& operator=(const PolicyEvidenceDecisionId&) noexcept = default;
    PolicyEvidenceDecisionId(PolicyEvidenceDecisionId&&) noexcept = default;
    PolicyEvidenceDecisionId& operator=(PolicyEvidenceDecisionId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const PolicyEvidenceDecisionId&,
        const PolicyEvidenceDecisionId&) noexcept = default;

private:
    explicit PolicyEvidenceDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionBridgePolicyEvidenceEvaluator;
};

class ProductionBridgePolicyEvidence final {
public:
    ProductionBridgePolicyEvidence(const ProductionBridgePolicyEvidence&) = default;
    ProductionBridgePolicyEvidence& operator=(const ProductionBridgePolicyEvidence&) = default;
    ProductionBridgePolicyEvidence(ProductionBridgePolicyEvidence&&) noexcept = default;
    ProductionBridgePolicyEvidence& operator=(ProductionBridgePolicyEvidence&&) noexcept = default;

    [[nodiscard]] const PolicyEvidenceDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const BridgePolicyEvidence& evidence() const noexcept {
        return evidence_;
    }
    [[nodiscard]] const VersionedProductionInterpretation&
    interpretation() const noexcept {
        return interpretation_;
    }

    [[nodiscard]] const SourceCaptureId& sourceCaptureId() const noexcept {
        return interpretation_.admissibleProvenance().envelope().sourceCaptureId();
    }
    [[nodiscard]] const ProvenanceItemId& provenanceItemId() const noexcept {
        return interpretation_.admissibleProvenance().envelope().provenanceItemId();
    }
    [[nodiscard]] const AdmissibilityDecisionId&
    admissibilityDecisionId() const noexcept {
        return interpretation_.admissibleProvenance().decisionId();
    }
    [[nodiscard]] const InterpretationDecisionId&
    interpretationDecisionId() const noexcept {
        return interpretation_.decisionId();
    }
    [[nodiscard]] std::size_t sourceNodeId() const noexcept {
        return interpretation_.admissibleProvenance().envelope().sourceNodeId();
    }
    [[nodiscard]] std::size_t targetNodeId() const noexcept {
        return interpretation_.admissibleProvenance().envelope().targetNodeId();
    }
    [[nodiscard]] std::uint64_t relationshipGeneration() const noexcept {
        return interpretation_.admissibleProvenance().envelope().relationshipGeneration();
    }
    [[nodiscard]] std::uint64_t stateVersion() const noexcept {
        return interpretation_.admissibleProvenance().envelope().stateVersion();
    }

private:
    ProductionBridgePolicyEvidence(
        PolicyEvidenceDecisionId decisionId,
        BridgePolicyEvidence evidence,
        VersionedProductionInterpretation interpretation)
        : decisionId_(std::move(decisionId)),
          evidence_(evidence),
          interpretation_(std::move(interpretation)) {}

    PolicyEvidenceDecisionId decisionId_;
    BridgePolicyEvidence evidence_;
    VersionedProductionInterpretation interpretation_;

    friend class ProductionBridgePolicyEvidenceEvaluator;
};

enum class ProductionPolicyEvidenceReason : std::uint8_t {
    UpstreamLineageInconsistent,
    EvidenceInvariantViolation,
    InternalDeterministicEvaluationFailure
};

class ProductionPolicyEvidenceRejection final {
public:
    ProductionPolicyEvidenceRejection(const ProductionPolicyEvidenceRejection&) = default;
    ProductionPolicyEvidenceRejection& operator=(const ProductionPolicyEvidenceRejection&) = default;
    ProductionPolicyEvidenceRejection(ProductionPolicyEvidenceRejection&&) noexcept = default;
    ProductionPolicyEvidenceRejection& operator=(ProductionPolicyEvidenceRejection&&) noexcept = default;

    [[nodiscard]] const PolicyEvidenceDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const InterpretationDecisionId&
    interpretationDecisionId() const noexcept {
        return interpretationDecisionId_;
    }
    [[nodiscard]] ProductionPolicyEvidenceReason primaryReason() const noexcept {
        return primaryReason_;
    }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept {
        return reasonFlags_;
    }

private:
    ProductionPolicyEvidenceRejection(
        PolicyEvidenceDecisionId decisionId,
        InterpretationDecisionId interpretationDecisionId,
        ProductionPolicyEvidenceReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          interpretationDecisionId_(std::move(interpretationDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    PolicyEvidenceDecisionId decisionId_;
    InterpretationDecisionId interpretationDecisionId_;
    ProductionPolicyEvidenceReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class ProductionBridgePolicyEvidenceEvaluator;
};

using ProductionPolicyEvidenceResult =
    std::variant<
        ProductionBridgePolicyEvidence,
        ProductionPolicyEvidenceRejection>;

class ProductionBridgePolicyEvidenceEvaluator final {
public:
    [[nodiscard]] std::optional<ProductionPolicyEvidenceResult> evaluate(
        const VersionedProductionInterpretation& interpretation) const;
};

class PersistenceProfileId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    PersistenceProfileId(const PersistenceProfileId&) noexcept = default;
    PersistenceProfileId& operator=(const PersistenceProfileId&) noexcept = default;
    PersistenceProfileId(PersistenceProfileId&&) noexcept = default;
    PersistenceProfileId& operator=(PersistenceProfileId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const PersistenceProfileId&,
        const PersistenceProfileId&) noexcept = default;

private:
    explicit PersistenceProfileId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionPersistencePolicyProvider;
};

class PersistencePolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    PersistencePolicySnapshotId(const PersistencePolicySnapshotId&) noexcept = default;
    PersistencePolicySnapshotId& operator=(const PersistencePolicySnapshotId&) noexcept = default;
    PersistencePolicySnapshotId(PersistencePolicySnapshotId&&) noexcept = default;
    PersistencePolicySnapshotId& operator=(PersistencePolicySnapshotId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const PersistencePolicySnapshotId&,
        const PersistencePolicySnapshotId&) noexcept = default;

private:
    explicit PersistencePolicySnapshotId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionPersistencePolicyProvider;
};

struct PersistencePolicyDescriptor final {
    PersistenceProfileId profileId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t, 32> implementationRevision;
};

class PersistencePolicySnapshot final {
public:
    PersistencePolicySnapshot(const PersistencePolicySnapshot&) = default;
    PersistencePolicySnapshot& operator=(const PersistencePolicySnapshot&) = default;
    PersistencePolicySnapshot(PersistencePolicySnapshot&&) noexcept = default;
    PersistencePolicySnapshot& operator=(PersistencePolicySnapshot&&) noexcept = default;

    [[nodiscard]] const PersistencePolicySnapshotId& snapshotId() const noexcept {
        return snapshotId_;
    }
    [[nodiscard]] const PersistencePolicyDescriptor& descriptor() const noexcept {
        return descriptor_;
    }
    [[nodiscard]] double activationThreshold() const noexcept {
        return activationThreshold_;
    }
    [[nodiscard]] double releaseThreshold() const noexcept {
        return releaseThreshold_;
    }
    [[nodiscard]] std::size_t activationSamples() const noexcept {
        return activationSamples_;
    }
    [[nodiscard]] std::size_t releaseSamples() const noexcept {
        return releaseSamples_;
    }

private:
    PersistencePolicySnapshot(
        PersistencePolicySnapshotId snapshotId,
        PersistencePolicyDescriptor descriptor,
        double activationThreshold,
        double releaseThreshold,
        std::size_t activationSamples,
        std::size_t releaseSamples) noexcept
        : snapshotId_(std::move(snapshotId)),
          descriptor_(std::move(descriptor)),
          activationThreshold_(activationThreshold),
          releaseThreshold_(releaseThreshold),
          activationSamples_(activationSamples),
          releaseSamples_(releaseSamples) {}

    PersistencePolicySnapshotId snapshotId_;
    PersistencePolicyDescriptor descriptor_;
    double activationThreshold_;
    double releaseThreshold_;
    std::size_t activationSamples_;
    std::size_t releaseSamples_;

    friend class ProductionPersistencePolicyProvider;
};

class ProductionPersistencePolicyProvider final {
public:
    [[nodiscard]] static std::optional<PersistencePolicySnapshot>
    createCurrent();
};

} // namespace AdaptiveMesh
