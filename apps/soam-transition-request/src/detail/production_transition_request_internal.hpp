#pragma once

#include "production_transition_eligibility.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace AdaptiveMesh::detail {

class ProductionTransitionRequestDerivationAccess final {
public:
    [[nodiscard]] static ProductionRelationshipIdentity relationship(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t generation) noexcept
    {
        return ProductionRelationshipIdentity{
            sourceNodeId, targetNodeId, generation};
    }

    [[nodiscard]] static ProductionStateVersion stateVersion(
        std::uint64_t value) noexcept
    {
        return ProductionStateVersion{value};
    }

    [[nodiscard]] static ProductionTransitionClassId
    bridgeCouplingAdjustmentV1() noexcept
    {
        return ProductionTransitionClassId{0x4252494447455631ULL};
    }

    [[nodiscard]] static ProductionTransitionRequestBinding binding(
        ProductionRelationshipIdentity relationship,
        RequestedTransitionDirection direction,
        ProductionTransitionClassId transitionClass,
        ProductionStateVersion stateVersion) noexcept
    {
        return ProductionTransitionRequestBinding{
            relationship, direction, transitionClass, stateVersion};
    }
};

using TransitionRequestOpaqueIdFillFunction =
    bool (*)(std::array<std::uint8_t,16>&) noexcept;

[[nodiscard]] std::optional<std::array<std::uint8_t,16>>
tryGenerateTransitionRequestOpaqueIdBytes() noexcept;

void setTransitionRequestOpaqueIdFillFunctionForTesting(
    TransitionRequestOpaqueIdFillFunction fillFunction) noexcept;

void resetTransitionRequestOpaqueIdGeneratorForTesting() noexcept;

} // namespace AdaptiveMesh::detail
