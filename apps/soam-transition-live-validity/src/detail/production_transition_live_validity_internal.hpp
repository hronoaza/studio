#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace AdaptiveMesh::detail {

using LiveValidityOpaqueIdFillFunction =
    bool (*)(std::array<std::uint8_t,16>&) noexcept;

[[nodiscard]]
std::optional<std::array<std::uint8_t,16>>
tryGenerateLiveValidityOpaqueIdBytes() noexcept;

void setLiveValidityOpaqueIdFillFunctionForTesting(
    LiveValidityOpaqueIdFillFunction fillFunction) noexcept;

void resetLiveValidityOpaqueIdGeneratorForTesting() noexcept;

} // namespace AdaptiveMesh::detail
