#include "production_transition_resilience_snapshot.hpp"
#include "system_architecture.hpp"
#include "detail/production_transition_resilience_snapshot_binding.hpp"

#include <condition_variable>
#include <mutex>
#include <utility>

namespace AdaptiveMesh::detail {

class ProductionTransitionResilienceSnapshotBindingState final {
public:
    explicit ProductionTransitionResilienceSnapshotBindingState(
        SpatialAdaptiveMesh* owner) noexcept
        : owner_(owner) {}

private:
    std::mutex mutex_;
    std::condition_variable drained_;
    SpatialAdaptiveMesh* owner_ = nullptr;
    std::size_t activeLeases_ = 0;
    bool acceptingLeases_ = true;

    friend class ResilienceSnapshotLease;
    friend class ::AdaptiveMesh::ProductionTransitionResilienceSnapshotSource;
    friend std::shared_ptr<
        ProductionTransitionResilienceSnapshotBindingState>
    makeProductionTransitionResilienceSnapshotBinding(SpatialAdaptiveMesh*);
    friend void invalidateProductionTransitionResilienceSnapshotBinding(
        const std::shared_ptr<
            ProductionTransitionResilienceSnapshotBindingState>&) noexcept;
};

class ResilienceSnapshotLease final {
public:
    ResilienceSnapshotLease() noexcept = default;
    ResilienceSnapshotLease(const ResilienceSnapshotLease&) = delete;
    ResilienceSnapshotLease& operator=(const ResilienceSnapshotLease&) = delete;

    ResilienceSnapshotLease(ResilienceSnapshotLease&& other) noexcept
        : state_(std::move(other.state_)),
          owner_(std::exchange(other.owner_, nullptr)) {}

    ~ResilienceSnapshotLease() { release(); }

    [[nodiscard]] explicit operator bool() const noexcept {
        return owner_ != nullptr;
    }

    [[nodiscard]] SpatialAdaptiveMesh& owner() const noexcept {
        return *owner_;
    }

    static ResilienceSnapshotLease acquire(
        const std::shared_ptr<
            ProductionTransitionResilienceSnapshotBindingState>& state) noexcept
    {
        if (!state) return {};

        std::lock_guard lock(state->mutex_);
        if (!state->acceptingLeases_ || state->owner_ == nullptr) return {};

        ++state->activeLeases_;
        return ResilienceSnapshotLease{state,state->owner_};
    }

private:
    ResilienceSnapshotLease(
        std::shared_ptr<ProductionTransitionResilienceSnapshotBindingState>
            state,
        SpatialAdaptiveMesh* owner) noexcept
        : state_(std::move(state)), owner_(owner) {}

    void release() noexcept {
        if (!state_) return;
        std::lock_guard lock(state_->mutex_);
        if (state_->activeLeases_ > 0) --state_->activeLeases_;
        if (!state_->acceptingLeases_ && state_->activeLeases_ == 0) {
            state_->drained_.notify_all();
        }
        owner_ = nullptr;
        state_.reset();
    }

    std::shared_ptr<ProductionTransitionResilienceSnapshotBindingState> state_;
    SpatialAdaptiveMesh* owner_ = nullptr;
};

std::shared_ptr<ProductionTransitionResilienceSnapshotBindingState>
makeProductionTransitionResilienceSnapshotBinding(SpatialAdaptiveMesh* owner)
{
    return std::make_shared<
        ProductionTransitionResilienceSnapshotBindingState>(owner);
}

void invalidateProductionTransitionResilienceSnapshotBinding(
    const std::shared_ptr<
        ProductionTransitionResilienceSnapshotBindingState>& state) noexcept
{
    if (!state) return;
    std::unique_lock lock(state->mutex_);
    state->acceptingLeases_ = false;
    state->drained_.wait(lock,[&] { return state->activeLeases_ == 0; });
    state->owner_ = nullptr;
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {

std::optional<ProductionTransitionResilienceSnapshot>
ProductionTransitionResilienceSnapshotSource::capture() const
{
    auto lease = detail::ResilienceSnapshotLease::acquire(state_);
    if (!lease) return std::nullopt;
    return lease.owner().captureTransitionResilienceSnapshot();
}

} // namespace AdaptiveMesh
