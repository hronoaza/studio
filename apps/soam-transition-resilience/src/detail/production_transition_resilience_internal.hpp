#pragma once

#include "production_transition_eligibility.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace AdaptiveMesh::detail {

class ProductionTransitionResiliencePolicyAccess final {
public:
    [[nodiscard]] static constexpr ProductionTransitionClassId
    bridgeCouplingAdjustmentV1() noexcept
    {
        return ProductionTransitionClassId{0x4252494447455631ULL};
    }
};

using ResilienceOpaqueIdFillFunction =
    bool (*)(std::array<std::uint8_t,16>&) noexcept;

[[nodiscard]]
std::optional<std::array<std::uint8_t,16>>
tryGenerateResilienceOpaqueIdBytes() noexcept;

void setResilienceOpaqueIdFillFunctionForTesting(
    ResilienceOpaqueIdFillFunction fillFunction) noexcept;

void resetResilienceOpaqueIdGeneratorForTesting() noexcept;

} // namespace AdaptiveMesh::detail
