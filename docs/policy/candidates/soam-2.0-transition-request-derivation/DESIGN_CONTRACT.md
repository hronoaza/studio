# SOAM 2.0 — Transition Request Derivation Design Contract

## Status

- Layer: post-D9 / pre-C1 transition request derivation
- Document class: reviewed design contract / implementation provenance
- Design origin baseline:
  `2b8080d2a393d893acc9dfab3ef6e42714367d1f`
- Accepted implementation PR: #22
- Accepted implementation head:
  `060509f24eedb0748f9fcd9f06288d3fc3056261`
- Accepted implementation baseline:
  `45b07590f78e9048a7a203c345f84fdd1456c68b`
- Upstream recommendation source: accepted D9
- Downstream contract: accepted C1 `ProductionTransitionEligibilityEvaluator`
- Acceptance status: design reviewed; implementation accepted and merged
- Final implementation CI: Transition Eligibility Validation and Transition
  Request Validation completed successfully on the exact accepted head
- Authority effect: none

This layer exists only to derive a restricted-origin transition **request**
from an accepted D9 persistent recommendation. It does not derive eligibility,
permission, authority, capability, execution, or topology mutation.

---

## 1. Boundary

Current accepted layers stop at:

```text
VersionedProductionInterpretation
-> D9 ProductionBridgePolicyEvidence
-> D9 persistence stream
-> ProductionPersistentBridgeRecommendation
-> STOP
```

C1 begins at:

```text
ProductionTransitionRequestBinding
+ five prerequisite evidence values
-> ProductionTransitionEligibilityDecision
-> eligible_for_authority_consideration | not_eligible
-> STOP
```

The missing production boundary is therefore:

```text
ProductionPersistentBridgeRecommendation
-> restricted-origin transition request derivation
-> ProductionTransitionRequestBinding
-> STOP
```

This design must not manufacture any C1 prerequisite evidence.

---

## 2. Principal separation

The following identities remain distinct:

```text
persistent recommendation
!= transition request
!= permission prerequisite
!= invariant prerequisite
!= resilience prerequisite
!= freshness prerequisite
!= revalidation prerequisite
!= eligibility
!= authority
!= execution
```

A valid transition request merely gives C1 something typed to evaluate later.

---

## 3. Recommendation mapping

Only directional D9 recommendations may produce a transition request.

Normative mapping:

```text
PersistentBridgeRecommendation::SUPPORT
    -> RequestedTransitionDirection::support

PersistentBridgeRecommendation::CONSTRAIN
    -> RequestedTransitionDirection::constrain

PersistentBridgeRecommendation::PRESERVE
    -> no transition request
```

`PRESERVE` must not be encoded as either direction and must not create a
synthetic "do nothing" request.

The derivation API therefore needs an explicit no-request outcome.

---

## 4. Relationship identity

The request relationship identity must come from the immutable D9 semantic stream
key carried by the recommendation:

```text
sourceNodeId
targetNodeId
relationshipGeneration
```

The adapter must not read those fields from arbitrary caller arguments.

The relationship is directional. Reverse direction is a different relationship
identity.

The D9 `PersistenceStreamInstanceId` is preserved as request provenance but is
not itself part of the C1 `ProductionRelationshipIdentity`.

---

## 5. State version

The C1 `ProductionStateVersion` must represent exactly the
`stateVersion` of the accepted D9 recommendation sample.

It must not be:

- the current live mesh version at derivation time;
- the stream-start epoch;
- a later revalidation version;
- caller supplied.

Later freshness/revalidation stages may compare this bound state version against
live runtime state, but request derivation must not silently rewrite it.

---

## 6. Transition class identity is an explicit design gate

The accepted C1 contract contains restricted-origin
`ProductionTransitionClassId`, but the Current Baseline does not define its
production semantic source.

The request-derivation layer therefore must not use an arbitrary integer,
recommendation enum value, node ID, generation, or stateVersion as a transition
class ID.

Candidate v1 design:

```text
TransitionClass: BridgeCouplingAdjustmentV1
direction: carried separately by RequestedTransitionDirection
```

A single stable production-owned transition class identifies the semantic class
of bridge coupling adjustment; SUPPORT vs CONSTRAIN remains the direction field.

The exact opaque ID value and revision/ownership rules must be accepted before
implementation.

---

## 7. Request provenance wrapper

Returning only a raw C1 `ProductionTransitionRequestBinding` would lose the
D9 decision lineage needed for audit and replay analysis.

Candidate restricted-origin wrapper:

```cpp
class ProductionDerivedTransitionRequest final {
public:
    [[nodiscard]] const TransitionRequestDecisionId&
        decisionId() const noexcept;

    [[nodiscard]] const ProductionTransitionRequestBinding&
        binding() const noexcept;

    [[nodiscard]] const PersistenceObservationDecisionId&
        persistenceObservationDecisionId() const noexcept;

    [[nodiscard]] const PersistenceStreamInstanceId&
        persistenceStreamInstanceId() const noexcept;

    [[nodiscard]] const PolicyEvidenceDecisionId&
        policyEvidenceDecisionId() const noexcept;

    [[nodiscard]] const SourceCaptureId&
        sourceCaptureId() const noexcept;

    [[nodiscard]] std::uint64_t stateVersion() const noexcept;

private:
    // restricted-origin construction only by the production derivation adapter
};
```

The wrapper preserves the D9 event that caused the request. It does not add
permission or authority.

---

## 8. Request decision identity

Candidate:

```cpp
class TransitionRequestDecisionId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;
    [[nodiscard]] const Bytes& bytes() const noexcept;

private:
    explicit TransitionRequestDecisionId(Bytes) noexcept;
    friend class ProductionTransitionRequestDeriver;
};
```

Requirements:

- 128-bit opaque identity;
- no public default constructor;
- no public raw-byte constructor;
- all-zero invalid;
- production-origin only;
- OS-backed random family consistent with existing production decision IDs;
- one unique ID per completed derivation attempt that reaches a typed outcome.

---

## 9. Derivation result

Candidate API:

```cpp
enum class TransitionRequestDerivationReason : std::uint8_t {
    PreserveRecommendation,
    RecommendationLineageInconsistent,
    TransitionClassUnrecognized,
    InternalDerivationFailure
};

class ProductionTransitionRequestDerivationRejection final {
    // immutable decision ID + D9 recommendation identity + reason/flags
};

using ProductionTransitionRequestDerivationResult =
    std::variant<
        ProductionDerivedTransitionRequest,
        ProductionTransitionRequestDerivationRejection>;

class ProductionTransitionRequestDeriver final {
public:
    [[nodiscard]]
    std::optional<ProductionTransitionRequestDerivationResult>
    derive(
        const ProductionPersistentBridgeRecommendation& recommendation,
        const TransitionRequestPolicySnapshot& policy) const;
};
```

`std::nullopt` is reserved for failure to establish restricted-origin decision
identity/internal production precondition.

`PRESERVE` is a typed no-request rejection/outcome, not an error and not a
request.

---

## 10. Transition request policy

Because transition class semantics are production policy, callers must not
supply an arbitrary class ID.

Candidate trusted policy:

```cpp
class TransitionRequestPolicySnapshot final {
public:
    [[nodiscard]] const TransitionRequestPolicySnapshotId&
        snapshotId() const noexcept;

    [[nodiscard]] const TransitionRequestPolicyDescriptor&
        descriptor() const noexcept;

    [[nodiscard]] const ProductionTransitionClassId&
        bridgeCouplingAdjustmentClass() const noexcept;

private:
    friend class ProductionTransitionRequestPolicyProvider;
};

class ProductionTransitionRequestPolicyProvider final {
public:
    [[nodiscard]]
    static std::optional<TransitionRequestPolicySnapshot>
    createCurrent();
};
```

The trusted provider owns the production transition-class identity and semantic
revision.

No caller-selected raw class ID enters the trusted derivation API.

---

## 11. Restricted construction seam into C1

C1 currently permits construction only through the test-only
`detail::ProductionTransitionConstructionAccess` seam and reserves a separate
`detail::ProductionAuthorityDerivationAccess` seam for other future concerns.

This layer must introduce its own narrowly scoped production friend, for example:

```cpp
namespace detail {
class ProductionTransitionRequestDerivationAccess;
}
```

Only that access type may construct:

- `ProductionRelationshipIdentity`;
- `ProductionStateVersion`;
- `ProductionTransitionClassId`;
- `ProductionTransitionRequestBinding`.

It must **not** receive friendship for:

- `PermissionPrerequisiteEvidence`;
- `InvariantPrerequisiteEvidence`;
- `ResiliencePrerequisiteEvidence`;
- `FreshnessPrerequisiteEvidence`;
- `RevalidationPrerequisiteEvidence`.

Therefore request derivation cannot satisfy its own C1 eligibility prerequisites.

The existing `ProductionAuthorityDerivationAccess` seam remains untouched and
inactive.

---

## 12. Lineage invariants

Before producing a request, derivation must require:

1. non-zero D9 observation decision ID;
2. non-zero persistence stream instance ID;
3. non-zero policy evidence decision ID;
4. non-zero SourceCaptureId;
5. recommendation stateVersion equals the D9 accepted sample stateVersion;
6. relationship identity/generation comes from the recommendation stream key;
7. recommendation value is one of the accepted enum values;
8. trusted transition-request policy revision is recognized.

The adapter does not re-evaluate D9 evidence or rerun `BridgePersistence`.

---

## 13. No live mesh read in request derivation

The derivation adapter is deterministic over immutable trusted input.

It must not:

- capture a new relationship snapshot;
- inspect current BridgeStatus;
- inspect current node state/health;
- update stateVersion;
- decide freshness;
- decide revalidation;
- mutate topology.

Live state belongs to later prerequisite derivation/evaluation.

This prevents request creation from silently collapsing into C1 eligibility.

---

## 14. Replay semantics

Repeated derivation from the same immutable D9 recommendation may produce a new
derivation decision event, but it must preserve the exact same C1 request
binding.

This layer does not itself deduplicate requests because it performs no mutation
and grants no authority.

Any later consumer that treats requests as commands must define its own
decision-event and replay policy before execution becomes possible.

---

## 15. Authority boundary

This layer may emit only:

```text
ProductionDerivedTransitionRequest
```

It may not emit or construct:

- any C1 prerequisite evidence;
- `ProductionTransitionEligibilityDecision`;
- authority/capability/grant objects;
- permission evidence;
- execution tokens;
- bridge mutation operations.

Terminal boundary:

```text
derived request -> STOP
```

---

## 16. Compile-fail contract

Before implementation acceptance, negative compilation tests must prove arbitrary
callers cannot:

1. construct `TransitionRequestDecisionId` from default/raw bytes;
2. directly construct `ProductionDerivedTransitionRequest`;
3. directly construct trusted `TransitionRequestPolicySnapshot`;
4. inject an arbitrary `ProductionTransitionClassId`;
5. directly construct a production `ProductionTransitionRequestBinding`;
6. use request derivation access to construct any C1 prerequisite evidence;
7. convert `PRESERVE` into SUPPORT or CONSTRAIN request;
8. obtain a `ProductionTransitionEligibilityDecision` from the derivation API;
9. obtain authority/capability/execution types from the derivation API.

---

## 17. Verification plan

At minimum:

1. SUPPORT maps exactly to requested support;
2. CONSTRAIN maps exactly to requested constrain;
3. PRESERVE yields typed no-request outcome;
4. source/target/generation copied exactly from D9 stream key;
5. request stateVersion equals D9 recommendation stateVersion;
6. persistence stream instance ID preserved in provenance wrapper;
7. persistence observation decision ID preserved;
8. policy evidence decision ID preserved;
9. SourceCaptureId preserved;
10. repeated derivation preserves identical request binding;
11. repeated derivation gets distinct derivation decision IDs;
12. unrecognized transition-request policy fails closed;
13. caller cannot inject transition class ID;
14. caller cannot construct C1 prerequisite evidence through this layer;
15. caller cannot derive eligibility/authority/execution;
16. prior D7/D8/D9/C1 regression suites remain green.

---

## 18. Open design gates

Before implementation:

1. accept/revise the boundary:
   `D9 recommendation -> derived C1 request -> STOP`;
2. accept/revise `PRESERVE -> no request`;
3. accept/revise one production transition class
   `BridgeCouplingAdjustmentV1` with direction carried separately;
4. assign the exact opaque transition-class identity and semantic revision rules;
5. accept/revise the provenance wrapper fields;
6. define exact restricted-origin C++ access/friend surface in the C1 header;
7. perform final design acceptance review;
8. only then implement on a separate candidate branch.


---

## 19. Transition-class identity acceptance

The v1 production transition class is fixed as one semantic class:

```text
BridgeCouplingAdjustmentV1
```

SUPPORT and CONSTRAIN are not separate classes. They are directions within the
same transition class.

The accepted C1 representation is currently an opaque `std::uint64_t`.
For v1, production derivation owns the stable internal value:

```text
0x4252494447455631
```

ASCII mnemonic: `BRIDGEV1`.

This value is an internal semantic identity, not a capability, permission,
authority token, counter, node ID, generation, or state version.

It must not be caller-selected.

A future incompatible transition-class semantic requires a new class identity;
it must not silently reuse `BridgeCouplingAdjustmentV1`.

The derivation policy descriptor separately versions the implementation that
maps trusted D9 recommendation events to this class.

---

## 20. Transition-request policy identity

The trusted derivation policy uses the same semantic/publication distinction as
D8/D9 policy families.

Candidate restricted-origin IDs:

```cpp
class TransitionRequestPolicyId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;
    [[nodiscard]] const Bytes& bytes() const noexcept;
private:
    explicit TransitionRequestPolicyId(Bytes) noexcept;
    friend class ProductionTransitionRequestPolicyProvider;
};

class TransitionRequestPolicySnapshotId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;
    [[nodiscard]] const Bytes& bytes() const noexcept;
private:
    explicit TransitionRequestPolicySnapshotId(Bytes) noexcept;
    friend class ProductionTransitionRequestPolicyProvider;
};
```

Descriptor:

```cpp
struct TransitionRequestPolicyDescriptor final {
    TransitionRequestPolicyId policyId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t, 32> implementationRevision;
};
```

V1:

```text
majorVersion = 1
minorVersion = 0
implementationRevisionKind = 1
transition class = BridgeCouplingAdjustmentV1
```

The implementation revision is a reproducible digest of the exact public,
internal, and source surface of this derivation layer.

A new publication of byte-identical semantics gets a new snapshot ID but retains
the same policy ID/version/revision descriptor.

---

## 21. Final trusted policy snapshot

```cpp
class TransitionRequestPolicySnapshot final {
public:
    [[nodiscard]] const TransitionRequestPolicySnapshotId&
        snapshotId() const noexcept;

    [[nodiscard]] const TransitionRequestPolicyDescriptor&
        descriptor() const noexcept;

    [[nodiscard]] const ProductionTransitionClassId&
        bridgeCouplingAdjustmentClass() const noexcept;

private:
    TransitionRequestPolicySnapshot(
        TransitionRequestPolicySnapshotId,
        TransitionRequestPolicyDescriptor,
        ProductionTransitionClassId) noexcept;

    friend class ProductionTransitionRequestPolicyProvider;
};

class ProductionTransitionRequestPolicyProvider final {
public:
    [[nodiscard]]
    static std::optional<TransitionRequestPolicySnapshot>
    createCurrent();
};
```

No public constructor accepts a raw `std::uint64_t` class value.

The provider constructs the C1 opaque class only through the restricted
production derivation access defined below.

---

## 22. Final C1 restricted-access surface

The C1 header should add exactly one production derivation friend:

```cpp
namespace detail {
class ProductionTransitionRequestDerivationAccess;
}
```

Friendship is added only to:

```text
ProductionRelationshipIdentity
ProductionStateVersion
ProductionTransitionClassId
ProductionTransitionRequestBinding
```

It is **not** added to:

```text
PermissionPrerequisiteEvidence
InvariantPrerequisiteEvidence
ResiliencePrerequisiteEvidence
FreshnessPrerequisiteEvidence
RevalidationPrerequisiteEvidence
ProductionTransitionEligibilityDecision
```

Candidate internal access API:

```cpp
class ProductionTransitionRequestDerivationAccess final {
public:
    [[nodiscard]] static ProductionRelationshipIdentity relationship(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t generation) noexcept;

    [[nodiscard]] static ProductionStateVersion stateVersion(
        std::uint64_t value) noexcept;

    [[nodiscard]] static ProductionTransitionClassId
    bridgeCouplingAdjustmentV1() noexcept;

    [[nodiscard]] static ProductionTransitionRequestBinding binding(
        ProductionRelationshipIdentity relationship,
        RequestedTransitionDirection direction,
        ProductionTransitionClassId transitionClass,
        ProductionStateVersion stateVersion) noexcept;
};
```

There is deliberately no generic
`transitionClass(std::uint64_t)` production function. Production code can
obtain only the accepted bridge-coupling class.

The existing test-only `ProductionTransitionConstructionAccess` remains for
isolated C1 tests. The existing `ProductionAuthorityDerivationAccess` remains
untouched and is not used by this layer.

---

## 23. Final derived-request provenance

The accepted wrapper must preserve enough identity to answer both:

1. which exact D9 recommendation event produced this request?
2. under which exact request-derivation policy publication and semantics was the
   request produced?

Final candidate:

```cpp
class ProductionDerivedTransitionRequest final {
public:
    [[nodiscard]] const TransitionRequestDecisionId&
        decisionId() const noexcept;

    [[nodiscard]] const ProductionTransitionRequestBinding&
        binding() const noexcept;

    [[nodiscard]] const TransitionRequestPolicySnapshotId&
        policySnapshotId() const noexcept;

    [[nodiscard]] const TransitionRequestPolicyDescriptor&
        policyDescriptor() const noexcept;

    [[nodiscard]] const PersistenceObservationDecisionId&
        persistenceObservationDecisionId() const noexcept;

    [[nodiscard]] const PersistenceStreamInstanceId&
        persistenceStreamInstanceId() const noexcept;

    [[nodiscard]] const ProductionPersistenceStreamKey&
        persistenceStreamKey() const noexcept;

    [[nodiscard]] const PolicyEvidenceDecisionId&
        policyEvidenceDecisionId() const noexcept;

    [[nodiscard]] const SourceCaptureId&
        sourceCaptureId() const noexcept;

    [[nodiscard]] PersistentBridgeRecommendation
        sourceRecommendation() const noexcept;

    [[nodiscard]] std::uint64_t stateVersion() const noexcept;

private:
    // restricted-origin construction by ProductionTransitionRequestDeriver only
};
```

The exact persistence stream key is retained, not reconstructed from the C1
binding. This preserves the D8D interpretation semantic revision and D9
persistence-profile semantic revision that defined the recommendation stream.

The wrapper therefore preserves a complete audit chain while the C1 binding
remains intentionally minimal.

---

## 24. Final derivation result semantics

The v1 outcome model distinguishes "no request" from invalid lineage/policy.

```cpp
enum class TransitionRequestDerivationReason : std::uint8_t {
    PreserveRecommendation,
    RecommendationLineageInconsistent,
    PolicyRevisionUnrecognized,
    InternalDeterministicDerivationFailure
};

class ProductionTransitionRequestDerivationRejection final {
public:
    [[nodiscard]] const TransitionRequestDecisionId&
        decisionId() const noexcept;

    [[nodiscard]] const PersistenceObservationDecisionId&
        persistenceObservationDecisionId() const noexcept;

    [[nodiscard]] TransitionRequestDerivationReason
        primaryReason() const noexcept;

    [[nodiscard]] std::uint64_t reasonFlags() const noexcept;

private:
    friend class ProductionTransitionRequestDeriver;
};

using ProductionTransitionRequestDerivationResult =
    std::variant<
        ProductionDerivedTransitionRequest,
        ProductionTransitionRequestDerivationRejection>;
```

Normative priority:

```text
1. policy revision recognized
2. recommendation lineage consistent
3. PRESERVE -> typed no-request outcome
4. SUPPORT/CONSTRAIN -> deterministic request
5. internal deterministic failure
```

`reasonFlags` uses one bit per enumerator. `primaryReason` is the first
applicable reason in this fixed order.

A completed PRESERVE derivation therefore has a decision ID and typed
`PreserveRecommendation` outcome, but no C1 request.

---

## 25. Final deriver API

```cpp
class ProductionTransitionRequestDeriver final {
public:
    [[nodiscard]]
    std::optional<ProductionTransitionRequestDerivationResult>
    derive(
        const ProductionPersistentBridgeRecommendation& recommendation,
        const TransitionRequestPolicySnapshot& policy) const;
};
```

The deriver has no mesh, registry, transition evaluator, prerequisite provider,
authority registry, or mutation handle.

It is a deterministic immutable-input adapter.

`std::nullopt` means the derivation attempt could not establish its
restricted-origin decision identity or satisfy an internal production
precondition before a typed decision could be issued.

---

## 26. Exact construction algorithm

For every call:

```text
generate non-zero TransitionRequestDecisionId
-> verify trusted request-policy revision
-> verify D9 recommendation lineage invariants
-> if PRESERVE: emit typed no-request rejection
-> map SUPPORT/CONSTRAIN to RequestedTransitionDirection
-> construct exact relationship from D9 stream key
-> construct exact ProductionStateVersion from D9 recommendation stateVersion
-> obtain BridgeCouplingAdjustmentV1 from restricted production access
-> construct ProductionTransitionRequestBinding
-> emit immutable ProductionDerivedTransitionRequest
-> STOP
```

No live state read and no C1 prerequisite derivation occurs.

---

## 27. Recommendation-lineage consistency

The v1 deriver must require:

```text
recommendation.streamKey.source/target/generation
    are the relationship identity used for binding

recommendation.stateVersion > 0

recommendation decision ID != zero
stream instance ID != zero
policy evidence decision ID != zero
SourceCaptureId != zero
```

Because `ProductionPersistentBridgeRecommendation` is restricted-origin, the
deriver does not attempt to reconstruct upstream D8 provenance from raw fields.

It preserves the trusted D9 identities verbatim.

---

## 28. Request stability and replay

For the same accepted recommendation event and the same semantic derivation
policy revision:

```text
binding #1 == binding #2
```

even though:

```text
TransitionRequestDecisionId #1 != #2
policy snapshot publication IDs may differ
```

provided the policy descriptors and transition class are semantically identical.

This makes request content deterministic while retaining distinct derivation
attempt identity.

---

## 29. Implementation-placement rule

The production adapter should live in the runtime/transition boundary, not inside
the domain persistence primitive and not inside the C1 eligibility evaluator.

Expected dependency direction:

```text
soam-domain
   ^
soam-runtime D9
   ^
transition-request derivation adapter
   v
soam-transition C1 types
```

The implementation must not create a dependency from the domain library back
toward runtime or transition eligibility.

Exact CMake placement is an implementation gate and must preserve an acyclic
dependency graph.

---

## 30. Design acceptance

The critical review closes the original open gates as follows:

- boundary accepted:
  `D9 recommendation -> derived C1 request -> STOP`;
- PRESERVE semantics accepted: typed no-request outcome;
- transition class accepted:
  one `BridgeCouplingAdjustmentV1` semantic class, stable internal identity
  `0x4252494447455631`, direction separate;
- transition-request policy identity/versioning accepted;
- provenance wrapper accepted with exact D9 stream/event lineage and derivation
  policy provenance;
- C1 friend surface accepted: request-binding types only, no prerequisite
  evidence and no eligibility decision;
- derivation is deterministic and performs no live mesh read;
- authority boundary remains intact.

No design-level authority or semantic blocker remains.

Before implementation, one engineering gate remains:

- choose the exact CMake/library placement that preserves an acyclic dependency
  graph between `soam-runtime`, the request-derivation adapter, and
  `soam-transition`.

Implementation must be a separate candidate branch and must not begin by
modifying Current Baseline directly.


---

## 31. Implementation placement acceptance

The engineering placement gate is resolved.

The request-derivation implementation must be a separate library/application
surface:

```text
apps/soam-transition-request/
```

Candidate CMake target:

```text
soam_transition_request
AdaptiveMesh::soam_transition_request
```

Dependency direction:

```text
soam-domain
    ^
soam-runtime
    ^
    |
soam-transition-request ----> soam-transition-eligibility
```

More explicitly:

```text
soam_transition_request
    links/depends on:
        AdaptiveMesh::soam_runtime
        soam_transition_eligibility

soam_runtime
    does NOT link to soam_transition_request
    does NOT link to soam_transition_eligibility

soam_transition_eligibility
    remains independent/header-only
    does NOT link to runtime
```

This preserves an acyclic graph.

The existing `ProductionTransitionEvaluator` inside `soam-runtime` remains
unchanged during this layer. It must not begin consuming the derived request in
the same implementation gate.

Integration of the derived request with live prerequisite production/evaluation
is a later, separate layer.

---

## 32. Expected implementation surface

A candidate implementation branch should introduce only the minimum surface
required for this layer.

Expected new files:

```text
apps/soam-transition-request/CMakeLists.txt
apps/soam-transition-request/include/production_transition_request.hpp
apps/soam-transition-request/src/production_transition_request.cpp
apps/soam-transition-request/src/detail/production_transition_request_internal.hpp
apps/soam-transition-request/src/detail/transition_request_implementation_revision.hpp.in
apps/soam-transition-request/tests/transition_request_tests.cpp
apps/soam-transition-request/tests/compile_fail/*
.github/workflows/soam-transition-request-validation.yml
```

Expected existing-file change:

```text
apps/soam-transition/include/production_transition_eligibility.hpp
```

That existing header change is limited to:

- forward declaration of
  `detail::ProductionTransitionRequestDerivationAccess`;
- friendship for the four request-binding construction types fixed in section
  22.

No C1 evaluator semantics, prerequisite evidence semantics, rejection precedence,
or eligibility result semantics may change.

No D9 runtime source file needs to change.

---

## 33. Build topology

Because the repository has independent app-level CMake roots rather than one
repository-wide root target graph, the new request-derivation CMake project may
compose its dependencies using dedicated sub-build directories.

Normative shape:

```cmake
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-runtime")

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-transition")

add_library(soam_transition_request STATIC ...)
add_library(AdaptiveMesh::soam_transition_request
    ALIAS soam_transition_request)

target_link_libraries(soam_transition_request
    PUBLIC
        AdaptiveMesh::soam_runtime
        soam_transition_eligibility)
```

If implementation discovers target-name/options collisions caused by nested
subprojects, the accepted architectural rule remains the same: solve build
composition locally in `apps/soam-transition-request`; do not reverse the
dependency and do not make `soam-runtime` depend on the adapter.

---

## 34. Regression boundary

The implementation PR must prove:

- `soam-runtime` can still configure/build/test independently;
- `soam-transition` can still configure/build/test independently;
- the new `soam-transition-request` project can configure/build/test while
  composing both dependencies;
- existing D9 workflows remain green;
- existing C1 transition eligibility workflow remains green;
- the new adapter workflow passes ASan/UBSan and TSan where supported.

A passing adapter build is insufficient if either upstream independent project
is broken.

---

## 35. Final design acceptance

All design gates are now resolved:

1. boundary:
   `D9 recommendation -> derived C1 request -> STOP`;
2. PRESERVE:
   typed no-request outcome;
3. direction:
   SUPPORT/CONSTRAIN map exactly to C1 direction values;
4. transition class:
   one production-owned `BridgeCouplingAdjustmentV1`,
   internal stable value `0x4252494447455631`;
5. request policy:
   restricted-origin semantic policy ID + publication snapshot ID + versioned
   implementation revision;
6. stateVersion:
   exact D9 accepted-sample state version, never live-resampled;
7. relationship:
   exact source/target/generation from D9 stream key;
8. provenance:
   D9 observation, stream instance/key, policy-evidence decision,
   SourceCaptureId, source recommendation, and request-policy provenance are
   preserved;
9. C1 friend surface:
   only relationship/stateVersion/class/request binding constructors;
10. C1 prerequisite evidence:
    inaccessible to this layer;
11. authority:
    no eligibility, permission, capability, authority, or execution output;
12. live state:
    no mesh read in request derivation;
13. placement:
    separate `soam-transition-request` library with one-way dependencies;
14. existing `ProductionTransitionEvaluator`:
    unchanged in this implementation gate.

No design-level or dependency-graph blocker remains before implementation.

Implementation remains a separate acceptance gate and must start from accepted
baseline:

`2b8080d2a393d893acc9dfab3ef6e42714367d1f`

on a separate candidate implementation branch.


---

## 36. Accepted implementation provenance

The transition-request derivation implementation was accepted separately from
this design branch.

Acceptance evidence:

- design PR: #21;
- implementation PR: #22;
- implementation candidate head:
  `060509f24eedb0748f9fcd9f06288d3fc3056261`;
- merge commit / accepted implementation baseline:
  `45b07590f78e9048a7a203c345f84fdd1456c68b`;
- implementation PR merged only after pre-acceptance audit;
- exact-head CI completed successfully for:
  - SOAM 2.0 Transition Eligibility Validation;
  - SOAM 2.0 Transition Request Validation;
- the transition-request validation job exercised the composed runtime,
  D7/D8/D9 regressions, C1 eligibility, positive request-derivation behavior,
  and compile-fail trust-boundary guards;
- accepted implementation preserves:
  `persistent recommendation != transition request != eligibility != permission != authority != execution`.

The accepted runtime surface is:

```text
D9 persistent recommendation
-> production transition request derivation
-> C1 ProductionTransitionRequestBinding
-> STOP
```

This design branch itself contains no executable runtime implementation. Its
post-acceptance purpose is durable architecture and audit provenance.
