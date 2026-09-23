#include "production_transition_permission.hpp"
#include "detail/production_transition_permission_internal.hpp"
#include "transition_permission_implementation_revision.hpp"

#include <sodium.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#elif defined(__linux__)
#include <sys/random.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <cstdlib>
#endif

namespace AdaptiveMesh::detail {
namespace {

constexpr std::size_t kMaxAttempts = 8;
constexpr std::size_t kRecentCount = 128;

std::atomic<PermissionOpaqueIdFillFunction> testFillFunction{nullptr};
std::atomic_flag recentIdsLock = ATOMIC_FLAG_INIT;
std::array<std::array<std::uint8_t,16>,kRecentCount> recentIds{};
std::size_t recentIdsSize = 0;
std::size_t nextRecentId = 0;

class RecentIdsGuard final {
public:
    RecentIdsGuard() noexcept {
        while (recentIdsLock.test_and_set(std::memory_order_acquire)) {}
    }
    ~RecentIdsGuard() { recentIdsLock.clear(std::memory_order_release); }
private:
    RecentIdsGuard(const RecentIdsGuard&) = delete;
    RecentIdsGuard& operator=(const RecentIdsGuard&) = delete;
};

[[nodiscard]] bool allZero(const std::array<std::uint8_t,16>& bytes) noexcept {
    return std::all_of(bytes.begin(),bytes.end(),[](std::uint8_t value) {
        return value == 0U;
    });
}

[[nodiscard]] bool reserveIfNotRecent(const std::array<std::uint8_t,16>& bytes) noexcept {
    RecentIdsGuard guard;
    for (std::size_t i=0; i<recentIdsSize; ++i) {
        if (recentIds[i] == bytes) return false;
    }
    if (recentIdsSize < kRecentCount) {
        recentIds[recentIdsSize++] = bytes;
    } else {
        recentIds[nextRecentId] = bytes;
        nextRecentId = (nextRecentId + 1U) % kRecentCount;
    }
    return true;
}

[[nodiscard]] bool fillFromOperatingSystem(std::array<std::uint8_t,16>& out) noexcept {
#if defined(_WIN32)
    const NTSTATUS status = BCryptGenRandom(
        nullptr,reinterpret_cast<PUCHAR>(out.data()),
        static_cast<ULONG>(out.size()),BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status >= 0;
#elif defined(__linux__)
    std::size_t offset = 0;
    while (offset < out.size()) {
        const auto result = ::getrandom(out.data()+offset,out.size()-offset,0);
        if (result > 0) {
            offset += static_cast<std::size_t>(result);
            continue;
        }
        if (result < 0 && errno == EINTR) continue;
        return false;
    }
    return true;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    ::arc4random_buf(out.data(),out.size());
    return true;
#else
    static_cast<void>(out);
    return false;
#endif
}

} // namespace

std::optional<std::array<std::uint8_t,16>>
tryGeneratePermissionOpaqueIdBytes() noexcept
{
    for (std::size_t attempt=0; attempt<kMaxAttempts; ++attempt) {
        std::array<std::uint8_t,16> candidate{};
        const auto fill = testFillFunction.load(std::memory_order_acquire);
        if (fill != nullptr) {
            if (!fill(candidate)) return std::nullopt;
        } else if (!fillFromOperatingSystem(candidate)) {
            return std::nullopt;
        }
        if (allZero(candidate)) continue;
        if (!reserveIfNotRecent(candidate)) continue;
        return candidate;
    }
    return std::nullopt;
}

void setPermissionOpaqueIdFillFunctionForTesting(
    PermissionOpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction,std::memory_order_release);
}

void resetPermissionOpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr,std::memory_order_release);
    RecentIdsGuard guard;
    recentIds = {};
    recentIdsSize = 0;
    nextRecentId = 0;
}

} // namespace AdaptiveMesh::detail

namespace AdaptiveMesh {
namespace {

constexpr PermissionPolicyId::Bytes kPolicyId{
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
    0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f
};
constexpr std::uint64_t kBridgeCouplingAdjustmentV1 = 0x4252494447455631ULL;

[[nodiscard]] constexpr std::uint8_t hexNibble(char value) noexcept {
    if (value >= '0' && value <= '9') return static_cast<std::uint8_t>(value - '0');
    if (value >= 'a' && value <= 'f') return static_cast<std::uint8_t>(value - 'a' + 10);
    if (value >= 'A' && value <= 'F') return static_cast<std::uint8_t>(value - 'A' + 10);
    return 0xffU;
}

[[nodiscard]] constexpr std::array<std::uint8_t,32>
implementationRevisionDigest() noexcept
{
    constexpr const char* hex = SOAM_TRANSITION_PERMISSION_IMPLEMENTATION_REVISION_SHA;
    std::array<std::uint8_t,32> digest{};
    for (std::size_t i=0; i<digest.size(); ++i) {
        const auto high=hexNibble(hex[i*2U]);
        const auto low=hexNibble(hex[i*2U+1U]);
        digest[i]=static_cast<std::uint8_t>((high<<4U)|low);
    }
    return digest;
}

constexpr auto kImplementationRevision=implementationRevisionDigest();

template<std::size_t N>
[[nodiscard]] bool nonZero(const std::array<std::uint8_t,N>& bytes) noexcept {
    return std::any_of(bytes.begin(),bytes.end(),[](std::uint8_t value) {
        return value != 0U;
    });
}

[[nodiscard]] std::uint64_t reasonFlag(TransitionPermissionReason reason) noexcept {
    return std::uint64_t{1} << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] std::uint16_t readU16Le(
    std::span<const std::uint8_t> bytes,std::size_t offset) noexcept
{
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[offset]) |
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(bytes[offset+1U]) << 8U));
}

[[nodiscard]] bool requestLineageIsConsistent(
    const ProductionDerivedTransitionRequest& request) noexcept
{
    const auto& descriptor=request.policyDescriptor();
    const auto& relation=request.binding().relationship();
    const auto& key=request.persistenceStreamKey();

    if (!nonZero(request.decisionId().bytes()) ||
        !nonZero(request.persistenceObservationDecisionId().bytes()) ||
        !nonZero(request.persistenceStreamInstanceId().bytes()) ||
        !nonZero(request.policyEvidenceDecisionId().bytes()) ||
        !nonZero(request.sourceCaptureId().bytes()) ||
        !nonZero(descriptor.policyId.bytes()) ||
        !nonZero(descriptor.implementationRevision) ||
        request.stateVersion()==0U ||
        descriptor.majorVersion!=1U ||
        descriptor.minorVersion!=0U ||
        descriptor.implementationRevisionKind!=1U) {
        return false;
    }

    if (relation.sourceNodeId()!=key.sourceNodeId() ||
        relation.targetNodeId()!=key.targetNodeId() ||
        relation.generation()!=key.relationshipGeneration()) {
        return false;
    }

    switch (request.sourceRecommendation()) {
        case PersistentBridgeRecommendation::SUPPORT:
            return request.binding().direction()==RequestedTransitionDirection::support;
        case PersistentBridgeRecommendation::CONSTRAIN:
            return request.binding().direction()==RequestedTransitionDirection::constrain;
        case PersistentBridgeRecommendation::PRESERVE:
            return false;
    }
    return false;
}

[[nodiscard]] bool recognizedPolicy(
    const PermissionVerificationPolicySnapshot& policy) noexcept
{
    const auto& d=policy.descriptor();
    return
        d.policyId.bytes()==kPolicyId &&
        d.majorVersion==1U &&
        d.minorVersion==0U &&
        d.implementationRevisionKind==1U &&
        d.implementationRevision==kImplementationRevision &&
        d.signatureProfile==PermissionSignatureProfile::ed25519 &&
        d.supportedTransitionClass==
            detail::ProductionTransitionPermissionPolicyAccess::bridgeCouplingAdjustmentV1();
}

[[nodiscard]] bool rawPolicyClaimsMatch(
    std::span<const std::uint8_t> payload,
    const PermissionVerificationPolicySnapshot& policy) noexcept
{
    if (payload.size()!=kPermissionAttestationV1PayloadSize) return false;
    const auto& d=policy.descriptor();
    if (!std::equal(
            d.policyId.bytes().begin(),d.policyId.bytes().end(),
            payload.begin()+125)) {
        return false;
    }
    return readU16Le(payload,141U)==d.majorVersion &&
           readU16Le(payload,143U)==d.minorVersion;
}

[[nodiscard]] bool directionMatches(
    PermissionAttestationDirection attestation,
    RequestedTransitionDirection request) noexcept
{
    if (attestation==PermissionAttestationDirection::support) {
        return request==RequestedTransitionDirection::support;
    }
    return request==RequestedTransitionDirection::constrain;
}

[[nodiscard]] bool requestBindingMatches(
    const PermissionAttestationPayloadV1& payload,
    const ProductionDerivedTransitionRequest& request) noexcept
{
    const auto& relation=request.binding().relationship();
    return
        payload.requestDecisionId()==request.decisionId().bytes() &&
        payload.sourceNodeId()==static_cast<std::uint64_t>(relation.sourceNodeId()) &&
        payload.targetNodeId()==static_cast<std::uint64_t>(relation.targetNodeId()) &&
        payload.relationshipGeneration()==relation.generation() &&
        directionMatches(payload.direction(),request.binding().direction()) &&
        payload.transitionClass()==kBridgeCouplingAdjustmentV1 &&
        payload.stateVersion()==request.stateVersion();
}

} // namespace

const TrustedPermissionIssuer*
PermissionVerificationPolicySnapshot::findIssuer(
    const PermissionIssuerId& issuerId) const noexcept
{
    const auto it=std::find_if(
        trustedIssuers_.begin(),trustedIssuers_.end(),
        [&](const TrustedPermissionIssuer& issuer) {
            return issuer.issuerId()==issuerId;
        });
    return it==trustedIssuers_.end() ? nullptr : &*it;
}

std::optional<PermissionVerificationPolicySnapshot>
ProductionPermissionPolicyProvider::createCurrent()
{
#ifndef SOAM_PERMISSION_TESTING
    return std::nullopt;
#else
    constexpr PermissionIssuerId::Bytes kTestIssuerId{
        0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
        0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f
    };
    constexpr Ed25519PublicKey::Bytes kTestPublicKey{
        0x03,0xa1,0x07,0xbf,0xf3,0xce,0x10,0xbe,
        0x1d,0x70,0xdd,0x18,0xe7,0x4b,0xc0,0x99,
        0x67,0xe4,0xd6,0x30,0x9b,0xa5,0x0d,0x5f,
        0x1d,0xdc,0x86,0x64,0x12,0x55,0x31,0xb8
    };

    const auto snapshotBytes=detail::tryGeneratePermissionOpaqueIdBytes();
    if (!snapshotBytes.has_value()) return std::nullopt;
    const auto publicKey=parseEd25519PublicKey(kTestPublicKey);
    if (!publicKey.has_value()) return std::nullopt;

    std::vector<TrustedPermissionIssuer> trustedIssuers;
    trustedIssuers.emplace_back(PermissionIssuerId{kTestIssuerId},*publicKey);

    return PermissionVerificationPolicySnapshot{
        PermissionPolicySnapshotId{*snapshotBytes},
        PermissionPolicyDescriptor{
            PermissionPolicyId{kPolicyId},
            1U,0U,1U,kImplementationRevision,
            PermissionSignatureProfile::ed25519,
            detail::ProductionTransitionPermissionPolicyAccess::bridgeCouplingAdjustmentV1()
        },
        std::move(trustedIssuers)
    };
#endif
}

std::optional<ProductionTransitionPermissionResult>
ProductionPermissionVerifier::evaluate(
    const ProductionDerivedTransitionRequest& request,
    const PermissionVerificationPolicySnapshot& policy,
    std::span<const std::uint8_t> payloadBytes,
    std::span<const std::uint8_t> signatureBytes) const
{
    const auto decisionBytes=detail::tryGeneratePermissionOpaqueIdBytes();
    if (!decisionBytes.has_value()) return std::nullopt;
    PermissionDecisionId decisionId{*decisionBytes};

    const auto reject=[&](TransitionPermissionReason reason)
        -> std::optional<ProductionTransitionPermissionResult>
    {
        return ProductionTransitionPermissionResult{
            ProductionTransitionPermissionRejection{
                decisionId,request.decisionId(),reason,reasonFlag(reason)}
        };
    };

    if (!requestLineageIsConsistent(request)) {
        return reject(TransitionPermissionReason::RequestLineageInconsistent);
    }
    if (request.binding().transitionClass()!=policy.descriptor().supportedTransitionClass) {
        return reject(TransitionPermissionReason::TransitionClassUnsupported);
    }
    if (!recognizedPolicy(policy)) {
        return reject(TransitionPermissionReason::PolicyRevisionUnrecognized);
    }
    if (payloadBytes.size()==kPermissionAttestationV1PayloadSize &&
        !rawPolicyClaimsMatch(payloadBytes,policy)) {
        return reject(TransitionPermissionReason::PolicyRevisionUnrecognized);
    }

    const auto payload=parsePermissionAttestationV1(payloadBytes);
    const auto signature=parseEd25519Signature(signatureBytes);
    if (!payload.has_value() || !signature.has_value()) {
        return reject(TransitionPermissionReason::AttestationMalformed);
    }

    PermissionIssuerId issuerId{payload->issuerId()};
    const auto* issuer=policy.findIssuer(issuerId);
    if (issuer==nullptr) {
        return reject(TransitionPermissionReason::IssuerUnrecognized);
    }

    if (sodium_init()<0) {
        return reject(TransitionPermissionReason::InternalVerificationFailure);
    }
    if (!verifyEd25519(issuer->publicKey(),*signature,*payload)) {
        return reject(TransitionPermissionReason::SignatureInvalid);
    }
    if (!requestBindingMatches(*payload,request)) {
        return reject(TransitionPermissionReason::RequestBindingMismatch);
    }

    return ProductionTransitionPermissionResult{
        ProductionPermissionPrerequisiteRecord{
            std::move(decisionId),
            request.binding(),
            PermissionAttestationId{payload->attestationId()},
            std::move(issuerId),
            policy.snapshotId(),
            payload->decision()==PermissionAttestationDecision::permit
        }
    };
}

} // namespace AdaptiveMesh
