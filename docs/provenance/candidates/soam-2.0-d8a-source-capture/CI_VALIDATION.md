# CI Validation — SOAM 2.0 D8A Source Capture

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `candidate/soam-2.0-d8a-native-provenance`
- Validated head: `39ca8ff736c3ad21f4f0b5b644b42c0147c73699`

Four runtime-relevant workflows completed successfully on this exact head.

| Workflow | Run | Result |
|---|---:|---|
| SOAM 2.0 Runtime Validation | #29 / `35653148265` | success |
| SOAM 2.0 D8A Source Capture Validation | #15 / `35653148247` | success |
| SOAM 2.0 Live Evaluator Validation | #26 / `35653148270` | success |
| SOAM 2.0 D7 Provenance Gate Validation | #24 / `35653148317` | success |

## Test matrix

Each workflow executed the eight-test runtime suite under:

- Debug + AddressSanitizer + UndefinedBehaviorSanitizer;
- ThreadSanitizer with assertions enabled.

Observed result in every reviewed job:

`100% tests passed, 0 tests failed out of 8`

The suite contains:

1. runtime behavior;
2. worker-pool lifecycle;
3. topology contract;
4. live evaluator;
5. D7 provenance gate;
6. D8A source snapshot;
7. compile-fail rejection of D7 public provenance injection;
8. compile-fail rejection of forged D8A source snapshots.

## Compile-fail evidence

Reviewed logs explicitly report:

`D7 public provenance injection misuse correctly rejected by compiler`

and:

`D8A forged source snapshot misuse correctly rejected by compiler`

The harness wording is therefore aligned with the actual negative-contract test.

## D8A behavior validated

At the validated head:

- `ProductionRelationshipLocator` is the neutral descriptive address type;
- both D8A capture and the C2 live evaluator consume that lower-level locator;
- the source-capture API no longer depends on a transition-evaluator-owned locator type;
- `ProductionRelationshipSourceSnapshot` cannot be caller-constructed;
- no relationship produces no snapshot;
- a connected relationship produces a snapshot with relationship generation and runtime state version;
- repeated capture without intervening state change preserves generation/version;
- runtime mutation advances state version while preserving relationship generation when the relationship incarnation remains;
- live evaluator and D7 fail-closed behavior remain intact.

## Sanitizer review

The reviewed successful jobs contain no reported:

- AddressSanitizer error;
- UBSan runtime error;
- ThreadSanitizer warning/error.

## Scope boundary

This evidence validates D8A source capture only.

It does not validate or claim:

- provenance envelope identity;
- producer/schema identity;
- integrity digest;
- dependency manifest;
- re-derivability;
- provenance admissibility;
- semantic interpretation into `InteractionObservation` / `BridgeConfidence`;
- positive live eligibility;
- authority/capability/commit.
