# CI Validation — SOAM 2.0 D8B Provenance Envelope

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `candidate/soam-2.0-d8b-provenance-envelope-runtime`
- Base Current Baseline: `379cd2e2cf17eb7181629945e670f7923ba662ce`
- Exact validated implementation/test head:
  `4b3db739b6e5315953f7c07ab901ddaf0c8efd13`

## Successful workflows on exact head

| Workflow | Run | Result |
|---|---:|---|
| SOAM 2.0 Runtime Validation | #47 / `35706826005` | success |
| SOAM 2.0 D8A Source Capture Validation | #33 / `35706825914` | success |
| SOAM 2.0 Live Evaluator Validation | #44 / `35706825962` | success |
| SOAM 2.0 D7 Provenance Gate Validation | #42 / `35706825923` | success |
| SOAM 2.0 D8B Provenance Envelope Validation | #6 / `35706825930` | success |

Every workflow executed both:

- Debug + AddressSanitizer + UndefinedBehaviorSanitizer;
- ThreadSanitizer with assertions enabled.

All ten reviewed jobs report:

`100% tests passed, 0 tests failed out of 17`

No reviewed job reports an ASan, UBSan or TSan failure.

## Seventeen-test suite

The suite contains:

1. runtime behavior;
2. worker-pool lifecycle;
3. topology contract;
4. live evaluator;
5. D7 provenance gate;
6. D8A source snapshot / SourceCaptureId;
7. D8B provenance envelope runtime/conformance;
8. D7 provenance-injection compile-fail;
9. D8A forged-source-snapshot compile-fail;
10. D8A SourceCaptureId default-construction compile-fail;
11. D8A SourceCaptureId arbitrary-byte construction compile-fail;
12. D8A SourceCaptureId mutation compile-fail;
13. D8B ProvenanceItemId default-construction compile-fail;
14. D8B ProvenanceItemId arbitrary-byte construction compile-fail;
15. D8B envelope default-construction compile-fail;
16. D8B CanonicalDigest arbitrary-byte construction compile-fail;
17. D8B ProvenanceItemId mutation compile-fail.

## D8B conformance evidence

The D8B runtime test verifies:

- exact normative V1 canonical bytes and digest;
- V2 negative-zero bit preservation;
- SourceCaptureId mutation changes canonical content/digest;
- ProvenanceItemId mutation changes canonical content/digest;
- dependency insertion order normalizes to one canonical byte sequence;
- accepted D8A SourceCaptureId is preserved exactly;
- repeated wrapping of one D8A snapshot preserves SourceCaptureId but gets a
  distinct ProvenanceItemId;
- canonical digest differs for distinct provenance-item identities;
- item-ID generator hard failure fails closed;
- all-zero item-ID candidate retries;
- locally detected duplicate item-ID candidate retries;
- finite retry exhaustion fails closed;
- forced canonicalization failure publishes no envelope;
- forced digest failure publishes no envelope;
- forced dependency-manifest failure publishes no envelope;
- live mesh state/health are not mutated by D8B failure paths.

## Implementation-revision binding

Production D8B envelope bytes set:

`implementationRevisionKind = 1`

The 32-byte revision is generated at configure time as SHA-256 over a
deterministic implementation manifest containing SHA-256 values for:

- `include/production_provenance_envelope.hpp`;
- `src/detail/provenance_envelope_internal.hpp`;
- `src/production_provenance_envelope.cpp`.

The manifest is independent of build-directory paths and documentation-only
commits.

The runtime test asserts that revision kind is 1 and that the serialized
revision field is non-zero.

## Validation history

Initial candidate head `18bb9f1d1d343349d3427fdd4caca169331b6128`
passed the first 17-test matrix before implementation-revision binding was
added.

Intermediate head `83b45b37dca163476f482b1cf869d511f28b4add`
failed compilation because the digest was transported through CMake compile
definitions with invalid quoting.

That head is retained as failed evidence and is not validation evidence.

The transport was replaced by a generated private header. Exact head
`4b3db739b6e5315953f7c07ab901ddaf0c8efd13` is the current valid evidence point.

## Scope boundary

This CI validates D8B provenance-envelope construction/conformance on the cited
Linux GitHub-hosted runner path.

It does not establish:

- D8C provenance admissibility;
- D8D semantic interpretation;
- authenticated producer origin;
- digital signatures/MACs;
- transition eligibility;
- authority/capability/commit;
- Windows BCrypt or Apple/BSD randomness execution paths.
