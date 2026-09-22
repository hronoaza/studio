#include "versioned_interpretation.hpp"
#include "system_architecture.hpp"
#include "retained_source_evidence.hpp"
#include "detail/versioned_interpretation_internal.hpp"

#include <array>
#include <atomic>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <variant>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if(!condition) fail(); }

template<std::size_t N>
bool nonZero(const std::array<std::uint8_t,N>& bytes) {
    for(const auto byte:bytes) {
        if(byte!=0U) return true;
    }
    return false;
}

void requireBits(double value,std::uint64_t expected) {
    require(std::bit_cast<std::uint64_t>(value)==expected);
}

AdmissibleProductionProvenance makeAdmissible(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store,
    const ProductionRelationshipLocator& locator)
{
    const auto snapshot=
        mesh.captureProductionRelationshipSourceSnapshot(locator);
    require(snapshot.has_value());
    require(store.retain(*snapshot)==
        SourceEvidenceRetentionResult::INSERTED);

    ProductionProvenanceEnvelopeProducer producer;
    const auto envelope=producer.produce(*snapshot);
    require(envelope.has_value());

    const auto policy=
        ProductionProvenanceAdmissibilityPolicyProvider::createCurrent();
    require(policy.has_value());

    ProvenanceAdmissibilityEvaluator evaluator;
    const auto decision=evaluator.evaluate(*envelope,*policy,store);
    require(decision.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(
        *decision));
    return std::get<AdmissibleProductionProvenance>(*decision);
}

namespace {
std::atomic<int> fillCalls{0};

bool hardFail(std::array<std::uint8_t,16>&) noexcept {
    ++fillCalls;
    return false;
}
}

static_assert(!std::is_default_constructible_v<InterpretationPolicyId>);
static_assert(!std::is_constructible_v<
    InterpretationPolicyId,InterpretationPolicyId::Bytes>);
static_assert(!std::is_default_constructible_v<
    InterpretationPolicySnapshotId>);
static_assert(!std::is_default_constructible_v<InterpretationDecisionId>);
static_assert(!std::is_constructible_v<
    InterpretationDecisionId,InterpretationDecisionId::Bytes>);
static_assert(!std::is_default_constructible_v<
    VersionedProductionInterpretation>);
static_assert(!std::is_default_constructible_v<
    InterpretationPolicySnapshot>);

int main() {
    detail::resetD8DOpaqueIdGeneratorForTesting();

    // Normative vector V1.
    {
        const auto c=detail::computeInterpretationV1(
            0.0,1.0,1.0,1.0,1.0,0.1);
        require(c.has_value());
        requireBits(c->attenuation,0x3ff0000000000000ULL);
        requireBits(c->compatibility,0x3ff0000000000000ULL);
        requireBits(c->confidence,0x3ff0000000000000ULL);
    }

    // V2: zero orientation.
    {
        const auto c=detail::computeInterpretationV1(
            5.0,0.0,1.0,1.0,1.0,0.1);
        require(c.has_value());
        requireBits(c->compatibility,0x0000000000000000ULL);
        requireBits(c->confidence,0x3ff0000000000000ULL);
    }

    // V3: exact neutral coupling.
    {
        const auto c=detail::computeInterpretationV1(
            10.0,1.0,1.0,1.0,1.0,0.1);
        require(c.has_value());
        requireBits(c->attenuation,0x3fe0000000000000ULL);
        requireBits(c->compatibility,0x3fe0000000000000ULL);
        requireBits(c->confidence,0x3ff0000000000000ULL);
    }

    // V4: zero-confidence suppression.
    {
        const auto c=detail::computeInterpretationV1(
            0.0,1.0,1.0,0.0,1.0,0.1);
        require(c.has_value());
        requireBits(c->compatibility,0x3ff0000000000000ULL);
        requireBits(c->confidence,0x0000000000000000ULL);
    }

    // V5: directional forward fixture.
    {
        const auto c=detail::computeInterpretationV1(
            1.0,1.0,0.8,0.9,0.7,0.1);
        require(c.has_value());
        requireBits(c->compatibility,0x3fe745d1745d1746ULL);
        requireBits(c->confidence,0x3fe6666666666666ULL);
    }

    // Invalid-domain vectors reject instead of clamping.
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    require(!detail::computeInterpretationV1(-1.0,1.0,1.0,1.0,1.0,0.1));
    require(!detail::computeInterpretationV1(0.0,-0.1,1.0,1.0,1.0,0.1));
    require(!detail::computeInterpretationV1(0.0,1.1,1.0,1.0,1.0,0.1));
    require(!detail::computeInterpretationV1(0.0,1.0,-0.1,1.0,1.0,0.1));
    require(!detail::computeInterpretationV1(0.0,1.0,1.1,1.0,1.0,0.1));
    require(!detail::computeInterpretationV1(0.0,1.0,1.0,-0.1,1.0,0.1));
    require(!detail::computeInterpretationV1(0.0,1.0,1.0,1.0,1.1,0.1));
    require(!detail::computeInterpretationV1(nan,1.0,1.0,1.0,1.0,0.1));
    require(!detail::computeInterpretationV1(inf,1.0,1.0,1.0,1.0,0.1));

    const auto policyA=ProductionInterpretationPolicyProvider::createCurrent();
    const auto policyB=ProductionInterpretationPolicyProvider::createCurrent();
    require(policyA.has_value()&&policyB.has_value());
    require(policyA->snapshotId()!=policyB->snapshotId());
    require(policyA->descriptor().policyId.bytes()==
        InterpretationPolicyId::Bytes{
            0x63,0xde,0x65,0x6b,0xeb,0x62,0x57,0xe5,
            0x36,0xea,0x0d,0x52,0xf4,0x21,0x43,0xf8});
    require(policyA->descriptor().majorVersion==1U);
    require(policyA->descriptor().minorVersion==0U);
    require(policyA->descriptor().implementationRevisionKind==1U);
    require(nonZero(policyA->descriptor().implementationRevision));
    require(policyA->distanceAttenuationCoefficient()==0.1);
    require(policyA->directional());

    // End-to-end forward full-coupling path.
    SpatialAdaptiveMesh forwardMesh(2);
    forwardMesh.addNode(0,{0.0,0.0,0.0},1.0);
    forwardMesh.addNode(1,{0.0,0.0,1.0},1.0);
    forwardMesh.connectNodes(0,1);
    RetainedSourceEvidenceStore forwardStore;
    const auto admittedForward=
        makeAdmissible(forwardMesh,forwardStore,{0,1});

    VersionedProductionInterpreter interpreter;
    const auto interpreted=interpreter.interpret(
        admittedForward,*policyA);
    require(interpreted.has_value());
    require(std::holds_alternative<VersionedProductionInterpretation>(
        *interpreted));

    const auto& success=
        std::get<VersionedProductionInterpretation>(*interpreted);
    require(nonZero(success.decisionId().bytes()));
    require(success.interpretationPolicySnapshotId()==policyA->snapshotId());
    require(success.admissibleProvenance().decisionId()==
        admittedForward.decisionId());
    require(success.admissibleProvenance().policySnapshotId()==
        admittedForward.policySnapshotId());
    require(success.admissibleProvenance().envelope().sourceCaptureId()==
        admittedForward.envelope().sourceCaptureId());
    require(success.admissibleProvenance().envelope().provenanceItemId()==
        admittedForward.envelope().provenanceItemId());
    requireBits(success.observation().compatibility(),
        0x3fed1745d1745d17ULL); // d=1, orientation=1, capacity=1 -> 1/1.1
    requireBits(success.confidence().value(),
        0x3ff0000000000000ULL);

    const auto repeated=interpreter.interpret(
        admittedForward,*policyA);
    require(repeated.has_value());
    const auto& repeatedSuccess=
        std::get<VersionedProductionInterpretation>(*repeated);
    require(repeatedSuccess.decisionId()!=success.decisionId());
    require(repeatedSuccess.observation().compatibility()==
        success.observation().compatibility());
    require(repeatedSuccess.confidence().value()==
        success.confidence().value());

    // Reverse direction intentionally differs because orientationWeight differs.
    RetainedSourceEvidenceStore reverseStore;
    const auto admittedReverse=
        makeAdmissible(forwardMesh,reverseStore,{1,0});
    const auto reversed=interpreter.interpret(
        admittedReverse,*policyA);
    require(reversed.has_value());
    const auto& reverseSuccess=
        std::get<VersionedProductionInterpretation>(*reversed);
    requireBits(reverseSuccess.observation().compatibility(),
        0x0000000000000000ULL);
    require(reverseSuccess.confidence().value()==1.0);

    // Decision-ID generation failure is infrastructure failure, not rejection.
    detail::resetD8DOpaqueIdGeneratorForTesting();
    fillCalls.store(0);
    detail::setD8DOpaqueIdFillFunctionForTesting(&hardFail);
    const auto noDecision=interpreter.interpret(
        admittedForward,*policyA);
    require(!noDecision.has_value());
    require(fillCalls.load()==1);

    detail::resetD8DOpaqueIdGeneratorForTesting();
    return 0;
}
