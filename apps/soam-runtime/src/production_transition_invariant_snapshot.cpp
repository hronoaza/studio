#include "production_transition_invariant_snapshot.hpp"
#include "system_architecture.hpp"
#include "detail/production_transition_invariant_snapshot_binding.hpp"

#include <condition_variable>
#include <mutex>
#include <utility>

namespace AdaptiveMesh::detail {

class ProductionTransitionInvariantSnapshotBindingState final {
public:
    explicit ProductionTransitionInvariantSnapshotBindingState(
        SpatialAdaptiveMesh* owner) noexcept
        : owner_(owner) {}

private:
    std::mutex mutex_;
    std::condition_variable drained_;
    SpatialAdaptiveMesh* owner_ = nullptr;
    std::size_t activeLeases_ = 0;
    bool acceptingLeases_ = true;

    friend class InvariantSnapshotLease;
    friend class ::AdaptiveMesh::ProductionTransitionInvariantSnapshotSource;
    friend std::shared_ptr<
        ProductionTransitionInvariantSnapshotBindingState>
    makeProductionTransitionInvariantSnapshotBinding(SpatialAdaptiveMesh*);
    friend void invalidateProductionTransitionInvariantSnapshotBinding(
        const std::shared_ptr<
            ProductionTransitionInvariantSnapshotBindingState>&) noexcept;
};

class InvariantSnapshotLease final {
public:
    InvariantSnapshotLease() noexcept = default;
    InvariantSnapshotLease(const InvariantSnapshotLease&) = delete;
    InvariantSnapshotLease& operator=(const InvariantSnapshotLease&) = delete;

    InvariantSnapshotLease(InvariantSnapshotLease&& other) noexcept
        : state_(std::move(other.state_)),
          owner_(std::exchange(other.owner_, nullptr)) {}

    ~InvariantSnapshotLease() { release(); }

    [[nodiscard]] explicit operator bool() const noexcept {
        return owner_ != nullptr;
    }

    [[nodiscard]] SpatialAdaptiveMesh& owner() const noexcept {
        return *owner_;
    }

    static InvariantSnapshotLease acquire(
        const std::shared_ptr<
            ProductionTransitionInvariantSnapshotBindingState>& state) noexcept
    {
        if (!state) return {};

        std::lock_guard lock(state->mutex_);
        if (!state->acceptingLeases_ || state->owner_ == nullptr) {
            return {};
        }

        ++state->activeLeases_;
        return InvariantSnapshotLease{state, state->owner_};
    }

private:
    InvariantSnapshotLease(
        std::shared_ptr<
            ProductionTransitionInvariantSnapshotBindingState> state,
        SpatialAdaptiveMesh* owner) noexcept
        : state_(std::move(state)), owner_(owner) {}

    void release() noexcept {
        if (!state_) return;

        std::lock_guard lock(state_->mutex_);
        if (state_->activeLeases_ > 0) {
            --state_->activeLeases_;
        }
        if (!state_->acceptingLeases_ && state_->activeLeases_ == 0) {
            state_->drained_.notify_all();
        }

        owner_ = nullptr;
        state_.reset();
    }

    std::shared_ptr<ProductionTransitionInvariantSnapshotBindingState> state_;
    SpatialAdaptiveMesh* owner_ = nullptr;
};

std::shared_ptr<ProductionTransitionInvariantSnapshotBindingState>
makeProductionTransitionInvariantSnapshotBinding(
    SpatialAdaptiveMesh* owner)
{
    return std::make_shared<
        ProductionTransitionInvariantSnapshotBindingState>(owner);
}

void invalidateProductionTransitionInvariantSnapshotBinding(
    const std::shared_ptr<
        ProductionTransitionInvariantSnapshotBindingState>& state) noexcept
{
    if (!state) return;

    std::unique_lock lock(state->mutex_);
    state->acceptingLeases_ = false;
    state->drained_.wait(lock, [&] {
        return state->activeLeases_ == 0;
    });
    state->owner_ = nullptr;
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {

std::optional<ProductionTransitionInvariantSnapshot>
ProductionTransitionInvariantSnapshotSource::capture(
    std::size_t sourceNodeId,
    std::size_t targetNodeId) const
{
    auto lease = detail::InvariantSnapshotLease::acquire(state_);
    if (!lease) return std::nullopt;

    return lease.owner().captureTransitionInvariantSnapshot(
        sourceNodeId, targetNodeId);
}

} // namespace AdaptiveMesh
