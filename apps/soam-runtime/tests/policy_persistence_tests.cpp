#include "policy_persistence.hpp"
#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"
#include "detail/policy_persistence_internal.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
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

VersionedProductionInterpretation makeInterpretation(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store,
    const ProductionRelationshipLocator& locator)
{
    const auto snapshot =
        mesh.captureProductionRelationshipSourceSnapshot(locator);
    require(snapshot.has_value());
    require(store.retain(*snapshot) ==
        SourceEvidenceRetentionResult::INSERTED);

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

    return std::get<VersionedProductionInterpretation>(*interpreted);
}

static_assert(!std::is_default_constructible_v<PolicyEvidenceDecisionId>);
static_assert(!std::is_constructible_v<
    PolicyEvidenceDecisionId, PolicyEvidenceDecisionId::Bytes>);
static_assert(!std::is_default_constructible_v<ProductionBridgePolicyEvidence>);
static_assert(!std::is_default_constructible_v<PersistenceProfileId>);
static_assert(!std::is_default_constructible_v<PersistencePolicySnapshotId>);
static_assert(!std::is_default_constructible_v<PersistencePolicySnapshot>);

int main() {
    detail::resetD9OpaqueIdGeneratorForTesting();

    const auto policyA = ProductionPersistencePolicyProvider::createCurrent();
    const auto policyB = ProductionPersistencePolicyProvider::createCurrent();
    require(policyA.has_value() && policyB.has_value());
    require(policyA->snapshotId() != policyB->snapshotId());
    require(nonZero(policyA->snapshotId().bytes()));
    require(nonZero(policyA->descriptor().profileId.bytes()));
    require(nonZero(policyA->descriptor().implementationRevision));
    require(policyA->descriptor().majorVersion == 1U);
    require(policyA->descriptor().minorVersion == 0U);
    require(policyA->descriptor().implementationRevisionKind == 1U);
    require(policyA->activationThreshold() == 0.50);
    require(policyA->releaseThreshold() == 0.25);
    require(policyA->activationSamples() == 2U);
    require(policyA->releaseSamples() == 2U);

    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0, {0.0,0.0,0.0}, 1.0);
    mesh.addNode(1, {0.0,0.0,1.0}, 1.0);
    mesh.connectNodes(0,1);

    RetainedSourceEvidenceStore store;
    const auto interpretation = makeInterpretation(mesh, store, {0,1});

    ProductionBridgePolicyEvidenceEvaluator evaluator;
    const auto first = evaluator.evaluate(interpretation);
    require(first.has_value());
    require(std::holds_alternative<ProductionBridgePolicyEvidence>(*first));

    const auto& evidence =
        std::get<ProductionBridgePolicyEvidence>(*first);
    require(nonZero(evidence.decisionId().bytes()));
    require(evidence.sourceCaptureId() ==
        interpretation.admissibleProvenance().envelope().sourceCaptureId());
    require(evidence.provenanceItemId() ==
        interpretation.admissibleProvenance().envelope().provenanceItemId());
    require(evidence.admissibilityDecisionId() ==
        interpretation.admissibleProvenance().decisionId());
    require(evidence.interpretationDecisionId() ==
        interpretation.decisionId());
    require(evidence.sourceNodeId() == 0U);
    require(evidence.targetNodeId() == 1U);
    require(evidence.relationshipGeneration() ==
        interpretation.admissibleProvenance().envelope().relationshipGeneration());
    require(evidence.stateVersion() ==
        interpretation.admissibleProvenance().envelope().stateVersion());

    AdaptiveBridgePolicy acceptedPolicy;
    const auto expected = acceptedPolicy.evaluate(
        interpretation.observation(), interpretation.confidence());
    require(evidence.evidence().value() == expected.value());

    const auto repeated = evaluator.evaluate(interpretation);
    require(repeated.has_value());
    require(std::holds_alternative<ProductionBridgePolicyEvidence>(*repeated));
    const auto& repeatedEvidence =
        std::get<ProductionBridgePolicyEvidence>(*repeated);
    require(repeatedEvidence.decisionId() != evidence.decisionId());
    require(repeatedEvidence.sourceCaptureId() == evidence.sourceCaptureId());
    require(repeatedEvidence.evidence().value() == evidence.evidence().value());

    detail::resetD9OpaqueIdGeneratorForTesting();
    return 0;
}
