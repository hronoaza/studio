# SOAM 2.0 D9 — Policy Evidence and Persistence Design Contract

## Status

- Layer: D9 — Policy Evidence / Persistence / Recommendation
- Document class: design-contract candidate
- Executable implementation: none
- Current Baseline effect: none
- Upstream baseline: accepted D8D Versioned Interpretation
  (`0f8a71444e17c0f17ae66ee3aed01f0a8395166e`)
- Acceptance status: design-review pending
- Authority effect: none

D9 connects accepted versioned interpretation to the already accepted domain
policy and persistence primitives without allowing recommendation to become
permission or execution.

---

## 1. Boundary

Accepted domain primitives already provide:

```text
InteractionObservation
+ BridgeConfidence
-> AdaptiveBridgePolicy
-> BridgePolicyEvidence

BridgePolicyEvidence
-> BridgePersistence
-> PersistentBridgeRecommendation
```

D9 production boundary adds lineage and stream coherence:

```text
VersionedProductionInterpretation
-> D9 lineage-bound policy evidence
-> D9 lineage-bound persistence stream
-> PersistentBridgeRecommendation record
-> STOP
```

D9 does not create authority.

---

## 2. Principal separation

```text
interpretation
!= policy evidence
!= persistent recommendation
!= transition request
!= eligibility
!= permission
!= authority
!= execution
```

A D9 recommendation is advisory evidence for a later transition-policy layer.

---

## 3. D9A — lineage-bound policy evidence

The accepted `AdaptiveBridgePolicy` already defines:

```text
BridgePolicyEvidence =
    ((2 * compatibility) - 1) * confidence
```

D9 does not replace this formula.

Candidate production wrapper:

`ProductionBridgePolicyEvidence`

It contains:

- immutable `BridgePolicyEvidence`;
- SourceCaptureId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- D8C PolicySnapshotId;
- InterpretationDecisionId;
- InterpretationPolicySnapshotId;
- interpretation policy descriptor/revision;
- source/target node IDs;
- relationship generation;
- state version;
- policy-evidence decision identity.

Only a D9 evaluator consuming
`VersionedProductionInterpretation` may construct it.

---

## 4. Policy-evidence identity

Candidate type:

`PolicyEvidenceDecisionId`

Semantics:

> one completed D9A evaluation event.

Candidate representation:

- 128-bit opaque bytes;
- OS-backed random generation family;
- all-zero invalid;
- restricted-origin;
- independent from all upstream IDs.

Repeated D9A evaluation may mint different event IDs, but downstream persistence
must not count repeated evaluation of the same captured source fact as multiple
samples.

---

## 5. No semantic re-interpretation in D9A

D9A must pass the D8D-owned values directly into accepted:

`AdaptiveBridgePolicy::evaluate(observation, confidence)`

It must not:

- renormalize compatibility;
- alter confidence;
- inspect BridgeStatus;
- read live mesh state;
- introduce new weights;
- clamp evidence.

Thus D9A is an adaptation/lineage layer, not a new semantic model.

---

## 6. Policy evidence invariant

Given a valid D8D success:

```text
evidence.value()
=
((2 * interpretation.observation.compatibility()) - 1)
* interpretation.confidence.value()
```

The accepted domain type keeps evidence bounded to [-1,1] by construction of
its inputs.

D9A records this evidence but does not turn its sign into a command.

---

## 7. Why raw BridgePersistence is insufficient for production

`BridgePersistence` is intentionally domain-local and stateful.

Its public `observe(BridgePolicyEvidence)` API has no knowledge of:

- relationship identity;
- relationship generation;
- state version;
- SourceCaptureId;
- interpretation policy revision;
- duplicate/replayed evidence.

Therefore a production caller could otherwise mix:

```text
sample for relationship A
sample for relationship B
-> one persistent recommendation
```

or replay one captured fact multiple times to satisfy activationSamples.

D9 must prevent both.

---

## 8. D9B — persistence stream identity

Candidate type:

`ProductionPersistenceStreamKey`

V1 semantic key:

```text
sourceNodeId
targetNodeId
relationshipGeneration
interpretationPolicyId
interpretationPolicyMajor
interpretationPolicyMinor
interpretationImplementationRevision
persistenceProfileId/version
```

The stream is directional.

D8C policy snapshot identity and D8D policy snapshot identity are preserved on
each sample but are not by themselves the stream key.

Reason:

- snapshot event IDs may change while semantic policy remains identical;
- persistence continuity should bind to semantic policy/revision, not one
  publication event.

---

## 9. Relationship generation boundary

A persistence stream belongs to one exact relationship incarnation.

If `relationshipGeneration` changes:

```text
old stream != new stream
```

No pending/confirmed persistence state carries across generations.

This prevents stale evidence from a removed/recreated relationship from
contributing to the new relationship.

---

## 10. State-version ordering

For one stream, accepted production samples must have strictly increasing:

`stateVersion`

Required rule:

```text
new.stateVersion <= lastAccepted.stateVersion
-> sample rejected / not counted
```

This prevents:

- replay of the same state;
- multiple captures of one unchanged state from satisfying persistence counts;
- out-of-order historical evidence from changing current persistence state.

This is a production-stream rule, not a change to domain
`BridgePersistence`.

---

## 11. SourceCaptureId de-duplication

A SourceCaptureId may contribute at most once to one production persistence
stream.

Required rule:

```text
already-seen SourceCaptureId
-> no second observe() call
```

This remains required even if the same captured evidence is:

- wrapped in another D8B ProvenanceItemId;
- admitted by another D8C decision;
- interpreted by another D8D decision;
- evaluated again by D9A.

The temporal sample identity is the source capture, not the later wrapper event.

---

## 12. Interpretation policy continuity

Samples may coexist in one persistence stream only if they use the same exact:

- InterpretationPolicyId;
- major/minor version;
- implementation revision kind/digest.

If the interpretation semantic revision changes:

```text
old persistence stream terminates
new semantic revision -> new stream
```

This prevents a recommendation from aggregating samples that mean different
things numerically.

---

## 13. D8C policy changes

Each sample preserves its D8C policy identity/snapshot.

V1 does not automatically reset persistence merely because a new D8C
`PolicySnapshotId` is used, provided each sample was independently admitted and
the D8D semantic policy remains identical.

However a future policy may choose a stricter continuity rule.

D9 must never accept a raw D8B envelope in place of D8C-admitted evidence.

---

## 14. Persistence profile

The domain primitive requires:

- activationThreshold;
- releaseThreshold;
- activationSamples;
- releaseSamples.

Production values must be versioned and trusted-origin, not caller-selected per
sample.

Candidate type:

`PersistencePolicySnapshot`

Candidate descriptor:

```text
PersistenceProfileId
majorVersion
minorVersion
implementationRevision
activationThreshold
releaseThreshold
activationSamples
releaseSamples
```

---

## 15. PersistencePolicyV1 candidate

Candidate new production policy:

```text
activationThreshold = 0.50
releaseThreshold = 0.25
activationSamples = 2
releaseSamples = 2
```

These values already exercise the accepted domain persistence behavior in domain
tests, but that does **not** make them an inherited production policy.

D9 explicitly classifies them as a new versioned production-policy choice.

The rationale for v1:

- activation requires two consecutive strong samples;
- release requires two consecutive samples crossing the smaller hysteresis
  boundary;
- `releaseThreshold < activationThreshold` provides hysteresis;
- the smallest accepted sample count (2) still prevents one-shot activation.

Changing any of these values requires a new persistence policy revision.

---

## 16. Persistence state ownership

Candidate production type:

`ProductionBridgePersistenceStream`

It owns:

- immutable stream key;
- trusted PersistencePolicySnapshot;
- one `BridgePersistence` instance;
- last accepted stateVersion;
- bounded/source-capture de-duplication state;
- current recommendation;
- recommendation event identity/history metadata.

One stream represents one directed relationship incarnation under one semantic
interpretation revision and persistence profile.

---

## 17. De-duplication retention

A stream must remember enough accepted SourceCaptureIds to prevent replay for the
stream lifetime.

Preferred v1:

- retain all SourceCaptureIds accepted into that active stream.

This is acceptable because a stream is bounded by relationship incarnation and
policy revision, but production memory limits must be reviewed before
implementation.

A bounded recent-ID cache alone is not sufficient if an old sample can later be
replayed after eviction and counted again.

---

## 18. Persistence input ordering

Candidate `observe` sequence:

```text
1. verify D9A evidence lineage belongs to this stream key
2. verify exact persistence policy/profile
3. reject duplicate SourceCaptureId
4. require stateVersion > lastAcceptedStateVersion
5. call accepted BridgePersistence::observe(evidence)
6. atomically record SourceCaptureId + stateVersion + resulting recommendation
7. publish immutable recommendation record
8. STOP
```

No state mutation occurs before all potentially failing validation/preparation
steps complete.

---

## 19. Recommendation event

Candidate result:

`ProductionPersistentBridgeRecommendation`

Contents:

- recommendation event ID;
- stream key;
- persistence policy descriptor;
- current `PersistentBridgeRecommendation`;
- accepted PolicyEvidenceDecisionId;
- SourceCaptureId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- InterpretationDecisionId;
- relationship generation;
- state version;
- persistence transition metadata.

It is restricted-origin.

---

## 20. Recommendation semantics

Accepted domain values remain:

- `PRESERVE`;
- `CONSTRAIN`;
- `SUPPORT`.

Required interpretation:

```text
PersistentBridgeRecommendation
= persistent policy recommendation
!= requested production transition
!= permission
!= authority
```

In particular:

- SUPPORT does not authorize strengthening a bridge;
- CONSTRAIN does not authorize reducing/disconnecting it;
- PRESERVE does not authorize inaction if a later safety layer requires action.

---

## 21. Reset semantics

A production stream resets/terminates on:

- relationship generation change;
- interpretation policy semantic/revision change;
- persistence profile semantic/revision change;
- explicit operator/system lifecycle reset;
- integrity fault.

A mere D8D decision-ID change does not reset the stream.

A mere D8C decision-ID change does not reset the stream.

---

## 22. Out-of-order and duplicate samples

Candidate rejection reasons:

- WrongRelationship;
- WrongRelationshipGeneration;
- WrongInterpretationPolicy;
- WrongPersistenceProfile;
- DuplicateSourceCapture;
- NonIncreasingStateVersion;
- LineageInconsistent;
- EvidenceInvariantViolation;
- StreamClosed;
- InternalPersistenceFailure.

Rejected samples do not call `BridgePersistence::observe`.

---

## 23. Concurrency

One production persistence stream must serialize state mutation.

Concurrent observations must result in a deterministic total order or explicit
rejection; they may not race the internal `BridgePersistence` state.

Preferred v1:

- one mutex per stream;
- validate current stream state while holding the stream lock;
- prepare any non-stateful data before lock;
- one accepted sample -> one atomic stream-state transition.

No global persistence lock is required across unrelated streams.

---

## 24. Stream registry

A future runtime manager may map:

`ProductionPersistenceStreamKey -> ProductionBridgePersistenceStream`

Registry semantics must be:

- no duplicate live stream for one exact key;
- no implicit key collision fallback;
- new relationship generation creates a new key;
- closed streams are not silently reused.

Whether closed stream history is persisted is separate from v1 in-memory
runtime state.

---

## 25. Current retention limitation

D8C retained source evidence is currently process-lifetime/in-memory.

D9 persistence in v1 should therefore not claim cross-restart causal continuity
unless a separate persistence store is implemented.

On restart, in-memory persistence state is lost.

A later persistent checkpoint/ledger layer may address this.

---

## 26. No live-runtime mutation

D9 does not mutate:

- mesh topology;
- bridge capacity/status;
- node state/health;
- D8A/D8B/D8C/D8D evidence;
- transition evaluator state;
- authority ledger.

It only updates its own persistence state and emits recommendation evidence.

---

## 27. Relationship to C1/C2

D9 recommendations do not replace C1/C2.

A later transition-request layer may consume a D9 recommendation, but live
transition consideration must still pass current eligibility/freshness checks.

Required distinction:

```text
persistent recommendation
!= current live eligibility
```

---

## 28. Relationship to authority

D9 contains no:

- grant;
- nonce;
- expiry;
- capability scope;
- revocation authority;
- commit permission.

Human/operator authority remains outside D9.

---

## 29. Verification plan

Before implementation acceptance, tests should include at least:

1. D8D success -> exact accepted AdaptiveBridgePolicy evidence;
2. neutral interpretation -> zero evidence;
3. zero confidence -> zero evidence;
4. complementary compatibility -> opposite evidence;
5. raw D8B/D8C objects cannot directly create trusted D9 evidence;
6. repeated D9A evaluation does not let one SourceCaptureId count twice;
7. same SourceCaptureId with another ProvenanceItemId does not count twice;
8. same source stateVersion with distinct SourceCaptureId does not count twice;
9. strictly newer stateVersion may count;
10. relationship generation change starts a fresh stream;
11. interpretation policy revision change starts/requires a fresh stream;
12. persistence profile revision change starts/requires a fresh stream;
13. first activation sample stays PRESERVE;
14. second valid activation sample becomes SUPPORT/CONSTRAIN as domain policy
    specifies;
15. direction reversal resets pending activation streak;
16. release hysteresis matches accepted BridgePersistence behavior;
17. duplicate/out-of-order rejection leaves persistence state unchanged;
18. concurrent observations cannot double-count one capture;
19. result preserves all upstream lineage identities;
20. recommendation cannot be constructed by arbitrary caller;
21. recommendation does not create RequestedTransitionDirection;
22. all D7/D8A/D8B/D8C/D8D/C2 regression suites remain green.

---

## 30. Design risks requiring review

### R1 — replay amplification

Without SourceCaptureId/stateVersion constraints, repeated evaluation of the same
fact can manufacture persistence.

Design response: de-duplicate by SourceCaptureId and require increasing
stateVersion.

### R2 — cross-relationship mixing

Raw BridgePersistence has no relationship identity.

Design response: one production stream per exact relationship incarnation/key.

### R3 — semantic-policy mixing

Samples interpreted under different D8D policy revisions must not aggregate.

Design response: interpretation policy semantic/revision is part of stream key.

### R4 — recommendation/authority collapse

Persistent recommendation could be mistaken for permission.

Design response: D9 stops at restricted-origin recommendation evidence and
contains no authority types.

### R5 — restart continuity claim

In-memory state cannot prove cross-restart persistence.

Design response: v1 explicitly limits persistence continuity to process
lifetime unless a later persistent store is added.

---

## 31. Next gates

Before D9 implementation:

1. critically review stream-key semantics;
2. accept/revise SourceCaptureId + stateVersion anti-replay rule;
3. accept/revise PersistencePolicyV1 values;
4. define exact restricted-origin C++ evidence/recommendation API;
5. define stream lifecycle/registry API;
6. define persistence policy identity/revision;
7. perform final design acceptance review;
8. only then implement on a separate candidate branch.
