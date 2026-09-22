# SOAM 2.0 D8D — Restricted-Origin Result API Design

## Status

- Layer: D8D
- Runtime implementation: none
- Acceptance status: final-review candidate

---

## 1. Public identity types

Candidate types:

- `InterpretationPolicyId`;
- `InterpretationPolicySnapshotId`;
- `InterpretationDecisionId`.

All are immutable opaque value types.

`InterpretationDecisionId`:

- 128-bit;
- OS-backed random generation;
- all-zero invalid;
- one completed interpretation event -> one ID;
- not caller-selectable.

---

## 2. Policy snapshot

Candidate:

`InterpretationPolicySnapshot`

Public read-only contents:

- InterpretationPolicySnapshotId;
- InterpretationPolicyDescriptor;
- field-usage manifest;
- distance attenuation coefficient;
- declared directionality.

Ordinary callers cannot construct arbitrary production snapshots.

Production v1 policy comes from a trusted provider analogous to D8C:

`ProductionInterpretationPolicyProvider::createCurrent()`

Negative/mutated policy variants belong only to internal test seams.

---

## 3. Success type

Candidate:

`VersionedProductionInterpretation`

Required immutable contents:

```text
VersionedProductionInterpretation
├── InterpretationDecisionId
├── InterpretationPolicySnapshotId
├── InterpretationPolicyDescriptor
├── SourceCaptureId
├── ProvenanceItemId
├── AdmissibilityDecisionId
├── D8C PolicySnapshotId
├── relationshipGeneration
├── stateVersion
├── InteractionObservation
├── BridgeConfidence
└── InterpretationTrace
```

The success object owns or safely retains the accepted
`AdmissibleProductionProvenance` lineage.

---

## 4. InterpretationTrace v1

Candidate trace:

```text
InterpretationTrace
- distance
- orientationWeight
- capacity
- sourceHealth
- targetHealth
- attenuation
- compatibility
- confidence
```

All are immutable.

The trace is sufficient to independently reproduce the v1 semantic computation.

BridgeStatus and raw state are not included as consumed inputs because v1 does
not use them.

---

## 5. Rejection type

Candidate:

`ProductionInterpretationRejection`

Required contents:

- InterpretationDecisionId;
- InterpretationPolicySnapshotId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- primary reason;
- reason flags.

Candidate closed v1 reasons:

- PolicyUnavailable;
- PolicyRevisionUnrecognized;
- UpstreamLineageInconsistent;
- DistanceInvalid;
- OrientationWeightInvalid;
- CapacityInvalid;
- SourceHealthInvalid;
- TargetHealthInvalid;
- CompatibilityComputationInvalid;
- ConfidenceComputationInvalid;
- OutputInvariantViolation;
- InternalDeterministicEvaluationFailure.

A D8D rejection does not invalidate the upstream D8C admissibility decision.

---

## 6. Result carrier

Preferred public shape:

```cpp
using ProductionInterpretationResult =
    std::variant<
        VersionedProductionInterpretation,
        ProductionInterpretationRejection>;
```

Candidate evaluator:

```cpp
class VersionedProductionInterpreter final {
public:
    [[nodiscard]]
    std::optional<ProductionInterpretationResult> interpret(
        const AdmissibleProductionProvenance& provenance,
        const InterpretationPolicySnapshot& policy) const;
};
```

Outer `nullopt` means infrastructure failure before a completed interpretation
decision exists, for example decision-ID generation failure.

Completed policy rejection is always the rejection variant.

---

## 7. Restricted-origin requirements

Ordinary public callers cannot:

- default-construct InterpretationDecisionId;
- construct it from arbitrary bytes;
- default-construct VersionedProductionInterpretation;
- construct trusted interpretation from raw D8B envelope;
- construct trusted interpretation from arbitrary InteractionObservation;
- construct trusted interpretation from arbitrary BridgeConfidence;
- replace upstream lineage;
- replace policy identity/snapshot identity;
- mutate semantic outputs after publication.

---

## 8. Evaluation ordering

Candidate fail-closed ordering:

```text
1. validate immutable policy snapshot
2. validate upstream D8C lineage presence
3. reserve InterpretationDecisionId
4. read declared v1 source fields only
5. validate distance/orientation/capacity/health domains
6. compute attenuation
7. compute compatibility
8. compute confidence
9. construct domain InteractionObservation / BridgeConfidence
10. validate trace/output invariants
11. publish one immutable success
12. STOP
```

Any failure after decision identity is reserved publishes one typed rejection if
the evaluator can complete the decision.

---

## 9. No hidden runtime reads

The interpreter MUST NOT query live mesh state.

All semantic input comes from the admitted provenance object.

Therefore:

```text
same admitted provenance + same policy snapshot
-> same semantic output
```

regardless of later live mesh mutation.

C2 remains the later live-current gate.

---

## 10. Compile-fail gates

Implementation acceptance requires rejection of at least:

1. default InterpretationDecisionId construction;
2. arbitrary-byte InterpretationDecisionId construction;
3. default VersionedProductionInterpretation construction;
4. raw D8B envelope -> trusted interpretation;
5. arbitrary InteractionObservation -> trusted interpretation;
6. arbitrary BridgeConfidence -> trusted interpretation;
7. mutation of interpretation lineage;
8. mutation of compatibility/confidence result;
9. public arbitrary production policy snapshot construction.

---

## 11. Runtime gates

Required tests include:

- fixed vectors;
- repeated event -> new InterpretationDecisionId;
- same input/policy -> identical semantic values;
- source->target reversal matches directional policy;
- BridgeStatus mutation alone cannot affect v1 output when all consumed fields
  are unchanged;
- sourceState/targetState mutation alone cannot affect v1 output when all
  consumed fields are unchanged;
- invalid consumed inputs reject;
- no live-runtime mutation/read dependency;
- all upstream lineage IDs preserved.
