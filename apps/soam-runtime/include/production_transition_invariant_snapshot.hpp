#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;
enum class BridgeStatus;

namespace detail {
class ProductionTransitionInvariantSnapshotBindingState;
}

class ProductionTransitionInvariantBridgeSnapshot final {
public:
    ProductionTransitionInvariantBridgeSnapshot(
        const ProductionTransitionInvariantBridgeSnapshot&) noexcept = default;
    ProductionTransitionInvariantBridgeSnapshot& operator=(
        const ProductionTransitionInvariantBridgeSnapshot&) noexcept = default;

    [[nodiscard]] std::size_t targetNodeId() const noexcept {
        return targetNodeId_;
    }
    [[nodiscard]] std::uint64_t generation() const noexcept {
        return generation_;
    }
    [[nodiscard]] double capacity() const noexcept { return capacity_; }
    [[nodiscard]] double distance() const noexcept { return distance_; }
    [[nodiscard]] double orientationWeight() const noexcept {
        return orientationWeight_;
    }
    [[nodiscard]] BridgeStatus status() const noexcept { return status_; }

private:
    ProductionTransitionInvariantBridgeSnapshot(
        std::size_t targetNodeId,
        std::uint64_t generation,
        double capacity,
        double distance,
        double orientationWeight,
        BridgeStatus status) noexcept
        : targetNodeId_(targetNodeId),
          generation_(generation),
          capacity_(capacity),
          distance_(distance),
          orientationWeight_(orientationWeight),
          status_(status) {}

    std::size_t targetNodeId_;
    std::uint64_t generation_;
    double capacity_;
    double distance_;
    double orientationWeight_;
    BridgeStatus status_;

    friend class SpatialAdaptiveMesh;
};

class ProductionTransitionInvariantSnapshot final {
public:
    ProductionTransitionInvariantSnapshot(
        const ProductionTransitionInvariantSnapshot&) noexcept = default;
    ProductionTransitionInvariantSnapshot& operator=(
        const ProductionTransitionInvariantSnapshot&) noexcept = default;

    [[nodiscard]] std::size_t sourceNodeId() const noexcept {
        return sourceNodeId_;
    }
    [[nodiscard]] std::size_t targetNodeId() const noexcept {
        return targetNodeId_;
    }
    [[nodiscard]] std::uint64_t stateVersion() const noexcept {
        return stateVersion_;
    }

    [[nodiscard]] const std::optional<
        ProductionTransitionInvariantBridgeSnapshot>&
    forwardBridge() const noexcept { return forwardBridge_; }

    [[nodiscard]] const std::optional<
        ProductionTransitionInvariantBridgeSnapshot>&
    reverseBridge() const noexcept { return reverseBridge_; }

    [[nodiscard]] double sourceState() const noexcept { return sourceState_; }
    [[nodiscard]] double targetState() const noexcept { return targetState_; }

    [[nodiscard]] double sourceInvariantBaseline() const noexcept {
        return sourceInvariantBaseline_;
    }
    [[nodiscard]] double sourceInvariantMaxEpsilon() const noexcept {
        return sourceInvariantMaxEpsilon_;
    }
    [[nodiscard]] double targetInvariantBaseline() const noexcept {
        return targetInvariantBaseline_;
    }
    [[nodiscard]] double targetInvariantMaxEpsilon() const noexcept {
        return targetInvariantMaxEpsilon_;
    }

private:
    ProductionTransitionInvariantSnapshot(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t stateVersion,
        std::optional<ProductionTransitionInvariantBridgeSnapshot> forwardBridge,
        std::optional<ProductionTransitionInvariantBridgeSnapshot> reverseBridge,
        double sourceState,
        double targetState,
        double sourceInvariantBaseline,
        double sourceInvariantMaxEpsilon,
        double targetInvariantBaseline,
        double targetInvariantMaxEpsilon) noexcept
        : sourceNodeId_(sourceNodeId),
          targetNodeId_(targetNodeId),
          stateVersion_(stateVersion),
          forwardBridge_(std::move(forwardBridge)),
          reverseBridge_(std::move(reverseBridge)),
          sourceState_(sourceState),
          targetState_(targetState),
          sourceInvariantBaseline_(sourceInvariantBaseline),
          sourceInvariantMaxEpsilon_(sourceInvariantMaxEpsilon),
          targetInvariantBaseline_(targetInvariantBaseline),
          targetInvariantMaxEpsilon_(targetInvariantMaxEpsilon) {}

    std::size_t sourceNodeId_;
    std::size_t targetNodeId_;
    std::uint64_t stateVersion_;
    std::optional<ProductionTransitionInvariantBridgeSnapshot> forwardBridge_;
    std::optional<ProductionTransitionInvariantBridgeSnapshot> reverseBridge_;
    double sourceState_;
    double targetState_;
    double sourceInvariantBaseline_;
    double sourceInvariantMaxEpsilon_;
    double targetInvariantBaseline_;
    double targetInvariantMaxEpsilon_;

    friend class SpatialAdaptiveMesh;
};

class ProductionTransitionInvariantSnapshotSource final {
public:
    ProductionTransitionInvariantSnapshotSource(
        const ProductionTransitionInvariantSnapshotSource&) noexcept = default;
    ProductionTransitionInvariantSnapshotSource& operator=(
        const ProductionTransitionInvariantSnapshotSource&) noexcept = default;

    [[nodiscard]]
    std::optional<ProductionTransitionInvariantSnapshot>
    capture(std::size_t sourceNodeId, std::size_t targetNodeId) const;

private:
    explicit ProductionTransitionInvariantSnapshotSource(
        std::shared_ptr<
            detail::ProductionTransitionInvariantSnapshotBindingState>
            state) noexcept
        : state_(std::move(state)) {}

    std::shared_ptr<detail::ProductionTransitionInvariantSnapshotBindingState>
        state_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
