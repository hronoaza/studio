# SOAM 2.0 D8B — Provenance Envelope Design Contract

## Status

- Layer: D8B — Provenance Envelope
- Document class: design-contract candidate
- Executable implementation: none
- Current Baseline effect: none
- Acceptance status: baseline-aligned / acceptance review pending
- Upstream dependency: accepted D8A Source Capture in Current Baseline
  (`379cd2e2cf17eb7181629945e670f7923ba662ce`, merged from PR #8)
- Downstream dependency: future D8C Provenance Admissibility
- Authority effect: none
- Incorporated review: `DESIGN_REVIEW_1.md`

This document defines the candidate contract for converting one restricted-origin
D8A source snapshot into one immutable provenance-bearing envelope.

It deliberately does **not** implement provenance admissibility,
interpretation, transition eligibility, authority, capability issuance or
commit.

---

## 1. Boundary

D8B begins with exactly one restricted-origin D8A value:

`ProductionRelationshipSourceSnapshot`

and ends with exactly one restricted-origin provenance envelope:

`ProductionProvenanceEnvelope`

Conceptual chain:

```text
ProductionRelationshipLocator
-> D8A ProductionRelationshipSourceSnapshot
   + SourceCaptureId
-> D8B ProductionProvenanceEnvelope
-> STOP
```

D8B MUST NOT produce:

- `InteractionObservation`;
- `BridgeConfidence`;
- `BridgePolicyEvidence`;
- `RequestedTransitionDirection`;
- `eligible_for_authority_consideration`;
- execution capability;
- production commit.

A D8B envelope is provenance-bearing evidence, not admissible evidence.

---

## 2. Identity model

D8B must keep the following identities distinct.

### 2.1 Relationship address

`ProductionRelationshipLocator`

Meaning:

> which directed relationship was requested?

This is descriptive addressing only.

### 2.2 Relationship incarnation

Candidate tuple:

`(sourceNodeId, targetNodeId, relationshipGeneration)`

Meaning:

> which concrete lifetime/incarnation of that relationship supplied the facts?

A recreated relationship with the same endpoint IDs is a different incarnation.

### 2.3 Source capture identity

Candidate type:

`SourceCaptureId`

Meaning:

> which coherent D8A capture event produced this immutable source snapshot?

Required ownership:

- minted at the D8A capture boundary;
- not caller-selectable;
- immutable;
- preserved unchanged by D8B;
- not inferred from payload equality;
- not re-minted when the same snapshot is passed to D8B again.

This identity belongs semantically to source capture, not provenance wrapping.

### 2.4 Provenance item identity

Candidate type:

`ProvenanceItemId`

Meaning:

> which immutable D8B provenance artifact is this?

Required properties:

- minted by the D8B producer;
- not caller-selectable;
- immutable;
- distinct from `SourceCaptureId`;
- distinct from the canonical digest.

One source capture may eventually produce more than one typed provenance item.
Therefore source capture identity and provenance item identity must not be
collapsed.

### 2.5 Canonical digest

Candidate type:

`CanonicalDigest`

Meaning:

> deterministic digest of the canonical D8B envelope preimage.

It is not an identity substitute.

---

## 3. Principal source-coherence rule

The principal D8B invariant is:

```text
one D8B envelope
-> one SourceCaptureId
-> one coherent D8A source snapshot
```

D8B must not accept separately supplied:

- observation;
- confidence;
- bridge recommendation;
- cached policy evidence;
- relationship generation;
- state version;
- endpoint state;
- bridge state

when those facts are already owned by the D8A snapshot.

All raw relationship facts used by one envelope must come from the same
restricted-origin D8A snapshot and therefore share one `SourceCaptureId`.

---

## 4. Anti-mixing invariant

For any downstream derivation that claims to represent one source sample:

```text
all contributing facts must share the same SourceCaptureId
```

D8D must not be able to combine:

`Observation(capture A)`

with:

`Confidence(capture B)`

and present the result as one coherent sample.

If a future operation intentionally aggregates multiple source captures, it must
use a separate aggregate type that explicitly carries:

- all contributing `SourceCaptureId` values;
- aggregation policy ID/version;
- ordering semantics where relevant;
- aggregate derivation identity.

Multi-capture aggregation is outside D8B v1.

---

## 5. Candidate envelope shape

Conceptual structure only:

```text
ProductionProvenanceEnvelope
├── EnvelopeHeader
│   ├── ProvenanceItemId
│   ├── ProducerDescriptor
│   ├── ProvenanceSchemaDescriptor
│   └── CanonicalEncodingVersion
├── SourceBinding
│   ├── SourceCaptureId
│   ├── sourceNodeId
│   ├── targetNodeId
│   ├── relationshipGeneration
│   └── stateVersion
├── SourceMeasurements
│   ├── distance
│   ├── orientationWeight
│   ├── capacity
│   ├── BridgeStatus
│   ├── sourceState
│   ├── sourceHealth
│   ├── targetState
│   └── targetHealth
├── DependencyManifest
├── optional SourceRecordReference
└── CanonicalDigest
```

Every semantic field has exactly one canonical owner.

The exact C++ spelling is not accepted by this document.

---

## 6. Source measurements

D8B preserves the raw facts exposed by the accepted D8A contract.

Candidate measurements:

- distance;
- orientation weight;
- capacity;
- bridge status;
- source endpoint state;
- source endpoint health;
- target endpoint state;
- target endpoint health.

Relationship IDs, relationship generation and state version belong to
`SourceBinding` and are not duplicated inside `SourceMeasurements`.

D8B must preserve these as source facts.

In particular, D8B must not interpret `BridgeStatus` as:

- requested direction;
- compatibility;
- confidence;
- permission;
- authority.

---

## 7. Source binding

`SourceBinding` is mandatory.

It binds the envelope to the exact capture context.

Candidate fields:

- `SourceCaptureId`;
- source node ID;
- target node ID;
- relationship generation;
- runtime state version.

Required distinction:

```text
source_binding_present
!= source_record_retrievable
!= source_rederivable
```

`SourceBinding` does not claim that an independently retrievable source record
exists.

---

## 8. Source record reference

`SourceRecordReference` is optional/future-facing.

It is present only if the system actually persists a source capture record that
can be independently located.

Candidate fields may include:

- source-record ID;
- source-record digest;
- storage namespace/object ID;
- retrieval/version metadata.

D8B v1 must distinguish:

### Canonical reproducibility

Can the envelope canonical bytes and digest be recomputed from the envelope?

### Source re-derivability

Can the original source material be independently reconstructed or retrieved?

The first is a D8B property.

The second is a D8C admissibility question and may fail even when the canonical
digest is valid.

---

## 9. Canonical encoding

D8B requires one versioned canonical binary representation.

Canonical bytes must never depend on:

- in-memory struct layout;
- padding bytes;
- compiler ABI;
- native endianness;
- `std::hash`;
- locale-dependent number formatting;
- JSON key iteration order;
- unspecified enum representation.

### 9.1 CanonicalEnvelopeV1

The canonical preimage contains each semantic field exactly once:

```text
CanonicalEnvelopeV1
├── EnvelopeHeader
├── SourceBinding
├── SourceMeasurements
├── DependencyManifest
└── optional SourceRecordReference
```

The `CanonicalDigest` field itself is excluded from the preimage.

### 9.2 Candidate encoding rules

- fixed schema-controlled field order;
- fixed-width unsigned integer primitives;
- integers encoded big-endian;
- enum values mapped to schema-defined fixed-width integer tags;
- floating-point source values encoded as IEEE-754 binary64 bit patterns;
- no locale or text conversion for numeric fields;
- variable-length byte/string fields use one explicitly defined length prefix;
- list count and list ordering are explicit;
- optional field presence is explicit;
- exact domain-separator bytes are normative.

### 9.3 Floating-point boundary

D8B does not silently normalize:

- negative zero;
- infinities;
- NaN payloads.

If D8A guarantees such values cannot occur, the exact canonical layout must
state that as an upstream invariant and omit unreachable vectors.

Otherwise D8B preserves the captured bit pattern and D8C decides admissibility.

---

## 10. Schema identity

Candidate descriptor:

```text
ProvenanceSchemaDescriptor
- schemaId
- majorVersion
- minorVersion
```

Canonical encoding version is a normative field of the envelope header.

The schema identity defines:

- field set;
- field order;
- primitive encodings;
- enum tags;
- required/optional fields;
- canonicalization rules.

A schema change that changes canonical bytes must be explicit.

D8B does not decide whether a schema is acceptable for current production.
That is D8C.

---

## 11. Producer identity

Candidate descriptor:

```text
ProducerDescriptor
- producerId
- producerVersion
- implementationRevision
```

Producer identity states which D8B implementation claims to have created the
envelope.

It is not, by itself, authentication.

D8B v1 does not claim that a textual producer ID proves origin.

Future authentication may bind producer identity to a signature, MAC, key,
attestation or external trust anchor, but that belongs to an independently
reviewed contract.

D8C may reject unknown or untrusted producers.

---

## 12. Dependency manifest

Candidate type:

`DependencyManifest`

The manifest lists materially participating dependencies not already represented
by normative header fields.

Candidate dependency kinds:

- source-capture contract;
- canonicalization implementation/specification;
- content-digest algorithm implementation/specification;
- additional transformation/runtime contract if one materially participated.

The manifest must not duplicate normative producer/schema identity.

Each dependency reference should contain:

- dependency kind;
- stable ID;
- version/revision;
- optional content digest where meaningful.

Canonical manifest ordering must be defined by the encoding specification.

Whether a dependency is current, retired or prohibited belongs to D8C.

---

## 13. Canonical digest

D8B v1 candidate algorithm:

`SHA-256`

Candidate definition:

```text
CanonicalDigest =
    SHA-256(
        domain_separator
        || canonical_encode(CanonicalEnvelopeV1)
    )
```

Candidate domain separator:

`SOAM:D8B:PROVENANCE-ENVELOPE:V1`

The exact byte sequence of the separator is part of the canonical-layout
specification.

### 13.1 Security boundary

A matching canonical digest establishes deterministic equality of the canonical
bytes relative to an expected digest or recomputation.

It does not establish:

- authenticated producer origin;
- tamper resistance against an active rewriter who can recompute the digest;
- source truth;
- admissibility;
- freshness;
- authorization;
- absence of malicious input.

Required distinction:

```text
canonical_digest_match
!= authenticated_origin
!= trusted_integrity
!= source_truth
```

Stronger authenticated-integrity semantics require a separate trust anchor and
are outside D8B v1.

---

## 14. Construction authority

`ProductionProvenanceEnvelope` must be restricted-origin.

Candidate rules:

- no public aggregate initialization;
- no public constructor accepting arbitrary raw fields;
- no public constructor accepting arbitrary canonical bytes;
- no public constructor accepting an arbitrary digest;
- caller cannot supply `ProvenanceItemId`;
- caller cannot replace `SourceCaptureId`;
- caller cannot claim a producer identity.

Only the accepted D8B producer/factory may construct the trusted envelope from
an authentic restricted-origin D8A source snapshot.

A forged-envelope compile-fail gate is required before acceptance.

---

## 15. Decode boundary

D8B v1 should not expose a public decoder that turns arbitrary bytes directly
into a trusted `ProductionProvenanceEnvelope`.

Preferred v1 design:

- canonical encoding exists for deterministic serialization/test vectors;
- trusted envelope construction remains snapshot-origin only;
- arbitrary external bytes do not become trusted provenance merely by parsing.

If persistent decoding is later required, bytes must decode into a separate
untrusted type, for example:

`DecodedProvenanceEnvelope`

which cannot be consumed as trusted D8B provenance until validation restores all
required construction invariants.

Therefore:

```text
parse_success != trusted_origin
```

---

## 16. Publication model

D8B construction should follow:

```text
restricted-origin D8A snapshot
-> read preserved SourceCaptureId
-> mint ProvenanceItemId
-> prepare detached header/binding/measurements/manifest
-> canonical encode
-> compute CanonicalDigest
-> validate internal construction invariants
-> publish immutable envelope
```

Potentially throwing work must happen before authoritative envelope publication.

D8B publication must not mutate:

- live mesh topology;
- relationship lifecycle;
- bridge persistence;
- authority ledger;
- production state.

Envelope publication is evidence creation, not production-state commit.

---

## 17. Identity generation domain

### 17.1 SourceCaptureId

Ownership: accepted D8A Current Baseline.

Accepted upstream properties:

- 128-bit / 16 opaque bytes;
- generated at D8A source-capture boundary;
- OS-backed CSPRNG;
- all-zero reserved invalid;
- finite fail-closed retry;
- repeated successful recaptures receive distinct IDs;
- copy/move preserves capture identity;
- not a capability, authentication token, digest or provenance-item identity.

D8B preserves the accepted 16 bytes exactly and does not remint or reinterpret
them.

### 17.2 ProvenanceItemId

Ownership: D8B.

Before implementation, one of the following families must be selected and
documented:

- persistent monotonic sequence scoped by stable producer instance;
- random 128-bit identity with explicit collision assumptions;
- composite stable producer-instance identity plus monotonic sequence.

Required property:

> an accepted provenance item identity is not silently reused across restart,
> concurrent producer instances or version transitions within its declared
> uniqueness domain.

The exact identity algorithm is not accepted by this contract.

---

## 18. Immutability

After successful construction:

- envelope header is immutable;
- source binding is immutable;
- source measurements are immutable;
- dependency manifest is immutable;
- optional source-record reference is immutable;
- canonical digest is immutable.

Corrections or superseding evidence create a new provenance item.

They do not rewrite the old item.

This supports future append-only evidence-ledger semantics.

---

## 19. Equality semantics

D8B must not collapse all equality into one operator.

At least four comparisons are conceptually distinct.

### Same provenance item

`ProvenanceItemId equal`

### Same source capture

`SourceCaptureId equal`

### Same relationship incarnation

`source/target/generation equal`

### Same canonical content

`canonical bytes equal`

Two provenance items may contain identical source measurements while
representing different source captures.

A digest must not substitute for every identity relation.

---

## 20. Failure semantics

D8B construction must fail closed.

Candidate failure categories:

- unavailable/invalid source snapshot;
- invalid source binding;
- unsupported schema;
- provenance-item identity allocation failure;
- canonicalization failure;
- dependency-manifest construction failure;
- digest computation failure;
- publication failure.

On failure:

- no partial envelope is observable;
- no failed `ProvenanceItemId` is later reused as if publication succeeded;
- the preserved `SourceCaptureId` remains the identity of the original D8A
  capture and is not reminted;
- no downstream admissibility/interpretation is possible;
- live production state remains unchanged.

Exact error representation is deferred to implementation design.

---

## 21. D8B does not establish admissibility

A correctly formed D8B envelope may still be rejected by D8C.

Examples:

- producer unknown;
- schema obsolete;
- dependency retired;
- source-record reference unavailable;
- source cannot be re-derived;
- lineage stale;
- canonical digest mismatch after storage/transport;
- required external authenticity evidence absent.

Therefore:

```text
well_formed_envelope != admissible_provenance
canonical_digest_valid != authenticated_integrity
canonical_digest_valid != source_true
```

---

## 22. D8B does not establish interpretation

D8B must not calculate or store semantic fields such as:

- compatibility score;
- observation score;
- confidence score;
- requested direction;
- recommendation;
- authority decision.

Therefore:

`provenance != interpretation`

D8D owns versioned interpretation after D8C admissibility.

---

## 23. Candidate API shape

Non-binding sketch:

```cpp
class ProductionProvenanceEnvelope;

class ProductionProvenanceEnvelopeProducer final {
public:
    ProductionProvenanceEnvelope produce(
        const ProductionRelationshipSourceSnapshot& snapshot);
};
```

The accepted implementation may instead use a free function, factory or internal
owner.

The architectural requirements are:

- restricted-origin input;
- restricted-origin output;
- D8A-owned source capture identity preserved exactly;
- D8B-owned provenance item identity;
- no caller-supplied digest;
- no interpretation;
- deterministic canonical representation.

---

## 24. Verification plan

D8B implementation acceptance requires at least the following tests.

### Positive

1. one valid D8A snapshot produces one envelope;
2. `SourceCaptureId` is preserved exactly;
3. relationship generation/state version are preserved exactly;
4. repeated canonical encoding is byte-identical;
5. repeated digest computation is identical;
6. same source capture re-enveloped as a new provenance item preserves
   `SourceCaptureId` while receiving a distinct `ProvenanceItemId`.

### Negative

7. public caller cannot construct a forged envelope;
8. public caller cannot select `ProvenanceItemId`;
9. public caller cannot replace `SourceCaptureId`;
10. public caller cannot inject an arbitrary digest;
11. arbitrary decoded bytes cannot directly become trusted provenance;
12. changing any canonical semantic field changes the canonical digest;
13. changing relationship generation changes the canonical digest;
14. changing state version changes the canonical digest;
15. changing `SourceCaptureId` changes the canonical digest;
16. changing producer/schema/dependency metadata changes the canonical digest;
17. mixed-capture single-sample derivation is rejected by type/API design;
18. D8B cannot manufacture `InteractionObservation` or `BridgeConfidence`.

### Failure / exception

19. provenance-item identity allocation failure publishes no envelope;
20. canonicalization failure publishes no envelope;
21. manifest-construction failure publishes no envelope;
22. digest failure publishes no envelope;
23. failed production leaves live runtime state unchanged.

### Portability / specification

24. canonical test vectors have exact expected bytes;
25. canonical digest vectors have exact expected SHA-256;
26. the same vectors match across supported compilers/platforms;
27. public D8B declarations are configuration-invariant.

---

## 25. Normative test vectors

The exact canonical bytes must be defined before the C++ producer implementation
is treated as conforming.

Each vector must include:

- human-readable field values;
- exact expected canonical byte sequence;
- exact expected SHA-256;
- schema identity/version;
- canonical encoding version;
- producer descriptor;
- `SourceCaptureId`;
- `ProvenanceItemId`.

Minimum vector classes:

1. ordinary finite positive values;
2. zero values;
3. negative zero if D8A can expose it;
4. min/max enum tags;
5. relationship-generation/state-version boundary values;
6. repeated source content with different `SourceCaptureId`;
7. same `SourceCaptureId` with different `ProvenanceItemId`;
8. dependency-manifest canonical ordering.

The vectors must not be generated dynamically by the implementation under test
and then treated as an independent oracle.

---

## 26. Concurrency/lifetime boundary

D8B v1 constructs from an already captured immutable D8A snapshot.

It must not hold the mesh topology lock while:

- allocating envelope storage;
- serializing canonical bytes;
- building manifests;
- computing digests;
- performing external callbacks.

No external provider callback is permitted while a live mesh topology lock is
held.

The D8A snapshot is the lock-boundary handoff.

---

## 27. Accepted D8A upstream boundary

The required upstream amendment is now accepted in Current Baseline at:

`379cd2e2cf17eb7181629945e670f7923ba662ce`

D8A provides a restricted-origin `SourceCaptureId` minted at the coherent
capture boundary.

D8A remains:

```text
relationship locator
-> coherent source capture
-> immutable raw source facts + SourceCaptureId
-> STOP
```

D8B therefore consumes the accepted D8A identity directly. It must not invent a
D8B-side acquisition identity.

---

## 28. Relationship to the independent audit evidence

The older audited implementation identified a provenance gap: already-composed
policy evidence could be supplied without acquisition/sample identity, allowing
observation/confidence from different semantic samples to be combined before a
later CAS check.

This design addresses that class of defect structurally by:

- assigning the sample identity at D8A capture;
- preserving it through D8B;
- prohibiting caller-composed policy evidence at D8B;
- requiring downstream one-sample derivations to retain the same capture ID.

This document does not claim the older implementation satisfies this contract.

---

## 29. Non-goals

D8B v1 explicitly excludes:

- digital signatures;
- MAC/key management;
- remote attestation;
- trust-store governance;
- authenticated integrity;
- schema-compatibility policy;
- retired-dependency policy;
- source-truth verification;
- semantic interpretation;
- confidence calibration;
- transition eligibility;
- authority;
- capability issuance;
- execution;
- rollback;
- production commit.

These exclusions prevent D8B from becoming a monolithic provenance layer.

---

## 30. Acceptance gates

Before D8B implementation begins:

1. this revised D8B contract must be accepted;
2. `ProvenanceItemId` generation/uniqueness domain must be selected;
3. exact canonical byte layout must be accepted;
4. SHA-256/domain-separator bytes must be accepted;
5. source-binding/source-record-reference semantics must be accepted;
6. normative test vectors must be accepted;
7. production schema/producer/dependency registries must be fixed;
8. restricted-origin C++ API and compile-fail misuse design must be accepted.

The former D8A acceptance and capture-identity gates are satisfied by Current
Baseline `379cd2e2cf17eb7181629945e670f7923ba662ce`.

Before D8B merges into Current Baseline:

1. implementation matches the accepted contract;
2. forged-envelope compile-fail gates pass;
3. canonical vectors pass;
4. digest mutation tests pass;
5. failure atomicity tests pass;
6. sanitizer/concurrency validation passes where applicable;
7. independent audit is completed;
8. explicit Root Operator acceptance is recorded.

---

## 31. Candidate invariants summary

```text
one source capture -> one SourceCaptureId

one D8B envelope -> one SourceCaptureId
                 -> one ProvenanceItemId

SourceCaptureId is minted by D8A and preserved by D8B

relationship address != relationship incarnation
relationship incarnation != source capture identity
source capture identity != provenance item identity
identity != canonical digest

every canonical semantic field has exactly one serialized owner

CanonicalDigest binds canonical bytes
but does not authenticate their producer

SourceBinding identifies capture context
but does not imply source-record retrievability

well_formed_envelope != admissible_provenance
canonical_digest_valid != authenticated_integrity
canonical_digest_valid != source_true
provenance != interpretation

all potentially throwing preparation occurs before envelope publication

failed D8B construction publishes no partial envelope
and mutates no live production state
```

---

## 32. Next gate

Do not implement D8B yet.

Next design artifact:

1. exact `CanonicalEnvelopeV1` byte layout;
2. fixed normative conformance vectors;
3. explicit `SourceCaptureId` / `ProvenanceItemId` uniqueness-domain decision;
4. only after those are accepted, restricted-origin C++ API design and
   compile-fail misuse tests.
