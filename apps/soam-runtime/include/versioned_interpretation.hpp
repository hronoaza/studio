#pragma once

#include "bridge_confidence.hpp"
#include "interaction_observation.hpp"
#include "provenance_admissibility.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

namespace AdaptiveMesh {

namespace detail {
struct VersionedInterpretationTestAccess;
}

class InterpretationPolicyId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    InterpretationPolicyId(const InterpretationPolicyId&) noexcept = default;
    InterpretationPolicyId& operator=(const InterpretationPolicyId&) noexcept = default;
    InterpretationPolicyId(InterpretationPolicyId&&) noexcept = default;
    InterpretationPolicyId& operator=(InterpretationPolicyId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const InterpretationPolicyId&,
        const InterpretationPolicyId&) noexcept = default;

private:
    explicit InterpretationPolicyId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionInterpretationPolicyProvider;
    friend struct detail::VersionedInterpretationTestAccess;
};

class InterpretationPolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    InterpretationPolicySnapshotId(
        const InterpretationPolicySnapshotId&) noexcept = default;
    InterpretationPolicySnapshotId& operator=(
        const InterpretationPolicySnapshotId&) noexcept = default;
    InterpretationPolicySnapshotId(
        InterpretationPolicySnapshotId&&) noexcept = default;
    InterpretationPolicySnapshotId& operator=(
        InterpretationPolicySnapshotId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const InterpretationPolicySnapshotId&,
        const InterpretationPolicySnapshotId&) noexcept = default;

private:
    explicit InterpretationPolicySnapshotId(Bytes bytes) noexcept
        : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionInterpretationPolicyProvider;
    friend struct detail::VersionedInterpretationTestAccess;
};

class InterpretationDecisionId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    InterpretationDecisionId(const InterpretationDecisionId&) noexcept = default;
    InterpretationDecisionId& operator=(const InterpretationDecisionId&) noexcept = default;
    InterpretationDecisionId(InterpretationDecisionId&&) noexcept = default;
    InterpretationDecisionId& operator=(InterpretationDecisionId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const InterpretationDecisionId&,
        const InterpretationDecisionId&) noexcept = default;

private:
    explicit InterpretationDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class VersionedProductionInterpreter;
};

struct InterpretationPolicyDescriptor final {
    InterpretationPolicyId policyId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t, 32> implementationRevision;
};

class InterpretationPolicySnapshot final {
public:
    InterpretationPolicySnapshot(const InterpretationPolicySnapshot&) = default;
    InterpretationPolicySnapshot& operator=(const InterpretationPolicySnapshot&) = default;
    InterpretationPolicySnapshot(InterpretationPolicySnapshot&&) noexcept = default;
    InterpretationPolicySnapshot& operator=(InterpretationPolicySnapshot&&) noexcept = default;

    [[nodiscard]] const InterpretationPolicySnapshotId& snapshotId() const noexcept {
        return snapshotId_;
    }
    [[nodiscard]] const InterpretationPolicyDescriptor& descriptor() const noexcept {
        return descriptor_;
    }
    [[nodiscard]] double distanceAttenuationCoefficient() const noexcept {
        return distanceAttenuationCoefficient_;
    }
    [[nodiscard]] bool directional() const noexcept { return directional_; }

private:
    InterpretationPolicySnapshot(
        InterpretationPolicySnapshotId snapshotId,
        InterpretationPolicyDescriptor descriptor,
        double distanceAttenuationCoefficient,
        bool directional) noexcept
        : snapshotId_(std::move(snapshotId)),
          descriptor_(std::move(descriptor)),
          distanceAttenuationCoefficient_(distanceAttenuationCoefficient),
          directional_(directional) {}

    InterpretationPolicySnapshotId snapshotId_;
    InterpretationPolicyDescriptor descriptor_;
    double distanceAttenuationCoefficient_;
    bool directional_;

    friend class ProductionInterpretationPolicyProvider;
    friend struct detail::VersionedInterpretationTestAccess;
};

class ProductionInterpretationPolicyProvider final {
public:
    [[nodiscard]] static std::optional<InterpretationPolicySnapshot>
    createCurrent();
};

struct InterpretationTrace final {
    double distance;
    double orientationWeight;
    double capacity;
    double sourceHealth;
    double targetHealth;
    double attenuation;
    double compatibility;
    double confidence;
};

enum class ProductionInterpretationReason : std::uint8_t {
    PolicyUnavailable,
    PolicyRevisionUnrecognized,
    UpstreamLineageInconsistent,
    DistanceInvalid,
    OrientationWeightInvalid,
    CapacityInvalid,
    SourceHealthInvalid,
    TargetHealthInvalid,
    CompatibilityComputationInvalid,
    ConfidenceComputationInvalid,
    OutputInvariantViolation,
    InternalDeterministicEvaluationFailure
};

class VersionedProductionInterpretation final {
public:
    VersionedProductionInterpretation(
        const VersionedProductionInterpretation&) = default;
    VersionedProductionInterpretation& operator=(
        const VersionedProductionInterpretation&) = default;
    VersionedProductionInterpretation(
        VersionedProductionInterpretation&&) noexcept = default;
    VersionedProductionInterpretation& operator=(
        VersionedProductionInterpretation&&) noexcept = default;

    [[nodiscard]] const InterpretationDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const InterpretationPolicySnapshotId&
    interpretationPolicySnapshotId() const noexcept {
        return interpretationPolicySnapshotId_;
    }
    [[nodiscard]] const InterpretationPolicyDescriptor&
    interpretationPolicyDescriptor() const noexcept {
        return interpretationPolicyDescriptor_;
    }
    [[nodiscard]] const AdmissibleProductionProvenance&
    admissibleProvenance() const noexcept {
        return admissibleProvenance_;
    }
    [[nodiscard]] const InteractionObservation& observation() const noexcept {
        return observation_;
    }
    [[nodiscard]] const BridgeConfidence& confidence() const noexcept {
        return confidence_;
    }
    [[nodiscard]] const InterpretationTrace& trace() const noexcept {
        return trace_;
    }

private:
    VersionedProductionInterpretation(
        InterpretationDecisionId decisionId,
        InterpretationPolicySnapshotId interpretationPolicySnapshotId,
        InterpretationPolicyDescriptor interpretationPolicyDescriptor,
        AdmissibleProductionProvenance admissibleProvenance,
        InteractionObservation observation,
        BridgeConfidence confidence,
        InterpretationTrace trace) noexcept
        : decisionId_(std::move(decisionId)),
          interpretationPolicySnapshotId_(
              std::move(interpretationPolicySnapshotId)),
          interpretationPolicyDescriptor_(
              std::move(interpretationPolicyDescriptor)),
          admissibleProvenance_(std::move(admissibleProvenance)),
          observation_(observation),
          confidence_(confidence),
          trace_(trace) {}

    InterpretationDecisionId decisionId_;
    InterpretationPolicySnapshotId interpretationPolicySnapshotId_;
    InterpretationPolicyDescriptor interpretationPolicyDescriptor_;
    AdmissibleProductionProvenance admissibleProvenance_;
    InteractionObservation observation_;
    BridgeConfidence confidence_;
    InterpretationTrace trace_;

    friend class VersionedProductionInterpreter;
};

class ProductionInterpretationRejection final {
public:
    ProductionInterpretationRejection(
        const ProductionInterpretationRejection&) = default;
    ProductionInterpretationRejection& operator=(
        const ProductionInterpretationRejection&) = default;
    ProductionInterpretationRejection(
        ProductionInterpretationRejection&&) noexcept = default;
    ProductionInterpretationRejection& operator=(
        ProductionInterpretationRejection&&) noexcept = default;

    [[nodiscard]] const InterpretationDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const InterpretationPolicySnapshotId&
    interpretationPolicySnapshotId() const noexcept {
        return interpretationPolicySnapshotId_;
    }
    [[nodiscard]] const ProvenanceItemId& provenanceItemId() const noexcept {
        return provenanceItemId_;
    }
    [[nodiscard]] const AdmissibilityDecisionId&
    admissibilityDecisionId() const noexcept {
        return admissibilityDecisionId_;
    }
    [[nodiscard]] ProductionInterpretationReason primaryReason() const noexcept {
        return primaryReason_;
    }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept {
        return reasonFlags_;
    }

private:
    ProductionInterpretationRejection(
        InterpretationDecisionId decisionId,
        InterpretationPolicySnapshotId interpretationPolicySnapshotId,
        ProvenanceItemId provenanceItemId,
        AdmissibilityDecisionId admissibilityDecisionId,
        ProductionInterpretationReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          interpretationPolicySnapshotId_(
              std::move(interpretationPolicySnapshotId)),
          provenanceItemId_(std::move(provenanceItemId)),
          admissibilityDecisionId_(std::move(admissibilityDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    InterpretationDecisionId decisionId_;
    InterpretationPolicySnapshotId interpretationPolicySnapshotId_;
    ProvenanceItemId provenanceItemId_;
    AdmissibilityDecisionId admissibilityDecisionId_;
    ProductionInterpretationReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class VersionedProductionInterpreter;
};

using ProductionInterpretationResult =
    std::variant<
        VersionedProductionInterpretation,
        ProductionInterpretationRejection>;

class VersionedProductionInterpreter final {
public:
    [[nodiscard]] std::optional<ProductionInterpretationResult> interpret(
        const AdmissibleProductionProvenance& provenance,
        const InterpretationPolicySnapshot& policy) const;
};

} // namespace AdaptiveMesh
