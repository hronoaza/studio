# SOAM 2.0 D8D — Policy Identity and Revision Contract

## Status

- Layer: D8D
- Policy: InterpretationPolicyV1
- Runtime implementation: none
- Current Baseline effect: none
- Acceptance status: final-review candidate

---

## 1. Policy family identity

InterpretationPolicyV1 uses one fixed 128-bit opaque policy-family ID.

Registry label:

`SOAM.D8D.INTERPRETATION.COUPLING_HEALTH.V1`

Candidate fixed bytes:

```text
63 de 65 6b eb 62 57 e5
36 ea 0d 52 f4 21 43 f8
```

These bytes are the first 16 bytes of:

```text
SHA-256(
  ASCII("SOAM.D8D.INTERPRETATION.COUPLING_HEALTH.V1")
)
```

The derivation is documented only to make the registry constant reproducible.

At runtime the policy ID is a fixed constant, not recomputed from arbitrary
caller text.

---

## 2. Semantic version

InterpretationPolicyV1:

```text
major = 1
minor = 0
```

Version rules:

- semantic meaning change -> major version;
- compatible trace/metadata extension -> minor version if no existing semantic
  output is reinterpreted;
- any change to compatibility/confidence equations -> major version;
- any change to consumed field set -> major version;
- any change to the inherited distance coefficient 0.1 -> major version.

No silent semantic drift is permitted under one policy version.

---

## 3. Policy snapshot identity

Candidate type:

`InterpretationPolicySnapshotId`

Semantics:

> identity of one immutable published D8D policy snapshot.

It is distinct from:

- InterpretationPolicyId;
- InterpretationDecisionId;
- D8C PolicySnapshotId;
- AdmissibilityDecisionId.

Candidate representation:

- 128-bit opaque bytes;
- OS-backed random generation family;
- all-zero invalid;
- restricted-origin;
- one immutable published policy snapshot -> one snapshot ID.

For v1, one production snapshot contains exactly the fixed policy family/version
and constants defined in InterpretationPolicyV1.

---

## 4. Implementation revision

The production policy descriptor includes:

```text
implementationRevisionKind
implementationRevision[32]
```

Candidate production rule:

- kind 1 = SHA-256;
- digest is SHA-256 over a deterministic D8D implementation manifest.

The implementation manifest should contain SHA-256 values of:

- public D8D API header;
- internal interpretation contract/header;
- D8D implementation source.

The manifest must be independent of:

- build-directory paths;
- timestamps;
- documentation-only commits;
- compiler temporary files.

This mirrors the accepted D8B implementation-revision discipline.

---

## 5. Semantic-policy provenance

The implementation revision identifies the concrete implementation of the
already versioned semantic policy.

It does not replace policy family/version.

Required distinction:

```text
policyId/version = semantic contract identity
implementationRevision = concrete reviewed implementation identity
```

Neither is producer authentication.

---

## 6. Policy-owned constants

InterpretationPolicyV1 owns exactly:

```text
distanceAttenuationCoefficient = 0.1
compatibilityFunction = effectiveCoupling
confidenceFunction = min(sourceHealth, targetHealth)
```

No other hidden thresholds/weights are permitted.

---

## 7. Field manifest identity

The field-usage manifest is part of policy semantics.

V1 declared semantic inputs:

- distance;
- orientationWeight;
- capacity;
- sourceHealth;
- targetHealth.

V1 declared non-scoring fields:

- BridgeStatus;
- sourceState;
- targetState;
- node IDs;
- relationship generation;
- state version.

Any change moves to a new major policy version.

---

## 8. Acceptance tests

Implementation must prove:

1. fixed policy ID bytes match this registry;
2. policy version is 1.0;
3. no caller-selected policy ID/version in production path;
4. implementation revision is non-zero kind 1;
5. docs-only changes do not change implementation revision;
6. semantic code change does change implementation revision;
7. policy snapshot identity is restricted-origin;
8. repeated policy snapshot publication yields distinct snapshot identity if a
   genuinely new snapshot event is published.
