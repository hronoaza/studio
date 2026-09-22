# CanonicalEnvelopeV1 — Normative Conformance Test Vectors

## Status

These vectors belong to the D8B wire-format candidate.

All schema/producer/item/capture IDs below are synthetic fixtures.

They do not assign production identities.

Digest definition for every vector:

```text
SHA-256(
  ASCII("SOAM:D8B:PROVENANCE-ENVELOPE:V1")
  || canonicalEnvelopeBytes
)
```

Domain separator hex:

`534f414d3a4438423a50524f56454e414e43452d454e56454c4f50453a5631`

Common schema fixture:

- schemaId = `00112233445566778899aabbccddeeff`
- schemaMajor = 1
- schemaMinor = 0
- canonicalEncodingVersion = 1

Common producer fixture:

- producerId = `102132435465768798a9bacbdcedfe0f`
- producerMajor = 1
- producerMinor = 0
- implementationRevisionKind = 0
- implementationRevision = 32 zero bytes

---

## V1 — Ordinary finite D8A-shaped source sample

Fields:

- provenanceItemId = `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
- sourceCaptureId = `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
- sourceNodeId = 0
- targetNodeId = 1
- relationshipGeneration = 7
- stateVersion = 11
- distance = 1.0
- orientationWeight = 0.5
- capacity = 1.0
- bridgeStatus = NORMAL / tag 0
- sourceState = 1.0
- targetState = 1.0
- sourceHealth = 1.0
- targetHealth = 1.0
- dependencies = empty
- SourceRecordReference = absent

Canonical length: **207 bytes**

Canonical hex:

```text
534f414d4438423100112233445566778899aabbccddeeff000100000001102132435465768798a9bacbdcedfe0f00010000000000000000000000000000000000000000000000000000000000000000000000aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb000000000000000000000000000000010000000000000007000000000000000b3ff00000000000003fe00000000000003ff0000000000000003ff00000000000003ff00000000000003ff00000000000003ff0000000000000000000
```

Expected SHA-256:

`3fb1f03828dadb48ddb23463b6d56e1679ef71731dc681fbebb920656a8fde70`

---

## V2 — Zero and negative-zero bit preservation + max BridgeStatus tag

Fields:

- provenanceItemId = `01010101010101010101010101010101`
- sourceCaptureId = `02020202020202020202020202020202`
- sourceNodeId = 0
- targetNodeId = 0
- relationshipGeneration = 0
- stateVersion = 0
- distance = +0.0
- orientationWeight = -0.0
- capacity = +0.0
- bridgeStatus = ISOLATED / tag 3
- sourceState = -0.0
- targetState = +0.0
- sourceHealth = +0.0
- targetHealth = -0.0
- dependencies = empty
- SourceRecordReference = absent

This vector is an encoding vector, not a claim that this relationship/state is
a valid D8A production capture.

Canonical length: **207 bytes**

Canonical hex:

```text
534f414d4438423100112233445566778899aabbccddeeff000100000001102132435465768798a9bacbdcedfe0f0001000000000000000000000000000000000000000000000000000000000000000000000001010101010101010101010101010101020202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000038000000000000000000000000000000000000000000000008000000000000000000000
```

Expected SHA-256:

`8de3c4b0f70de13830d892a7b59598ffe162972cb4d454edca02ed6b3981ed18`

This vector proves that `-0.0` and `+0.0` have different canonical bytes.

---

## V3 — Same source content, different SourceCaptureId

Same semantic source values as V1, except:

- sourceCaptureId = `cccccccccccccccccccccccccccccccc`

Canonical length: **207 bytes**

Canonical hex:

```text
534f414d4438423100112233445566778899aabbccddeeff000100000001102132435465768798a9bacbdcedfe0f00010000000000000000000000000000000000000000000000000000000000000000000000aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaacccccccccccccccccccccccccccccccc000000000000000000000000000000010000000000000007000000000000000b3ff00000000000003fe00000000000003ff0000000000000003ff00000000000003ff00000000000003ff00000000000003ff0000000000000000000
```

Expected SHA-256:

`ad283e8f6dcd10f7ff7544a85fa208c860259d0b328c17056cb19e2cbfcf6b99`

Required property:

`digest(V3) != digest(V1)`

even though source measurements and relationship/state values are equal.

---

## V4 — Same capture, different ProvenanceItemId

Same values as V1, except:

- provenanceItemId = `dddddddddddddddddddddddddddddddd`

Canonical length: **207 bytes**

Canonical hex:

```text
534f414d4438423100112233445566778899aabbccddeeff000100000001102132435465768798a9bacbdcedfe0f00010000000000000000000000000000000000000000000000000000000000000000000000ddddddddddddddddddddddddddddddddbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb000000000000000000000000000000010000000000000007000000000000000b3ff00000000000003fe00000000000003ff0000000000000003ff00000000000003ff00000000000003ff00000000000003ff0000000000000000000
```

Expected SHA-256:

`25328c3c1ffa7199c0c5a9d1ce05e2725ce4c58b7c2e498fc1326b2d63c9f7aa`

Required property:

`digest(V4) != digest(V1)`.

This distinguishes provenance artifact identity from source capture identity.

---

## V5 — Dependency canonical ordering

Same base values as V1.

The caller/input order for this vector is deliberately:

1. dependency kind 2;
2. dependency kind 1.

Dependency A input:

- kind = 2
- dependencyId = `22222222222222222222222222222222`
- version = 1.0
- revisionKind = 1
- revisionDigest = 32 bytes of `11`

Dependency B input:

- kind = 1
- dependencyId = `11111111111111111111111111111111`
- version = 2.3
- revisionKind = 0
- revisionDigest = 32 zero bytes

Canonical order MUST be B then A because complete encoded entries are sorted
lexicographically.

Canonical length: **315 bytes**

Canonical hex:

```text
534f414d4438423100112233445566778899aabbccddeeff000100000001102132435465768798a9bacbdcedfe0f00010000000000000000000000000000000000000000000000000000000000000000000000aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb000000000000000000000000000000010000000000000007000000000000000b3ff00000000000003fe00000000000003ff0000000000000003ff00000000000003ff00000000000003ff00000000000003ff0000000000000000201111111111111111111111111111111110002000300000000000000000000000000000000000000000000000000000000000000000002222222222222222222222222222222220001000001111111111111111111111111111111111111111111111111111111111111111100
```

Expected SHA-256:

`9b267dd7bc1c86745d1c99f5da9922837ac3eed0bd6ea70b6fbdb4665ca6d4ee`

Required property:

serializing the same two entries in input orders `[A,B]` and `[B,A]` MUST
produce these exact same canonical bytes and digest.

---

## Required conformance assertions

A future independent encoder must demonstrate all of the following:

```text
V1 length == 207
V2 length == 207
V3 length == 207
V4 length == 207
V5 length == 315

SHA256(V1 preimage) ==
3fb1f03828dadb48ddb23463b6d56e1679ef71731dc681fbebb920656a8fde70

SHA256(V2 preimage) ==
8de3c4b0f70de13830d892a7b59598ffe162972cb4d454edca02ed6b3981ed18

SHA256(V3 preimage) ==
ad283e8f6dcd10f7ff7544a85fa208c860259d0b328c17056cb19e2cbfcf6b99

SHA256(V4 preimage) ==
25328c3c1ffa7199c0c5a9d1ce05e2725ce4c58b7c2e498fc1326b2d63c9f7aa

SHA256(V5 preimage) ==
9b267dd7bc1c86745d1c99f5da9922837ac3eed0bd6ea70b6fbdb4665ca6d4ee
```

These values are specification fixtures.

They must not be regenerated from the production implementation and then used
as evidence that the same implementation is correct.

---

## Deferred vectors

The following are intentionally deferred until their upstream semantics are
accepted:

- production `SourceCaptureId` generation;
- production `ProvenanceItemId` generation;
- actual schema/producer registry IDs;
- persistent `SourceRecordReference`;
- authenticated-integrity/signature material;
- NaN/infinity vectors if D8A makes such values unreachable.
