# CanonicalEnvelopeV1 — Normative Byte Layout Candidate

## Status

- Layer: D8B
- Artifact class: wire-format / canonicalization candidate
- Runtime implementation: none
- Current Baseline effect: none
- Parent contract: `DESIGN_CONTRACT.md`
- Review prerequisite: `DESIGN_REVIEW_1.md`
- Acceptance status: candidate

This document defines the exact candidate byte representation used to compute
the D8B `CanonicalDigest`.

It is intentionally independent of native C++ object layout.

---

## 1. Digest definition

The candidate digest is:

```text
SHA-256(
  DOMAIN_SEPARATOR_BYTES
  || CANONICAL_ENVELOPE_V1_BYTES
)
```

Exact domain separator:

```text
ASCII: SOAM:D8B:PROVENANCE-ENVELOPE:V1
HEX:   534f414d3a4438423a50524f56454e414e43452d454e56454c4f50453a5631
LEN:   31 bytes
```

No terminating NUL byte is included.

The digest field itself is not part of
`CANONICAL_ENVELOPE_V1_BYTES`.

---

## 2. Primitive encoding

All integer values are unsigned and big-endian unless stated otherwise.

| Primitive | Encoding |
|---|---|
| `u8` | 1 byte |
| `u16` | 2 bytes, big-endian |
| `u64` | 8 bytes, big-endian |
| `id128` | exactly 16 opaque bytes |
| `digest256` | exactly 32 bytes |
| `f64bits` | exact IEEE-754 binary64 bits, serialized most-significant byte first |

No native `sizeof`, padding, alignment or host endianness participates.

### 2.1 C++ `std::size_t` boundary

D8A currently exposes source/target node IDs as `std::size_t`.

The wire representation is always `u64`.

Encoding must fail rather than truncate if a future implementation can represent
a node ID outside the `u64` range.

### 2.2 Floating-point boundary

The encoder preserves the exact binary64 bit pattern supplied by the accepted
D8A snapshot.

It does not normalize:

- positive/negative zero;
- NaN payload bits;
- infinity.

D8C may reject values that violate accepted runtime/source invariants.

---

## 3. Fixed wire tags

### 3.1 BridgeStatus

Wire tags are schema-owned and do not depend on the C++ enum's native
underlying representation.

| Runtime semantic value | Wire tag |
|---|---:|
| `NORMAL` | 0 |
| `DAMPING` | 1 |
| `RECOVERY` | 2 |
| `ISOLATED` | 3 |

Any other tag is invalid under CanonicalEnvelopeV1.

### 3.2 Revision kind

| Meaning | Wire tag |
|---|---:|
| no revision digest | 0 |
| SHA-256 revision digest | 1 |

If revision kind is 0, the associated 32-byte revision field MUST contain all
zero bytes.

---

## 4. Fixed envelope prefix

The envelope begins with this fixed magic:

```text
ASCII: SOAMD8B1
HEX:   534f414d44384231
LEN:   8 bytes
```

The magic is part of canonical bytes.

The external digest domain separator remains separate.

---

## 5. Exact field order

For zero dependencies, the canonical envelope is 207 bytes.

| Offset | Size | Field | Encoding |
|---:|---:|---|---|
| 0 | 8 | magic | exact bytes `SOAMD8B1` |
| 8 | 16 | schemaId | `id128` |
| 24 | 2 | schemaMajor | `u16` |
| 26 | 2 | schemaMinor | `u16` |
| 28 | 2 | canonicalEncodingVersion | `u16` |
| 30 | 16 | producerId | `id128` |
| 46 | 2 | producerMajor | `u16` |
| 48 | 2 | producerMinor | `u16` |
| 50 | 1 | implementationRevisionKind | `u8` |
| 51 | 32 | implementationRevision | `digest256` |
| 83 | 16 | provenanceItemId | `id128` |
| 99 | 16 | sourceCaptureId | `id128` |
| 115 | 8 | sourceNodeId | `u64` |
| 123 | 8 | targetNodeId | `u64` |
| 131 | 8 | relationshipGeneration | `u64` |
| 139 | 8 | stateVersion | `u64` |
| 147 | 8 | distance | `f64bits` |
| 155 | 8 | orientationWeight | `f64bits` |
| 163 | 8 | capacity | `f64bits` |
| 171 | 1 | bridgeStatus | wire tag |
| 172 | 8 | sourceState | `f64bits` |
| 180 | 8 | targetState | `f64bits` |
| 188 | 8 | sourceHealth | `f64bits` |
| 196 | 8 | targetHealth | `f64bits` |
| 204 | 2 | dependencyCount | `u16` |
| 206 | variable | dependencies | zero or more 54-byte entries |
| after dependencies | 1 | sourceRecordReferencePresent | `u8`, MUST be 0 in v1 |

No semantic field appears twice.

---

## 6. Dependency entry

Each dependency entry is exactly 54 bytes.

| Relative offset | Size | Field | Encoding |
|---:|---:|---|---|
| 0 | 1 | dependencyKind | `u8` |
| 1 | 16 | dependencyId | `id128` |
| 17 | 2 | versionMajor | `u16` |
| 19 | 2 | versionMinor | `u16` |
| 21 | 1 | revisionKind | `u8` |
| 22 | 32 | revisionDigest | `digest256` |

### 6.1 Dependency ordering

Before serialization, entries MUST be sorted by lexicographic comparison of the
complete 54-byte encoded entry.

Duplicate encoded entries are invalid.

This makes dependency order independent of insertion/container order.

### 6.2 Dependency kinds

The numeric registry of dependency kinds is deliberately not accepted by this
document.

Normative conformance vectors may use synthetic kind values.

Production kind values require a separately reviewed registry.

---

## 7. SourceRecordReference boundary

CanonicalEnvelopeV1 encodes one presence byte after the dependency list.

For v1:

```text
sourceRecordReferencePresent = 0
```

is the only valid value.

No SourceRecordReference payload follows.

A future persistent source-record format must use a new schema/encoding version
rather than silently reinterpret v1 bytes.

---

## 8. ID widths versus generation algorithms

`schemaId`, `producerId`, `provenanceItemId` and
`sourceCaptureId` are 16-byte opaque wire values.

This document fixes their representation width, not their generation
algorithms.

In particular it does not yet choose between:

- random 128-bit identities;
- composite producer-instance/sequence identities;
- another reviewed 128-bit construction.

Uniqueness-domain semantics remain an acceptance gate in the parent contract.

---

## 9. Schema and producer registries

The wire representation fixes field widths but does not assign production
registry identities in this document.

The conformance vectors use synthetic IDs solely to make the byte specification
independently testable.

Production schema/producer IDs must be fixed by a separate reviewed registry
before implementation acceptance.

---

## 10. Decoder rule

A parser for these bytes, if one is later introduced, produces an untrusted
decoded representation.

```text
well_formed_bytes != trusted ProductionProvenanceEnvelope
```

Parsing cannot recreate the restricted-origin D8A capture boundary.

---

## 11. Canonical validation algorithm

Conceptually:

```text
1. Validate representability of all fixed-width fields.
2. Map BridgeStatus to its schema wire tag.
3. Encode all fixed fields in the exact order above.
4. Encode each dependency entry to 54 bytes.
5. Reject duplicate dependency entries.
6. Sort dependency encodings lexicographically.
7. Encode dependencyCount.
8. Append sorted dependency bytes.
9. Append SourceRecordReference presence byte 0.
10. Compute SHA-256(domainSeparator || canonicalBytes).
```

No serialization framework defaults are normative.

---

## 12. Current D8A correspondence

The reviewed D8A candidate supplies:

- source/target IDs as `std::size_t`;
- relationship generation as `std::uint64_t`;
- state version as `std::uint64_t`;
- distance/orientation/capacity as `double`;
- `BridgeStatus`;
- source/target state as `double`;
- source/target health as `double`.

The D8A amendment proposed by the D8B design additionally supplies a
restricted-origin `SourceCaptureId`.

This wire contract does not modify PR #8.

---

## 13. Conformance

An implementation conforms to this candidate layout only if it reproduces the
exact bytes and SHA-256 values in
`CANONICAL_ENVELOPE_V1_TEST_VECTORS.md`.

The implementation under test must not generate its own oracle vectors.
