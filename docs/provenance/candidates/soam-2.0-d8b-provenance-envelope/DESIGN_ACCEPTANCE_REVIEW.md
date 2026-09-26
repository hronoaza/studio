# SOAM 2.0 D8B — Design Acceptance Review

## Review target

- PR: #12
- Branch: `candidate/soam-2.0-d8b-provenance-envelope-design`
- Upstream Current Baseline: `379cd2e2cf17eb7181629945e670f7923ba662ce`
- Executable D8B implementation: none
- Review scope: architecture/design only
- Disposition: **ACCEPTED — IMPLEMENTATION AUTHORIZED**

---

## 1. Upstream correctness

The original D8B blocker around acquisition/sample identity has been resolved by
accepted D8A.

D8B now consumes an immutable restricted-origin snapshot carrying an accepted
`SourceCaptureId`.

It does not mint a replacement acquisition identity.

PASS.

---

## 2. Identity separation

The design keeps distinct:

```text
relationship address
relationship incarnation
SourceCaptureId
ProvenanceItemId
CanonicalDigest
```

The reviewed v1 decision selects an independent 128-bit OS-backed random
`ProvenanceItemId` family and retains the accepted D8A `SourceCaptureId`
unchanged.

PASS.

---

## 3. Canonical representation

`CanonicalEnvelopeV1` defines an exact portable binary preimage.

The review found no duplicate canonical ownership, native-ABI dependence or
ambiguous variable-field concatenation in v1.

Node IDs are normalized to u64 and enum tags are schema-owned.

PASS.

---

## 4. Digest boundary

The design correctly uses the narrower term `CanonicalDigest`.

SHA-256 binds canonical bytes under explicit domain separation but does not
claim authentication, source truth or authority.

PASS.

---

## 5. Source binding/retrieval boundary

Mandatory `SourceBinding` is separated from future
`SourceRecordReference`.

The design does not claim that a binding is independently retrievable source
material.

PASS.

---

## 6. Normative vectors

All five published vector lengths and SHA-256 values were independently
recomputed from the documented canonical hex and matched exactly.

PASS.

---

## 7. Restricted-origin boundary

The proposed C++ surface permits ordinary callers to provide only an accepted
D8A snapshot.

Identity, schema/producer metadata, dependency manifest and digest remain
producer-owned.

Arbitrary decoded bytes cannot directly become trusted D8B provenance.

PASS.

---

## 8. Failure/publication boundary

The design requires detached preparation and fail-closed publication.

No failure path is allowed to partially publish an envelope or mutate live mesh
state.

PASS.

---

## 9. Layer separation

D8B does not create:

- observation/confidence;
- policy recommendation;
- requested transition direction;
- eligibility;
- capability;
- authority;
- production commit.

D8C and D8D remain separate future layers.

PASS.

---

## 10. Remaining implementation details

The following remain implementation work rather than design blockers:

- exact registry bytes for production schema/producer IDs;
- canonical implementation-revision manifest format;
- concrete SHA-256 implementation/provider;
- concrete internal failure-injection seams;
- exact file/type placement;
- exact CI matrix beyond the currently accepted Linux baseline.

These must be reviewed with implementation evidence.

---

## 11. Final disposition

The D8B v1 design is coherent with accepted D8A and with the provenance
refinement boundary.

No blocking design defect remains from `DESIGN_REVIEW_1.md`.

Disposition:

```text
architecture review: PASS
wire-format review: PASS
normative vectors: independently verified
restricted-origin design: PASS
layer separation: PASS
implementation authorization: pending explicit Root Operator acceptance
merge authorization: not granted by this review
```

If explicit D8B design acceptance is granted, the next step is implementation on
a controlled candidate branch followed by exact-head CI, conformance tests,
negative misuse gates and audit.


---

## Explicit acceptance record

The Root Operator explicitly accepted the D8B v1 design after this review.

Recorded consequence:

```text
D8B design: accepted
runtime implementation: authorized on controlled candidate branch
PR #12 merge: not implied by design acceptance
Current Baseline: unchanged by this acceptance record
```

Implementation proceeds separately in PR #14 against the accepted D8A Current
Baseline.
