#pragma once

#include "production_transition_live_snapshot.hpp"
#include "production_transition_request.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

namespace AdaptiveMesh {

class LiveValidityDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    LiveValidityDecisionId(const LiveValidityDecisionId&) noexcept = default;
    LiveValidityDecisionId& operator=(const LiveValidityDecisionId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const LiveValidityDecisionId&, const LiveValidityDecisionId&) noexcept = default;
private:
    explicit LiveValidityDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionLiveValidityEvaluator;
};

class FreshnessDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    FreshnessDecisionId(const FreshnessDecisionId&) noexcept = default;
    FreshnessDecisionId& operator=(const FreshnessDecisionId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const FreshnessDecisionId&, const FreshnessDecisionId&) noexcept = default;
private:
    explicit FreshnessDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionLiveValidityEvaluator;
};

class RevalidationDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    RevalidationDecisionId(const RevalidationDecisionId&) noexcept = default;
    RevalidationDecisionId& operator=(const RevalidationDecisionId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const RevalidationDecisionId&, const RevalidationDecisionId&) noexcept = default;
private:
    explicit RevalidationDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionTransitionLiveValidityEvaluator;
};

class ProductionFreshnessPrerequisiteRecord final {
public:
    ProductionFreshnessPrerequisiteRecord(const ProductionFreshnessPrerequisiteRecord&) = default;
    ProductionFreshnessPrerequisiteRecord& operator=(const ProductionFreshnessPrerequisiteRecord&) = default;

    [[nodiscard]] const FreshnessDecisionId& decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const ProductionTransitionRequestBinding& binding() const noexcept { return binding_; }
    [[nodiscard]] std::uint64_t observedStateVersion() const noexcept { return observedStateVersion_; }
    [[nodiscard]] bool satisfied() const noexcept { return satisfied_; }

private:
    ProductionFreshnessPrerequisiteRecord(
        FreshnessDecisionId decisionId,
        ProductionTransitionRequestBinding binding,
        std::uint64_t observedStateVersion,
        bool satisfied) noexcept
        : decisionId_(std::move(decisionId)),
          binding_(std::move(binding)),
          observedStateVersion_(observedStateVersion),
          satisfied_(satisfied) {}

    FreshnessDecisionId decisionId_;
    ProductionTransitionRequestBinding binding_;
    std::uint64_t observedStateVersion_;
    bool satisfied_;

    friend class ProductionTransitionLiveValidityEvaluator;
};

class ProductionRevalidationPrerequisiteRecord final {
public:
    ProductionRevalidationPrerequisiteRecord(const ProductionRevalidationPrerequisiteRecord&) = default;
    ProductionRevalidationPrerequisiteRecord& operator=(const ProductionRevalidationPrerequisiteRecord&) = default;

    [[nodiscard]] const RevalidationDecisionId& decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const ProductionTransitionRequestBinding& binding() const noexcept { return binding_; }
    [[nodiscard]] bool relationshipPresent() const noexcept { return relationshipPresent_; }
    [[nodiscard]] const std::optional<std::uint64_t>& observedRelationshipGeneration() const noexcept {
        return observedRelationshipGeneration_;
    }
    [[nodiscard]] bool satisfied() const noexcept { return satisfied_; }

private:
    ProductionRevalidationPrerequisiteRecord(
        RevalidationDecisionId decisionId,
        ProductionTransitionRequestBinding binding,
        bool relationshipPresent,
        std::optional<std::uint64_t> observedRelationshipGeneration,
        bool satisfied) noexcept
        : decisionId_(std::move(decisionId)),
          binding_(std::move(binding)),
          relationshipPresent_(relationshipPresent),
          observedRelationshipGeneration_(observedRelationshipGeneration),
          satisfied_(satisfied) {}

    RevalidationDecisionId decisionId_;
    ProductionTransitionRequestBinding binding_;
    bool relationshipPresent_;
    std::optional<std::uint64_t> observedRelationshipGeneration_;
    bool satisfied_;

    friend class ProductionTransitionLiveValidityEvaluator;
};

class ProductionTransitionLiveValidityEvidence final {
public:
    ProductionTransitionLiveValidityEvidence(const ProductionTransitionLiveValidityEvidence&) = default;
    ProductionTransitionLiveValidityEvidence& operator=(const ProductionTransitionLiveValidityEvidence&) = default;

    [[nodiscard]] const LiveValidityDecisionId& decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const ProductionDerivedTransitionRequest& request() const noexcept { return request_; }
    [[nodiscard]] const ProductionFreshnessPrerequisiteRecord& freshness() const noexcept { return freshness_; }
    [[nodiscard]] const ProductionRevalidationPrerequisiteRecord& revalidation() const noexcept { return revalidation_; }

private:
    ProductionTransitionLiveValidityEvidence(
        LiveValidityDecisionId decisionId,
        ProductionDerivedTransitionRequest request,
        ProductionFreshnessPrerequisiteRecord freshness,
        ProductionRevalidationPrerequisiteRecord revalidation)
        : decisionId_(std::move(decisionId)),
          request_(std::move(request)),
          freshness_(std::move(freshness)),
          revalidation_(std::move(revalidation)) {}

    LiveValidityDecisionId decisionId_;
    ProductionDerivedTransitionRequest request_;
    ProductionFreshnessPrerequisiteRecord freshness_;
    ProductionRevalidationPrerequisiteRecord revalidation_;

    friend class ProductionTransitionLiveValidityEvaluator;
};

enum class TransitionLiveValidityReason : std::uint8_t {
    RequestLineageInconsistent,
    InvalidRelationshipIdentity,
    SnapshotUnavailable,
    InternalDecisionFailure
};

class ProductionTransitionLiveValidityRejection final {
public:
    ProductionTransitionLiveValidityRejection(const ProductionTransitionLiveValidityRejection&) = default;
    ProductionTransitionLiveValidityRejection& operator=(const ProductionTransitionLiveValidityRejection&) = default;

    [[nodiscard]] const LiveValidityDecisionId& decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const TransitionRequestDecisionId& requestDecisionId() const noexcept { return requestDecisionId_; }
    [[nodiscard]] TransitionLiveValidityReason primaryReason() const noexcept { return primaryReason_; }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept { return reasonFlags_; }

private:
    ProductionTransitionLiveValidityRejection(
        LiveValidityDecisionId decisionId,
        TransitionRequestDecisionId requestDecisionId,
        TransitionLiveValidityReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          requestDecisionId_(std::move(requestDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}

    LiveValidityDecisionId decisionId_;
    TransitionRequestDecisionId requestDecisionId_;
    TransitionLiveValidityReason primaryReason_;
    std::uint64_t reasonFlags_;

    friend class ProductionTransitionLiveValidityEvaluator;
};

using ProductionTransitionLiveValidityResult =
    std::variant<
        ProductionTransitionLiveValidityEvidence,
        ProductionTransitionLiveValidityRejection>;

class ProductionTransitionLiveValidityEvaluator final {
public:
    explicit ProductionTransitionLiveValidityEvaluator(
        ProductionTransitionLiveSnapshotSource source) noexcept
        : source_(std::move(source)) {}

    [[nodiscard]]
    std::optional<ProductionTransitionLiveValidityResult>
    evaluate(const ProductionDerivedTransitionRequest& request) const;

private:
    ProductionTransitionLiveSnapshotSource source_;
};

} // namespace AdaptiveMesh
