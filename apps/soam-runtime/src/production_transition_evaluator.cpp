#include "production_transition_evaluator.hpp"
#include "system_architecture.hpp"
#include "detail/production_transition_evaluation_binding.hpp"

#include <condition_variable>
#include <mutex>
#include <utility>

namespace AdaptiveMesh::detail {

class ProductionTransitionEvaluationBindingState final {
public:
    explicit ProductionTransitionEvaluationBindingState(
        SpatialAdaptiveMesh* owner) noexcept
        : owner_(owner) {}

private:
    std::mutex mutex_;
    std::condition_variable drained_;
    SpatialAdaptiveMesh* owner_ = nullptr;
    std::size_t activeLeases_ = 0;
    bool acceptingLeases_ = true;

    friend class EvaluationLease;
    friend class ::AdaptiveMesh::ProductionTransitionEvaluator;
    friend std::shared_ptr<ProductionTransitionEvaluationBindingState>
    makeProductionTransitionEvaluationBinding(SpatialAdaptiveMesh*);
    friend void invalidateProductionTransitionEvaluationBinding(
        const std::shared_ptr<ProductionTransitionEvaluationBindingState>&) noexcept;
};

class EvaluationLease final {
public:
    EvaluationLease() noexcept = default;
    EvaluationLease(const EvaluationLease&) = delete;
    EvaluationLease& operator=(const EvaluationLease&) = delete;

    EvaluationLease(EvaluationLease&& other) noexcept
        : state_(std::move(other.state_)),
          owner_(std::exchange(other.owner_, nullptr)) {}

    EvaluationLease& operator=(EvaluationLease&& other) noexcept {
        if (this != &other) {
            release();
            state_ = std::move(other.state_);
            owner_ = std::exchange(other.owner_, nullptr);
        }
        return *this;
    }

    ~EvaluationLease() { release(); }

    [[nodiscard]] explicit operator bool() const noexcept {
        return owner_ != nullptr;
    }

    [[nodiscard]] SpatialAdaptiveMesh& owner() const noexcept {
        return *owner_;
    }

    static EvaluationLease acquire(
        const std::shared_ptr<ProductionTransitionEvaluationBindingState>& state) noexcept
    {
        if (!state) return {};

        std::lock_guard lock(state->mutex_);
        if (!state->acceptingLeases_ || state->owner_ == nullptr) {
            return {};
        }

        ++state->activeLeases_;
        return EvaluationLease{state, state->owner_};
    }

private:
    EvaluationLease(
        std::shared_ptr<ProductionTransitionEvaluationBindingState> state,
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

    std::shared_ptr<ProductionTransitionEvaluationBindingState> state_;
    SpatialAdaptiveMesh* owner_ = nullptr;
};

std::shared_ptr<ProductionTransitionEvaluationBindingState>
makeProductionTransitionEvaluationBinding(
    SpatialAdaptiveMesh* owner)
{
    return std::make_shared<ProductionTransitionEvaluationBindingState>(owner);
}

void invalidateProductionTransitionEvaluationBinding(
    const std::shared_ptr<ProductionTransitionEvaluationBindingState>& state) noexcept
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

ProductionTransitionEvaluation ProductionTransitionEvaluator::evaluate(
    const ProductionRelationshipLocator& locator) const
{
    auto lease = detail::EvaluationLease::acquire(state_);
    if (!lease) {
        return ProductionTransitionEvaluation::not_eligible;
    }

    auto& mesh = lease.owner();

    if (!mesh.transitionLocatorIsValid(
            locator.sourceNodeId,
            locator.targetNodeId)) {
        return ProductionTransitionEvaluation::not_eligible;
    }

    const auto snapshot = mesh.captureTransitionSnapshot(
        locator.sourceNodeId,
        locator.targetNodeId);

    if (!snapshot) {
        return ProductionTransitionEvaluation::no_request;
    }

    if (!mesh.revalidateTransitionSnapshot(*snapshot)) {
        return ProductionTransitionEvaluation::not_eligible;
    }

    // Phase C2 remains fail-closed here. The accepted runtime does not yet
    // expose production-native observation/confidence provenance sufficient
    // to derive a transition direction without conflating BridgeStatus with
    // PersistentBridgeRecommendation.
    return ProductionTransitionEvaluation::not_eligible;
}

} // namespace AdaptiveMesh
