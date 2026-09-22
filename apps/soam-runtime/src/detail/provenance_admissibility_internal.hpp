#pragma once

#include "provenance_admissibility.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace AdaptiveMesh::detail {

using D8COpaqueIdFillFunction =
    bool (*)(std::array<std::uint8_t, 16>&) noexcept;

[[nodiscard]] std::optional<std::array<std::uint8_t, 16>>
tryGenerateD8COpaqueIdBytes() noexcept;

void setD8COpaqueIdFillFunctionForTesting(
    D8COpaqueIdFillFunction fillFunction) noexcept;

void resetD8COpaqueIdGeneratorForTesting() noexcept;

struct ProvenanceAdmissibilityTestAccess final {
    [[nodiscard]] static ProvenanceAdmissibilityPolicySnapshot makePolicy(
        PolicySnapshotId::Bytes snapshotId,
        AdmissibilityPolicyDescriptor descriptor,
        std::vector<ProducerPolicyEntry> producers,
        std::vector<SchemaPolicyEntry> schemas,
        std::vector<DependencyPolicyEntry> dependencies,
        std::vector<std::uint8_t> requiredDependencyKinds);
};

} // namespace AdaptiveMesh::detail
