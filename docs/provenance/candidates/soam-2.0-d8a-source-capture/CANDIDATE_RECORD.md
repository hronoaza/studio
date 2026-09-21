# SOAM 2.0 D8A — Production Relationship Source Capture

## Status

- Development class: new candidate layer
- Acceptance status: `acceptance-pending` — not accepted into Current Baseline
- Branch: `candidate/soam-2.0-d8a-native-provenance`
- Base architecture: accepted D7 boundary plus accepted provenance refinement map

## Purpose

D8A is a source-capture primitive only.

It creates one immutable raw runtime snapshot for one directed relationship
incarnation under the existing runtime topology/state lock.

Public flow:

`ProductionRelationshipLocator
-> coherent runtime capture
-> ProductionRelationshipSourceSnapshot`

## Captured source facts

The snapshot contains:

- source node ID;
- target node ID;
- relationship generation;
- runtime transition-state version;
- bridge distance;
- bridge orientation weight;
- bridge capacity;
- bridge status;
- source state;
- target state;
- source health;
- target health.

These values are raw source facts. D8A makes no claim that they are
admissible provenance evidence.

## Restricted-origin construction

`ProductionRelationshipSourceSnapshot` has no public constructor.

A caller may request a capture for a descriptive relationship locator, but
cannot manufacture arbitrary generation/version/runtime values.

## Semantic boundary

D8A does not create or validate:

- provenance item identity;
- producer identity;
- schema identity/version;
- integrity digest;
- dependency manifest;
- re-derivability metadata;
- `AdmissibleProductionProvenance`;
- `InteractionObservation`;
- `BridgeConfidence`;
- policy evidence;
- persistent recommendation;
- requested transition direction;
- C1 prerequisite evidence;
- positive live eligibility;
- authority/capability/commit.

In particular:

`raw source snapshot != provenance envelope != admissible provenance`

## Refinement relationship

The accepted refinement map defines the intended later sequence:

`D8A Source Capture
-> D8B Provenance Envelope
-> D8C Provenance Admissibility
-> D8D Versioned Interpretation`

D8A implements only the first arrow's source value.

No claim is made that the C++ implementation formally refines the TLA+ model.

## Dependency boundary

The source-snapshot value is defined in a standalone header that does not
include `system_architecture.hpp` and does not depend on transition evaluator
implementation details.

`system_architecture.hpp` may expose capture of the complete snapshot type,
while the snapshot header only uses an opaque declaration of `BridgeStatus`
and friendship to `SpatialAdaptiveMesh`.

This avoids the cyclic public-header dependency present in the first D8A
candidate attempt.

## Validation requirements

Before acceptance D8A must show:

- successful runtime compilation;
- sanitizer-clean source-snapshot tests;
- runtime/live-evaluator/D7 regression success;
- compile-fail rejection of caller-forged snapshots;
- coherent generation/version behavior across state changes.

Final acceptance remains an explicit Root Operator decision.


## Canonical addressing layer

D8A and the live transition evaluator share the neutral descriptive type
`ProductionRelationshipLocator`.

The locator contains only:

- source node ID;
- target node ID.

It contains no relationship generation, state version, transition direction,
eligibility state, provenance semantics, or authority semantics.

Relationship generation and state version are resolved by the runtime during
capture. Therefore:

`locator = what relationship is requested`

while:

`source snapshot = which live relationship incarnation was observed`

The lower addressing type is independent of transition-evaluator semantics.


## Final validation state

Validated implementation/test head:

`39ca8ff736c3ad21f4f0b5b644b42c0147c73699`

Successful workflows:

- Runtime Validation #29, ID `35653148265`;
- D8A Source Capture Validation #15, ID `35653148247`;
- Live Evaluator Validation #26, ID `35653148270`;
- D7 Provenance Gate Validation #24, ID `35653148317`.

Each workflow passed 8/8 tests under both Debug ASan/UBSan and TSan jobs.

Detailed evidence: `CI_VALIDATION.md`.

Pre-acceptance review: `AUDIT.md`.

Documentation commits after the validated head do not modify the validated
runtime implementation, tests, CMake registration, or workflow definitions.

## Remaining gate

Explicit Root Operator acceptance is required before merge into Current Baseline.
