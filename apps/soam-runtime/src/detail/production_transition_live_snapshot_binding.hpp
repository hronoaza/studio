#pragma once

#include <memory>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

namespace detail {

class ProductionTransitionLiveSnapshotBindingState;

[[nodiscard]]
std::shared_ptr<ProductionTransitionLiveSnapshotBindingState>
makeProductionTransitionLiveSnapshotBinding(
    SpatialAdaptiveMesh* owner);

void invalidateProductionTransitionLiveSnapshotBinding(
    const std::shared_ptr<
        ProductionTransitionLiveSnapshotBindingState>& state) noexcept;

} // namespace detail
} // namespace AdaptiveMesh
