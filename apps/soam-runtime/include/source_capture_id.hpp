#pragma once

#include <array>
#include <cstdint>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

class SourceCaptureId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    SourceCaptureId(const SourceCaptureId&) noexcept = default;
    SourceCaptureId& operator=(const SourceCaptureId&) noexcept = default;
    SourceCaptureId(SourceCaptureId&&) noexcept = default;
    SourceCaptureId& operator=(SourceCaptureId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const SourceCaptureId&,
        const SourceCaptureId&) noexcept = default;

private:
    explicit SourceCaptureId(Bytes bytes) noexcept
        : bytes_(bytes) {}

    Bytes bytes_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
