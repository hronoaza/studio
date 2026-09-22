#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

namespace detail {
class ProductionTransitionLiveSnapshotBindingState;
}

class ProductionTransitionLiveSnapshot final {
public:
    ProductionTransitionLiveSnapshot(
        const ProductionTransitionLiveSnapshot&) noexcept = default;
    ProductionTransitionLiveSnapshot& operator=(
        const ProductionTransitionLiveSnapshot&) noexcept = default;

    [[nodiscard]] std::size_t sourceNodeId() const noexcept {
        return sourceNodeId_;
    }
    [[nodiscard]] std::size_t targetNodeId() const noexcept {
        return targetNodeId_;
    }
    [[nodiscard]] std::uint64_t stateVersion() const noexcept {
        return stateVersion_;
    }
    [[nodiscard]] bool relationshipPresent() const noexcept {
        return relationshipPresent_;
    }
    [[nodiscard]] const std::optional<std::uint64_t>&
    relationshipGeneration() const noexcept {
        return relationshipGeneration_;
    }

private:
    ProductionTransitionLiveSnapshot(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t stateVersion,
        bool relationshipPresent,
        std::optional<std::uint64_t> relationshipGeneration) noexcept
        : sourceNodeId_(sourceNodeId),
          targetNodeId_(targetNodeId),
          stateVersion_(stateVersion),
          relationshipPresent_(relationshipPresent),
          relationshipGeneration_(relationshipGeneration) {}

    std::size_t sourceNodeId_;
    std::size_t targetNodeId_;
    std::uint64_t stateVersion_;
    bool relationshipPresent_;
    std::optional<std::uint64_t> relationshipGeneration_;

    friend class SpatialAdaptiveMesh;
};

class ProductionTransitionLiveSnapshotSource final {
public:
    ProductionTransitionLiveSnapshotSource(
        const ProductionTransitionLiveSnapshotSource&) noexcept = default;
    ProductionTransitionLiveSnapshotSource& operator=(
        const ProductionTransitionLiveSnapshotSource&) noexcept = default;

    [[nodiscard]] std::optional<ProductionTransitionLiveSnapshot>
    capture(
        std::size_t sourceNodeId,
        std::size_t targetNodeId) const;

private:
    explicit ProductionTransitionLiveSnapshotSource(
        std::shared_ptr<detail::ProductionTransitionLiveSnapshotBindingState>
            state) noexcept
        : state_(std::move(state)) {}

    std::shared_ptr<detail::ProductionTransitionLiveSnapshotBindingState> state_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
