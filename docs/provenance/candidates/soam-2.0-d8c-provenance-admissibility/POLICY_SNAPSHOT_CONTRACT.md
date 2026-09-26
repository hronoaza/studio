# SOAM 2.0 D8C — ProvenanceAdmissibilityPolicySnapshot Contract

## Status

- Layer: D8C
- Artifact class: policy-snapshot design contract
- Runtime implementation: none
- Current Baseline effect: none
- Acceptance status: candidate

This document fixes the coherence boundary for one D8C admissibility decision.

---

## 1. Core invariant

One admissibility evaluation uses exactly one immutable policy snapshot:

```text
one evaluation
-> one policySnapshotId
-> one policy descriptor
-> one producer registry snapshot
-> one schema registry snapshot
-> one dependency registry snapshot
```

No registry may drift independently during the evaluation.

---

## 2. Candidate type

`ProvenanceAdmissibilityPolicySnapshot`

Conceptual shape:

```text
ProvenanceAdmissibilityPolicySnapshot
├── PolicySnapshotId
├── AdmissibilityPolicyDescriptor
│   ├── policyId
│   ├── majorVersion
│   └── minorVersion
├── ProducerRegistrySnapshot
├── SchemaRegistrySnapshot
├── DependencyRegistrySnapshot
└── ReDerivabilityPolicy
```

The snapshot is immutable after publication.

---

## 3. PolicySnapshotId

Candidate type:

`PolicySnapshotId`

Semantics:

> identity of one immutable admissibility-policy state.

It is distinct from:

- policy family/version;
- AdmissibilityDecisionId;
- ProvenanceItemId;
- SourceCaptureId.

Candidate generation:

- 128-bit opaque ID;
- generated when a new immutable policy snapshot is published;
- not caller-selectable by ordinary D8C callers;
- all-zero invalid.

A policy version may have more than one snapshot only if the backing registry
state actually differs; such snapshots must have distinct IDs.

---

## 4. Producer registry snapshot

Each entry identifies one recognized producer/revision combination.

Candidate entry:

```text
ProducerPolicyEntry
- producerId
- producerMajor
- producerMinor
- implementationRevisionKind
- implementationRevision
- lifecycleStatus
```

Candidate lifecycle statuses:

```text
recognized
retired
prohibited
```

Absence means `unknown`.

Positive D8C requires an exact matching `recognized` entry.

No wildcard implementation revision is permitted in v1 production policy.

This prevents a producer family from being recognized while silently accepting
an unreviewed implementation revision.

---

## 5. Schema registry snapshot

Candidate entry:

```text
SchemaPolicyEntry
- schemaId
- schemaMajor
- schemaMinor
- canonicalEncodingVersion
- lifecycleStatus
```

Candidate lifecycle statuses:

```text
compatible
retired
prohibited
incompatible
```

Absence means `unknown`.

Positive D8C requires an exact compatible tuple unless a future policy version
introduces an explicitly reviewed compatibility range.

No implicit semantic-version compatibility exists in v1.

---

## 6. Dependency registry snapshot

Candidate entry:

```text
DependencyPolicyEntry
- dependencyKind
- dependencyId
- versionMajor
- versionMinor
- revisionKind
- revisionDigest
- semanticCategory
- lifecycleStatus
```

Candidate semantic categories:

- source_capture_contract;
- canonical_encoding;
- digest_profile;
- runtime_contract;
- interpretation_policy;
- other.

Candidate lifecycle statuses:

- active;
- retired;
- prohibited;
- incompatible.

Absence means `unknown`.

Positive D8C requires every envelope dependency to have one exact active
matching policy entry.

---

## 7. Required-dependency set

The policy snapshot also defines which dependency kinds are mandatory for D8B
v1.

Candidate mandatory set:

```text
SOURCE_CAPTURE_CONTRACT
CANONICAL_ENCODING_SPEC
DIGEST_PROFILE
```

Therefore D8C rejects an envelope that omits one of these even if all present
dependencies are individually active.

This closes the difference between:

```text
all present dependencies are acceptable
```

and:

```text
all required dependencies are present and acceptable
```

---

## 8. Duplicate/conflict semantics

Within one policy snapshot:

- duplicate identical registry entries are invalid configuration;
- conflicting entries for the same registry key are invalid configuration;
- one registry key maps to exactly one lifecycle decision.

A policy snapshot with internal duplicate/conflicting entries is not publishable.

Therefore production D8C never needs to guess which duplicate registry entry
wins.

---

## 9. ReDerivabilityPolicy

Candidate v1:

```text
requireIndependentRetainedSourceRecord = true
allowEnvelopeSelfReconstruction = false
sourceComparison = bit_exact
resolverAttemptsPerEvaluation = 1
```

No configuration exists in v1 that permits positive admission without retained
source evidence.

---

## 10. Snapshot construction

Policy snapshot publication follows:

```text
prepare descriptor
-> prepare producer registry
-> prepare schema registry
-> prepare dependency registry
-> validate duplicates/conflicts
-> validate mandatory dependency policy
-> mint PolicySnapshotId
-> publish immutable snapshot
```

Potentially failing preparation occurs before publication.

A partially constructed policy state must not be observable.

---

## 11. Evaluation usage

`ProvenanceAdmissibilityEvaluator` receives:

- one immutable D8B envelope;
- one immutable `ProvenanceAdmissibilityPolicySnapshot`;
- one source resolver interface.

It does not query mutable global registries during the decision.

Required property:

```text
decision.policySnapshotId
== exact snapshot used for every local policy predicate
```

---

## 12. Policy failure semantics

If the snapshot itself is unavailable or structurally invalid:

```text
policy unavailable/invalid
-> no positive admissibility result
```

The evaluator must not silently substitute:

- latest policy;
- default policy;
- another version;
- permissive fallback.

---

## 13. Equality and identity

Two policy snapshots are the same decision context only if:

`PolicySnapshotId equal`

Content equality alone does not collapse separately published policy snapshots.

This keeps audit identity explicit.

---

## 14. Non-goals

The policy snapshot does not:

- authenticate producer keys;
- validate signatures;
- perform D8D interpretation;
- grant execution rights;
- mutate runtime state.

---

## 15. Required tests

Implementation should verify:

1. valid snapshot is immutable;
2. caller cannot forge PolicySnapshotId;
3. exact producer revision required;
4. unknown producer rejects;
5. retired/prohibited producer rejects;
6. exact schema tuple required;
7. missing mandatory dependency rejects;
8. unknown/retired/prohibited dependency rejects;
9. interpretation-policy dependency rejects;
10. duplicate/conflicting registry entry prevents snapshot publication;
11. evaluation does not observe registry mutation after snapshot creation;
12. no fallback policy is used when snapshot unavailable.
