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
interpretationImplementationRevisionKind
interpretationImplementationRevisionDigest
persistenceProfileId
persistenceProfileMajor
persistenceProfileMinor
persistenceImplementationRevisionKind
persistenceImplementationRevisionDigest
```

The stream is directional.

The semantic stream key identifies compatibility/continuity semantics; it does
not uniquely identify one runtime stream incarnation. A separate restricted-
origin `PersistenceStreamInstanceId` identifies each concrete live stream
instance. Closing/resetting a stream and later creating another stream with the
same semantic key must mint a new instance ID.

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
implementationRevisionKind
implementationRevisionDigest
activationThreshold
releaseThreshold
activationSamples
releaseSamples
```

The persistence-profile semantic identity used by the stream key is the exact
tuple of profile ID, major/minor version, implementation revision kind, and
implementation revision digest. Snapshot/publication event identity is not a
substitute for this semantic identity.

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
- release uses the exact direction-dependent predicates already implemented by
  `BridgePersistence`: while SUPPORT is active, a release sample is
  `value <= +releaseThreshold`; while CONSTRAIN is active, a release sample is
  `value >= -releaseThreshold`;
- two consecutive release-predicate samples are required to return to PRESERVE;
- `releaseThreshold < activationThreshold` provides hysteresis;
- the smallest accepted sample count (2) still prevents one-shot activation.

Changing any of these values requires a new persistence policy revision.

---

## 16. Persistence state ownership

Candidate production type:

`ProductionBridgePersistenceStream`

It owns:

- immutable semantic stream key;
- immutable PersistenceStreamInstanceId;
- trusted PersistencePolicySnapshot;
- one `BridgePersistence` instance;
- last accepted stateVersion;
- bounded/source-capture de-duplication state;
- current recommendation;
- recommendation event identity/history metadata.

One stream represents one directed relationship incarnation under one semantic
interpretation revision and persistence profile.

Each newly created live stream also owns an immutable observation-epoch lower
bound:

`streamStartStateVersion`

For v1 live semantics, a counted sample must satisfy:

```text
sample.stateVersion > streamStartStateVersion
```

The bound is captured when the live stream is created from one trusted,
atomic relationship snapshot containing at least source/target node IDs,
relationship generation, and stateVersion. The snapshot's relationship identity
and generation must exactly match the semantic stream key before publication of
the stream instance.

The stream-creation snapshot must be obtained under the same consistency
boundary used by the runtime to publish relationship generation and stateVersion;
separate unsynchronized reads are not sufficient. If the relationship changes
or disappears before a coherent creation snapshot can be established, stream
creation fails closed.

This prevents a newly created stream, including one created
after an interpretation-policy or persistence-profile revision, from rebuilding
a live persistence streak by replaying older retained captures.

Historical/backfill evaluation, if introduced later, must use a separate
explicit mode and must not emit a live
`ProductionPersistentBridgeRecommendation`.

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
3. require stateVersion > streamStartStateVersion
4. reject duplicate SourceCaptureId
5. require stateVersion > lastAcceptedStateVersion
6. call accepted BridgePersistence::observe(evidence)
7. atomically record SourceCaptureId + stateVersion + resulting recommendation
8. publish immutable recommendation record
9. STOP
```

No state mutation occurs before all potentially failing validation/preparation
steps complete.

---

## 19. Recommendation event

Candidate result:

`ProductionPersistentBridgeRecommendation`

Contents:

- recommendation event ID;
- PersistenceStreamInstanceId;
- semantic stream key;
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

A production stream instance resets/terminates on:

- relationship generation change;
- interpretation policy semantic/revision change;
- persistence profile semantic/revision change;
- explicit operator/system lifecycle reset;
- integrity fault.

A mere D8D decision-ID change does not reset the stream.

A mere D8C decision-ID change does not reset the stream.

Any replacement stream created after termination/reset is a new stream instance,
even if its semantic stream key is byte-for-byte identical. It receives a new
`PersistenceStreamInstanceId` and a new trusted creation snapshot/epoch bound.

---

## 22. Out-of-order and duplicate samples

Candidate rejection reasons:

- WrongRelationship;
- WrongRelationshipGeneration;
- WrongInterpretationPolicy;
- WrongPersistenceProfile;
- PreStreamEpochStateVersion;
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

- no duplicate live stream for one exact semantic key;
- one semantic key may have multiple historical stream instances, never more
  than one live instance at a time;
- every concrete stream instance has a unique PersistenceStreamInstanceId;
- no implicit key collision fallback;
- new relationship generation creates a new semantic key;
- closed stream instances are not silently reused;
- reopening the same semantic key creates a fresh instance ID and fresh
  creation-time epoch boundary.

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
10. a capture at/before streamStartStateVersion cannot seed a newly created
    live stream;
11. interpretation-policy revision cannot rebuild a live streak from pre-stream
    retained captures;
12. persistence-profile revision cannot rebuild a live streak from pre-stream
    retained captures;
13. relationship generation change starts a fresh stream;
14. interpretation policy revision change starts/requires a fresh stream;
15. persistence profile revision change starts/requires a fresh stream;
16. reset/close followed by recreation of the same semantic key produces a new
    PersistenceStreamInstanceId and fresh epoch boundary;
17. stream creation uses one coherent relationship-generation/stateVersion
    snapshot and fails closed on lineage mismatch;
18. first activation sample stays PRESERVE;
19. second valid activation sample becomes SUPPORT/CONSTRAIN as domain policy
    specifies;
20. direction reversal resets pending activation streak;
21. SUPPORT release uses value <= +releaseThreshold;
22. CONSTRAIN release uses value >= -releaseThreshold;
23. release hysteresis matches accepted BridgePersistence behavior;
24. duplicate/out-of-order/pre-epoch rejection leaves persistence state unchanged;
25. concurrent observations cannot double-count one capture;
26. result preserves all upstream lineage identities;
27. recommendation preserves exact PersistenceStreamInstanceId;
28. recommendation cannot be constructed by arbitrary caller;
29. recommendation does not create RequestedTransitionDirection;
30. all D7/D8A/D8B/D8C/D8D/C2 regression suites remain green.

---

## 30. Design risks requiring review

### R1 — replay amplification

Without SourceCaptureId/stateVersion constraints, repeated evaluation of the same
fact can manufacture persistence.

Design response: de-duplicate by SourceCaptureId, require increasing
stateVersion, and bind each live stream to a creation-time
streamStartStateVersion epoch so older retained captures cannot seed a newly
created live persistence stream.

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

### R5 — cross-policy historical backfill amplification

A new stream created after an interpretation-policy or persistence-profile
revision would otherwise begin with empty de-duplication/order state. Older
retained captures could then be reinterpreted under the new policy and used to
manufacture a fresh live persistence streak.

Design response: every live stream has a trusted creation-time
`streamStartStateVersion`; samples at or below that boundary are not counted.
Historical/backfill processing is separate from live recommendation semantics.

### R6 — stream-instance identity collapse

The semantic stream key can recur after explicit reset, integrity-fault closure,
or other lifecycle termination. Treating the key itself as the unique stream
identity would make recommendation history and lifecycle provenance ambiguous.

Design response: each concrete live stream has a restricted-origin
`PersistenceStreamInstanceId`. Recreating the same semantic key creates a new
instance ID and a fresh trusted epoch boundary.

### R7 — non-atomic stream epoch capture

If relationshipGeneration and streamStartStateVersion were read independently,
a stream could bind an epoch from one live state to a relationship identity from
another.

Design response: stream creation consumes one coherent trusted relationship
snapshot containing the relationship identity/generation and stateVersion, and
fails closed if the snapshot does not match the semantic key.

### R8 — restart continuity claim

In-memory state cannot prove cross-restart persistence.

Design response: v1 explicitly limits persistence continuity to process
lifetime unless a later persistent store is added.

---

## 31. Candidate restricted-origin C++ API contract

This section fixes the public construction/ownership boundaries before any
runtime implementation exists. Names are candidate API names; the semantic
constraints are normative for D9 v1.

### 31.1 D9A decision identity

```cpp
class PolicyEvidenceDecisionId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    PolicyEvidenceDecisionId(const PolicyEvidenceDecisionId&) noexcept = default;
    PolicyEvidenceDecisionId& operator=(const PolicyEvidenceDecisionId&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept;

private:
    explicit PolicyEvidenceDecisionId(Bytes) noexcept;

    friend class ProductionBridgePolicyEvidenceEvaluator;
};
```

Required properties:

- no public default constructor;
- no public construction from raw bytes;
- all-zero is invalid;
- generated only by the D9A evaluator through the same OS-backed/random-family
  discipline used by existing restricted-origin production IDs.

### 31.2 D9A trusted evidence wrapper

```cpp
class ProductionBridgePolicyEvidence final {
public:
    [[nodiscard]] const PolicyEvidenceDecisionId& decisionId() const noexcept;
    [[nodiscard]] const BridgePolicyEvidence& evidence() const noexcept;
    [[nodiscard]] const VersionedProductionInterpretation&
        interpretation() const noexcept;

    [[nodiscard]] const SourceCaptureId& sourceCaptureId() const noexcept;
    [[nodiscard]] const ProvenanceItemId& provenanceItemId() const noexcept;
    [[nodiscard]] const AdmissibilityDecisionId&
        admissibilityDecisionId() const noexcept;
    [[nodiscard]] const InterpretationDecisionId&
        interpretationDecisionId() const noexcept;

    [[nodiscard]] std::size_t sourceNodeId() const noexcept;
    [[nodiscard]] std::size_t targetNodeId() const noexcept;
    [[nodiscard]] std::uint64_t relationshipGeneration() const noexcept;
    [[nodiscard]] std::uint64_t stateVersion() const noexcept;

private:
    ProductionBridgePolicyEvidence(
        PolicyEvidenceDecisionId,
        BridgePolicyEvidence,
        VersionedProductionInterpretation) noexcept;

    friend class ProductionBridgePolicyEvidenceEvaluator;
};
```

The wrapper owns/copies the accepted D8D interpretation rather than accepting
caller-supplied lineage fields independently. Convenience accessors project
lineage from that owned trusted interpretation. This avoids constructor-level
possibilities for mixed IDs, generation, or stateVersion.

### 31.3 D9A evaluator result

```cpp
enum class ProductionPolicyEvidenceReason : std::uint8_t {
    UpstreamLineageInconsistent,
    EvidenceInvariantViolation,
    InternalDeterministicEvaluationFailure
};

class ProductionPolicyEvidenceRejection final {
    // restricted-origin rejection carrying the attempted D8D decision identity
    // and a reason/flags representation
};

using ProductionPolicyEvidenceResult =
    std::variant<
        ProductionBridgePolicyEvidence,
        ProductionPolicyEvidenceRejection>;

class ProductionBridgePolicyEvidenceEvaluator final {
public:
    [[nodiscard]] std::optional<ProductionPolicyEvidenceResult> evaluate(
        const VersionedProductionInterpretation& interpretation) const;
};
```

`std::nullopt` is reserved for inability to establish the required restricted-
origin decision identity/internal production precondition, matching the
fail-closed style already used by upstream production stages.

The evaluator has no policy arguments. It must use accepted
`AdaptiveBridgePolicy::evaluate(observation, confidence)` exactly; allowing a
caller-selected D9A policy object would create an unauthorized semantic input
that D9A does not own.

### 31.4 Persistence policy identities

Candidate restricted-origin identities:

```cpp
class PersistenceProfileId final { /* opaque 128-bit identity */ };
class PersistencePolicySnapshotId final { /* opaque 128-bit publication ID */ };

struct PersistencePolicyDescriptor final {
    PersistenceProfileId profileId;
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint8_t implementationRevisionKind;
    std::array<std::uint8_t, 32> implementationRevision;
};
```

Candidate trusted snapshot:

```cpp
class PersistencePolicySnapshot final {
public:
    [[nodiscard]] const PersistencePolicySnapshotId& snapshotId() const noexcept;
    [[nodiscard]] const PersistencePolicyDescriptor& descriptor() const noexcept;
    [[nodiscard]] double activationThreshold() const noexcept;
    [[nodiscard]] double releaseThreshold() const noexcept;
    [[nodiscard]] std::size_t activationSamples() const noexcept;
    [[nodiscard]] std::size_t releaseSamples() const noexcept;

private:
    // private validated constructor
    friend class ProductionPersistencePolicyProvider;
};

class ProductionPersistencePolicyProvider final {
public:
    [[nodiscard]] static std::optional<PersistencePolicySnapshot>
        createCurrent();
};
```

No public API accepts raw threshold/sample values to create a trusted production
snapshot. The v1 provider owns the candidate 0.50/0.25/2/2 production choice.

### 31.5 Semantic stream key

Candidate value:

```cpp
class ProductionPersistenceStreamKey final {
public:
    // read-only accessors for every exact field fixed in section 8
    friend bool operator==(
        const ProductionPersistenceStreamKey&,
        const ProductionPersistenceStreamKey&) noexcept = default;

private:
    // private constructor from trusted D9A evidence + persistence descriptor
    friend class ProductionBridgePersistenceRegistry;
};
```

The key is not accepted from arbitrary caller fields when creating a trusted
stream. It is derived from:

- the trusted relationship identity/generation carried by D9A evidence;
- the exact D8D interpretation semantic descriptor;
- the exact trusted D9 persistence descriptor.

### 31.6 Stream instance identity

```cpp
class PersistenceStreamInstanceId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;
    [[nodiscard]] const Bytes& bytes() const noexcept;

private:
    explicit PersistenceStreamInstanceId(Bytes) noexcept;
    friend class ProductionBridgePersistenceRegistry;
};
```

No public default/raw-byte constructor. Every concrete stream creation mints a
new ID, including recreation of a byte-identical semantic key.

### 31.7 Trusted creation snapshot

D9 must not synthesize the stream epoch from unrelated reads.

Candidate trusted runtime view:

```cpp
struct PersistenceStreamCreationSnapshot final {
    std::size_t sourceNodeId;
    std::size_t targetNodeId;
    std::uint64_t relationshipGeneration;
    std::uint64_t stateVersion;
};
```

This plain representation is not itself sufficient authority to create a stream.
The registry obtains the values through a restricted runtime binding/factory that
captures them atomically under the mesh consistency boundary.

The registry must verify:

```text
snapshot.source/target == stream key source/target
snapshot.relationshipGeneration == stream key relationshipGeneration
```

before stream publication.

### 31.8 Stateful stream ownership

Candidate type:

```cpp
class ProductionBridgePersistenceStream final {
public:
    ProductionBridgePersistenceStream(
        const ProductionBridgePersistenceStream&) = delete;
    ProductionBridgePersistenceStream& operator=(
        const ProductionBridgePersistenceStream&) = delete;

    [[nodiscard]] const PersistenceStreamInstanceId&
        instanceId() const noexcept;
    [[nodiscard]] const ProductionPersistenceStreamKey&
        key() const noexcept;
    [[nodiscard]] std::uint64_t streamStartStateVersion() const noexcept;

    [[nodiscard]] ProductionPersistenceObservationResult observe(
        const ProductionBridgePolicyEvidence& evidence);

private:
    // private construction only by registry
    // owns mutex, BridgePersistence, de-duplication set, ordering state
    friend class ProductionBridgePersistenceRegistry;
};
```

The stream object is stateful infrastructure, not a freely copyable value. A
move operation may be omitted entirely in v1 to keep mutex/address/lifecycle
identity stable.

### 31.9 Observation result

Candidate rejection enum:

```cpp
enum class ProductionPersistenceReason : std::uint8_t {
    WrongRelationship,
    WrongRelationshipGeneration,
    WrongInterpretationPolicy,
    WrongPersistenceProfile,
    PreStreamEpochStateVersion,
    DuplicateSourceCapture,
    NonIncreasingStateVersion,
    LineageInconsistent,
    EvidenceInvariantViolation,
    StreamClosed,
    InternalPersistenceFailure
};
```

Candidate immutable success record:

```cpp
class ProductionPersistentBridgeRecommendation final {
public:
    [[nodiscard]] const RecommendationEventId& eventId() const noexcept;
    [[nodiscard]] const PersistenceStreamInstanceId&
        streamInstanceId() const noexcept;
    [[nodiscard]] const ProductionPersistenceStreamKey& streamKey() const noexcept;
    [[nodiscard]] PersistentBridgeRecommendation recommendation() const noexcept;
    [[nodiscard]] const PolicyEvidenceDecisionId&
        policyEvidenceDecisionId() const noexcept;
    [[nodiscard]] const SourceCaptureId& sourceCaptureId() const noexcept;
    [[nodiscard]] std::uint64_t stateVersion() const noexcept;

private:
    // private constructor; only a validated stream transition may mint this
    friend class ProductionBridgePersistenceStream;
};

class ProductionPersistenceRejection final {
    // immutable stream-instance identity, attempted evidence identity,
    // primary reason + reason flags; restricted-origin
};

using ProductionPersistenceObservationResult =
    std::variant<
        ProductionPersistentBridgeRecommendation,
        ProductionPersistenceRejection>;
```

A successful observation emits one immutable recommendation event even when the
enumerated recommendation value did not change. This preserves the exact
accepted sample/event lineage without implying a topology transition.

### 31.10 Registry/factory boundary

Candidate owner:

```cpp
class ProductionBridgePersistenceRegistry final {
public:
    [[nodiscard]] std::optional<ProductionPersistenceStreamHandle>
        openLiveStream(
            const ProductionBridgePolicyEvidence& seedLineage,
            const PersistencePolicySnapshot& policy);

    // explicit close/reset operations require a concrete instance identity;
    // they do not accept only the semantic key.
};
```

The exact handle representation remains an implementation design choice, but v1
must satisfy:

- registry owns stream lifetime;
- at most one live instance per exact semantic key;
- lookup by semantic key cannot silently return a closed instance;
- close/reset targets a `PersistenceStreamInstanceId`;
- stream creation obtains its own fresh coherent runtime snapshot;
- `seedLineage` establishes relationship/policy identity only and is not itself
  counted as an observation;
- no `BridgePersistence::observe()` occurs during stream creation;
- stream creation does not emit a recommendation.

This avoids an off-by-one persistence bug where the evidence used to identify
and open a stream would accidentally become activation sample #1.

### 31.11 Restricted-origin compile-fail contract

Before runtime acceptance, compile-fail tests must prove arbitrary callers cannot:

1. default-construct or raw-byte construct `PolicyEvidenceDecisionId`;
2. directly construct `ProductionBridgePolicyEvidence`;
3. directly construct a trusted `PersistencePolicySnapshot`;
4. raw-byte/default construct `PersistenceStreamInstanceId`;
5. construct a trusted stream from a caller-created semantic key/snapshot;
6. copy a `ProductionBridgePersistenceStream`;
7. directly construct `ProductionPersistentBridgeRecommendation`;
8. turn a recommendation into `RequestedTransitionDirection` through any D9
   API;
9. invoke raw `BridgePersistence` through the D9 production surface;
10. supply caller-selected activation/release values to a trusted v1 stream.

### 31.12 API-level atomicity rule

For an accepted observation, the following state transition is one serialized
critical section:

```text
validate stream identity/epoch/order/de-duplication
-> BridgePersistence::observe()
-> record SourceCaptureId
-> advance lastAcceptedStateVersion
-> mint immutable recommendation event
```

If a pre-`observe()` validation fails, none of the state changes occur.

Because current `BridgePersistence::observe()` is `noexcept`, mutation after
the validation boundary can be designed so allocation/event preparation that
may fail occurs before mutating the domain persistence state. The implementation
design must preserve strong fail-closed semantics rather than allowing a
successful internal `observe()` followed by failure to record its replay/order
metadata.

---

## 32. Remaining gates before implementation

The earlier stream-key, anti-replay, epoch, stream-instance and policy-value
design questions are now resolved at design-contract level.

Before D9 implementation:

1. critically review this exact C++ API ownership surface;
2. resolve the concrete stream-handle/lifetime representation;
3. resolve allocation strategy needed for strong atomic observation semantics;
4. define exact recommendation/rejection event-ID types and reason-flag layout;
5. define the restricted runtime binding used for coherent stream-creation
   snapshots;
6. perform final D9 API design acceptance review;
7. only then implement on a separate candidate branch.
