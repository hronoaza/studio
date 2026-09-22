# CI Validation — SOAM 2.0 D8C Provenance Admissibility

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `candidate/soam-2.0-d8c-provenance-admissibility-runtime`
- Base Current Baseline: `09b531bc2950a1e219872b7f6291212bdb3b80f3`
- Exact validated implementation/test head:
  `70e5dd371347a3218dba4298c9072a9c452fb0c9`

## Successful workflows on exact head

| Workflow | Run | Result |
|---|---:|---|
| Runtime Validation | #50 / `35710293181` | success |
| D8A Source Capture Validation | #36 / `35710293197` | success |
| D8B Provenance Envelope Validation | #9 / `35710293138` | success |
| Live Evaluator Validation | #47 / `35710293183` | success |
| D7 Provenance Gate Validation | #45 / `35710293144` | success |
| D8C Provenance Admissibility Validation | #1 / `35710293291` | success |

Every workflow executed both:

- Debug + AddressSanitizer + UndefinedBehaviorSanitizer;
- ThreadSanitizer with assertions enabled.

All twelve reviewed jobs report:

`100% tests passed, 0 tests failed out of 24`

No reviewed job reports an ASan, UBSan or TSan failure.

## D8C validation scope

The 24-test suite includes all previously accepted runtime/D7/D8A/D8B tests plus:

- D8C positive admissibility evaluation;
- trusted production policy provider;
- independent retained source evidence;
- exact producer revision recognition;
- schema recognition;
- required dependency presence;
- dependency recognition;
- legacy interpretation dependency rejection;
- source-record unavailable rejection;
- source resolver failure rejection;
- source evidence integrity-conflict rejection;
- bit-exact source-record mismatch rejection;
- CanonicalDigest mismatch rejection;
- resolver short-circuit behavior;
- decision-ID infrastructure failure;
- six D8C compile-fail misuse gates.

## Critical trust-boundary evidence

The initial prototype contained a public policy publisher capable of accepting
caller-supplied producer/schema/dependency entries.

That would have permitted self-authorized positive admissibility.

Before the validated head, that API was removed.

Production policy now comes only from:

`ProductionProvenanceAdmissibilityPolicyProvider::createCurrent()`

which constructs a restricted policy snapshot from compile-time accepted:

- producer family/version/exact implementation revision;
- schema family/version/canonical encoding;
- D8B dependency registry;
- required dependency kinds.

Arbitrary policy variants are available only through the non-installed internal
test seam.

## D8B metadata-view amendment

The D8B envelope now exposes immutable typed `ProvenanceMetadataView`.

The D8B producer creates the typed metadata first and uses that same structured
metadata to build CanonicalEnvelopeV1 input.

Tests verify implementation revision metadata corresponds to canonical bytes.

No CanonicalEnvelopeV1 wire-format version change was introduced.

## Retained source evidence

The D8C positive path uses
`RetainedSourceEvidenceStore`, populated directly from an authentic
restricted-origin D8A snapshot.

The retained record is not reconstructed from D8B envelope bytes/accessors.

Repeated identical retention is idempotent.

A missing retained record causes fail-closed rejection.

## Evidence limits

The cited CI validates Linux GitHub-hosted runner paths.

Windows BCrypt and Apple/BSD random-ID paths are not independently exercised by
these runs.

The retained source store in this candidate is in-memory/process-lifetime.
Therefore this evidence establishes in-process source re-derivability only; it
does not establish source retention across restart or machine migration.

No claim is made for:

- D8D interpretation;
- producer cryptographic authentication;
- source truth;
- cross-restart retained-source availability;
- C2 live freshness through D8C;
- authority/capability/commit.
