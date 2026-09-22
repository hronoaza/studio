#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace AdaptiveMesh {

inline constexpr std::size_t kPermissionAttestationV1PayloadSize = 145U;
inline constexpr std::size_t kEd25519PublicKeySize = 32U;
inline constexpr std::size_t kEd25519SignatureSize = 64U;

enum class PermissionAttestationDirection : std::uint8_t {
    constrain = 0,
    support = 1
};

enum class PermissionAttestationDecision : std::uint8_t {
    deny = 0,
    permit = 1
};

class PermissionAttestationPayloadV1 final {
public:
    using Bytes = std::array<std::uint8_t,kPermissionAttestationV1PayloadSize>;
    using Id = std::array<std::uint8_t,16>;

    PermissionAttestationPayloadV1(
        const PermissionAttestationPayloadV1&) noexcept = default;
    PermissionAttestationPayloadV1& operator=(
        const PermissionAttestationPayloadV1&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    [[nodiscard]] std::uint16_t formatMajor() const noexcept { return formatMajor_; }
    [[nodiscard]] std::uint16_t formatMinor() const noexcept { return formatMinor_; }
    [[nodiscard]] const Id& attestationId() const noexcept { return attestationId_; }
    [[nodiscard]] const Id& issuerId() const noexcept { return issuerId_; }
    [[nodiscard]] const Id& requestDecisionId() const noexcept { return requestDecisionId_; }
    [[nodiscard]] std::uint64_t sourceNodeId() const noexcept { return sourceNodeId_; }
    [[nodiscard]] std::uint64_t targetNodeId() const noexcept { return targetNodeId_; }
    [[nodiscard]] std::uint64_t relationshipGeneration() const noexcept {
        return relationshipGeneration_;
    }
    [[nodiscard]] PermissionAttestationDirection direction() const noexcept {
        return direction_;
    }
    [[nodiscard]] std::uint64_t transitionClass() const noexcept {
        return transitionClass_;
    }
    [[nodiscard]] std::uint64_t stateVersion() const noexcept { return stateVersion_; }
    [[nodiscard]] PermissionAttestationDecision decision() const noexcept {
        return decision_;
    }
    [[nodiscard]] const Id& permissionPolicyId() const noexcept {
        return permissionPolicyId_;
    }
    [[nodiscard]] std::uint16_t permissionPolicyMajor() const noexcept {
        return permissionPolicyMajor_;
    }
    [[nodiscard]] std::uint16_t permissionPolicyMinor() const noexcept {
        return permissionPolicyMinor_;
    }

private:
    PermissionAttestationPayloadV1(
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
        std::uint16_t permissionPolicyMinor) noexcept;

    Bytes bytes_;
    std::uint16_t formatMajor_;
    std::uint16_t formatMinor_;
    Id attestationId_;
    Id issuerId_;
    Id requestDecisionId_;
    std::uint64_t sourceNodeId_;
    std::uint64_t targetNodeId_;
    std::uint64_t relationshipGeneration_;
    PermissionAttestationDirection direction_;
    std::uint64_t transitionClass_;
    std::uint64_t stateVersion_;
    PermissionAttestationDecision decision_;
    Id permissionPolicyId_;
    std::uint16_t permissionPolicyMajor_;
    std::uint16_t permissionPolicyMinor_;

    friend std::optional<PermissionAttestationPayloadV1>
    parsePermissionAttestationV1(std::span<const std::uint8_t>) noexcept;
};

class Ed25519PublicKey final {
public:
    using Bytes = std::array<std::uint8_t,kEd25519PublicKeySize>;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
private:
    explicit Ed25519PublicKey(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend std::optional<Ed25519PublicKey>
    parseEd25519PublicKey(std::span<const std::uint8_t>) noexcept;
};

class Ed25519Signature final {
public:
    using Bytes = std::array<std::uint8_t,kEd25519SignatureSize>;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
private:
    explicit Ed25519Signature(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend std::optional<Ed25519Signature>
    parseEd25519Signature(std::span<const std::uint8_t>) noexcept;
};

[[nodiscard]]
std::optional<PermissionAttestationPayloadV1>
parsePermissionAttestationV1(
    std::span<const std::uint8_t> input) noexcept;

[[nodiscard]]
std::optional<Ed25519PublicKey>
parseEd25519PublicKey(
    std::span<const std::uint8_t> input) noexcept;

[[nodiscard]]
std::optional<Ed25519Signature>
parseEd25519Signature(
    std::span<const std::uint8_t> input) noexcept;

[[nodiscard]]
bool verifyEd25519(
    const Ed25519PublicKey& publicKey,
    const Ed25519Signature& signature,
    const PermissionAttestationPayloadV1& payload) noexcept;

} // namespace AdaptiveMesh
