#include "production_permission_attestation.hpp"
#include "permission_attestation_v1_vectors.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <span>
#include <vector>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

int main() {
    using namespace AdaptiveMesh::test_fixture;

    const auto payload=parsePermissionAttestationV1(kPermissionPayloadV1);
    const auto publicKey=parseEd25519PublicKey(kEd25519PublicKey);
    const auto signature=parseEd25519Signature(kEd25519Signature);

    require(payload.has_value());
    require(publicKey.has_value());
    require(signature.has_value());

    require(payload->formatMajor()==1U);
    require(payload->formatMinor()==0U);
    require(payload->sourceNodeId()==1U);
    require(payload->targetNodeId()==2U);
    require(payload->relationshipGeneration()==3U);
    require(payload->direction()==PermissionAttestationDirection::support);
    require(payload->transitionClass()==0x4252494447455631ULL);
    require(payload->stateVersion()==4U);
    require(payload->decision()==PermissionAttestationDecision::permit);
    require(payload->permissionPolicyMajor()==1U);
    require(payload->permissionPolicyMinor()==0U);

    require(verifyEd25519(*publicKey,*signature,*payload));

    {
        auto changed=kPermissionPayloadV1;
        changed[116U]^=0x01U;
        const auto parsed=parsePermissionAttestationV1(changed);
        require(parsed.has_value());
        require(!verifyEd25519(*publicKey,*signature,*parsed));
    }

    {
        auto changed=kPermissionPayloadV1;
        changed[107U]=0U;
        const auto parsed=parsePermissionAttestationV1(changed);
        require(parsed.has_value());
        require(!verifyEd25519(*publicKey,*signature,*parsed));
    }

    {
        auto changed=kPermissionPayloadV1;
        changed[51U]^=0x01U;
        const auto parsed=parsePermissionAttestationV1(changed);
        require(parsed.has_value());
        require(!verifyEd25519(*publicKey,*signature,*parsed));
    }

    {
        auto changed=kPermissionPayloadV1;
        changed[0U]^=0x01U;
        require(!parsePermissionAttestationV1(changed).has_value());
    }

    {
        auto changedSignature=kEd25519Signature;
        changedSignature[0U]^=0x01U;
        const auto parsedSignature=parseEd25519Signature(changedSignature);
        require(parsedSignature.has_value());
        require(!verifyEd25519(*publicKey,*parsedSignature,*payload));
    }

    {
        std::vector<std::uint8_t> truncated(
            kPermissionPayloadV1.begin(),kPermissionPayloadV1.end()-1);
        require(!parsePermissionAttestationV1(truncated).has_value());
    }

    {
        std::vector<std::uint8_t> extended(
            kPermissionPayloadV1.begin(),kPermissionPayloadV1.end());
        extended.push_back(0U);
        require(!parsePermissionAttestationV1(extended).has_value());
    }

    {
        std::array<std::uint8_t,31> wrongDomain{};
        std::copy_n(kPermissionPayloadV1.begin(),31U,wrongDomain.begin());
        auto changed=kPermissionPayloadV1;
        changed[30U]=0x01U;
        require(!parsePermissionAttestationV1(changed).has_value());
    }

    {
        std::array<std::uint8_t,31> zeroIdPayloadPrefix{};
        (void)zeroIdPayloadPrefix;
        auto changed=kPermissionPayloadV1;
        std::fill(changed.begin()+35,changed.begin()+51,0U);
        require(!parsePermissionAttestationV1(changed).has_value());
    }

    require(!parseEd25519PublicKey(
        std::span<const std::uint8_t>{kEd25519PublicKey.data(),31U}).has_value());
    require(!parseEd25519Signature(
        std::span<const std::uint8_t>{kEd25519Signature.data(),63U}).has_value());

    return 0;
}
