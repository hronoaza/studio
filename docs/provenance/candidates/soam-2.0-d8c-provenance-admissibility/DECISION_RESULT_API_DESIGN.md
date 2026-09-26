# SOAM 2.0 D8C — Decision Identity and Result API Design

## Status

- Layer: D8C
- Artifact class: C++ API design
- Runtime implementation: none
- Current Baseline effect: none
- Acceptance status: candidate

---

## 1. Decision identity

Candidate public value type:

`AdmissibilityDecisionId`

Semantics:

> identity of one completed D8C admissibility evaluation event.

Candidate representation:

- 128-bit / 16 opaque bytes;
- OS-backed random generation family;
- all-zero reserved invalid;
- no timestamp/UUID textual semantics;
- not caller-selectable;
- immutable.

It is distinct from:

```text
SourceCaptureId
ProvenanceItemId
PolicySnapshotId
CanonicalDigest
```

---

## 2. When the decision ID is minted

The decision ID is prepared before final result publication but after local
input/context validation establishes that an evaluation can proceed.

Preferred ordering:

```text
validate inputs/context
-> reserve AdmissibilityDecisionId
-> execute deterministic fail-closed predicates
-> publish exactly one completed decision result
```

If decision-ID generation fails:

```text
no decision ID
-> no completed D8C result publication
```

No positive result may exist without a decision ID.

---

## 3. Both success and rejection carry decision identity

D8C v1 records both completed outcomes.

Therefore:

- `AdmissibleProductionProvenance` carries AdmissibilityDecisionId;
- `ProvenanceAdmissibilityRejection` carries AdmissibilityDecisionId.

This gives one audit correlation key per completed evaluation.

A provider crash/failure before a completed decision is formed may yield an API
execution error rather than a completed rejection record; exact transport is
implementation-design detail.

---

## 4. Success type

Candidate:

`AdmissibleProductionProvenance`

Required contents:

```text
AdmissibleProductionProvenance
├── AdmissibilityDecisionId
├── PolicySnapshotId
├── AdmissibilityPolicyDescriptor
├── ProvenanceItemId
├── SourceCaptureId
├── relationshipGeneration
├── stateVersion
├── sourceVerificationSummary
└── immutable reference/value ownership of accepted D8B envelope
```

No interpretation score or authority bit is present.

---

## 5. SourceVerificationSummary

Candidate contents:

- retainedSourceRecordId or equivalent stable store identity if the selected
  source store has one;
- SourceCaptureId;
- bitExactMatch = true;
- resolver result class = found;
- verification method/version.

If the v1 source store has no independent record ID, the summary may instead
carry the SourceCaptureId plus store/snapshot identity.

It must not claim more than the resolver actually proves.

---

## 6. Rejection type

Candidate:

`ProvenanceAdmissibilityRejection`

Required contents:

```text
ProvenanceAdmissibilityRejection
├── AdmissibilityDecisionId
├── PolicySnapshotId
├── ProvenanceItemId
├── primaryReason
└── reasonFlags
```

The rejection does not contain an admissible wrapper.

---

## 7. Rejection reason enum

Candidate closed v1 enum:

```text
PolicyUnavailable
MetadataInconsistent
CanonicalDigestMismatch
ProducerUnknown
ProducerRetired
ProducerProhibited
ImplementationRevisionUnrecognized
SchemaUnknown
SchemaIncompatible
SchemaRetired
CanonicalEncodingUnsupported
RequiredDependencyMissing
DependencyUnknown
DependencyRetired
DependencyProhibited
DependencyIncompatible
LegacyInterpretationDependency
SourceRecordUnavailable
SourceResolverFailure
SourceEvidenceIntegrityConflict
SourceRecordMismatch
```

The exact integer tags are deferred until runtime API implementation but must be
stable once public.

---

## 8. Deterministic primary reason

Primary reason follows fixed precedence:

```text
1  PolicyUnavailable
2  MetadataInconsistent
3  CanonicalDigestMismatch
4  Producer*
5  Schema*
6  RequiredDependencyMissing
7  Dependency*
8  LegacyInterpretationDependency
9  SourceResolverFailure
10 SourceEvidenceIntegrityConflict
11 SourceRecordUnavailable
12 SourceRecordMismatch
```

Within the producer/schema/dependency groups, the closed enum order is used.

This precedence is diagnostic only; no lower-precedence reason can turn a
failure into success.

---

## 9. Result carrier

Preferred public shape:

```cpp
using ProvenanceAdmissibilityResult =
    std::variant<
        AdmissibleProductionProvenance,
        ProvenanceAdmissibilityRejection>;
```

Candidate evaluator:

```cpp
class ProvenanceAdmissibilityEvaluator final {
public:
    [[nodiscard]]
    std::optional<ProvenanceAdmissibilityResult> evaluate(
        const ProductionProvenanceEnvelope& envelope,
        const ProvenanceAdmissibilityPolicySnapshot& policy,
        const SourceEvidenceResolver& resolver) const;
};
```

Interpretation of outer `std::optional`:

- value present = completed D8C decision, accepted or rejected;
- nullopt = evaluator infrastructure failure before a completed decision could
  be published, such as decision-ID generation failure.

This keeps policy rejection distinct from evaluator infrastructure failure.

The exact error carrier may later replace `optional`, but a completed rejection
must never be conflated with execution failure.

---

## 10. Restricted-origin success

`AdmissibleProductionProvenance` has:

- no public default constructor;
- no public raw-field constructor;
- no public success-flag setter.

Only `ProvenanceAdmissibilityEvaluator` may construct it after all predicates
pass.

A public caller cannot convert a raw D8B envelope into admissible provenance.

---

## 11. Rejection construction

The evaluator owns production rejection creation.

Tests may use internal builders, but ordinary callers must not be able to
construct a rejection and present it as evaluator-produced audit evidence unless
the API intentionally marks it as synthetic/test data.

Preferred v1: production result types remain restricted-origin.

---

## 12. Immutability

Both success and rejection result objects are immutable after publication.

Corrections or re-evaluation create a new decision ID and a new result.

No prior decision is rewritten.

---

## 13. Re-evaluation semantics

Evaluating the same D8B envelope twice under the same policy snapshot creates two
distinct decision events:

```text
same ProvenanceItemId
same PolicySnapshotId
different AdmissibilityDecisionId
```

The results should be semantically identical if resolver state and retained
evidence are unchanged, but they are still separate evaluation events.

---

## 14. Resolver call rule

Production evaluator invokes resolver at most once.

It invokes the resolver only after all local predicates required before
re-derivation pass.

Therefore:

```text
local terminal rejection
-> no resolver call
```

This is part of deterministic side-effect control.

---

## 15. C++ ownership

Preferred v1 success result owns/copies the immutable D8B envelope or stores a
shared immutable lifetime-safe value.

It must not retain a dangling reference to a caller stack object.

Likewise, policy identity/version and source-verification summary are stored by
value.

The exact ownership mechanism is implementation design, but lifetime safety is
mandatory.

---

## 16. Compile-fail gates

Implementation acceptance requires public misuse rejection for:

1. default construction of AdmissibilityDecisionId;
2. arbitrary-byte construction of AdmissibilityDecisionId;
3. default construction of AdmissibleProductionProvenance;
4. raw-envelope construction of AdmissibleProductionProvenance;
5. caller-selected PolicySnapshotId on success object;
6. mutation of decision identity;
7. mutation of acceptance result lineage fields.

---

## 17. Runtime tests

Required:

1. positive evaluation produces success variant;
2. rejection produces rejection variant, not infrastructure failure;
3. success/rejection both carry non-zero decision ID;
4. repeated evaluation produces different decision IDs;
5. SourceCaptureId/ProvenanceItemId preserved;
6. exact PolicySnapshotId preserved;
7. decision-ID generator failure produces no completed result;
8. terminal local rejection causes zero resolver calls;
9. re-derivation path causes exactly one resolver call;
10. evaluator mutates no live runtime state.

---

## 18. Non-goals

The result API does not contain:

- D8D interpretation;
- confidence;
- transition direction;
- eligibility;
- authority capability;
- execution state.

---

## 19. Candidate invariant summary

```text
one completed evaluation -> one AdmissibilityDecisionId

accepted and rejected are both completed decisions

completed rejection != evaluator infrastructure failure

same envelope re-evaluated -> new decision identity

success object is restricted-origin

decision identity != capture identity != item identity != policy snapshot identity

D8C result != authority
```
