#pragma once

#include "production_transition_invariant_snapshot.hpp"
#include "production_transition_request.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

namespace AdaptiveMesh {

class InvariantDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    InvariantDecisionId(const InvariantDecisionId&) noexcept = default;
    InvariantDecisionId& operator=(const InvariantDecisionId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const InvariantDecisionId&, const InvariantDecisionId&) noexcept = default;
private:
    explicit InvariantDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionInvariantEvaluator;
};

class InvariantPolicyId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    InvariantPolicyId(const InvariantPolicyId&) noexcept = default;
    InvariantPolicyId& operator=(const InvariantPolicyId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const InvariantPolicyId&, const InvariantPolicyId&) noexcept = default;
private:
    explicit InvariantPolicyId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionInvariantPolicyProvider;
};

class InvariantPolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    InvariantPolicySnapshotId(const InvariantPolicySnapshotId&) noexcept = default;
    InvariantPolicySnapshotId& operator=(const InvariantPolicySnapshotId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const InvariantPolicySnapshotId&, const InvariantPolicySnapshotId&) noexcept = default;
private:
    explicit InvariantPolicySnapshotId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionInvariantPolicyProvider;
};

struct InvariantPolicyDescriptor final {
    InvariantPolicyId policyId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t,32> implementationRevision;
};

class TransitionInvariantPolicySnapshot final {
public:
    TransitionInvariantPolicySnapshot(const TransitionInvariantPolicySnapshot&) = default;
    TransitionInvariantPolicySnapshot& operator=(const TransitionInvariantPolicySnapshot&) = default;

    [[nodiscard]] const InvariantPolicySnapshotId& snapshotId() const noexcept {
        return snapshotId_;
    }
    [[nodiscard]] const InvariantPolicyDescriptor& descriptor() const noexcept {
        return descriptor_;
    }
    [[nodiscard]] double adjustmentFraction() const noexcept {
        return adjustmentFraction_;
    }
    [[nodiscard]] const ProductionTransitionClassId& transitionClass() const noexcept {
        return transitionClass_;
    }

private:
    TransitionInvariantPolicySnapshot(
        InvariantPolicySnapshotId snapshotId,
        InvariantPolicyDescriptor descriptor,
        double adjustmentFraction,
        ProductionTransitionClassId transitionClass) noexcept
        : snapshotId_(std::move(snapshotId)),
          descriptor_(std::move(descriptor)),
          adjustmentFraction_(adjustmentFraction),
          transitionClass_(transitionClass) {}

    InvariantPolicySnapshotId snapshotId_;
    InvariantPolicyDescriptor descriptor_;
    double adjustmentFraction_;
    ProductionTransitionClassId transitionClass_;

    friend class ProductionTransitionInvariantPolicyProvider;
};

class ProductionTransitionInvariantPolicyProvider final {
public:
    [[nodiscard]] static std::optional<TransitionInvariantPolicySnapshot>
    createCurrent();
};

class ProductionInvariantPrerequisiteRecord final {
public:
    ProductionInvariantPrerequisiteRecord(const ProductionInvariantPrerequisiteRecord&) = default;
    ProductionInvariantPrerequisiteRecord& operator=(const ProductionInvariantPrerequisiteRecord&) = default;

    [[nodiscard]] const InvariantDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const ProductionTransitionRequestBinding& binding() const noexcept {
        return binding_;
    }
    [[nodiscard]] const InvariantPolicySnapshotId& policySnapshotId() const noexcept {
        return policySnapshotId_;
    }
    [[nodiscard]] bool satisfied() const noexcept { return satisfied_; }
    [[nodiscard]] bool sourceIdentityInvariantSatisfied() const noexcept {
        return sourceIdentityInvariantSatisfied_;
    }
    [[nodiscard]] bool targetIdentityInvariantSatisfied() const noexcept {
        return targetIdentityInvariantSatisfied_;
    }
    [[nodiscard]] bool structuralRelationshipInvariantSatisfied() const noexcept {
        return structuralRelationshipInvariantSatisfied_;
    }
    [[nodiscard]] double forwardProjectedCapacity() const noexcept {
        return forwardProjectedCapacity_;
    }
    [[nodiscard]] double reverseProjectedCapacity() const noexcept {
        return reverseProjectedCapacity_;
    }
    [[nodiscard]] bool forwardEffectiveCouplingValid() const noexcept {
        return forwardEffectiveCouplingValid_;
    }
    [[nodiscard]] bool reverseEffectiveCouplingValid() const noexcept {
        return reverseEffectiveCouplingValid_;
    }

private:
    ProductionInvariantPrerequisiteRecord(
        InvariantDecisionId decisionId,
        ProductionTransitionRequestBinding binding,
        InvariantPolicySnapshotId policySnapshotId,
        bool satisfied,
        bool sourceIdentityInvariantSatisfied,
        bool targetIdentityInvariantSatisfied,
        bool structuralRelationshipInvariantSatisfied,
        double forwardProjectedCapacity,
        double reverseProjectedCapacity,
        bool forwardEffectiveCouplingValid,
        bool reverseEffectiveCouplingValid) noexcept
        : decisionId_(std::move(decisionId)),
          binding_(std::move(binding)),
          policySnapshotId_(std::move(policySnapshotId)),
          satisfied_(satisfied),
          sourceIdentityInvariantSatisfied_(sourceIdentityInvariantSatisfied),
          targetIdentityInvariantSatisfied_(targetIdentityInvariantSatisfied),
          structuralRelationshipInvariantSatisfied_(structuralRelationshipInvariantSatisfied),
          forwardProjectedCapacity_(forwardProjectedCapacity),
          reverseProjectedCapacity_(reverseProjectedCapacity),
          forwardEffectiveCouplingValid_(forwardEffectiveCouplingValid),
          reverseEffectiveCouplingValid_(reverseEffectiveCouplingValid) {}

    InvariantDecisionId decisionId_;
    ProductionTransitionRequestBinding binding_;
    InvariantPolicySnapshotId policySnapshotId_;
    bool satisfied_;
    bool sourceIdentityInvariantSatisfied_;
    bool targetIdentityInvariantSatisfied_;
    bool structuralRelationshipInvariantSatisfied_;
    double forwardProjectedCapacity_;
    double reverseProjectedCapacity_;
    bool forwardEffectiveCouplingValid_;
    bool reverseEffectiveCouplingValid_;

    friend class ProductionTransitionInvariantEvaluator;
};

enum class TransitionInvariantReason : std::uint8_t {
    RequestLineageInconsistent,
    TransitionClassUnsupported,
    PolicyRevisionUnrecognized,
    SnapshotUnavailable,
    RequestStateVersionMismatch,
    RelationshipIdentityMismatch,
    ProjectionUnavailable,
    InternalDecisionFailure
};

class ProductionTransitionInvariantRejection final {
public:
    ProductionTransitionInvariantRejection(const ProductionTransitionInvariantRejection&) = default;
    ProductionTransitionInvariantRejection& operator=(const ProductionTransitionInvariantRejection&) = default;

    [[nodiscard]] const InvariantDecisionId& decisionId() const noexcept {
        return decisionId_;
    }
    [[nodiscard]] const TransitionRequestDecisionId& requestDecisionId() const noexcept {
        return requestDecisionId_;
    }
    [[nodiscard]] TransitionInvariantReason primaryReason() const noexcept {
        return primaryReason_;
    }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept {
        return reasonFlags_;
    }

private:
    ProductionTransitionInvariantRejection(
        InvariantDecisionId decisionId,
        TransitionRequestDecisionId requestDecisionId,
        TransitionInvariantReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          requestDecisionId_(std::move(requestDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    InvariantDecisionId decisionId_;
    TransitionRequestDecisionId requestDecisionId_;
    TransitionInvariantReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class ProductionTransitionInvariantEvaluator;
};

using ProductionTransitionInvariantResult =
    std::variant<
        ProductionInvariantPrerequisiteRecord,
        ProductionTransitionInvariantRejection>;

class ProductionTransitionInvariantEvaluator final {
public:
    explicit ProductionTransitionInvariantEvaluator(
        ProductionTransitionInvariantSnapshotSource source) noexcept
        : source_(std::move(source)) {}

    [[nodiscard]]
    std::optional<ProductionTransitionInvariantResult>
    evaluate(
        const ProductionDerivedTransitionRequest& request,
        const TransitionInvariantPolicySnapshot& policy) const;

private:
    ProductionTransitionInvariantSnapshotSource source_;
};

} // namespace AdaptiveMesh
