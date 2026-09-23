# SOAM 2.0 — Resilience to C1 Adapter Design Contract

## 0. Status

- Layer: C.2 — accepted C resilience evidence -> accepted C1 resilience
           prerequisite input
- Document class: design-contract candidate
- Scope: ProductionResiliencePrerequisiteRecord
         -> ResiliencePrerequisiteEvidence
- Upstream source: C resilience prerequisite record
                  (ratified C design)
- Upstream design reference:
                  docs/policy/accepted/soam-2.0-resilience-evidence/
                    DESIGN_CONTRACT.md
                  §§43, 48, 52 (ratified)
- Downstream contract: C1 ResiliencePrerequisiteEvidence
                       (accepted)
- Cross references:
                  accepted transition-pipeline contract
                    §§7, 8, 10, 11
                  accepted B1 adapter contract
                    §§9, 22
- Acceptance status: design-review pending
- Authority effect: none
- Origin baseline: main @ 0a371582f612d406e2c118c162e4f1fca04fda20
- Proposed module placement: apps/soam-transition-resilience-c1/
- Proposed target: soam_transition_resilience_c1
                  (provisional; subject to ratification)

## 1. Boundary

This adapter introduces exactly one conversion boundary:

  ProductionResiliencePrerequisiteRecord
      ->
  ResiliencePrerequisiteEvidence
      ->
  STOP

The adapter consumes only a successfully produced C
"ProductionResiliencePrerequisiteRecord".

It does not accept:

  - "ProductionTransitionResilienceResult";
  - "ProductionTransitionResilienceRejection";
  - raw booleans;
  - caller-supplied request bindings;
  - policy snapshot identifiers supplied independently from the record;
  - diagnostic fields supplied independently from the record.

A C rejection does not enter this adapter.

The adapter must not translate a C rejection into
ResiliencePrerequisiteEvidence{..., false}.

When C evaluation does not produce a prerequisite record, the downstream
composition layer represents the corresponding C1 prerequisite slot as
absent.

## 2. Principal separation

  C resilience evaluation
    != C resilience prerequisite record
    != C -> C1 conversion
    != C1 prerequisite set
    != C1 eligibility decision
    != permission
    != authority
    != execution

This adapter is a representation boundary.

It is not:

  - a resilience evaluator;
  - a prerequisite-set assembler;
  - an eligibility evaluator;
  - a decision-reason translator;
  - a permission source;
  - an authority source;
  - an execution surface.

## 3. Trusted input requirement

The adapter accepts only an already constructed
"ProductionResiliencePrerequisiteRecord".

That record is restricted-origin upstream evidence produced by the
accepted C evaluator.

The adapter must not expose any overload that allows a caller to
provide independently:

  - "ProductionTransitionRequestBinding";
  - "bool satisfied";
  - "ResilienceDecisionId";
  - "ResiliencePolicySnapshotId";
  - "alternativePathHopCount";
  - any other diagnostic field.

The exact C1 context and satisfied value are derived only from the
accepted upstream record.

## 4. Exact semantic mapping

The conversion is:

  ResiliencePrerequisiteEvidence.context
      = record.binding()

  ResiliencePrerequisiteEvidence.satisfied
      = record.satisfied()

No other C field changes C1 prerequisite semantics.

The adapter must not recompute resilience satisfaction.

The adapter must not inspect runtime mesh state.

The adapter must not re-evaluate hop count or alternative path.

The adapter must not derive a new decision identity.

## 5. Lossy projection boundary

"ProductionResiliencePrerequisiteRecord" contains more information than
"ResiliencePrerequisiteEvidence".

Upstream information includes, among other fields:

  - "ResilienceDecisionId";
  - "ResiliencePolicySnapshotId";
  - "alternativePathHopCount".

C1 "ResiliencePrerequisiteEvidence" contains only:

  ProductionTransitionRequestBinding
  bool satisfied

Therefore C -> C1 conversion is intentionally lossy with respect to
diagnostic and provenance detail.

The loss is bounded and explicit.

No omitted field is re-encoded into C1 evidence.

## 6. Diagnostic-only field semantics

"alternativePathHopCount" is a diagnostic field.

Its semantics are defined normatively by the accepted C design:

  - §43: shortest alternative-path hop count may be exposed as a
         diagnostic; it does not participate in satisfaction;
  - §52: hop count is diagnostic-only.

Therefore:

  - hop count does not affect C1 prerequisite satisfaction;
  - hop count does not affect C1 eligibility;
  - hop count is not projected into C1 evidence;
  - hop count remains available to direct callers through the upstream
    "ProductionResiliencePrerequisiteRecord".

C -> C1 conversion does not re-interpret hop count.

## 7. Adapter output

Preferred public API:

    class ProductionResilienceC1Adapter final {
    public:
        [[nodiscard]]
        ResiliencePrerequisiteEvidence
        convert(
            const ProductionResiliencePrerequisiteRecord& record)
            const noexcept;
    };

The conversion is total for a valid restricted-origin
"ProductionResiliencePrerequisiteRecord".

No adapter-local rejection type is introduced.

No "std::optional" result is required by this contract because the
adapter does not accept independently supplied values whose coherence
must be re-established.

Fallibility rationale

  A2 conversion is fallible because it converts two restricted-origin
  child records and must validate binding coherence between those
  records and their parent request before emitting C1 evidence.

  C conversion consumes exactly one restricted-origin
  "ProductionResiliencePrerequisiteRecord".

  Its request binding and resilience result belong to one record
  constructed by the accepted C evaluator. There is no cross-record
  coherence relationship for the adapter to re-establish.

  Therefore C conversion is total for a valid C prerequisite record.

If a future implementation discovers a concrete conversion-local
failure that cannot be represented by the upstream C result model,
that requires an explicit design amendment before changing the public
outcome type.

## 8. Traceability note

The adapter returns naked "ResiliencePrerequisiteEvidence".

"ResilienceDecisionId", "ResiliencePolicySnapshotId", and
"alternativePathHopCount" remain in the upstream
"ProductionResiliencePrerequisiteRecord" and do not appear on the
adapter output surface.

This follows the same structural choice as the accepted B1 adapter
contract §6.

B1 and C both convert one coherent prerequisite record into one C1
prerequisite evidence value. Both return naked evidence. Both leave
decision lineage and diagnostic detail upstream.

A2's wrapper pattern is not required here. A2 converts two coherent
child records together and preserves their evaluation lineage in a
restricted-origin container; C converts one already coherent record
into one C1 evidence value.

Traceability scope

  This applies to direct adapter callers.

  Pipeline callers cannot access C decision lineage, policy provenance,
  or hop count through the pipeline outcome in v0. That limitation is
  defined by the accepted transition-pipeline contract §11,
  "Observability limitation".

  Direct adapter callers, outside the pipeline, may retain the
  upstream "ProductionResiliencePrerequisiteRecord" when they require
  C decision lineage, policy provenance, or hop-count diagnostics.

## 9. C1 construction-seam convention compliance

This adapter does not re-proclaim the per-adapter narrow production
construction seam convention.

That convention is ratified in the accepted B1 design contract §9,
and its non-semantic inheritance rule is ratified in §22.

C adopts the convention without deviation.

C introduces no generic production C1 evidence construction access.

C introduces no additional production friend seam beyond the one
required for "ResiliencePrerequisiteEvidence".

C does not widen "detail::ProductionTransitionConstructionAccess"
into production use.

## 10. Narrow construction authority

"detail::ProductionResilienceC1ConversionAccess" may construct only
"ResiliencePrerequisiteEvidence".

Its internal API should expose exactly the construction operation
required by C conversion, equivalent to:

    static constexpr ResiliencePrerequisiteEvidence resilience(
        ProductionTransitionRequestBinding context,
        bool satisfied) noexcept;

It must not construct:

  - "PermissionPrerequisiteEvidence";
  - "InvariantPrerequisiteEvidence";
  - "FreshnessPrerequisiteEvidence";
  - "RevalidationPrerequisiteEvidence";
  - "ProductionTransitionRequestBinding";
  - request identity/version/class primitives;
  - "ProductionTransitionEligibilityDecision";
  - authority, capability, or execution types.

## 11. Structural effect on the accepted C1 header

C implementation will require a bounded production change to:

  apps/soam-transition/include/production_transition_eligibility.hpp

The permitted change is limited to:

  1. one forward declaration:

       namespace detail {
       class ProductionResilienceC1ConversionAccess;
       }

  2. one friend declaration inside "ResiliencePrerequisiteEvidence":

       friend class detail::ProductionResilienceC1ConversionAccess;

  3. an organizational comment may be added to group production
     adapter conversion seams, provided it changes no semantics.

No other C1 contract change is authorized by this design.

In particular, C must not modify:

  - constructor visibility;
  - prerequisite-set shape;
  - C1 evaluation ordering;
  - rejection reasons;
  - binding-mismatch semantics;
  - missing-prerequisite semantics;
  - permission semantics;
  - invariant semantics;
  - freshness semantics;
  - revalidation semantics;
  - eligibility decision construction.

The C1 header modification belongs to the C implementation PR.

This design-contract PR itself is documentation-only.

## 12. Relation to ratified C design contract

This contract opens the deferred C -> C1 adapter gate that was
explicitly designed as a separate boundary in the accepted C design
contract §48.

This contract does not:

  - re-interpret C resilience semantics;
  - amend §43 diagnostic-only hop-count rule;
  - amend §52 final accepted C V1 semantics;
  - extend the C rejection taxonomy;
  - re-define C operational edge criteria.

It uses §43 and §52 as external ratified references.

It uses §48 as the deferred-gate origin that this contract now
activates in design form.

## 13. Dependency placement

Proposed module:

  apps/soam-transition-resilience-c1/

Proposed target:

  soam_transition_resilience_c1

Direct dependencies:

  AdaptiveMesh::soam_transition_resilience
  soam_transition_eligibility

Dependency direction:

  soam_transition_resilience_c1
      -> soam_transition_resilience
      -> existing upstream dependencies

  soam_transition_resilience_c1
      -> soam_transition_eligibility

No dependency may be introduced from:

  - "soam_transition_eligibility" back to the adapter module;
  - "soam_transition_resilience" back to the adapter module;
  - "soam_runtime" back to the adapter module.

The adapter is a downstream sibling composition boundary.

## 14. No prerequisite-set assembly

The adapter must not construct or return
"ProductionTransitionPrerequisiteSet".

It owns only the resilience prerequisite representation boundary.

Insertion into the C1 resilience slot belongs to the higher
composition layer.

## 15. No eligibility evaluation

The adapter must not instantiate or invoke
"ProductionTransitionEligibilityEvaluator".

It must not return "ProductionTransitionEligibilityDecision".

It must not expose:

  eligible_for_authority_consideration

or any C1 rejection reason as its own result.

Its terminal successful output is:

  ResiliencePrerequisiteEvidence

and then STOP.

## 16. No runtime or evaluator recomputation

The adapter performs no:

  - topology capture;
  - mesh read;
  - policy lookup;
  - alternative-path search;
  - hop-count computation;
  - resilience re-evaluation;
  - request-lineage validation beyond what is already represented
    by the restricted-origin record;
  - random decision-ID generation.

It consumes the record as trusted upstream evidence.

## 17. Deterministic conversion invariant

For every accepted C prerequisite record "R":

  convert(R).context()   == R.binding()

  convert(R).satisfied() == R.satisfied()

Repeated conversion of the same record must preserve identical C1
semantics.

No adapter-local mutable state is required.

## 18. Compile-fail requirements

Implementation acceptance must prove that arbitrary callers cannot:

  - directly construct "ResiliencePrerequisiteEvidence";
  - use "ProductionResilienceC1ConversionAccess" to construct any
    non-resilience C1 prerequisite evidence;
  - construct request bindings through the C adapter;
  - obtain "ProductionTransitionPrerequisiteSet" from the adapter;
  - invoke C1 eligibility through the adapter;
  - obtain authority, capability, or execution objects through the
    adapter.

Existing compile-fail protection around direct construction of C1
resilience evidence must remain effective.

The accepted C test surface already contains
"apps/soam-transition-resilience/tests/compile_fail/construct_c1_resilience.cpp"
and this protection must continue to hold after the adapter is
introduced.

C implementation must add seam-specific negative coverage so that
"ProductionResilienceC1ConversionAccess" cannot construct unrelated
evidence types.

## 19. Positive verification

Implementation tests must prove:

  - "satisfied=true" maps exactly to C1 resilience satisfied=true;
  - "satisfied=false" maps exactly to C1 resilience satisfied=false;
  - converted context equals the exact upstream C binding;
  - no C diagnostic field changes conversion output;
  - "alternativePathHopCount" does not affect C1 evidence;
  - repeated conversion is deterministic;
  - the adapter does not invoke eligibility;
  - the adapter does not access runtime state;
  - no new decision ID is minted;
  - C rejection cannot be passed to the adapter;
  - the production conversion seam cannot construct unrelated
    prerequisite types.

## 20. Regression boundary

Implementation acceptance must keep green the existing relevant
regression surface, including:

  - C1 transition eligibility tests;
  - transition-pipeline tests, if such an implementation/test surface
    exists at the time of C adapter implementation acceptance;
  - C resilience tests;
  - C compile-fail tests;
  - B1 invariant tests (composition overlap);
  - A1/A2 tests (composition overlap);
  - the new C -> C1 adapter tests and compile-fail tests.

The adapter must not alter accepted C evaluator semantics or accepted
C1 eligibility semantics.

## 21. Security property

Successful conversion means only:

  trusted C resilience evidence
      ->
  representable C1 resilience prerequisite evidence

It does not mean:

  all prerequisites present
  permission granted
  eligible
  authorized
  capability issued
  executable

A converted "ResiliencePrerequisiteEvidence{satisfied=true}" is
exactly one of five C1 prerequisite inputs.

## 22. Convention compliance and non-semantic inheritance

This contract adopts the per-adapter narrow production construction
seam convention without deviation, as stated in §9.

C defines no provenance container.

Its output is naked "ResiliencePrerequisiteEvidence", as specified in
§7.

This contract does not define:

  - permission mapping semantics;
  - invariant mapping semantics;
  - future adapter outcome algebra;
  - provenance-container requirements for other adapters;
  - generic adapter framework semantics.

Those remain independent design matters.

## 23. Expected implementation surface

After ratification, the expected implementation workstream is bounded
to a new sibling module plus the narrow C1 seam extension.

Expected additions:

  apps/soam-transition-resilience-c1/CMakeLists.txt
  apps/soam-transition-resilience-c1/include/production_resilience_c1_adapter.hpp
  apps/soam-transition-resilience-c1/src/production_resilience_c1_adapter.cpp
  apps/soam-transition-resilience-c1/src/detail/production_resilience_c1_conversion_access.hpp
  apps/soam-transition-resilience-c1/tests/...

Expected existing-file change:

  apps/soam-transition/include/production_transition_eligibility.hpp

limited exactly as defined in §11.

Build/workflow files required specifically to test the new module may
be added by the implementation workstream.

No implementation artifact is created by acceptance of this design
contract.

## 24. Governance separation

  design candidate
    != ratified design
    != implementation
    != implementation acceptance

Ratification of this document would establish only the accepted
C -> C1 adapter design contract.

It would not:

  - create "apps/soam-transition-resilience-c1/";
  - change "production_transition_eligibility.hpp";
  - register a CMake target;
  - change runtime behavior;
  - constitute implementation acceptance.

Ratification of this document does not affect whether C is reachable
through the transition pipeline.

No transition-pipeline implementation currently exists.

C reachability through that pipeline depends on the separate
transition-pipeline implementation workstream, including its eventual
composition of all required prerequisite channels.

The C adapter implementation remains a separate PR and separate
exact-head acceptance event.

## 25. STOP boundary

After successful adapter conversion, the system possesses:

  ResiliencePrerequisiteEvidence

and nothing more from this adapter.

The adapter does not assemble the five-prerequisite set.

The adapter does not invoke C1.

The adapter does not infer the state of missing channels.

The adapter does not create terminal pipeline semantics.

The adapter does not grant authority or execution.

C -> C1 conversion ends here.
