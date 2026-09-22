#pragma once

#include "versioned_interpretation.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace AdaptiveMesh::detail {

using D8DOpaqueIdFillFunction =
    bool (*)(std::array<std::uint8_t, 16>&) noexcept;

[[nodiscard]] std::optional<std::array<std::uint8_t, 16>>
tryGenerateD8DOpaqueIdBytes() noexcept;

void setD8DOpaqueIdFillFunctionForTesting(
    D8DOpaqueIdFillFunction fillFunction) noexcept;

void resetD8DOpaqueIdGeneratorForTesting() noexcept;

struct InterpretationV1Computation final {
    double attenuation;
    double compatibility;
    double confidence;
};

[[nodiscard]] std::optional<InterpretationV1Computation>
computeInterpretationV1(
    double distance,
    double orientationWeight,
    double capacity,
    double sourceHealth,
    double targetHealth,
    double distanceAttenuationCoefficient) noexcept;

struct VersionedInterpretationTestAccess final {
    [[nodiscard]] static InterpretationPolicySnapshot makePolicy(
        InterpretationPolicySnapshotId::Bytes snapshotId,
        InterpretationPolicyId::Bytes policyId,
        std::uint16_t majorVersion,
        std::uint16_t minorVersion,
        std::uint8_t revisionKind,
        std::array<std::uint8_t,32> revision,
        double distanceAttenuationCoefficient,
        bool directional);
};

} // namespace AdaptiveMesh::detail
