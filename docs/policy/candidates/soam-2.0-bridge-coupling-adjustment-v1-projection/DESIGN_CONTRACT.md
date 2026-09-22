# SOAM 2.0 — BridgeCouplingAdjustmentV1 Projection Contract

## Status

- Layer: B1.1 — invariant projection semantics
- Document class: design-contract candidate
- Design origin baseline: `28f2370f3b8f2c2474bfbbdfc67474ceafc87db8`
- Upstream: accepted transition request + accepted A1/A2 live-validity path
- Downstream: B1 invariant evidence design PR #27
- Executable implementation: none
- Acceptance status: critical-review pending
- Authority effect: none

## 1. Purpose

B1 cannot produce trusted invariant evidence until the requested
`BridgeCouplingAdjustmentV1` transition has deterministic, versioned projection
semantics.

This contract defines a minimal bridge-only V1 projection for review.

It does not define execution authority, mutation permission, eligibility, or
runtime commit behavior.

## 2. Scope

V1 projects only the mutable bridge `capacity` field for the exact
request-bound relationship.

It does not project:

- node state;
- node health;
- node position;
- bridge distance;
- bridge orientationWeight;
- bridge status;
- relationship generation;
- topology membership;
- any unrelated bridge.

The projection is pure and side-effect free.

## 3. Relationship scope

The request is bound to one logical relationship identified by:

```text
sourceNodeId
targetNodeId
relationshipGeneration
```

The runtime representation is symmetric and contains two directed edges:

```text
source -> target
target -> source
```

B1.1 V1 requires both directed edges to exist and both to carry the exact same
request-bound generation before projection is available.

Each directed edge keeps its own current capacity and geometry.

## 4. V1 policy constant

V1 introduces one trusted fixed adjustment fraction:

```text
adjustmentFraction = 1/8 = 0.125
```

This value is part of the versioned projection policy and is not caller
configurable.

Rationale:

- it is exactly representable in binary floating point;
- it gives a bounded monotonic adjustment;
- it does not reuse simulation heuristics such as +0.15 or *0.85;
- it avoids an absolute target or caller-selected magnitude;
- repeated application approaches the bound gradually rather than overshooting.

No semantic meaning is inferred from the numeric value beyond this V1 policy.

## 5. SUPPORT projection

For each directed bridge capacity `c`:

```text
support(c) = c + (1 - c) * 0.125
```

Equivalent form:

```text
support(c) = 0.875*c + 0.125
```

Required properties for valid `c in [0,1]`:

- result is finite;
- result remains in `[0,1]`;
- result is never less than `c`;
- `support(1) == 1`;
- no saturation branch is needed.

## 6. CONSTRAIN projection

For each directed bridge capacity `c`:

```text
constrain(c) = c * 0.875
```

Required properties for valid `c in [0,1]`:

- result is finite;
- result remains in `[0,1]`;
- result is never greater than `c`;
- `constrain(0) == 0`;
- no saturation branch is needed.

## 7. Directional asymmetry

The two directed capacities may differ before projection.

V1 does not force them equal.

Instead the same transformation is applied independently:

```text
forwardProjected = transform(forwardCurrent)
reverseProjected = transform(reverseCurrent)
```

Structural symmetry means both directed edges exist with the same relationship
generation. It does not mean their capacities are numerically equal.

## 8. Bridge status

V1 does not project `BridgeStatus`.

The current status is retained as an observed field if B1 diagnostics need it,
but status does not determine the projected capacity.

This intentionally avoids importing simulation-state transitions into the
transition projection contract.

If a later execution layer requires coupled status mutation, that is a separate
versioned execution-policy decision.

## 9. Node state

V1 does not alter source or target node state.

Therefore:

```text
projectedSourceState = currentSourceState
projectedTargetState = currentTargetState
```

This permits B1 to evaluate both captured `IdentityInvariant` values without
inventing a node-state transition model.

A node that is already outside its identity bound cannot receive
`invariant satisfied=true` merely because bridge capacity projection is safe.

## 10. V1 invariant domains

B1 V1 must evaluate all of these domains.

### 10.1 Request/relationship identity

- exact source/target;
- source != target;
- exact relationship generation;
- forward edge exists;
- reverse edge exists;
- both edges carry the request-bound generation.

### 10.2 Current bridge validity

For each directed edge:

- capacity finite and in `[0,1]`;
- distance finite and `>= 0`;
- orientationWeight finite and in `[0,1]`.

### 10.3 Projected bridge validity

For each projected directed edge:

- projected capacity finite and in `[0,1]`;
- SUPPORT is monotonic non-decreasing;
- CONSTRAIN is monotonic non-increasing.

### 10.4 Effective coupling validity

For each direction:

```text
effectiveCoupling =
    projectedCapacity
    * (1 / (1 + 0.1 * distance))
    * orientationWeight
```

The projected effective coupling must be finite and in `[0,1]`.

This formula already exists in the accepted runtime bridge model; B1 uses it as
a deterministic validity check, not as a new recommendation policy.

### 10.5 Endpoint identity invariants

Because V1 leaves node state unchanged:

```text
sourceInvariantSatisfied =
    abs(sourceState - sourceBaseline) <= sourceMaxEpsilon

targetInvariantSatisfied =
    abs(targetState - targetBaseline) <= targetMaxEpsilon
```

Both must be satisfied.

## 11. Overall invariant result

A valid B1 invariant record is satisfied only if every V1 domain is satisfied:

```text
satisfied =
    relationshipIdentitySatisfied
    && currentBridgeValiditySatisfied
    && projectedBridgeValiditySatisfied
    && effectiveCouplingValiditySatisfied
    && sourceIdentityInvariantSatisfied
    && targetIdentityInvariantSatisfied
```

A legitimate violation produces a trusted record with `satisfied=false`.

An inability to establish trusted inputs produces a typed rejection, not a
false record.

## 12. Coherent snapshot fields

The runtime B1 snapshot must capture under one topology-consistency epoch:

- sourceNodeId;
- targetNodeId;
- stateVersion;
- relationship generation;
- forward edge present;
- reverse edge present;
- forward generation;
- reverse generation;
- forward capacity;
- reverse capacity;
- forward distance;
- reverse distance;
- forward orientationWeight;
- reverse orientationWeight;
- forward status;
- reverse status;
- source node state;
- target node state;
- source IdentityInvariant baseline/maxEpsilon;
- target IdentityInvariant baseline/maxEpsilon.

The snapshot is immutable and restricted-origin.

No unlocked assembly from public getters is allowed.

## 13. Request epoch precondition

B1 does not reissue Freshness evidence.

However projection requires the invariant snapshot to correspond to the exact
request epoch:

```text
snapshot.stateVersion == request.stateVersion
```

A mismatch is a typed rejection:
`RequestStateVersionMismatch`.

This is a B1 evaluation precondition, not a second freshness decision.

## 14. Relationship precondition

Projection also requires exact request-bound relationship identity.

Any missing edge or generation mismatch is a typed rejection:
`RelationshipIdentityMismatch`.

This is an invariant-evaluation availability condition, not a second
revalidation record.

## 15. Policy identity

Candidate policy family:

```text
BridgeCouplingAdjustmentProjectionPolicyV1
major = 1
minor = 0
adjustmentFraction = 0.125
transitionClass = BridgeCouplingAdjustmentV1
```

The production policy snapshot must include:

- opaque policy snapshot ID;
- semantic policy ID;
- version;
- implementation revision kind;
- reproducible implementation digest;
- fixed transition class;
- fixed adjustmentFraction.

No caller-selected projection parameter is accepted.

## 16. Determinism

Projection uses only immutable request, policy, and snapshot values.

It uses no:

- wall clock;
- randomness;
- external service;
- mesh mutation;
- global mutable threshold;
- caller override.

Identical inputs produce identical projected values and invariant semantics.

## 17. Numeric behavior

All arithmetic uses binary64 `double`.

Inputs must be finite before projection.

Implementations must reject non-finite intermediate or result values.

Tests must include exact boundary vectors:

```text
c = 0
SUPPORT -> 0.125
CONSTRAIN -> 0

c = 0.5
SUPPORT -> 0.5625
CONSTRAIN -> 0.4375

c = 1
SUPPORT -> 1
CONSTRAIN -> 0.875
```

Additional near-boundary vectors are required.

## 18. No execution semantics

This contract defines what B1 projects for invariant evaluation.

It does not authorize a runtime mutation to apply the same transformation.

A future execution layer may reuse the accepted projection policy only after
separate authority/capability/commit design.

Therefore:

```text
projection semantics != execution authority
```

## 19. No recommendation reinterpretation

D9 evidence magnitude, confidence, persistence duration, sample count, node
health, and current capacity do not select the projection fraction.

Only the categorical request direction selects SUPPORT vs CONSTRAIN.

## 20. Rejection precedence candidate

Candidate B1 precedence after decision-ID establishment:

1. RequestLineageInconsistent
2. TransitionClassUnsupported
3. PolicyRevisionUnrecognized
4. SnapshotUnavailable
5. RequestStateVersionMismatch
6. RelationshipIdentityMismatch
7. ProjectionUnavailable
8. InternalDecisionFailure

A projected invariant violation is not a rejection.

## 21. Diagnostic record candidate

B1 may expose immutable diagnostics:

- sourceIdentityInvariantSatisfied;
- targetIdentityInvariantSatisfied;
- forwardProjectedCapacity;
- reverseProjectedCapacity;
- forwardEffectiveCouplingValid;
- reverseEffectiveCouplingValid;
- structuralRelationshipInvariantSatisfied.

Diagnostics are observational only.

## 22. Security boundary

Even when:

```text
freshness = true
revalidation = true
invariant = true
```

the system still lacks:

- resilience evidence;
- permission evidence;
- complete prerequisite assembly;
- eligibility decision;
- authority;
- capability;
- execution.

## 23. Verification requirements

Before implementation acceptance, tests must prove:

- SUPPORT vectors exactly;
- CONSTRAIN vectors exactly;
- both directed edges projected independently;
- relationship generation mismatch rejects;
- missing reverse edge rejects;
- stateVersion mismatch rejects;
- current invalid bridge input fails closed;
- projected capacity never escapes `[0,1]`;
- node state is unchanged;
- both endpoint IdentityInvariant values are checked;
- no mesh mutation occurs;
- no status mutation occurs;
- no caller-selected adjustment fraction exists;
- no resilience/permission/eligibility/authority/execution output exists.

## 24. Design questions closed by this candidate

This candidate supplies concrete answers for the B1 blockers:

- projected property: bridge capacity only;
- SUPPORT: bounded 1/8 movement toward 1;
- CONSTRAIN: 7/8 of current capacity;
- numeric parameter: fixed 0.125;
- symmetric relationship handling: both directed edges required, each transformed independently;
- node-state impact: none;
- bridge-status impact: none;
- invariant domains: relationship, bridge validity, effective coupling validity, both endpoint identity invariants;
- snapshot field set: explicitly enumerated;
- policy shape: versioned V1 with reproducible implementation revision;
- rejection precedence: explicitly proposed.

## 25. STOP

This is design only.

No runtime snapshot source is added.
No projection evaluator is implemented.
No InvariantPrerequisiteEvidence is created.
No C1 eligibility decision is produced.
No authority or execution boundary is crossed.


## 26. Critical review — policy constant disposition

The value `0.125` is mathematically well-behaved but is not derivable from
accepted D9 evidence, persistence duration, confidence, runtime simulation
constants, or any other existing trusted semantic source.

Therefore:

```text
adjustmentFraction = 0.125
```

remains a candidate V1 policy choice until explicitly accepted.

Its favorable properties are engineering rationale only:

- exactly representable in binary64;
- bounded monotonic transform;
- no overshoot for valid capacity;
- independent from existing simulation constants.

These properties do not by themselves establish domain correctness.

No implementation may treat `0.125` as accepted until this projection contract
passes the separate acceptance gate.

## 27. Critical review — stronger reciprocal structural invariant

Review of the accepted runtime confirms `connectNodes()/connectPairsUnlocked()`
creates one logical relationship as two directed edges with:

- reciprocal targets;
- one shared relationship generation;
- one shared distance value;
- direction-specific orientationWeight;
- initially equal capacity/status, though later runtime evolution may diverge.

Therefore B1.1 structural validity must require:

```text
forward.target == requested target
reverse.target == requested source
forward.generation == request generation
reverse.generation == request generation
forward.distance == reverse.distance
```

The distance equality may be checked as exact binary64 equality because the
accepted connection constructor computes the distance once and copies that same
value into both edges.

Capacity equality is NOT required.

Orientation-weight equality is NOT required.

## 28. Critical review — ISOLATED status

The accepted runtime can temporarily contain an edge whose status is
`BridgeStatus::ISOLATED` before `pruneIsolatedBridges()` removes the logical
pair.

B1.1 V1 must not project SUPPORT or CONSTRAIN capacity over such a relationship
while retaining `ISOLATED` status.

Therefore:

```text
forward.status == ISOLATED
OR reverse.status == ISOLATED
=> ProjectionUnavailable rejection
```

This is not `InvariantPrerequisiteEvidence{satisfied=false}`, because the V1
projection semantics are intentionally undefined for an isolated relationship.

No rule is introduced here that infers resilience from NORMAL/DAMPING/RECOVERY
status.

## 29. Critical review — non-isolated status preservation

For `NORMAL`, `DAMPING`, and `RECOVERY`, V1 keeps status unchanged during
projection.

This means B1.1 evaluates only whether the capacity projection is structurally
and numerically admissible under the retained observed status.

It does not claim that a future execution layer must preserve status. Execution
remains a separate versioned design gate.

## 30. Critical review — effective coupling check is derived, not independent

Given valid:

```text
capacity in [0,1]
distance >= 0
orientationWeight in [0,1]
```

the accepted effective-coupling formula necessarily yields a finite value in
`[0,1]`, barring floating-point non-finiteness already rejected by input
validation.

Therefore effective-coupling validity is retained as a diagnostic/consistency
check, not treated as a logically independent invariant domain.

The independent V1 invariant domains are revised to:

1. exact request/relationship structural identity;
2. current bridge field validity;
3. projected bridge capacity validity and direction monotonicity;
4. source endpoint IdentityInvariant;
5. target endpoint IdentityInvariant.

## 31. Critical review — endpoint IdentityInvariant semantics

Because V1 projects no node-state change:

```text
projectedSourceState == currentSourceState
projectedTargetState == currentTargetState
```

Endpoint IdentityInvariant checks therefore establish that the unchanged
endpoints are already within their accepted safety bounds at the exact request
epoch.

They do not prove a bridge-capacity change would alter node-state safety in a
future simulation step.

That future dynamic effect is outside B1.1 and must not be implied by
`invariant=true`.

## 32. Critical review — stateVersion precondition

Requiring:

```text
snapshot.stateVersion == request.stateVersion
```

is retained.

This makes B1 evaluation intentionally conservative: any runtime state advance
after the request was derived invalidates the projection attempt.

This check is an evaluation precondition only and does not mint or replace A1
Freshness evidence.

## 33. Critical review disposition

The structural projection design is coherent with the accepted runtime model
after the corrections above.

Closed items:

- bridge-only projection surface;
- no node-state mutation;
- no bridge-status mutation;
- reciprocal edge requirement;
- exact shared generation requirement;
- exact shared distance requirement;
- independent per-direction capacity projection;
- ISOLATED relationship fail-closed;
- exact request stateVersion precondition;
- endpoint IdentityInvariant semantics;
- no resilience/permission/eligibility/authority promotion.

One normative policy decision remains deliberately open:

```text
Is adjustmentFraction = 0.125 accepted as
BridgeCouplingAdjustmentProjectionPolicyV1?
```

Until that value is explicitly accepted, B1.1 is not implementation-ready.

## 34. Review STOP

Critical review stops before implementation.

No executable code change is authorized by this review.
No C1 InvariantPrerequisiteEvidence is constructed.
No eligibility decision is produced.
No authority or execution surface is opened.


## 35. Accepted V1 projection policy constant

The remaining normative policy decision is now accepted:

```text
BridgeCouplingAdjustmentProjectionPolicyV1
adjustmentFraction = 0.125
```

This value is accepted as part of the V1 policy contract, not as a value derived
from D9 evidence, confidence, persistence duration, runtime simulation
heuristics, or caller input.

Accepted transforms:

```text
SUPPORT:
    projectedCapacity =
        currentCapacity + (1 - currentCapacity) * 0.125

CONSTRAIN:
    projectedCapacity =
        currentCapacity * 0.875
```

The same transform is applied independently to both reciprocal directed
capacities after all structural/request-epoch preconditions succeed.

No caller override is permitted.

## 36. B1.1 implementation authorization boundary

With the policy constant accepted, B1.1 now has no remaining semantic blocker
for a candidate implementation of:

- trusted/versioned projection policy snapshot;
- lifecycle-safe coherent invariant snapshot source;
- pure deterministic bridge-capacity projection;
- invariant evaluation record/rejection model;
- tests and compile-fail trust-boundary checks.

This acceptance still does not authorize:

- C1 InvariantPrerequisiteEvidence conversion;
- Resilience prerequisite evidence;
- Permission prerequisite evidence;
- full prerequisite-set assembly;
- C1 eligibility integration;
- authority/capability/execution.

Those remain separate gates.

## 37. Final reviewed B1.1 contract

The accepted candidate semantics are:

- transition class: BridgeCouplingAdjustmentV1;
- projected field: bridge capacity only;
- SUPPORT fraction: 0.125 toward 1;
- CONSTRAIN multiplier: 0.875;
- reciprocal directed edges required;
- exact shared relationship generation required;
- exact shared distance required;
- capacities projected independently;
- orientation weights remain direction-specific;
- node state unchanged;
- bridge status unchanged for NORMAL/DAMPING/RECOVERY;
- ISOLATED => ProjectionUnavailable;
- exact request stateVersion required;
- exact relationship identity required;
- both endpoint IdentityInvariant checks required;
- effective coupling retained as consistency diagnostic;
- no runtime mutation;
- no eligibility/permission/authority/execution semantics.

B1.1 design is ready for implementation review.


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
