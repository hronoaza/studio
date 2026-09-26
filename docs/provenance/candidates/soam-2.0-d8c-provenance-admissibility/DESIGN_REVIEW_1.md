# SOAM 2.0 D8C — Design Review 1

## Status

- Reviewed artifact: `DESIGN_CONTRACT.md`
- Review class: architecture/design review
- Runtime changes: none
- Disposition: **REVISE BEFORE DESIGN ACCEPTANCE**

The D8C decomposition is sound, but four contract points require tightening
before implementation.

---

## Findings

| ID | Severity | Topic | Disposition |
|---|---:|---|---|
| D8C-DR-001 | BLOCKING | source-record origin / circular re-derivation | revise |
| D8C-DR-002 | BLOCKING | metadata-view / canonical-byte consistency | revise |
| D8C-DR-003 | MAJOR | policy snapshot coherence | make explicit immutable snapshot |
| D8C-DR-004 | MAJOR | decision identity semantics | fix ownership/generation |
| D8C-DR-005 | MAJOR | rejection aggregation across external resolver | bound deterministically |
| D8C-DR-006 | MAJOR | lineage predicate naming | narrow claim |
| D8C-DR-007 | MAJOR | source-record retention failure | define fail-closed lifecycle |

---

## D8C-DR-001 — Retained source evidence must have an independent origin

### Problem

The draft requires an independent `SourceEvidenceResolver` but does not define
who may create the retained record.

If a record can be reconstructed from the D8B envelope being evaluated, then:

```text
envelope -> source record -> compare to same envelope
```

is circular and does not satisfy the intended `ReDerivable(x)` property.

### Required correction

Introduce a restricted-origin retained source record created from the authentic
D8A snapshot through a separate source-retention path.

Required chain:

```text
D8A ProductionRelationshipSourceSnapshot
├── D8B envelope production
└── source-evidence retention
    -> RetainedSourceEvidenceRecord
```

The retained record MUST NOT be constructed from:

- D8B canonical bytes;
- D8B envelope accessors;
- D8C evaluation inputs.

D8C resolves it by `SourceCaptureId`.

This establishes independent retention lineage, though it still does not prove
the original runtime source facts were true.

---

## D8C-DR-002 — Typed metadata cannot diverge from canonical bytes

### Problem

Adding `ProvenanceMetadataView` as separately mutable/independent metadata
could create two semantic owners:

```text
typed metadata A
canonical bytes B
```

with no guarantee that A == B.

### Required correction

The accepted D8B producer must create both from one immutable structured
construction input.

After construction:

- metadata view is immutable;
- canonical bytes are immutable;
- canonical digest binds canonical bytes;
- no public caller can replace either;
- D8C first validates canonical digest before trusting metadata for a positive
  decision.

Required invariant:

```text
one D8B construction input
-> one typed metadata view
-> one canonical encoding
```

A future D8B amendment must include consistency tests proving that typed metadata
and canonical bytes correspond to the same accepted fields.

---

## D8C-DR-003 — Admissibility context must be one immutable policy snapshot

### Problem

A context containing separate registry references can drift during one
evaluation.

### Required correction

Replace the loose context model with:

`ProvenanceAdmissibilityPolicySnapshot`

It contains one immutable policy revision and all registry decisions used by one
evaluation.

Required invariant:

```text
one evaluation
-> one policySnapshotId
-> one policy version
-> one producer registry state
-> one schema registry state
-> one dependency registry state
```

The source resolver may be external, but the policy determining how its result
is evaluated is fixed by the snapshot.

---

## D8C-DR-004 — Decision identity must be D8C-owned

`AdmissibilityDecisionId` must be minted only after all checks needed for a
decision event are available.

Recommended semantics:

- 128-bit opaque ID;
- OS-backed random generation family;
- all-zero invalid;
- not caller-selectable;
- distinct from SourceCaptureId / ProvenanceItemId / digest;
- one evaluation event gets one decision ID whether accepted or rejected, if a
  decision record is published.

If ID generation fails, no positive admissibility object is published.

Whether rejected decisions receive externally visible IDs should be fixed before
implementation.

Recommended v1: both accepted and rejected completed decisions carry a decision
ID, so audit records can correlate one deterministic evaluation event.

---

## D8C-DR-005 — External resolver failure bounds

The evaluator may collect multiple local registry failures without external
calls.

It must not repeatedly invoke the source resolver while collecting diagnostics.

Recommended rule:

- at most one source-resolution attempt per evaluation;
- no source-resolution call if an earlier terminal local predicate already makes
  positive admission impossible, unless an explicit diagnostic mode is later
  designed;
- production v1 uses short-circuit fail-closed evaluation.

This keeps D8C deterministic and prevents external/provider side effects from
being multiplied by diagnostics.

---

## D8C-DR-006 — Narrow `LineageRecognized`

The term can imply live-current lineage.

D8C only proves correspondence between the envelope and retained source record.

Rename candidate predicate to:

`SourceLineageConsistent`

Meaning:

> SourceCaptureId, relationship incarnation and state version in the retained
> record exactly match the envelope.

It does not mean current-live or authoritative.

---

## D8C-DR-007 — Retention failure semantics

The source-retention path must define publication independently of D8B.

Required rules:

- failed retention publishes no partial record;
- one SourceCaptureId maps to at most one accepted retained record;
- conflicting second record for the same SourceCaptureId is an integrity fault,
  not an overwrite;
- records are immutable;
- missing record means D8C rejection, not reconstruction from envelope;
- D8B envelope creation may succeed even if independent retention fails, but
  such envelope is not positively admissible under D8C v1.

This distinction is essential:

```text
D8B production success != D8C admissibility success
```

---

## Revised D8C chain

```text
D8A snapshot
├── D8B envelope
└── retained source record

D8B envelope
+ immutable admissibility policy snapshot
+ resolver(SourceCaptureId -> retained source record)
-> local checks
-> one bounded source resolution
-> bit-exact source comparison
-> D8C decision
-> accepted: AdmissibleProductionProvenance
   rejected: ProvenanceAdmissibilityRejection
-> STOP
```

---

## Disposition

The initial D8C contract is not rejected.

Its core separation of:

```text
well-formed provenance
!= admissible provenance
!= interpretation
!= authority
```

is retained.

Before acceptance, resolve D8C-DR-001 and D8C-DR-002 in supporting contracts and
make D8C-DR-003 through D8C-DR-007 explicit in the main contract.
