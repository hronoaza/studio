# SOAM 2.0 D8D — InterpretationPolicyV1 Candidate

## Status

- Layer: D8D — Versioned Interpretation
- Artifact class: new versioned semantic policy candidate
- Runtime implementation: none
- Current Baseline effect: none
- Acceptance status: semantic review pending

This policy is a **new explicit interpretation choice** built from accepted
runtime quantities.

It is not presented as a previously existing hidden contract.

---

## 1. Policy identity

Candidate label:

`SOAM.D8D.INTERPRETATION.COUPLING_HEALTH.V1`

Candidate semantic version:

```text
major = 1
minor = 0
```

A fixed 128-bit policy ID and implementation-revision scheme must be assigned
before runtime implementation.

---

## 2. Consumed source fields

V1 consumes exactly:

- `distance`;
- `orientationWeight`;
- `capacity`;
- `sourceHealth`;
- `targetHealth`.

V1 does **not** consume:

- `BridgeStatus`;
- `sourceState`;
- `targetState`;
- source/target node IDs as semantic scores;
- relationship generation/state version as semantic scores.

Lineage fields remain preserved but are not numerical interpretation inputs.

---

## 3. Why these fields

### distance / orientationWeight / capacity

Accepted runtime already defines the composite:

```text
effectiveCoupling =
    capacity
    * (1 / (1 + 0.1 * distance))
    * orientationWeight
```

with runtime validation requiring:

```text
distance >= 0
0 <= orientationWeight <= 1
0 <= capacity <= 1
```

Therefore `effectiveCoupling` is a deterministic bounded quantity in [0,1].

D8D v1 explicitly chooses to interpret that accepted runtime coupling quantity
as compatibility.

This is a new policy decision, but it reuses an already-defined runtime
measurement rather than inventing a second geometric/capacity formula.

### sourceHealth / targetHealth

Accepted runtime constrains health to [0,1].

D8D v1 chooses the conservative endpoint rule:

```text
confidence = min(sourceHealth, targetHealth)
```

Rationale:

- confidence remains in [0,1];
- the weaker endpoint bounds evidentiary strength;
- the rule is symmetric under endpoint reversal;
- no extra weighting constant is introduced.

This confidence rule is a new D8D policy choice.

---

## 4. Compatibility function

Normative v1 function:

```text
compatibility =
    capacity
    * (1 / (1 + 0.1 * distance))
    * orientationWeight
```

Equivalent:

```text
compatibility = effectiveCoupling
```

under the accepted runtime formula.

Required input domain:

```text
distance finite and >= 0
orientationWeight finite and in [0,1]
capacity finite and in [0,1]
```

Required output:

```text
compatibility finite and in [0,1]
```

No clipping is permitted.

If the computed value is non-finite or outside [0,1], interpretation rejects.

---

## 5. Confidence function

Normative v1 function:

```text
confidence = min(sourceHealth, targetHealth)
```

Required input domain:

```text
sourceHealth finite and in [0,1]
targetHealth finite and in [0,1]
```

Required output:

```text
confidence finite and in [0,1]
```

No clipping is permitted.

---

## 6. Downstream evidence correspondence

The accepted `AdaptiveBridgePolicy` computes:

```text
BridgePolicyEvidence =
    ((2 * compatibility) - 1)
    * confidence
```

Under D8D v1 this becomes:

```text
BridgePolicyEvidence =
    (
      2
      * capacity
      * (1 / (1 + 0.1 * distance))
      * orientationWeight
      - 1
    )
    * min(sourceHealth, targetHealth)
```

D8D itself does not compute recommendation or transition direction.

This equation is documented only to demonstrate compatibility with the existing
downstream domain contract.

---

## 7. Neutral point

Because the downstream neutral compatibility point is 0.5:

```text
effectiveCoupling = 0.5
-> neutral signed observation
```

This is a consequence of the accepted downstream policy.

D8D v1 does not move the neutral point.

---

## 8. Directionality

V1 compatibility is **directional** because `orientationWeight` is
directional.

For one undirected relationship:

- distance is symmetric;
- capacity may currently be related but is stored per directed bridge;
- endpoint health minimum is symmetric;
- orientationWeight may differ by source/target direction.

Therefore:

```text
interpret(source -> target)
may differ from
interpret(target -> source)
```

This is intentional in v1.

Direction reversal tests are mandatory.

---

## 9. BridgeStatus exclusion

`BridgeStatus` is explicitly excluded from the v1 semantic function.

Reason:

- D7 prohibits status substitution for transition direction;
- the runtime status is correlated with capacity updates;
- including both status and capacity without an independent semantic contract
  risks double-counting the same runtime mechanism.

The status remains preserved in upstream provenance but is not consumed by D8D
v1.

---

## 10. State exclusion

`sourceState` and `targetState` are explicitly excluded from v1.

Accepted runtime state semantics are relative to each node's
`IdentityInvariant.baseline` and `maxEpsilon`.

D8A/D8B provenance does not currently carry those invariant parameters.

Therefore D8D cannot reproduce a baseline-relative state normalization from the
admissible provenance alone.

Using raw state magnitude would be semantically unjustified.

---

## 11. Health semantics caveat

Accepted runtime defines health from deviation from each node's baseline:

```text
health =
    max(
      0,
      1 - abs(state - baseline) / maxEpsilon
    )
```

D8D v1 consumes the captured health value, not the hidden baseline/maxEpsilon.

D8C already established bit-exact re-derivability of that captured health value
within the retained source-evidence scope.

D8D does not claim to independently re-derive health from state.

---

## 12. Field-usage manifest

Normative v1 manifest:

| Field | Used | Semantic role |
|---|---|---|
| distance | yes | attenuation term in compatibility |
| orientationWeight | yes | directional geometric term in compatibility |
| capacity | yes | runtime coupling-capacity term in compatibility |
| sourceHealth | yes | endpoint quality bound for confidence |
| targetHealth | yes | endpoint quality bound for confidence |
| BridgeStatus | no | retained context only |
| sourceState | no | no reproducible normalization available |
| targetState | no | no reproducible normalization available |
| relationshipGeneration | lineage only | no score |
| stateVersion | lineage only | no score |
| source/target IDs | lineage only | no score |

No unlisted field may influence v1 output.

---

## 13. Determinism

For fixed admissible provenance:

```text
compatibility_v1 = deterministic
confidence_v1 = deterministic
```

No clock, randomness, environment variable, global mutable threshold or
caller-provided weight participates.

Only `InterpretationDecisionId` changes between repeated interpretation events.

---

## 14. Failure conditions

V1 rejects if any consumed field is:

- non-finite;
- outside its accepted domain;
- unavailable.

It also rejects if:

- the D8C lineage carried by the input is structurally inconsistent;
- the interpretation policy revision is unavailable;
- result construction violates domain invariants.

No clamping/coercion converts invalid input into valid semantics.

---

## 15. Policy constants

V1 contains one inherited runtime constant:

```text
distance attenuation coefficient = 0.1
```

This value is not newly invented by D8D; it is taken from the accepted runtime
`SpatialBridge::getEffectiveCoupling()` definition.

Once InterpretationPolicyV1 is accepted, this coefficient becomes part of the
D8D policy revision as well.

Changing it requires a new interpretation policy revision.

---

## 16. Example vectors

### V1 — full coupling / full health

```text
distance = 0
orientationWeight = 1
capacity = 1
sourceHealth = 1
targetHealth = 1

compatibility = 1
confidence = 1
downstream BridgePolicyEvidence = +1
```

### V2 — zero orientation

```text
distance = 5
orientationWeight = 0
capacity = 1
sourceHealth = 1
targetHealth = 1

compatibility = 0
confidence = 1
downstream BridgePolicyEvidence = -1
```

This is semantic evidence, not a direct CONSTRAIN command.

### V3 — neutral effective coupling

One exact example:

```text
distance = 10
orientationWeight = 1
capacity = 1
attenuation = 1 / (1 + 1) = 0.5

compatibility = 0.5
confidence = 1
downstream BridgePolicyEvidence = 0
```

### V4 — confidence suppression

```text
distance = 0
orientationWeight = 1
capacity = 1
sourceHealth = 0
targetHealth = 1

compatibility = 1
confidence = 0
downstream BridgePolicyEvidence = 0
```

### V5 — directional reversal

For endpoints separated only on z-axis:

```text
source z=0 -> target z=1:
orientationWeight = 1

reverse:
orientationWeight = 0
```

with otherwise equal fields, v1 intentionally produces opposite extrema in
compatibility.

This test demonstrates declared directional semantics.

---

## 17. Interpretation trace

V1 trace must contain enough values to reproduce the result:

- input distance;
- input orientationWeight;
- input capacity;
- input sourceHealth;
- input targetHealth;
- attenuation term;
- compatibility;
- confidence;
- interpretation policy ID/version/revision.

It must also preserve all required upstream lineage identities.

---

## 18. What v1 claims

D8D v1 claims only:

> under the accepted versioned interpretation policy, the runtime's effective
> coupling is interpreted as compatibility, and the weaker captured endpoint
> health bounds confidence.

It does not claim that this is a universal physical truth or the only possible
interpretation.

---

## 19. What v1 does not claim

V1 does not claim:

- BridgeStatus is a command;
- health is probability;
- compatibility is authority;
- effective coupling is physical truth;
- D8D result is transition eligibility;
- D8D result is current-live after C2 revalidation;
- output is authorized for execution.

---

## 20. Acceptance gate

Before runtime implementation:

1. review whether `effectiveCoupling -> compatibility` is an acceptable new
   semantic policy choice;
2. review whether `min(endpointHealth) -> confidence` is an acceptable new
   confidence policy choice;
3. verify directionality is intended;
4. independently recompute fixed vectors;
5. assign fixed policy identity/revision;
6. define restricted-origin C++ result API;
7. perform final D8D design acceptance review.
