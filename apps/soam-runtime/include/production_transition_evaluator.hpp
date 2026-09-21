#pragma once

#include <cstddef>
#include <memory>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

namespace detail {
class ProductionTransitionEvaluationBindingState;
}

struct ProductionTransitionEvaluationLocator final {
    std::size_t sourceNodeId;
    std::size_t targetNodeId;
};

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
        const ProductionTransitionEvaluationLocator& locator) const;

private:
    explicit ProductionTransitionEvaluator(
        std::shared_ptr<detail::ProductionTransitionEvaluationBindingState> state) noexcept
        : state_(std::move(state)) {}

    std::shared_ptr<detail::ProductionTransitionEvaluationBindingState> state_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
