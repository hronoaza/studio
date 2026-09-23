#pragma once

#include "production_permission_attestation.hpp"
#include "production_transition_request.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>
#include <variant>
#include <vector>

namespace AdaptiveMesh {

class PermissionDecisionId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    PermissionDecisionId(const PermissionDecisionId&) noexcept = default;
    PermissionDecisionId& operator=(const PermissionDecisionId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const PermissionDecisionId&, const PermissionDecisionId&) noexcept = default;
private:
    explicit PermissionDecisionId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionPermissionVerifier;
};

class PermissionAttestationId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    PermissionAttestationId(const PermissionAttestationId&) noexcept = default;
    PermissionAttestationId& operator=(const PermissionAttestationId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const PermissionAttestationId&, const PermissionAttestationId&) noexcept = default;
private:
    explicit PermissionAttestationId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionPermissionVerifier;
};

class PermissionIssuerId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    PermissionIssuerId(const PermissionIssuerId&) noexcept = default;
    PermissionIssuerId& operator=(const PermissionIssuerId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const PermissionIssuerId&, const PermissionIssuerId&) noexcept = default;
private:
    explicit PermissionIssuerId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionPermissionVerifier;
    friend class ProductionPermissionPolicyProvider;
};

class PermissionPolicyId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    PermissionPolicyId(const PermissionPolicyId&) noexcept = default;
    PermissionPolicyId& operator=(const PermissionPolicyId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const PermissionPolicyId&, const PermissionPolicyId&) noexcept = default;
private:
    explicit PermissionPolicyId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionPermissionPolicyProvider;
};

class PermissionPolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    PermissionPolicySnapshotId(const PermissionPolicySnapshotId&) noexcept = default;
    PermissionPolicySnapshotId& operator=(const PermissionPolicySnapshotId&) noexcept = default;
    [[nodiscard]] const Bytes& bytes() const noexcept { return bytes_; }
    friend bool operator==(const PermissionPolicySnapshotId&, const PermissionPolicySnapshotId&) noexcept = default;
private:
    explicit PermissionPolicySnapshotId(Bytes bytes) noexcept : bytes_(bytes) {}
    Bytes bytes_;
    friend class ProductionPermissionPolicyProvider;
};

enum class PermissionSignatureProfile : std::uint8_t { ed25519 = 1 };

struct PermissionPolicyDescriptor final {
    PermissionPolicyId policyId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t,32> implementationRevision;
    PermissionSignatureProfile signatureProfile;
    ProductionTransitionClassId supportedTransitionClass;
};

class TrustedPermissionIssuer final {
public:
    TrustedPermissionIssuer(const TrustedPermissionIssuer&) = default;
    TrustedPermissionIssuer& operator=(const TrustedPermissionIssuer&) = default;
    [[nodiscard]] const PermissionIssuerId& issuerId() const noexcept { return issuerId_; }
    [[nodiscard]] const Ed25519PublicKey& publicKey() const noexcept { return publicKey_; }
private:
    TrustedPermissionIssuer(PermissionIssuerId issuerId, Ed25519PublicKey publicKey) noexcept
        : issuerId_(std::move(issuerId)), publicKey_(std::move(publicKey)) {}
    PermissionIssuerId issuerId_;
    Ed25519PublicKey publicKey_;
    friend class ProductionPermissionPolicyProvider;
};

class PermissionVerificationPolicySnapshot final {
public:
    PermissionVerificationPolicySnapshot(const PermissionVerificationPolicySnapshot&) = default;
    PermissionVerificationPolicySnapshot& operator=(const PermissionVerificationPolicySnapshot&) = default;
    [[nodiscard]] const PermissionPolicySnapshotId& snapshotId() const noexcept { return snapshotId_; }
    [[nodiscard]] const PermissionPolicyDescriptor& descriptor() const noexcept { return descriptor_; }
    [[nodiscard]] const TrustedPermissionIssuer* findIssuer(const PermissionIssuerId& issuerId) const noexcept;
private:
    PermissionVerificationPolicySnapshot(
        PermissionPolicySnapshotId snapshotId,
        PermissionPolicyDescriptor descriptor,
        std::vector<TrustedPermissionIssuer> trustedIssuers)
        : snapshotId_(std::move(snapshotId)),
          descriptor_(std::move(descriptor)),
          trustedIssuers_(std::move(trustedIssuers)) {}
    PermissionPolicySnapshotId snapshotId_;
    PermissionPolicyDescriptor descriptor_;
    std::vector<TrustedPermissionIssuer> trustedIssuers_;
    friend class ProductionPermissionPolicyProvider;
};

class ProductionPermissionPolicyProvider final {
public:
    [[nodiscard]] static std::optional<PermissionVerificationPolicySnapshot> createCurrent();
};

class ProductionPermissionPrerequisiteRecord final {
public:
    ProductionPermissionPrerequisiteRecord(const ProductionPermissionPrerequisiteRecord&) = default;
    ProductionPermissionPrerequisiteRecord& operator=(const ProductionPermissionPrerequisiteRecord&) = default;
    [[nodiscard]] const PermissionDecisionId& decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const ProductionTransitionRequestBinding& binding() const noexcept { return binding_; }
    [[nodiscard]] const PermissionAttestationId& attestationId() const noexcept { return attestationId_; }
    [[nodiscard]] const PermissionIssuerId& issuerId() const noexcept { return issuerId_; }
    [[nodiscard]] const PermissionPolicySnapshotId& policySnapshotId() const noexcept { return policySnapshotId_; }
    [[nodiscard]] bool satisfied() const noexcept { return satisfied_; }
private:
    ProductionPermissionPrerequisiteRecord(
        PermissionDecisionId decisionId,
        ProductionTransitionRequestBinding binding,
        PermissionAttestationId attestationId,
        PermissionIssuerId issuerId,
        PermissionPolicySnapshotId policySnapshotId,
        bool satisfied) noexcept
        : decisionId_(std::move(decisionId)),
          binding_(std::move(binding)),
          attestationId_(std::move(attestationId)),
          issuerId_(std::move(issuerId)),
          policySnapshotId_(std::move(policySnapshotId)),
          satisfied_(satisfied) {}
    PermissionDecisionId decisionId_;
    ProductionTransitionRequestBinding binding_;
    PermissionAttestationId attestationId_;
    PermissionIssuerId issuerId_;
    PermissionPolicySnapshotId policySnapshotId_;
    bool satisfied_;
    friend class ProductionPermissionVerifier;
};

enum class TransitionPermissionReason : std::uint8_t {
    RequestLineageInconsistent,
    TransitionClassUnsupported,
    PolicyRevisionUnrecognized,
    AttestationMalformed,
    IssuerUnrecognized,
    SignatureInvalid,
    RequestBindingMismatch,
    InternalVerificationFailure
};

class ProductionTransitionPermissionRejection final {
public:
    ProductionTransitionPermissionRejection(const ProductionTransitionPermissionRejection&) = default;
    ProductionTransitionPermissionRejection& operator=(const ProductionTransitionPermissionRejection&) = default;
    [[nodiscard]] const PermissionDecisionId& decisionId() const noexcept { return decisionId_; }
    [[nodiscard]] const TransitionRequestDecisionId& requestDecisionId() const noexcept { return requestDecisionId_; }
    [[nodiscard]] TransitionPermissionReason primaryReason() const noexcept { return primaryReason_; }
    [[nodiscard]] std::uint64_t reasonFlags() const noexcept { return reasonFlags_; }
private:
    ProductionTransitionPermissionRejection(
        PermissionDecisionId decisionId,
        TransitionRequestDecisionId requestDecisionId,
        TransitionPermissionReason primaryReason,
        std::uint64_t reasonFlags) noexcept
        : decisionId_(std::move(decisionId)),
          requestDecisionId_(std::move(requestDecisionId)),
          primaryReason_(primaryReason),
          reasonFlags_(reasonFlags) {}
    PermissionDecisionId decisionId_;
    TransitionRequestDecisionId requestDecisionId_;
    TransitionPermissionReason primaryReason_;
    std::uint64_t reasonFlags_;
    friend class ProductionPermissionVerifier;
};

using ProductionTransitionPermissionResult =
    std::variant<ProductionPermissionPrerequisiteRecord,ProductionTransitionPermissionRejection>;

class ProductionPermissionVerifier final {
public:
    [[nodiscard]]
    std::optional<ProductionTransitionPermissionResult>
    evaluate(
        const ProductionDerivedTransitionRequest& request,
        const PermissionVerificationPolicySnapshot& policy,
        std::span<const std::uint8_t> payloadBytes,
        std::span<const std::uint8_t> signatureBytes) const;
};

} // namespace AdaptiveMesh
