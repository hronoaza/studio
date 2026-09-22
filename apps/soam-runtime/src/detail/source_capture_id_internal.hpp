#pragma once

#include "source_capture_id.hpp"

#include <optional>

namespace AdaptiveMesh::detail {

using SourceCaptureIdFillFunction =
    bool (*)(SourceCaptureId::Bytes&) noexcept;

[[nodiscard]] std::optional<SourceCaptureId::Bytes>
tryGenerateSourceCaptureIdBytes() noexcept;

void setSourceCaptureIdFillFunctionForTesting(
    SourceCaptureIdFillFunction fillFunction) noexcept;

void resetSourceCaptureIdGeneratorForTesting() noexcept;

} // namespace AdaptiveMesh::detail
