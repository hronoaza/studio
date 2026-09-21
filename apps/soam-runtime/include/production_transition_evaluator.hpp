#pragma once

#include "production_relationship_locator.hpp"

#include <cstddef>
#include <memory>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

namespace detail {
class ProductionTransitionEvaluationBindingState;
}

enum class ProductionTransitionEvaluation {
    no_request,
    not_eligible,
    eligible_for_authority_consideration
};

class ProductionTransitionEvaluator final {
public:
    ProductionTransitionEvaluator(const ProductionTransitionEvaluator&) noexcept = default;
    ProductionTransitionEvaluator& operator=(const ProductionTransitionEvaluator&) noexcept = default;

    [[nodiscard]] ProductionTransitionEvaluation evaluate(
        const ProductionRelationshipLocator& locator) const;

private:
    explicit ProductionTransitionEvaluator(
        std::shared_ptr<detail::ProductionTransitionEvaluationBindingState> state) noexcept
        : state_(std::move(state)) {}

    std::shared_ptr<detail::ProductionTransitionEvaluationBindingState> state_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
