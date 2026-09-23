# SOAM 2.0 — Evidence Inventory v1

**Document version:** Evidence Inventory v1  
**Status:** draft consolidated baseline / internally consistency-reviewed  
**Ratification:** not performed  
**Repository mutation:** documentation-only candidate on review branch  
**Scope:** through C1 eligibility  
**Terminal state:** eligible_for_authority_consideration  
**Outside v1:** Authority, capability issuance, execution, commit-time mutation

---

## §0. Conventions and boundary discipline

### §0.0 Boundary discipline invariant

Presence, possession, or construction capability of an artifact does not confer authority to trust, accept, act on, or derive privileged state from it. Such authority is introduced only by a distinct boundary with its own contract.

### §0.1 Invariant forms

#### Identity / authenticity

- identity != credential
- event identity != content authenticity
- event identity != logical intent identity
- typed identity != global byte-space uniqueness
- temporal sample identity != later wrapper/evaluation identity

#### Authority / admissibility

- parsed artifact != trusted artifact
- semantic state present in a type != authority to produce it
- caller-chosen policy != trusted production policy
- envelope possession != authority to assert admissibility
- domain recommendation != production-bound recommendation
- projected transition != executed transition
- reserved inactive seam != active integration

Reserved inactive seam means a private access type may exist as a declared future integration boundary without any current production path using it. Presence of the seam does not imply integration, authority, or active use.

Observed examples:
- Domain -> detail::ProductionPersistenceAccess
- C1 -> detail::ProductionAuthorityDerivationAccess

#### Declaration / reachability

- declared taxonomy != produced taxonomy
- declared field polarity range != materialized polarity range
- workflow / job count != coverage
- absence of evidence != negative evidence

Taxonomy reachability levels are distinct:

- declared reason
- emission branch implemented
- production-reachable from legitimate restricted-origin inputs

#### Lineage / representation

- nested-by-value lineage != projected lineage
- projection mode is not monotone
- local self-containment != restored historical self-containment
- restricted-origin container != independently identified artifact
- nested lineage != flat serialized lineage
- information-preserving computation != authority-preserving conversion
- snapshot surface != consistency-lock domain

#### Decision / evidence semantics

- evaluation outcome != artifact polarity
- decision completeness != evidence satisfaction
- negative variant arm != semantic error
- decision-sufficient evidence != explanation-sufficient evidence
- decision value != decision event identity
- multiple unsatisfied inputs != multiple published rejection reasons

A terminal gate may publish one primary reason even when multiple inputs are negative. Fixed precedence is a semantic choice, not an aggregation failure. Consumers wanting full diagnostics must query per-channel evidence.

#### Governance / lifecycle

- implemented != ratified
- design-document lifecycle != runtime acceptance lifecycle
- historical provenance state != current repository state
- PR history != CI evidence
- same policy family != same semantic derivation contract

Several forms above are instances of the broader idea "representation of a state != establishment of that state"; that broader phrase is explanatory, not an additional canonical form.

### §0.2 Layer roles, outcome algebra, STOP semantics

Layer roles:

- construction layer
- decision-bearing refinement layer
- early terminal gate
- composite evidence decision
- projection adapter
- stateful persistence layer
- total deterministic decision gate

Outcome algebras:

- construction: optional<Artifact>
- decision-bearing refinement: optional<variant<PositiveArtifact, TypedRejection>>
- early terminal gate: plain enum, no optional, no infrastructure-failure path
- composite evidence decision: optional<variant<CompositeEvidence, TypedRejection>>
- projection adapter: optional<ProjectionContainer>
- stateful persistence: optional<variant<PositiveArtifact, TypedRejection>>
- total deterministic decision gate: DecisionObject; no optional; no variant; no infrastructure-failure path

Type-shape rule:

type shape != layer role

The same C++ generic shape may serve different semantic roles; the role is defined by the contract.

STOP types:

- STOP (artifact)
- STOP (decision)
- STOP (decision + positive artifact)
- STOP (composite evidence decision)
- STOP (projection)
- STOP (integration boundary)

D7 is an early terminal gate with a plain enum. C1 is a total deterministic decision gate with an immutable decision object. Both terminate as STOP (decision).

### §0.3 Evidence interpretation / assurance vocabulary

- **implemented**: code or artifact present in accepted main.
- **tested**: executable tests ran against a specific commit SHA, and that exact tested implementation reached accepted main.
- **CI-validated at exact head**: a specified workflow succeeded for the indicated SHA.
- **verified**: not used without qualification of verification kind.
- **formally verified**: mathematical or machine-checked proof; out of scope for v0.

Rules:

- Successful CI proves only exercised properties.
- PR history != CI evidence. Canonical CI evidence binds to the exact validated implementation head, not a PR branch name, final PR head, or merge commit.
- Declared rejection taxonomy != produced rejection taxonomy.
- Decision-sufficient evidence != explanation-sufficient evidence.
- Workflow/job count != coverage.

### §0.4 Cross-layer temporal / lineage / epoch rules

Live snapshot surfaces on impl_->topologyMutex:

- D7 — transition gate read
- D8A — source capture
- A1 — live-validity snapshot
- B1 — invariant snapshot
- C — topology snapshot

Each surface exposes a different schema for a different layer contract. All use shared_lock on the same topology consistency domain. No surface is a substitute for another.

same consistency domain != same temporal state

Freshness families:

- live evidence-polarity: A1
- live trusted-context: B1, C
- no live check: D; exact signed request-epoch binding only

Lineage modes:

- nested-by-value: D8A -> D9A
- projection: D9B -> request -> A2
- local-embed: A1

Runtime epoch guarantees are three distinct contracts:

- L1 same request binding — verified by C1.
- L2 cross-channel atomic snapshot — not implemented; A1/B1/C are separate reads.
- L3 current-at-decision / commit-time currentness — delegated to Authority/Execution and must be atomic with mutation.

same request epoch binding != cross-channel atomic snapshot != current-at-decision state

Prerequisite eligibility != commit-time revalidation.

C1 establishes bound-epoch prerequisite satisfaction. It does not establish current runtime state at decision time. Final current-state validation is delegated to Authority/Execution and must be atomic with execution.

Absence of L2 != delegation. L3 is a delegation.

Infrastructure-failure publication is layer-specific, not globally normalized.

A reserved or established decision identity does not imply one uniform failure-publication rule.

Observed behaviors include:
- nullopt before a decision exists
- nullopt after a decision ID has been reserved
- typed infrastructure rejection carrying the established ID
- typed output-invariant rejection

Per-layer failure semantics are recorded in dossiers and summarized in §18.

Request -> A1 retry/freeze/staleness contract:
- no automatic retry
- no runtime freeze
- no bounded staleness window
- freshness is strict stateVersion equality

Canonical encoding conventions may differ per protocol domain:

- D8B provenance envelope: big-endian fixed-width canonical encoding
- D permission attestation: little-endian fixed-width canonical encoding

Cross-domain tooling must not assume a single repository-wide endianness. Frozen test vectors are convention-specific.

---

## §1. System topology

    DOMAIN SEMANTICS
    InteractionObservation + BridgeConfidence
          |
          v
    AdaptiveBridgePolicy
          |
          v
    BridgePolicyEvidence
          |
          v
    BridgePersistence
          |
          v
    PersistentBridgeRecommendation


    LIVE RUNTIME — SpatialAdaptiveMesh
          |
          +-- D7  fail-closed transition gate          STOP (decision)
          |
          +-- D8A source capture                       STOP (artifact)
                  |
                  v
                  D8B provenance envelope              STOP (artifact)
                  |
                  v
                  D8C provenance admissibility         STOP (decision + positive artifact)
                  |
                  v
                  D8D versioned interpretation         STOP (decision + positive artifact)
                  |
                  v
                  D9A policy evidence                  STOP (decision + positive artifact)
                  |
                  v
                  D9B persistence / recommendation     STOP (decision + positive artifact)
                  |
                  v
                  Transition Request Derivation        STOP (decision + positive artifact)
                  |
                  v
                  ProductionDerivedTransitionRequest
                  |
                  v
            prerequisite fan-out

              A1                 B1                 C                  D
               |                  |                  |                  |
               v                  v                  v                  v
          A1 evidence         B1 record          C record        crypto slice only
               |                  |                  |                  |
               A2               [gap]              [gap]          [channel gap]
               |                                                       |
               |                                                  [adapter gap]
               +------------------+------------------+
                                  |
                           [assembler gap]
                                  |
                                  v
                   ProductionTransitionPrerequisiteSet
                                  |
                                  v
                                  C1
                                  |
                                  v
                   eligible_for_authority_consideration

    -------------------- Authority / Execution --------------------
    outside v1; L3 commit-time revalidation delegated here

Current implemented reach:

Individual accepted components exist from D8A through C1, but no end-to-end production path from request derivation to C1 is currently reachable.

Blocking integration boundaries:
- B1 -> C1 adapter
- C -> C1 adapter
- complete D evaluator
- D -> C1 adapter
- prerequisite assembler

---

## §2. Normalized inventory index

| # | Layer | Role | Outcome algebra | Lineage | Live read | STOP | Integration | Design / runtime |
|---|---|---|---|---|---|---|---|---|
| 1 | Domain | foundational primitives | n/a | recommendation only | no | recommendation only | partial | runtime merged |
| 2 | D7 | early terminal gate | plain enum | terminal | topologyMutex | decision | negative gate integrated | runtime merged |
| 3 | D8A | construction | optional<Artifact> | root capture | topologyMutex | artifact | yes | runtime merged |
| 4 | D8B | construction | optional<Artifact> | nested-by-value | no | artifact | partial | runtime merged |
| 5 | D8C | decision-bearing | optional<variant<...>> | nested-by-value | no | decision + positive artifact | partial | runtime merged |
| 6 | D8D | decision-bearing derivation | optional<variant<...>> | nested-by-value | no | decision + positive artifact | partial | runtime merged |
| 7 | D9A | decision-bearing | optional<variant<...>> | nested-by-value | no | decision + positive artifact | partial | design/runtime merged |
| 8 | D9B | stateful persistence + projection | optional<variant<...>> | projection | lifecycle-only | decision + positive artifact | partial | design/runtime merged |
| 9 | Request | decision-bearing refinement | optional<variant<...>> | projection | no | decision + positive artifact | partial | design/runtime merged |
| 10 | A1 | composite evidence decision | optional<variant<CompositeEvidence, Rejection>> | local-embed | topologyMutex | composite evidence decision | partial | design/runtime merged |
| 11 | A2 | projection adapter | optional<ProjectionContainer> | projection | no | projection | partial | design open / runtime merged |
| 12 | B1 | prerequisite channel | optional<variant<...>> | projection / local evaluation context | topologyMutex | decision + positive artifact | partial | design/runtime merged |
| 13 | C | prerequisite channel | optional<variant<...>> | projection / local evaluation context | full topology under topologyMutex | decision + positive artifact | partial | design/runtime merged |
| 14 | D | permission verifier slice | current parser optional<T> + verifier bool; target optional<variant<...>> | n/a | no | integration boundary now | no beyond crypto slice | design merged unratified / runtime slice merged |
| 15 | C1 | total deterministic eligibility gate | immutable DecisionObject | projection | no | decision | evaluator complete; pipeline partial | historical design/runtime merged |

C1 prerequisite assembly status:

- Freshness: ready via A2
- Revalidation: ready via A2
- Invariant: B1 record exists; adapter missing
- Resilience: C record exists; adapter missing
- Permission: channel incomplete; adapter designed but not implemented

Therefore full prerequisite assembly is currently not reachable.

---

## §3. Domain Core

**Module:** apps/soam-domain  
**Type:** C++20 INTERFACE library  
**Role:** foundational primitives, not a pipeline stage

Input:
- InteractionObservation
- BridgeConfidence

Output:
- PersistentBridgeRecommendation { PRESERVE, CONSTRAIN, SUPPORT }

Friend semantics:
- active structural: AdaptiveBridgePolicy -> BridgePolicyEvidence
- reserved inactive: detail::ProductionPersistenceAccess -> BridgePersistence

The reserved seam is not used by the current production persistence path.

Implemented: yes  
Tested: yes  
PR: #3  
CI: SOAM 2.0 Domain Core Validation

Guaranteed:
- finite/range checks
- explicit scalar construction
- restricted BridgePolicyEvidence construction
- tested mapping, bounds, symmetry
- persistence activation/release/reversal/reset
- tested instance independence

Not claimed:
- exhaustive coverage
- concurrency
- zero-evidence origin distinguishability
- provenance authenticity
- request / permission / eligibility / authority / execution
- formal verification

STOP: PersistentBridgeRecommendation only  
Integration: partial

---

## §4. D7 — Provenance Gate

**Module:** apps/soam-runtime  
**Header:** include/production_transition_evaluator.hpp  
**Source:** src/production_transition_evaluator.cpp

Role: fail-closed live transition gate.

Input: ProductionRelationshipLocator

Output:
- no_request
- not_eligible
- eligible_for_authority_consideration

Current evaluator emits only no_request and not_eligible. The positive enum arm exists in the type surface but is not produced by the current evaluator.

The local TransitionSnapshot is ephemeral, not persisted, and not handed to D8A.

Construction:
- ProductionTransitionEvaluator is not default-constructible
- obtained from SpatialAdaptiveMesh
- lifetime protected through binding state + lease

Implemented: yes  
Positive D7 path: evidence-blocked  
PR: #7  
CI: SOAM 2.0 D7 Provenance Gate Validation

Guaranteed:
- missing relationship -> no_request
- invalid/lost binding -> not_eligible
- snapshot revalidation failure -> not_eligible
- current Phase C2 remains fail-closed
- caller cannot inject production-native observation/confidence provenance

Not claimed:
- exhaustive branch coverage
- positive eligibility path
- authority / execution

STOP: STOP (decision)  
Integration: yes, as negative gate

---

## §5. D8A — Source Capture

**Module:** apps/soam-runtime  
**Role:** coherent raw relationship-state capture from live mesh

Input: ProductionRelationshipLocator  
Output: ProductionRelationshipSourceSnapshot

Identity:
- SourceCaptureId
- opaque 128-bit OS-backed event identity
- reserved before topology lock
- identifies a capture attempt reaching successful ID reservation, not logical intent
- repeated retry is a new event identity

Value model:
- immutable restricted-origin object
- copyable/movable
- independent of mesh lifetime

Guaranteed:
- coherent capture under topology locking
- non-zero ID
- recent duplicate rejection/retry
- fail-closed on RNG failure
- finite retry exhaustion
- repeated capture gets distinct event identity

Not claimed:
- global uniqueness
- cross-domain byte-space uniqueness
- cross-restart persistence
- authenticated provenance
- transactional coupling to later snapshots

STOP: STOP (artifact)  
Integration: yes

---

## §6. D8B — Provenance Envelope

**Module:** apps/soam-runtime  
**Header:** include/production_provenance_envelope.hpp  
**Source:** src/production_provenance_envelope.cpp

Role: materialize an independent provenance-bearing artifact.

Input: ProductionRelationshipSourceSnapshot  
Output: ProductionProvenanceEnvelope

Identity:
- ProvenanceItemId
- preserves SourceCaptureId exactly
- D8A and D8B IDs use independent generators

Lineage:
- source facts embedded by value
- no mesh handle retained

Canonical encoding:
- big-endian fixed-width integers
- exact IEEE-754 binary64 bit patterns
- see D permission attestation for contrast: D uses little-endian

Revision:
- implementationRevisionKind = 1
- deterministic manifest hash, not a git SHA/version identity

Live read: none

Guaranteed:
- deterministic canonical encoding for accepted V1 contract
- SHA-256 content binding
- restricted construction
- fail-closed producer/canonicalization/digest/manifest paths exercised by tests

Not claimed:
- source truth
- authenticated producer origin
- admissibility
- interpretation
- cross-restart persistence
- formal verification

STOP: STOP (artifact)  
Integration: partial

---

## §7. D8C — Provenance Admissibility

Role: decision-bearing admissibility refinement.

Input:
- ProductionProvenanceEnvelope
- ProvenanceAdmissibilityPolicySnapshot
- SourceEvidenceResolver

Output:
- AdmissibleProductionProvenance
- or ProvenanceAdmissibilityRejection

Outcome: optional<variant<...>>

Identity:
- AdmissibilityDecisionId
- reserved before semantic gates
- published in typed rejection

Checks include:
- canonical digest
- producer/revision
- schema/canonical encoding
- dependency status
- retained-source bit-exact consistency

Live read: none  
Freshness: not evaluated

Declared taxonomy includes PolicyUnavailable, metadata/digest/producer/schema/dependency/source-resolution failures.

Current evaluator has implemented emission branches for the semantic rejection paths it evaluates. PolicyUnavailable is declared but not produced by evaluate; provider failure occurs before evaluator entry.

Infrastructure:
- failure to generate decision ID -> nullopt
- after decision ID exists, semantic failures -> typed rejection
- current evaluator has no explicit catch-to-nullopt wrapper around final positive construction

Not claimed:
- source truth
- producer cryptographic authentication
- cross-restart retained evidence
- live freshness
- authority / execution

STOP: STOP (decision + positive artifact)  
Integration: partial

---

## §8. D8D — Versioned Interpretation

Role: derivation layer producing new semantic content.

Input:
- AdmissibleProductionProvenance
- InterpretationPolicySnapshot

Output:
- VersionedProductionInterpretation
- or ProductionInterpretationRejection

Outcome: optional<variant<...>>

Derivation:
- attenuation = 1 / (1 + 0.1 * distance)
- compatibility = capacity * attenuation * orientationWeight
- confidence = min(sourceHealth, targetHealth)

Own identity: InterpretationDecisionId  
Positive lineage: nested-by-value admitted provenance

Declared but not produced in current implementation:
- PolicyUnavailable
- CompatibilityComputationInvalid
- ConfidenceComputationInvalid

Produced:
- PolicyRevisionUnrecognized
- UpstreamLineageInconsistent
- DistanceInvalid
- OrientationWeightInvalid
- CapacityInvalid
- SourceHealthInvalid
- TargetHealthInvalid
- OutputInvariantViolation
- InternalDeterministicEvaluationFailure

D8D-specific failure semantics:
- computeInterpretationV1() returns no value -> InternalDeterministicEvaluationFailure
- exception while materializing positive interpretation -> OutputInvariantViolation
- decision-ID generation failure -> nullopt

Live read: none

Not claimed:
- physical truth of mapping
- live freshness
- recommendation / eligibility / authority / execution
- formal refinement

STOP: STOP (decision + positive artifact)  
Integration: partial

---

## §9. D9A — Policy Evidence

Role: adaptation + lineage binding, not a new live derivation.

Input: VersionedProductionInterpretation  
Output: ProductionBridgePolicyEvidence or ProductionPolicyEvidenceRejection

Computation:
- passes observation + confidence into AdaptiveBridgePolicy::evaluate()
- no renormalization/clamping
- no live read

Identity: PolicyEvidenceDecisionId  
Lineage: nested-by-value

Declared reasons:
- UpstreamLineageInconsistent
- EvidenceInvariantViolation
- InternalDeterministicEvaluationFailure

Current evaluator produces the first two. Final positive construction exception returns nullopt; the InternalDeterministicEvaluationFailure enum arm is not produced by the current implementation path.

STOP: STOP (decision + positive artifact)  
Integration: partial

---

## §10. D9B — Persistence / Recommendation

Role: stateful persistence/hysteresis + lineage projection.

Input:
- ProductionBridgePolicyEvidence
- ProductionPersistenceStreamKey
- live relationship lifecycle state

Output:
- ProductionPersistentBridgeRecommendation
- or ProductionPersistenceRejection

Outcome: optional<variant<...>>

Stream state includes:
- PersistenceStreamInstanceId
- key
- policy snapshot
- BridgePersistence
- stream start stateVersion
- last accepted stateVersion
- accepted SourceCaptureIds

Policy V1:
- activationThreshold 0.50
- releaseThreshold 0.25
- activationSamples 2
- releaseSamples 2

Persistence here means in-process stream-state continuity/hysteresis, not durable storage.

Live read:
- lifecycle-only
- relationship existence/generation coherence
- no semantic recomputation

Anti-replay:
- SourceCaptureId counts once per stream
- accepted stateVersion must strictly increase
- pre-stream epoch rejected

Lineage: selected projection, not full nested lineage.

Declared reasons include WrongRelationship, WrongRelationshipGeneration, WrongInterpretationPolicy, WrongPersistenceProfile, PreStreamEpochStateVersion, DuplicateSourceCapture, NonIncreasingStateVersion, LineageInconsistent, EvidenceInvariantViolation, StreamClosed, InternalPersistenceFailure.

For StreamClosed and InternalPersistenceFailure, direct production reachability is not independently established in the current path.

Infrastructure/allocation failure paths may return nullopt.

Not claimed:
- durable storage
- cross-restart continuity
- cross-revision migration
- bounded stream registry size
- full historical audit self-containment
- eligibility / authority / execution

STOP: STOP (decision + positive artifact)  
Integration: partial

---

## §11. Transition Request Derivation

**Module:** apps/soam-transition-request

Role: decision-bearing refinement.

Input:
- ProductionPersistentBridgeRecommendation
- TransitionRequestPolicySnapshot

Output:
- ProductionDerivedTransitionRequest
- or ProductionTransitionRequestDerivationRejection

Mapping:
- SUPPORT -> support
- CONSTRAIN -> constrain
- PRESERVE -> completed terminal no-request using PreserveRecommendation

PRESERVE is a semantic terminal no-request, not an infrastructure error.

Class:
- BridgeCouplingAdjustmentV1
- 0x4252494447455631
- design mnemonic BRIDGEV1

Binding:
- exact relationship identity
- generation
- requested direction
- transition class
- exact source stateVersion

Identity: TransitionRequestDecisionId

The request retains sourceRecommendation and binding.direction as an intentional cross-layer integrity witness.

Declared reasons:
- PreserveRecommendation
- RecommendationLineageInconsistent
- PolicyRevisionUnrecognized
- InternalDeterministicDerivationFailure

Current evaluator produces the first three. Final positive construction exception returns nullopt; InternalDeterministicDerivationFailure is not produced by the current implementation path.

Live read: none

Not claimed:
- live freshness
- prerequisite satisfaction
- eligibility / authority / execution

STOP: STOP (decision + positive artifact)  
Integration: partial

---

## §12. A1 — Live Validity

**Module:** apps/soam-transition-live-validity

Role: composite evidence decision.

Input:
- ProductionDerivedTransitionRequest
- ProductionTransitionLiveSnapshotSource

Output:
- ProductionTransitionLiveValidityEvidence
- or ProductionTransitionLiveValidityRejection

Outcome: optional<variant<CompositeEvidence, TypedRejection>>

Identity:
- parent LiveValidityDecisionId
- child FreshnessDecisionId
- child RevalidationDecisionId
- parent is one completed evaluation event
- children are independently addressable evidence records
- partial child publication is not part of the outcome algebra

Live read:
- one coherent live-validity snapshot
- shared topology consistency domain
- separate temporal acquisition from D8A/B1/C

Freshness:
- live stateVersion == request stateVersion
- mismatch is valid evidence with freshness.satisfied=false
- evidence-polarity family

Revalidation:
- relationship present
- observed generation == request generation

Taxonomy:
- RequestLineageInconsistent
- InvalidRelationshipIdentity
- SnapshotUnavailable
- InternalDecisionFailure

Implemented emission branches exist for all four. Legitimate restricted-origin production reachability is not independently established for every branch.

InternalDecisionFailure is reachable through child-ID generation failure after the parent ID exists, and through failure to materialize the final composite.

InvalidRelationshipIdentity has an emission branch; upstream restricted construction may prevent a legitimate source==target request from reaching A1.

Infrastructure:
- parent-ID generation failure -> nullopt
- child-ID failure after parent exists -> typed InternalDecisionFailure
- final composite construction failure -> typed InternalDecisionFailure

Not claimed:
- staleness tolerance
- runtime freeze
- automatic retry/re-derivation
- eligibility / authority / execution

STOP: STOP (composite evidence decision)  
Integration: partial

---

## §13. A2 — Live Validity -> C1 Adapter

**Module:** apps/soam-transition-live-validity-c1

Role: bounded-context / construction-authority adapter; lossy diagnostic projection.

Input: ProductionTransitionLiveValidityEvidence  
Output: ProductionLiveValidityC1Prerequisites

Outcome: optional<ProjectionContainer>

Identity:
- no new identity
- preserves LiveValidityDecisionId, FreshnessDecisionId, RevalidationDecisionId in wrapper
- C1 evidence preserves exact request binding and satisfied polarity

C1 outputs:
- FreshnessPrerequisiteEvidence { context, satisfied }
- RevalidationPrerequisiteEvidence { context, satisfied }

Drops:
- full ProductionDerivedTransitionRequest
- observedStateVersion
- relationshipPresent
- observedRelationshipGeneration
- broader upstream lineage

Live read: none  
Policy: none  
New decision: none

nullopt means projection binding-coherence invariant could not be established, not semantic negative evidence.

Design governance:
- runtime PR #26 merged
- design PR #25 remained open at the reviewed snapshot
- therefore implemented != ratified/design-anchored

STOP: STOP (projection)  
Integration: partial

---

## §14. B1 — Transition Invariant Evidence

Role: decision-bearing prerequisite evaluation.

Input:
- ProductionDerivedTransitionRequest
- TransitionInvariantPolicySnapshot
- trusted runtime invariant snapshot source

Output:
- ProductionInvariantPrerequisiteRecord
- or ProductionTransitionInvariantRejection

Outcome: optional<variant<...>>

Identity: InvariantDecisionId

Policy V1:
- class-specific
- BridgeCouplingAdjustmentV1
- adjustmentFraction = 0.125
- exact implementation revision

Projection:
- SUPPORT: c' = c + (1-c)*0.125
- CONSTRAIN: c' = c*0.875

Projected transition != executed transition.

Live read:
- semantic/projection inputs
- same topology consistency domain
- separate temporal acquisition

Freshness:
- strict request stateVersion equality
- mismatch -> RequestStateVersionMismatch typed rejection
- trusted-context family

Positive record may have satisfied=true or satisfied=false.

structuralRelationshipInvariantSatisfied is a declared diagnostic field whose current positive-arm materialization range is {true}; structural failures are routed to typed rejection before a positive record is constructed. false is not claimed permanently impossible or formally reserved.

Declared reasons:
- RequestLineageInconsistent
- TransitionClassUnsupported
- PolicyRevisionUnrecognized
- SnapshotUnavailable
- RequestStateVersionMismatch
- RelationshipIdentityMismatch
- ProjectionUnavailable
- InternalDecisionFailure

All except InternalDecisionFailure have current emission paths. Direct production reachability of InternalDecisionFailure is not established in the current implementation path.

No B1 -> C1 adapter exists in the reviewed repository state.

STOP: STOP (decision + positive artifact)  
Integration: partial

---

## §15. C — Transition Resilience Evidence

Role: decision-bearing prerequisite evaluation.

Input:
- ProductionDerivedTransitionRequest
- TransitionResiliencePolicySnapshot
- trusted runtime topology snapshot

Output:
- ProductionResiliencePrerequisiteRecord
- or ProductionTransitionResilienceRejection

Outcome: optional<variant<...>>

Identity: ResilienceDecisionId

Criterion:
- single_request_relationship_loss_survivability

Method:
- exclude exact request-bound logical relationship
- build graph from operational relationships
- BFS for an alternate path between the same endpoints

Operational pair requires:
- structurally valid reciprocal pair
- both directions non-ISOLATED
- capacity > 0 both directions

Policy V1:
- class-specific
- BridgeCouplingAdjustmentV1
- exact implementation revision

Live read:
- full topology semantic snapshot
- same topology consistency domain
- separate temporal acquisition

Freshness:
- strict request stateVersion equality
- mismatch -> RequestStateVersionMismatch rejection
- trusted-context family

Outcome distinction:
- malformed/invalid topology -> typed rejection
- valid topology, no alternate path -> positive record satisfied=false

Declared reasons:
- RequestLineageInconsistent
- TransitionClassUnsupported
- PolicyRevisionUnrecognized
- SnapshotUnavailable
- RequestStateVersionMismatch
- RelationshipIdentityMismatch
- TopologySnapshotInvalid
- InternalDecisionFailure

All except InternalDecisionFailure have current emission paths. Direct production reachability of InternalDecisionFailure is not established in the current implementation path.

No C -> C1 adapter exists in the reviewed repository state.

STOP: STOP (decision + positive artifact)  
Integration: partial

---

## §16. D — Permission Evidence

**Module:** apps/soam-transition-permission

Current role: verifier-only crypto slice D1/D3.  
Target role: decision-bearing permission prerequisite evaluation.

Current input:
- raw 145-byte attestation payload
- Ed25519 public key
- Ed25519 signature

Current output:
- typed parsed payload/key/signature
- bool signature verification result

Target input:
- ProductionDerivedTransitionRequest
- trusted permission attestation
- trusted/versioned permission policy

Target output:
- ProductionPermissionPrerequisiteRecord
- or typed permission rejection

Current outcome:
- parser optional<T>
- verifier bool

Target outcome:
- optional<variant<Record, TypedRejection>>

Current runtime decision identity: none  
Design target: PermissionDecisionId

Canonical payload:
- exactly 145 bytes
- fixed-width
- little-endian integers
- 31-byte domain separator including NUL
- contrast with D8B big-endian provenance encoding

Live read: none

Freshness:
- no live freshness check
- signed exact request/stateVersion binding only
- request-epoch binding is not a live freshness probe

Current slice guarantees:
- exact-size parser
- fixed-size typed payload/key/signature
- Ed25519 verification through libsodium
- verifier-only production module
- frozen interoperability vectors
- compile-fail guards against signing/dynamic trust surfaces

Design-only rejection taxonomy:
- RequestLineageInconsistent
- TransitionClassUnsupported
- PolicyRevisionUnrecognized
- AttestationMalformed
- IssuerUnrecognized
- SignatureInvalid
- RequestBindingMismatch
- InternalVerificationFailure

Produced D evaluator taxonomy: none, because the D evaluator is not implemented.

Designed D -> C1 conversion:
- total
- non-decision-bearing
- maps record.binding -> PermissionPrerequisiteEvidence.context
- maps record.satisfied -> PermissionPrerequisiteEvidence.satisfied
- not implemented

Governance:
- design PR #32 merged as design baseline v0
- ratification not performed
- runtime PR #33 merged as verifier-only slice
- IMPLEMENTED = true for crypto slice
- PROVISIONED = false
- RATIFIED = false
- ACTIVATED = false

STOP current: STOP (integration boundary)  
STOP design target: STOP (decision + positive artifact)  
Integration: no beyond crypto slice

---

## §17. C1 — Transition Eligibility

**Module:** apps/soam-transition  
**Header:** include/production_transition_eligibility.hpp  
**Source:** header-only

Role: total deterministic eligibility gate.

Input:
- ProductionTransitionRequestBinding
- ProductionTransitionPrerequisiteSet

Prerequisite set contains optional:
- PermissionPrerequisiteEvidence
- InvariantPrerequisiteEvidence
- ResiliencePrerequisiteEvidence
- FreshnessPrerequisiteEvidence
- RevalidationPrerequisiteEvidence

Each C1 evidence object contains only:
- exact ProductionTransitionRequestBinding context
- bool satisfied

Output: ProductionTransitionEligibilityDecision

Decision fields:
- eligibility
- rejectionReason
- binding

Outcome:
- plain immutable decision object
- no optional
- no variant
- no infrastructure-failure path

Identity:
- no EligibilityDecisionId
- deterministic value identity only
- decision value != decision event identity

Positive terminal:
- eligible_for_authority_consideration

Negative:
- not_eligible + one primary reason

Live read: none  
Stateful: no  
Deterministic: yes  
noexcept: yes  
constexpr: yes

C1 verifies L1: all supplied evidence must carry the same exact request binding.

C1 does not establish:
- L2 cross-channel atomic snapshot
- L3 current runtime epoch at decision time

Declared reasons:
- none
- invalid_input
- missing_prerequisite
- binding_mismatch
- permission_not_satisfied
- invariant_not_satisfied
- resilience_not_satisfied
- freshness_not_satisfied
- revalidation_failed

Current evaluator produces all except invalid_input. Direct production reachability of invalid_input is not established because there is no emission branch.

Fixed precedence:
1. missing prerequisite
2. binding mismatch
3. permission
4. invariant
5. resilience
6. freshness
7. revalidation
8. eligible

Multiple unsatisfied inputs != multiple published rejection reasons.

C1 retains only terminal eligibility semantics, not full prerequisite audit provenance.

C1 does not assemble ProductionTransitionPrerequisiteSet; an external production assembler is required and currently missing.

STOP: STOP (decision)  
Integration: evaluator complete; end-to-end production path not reachable

---

## §18. Cross-layer findings

### 18.1 Taxonomy reachability

Use three levels:
- declared reason
- emission branch implemented
- production-reachable from legitimate restricted-origin inputs

Do not collapse these into one "produced" label where evidence is insufficient.

Examples:
- D8C PolicyUnavailable: declared; evaluator does not emit it
- D8D PolicyUnavailable / CompatibilityComputationInvalid / ConfidenceComputationInvalid: declared but not produced in current implementation
- D9A InternalDeterministicEvaluationFailure: declared; current evaluator does not emit it
- D9B StreamClosed / InternalPersistenceFailure: declared; direct production reachability not independently established
- request InternalDeterministicDerivationFailure: declared; current evaluator does not emit it
- B1/C InternalDecisionFailure: declared; direct production reachability not established
- C1 invalid_input: declared; no current emission branch
- A1: all declared reasons have implemented emission branches, but full legitimate-input production reachability is not established for every branch

### 18.2 Infrastructure-failure publication matrix

Infrastructure behavior is per-layer.

- D8C: decision-ID failure -> nullopt; semantic failures after ID -> typed rejection; no explicit final positive catch-to-nullopt path
- D8D: decision-ID failure -> nullopt; deterministic computation failure -> InternalDeterministicEvaluationFailure; final positive construction exception -> OutputInvariantViolation
- D9A: decision-ID failure -> nullopt; final positive construction exception -> nullopt
- D9B: allocation/infrastructure failures in current observation path -> nullopt
- request derivation: decision-ID failure -> nullopt; final request construction exception -> nullopt
- A1: parent-ID failure -> nullopt; child-ID/final-composite failure after parent exists -> typed InternalDecisionFailure carrying parent ID

No repository-wide publication rule is claimed.

### 18.3 Canonical encoding domains

- D8B provenance envelope: big-endian fixed-width canonical encoding
- D permission attestation: little-endian fixed-width canonical encoding

Cross-domain tooling must be protocol-aware. A repository-wide single-endianness assumption is invalid.

### 18.4 C1 diagnostic projection limits

C1 receives only binding + satisfied from each prerequisite channel. Full explanation requires the original A1/B1/C/D evidence artifacts.

decision-sufficient evidence != explanation-sufficient evidence

### 18.5 Adapter / assembler state

- A2 implemented
- B1 -> C1 adapter missing
- C -> C1 adapter missing
- D evaluator incomplete
- D -> C1 adapter designed but missing
- prerequisite assembler missing

### 18.6 Freshness families

- A1: live evidence-polarity
- B1/C: live trusted-context
- D: no live check; exact signed request-epoch binding only

### 18.7 Reserved inactive seams

- Domain: ProductionPersistenceAccess
- C1: ProductionAuthorityDerivationAccess

Presence does not imply active integration.

### 18.8 Governance asymmetry

- A2: runtime merged while reviewed design anchor remained open
- D: design baseline merged but explicitly unratified; runtime contains only crypto slice

implemented != ratified

### 18.9 Identity classes

- SourceCaptureId: capture event identity
- D8C/D8D/D9A/D9B/request/A1/B1/C IDs: evaluation-event identities
- C1: deterministic decision value without event identity

### 18.10 Lineage modes

- nested-by-value: D8A -> D9A
- projection: D9B -> request -> A2
- local-embed: A1

---

## §19. Known boundaries / not-yet-implemented surfaces

These are known boundaries, not automatically architectural debt.

1. prerequisite assembler between channel outputs and C1 input
2. B1 -> C1 adapter
3. C -> C1 adapter
4. complete D evaluator
5. D -> C1 adapter
6. D2 issuer ceremony / provisioning
7. Authority / Execution
8. L2 cross-channel atomic snapshot: absent in current design/implementation; not delegated, simply not introduced
9. L3 commit-time current-state validation: explicitly delegated to Authority/Execution; must be atomic with mutation

Absence of L2 != delegation.  
L3 is a delegation.  
Do not collapse L2 and L3 under one "delegated" label.

---

## §20. Evidence provenance appendix

Canonical CI evidence is bound to exact validated implementation heads, not PR branch names, final PR heads, or merge commits.

Selected accepted provenance anchors:

| Layer | PR | Validated head | Merge | Workflow / run |
|---|---:|---|---|---|
| Domain | #3 | 3e4825eeb7560b8fff49f1853b571f1fee081fbd | fde04915... | Domain Core Validation / 35642257137 |
| D8B | #14 | 4b3db739b6e5315953f7c07ab901ddaf0c8efd13 | 09b531... | dedicated run 35706825930 |
| D8C | #16 | 70e5dd... | 391536... | Provenance Admissibility / 35710293291 |
| D8D | #18 | f128203... | 0f8a714... | Versioned Interpretation / 35713247312 |
| D9B | #20 | b8a164a7fba2fa702630326633d00312cbc32ccc | fa83f337... | D9 Policy Persistence / 35719911573 |
| Request | #22 | 060509f24eedb0748f9fcd9f06288d3fc3056261 | 45b07590... | Transition Request / 35723993331 |
| A1 | #24 | bf7361df2d7ffdb837479df9ed397e8387010109 | eceed69eeea508e3388ecc79aaf2daf2aeda8189 | Transition Live Validity / 35727059119 |
| A2 | #26 | 154f030ab59be4c9017ecc3f611d2af23bd7a311 | 28f2370f3b8f2c2474bfbbdfc67474ceafc87db8 | Live Validity C1 Adapter / 35732028710 |
| B1 | #29 | 990e69176c015d3273147133d0dbdaca49249db9 | b18efd22992af82654217cd38015b691275336a7 | Transition Invariant / 35735966849 |
| C | #31 | 14faf28b99cf274cfd9412228533bc57996c95eb | aa9b5bf77bf060066c0f0dd5051a1cbfcf47f347 | Transition Resilience / 35738087378 |
| D crypto slice | #33 | 621f29eaa85254bad3ca8bcfb800b3914d807d82 | 41627df13777aefd625e85003ca45ff6f2cc50e9 | Permission Verifier / 35745558003 |
| C1 | #5 | dc2930b4a6bdc68d8d564122b6c8f4905ae3fe2c | 400ccfa75f128644b648f36e179208923bad6eb5 | Transition Eligibility / 35644105465 |

For C1, executable implementation was validated at dc2930b4a6bdc68d8d564122b6c8f4905ae3fe2c. The final PR head cce56b649146c79a7c60c457094c230a09a478fb added only provenance/audit documentation before merge.

CI count != coverage.

Per-layer guarantees must come from the exercised contract and source evidence, not from the number of successful jobs.

---

## Freeze statement

Authority axis:

    recommendation
    != request
    != prerequisite evidence
    != eligibility
    != authority
    != execution

Temporal axis:

    L1 binding
    != L2 atomic snapshot
    != L3 commit-time currentness

Identity axis:

    artifact / event identity
    != evaluation-event identity
    != deterministic decision value

Evidence axis:

    missing evidence
    != negative evidence
    != invalid evaluation context
    != terminal primary reason

Governance axis:

    implemented
    != merged design
    != ratified
    != provisioned
    != activated

Final integration statement:

- A1/A2: freshness + revalidation C1-ready
- B1: rich invariant record implemented; adapter missing
- C: rich resilience record implemented; adapter missing
- D: verifier crypto slice implemented; full permission evaluator missing; adapter missing
- assembler: missing

Therefore C1 evaluator is implemented, but complete production prerequisite assembly and end-to-end request -> C1 reachability do not currently exist.

This document intentionally stops at the Authority boundary.
