#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"

#include <bit>
#include <utility>

namespace AdaptiveMesh {
namespace {

[[nodiscard]] bool sameBits(double lhs, double rhs) noexcept {
    return std::bit_cast<std::uint64_t>(lhs) ==
        std::bit_cast<std::uint64_t>(rhs);
}

[[nodiscard]] bool recordMatchesSnapshot(
    const RetainedSourceEvidenceRecord& record,
    const ProductionRelationshipSourceSnapshot& snapshot) noexcept
{
    return
        record.sourceCaptureId() == snapshot.sourceCaptureId() &&
        record.sourceNodeId() == snapshot.sourceNodeId() &&
        record.targetNodeId() == snapshot.targetNodeId() &&
        record.relationshipGeneration() == snapshot.relationshipGeneration() &&
        record.stateVersion() == snapshot.stateVersion() &&
        sameBits(record.distance(), snapshot.distance()) &&
        sameBits(record.orientationWeight(), snapshot.orientationWeight()) &&
        sameBits(record.capacity(), snapshot.capacity()) &&
        record.bridgeStatus() == snapshot.bridgeStatus() &&
        sameBits(record.sourceState(), snapshot.sourceState()) &&
        sameBits(record.targetState(), snapshot.targetState()) &&
        sameBits(record.sourceHealth(), snapshot.sourceHealth()) &&
        sameBits(record.targetHealth(), snapshot.targetHealth());
}

} // namespace

RetainedSourceEvidenceRecord::RetainedSourceEvidenceRecord(
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
    double targetHealth) noexcept
    : sourceCaptureId_(std::move(sourceCaptureId)),
      sourceNodeId_(sourceNodeId),
      targetNodeId_(targetNodeId),
      relationshipGeneration_(relationshipGeneration),
      stateVersion_(stateVersion),
      distance_(distance),
      orientationWeight_(orientationWeight),
      capacity_(capacity),
      bridgeStatus_(bridgeStatus),
      sourceState_(sourceState),
      targetState_(targetState),
      sourceHealth_(sourceHealth),
      targetHealth_(targetHealth)
{
}

SourceEvidenceRetentionResult RetainedSourceEvidenceStore::retain(
    const ProductionRelationshipSourceSnapshot& snapshot)
{
    RetainedSourceEvidenceRecord candidate{
        snapshot.sourceCaptureId(),
        snapshot.sourceNodeId(),
        snapshot.targetNodeId(),
        snapshot.relationshipGeneration(),
        snapshot.stateVersion(),
        snapshot.distance(),
        snapshot.orientationWeight(),
        snapshot.capacity(),
        snapshot.bridgeStatus(),
        snapshot.sourceState(),
        snapshot.targetState(),
        snapshot.sourceHealth(),
        snapshot.targetHealth()
    };

    std::lock_guard lock(mutex_);
    const auto key = snapshot.sourceCaptureId().bytes();
    const auto existing = records_.find(key);
    if (existing != records_.end()) {
        return recordMatchesSnapshot(existing->second, snapshot)
            ? SourceEvidenceRetentionResult::ALREADY_RETAINED
            : SourceEvidenceRetentionResult::INTEGRITY_CONFLICT;
    }

    records_.emplace(key, std::move(candidate));
    return SourceEvidenceRetentionResult::INSERTED;
}

SourceResolutionResult RetainedSourceEvidenceStore::resolve(
    const SourceCaptureId& sourceCaptureId) const
{
    std::lock_guard lock(mutex_);
    const auto existing = records_.find(sourceCaptureId.bytes());
    if (existing == records_.end()) {
        return SourceResolutionResult{
            SourceResolutionStatus::NOT_FOUND,
            std::nullopt
        };
    }

    return SourceResolutionResult{
        SourceResolutionStatus::FOUND,
        existing->second
    };
}

std::size_t RetainedSourceEvidenceStore::size() const noexcept
{
    std::lock_guard lock(mutex_);
    return records_.size();
}

} // namespace AdaptiveMesh
