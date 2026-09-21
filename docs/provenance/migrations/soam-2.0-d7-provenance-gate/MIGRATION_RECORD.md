# SOAM 2.0 D7 Provenance Gate

## Status

- Migration stage: `verified`
- Acceptance status: `acceptance-pending` — not accepted into Current Baseline
- Source archive: `adaptive-mesh-main (2).zip`
- Source archive SHA-256: `f14900ddb77852dbf3935562fdb5f73a051338ade1bb60a84f4d1fde59577c03`
- Branch: `migration/soam-2.0-d7-provenance-gate`
- Base: canonical Current Baseline after Phase C2

## Evidence-blocked finding

The supplied SOAM 2.0 development archive explicitly states that the D7
production provenance slice is not authorized/available:

- no production observation producer;
- no production confidence producer;
- no adaptive evidence production path;
- no persistence update from production adaptive evidence;
- no production-native direction derivation.

The archive's live backend therefore fails closed before direction derivation.

## Gate established by this migration

This migration does not invent the missing D7 producer.

It adds negative assurance that:

1. the live evaluator cannot be default-constructed by a caller;
2. the public evaluator accepts only a descriptive relationship locator;
3. callers cannot inject `InteractionObservation` and `BridgeConfidence`
   into the live evaluator;
4. repeated live runtime evolution cannot promote C2 to
   `eligible_for_authority_consideration`;
5. an existing relationship remains fail-closed at `not_eligible`.

## Required future D7 work

A future positive D7 implementation must separately establish:

`production source -> observation provenance -> confidence provenance ->
AdaptiveBridgePolicy evidence -> BridgePersistence evolution ->
PersistentBridgeRecommendation -> requested direction`

and prove that each value is bound to one coherent relationship incarnation and
state lineage.

Only after that may the existing C1 prerequisite-materialization pipeline be
connected to a positive live eligibility path.

## Explicitly prohibited in this gate

- deriving direction from `BridgeStatus`;
- caller-supplied observation/confidence injection;
- synthetic evidence values;
- positive eligibility by test hook;
- transition authorization;
- capability issuance;
- production commit.

## Governance

This is a blocking boundary, not a feature-completion claim.

Final acceptance remains an explicit Root Operator decision.


## Current repository validation

Validated head:

`fb2d63a3f40249e3197df625315e2bbcbcc2f4d0`

GitHub Actions evidence:

- D7 provenance gate run #1, ID `35646956852`: 6/6 PASS under ASan/UBSan and 6/6 PASS under TSan;
- live evaluator regression run #3, ID `35646956772`: success;
- runtime regression run #6, ID `35646956777`: success;
- compile-fail synthetic provenance injection: correctly rejected;
- no sanitizer-reported error observed in reviewed D7 logs.

Detailed evidence: `CI_VALIDATION.md`.

Pre-acceptance review: `AUDIT.md`.

## Current semantic status

The negative D7 boundary is verified.

The positive D7 path remains `evidence-blocked` until a separately evidenced
production-native provenance producer is designed and accepted.

## Remaining gate

Explicit Root Operator acceptance is required before merge into Current Baseline.
