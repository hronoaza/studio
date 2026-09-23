# SOAM 2.0 — Invariant to C1 Adapter Design Contract

## 0. Status

- Layer: B1.2 — accepted B1 Invariant Evidence -> accepted C1 Invariant prerequisite input
- Document class: concrete design contract
- Scope: `ProductionInvariantPrerequisiteRecord` -> `InvariantPrerequisiteEvidence`
- Upstream source: B1 invariant prerequisite record
- Downstream contract: C1 `InvariantPrerequisiteEvidence`
- Acceptance status: accepted
- Authority effect: none
- Module placement: `apps/soam-transition-invariant-c1/`; target `soam_transition_invariant_c1`
- Origin baseline: main @ `192708ee9dbffe2e52f543c2f906bada215cc18c`

## 1. Boundary

This adapter introduces exactly one conversion boundary:

```text
ProductionInvariantPrerequisiteRecord
    ->
InvariantPrerequisiteEvidence
    ->
STOP
```

The adapter consumes only a successfully produced B1
`ProductionInvariantPrerequisiteRecord`.

It does not accept:

- `ProductionTransitionInvariantResult`;
- `ProductionTransitionInvariantRejection`;
- raw booleans;
- caller-supplied request bindings;
- policy snapshot identifiers supplied independently from the record;
- diagnostic fields supplied independently from the record.

A B1 rejection does not enter this adapter.

When B1 evaluation does not produce a prerequisite record, the downstream
composition layer represents the corresponding C1 prerequisite slot as absent.

## 2. Principal separation

```text
B1 invariant evaluation
!= B1 prerequisite record
!= B1 -> C1 conversion
!= C1 prerequisite set
!= C1 eligibility decision
!= permission
!= authority
!= execution
```

This adapter is a representation boundary.

It is not:

- an invariant evaluator;
- a prerequisite-set assembler;
- an eligibility evaluator;
- a decision-reason translator;
- a permission source;
- an authority source;
- an execution surface.

## 3. Trusted input requirement

The adapter accepts only an already constructed
`ProductionInvariantPrerequisiteRecord`.

That record is restricted-origin upstream evidence produced by the accepted B1
evaluator.

The adapter must not expose any overload that allows a caller to provide
independently:

- `ProductionTransitionRequestBinding`;
- `bool satisfied`;
- `InvariantDecisionId`;
- `InvariantPolicySnapshotId`;
- projected capacities;
- individual invariant diagnostic booleans.

The exact C1 context and satisfied value are derived only from the accepted
upstream record.

## 4. Exact semantic mapping

The conversion is:

```text
InvariantPrerequisiteEvidence.context
    = record.binding()

InvariantPrerequisiteEvidence.satisfied
    = record.satisfied()
```

No other B1 field changes C1 prerequisite semantics.

The adapter must not recompute invariant satisfaction.

The adapter must not inspect runtime mesh state.

The adapter must not reinterpret individual B1 diagnostic fields.

The adapter must not derive a new decision identity.

## 5. Lossy projection boundary

`ProductionInvariantPrerequisiteRecord` contains more information than
`InvariantPrerequisiteEvidence`.

Upstream information includes, among other fields:

- `InvariantDecisionId`;
- `InvariantPolicySnapshotId`;
- source identity invariant result;
- target identity invariant result;
- structural relationship invariant result;
- forward projected capacity;
- reverse projected capacity;
- forward effective-coupling validity;
- reverse effective-coupling validity.

C1 `InvariantPrerequisiteEvidence` contains only:

```text
ProductionTransitionRequestBinding
bool satisfied
```

Therefore B1 -> C1 conversion is intentionally lossy with respect to diagnostic
and provenance detail.

The loss is bounded and explicit.

No omitted field is re-encoded into C1 evidence.

## 6. Traceability note

The adapter returns naked `InvariantPrerequisiteEvidence`.

`InvariantDecisionId`, `InvariantPolicySnapshotId`, and B1 diagnostic fields
remain in the upstream `ProductionInvariantPrerequisiteRecord` and do not appear
on the adapter output surface.

This is a deliberate departure from the accepted A2 adapter pattern, where a
restricted-origin wrapper retains parent and child decision identifiers.

The difference is intentional.

A2 converts two coherent prerequisite records simultaneously and requires a
container that preserves their common evaluation lineage.

B1 converts one already coherent prerequisite record into one C1 prerequisite
evidence value.

The accepted transition-pipeline contract materializes prerequisite evidence
objects directly into C1 slots and does not carry adapter-local provenance
containers.

### Traceability scope

This applies to direct adapter callers.

Pipeline callers cannot access B1 decision lineage through the pipeline outcome
in v0. That limitation is defined by the accepted transition-pipeline contract
§11, `Observability limitation`.

Direct adapter callers, outside the pipeline, may retain the upstream
`ProductionInvariantPrerequisiteRecord` when they require B1 decision lineage,
policy provenance, or diagnostic detail.

The B1 adapter does not duplicate that record's provenance into a second output
container.

## 7. Adapter output

Preferred public API:

```cpp
class ProductionInvariantC1Adapter final {
public:
    [[nodiscard]]
    InvariantPrerequisiteEvidence
    convert(
        const ProductionInvariantPrerequisiteRecord& record) const noexcept;
};
```

The conversion is total for a valid restricted-origin
`ProductionInvariantPrerequisiteRecord`.

No adapter-local rejection type is introduced.

No `std::optional` result is required by this contract because the adapter does
not accept independently supplied values whose coherence must be re-established.

### Fallibility rationale

A2 conversion is fallible because it converts two restricted-origin child
records and must validate binding coherence between those records and their
parent request before emitting C1 evidence.

B1 conversion consumes exactly one restricted-origin
`ProductionInvariantPrerequisiteRecord`.

Its request binding and invariant result belong to one record constructed by the
accepted B1 evaluator. There is no cross-record coherence relationship for the
adapter to re-establish.

Therefore B1 conversion is total for a valid B1 prerequisite record.

If a future implementation discovers a concrete conversion-local failure that
cannot be represented by the upstream B1 result model, that requires an explicit
design amendment before changing the public outcome type.

## 8. Rejection handling

`ProductionTransitionInvariantRejection` is outside the adapter input domain.

The adapter must not translate a B1 rejection into:

```text
InvariantPrerequisiteEvidence{..., false}
```

and must not synthesize any other C1 evidence from a rejection.

The accepted pipeline semantics are:

```text
successful B1 record
    -> adapter conversion
    -> InvariantPrerequisiteEvidence
    -> C1 invariant slot present

B1 result without prerequisite record
    -> adapter not invoked
    -> C1 invariant slot absent
    -> C1 owns terminal diagnosis
```

This preserves:

```text
absence of evidence != present unsatisfied evidence
```

## 9. C1 construction-seam convention

`detail::ProductionTransitionConstructionAccess` is the generic C1 test
construction seam.

It is not a production adapter construction surface.

Production adapters must not reuse it.

The accepted A2 design already introduced the adapter-specific narrow production
construction seam pattern through:

```cpp
detail::ProductionLiveValidityC1ConversionAccess
```

This contract codifies that existing pattern as an explicit normative convention
and applies it to B1.

The convention is:

```text
one production adapter boundary
    ->
one narrowly scoped adapter-specific detail access
    ->
friend access only to the C1 evidence type or types owned by that adapter
```

For B1 the production seam is:

```cpp
detail::ProductionInvariantC1ConversionAccess
```

This contract does not invent the convention.

It makes the accepted A2 structural practice explicit and normatively binding for
B1 and for subsequent adapters that choose to reuse this structural pattern.

This contract does not authorize a generic production construction access for
all C1 prerequisite evidence types.

This contract does not widen
`detail::ProductionTransitionConstructionAccess` into production use.

The convention is structural only. It does not impose B1 semantic choices on
other prerequisite channels.

## 10. Narrow construction authority

`detail::ProductionInvariantC1ConversionAccess` may construct only
`InvariantPrerequisiteEvidence`.

Its internal API should expose exactly the construction operation required by
B1 conversion, equivalent to:

```cpp
static constexpr InvariantPrerequisiteEvidence invariant(
    ProductionTransitionRequestBinding context,
    bool satisfied) noexcept;
```

It must not construct:

- `PermissionPrerequisiteEvidence`;
- `ResiliencePrerequisiteEvidence`;
- `FreshnessPrerequisiteEvidence`;
- `RevalidationPrerequisiteEvidence`;
- `ProductionTransitionRequestBinding`;
- request identity/version/class primitives;
- `ProductionTransitionEligibilityDecision`;
- authority, capability, or execution types.

## 11. Structural effect on the accepted C1 header

B1 implementation will require a bounded production change to:

```text
apps/soam-transition/include/production_transition_eligibility.hpp
```

The permitted change is limited to:

1. one forward declaration:

```cpp
namespace detail {
class ProductionInvariantC1ConversionAccess;
}
```

2. one friend declaration inside `InvariantPrerequisiteEvidence`:

```cpp
friend class detail::ProductionInvariantC1ConversionAccess;
```

3. an organizational comment may be added to group production adapter conversion
   seams, provided it changes no semantics.

No other C1 contract change is authorized by this design.

In particular, B1 must not modify:

- constructor visibility;
- prerequisite-set shape;
- C1 evaluation ordering;
- rejection reasons;
- binding-mismatch semantics;
- missing-prerequisite semantics;
- permission semantics;
- resilience semantics;
- freshness semantics;
- revalidation semantics;
- eligibility decision construction.

The C1 header modification belongs to the B1 implementation PR.

This design-contract PR itself is documentation-only.

## 12. C1 header organization

Production conversion seams should remain visibly grouped in the C1 header.

The intended organization immediately after B1 implementation is conceptually:

```cpp
namespace detail {

// Test-only construction seam.
class ProductionTransitionConstructionAccess;

// Production adapter conversion seams.
// Each seam grants construction authority only to its owned C1 evidence type(s).
class ProductionLiveValidityC1ConversionAccess;
class ProductionInvariantC1ConversionAccess;

}
```

This example shows the expected organization immediately after B1 implementation.

It is not the final set of production conversion seams.

Future adapters may add their own narrowly scoped conversion seams following the
same structural pattern. The grouping comment is expected to accommodate such
extensions without changing the distinction between test-only and production
construction authority.

The exact comment wording is implementation-level formatting, not semantic API.

The structural invariants are normative:

```text
test construction access
!= production adapter conversion access
```

and:

```text
one production seam must not acquire unrelated evidence-construction authority
```

## 13. Dependency placement

Proposed module:

```text
apps/soam-transition-invariant-c1/
```

Proposed target:

```text
soam_transition_invariant_c1
AdaptiveMesh::soam_transition_invariant_c1
```

Direct dependencies:

```text
AdaptiveMesh::soam_transition_invariant
soam_transition_eligibility
```

Dependency direction:

```text
soam_transition_invariant_c1
    -> soam_transition_invariant
    -> existing upstream dependencies

soam_transition_invariant_c1
    -> soam_transition_eligibility
```

No dependency may be introduced from:

- `soam_transition_eligibility` back to the adapter module;
- `soam_transition_invariant` back to the adapter module;
- `soam_runtime` back to the adapter module.

The adapter is a downstream sibling composition boundary.

## 14. No prerequisite-set assembly

The adapter must not construct or return
`ProductionTransitionPrerequisiteSet`.

It owns only the invariant prerequisite representation boundary.

Insertion into the C1 invariant slot belongs to the higher composition layer.

## 15. No eligibility evaluation

The adapter must not instantiate or invoke
`ProductionTransitionEligibilityEvaluator`.

It must not return `ProductionTransitionEligibilityDecision`.

It must not expose:

```text
eligible_for_authority_consideration
```

or any C1 rejection reason as its own result.

Its terminal successful output is:

```text
InvariantPrerequisiteEvidence
```

and then STOP.

## 16. No runtime or evaluator recomputation

The adapter performs no:

- topology capture;
- mesh read;
- policy lookup;
- invariant projection;
- capacity computation;
- effective-coupling computation;
- request-lineage validation beyond what is already represented by the
  restricted-origin record;
- random decision-ID generation.

It consumes the record as trusted upstream evidence.

## 17. Deterministic conversion invariant

For every accepted B1 prerequisite record `R`:

```text
convert(R).context() == R.binding()

convert(R).satisfied() == R.satisfied()
```

Repeated conversion of the same record must preserve identical C1 semantics.

No adapter-local mutable state is required.

## 18. Compile-fail requirements

Implementation acceptance must prove that arbitrary callers cannot:

- directly construct `InvariantPrerequisiteEvidence`;
- use `ProductionInvariantC1ConversionAccess` to construct any non-invariant C1
  prerequisite evidence;
- construct request bindings through the B1 adapter;
- obtain `ProductionTransitionPrerequisiteSet` from the adapter;
- invoke C1 eligibility through the adapter;
- obtain authority, capability, or execution objects through the adapter.

B1 implementation must ensure that compile-fail protection around direct
construction of C1 invariant evidence is effective.

If such protection already exists in the accepted C1 or B1 regression surface,
it must remain in force.

If it does not exist, the B1 implementation workstream must introduce it.

Compile-fail coverage must target the production conversion seam itself where
practical, not merely rely on the already-private evidence constructor.

## 19. Positive verification

Implementation tests must prove:

- `satisfied=true` maps exactly to C1 invariant `satisfied=true`;
- `satisfied=false` maps exactly to C1 invariant `satisfied=false`;
- converted context equals the exact upstream B1 binding;
- no B1 diagnostic field changes conversion output;
- repeated conversion is deterministic;
- the adapter does not invoke eligibility;
- the adapter does not access runtime state;
- no new decision ID is minted;
- B1 rejection cannot be passed to the adapter;
- the production conversion seam cannot construct unrelated prerequisite types.

## 20. Regression boundary

Implementation acceptance must keep green the existing relevant regression
surface, including:

- C1 transition eligibility tests;
- transition-request tests;
- B1 invariant tests;
- B1 compile-fail tests;
- A1/A2 tests where dependency/build integration overlaps;
- the new B1 -> C1 adapter tests and compile-fail tests.

The adapter must not alter accepted B1 evaluator semantics or accepted C1
eligibility semantics.

## 21. Security property

Successful conversion means only:

```text
trusted B1 invariant evidence
    ->
representable C1 invariant prerequisite evidence
```

It does not mean:

```text
all prerequisites present
permission granted
eligible
authorized
capability issued
executable
```

A converted `InvariantPrerequisiteEvidence{satisfied=true}` is exactly one of five
C1 prerequisite inputs.

## 22. Structural precedent and non-semantic inheritance

This contract codifies the per-adapter narrow production construction seam as the
accepted structural pattern for B1 C1-prerequisite conversion.

The pattern is:

```text
adapter-specific conversion access
+ evidence-specific friend grant
+ no generic production C1 evidence factory
```

B1 itself defines no provenance container.

Its output is naked `InvariantPrerequisiteEvidence`, as specified in §6.

Other adapters may independently decide whether their own conversion boundary
requires a provenance container.

B1 neither requires nor forbids such containers for other adapters.

Future adapters may reuse the narrow-seam structural pattern without inheriting
B1 semantic choices.

This contract does not define:

- resilience mapping semantics;
- permission mapping semantics;
- future adapter outcome algebra;
- provenance-container requirements for other adapters;
- generic adapter framework semantics.

Those remain independent design matters.

## 23. Expected implementation surface

After ratification, the expected implementation workstream is bounded to a new
sibling module plus the narrow C1 seam extension.

Expected additions:

```text
apps/soam-transition-invariant-c1/CMakeLists.txt
apps/soam-transition-invariant-c1/include/production_invariant_c1_adapter.hpp
apps/soam-transition-invariant-c1/src/production_invariant_c1_adapter.cpp
apps/soam-transition-invariant-c1/src/detail/production_invariant_c1_conversion_access.hpp
apps/soam-transition-invariant-c1/tests/...
```

Expected existing-file change:

```text
apps/soam-transition/include/production_transition_eligibility.hpp
```

limited exactly as defined in §11.

Build/workflow files required specifically to test the new module may be added by
the implementation workstream.

No implementation artifact is created by acceptance of this design contract.

## 24. Governance separation

```text
design candidate
!= ratified design
!= implementation
!= implementation acceptance
```

Ratification of this document would establish only the accepted B1 -> C1 adapter
design contract.

It would not:

- create `apps/soam-transition-invariant-c1/`;
- change `production_transition_eligibility.hpp`;
- register a CMake target;
- change runtime behavior;
- constitute implementation acceptance.

Ratification of this document does not affect whether B1 is reachable through
the transition pipeline.

No transition-pipeline implementation currently exists.

B1 reachability through that pipeline depends on the separate transition-pipeline
implementation workstream, including its eventual composition of all required
prerequisite channels.

The B1 adapter implementation remains a separate PR and separate exact-head
acceptance event.

## 25. STOP boundary

After successful adapter conversion, the system possesses:

```text
InvariantPrerequisiteEvidence
```

and nothing more from this adapter.

The adapter does not assemble the five-prerequisite set.

The adapter does not invoke C1.

The adapter does not infer the state of missing channels.

The adapter does not create terminal pipeline semantics.

The adapter does not grant authority or execution.

B1 -> C1 conversion ends here.
