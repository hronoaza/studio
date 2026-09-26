# D8B Supporting Amendment for D8C — ProvenanceMetadataView

## Status

- Purpose: typed immutable metadata access for D8C
- Layer ownership: D8B
- Canonical byte-layout change: none
- Runtime implementation: none
- Acceptance status: candidate

---

## 1. Goal

Expose metadata already owned by accepted D8B in typed immutable form without
forcing D8C to implement a second CanonicalEnvelopeV1 parser.

Candidate type:

`ProvenanceMetadataView`

This is an API visibility amendment only.

It does not add admissibility semantics.

---

## 2. Contents

Candidate immutable view:

```text
ProvenanceMetadataView
├── ProvenanceSchemaDescriptor
│   ├── schemaId
│   ├── schemaMajor
│   ├── schemaMinor
│   └── canonicalEncodingVersion
├── ProducerDescriptor
│   ├── producerId
│   ├── producerMajor
│   ├── producerMinor
│   ├── implementationRevisionKind
│   └── implementationRevision
└── DependencyManifestView
    └── zero or more immutable DependencyDescriptor values
```

Dependency descriptor:

```text
DependencyDescriptor
- kind
- dependencyId
- versionMajor
- versionMinor
- revisionKind
- revisionDigest
```

---

## 3. Single semantic owner invariant

D8B must not independently construct the typed view and canonical bytes.

Required construction:

```text
one immutable structured envelope input
-> typed envelope metadata/source fields
-> CanonicalEnvelopeV1 bytes
-> CanonicalDigest
```

Therefore:

```text
metadataView fields
== fields encoded into canonical bytes
```

by construction.

The metadata view is not parsed by D8C from raw bytes.

---

## 4. Public API candidate

Conceptual shape:

```cpp
class ProvenanceMetadataView final {
public:
    // read-only accessors only
};

class ProductionProvenanceEnvelope final {
public:
    [[nodiscard]]
    const ProvenanceMetadataView& metadata() const noexcept;
};
```

The exact C++ decomposition remains implementation-design work.

No public constructor from arbitrary metadata is added.

---

## 5. Restricted-origin requirements

Ordinary callers cannot:

- construct a trusted metadata view from arbitrary fields;
- replace schema identity;
- replace producer identity;
- replace implementation revision;
- insert/remove dependency entries;
- attach a metadata view to another envelope.

Copy/move of an existing envelope preserves the same metadata.

---

## 6. Canonical consistency verification

D8B amendment tests must prove at minimum:

1. metadata schema fields equal the canonical byte fields;
2. producer fields equal canonical byte fields;
3. implementation revision equals canonical byte field;
4. dependency count equals canonical byte count;
5. each typed dependency corresponds exactly to its canonical encoded entry;
6. dependency canonical ordering does not change metadata meaning;
7. no post-construction mutation is possible;
8. changing structured metadata changes canonical bytes/digest.

D8C must still verify the envelope's CanonicalDigest before a positive result.

---

## 7. Lifetime

The view should be owned by the immutable
`ProductionProvenanceEnvelope` or reference storage with exactly the envelope's
lifetime.

It must not refer to producer temporaries.

---

## 8. Non-goals

This amendment does not establish:

- schema compatibility;
- producer trust;
- dependency admissibility;
- source re-derivability;
- authentication;
- interpretation;
- authority.

Those remain D8C/later concerns.

---

## 9. Acceptance gate

Before D8C runtime implementation:

- exact C++ representation must be reviewed;
- D8B conformance tests must be extended;
- D8B regression CI must pass;
- no CanonicalEnvelopeV1 wire-format change may occur implicitly.
