# Migration Record — SOAM 2.0 Live Evaluator Binding (Phase C2)

## Status

- Migration stage: `verified`
- Acceptance status: `acceptance-pending` — not accepted into Current Baseline
- Source archive: `adaptive-mesh-main (2).zip`
- Source archive SHA-256: `f14900ddb77852dbf3935562fdb5f73a051338ade1bb60a84f4d1fde59577c03`
- Migration branch: `migration/soam-2.0-live-evaluator`
- Base: canonical Current Baseline after Phase C1

## Phase C2 boundary

This phase introduces only the live evaluator binding and lineage/revalidation
surface between the compiled runtime and transition evaluation.

Public flow:

`locator -> fresh runtime relationship resolution -> snapshot -> revalidation -> fail-closed result`

The public result vocabulary is:

- `no_request`;
- `not_eligible`;
- `eligible_for_authority_consideration`.

In this phase the production live path never emits
`eligible_for_authority_consideration` because production-native
observation/confidence provenance sufficient to derive a transition direction
has not yet been accepted into the runtime.

## Included

- runtime-bound evaluator handle;
- lease-based lifetime safety;
- binding invalidation and drain before mesh destruction;
- directed relationship generation carried by runtime bridges;
- monotonic runtime transition-state version;
- fresh snapshot capture;
- generation + state-version revalidation;
- fail-closed behavior for invalid locators and unavailable derivation;
- concurrency/lifetime regression tests.

## Deliberate fail-closed rule

Phase C2 does not infer a transition direction from `BridgeStatus`.

`BridgeStatus` and `PersistentBridgeRecommendation` are distinct semantic
types. Treating one as the other would collapse the Phase A/C boundary and
create an unjustified authority-relevant derivation path.

Therefore an existing live relationship currently evaluates to
`not_eligible` after successful resolution/capture/revalidation.

## Relationship lineage

Each newly created bridge pair receives a fresh relationship generation.
If the visible endpoint pair disappears and is recreated, the new relation
receives a different generation.

A monotonic transition-state version advances on runtime changes relevant to
this conservative C2 domain. Revalidation requires both the same relationship
generation and the same state version.

The C2 version domain is intentionally conservative: unrelated runtime changes
may invalidate a captured attempt. Narrower domain-specific versions can only
be introduced after their invalidation semantics are separately proven.

## Explicitly deferred

- production-native direction derivation from accepted observation/confidence provenance;
- construction of C1 prerequisite evidence from live validators;
- positive live eligibility;
- BridgeTransitionAuthorization;
- production authority/capability derivation;
- production transition commit;
- K11/K12 mutation authority;
- distributed/cryptographic authority mechanisms.

## Source adaptation

The archive contains the desired binding/lease, relationship-generation,
state-version and revalidation concepts, but co-locates them with later
authority derivation and commit machinery.

This migration extracts only the pre-authority live-evaluation portion.

Final acceptance remains an explicit Root Operator decision.


## Current repository validation

Validated head:

`1e5c17100cc768f653b2f6127808792ff940965e`

GitHub Actions evidence:

- runtime workflow run #4, ID `35645827357`: 4/4 PASS under ASan/UBSan and 4/4 PASS under TSan;
- live evaluator workflow run #1, ID `35645827383`: 4/4 PASS under ASan/UBSan and 4/4 PASS under TSan;
- no sanitizer-reported error observed in reviewed logs.

Detailed evidence: `CI_VALIDATION.md`.

Pre-acceptance review: `AUDIT.md`.

## Remaining gate

Explicit Root Operator acceptance is required before merge into Current Baseline.
