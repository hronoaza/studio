#pragma once

#include <memory>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

namespace detail {

class ProductionTransitionEvaluationBindingState;

[[nodiscard]]
std::shared_ptr<ProductionTransitionEvaluationBindingState>
makeProductionTransitionEvaluationBinding(
    SpatialAdaptiveMesh* owner);

void invalidateProductionTransitionEvaluationBinding(
    const std::shared_ptr<ProductionTransitionEvaluationBindingState>& state) noexcept;

} // namespace detail
} // namespace AdaptiveMesh
