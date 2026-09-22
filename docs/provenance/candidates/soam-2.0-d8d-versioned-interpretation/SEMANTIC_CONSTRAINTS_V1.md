# SOAM 2.0 D8D — Semantic Constraints v1

## Status

- Artifact class: accepted-source-derived semantic constraints candidate
- Runtime implementation: none
- Source-to-semantic mapping: intentionally unspecified
- Acceptance status: review pending

This document records only semantic properties supported by the accepted domain
types and tests.

It does not invent the D8D interpretation formula.

---

## 1. InteractionObservation semantic domain

`InteractionObservation` carries one finite value:

`compatibility`

Domain:

```text
0.0 <= compatibility <= 1.0
```

Normative interpretation inherited from `AdaptiveBridgePolicy`:

```text
signed compatibility = 2 * compatibility - 1
```

Therefore:

| compatibility | signed semantic meaning |
|---:|---:|
| 0.0 | -1.0 |
| 0.25 | -0.5 |
| 0.5 | 0.0 |
| 0.75 | +0.5 |
| 1.0 | +1.0 |

D8D MUST NOT shift the neutral point away from 0.5 without changing the
downstream domain policy contract.

---

## 2. BridgeConfidence semantic domain

`BridgeConfidence` carries one finite value:

`confidence`

Domain:

```text
0.0 <= confidence <= 1.0
```

Its accepted downstream role is multiplicative magnitude scaling:

```text
evidence =
    signed_compatibility * confidence
```

Therefore:

- 0.0 = no downstream evidentiary force;
- 1.0 = full evidentiary force of the interpreted compatibility;
- intermediate values scale magnitude linearly;
- confidence does not select sign/direction.

---

## 3. Combined downstream invariant

Given valid D8D outputs:

```text
compatibility in [0,1]
confidence in [0,1]
```

the existing `AdaptiveBridgePolicy` guarantees:

```text
BridgePolicyEvidence in [-1,1]
```

and:

```text
compatibility = 0.5
-> evidence = 0
for every valid confidence
```

and:

```text
confidence = 0
-> evidence = 0
for every valid compatibility
```

These are compatibility constraints on any future D8D policy.

---

## 4. Complement symmetry

For equal confidence:

```text
evidence(c, q) = -evidence(1-c, q)
```

where:

- c is compatibility;
- q is confidence.

Any future D8D normalization should be reviewed for whether its source-space
symmetry/asymmetry intentionally maps into this fixed semantic scale.

---

## 5. Non-finite values

The accepted types reject:

- NaN;
- +Infinity;
- -Infinity.

Therefore a D8D policy must reject unsupported/non-finite source situations
before attempting to construct semantic outputs.

It must not silently convert non-finite input into 0, 0.5 or 1.

---

## 6. Interpretation versus recommendation

D8D output semantics terminate at:

```text
InteractionObservation
+ BridgeConfidence
```

The next accepted domain operation is:

```text
AdaptiveBridgePolicy::evaluate(...)
-> BridgePolicyEvidence
```

and persistence subsequently maps repeated evidence into:

- PRESERVE;
- CONSTRAIN;
- SUPPORT.

Therefore D8D is prohibited from treating those recommendations as if they were
its own semantic labels.

---

## 7. Confidence naming constraint

Because `BridgeConfidence` scales policy evidence, a D8D implementation must
document what uncertainty or evidentiary quality the value represents.

It must not reuse confidence to mean:

- D8B canonical-integrity confidence;
- D8C admissibility confidence;
- probability that source facts are true;
- authorization confidence.

Those concepts are not supported by the accepted type contract.

---

## 8. Source fields currently available to D8D

Through accepted upstream lineage, D8D can access facts including:

- distance;
- orientationWeight;
- capacity;
- BridgeStatus;
- sourceState;
- targetState;
- sourceHealth;
- targetHealth;
- source/target node identity;
- relationship generation;
- state version.

Availability does not imply semantic relevance.

A future D8D policy must explicitly justify and declare each consumed field.

---

## 9. No normative source normalization exists yet

The accepted repository currently does not define normative semantic ranges or
normalization transforms for the captured source facts sufficient to produce
compatibility/confidence.

Consequently this document intentionally contains no formula such as:

```text
normalize(distance)
weighted average
min/max health confidence
status lookup table
```

until separately accepted domain semantics exist.

---

## 10. Gate for Interpretation Policy v1

A complete D8D v1 policy must add:

1. exact consumed field set;
2. semantic role of each field;
3. valid domain/range for each field;
4. normalization transform for each field;
5. aggregation function for compatibility;
6. derivation function for confidence;
7. direction-reversal behavior;
8. explicit handling of BridgeStatus;
9. deterministic test vectors;
10. rationale/provenance for those choices.

Until then there is no accepted `InterpretationPolicyV1`.
