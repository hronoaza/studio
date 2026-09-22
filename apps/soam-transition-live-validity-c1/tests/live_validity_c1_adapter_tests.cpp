#include "production_live_validity_c1_adapter.hpp"
#include "detail/production_transition_live_validity_internal.hpp"
#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"

#include <cstdlib>
#include <optional>
#include <type_traits>
#include <variant>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

ProductionBridgePolicyEvidence makeEvidence(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store)
{
    const auto snapshot =
        mesh.captureProductionRelationshipSourceSnapshot({0,1});
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
    const auto admitted =
        admissibility.evaluate(*envelope,*admissibilityPolicy,store);
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

    ProductionBridgePolicyEvidenceEvaluator d9a;
    const auto evidence = d9a.evaluate(
        std::get<VersionedProductionInterpretation>(*interpreted));
    require(evidence.has_value());
    require(std::holds_alternative<ProductionBridgePolicyEvidence>(*evidence));

    return std::get<ProductionBridgePolicyEvidence>(*evidence);
}

ProductionDerivedTransitionRequest makeSupportRequest(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store)
{
    const auto seed = makeEvidence(mesh,store);
    const auto persistencePolicy =
        ProductionPersistencePolicyProvider::createCurrent();
    require(persistencePolicy.has_value());

    const auto registry = mesh.productionBridgePersistenceRegistry();
    const auto stream = registry.openLiveStream(seed,*persistencePolicy);
    require(stream.has_value());

    mesh.injectExternalShock(0,0.1);
    const auto first = makeEvidence(mesh,store);
    const auto firstResult = stream->observe(first);
    require(firstResult.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*firstResult));
    require(std::get<ProductionPersistentBridgeRecommendation>(
        *firstResult).recommendation() ==
            PersistentBridgeRecommendation::PRESERVE);

    mesh.injectExternalShock(0,0.1);
    const auto second = makeEvidence(mesh,store);
    const auto secondResult = stream->observe(second);
    require(secondResult.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*secondResult));

    const auto recommendation =
        std::get<ProductionPersistentBridgeRecommendation>(*secondResult);
    require(recommendation.recommendation() ==
        PersistentBridgeRecommendation::SUPPORT);

    const auto requestPolicy =
        ProductionTransitionRequestPolicyProvider::createCurrent();
    require(requestPolicy.has_value());

    ProductionTransitionRequestDeriver deriver;
    const auto derived = deriver.derive(recommendation,*requestPolicy);
    require(derived.has_value());
    require(std::holds_alternative<ProductionDerivedTransitionRequest>(
        *derived));

    return std::get<ProductionDerivedTransitionRequest>(*derived);
}

void verifyProjection(
    const ProductionTransitionLiveValidityEvidence& source,
    const ProductionLiveValidityC1Prerequisites& projected)
{
    require(projected.liveValidityDecisionId() == source.decisionId());
    require(projected.freshnessDecisionId() == source.freshness().decisionId());
    require(
        projected.revalidationDecisionId() ==
        source.revalidation().decisionId());

    require(projected.freshness().context() == source.request().binding());
    require(projected.revalidation().context() == source.request().binding());

    require(
        projected.freshness().satisfied() ==
        source.freshness().satisfied());
    require(
        projected.revalidation().satisfied() ==
        source.revalidation().satisfied());
}

static_assert(!std::is_default_constructible_v<
    ProductionLiveValidityC1Prerequisites>);
static_assert(!std::is_constructible_v<
    FreshnessPrerequisiteEvidence,
    ProductionTransitionRequestBinding,
    bool>);
static_assert(!std::is_constructible_v<
    RevalidationPrerequisiteEvidence,
    ProductionTransitionRequestBinding,
    bool>);

int main() {
    detail::resetLiveValidityOpaqueIdGeneratorForTesting();

    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0,{0.0,0.0,0.0},1.0);
    mesh.addNode(1,{0.0,0.0,1.0},1.0);
    mesh.connectNodes(0,1);

    RetainedSourceEvidenceStore store;
    const auto request = makeSupportRequest(mesh,store);

    ProductionTransitionLiveValidityEvaluator evaluator{
        mesh.productionTransitionLiveSnapshotSource()};
    ProductionLiveValidityC1Adapter adapter;

    const auto current = evaluator.evaluate(request);
    require(current.has_value());
    require(std::holds_alternative<
        ProductionTransitionLiveValidityEvidence>(*current));

    const auto& currentEvidence =
        std::get<ProductionTransitionLiveValidityEvidence>(*current);
    const auto projectedCurrent = adapter.convert(currentEvidence);
    require(projectedCurrent.has_value());
    verifyProjection(currentEvidence,*projectedCurrent);
    require(projectedCurrent->freshness().satisfied());
    require(projectedCurrent->revalidation().satisfied());

    const auto repeatedCurrent = adapter.convert(currentEvidence);
    require(repeatedCurrent.has_value());
    verifyProjection(currentEvidence,*repeatedCurrent);
    require(
        repeatedCurrent->liveValidityDecisionId() ==
        projectedCurrent->liveValidityDecisionId());
    require(
        repeatedCurrent->freshnessDecisionId() ==
        projectedCurrent->freshnessDecisionId());
    require(
        repeatedCurrent->revalidationDecisionId() ==
        projectedCurrent->revalidationDecisionId());

    mesh.injectExternalShock(0,0.1);

    const auto stale = evaluator.evaluate(request);
    require(stale.has_value());
    require(std::holds_alternative<
        ProductionTransitionLiveValidityEvidence>(*stale));
    const auto& staleEvidence =
        std::get<ProductionTransitionLiveValidityEvidence>(*stale);

    const auto projectedStale = adapter.convert(staleEvidence);
    require(projectedStale.has_value());
    verifyProjection(staleEvidence,*projectedStale);
    require(!projectedStale->freshness().satisfied());
    require(projectedStale->revalidation().satisfied());

    mesh.pruneIsolatedBridges(1.1);

    const auto removed = evaluator.evaluate(request);
    require(removed.has_value());
    require(std::holds_alternative<
        ProductionTransitionLiveValidityEvidence>(*removed));
    const auto& removedEvidence =
        std::get<ProductionTransitionLiveValidityEvidence>(*removed);

    const auto projectedRemoved = adapter.convert(removedEvidence);
    require(projectedRemoved.has_value());
    verifyProjection(removedEvidence,*projectedRemoved);
    require(!projectedRemoved->freshness().satisfied());
    require(!projectedRemoved->revalidation().satisfied());

    detail::resetLiveValidityOpaqueIdGeneratorForTesting();
    return 0;
}
