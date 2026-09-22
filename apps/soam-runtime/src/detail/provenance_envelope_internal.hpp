#pragma once

#include "production_provenance_envelope.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace AdaptiveMesh::detail {

using Id128 = std::array<std::uint8_t, 16>;
using Digest256 = std::array<std::uint8_t, 32>;

struct CanonicalDependencyInput final {
    std::uint8_t kind;
    Id128 id;
    std::uint16_t major;
    std::uint16_t minor;
    std::uint8_t revisionKind;
    Digest256 revisionDigest;
};

struct CanonicalEnvelopeInput final {
    Id128 schemaId;
    std::uint16_t schemaMajor;
    std::uint16_t schemaMinor;
    std::uint16_t canonicalEncodingVersion;

    Id128 producerId;
    std::uint16_t producerMajor;
    std::uint16_t producerMinor;
    std::uint8_t implementationRevisionKind;
    Digest256 implementationRevision;

    Id128 provenanceItemId;
    Id128 sourceCaptureId;

    std::uint64_t sourceNodeId;
    std::uint64_t targetNodeId;
    std::uint64_t relationshipGeneration;
    std::uint64_t stateVersion;

    double distance;
    double orientationWeight;
    double capacity;
    std::uint8_t bridgeStatusTag;
    double sourceState;
    double targetState;
    double sourceHealth;
    double targetHealth;

    std::vector<CanonicalDependencyInput> dependencies;
};

using ProvenanceItemIdFillFunction =
    bool (*)(ProvenanceItemId::Bytes&) noexcept;

[[nodiscard]] std::optional<ProvenanceItemId::Bytes>
tryGenerateProvenanceItemIdBytes() noexcept;

[[nodiscard]] std::optional<std::vector<std::uint8_t>>
encodeCanonicalEnvelopeV1(const CanonicalEnvelopeInput& input);

[[nodiscard]] Digest256 sha256D8BDomainSeparated(
    const std::vector<std::uint8_t>& canonicalBytes) noexcept;

void setProvenanceItemIdFillFunctionForTesting(
    ProvenanceItemIdFillFunction fillFunction) noexcept;
void setD8BCanonicalEncodingFailureForTesting(bool enabled) noexcept;
void setD8BDigestFailureForTesting(bool enabled) noexcept;
void setD8BManifestFailureForTesting(bool enabled) noexcept;
void resetD8BTestSeams() noexcept;

[[nodiscard]] bool d8bDigestFailureForTesting() noexcept;
[[nodiscard]] bool d8bManifestFailureForTesting() noexcept;

} // namespace AdaptiveMesh::detail
