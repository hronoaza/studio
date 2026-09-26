# SOAM 2.0 D8A — SourceCaptureId Amendment Design

## Status

- Layer: D8A — Source Capture
- Document class: upstream design-amendment candidate
- Executable implementation: none
- Current Baseline effect: none
- Related implementation candidate: PR #8
- Related downstream design: D8B Provenance Envelope / PR #12
- Acceptance status: design-review pending

This amendment introduces one additional source-capture identity concept:

`SourceCaptureId`

It does not add provenance, admissibility, interpretation, authority or commit
semantics to D8A.

---

## 1. Why this amendment exists

The D8B design requires a stable identity for one coherent source-capture event.

That identity cannot be minted in D8B without changing its meaning.

If the same immutable D8A snapshot is wrapped twice by D8B and receives two
different "acquisition" IDs, those IDs identify wrapping events rather than the
original source sample.

Therefore the sample/capture identity must originate at the source-capture
boundary.

---

## 2. Revised D8A chain

Current conceptual chain:

```text
ProductionRelationshipLocator
-> coherent runtime capture
-> ProductionRelationshipSourceSnapshot
```

Amended conceptual chain:

```text
ProductionRelationshipLocator
-> coherent runtime capture
-> mint SourceCaptureId
-> ProductionRelationshipSourceSnapshot(SourceCaptureId, raw facts)
-> STOP
```

The capture ID is source-capture metadata only.

---

## 3. SourceCaptureId semantics

Candidate type:

`SourceCaptureId`

Meaning:

> identity of one successful coherent source snapshot capture event.

Required properties:

- created by trusted D8A capture code;
- not caller-selectable;
- immutable;
- associated with exactly one successfully published snapshot;
- distinct from relationship identity;
- distinct from relationship generation;
- distinct from state version;
- distinct from future provenance item identity;
- distinct from any digest/hash.

Required distinction:

```text
relationship locator
!= relationship incarnation
!= source capture identity
!= provenance item identity
```

---

## 4. Capture-event semantics

Every successful call that actually publishes a new source snapshot receives a
new `SourceCaptureId`.

This remains true when:

- relationship generation is unchanged;
- runtime state version is unchanged;
- all captured source measurements are byte-identical.

Therefore:

```text
same source content != same capture event
```

Two successful captures of the same unchanged live relationship are two
distinct observations and must have different capture identities.

This resolves an important ambiguity: identity belongs to the observation event,
not merely to source state.

---

## 5. Failed capture semantics

A request that produces no snapshot must not publish a `SourceCaptureId`.

Examples:

- source node does not exist;
- target node does not exist;
- self relationship is invalid;
- requested relationship does not exist;
- internal capture precondition fails.

Implementation may reserve an internal candidate ID before the capture is
published, but a failed/reserved value must never later be reused as the identity
of a different successful capture.

Externally:

```text
capture failure -> no observable SourceCaptureId
```

---

## 6. Coherence boundary

The capture identity must be bound while the same coherent capture operation
owns the source facts.

Conceptually:

```text
acquire topology/state shared lock
-> resolve relationship
-> validate capture preconditions
-> read all D8A source facts
-> assign/mint SourceCaptureId
-> construct immutable snapshot
-> release lock
-> publish snapshot
```

The exact moment of ID generation may be moved earlier/later internally if:

- no caller can observe a partially constructed snapshot;
- one published snapshot has exactly one capture ID;
- the capture ID cannot become attached to facts from a different capture event.

---

## 7. Identity width

Candidate wire-compatible width:

`128 bits`

Rationale:

D8B CanonicalEnvelopeV1 already reserves exactly 16 bytes for
`SourceCaptureId`.

The C++ representation should therefore have deterministic 128-bit semantics
rather than inherit platform-dependent widths.

The exact C++ type is not fixed by this document.

---

## 8. Uniqueness domain

A process-local monotonic integer alone is insufficient if it can silently
restart from zero.

Required uniqueness scope:

> no accepted SourceCaptureId may be silently reused by the same declared
> runtime identity domain across process restart, concurrent runtime instances,
> or version transition.

Candidate construction families:

### Option A — random 128-bit ID

Properties:

- no persistent counter required;
- collision probability must be explicitly documented;
- secure randomness is not required for secrecy, but uniqueness quality must be
  adequate.

### Option B — runtime-instance ID + monotonic sequence

Example conceptual split:

```text
64-bit stable/runtime-instance identity
+
64-bit monotonic capture sequence
```

Requires an accepted rule for runtime-instance uniqueness.

### Option C — persistent monotonic source-capture namespace

Requires durable counter/state.

No option is accepted by this amendment yet.

---

## 9. Non-security boundary

`SourceCaptureId` is an identity token, not a secret.

It must not be treated as:

- authentication credential;
- capability;
- nonce authorizing execution;
- tamper-proof token;
- proof of source truth.

Required invariant:

```text
capture_identity != authority
```

---

## 10. Snapshot API amendment

Conceptual additional accessor:

```cpp
[[nodiscard]] SourceCaptureId sourceCaptureId() const noexcept;
```

The snapshot remains restricted-origin.

No public constructor is introduced.

No public setter exists.

Caller code may read the capture ID but cannot select or replace it.

---

## 11. Relationship to D8B

D8B must preserve the D8A-provided `SourceCaptureId` exactly.

D8B must not:

- mint a replacement capture ID;
- normalize multiple captures into one ID;
- infer capture identity from content equality;
- accept a caller-supplied substitute.

D8B may independently mint a future:

`ProvenanceItemId`

Therefore:

```text
D8A owns SourceCaptureId
D8B owns ProvenanceItemId
```

---

## 12. Relationship to D8C/D8D

D8C may use `SourceCaptureId` as part of provenance/admissibility checks.

D8D may require all same-sample semantic derivations to share one
`SourceCaptureId`.

But D8A itself does not evaluate:

- producer trust;
- schema compatibility;
- integrity digest;
- source re-derivability;
- confidence;
- compatibility;
- interpretation.

---

## 13. Interaction with stateVersion

`stateVersion` and `SourceCaptureId` have different meanings.

`stateVersion`:

> which runtime state version was observed?

`SourceCaptureId`:

> which observation event captured it?

Therefore two snapshots may legally satisfy:

```text
captureA.stateVersion == captureB.stateVersion
captureA.SourceCaptureId != captureB.SourceCaptureId
```

This is expected, not an error.

---

## 14. Interaction with relationshipGeneration

`relationshipGeneration` identifies relationship incarnation.

`SourceCaptureId` identifies an observation of that incarnation.

Therefore:

```text
same relationshipGeneration
+ same stateVersion
+ same measurements
!= same source capture
```

unless the same immutable snapshot object/value is literally being preserved
rather than recaptured.

---

## 15. Copy and move semantics

Copying or moving an already-created immutable snapshot must preserve the same
`SourceCaptureId`.

Required invariant:

```text
copy(snapshot).SourceCaptureId == snapshot.SourceCaptureId
move(snapshot).SourceCaptureId refers to the same capture event
```

A copy is not a new source capture.

Only executing the source-capture operation again creates a new capture ID.

---

## 16. Equality semantics

Snapshot equality, if ever exposed, must not accidentally imply content-only
identity.

Conceptually distinct comparisons include:

### Same capture

`SourceCaptureId equal`

### Same relationship incarnation

`source/target/generation equal`

### Same observed runtime state version

`stateVersion equal`

### Same measured source facts

all raw measurement fields equal

These relations are not interchangeable.

---

## 17. Concurrency semantics

Concurrent successful captures of the same relationship are distinct capture
events.

Even if they observe the same state version and equal source facts:

```text
capture A SourceCaptureId != capture B SourceCaptureId
```

The identity allocator must therefore be concurrency-safe.

The capture ID generation mechanism must not require calling external providers
while the topology lock is held.

---

## 18. Failure atomicity

Potential identity-generation failure must not publish a snapshot without a
valid identity.

Required outcome:

```text
cannot obtain valid SourceCaptureId
-> no snapshot publication
```

No partially initialized capture ID is observable.

The live mesh state is unchanged by capture-ID failure.

---

## 19. Candidate verification plan

Before implementation acceptance, tests should include:

1. successful capture exposes a non-default/valid `SourceCaptureId`;
2. two repeated captures with unchanged runtime state have different IDs;
3. copied snapshot preserves the same ID;
4. moved snapshot still represents the same capture event;
5. state mutation changes stateVersion and a recapture also gets a new ID;
6. relationship recreation changes relationshipGeneration and recapture ID;
7. reverse-direction capture gets its own distinct capture ID;
8. concurrent successful captures get distinct IDs;
9. failed capture exposes no capture ID;
10. simulated identity-allocation failure publishes no snapshot;
11. public caller cannot construct snapshot with arbitrary capture ID;
12. public caller cannot replace capture ID;
13. D7/C2 behavior remains unchanged;
14. sanitizer and TSan suites remain clean.

---

## 20. Compile-fail requirement

The existing forged-snapshot negative test should eventually be extended so a
caller also cannot provide an arbitrary `SourceCaptureId`.

Conceptual misuse:

```cpp
ProductionRelationshipSourceSnapshot forged{
    attackerChosenCaptureId,
    ...
};
```

must remain impossible.

---

## 21. No provenance upgrade

Adding `SourceCaptureId` does not change this invariant:

```text
raw source snapshot != provenance envelope
```

The snapshot still contains only:

- source identity/context facts;
- one capture-event identity;
- raw relationship/endpoint measurements.

It still does not contain:

- provenance producer identity;
- provenance schema;
- canonical digest;
- dependency manifest;
- source-record reference;
- admissibility;
- interpretation.

---

## 22. PR #8 governance

This document does not modify PR #8.

If this amendment is accepted:

1. PR #8 must be updated explicitly;
2. the exact implementation commit must be revalidated;
3. source-snapshot tests must be extended;
4. forged-snapshot compile-fail evidence must be re-run;
5. ASan/UBSan and TSan CI must be re-run;
6. the D8A audit must be amended;
7. only then can D8A acceptance/merge be reconsidered.

The prior D8A validation evidence remains valid only for the prior exact
implementation head and must not be represented as validation of the amended
implementation.

---

## 23. Candidate invariants summary

```text
one successful capture event -> one SourceCaptureId

recapture != copy

same source facts != same capture identity

SourceCaptureId is minted at D8A capture
and preserved downstream

SourceCaptureId != relationshipGeneration
SourceCaptureId != stateVersion
SourceCaptureId != ProvenanceItemId
SourceCaptureId != digest
SourceCaptureId != capability

failed capture -> no observable SourceCaptureId

copy/move preserves capture identity

concurrent successful captures have distinct identities
```

---

## 24. Next decision

Before changing PR #8, review and decide:

1. accept/reject D8A ownership of `SourceCaptureId`;
2. accept/reject 128-bit width;
3. select an identity-generation family;
4. define the exact uniqueness domain;
5. only then amend implementation and re-run the full D8A validation gate.
