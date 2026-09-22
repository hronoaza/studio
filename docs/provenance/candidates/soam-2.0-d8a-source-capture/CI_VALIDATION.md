# CI Validation — SOAM 2.0 D8A Source Capture

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `candidate/soam-2.0-d8a-native-provenance`
- Current validated implementation/test head:
  `a7ed4edcf83b5787852265f99fc77ef4b57471d4`
- Prior pre-amendment validated head:
  `39ca8ff736c3ad21f4f0b5b644b42c0147c73699`

The prior head remains historical evidence only. It does not validate the
SourceCaptureId amendment.

## Successful workflows on exact amended head

| Workflow | Run | Result |
|---|---:|---|
| SOAM 2.0 Runtime Validation | #38 / `35702949626` | success |
| SOAM 2.0 D8A Source Capture Validation | #24 / `35702949664` | success |
| SOAM 2.0 Live Evaluator Validation | #35 / `35702949681` | success |
| SOAM 2.0 D7 Provenance Gate Validation | #33 / `35702949673` | success |

Each workflow executed:

- Debug + AddressSanitizer + UndefinedBehaviorSanitizer;
- ThreadSanitizer with assertions enabled.

All eight reviewed jobs report:

`100% tests passed, 0 tests failed out of 11`

## Eleven-test suite

1. runtime behavior;
2. worker-pool lifecycle;
3. topology contract;
4. live evaluator;
5. D7 provenance gate;
6. D8A source snapshot + SourceCaptureId lifecycle;
7. compile-fail rejection of D7 public provenance injection;
8. compile-fail rejection of forged D8A source snapshots;
9. compile-fail rejection of SourceCaptureId default construction;
10. compile-fail rejection of arbitrary-byte SourceCaptureId construction;
11. compile-fail rejection of SourceCaptureId mutation.

## SourceCaptureId behavior validated

The amended tests establish:

- every successful source snapshot carries a 16-byte non-zero
  `SourceCaptureId`;
- repeated capture without intervening runtime mutation preserves relationship
  generation/state version but receives a distinct capture ID;
- copy/move preserves the original capture identity;
- reverse-direction capture receives its own capture identity;
- state mutation + recapture receives a new capture identity;
- concurrent successful captures receive distinct IDs;
- hard random-source failure publishes no snapshot and leaves observed runtime
  state unchanged;
- all-zero random candidates are retried;
- a locally detected duplicate candidate is retried;
- eight invalid candidates exhaust the finite retry budget and fail closed.

The test-only deterministic random-source seam is internal and does not alter
the ordinary public class definitions.

## Compile-fail evidence

Reviewed logs explicitly report:

- `D7 public provenance injection misuse correctly rejected by compiler`
- `D8A forged source snapshot misuse correctly rejected by compiler`
- `D8A SourceCaptureId default construction misuse correctly rejected by compiler`
- `D8A SourceCaptureId arbitrary-byte construction misuse correctly rejected by compiler`
- `D8A SourceCaptureId mutation misuse correctly rejected by compiler`

## Sanitizer review

The reviewed successful jobs contain no reported:

- AddressSanitizer error;
- UBSan runtime error;
- ThreadSanitizer warning/error.

## Scope boundary

This evidence validates D8A source capture plus capture-event identity only.

It does not validate or claim:

- provenance item identity;
- producer/schema identity;
- canonical digest;
- dependency manifest;
- source-record reference/re-derivability;
- provenance admissibility;
- semantic interpretation into `InteractionObservation` /
  `BridgeConfidence`;
- positive live eligibility;
- authority/capability/commit.
