#include "source_evidence.hpp"

#include <bit>
#include <cstdint>
#include <utility>

namespace AdaptiveMesh {
namespace {

[[nodiscard]] bool sameDoubleBits(double lhs, double rhs) noexcept {
    return std::bit_cast<std::uint64_t>(lhs) ==
           std::bit_cast<std::uint64_t>(rhs);
}

[[nodiscard]] bool sameRecord(
    const RetainedSourceEvidenceRecord& lhs,
    const RetainedSourceEvidenceRecord& rhs) noexcept
{
    return lhs.sourceCaptureId() == rhs.sourceCaptureId() &&
           lhs.sourceNodeId() == rhs.sourceNodeId() &&
           lhs.targetNodeId() == rhs.targetNodeId() &&
           lhs.relationshipGeneration() == rhs.relationshipGeneration() &&
           lhs.stateVersion() == rhs.stateVersion() &&
           sameDoubleBits(lhs.distance(), rhs.distance()) &&
           sameDoubleBits(lhs.orientationWeight(), rhs.orientationWeight()) &&
           sameDoubleBits(lhs.capacity(), rhs.capacity()) &&
           lhs.bridgeStatus() == rhs.bridgeStatus() &&
           sameDoubleBits(lhs.sourceState(), rhs.sourceState()) &&
           sameDoubleBits(lhs.targetState(), rhs.targetState()) &&
           sameDoubleBits(lhs.sourceHealth(), rhs.sourceHealth()) &&
           sameDoubleBits(lhs.targetHealth(), rhs.targetHealth());
}

} // namespace

RetainedSourceEvidenceRecord::RetainedSourceEvidenceRecord(
    const ProductionRelationshipSourceSnapshot& snapshot) noexcept
    : sourceCaptureId_(snapshot.sourceCaptureId()),
      sourceNodeId_(snapshot.sourceNodeId()),
      targetNodeId_(snapshot.targetNodeId()),
      relationshipGeneration_(snapshot.relationshipGeneration()),
      stateVersion_(snapshot.stateVersion()),
      distance_(snapshot.distance()),
      orientationWeight_(snapshot.orientationWeight()),
      capacity_(snapshot.capacity()),
      bridgeStatus_(snapshot.bridgeStatus()),
      sourceState_(snapshot.sourceState()),
      targetState_(snapshot.targetState()),
      sourceHealth_(snapshot.sourceHealth()),
      targetHealth_(snapshot.targetHealth())
{
}

SourceEvidenceRetentionResult SourceEvidenceStore::retain(
    const ProductionRelationshipSourceSnapshot& snapshot)
{
    RetainedSourceEvidenceRecord candidate{snapshot};
    const auto key = snapshot.sourceCaptureId().bytes();

    std::lock_guard lock{mutex_};
    const auto existing = records_.find(key);
    if (existing == records_.end()) {
        records_.emplace(key, std::move(candidate));
        return SourceEvidenceRetentionResult::Inserted;
    }

    return sameRecord(existing->second, candidate)
        ? SourceEvidenceRetentionResult::AlreadyPresent
        : SourceEvidenceRetentionResult::IntegrityConflict;
}

SourceResolutionResult SourceEvidenceStore::resolve(
    const SourceCaptureId& sourceCaptureId) const
{
    std::lock_guard lock{mutex_};
    const auto found = records_.find(sourceCaptureId.bytes());
    if (found == records_.end()) {
        return {SourceResolutionStatus::NotFound, std::nullopt};
    }
    return {SourceResolutionStatus::Found, found->second};
}

} // namespace AdaptiveMesh
