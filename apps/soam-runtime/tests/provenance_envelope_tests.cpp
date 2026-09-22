#include "production_provenance_envelope.hpp"
#include "system_architecture.hpp"
#include "detail/provenance_envelope_internal.hpp"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }
void requireNear(double actual, double expected, double epsilon = 1e-12) noexcept {
    if (std::abs(actual - expected) > epsilon) fail();
}

std::vector<std::uint8_t> parseHex(std::string_view hex) {
    require((hex.size() % 2U) == 0U);
    auto nibble=[](char c)->std::uint8_t {
        if (c>='0'&&c<='9') return static_cast<std::uint8_t>(c-'0');
        if (c>='a'&&c<='f') return static_cast<std::uint8_t>(c-'a'+10);
        if (c>='A'&&c<='F') return static_cast<std::uint8_t>(c-'A'+10);
        fail();
    };
    std::vector<std::uint8_t> out;
    out.reserve(hex.size()/2U);
    for (std::size_t i=0;i<hex.size();i+=2U) {
        out.push_back(static_cast<std::uint8_t>((nibble(hex[i])<<4U)|nibble(hex[i+1U])));
    }
    return out;
}

template <std::size_t N>
std::array<std::uint8_t,N> parseArray(std::string_view hex) {
    const auto bytes=parseHex(hex);
    require(bytes.size()==N);
    std::array<std::uint8_t,N> out{};
    for(std::size_t i=0;i<N;++i) out[i]=bytes[i];
    return out;
}

std::string hexDigest(const detail::Digest256& digest) {
    static constexpr char digits[]="0123456789abcdef";
    std::string out;
    out.reserve(64);
    for(auto b:digest){
        out.push_back(digits[(b>>4U)&0x0fU]);
        out.push_back(digits[b&0x0fU]);
    }
    return out;
}

detail::CanonicalEnvelopeInput baseVector() {
    return {
        parseArray<16>("00112233445566778899aabbccddeeff"),1,0,1,
        parseArray<16>("102132435465768798a9bacbdcedfe0f"),1,0,0,{},
        parseArray<16>("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"),
        parseArray<16>("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"),
        0,1,7,11,
        1.0,0.5,1.0,0,
        1.0,1.0,1.0,1.0,
        {}
    };
}

void verifyVector(
    const detail::CanonicalEnvelopeInput& input,
    std::string_view expectedHex,
    std::string_view expectedDigest)
{
    const auto encoded=detail::encodeCanonicalEnvelopeV1(input);
    require(encoded.has_value());
    require(*encoded==parseHex(expectedHex));
    require(hexDigest(detail::sha256D8BDomainSeparated(*encoded))==expectedDigest);
}

namespace {
std::atomic<int> fillCall{0};
ProvenanceItemId::Bytes fixedA{};
ProvenanceItemId::Bytes fixedB{};

bool hardFail(ProvenanceItemId::Bytes&) noexcept { return false; }
bool zeroFill(ProvenanceItemId::Bytes& out) noexcept {
    out={}; ++fillCall; return true;
}
bool zeroThenFixed(ProvenanceItemId::Bytes& out) noexcept {
    const int call=fillCall.fetch_add(1);
    out = call==0 ? ProvenanceItemId::Bytes{} : fixedA;
    return true;
}
bool duplicateThenSecond(ProvenanceItemId::Bytes& out) noexcept {
    const int call=fillCall.fetch_add(1);
    out = call<=1 ? fixedA : fixedB;
    return true;
}
}

static_assert(!std::is_default_constructible_v<ProvenanceItemId>);
static_assert(!std::is_constructible_v<ProvenanceItemId,ProvenanceItemId::Bytes>);
static_assert(!std::is_default_constructible_v<CanonicalDigest>);
static_assert(!std::is_default_constructible_v<ProductionProvenanceEnvelope>);

int main() {
    detail::resetD8BTestSeams();

    const std::string v1=
"534f414d4438423100112233445566778899aabbccddeeff000100000001102132435465768798a9bacbdcedfe0f00010000000000000000000000000000000000000000000000000000000000000000000000aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb000000000000000000000000000000010000000000000007000000000000000b3ff00000000000003fe00000000000003ff0000000000000003ff00000000000003ff00000000000003ff00000000000003ff0000000000000000000";
    verifyVector(baseVector(),v1,
"3fb1f03828dadb48ddb23463b6d56e1679ef71731dc681fbebb920656a8fde70");

    auto v2=baseVector();
    v2.provenanceItemId=parseArray<16>("01010101010101010101010101010101");
    v2.sourceCaptureId=parseArray<16>("02020202020202020202020202020202");
    v2.targetNodeId=0; v2.relationshipGeneration=0; v2.stateVersion=0;
    v2.distance=0.0; v2.orientationWeight=-0.0; v2.capacity=0.0;
    v2.bridgeStatusTag=3; v2.sourceState=-0.0; v2.targetState=0.0;
    v2.sourceHealth=0.0; v2.targetHealth=-0.0;
    const std::string v2hex=
"534f414d4438423100112233445566778899aabbccddeeff000100000001102132435465768798a9bacbdcedfe0f0001000000000000000000000000000000000000000000000000000000000000000000000001010101010101010101010101010101020202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000038000000000000000000000000000000000000000000000008000000000000000000000";
    verifyVector(v2,v2hex,
"8de3c4b0f70de13830d892a7b59598ffe162972cb4d454edca02ed6b3981ed18");

    auto v3=baseVector();
    v3.sourceCaptureId=parseArray<16>("cccccccccccccccccccccccccccccccc");
    auto e1=detail::encodeCanonicalEnvelopeV1(baseVector());
    auto e3=detail::encodeCanonicalEnvelopeV1(v3);
    require(e1.has_value()&&e3.has_value()&&*e1!=*e3);
    require(hexDigest(detail::sha256D8BDomainSeparated(*e3))==
"ad283e8f6dcd10f7ff7544a85fa208c860259d0b328c17056cb19e2cbfcf6b99");

    auto v4=baseVector();
    v4.provenanceItemId=parseArray<16>("dddddddddddddddddddddddddddddddd");
    auto e4=detail::encodeCanonicalEnvelopeV1(v4);
    require(e4.has_value());
    require(hexDigest(detail::sha256D8BDomainSeparated(*e4))==
"25328c3c1ffa7199c0c5a9d1ce05e2725ce4c58b7c2e498fc1326b2d63c9f7aa");

    auto v5=baseVector();
    detail::CanonicalDependencyInput depA{
        2,parseArray<16>("22222222222222222222222222222222"),1,0,1,{}
    };
    depA.revisionDigest.fill(0x11);
    detail::CanonicalDependencyInput depB{
        1,parseArray<16>("11111111111111111111111111111111"),2,3,0,{}
    };
    v5.dependencies={depA,depB};
    auto e5=detail::encodeCanonicalEnvelopeV1(v5);
    require(e5.has_value());
    require(e5->size()==315U);
    require(hexDigest(detail::sha256D8BDomainSeparated(*e5))==
"9b267dd7bc1c86745d1c99f5da9922837ac3eed0bd6ea70b6fbdb4665ca6d4ee");
    v5.dependencies={depB,depA};
    auto e5reverse=detail::encodeCanonicalEnvelopeV1(v5);
    require(e5reverse.has_value()&&*e5reverse==*e5);

    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0,{0.0,0.0,0.0},1.0);
    mesh.addNode(1,{1.0,0.0,0.0},1.0);
    mesh.connectNodes(0,1);
    const auto snapshot=mesh.captureProductionRelationshipSourceSnapshot({0,1});
    require(snapshot.has_value());

    ProductionProvenanceEnvelopeProducer producer;
    const auto envelope=producer.produce(*snapshot);
    require(envelope.has_value());
    require(envelope->sourceCaptureId()==snapshot->sourceCaptureId());
    require(envelope->sourceNodeId()==snapshot->sourceNodeId());
    require(envelope->targetNodeId()==snapshot->targetNodeId());
    require(envelope->relationshipGeneration()==snapshot->relationshipGeneration());
    require(envelope->stateVersion()==snapshot->stateVersion());
    requireNear(envelope->distance(),snapshot->distance());
    requireNear(envelope->orientationWeight(),snapshot->orientationWeight());
    requireNear(envelope->capacity(),snapshot->capacity());
    require(envelope->bridgeStatus()==snapshot->bridgeStatus());
    require(envelope->canonicalBytes().size()==369U);

    const auto second=producer.produce(*snapshot);
    require(second.has_value());
    require(second->sourceCaptureId()==envelope->sourceCaptureId());
    require(second->provenanceItemId()!=envelope->provenanceItemId());
    require(second->canonicalDigest()!=envelope->canonicalDigest());

    const double stateBefore=mesh.getNodeState(0);
    const double healthBefore=mesh.getNodeHealth(0);

    detail::resetD8BTestSeams();
    detail::setProvenanceItemIdFillFunctionForTesting(&hardFail);
    require(!producer.produce(*snapshot).has_value());
    requireNear(mesh.getNodeState(0),stateBefore);
    requireNear(mesh.getNodeHealth(0),healthBefore);

    detail::resetD8BTestSeams();
    fillCall.store(0); fixedA.fill(0x51);
    detail::setProvenanceItemIdFillFunctionForTesting(&zeroThenFixed);
    const auto zeroRetry=producer.produce(*snapshot);
    require(zeroRetry.has_value());
    require(zeroRetry->provenanceItemId().bytes()==fixedA);
    require(fillCall.load()==2);

    detail::resetD8BTestSeams();
    fillCall.store(0); fixedA.fill(0x61); fixedB.fill(0x62);
    detail::setProvenanceItemIdFillFunctionForTesting(&duplicateThenSecond);
    const auto firstFixed=producer.produce(*snapshot);
    const auto duplicateRetry=producer.produce(*snapshot);
    require(firstFixed.has_value()&&duplicateRetry.has_value());
    require(firstFixed->provenanceItemId().bytes()==fixedA);
    require(duplicateRetry->provenanceItemId().bytes()==fixedB);
    require(fillCall.load()==3);

    detail::resetD8BTestSeams();
    fillCall.store(0);
    detail::setProvenanceItemIdFillFunctionForTesting(&zeroFill);
    require(!producer.produce(*snapshot).has_value());
    require(fillCall.load()==8);

    detail::resetD8BTestSeams();
    detail::setD8BCanonicalEncodingFailureForTesting(true);
    require(!producer.produce(*snapshot).has_value());

    detail::resetD8BTestSeams();
    detail::setD8BDigestFailureForTesting(true);
    require(!producer.produce(*snapshot).has_value());

    detail::resetD8BTestSeams();
    detail::setD8BManifestFailureForTesting(true);
    require(!producer.produce(*snapshot).has_value());

    detail::resetD8BTestSeams();
    return 0;
}
