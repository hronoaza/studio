#pragma once

#include "production_transition_live_validity.hpp"
#include "production_transition_eligibility.hpp"

#include <optional>
#include <utility>

namespace AdaptiveMesh {

class ProductionLiveValidityC1Prerequisites final {
public:
    ProductionLiveValidityC1Prerequisites(
        const ProductionLiveValidityC1Prerequisites&) = default;
    ProductionLiveValidityC1Prerequisites& operator=(
        const ProductionLiveValidityC1Prerequisites&) = default;
    ProductionLiveValidityC1Prerequisites(
        ProductionLiveValidityC1Prerequisites&&) noexcept = default;
    ProductionLiveValidityC1Prerequisites& operator=(
        ProductionLiveValidityC1Prerequisites&&) noexcept = default;

    [[nodiscard]]
    const LiveValidityDecisionId& liveValidityDecisionId() const noexcept {
        return liveValidityDecisionId_;
    }

    [[nodiscard]]
    const FreshnessDecisionId& freshnessDecisionId() const noexcept {
        return freshnessDecisionId_;
    }

    [[nodiscard]]
    const RevalidationDecisionId& revalidationDecisionId() const noexcept {
        return revalidationDecisionId_;
    }

    [[nodiscard]]
    const FreshnessPrerequisiteEvidence& freshness() const noexcept {
        return freshness_;
    }

    [[nodiscard]]
    const RevalidationPrerequisiteEvidence& revalidation() const noexcept {
        return revalidation_;
    }

private:
    ProductionLiveValidityC1Prerequisites(
        LiveValidityDecisionId liveValidityDecisionId,
        FreshnessDecisionId freshnessDecisionId,
        RevalidationDecisionId revalidationDecisionId,
        FreshnessPrerequisiteEvidence freshness,
        RevalidationPrerequisiteEvidence revalidation) noexcept
        : liveValidityDecisionId_(std::move(liveValidityDecisionId)),
          freshnessDecisionId_(std::move(freshnessDecisionId)),
          revalidationDecisionId_(std::move(revalidationDecisionId)),
          freshness_(std::move(freshness)),
          revalidation_(std::move(revalidation)) {}

    LiveValidityDecisionId liveValidityDecisionId_;
    FreshnessDecisionId freshnessDecisionId_;
    RevalidationDecisionId revalidationDecisionId_;
    FreshnessPrerequisiteEvidence freshness_;
    RevalidationPrerequisiteEvidence revalidation_;

    friend class ProductionLiveValidityC1Adapter;
};

class ProductionLiveValidityC1Adapter final {
public:
    [[nodiscard]]
    std::optional<ProductionLiveValidityC1Prerequisites>
    convert(const ProductionTransitionLiveValidityEvidence& evidence) const noexcept;
};

} // namespace AdaptiveMesh
