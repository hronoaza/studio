# Pre-Acceptance Audit — SOAM 2.0 D8B Provenance Envelope

## Repository state

- Current Baseline: `379cd2e2cf17eb7181629945e670f7923ba662ce`
- Candidate branch: `candidate/soam-2.0-d8b-provenance-envelope-runtime`
- Exact validated implementation/test head: `4b3db739b6e5315953f7c07ab901ddaf0c8efd13`
- PR: #14
- Mergeable at audit: yes
- Branch relation to main at audit: ahead 17 / behind 0
- Acceptance status: `acceptance-pending`

## Architecture boundary

Implemented chain:

```text
accepted D8A ProductionRelationshipSourceSnapshot
+ accepted SourceCaptureId
-> D8B ProductionProvenanceEnvelopeProducer
-> immutable ProductionProvenanceEnvelope
-> STOP
```

D8B does not perform D8C admissibility or D8D interpretation.

## Identity model

The implementation preserves the accepted D8A `SourceCaptureId` exactly.

D8B independently mints a restricted-origin `ProvenanceItemId`.

The identities remain distinct from:

- relationship address;
- relationship generation;
- state version;
- canonical digest;
- authority/capability.

Repeated envelope production from the same immutable D8A snapshot therefore
produces:

```text
same SourceCaptureId
different ProvenanceItemId
```

This matches the accepted design.

## Restricted-origin boundary

Ordinary public callers cannot:

- default-construct `ProvenanceItemId`;
- construct trusted `ProvenanceItemId` from arbitrary bytes;
- default-construct `ProductionProvenanceEnvelope`;
- construct `CanonicalDigest` from arbitrary bytes;
- mutate an envelope's provenance-item identity.

Envelope construction is owned by
`ProductionProvenanceEnvelopeProducer`.

The ordinary producer API accepts only an accepted D8A snapshot.

## Canonical representation

The implementation encodes the accepted `CanonicalEnvelopeV1`:

- fixed magic;
- big-endian fixed-width integers;
- node IDs normalized to u64;
- binary64 floating-point bit patterns;
- schema-owned BridgeStatus tags;
- canonical dependency sorting;
- duplicate dependency rejection;
- v1 SourceRecordReference presence byte fixed absent;
- canonical digest excluded from its own preimage.

The implementation reproduces the normative fixed vectors.

## Digest semantics

The implementation computes:

```text
SHA-256(
  "SOAM:D8B:PROVENANCE-ENVELOPE:V1"
  || canonicalEnvelopeBytes
)
```

This is a `CanonicalDigest`, not authenticated integrity.

The audit does not interpret digest equality as proof of producer origin,
source truth or admissibility.

## Producer/schema/dependency registry

The implementation uses fixed 128-bit constants for:

- schema family;
- producer family;
- source-capture-contract dependency;
- canonical-encoding dependency;
- digest-profile dependency.

The dependency manifest uses v1 tags:

- 1 = SOURCE_CAPTURE_CONTRACT;
- 2 = CANONICAL_ENCODING_SPEC;
- 3 = DIGEST_PROFILE.

No schema/producer identity is duplicated into the dependency manifest.

## Implementation revision

The candidate now sets `implementationRevisionKind = 1`.

Its 32-byte revision is reproducibly derived from a deterministic manifest over
the D8B public header, internal canonicalization contract and implementation
source.

This binds envelope metadata to the reviewed implementation source surface
without coupling it to documentation-only Git commits or build-directory paths.

It still does not authenticate producer origin.

## Failure atomicity

The producer prepares identity, manifest, canonical bytes and digest before
publishing the immutable envelope.

Test seams demonstrate fail-closed behavior for:

- item-ID generation failure;
- all-zero/duplicate/retry exhaustion;
- canonicalization failure;
- digest failure;
- dependency-manifest failure.

No partial envelope is returned.

Observed mesh state and health remain unchanged through D8B failure cases.

## Regression boundary

Exact head `4b3db739b6e5315953f7c07ab901ddaf0c8efd13` passed:

- Runtime Validation #47 / `35706826005`;
- D8A Source Capture Validation #33 / `35706825914`;
- Live Evaluator Validation #44 / `35706825962`;
- D7 Provenance Gate Validation #42 / `35706825923`;
- D8B Provenance Envelope Validation #6 / `35706825930`.

All ten Debug ASan/UBSan and TSan jobs report 17/17 PASS.

The accepted D7 negative authority boundary remains green.

## Known evidence limits

Current CI evidence is Linux-hosted.

Windows and Apple/BSD randomness branches are present but not independently
validated by these runs.

The audit does not establish:

- source truth;
- source-record re-derivability;
- admissibility;
- semantic interpretation;
- producer authentication;
- formal refinement proof;
- authority/capability/commit readiness.

## Audit disposition

```text
technical status: verified-on-cited-linux-CI
canonical conformance: verified
restricted-origin boundary: verified
D8A/C2/D7 regression: verified
implementation-revision binding: verified at candidate-contract level
architecture status: D8B provenance-envelope boundary coherent
acceptance status: ready-for-explicit-acceptance
merge authorization: not granted by this audit
```
