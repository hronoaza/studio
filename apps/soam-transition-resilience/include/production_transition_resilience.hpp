#pragma once

#include "production_transition_resilience_snapshot.hpp"
#include "production_transition_request.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

namespace AdaptiveMesh {

enum class ResilienceCriterion : std::uint8_t {
    single_request_relationship_loss_survivability = 1
};

class ResilienceDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    ResilienceDecisionId(const ResilienceDecisionId&) noexcept = default;
    ResilienceDecisionId& operator=(const ResilienceDecisionId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const ResilienceDecisionId&, const ResilienceDecisionId&) noexcept = default;
private:
    explicit ResilienceDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionResilienceEvaluator;
};

class ResiliencePolicyId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    ResiliencePolicyId(const ResiliencePolicyId&) noexcept = default;
    ResiliencePolicyId& operator=(const ResiliencePolicyId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const ResiliencePolicyId&, const ResiliencePolicyId&) noexcept = default;
private:
    explicit ResiliencePolicyId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionResiliencePolicyProvider;
};

class ResiliencePolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    ResiliencePolicySnapshotId(const ResiliencePolicySnapshotId&) noexcept = default;
    ResiliencePolicySnapshotId& operator=(const ResiliencePolicySnapshotId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const ResiliencePolicySnapshotId&, const ResiliencePolicySnapshotId&) noexcept = default;
private:
    explicit ResiliencePolicySnapshotId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionResiliencePolicyProvider;
};

struct ResiliencePolicyDescriptor final {
    ResiliencePolicyId policyId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t,32> implementationRevision;
};

class TransitionResiliencePolicySnapshot final {
public:
    TransitionResiliencePolicySnapshot(
        const TransitionResiliencePolicySnapshot&) = default;
    TransitionResiliencePolicySnapshot& operator=(
        const TransitionResiliencePolicySnapshot&) = default;

    [[nodiscard]] const ResiliencePolicySnapshotId& snapshotId() const noexcept {
        return snapshotId_;
    }
    [[nodiscard]] const ResiliencePolicyDescriptor& descriptor() const noexcept {
        return descriptor_;
    }
    [[nodiscard]] ResilienceCriterion criterion() const noexcept {
        return criterion_;
    }
    [[nodiscard]] const ProductionTransitionClassId& transitionClass() const noexcept {
        return transitionClass_;
    }

private:
    TransitionResiliencePolicySnapshot(
        ResiliencePolicySnapshotId snapshotId,
        ResiliencePolicyDescriptor descriptor,
        ResilienceCriterion criterion,
        ProductionTransitionClassId transitionClass) noexcept
        : snapshotId_(std::move(snapshotId)),
          descriptor_(std::move(descriptor)),
          criterion_(criterion),
          transitionClass_(transitionClass) {}

    ResiliencePolicySnapshotId snapshotId_;
    ResiliencePolicyDescriptor descriptor_;
    ResilienceCriterion criterion_;
    ProductionTransitionClassId transitionClass_;

    friend class ProductionTransitionResiliencePolicyProvider;
};

class ProductionTransitionResiliencePolicyProvider final {
public:
    [[nodiscard]] static std::optional<TransitionResiliencePolicySnapshot>
    createCurrent();
};

class ProductionResiliencePrerequisiteRecord final {
public:
    ProductionResiliencePrerequisiteRecord(
        const ProductionResiliencePrerequisiteRecord&) = default;
    ProductionResiliencePrerequisiteRecord& operator=(
        const ProductionResiliencePrerequisiteRecord&) = default;

    [[nodiscard]] const ResilienceDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const ProductionTransitionRequestBinding& binding() const noexcept {
        return binding_;
    }
    [[nodiscard]] const ResiliencePolicySnapshotId& policySnapshotId() const noexcept {
        return policySnapshotId_;
    }
    [[nodiscard]] bool satisfied() const noexcept { return satisfied_; }
    [[nodiscard]] std::size_t alternativePathHopCount() const noexcept {
        return alternativePathHopCount_;
    }

private:
    ProductionResiliencePrerequisiteRecord(
        ResilienceDecisionId decisionId,
        ProductionTransitionRequestBinding binding,
        ResiliencePolicySnapshotId policySnapshotId,
        bool satisfied,
        std::size_t alternativePathHopCount) noexcept
        : decisionId_(std::move(decisionId)),
          binding_(std::move(binding)),
          policySnapshotId_(std::move(policySnapshotId)),
          satisfied_(satisfied),
          alternativePathHopCount_(alternativePathHopCount) {}

    ResilienceDecisionId decisionId_;
    ProductionTransitionRequestBinding binding_;
    ResiliencePolicySnapshotId policySnapshotId_;
    bool satisfied_;
    std::size_t alternativePathHopCount_;

    friend class ProductionTransitionResilienceEvaluator;
};

enum class TransitionResilienceReason : std::uint8_t {
    RequestLineageInconsistent,
    TransitionClassUnsupported,
    PolicyRevisionUnrecognized,
    SnapshotUnavailable,
    RequestStateVersionMismatch,
    RelationshipIdentityMismatch,
    TopologySnapshotInvalid,
    InternalDecisionFailure
};

class ProductionTransitionResilienceRejection final {
public:
    ProductionTransitionResilienceRejection(
        const ProductionTransitionResilienceRejection&) = default;
    ProductionTransitionResilienceRejection& operator=(
        const ProductionTransitionResilienceRejection&) = default;

    [[nodiscard]] const ResilienceDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const TransitionRequestDecisionId& requestDecisionId() const noexcept {
        return requestDecisionId_;
    }
    [[nodiscard]] TransitionResilienceReason primaryReason() const noexcept {
        return primaryReason_;
    }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept {
        return reasonFlags_;
    }

private:
    ProductionTransitionResilienceRejection(
        ResilienceDecisionId decisionId,
        TransitionRequestDecisionId requestDecisionId,
        TransitionResilienceReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          requestDecisionId_(std::move(requestDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    ResilienceDecisionId decisionId_;
    TransitionRequestDecisionId requestDecisionId_;
    TransitionResilienceReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class ProductionTransitionResilienceEvaluator;
};

using ProductionTransitionResilienceResult =
    std::variant<
        ProductionResiliencePrerequisiteRecord,
        ProductionTransitionResilienceRejection>;

class ProductionTransitionResilienceEvaluator final {
public:
    explicit ProductionTransitionResilienceEvaluator(
        ProductionTransitionResilienceSnapshotSource source) noexcept
        : source_(std::move(source)) {}

    [[nodiscard]]
    std::optional<ProductionTransitionResilienceResult>
    evaluate(
        const ProductionDerivedTransitionRequest& request,
        const TransitionResiliencePolicySnapshot& policy) const;

private:
    ProductionTransitionResilienceSnapshotSource source_;
};

} // namespace AdaptiveMesh
