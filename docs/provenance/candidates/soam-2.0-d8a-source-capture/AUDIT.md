# Pre-Acceptance Audit — SOAM 2.0 D8A Source Capture

## Repository state

- Current canonical `main`: `f7477ca28cbfe1d94470533d7ae44a7ac99d9b60`
- Validated D8A head: `39ca8ff736c3ad21f4f0b5b644b42c0147c73699`
- PR: #8
- PR state at audit: draft candidate
- Compare state: ahead 35, behind 1

The branch is behind `main` by one documentation-only commit:
`docs/architecture/SOAM_PROVENANCE_REFINEMENT_MAP.md`.

No runtime/code change exists in that behind delta.

## Architectural correction history

The initial D8A attempt used the semantically stronger
`ProductionRelationshipProvenance` name and depended on a
transition-evaluator-owned locator.

That design was not carried forward.

The accepted refinement map established:

`source facts != provenance envelope != admissible provenance != interpretation`

The revised candidate therefore uses:

`ProductionRelationshipLocator
-> ProductionRelationshipSourceSnapshot`

and reserves provenance/admissibility semantics for later D8B/D8C layers.

## Dependency direction

The canonical lower-level address type is:

`ProductionRelationshipLocator`

It contains only source and target node IDs.

It contains no:

- relationship generation;
- runtime state version;
- transition direction;
- eligibility state;
- provenance semantics;
- authority semantics.

Both source capture and live transition evaluation depend on this locator.
D8A no longer depends on transition-evaluator identity semantics.

## Source snapshot semantics

`ProductionRelationshipSourceSnapshot` is restricted-origin and immutable
after construction.

It records raw runtime facts for one observed directed relationship incarnation:

- source/target IDs;
- relationship generation;
- runtime state version;
- distance;
- orientation weight;
- capacity;
- bridge status;
- source/target state;
- source/target health.

Capture occurs while the runtime topology/state shared lock is held.

## Non-forgeability

The snapshot has no public constructor.

The compile-fail gate demonstrates that a caller cannot manufacture a snapshot
with arbitrary generation/version/runtime values.

This is a source-integrity boundary only; it is not yet provenance
admissibility.

## Regression boundary

Final CI on exact head `39ca8ff736c3ad21f4f0b5b644b42c0147c73699` shows:

- runtime validation: success, 8/8 under both sanitizer jobs;
- D8A source capture validation: success, 8/8 under both sanitizer jobs;
- live evaluator validation: success, 8/8 under both sanitizer jobs;
- D7 provenance gate validation: success, 8/8 under both sanitizer jobs.

The accepted fail-closed boundary is preserved:

`source snapshot != transition direction`

No D8A path creates `InteractionObservation`, `BridgeConfidence`,
`PersistentBridgeRecommendation`, or
`eligible_for_authority_consideration`.

## Naming hygiene

Active D8A implementation/API/workflow naming is SOAM/production-neutral.
Legacy project labels from research archives are not introduced into the active
runtime surface.

Historical source names remain relevant only as provenance/evidence references
outside active implementation naming.

## Remaining refinement gaps

D8A deliberately leaves the following for later layers:

- D8B provenance envelope;
- D8C admissibility;
- D8D versioned interpretation;
- persistence/direction binding to live production evidence;
- C1 prerequisite materialization;
- authority/capability/commit.

## Audit disposition

Technical status: `verified`.

Architectural status: `source-capture boundary verified`.

Acceptance status: `acceptance-pending`.

No merge is authorized by this audit.
