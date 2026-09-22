#include "policy_persistence.hpp"
#include "system_architecture.hpp"
#include "detail/policy_persistence_binding.hpp"
#include "detail/policy_persistence_internal.hpp"

#include <algorithm>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace AdaptiveMesh {

namespace {

template<std::size_t N>
[[nodiscard]] bool nonZero(
    const std::array<std::uint8_t,N>& bytes) noexcept
{
    for (const auto byte : bytes) {
        if (byte != 0U) return true;
    }
    return false;
}

[[nodiscard]] std::uint64_t persistenceReasonFlag(
    ProductionPersistenceReason reason) noexcept
{
    return std::uint64_t{1}
        << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool interpretationMatchesKey(
    const ProductionBridgePolicyEvidence& evidence,
    const ProductionPersistenceStreamKey& key) noexcept
{
    const auto& descriptor =
        evidence.interpretation().interpretationPolicyDescriptor();

    return
        descriptor.policyId == key.interpretationPolicyId() &&
        descriptor.majorVersion == key.interpretationPolicyMajor() &&
        descriptor.minorVersion == key.interpretationPolicyMinor() &&
        descriptor.implementationRevisionKind ==
            key.interpretationImplementationRevisionKind() &&
        descriptor.implementationRevision ==
            key.interpretationImplementationRevision();
}

[[nodiscard]] bool persistencePolicyMatchesKey(
    const PersistencePolicySnapshot& policy,
    const ProductionPersistenceStreamKey& key) noexcept
{
    const auto& descriptor = policy.descriptor();

    return
        descriptor.profileId == key.persistenceProfileId() &&
        descriptor.majorVersion == key.persistenceProfileMajor() &&
        descriptor.minorVersion == key.persistenceProfileMinor() &&
        descriptor.implementationRevisionKind ==
            key.persistenceImplementationRevisionKind() &&
        descriptor.implementationRevision ==
            key.persistenceImplementationRevision();
}

[[nodiscard]] bool evidenceLineageLooksValid(
    const ProductionBridgePolicyEvidence& evidence) noexcept
{
    return
        nonZero(evidence.decisionId().bytes()) &&
        nonZero(evidence.sourceCaptureId().bytes()) &&
        nonZero(evidence.provenanceItemId().bytes()) &&
        nonZero(evidence.admissibilityDecisionId().bytes()) &&
        nonZero(evidence.interpretationDecisionId().bytes());
}

} // namespace

struct ProductionBridgePersistenceStream::Impl final {
    Impl(
        PersistenceStreamInstanceId instanceIdValue,
        ProductionPersistenceStreamKey keyValue,
        PersistencePolicySnapshot policyValue,
        std::uint64_t streamStartStateVersionValue)
        : instanceId(std::move(instanceIdValue)),
          key(std::move(keyValue)),
          policy(std::move(policyValue)),
          persistence(
              policy.activationThreshold(),
              policy.releaseThreshold(),
              policy.activationSamples(),
              policy.releaseSamples()),
          streamStartStateVersion(streamStartStateVersionValue)
    {
    }

    mutable std::mutex mutex;
    PersistenceStreamInstanceId instanceId;
    ProductionPersistenceStreamKey key;
    PersistencePolicySnapshot policy;
    BridgePersistence persistence;
    std::uint64_t streamStartStateVersion;
    std::optional<std::uint64_t> lastAcceptedStateVersion;
    std::vector<SourceCaptureId::Bytes> acceptedCaptureIds;
};

ProductionBridgePersistenceStream::ProductionBridgePersistenceStream(
    PersistenceStreamInstanceId instanceId,
    ProductionPersistenceStreamKey key,
    PersistencePolicySnapshot policy,
    std::uint64_t streamStartStateVersion)
    : impl_(std::make_unique<Impl>(
          std::move(instanceId),
          std::move(key),
          std::move(policy),
          streamStartStateVersion))
{
}

ProductionBridgePersistenceStream::~ProductionBridgePersistenceStream() = default;

const PersistenceStreamInstanceId&
ProductionBridgePersistenceStream::instanceId() const noexcept
{
    return impl_->instanceId;
}

const ProductionPersistenceStreamKey&
ProductionBridgePersistenceStream::key() const noexcept
{
    return impl_->key;
}

std::uint64_t
ProductionBridgePersistenceStream::streamStartStateVersion() const noexcept
{
    return impl_->streamStartStateVersion;
}

std::optional<ProductionPersistenceObservationResult>
ProductionBridgePersistenceStream::rejectWithoutMutation(
    const ProductionBridgePolicyEvidence& evidence,
    ProductionPersistenceReason reason)
{
    std::lock_guard lock(impl_->mutex);

    const auto decisionBytes = detail::tryGenerateD9OpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;

    return ProductionPersistenceObservationResult{
        ProductionPersistenceRejection{
            PersistenceObservationDecisionId{*decisionBytes},
            impl_->instanceId,
            evidence.decisionId(),
            reason,
            persistenceReasonFlag(reason)
        }
    };
}

std::optional<ProductionPersistenceObservationResult>
ProductionBridgePersistenceStream::observe(
    const ProductionBridgePolicyEvidence& evidence)
{
    std::lock_guard lock(impl_->mutex);

    const auto decisionBytes = detail::tryGenerateD9OpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;

    PersistenceObservationDecisionId decisionId{*decisionBytes};

    const auto reject = [&](
        ProductionPersistenceReason reason)
        -> std::optional<ProductionPersistenceObservationResult>
    {
        return ProductionPersistenceObservationResult{
            ProductionPersistenceRejection{
                decisionId,
                impl_->instanceId,
                evidence.decisionId(),
                reason,
                persistenceReasonFlag(reason)
            }
        };
    };

    if (evidence.sourceNodeId() != impl_->key.sourceNodeId() ||
        evidence.targetNodeId() != impl_->key.targetNodeId()) {
        return reject(ProductionPersistenceReason::WrongRelationship);
    }

    if (evidence.relationshipGeneration() !=
        impl_->key.relationshipGeneration()) {
        return reject(
            ProductionPersistenceReason::WrongRelationshipGeneration);
    }

    if (!interpretationMatchesKey(evidence, impl_->key)) {
        return reject(
            ProductionPersistenceReason::WrongInterpretationPolicy);
    }

    if (!persistencePolicyMatchesKey(impl_->policy, impl_->key)) {
        return reject(
            ProductionPersistenceReason::WrongPersistenceProfile);
    }

    if (evidence.stateVersion() <= impl_->streamStartStateVersion) {
        return reject(
            ProductionPersistenceReason::PreStreamEpochStateVersion);
    }

    const auto captureBytes = evidence.sourceCaptureId().bytes();
    if (std::find(
            impl_->acceptedCaptureIds.begin(),
            impl_->acceptedCaptureIds.end(),
            captureBytes) != impl_->acceptedCaptureIds.end()) {
        return reject(
            ProductionPersistenceReason::DuplicateSourceCapture);
    }

    if (impl_->lastAcceptedStateVersion.has_value() &&
        evidence.stateVersion() <= *impl_->lastAcceptedStateVersion) {
        return reject(
            ProductionPersistenceReason::NonIncreasingStateVersion);
    }

    if (!evidenceLineageLooksValid(evidence)) {
        return reject(
            ProductionPersistenceReason::LineageInconsistent);
    }

    AdaptiveBridgePolicy acceptedPolicy;
    const auto recomputed = acceptedPolicy.evaluate(
        evidence.interpretation().observation(),
        evidence.interpretation().confidence());

    if (recomputed.value() != evidence.evidence().value()) {
        return reject(
            ProductionPersistenceReason::EvidenceInvariantViolation);
    }

    try {
        if (impl_->acceptedCaptureIds.capacity() <
            impl_->acceptedCaptureIds.size() + 1U) {
            impl_->acceptedCaptureIds.reserve(
                impl_->acceptedCaptureIds.size() + 1U);
        }
    } catch (...) {
        return std::nullopt;
    }

    const auto recommendation =
        impl_->persistence.observe(evidence.evidence());

    impl_->acceptedCaptureIds.push_back(captureBytes);
    impl_->lastAcceptedStateVersion = evidence.stateVersion();

    return ProductionPersistenceObservationResult{
        ProductionPersistentBridgeRecommendation{
            std::move(decisionId),
            impl_->instanceId,
            impl_->key,
            recommendation,
            evidence.decisionId(),
            evidence.sourceCaptureId(),
            evidence.stateVersion()
        }
    };
}

namespace detail {

class ProductionPersistenceStreamBindingState final {
public:
    explicit ProductionPersistenceStreamBindingState(
        ProductionBridgePersistenceStream* stream) noexcept
        : stream_(stream) {}

private:
    std::mutex mutex_;
    std::condition_variable drained_;
    ProductionBridgePersistenceStream* stream_ = nullptr;
    std::size_t activeLeases_ = 0;
    bool acceptingLeases_ = true;

    friend class PersistenceStreamLease;
    friend class ProductionPersistenceRegistryState;
};

class PersistenceStreamLease final {
public:
    PersistenceStreamLease() noexcept = default;
    PersistenceStreamLease(const PersistenceStreamLease&) = delete;
    PersistenceStreamLease& operator=(const PersistenceStreamLease&) = delete;

    PersistenceStreamLease(PersistenceStreamLease&& other) noexcept
        : state_(std::move(other.state_)),
          stream_(std::exchange(other.stream_, nullptr)) {}

    PersistenceStreamLease& operator=(PersistenceStreamLease&& other) noexcept {
        if (this != &other) {
            release();
            state_ = std::move(other.state_);
            stream_ = std::exchange(other.stream_, nullptr);
        }
        return *this;
    }

    ~PersistenceStreamLease() { release(); }

    [[nodiscard]] explicit operator bool() const noexcept {
        return stream_ != nullptr;
    }

    [[nodiscard]] ProductionBridgePersistenceStream& stream() const noexcept {
        return *stream_;
    }

    static PersistenceStreamLease acquire(
        const std::shared_ptr<ProductionPersistenceStreamBindingState>& state)
        noexcept
    {
        if (!state) return {};

        std::lock_guard lock(state->mutex_);
        if (!state->acceptingLeases_ || state->stream_ == nullptr) {
            return {};
        }

        ++state->activeLeases_;
        return PersistenceStreamLease{state, state->stream_};
    }

private:
    PersistenceStreamLease(
        std::shared_ptr<ProductionPersistenceStreamBindingState> state,
        ProductionBridgePersistenceStream* stream) noexcept
        : state_(std::move(state)),
          stream_(stream) {}

    void release() noexcept {
        if (!state_) return;

        std::lock_guard lock(state_->mutex_);
        if (state_->activeLeases_ > 0U) {
            --state_->activeLeases_;
        }
        if (!state_->acceptingLeases_ && state_->activeLeases_ == 0U) {
            state_->drained_.notify_all();
        }

        stream_ = nullptr;
        state_.reset();
    }

    std::shared_ptr<ProductionPersistenceStreamBindingState> state_;
    ProductionBridgePersistenceStream* stream_ = nullptr;
};

class ProductionPersistenceRegistryState final
    : public std::enable_shared_from_this<ProductionPersistenceRegistryState> {
public:
    explicit ProductionPersistenceRegistryState(
        SpatialAdaptiveMesh* owner) noexcept
        : owner_(owner) {}

    [[nodiscard]] std::optional<ProductionPersistenceStreamHandle>
    open(
        const ProductionBridgePolicyEvidence& seedLineage,
        const PersistencePolicySnapshot& policy)
    {
        auto lease = acquireOwnerLease();
        if (!lease.has_value()) return std::nullopt;

        const auto& interpretationDescriptor =
            seedLineage.interpretation().interpretationPolicyDescriptor();
        const auto& persistenceDescriptor = policy.descriptor();

        ProductionPersistenceStreamKey key{
            seedLineage.sourceNodeId(),
            seedLineage.targetNodeId(),
            seedLineage.relationshipGeneration(),
            interpretationDescriptor.policyId,
            interpretationDescriptor.majorVersion,
            interpretationDescriptor.minorVersion,
            interpretationDescriptor.implementationRevisionKind,
            interpretationDescriptor.implementationRevision,
            persistenceDescriptor.profileId,
            persistenceDescriptor.majorVersion,
            persistenceDescriptor.minorVersion,
            persistenceDescriptor.implementationRevisionKind,
            persistenceDescriptor.implementationRevision
        };

        {
            std::lock_guard streamLock(streamsMutex_);
            for (const auto& entry : liveStreams_) {
                if (entry.stream->key() == key) {
                    return ProductionPersistenceStreamHandle{
                        shared_from_this(),
                        entry.binding,
                        entry.stream->instanceId()
                    };
                }
            }
        }

        const auto snapshot =
            lease->owner().capturePersistenceCreationSnapshot(
                seedLineage.sourceNodeId(),
                seedLineage.targetNodeId());
        if (!snapshot.has_value()) return std::nullopt;

        if (snapshot->sourceNodeId != key.sourceNodeId() ||
            snapshot->targetNodeId != key.targetNodeId() ||
            snapshot->relationshipGeneration !=
                key.relationshipGeneration()) {
            return std::nullopt;
        }

        const auto instanceBytes = tryGenerateD9OpaqueIdBytes();
        if (!instanceBytes.has_value()) return std::nullopt;

        std::unique_ptr<ProductionBridgePersistenceStream> stream;
        std::shared_ptr<ProductionPersistenceStreamBindingState> binding;
        try {
            stream = std::unique_ptr<ProductionBridgePersistenceStream>(
                new ProductionBridgePersistenceStream{
                    PersistenceStreamInstanceId{*instanceBytes},
                    key,
                    policy,
                    snapshot->stateVersion
                });
            binding = std::make_shared<ProductionPersistenceStreamBindingState>(
                stream.get());
        } catch (...) {
            return std::nullopt;
        }

        std::lock_guard streamLock(streamsMutex_);

        for (const auto& entry : liveStreams_) {
            if (entry.stream->key() == key) {
                return ProductionPersistenceStreamHandle{
                    shared_from_this(),
                    entry.binding,
                    entry.stream->instanceId()
                };
            }
        }

        try {
            liveStreams_.push_back(
                LiveStreamEntry{
                    std::move(stream),
                    binding
                });
        } catch (...) {
            return std::nullopt;
        }

        const auto& published = liveStreams_.back();
        return ProductionPersistenceStreamHandle{
            shared_from_this(),
            published.binding,
            published.stream->instanceId()
        };
    }

    [[nodiscard]] std::optional<ProductionPersistenceObservationResult>
    observe(
        const std::shared_ptr<ProductionPersistenceStreamBindingState>& binding,
        const ProductionBridgePolicyEvidence& evidence)
    {
        std::optional<ProductionPersistenceObservationResult> result;
        std::optional<PersistenceStreamInstanceId> closeInstance;

        {
            auto streamLease = PersistenceStreamLease::acquire(binding);
            if (!streamLease) return std::nullopt;

            auto ownerLease = acquireOwnerLease();
            if (!ownerLease.has_value()) return std::nullopt;

            const auto& key = streamLease.stream().key();
            const auto snapshot =
                ownerLease->owner().capturePersistenceCreationSnapshot(
                    key.sourceNodeId(),
                    key.targetNodeId());

            if (!snapshot.has_value()) {
                result = streamLease.stream().rejectWithoutMutation(
                    evidence,
                    ProductionPersistenceReason::WrongRelationship);
                closeInstance = streamLease.stream().instanceId();
            } else if (
                snapshot->relationshipGeneration !=
                    key.relationshipGeneration()) {
                result = streamLease.stream().rejectWithoutMutation(
                    evidence,
                    ProductionPersistenceReason::WrongRelationshipGeneration);
                closeInstance = streamLease.stream().instanceId();
            } else {
                return streamLease.stream().observe(evidence);
            }
        }

        if (closeInstance.has_value()) {
            static_cast<void>(close(*closeInstance));
        }
        return result;
    }

    [[nodiscard]] bool close(
        const PersistenceStreamInstanceId& instanceId)
    {
        auto lease = acquireOwnerLease();
        if (!lease.has_value()) return false;

        std::unique_lock streamLock(streamsMutex_);
        const auto found = std::find_if(
            liveStreams_.begin(),
            liveStreams_.end(),
            [&instanceId](const LiveStreamEntry& entry) {
                return entry.stream->instanceId() == instanceId;
            });
        if (found == liveStreams_.end()) return false;

        invalidateStreamBinding(found->binding);
        liveStreams_.erase(found);
        return true;
    }

private:
    class OwnerLease final {
    public:
        OwnerLease() noexcept = default;
        OwnerLease(const OwnerLease&) = delete;
        OwnerLease& operator=(const OwnerLease&) = delete;

        OwnerLease(OwnerLease&& other) noexcept
            : state_(std::exchange(other.state_, nullptr)),
              owner_(std::exchange(other.owner_, nullptr)) {}

        OwnerLease& operator=(OwnerLease&& other) noexcept {
            if (this != &other) {
                release();
                state_ = std::exchange(other.state_, nullptr);
                owner_ = std::exchange(other.owner_, nullptr);
            }
            return *this;
        }

        ~OwnerLease() { release(); }

        [[nodiscard]] SpatialAdaptiveMesh& owner() const noexcept {
            return *owner_;
        }

    private:
        OwnerLease(
            ProductionPersistenceRegistryState* state,
            SpatialAdaptiveMesh* owner) noexcept
            : state_(state), owner_(owner) {}

        void release() noexcept {
            if (state_ == nullptr) return;
            std::lock_guard lock(state_->lifecycleMutex_);
            if (state_->activeOwnerLeases_ > 0U) {
                --state_->activeOwnerLeases_;
            }
            if (!state_->acceptingOwnerLeases_ &&
                state_->activeOwnerLeases_ == 0U) {
                state_->ownerDrained_.notify_all();
            }
            state_ = nullptr;
            owner_ = nullptr;
        }

        ProductionPersistenceRegistryState* state_ = nullptr;
        SpatialAdaptiveMesh* owner_ = nullptr;

        friend class ProductionPersistenceRegistryState;
    };

    struct LiveStreamEntry final {
        std::unique_ptr<ProductionBridgePersistenceStream> stream;
        std::shared_ptr<ProductionPersistenceStreamBindingState> binding;
    };

    [[nodiscard]] std::optional<OwnerLease> acquireOwnerLease() noexcept {
        std::lock_guard lock(lifecycleMutex_);
        if (!acceptingOwnerLeases_ || owner_ == nullptr) {
            return std::nullopt;
        }
        ++activeOwnerLeases_;
        return OwnerLease{this, owner_};
    }

    static void invalidateStreamBinding(
        const std::shared_ptr<ProductionPersistenceStreamBindingState>& state)
        noexcept
    {
        if (!state) return;

        std::unique_lock lock(state->mutex_);
        state->acceptingLeases_ = false;
        state->drained_.wait(lock, [&] {
            return state->activeLeases_ == 0U;
        });
        state->stream_ = nullptr;
    }

    void invalidate() noexcept {
        {
            std::unique_lock lock(lifecycleMutex_);
            acceptingOwnerLeases_ = false;
            ownerDrained_.wait(lock, [&] {
                return activeOwnerLeases_ == 0U;
            });
            owner_ = nullptr;
        }

        std::lock_guard streamLock(streamsMutex_);
        for (auto& entry : liveStreams_) {
            invalidateStreamBinding(entry.binding);
        }
        liveStreams_.clear();
    }

    std::mutex lifecycleMutex_;
    std::condition_variable ownerDrained_;
    SpatialAdaptiveMesh* owner_ = nullptr;
    std::size_t activeOwnerLeases_ = 0;
    bool acceptingOwnerLeases_ = true;

    std::mutex streamsMutex_;
    std::vector<LiveStreamEntry> liveStreams_;

    friend std::shared_ptr<ProductionPersistenceRegistryState>
    makeProductionPersistenceRegistryState(SpatialAdaptiveMesh*);
    friend void invalidateProductionPersistenceRegistryState(
        const std::shared_ptr<ProductionPersistenceRegistryState>&) noexcept;
};

std::shared_ptr<ProductionPersistenceRegistryState>
makeProductionPersistenceRegistryState(
    SpatialAdaptiveMesh* owner)
{
    return std::make_shared<ProductionPersistenceRegistryState>(owner);
}

void invalidateProductionPersistenceRegistryState(
    const std::shared_ptr<ProductionPersistenceRegistryState>& state) noexcept
{
    if (state) state->invalidate();
}

} // namespace detail

std::optional<ProductionPersistenceObservationResult>
ProductionPersistenceStreamHandle::observe(
    const ProductionBridgePolicyEvidence& evidence) const
{
    if (!registryState_) return std::nullopt;
    return registryState_->observe(state_, evidence);
}

std::optional<ProductionPersistenceStreamHandle>
ProductionBridgePersistenceRegistry::openLiveStream(
    const ProductionBridgePolicyEvidence& seedLineage,
    const PersistencePolicySnapshot& policy) const
{
    if (!state_) return std::nullopt;
    return state_->open(seedLineage, policy);
}

bool ProductionBridgePersistenceRegistry::closeLiveStream(
    const PersistenceStreamInstanceId& instanceId) const
{
    return state_ != nullptr && state_->close(instanceId);
}

} // namespace AdaptiveMesh
