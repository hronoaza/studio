# SOAM 2.0 — Live Validity to C1 Adapter Design Contract

## Status

- Layer: A2 — accepted A1 Live Validity -> accepted C1 prerequisite inputs
- Document class: concrete design contract
- Scope: A1 Live Validity -> C1 Freshness/Revalidation adapter boundary
- Design origin baseline: `71a34611c3acf847fb26b47c23567cec5a166ec1`
- Upstream accepted source: A1 Live Validity Evidence
- Downstream accepted contract: C1 transition eligibility prerequisite types
- Acceptance status: accepted; applied by implementation PR #26
- Ratification scope: bounded to the adapter boundary described in this document; not a generic adapter framework
- Authority effect: none

## Scope limitation

This contract governs only the A1 Live Validity -> C1 Freshness/Revalidation adapter boundary.

It does not define a generic adapter framework and does not prescribe failure semantics, provenance containers, decision identity, or projection semantics for B1, C, D, or future adapters.

Those require independent design decisions.

Structural implementation patterns may be reused without implying semantic inheritance. In particular:

- sibling module organization
- private constructor + friend seam
- compile-fail boundary tests
- presence of a restricted-origin wrapper type

are structural patterns, not semantic obligations. Semantic choice (lossy vs lossless, nullopt vs typed rejection, container vs decision identity) remains open for each new adapter.

## 1. Boundary

Accepted upstream: ProductionDerivedTransitionRequest + trusted runtime live snapshot source -> ProductionFreshnessPrerequisiteRecord + ProductionRevalidationPrerequisiteRecord -> STOP.

A2 introduces only:
- ProductionFreshnessPrerequisiteRecord -> FreshnessPrerequisiteEvidence
- ProductionRevalidationPrerequisiteRecord -> RevalidationPrerequisiteEvidence
- STOP.

A2 does not construct or evaluate a complete ProductionTransitionPrerequisiteSet.

## 2. Principal separation

`live validity record != C1 prerequisite input != prerequisite set != eligibility decision != permission != authority != execution`

A2 is a typed conversion boundary, not a decision layer.

## 3. Trusted input requirement

The adapter consumes only accepted A1 restricted-origin records through the parent ProductionTransitionLiveValidityEvidence. It must not accept raw bools, caller-provided request bindings, state versions, relationship generations, arbitrary snapshots, or two loose records. The C1 context is copied from each trusted A1 record's exact binding().

## 4. Exact semantic mapping

Freshness mapping: context = record.binding(); satisfied = record.satisfied().

Revalidation mapping: context = record.binding(); satisfied = record.satisfied().

The adapter does not recompute either decision, inspect live mesh state, or reinterpret observed state/generation fields.

## 5. Provenance preservation

C1 prerequisite types intentionally contain only exact request context plus a satisfied flag. A2 preserves A1 provenance in a wrapper containing LiveValidityDecisionId, FreshnessDecisionId, RevalidationDecisionId, FreshnessPrerequisiteEvidence, and RevalidationPrerequisiteEvidence.

## 6. Parent A1 coherence

Preferred API: `std::optional<ProductionLiveValidityC1Prerequisites> convert(const ProductionTransitionLiveValidityEvidence& evidence) const;`

The parent wrapper proves freshness and revalidation came from one coherent A1 evaluation event. No V1 overload accepting two independently supplied records is allowed.

## 7. C1 construction seam

Current C1 prerequisite constructors are private and friend only detail::ProductionTransitionConstructionAccess, which is a test seam. A2 must not reuse that generic test access in production.

Introduce `detail::ProductionLiveValidityC1ConversionAccess`.

This access may be friend only to FreshnessPrerequisiteEvidence and RevalidationPrerequisiteEvidence. It must not be friend to PermissionPrerequisiteEvidence, InvariantPrerequisiteEvidence, ResiliencePrerequisiteEvidence, ProductionTransitionEligibilityDecision, request identity/version/class/binding constructors, or authority/capability/execution types.

## 8. Narrow internal API

The internal access exposes exactly two functions: freshness(context, satisfied) and revalidation(context, satisfied). No generic prerequisite factory or template capable of constructing arbitrary evidence is allowed.

## 9. No prerequisite-set assembly

A2 must not return ProductionTransitionPrerequisiteSet and must not insert outputs into such a set internally. A full set carries permission/invariant/resilience semantics that A2 does not own.

## 10. No eligibility evaluation

A2 must not instantiate ProductionTransitionEligibilityEvaluator, return ProductionTransitionEligibilityDecision, expose eligible_for_authority_consideration, infer missing prerequisites as false, or synthesize permission/invariant/resilience evidence. Terminal boundary: two C1 prerequisite evidence values -> STOP.

## 11. Binding equality

Before conversion the adapter verifies: evidence.freshness().binding() == evidence.revalidation().binding() == evidence.request().binding(). Conversion precondition failure returns std::nullopt and emits no partial C1 output.

## 12. Decision IDs

A2 does not need a new random decision ID in V1 because it performs deterministic lossless projection of already identified trusted decisions. The wrapper retains parent LiveValidityDecisionId, FreshnessDecisionId, and RevalidationDecisionId.

## 13. Dependency placement

Preferred project: `apps/soam-transition-live-validity-c1/`. Target: `soam_transition_live_validity_c1` / `AdaptiveMesh::soam_transition_live_validity_c1`.

Dependencies are one-way: adapter -> AdaptiveMesh::soam_transition_live_validity and adapter -> soam_transition_eligibility. No dependency from runtime, transition-request, or live-validity back to the adapter.

## 14. C1 header modification

The only accepted C1 production change for A2 is a forward declaration of detail::ProductionLiveValidityC1ConversionAccess and friend declarations on exactly FreshnessPrerequisiteEvidence and RevalidationPrerequisiteEvidence. No constructor visibility, eligibility ordering, rejection-reason, permission/invariant/resilience, request-binding, or eligibility-decision changes.

## 15. Compile-fail requirements

A2 acceptance must prove arbitrary callers cannot directly construct Freshness/Revalidation evidence; use A2 access to construct Permission/Invariant/Resilience evidence; construct request bindings; create a full prerequisite set through the adapter; invoke eligibility through the adapter; obtain authority/capability/execution objects; or convert independently supplied/mixed A1 records.

## 16. Positive verification

Tests must prove true/false A1 freshness map exactly to C1 freshness; true/false A1 revalidation map exactly to C1 revalidation; both C1 contexts equal the exact accepted request binding; all three A1 decision IDs are preserved; repeated conversion is deterministic; no mesh read occurs; no random ID is minted; and no eligibility evaluation occurs.

## 17. Regression boundary

A2 CI must compose and keep green runtime, D7/D8/D9, transition request, A1 Live Validity, C1 eligibility contract tests, A2 conversion tests, and A2 compile-fail tests.

## 18. Security property

`freshness=true && revalidation=true` does not imply eligible, permitted, authorized, or executable. It means only that two of five C1 prerequisite inputs can now be represented from trusted production evidence.

## 19. A2 STOP

After successful conversion the system possesses only FreshnessPrerequisiteEvidence and RevalidationPrerequisiteEvidence. It still lacks independently accepted production sources for PermissionPrerequisiteEvidence, InvariantPrerequisiteEvidence, and ResiliencePrerequisiteEvidence. Therefore C1 production eligibility integration remains intentionally incomplete.

A2 ends here.


## 20. Critical review: restricted-origin wrapper

The output wrapper itself must not become an alternate public construction path for C1 evidence. ProductionLiveValidityC1Prerequisites must have no public default constructor, no public constructor accepting C1 evidence, construction reserved to ProductionLiveValidityC1Adapter, and copy/move allowed only after a trusted instance exists.

## 21. No runtime dependency

A2 performs no live-state acquisition. Normative direct dependencies are only AdaptiveMesh::soam_transition_live_validity and soam_transition_eligibility. The adapter has no mesh, snapshot-source, lifecycle lease, clock, RNG, filesystem, network, or authority dependency.

## 22. Deterministic conversion invariant

For one accepted A1 evidence object E, converted freshness context equals E.request().binding(), converted freshness value equals E.freshness().satisfied(), converted revalidation context equals E.request().binding(), and converted revalidation value equals E.revalidation().satisfied(). Repeated conversion preserves identical semantics and the same referenced A1 decision IDs.

## 23. No semantic promotion

Conversion into C1 input types does not upgrade A1 evidence into permission, full prerequisite satisfaction, eligibility, authorization, capability, or execution. A2 is representation adaptation only.

## 24. Friend-surface audit requirement

Before implementation acceptance, detail::ProductionLiveValidityC1ConversionAccess must appear as friend exactly twice in the C1 contract: FreshnessPrerequisiteEvidence and RevalidationPrerequisiteEvidence. It must appear nowhere else. Compile-fail tests should target the access class itself where practical, not merely already-private constructors.

## 25. Accepted implementation placement

Reviewed placement: apps/soam-transition-live-validity-c1/. Expected surface: CMakeLists.txt, include/production_live_validity_c1_adapter.hpp, src/detail/production_live_validity_c1_conversion_access.hpp, src/production_live_validity_c1_adapter.cpp, tests, compile-fail tests, and .github/workflows/soam-live-validity-c1-validation.yml.

The only existing production file expected to change is apps/soam-transition/include/production_transition_eligibility.hpp, and only for the narrow forward declaration plus two friend declarations.

## 26. Critical review disposition

A2 implementation acceptance required the exact two-friend seam, restricted-origin wrapper, deterministic exact-value projection, no loose-record overload, no prerequisite-set construction, no eligibility API, no permission/invariant/resilience construction, no authority/execution surface, and green composed regression CI on the accepted implementation head.


## 27. Accepted A2 implementation provenance

The A2 implementation was accepted separately from this design branch.

Acceptance evidence:

- design PR: #25;
- implementation PR: #26;
- accepted implementation head: `154f030ab59be4c9017ecc3f611d2af23bd7a311`;
- accepted implementation merge/current baseline: `28f2370f3b8f2c2474bfbbdfc67474ceafc87db8`;
- exact-head validation completed successfully for Transition Eligibility, Transition Request, Transition Live Validity, and Live Validity C1 Adapter workflows.

Accepted runtime boundary:

`ProductionTransitionLiveValidityEvidence -> FreshnessPrerequisiteEvidence + RevalidationPrerequisiteEvidence -> STOP`

The accepted implementation preserves the reviewed constraints:

- the output wrapper is restricted-origin;
- parent LiveValidity/Freshness/Revalidation decision IDs are preserved;
- conversion is deterministic and does not mint a new A2 decision identity;
- C1 production friendship is granted only to FreshnessPrerequisiteEvidence and RevalidationPrerequisiteEvidence;
- no loose-record conversion overload exists;
- no Permission, Invariant, or Resilience prerequisite evidence is constructed;
- no ProductionTransitionPrerequisiteSet is assembled;
- no ProductionTransitionEligibilityEvaluator call occurs;
- no authority, capability, or execution surface is introduced.

## 28. Post-acceptance status

A2 runtime is complete and accepted in main.

Remaining prerequisite channels are independent work items:

- B1 Invariant evidence;
- C Resilience evidence;
- D Permission attestation/evidence;
- E prerequisite-set assembly;
- F C1 production eligibility integration.

This document now serves as durable design and audit provenance for the accepted A2 layer.
