#include "production_transition_live_validity.hpp"
#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"

#include <array>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <variant>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

template<std::size_t N>
bool nonZero(const std::array<std::uint8_t,N>& bytes) noexcept {
    for (const auto byte : bytes) {
        if (byte != 0U) return true;
    }
    return false;
}

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

static_assert(!std::is_default_constructible_v<LiveValidityDecisionId>);
static_assert(!std::is_default_constructible_v<FreshnessDecisionId>);
static_assert(!std::is_default_constructible_v<RevalidationDecisionId>);
static_assert(!std::is_default_constructible_v<
    ProductionFreshnessPrerequisiteRecord>);
static_assert(!std::is_default_constructible_v<
    ProductionRevalidationPrerequisiteRecord>);
static_assert(!std::is_default_constructible_v<
    ProductionTransitionLiveValidityEvidence>);

int main() {
    detail::resetLiveValidityOpaqueIdGeneratorForTesting();

    std::optional<ProductionTransitionLiveValidityEvaluator> staleEvaluator;
    std::optional<ProductionDerivedTransitionRequest> retainedRequest;

    {
        SpatialAdaptiveMesh mesh(2);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.connectNodes(0,1);

        RetainedSourceEvidenceStore store;
        const auto request = makeSupportRequest(mesh,store);
        retainedRequest.emplace(request);

        ProductionTransitionLiveValidityEvaluator evaluator{
            mesh.productionTransitionLiveSnapshotSource()};
        staleEvaluator.emplace(evaluator);

        const auto current = evaluator.evaluate(request);
        require(current.has_value());
        require(std::holds_alternative<
            ProductionTransitionLiveValidityEvidence>(*current));
        const auto& currentEvidence =
            std::get<ProductionTransitionLiveValidityEvidence>(*current);

        require(nonZero(currentEvidence.decisionId().bytes()));
        require(nonZero(currentEvidence.freshness().decisionId().bytes()));
        require(nonZero(currentEvidence.revalidation().decisionId().bytes()));
        require(currentEvidence.freshness().decisionId().bytes() !=
            currentEvidence.revalidation().decisionId().bytes());
        require(currentEvidence.freshness().binding() == request.binding());
        require(currentEvidence.revalidation().binding() == request.binding());
        require(currentEvidence.freshness().satisfied());
        require(currentEvidence.revalidation().satisfied());
        require(currentEvidence.revalidation().relationshipPresent());
        require(currentEvidence.revalidation()
            .observedRelationshipGeneration().has_value());

        mesh.injectExternalShock(0,0.1);

        const auto stale = evaluator.evaluate(request);
        require(stale.has_value());
        require(std::holds_alternative<
            ProductionTransitionLiveValidityEvidence>(*stale));
        const auto& staleEvidence =
            std::get<ProductionTransitionLiveValidityEvidence>(*stale);
        require(!staleEvidence.freshness().satisfied());
        require(staleEvidence.revalidation().satisfied());

        mesh.pruneIsolatedBridges(1.1);

        const auto removed = evaluator.evaluate(request);
        require(removed.has_value());
        require(std::holds_alternative<
            ProductionTransitionLiveValidityEvidence>(*removed));
        const auto& removedEvidence =
            std::get<ProductionTransitionLiveValidityEvidence>(*removed);
        require(!removedEvidence.freshness().satisfied());
        require(!removedEvidence.revalidation().satisfied());
        require(!removedEvidence.revalidation().relationshipPresent());
        require(!removedEvidence.revalidation()
            .observedRelationshipGeneration().has_value());

        mesh.connectNodes(0,1);

        const auto recreated = evaluator.evaluate(request);
        require(recreated.has_value());
        require(std::holds_alternative<
            ProductionTransitionLiveValidityEvidence>(*recreated));
        const auto& recreatedEvidence =
            std::get<ProductionTransitionLiveValidityEvidence>(*recreated);
        require(!recreatedEvidence.freshness().satisfied());
        require(!recreatedEvidence.revalidation().satisfied());
        require(recreatedEvidence.revalidation().relationshipPresent());
        require(recreatedEvidence.revalidation()
            .observedRelationshipGeneration().has_value());
        require(*recreatedEvidence.revalidation()
            .observedRelationshipGeneration() !=
            request.binding().relationship().generation());

        const auto repeated = evaluator.evaluate(request);
        require(repeated.has_value());
        require(std::holds_alternative<
            ProductionTransitionLiveValidityEvidence>(*repeated));
        const auto& repeatedEvidence =
            std::get<ProductionTransitionLiveValidityEvidence>(*repeated);
        require(repeatedEvidence.freshness().satisfied() ==
            recreatedEvidence.freshness().satisfied());
        require(repeatedEvidence.revalidation().satisfied() ==
            recreatedEvidence.revalidation().satisfied());
        require(repeatedEvidence.decisionId() != recreatedEvidence.decisionId());
    }

    require(staleEvaluator.has_value());
    require(retainedRequest.has_value());

    const auto afterDestruction =
        staleEvaluator->evaluate(*retainedRequest);
    require(afterDestruction.has_value());
    require(std::holds_alternative<
        ProductionTransitionLiveValidityRejection>(*afterDestruction));
    require(std::get<ProductionTransitionLiveValidityRejection>(
        *afterDestruction).primaryReason() ==
            TransitionLiveValidityReason::SnapshotUnavailable);

    detail::resetLiveValidityOpaqueIdGeneratorForTesting();
    return 0;
}
