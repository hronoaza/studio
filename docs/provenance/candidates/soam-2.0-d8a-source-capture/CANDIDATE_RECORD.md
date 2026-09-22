# SOAM 2.0 D8A — Production Relationship Source Capture

## Status

- Development class: candidate runtime layer
- Acceptance status: `acceptance-pending`
- Branch: `candidate/soam-2.0-d8a-native-provenance`
- Validated implementation/test head:
  `a7ed4edcf83b5787852265f99fc77ef4b57471d4`
- Base architecture: accepted D7 boundary plus accepted provenance refinement map

## Purpose

D8A creates one immutable raw runtime snapshot for one coherent observation of
one directed relationship incarnation.

Public flow:

```text
ProductionRelationshipLocator
-> coherent runtime capture
-> ProductionRelationshipSourceSnapshot
```

The snapshot now also carries a restricted-origin `SourceCaptureId` identifying
that specific capture event.

## SourceCaptureId

`SourceCaptureId` is:

- 128 bits / 16 opaque bytes;
- generated through an OS-backed CSPRNG;
- all-zero reserved invalid;
- immutable;
- not caller-selectable;
- not caller-mutable;
- distinct for repeated successful recaptures;
- preserved by copy/move.

It is not:

- provenance item identity;
- digest;
- authentication;
- capability;
- authority.

## Captured source facts

The snapshot contains:

- SourceCaptureId;
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

These remain raw source facts.

## Identity distinctions

```text
relationship locator
!= relationship incarnation
!= SourceCaptureId
!= future ProvenanceItemId
!= future CanonicalDigest
```

Repeated capture may legally satisfy:

```text
same relationship generation
same state version
same source facts
different SourceCaptureId
```

because recapture is a new observation event.

## Restricted-origin boundary

The public caller cannot manufacture:

- a source snapshot;
- a SourceCaptureId from arbitrary bytes;
- a default SourceCaptureId;
- a mutable capture identity.

The capture method is the only ordinary runtime path that binds a newly generated
ID to coherent source facts.

## Capture ordering

```text
reserve SourceCaptureId outside topology lock
-> acquire shared topology/state lock
-> validate/resolve relationship
-> read all source facts
-> construct immutable snapshot
-> return snapshot
```

Reserved IDs abandoned by failed captures are not observable.

## Semantic boundary

D8A does not create or validate:

- provenance item/producer identity;
- provenance schema;
- canonical digest;
- dependency manifest;
- source-record reference;
- provenance admissibility;
- re-derivability;
- `InteractionObservation`;
- `BridgeConfidence`;
- policy/persistence direction;
- positive eligibility;
- authority/capability/commit.

```text
raw source snapshot != provenance envelope != admissible provenance
```

## Dependency boundary

The neutral lower-level address remains:

`ProductionRelationshipLocator`

with only source/target node IDs.

Both D8A source capture and the C2 live evaluator depend on that lower-level
locator.

D8A does not depend on transition-evaluator-owned identity semantics.

## Validation

Successful exact-head workflows:

- Runtime Validation #38 / `35702949626`;
- D8A Source Capture Validation #24 / `35702949664`;
- Live Evaluator Validation #35 / `35702949681`;
- D7 Provenance Gate Validation #33 / `35702949673`.

Every reviewed Debug ASan/UBSan and TSan job reports:

`100% tests passed, 0 tests failed out of 11`

The eleven tests include the original runtime/C2/D7/D8A regression set plus
three SourceCaptureId compile-fail gates.

Detailed evidence: `CI_VALIDATION.md`.

Pre-acceptance review: `AUDIT.md`.

## Relationship to D8B design

The D8B candidate consumes the exact 16 SourceCaptureId bytes and must preserve
them in `CanonicalEnvelopeV1.SourceBinding.SourceCaptureId`.

D8B must not remint or normalize the capture identity.

No D8B runtime implementation exists in this D8A candidate.

## Remaining gate

Explicit Root Operator acceptance is required before merge into Current
Baseline.
