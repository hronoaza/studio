#pragma once
#include <memory>

namespace AdaptiveMesh {
class SpatialAdaptiveMesh;

namespace detail {
class ProductionTransitionResilienceSnapshotBindingState;

[[nodiscard]]
std::shared_ptr<ProductionTransitionResilienceSnapshotBindingState>
makeProductionTransitionResilienceSnapshotBinding(SpatialAdaptiveMesh* owner);

void invalidateProductionTransitionResilienceSnapshotBinding(
    const std::shared_ptr<
        ProductionTransitionResilienceSnapshotBindingState>& state) noexcept;

} // namespace detail
} // namespace AdaptiveMesh
