#pragma once

#include "policy_persistence.hpp"
#include "production_transition_eligibility.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

namespace AdaptiveMesh {

class TransitionRequestDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;

    TransitionRequestDecisionId(const TransitionRequestDecisionId&) noexcept = default;
    TransitionRequestDecisionId& operator=(const TransitionRequestDecisionId&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const TransitionRequestDecisionId&,
        const TransitionRequestDecisionId&) noexcept = default;

private:
    explicit TransitionRequestDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionTransitionRequestDeriver;
};

class TransitionRequestPolicyId final {
public:
    using Bytes = std::array<std::uint8_t,16>;

    TransitionRequestPolicyId(const TransitionRequestPolicyId&) noexcept = default;
    TransitionRequestPolicyId& operator=(const TransitionRequestPolicyId&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const TransitionRequestPolicyId&,
        const TransitionRequestPolicyId&) noexcept = default;

private:
    explicit TransitionRequestPolicyId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionTransitionRequestPolicyProvider;
};

class TransitionRequestPolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t,16>;

    TransitionRequestPolicySnapshotId(
        const TransitionRequestPolicySnapshotId&) noexcept = default;
    TransitionRequestPolicySnapshotId& operator=(
        const TransitionRequestPolicySnapshotId&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const TransitionRequestPolicySnapshotId&,
        const TransitionRequestPolicySnapshotId&) noexcept = default;

private:
    explicit TransitionRequestPolicySnapshotId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionTransitionRequestPolicyProvider;
};

struct TransitionRequestPolicyDescriptor final {
    TransitionRequestPolicyId policyId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t,32> implementationRevision;
};

class TransitionRequestPolicySnapshot final {
public:
    TransitionRequestPolicySnapshot(
        const TransitionRequestPolicySnapshot&) = default;
    TransitionRequestPolicySnapshot& operator=(
        const TransitionRequestPolicySnapshot&) = default;
    TransitionRequestPolicySnapshot(
        TransitionRequestPolicySnapshot&&) noexcept = default;
    TransitionRequestPolicySnapshot& operator=(
        TransitionRequestPolicySnapshot&&) noexcept = default;

    [[nodiscard]] const TransitionRequestPolicySnapshotId&
    snapshotId() const noexcept { return snapshotId_; }

    [[nodiscard]] const TransitionRequestPolicyDescriptor&
    descriptor() const noexcept { return descriptor_; }

    [[nodiscard]] const ProductionTransitionClassId&
    bridgeCouplingAdjustmentClass() const noexcept { return transitionClass_; }

private:
    TransitionRequestPolicySnapshot(
        TransitionRequestPolicySnapshotId snapshotId,
        TransitionRequestPolicyDescriptor descriptor,
        ProductionTransitionClassId transitionClass) noexcept
        : snapshotId_(std::move(snapshotId)),
          descriptor_(std::move(descriptor)),
          transitionClass_(transitionClass) {}

    TransitionRequestPolicySnapshotId snapshotId_;
    TransitionRequestPolicyDescriptor descriptor_;
    ProductionTransitionClassId transitionClass_;

    friend class ProductionTransitionRequestPolicyProvider;
};

class ProductionTransitionRequestPolicyProvider final {
public:
    [[nodiscard]]
    static std::optional<TransitionRequestPolicySnapshot> createCurrent();
};

class ProductionDerivedTransitionRequest final {
public:
    ProductionDerivedTransitionRequest(
        const ProductionDerivedTransitionRequest&) = default;
    ProductionDerivedTransitionRequest& operator=(
        const ProductionDerivedTransitionRequest&) = default;
    ProductionDerivedTransitionRequest(
        ProductionDerivedTransitionRequest&&) noexcept = default;
    ProductionDerivedTransitionRequest& operator=(
        ProductionDerivedTransitionRequest&&) noexcept = default;

    [[nodiscard]] const TransitionRequestDecisionId&
    decisionId() const noexcept { return decisionId_; }

    [[nodiscard]] const ProductionTransitionRequestBinding&
    binding() const noexcept { return binding_; }

    [[nodiscard]] const TransitionRequestPolicySnapshotId&
    policySnapshotId() const noexcept { return policySnapshotId_; }

    [[nodiscard]] const TransitionRequestPolicyDescriptor&
    policyDescriptor() const noexcept { return policyDescriptor_; }

    [[nodiscard]] const PersistenceObservationDecisionId&
    persistenceObservationDecisionId() const noexcept {
        return persistenceObservationDecisionId_;
    }

    [[nodiscard]] const PersistenceStreamInstanceId&
    persistenceStreamInstanceId() const noexcept {
        return persistenceStreamInstanceId_;
    }

    [[nodiscard]] const ProductionPersistenceStreamKey&
    persistenceStreamKey() const noexcept { return persistenceStreamKey_; }

    [[nodiscard]] const PolicyEvidenceDecisionId&
    policyEvidenceDecisionId() const noexcept {
        return policyEvidenceDecisionId_;
    }

    [[nodiscard]] const SourceCaptureId&
    sourceCaptureId() const noexcept { return sourceCaptureId_; }

    [[nodiscard]] PersistentBridgeRecommendation
    sourceRecommendation() const noexcept { return sourceRecommendation_; }

    [[nodiscard]] std::uint64_t stateVersion() const noexcept {
        return stateVersion_;
    }

private:
    ProductionDerivedTransitionRequest(
        TransitionRequestDecisionId decisionId,
        ProductionTransitionRequestBinding binding,
        TransitionRequestPolicySnapshotId policySnapshotId,
        TransitionRequestPolicyDescriptor policyDescriptor,
        PersistenceObservationDecisionId persistenceObservationDecisionId,
        PersistenceStreamInstanceId persistenceStreamInstanceId,
        ProductionPersistenceStreamKey persistenceStreamKey,
        PolicyEvidenceDecisionId policyEvidenceDecisionId,
        SourceCaptureId sourceCaptureId,
        PersistentBridgeRecommendation sourceRecommendation,
        std::uint64_t stateVersion)
        : decisionId_(std::move(decisionId)),
          binding_(std::move(binding)),
          policySnapshotId_(std::move(policySnapshotId)),
          policyDescriptor_(std::move(policyDescriptor)),
          persistenceObservationDecisionId_(
              std::move(persistenceObservationDecisionId)),
          persistenceStreamInstanceId_(
              std::move(persistenceStreamInstanceId)),
          persistenceStreamKey_(std::move(persistenceStreamKey)),
          policyEvidenceDecisionId_(std::move(policyEvidenceDecisionId)),
          sourceCaptureId_(std::move(sourceCaptureId)),
          sourceRecommendation_(sourceRecommendation),
          stateVersion_(stateVersion) {}

    TransitionRequestDecisionId decisionId_;
    ProductionTransitionRequestBinding binding_;
    TransitionRequestPolicySnapshotId policySnapshotId_;
    TransitionRequestPolicyDescriptor policyDescriptor_;
    PersistenceObservationDecisionId persistenceObservationDecisionId_;
    PersistenceStreamInstanceId persistenceStreamInstanceId_;
    ProductionPersistenceStreamKey persistenceStreamKey_;
    PolicyEvidenceDecisionId policyEvidenceDecisionId_;
    SourceCaptureId sourceCaptureId_;
    PersistentBridgeRecommendation sourceRecommendation_;
    std::uint64_t stateVersion_;

    friend class ProductionTransitionRequestDeriver;
};

enum class TransitionRequestDerivationReason : std::uint8_t {
    PreserveRecommendation,
    RecommendationLineageInconsistent,
    PolicyRevisionUnrecognized,
    InternalDeterministicDerivationFailure
};

class ProductionTransitionRequestDerivationRejection final {
public:
    ProductionTransitionRequestDerivationRejection(
        const ProductionTransitionRequestDerivationRejection&) = default;
    ProductionTransitionRequestDerivationRejection& operator=(
        const ProductionTransitionRequestDerivationRejection&) = default;
    ProductionTransitionRequestDerivationRejection(
        ProductionTransitionRequestDerivationRejection&&) noexcept = default;
    ProductionTransitionRequestDerivationRejection& operator=(
        ProductionTransitionRequestDerivationRejection&&) noexcept = default;

    [[nodiscard]] const TransitionRequestDecisionId&
    decisionId() const noexcept { return decisionId_; }

    [[nodiscard]] const PersistenceObservationDecisionId&
    persistenceObservationDecisionId() const noexcept {
        return persistenceObservationDecisionId_;
    }

    [[nodiscard]] TransitionRequestDerivationReason
    primaryReason() const noexcept { return primaryReason_; }

    [[nodiscard]] std::uint64_t reasonFlags() const noexcept {
        return reasonFlags_;
    }

private:
    ProductionTransitionRequestDerivationRejection(
        TransitionRequestDecisionId decisionId,
        PersistenceObservationDecisionId persistenceObservationDecisionId,
        TransitionRequestDerivationReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          persistenceObservationDecisionId_(
              std::move(persistenceObservationDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    TransitionRequestDecisionId decisionId_;
    PersistenceObservationDecisionId persistenceObservationDecisionId_;
    TransitionRequestDerivationReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class ProductionTransitionRequestDeriver;
};

using ProductionTransitionRequestDerivationResult =
    std::variant<
        ProductionDerivedTransitionRequest,
        ProductionTransitionRequestDerivationRejection>;

class ProductionTransitionRequestDeriver final {
public:
    [[nodiscard]]
    std::optional<ProductionTransitionRequestDerivationResult>
    derive(
        const ProductionPersistentBridgeRecommendation& recommendation,
        const TransitionRequestPolicySnapshot& policy) const;
};

} // namespace AdaptiveMesh
