#include "versioned_interpretation.hpp"
#include "detail/versioned_interpretation_internal.hpp"
#include "d8d_implementation_revision.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#elif defined(__linux__)
#include <sys/random.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <cstdlib>
#endif

namespace AdaptiveMesh {
namespace detail {
namespace {

constexpr std::size_t kMaxAttempts = 8;
constexpr std::size_t kRecentCount = 128;

std::atomic<D8DOpaqueIdFillFunction> testFillFunction{nullptr};
std::atomic_flag recentIdsLock = ATOMIC_FLAG_INIT;
std::array<std::array<std::uint8_t,16>, kRecentCount> recentIds{};
std::size_t recentIdsSize = 0;
std::size_t nextRecentId = 0;

class RecentIdsGuard final {
public:
    RecentIdsGuard() noexcept {
        while (recentIdsLock.test_and_set(std::memory_order_acquire)) {}
    }
    ~RecentIdsGuard() {
        recentIdsLock.clear(std::memory_order_release);
    }
    RecentIdsGuard(const RecentIdsGuard&) = delete;
    RecentIdsGuard& operator=(const RecentIdsGuard&) = delete;
};

[[nodiscard]] bool allZero(
    const std::array<std::uint8_t,16>& bytes) noexcept
{
    for (const auto byte:bytes) {
        if (byte!=0U) return false;
    }
    return true;
}

[[nodiscard]] bool reserveIfNotRecent(
    const std::array<std::uint8_t,16>& bytes) noexcept
{
    RecentIdsGuard guard;
    for(std::size_t i=0;i<recentIdsSize;++i) {
        if(recentIds[i]==bytes) return false;
    }
    if(recentIdsSize<kRecentCount) {
        recentIds[recentIdsSize++]=bytes;
    } else {
        recentIds[nextRecentId]=bytes;
        nextRecentId=(nextRecentId+1U)%kRecentCount;
    }
    return true;
}

[[nodiscard]] bool fillFromOperatingSystem(
    std::array<std::uint8_t,16>& out) noexcept
{
#if defined(_WIN32)
    const NTSTATUS status=BCryptGenRandom(
        nullptr,
        reinterpret_cast<PUCHAR>(out.data()),
        static_cast<ULONG>(out.size()),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status>=0;
#elif defined(__linux__)
    std::size_t offset=0;
    while(offset<out.size()) {
        const auto result=::getrandom(
            out.data()+offset,
            out.size()-offset,
            0);
        if(result>0) {
            offset+=static_cast<std::size_t>(result);
            continue;
        }
        if(result<0 && errno==EINTR) continue;
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

[[nodiscard]] bool finiteUnit(double value) noexcept {
    return std::isfinite(value) && value>=0.0 && value<=1.0;
}

} // namespace

std::optional<std::array<std::uint8_t,16>>
tryGenerateD8DOpaqueIdBytes() noexcept
{
    for(std::size_t attempt=0;attempt<kMaxAttempts;++attempt) {
        std::array<std::uint8_t,16> candidate{};
        const auto fill=testFillFunction.load(std::memory_order_acquire);
        if(fill!=nullptr) {
            if(!fill(candidate)) return std::nullopt;
        } else if(!fillFromOperatingSystem(candidate)) {
            return std::nullopt;
        }
        if(allZero(candidate)) continue;
        if(!reserveIfNotRecent(candidate)) continue;
        return candidate;
    }
    return std::nullopt;
}

void setD8DOpaqueIdFillFunctionForTesting(
    D8DOpaqueIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction,std::memory_order_release);
}

void resetD8DOpaqueIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr,std::memory_order_release);
    RecentIdsGuard guard;
    recentIds={};
    recentIdsSize=0;
    nextRecentId=0;
}

std::optional<InterpretationV1Computation>
computeInterpretationV1(
    double distance,
    double orientationWeight,
    double capacity,
    double sourceHealth,
    double targetHealth,
    double distanceAttenuationCoefficient) noexcept
{
    if(!std::isfinite(distance) || distance<0.0) return std::nullopt;
    if(!finiteUnit(orientationWeight)) return std::nullopt;
    if(!finiteUnit(capacity)) return std::nullopt;
    if(!finiteUnit(sourceHealth)) return std::nullopt;
    if(!finiteUnit(targetHealth)) return std::nullopt;
    if(!std::isfinite(distanceAttenuationCoefficient) ||
       distanceAttenuationCoefficient<=0.0) return std::nullopt;

    const double attenuation=
        1.0/(1.0+(distanceAttenuationCoefficient*distance));
    const double compatibility=
        capacity*attenuation*orientationWeight;
    const double confidence=std::min(sourceHealth,targetHealth);

    if(!finiteUnit(attenuation) ||
       !finiteUnit(compatibility) ||
       !finiteUnit(confidence)) {
        return std::nullopt;
    }

    return InterpretationV1Computation{
        attenuation,
        compatibility,
        confidence
    };
}

InterpretationPolicySnapshot
VersionedInterpretationTestAccess::makePolicy(
    InterpretationPolicySnapshotId::Bytes snapshotId,
    InterpretationPolicyId::Bytes policyId,
    std::uint16_t majorVersion,
    std::uint16_t minorVersion,
    std::uint8_t revisionKind,
    std::array<std::uint8_t,32> revision,
    double distanceAttenuationCoefficient,
    bool directional)
{
    return InterpretationPolicySnapshot{
        InterpretationPolicySnapshotId{snapshotId},
        InterpretationPolicyDescriptor{
            InterpretationPolicyId{policyId},
            majorVersion,
            minorVersion,
            revisionKind,
            revision
        },
        distanceAttenuationCoefficient,
        directional
    };
}

} // namespace detail

namespace {

constexpr InterpretationPolicyId::Bytes kPolicyId{
    0x63,0xde,0x65,0x6b,0xeb,0x62,0x57,0xe5,
    0x36,0xea,0x0d,0x52,0xf4,0x21,0x43,0xf8
};
constexpr double kDistanceAttenuationCoefficient=0.1;

[[nodiscard]] constexpr std::uint8_t hexNibble(char value) {
    if(value>='0' && value<='9') {
        return static_cast<std::uint8_t>(value-'0');
    }
    if(value>='a' && value<='f') {
        return static_cast<std::uint8_t>(value-'a'+10);
    }
    if(value>='A' && value<='F') {
        return static_cast<std::uint8_t>(value-'A'+10);
    }
    return 0xffU;
}

[[nodiscard]] constexpr std::array<std::uint8_t,32>
implementationRevisionDigest()
{
    constexpr const char* hex=SOAM_D8D_IMPLEMENTATION_REVISION_SHA;
    std::array<std::uint8_t,32> digest{};
    for(std::size_t i=0;i<digest.size();++i) {
        const auto high=hexNibble(hex[i*2U]);
        const auto low=hexNibble(hex[i*2U+1U]);
        digest[i]=static_cast<std::uint8_t>((high<<4U)|low);
    }
    return digest;
}

constexpr auto kImplementationRevision=implementationRevisionDigest();

template<std::size_t N>
[[nodiscard]] bool nonZero(
    const std::array<std::uint8_t,N>& bytes) noexcept
{
    for(const auto byte:bytes) {
        if(byte!=0U) return true;
    }
    return false;
}

[[nodiscard]] bool policyIsCurrent(
    const InterpretationPolicySnapshot& policy) noexcept
{
    const auto& descriptor=policy.descriptor();
    return
        descriptor.policyId.bytes()==kPolicyId &&
        descriptor.majorVersion==1U &&
        descriptor.minorVersion==0U &&
        descriptor.implementationRevisionKind==1U &&
        descriptor.implementationRevision==kImplementationRevision &&
        policy.distanceAttenuationCoefficient()==
            kDistanceAttenuationCoefficient &&
        policy.directional();
}

[[nodiscard]] std::uint64_t reasonFlag(
    ProductionInterpretationReason reason) noexcept
{
    return std::uint64_t{1}
        << static_cast<std::uint8_t>(reason);
}

[[nodiscard]] bool upstreamLineageIsConsistent(
    const AdmissibleProductionProvenance& provenance) noexcept
{
    const auto& envelope=provenance.envelope();
    return
        provenance.sourceVerification().bitExactMatch &&
        provenance.sourceVerification().sourceCaptureId==
            envelope.sourceCaptureId() &&
        nonZero(envelope.sourceCaptureId().bytes()) &&
        nonZero(envelope.provenanceItemId().bytes()) &&
        nonZero(provenance.decisionId().bytes()) &&
        nonZero(provenance.policySnapshotId().bytes());
}

} // namespace

std::optional<InterpretationPolicySnapshot>
ProductionInterpretationPolicyProvider::createCurrent()
{
    const auto snapshotId=detail::tryGenerateD8DOpaqueIdBytes();
    if(!snapshotId.has_value()) return std::nullopt;

    return InterpretationPolicySnapshot{
        InterpretationPolicySnapshotId{*snapshotId},
        InterpretationPolicyDescriptor{
            InterpretationPolicyId{kPolicyId},
            1U,
            0U,
            1U,
            kImplementationRevision
        },
        kDistanceAttenuationCoefficient,
        true
    };
}

std::optional<ProductionInterpretationResult>
VersionedProductionInterpreter::interpret(
    const AdmissibleProductionProvenance& provenance,
    const InterpretationPolicySnapshot& policy) const
{
    if(!policyIsCurrent(policy)) {
        return std::nullopt;
    }
    if(!upstreamLineageIsConsistent(provenance)) {
        return std::nullopt;
    }

    const auto decisionBytes=detail::tryGenerateD8DOpaqueIdBytes();
    if(!decisionBytes.has_value()) return std::nullopt;
    InterpretationDecisionId decisionId{*decisionBytes};

    const auto reject=[&](
        ProductionInterpretationReason reason)
        -> std::optional<ProductionInterpretationResult>
    {
        return ProductionInterpretationResult{
            ProductionInterpretationRejection{
                decisionId,
                policy.snapshotId(),
                provenance.envelope().provenanceItemId(),
                provenance.decisionId(),
                reason,
                reasonFlag(reason)
            }
        };
    };

    const auto& envelope=provenance.envelope();

    if(!std::isfinite(envelope.distance()) || envelope.distance()<0.0) {
        return reject(ProductionInterpretationReason::DistanceInvalid);
    }
    if(!std::isfinite(envelope.orientationWeight()) ||
       envelope.orientationWeight()<0.0 ||
       envelope.orientationWeight()>1.0) {
        return reject(ProductionInterpretationReason::OrientationWeightInvalid);
    }
    if(!std::isfinite(envelope.capacity()) ||
       envelope.capacity()<0.0 ||
       envelope.capacity()>1.0) {
        return reject(ProductionInterpretationReason::CapacityInvalid);
    }
    if(!std::isfinite(envelope.sourceHealth()) ||
       envelope.sourceHealth()<0.0 ||
       envelope.sourceHealth()>1.0) {
        return reject(ProductionInterpretationReason::SourceHealthInvalid);
    }
    if(!std::isfinite(envelope.targetHealth()) ||
       envelope.targetHealth()<0.0 ||
       envelope.targetHealth()>1.0) {
        return reject(ProductionInterpretationReason::TargetHealthInvalid);
    }

    const auto computed=detail::computeInterpretationV1(
        envelope.distance(),
        envelope.orientationWeight(),
        envelope.capacity(),
        envelope.sourceHealth(),
        envelope.targetHealth(),
        policy.distanceAttenuationCoefficient());
    if(!computed.has_value()) {
        return reject(
            ProductionInterpretationReason::InternalDeterministicEvaluationFailure);
    }

    try {
        InteractionObservation observation{computed->compatibility};
        BridgeConfidence confidence{computed->confidence};

        return ProductionInterpretationResult{
            VersionedProductionInterpretation{
                std::move(decisionId),
                policy.snapshotId(),
                policy.descriptor(),
                provenance,
                observation,
                confidence,
                InterpretationTrace{
                    envelope.distance(),
                    envelope.orientationWeight(),
                    envelope.capacity(),
                    envelope.sourceHealth(),
                    envelope.targetHealth(),
                    computed->attenuation,
                    computed->compatibility,
                    computed->confidence
                }
            }
        };
    } catch(...) {
        return reject(
            ProductionInterpretationReason::OutputInvariantViolation);
    }
}

} // namespace AdaptiveMesh
