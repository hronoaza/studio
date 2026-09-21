# Pre-Acceptance Audit — SOAM 2.0 D7 Provenance Gate

## Repository state

- Base: `main`
- Base SHA: `9f9c8bb7e73c25aa3be497eba98721eef4b9b9f2`
- Validated head: `fb2d63a3f40249e3197df625315e2bbcbcc2f4d0`
- Compare state at audit: ahead 6, behind 0
- PR: #7
- Mergeability at audit: mergeable

## Changed surface

This migration changes no production runtime implementation.

It adds only:

- one D7 runtime negative-contract test;
- one compile-fail misuse case;
- one compile-fail harness;
- one CMake test registration change;
- one dedicated D7 validation workflow;
- provenance documentation.

## Archive boundary finding

The source archive does not provide an accepted production-native chain for:

`observation -> confidence -> adaptive evidence -> persistence -> direction`.

The current live evaluator therefore has no justified basis for a positive
direction derivation.

## Trust-boundary audit

The D7 gate prevents two unsafe shortcuts:

### Shortcut 1 — caller-injected provenance

A caller cannot feed arbitrary `InteractionObservation` and
`BridgeConfidence` values into the live evaluator through a public overload.

The compile-fail test proves the attempted misuse does not compile.

### Shortcut 2 — semantic substitution

The runtime does not substitute `BridgeStatus` for
`PersistentBridgeRecommendation`.

Therefore the existing bridge state machine cannot be used as an implicit
production direction oracle.

## Positive-path status

Positive D7 remains evidence-blocked.

A future implementation must separately establish and validate:

`production source
-> observation provenance
-> confidence provenance
-> AdaptiveBridgePolicy evidence
-> BridgePersistence evolution
-> PersistentBridgeRecommendation
-> requested direction`

The produced values must be bound to one coherent relationship incarnation and
runtime state lineage before C1 prerequisite materialization can be connected.

## Validation

At head `fb2d63a3f40249e3197df625315e2bbcbcc2f4d0`:

- D7 workflow run `35646956852`: 6/6 PASS under ASan/UBSan and 6/6 PASS under TSan;
- live evaluator regression run `35646956772`: success;
- runtime regression run `35646956777`: success;
- compile-fail provenance injection was correctly rejected;
- no sanitizer-reported error observed in reviewed D7 logs.

## Audit disposition

Technical state: `verified`.

Semantic status: `evidence-blocked-positive-path`.

Acceptance state: `acceptance-pending`.

No merge is authorized by this audit.
