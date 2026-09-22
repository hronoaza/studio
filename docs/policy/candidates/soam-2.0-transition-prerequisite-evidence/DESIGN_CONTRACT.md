# SOAM 2.0 — Transition Prerequisite Evidence Design Contract

## Status

- Layer: post-transition-request / pre-C1 eligibility
- Document class: reviewed design contract / implementation provenance
- Design origin baseline:
  `037eb4397d6bb3a5af5eea911b8b679407383fcd`
- Accepted A1 implementation PR: #24
- Accepted A1 implementation head:
  `bf7361df2d7ffdb837479df9ed397e8387010109`
- Accepted A1 implementation baseline:
  `eceed69eeea508e3388ecc79aaf2daf2aeda8189`
- Upstream request source: accepted transition-request derivation
- Downstream contract: accepted C1 `ProductionTransitionEligibilityEvaluator`
- Acceptance status: Live Validity A1 design reviewed; implementation accepted and merged
- Final A1 CI: all 10 exact-head workflows completed successfully
- Authority effect: none

This layer defines how the five C1 prerequisite evidence values may be produced
without allowing eligibility machinery to create its own permission or authority.

---

## 1. Boundary

Accepted upstream:

```text
D9 persistent recommendation
-> ProductionDerivedTransitionRequest
-> ProductionTransitionRequestBinding
-> STOP
```

Accepted C1 begins at:

```text
ProductionTransitionRequestBinding
+ PermissionPrerequisiteEvidence
+ InvariantPrerequisiteEvidence
+ ResiliencePrerequisiteEvidence
+ FreshnessPrerequisiteEvidence
+ RevalidationPrerequisiteEvidence
-> ProductionTransitionEligibilityDecision
-> STOP
```

The missing production boundary is:

```text
ProductionDerivedTransitionRequest
+ trusted prerequisite sources
-> lineage-bound prerequisite evidence set
-> C1 eligibility evaluation
-> STOP
```

---

## 2. Principal separation

```text
request
!= permission evidence
!= invariant evidence
!= resilience evidence
!= freshness evidence
!= revalidation evidence
!= eligibility
!= authority
!= execution
```

No prerequisite may be inferred merely because a D9 recommendation or derived
request exists.

---

## 3. Five independent prerequisite channels

The five C1 prerequisites are semantically independent.

### 3.1 Permission

Permission evidence is authority-adjacent.

It MUST NOT be derived from:

- D9 recommendation;
- request direction;
- request class;
- persistence confidence/evidence;
- eligibility state;
- mesh health;
- relationship existence;
- successful tests;
- technical GitHub/API permission.

A production permission prerequisite must originate from a separately accepted
authority/permission attestation mechanism.

Until such a mechanism exists, production permission evidence is unavailable and
eligibility must fail closed.

### 3.2 Invariant

Invariant evidence concerns whether the requested transition is compatible with
accepted domain invariants.

Current baseline contains `IdentityInvariant` and safety-bound logic, but C1 does
not yet define which node(s), projected post-transition state, or invariant set
must be checked for a transition request.

Therefore invariant prerequisite semantics remain an explicit design gate.

### 3.3 Resilience

Resilience evidence concerns whether the requested transition preserves required
operational resilience.

Current baseline exposes node health and bridge state, but no accepted production
resilience policy maps those values to C1 `ResiliencePrerequisiteEvidence`.

Therefore resilience semantics remain an explicit design gate.

### 3.4 Freshness

Freshness can be grounded in accepted runtime state-version lineage.

Minimum candidate rule:

```text
request.binding.stateVersion == current coherent mesh transitionStateVersion
```

Any mismatch means `FreshnessPrerequisiteEvidence{satisfied=false}`.

Freshness must not silently update or rewrite the request's bound stateVersion.

### 3.5 Revalidation

Revalidation can be grounded in current coherent relationship identity.

Minimum candidate rule requires all of:

```text
relationship still exists
sourceNodeId matches
targetNodeId matches
relationship generation matches exactly
current stateVersion still equals request-bound stateVersion
```

If any fails, revalidation is false.

Freshness and revalidation remain separate evidence objects even if they inspect
overlapping state.

---

## 4. Coherent live snapshot

Freshness and revalidation must be derived from one coherent mesh snapshot under
the topology consistency lock.

Candidate snapshot:

```cpp
struct TransitionPrerequisiteSnapshot {
    std::size_t sourceNodeId;
    std::size_t targetNodeId;
    std::uint64_t relationshipGeneration;
    std::uint64_t stateVersion;
    bool relationshipPresent;
};
```

The snapshot must not be assembled through separate unlocked reads.

---

## 5. Request binding is immutable input

All prerequisite evidence must bind to the exact accepted
`ProductionTransitionRequestBinding`.

No provider may replace:

- source node;
- target node;
- relationship generation;
- requested direction;
- transition class;
- stateVersion.

If live state has moved, the evidence should be unsatisfied or unavailable;
the request is not rewritten.

---

## 6. Lineage-preserving prerequisite result

Candidate wrapper:

```cpp
class ProductionTransitionPrerequisiteEvidenceSet final {
public:
    [[nodiscard]] const TransitionPrerequisiteDecisionId&
        decisionId() const noexcept;

    [[nodiscard]] const ProductionDerivedTransitionRequest&
        request() const noexcept;

    [[nodiscard]] const ProductionTransitionPrerequisiteSet&
        prerequisites() const noexcept;

private:
    // restricted-origin only
};
```

The wrapper preserves exact request provenance and the exact five evidence
objects supplied to C1.

---

## 7. Decision identity

Candidate `TransitionPrerequisiteDecisionId`:

- opaque 128-bit;
- no public default/raw-byte construction;
- all-zero invalid;
- production-origin only;
- unique per completed evidence-assembly decision;
- OS-backed random generation consistent with existing decision-ID families.

---

## 8. Production evidence assembly

Candidate API shape:

```cpp
class ProductionTransitionPrerequisiteAssembler final {
public:
    [[nodiscard]]
    std::optional<ProductionTransitionPrerequisiteAssemblyResult>
    assemble(
        const ProductionDerivedTransitionRequest& request,
        const TransitionPrerequisitePolicySnapshot& policy,
        const ProductionPermissionPrerequisiteInput& permissionInput) const;
};
```

Important: this class does not create permission. It may only validate and bind a
trusted permission input produced elsewhere.

If no accepted permission source exists, no production call path may fabricate
`ProductionPermissionPrerequisiteInput`.

---

## 9. Restricted C1 construction surface

The existing transition-request derivation access must remain unable to create
any prerequisite evidence.

A new separate production friend may be introduced:

```cpp
namespace detail {
class ProductionTransitionPrerequisiteEvidenceAccess;
}
```

Friendship may be granted only to:

- `PermissionPrerequisiteEvidence`;
- `InvariantPrerequisiteEvidence`;
- `ResiliencePrerequisiteEvidence`;
- `FreshnessPrerequisiteEvidence`;
- `RevalidationPrerequisiteEvidence`.

It must NOT receive friendship for:

- `ProductionTransitionEligibilityDecision`;
- authority/capability types;
- execution/mutation types.

The request-binding construction friend remains separate.

---

## 10. Permission input must be restricted-origin

Candidate type:

```cpp
class ProductionPermissionPrerequisiteInput final {
public:
    [[nodiscard]] const PermissionAttestationId&
        attestationId() const noexcept;

    [[nodiscard]] const ProductionTransitionRequestBinding&
        binding() const noexcept;

    [[nodiscard]] bool satisfied() const noexcept;

private:
    // construction reserved for a later accepted permission authority layer
};
```

This layer may consume the type once a legitimate producer exists, but must not
define a permissive public constructor or self-issue attestations.

Until the producer is accepted, production assembly should fail closed before
creating a satisfied permission prerequisite.

---

## 11. Permission is not Root Operator inference

The existence of a Root Operator or authority registry entry does not by itself
constitute per-request permission evidence.

A future permission-attestation layer must explicitly define:

- who/what may issue an attestation;
- scope;
- exact request binding;
- validity interval or state version;
- revocation;
- replay semantics;
- provenance.

This design does not create that mechanism.

---

## 12. Invariant evidence design gate

Before implementation, invariant semantics must answer:

1. which invariant domains are relevant to `BridgeCouplingAdjustmentV1`;
2. whether source, target, or both nodes are evaluated;
3. whether current state or projected post-transition state is evaluated;
4. exact fail-closed behavior when required state is unavailable;
5. versioned invariant policy identity;
6. whether the check is purely observational or requires a deterministic
   transition projection.

No implementation may equate "current node state is within its local
`IdentityInvariant`" with full transition invariant satisfaction without
explicit design acceptance.

---

## 13. Resilience evidence design gate

Before implementation, resilience semantics must answer:

1. exact health/capacity/status inputs;
2. thresholds;
3. whether direction changes the rule;
4. whether one endpoint or both endpoints must satisfy it;
5. whether topology redundancy matters;
6. versioned policy identity;
7. exact fail-closed behavior.

No caller-selected thresholds may enter trusted production evidence.

---

## 14. Freshness candidate policy v1

Candidate semantics:

```text
satisfied =
    current coherent transitionStateVersion
    == request.binding.stateVersion
```

No age-in-seconds clock is required in v1.

State-version mismatch is sufficient to make freshness false.

The exact current state version must be captured from the mesh under the same
lock as relationship generation used by revalidation.

---

## 15. Revalidation candidate policy v1

Candidate semantics:

```text
satisfied =
    relationshipPresent
    && current source == request source
    && current target == request target
    && current generation == request generation
    && current stateVersion == request stateVersion
```

A recreated relationship with the same node pair but a new generation fails
revalidation.

---

## 16. Assembly ordering

Normative candidate ordering:

```text
establish prerequisite decision ID
-> validate prerequisite policy
-> validate exact request lineage
-> validate trusted permission input binding
-> capture one coherent live transition snapshot
-> derive freshness
-> derive revalidation
-> derive invariant according to accepted policy
-> derive resilience according to accepted policy
-> construct five C1 prerequisite evidence objects
-> emit immutable lineage-bound prerequisite set
-> STOP
```

This layer must not call C1 eligibility internally unless a later integration
contract explicitly allows it.

---

## 17. No authority derivation

A successful five-prerequisite evidence set still means only:

```text
evidence is available for C1 evaluation
```

It does not mean:

- eligible;
- permitted for execution;
- authorized;
- capability issued;
- transition committed.

Even `permission.satisfied=true` is merely one prerequisite fact bound to one
request; it does not replace downstream governance or authority semantics.

---

## 18. No execution surface

This layer may not expose:

- topology mutation methods;
- bridge capacity setters;
- state mutation;
- transition commit;
- authority token issuance;
- capability issuance;
- automatic merge/deploy behavior.

Terminal boundary:

```text
prerequisite evidence set -> STOP
```

---

## 19. Result/rejection semantics

Candidate reasons:

```cpp
enum class TransitionPrerequisiteReason : std::uint8_t {
    PermissionInputUnavailable,
    PermissionBindingMismatch,
    PermissionNotSatisfied,
    LiveRelationshipMissing,
    RelationshipGenerationChanged,
    StateVersionChanged,
    InvariantPolicyUnsatisfied,
    ResiliencePolicyUnsatisfied,
    PolicyRevisionUnrecognized,
    RequestLineageInconsistent,
    InternalDeterministicFailure
};
```

The design must distinguish absence/mismatch from a legitimate unsatisfied
prerequisite.

Final reason precedence remains an open design gate.

---

## 20. Policy identity

Invariant/resilience/freshness/revalidation rules must be versioned under a
trusted `TransitionPrerequisitePolicySnapshot`.

Candidate descriptor includes:

- semantic policy ID;
- major/minor version;
- implementation revision kind;
- reproducible implementation digest;
- fixed invariant policy descriptor;
- fixed resilience policy descriptor;
- freshness policy version;
- revalidation policy version.

No caller-selected thresholds or opaque class identities.

Permission-attestation semantics remain separately versioned and are not owned by
this policy snapshot.

---

## 21. Concurrency and lifetime

Any live mesh-backed prerequisite assembler must use a lease/invalidation model
equivalent in strength to accepted transition evaluator/persistence registry
patterns:

- no new leases after invalidation begins;
- active leases drain before mesh destruction;
- no raw public mesh pointer;
- stale façade fails closed.

The coherent transition snapshot is captured while a valid owner lease exists.

---

## 22. Compile-fail contract

Before implementation acceptance, negative tests must prove arbitrary callers
cannot:

1. construct prerequisite decision IDs from default/raw bytes;
2. directly construct trusted prerequisite policy snapshots;
3. directly construct any five C1 prerequisite evidence objects;
4. use transition-request derivation access to construct prerequisites;
5. use prerequisite evidence access to construct request bindings;
6. fabricate a trusted permission input;
7. inject caller-selected invariant/resilience thresholds;
8. obtain a C1 eligibility decision from the prerequisite assembler;
9. obtain authority/capability/execution types from the assembler;
10. mutate mesh state through the prerequisite API.

---

## 23. Verification plan

When semantics are fully accepted, tests must include:

- fresh exact request -> freshness true;
- state-version advance -> freshness false;
- exact generation still live -> revalidation true;
- relationship removal -> revalidation false;
- relationship recreation -> generation mismatch -> revalidation false;
- permission input exact binding accepted;
- permission binding mismatch rejected/fails closed;
- absent permission producer cannot create satisfied permission evidence;
- invariant positive/negative cases under accepted policy;
- resilience positive/negative cases under accepted policy;
- prerequisite evidence contexts equal exact request binding;
- repeated immutable-input evaluation is deterministic except decision IDs;
- mesh destruction/invalidation fails closed;
- no eligibility/authority/execution surface;
- D7/D8/D9/transition-request/C1 regressions remain green.

---

## 24. Open design gates

Implementation MUST NOT begin until these are resolved:

1. exact invariant prerequisite semantics;
2. exact resilience prerequisite semantics;
3. accepted permission-attestation producer or explicit decision to implement
   only freshness/revalidation sublayer first;
4. prerequisite rejection precedence and reason flags;
5. exact policy descriptor/versioning for invariant/resilience;
6. exact placement/CMake dependency graph;
7. whether assembly is monolithic or split into independently reviewable
   providers;
8. whether C1 evaluation integration remains separate after all five evidence
   providers exist.

---

## 25. Recommended decomposition

To preserve evidence-first development, the preferred implementation sequence is:

```text
A. live snapshot + freshness/revalidation evidence
B. invariant evidence policy
C. resilience evidence policy
D. permission attestation consumption
E. prerequisite set assembly
F. C1 eligibility integration
G. STOP before authority
```

Each sublayer should have its own acceptance gate if it changes trust or authority
semantics.

This avoids implementing the three unresolved semantic channels merely to make
C1 return `eligible_for_authority_consideration`.


---

## 26. First implementation sublayer: Live Validity Evidence

The first executable sublayer is intentionally narrower than the full
prerequisite-evidence architecture.

Accepted boundary:

```text
ProductionDerivedTransitionRequest
+ one coherent live transition snapshot
-> ProductionFreshnessPrerequisiteRecord
+ ProductionRevalidationPrerequisiteRecord
-> STOP
```

This sublayer does **not** produce:

- PermissionPrerequisiteEvidence;
- InvariantPrerequisiteEvidence;
- ResiliencePrerequisiteEvidence;
- ProductionTransitionPrerequisiteSet;
- ProductionTransitionEligibilityDecision;
- authority/capability/execution output.

The purpose is to establish live validity facts only.

---

## 27. One coherent snapshot

Freshness and revalidation must be computed from the same immutable snapshot
captured under the accepted topology consistency lock.

Final candidate snapshot:

```cpp
struct ProductionTransitionLiveSnapshot final {
    std::size_t sourceNodeId;
    std::size_t targetNodeId;
    std::uint64_t relationshipGeneration;
    std::uint64_t stateVersion;
    bool relationshipPresent;
};
```

The snapshot is internal production data. Callers cannot supply arbitrary
snapshot values.

The capture operation must happen while a valid mesh lifecycle lease exists.

---

## 28. Freshness semantics

Freshness answers only:

```text
Is the request still bound to the current transition state version?
```

V1 rule:

```text
fresh =
    snapshot.stateVersion
    == request.binding.stateVersion
```

It does not check relationship existence or generation.

A stale request is not rewritten to the current state version.

---

## 29. Revalidation semantics

Revalidation answers only:

```text
Does the exact relationship identity bound into the request still exist?
```

V1 rule:

```text
revalidated =
    snapshot.relationshipPresent
    && snapshot.sourceNodeId == request.binding.relationship.sourceNodeId
    && snapshot.targetNodeId == request.binding.relationship.targetNodeId
    && snapshot.relationshipGeneration
       == request.binding.relationship.generation
```

State-version equality is deliberately excluded from revalidation because that is
the freshness channel.

A relationship recreated for the same node pair with a new generation fails
revalidation.

---

## 30. Independent restricted-origin records

Freshness and revalidation are separate production records, not raw booleans.

Candidate identities:

```cpp
class FreshnessDecisionId final;
class RevalidationDecisionId final;
```

Each is:

- opaque 128-bit;
- no public default/raw-byte construction;
- all-zero invalid;
- production-origin only;
- unique per completed decision attempt.

Candidate records:

```cpp
class ProductionFreshnessPrerequisiteRecord final {
public:
    [[nodiscard]] const FreshnessDecisionId& decisionId() const noexcept;
    [[nodiscard]] const ProductionTransitionRequestBinding&
        binding() const noexcept;
    [[nodiscard]] std::uint64_t observedStateVersion() const noexcept;
    [[nodiscard]] bool satisfied() const noexcept;
};

class ProductionRevalidationPrerequisiteRecord final {
public:
    [[nodiscard]] const RevalidationDecisionId& decisionId() const noexcept;
    [[nodiscard]] const ProductionTransitionRequestBinding&
        binding() const noexcept;
    [[nodiscard]] std::uint64_t observedRelationshipGeneration() const noexcept;
    [[nodiscard]] bool relationshipPresent() const noexcept;
    [[nodiscard]] bool satisfied() const noexcept;
};
```

Both records preserve the exact request binding.

---

## 31. Live validity result

Candidate output:

```cpp
class ProductionTransitionLiveValidityEvidence final {
public:
    [[nodiscard]] const ProductionDerivedTransitionRequest&
        request() const noexcept;

    [[nodiscard]] const ProductionFreshnessPrerequisiteRecord&
        freshness() const noexcept;

    [[nodiscard]] const ProductionRevalidationPrerequisiteRecord&
        revalidation() const noexcept;
};
```

This wrapper is also restricted-origin.

It does not contain invariant, resilience, permission, eligibility, authority, or
execution data.

---

## 32. Producer API

Candidate producer:

```cpp
class ProductionTransitionLiveValidityEvaluator final {
public:
    [[nodiscard]]
    std::optional<ProductionTransitionLiveValidityResult>
    evaluate(const ProductionDerivedTransitionRequest& request) const;
};
```

The evaluator is mesh-bound through a lifecycle-safe façade created by
`SpatialAdaptiveMesh`, following the accepted lease/invalidation pattern.

It does not accept caller-provided mesh state, generation, or state version.

---

## 33. Result semantics

Candidate result:

```cpp
enum class TransitionLiveValidityReason : std::uint8_t {
    RequestLineageInconsistent,
    RelationshipUnavailable,
    InternalSnapshotFailure
};

class ProductionTransitionLiveValidityRejection final;

using ProductionTransitionLiveValidityResult =
    std::variant<
        ProductionTransitionLiveValidityEvidence,
        ProductionTransitionLiveValidityRejection>;
```

Important distinction:

- stale state is a valid freshness record with `satisfied=false`;
- changed/missing relationship is a valid revalidation record with
  `satisfied=false`;
- those are not internal errors.

Typed rejection is reserved for inability to establish a trustworthy live
evaluation at all.

---

## 34. No duplicated policy semantics

V1 freshness and revalidation are structural runtime-validity rules, not
caller-configurable policy thresholds.

Therefore this first sublayer does not need a caller-selectable policy snapshot.

If future semantics introduce age windows, tolerated versions, relationship
aliases, or other policy choices, that change requires a separately versioned
policy layer and a new acceptance gate.

---

## 35. C1 conversion remains deferred

This sublayer does not directly construct C1
`FreshnessPrerequisiteEvidence` or `RevalidationPrerequisiteEvidence`.

Reason:

the live-validity records are evidence-producing records with their own decision
provenance, while the C1 types are minimal evaluator inputs.

A later narrow adapter may convert:

```text
ProductionFreshnessPrerequisiteRecord
-> FreshnessPrerequisiteEvidence

ProductionRevalidationPrerequisiteRecord
-> RevalidationPrerequisiteEvidence
```

only after the conversion boundary is separately reviewed.

This prevents the live evaluator from silently growing into a C1 prerequisite
assembler.

---

## 36. Lifetime model

The evaluator must use the same strength of lifetime protection already accepted
for runtime evaluators:

```text
mesh owns shared binding state
-> evaluator façade holds shared binding state
-> evaluate() acquires lease
-> coherent snapshot captured while lease is valid
-> decision records created
-> lease released
```

On mesh destruction:

```text
stop new leases
-> drain active leases
-> invalidate owner
-> destroy mesh
```

Stale evaluator façades fail closed.

No public raw mesh pointer is exposed.

---

## 37. Atomic decision ordering

For one evaluate call:

```text
acquire live lease
-> establish non-zero freshness decision ID
-> establish non-zero revalidation decision ID
-> validate request lineage
-> capture one coherent snapshot
-> compute freshness
-> compute revalidation
-> construct immutable records
-> construct immutable live-validity wrapper
-> return
```

If a required decision ID cannot be established before snapshot evaluation,
return `std::nullopt` and do not emit a partial record.

No mesh state is mutated.

---

## 38. Exact verification matrix

Positive tests must prove:

- exact current state version -> freshness true;
- later state version -> freshness false;
- exact live relationship generation -> revalidation true;
- relationship removed -> revalidation false;
- same nodes reconnected with new generation -> revalidation false;
- freshness and revalidation came from one snapshot;
- both records bind the exact original request;
- both decision IDs are non-zero and distinct;
- repeated evaluation may mint new decision IDs while preserving deterministic
  satisfied values for unchanged live state;
- evaluator becomes unavailable after mesh destruction/invalidation.

Negative/compile-fail tests must prove callers cannot:

- construct either decision ID directly;
- construct either record directly;
- construct the live-validity wrapper directly;
- inject arbitrary snapshot values;
- obtain invariant/resilience/permission evidence;
- obtain C1 eligibility;
- obtain authority/capability/execution types;
- mutate mesh through this API.

---

## 39. Revised implementation sequence

The preferred sequence is now:

```text
A1. Live Validity Evidence
    - coherent snapshot
    - freshness record
    - revalidation record
    - STOP

A2. C1 freshness/revalidation adapter
    - only after A1 acceptance

B. Invariant evidence policy
C. Resilience evidence policy
D. Permission attestation producer/consumer
E. Full prerequisite set assembly
F. C1 eligibility integration
G. STOP before authority
```

This sequence introduces one new trust assumption at a time.

---

## 40. Current design disposition

The full five-prerequisite layer remains design-incomplete.

However, the Live Validity Evidence sublayer is now sufficiently narrow for
critical review because it depends only on already accepted runtime facts:

- request-bound relationship identity;
- relationship generation;
- transition state version;
- coherent topology snapshot;
- accepted lifecycle lease pattern.

Implementation must still wait for explicit acceptance of this narrowed
sublayer.


---

## 41. Critical review finding: absence must be observable

The accepted runtime helper `captureTransitionSnapshot()` is not sufficient for
Live Validity Evidence because it returns `std::nullopt` when the relationship
does not exist.

For this layer, relationship absence is not an internal capture failure. It is
valid live evidence:

```text
relationshipPresent = false
revalidation.satisfied = false
```

Therefore this layer requires a new coherent capture primitive that distinguishes:

```text
invalid node pair / unavailable mesh -> capture failure

valid node pair, relationship absent -> successful snapshot
    relationshipPresent = false

valid node pair, relationship present -> successful snapshot
    relationshipPresent = true
    relationshipGeneration = exact current generation
```

Candidate internal snapshot:

```cpp
struct TransitionLiveValiditySnapshot final {
    std::size_t sourceNodeId;
    std::size_t targetNodeId;
    std::uint64_t stateVersion;
    bool relationshipPresent;
    std::optional<std::uint64_t> relationshipGeneration;
};
```

The entire object is captured under one `shared_lock(topologyMutex)`.

---

## 42. Request state-version comparison

C1 `ProductionStateVersion` intentionally exposes no raw-value accessor.

The accepted `ProductionDerivedTransitionRequest` already preserves the exact
D9 sample state version as:

```cpp
std::uint64_t stateVersion() const noexcept;
```

and its restricted constructor creates the C1 binding from that exact same
value.

Therefore Live Validity v1 compares:

```text
snapshot.stateVersion == derivedRequest.stateVersion()
```

It does not open the C1 opaque `ProductionStateVersion` merely to inspect its
internal integer.

This avoids widening the C1 public surface.

A positive test must additionally prove that the derived request binding used by
the record is the same binding already produced by the accepted request layer.

---

## 43. Evaluation identity and channel identities

Critical review adds one parent decision identity:

```cpp
class LiveValidityDecisionId final;
```

Every completed evaluate attempt has exactly one non-zero
`LiveValidityDecisionId`.

Successful evaluation also contains two independent channel identities:

```cpp
class FreshnessDecisionId final;
class RevalidationDecisionId final;
```

Thus:

```text
LiveValidityDecisionId
    |
    +-- FreshnessDecisionId
    +-- RevalidationDecisionId
```

The parent ID identifies the coherent evaluation event.
The child IDs identify each independently consumable evidence record.

A typed rejection carries the parent `LiveValidityDecisionId`.
It does not mint freshness/revalidation records when no trustworthy snapshot
exists.

---

## 44. Revised result types

Candidate success wrapper:

```cpp
class ProductionTransitionLiveValidityEvidence final {
public:
    [[nodiscard]] const LiveValidityDecisionId&
        decisionId() const noexcept;

    [[nodiscard]] const ProductionDerivedTransitionRequest&
        request() const noexcept;

    [[nodiscard]] const ProductionFreshnessPrerequisiteRecord&
        freshness() const noexcept;

    [[nodiscard]] const ProductionRevalidationPrerequisiteRecord&
        revalidation() const noexcept;
};
```

Candidate rejection:

```cpp
class ProductionTransitionLiveValidityRejection final {
public:
    [[nodiscard]] const LiveValidityDecisionId&
        decisionId() const noexcept;

    [[nodiscard]] const TransitionRequestDecisionId&
        requestDecisionId() const noexcept;

    [[nodiscard]] TransitionLiveValidityReason
        primaryReason() const noexcept;

    [[nodiscard]] std::uint64_t reasonFlags() const noexcept;
};
```

This keeps success and rejection under one top-level decision-event identity
family.

---

## 45. Final rejection semantics for A1

For the first implementation sublayer, valid runtime changes are represented by
unsatisfied evidence records, not rejection.

Final candidate reasons:

```cpp
enum class TransitionLiveValidityReason : std::uint8_t {
    RequestLineageInconsistent,
    InvalidRelationshipIdentity,
    SnapshotUnavailable,
    InternalDecisionFailure
};
```

Notably absent:

- `RelationshipUnavailable`;
- `RelationshipGenerationChanged`;
- `StateVersionChanged`.

Those are normal evidence outcomes:

```text
relationship absent/changed -> revalidation.satisfied=false
state version changed        -> freshness.satisfied=false
```

---

## 46. Dedicated mesh binding

The existing `ProductionTransitionEvaluationBindingState` remains owned by the
older fail-closed `ProductionTransitionEvaluator` and should not be widened
with unrelated friend access.

Live Validity gets its own narrowly scoped mesh-owned binding state:

```cpp
detail::ProductionTransitionLiveValidityBindingState
```

and public façade:

```cpp
class ProductionTransitionLiveValidityEvaluator;
```

`SpatialAdaptiveMesh` exposes:

```cpp
[[nodiscard]]
ProductionTransitionLiveValidityEvaluator
productionTransitionLiveValidityEvaluator() const noexcept;
```

The binding follows the same accepted lease/drain lifecycle pattern, but its
friend surface is limited to this evaluator and its internal lease helper.

The existing `ProductionTransitionEvaluator` remains unchanged.

---

## 47. Coherent capture contract

Candidate private mesh helper:

```cpp
[[nodiscard]]
std::optional<TransitionLiveValiditySnapshot>
captureTransitionLiveValiditySnapshot(
    std::size_t sourceNodeId,
    std::size_t targetNodeId) const;
```

Under one topology shared lock it must:

1. validate source/target indexes and source != target;
2. capture current `transitionStateVersion`;
3. inspect the directional source -> target relationship;
4. if absent, return a successful snapshot with
   `relationshipPresent=false` and no generation;
5. if present, return exact current generation.

It must not call the older two-step
`captureTransitionSnapshot()+revalidateTransitionSnapshot()` path, because
that would introduce a time-of-check/time-of-use gap and would collapse absence
into capture failure.

---

## 48. Final atomic ordering

For one evaluation call:

```text
acquire live-validity lease
-> establish parent LiveValidityDecisionId
-> validate trusted derived-request lineage
-> establish FreshnessDecisionId
-> establish RevalidationDecisionId
-> capture one coherent live snapshot
-> compute both channel results from that same snapshot
-> prepare immutable record storage
-> construct freshness record
-> construct revalidation record
-> construct success wrapper
-> release lease
-> return
```

If parent decision identity cannot be established: `std::nullopt`.

If child identity establishment fails after the parent exists:
emit typed `InternalDecisionFailure` rejection using the parent ID and emit no
partial channel record.

No mesh state is mutated and no rollback is required.

---

## 49. Request-lineage validation

The evaluator must minimally require:

- non-zero transition-request decision ID;
- recognized transition-request policy descriptor/version;
- binding relationship equals preserved persistence stream-key
  source/target/generation;
- derived-request stateVersion is non-zero;
- preserved source recommendation is SUPPORT or CONSTRAIN;
- preserved D9 observation/stream/policy-evidence/source-capture IDs are
  non-zero.

The evaluator does not recompute D8/D9 policy or persistence.

This validates that the input is a trusted derived request rather than merely a
structurally copyable request binding.

---

## 50. Critical review disposition for A1

After review, the Live Validity Evidence sublayer has the following accepted
design constraints:

1. relationship absence is a successful snapshot fact, not capture failure;
2. freshness uses only current stateVersion equality;
3. revalidation uses only relationship presence + exact source/target/generation;
4. both channels consume one coherent snapshot;
5. C1 opaque state-version internals remain closed;
6. one parent evaluation decision ID plus independent freshness/revalidation
   decision IDs preserve audit structure;
7. normal staleness/topology changes produce unsatisfied evidence, not rejection;
8. a dedicated live-validity lifecycle binding is used rather than widening the
   legacy transition evaluator;
9. no C1 prerequisite conversion occurs in A1;
10. no invariant, resilience, permission, eligibility, authority, or execution
    surface occurs in A1.

One engineering design gate remains before implementation:

- exact library/CMake placement must preserve the already accepted one-way
  dependency graph around `soam-runtime`, `soam-transition-request`, and
  `soam-transition`.


---

## 51. Dependency-cycle review

A direct mesh-owned `ProductionTransitionLiveValidityEvaluator` implemented in a
new library would create an undesirable dependency problem:

```text
soam-transition-live-validity -> soam-runtime
soam-transition-live-validity -> soam-transition-request

but SpatialAdaptiveMesh would need to construct/own
a live-validity binding type from that downstream library
```

That would force `soam-runtime` to depend back on a library that already depends
on it.

This design is rejected.

---

## 52. Accepted split: runtime snapshot source + downstream evaluator

A1 is split into two narrowly scoped pieces.

### Runtime-owned source

```text
SpatialAdaptiveMesh
-> ProductionTransitionLiveSnapshotSource
-> restricted-origin ProductionTransitionLiveSnapshot
```

The source belongs to `soam-runtime` and depends on no transition-request type.

### Downstream evaluator

```text
ProductionDerivedTransitionRequest
+ ProductionTransitionLiveSnapshotSource
-> Live Validity Evidence
-> STOP
```

The evaluator belongs to a new downstream library and depends one-way on:

- `AdaptiveMesh::soam_runtime`;
- `AdaptiveMesh::soam_transition_request`.

No reverse dependency is introduced.

---

## 53. Runtime snapshot source

Candidate runtime API:

```cpp
class ProductionTransitionLiveSnapshotSource final {
public:
    ProductionTransitionLiveSnapshotSource(
        const ProductionTransitionLiveSnapshotSource&) noexcept = default;

    [[nodiscard]]
    std::optional<ProductionTransitionLiveSnapshot>
    capture(
        std::size_t sourceNodeId,
        std::size_t targetNodeId) const;

private:
    // shared runtime-owned lifecycle binding
    friend class SpatialAdaptiveMesh;
};
```

`SpatialAdaptiveMesh` exposes:

```cpp
[[nodiscard]]
ProductionTransitionLiveSnapshotSource
productionTransitionLiveSnapshotSource() const noexcept;
```

The source is read-only.

It exposes no mutation, eligibility, authority, or transition-request logic.

---

## 54. Restricted-origin live snapshot

```cpp
class ProductionTransitionLiveSnapshot final {
public:
    [[nodiscard]] std::size_t sourceNodeId() const noexcept;
    [[nodiscard]] std::size_t targetNodeId() const noexcept;
    [[nodiscard]] std::uint64_t stateVersion() const noexcept;
    [[nodiscard]] bool relationshipPresent() const noexcept;
    [[nodiscard]] std::optional<std::uint64_t>
        relationshipGeneration() const noexcept;

private:
    // runtime source construction only
};
```

Public callers may inspect a snapshot returned by the trusted runtime source, but
cannot construct or modify one.

This is evidence acquisition, not evidence evaluation.

---

## 55. Runtime source lifecycle

The source uses a runtime-owned binding state:

```cpp
detail::ProductionTransitionLiveSnapshotBindingState
```

owned by `SpatialAdaptiveMesh`.

Lifecycle:

```text
mesh creates binding state
-> snapshot source façades share binding state
-> capture() acquires lease
-> one topology shared lock
-> immutable snapshot created
-> lease released
```

On destruction:

```text
stop new snapshot leases
-> drain active snapshot leases
-> invalidate owner
-> destroy mesh
```

Stale snapshot sources return `std::nullopt`.

This lifecycle belongs entirely to `soam-runtime`.

---

## 56. Downstream live-validity library

Accepted placement:

```text
apps/soam-transition-live-validity/
```

Candidate target:

```text
soam_transition_live_validity
AdaptiveMesh::soam_transition_live_validity
```

Dependency graph:

```text
soam-domain
    ^
soam-runtime
    ^             ^
    |             |
soam-transition-request
    ^
    |
soam-transition-live-validity
```

More precisely:

```text
soam_transition_live_validity
    depends on:
        AdaptiveMesh::soam_runtime
        AdaptiveMesh::soam_transition_request

soam_transition_request
    continues to depend on:
        AdaptiveMesh::soam_runtime
        soam_transition_eligibility

soam_runtime
    does NOT depend on:
        soam_transition_request
        soam_transition_live_validity

soam_transition
    remains header-only and independent
```

The graph remains acyclic.

---

## 57. Evaluator construction

The downstream evaluator does not own or access a raw mesh pointer.

Candidate:

```cpp
class ProductionTransitionLiveValidityEvaluator final {
public:
    explicit ProductionTransitionLiveValidityEvaluator(
        ProductionTransitionLiveSnapshotSource source) noexcept;

    [[nodiscard]]
    std::optional<ProductionTransitionLiveValidityResult>
    evaluate(
        const ProductionDerivedTransitionRequest& request) const;

private:
    ProductionTransitionLiveSnapshotSource source_;
};
```

The trusted runtime source is the only live-state capability the evaluator
receives.

---

## 58. Snapshot request identity

The evaluator asks the source to capture exactly:

```text
request.binding.relationship.sourceNodeId
request.binding.relationship.targetNodeId
```

The caller cannot separately supply node IDs to `evaluate()`.

After capture, the evaluator independently checks that the snapshot source/target
equal the request-bound relationship before computing evidence.

---

## 59. Revised failure boundary

`ProductionTransitionLiveSnapshotSource::capture()` returns `std::nullopt`
only when:

- its runtime binding is stale/unavailable;
- source/target indexes are invalid;
- source == target;
- an internal trusted capture precondition fails.

A missing relationship is still a successful snapshot.

The downstream evaluator maps snapshot-source `nullopt` to typed
`SnapshotUnavailable` if the parent decision ID already exists.

---

## 60. Expected A1 implementation surface

Runtime-side additions:

```text
apps/soam-runtime/include/production_transition_live_snapshot.hpp
apps/soam-runtime/src/production_transition_live_snapshot.cpp
apps/soam-runtime/src/detail/production_transition_live_snapshot_binding.hpp
apps/soam-runtime/include/system_architecture.hpp        [narrow additions]
apps/soam-runtime/src/system_architecture.cpp            [narrow additions]
```

Downstream additions:

```text
apps/soam-transition-live-validity/CMakeLists.txt
apps/soam-transition-live-validity/include/production_transition_live_validity.hpp
apps/soam-transition-live-validity/src/production_transition_live_validity.cpp
apps/soam-transition-live-validity/src/detail/*
apps/soam-transition-live-validity/tests/*
.github/workflows/soam-transition-live-validity-validation.yml
```

No change is required to:

- C1 prerequisite evidence classes;
- C1 evaluator;
- D9 persistence semantics;
- transition-request derivation semantics;
- authority documents/types;
- mutation/execution surfaces.

---

## 61. A1 acceptance boundary

A1 implementation is acceptable only if it proves:

1. runtime snapshot source can independently build/test inside
   `soam-runtime`;
2. transition-request project remains independently buildable;
3. live-validity project composes both without reverse dependency;
4. relationship absence is represented as data, not capture failure;
5. freshness and revalidation use the same returned snapshot;
6. stale runtime source fails closed;
7. no raw mesh pointer leaves runtime internals;
8. no C1 prerequisite object is constructed;
9. no eligibility/authority/execution surface appears;
10. all prior D7/D8/D9/request/C1 regressions remain green.

With this placement, no architecture or dependency blocker remains for the A1
Live Validity Evidence implementation.


---

## 62. Accepted A1 implementation provenance

The Live Validity Evidence A1 implementation was accepted separately from this
design branch.

Acceptance evidence:

- design PR: #23;
- implementation PR: #24;
- implementation candidate head:
  `bf7361df2d7ffdb837479df9ed397e8387010109`;
- merge commit / accepted A1 baseline:
  `eceed69eeea508e3388ecc79aaf2daf2aeda8189`;
- implementation PR merged only after exact-head pre-acceptance audit;
- all 10 relevant workflows completed successfully on the exact accepted head,
  including:
  - SOAM 2.0 D7 Provenance Gate Validation;
  - SOAM 2.0 D8A Source Capture Validation;
  - SOAM 2.0 D8B Provenance Envelope Validation;
  - SOAM 2.0 D8C Provenance Admissibility Validation;
  - SOAM 2.0 D8D Versioned Interpretation Validation;
  - SOAM 2.0 Runtime Validation;
  - SOAM 2.0 Live Evaluator Validation;
  - SOAM 2.0 D9 Policy Persistence Validation;
  - SOAM 2.0 Transition Request Validation;
  - SOAM 2.0 Transition Live Validity Validation.

Accepted A1 boundary:

```text
ProductionDerivedTransitionRequest
+ trusted runtime live snapshot source
-> ProductionFreshnessPrerequisiteRecord
+ ProductionRevalidationPrerequisiteRecord
-> STOP
```

The accepted implementation preserves these design facts:

- relationship absence is a valid snapshot fact, not an internal capture error;
- freshness is state-version equality only;
- revalidation is relationship presence + exact source/target/generation only;
- both channels are derived from one coherent live snapshot;
- runtime snapshot acquisition is lifecycle-safe and read-only;
- the downstream evaluator depends one-way on runtime/request surfaces;
- C1 prerequisite conversion is not implemented;
- invariant, resilience, permission, eligibility, authority, capability, and
  execution remain outside A1.

The accepted trust invariant remains:

```text
recommendation
!= request
!= live validity evidence
!= prerequisite set
!= eligibility
!= permission
!= authority
!= execution
```

This design branch itself contains no executable A1 implementation. Its
post-acceptance purpose is durable architecture and audit provenance.

---

## 63. Remaining design scope

Acceptance of A1 does not accept the full five-prerequisite layer.

Still unresolved and deferred:

- A2 conversion from trusted Live Validity records into the minimal C1
  Freshness/Revalidation prerequisite inputs;
- invariant prerequisite semantics;
- resilience prerequisite semantics;
- permission attestation semantics and producer;
- complete prerequisite set assembly;
- C1 eligibility integration;
- any authority/capability/execution layer.

Each remains a separate acceptance gate.
