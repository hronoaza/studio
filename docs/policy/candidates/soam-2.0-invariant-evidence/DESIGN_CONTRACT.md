# SOAM 2.0 — Invariant Evidence Design Contract

## Status

- Layer: B1 — invariant prerequisite evidence design
- Document class: design-contract candidate
- Design origin baseline: `28f2370f3b8f2c2474bfbbdfc67474ceafc87db8`
- Accepted upstream:
  - A1 Live Validity Evidence
  - A2 Live Validity -> C1 Freshness/Revalidation adapter
- Downstream contract: C1 `InvariantPrerequisiteEvidence`
- Acceptance status: design-review pending
- Executable implementation: none
- Authority effect: none

## 1. Boundary

Accepted upstream currently provides:

```text
ProductionDerivedTransitionRequest
+ trusted live validity evidence
-> FreshnessPrerequisiteEvidence
+ RevalidationPrerequisiteEvidence
-> STOP
```

B1 defines only the missing invariant channel:

```text
ProductionDerivedTransitionRequest
+ trusted invariant policy
+ trusted read-only invariant state/projection inputs
-> ProductionInvariantPrerequisiteRecord
-> later C1 InvariantPrerequisiteEvidence adapter
-> STOP
```

B1 does not produce:
- PermissionPrerequisiteEvidence;
- ResiliencePrerequisiteEvidence;
- ProductionTransitionPrerequisiteSet;
- ProductionTransitionEligibilityDecision;
- authority/capability/execution output.

## 2. Principal separation

```text
request
!= invariant policy
!= invariant observation
!= invariant projection
!= invariant evidence
!= eligibility
!= permission
!= authority
!= execution
```

Invariant satisfaction must not be inferred from request existence, request direction, D9 recommendation, live-validity success, GitHub permissions, or test success.

## 3. Existing runtime invariant is insufficient by itself

Current runtime exposes:

```cpp
struct IdentityInvariant {
    double baseline;
    double maxEpsilon;

    bool isWithinSafetyBound(double state) const;
};
```

This answers only whether one concrete node state lies within one local safety bound.

It does not define whether a requested `BridgeCouplingAdjustmentV1` transition preserves all transition-relevant invariants.

Therefore the following shortcut is rejected:

```text
current source/target state within IdentityInvariant
=> InvariantPrerequisiteEvidence{satisfied=true}
```

## 4. Transition class scope

B1 V1 is limited to the already accepted transition class represented by the trusted transition-request policy as `BridgeCouplingAdjustmentV1`.

No generic transition-class registry or caller-selected class semantics are introduced.

Any unrecognized transition class fails closed.

## 5. Required invariant domains

B1 distinguishes at least two domains:

1. Node identity/safety-bound invariants.
2. Relationship/topology invariants relevant to the requested bridge adjustment.

A future extension may add more domains, but V1 must explicitly enumerate every domain it claims to cover.

A satisfied result is valid only when every required V1 invariant domain is satisfied.

## 6. Endpoint coverage

For `BridgeCouplingAdjustmentV1`, both request-bound endpoints are in scope.

Invariant evaluation must bind exactly to:

```text
request.binding.relationship.sourceNodeId
request.binding.relationship.targetNodeId
request.binding.relationship.generation
request.binding.direction
request.binding.transitionClass
request.binding.stateVersion
```

No caller may replace either endpoint or relationship identity.

## 7. Current-state-only evaluation is not accepted

Current node state alone cannot prove transition safety because the requested transition intends to alter bridge coupling semantics.

B1 therefore requires a deterministic transition projection before a satisfied invariant result may be emitted.

The projection is observational and side-effect free.

## 8. Deterministic projection requirement

Candidate V1 projection boundary:

```text
trusted request
+ coherent current transition/invariant snapshot
+ trusted invariant policy snapshot
-> projected post-transition invariant state
-> invariant evaluation
```

The projection must:
- perform no mesh mutation;
- perform no authority check;
- perform no eligibility evaluation;
- use no randomness;
- use no wall-clock input;
- use no caller-selected thresholds;
- be deterministic for identical immutable inputs.

## 9. Projection semantics are a design gate

Implementation must not begin until V1 defines exactly what SUPPORT and CONSTRAIN project.

At minimum the contract must specify:
- which bridge property is projected;
- exact delta or transformation rule;
- legal bounds;
- whether symmetric reverse-edge state is projected;
- whether node state is projected or only bridge state;
- whether capacity/status/effective coupling participate;
- behavior at saturation boundaries.

Until these rules are accepted, B1 remains design-only.

## 10. No mutation-as-validation

The evaluator must not apply the transition and then inspect whether invariants survived.

Rejected pattern:

```text
mutate mesh
-> test invariants
-> rollback
```

Accepted pattern:

```text
capture trusted immutable state
-> pure deterministic projection
-> evaluate invariants
-> emit evidence
-> STOP
```

## 11. Coherent invariant snapshot

Invariant evaluation must consume one coherent trusted snapshot sufficient for every invariant domain claimed by V1.

The snapshot must not be assembled from separate unlocked public getters such as `getNodeState()`, `getNodeHealth()`, and bridge reads.

Candidate snapshot fields may include:
- source/target node identity;
- exact relationship generation;
- transition state version;
- source/target current state;
- source/target `IdentityInvariant` parameters;
- exact directed bridge capacity/status/generation;
- corresponding reverse-edge data if symmetry is a V1 invariant.

Final field set remains tied to accepted projection semantics.

## 12. Snapshot origin

The snapshot must be runtime-origin, read-only, restricted construction, and lifecycle-safe.

A caller may not fabricate node state, invariant parameters, bridge capacity, status, generation, or state version.

If the runtime source is stale/unavailable, evaluation fails closed.

## 13. Request freshness is not reimplemented

B1 must not duplicate A1/A2 freshness semantics.

If B1 requires an exact state-version match as an evaluation precondition, it may reject/fail closed when its snapshot no longer matches the request-bound state version.

It must not create a second FreshnessPrerequisiteEvidence or reinterpret A2 freshness.

## 14. Relationship revalidation is not reimplemented

B1 may require that the exact request-bound relationship still exists with the same generation before projecting.

A mismatch means invariant evaluation is unavailable or unsatisfied according to the final B1 rejection contract.

It must not create a second RevalidationPrerequisiteEvidence.

## 15. Invariant policy must be versioned

B1 requires a trusted immutable policy snapshot with:
- opaque policy snapshot ID;
- semantic policy ID;
- major/minor version;
- implementation revision kind;
- reproducible implementation digest;
- fixed V1 projection semantics;
- fixed V1 invariant-domain list.

No caller-selected `maxEpsilon`, deltas, thresholds, or domain masks may enter a trusted production policy snapshot.

## 16. Runtime IdentityInvariant ownership

Existing per-node `IdentityInvariant.baseline` and `maxEpsilon` remain runtime state owned by the node.

B1 policy does not overwrite them.

The trusted snapshot may carry their exact captured values solely for deterministic evaluation.

## 17. Candidate evidence record

B1 should introduce a restricted-origin record distinct from the C1 evidence type:

```cpp
class InvariantDecisionId final;

class ProductionInvariantPrerequisiteRecord final {
public:
    const InvariantDecisionId& decisionId() const noexcept;
    const ProductionTransitionRequestBinding& binding() const noexcept;
    const InvariantPolicySnapshotId& policySnapshotId() const noexcept;
    bool satisfied() const noexcept;

private:
    // evaluator-only construction
};
```

The record may additionally expose typed diagnostic facts if they are immutable and do not widen authority.

## 18. Diagnostic facts

If diagnostics are exposed, they must distinguish facts from the final boolean.

Candidate facts:
- source identity invariant satisfied;
- target identity invariant satisfied;
- topology/symmetry invariant satisfied;
- projected bridge value within legal bound;
- projection available.

Diagnostics must not expose mutation handles or permit callers to reconstruct trusted evidence.

## 19. Decision identity

Every completed B1 invariant evaluation event should have one non-zero opaque 128-bit `InvariantDecisionId`.

Properties:
- no public default construction;
- no raw-byte public construction;
- production-origin only;
- copyable after creation;
- OS-backed generation consistent with accepted decision-ID families.

If identity generation fails before a decision event exists, return `std::nullopt`.

## 20. Candidate evaluator

Candidate shape:

```cpp
class ProductionTransitionInvariantEvaluator final {
public:
    std::optional<ProductionTransitionInvariantResult>
    evaluate(
        const ProductionDerivedTransitionRequest& request,
        const TransitionInvariantPolicySnapshot& policy) const;

private:
    ProductionTransitionInvariantSnapshotSource source_;
};
```

The evaluator receives only a trusted read-only snapshot capability, never a raw mesh pointer.

## 21. Result model

Candidate result:

```text
ProductionInvariantPrerequisiteRecord
OR
ProductionTransitionInvariantRejection
```

A rejection is for inability to make a trusted invariant decision.

A legitimate projected invariant violation should normally produce a valid record with `satisfied=false`, not a rejection.

## 22. Candidate rejection reasons

Design candidate reasons:

```cpp
enum class TransitionInvariantReason : std::uint8_t {
    RequestLineageInconsistent,
    TransitionClassUnsupported,
    PolicyRevisionUnrecognized,
    SnapshotUnavailable,
    RequestStateVersionMismatch,
    RelationshipIdentityMismatch,
    ProjectionUnavailable,
    InternalDecisionFailure
};
```

The final reason ordering and reason flags remain design-review items.

## 23. Unsatisfied evidence vs rejection

Examples of valid unsatisfied evidence:
- projected source state violates its captured identity bound;
- projected target state violates its captured identity bound;
- projected bridge property exceeds a fixed accepted policy bound;
- projected topology rule fails.

Examples of rejection:
- trusted snapshot cannot be obtained;
- policy is unrecognized;
- request lineage is inconsistent;
- projection semantics do not apply to the transition class;
- snapshot no longer corresponds to the exact request lineage.

## 24. Restricted C1 conversion seam

B1 itself should stop at `ProductionInvariantPrerequisiteRecord`.

A later narrow adapter may convert the accepted record into C1 `InvariantPrerequisiteEvidence`.

That adapter, if introduced, may receive a friend seam only for `InvariantPrerequisiteEvidence`.

It must not receive friendship for Permission, Resilience, Freshness, Revalidation, request binding, eligibility decision, authority, capability, or execution types.

## 25. No full prerequisite assembly

B1 must not construct `ProductionTransitionPrerequisiteSet`.

At B1 completion, the system will still lack:
- ResiliencePrerequisiteEvidence;
- PermissionPrerequisiteEvidence.

Therefore full C1 eligibility integration remains intentionally incomplete.

## 26. No eligibility call

B1 must not instantiate or invoke `ProductionTransitionEligibilityEvaluator`.

`invariant=true` does not imply eligible.

## 27. No resilience inference

Node health, bridge capacity, bridge status, or topology redundancy must not be silently promoted into `ResiliencePrerequisiteEvidence`.

B1 may inspect a value only when that value is explicitly required by an accepted invariant rule.

Resilience semantics remain layer C.

## 28. No permission inference

Invariant satisfaction must never create or imply permission.

No Root Operator state, repository permission, account identity, policy ownership, or technical capability constitutes B1 permission evidence.

## 29. Dependency placement

Preferred decomposition mirrors A1:

Runtime-owned:
```text
trusted ProductionTransitionInvariantSnapshotSource
```

Downstream:
```text
apps/soam-transition-invariant/
```

Candidate target:
```text
soam_transition_invariant
AdaptiveMesh::soam_transition_invariant
```

The downstream evaluator may depend one-way on:
- `AdaptiveMesh::soam_runtime`;
- `AdaptiveMesh::soam_transition_request`.

Runtime must not depend back on the downstream invariant library.

## 30. Runtime source must remain generic enough for B1 only

The runtime source should expose only the immutable facts required by the accepted B1 projection/invariant contract.

It must not become a generic unrestricted mesh-introspection API.

## 31. Compile-fail requirements

Before B1 implementation acceptance, negative tests must prove arbitrary callers cannot:
1. default/raw-byte construct invariant decision IDs;
2. construct trusted invariant policy snapshots;
3. fabricate invariant snapshots;
4. inject caller-selected projection parameters or thresholds;
5. directly construct `ProductionInvariantPrerequisiteRecord`;
6. directly construct C1 `InvariantPrerequisiteEvidence`;
7. use invariant access to construct Permission/Resilience/Freshness/Revalidation evidence;
8. construct request bindings through the invariant layer;
9. invoke C1 eligibility through the invariant evaluator;
10. obtain authority/capability/execution objects;
11. mutate mesh state through the invariant API.

## 32. Positive verification requirements

Implementation acceptance must prove:
- both request-bound endpoints are evaluated where required;
- exact request binding is preserved;
- recognized transition class only;
- identical immutable inputs produce identical invariant semantics;
- legitimate violation emits `satisfied=false`;
- state-version/relationship-lineage drift fails closed;
- no mesh mutation occurs;
- no random ID beyond the invariant decision identity is minted;
- no eligibility evaluation occurs;
- no resilience or permission evidence is produced.

## 33. Regression boundary

B1 CI must compose and keep green:
- domain core;
- runtime;
- D7/D8/D9;
- transition request;
- A1 Live Validity;
- A2 C1 live-validity adapter;
- C1 eligibility contract tests;
- B1 invariant tests and compile-fail tests.

## 34. Security property

```text
freshness=true
&& revalidation=true
&& invariant=true
```

still does not imply:
- resilience;
- permission;
- eligibility;
- authorization;
- capability;
- execution.

## 35. Critical design gates before implementation

B1 implementation MUST NOT begin until review accepts all of:

1. exact SUPPORT projection semantics;
2. exact CONSTRAIN projection semantics;
3. exact projected property/properties;
4. whether source state, target state, or bridge-only state changes in projection;
5. exact relationship symmetry rule;
6. exact invariant-domain list;
7. exact coherent snapshot fields;
8. exact trusted policy descriptor/version;
9. exact rejection precedence;
10. exact runtime-source/CMake placement.

## 36. Current finding

The repository contains a valid local `IdentityInvariant` primitive, but no accepted transition projection contract for `BridgeCouplingAdjustmentV1`.

Therefore B1 can define architecture, provenance, fail-closed behavior, and trust boundaries now, but it must not yet define `InvariantPrerequisiteEvidence=true` from current state alone.

## 37. B1 STOP

B1 ends after an accepted design for producing one trusted invariant prerequisite record.

No runtime implementation is accepted by this document.
No C1 invariant adapter is accepted by this document.
No full prerequisite set is assembled.
No eligibility decision is produced.
No authority boundary is crossed.


## 38. Critical review — existing semantics inventory

Review of the accepted baseline confirms that the transition-request layer currently defines only:

```text
PersistentBridgeRecommendation::SUPPORT
-> RequestedTransitionDirection::support

PersistentBridgeRecommendation::CONSTRAIN
-> RequestedTransitionDirection::constrain
```

and binds both directions to the trusted transition class identity represented internally as `BridgeCouplingAdjustmentV1`.

No accepted production code currently defines:
- a SUPPORT coupling delta;
- a CONSTRAIN coupling delta;
- a multiplicative coupling factor;
- an absolute capacity target;
- a projected bridge status transition;
- projected node-state mutation;
- saturation behavior;
- reverse-edge/symmetry update semantics;
- execution semantics for this transition class.

Therefore B1 must not infer any of those semantics from names alone.

## 39. Critical review — local runtime behavior is not transition semantics

Existing runtime bridge behavior such as:

```text
DESTRUCTIVE_DRIFT -> capacity *= 0.85
CREATIVE_SIGNAL  -> capacity += 0.15
NOISE            -> capacity = 0 / ISOLATED
```

belongs to the simulation/meta-evaluation path.

Those rules are not accepted as the execution semantics of `BridgeCouplingAdjustmentV1`.

B1 must not reuse the numeric constants `0.85`, `0.15`, or any existing bridge-state update rule unless a separate design decision explicitly adopts them for the transition class.

## 40. Critical review — no semantic equivalence with D9 recommendation magnitude

D9 persistence yields a categorical recommendation:

```text
SUPPORT | CONSTRAIN | PRESERVE
```

The accepted transition request preserves only direction and lineage.

It does not preserve a trusted numeric adjustment magnitude.

Therefore B1 cannot derive a projection magnitude from:
- `BridgePolicyEvidence::value()`;
- confidence;
- persistence threshold distance;
- recommendation duration;
- sample count;
- current bridge capacity;
- node health.

Any such mapping would be a new policy and requires explicit versioned acceptance.

## 41. Critical review — invariant domain split

The review separates candidate invariant domains into:

### 41.1 Structural invariants

Facts that can be stated independently of adjustment magnitude:
- exact request-bound source/target identity;
- source != target;
- exact relationship generation;
- relationship still exists where projection requires it;
- directed edge identity remains coherent;
- reverse-edge symmetry remains coherent if V1 execution semantics require symmetric mutation;
- projected bridge fields remain representable and finite.

### 41.2 State safety invariants

Facts that depend on the actual projected transition:
- whether projected source-node state remains within its `IdentityInvariant`;
- whether projected target-node state remains within its `IdentityInvariant`;
- whether projected bridge capacity remains within accepted bounds;
- whether projected status/coupling combination is valid.

Because V1 projection semantics are not yet defined, state safety invariants cannot yet produce a trusted `satisfied=true` result.

## 42. Critical review — present-state validity is only a precondition

Current-state checks may be used only as fail-closed preconditions.

Examples:
- current node state is finite;
- captured `IdentityInvariant` parameters validate;
- current bridge capacity/status are internally valid;
- relationship identity matches the request.

Passing these checks proves only that projection may be attempted.

It does not prove that the requested transition preserves invariants.

## 43. Critical review — required next design artifact

Before runtime implementation, B1 requires one additional normative design artifact:

```text
BridgeCouplingAdjustmentV1 Projection Contract
```

This contract must define exactly:

1. SUPPORT transformation;
2. CONSTRAIN transformation;
3. projected field set;
4. numeric parameters and legal ranges;
5. saturation behavior;
6. directed vs symmetric application semantics;
7. node-state impact, if any;
8. bridge-status impact, if any;
9. effective-coupling interpretation;
10. deterministic failure cases.

The projection contract must be versioned and must not be derived implicitly from simulation heuristics.

## 44. Critical review — preferred minimal V1 scope

To minimize trust expansion, the preferred V1 design should attempt to project the smallest state surface capable of representing the requested class.

Preferred review order:

```text
bridge-only projection
-> verify whether node-state projection is actually necessary
-> add node-state projection only if required by accepted semantics
```

This is a design preference, not an accepted transformation rule.

No specific delta, multiplier, or target value is accepted by this section.

## 45. Critical review — rejection vs unsatisfied disposition

Until projection semantics exist:

- unsupported/unrecognized projection semantics => rejection/fail closed;
- malformed/unavailable trusted snapshot => rejection/fail closed;
- exact trusted projection that violates an invariant => valid evidence with `satisfied=false`.

This distinction is accepted for B1.

## 46. Critical review — dependency disposition

The proposed split remains sound:

```text
soam-runtime
    owns restricted read-only invariant snapshot source

soam-transition-invariant
    consumes runtime snapshot source
    + trusted transition request
    + trusted invariant policy
```

No reverse dependency is required.

However, the final runtime snapshot field set remains blocked on the projection contract and must not be implemented speculatively.

## 47. Critical review verdict

PR #27 is accepted as a correct boundary/gate document, but it is not yet implementation-ready.

Closed design questions:
- invariant evidence is independent from freshness/revalidation/resilience/permission;
- current-state-only success is rejected;
- mutation-as-validation is rejected;
- exact request binding is mandatory;
- trusted versioned policy is mandatory;
- deterministic pure projection is mandatory;
- full prerequisite assembly and eligibility remain outside B1;
- dependency direction is constrained and acyclic.

Open blocking questions:
- SUPPORT projection semantics;
- CONSTRAIN projection semantics;
- projected field set;
- numeric transformation parameters;
- symmetry semantics;
- node-state impact;
- invariant-domain final list;
- final snapshot field set;
- policy descriptor contents tied to the projection;
- final rejection precedence.

No runtime implementation may begin while any of these remain open.

## 48. Next design step

The next accepted work item is:

```text
B1.1 — BridgeCouplingAdjustmentV1 Projection Contract
```

Only after B1.1 is critically reviewed and accepted may B1 move to executable invariant evidence implementation.

## 49. Review STOP

Critical review stops here.

No production code change is authorized by this review.
No projection numeric rule is invented.
No invariant evidence is emitted.
No C1 invariant adapter is implemented.
No eligibility or authority surface is opened.


## Accepted B1 implementation provenance

The executable B1 implementation was reviewed and accepted separately.

Acceptance evidence:

- runtime implementation PR: #29;
- accepted implementation head: `990e69176c015d3273147133d0dbdaca49249db9`;
- accepted merge/current baseline: `b18efd22992af82654217cd38015b691275336a7`;
- exact-head validation: 13/13 workflows completed successfully, including
  Transition Invariant, Runtime, D7, D8A/B/C/D, D9, Transition Request,
  Transition Live Validity, Live Validity C1 Adapter, Transition Eligibility,
  and Live Evaluator.

Accepted implementation boundary:

```text
trusted derived transition request
+ trusted/versioned invariant projection policy
+ lifecycle-safe coherent invariant snapshot
-> ProductionInvariantPrerequisiteRecord
   OR typed invariant rejection
-> STOP
```

The accepted runtime implementation does not construct C1
`InvariantPrerequisiteEvidence`, does not assemble the full prerequisite set,
does not invoke C1 eligibility, and does not create resilience, permission,
authority, capability, or execution artifacts.

This document now serves as durable design/audit provenance for the accepted B1
runtime layer.
