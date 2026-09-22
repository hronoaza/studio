#pragma once

#include "production_relationship_source_snapshot.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace AdaptiveMesh {

enum class BridgeStatus;

class ProvenanceItemId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    ProvenanceItemId(const ProvenanceItemId&) noexcept = default;
    ProvenanceItemId& operator=(const ProvenanceItemId&) noexcept = default;
    ProvenanceItemId(ProvenanceItemId&&) noexcept = default;
    ProvenanceItemId& operator=(ProvenanceItemId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const ProvenanceItemId&,
        const ProvenanceItemId&) noexcept = default;

private:
    explicit ProvenanceItemId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionProvenanceEnvelopeProducer;
};

struct ProvenanceDependencyDescriptor final {
    std::uint8_t kind;
    std::array<std::uint8_t, 16> dependencyId;
    std::uint16_t versionMajor;
    std::uint16_t versionMinor;
    std::uint8_t revisionKind;
    std::array<std::uint8_t, 32> revisionDigest;

    friend bool operator==(
        const ProvenanceDependencyDescriptor&,
        const ProvenanceDependencyDescriptor&) noexcept = default;
};

class ProvenanceMetadataView final {
public:
    using Id128 = std::array<std::uint8_t, 16>;
    using Digest256 = std::array<std::uint8_t, 32>;

    [[nodiscard]] const Id128& schemaId() const noexcept { return schemaId_; }
    [[nodiscard]] std::uint16_t schemaMajor() const noexcept { return schemaMajor_; }
    [[nodiscard]] std::uint16_t schemaMinor() const noexcept { return schemaMinor_; }
    [[nodiscard]] std::uint16_t canonicalEncodingVersion() const noexcept {
        return canonicalEncodingVersion_;
    }

    [[nodiscard]] const Id128& producerId() const noexcept { return producerId_; }
    [[nodiscard]] std::uint16_t producerMajor() const noexcept { return producerMajor_; }
    [[nodiscard]] std::uint16_t producerMinor() const noexcept { return producerMinor_; }
    [[nodiscard]] std::uint8_t implementationRevisionKind() const noexcept {
        return implementationRevisionKind_;
    }
    [[nodiscard]] const Digest256& implementationRevision() const noexcept {
        return implementationRevision_;
    }

    [[nodiscard]] const std::vector<ProvenanceDependencyDescriptor>&
    dependencies() const noexcept {
        return dependencies_;
    }

private:
    ProvenanceMetadataView(
        Id128 schemaId,
        std::uint16_t schemaMajor,
        std::uint16_t schemaMinor,
        std::uint16_t canonicalEncodingVersion,
        Id128 producerId,
        std::uint16_t producerMajor,
        std::uint16_t producerMinor,
        std::uint8_t implementationRevisionKind,
        Digest256 implementationRevision,
        std::vector<ProvenanceDependencyDescriptor> dependencies) noexcept
        : schemaId_(schemaId),
          schemaMajor_(schemaMajor),
          schemaMinor_(schemaMinor),
          canonicalEncodingVersion_(canonicalEncodingVersion),
          producerId_(producerId),
          producerMajor_(producerMajor),
          producerMinor_(producerMinor),
          implementationRevisionKind_(implementationRevisionKind),
          implementationRevision_(implementationRevision),
          dependencies_(std::move(dependencies)) {}

    Id128 schemaId_;
    std::uint16_t schemaMajor_;
    std::uint16_t schemaMinor_;
    std::uint16_t canonicalEncodingVersion_;
    Id128 producerId_;
    std::uint16_t producerMajor_;
    std::uint16_t producerMinor_;
    std::uint8_t implementationRevisionKind_;
    Digest256 implementationRevision_;
    std::vector<ProvenanceDependencyDescriptor> dependencies_;

    friend class ProductionProvenanceEnvelopeProducer;
};

class CanonicalDigest final {
public:
    using Bytes = std::array<std::uint8_t, 32>;

    CanonicalDigest(const CanonicalDigest&) noexcept = default;
    CanonicalDigest& operator=(const CanonicalDigest&) noexcept = default;
    CanonicalDigest(CanonicalDigest&&) noexcept = default;
    CanonicalDigest& operator=(CanonicalDigest&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }

    friend bool operator==(
        const CanonicalDigest&,
        const CanonicalDigest&) noexcept = default;

private:
    explicit CanonicalDigest(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;

    friend class ProductionProvenanceEnvelopeProducer;
};

class ProductionProvenanceEnvelope final {
public:
    ProductionProvenanceEnvelope(const ProductionProvenanceEnvelope&) = default;
    ProductionProvenanceEnvelope& operator=(const ProductionProvenanceEnvelope&) = default;
    ProductionProvenanceEnvelope(ProductionProvenanceEnvelope&&) noexcept = default;
    ProductionProvenanceEnvelope& operator=(ProductionProvenanceEnvelope&&) noexcept = default;

    [[nodiscard]] const ProvenanceItemId& provenanceItemId() const noexcept {
        return provenanceItemId_;
    }
    [[nodiscard]] const SourceCaptureId& sourceCaptureId() const noexcept {
        return sourceCaptureId_;
    }
    [[nodiscard]] std::size_t sourceNodeId() const noexcept { return sourceNodeId_; }
    [[nodiscard]] std::size_t targetNodeId() const noexcept { return targetNodeId_; }
    [[nodiscard]] std::uint64_t relationshipGeneration() const noexcept {
        return relationshipGeneration_;
    }
    [[nodiscard]] std::uint64_t stateVersion() const noexcept {
        return stateVersion_;
    }
    [[nodiscard]] double distance() const noexcept { return distance_; }
    [[nodiscard]] double orientationWeight() const noexcept { return orientationWeight_; }
    [[nodiscard]] double capacity() const noexcept { return capacity_; }
    [[nodiscard]] BridgeStatus bridgeStatus() const noexcept { return bridgeStatus_; }
    [[nodiscard]] double sourceState() const noexcept { return sourceState_; }
    [[nodiscard]] double targetState() const noexcept { return targetState_; }
    [[nodiscard]] double sourceHealth() const noexcept { return sourceHealth_; }
    [[nodiscard]] double targetHealth() const noexcept { return targetHealth_; }
    [[nodiscard]] const ProvenanceMetadataView& metadata() const noexcept {
        return metadata_;
    }
    [[nodiscard]] const CanonicalDigest& canonicalDigest() const noexcept {
        return canonicalDigest_;
    }
    [[nodiscard]] const std::vector<std::uint8_t>& canonicalBytes() const noexcept {
        return canonicalBytes_;
    }

private:
    ProductionProvenanceEnvelope(
        ProvenanceItemId provenanceItemId,
        SourceCaptureId sourceCaptureId,
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t relationshipGeneration,
        std::uint64_t stateVersion,
        double distance,
        double orientationWeight,
        double capacity,
        BridgeStatus bridgeStatus,
        double sourceState,
        double targetState,
        double sourceHealth,
        double targetHealth,
        ProvenanceMetadataView metadata,
        CanonicalDigest canonicalDigest,
        std::vector<std::uint8_t> canonicalBytes) noexcept;

    ProvenanceItemId provenanceItemId_;
    SourceCaptureId sourceCaptureId_;
    std::size_t sourceNodeId_;
    std::size_t targetNodeId_;
    std::uint64_t relationshipGeneration_;
    std::uint64_t stateVersion_;
    double distance_;
    double orientationWeight_;
    double capacity_;
    BridgeStatus bridgeStatus_;
    double sourceState_;
    double targetState_;
    double sourceHealth_;
    double targetHealth_;
    ProvenanceMetadataView metadata_;
    CanonicalDigest canonicalDigest_;
    std::vector<std::uint8_t> canonicalBytes_;

    friend class ProductionProvenanceEnvelopeProducer;
};

class ProductionProvenanceEnvelopeProducer final {
public:
    [[nodiscard]] std::optional<ProductionProvenanceEnvelope> produce(
        const ProductionRelationshipSourceSnapshot& snapshot) const;
};

} // namespace AdaptiveMesh
