#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;
enum class BridgeStatus;

namespace detail {
class ProductionTransitionResilienceSnapshotBindingState;
}

class ProductionTransitionResilienceDirectedEdgeSnapshot final {
public:
    ProductionTransitionResilienceDirectedEdgeSnapshot(
        const ProductionTransitionResilienceDirectedEdgeSnapshot&) noexcept = default;
    ProductionTransitionResilienceDirectedEdgeSnapshot& operator=(
        const ProductionTransitionResilienceDirectedEdgeSnapshot&) noexcept = default;

    [[nodiscard]] std::size_t sourceNodeId() const noexcept { return sourceNodeId_; }
    [[nodiscard]] std::size_t targetNodeId() const noexcept { return targetNodeId_; }
    [[nodiscard]] std::uint64_t generation() const noexcept { return generation_; }
    [[nodiscard]] double capacity() const noexcept { return capacity_; }
    [[nodiscard]] double distance() const noexcept { return distance_; }
    [[nodiscard]] double orientationWeight() const noexcept { return orientationWeight_; }
    [[nodiscard]] BridgeStatus status() const noexcept { return status_; }

private:
    ProductionTransitionResilienceDirectedEdgeSnapshot(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t generation,
        double capacity,
        double distance,
        double orientationWeight,
        BridgeStatus status) noexcept
        : sourceNodeId_(sourceNodeId),
          targetNodeId_(targetNodeId),
          generation_(generation),
          capacity_(capacity),
          distance_(distance),
          orientationWeight_(orientationWeight),
          status_(status) {}

    std::size_t sourceNodeId_;
    std::size_t targetNodeId_;
    std::uint64_t generation_;
    double capacity_;
    double distance_;
    double orientationWeight_;
    BridgeStatus status_;

    friend class SpatialAdaptiveMesh;
};

class ProductionTransitionResilienceRelationshipSnapshot final {
public:
    ProductionTransitionResilienceRelationshipSnapshot(
        const ProductionTransitionResilienceRelationshipSnapshot&) = default;
    ProductionTransitionResilienceRelationshipSnapshot& operator=(
        const ProductionTransitionResilienceRelationshipSnapshot&) = default;

    [[nodiscard]] std::size_t nodeA() const noexcept { return nodeA_; }
    [[nodiscard]] std::size_t nodeB() const noexcept { return nodeB_; }

    [[nodiscard]] const std::vector<
        ProductionTransitionResilienceDirectedEdgeSnapshot>&
    aToB() const noexcept { return aToB_; }

    [[nodiscard]] const std::vector<
        ProductionTransitionResilienceDirectedEdgeSnapshot>&
    bToA() const noexcept { return bToA_; }

private:
    ProductionTransitionResilienceRelationshipSnapshot(
        std::size_t nodeA,
        std::size_t nodeB,
        std::vector<ProductionTransitionResilienceDirectedEdgeSnapshot> aToB,
        std::vector<ProductionTransitionResilienceDirectedEdgeSnapshot> bToA)
        : nodeA_(nodeA),
          nodeB_(nodeB),
          aToB_(std::move(aToB)),
          bToA_(std::move(bToA)) {}

    std::size_t nodeA_;
    std::size_t nodeB_;
    std::vector<ProductionTransitionResilienceDirectedEdgeSnapshot> aToB_;
    std::vector<ProductionTransitionResilienceDirectedEdgeSnapshot> bToA_;

    friend class SpatialAdaptiveMesh;
};

class ProductionTransitionResilienceSnapshot final {
public:
    ProductionTransitionResilienceSnapshot(
        const ProductionTransitionResilienceSnapshot&) = default;
    ProductionTransitionResilienceSnapshot& operator=(
        const ProductionTransitionResilienceSnapshot&) = default;

    [[nodiscard]] std::uint64_t stateVersion() const noexcept {
        return stateVersion_;
    }

    [[nodiscard]] const std::vector<std::size_t>& nodeIds() const noexcept {
        return nodeIds_;
    }

    [[nodiscard]] const std::vector<
        ProductionTransitionResilienceRelationshipSnapshot>&
    relationships() const noexcept { return relationships_; }

private:
    ProductionTransitionResilienceSnapshot(
        std::uint64_t stateVersion,
        std::vector<std::size_t> nodeIds,
        std::vector<ProductionTransitionResilienceRelationshipSnapshot>
            relationships)
        : stateVersion_(stateVersion),
          nodeIds_(std::move(nodeIds)),
          relationships_(std::move(relationships)) {}

    std::uint64_t stateVersion_;
    std::vector<std::size_t> nodeIds_;
    std::vector<ProductionTransitionResilienceRelationshipSnapshot>
        relationships_;

    friend class SpatialAdaptiveMesh;
};

class ProductionTransitionResilienceSnapshotSource final {
public:
    ProductionTransitionResilienceSnapshotSource(
        const ProductionTransitionResilienceSnapshotSource&) noexcept = default;
    ProductionTransitionResilienceSnapshotSource& operator=(
        const ProductionTransitionResilienceSnapshotSource&) noexcept = default;

    [[nodiscard]]
    std::optional<ProductionTransitionResilienceSnapshot>
    capture() const;

private:
    explicit ProductionTransitionResilienceSnapshotSource(
        std::shared_ptr<
            detail::ProductionTransitionResilienceSnapshotBindingState>
            state) noexcept
        : state_(std::move(state)) {}

    std::shared_ptr<
        detail::ProductionTransitionResilienceSnapshotBindingState> state_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
