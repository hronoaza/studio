# Pre-Acceptance Audit — SOAM 2.0 D8C Provenance Admissibility

## Repository state

- Current Baseline: `09b531bc2950a1e219872b7f6291212bdb3b80f3`
- Candidate branch:
  `candidate/soam-2.0-d8c-provenance-admissibility-runtime`
- Exact validated implementation/test head:
  `70e5dd371347a3218dba4298c9072a9c452fb0c9`
- PR: #16
- Acceptance status: `acceptance-pending`

## Implemented boundary

```text
accepted D8B ProductionProvenanceEnvelope
+ immutable trusted policy snapshot
+ independent retained D8A source evidence
-> ProvenanceAdmissibilityEvaluator
-> accepted: AdmissibleProductionProvenance
   rejected: ProvenanceAdmissibilityRejection
-> STOP
```

D8C does not perform D8D interpretation or authority.

## D8B metadata view

D8B now exposes immutable typed metadata:

- schema family/version/encoding;
- producer family/version/implementation revision;
- dependency manifest.

The metadata view is produced by D8B and used to construct canonical input.

It is not caller-replaceable.

The wire encoding remains CanonicalEnvelopeV1.

## Policy trust boundary

A critical fail-open design was identified during implementation review:

> a public arbitrary policy publisher would allow a caller to approve its own
> envelope metadata.

That API was removed before the validated head.

Production admissibility uses only a trusted provider with compiled accepted
registry values.

Therefore:

```text
caller-chosen policy != production admissibility policy
```

Negative policy states exist only in the internal test surface.

## Exact implementation revision

The production policy requires the exact D8B implementation revision generated
from the accepted D8B implementation manifest.

Recognizing the producer family alone is insufficient.

Unknown revision fails closed.

## Schema/dependency policy

Positive admission requires:

- exact recognized schema/version/canonical encoding;
- required dependency kinds 1, 2 and 3 present;
- each dependency exactly recognized;
- each dependency active;
- no interpretation-policy dependency.

Missing/unknown/retired/prohibited/incompatible conditions reject.

## Canonical digest gate

D8C recomputes the accepted D8B domain-separated SHA-256 over the immutable
canonical bytes and compares it to the stored `CanonicalDigest`.

Digest failure short-circuits before source resolver invocation.

This establishes canonical-content consistency only.

It does not authenticate producer origin or prove source truth.

## Independent source re-derivability

Retained source evidence is created from the restricted-origin D8A snapshot
through a separate store path.

D8B envelope production and retained-source publication are sibling operations.

D8C compares retained source facts with envelope source binding/measurements using
bit-exact binary64 comparison.

Therefore the positive path does not use envelope self-reconstruction as
evidence of re-derivability.

## Retention scope

The current `RetainedSourceEvidenceStore` is process-lifetime/in-memory.

Thus the implemented claim is:

```text
retained source record available in this store
-> in-process independent re-derivability check possible
```

It is not:

```text
source remains re-derivable across restart/machine failure
```

Persistent source retention is a separate future evidence-storage boundary.

## Decision identity

Each completed accepted or rejected evaluation is assigned a fresh
`AdmissibilityDecisionId`.

Decision identity is distinct from:

- SourceCaptureId;
- ProvenanceItemId;
- PolicySnapshotId;
- CanonicalDigest.

Decision-ID generation failure yields no completed D8C result.

## Resolver side-effect boundary

Terminal local rejection occurs before resolver invocation.

The positive/re-derivation path invokes the resolver once.

This preserves the accepted deterministic external-resolution bound.

## C2 freshness separation

D8C verifies source lineage consistency against retained evidence.

It does not claim the relationship/state version is still current for a live
transition.

C2 remains responsible for live transition revalidation.

## Regression evidence

Exact head `70e5dd371347a3218dba4298c9072a9c452fb0c9` passed:

- Runtime #50 / `35710293181`;
- D8A #36 / `35710293197`;
- D8B #9 / `35710293138`;
- Live Evaluator #47 / `35710293183`;
- D7 #45 / `35710293144`;
- D8C #1 / `35710293291`.

All twelve sanitizer jobs report 24/24 PASS.

## Evidence limits

This audit does not establish:

- source truth;
- cross-restart source retention;
- cryptographic producer authentication;
- D8D interpretation correctness;
- current live-transition freshness;
- formal C++/TLA+ refinement;
- authority/capability/commit readiness.

## Audit disposition

```text
D8B metadata-view boundary: verified
production policy trust boundary: verified
exact producer-revision gate: verified
canonical digest gate: verified
independent in-process source re-derivability: verified
resolver short-circuit/single-call boundary: verified
D8A/D8B/C2/D7 regression: verified
technical status: verified-on-cited-linux-CI
acceptance status: ready-for-explicit-acceptance
merge authorization: not granted by this audit
```
