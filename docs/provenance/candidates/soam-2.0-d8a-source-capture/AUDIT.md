# Pre-Acceptance Audit — SOAM 2.0 D8A Source Capture

## Repository state

- Current canonical `main`:
  `f7477ca28cbfe1d94470533d7ae44a7ac99d9b60`
- Validated amended implementation/test head:
  `a7ed4edcf83b5787852265f99fc77ef4b57471d4`
- PR: #8
- Acceptance status: `acceptance-pending`

Documentation commits after the validated implementation/test head do not change
the code/test evidence represented by that exact head.

## Architecture boundary

D8A remains source capture only:

```text
ProductionRelationshipLocator
-> SourceCaptureId reservation
-> coherent runtime capture
-> ProductionRelationshipSourceSnapshot
-> STOP
```

The SourceCaptureId amendment does not make the snapshot a provenance envelope.

Required distinction remains:

```text
raw source snapshot
!= provenance envelope
!= admissible provenance
!= interpretation
```

## SourceCaptureId design realized

The implementation uses an immutable 16-byte `SourceCaptureId`.

Properties verified in the candidate:

- no public default construction;
- no public arbitrary-byte construction;
- no public mutation;
- read-only byte access;
- copy/move preserves identity;
- every new successful capture event receives a distinct ID;
- all-zero is reserved invalid;
- generation uses an OS-backed CSPRNG in production;
- there is no timestamp/process/node semantic encoding;
- the ID is not a capability, credential or proof of source truth.

The test-only deterministic provider is internal and does not modify canonical
public API declarations across build modes.

## Capture ordering

The implemented ordering is:

```text
generate/reserve SourceCaptureId
-> acquire topology/state shared lock
-> validate relationship
-> read coherent source facts
-> construct immutable snapshot
-> release lock
-> publish/return
```

This keeps potentially blocking OS randomness outside the topology lock.

An ID reserved for a capture that later fails validation is never observable as
a published capture identity and is not reused by the local generator path.

## Randomness/collision boundary

Production generation is 128-bit OS-backed randomness.

A bounded process-local recent-ID tracker provides deterministic local collision
rejection and testability; it is defense-in-depth, not a global provenance
ledger.

The design does not claim mathematical collision impossibility or globally
persistent duplicate detection.

Random-source failure or retry exhaustion fails closed:

```text
no valid SourceCaptureId
-> no source snapshot publication
```

No fallback to weak PRNG, timestamp, zero ID or process-local counter exists.

## Source snapshot semantics

`ProductionRelationshipSourceSnapshot` is restricted-origin and immutable
after construction.

It records one observed directed relationship capture:

- SourceCaptureId;
- source/target IDs;
- relationship generation;
- runtime state version;
- distance;
- orientation weight;
- capacity;
- bridge status;
- source/target state;
- source/target health.

The capture ID identifies the observation event.

It is distinct from:

- relationship generation;
- runtime state version;
- future provenance item identity;
- future digest;
- authority capability.

## Non-forgeability

The public caller cannot:

- construct a source snapshot;
- default-construct a SourceCaptureId;
- construct SourceCaptureId from arbitrary bytes;
- mutate the capture ID exposed by a snapshot.

Compile-fail gates demonstrate these boundaries.

## Regression evidence

Exact head
`a7ed4edcf83b5787852265f99fc77ef4b57471d4`
completed successfully in four workflows:

- Runtime Validation #38 / `35702949626`;
- D8A Source Capture Validation #24 / `35702949664`;
- Live Evaluator Validation #35 / `35702949681`;
- D7 Provenance Gate Validation #33 / `35702949673`.

Each workflow passed the full 11-test suite under both:

- Debug ASan/UBSan;
- TSan.

Observed in every reviewed job:

`100% tests passed, 0 tests failed out of 11`

No reviewed sanitizer report indicates ASan, UBSan or TSan failure.

## Fail-closed boundary

No D8A path creates:

- `InteractionObservation`;
- `BridgeConfidence`;
- `PersistentBridgeRecommendation`;
- requested transition direction;
- `eligible_for_authority_consideration`;
- authority/capability/commit.

Therefore the accepted D7/C2 negative boundary remains intact.

## Portability note

The current CI evidence validates the Linux GitHub-hosted runner paths.

The production implementation also contains a Windows BCrypt path and
Apple/BSD `arc4random_buf` path, but those platform paths are not independently
validated by the cited Linux workflow runs.

They must not be represented as tested platform evidence until exercised on
those platforms.

## Remaining gaps

D8A deliberately leaves later layers to:

- D8B provenance envelope;
- D8C provenance admissibility;
- D8D versioned interpretation;
- evidence ledger;
- authority/capability/commit.

D8B's canonical 16-byte SourceCaptureId wire representation is design evidence
only until D8B implementation exists.

## Audit disposition

Technical status: `verified-on-cited-linux-CI`.

Architectural status:
`source-capture + capture-identity boundary verified`.

Acceptance status: `acceptance-pending`.

No merge is authorized by this audit.
