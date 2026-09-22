# Pre-Acceptance Audit — SOAM 2.0 D8D Versioned Interpretation

## Repository state

- Current Baseline: `391536d444eeac8f7284f30f4e6241b1d944560c`
- Candidate branch:
  `candidate/soam-2.0-d8d-versioned-interpretation-runtime`
- Exact validated implementation/test head:
  `f128203036c9bb6031ea753c821bdcba233d9d73`
- PR: #18
- Acceptance status: `acceptance-pending`

---

## Implemented boundary

```text
accepted D8C AdmissibleProductionProvenance
+ trusted immutable D8D policy snapshot
-> VersionedProductionInterpreter
-> accepted: VersionedProductionInterpretation
   rejected: ProductionInterpretationRejection
-> STOP
```

D8D does not produce recommendation, eligibility, authority or commit.

---

## InterpretationPolicyV1

The implementation uses the explicitly accepted v1 policy:

```text
compatibility =
    capacity
    * (1 / (1 + 0.1 * distance))
    * orientationWeight

confidence =
    min(sourceHealth, targetHealth)
```

The compatibility equation exactly reuses the accepted runtime
effective-coupling function.

The semantic equivalence between effective coupling and compatibility is a
versioned D8D policy choice, not a claim of physical truth.

The confidence rule is likewise a versioned D8D semantic choice.

---

## Consumed-field boundary

V1 consumes exactly:

- distance;
- orientationWeight;
- capacity;
- sourceHealth;
- targetHealth.

It does not score:

- BridgeStatus;
- sourceState;
- targetState;
- relationship generation;
- state version;
- node IDs.

This prevents hidden command mapping and unsupported state normalization.

---

## Policy trust boundary

Production interpretation policy comes only from:

`ProductionInterpretationPolicyProvider::createCurrent()`

Ordinary callers cannot default-construct a production policy snapshot.

The production descriptor fixes:

- policy family ID;
- semantic version 1.0;
- implementation revision kind/digest;
- distance coefficient 0.1;
- directional semantics.

Arbitrary policy mutation is available only through an internal test seam.

---

## Policy identity/revision

The fixed policy-family ID corresponds to the documented registry bytes:

```text
63 de 65 6b eb 62 57 e5
36 ea 0d 52 f4 21 43 f8
```

The concrete implementation revision is independently bound through a
deterministic SHA-256 source manifest.

Policy semantic identity and implementation identity remain separate.

---

## Restricted-origin result boundary

Ordinary callers cannot:

- default-construct InterpretationDecisionId;
- construct InterpretationDecisionId from arbitrary bytes;
- default-construct VersionedProductionInterpretation;
- manufacture trusted interpretation from arbitrary
  InteractionObservation/BridgeConfidence;
- default-construct production InterpretationPolicySnapshot.

The only production success path is VersionedProductionInterpreter operating on
accepted D8C provenance.

---

## Upstream lineage

The success object owns/preserves the admitted D8C object.

Therefore D8D retains:

- SourceCaptureId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- D8C PolicySnapshotId;
- relationship generation;
- state version.

D8D adds:

- InterpretationPolicySnapshotId;
- InterpretationDecisionId;
- policy descriptor/revision;
- semantic trace.

It does not replace upstream lineage.

---

## Determinism

For fixed admitted provenance and fixed policy snapshot, the implementation uses
no:

- clock;
- environment override;
- mutable global weight;
- caller-provided threshold;
- random semantic input.

Only event identity changes between repeated interpretation events.

Tests verify repeated evaluations preserve compatibility/confidence while
producing different InterpretationDecisionId values.

---

## Directionality

V1 is intentionally directional.

The policy consumes accepted directed `orientationWeight`.

Forward/reverse relationship interpretations may therefore differ.

This is explicit, versioned and regression-tested.

---

## Failure semantics

Invalid consumed values fail closed.

The implementation distinguishes:

```text
completed semantic/policy rejection
!= evaluator infrastructure failure
```

Policy revision mismatch is a typed rejection.

Decision-ID generation failure yields no completed interpretation result.

No failure path creates a partial trusted success object.

---

## Downstream boundary

D8D does not directly construct:

- BridgePolicyEvidence;
- PersistentBridgeRecommendation;
- RequestedTransitionDirection;
- eligible_for_authority_consideration;
- capability/grant;
- production commit.

The accepted domain chain remains downstream and separate.

---

## Regression evidence

Exact head `f128203036c9bb6031ea753c821bdcba233d9d73` passed:

- Runtime #55 / `35713247314`;
- D8A #41 / `35713247321`;
- D8B #14 / `35713247305`;
- D8C #6 / `35713247352`;
- D8D #4 / `35713247312`;
- Live Evaluator #52 / `35713247350`;
- D7 #50 / `35713247396`.

All fourteen sanitizer jobs report 30/30 PASS.

---

## Known evidence limits

This audit does not establish:

- universal correctness of InterpretationPolicyV1 as a physical model;
- cross-restart D8C retained-source availability;
- cryptographic authentication;
- live freshness as a D8D property;
- transition eligibility;
- authority/capability/commit readiness;
- formal C++/TLA+ refinement.

---

## Audit disposition

```text
InterpretationPolicyV1 implementation: verified
policy trust boundary: verified
implementation-revision binding: verified
lineage preservation: verified
deterministic vectors: verified
directionality: verified
restricted-origin result boundary: verified
policy-rejection/infrastructure separation: verified
D8A/D8B/D8C/C2/D7 regression: verified
technical status: verified-on-cited-linux-CI
acceptance status: ready-for-explicit-acceptance
merge authorization: not granted by this audit
```
