#include "provenance_admissibility.hpp"
#include "system_architecture.hpp"
#include "detail/provenance_admissibility_internal.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

template <std::size_t N>
bool nonZero(const std::array<std::uint8_t,N>& bytes) {
    for (auto b:bytes) if (b!=0U) return true;
    return false;
}

AdmissibilityPolicyDescriptor policyDescriptor() {
    AdmissibilityPolicyDescriptor d{};
    d.policyId.fill(0x91);
    d.majorVersion=1;
    d.minorVersion=0;
    return d;
}

ProvenanceAdmissibilityPolicySnapshot makePolicy(
    const ProductionProvenanceEnvelope& envelope,
    bool includeProducer=true,
    bool includeSchema=true,
    bool includeDependencies=true)
{
    const auto& m=envelope.metadata();

    std::vector<ProducerPolicyEntry> producers;
    if (includeProducer) {
        producers.push_back(ProducerPolicyEntry{
            m.producerId(),
            m.producerMajor(),
            m.producerMinor(),
            m.implementationRevisionKind(),
            m.implementationRevision(),
            ProducerLifecycleStatus::RECOGNIZED
        });
    }

    std::vector<SchemaPolicyEntry> schemas;
    if (includeSchema) {
        schemas.push_back(SchemaPolicyEntry{
            m.schemaId(),
            m.schemaMajor(),
            m.schemaMinor(),
            m.canonicalEncodingVersion(),
            SchemaLifecycleStatus::COMPATIBLE
        });
    }

    std::vector<DependencyPolicyEntry> dependencies;
    if (includeDependencies) {
        for (const auto& dep:m.dependencies()) {
            DependencySemanticCategory category=DependencySemanticCategory::OTHER;
            if (dep.kind==1U) category=DependencySemanticCategory::SOURCE_CAPTURE_CONTRACT;
            if (dep.kind==2U) category=DependencySemanticCategory::CANONICAL_ENCODING;
            if (dep.kind==3U) category=DependencySemanticCategory::DIGEST_PROFILE;
            dependencies.push_back(DependencyPolicyEntry{
                dep.kind,
                dep.dependencyId,
                dep.versionMajor,
                dep.versionMinor,
                dep.revisionKind,
                dep.revisionDigest,
                category,
                DependencyLifecycleStatus::ACTIVE
            });
        }
    }

    ProvenanceAdmissibilityPolicyPublisher publisher;
    auto policy=publisher.publish(
        policyDescriptor(),
        std::move(producers),
        std::move(schemas),
        std::move(dependencies));
    require(policy.has_value());
    return std::move(*policy);
}

class CountingResolver final : public SourceEvidenceResolver {
public:
    explicit CountingResolver(const SourceEvidenceResolver& delegate)
        : delegate_(delegate) {}

    SourceResolutionResult resolve(
        const SourceCaptureId& id) const override
    {
        ++calls_;
        return delegate_.resolve(id);
    }

    [[nodiscard]] int calls() const noexcept { return calls_.load(); }

private:
    const SourceEvidenceResolver& delegate_;
    mutable std::atomic<int> calls_{0};
};

class FixedResolver final : public SourceEvidenceResolver {
public:
    explicit FixedResolver(SourceResolutionResult result)
        : result_(std::move(result)) {}

    SourceResolutionResult resolve(
        const SourceCaptureId&) const override
    {
        ++calls_;
        return result_;
    }

    [[nodiscard]] int calls() const noexcept { return calls_.load(); }

private:
    SourceResolutionResult result_;
    mutable std::atomic<int> calls_{0};
};

namespace {
std::atomic<int> fillCalls{0};

bool failFill(std::array<std::uint8_t,16>&) noexcept {
    ++fillCalls;
    return false;
}
}

static_assert(!std::is_default_constructible_v<PolicySnapshotId>);
static_assert(!std::is_default_constructible_v<AdmissibilityDecisionId>);
static_assert(!std::is_default_constructible_v<AdmissibleProductionProvenance>);

int main() {
    detail::resetD8COpaqueIdGeneratorForTesting();

    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0,{0.0,0.0,0.0},1.0);
    mesh.addNode(1,{1.0,0.0,0.0},1.0);
    mesh.connectNodes(0,1);

    const auto snapshot=mesh.captureProductionRelationshipSourceSnapshot({0,1});
    require(snapshot.has_value());

    RetainedSourceEvidenceStore store;
    require(store.retain(*snapshot)==SourceEvidenceRetentionResult::INSERTED);
    require(store.retain(*snapshot)==SourceEvidenceRetentionResult::ALREADY_RETAINED);
    require(store.size()==1U);

    ProductionProvenanceEnvelopeProducer producer;
    const auto envelope=producer.produce(*snapshot);
    require(envelope.has_value());

    require(envelope->metadata().implementationRevisionKind()==1U);
    require(nonZero(envelope->metadata().implementationRevision()));
    require(envelope->metadata().dependencies().size()==3U);
    require(envelope->metadata().dependencies()[0].kind==1U);
    require(envelope->metadata().dependencies()[1].kind==2U);
    require(envelope->metadata().dependencies()[2].kind==3U);

    auto policy=makePolicy(*envelope);
    require(nonZero(policy.policySnapshotId().bytes()));

    CountingResolver countingStore(store);
    ProvenanceAdmissibilityEvaluator evaluator;

    const auto accepted=evaluator.evaluate(*envelope,policy,countingStore);
    require(accepted.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(*accepted));
    const auto& success=std::get<AdmissibleProductionProvenance>(*accepted);
    require(nonZero(success.decisionId().bytes()));
    require(success.policySnapshotId()==policy.policySnapshotId());
    require(success.envelope().provenanceItemId()==envelope->provenanceItemId());
    require(success.envelope().sourceCaptureId()==envelope->sourceCaptureId());
    require(success.sourceVerification().bitExactMatch);
    require(countingStore.calls()==1);

    const auto acceptedAgain=evaluator.evaluate(*envelope,policy,countingStore);
    require(acceptedAgain.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(*acceptedAgain));
    const auto& successAgain=
        std::get<AdmissibleProductionProvenance>(*acceptedAgain);
    require(successAgain.decisionId()!=success.decisionId());
    require(successAgain.policySnapshotId()==success.policySnapshotId());
    require(countingStore.calls()==2);

    auto unknownProducerPolicy=makePolicy(*envelope,false,true,true);
    CountingResolver mustNotRun(store);
    const auto unknownProducer=
        evaluator.evaluate(*envelope,unknownProducerPolicy,mustNotRun);
    require(unknownProducer.has_value());
    require(std::holds_alternative<ProvenanceAdmissibilityRejection>(
        *unknownProducer));
    require(std::get<ProvenanceAdmissibilityRejection>(*unknownProducer)
        .primaryReason()==ProvenanceAdmissibilityReason::PRODUCER_UNKNOWN);
    require(mustNotRun.calls()==0);

    auto unknownSchemaPolicy=makePolicy(*envelope,true,false,true);
    CountingResolver schemaResolver(store);
    const auto unknownSchema=
        evaluator.evaluate(*envelope,unknownSchemaPolicy,schemaResolver);
    require(unknownSchema.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*unknownSchema)
        .primaryReason()==ProvenanceAdmissibilityReason::SCHEMA_UNKNOWN);
    require(schemaResolver.calls()==0);

    auto missingDependencyPolicy=makePolicy(*envelope,true,true,false);
    CountingResolver dependencyResolver(store);
    const auto dependencyReject=
        evaluator.evaluate(*envelope,missingDependencyPolicy,dependencyResolver);
    require(dependencyReject.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*dependencyReject)
        .primaryReason()==ProvenanceAdmissibilityReason::DEPENDENCY_UNKNOWN);
    require(dependencyResolver.calls()==0);

    RetainedSourceEvidenceStore emptyStore;
    CountingResolver emptyResolver(emptyStore);
    const auto unavailable=evaluator.evaluate(*envelope,policy,emptyResolver);
    require(unavailable.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*unavailable)
        .primaryReason()==ProvenanceAdmissibilityReason::SOURCE_RECORD_UNAVAILABLE);
    require(emptyResolver.calls()==1);

    const auto secondSnapshot=
        mesh.captureProductionRelationshipSourceSnapshot({0,1});
    require(secondSnapshot.has_value());
    RetainedSourceEvidenceStore secondStore;
    require(secondStore.retain(*secondSnapshot)==
        SourceEvidenceRetentionResult::INSERTED);
    const auto otherResolution=
        secondStore.resolve(secondSnapshot->sourceCaptureId());
    require(otherResolution.status==SourceResolutionStatus::FOUND);
    FixedResolver mismatching(otherResolution);
    const auto mismatch=evaluator.evaluate(*envelope,policy,mismatching);
    require(mismatch.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*mismatch)
        .primaryReason()==ProvenanceAdmissibilityReason::SOURCE_RECORD_MISMATCH);
    require(mismatching.calls()==1);

    FixedResolver resolverFailure(SourceResolutionResult{
        SourceResolutionStatus::RESOLVER_FAILURE,
        std::nullopt
    });
    const auto failedResolve=evaluator.evaluate(*envelope,policy,resolverFailure);
    require(failedResolve.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*failedResolve)
        .primaryReason()==ProvenanceAdmissibilityReason::SOURCE_RESOLVER_FAILURE);
    require(resolverFailure.calls()==1);

    auto corrupted=*envelope;
    auto& corruptedBytes=
        const_cast<std::vector<std::uint8_t>&>(corrupted.canonicalBytes());
    corruptedBytes[0]^=0x01U;
    CountingResolver digestResolver(store);
    const auto digestReject=evaluator.evaluate(corrupted,policy,digestResolver);
    require(digestReject.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*digestReject)
        .primaryReason()==ProvenanceAdmissibilityReason::CANONICAL_DIGEST_MISMATCH);
    require(digestResolver.calls()==0);

    const double stateBefore=mesh.getNodeState(0);
    const double healthBefore=mesh.getNodeHealth(0);
    require(mesh.getNodeState(0)==stateBefore);
    require(mesh.getNodeHealth(0)==healthBefore);

    detail::resetD8COpaqueIdGeneratorForTesting();
    fillCalls.store(0);
    detail::setD8COpaqueIdFillFunctionForTesting(&failFill);
    const auto noDecision=evaluator.evaluate(*envelope,policy,store);
    require(!noDecision.has_value());
    require(fillCalls.load()==1);

    detail::resetD8COpaqueIdGeneratorForTesting();

    // Duplicate producer key makes policy publication fail closed.
    const auto& m=envelope->metadata();
    ProducerPolicyEntry duplicate{
        m.producerId(),m.producerMajor(),m.producerMinor(),
        m.implementationRevisionKind(),m.implementationRevision(),
        ProducerLifecycleStatus::RECOGNIZED
    };
    ProvenanceAdmissibilityPolicyPublisher publisher;
    auto invalidPolicy=publisher.publish(
        policyDescriptor(),
        {duplicate,duplicate},
        {},
        {});
    require(!invalidPolicy.has_value());

    return 0;
}
