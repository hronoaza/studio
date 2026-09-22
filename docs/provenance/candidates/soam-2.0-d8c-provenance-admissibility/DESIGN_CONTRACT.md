# SOAM 2.0 D8C — Provenance Admissibility Design Contract

## Status

- Layer: D8C — Provenance Admissibility
- Document class: design-contract candidate
- Executable implementation: none
- Current Baseline effect: none
- Upstream baseline: accepted D8B Provenance Envelope
  (`09b531bc2950a1e219872b7f6291212bdb3b80f3`)
- Downstream dependency: future D8D Versioned Interpretation
- Acceptance status: design-review pending
- Authority effect: none

This document defines the executable analogue of the formal admissibility
predicate:

```text
AdmissibleB(x)
```

without importing interpretation or authority into D8C.

---

## 1. Boundary

Input:

`ProductionProvenanceEnvelope`

Output on success:

`AdmissibleProductionProvenance`

Output on failure:

`ProvenanceAdmissibilityRejection`

Conceptual chain:

```text
accepted D8B ProductionProvenanceEnvelope
-> D8C ProvenanceAdmissibilityEvaluator
-> AdmissibleProductionProvenance
-> STOP
```

D8C MUST NOT produce:

- `InteractionObservation`;
- `BridgeConfidence`;
- `BridgePolicyEvidence`;
- `RequestedTransitionDirection`;
- `eligible_for_authority_consideration`;
- authority capability;
- production commit.

---

## 2. Formal correspondence

The architecture refinement map identifies the formal candidate predicate:

```text
AdmissibleB(x) =
    ProvenanceKnown(x)
    /\ IntegrityValid(x)
    /\ SchemaCompatibleB(x)
    /\ ~DependsOnRetiredInvariant(x)
    /\ ~ContainsInterpretationA(x)
    /\ ReDerivable(x)
```

D8C v1 maps these concepts to explicit executable predicates.

The mapping is structural, not a formal refinement proof.

---

## 3. D8C v1 predicates

A provenance envelope is admissible only if **all** required predicates pass.

### P1 — ProvenanceOriginRecognized

Meaning:

> the value is a restricted-origin accepted D8B envelope and its declared
> producer family/version/revision is recognized by the active admissibility
> policy.

This is not cryptographic authentication.

Required distinction:

```text
recognized_producer != authenticated_producer
```

D8C v1 does not claim signatures, MACs or attestation.

### P2 — CanonicalIntegrityValid

Meaning:

> recomputing the accepted D8B domain-separated SHA-256 over the exact
> canonical bytes equals the envelope's stored `CanonicalDigest`.

Required distinction:

```text
canonical_digest_valid != source_true
canonical_digest_valid != authenticated_integrity
```

### P3 — SchemaCompatible

Meaning:

> schema family, schema version and canonical encoding version are accepted by
> the active D8C policy.

Unknown schema/version/encoding fails closed.

### P4 — DependenciesAdmissible

Meaning:

> every dependency recorded by D8B is recognized and permitted by the active
> dependency registry/policy.

A dependency classified as:

- unknown;
- retired;
- prohibited;
- incompatible

causes rejection.

### P5 — NoLegacyInterpretationDependency

Meaning:

> no dependency contributing to this provenance envelope is classified as a
> legacy/retired interpretation semantic.

This is the executable analogue of:

`~ContainsInterpretationA(x)`

D8C must not itself interpret source facts.

### P6 — SourceReDerivable

Meaning:

> an independent source-evidence resolver can resolve the envelope's
> `SourceCaptureId` to retained source evidence and reproduce/compare the
> source facts bound by the envelope.

If no retained source evidence is available:

`SourceReDerivable = false`

D8B's `SourceBinding` does not by itself satisfy this predicate.

### P7 — LineageRecognized

Meaning:

> relationship incarnation and state-version lineage are structurally
> recognized by the source evidence used for re-derivation.

D8C v1 does **not** require the envelope to describe the latest live runtime
state.

Live transition freshness remains C2's responsibility.

Therefore:

```text
admissible historical provenance
!= currently executable transition evidence
```

---

## 4. Why D8B alone is insufficient for positive D8C

Accepted D8B v1 intentionally has:

`SourceRecordReferencePresent = 0`

and does not claim source-record retrievability.

Therefore:

```text
well_formed D8B envelope
!= re-derivable source evidence
!= admissible provenance
```

A positive D8C path requires an external, explicitly supplied admissibility
context containing a source-evidence resolver.

Without that context D8C fails closed.

---

## 5. Admissibility context

Candidate type:

`ProvenanceAdmissibilityContext`

It supplies policy/state required to decide admissibility but does not carry
authority.

Conceptual contents:

```text
ProvenanceAdmissibilityContext
├── AdmissibilityPolicyDescriptor
├── ProducerRegistry
├── SchemaRegistry
├── DependencyRegistry
└── SourceEvidenceResolver
```

No component may issue transition capability or commit production state.

---

## 6. Admissibility policy identity

Candidate descriptor:

```text
AdmissibilityPolicyDescriptor
- policyId
- majorVersion
- minorVersion
```

The policy determines:

- recognized producer families/versions/revisions;
- compatible schema versions;
- compatible canonical encoding versions;
- dependency classifications/status;
- source re-derivation requirements;
- deterministic rejection precedence.

D8D later records which accepted admissibility policy admitted the provenance.

---

## 7. Producer registry

The registry maps producer identity/version/revision to a recognition status.

Candidate statuses:

```text
recognized
retired
prohibited
unknown
```

Only `recognized` passes.

Producer recognition is policy recognition only.

It is not cryptographic authentication.

---

## 8. Schema registry

The registry maps:

```text
schemaId
+ schemaMajor
+ schemaMinor
+ canonicalEncodingVersion
```

to compatibility status.

Candidate statuses:

```text
compatible
incompatible
retired
unknown
```

Only `compatible` passes.

No implicit "same major is compatible" rule exists unless explicitly encoded by
the registry.

---

## 9. Dependency registry

Each dependency is classified by:

- stable dependency ID;
- kind;
- version;
- revision where present;
- semantic category;
- lifecycle status.

Candidate semantic categories include:

```text
source_capture_contract
canonical_encoding
digest_profile
runtime_contract
interpretation_policy
other
```

Candidate lifecycle statuses:

```text
active
retired
prohibited
unknown
```

Required rule:

```text
status != active -> reject
```

Additional rule:

```text
category == interpretation_policy -> reject in D8C v1
```

because D8B provenance must remain interpretation-free.

---

## 10. SourceEvidenceResolver

Candidate interface concept:

```cpp
class SourceEvidenceResolver {
public:
    virtual SourceResolutionResult resolve(
        const SourceCaptureId& captureId) const = 0;
};
```

The exact C++ shape is not accepted by this document.

A successful source resolution returns an immutable retained source record
containing at least:

- SourceCaptureId;
- source/target node IDs;
- relationship generation;
- state version;
- distance;
- orientation weight;
- capacity;
- BridgeStatus;
- source/target state;
- source/target health.

The record must originate outside the envelope being checked.

Returning the envelope's own fields as its "independent source record" is not
re-derivation.

---

## 11. Source re-derivation comparison

D8C v1 compares the retained source record to the D8B source binding and source
measurements exactly.

For integers/enums:

- exact value equality.

For floating source values:

- exact IEEE-754 binary64 bit equality.

No tolerance comparison is permitted in provenance re-derivation.

Therefore:

```text
+0.0 != -0.0
```

for re-derivation purposes if their bit patterns differ.

NaN values, if ever admitted upstream, compare by exact captured bit pattern,
not numeric equality.

---

## 12. Required D8B metadata view amendment

Accepted D8B public API currently exposes:

- source identity/facts;
- `CanonicalDigest`;
- canonical bytes.

It does not expose typed immutable access to:

- schema descriptor;
- producer descriptor;
- implementation revision;
- dependency manifest.

D8C should not independently reimplement a second canonical parser merely to
recover metadata that D8B already constructed.

Therefore D8C implementation requires a narrow upstream D8B amendment:

`ProvenanceMetadataView`

Candidate immutable contents:

```text
ProvenanceMetadataView
├── schemaId / schema version / encoding version
├── producerId / producer version / implementation revision
└── dependency manifest entries
```

Required properties:

- read-only;
- derived/stored by accepted D8B producer;
- no new caller-controlled constructor;
- no canonical byte-layout change;
- no interpretation;
- no admissibility result.

This is an API visibility amendment, not a new provenance semantic layer.

---

## 13. Integrity recomputation

D8C should reuse the accepted D8B canonical digest implementation/profile, not
create a semantically different hash routine.

Candidate internal boundary:

```text
recomputeCanonicalDigest(envelope.canonicalBytes())
```

The result must equal:

`envelope.canonicalDigest()`

before any positive admissibility result is possible.

A digest mismatch is terminal rejection.

---

## 14. Positive result type

Candidate type:

`AdmissibleProductionProvenance`

It is restricted-origin.

Only `ProvenanceAdmissibilityEvaluator` may construct it.

Conceptual contents:

```text
AdmissibleProductionProvenance
├── accepted ProductionProvenanceEnvelope
├── AdmissibilityPolicyDescriptor
├── SourceVerificationRecord
└── admissibility decision identity
```

It preserves access to:

- `ProvenanceItemId`;
- `SourceCaptureId`;
- relationship generation;
- state version.

It does not add interpretation or authority.

---

## 15. Admissibility decision identity

Candidate type:

`AdmissibilityDecisionId`

Purpose:

> identify one concrete evaluation event of one provenance item under one
> admissibility policy/context.

It must remain distinct from:

- SourceCaptureId;
- ProvenanceItemId;
- CanonicalDigest.

A future D8D interpretation should retain the decision identity/policy version
that admitted its source provenance.

Generation mechanism is deferred to implementation design.

---

## 16. Rejection result

Candidate type:

`ProvenanceAdmissibilityRejection`

It contains a deterministic set of failed predicates/reasons.

Candidate reason flags:

- producer_unknown;
- producer_retired;
- producer_prohibited;
- implementation_revision_unrecognized;
- canonical_digest_mismatch;
- schema_unknown;
- schema_incompatible;
- schema_retired;
- canonical_encoding_unsupported;
- dependency_unknown;
- dependency_retired;
- dependency_prohibited;
- dependency_incompatible;
- legacy_interpretation_dependency;
- source_record_unavailable;
- source_record_mismatch;
- source_resolver_failure;
- policy_unavailable;
- metadata_inconsistent.

The rejection type carries no authority effect.

---

## 17. Deterministic rejection semantics

Positive result requires all required predicates to pass.

For diagnostics, the evaluator may collect more than one local failure reason.

However external resolution/provider failures must not be retried indefinitely.

Candidate deterministic primary-reason precedence:

```text
1 policy unavailable
2 metadata inconsistent
3 canonical digest mismatch
4 producer failure
5 schema failure
6 dependency failure
7 legacy interpretation dependency
8 source resolver failure/unavailable
9 source record mismatch
```

The full failure set may contain multiple reasons.

No failure reason may be reinterpreted as success.

---

## 18. Evaluation ordering

Candidate fail-closed ordering:

```text
1. validate policy/context availability
2. obtain typed D8B metadata view
3. recompute/compare CanonicalDigest
4. check producer recognition
5. check schema compatibility
6. check every dependency
7. reject legacy interpretation dependency
8. resolve retained source evidence by SourceCaptureId
9. compare source binding + measurements bit-exactly
10. construct restricted-origin AdmissibleProductionProvenance
11. STOP
```

No D8D interpretation occurs inside this sequence.

---

## 19. No live-runtime mutation

D8C evaluation must not mutate:

- mesh topology;
- relationship generation;
- endpoint state/health;
- bridge persistence;
- D8B envelope;
- source evidence record;
- authority ledger;
- production state.

D8C is a decision/evidence gate only.

---

## 20. Concurrency and lifetime

The D8B envelope is immutable.

The admissibility context must provide stable registry/resolver snapshots for one
evaluation event.

A positive result must not combine:

- producer registry from policy revision A;
- schema registry from policy revision B;
- dependency registry from policy revision C

while claiming one coherent admissibility policy version.

Required invariant:

```text
one admissibility decision
-> one coherent policy/context revision
```

How that snapshot is represented is deferred to implementation design.

---

## 21. Relationship to C2 freshness

D8C does not duplicate C2 live revalidation.

D8C asks:

> is this provenance item admissible under the evidence policy?

C2 later asks:

> is the requested relationship/state binding still live/current at transition
> consideration time?

Therefore an admissible provenance item may later become unusable for a live
transition without becoming historically invalid provenance.

---

## 22. Relationship to D8D

D8D may accept only:

`AdmissibleProductionProvenance`

not raw:

`ProductionProvenanceEnvelope`

D8D must preserve:

- SourceCaptureId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- admissibility policy ID/version;
- relationship generation/state version.

This prevents interpretation from silently dropping lineage.

---

## 23. Fail-closed invariant

The principal D8C invariant is:

```text
missing
or unknown
or retired
or prohibited
or unverifiable
or inconsistent
=> not admissible
```

There is no "best effort" positive path.

---

## 24. Verification plan

Before implementation acceptance, tests should include at least:

### Positive

1. recognized producer + compatible schema + active dependencies + valid digest
   + independently matching source record -> admissible;
2. positive result preserves ProvenanceItemId and SourceCaptureId;
3. policy/decision identity is retained.

### Digest / metadata negative

4. digest mismatch -> reject;
5. unknown producer -> reject;
6. retired producer -> reject;
7. unrecognized implementation revision -> reject;
8. unknown schema -> reject;
9. incompatible schema -> reject;
10. unsupported canonical encoding -> reject.

### Dependency negative

11. missing required dependency -> reject;
12. unknown dependency -> reject;
13. retired dependency -> reject;
14. prohibited dependency -> reject;
15. legacy interpretation dependency -> reject;
16. duplicate/inconsistent dependency metadata -> reject.

### Re-derivation negative

17. source record unavailable -> reject;
18. resolver failure -> reject;
19. SourceCaptureId mismatch -> reject;
20. relationship generation mismatch -> reject;
21. state version mismatch -> reject;
22. source/target identity mismatch -> reject;
23. any source measurement bit mismatch -> reject;
24. +0/-0 source mismatch -> reject.

### Restricted-origin / misuse

25. public caller cannot construct `AdmissibleProductionProvenance`;
26. public caller cannot manufacture successful decision identity;
27. raw D8B envelope cannot be passed to D8D API once D8D exists.

### Regression

28. all D8A/D8B/C2/D7 tests remain green;
29. sanitizer/concurrency validation remains clean.

---

## 25. Current positive-path blocker

A full positive D8C implementation is intentionally blocked until two supporting
contracts exist:

1. typed immutable D8B metadata view;
2. independent retained source-evidence resolver/record contract.

Without both, D8C may implement only negative/fail-closed checks.

The architecture must not fake `ReDerivable(x)` by comparing an envelope to
itself.

---

## 26. Non-goals

D8C v1 does not provide:

- semantic normalization;
- compatibility/confidence scoring;
- D8D interpretation;
- live transition freshness;
- signatures/MAC;
- producer cryptographic authentication;
- authority/capability;
- grant/nonce/revocation;
- execution;
- production commit.

---

## 27. Candidate invariants summary

```text
well_formed_envelope != admissible_provenance

recognized_producer != authenticated_producer

canonical_digest_valid != source_true

SourceBinding present != source re-derivable

all required admissibility predicates must pass

one decision -> one coherent policy/context revision

source re-derivation uses independent retained evidence

D8C does not interpret source facts

admissible provenance != currently live transition evidence

AdmissibleProductionProvenance != authority capability
```

---

## 28. Next design gates

Before D8C implementation begins:

1. review/accept this admissibility decomposition;
2. design the D8B typed `ProvenanceMetadataView` amendment;
3. design retained `SourceEvidenceRecord` + `SourceEvidenceResolver`;
4. define admissibility policy/registry snapshot semantics;
5. define `AdmissibilityDecisionId` generation;
6. define restricted-origin C++ result/rejection API;
7. only then implement and validate D8C.
