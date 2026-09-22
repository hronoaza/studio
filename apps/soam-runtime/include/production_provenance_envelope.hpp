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
