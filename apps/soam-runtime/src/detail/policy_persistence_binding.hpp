#pragma once

#include <memory>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

namespace detail {

class ProductionPersistenceRegistryState;

[[nodiscard]]
std::shared_ptr<ProductionPersistenceRegistryState>
makeProductionPersistenceRegistryState(SpatialAdaptiveMesh* owner);

void invalidateProductionPersistenceRegistryState(
    const std::shared_ptr<ProductionPersistenceRegistryState>& state) noexcept;

} // namespace detail
} // namespace AdaptiveMesh
