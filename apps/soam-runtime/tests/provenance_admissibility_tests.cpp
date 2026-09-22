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
    for (const auto byte:bytes) {
        if (byte!=0U) return true;
    }
    return false;
}

ProvenanceAdmissibilityPolicySnapshot clonePolicy(
    const ProvenanceAdmissibilityPolicySnapshot& source,
    std::vector<ProducerPolicyEntry> producers,
    std::vector<SchemaPolicyEntry> schemas,
    std::vector<DependencyPolicyEntry> dependencies,
    std::vector<std::uint8_t> requiredKinds)
{
    return detail::ProvenanceAdmissibilityTestAccess::makePolicy(
        source.policySnapshotId().bytes(),
        source.descriptor(),
        std::move(producers),
        std::move(schemas),
        std::move(dependencies),
        std::move(requiredKinds));
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
static_assert(!std::is_constructible_v<PolicySnapshotId,PolicySnapshotId::Bytes>);
static_assert(!std::is_default_constructible_v<AdmissibilityDecisionId>);
static_assert(!std::is_constructible_v<
    AdmissibilityDecisionId,AdmissibilityDecisionId::Bytes>);
static_assert(!std::is_default_constructible_v<AdmissibleProductionProvenance>);
static_assert(!std::is_default_constructible_v<
    ProvenanceAdmissibilityPolicySnapshot>);

int main() {
    detail::resetD8COpaqueIdGeneratorForTesting();

    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0,{0.0,0.0,0.0},1.0);
    mesh.addNode(1,{1.0,0.0,0.0},1.0);
    mesh.connectNodes(0,1);

    const auto snapshot=
        mesh.captureProductionRelationshipSourceSnapshot({0,1});
    require(snapshot.has_value());

    RetainedSourceEvidenceStore store;
    require(store.retain(*snapshot)==
        SourceEvidenceRetentionResult::INSERTED);
    require(store.retain(*snapshot)==
        SourceEvidenceRetentionResult::ALREADY_RETAINED);
    require(store.size()==1U);

    ProductionProvenanceEnvelopeProducer producer;
    const auto envelope=producer.produce(*snapshot);
    require(envelope.has_value());

    // D8B typed metadata must describe the same canonical header bytes.
    require(envelope->metadata().implementationRevisionKind()==1U);
    require(nonZero(envelope->metadata().implementationRevision()));
    require(envelope->metadata().dependencies().size()==3U);
    require(envelope->metadata().dependencies()[0].kind==1U);
    require(envelope->metadata().dependencies()[1].kind==2U);
    require(envelope->metadata().dependencies()[2].kind==3U);
    require(envelope->canonicalBytes().at(50)==
        envelope->metadata().implementationRevisionKind());
    for (std::size_t i=0;i<32U;++i) {
        require(envelope->canonicalBytes().at(51U+i)==
            envelope->metadata().implementationRevision()[i]);
    }

    auto policy=
        ProductionProvenanceAdmissibilityPolicyProvider::createCurrent();
    require(policy.has_value());
    require(nonZero(policy->policySnapshotId().bytes()));
    require(policy->producers().size()==1U);
    require(policy->schemas().size()==1U);
    require(policy->dependencies().size()==3U);
    require(policy->requiredDependencyKinds()==
        std::vector<std::uint8_t>({1U,2U,3U}));

    CountingResolver countingStore(store);
    ProvenanceAdmissibilityEvaluator evaluator;

    const auto accepted=evaluator.evaluate(*envelope,*policy,countingStore);
    require(accepted.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(*accepted));
    const auto& success=std::get<AdmissibleProductionProvenance>(*accepted);
    require(nonZero(success.decisionId().bytes()));
    require(success.policySnapshotId()==policy->policySnapshotId());
    require(success.envelope().provenanceItemId()==
        envelope->provenanceItemId());
    require(success.envelope().sourceCaptureId()==
        envelope->sourceCaptureId());
    require(success.sourceVerification().bitExactMatch);
    require(countingStore.calls()==1);

    const auto acceptedAgain=evaluator.evaluate(
        *envelope,*policy,countingStore);
    require(acceptedAgain.has_value());
    require(std::holds_alternative<AdmissibleProductionProvenance>(
        *acceptedAgain));
    const auto& successAgain=
        std::get<AdmissibleProductionProvenance>(*acceptedAgain);
    require(successAgain.decisionId()!=success.decisionId());
    require(successAgain.policySnapshotId()==success.policySnapshotId());
    require(countingStore.calls()==2);

    // Unknown producer rejects before resolver.
    auto unknownProducerPolicy=clonePolicy(
        *policy,
        {},
        policy->schemas(),
        policy->dependencies(),
        policy->requiredDependencyKinds());
    CountingResolver mustNotRun(store);
    const auto unknownProducer=evaluator.evaluate(
        *envelope,unknownProducerPolicy,mustNotRun);
    require(unknownProducer.has_value());
    require(std::holds_alternative<ProvenanceAdmissibilityRejection>(
        *unknownProducer));
    require(std::get<ProvenanceAdmissibilityRejection>(*unknownProducer)
        .primaryReason()==ProvenanceAdmissibilityReason::ProducerUnknown);
    require(mustNotRun.calls()==0);

    // Unknown implementation revision is distinct from unknown producer.
    auto wrongRevisionProducers=policy->producers();
    wrongRevisionProducers[0].implementationRevision[0]^=0xffU;
    auto wrongRevisionPolicy=clonePolicy(
        *policy,
        std::move(wrongRevisionProducers),
        policy->schemas(),
        policy->dependencies(),
        policy->requiredDependencyKinds());
    CountingResolver revisionResolver(store);
    const auto wrongRevision=evaluator.evaluate(
        *envelope,wrongRevisionPolicy,revisionResolver);
    require(wrongRevision.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*wrongRevision)
        .primaryReason()==
            ProvenanceAdmissibilityReason::ImplementationRevisionUnrecognized);
    require(revisionResolver.calls()==0);

    auto unknownSchemaPolicy=clonePolicy(
        *policy,
        policy->producers(),
        {},
        policy->dependencies(),
        policy->requiredDependencyKinds());
    CountingResolver schemaResolver(store);
    const auto unknownSchema=evaluator.evaluate(
        *envelope,unknownSchemaPolicy,schemaResolver);
    require(unknownSchema.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*unknownSchema)
        .primaryReason()==ProvenanceAdmissibilityReason::SchemaUnknown);
    require(schemaResolver.calls()==0);

    // Missing required kind rejects before resolver.
    auto requiredKinds=policy->requiredDependencyKinds();
    requiredKinds.push_back(4U);
    auto requiredMissingPolicy=clonePolicy(
        *policy,
        policy->producers(),
        policy->schemas(),
        policy->dependencies(),
        std::move(requiredKinds));
    CountingResolver requiredResolver(store);
    const auto requiredMissing=evaluator.evaluate(
        *envelope,requiredMissingPolicy,requiredResolver);
    require(requiredMissing.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*requiredMissing)
        .primaryReason()==
            ProvenanceAdmissibilityReason::RequiredDependencyMissing);
    require(requiredResolver.calls()==0);

    // Unknown dependency rejects before resolver.
    auto dependencyUnknownPolicy=clonePolicy(
        *policy,
        policy->producers(),
        policy->schemas(),
        {},
        policy->requiredDependencyKinds());
    CountingResolver dependencyResolver(store);
    const auto dependencyReject=evaluator.evaluate(
        *envelope,dependencyUnknownPolicy,dependencyResolver);
    require(dependencyReject.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*dependencyReject)
        .primaryReason()==ProvenanceAdmissibilityReason::DependencyUnknown);
    require(dependencyResolver.calls()==0);

    // Interpretation dependency is explicitly prohibited in D8C v1.
    auto interpretationDeps=policy->dependencies();
    interpretationDeps[0].category=
        DependencySemanticCategory::InterpretationPolicy;
    auto interpretationPolicy=clonePolicy(
        *policy,
        policy->producers(),
        policy->schemas(),
        std::move(interpretationDeps),
        policy->requiredDependencyKinds());
    CountingResolver interpretationResolver(store);
    const auto interpretationReject=evaluator.evaluate(
        *envelope,interpretationPolicy,interpretationResolver);
    require(interpretationReject.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(
        *interpretationReject).primaryReason()==
        ProvenanceAdmissibilityReason::LegacyInterpretationDependency);
    require(interpretationResolver.calls()==0);

    RetainedSourceEvidenceStore emptyStore;
    CountingResolver emptyResolver(emptyStore);
    const auto unavailable=evaluator.evaluate(
        *envelope,*policy,emptyResolver);
    require(unavailable.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*unavailable)
        .primaryReason()==
            ProvenanceAdmissibilityReason::SourceRecordUnavailable);
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
    const auto mismatch=evaluator.evaluate(*envelope,*policy,mismatching);
    require(mismatch.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*mismatch)
        .primaryReason()==
            ProvenanceAdmissibilityReason::SourceRecordMismatch);
    require(mismatching.calls()==1);

    FixedResolver resolverFailure(SourceResolutionResult{
        SourceResolutionStatus::RESOLVER_FAILURE,
        std::nullopt
    });
    const auto failedResolve=evaluator.evaluate(
        *envelope,*policy,resolverFailure);
    require(failedResolve.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*failedResolve)
        .primaryReason()==
            ProvenanceAdmissibilityReason::SourceResolverFailure);
    require(resolverFailure.calls()==1);

    FixedResolver integrityConflict(SourceResolutionResult{
        SourceResolutionStatus::INTEGRITY_CONFLICT,
        std::nullopt
    });
    const auto conflict=evaluator.evaluate(
        *envelope,*policy,integrityConflict);
    require(conflict.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*conflict)
        .primaryReason()==
            ProvenanceAdmissibilityReason::SourceEvidenceIntegrityConflict);

    // Test-only corruption demonstrates digest mismatch short-circuits resolver.
    auto corrupted=*envelope;
    auto& corruptedBytes=
        const_cast<std::vector<std::uint8_t>&>(corrupted.canonicalBytes());
    corruptedBytes[0]^=0x01U;
    CountingResolver digestResolver(store);
    const auto digestReject=evaluator.evaluate(
        corrupted,*policy,digestResolver);
    require(digestReject.has_value());
    require(std::get<ProvenanceAdmissibilityRejection>(*digestReject)
        .primaryReason()==
            ProvenanceAdmissibilityReason::CanonicalDigestMismatch);
    require(digestResolver.calls()==0);

    const double stateBefore=mesh.getNodeState(0);
    const double healthBefore=mesh.getNodeHealth(0);
    require(mesh.getNodeState(0)==stateBefore);
    require(mesh.getNodeHealth(0)==healthBefore);

    // Decision-ID infrastructure failure is not a policy rejection.
    detail::resetD8COpaqueIdGeneratorForTesting();
    fillCalls.store(0);
    detail::setD8COpaqueIdFillFunctionForTesting(&failFill);
    const auto noDecision=evaluator.evaluate(*envelope,*policy,store);
    require(!noDecision.has_value());
    require(fillCalls.load()==1);

    detail::resetD8COpaqueIdGeneratorForTesting();
    return 0;
}
