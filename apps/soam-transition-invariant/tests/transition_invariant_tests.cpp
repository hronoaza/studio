#include "production_transition_invariant.hpp"
#include "detail/production_transition_invariant_internal.hpp"
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

static_assert(!std::is_default_constructible_v<InvariantDecisionId>);
static_assert(!std::is_default_constructible_v<InvariantPolicyId>);
static_assert(!std::is_default_constructible_v<InvariantPolicySnapshotId>);
static_assert(!std::is_default_constructible_v<TransitionInvariantPolicySnapshot>);
static_assert(!std::is_default_constructible_v<ProductionInvariantPrerequisiteRecord>);

int main() {
    detail::resetInvariantOpaqueIdGeneratorForTesting();

    require(detail::projectInvariantCapacityForTesting(
        0.0,RequestedTransitionDirection::support) == 0.125);
    require(detail::projectInvariantCapacityForTesting(
        0.0,RequestedTransitionDirection::constrain) == 0.0);
    require(detail::projectInvariantCapacityForTesting(
        0.5,RequestedTransitionDirection::support) == 0.5625);
    require(detail::projectInvariantCapacityForTesting(
        0.5,RequestedTransitionDirection::constrain) == 0.4375);
    require(detail::projectInvariantCapacityForTesting(
        1.0,RequestedTransitionDirection::support) == 1.0);
    require(detail::projectInvariantCapacityForTesting(
        1.0,RequestedTransitionDirection::constrain) == 0.875);

    std::optional<ProductionTransitionInvariantEvaluator> staleEvaluator;
    std::optional<ProductionDerivedTransitionRequest> retainedRequest;
    std::optional<TransitionInvariantPolicySnapshot> retainedPolicy;

    {
        SpatialAdaptiveMesh mesh(1);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.connectNodes(0,1);

        RetainedSourceEvidenceStore store;
        const auto request = makeSupportRequest(mesh,store);
        retainedRequest.emplace(request);

        const auto policy =
            ProductionTransitionInvariantPolicyProvider::createCurrent();
        require(policy.has_value());
        require(policy->adjustmentFraction() == 0.125);
        retainedPolicy.emplace(*policy);

        ProductionTransitionInvariantEvaluator evaluator{
            mesh.productionTransitionInvariantSnapshotSource()};
        staleEvaluator.emplace(evaluator);

        const auto result = evaluator.evaluate(request,*policy);
        require(result.has_value());
        require(std::holds_alternative<
            ProductionInvariantPrerequisiteRecord>(*result));

        const auto& record =
            std::get<ProductionInvariantPrerequisiteRecord>(*result);
        require(nonZero(record.decisionId().bytes()));
        require(record.binding() == request.binding());
        require(record.policySnapshotId() == policy->snapshotId());
        require(record.satisfied());
        require(record.sourceIdentityInvariantSatisfied());
        require(record.targetIdentityInvariantSatisfied());
        require(record.structuralRelationshipInvariantSatisfied());
        require(record.forwardProjectedCapacity() == 1.0);
        require(record.reverseProjectedCapacity() == 1.0);
        require(record.forwardEffectiveCouplingValid());
        require(record.reverseEffectiveCouplingValid());

        const auto repeated = evaluator.evaluate(request,*policy);
        require(repeated.has_value());
        require(std::holds_alternative<
            ProductionInvariantPrerequisiteRecord>(*repeated));
        const auto& repeatedRecord =
            std::get<ProductionInvariantPrerequisiteRecord>(*repeated);
        require(repeatedRecord.satisfied() == record.satisfied());
        require(repeatedRecord.forwardProjectedCapacity() ==
            record.forwardProjectedCapacity());
        require(repeatedRecord.reverseProjectedCapacity() ==
            record.reverseProjectedCapacity());
        require(repeatedRecord.decisionId() != record.decisionId());

        mesh.injectExternalShock(0,0.1);

        const auto stale = evaluator.evaluate(request,*policy);
        require(stale.has_value());
        require(std::holds_alternative<
            ProductionTransitionInvariantRejection>(*stale));
        require(std::get<ProductionTransitionInvariantRejection>(
            *stale).primaryReason() ==
            TransitionInvariantReason::RequestStateVersionMismatch);
    }

    require(staleEvaluator.has_value());
    require(retainedRequest.has_value());
    require(retainedPolicy.has_value());

    const auto afterDestruction =
        staleEvaluator->evaluate(*retainedRequest,*retainedPolicy);
    require(afterDestruction.has_value());
    require(std::holds_alternative<
        ProductionTransitionInvariantRejection>(*afterDestruction));
    require(std::get<ProductionTransitionInvariantRejection>(
        *afterDestruction).primaryReason() ==
        TransitionInvariantReason::SnapshotUnavailable);

    detail::resetInvariantOpaqueIdGeneratorForTesting();
    return 0;
}
