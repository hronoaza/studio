#include "production_transition_resilience.hpp"
#include "detail/production_transition_resilience_internal.hpp"
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
    for (const auto byte:bytes) if (byte!=0U) return true;
    return false;
}

ProductionBridgePolicyEvidence makeEvidence(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store)
{
    const auto snapshot=mesh.captureProductionRelationshipSourceSnapshot({0,1});
    require(snapshot.has_value());

    const auto retention=store.retain(*snapshot);
    require(
        retention==SourceEvidenceRetentionResult::INSERTED ||
        retention==SourceEvidenceRetentionResult::ALREADY_RETAINED);

    ProductionProvenanceEnvelopeProducer producer;
    const auto envelope=producer.produce(*snapshot);
    require(envelope.has_value());

    const auto admissibilityPolicy=
        ProductionProvenanceAdmissibilityPolicyProvider::createCurrent();
    require(admissibilityPolicy.has_value());

    ProvenanceAdmissibilityEvaluator admissibility;
    const auto admitted=
        admissibility.evaluate(*envelope,*admissibilityPolicy,store);
    require(admitted.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(*admitted));

    const auto interpretationPolicy=
        ProductionInterpretationPolicyProvider::createCurrent();
    require(interpretationPolicy.has_value());

    VersionedProductionInterpreter interpreter;
    const auto interpreted=interpreter.interpret(
        std::get<AdmissibleProductionProvenance>(*admitted),
        *interpretationPolicy);
    require(interpreted.has_value());
    require(std::holds_alternative<VersionedProductionInterpretation>(*interpreted));

    ProductionBridgePolicyEvidenceEvaluator d9a;
    const auto evidence=d9a.evaluate(
        std::get<VersionedProductionInterpretation>(*interpreted));
    require(evidence.has_value());
    require(std::holds_alternative<ProductionBridgePolicyEvidence>(*evidence));

    return std::get<ProductionBridgePolicyEvidence>(*evidence);
}

ProductionDerivedTransitionRequest makeSupportRequest(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store)
{
    const auto seed=makeEvidence(mesh,store);
    const auto persistencePolicy=
        ProductionPersistencePolicyProvider::createCurrent();
    require(persistencePolicy.has_value());

    const auto registry=mesh.productionBridgePersistenceRegistry();
    const auto stream=registry.openLiveStream(seed,*persistencePolicy);
    require(stream.has_value());

    mesh.injectExternalShock(0,0.1);
    const auto first=makeEvidence(mesh,store);
    const auto firstResult=stream->observe(first);
    require(firstResult.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*firstResult));

    mesh.injectExternalShock(0,0.1);
    const auto second=makeEvidence(mesh,store);
    const auto secondResult=stream->observe(second);
    require(secondResult.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*secondResult));

    const auto recommendation=
        std::get<ProductionPersistentBridgeRecommendation>(*secondResult);
    require(recommendation.recommendation()==
        PersistentBridgeRecommendation::SUPPORT);

    const auto requestPolicy=
        ProductionTransitionRequestPolicyProvider::createCurrent();
    require(requestPolicy.has_value());

    ProductionTransitionRequestDeriver deriver;
    const auto derived=deriver.derive(recommendation,*requestPolicy);
    require(derived.has_value());
    require(std::holds_alternative<ProductionDerivedTransitionRequest>(*derived));

    return std::get<ProductionDerivedTransitionRequest>(*derived);
}

static_assert(!std::is_default_constructible_v<ResilienceDecisionId>);
static_assert(!std::is_default_constructible_v<ResiliencePolicyId>);
static_assert(!std::is_default_constructible_v<ResiliencePolicySnapshotId>);
static_assert(!std::is_default_constructible_v<TransitionResiliencePolicySnapshot>);
static_assert(!std::is_default_constructible_v<ProductionResiliencePrerequisiteRecord>);

int main() {
    detail::resetResilienceOpaqueIdGeneratorForTesting();

    {
        SpatialAdaptiveMesh mesh(1);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.addNode(2,{1.0,0.0,0.0},1.0);
        mesh.connectNodePairs({{0,1},{0,2},{2,1}});

        RetainedSourceEvidenceStore store;
        const auto request=makeSupportRequest(mesh,store);
        const auto policy=
            ProductionTransitionResiliencePolicyProvider::createCurrent();
        require(policy.has_value());
        require(policy->criterion()==
            ResilienceCriterion::single_request_relationship_loss_survivability);

        ProductionTransitionResilienceEvaluator evaluator{
            mesh.productionTransitionResilienceSnapshotSource()};

        const auto result=evaluator.evaluate(request,*policy);
        require(result.has_value());
        require(std::holds_alternative<
            ProductionResiliencePrerequisiteRecord>(*result));

        const auto& record=
            std::get<ProductionResiliencePrerequisiteRecord>(*result);
        require(nonZero(record.decisionId().bytes()));
        require(record.binding()==request.binding());
        require(record.policySnapshotId()==policy->snapshotId());
        require(record.satisfied());
        require(record.alternativePathHopCount()==2U);

        const auto repeated=evaluator.evaluate(request,*policy);
        require(repeated.has_value());
        require(std::holds_alternative<
            ProductionResiliencePrerequisiteRecord>(*repeated));
        const auto& repeatedRecord=
            std::get<ProductionResiliencePrerequisiteRecord>(*repeated);
        require(repeatedRecord.satisfied()==record.satisfied());
        require(repeatedRecord.alternativePathHopCount()==
            record.alternativePathHopCount());
        require(repeatedRecord.decisionId()!=record.decisionId());

        mesh.injectExternalShock(0,0.1);
        const auto stale=evaluator.evaluate(request,*policy);
        require(stale.has_value());
        require(std::holds_alternative<
            ProductionTransitionResilienceRejection>(*stale));
        require(std::get<ProductionTransitionResilienceRejection>(
            *stale).primaryReason()==
            TransitionResilienceReason::RequestStateVersionMismatch);
    }

    {
        SpatialAdaptiveMesh mesh(1);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.connectNodes(0,1);

        RetainedSourceEvidenceStore store;
        const auto request=makeSupportRequest(mesh,store);
        const auto policy=
            ProductionTransitionResiliencePolicyProvider::createCurrent();
        require(policy.has_value());

        ProductionTransitionResilienceEvaluator evaluator{
            mesh.productionTransitionResilienceSnapshotSource()};

        const auto result=evaluator.evaluate(request,*policy);
        require(result.has_value());
        require(std::holds_alternative<
            ProductionResiliencePrerequisiteRecord>(*result));
        const auto& record=
            std::get<ProductionResiliencePrerequisiteRecord>(*result);
        require(!record.satisfied());
        require(record.alternativePathHopCount()==0U);
    }

    std::optional<ProductionTransitionResilienceEvaluator> staleEvaluator;
    std::optional<ProductionDerivedTransitionRequest> retainedRequest;
    std::optional<TransitionResiliencePolicySnapshot> retainedPolicy;

    {
        SpatialAdaptiveMesh mesh(1);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.connectNodes(0,1);

        RetainedSourceEvidenceStore store;
        retainedRequest.emplace(makeSupportRequest(mesh,store));
        retainedPolicy=
            ProductionTransitionResiliencePolicyProvider::createCurrent();
        require(retainedPolicy.has_value());

        staleEvaluator.emplace(
            mesh.productionTransitionResilienceSnapshotSource());
    }

    require(staleEvaluator.has_value());
    require(retainedRequest.has_value());
    require(retainedPolicy.has_value());

    const auto afterDestruction=
        staleEvaluator->evaluate(*retainedRequest,*retainedPolicy);
    require(afterDestruction.has_value());
    require(std::holds_alternative<
        ProductionTransitionResilienceRejection>(*afterDestruction));
    require(std::get<ProductionTransitionResilienceRejection>(
        *afterDestruction).primaryReason()==
        TransitionResilienceReason::SnapshotUnavailable);

    detail::resetResilienceOpaqueIdGeneratorForTesting();
    return 0;
}
