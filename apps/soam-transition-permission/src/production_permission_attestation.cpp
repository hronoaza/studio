#include "production_permission_attestation.hpp"

#include <sodium.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace AdaptiveMesh {
namespace {

constexpr std::array<std::uint8_t,31> kDomainSeparator{
    0x53,0x4f,0x41,0x4d,0x3a,0x50,0x45,0x52,
    0x4d,0x49,0x53,0x53,0x49,0x4f,0x4e,0x2d,
    0x41,0x54,0x54,0x45,0x53,0x54,0x41,0x54,
    0x49,0x4f,0x4e,0x3a,0x56,0x31,0x00
};

template<std::size_t N>
[[nodiscard]] bool allZero(const std::array<std::uint8_t,N>& value) noexcept {
    return std::all_of(value.begin(),value.end(),[](std::uint8_t byte) {
        return byte==0U;
    });
}

[[nodiscard]] std::uint16_t readU16Le(
    std::span<const std::uint8_t> bytes,
    std::size_t offset) noexcept
{
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[offset]) |
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(bytes[offset+1U])<<8U));
}

[[nodiscard]] std::uint64_t readU64Le(
    std::span<const std::uint8_t> bytes,
    std::size_t offset) noexcept
{
    std::uint64_t result=0U;
    for (std::size_t i=0; i<8U; ++i) {
        result |= static_cast<std::uint64_t>(bytes[offset+i])<<(i*8U);
    }
    return result;
}

template<std::size_t N>
[[nodiscard]] std::array<std::uint8_t,N> copyArray(
    std::span<const std::uint8_t> bytes,
    std::size_t offset) noexcept
{
    std::array<std::uint8_t,N> out{};
    std::copy_n(bytes.begin()+static_cast<std::ptrdiff_t>(offset),N,out.begin());
    return out;
}

} // namespace

PermissionAttestationPayloadV1::PermissionAttestationPayloadV1(
    Bytes bytes,
    std::uint16_t formatMajor,
    std::uint16_t formatMinor,
    Id attestationId,
    Id issuerId,
    Id requestDecisionId,
    std::uint64_t sourceNodeId,
    std::uint64_t targetNodeId,
    std::uint64_t relationshipGeneration,
    PermissionAttestationDirection direction,
    std::uint64_t transitionClass,
    std::uint64_t stateVersion,
    PermissionAttestationDecision decision,
    Id permissionPolicyId,
    std::uint16_t permissionPolicyMajor,
    std::uint16_t permissionPolicyMinor) noexcept
    : bytes_(bytes),
      formatMajor_(formatMajor),
      formatMinor_(formatMinor),
      attestationId_(attestationId),
      issuerId_(issuerId),
      requestDecisionId_(requestDecisionId),
      sourceNodeId_(sourceNodeId),
      targetNodeId_(targetNodeId),
      relationshipGeneration_(relationshipGeneration),
      direction_(direction),
      transitionClass_(transitionClass),
      stateVersion_(stateVersion),
      decision_(decision),
      permissionPolicyId_(permissionPolicyId),
      permissionPolicyMajor_(permissionPolicyMajor),
      permissionPolicyMinor_(permissionPolicyMinor)
{
}

std::optional<PermissionAttestationPayloadV1>
parsePermissionAttestationV1(
    std::span<const std::uint8_t> input) noexcept
{
    if (input.size()!=kPermissionAttestationV1PayloadSize) return std::nullopt;
    if (!std::equal(kDomainSeparator.begin(),kDomainSeparator.end(),input.begin())) {
        return std::nullopt;
    }

    const auto formatMajor=readU16Le(input,31U);
    const auto formatMinor=readU16Le(input,33U);
    if (formatMajor!=1U || formatMinor!=0U) return std::nullopt;

    const auto attestationId=copyArray<16U>(input,35U);
    const auto issuerId=copyArray<16U>(input,51U);
    const auto requestDecisionId=copyArray<16U>(input,67U);
    const auto permissionPolicyId=copyArray<16U>(input,125U);

    if (allZero(attestationId) ||
        allZero(issuerId) ||
        allZero(requestDecisionId) ||
        allZero(permissionPolicyId)) {
        return std::nullopt;
    }

    const auto rawDirection=input[107U];
    if (rawDirection>1U) return std::nullopt;

    const auto rawDecision=input[124U];
    if (rawDecision>1U) return std::nullopt;

    const auto bytes=copyArray<kPermissionAttestationV1PayloadSize>(input,0U);

    return PermissionAttestationPayloadV1{
        bytes,
        formatMajor,
        formatMinor,
        attestationId,
        issuerId,
        requestDecisionId,
        readU64Le(input,83U),
        readU64Le(input,91U),
        readU64Le(input,99U),
        static_cast<PermissionAttestationDirection>(rawDirection),
        readU64Le(input,108U),
        readU64Le(input,116U),
        static_cast<PermissionAttestationDecision>(rawDecision),
        permissionPolicyId,
        readU16Le(input,141U),
        readU16Le(input,143U)
    };
}

std::optional<Ed25519PublicKey>
parseEd25519PublicKey(
    std::span<const std::uint8_t> input) noexcept
{
    if (input.size()!=kEd25519PublicKeySize) return std::nullopt;
    return Ed25519PublicKey{copyArray<kEd25519PublicKeySize>(input,0U)};
}

std::optional<Ed25519Signature>
parseEd25519Signature(
    std::span<const std::uint8_t> input) noexcept
{
    if (input.size()!=kEd25519SignatureSize) return std::nullopt;
    return Ed25519Signature{copyArray<kEd25519SignatureSize>(input,0U)};
}

bool verifyEd25519(
    const Ed25519PublicKey& publicKey,
    const Ed25519Signature& signature,
    const PermissionAttestationPayloadV1& payload) noexcept
{
    static const bool sodiumReady=[]() noexcept {
        return sodium_init()>=0;
    }();

    if (!sodiumReady) return false;

    return crypto_sign_verify_detached(
        signature.bytes().data(),
        payload.bytes().data(),
        static_cast<unsigned long long>(payload.bytes().size()),
        publicKey.bytes().data())==0;
}

} // namespace AdaptiveMesh
