# SOAM 2.0 D8D — Design Review 1

## Status

- Reviewed artifact: `DESIGN_CONTRACT.md`
- Review class: semantic/architecture review
- Runtime changes: none
- Disposition: **REVISE BEFORE DESIGN ACCEPTANCE**

The structural D8D boundary is coherent.

The accepted repository sources define the semantics and invariants of
`InteractionObservation`, `BridgeConfidence`, `AdaptiveBridgePolicy` and
`BridgePersistence`, but do **not** define a source-facts-to-interpretation
mapping.

Therefore D8D v1 implementation remains blocked on one semantic contract.

---

## 1. Accepted output semantics

### InteractionObservation

Accepted domain contract:

```text
compatibility is finite
0.0 <= compatibility <= 1.0
```

Downstream policy semantics establish:

```text
compatibility = 0.0 -> full negative signed observation
compatibility = 0.5 -> neutral signed observation
compatibility = 1.0 -> full positive signed observation
```

because `AdaptiveBridgePolicy` computes:

```text
signedObservation = (2 * compatibility) - 1
```

Thus the accepted semantic interpretation is:

- 0.0 = maximally incompatible / negative evidence endpoint;
- 0.5 = neutral evidence;
- 1.0 = maximally compatible / positive evidence endpoint.

D8D MUST preserve that meaning.

---

## 2. Accepted BridgeConfidence semantics

Accepted domain contract:

```text
confidence is finite
0.0 <= confidence <= 1.0
```

Downstream policy multiplies signed compatibility by confidence:

```text
BridgePolicyEvidence =
    ((2 * compatibility) - 1) * confidence
```

Therefore:

- confidence 0.0 suppresses all downstream evidence;
- increasing confidence increases evidence magnitude for a fixed non-neutral
  compatibility;
- confidence does not determine evidence direction;
- confidence is not provenance integrity;
- confidence is not authentication confidence;
- confidence is not authority confidence.

D8D MUST preserve that meaning.

---

## 3. Accepted downstream symmetry

Domain tests establish complementary compatibility symmetry:

```text
compatibility 0.75 at confidence c
and compatibility 0.25 at confidence c
produce equal-and-opposite BridgePolicyEvidence
```

D8D must not redefine `InteractionObservation` so that its numerical meaning
breaks this accepted downstream symmetry.

---

## 4. Persistence expectations

`BridgePersistence` treats downstream evidence as signed and bounded.

Positive evidence can lead toward SUPPORT.

Negative evidence can lead toward CONSTRAIN.

Near-zero evidence supports PRESERVE/release behavior according to persistence
thresholds.

D8D therefore produces semantic evidence inputs, not recommendations.

Required distinction:

```text
D8D compatibility/confidence
-> AdaptiveBridgePolicy evidence
-> BridgePersistence recommendation
```

D8D must not skip the latter two stages.

---

## 5. Missing source-to-semantic function

The accepted sources do not define any normative function of:

- distance;
- orientationWeight;
- capacity;
- sourceState;
- targetState;
- sourceHealth;
- targetHealth;
- BridgeStatus

that yields:

- compatibility;
- confidence.

No accepted source assigns weights to these fields.

No accepted source defines normalization ranges for them.

No accepted source says which fields are mandatory semantic inputs versus merely
captured provenance facts.

Therefore any formula such as:

```text
compatibility = weighted_average(...)
confidence = min(sourceHealth, targetHealth)
```

would be an invented policy, not a recovered accepted contract.

---

## 6. BridgeStatus remains non-commanding

The D7 negative boundary remains controlling.

`BridgeStatus` may not be interpreted as a direct command or recommendation.

In particular the review rejects any D8D policy equivalent to:

```text
NORMAL   -> SUPPORT
DAMPING  -> CONSTRAIN
RECOVERY -> SUPPORT
ISOLATED -> CONSTRAIN/disconnect
```

without a separately accepted semantic justification.

---

## 7. What can be accepted now

The following D8D semantic constraints are supported by current accepted code:

1. compatibility range is [0,1];
2. 0.5 is neutral;
3. confidence range is [0,1];
4. confidence scales evidence magnitude only;
5. zero confidence suppresses evidence;
6. downstream evidence range remains [-1,1];
7. complementary compatibility values are symmetric downstream;
8. D8D does not directly emit recommendation/eligibility/authority;
9. non-finite semantic outputs are invalid;
10. exact source-to-semantic mapping remains undefined.

---

## 8. Blocking question

Before D8D design acceptance, one explicit domain contract must answer:

> Which admissible source fields define compatibility and confidence, and by
> what deterministic versioned normalization/function?

Until that is fixed:

```text
D8D structural design: coherent
D8D output semantics: constrained
D8D source mapping: undefined
D8D implementation: blocked
```

---

## 9. Review disposition

No structural defect requires redesign of D8D.

The blocker is semantic provenance: the repository does not yet contain enough
accepted evidence to derive the interpretation function.

Disposition:

```text
layer separation: PASS
output semantic contract: PASS
downstream compatibility: PASS
lineage preservation: PASS
source-to-semantic mapping: BLOCKED
overall design acceptance: NOT YET READY
```
