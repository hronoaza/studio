#pragma once

#include "source_capture_id.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace AdaptiveMesh {

enum class BridgeStatus;
class SpatialAdaptiveMesh;

class ProductionRelationshipSourceSnapshot final {
public:
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
    [[nodiscard]] BridgeStatus bridgeStatus() const noexcept {
        return bridgeStatus_;
    }

    [[nodiscard]] double sourceState() const noexcept { return sourceState_; }
    [[nodiscard]] double targetState() const noexcept { return targetState_; }
    [[nodiscard]] double sourceHealth() const noexcept { return sourceHealth_; }
    [[nodiscard]] double targetHealth() const noexcept { return targetHealth_; }

private:
    ProductionRelationshipSourceSnapshot(
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
          targetHealth_(targetHealth) {}

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

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
