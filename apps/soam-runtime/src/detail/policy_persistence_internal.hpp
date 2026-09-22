#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace AdaptiveMesh::detail {

using D9OpaqueIdFillFunction =
    bool (*)(std::array<std::uint8_t, 16>&) noexcept;

[[nodiscard]] std::optional<std::array<std::uint8_t, 16>>
tryGenerateD9OpaqueIdBytes() noexcept;

void setD9OpaqueIdFillFunctionForTesting(
    D9OpaqueIdFillFunction fillFunction) noexcept;

void resetD9OpaqueIdGeneratorForTesting() noexcept;

} // namespace AdaptiveMesh::detail
