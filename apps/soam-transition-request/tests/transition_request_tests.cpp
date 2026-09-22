#include "production_transition_request.hpp"
#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"
#include "detail/production_transition_request_internal.hpp"

#include <array>
#include <cstdlib>
#include <type_traits>
#include <variant>

using namespace AdaptiveMesh;
using RequestAccess =
    AdaptiveMesh::detail::ProductionTransitionRequestDerivationAccess;

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

    ProductionBridgePolicyEvidenceEvaluator d9a;
    const auto evidence = d9a.evaluate(
        std::get<VersionedProductionInterpretation>(*interpreted));
    require(evidence.has_value());
    require(std::holds_alternative<ProductionBridgePolicyEvidence>(*evidence));

    return std::get<ProductionBridgePolicyEvidence>(*evidence);
}

struct RecommendationFixture final {
    ProductionPersistentBridgeRecommendation preserve;
    ProductionPersistentBridgeRecommendation directional;
};

RecommendationFixture makeRecommendationFixture(
    double targetZ,
    PersistentBridgeRecommendation expectedDirectional)
{
    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0, {0.0,0.0,0.0}, 1.0);
    mesh.addNode(1, {0.0,0.0,targetZ}, 1.0);
    mesh.connectNodes(0,1);

    RetainedSourceEvidenceStore store;
    const auto seed = makeEvidence(mesh, store, {0,1});

    const auto persistencePolicy =
        ProductionPersistencePolicyProvider::createCurrent();
    require(persistencePolicy.has_value());

    const auto registry = mesh.productionBridgePersistenceRegistry();
    const auto stream = registry.openLiveStream(seed, *persistencePolicy);
    require(stream.has_value());

    mesh.injectExternalShock(0,0.1);
    const auto firstEvidence = makeEvidence(mesh, store, {0,1});
    const auto firstResult = stream->observe(firstEvidence);
    require(firstResult.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*firstResult));

    const auto preserve =
        std::get<ProductionPersistentBridgeRecommendation>(*firstResult);
    require(preserve.recommendation() ==
        PersistentBridgeRecommendation::PRESERVE);

    mesh.injectExternalShock(0,0.1);
    const auto secondEvidence = makeEvidence(mesh, store, {0,1});
    const auto secondResult = stream->observe(secondEvidence);
    require(secondResult.has_value());
    require(std::holds_alternative<
        ProductionPersistentBridgeRecommendation>(*secondResult));

    const auto directional =
        std::get<ProductionPersistentBridgeRecommendation>(*secondResult);
    require(directional.recommendation() == expectedDirectional);

    return {preserve,directional};
}

static_assert(!std::is_default_constructible_v<TransitionRequestDecisionId>);
static_assert(!std::is_constructible_v<
    TransitionRequestDecisionId, TransitionRequestDecisionId::Bytes>);
static_assert(!std::is_default_constructible_v<TransitionRequestPolicyId>);
static_assert(!std::is_default_constructible_v<TransitionRequestPolicySnapshotId>);
static_assert(!std::is_default_constructible_v<TransitionRequestPolicySnapshot>);
static_assert(!std::is_default_constructible_v<ProductionDerivedTransitionRequest>);
static_assert(!std::is_default_constructible_v<
    ProductionTransitionRequestDerivationRejection>);

void verifyDirectional(
    const ProductionPersistentBridgeRecommendation& recommendation,
    RequestedTransitionDirection expectedDirection,
    const TransitionRequestPolicySnapshot& policy)
{
    ProductionTransitionRequestDeriver deriver;

    const auto first = deriver.derive(recommendation, policy);
    require(first.has_value());
    require(std::holds_alternative<ProductionDerivedTransitionRequest>(*first));

    const auto& derived =
        std::get<ProductionDerivedTransitionRequest>(*first);

    require(nonZero(derived.decisionId().bytes()));
    require(derived.binding().direction() == expectedDirection);
    require(derived.policySnapshotId() == policy.snapshotId());
    require(derived.policyDescriptor().policyId ==
        policy.descriptor().policyId);
    require(derived.policyDescriptor().majorVersion ==
        policy.descriptor().majorVersion);
    require(derived.policyDescriptor().minorVersion ==
        policy.descriptor().minorVersion);
    require(derived.policyDescriptor().implementationRevision ==
        policy.descriptor().implementationRevision);
    require(derived.persistenceObservationDecisionId() ==
        recommendation.decisionId());
    require(derived.persistenceStreamInstanceId() ==
        recommendation.streamInstanceId());
    require(derived.persistenceStreamKey() ==
        recommendation.streamKey());
    require(derived.policyEvidenceDecisionId() ==
        recommendation.policyEvidenceDecisionId());
    require(derived.sourceCaptureId() == recommendation.sourceCaptureId());
    require(derived.sourceRecommendation() ==
        recommendation.recommendation());
    require(derived.stateVersion() == recommendation.stateVersion());

    const auto expectedBinding = RequestAccess::binding(
        RequestAccess::relationship(
            recommendation.streamKey().sourceNodeId(),
            recommendation.streamKey().targetNodeId(),
            recommendation.streamKey().relationshipGeneration()),
        expectedDirection,
        RequestAccess::bridgeCouplingAdjustmentV1(),
        RequestAccess::stateVersion(recommendation.stateVersion()));

    require(derived.binding() == expectedBinding);

    const auto second = deriver.derive(recommendation, policy);
    require(second.has_value());
    require(std::holds_alternative<ProductionDerivedTransitionRequest>(*second));
    const auto& repeated =
        std::get<ProductionDerivedTransitionRequest>(*second);
    require(repeated.binding() == derived.binding());
    require(repeated.decisionId() != derived.decisionId());
}

int main() {
    detail::resetTransitionRequestOpaqueIdGeneratorForTesting();

    const auto policyA =
        ProductionTransitionRequestPolicyProvider::createCurrent();
    const auto policyB =
        ProductionTransitionRequestPolicyProvider::createCurrent();
    require(policyA.has_value() && policyB.has_value());
    require(policyA->snapshotId() != policyB->snapshotId());
    require(policyA->descriptor().policyId ==
        policyB->descriptor().policyId);
    require(policyA->descriptor().majorVersion == 1U);
    require(policyA->descriptor().minorVersion == 0U);
    require(policyA->descriptor().implementationRevisionKind == 1U);
    require(nonZero(policyA->descriptor().implementationRevision));
    require(policyA->bridgeCouplingAdjustmentClass() ==
        RequestAccess::bridgeCouplingAdjustmentV1());

    const auto support = makeRecommendationFixture(
        1.0, PersistentBridgeRecommendation::SUPPORT);
    verifyDirectional(
        support.directional,
        RequestedTransitionDirection::support,
        *policyA);

    // A first accepted D9 sample is still PRESERVE and must not become a request.
    ProductionTransitionRequestDeriver deriver;
    const auto preserveResult = deriver.derive(support.preserve, *policyA);
    require(preserveResult.has_value());
    require(std::holds_alternative<
        ProductionTransitionRequestDerivationRejection>(*preserveResult));
    const auto& preserve =
        std::get<ProductionTransitionRequestDerivationRejection>(
            *preserveResult);
    require(preserve.primaryReason() ==
        TransitionRequestDerivationReason::PreserveRecommendation);
    require(nonZero(preserve.decisionId().bytes()));
    require(preserve.persistenceObservationDecisionId() ==
        support.preserve.decisionId());

    const auto constrain = makeRecommendationFixture(
        100.0, PersistentBridgeRecommendation::CONSTRAIN);
    verifyDirectional(
        constrain.directional,
        RequestedTransitionDirection::constrain,
        *policyB);

    detail::resetTransitionRequestOpaqueIdGeneratorForTesting();
    return 0;
}
