#pragma once

#include "production_transition_eligibility.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace AdaptiveMesh::detail {

class ProductionTransitionInvariantPolicyAccess final {
public:
    [[nodiscard]] static constexpr ProductionTransitionClassId
    bridgeCouplingAdjustmentV1() noexcept
    {
        return ProductionTransitionClassId{0x4252494447455631ULL};
    }
};

using InvariantOpaqueIdFillFunction =
    bool (*)(std::array<std::uint8_t,16>&) noexcept;

[[nodiscard]]
std::optional<std::array<std::uint8_t,16>>
tryGenerateInvariantOpaqueIdBytes() noexcept;

void setInvariantOpaqueIdFillFunctionForTesting(
    InvariantOpaqueIdFillFunction fillFunction) noexcept;

void resetInvariantOpaqueIdGeneratorForTesting() noexcept;

[[nodiscard]] double projectInvariantCapacityForTesting(
    double current,
    RequestedTransitionDirection direction) noexcept;

} // namespace AdaptiveMesh::detail
