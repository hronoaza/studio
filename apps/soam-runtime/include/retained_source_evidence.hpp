#pragma once

#include "production_relationship_source_snapshot.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>

namespace AdaptiveMesh {

class RetainedSourceEvidenceRecord final {
public:
    RetainedSourceEvidenceRecord(const RetainedSourceEvidenceRecord&) = default;
    RetainedSourceEvidenceRecord& operator=(const RetainedSourceEvidenceRecord&) = default;
    RetainedSourceEvidenceRecord(RetainedSourceEvidenceRecord&&) noexcept = default;
    RetainedSourceEvidenceRecord& operator=(RetainedSourceEvidenceRecord&&) noexcept = default;

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
    [[nodiscard]] double orientationWeight() const noexcept {
        return orientationWeight_;
    }
    [[nodiscard]] double capacity() const noexcept { return capacity_; }
    [[nodiscard]] BridgeStatus bridgeStatus() const noexcept { return bridgeStatus_; }
    [[nodiscard]] double sourceState() const noexcept { return sourceState_; }
    [[nodiscard]] double targetState() const noexcept { return targetState_; }
    [[nodiscard]] double sourceHealth() const noexcept { return sourceHealth_; }
    [[nodiscard]] double targetHealth() const noexcept { return targetHealth_; }

private:
    RetainedSourceEvidenceRecord(
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
        double targetHealth) noexcept;

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

    friend class RetainedSourceEvidenceStore;
};

enum class SourceEvidenceRetentionResult : std::uint8_t {
    INSERTED,
    ALREADY_RETAINED,
    INTEGRITY_CONFLICT
};

enum class SourceResolutionStatus : std::uint8_t {
    FOUND,
    NOT_FOUND,
    RESOLVER_FAILURE,
    INTEGRITY_CONFLICT
};

struct SourceResolutionResult final {
    SourceResolutionStatus status;
    std::optional<RetainedSourceEvidenceRecord> record;
};

class SourceEvidenceResolver {
public:
    virtual ~SourceEvidenceResolver() = default;

    [[nodiscard]] virtual SourceResolutionResult resolve(
        const SourceCaptureId& sourceCaptureId) const = 0;
};

class RetainedSourceEvidenceStore final : public SourceEvidenceResolver {
public:
    [[nodiscard]] SourceEvidenceRetentionResult retain(
        const ProductionRelationshipSourceSnapshot& snapshot);

    [[nodiscard]] SourceResolutionResult resolve(
        const SourceCaptureId& sourceCaptureId) const override;

    [[nodiscard]] std::size_t size() const noexcept;

private:
    using Key = SourceCaptureId::Bytes;

    mutable std::mutex mutex_;
    std::map<Key, RetainedSourceEvidenceRecord> records_;
};

} // namespace AdaptiveMesh
