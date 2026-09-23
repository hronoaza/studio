# SOAM 2.0 — Resilience Evidence Design Contract

## Status

- Layer: C — resilience prerequisite evidence
- Document class: concrete design contract
- Design origin baseline: `31ebcccde5b4ff0cf456ba4edffa49829ba2e982`
- Accepted upstream:
  - Transition Request derivation
  - A1 Live Validity
  - A2 Freshness/Revalidation -> C1 adapter
  - B1 invariant evidence
- Downstream contract: C1 `ResiliencePrerequisiteEvidence`
- Acceptance status: accepted
- Executable implementation: accepted separately; see §55
- Ratification scope: bounded to the resilience prerequisite channel evidence semantics described in this document. This ratification does not accept or define the deferred C->C1 adapter contract; that is a separate design workstream.
- Authority effect: none

### Historical terminology

Earlier uses of "candidate" describe the state at that review stage.
Those stages are followed by §§51–55, which record the final accepted semantics,
implementation-ready disposition, and accepted implementation provenance.

Ratification does not rewrite those historical review stages.

## 1. Boundary

C defines only the resilience prerequisite channel:

```text
ProductionDerivedTransitionRequest
+ trusted/versioned resilience policy
+ coherent trusted topology snapshot
-> ProductionResiliencePrerequisiteRecord
   OR typed resilience rejection
-> STOP
```

C does not produce:

- PermissionPrerequisiteEvidence;
- InvariantPrerequisiteEvidence;
- FreshnessPrerequisiteEvidence;
- RevalidationPrerequisiteEvidence;
- ProductionTransitionPrerequisiteSet;
- ProductionTransitionEligibilityDecision;
- authority/capability/execution output.

## 2. Principal separation

```text
request
!= live validity
!= invariant safety
!= resilience
!= permission
!= eligibility
!= authority
!= execution
```

Resilience must not be inferred from:

- D9 recommendation;
- request direction;
- invariant=true;
- freshness=true;
- revalidation=true;
- current GitHub permissions;
- CI success;
- operator identity;
- current node health alone.

## 3. Existing runtime facts are not resilience evidence

The accepted runtime contains:

- node `healthIndex`;
- directed bridge capacity;
- directed bridge status;
- symmetric topology;
- relationship generations;
- topology mutation and pruning;
- simulation stability controls.

These are observable facts or simulation mechanisms.

None is currently an accepted production definition of transition resilience.

In particular:

```text
healthIndex > X
```

is not accepted because no trusted resilience threshold `X` exists.

Likewise:

```text
degree >= N
capacity >= C
effectiveCoupling >= E
```

are not accepted without separate versioned policy semantics.

## 4. V1 resilience property

Candidate C V1 defines resilience as:

```text
single-request-relationship-loss survivability
```

For the exact request-bound logical relationship between source and target:

1. capture one coherent topology snapshot;
2. verify the request epoch and exact relationship identity;
3. exclude the request-bound relationship pair from the projected topology;
4. determine whether source and target remain connected through at least one
   other operational relationship path.

If such a path exists:

```text
resilience satisfied = true
```

If no such path exists:

```text
resilience satisfied = false
```

This is graph-theoretic redundancy, not a prediction of future simulation
performance.

## 5. Why this V1 is deliberately structural

The repository currently has no accepted:

- health threshold;
- minimum spare capacity threshold;
- minimum path bandwidth;
- maximum path length;
- weighted redundancy score;
- failure probability model;
- recovery-time objective.

Therefore C V1 avoids inventing numeric resilience semantics.

It asks one narrow question:

> If the exact requested relationship pair disappears, does an alternative
> operational path still connect the same endpoints?

## 6. Direction independence

The C V1 resilience property is the same for:

- SUPPORT;
- CONSTRAIN.

Request direction remains part of the exact binding, but it does not alter the
V1 graph criterion.

This avoids silently inferring that SUPPORT is inherently safe or CONSTRAIN is
inherently dangerous.

A future resilience policy version may introduce direction-specific criteria
only through a separate design/acceptance gate.

## 7. Logical relationship model

The accepted runtime represents one logical relationship as two reciprocal
directed edges.

For each logical relationship `A <-> B`, V1 requires an operational pair:

- A -> B exists;
- B -> A exists;
- both carry the same non-zero relationship generation;
- neither is `BridgeStatus::ISOLATED`;
- both capacities are finite and greater than zero;
- both distances are finite and non-negative;
- both orientation weights are finite and in `[0,1]`.

A malformed pair is not silently treated as valid redundancy.

## 8. Request-bound relationship handling

The exact request-bound pair is identified by:

```text
sourceNodeId
targetNodeId
relationshipGeneration
```

Before redundancy search begins, C must verify:

- source != target;
- both endpoints exist;
- forward request edge exists;
- reverse request edge exists;
- both request edges carry the exact request-bound generation.

Failure to establish the exact relationship identity is a typed rejection.

The request-bound pair is then excluded from the candidate graph for the V1
survivability test.

## 9. Alternative path definition

An alternative path is a sequence of logical operational relationships:

```text
source = v0, v1, ..., vn = target
```

such that:

- `n >= 2`;
- no logical edge in the path is the excluded request-bound pair;
- each consecutive node pair has one valid reciprocal operational
  relationship;
- no self-edge is used;
- no duplicate directed-edge ambiguity exists in the trusted snapshot.

V1 does not require multiple disjoint alternative paths.

One valid alternative path is sufficient.

## 10. Path algorithm

Preferred V1 algorithm:

```text
breadth-first search over logical undirected operational relationships
```

with the exact request-bound logical pair removed.

The result is deterministic for the same immutable topology snapshot.

Traversal order must not affect the final boolean.

No randomness or wall clock is permitted.

## 11. No path-quality threshold in V1

V1 deliberately does not score:

- hop count;
- minimum path capacity;
- aggregate effective coupling;
- latency;
- geometry;
- node health;
- path diversity beyond one alternative path.

These may be exposed as immutable diagnostics if useful, but they do not
participate in the V1 boolean.

This keeps V1 free of unaccepted numeric thresholds.

## 12. Coherent snapshot requirement

C must consume one coherent trusted topology snapshot captured under one
topology-consistency epoch.

The snapshot must be sufficient to reconstruct the logical operational graph
without later unlocked public reads.

At minimum it must contain:

- stateVersion;
- all node identities;
- all directed relationships;
- each directed edge target;
- relationship generation;
- capacity;
- status;
- distance;
- orientationWeight.

The final representation may be normalized to logical relationship pairs during
capture, provided normalization is deterministic and restricted-origin.

## 13. Snapshot origin and lifecycle

The topology snapshot source must be:

- runtime-owned;
- read-only;
- restricted construction;
- lifecycle-safe;
- invalidated when the mesh owner is destroyed.

A caller must not fabricate topology or edge state.

A stale source returns snapshot-unavailable/fail-closed behavior.

## 14. Epoch precondition

C requires:

```text
snapshot.stateVersion == request.stateVersion
```

A mismatch is a typed rejection:

```text
RequestStateVersionMismatch
```

This does not mint or replace A1 Freshness evidence.

It is only a precondition for evaluating resilience against the exact request
epoch.

## 15. Relationship precondition

The request-bound relationship must still exist with the exact generation.

A missing or regenerated relationship is a typed rejection:

```text
RelationshipIdentityMismatch
```

This does not mint or replace A1 Revalidation evidence.

## 16. V1 policy identity

C requires a trusted immutable resilience policy snapshot.

Candidate family:

```text
SingleRelationshipLossResiliencePolicyV1
major = 1
minor = 0
criterion = alternative operational path exists
```

The production policy snapshot must include:

- opaque policy snapshot ID;
- semantic policy ID;
- major/minor version;
- implementation revision kind;
- reproducible implementation digest;
- fixed V1 criterion identifier.

No caller-selected thresholds or graph criteria are permitted.

## 17. Decision identity

Every completed resilience decision event should have one non-zero opaque
128-bit `ResilienceDecisionId`.

Properties:

- no public default construction;
- no raw-byte public construction;
- production-origin only;
- copyable after creation;
- OS-backed generation consistent with accepted decision-ID families.

If identity generation fails before a decision event exists, evaluation returns
`std::nullopt`.

## 18. Candidate evidence record

C should introduce a restricted-origin record distinct from C1 evidence:

```cpp
class ResilienceDecisionId final;

class ProductionResiliencePrerequisiteRecord final {
public:
    const ResilienceDecisionId& decisionId() const noexcept;
    const ProductionTransitionRequestBinding& binding() const noexcept;
    const ResiliencePolicySnapshotId& policySnapshotId() const noexcept;
    bool satisfied() const noexcept;

    // optional immutable diagnostics
    std::size_t alternativePathHopCount() const noexcept;

private:
    // evaluator-only construction
};
```

When `satisfied=false`, `alternativePathHopCount` should be zero.

## 19. Rejection vs unsatisfied evidence

A legitimate lack of redundancy is not a rejection.

Example:

```text
source -- target
```

with no other route:

```text
valid trusted evaluation
=> satisfied=false
```

Rejection is reserved for inability to establish a trusted decision.

Examples:

- snapshot source unavailable;
- request lineage inconsistent;
- request epoch mismatch;
- request relationship generation mismatch;
- malformed topology snapshot;
- unsupported policy revision;
- internal decision failure.

## 20. Candidate rejection reasons

```cpp
enum class TransitionResilienceReason : std::uint8_t {
    RequestLineageInconsistent,
    TransitionClassUnsupported,
    PolicyRevisionUnrecognized,
    SnapshotUnavailable,
    RequestStateVersionMismatch,
    RelationshipIdentityMismatch,
    TopologySnapshotInvalid,
    InternalDecisionFailure
};
```

Exact reason precedence remains a critical-review item.

## 21. Transition class scope

C V1 applies only to the already accepted:

```text
BridgeCouplingAdjustmentV1
```

Unrecognized transition classes fail closed.

C may use a narrow trusted class-identity seam analogous to B1.

That seam must not permit construction of request bindings, prerequisite
evidence, eligibility decisions, authority, capability, or execution objects.

## 22. No health threshold

Node `healthIndex` is intentionally excluded from the V1 resilience boolean.

Reason:

- health already derives from node state/invariant state;
- no accepted resilience threshold exists;
- adding a threshold here would introduce a second policy decision unrelated to
  the graph redundancy property.

Health may be considered in a future V2 after an explicit policy design.

## 23. No capacity threshold beyond operationality

V1 uses only:

```text
capacity > 0
```

to distinguish a non-isolated usable edge from zero-capacity connectivity.

There is no minimum spare-capacity threshold.

This is a structural reachability criterion, not a bandwidth guarantee.

If review concludes that `capacity > 0` is too weak to represent
operationality, implementation must remain blocked until an alternative rule is
accepted.

## 24. Status semantics

`BridgeStatus::ISOLATED` is never operational in C V1.

Candidate operational statuses:

- NORMAL;
- DAMPING;
- RECOVERY.

C V1 does not rank these statuses.

It does not infer resilience strength from them.

## 25. No invariant duplication

C must not re-evaluate B1 projection or endpoint IdentityInvariant semantics.

It may validate edge fields only enough to determine whether the topology
snapshot is trustworthy and whether a logical edge is operational.

Therefore:

```text
invariant=true
!= resilience=true
```

A transition may satisfy B1 while failing C due to lack of an alternate path.

## 26. No mutation-as-test

Rejected:

```text
remove relationship from live mesh
-> test connectivity
-> restore relationship
```

Accepted:

```text
capture immutable topology snapshot
-> exclude request-bound pair in memory
-> BFS
-> emit record
-> STOP
```

No live topology mutation is permitted.

## 27. No permission inference

Resilience success does not create or imply permission.

No repository access, account identity, operator role, or technical capability
may be promoted into resilience evidence.

## 28. No eligibility call

C must not instantiate or invoke:

```text
ProductionTransitionEligibilityEvaluator
```

At C completion the system will still lack Permission evidence and a complete
production prerequisite assembly path.

## 29. Dependency placement

Preferred split:

Runtime-owned:

```text
ProductionTransitionResilienceSnapshotSource
```

Downstream:

```text
apps/soam-transition-resilience/
```

Candidate target:

```text
soam_transition_resilience
AdaptiveMesh::soam_transition_resilience
```

The downstream layer may depend one-way on:

- AdaptiveMesh::soam_runtime;
- AdaptiveMesh::soam_transition_request.

Runtime must not depend back on the resilience library.

## 30. Runtime source surface

The resilience snapshot source must expose only topology facts required by C V1.

It must not become a generic unrestricted mesh-introspection API.

No mutable mesh handle may escape.

## 31. Restricted C1 conversion seam

C itself stops at `ProductionResiliencePrerequisiteRecord`.

A later narrow adapter may convert an accepted record into:

```text
ResiliencePrerequisiteEvidence
```

That adapter may receive friendship only for
`ResiliencePrerequisiteEvidence`.

It must not gain access to:

- PermissionPrerequisiteEvidence;
- InvariantPrerequisiteEvidence;
- FreshnessPrerequisiteEvidence;
- RevalidationPrerequisiteEvidence;
- ProductionTransitionRequestBinding constructors;
- eligibility decisions;
- authority;
- capability;
- execution.

## 32. Compile-fail requirements

Before C implementation acceptance, negative tests must prove arbitrary callers
cannot:

1. default/raw-byte construct ResilienceDecisionId;
2. construct trusted resilience policy snapshots;
3. fabricate resilience topology snapshots;
4. directly construct ProductionResiliencePrerequisiteRecord;
5. directly construct C1 ResiliencePrerequisiteEvidence;
6. construct request bindings through C;
7. use C access to construct Permission/Invariant/Freshness/Revalidation
   evidence;
8. invoke C1 eligibility through the resilience evaluator;
9. mutate mesh topology through the resilience snapshot API;
10. obtain authority/capability/execution objects.

## 33. Positive verification requirements

Implementation acceptance must prove:

- exact request binding is preserved;
- exact request stateVersion is required;
- exact request relationship generation is required;
- request-bound pair is excluded from the graph;
- one valid alternate path => satisfied=true;
- no alternate path => valid satisfied=false record;
- paths do not traverse ISOLATED relationships;
- malformed reciprocal pairs fail closed;
- stale snapshot source fails closed;
- traversal is deterministic;
- no mesh mutation occurs;
- no node-health threshold is consulted;
- no eligibility/permission/authority/execution output exists.

## 34. Security property

Even when:

```text
freshness=true
&& revalidation=true
&& invariant=true
&& resilience=true
```

the system still lacks:

- permission evidence;
- complete prerequisite assembly;
- eligibility decision;
- authority;
- capability;
- execution.

## 35. Critical design questions

Before C implementation begins, review must decide:

1. Is single-request-relationship-loss survivability the accepted V1 resilience
   property?
2. Is one alternate operational path sufficient?
3. Is `capacity > 0` sufficient for V1 operationality?
4. Are NORMAL/DAMPING/RECOVERY all operational for V1?
5. Must reciprocal edges have exact shared generation?
6. Must reciprocal edges have exact shared distance, as in B1?
7. Is hop count diagnostic-only?
8. Is node health correctly excluded from V1?
9. Is malformed unrelated topology a global rejection or may search ignore
   unreachable malformed components?
10. Exact rejection precedence.
11. Exact normalized snapshot representation.

## 36. Initial design disposition

The strongest non-arbitrary resilience property currently grounded by the
runtime is structural redundancy under removal of the exact request-bound
logical relationship.

This avoids inventing health, capacity, bandwidth, or probability thresholds.

However the candidate remains design-only until the questions in section 35
are critically reviewed.

## 37. STOP

No production code change is authorized by this document.

No C1 ResiliencePrerequisiteEvidence is created.
No full prerequisite set is assembled.
No eligibility decision is produced.
No permission, authority, capability, or execution boundary is crossed.


## 38. Critical review — accepted candidate property shape

Review of the accepted runtime topology model supports the candidate V1
property:

```text
single-request-relationship-loss survivability
```

as a coherent resilience concept distinct from B1 invariant safety.

The property asks whether the exact endpoints remain connected after the exact
request-bound logical relationship is removed from an immutable graph
projection.

This does not claim:
- bandwidth sufficiency;
- latency sufficiency;
- probabilistic fault tolerance;
- node health sufficiency;
- multi-failure survivability.

It is one explicit structural resilience guarantee.

## 39. Critical review — one alternate path

For V1, one alternate operational path is sufficient.

Therefore:

```text
alternative path exists => satisfied=true
no alternative path     => satisfied=false
```

V1 does not require:
- two edge-disjoint alternate paths;
- vertex-disjoint paths;
- a minimum redundancy count.

Those are possible future policy versions.

## 40. Critical review — operational logical relationship

The accepted runtime makes `ISOLATED` the only explicit non-operational bridge
status and sets capacity to zero when entering that status.

For V1, a logical relationship is operational only when both reciprocal
directed edges satisfy all of:

```text
status in {NORMAL, DAMPING, RECOVERY}
capacity > 0
capacity <= 1
capacity finite
distance finite && distance >= 0
orientationWeight finite && orientationWeight in [0,1]
same non-zero relationship generation
same exact distance
reciprocal targets are exact
```

The `capacity > 0` test is not a quality threshold. It distinguishes a
zero-capacity relationship from one that still participates in structural
reachability.

No minimum spare capacity is implied.

## 41. Critical review — exact reciprocal generation and distance

The accepted runtime creates both directed members of a logical relationship
from one relationship-generation value and one computed distance.

Simulation may change capacity and status independently per direction, but it
does not rewrite generation or distance.

Therefore C V1 requires:

```text
forward.generation == reverse.generation
forward.distance == reverse.distance
```

and exact reciprocal endpoints.

This matches the runtime's existing topology contract rather than introducing a
new resilience policy.

## 42. Critical review — health excluded

Node `healthIndex` remains excluded from V1.

There is no accepted resilience threshold, and importing a threshold from other
runtime logic would create a hidden second policy.

Endpoint health may be carried only if later diagnostics require it, but it
must not affect the V1 resilience boolean.

Preferred V1 snapshot therefore does not need health values at all.

## 43. Critical review — hop count

The shortest alternative-path hop count may be exposed as a diagnostic.

It does not participate in satisfaction.

BFS naturally yields the minimum hop count in the normalized unweighted graph,
but V1 does not define a maximum acceptable hop count.

## 44. Critical review — malformed topology scope

C V1 fails closed if any logical relationship included in the coherent
resilience snapshot is malformed.

It must not silently ignore malformed relationships merely because they are not
selected by the eventual BFS path.

Rationale:

- the snapshot is a trusted representation of one global topology epoch;
- the accepted runtime treats symmetry as a global topology invariant;
- selective tolerance would make the result depend on traversal/search details;
- fail-closed global validation gives deterministic trust semantics.

Therefore:

```text
any malformed pair in snapshot
=> TopologySnapshotInvalid rejection
```

before the alternate-path result is accepted.

## 45. Critical review — request-bound pair

The request-bound pair must itself be a structurally valid reciprocal pair with
the exact request generation before it is excluded from the graph.

If either directed member is missing or has a different generation:

```text
RelationshipIdentityMismatch
```

If the pair exists with exact identity but its fields are malformed:

```text
TopologySnapshotInvalid
```

If the exact pair is structurally valid but currently `ISOLATED` or
zero-capacity, V1 cannot treat removal of that pair as a meaningful additional
failure scenario.

Candidate disposition:

```text
request-bound pair non-operational
=> TopologySnapshotInvalid
```

rather than `satisfied=false`.

This remains a semantic classification choice to be accepted with the V1
policy.

## 46. Critical review — deterministic normalized snapshot

Preferred runtime representation is a normalized immutable list of logical
relationship pairs rather than exposing two arbitrary directed-edge vectors to
the downstream evaluator.

Each normalized item should contain:

```text
nodeA
nodeB
generation
distance
A->B capacity/status/orientationWeight
B->A capacity/status/orientationWeight
```

with canonical endpoint ordering for storage.

The runtime snapshot source performs only structural capture/normalization.

The downstream resilience layer applies policy operationality and BFS.

This keeps runtime generic enough for C while avoiding a generic mutable mesh
surface.

## 47. Critical review — rejection precedence

Candidate V1 precedence after decision-ID establishment:

1. RequestLineageInconsistent
2. TransitionClassUnsupported
3. PolicyRevisionUnrecognized
4. SnapshotUnavailable
5. RequestStateVersionMismatch
6. RelationshipIdentityMismatch
7. TopologySnapshotInvalid
8. InternalDecisionFailure

A valid topology with no alternate path is not a rejection; it emits
`satisfied=false`.

## 48. Critical review — C1 seam remains deferred

C runtime implementation must still stop at
`ProductionResiliencePrerequisiteRecord`.

The C1 conversion seam for `ResiliencePrerequisiteEvidence` is a separate
adapter/gate after the record semantics are accepted.

This mirrors the separation used for A1/A2 and prevents the evaluator from
acquiring eligibility-construction authority.

## 49. Critical review disposition

Closed candidate questions:

- V1 property: single-request-relationship-loss survivability;
- one alternate path is sufficient;
- operationality: reciprocal non-ISOLATED pair with finite valid fields and
  capacity > 0 in both directions;
- NORMAL/DAMPING/RECOVERY are operational;
- reciprocal generation equality required;
- reciprocal exact distance equality required;
- hop count diagnostic-only;
- node health excluded;
- malformed snapshot rejected globally;
- normalized logical-pair snapshot preferred;
- rejection precedence fixed as above.

One normative classification remains explicit:

```text
exact request-bound pair exists but is non-operational
=> TopologySnapshotInvalid
```

If accepted, C V1 is implementation-ready.

## 50. Review STOP

No executable code change is authorized by this review.

No ResiliencePrerequisiteEvidence is constructed.
No prerequisite set is assembled.
No eligibility decision is produced.
No permission, authority, capability, or execution surface is opened.


## 51. Accepted request-bound non-operational classification

The remaining C V1 classification decision is accepted:

```text
exact request-bound pair exists
&& exact request generation matches
&& request-bound pair is non-operational
=> TopologySnapshotInvalid
```

This condition does not emit `satisfied=false`.

Reason: C V1 evaluates survivability under a hypothetical removal of the exact
request-bound operational relationship. If that relationship is already
non-operational at the request epoch, the V1 failure scenario is no longer a
well-formed counterfactual input.

The evaluator therefore fails closed rather than reinterpreting an already
degraded topology as a valid resilience trial.

## 52. Final accepted C V1 semantics

The reviewed C V1 contract is now:

- resilience property:
  single-request-relationship-loss survivability;
- one alternate operational path is sufficient;
- request-bound pair is removed only in the immutable graph projection;
- BFS over normalized logical relationships;
- operational relationship requires reciprocal exact endpoints, shared non-zero
  generation, exact shared distance, finite valid fields, non-ISOLATED status,
  and positive capacity in both directions;
- NORMAL, DAMPING, and RECOVERY are operational;
- node health is excluded;
- hop count is diagnostic-only;
- malformed topology anywhere in the coherent snapshot fails closed;
- non-operational request-bound pair => TopologySnapshotInvalid;
- no live mesh mutation;
- no health/capacity quality threshold beyond structural operationality;
- no C1 conversion;
- no eligibility;
- no permission;
- no authority/capability/execution.

## 53. C implementation authorization boundary

With the final classification accepted, C has no remaining semantic blocker for
a candidate runtime implementation of:

- lifecycle-safe coherent normalized topology snapshot source;
- trusted/versioned SingleRelationshipLossResiliencePolicyV1;
- deterministic topology validation;
- request-bound pair exclusion;
- BFS alternate-path evaluation;
- restricted-origin ResilienceDecisionId;
- ProductionResiliencePrerequisiteRecord;
- typed resilience rejection;
- positive and compile-fail tests;
- dedicated sanitizer CI.

This acceptance does not authorize:

- C1 ResiliencePrerequisiteEvidence conversion;
- Permission evidence;
- full prerequisite-set assembly;
- C1 eligibility integration;
- authority/capability/execution.

## 54. Final review status

C V1 design is implementation-ready.

No executable code is introduced by this document.


## 55. Accepted C implementation provenance

The executable C resilience implementation was reviewed and accepted separately.

Acceptance evidence:

- design PR: #30;
- runtime implementation PR: #31;
- accepted implementation head: `14faf28b99cf274cfd9412228533bc57996c95eb`;
- accepted runtime merge baseline: `aa9b5bf77bf060066c0f0dd5051a1cbfcf47f347`;
- exact-head validation: 14/14 workflows completed successfully, including
  Transition Resilience, Transition Invariant, Runtime, D7, D8A/B/C/D, D9,
  Transition Request, Transition Live Validity, Live Validity C1 Adapter,
  Transition Eligibility, and Live Evaluator.

Accepted runtime boundary:

```text
ProductionDerivedTransitionRequest
+ SingleRelationshipLossResiliencePolicyV1
+ lifecycle-safe coherent normalized topology snapshot
-> ProductionResiliencePrerequisiteRecord
   OR typed resilience rejection
-> STOP
```

The accepted implementation preserves the reviewed C V1 semantics:

- exact request epoch and relationship generation required;
- global topology validation is fail-closed;
- request-bound operational pair is excluded only from the immutable graph projection;
- deterministic BFS decides whether one alternate operational path exists;
- no alternate path emits a valid record with `satisfied=false`;
- stale source emits a typed rejection;
- node health and arbitrary quality thresholds are not consulted;
- no live mesh mutation occurs.

The accepted implementation does not construct C1
`ResiliencePrerequisiteEvidence`, does not assemble the prerequisite set,
does not invoke eligibility, and does not create permission, authority,
capability, or execution artifacts.

This document now serves as durable design/audit provenance for the accepted C
runtime layer.
