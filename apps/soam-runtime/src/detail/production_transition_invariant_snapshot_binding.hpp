#pragma once

#include <memory>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

namespace detail {

class ProductionTransitionInvariantSnapshotBindingState;

[[nodiscard]]
std::shared_ptr<ProductionTransitionInvariantSnapshotBindingState>
makeProductionTransitionInvariantSnapshotBinding(
    SpatialAdaptiveMesh* owner);

void invalidateProductionTransitionInvariantSnapshotBinding(
    const std::shared_ptr<
        ProductionTransitionInvariantSnapshotBindingState>& state) noexcept;

} // namespace detail
} // namespace AdaptiveMesh
