# SOAM 2.0 D8A — Production-Native Provenance Producer

## Status

- Development class: new candidate layer
- Migration status: not an archive code migration
- Acceptance status: not accepted into Current Baseline
- Branch: `candidate/soam-2.0-d8a-native-provenance`
- Base: canonical Current Baseline after D7

## Purpose

D8A creates the first production-native raw provenance value directly from one
coherent runtime relationship snapshot.

It deliberately stops before interpretation.

Public flow:

`descriptive locator -> runtime lock -> relationship resolution ->
immutable raw provenance value`

## Provenance fields

A produced `ProductionRelationshipProvenance` contains:

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

All fields are captured while the runtime topology/state lock is held, so the
returned value belongs to one coherent runtime snapshot.

## Non-forgeability boundary

The provenance value has no public constructor.

Callers can request capture with a descriptive relationship locator, but they
cannot manufacture a value carrying arbitrary generation/version/runtime facts.

## Important semantic boundary

D8A does **not** convert raw runtime facts into:

- `InteractionObservation`;
- `BridgeConfidence`;
- `BridgePolicyEvidence`;
- `PersistentBridgeRecommendation`;
- requested transition direction;
- C1 prerequisite evidence;
- positive live eligibility.

In particular, D8A assigns no epistemic meaning to node health, bridge capacity,
orientation, distance, or bridge status.

## Why D8A is separate from D8B

The accepted domain types require semantic interpretation:

`raw facts -> compatibility observation + confidence`

That mapping must be explicit, independently reviewable, and tested. Encoding a
formula inside the producer would collapse source provenance and interpretation
into one opaque step.

## D8B gate

A future D8B candidate may consume D8A provenance and define a versioned
interpretation policy. It must specify:

- which raw fields affect compatibility;
- what `BridgeConfidence` means operationally;
- normalization/bounds;
- lineage binding;
- invalidation rules;
- behavior under conflicting directional snapshots.

Only after D8B is accepted may persistence/direction derivation be considered.

## Authority boundary

D8A has no authorization, capability issuance, transition intent, or production
commit behavior.

Final acceptance remains an explicit Root Operator decision.
