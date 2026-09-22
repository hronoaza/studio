#include "production_transition_live_snapshot.hpp"
#include "system_architecture.hpp"
#include "detail/production_transition_live_snapshot_binding.hpp"

#include <condition_variable>
#include <mutex>
#include <utility>

namespace AdaptiveMesh::detail {

class ProductionTransitionLiveSnapshotBindingState final {
public:
    explicit ProductionTransitionLiveSnapshotBindingState(
        SpatialAdaptiveMesh* owner) noexcept
        : owner_(owner) {}

private:
    std::mutex mutex_;
    std::condition_variable drained_;
    SpatialAdaptiveMesh* owner_ = nullptr;
    std::size_t activeLeases_ = 0;
    bool acceptingLeases_ = true;

    friend class SnapshotLease;
    friend class ::AdaptiveMesh::ProductionTransitionLiveSnapshotSource;
    friend std::shared_ptr<ProductionTransitionLiveSnapshotBindingState>
    makeProductionTransitionLiveSnapshotBinding(SpatialAdaptiveMesh*);
    friend void invalidateProductionTransitionLiveSnapshotBinding(
        const std::shared_ptr<
            ProductionTransitionLiveSnapshotBindingState>&) noexcept;
};

class SnapshotLease final {
public:
    SnapshotLease() noexcept = default;
    SnapshotLease(const SnapshotLease&) = delete;
    SnapshotLease& operator=(const SnapshotLease&) = delete;

    SnapshotLease(SnapshotLease&& other) noexcept
        : state_(std::move(other.state_)),
          owner_(std::exchange(other.owner_, nullptr)) {}

    ~SnapshotLease() { release(); }

    [[nodiscard]] explicit operator bool() const noexcept {
        return owner_ != nullptr;
    }

    [[nodiscard]] SpatialAdaptiveMesh& owner() const noexcept {
        return *owner_;
    }

    static SnapshotLease acquire(
        const std::shared_ptr<
            ProductionTransitionLiveSnapshotBindingState>& state) noexcept
    {
        if (!state) return {};

        std::lock_guard lock(state->mutex_);
        if (!state->acceptingLeases_ || state->owner_ == nullptr) {
            return {};
        }

        ++state->activeLeases_;
        return SnapshotLease{state,state->owner_};
    }

private:
    SnapshotLease(
        std::shared_ptr<ProductionTransitionLiveSnapshotBindingState> state,
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

    std::shared_ptr<ProductionTransitionLiveSnapshotBindingState> state_;
    SpatialAdaptiveMesh* owner_ = nullptr;
};

std::shared_ptr<ProductionTransitionLiveSnapshotBindingState>
makeProductionTransitionLiveSnapshotBinding(
    SpatialAdaptiveMesh* owner)
{
    return std::make_shared<
        ProductionTransitionLiveSnapshotBindingState>(owner);
}

void invalidateProductionTransitionLiveSnapshotBinding(
    const std::shared_ptr<
        ProductionTransitionLiveSnapshotBindingState>& state) noexcept
{
    if (!state) return;

    std::unique_lock lock(state->mutex_);
    state->acceptingLeases_ = false;
    state->drained_.wait(lock,[&] {
        return state->activeLeases_ == 0;
    });
    state->owner_ = nullptr;
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {

std::optional<ProductionTransitionLiveSnapshot>
ProductionTransitionLiveSnapshotSource::capture(
    std::size_t sourceNodeId,
    std::size_t targetNodeId) const
{
    auto lease = detail::SnapshotLease::acquire(state_);
    if (!lease) return std::nullopt;

    return lease.owner().captureTransitionLiveValiditySnapshot(
        sourceNodeId,targetNodeId);
}

} // namespace AdaptiveMesh
