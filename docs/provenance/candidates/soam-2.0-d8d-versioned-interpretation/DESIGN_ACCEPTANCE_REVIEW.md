# SOAM 2.0 D8D — Design Acceptance Review

## Review target

- PR: #17
- Base Current Baseline:
  `391536d444eeac8f7284f30f4e6241b1d944560c`
- Runtime changes: none
- Scope: D8D structure + InterpretationPolicyV1 semantics
- Disposition: **PASS — READY FOR EXPLICIT DESIGN ACCEPTANCE**

---

## 1. Upstream boundary

D8D accepts only `AdmissibleProductionProvenance`.

Raw D8B envelope and D8C rejection are excluded.

PASS.

---

## 2. Lineage preservation

The design preserves:

- SourceCaptureId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- D8C PolicySnapshotId;
- relationship generation;
- state version.

D8D adds rather than replaces interpretation-policy/decision identity.

PASS.

---

## 3. Output semantics

Accepted domain semantics are preserved:

- compatibility finite in [0,1];
- 0.5 neutral;
- confidence finite in [0,1];
- confidence scales magnitude, not sign;
- no direct recommendation/authority output.

PASS.

---

## 4. Compatibility policy

InterpretationPolicyV1 selects:

```text
compatibility = accepted runtime effectiveCoupling
```

using exactly the already-defined runtime function:

```text
capacity * (1 / (1 + 0.1 * distance)) * orientationWeight
```

The review explicitly classifies this equivalence as a **new D8D semantic policy
choice**, not an inherited runtime truth.

PASS.

---

## 5. Confidence policy

InterpretationPolicyV1 selects:

```text
confidence = min(sourceHealth, targetHealth)
```

This is explicitly a new conservative policy choice.

It introduces no new weight/threshold and is symmetric under endpoint health
reversal.

PASS.

---

## 6. BridgeStatus/state handling

BridgeStatus is excluded from v1 scoring.

Raw source/target state is excluded because reproducible normalization would
require baseline/maxEpsilon not currently present in admitted provenance.

This avoids command leakage and unsupported normalization.

PASS.

---

## 7. Directionality

V1 is explicitly directional because orientationWeight is directional.

Direction reversal is therefore permitted to alter compatibility.

This behavior is versioned and testable, not accidental.

PASS.

---

## 8. Policy identity/version

A fixed policy-family ID derivation and semantic version 1.0 are defined.

Implementation revision will bind the concrete D8D source implementation through
a deterministic SHA-256 manifest.

PASS.

---

## 9. Restricted-origin API

The proposed result API prevents callers from manufacturing trusted D8D success
from raw envelope or arbitrary observation/confidence values.

Production policy snapshot construction is trusted-origin.

PASS.

---

## 10. Deterministic vectors

Five normative fixtures plus invalid-domain cases are defined.

The expected values and binary64 encodings were independently recomputed before
runtime implementation exists.

PASS.

---

## 11. Downstream boundary

D8D stops at semantic interpretation.

It does not emit:

- BridgePolicyEvidence directly;
- PersistentBridgeRecommendation;
- RequestedTransitionDirection;
- eligibility;
- authority capability;
- production commit.

PASS.

---

## 12. Remaining implementation work

After explicit design acceptance:

1. create separate implementation branch from Current Baseline;
2. implement policy/decision IDs;
3. implement trusted production policy snapshot;
4. implement restricted-origin result/rejection types;
5. implement deterministic v1 formulas;
6. implement trace/lineage preservation;
7. add compile-fail misuse gates;
8. validate fixed vectors independently;
9. run exact-head regression/sanitizer/concurrency CI;
10. audit before any merge.

These are implementation tasks, not unresolved design semantics.

---

## 13. Final disposition

```text
structural boundary: PASS
output semantics: PASS
InterpretationPolicyV1 compatibility: PASS
InterpretationPolicyV1 confidence: PASS
directionality: PASS
status/state exclusion: PASS
policy identity/revision: PASS
restricted-origin API: PASS
deterministic vectors: PASS
layer separation: PASS
overall D8D design: READY FOR EXPLICIT DESIGN ACCEPTANCE
implementation authorization: pending explicit acceptance
merge authorization: not granted by this review
```
