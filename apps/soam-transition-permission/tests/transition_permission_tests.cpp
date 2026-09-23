#include "production_transition_permission.hpp"
#include "detail/production_transition_permission_internal.hpp"
#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"

#include <sodium.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <type_traits>
#include <variant>
#include <vector>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

template<std::size_t N>
bool nonZero(const std::array<std::uint8_t,N>& bytes) noexcept {
    return std::any_of(bytes.begin(),bytes.end(),[](std::uint8_t value) {
        return value!=0U;
    });
}

ProductionBridgePolicyEvidence makeEvidence(
    SpatialAdaptiveMesh& mesh,
    RetainedSourceEvidenceStore& store)
{
    const auto snapshot=mesh.captureProductionRelationshipSourceSnapshot({0,1});
    require(snapshot.has_value());
    const auto retention=store.retain(*snapshot);
    require(retention==SourceEvidenceRetentionResult::INSERTED ||
            retention==SourceEvidenceRetentionResult::ALREADY_RETAINED);

    ProductionProvenanceEnvelopeProducer producer;
    const auto envelope=producer.produce(*snapshot);
    require(envelope.has_value());

    const auto admissibilityPolicy=
        ProductionProvenanceAdmissibilityPolicyProvider::createCurrent();
    require(admissibilityPolicy.has_value());
    ProvenanceAdmissibilityEvaluator admissibility;
    const auto admitted=admissibility.evaluate(*envelope,*admissibilityPolicy,store);
    require(admitted.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(*admitted));

    const auto interpretationPolicy=ProductionInterpretationPolicyProvider::createCurrent();
    require(interpretationPolicy.has_value());
    VersionedProductionInterpreter interpreter;
    const auto interpreted=interpreter.interpret(
        std::get<AdmissibleProductionProvenance>(*admitted),*interpretationPolicy);
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
    const auto persistencePolicy=ProductionPersistencePolicyProvider::createCurrent();
    require(persistencePolicy.has_value());
    const auto registry=mesh.productionBridgePersistenceRegistry();
    const auto stream=registry.openLiveStream(seed,*persistencePolicy);
    require(stream.has_value());

    mesh.injectExternalShock(0,0.1);
    const auto first=makeEvidence(mesh,store);
    const auto firstResult=stream->observe(first);
    require(firstResult.has_value());
    require(std::holds_alternative<ProductionPersistentBridgeRecommendation>(*firstResult));

    mesh.injectExternalShock(0,0.1);
    const auto second=makeEvidence(mesh,store);
    const auto secondResult=stream->observe(second);
    require(secondResult.has_value());
    require(std::holds_alternative<ProductionPersistentBridgeRecommendation>(*secondResult));
    const auto recommendation=std::get<ProductionPersistentBridgeRecommendation>(*secondResult);
    require(recommendation.recommendation()==PersistentBridgeRecommendation::SUPPORT);

    const auto requestPolicy=ProductionTransitionRequestPolicyProvider::createCurrent();
    require(requestPolicy.has_value());
    ProductionTransitionRequestDeriver deriver;
    const auto derived=deriver.derive(recommendation,*requestPolicy);
    require(derived.has_value());
    require(std::holds_alternative<ProductionDerivedTransitionRequest>(*derived));
    return std::get<ProductionDerivedTransitionRequest>(*derived);
}

void writeU16Le(std::array<std::uint8_t,145>& out,std::size_t offset,std::uint16_t value) {
    out[offset]=static_cast<std::uint8_t>(value&0xffU);
    out[offset+1U]=static_cast<std::uint8_t>((value>>8U)&0xffU);
}

void writeU64Le(std::array<std::uint8_t,145>& out,std::size_t offset,std::uint64_t value) {
    for (std::size_t i=0;i<8U;++i) {
        out[offset+i]=static_cast<std::uint8_t>((value>>(i*8U))&0xffU);
    }
}

std::array<std::uint8_t,145> makePayload(
    const ProductionDerivedTransitionRequest& request,
    PermissionAttestationDecision decision)
{
    std::array<std::uint8_t,145> out{};
    constexpr std::array<std::uint8_t,31> domain{
        0x53,0x4f,0x41,0x4d,0x3a,0x50,0x45,0x52,
        0x4d,0x49,0x53,0x53,0x49,0x4f,0x4e,0x2d,
        0x41,0x54,0x54,0x45,0x53,0x54,0x41,0x54,
        0x49,0x4f,0x4e,0x3a,0x56,0x31,0x00
    };
    std::copy(domain.begin(),domain.end(),out.begin());
    writeU16Le(out,31U,1U);
    writeU16Le(out,33U,0U);
    for (std::size_t i=0;i<16U;++i) {
        out[35U+i]=static_cast<std::uint8_t>(0x10U+i);
        out[51U+i]=static_cast<std::uint8_t>(0x20U+i);
        out[125U+i]=static_cast<std::uint8_t>(0x40U+i);
    }
    std::copy(request.decisionId().bytes().begin(),request.decisionId().bytes().end(),out.begin()+67);
    const auto& relation=request.binding().relationship();
    writeU64Le(out,83U,static_cast<std::uint64_t>(relation.sourceNodeId()));
    writeU64Le(out,91U,static_cast<std::uint64_t>(relation.targetNodeId()));
    writeU64Le(out,99U,relation.generation());
    out[107U]=request.binding().direction()==RequestedTransitionDirection::support ? 1U : 0U;
    writeU64Le(out,108U,0x4252494447455631ULL);
    writeU64Le(out,116U,request.stateVersion());
    out[124U]=decision==PermissionAttestationDecision::permit ? 1U : 0U;
    writeU16Le(out,141U,1U);
    writeU16Le(out,143U,0U);
    return out;
}

std::array<std::uint8_t,64> signPayload(const std::array<std::uint8_t,145>& payload) {
    require(sodium_init()>=0);
    std::array<unsigned char,crypto_sign_SEEDBYTES> seed{};
    for (std::size_t i=0;i<seed.size();++i) seed[i]=static_cast<unsigned char>(i);
    std::array<unsigned char,crypto_sign_PUBLICKEYBYTES> publicKey{};
    std::array<unsigned char,crypto_sign_SECRETKEYBYTES> secretKey{};
    require(crypto_sign_seed_keypair(publicKey.data(),secretKey.data(),seed.data())==0);
    std::array<std::uint8_t,64> signature{};
    unsigned long long signatureLength=0;
    require(crypto_sign_detached(
        signature.data(),&signatureLength,payload.data(),
        static_cast<unsigned long long>(payload.size()),secretKey.data())==0);
    require(signatureLength==signature.size());
    sodium_memzero(secretKey.data(),secretKey.size());
    return signature;
}

static_assert(!std::is_same_v<PermissionDecisionId,TransitionRequestDecisionId>);
static_assert(!std::is_convertible_v<PermissionDecisionId,TransitionRequestDecisionId>);
static_assert(!std::is_convertible_v<TransitionRequestDecisionId,PermissionDecisionId>);
static_assert(!std::is_base_of_v<TransitionRequestDecisionId,PermissionDecisionId>);

static_assert(!std::is_same_v<PermissionPolicySnapshotId,TransitionRequestPolicySnapshotId>);
static_assert(!std::is_convertible_v<PermissionPolicySnapshotId,TransitionRequestPolicySnapshotId>);
static_assert(!std::is_convertible_v<TransitionRequestPolicySnapshotId,PermissionPolicySnapshotId>);
static_assert(!std::is_base_of_v<TransitionRequestPolicySnapshotId,PermissionPolicySnapshotId>);

static_assert(!std::is_default_constructible_v<PermissionDecisionId>);
static_assert(!std::is_constructible_v<PermissionDecisionId,PermissionDecisionId::Bytes>);
static_assert(!std::is_default_constructible_v<PermissionPolicySnapshotId>);
static_assert(!std::is_constructible_v<PermissionPolicySnapshotId,PermissionPolicySnapshotId::Bytes>);
static_assert(!std::is_default_constructible_v<ProductionPermissionPrerequisiteRecord>);
static_assert(!std::is_default_constructible_v<PermissionVerificationPolicySnapshot>);

int main() {
    detail::resetPermissionOpaqueIdGeneratorForTesting();

    SpatialAdaptiveMesh mesh(1);
    mesh.addNode(0,{0.0,0.0,0.0},1.0);
    mesh.addNode(1,{0.0,0.0,1.0},1.0);
    mesh.connectNodes(0,1);
    RetainedSourceEvidenceStore store;
    const auto request=makeSupportRequest(mesh,store);

    const auto policy=ProductionPermissionPolicyProvider::createCurrent();
    require(policy.has_value());
    require(nonZero(policy->snapshotId().bytes()));

    ProductionPermissionVerifier verifier;

    const auto permitPayload=makePayload(request,PermissionAttestationDecision::permit);
    const auto permitSignature=signPayload(permitPayload);
    const auto permit=verifier.evaluate(request,*policy,permitPayload,permitSignature);
    require(permit.has_value());
    require(std::holds_alternative<ProductionPermissionPrerequisiteRecord>(*permit));
    const auto& permitRecord=std::get<ProductionPermissionPrerequisiteRecord>(*permit);
    require(nonZero(permitRecord.decisionId().bytes()));
    require(permitRecord.binding()==request.binding());
    require(permitRecord.policySnapshotId()==policy->snapshotId());
    require(permitRecord.satisfied());

    const auto denyPayload=makePayload(request,PermissionAttestationDecision::deny);
    const auto denySignature=signPayload(denyPayload);
    const auto deny=verifier.evaluate(request,*policy,denyPayload,denySignature);
    require(deny.has_value());
    require(std::holds_alternative<ProductionPermissionPrerequisiteRecord>(*deny));
    require(!std::get<ProductionPermissionPrerequisiteRecord>(*deny).satisfied());

    {
        auto changed=permitSignature;
        changed[0U]^=0x01U;
        const auto result=verifier.evaluate(request,*policy,permitPayload,changed);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::SignatureInvalid);
    }
    {
        auto changed=permitPayload;
        changed[51U]^=0x01U;
        const auto changedSignature=signPayload(changed);
        const auto result=verifier.evaluate(request,*policy,changed,changedSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::IssuerUnrecognized);
    }
    {
        auto changed=permitPayload;
        changed[67U]^=0x01U;
        const auto changedSignature=signPayload(changed);
        const auto result=verifier.evaluate(request,*policy,changed,changedSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::RequestBindingMismatch);
    }
    {
        auto changed=permitPayload;
        changed[83U]^=0x01U;
        const auto changedSignature=signPayload(changed);
        const auto result=verifier.evaluate(request,*policy,changed,changedSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::RequestBindingMismatch);
    }
    {
        auto changed=permitPayload;
        changed[107U]=0U;
        const auto changedSignature=signPayload(changed);
        const auto result=verifier.evaluate(request,*policy,changed,changedSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::RequestBindingMismatch);
    }
    {
        auto changed=permitPayload;
        changed[108U]^=0x01U;
        const auto changedSignature=signPayload(changed);
        const auto result=verifier.evaluate(request,*policy,changed,changedSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::RequestBindingMismatch);
    }
    {
        auto changed=permitPayload;
        changed[116U]^=0x01U;
        const auto changedSignature=signPayload(changed);
        const auto result=verifier.evaluate(request,*policy,changed,changedSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::RequestBindingMismatch);
    }
    {
        auto changed=permitPayload;
        changed[125U]^=0x01U;
        const auto changedSignature=signPayload(changed);
        const auto result=verifier.evaluate(request,*policy,changed,changedSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::PolicyRevisionUnrecognized);
    }
    {
        std::vector<std::uint8_t> truncated(permitPayload.begin(),permitPayload.end()-1);
        const auto result=verifier.evaluate(request,*policy,truncated,permitSignature);
        require(result.has_value());
        require(std::get<ProductionTransitionPermissionRejection>(*result).primaryReason()==
            TransitionPermissionReason::AttestationMalformed);
    }

    detail::resetPermissionOpaqueIdGeneratorForTesting();
    return 0;
}
