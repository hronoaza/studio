#pragma once

#include <cstddef>

namespace AdaptiveMesh {

struct ProductionRelationshipLocator final {
    std::size_t sourceNodeId;
    std::size_t targetNodeId;

    friend bool operator==(
        const ProductionRelationshipLocator&,
        const ProductionRelationshipLocator&) noexcept = default;
};

} // namespace AdaptiveMesh
