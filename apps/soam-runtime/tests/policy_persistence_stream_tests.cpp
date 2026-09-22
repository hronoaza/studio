#include "policy_persistence.hpp"
#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"

#include <array>
#include <cstdlib>
#include <thread>
#include <type_traits>
#include <variant>
#include <vector>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

ProductionBridgePolicyEvidence makeEvidence(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store,
    const ProductionRelationshipLocator& locator)
{
    const auto snapshot =
        mesh.captureProductionRelationshipSourceSnapshot(locator);
    require(snapshot.has_value());

    const auto retention = store.retain(*snapshot);
    require(
        retention == SourceEvidenceRetentionResult::INSERTED ||
        retention == SourceEvidenceRetentionResult::ALREADY_RETAINED);

    ProductionProvenanceEnvelopeProducer producer;
    const auto envelope = producer.produce(*snapshot);
    require(envelope.has_value());

    const auto admissibilityPolicy =
        ProductionProvenanceAdmissibilityPolicyProvider::createCurrent();
    require(admissibilityPolicy.has_value());

    ProvenanceAdmissibilityEvaluator admissibility;
    const auto admitted = admissibility.evaluate(
        *envelope, *admissibilityPolicy, store);
    require(admitted.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(*admitted));

    const auto interpretationPolicy =
        ProductionInterpretationPolicyProvider::createCurrent();
    require(interpretationPolicy.has_value());

    VersionedProductionInterpreter interpreter;
    const auto interpreted = interpreter.interpret(
        std::get<AdmissibleProductionProvenance>(*admitted),
        *interpretationPolicy);
    require(interpreted.has_value());
    require(std::holds_alternative<VersionedProductionInterpretation>(
        *interpreted));

    ProductionBridgePolicyEvidenceEvaluator evidenceEvaluator;
    const auto result = evidenceEvaluator.evaluate(
        std::get<VersionedProductionInterpretation>(*interpreted));
    require(result.has_value());
    require(std::holds_alternative<ProductionBridgePolicyEvidence>(*result));

    return std::get<ProductionBridgePolicyEvidence>(*result);
}

static_assert(!std::is_default_constructible_v<PersistenceStreamInstanceId>);
static_assert(!std::is_default_constructible_v<PersistenceObservationDecisionId>);
static_assert(!std::is_copy_constructible_v<ProductionBridgePersistenceStream>);
static_assert(!std::is_move_constructible_v<ProductionBridgePersistenceStream>);
static_assert(!std::is_default_constructible_v<ProductionPersistenceStreamHandle>);
static_assert(!std::is_default_constructible_v<
    ProductionPersistentBridgeRecommendation>);

int main() {
    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0, {0.0,0.0,0.0}, 1.0);
    mesh.addNode(1, {0.0,0.0,1.0}, 1.0);
    mesh.connectNodes(0,1);

    RetainedSourceEvidenceStore store;
    const auto seed = makeEvidence(mesh, store, {0,1});

    const auto persistencePolicy =
        ProductionPersistencePolicyProvider::createCurrent();
    require(persistencePolicy.has_value());

    const auto registry = mesh.productionBridgePersistenceRegistry();
    const auto opened = registry.openLiveStream(seed, *persistencePolicy);
    require(opened.has_value());

    auto handle = *opened;
    const auto firstInstance = handle.instanceId();

    // Opening evidence is lineage only; its state version is at the stream epoch.
    const auto seedObservation = handle.observe(seed);
    require(seedObservation.has_value());
    require(std::holds_alternative<ProductionPersistenceRejection>(
        *seedObservation));
    require(std::get<ProductionPersistenceRejection>(*seedObservation)
        .primaryReason() ==
            ProductionPersistenceReason::PreStreamEpochStateVersion);

    mesh.injectExternalShock(0, 0.1);
    const auto first = makeEvidence(mesh, store, {0,1});
    require(first.stateVersion() > seed.stateVersion());

    const auto firstObservation = handle.observe(first);
    require(firstObservation.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*firstObservation));
    require(std::get<ProductionPersistentBridgeRecommendation>(
        *firstObservation).recommendation() ==
            PersistentBridgeRecommendation::PRESERVE);

    // Re-evaluating one capture may mint a new D9A decision but cannot count again.
    ProductionBridgePolicyEvidenceEvaluator d9a;
    const auto replayEvaluation = d9a.evaluate(first.interpretation());
    require(replayEvaluation.has_value());
    const auto replayEvidence =
        std::get<ProductionBridgePolicyEvidence>(*replayEvaluation);
    require(replayEvidence.sourceCaptureId() == first.sourceCaptureId());
    require(replayEvidence.decisionId() != first.decisionId());

    const auto duplicate = handle.observe(replayEvidence);
    require(duplicate.has_value());
    require(std::holds_alternative<ProductionPersistenceRejection>(*duplicate));
    require(std::get<ProductionPersistenceRejection>(*duplicate)
        .primaryReason() ==
            ProductionPersistenceReason::DuplicateSourceCapture);

    // A distinct capture of the unchanged mesh state is not a second sample.
    const auto sameState = makeEvidence(mesh, store, {0,1});
    require(sameState.sourceCaptureId() != first.sourceCaptureId());
    require(sameState.stateVersion() == first.stateVersion());

    const auto sameStateObservation = handle.observe(sameState);
    require(sameStateObservation.has_value());
    require(std::holds_alternative<ProductionPersistenceRejection>(
        *sameStateObservation));
    require(std::get<ProductionPersistenceRejection>(*sameStateObservation)
        .primaryReason() ==
            ProductionPersistenceReason::NonIncreasingStateVersion);

    // Second strong sample at a new state version activates SUPPORT.
    mesh.injectExternalShock(0, 0.1);
    const auto second = makeEvidence(mesh, store, {0,1});
    require(second.stateVersion() > first.stateVersion());

    const auto secondObservation = handle.observe(second);
    require(secondObservation.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*secondObservation));
    require(std::get<ProductionPersistentBridgeRecommendation>(
        *secondObservation).recommendation() ==
            PersistentBridgeRecommendation::SUPPORT);

    // Close drains the concrete instance; stale handles fail closed.
    require(registry.closeLiveStream(firstInstance));
    require(!handle.observe(second).has_value());
    require(!registry.closeLiveStream(firstInstance));

    // Re-opening the same semantic key creates a new instance and fresh epoch.
    const auto reopened = registry.openLiveStream(second, *persistencePolicy);
    require(reopened.has_value());
    require(reopened->instanceId() != firstInstance);

    const auto historical = reopened->observe(second);
    require(historical.has_value());
    require(std::holds_alternative<ProductionPersistenceRejection>(*historical));
    require(std::get<ProductionPersistenceRejection>(*historical)
        .primaryReason() ==
            ProductionPersistenceReason::PreStreamEpochStateVersion);

    // Concurrent replay of one fresh capture counts exactly once.
    mesh.injectExternalShock(0, 0.1);
    const auto concurrentEvidence = makeEvidence(mesh, store, {0,1});

    std::array<std::optional<ProductionPersistenceObservationResult>,2> results;
    std::thread a([&] { results[0] = reopened->observe(concurrentEvidence); });
    std::thread b([&] { results[1] = reopened->observe(concurrentEvidence); });
    a.join();
    b.join();

    std::size_t successes = 0;
    std::size_t duplicates = 0;
    for (const auto& result : results) {
        require(result.has_value());
        if (std::holds_alternative<
                ProductionPersistentBridgeRecommendation>(*result)) {
            ++successes;
        } else {
            const auto reason =
                std::get<ProductionPersistenceRejection>(*result)
                    .primaryReason();
            require(reason == ProductionPersistenceReason::DuplicateSourceCapture);
            ++duplicates;
        }
    }
    require(successes == 1U);
    require(duplicates == 1U);

    // Relationship removal terminates the old stream instance.
    mesh.pruneIsolatedBridges(1.1);
    const auto afterRemoval = reopened->observe(concurrentEvidence);
    require(afterRemoval.has_value());
    require(std::holds_alternative<ProductionPersistenceRejection>(
        *afterRemoval));
    require(std::get<ProductionPersistenceRejection>(*afterRemoval)
        .primaryReason() ==
            ProductionPersistenceReason::WrongRelationship);

    // The lifecycle rejection closes the instance; stale handle is now dead.
    require(!reopened->observe(concurrentEvidence).has_value());

    // Recreate the relationship: the new generation cannot reuse old stream state.
    mesh.connectNodes(0,1);
    const auto newGenerationSeed = makeEvidence(mesh, store, {0,1});
    require(newGenerationSeed.relationshipGeneration() !=
        concurrentEvidence.relationshipGeneration());

    const auto newGenerationStream =
        registry.openLiveStream(newGenerationSeed, *persistencePolicy);
    require(newGenerationStream.has_value());
    require(newGenerationStream->instanceId() != reopened->instanceId());

    const auto newGenerationHistorical =
        newGenerationStream->observe(newGenerationSeed);
    require(newGenerationHistorical.has_value());
    require(std::holds_alternative<ProductionPersistenceRejection>(
        *newGenerationHistorical));
    require(std::get<ProductionPersistenceRejection>(
        *newGenerationHistorical).primaryReason() ==
            ProductionPersistenceReason::PreStreamEpochStateVersion);

    return 0;
}
