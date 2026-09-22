#pragma once

#include "adaptive_bridge_policy.hpp"
#include "bridge_persistence.hpp"
#include "versioned_interpretation.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <variant>
#include <utility>

namespace AdaptiveMesh {

namespace detail {
class ProductionPersistenceRegistryState;
class ProductionPersistenceStreamBindingState;
}


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


class ProductionPersistenceStreamKey final {
public:
    ProductionPersistenceStreamKey(const ProductionPersistenceStreamKey&) = default;
    ProductionPersistenceStreamKey& operator=(const ProductionPersistenceStreamKey&) = default;
    ProductionPersistenceStreamKey(ProductionPersistenceStreamKey&&) noexcept = default;
    ProductionPersistenceStreamKey& operator=(ProductionPersistenceStreamKey&&) noexcept = default;

    [[nodiscard]] std::size_t sourceNodeId() const noexcept { return sourceNodeId_; }
    [[nodiscard]] std::size_t targetNodeId() const noexcept { return targetNodeId_; }
    [[nodiscard]] std::uint64_t relationshipGeneration() const noexcept {
        return relationshipGeneration_;
    }
    [[nodiscard]] const InterpretationPolicyId& interpretationPolicyId() const noexcept {
        return interpretationPolicyId_;
    }
    [[nodiscard]] std::uint16_t interpretationPolicyMajor() const noexcept {
        return interpretationPolicyMajor_;
    }
    [[nodiscard]] std::uint16_t interpretationPolicyMinor() const noexcept {
        return interpretationPolicyMinor_;
    }
    [[nodiscard]] std::uint8_t interpretationImplementationRevisionKind() const noexcept {
        return interpretationImplementationRevisionKind_;
    }
    [[nodiscard]] const std::array<std::uint8_t,32>&
    interpretationImplementationRevision() const noexcept {
        return interpretationImplementationRevision_;
    }
    [[nodiscard]] const PersistenceProfileId& persistenceProfileId() const noexcept {
        return persistenceProfileId_;
    }
    [[nodiscard]] std::uint16_t persistenceProfileMajor() const noexcept {
        return persistenceProfileMajor_;
    }
    [[nodiscard]] std::uint16_t persistenceProfileMinor() const noexcept {
        return persistenceProfileMinor_;
    }
    [[nodiscard]] std::uint8_t persistenceImplementationRevisionKind() const noexcept {
        return persistenceImplementationRevisionKind_;
    }
    [[nodiscard]] const std::array<std::uint8_t,32>&
    persistenceImplementationRevision() const noexcept {
        return persistenceImplementationRevision_;
    }

    friend bool operator==(
        const ProductionPersistenceStreamKey&,
        const ProductionPersistenceStreamKey&) noexcept = default;

private:
    ProductionPersistenceStreamKey(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t relationshipGeneration,
        InterpretationPolicyId interpretationPolicyId,
        std::uint16_t interpretationPolicyMajor,
        std::uint16_t interpretationPolicyMinor,
        std::uint8_t interpretationImplementationRevisionKind,
        std::array<std::uint8_t,32> interpretationImplementationRevision,
        PersistenceProfileId persistenceProfileId,
        std::uint16_t persistenceProfileMajor,
        std::uint16_t persistenceProfileMinor,
        std::uint8_t persistenceImplementationRevisionKind,
        std::array<std::uint8_t,32> persistenceImplementationRevision) noexcept
        : sourceNodeId_(sourceNodeId),
          targetNodeId_(targetNodeId),
          relationshipGeneration_(relationshipGeneration),
          interpretationPolicyId_(std::move(interpretationPolicyId)),
          interpretationPolicyMajor_(interpretationPolicyMajor),
          interpretationPolicyMinor_(interpretationPolicyMinor),
          interpretationImplementationRevisionKind_(
              interpretationImplementationRevisionKind),
          interpretationImplementationRevision_(
              interpretationImplementationRevision),
          persistenceProfileId_(std::move(persistenceProfileId)),
          persistenceProfileMajor_(persistenceProfileMajor),
          persistenceProfileMinor_(persistenceProfileMinor),
          persistenceImplementationRevisionKind_(
              persistenceImplementationRevisionKind),
          persistenceImplementationRevision_(persistenceImplementationRevision) {}

    std::size_t sourceNodeId_;
    std::size_t targetNodeId_;
    std::uint64_t relationshipGeneration_;
    InterpretationPolicyId interpretationPolicyId_;
    std::uint16_t interpretationPolicyMajor_;
    std::uint16_t interpretationPolicyMinor_;
    std::uint8_t interpretationImplementationRevisionKind_;
    std::array<std::uint8_t,32> interpretationImplementationRevision_;
    PersistenceProfileId persistenceProfileId_;
    std::uint16_t persistenceProfileMajor_;
    std::uint16_t persistenceProfileMinor_;
    std::uint8_t persistenceImplementationRevisionKind_;
    std::array<std::uint8_t,32> persistenceImplementationRevision_;

    friend class detail::ProductionPersistenceRegistryState;
};

class PersistenceStreamInstanceId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    PersistenceStreamInstanceId(const PersistenceStreamInstanceId&) noexcept = default;
    PersistenceStreamInstanceId& operator=(const PersistenceStreamInstanceId&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const PersistenceStreamInstanceId&,
        const PersistenceStreamInstanceId&) noexcept = default;

private:
    explicit PersistenceStreamInstanceId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class detail::ProductionPersistenceRegistryState;
};

class PersistenceObservationDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    PersistenceObservationDecisionId(const PersistenceObservationDecisionId&) noexcept = default;
    PersistenceObservationDecisionId& operator=(const PersistenceObservationDecisionId&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const PersistenceObservationDecisionId&,
        const PersistenceObservationDecisionId&) noexcept = default;

private:
    explicit PersistenceObservationDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionBridgePersistenceStream;
};

enum class ProductionPersistenceReason : std::uint8_t {
    WrongRelationship,
    WrongRelationshipGeneration,
    WrongInterpretationPolicy,
    WrongPersistenceProfile,
    PreStreamEpochStateVersion,
    DuplicateSourceCapture,
    NonIncreasingStateVersion,
    LineageInconsistent,
    EvidenceInvariantViolation,
    StreamClosed,
    InternalPersistenceFailure
};

class ProductionPersistentBridgeRecommendation final {
public:
    ProductionPersistentBridgeRecommendation(
        const ProductionPersistentBridgeRecommendation&) = default;
    ProductionPersistentBridgeRecommendation& operator=(
        const ProductionPersistentBridgeRecommendation&) = default;
    ProductionPersistentBridgeRecommendation(
        ProductionPersistentBridgeRecommendation&&) noexcept = default;
    ProductionPersistentBridgeRecommendation& operator=(
        ProductionPersistentBridgeRecommendation&&) noexcept = default;

    [[nodiscard]] const PersistenceObservationDecisionId&
    decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const PersistenceStreamInstanceId&
    streamInstanceId() const noexcept { return streamInstanceId_; }
    [[nodiscard]] const ProductionPersistenceStreamKey&
    streamKey() const noexcept { return streamKey_; }
    [[nodiscard]] PersistentBridgeRecommendation recommendation() const noexcept {
        return recommendation_;
    }
    [[nodiscard]] const PolicyEvidenceDecisionId&
    policyEvidenceDecisionId() const noexcept { return policyEvidenceDecisionId_; }
    [[nodiscard]] const SourceCaptureId& sourceCaptureId() const noexcept {
        return sourceCaptureId_;
    }
    [[nodiscard]] std::uint64_t stateVersion() const noexcept {
        return stateVersion_;
    }

private:
    ProductionPersistentBridgeRecommendation(
        PersistenceObservationDecisionId decisionId,
        PersistenceStreamInstanceId streamInstanceId,
        ProductionPersistenceStreamKey streamKey,
        PersistentBridgeRecommendation recommendation,
        PolicyEvidenceDecisionId policyEvidenceDecisionId,
        SourceCaptureId sourceCaptureId,
        std::uint64_t stateVersion) noexcept
        : decisionId_(std::move(decisionId)),
          streamInstanceId_(std::move(streamInstanceId)),
          streamKey_(std::move(streamKey)),
          recommendation_(recommendation),
          policyEvidenceDecisionId_(std::move(policyEvidenceDecisionId)),
          sourceCaptureId_(std::move(sourceCaptureId)),
          stateVersion_(stateVersion) {}

    PersistenceObservationDecisionId decisionId_;
    PersistenceStreamInstanceId streamInstanceId_;
    ProductionPersistenceStreamKey streamKey_;
    PersistentBridgeRecommendation recommendation_;
    PolicyEvidenceDecisionId policyEvidenceDecisionId_;
    SourceCaptureId sourceCaptureId_;
    std::uint64_t stateVersion_;

    friend class ProductionBridgePersistenceStream;
};

class ProductionPersistenceRejection final {
public:
    ProductionPersistenceRejection(const ProductionPersistenceRejection&) = default;
    ProductionPersistenceRejection& operator=(const ProductionPersistenceRejection&) = default;
    ProductionPersistenceRejection(ProductionPersistenceRejection&&) noexcept = default;
    ProductionPersistenceRejection& operator=(ProductionPersistenceRejection&&) noexcept = default;

    [[nodiscard]] const PersistenceObservationDecisionId&
    decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const PersistenceStreamInstanceId&
    streamInstanceId() const noexcept { return streamInstanceId_; }
    [[nodiscard]] const PolicyEvidenceDecisionId&
    policyEvidenceDecisionId() const noexcept { return policyEvidenceDecisionId_; }
    [[nodiscard]] ProductionPersistenceReason primaryReason() const noexcept {
        return primaryReason_;
    }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept {
        return reasonFlags_;
    }

private:
    ProductionPersistenceRejection(
        PersistenceObservationDecisionId decisionId,
        PersistenceStreamInstanceId streamInstanceId,
        PolicyEvidenceDecisionId policyEvidenceDecisionId,
        ProductionPersistenceReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          streamInstanceId_(std::move(streamInstanceId)),
          policyEvidenceDecisionId_(std::move(policyEvidenceDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    PersistenceObservationDecisionId decisionId_;
    PersistenceStreamInstanceId streamInstanceId_;
    PolicyEvidenceDecisionId policyEvidenceDecisionId_;
    ProductionPersistenceReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class ProductionBridgePersistenceStream;
};

using ProductionPersistenceObservationResult =
    std::variant<
        ProductionPersistentBridgeRecommendation,
        ProductionPersistenceRejection>;

class ProductionBridgePersistenceStream final {
public:
    ~ProductionBridgePersistenceStream();

    ProductionBridgePersistenceStream(
        const ProductionBridgePersistenceStream&) = delete;
    ProductionBridgePersistenceStream& operator=(
        const ProductionBridgePersistenceStream&) = delete;
    ProductionBridgePersistenceStream(
        ProductionBridgePersistenceStream&&) = delete;
    ProductionBridgePersistenceStream& operator=(
        ProductionBridgePersistenceStream&&) = delete;

    [[nodiscard]] const PersistenceStreamInstanceId&
    instanceId() const noexcept;
    [[nodiscard]] const ProductionPersistenceStreamKey&
    key() const noexcept;
    [[nodiscard]] std::uint64_t streamStartStateVersion() const noexcept;

    [[nodiscard]] std::optional<ProductionPersistenceObservationResult>
    observe(const ProductionBridgePolicyEvidence& evidence);

private:
    struct Impl;

    ProductionBridgePersistenceStream(
        PersistenceStreamInstanceId instanceId,
        ProductionPersistenceStreamKey key,
        PersistencePolicySnapshot policy,
        std::uint64_t streamStartStateVersion);

    std::unique_ptr<Impl> impl_;

    friend class detail::ProductionPersistenceRegistryState;
};

class ProductionPersistenceStreamHandle final {
public:
    ProductionPersistenceStreamHandle(
        const ProductionPersistenceStreamHandle&) noexcept = default;
    ProductionPersistenceStreamHandle& operator=(
        const ProductionPersistenceStreamHandle&) noexcept = default;

    [[nodiscard]] std::optional<ProductionPersistenceObservationResult>
    observe(const ProductionBridgePolicyEvidence& evidence) const;

    [[nodiscard]] const PersistenceStreamInstanceId&
    instanceId() const noexcept { return instanceId_; }

private:
    ProductionPersistenceStreamHandle(
        std::shared_ptr<detail::ProductionPersistenceStreamBindingState> state,
        PersistenceStreamInstanceId instanceId) noexcept
        : state_(std::move(state)),
          instanceId_(std::move(instanceId)) {}

    std::shared_ptr<detail::ProductionPersistenceStreamBindingState> state_;
    PersistenceStreamInstanceId instanceId_;

    friend class detail::ProductionPersistenceRegistryState;
};

class ProductionBridgePersistenceRegistry final {
public:
    ProductionBridgePersistenceRegistry(
        const ProductionBridgePersistenceRegistry&) noexcept = default;
    ProductionBridgePersistenceRegistry& operator=(
        const ProductionBridgePersistenceRegistry&) noexcept = default;

    [[nodiscard]] std::optional<ProductionPersistenceStreamHandle>
    openLiveStream(
        const ProductionBridgePolicyEvidence& seedLineage,
        const PersistencePolicySnapshot& policy) const;

    [[nodiscard]] bool closeLiveStream(
        const PersistenceStreamInstanceId& instanceId) const;

private:
    explicit ProductionBridgePersistenceRegistry(
        std::shared_ptr<detail::ProductionPersistenceRegistryState> state) noexcept
        : state_(std::move(state)) {}

    std::shared_ptr<detail::ProductionPersistenceRegistryState> state_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
