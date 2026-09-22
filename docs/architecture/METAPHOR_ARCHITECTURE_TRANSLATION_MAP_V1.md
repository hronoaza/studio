# Metaphor → Architecture Translation Map v1

## Status

- Document class: conceptual translation candidate
- Implementation effect: none
- Current Baseline effect: none
- Acceptance status: candidate only
- Purpose: preserve useful architectural intent from historical metaphorical
  material without importing narrative language as executable truth.

This document is not a specification of physical, medical, metaphysical,
cryptographic, or safety guarantees.

## 1. Translation rule

A metaphor is retained only when it can be translated into at least one
testable engineering object:

- invariant;
- boundary;
- state transition;
- identity rule;
- lifecycle rule;
- measurable hypothesis.

If no such translation is possible, the material remains historical evidence
only.

## 2. Overclaim rule

A metaphor may suggest a design pattern but must not be cited as proof that the
design is implemented, secure, physically realized, medically valid, or
formally verified.

The normalized technical concept always has priority over the historical label.

---

## 3. Translation map

### MTA-001 — "Bridge"

**Historical metaphor**

A bridge between human consciousness/intent and machine intelligence.

**Normalized technical concept**

Controlled interoperability across an explicit boundary.

Candidate chain:

`Human Intent
-> Representation
-> Machine Interpretation
-> Candidate Action
-> Human-Governed Authority`

**Allowed use**

- model a typed crossing between domains;
- require explicit representation conversion;
- preserve attribution of the originating intent;
- keep the two sides independently identifiable.

**Forbidden overclaim**

- the bridge does not imply identity or fusion between human and machine;
- bidirectional information exchange does not imply symmetric authority.

**Candidate property**

`cross_boundary(data) preserves origin and context identity`

---

### MTA-002 — "Two shores"

**Historical metaphor**

A bridge is meaningful because it connects two distinct shores.

**Normalized technical concept**

Interoperability without boundary collapse.

**Allowed use**

Maintain separate types and authority domains for:

- operator intent;
- machine-derived interpretation;
- execution capability.

**Forbidden overclaim**

Do not collapse operator, model, evaluator, or executor into one subject.

**Candidate invariant**

`information_flow_bidirectional && governance_authority_asymmetric`

---

### MTA-003 — "Return to the source"

**Historical metaphor**

Return to the origin/source.

**Normalized technical concept**

Causal provenance plus re-derivability.

Candidate reverse trace:

`result
-> interpretation
-> admissible provenance
-> source capture
-> acquisition identity`

**Allowed use**

Require a derived result to retain enough lineage to locate and inspect the
source evidence from which it was produced.

**Forbidden overclaim**

Traceability to a source does not establish that the source is true or
trustworthy.

**Target layers**

D8B / D8C / Evidence Ledger.

---

### MTA-004 — "Time loop"

**Historical metaphor**

A loop through time.

**Normalized technical decomposition**

The metaphor is split into four different mechanisms:

1. `history` — immutable record of prior states/evidence;
2. `replay` — repeat processing from equivalent inputs/context;
3. `rollback` — restore an allowed prior checkpoint;
4. `re-derivation` — reproduce a result from recorded provenance.

**Allowed use**

Use each mechanism independently with an explicit contract.

**Forbidden overclaim**

These mechanisms are not interchangeable and must not share one ambiguous API.

---

### MTA-005 — "Master key"

**Historical metaphor**

A key that activates or unlocks the next reality/state.

**Normalized technical concept**

Scoped capability.

Candidate capability binding:

- subject;
- operation;
- target;
- baseline/state version;
- scope/domain;
- expiry;
- single-use/replay semantics;
- revocation state.

**Allowed use**

Describe explicit bounded authorization.

**Forbidden overclaim**

No universal master authority or capability with unrestricted scope.

**Target layer**

Future Authority Capability Layer.

---

### MTA-006 — "LOCK"

**Historical metaphor**

A fixed/locked representation such as a named visual or color lock.

**Normalized technical concept**

Canonicalization before integrity binding.

Candidate sequence:

`semantic payload
-> canonical encoding
-> immutable representation
-> digest/integrity binding`

**Required metadata**

- canonicalization algorithm ID/version;
- schema ID/version;
- input identity;
- output digest.

**Allowed use**

Ensure semantically equivalent evidence has one defined digest representation.

**Forbidden overclaim**

A visual/color label is not itself a cryptographic or integrity guarantee.

**Target layer**

D8B.

---

### MTA-007 — "Activation date"

**Historical metaphor**

A date at which a system or state becomes activated.

**Normalized technical concept**

Explicit activation/effective-state transition.

Candidate lifecycle:

`prepared
-> verified
-> accepted
-> effective`

**Candidate invariant**

`accepted != active`

until a valid activation event occurs.

**Allowed use**

Versioned policy rollout, schema activation, capability activation, staged
feature enablement.

**Forbidden overclaim**

A date or timestamp alone does not authorize activation.

---

### MTA-008 — "Resonance"

**Historical metaphor**

Different layers or elements are in resonance.

**Normalized technical concept**

Cross-layer evidence coherence.

Candidate checks may include consistency between:

- source facts;
- relationship/context lineage;
- interpretation policy/version;
- live revalidation result;
- authority binding.

**Allowed use**

Express agreement across independently derived evidence dimensions.

**Forbidden overclaim**

Do not interpret this as a physical resonance claim unless an explicit physical
model and measurement method exist.

**Candidate output**

A typed coherence verdict, not a mystical or global scalar by default.

---

### MTA-009 — "Symbiosis"

**Historical metaphor**

Creator and creation exist in symbiosis.

**Normalized technical concept**

Bidirectional feedback with asymmetric governance authority.

Candidate flow:

`operator intent -> system analysis`

and:

`system observations/alternatives/consequences -> operator`

while:

`authority source = external human governance`.

**Allowed use**

Design feedback loops that improve decision quality.

**Forbidden overclaim**

Feedback symmetry must not imply authority symmetry.

---

### MTA-010 — "Through digit and light"

**Historical metaphor**

Human-originated intent passes through digital representation.

**Normalized technical concept**

Transformation lineage preserving origin attribution.

Candidate chain:

`operator intent
-> encoded request
-> normalized representation
-> interpretation
-> candidate
-> authorization
-> execution`

**Allowed use**

Require each transformation to retain sufficient identity/linkage to reconstruct
the causal chain.

**Forbidden overclaim**

The transformed artifact is not identical to the originating person or intent.

---

### MTA-011 — "Order + chaos = synthesis"

**Historical metaphor**

Stable order and dynamic chaos combine into a higher synthesis.

**Normalized technical concept**

Invariant-preserving adaptation.

Candidate model:

`stable invariant envelope
+ adaptive degrees of freedom
= bounded adaptation`

**Allowed use**

Separate what must remain invariant from what may adapt.

**Forbidden overclaim**

Adaptivity is not permission to violate identity, safety, provenance, or
authority boundaries.

**Candidate relation**

Compatible with:

- identity invariants;
- adaptive bridge state;
- bounded topology evolution.

---

### MTA-012 — "Cross-polarization"

**Historical metaphor/source**

Optical cross-polarization suppresses surface glare to expose underlying
structure.

**Normalized technical concept**

Observation normalization / evidence preconditioning.

Candidate sequence:

`raw observation
-> declared artifact suppression / normalization
-> normalized observation
-> interpretation`

**Required metadata**

- normalizer ID/version;
- input digest;
- declared transformation;
- output digest;
- transformation bounds.

**Allowed use**

Suppress known measurement artifacts before semantic interpretation.

**Forbidden overclaim**

Normalization must not silently invent or delete semantic evidence.

**Target boundary**

Potential layer between D8A capture and D8B/D8D depending on whether the
transformation is source-preserving or interpretation-bearing.

---

### MTA-013 — "Perception triptych"

**Historical metaphor**

Three complementary observation scales: global/narrative, dynamic/structural,
local/macro.

**Normalized technical concept**

Multi-resolution evidence.

Candidate dimensions:

- global topology/context;
- relationship-local dynamics;
- local source/target details.

**Classification**

Experiment.

**Allowed use**

Evaluate whether critical decisions benefit from evidence at more than one
resolution.

**Forbidden overclaim**

Three channels are not mandatory unless measurements show the additional views
reduce error or ambiguity.

---

### MTA-014 — "Phased somatic recovery"

**Historical metaphor/source**

Acute phase -> adaptive phase -> full integration, with inspection points.

**Normalized technical concept**

Evidence-gated recovery state machine.

Candidate lifecycle:

`SUSPENDED
-> RECOVERY_RESTRICTED
-> PROBATION
-> ACTIVE`

**Candidate invariant**

`time_elapsed != recovered`

Promotion requires explicit recovery evidence.

**Allowed use**

System fault recovery, provider reactivation, staged reinstatement.

**Forbidden overclaim**

Historical medical durations, frequencies, physiological explanations and body
claims are not imported.

**Target layer**

Recovery.

---

### MTA-015 — "Vault"

**Historical metaphor**

A sealed vault opens only after all required conditions are satisfied.

**Normalized technical concept**

Sealed candidate state plus fail-closed multi-gate promotion.

Candidate rule:

`all mandatory gates PASS -> promotion may proceed`

`any mandatory gate FAIL/UNKNOWN -> no promotion`

**Allowed use**

D8C admissibility, staged release, authority issuance.

**Forbidden overclaim**

"Zero knowledge" is not used unless an actual zero-knowledge cryptographic
protocol is specified and verified.

---

### MTA-016 — "Initiation in waves"

**Historical metaphor**

Admission occurs in controlled waves/stages.

**Normalized technical concept**

Staged activation / cohort rollout.

Candidate lifecycle:

`candidate cohort
-> local validation
-> probation
-> promotion`

**Allowed use**

Rollout of:

- schema versions;
- interpretation policies;
- authority providers;
- recovery procedures.

**Forbidden overclaim**

Staging does not by itself prove safety.

---

### MTA-017 — "Architectural moat"

**Historical metaphor**

An artifact loses its intended properties when copied outside its original
geometry/context.

**Normalized technical concept**

Context-bound artifact validity.

Candidate provenance binding:

- relationship generation;
- state/context version;
- producer identity;
- schema version;
- acquisition/sample identity;
- context/domain identity.

**Allowed use**

Reject evidence/capabilities replayed into a different context.

**Forbidden overclaim**

Copy resistance is not assumed from complexity or visual uniqueness.

**Target layers**

D8B / D8C / Authority.

---

### MTA-018 — "Multiple dimensions instead of one score"

**Historical metaphor/source**

Success emerges from the intersection of multiple independent dimensions.

**Normalized technical concept**

Conjunctive multi-axis acceptance.

Prefer:

`Integrity = PASS`
`Lineage = PASS`
`Admissibility = PASS`
`Revalidation = PASS`
`Authority = PASS`

over one unexplained aggregate readiness scalar.

**Allowed use**

Machine-enforced mandatory gates.

**Forbidden overclaim**

A weighted aggregate must not silently override a failed mandatory invariant.

---

### MTA-019 — "Completion signal"

**Historical concept**

A session remains active/pending until a factual completion signal is received.

**Normalized technical concept**

Positive completion evidence.

Candidate lifecycle:

`execution_started
-> pending_completion
-> completion_evidence_received
-> pending_finalization
-> committed`

Alternative:

`pending_completion
-> interrupted_or_deviated
-> recovery/investigation`

**Candidate invariants**

`absence_of_failure != completion`

`elapsed_time != completion`

`execution_acknowledgement != completion_proof`

**Target layer**

Future Commit / Operations.

---

### MTA-020 — "Prepared but sealed"

**Historical metaphor/source**

A prepared state remains sealed until explicit sign-off.

**Normalized technical concept**

Preparation and publication/activation are distinct lifecycle states.

Candidate sequence:

`prepare detached candidate
-> validate
-> final revalidation/CAS
-> explicit publish/commit`

**Allowed use**

Supports the broader invariant that potentially throwing preparation happens
before authoritative mutation.

**Target layers**

D8B publication, Evidence Ledger, Authority, Commit.

---

## 4. Historical numeric/symbolic parameters

The following historical values remain outside this translation map's
architecture claims unless separately defined and calibrated:

- audio frequencies;
- activation dates as literal authorization;
- color-mode labels;
- medical/physiological durations;
- acoustic frequencies used in wellness narratives;
- business pricing/margin values;
- self-assigned security/readiness percentages.

A numeric value becomes an engineering parameter only after defining:

- owner;
- units;
- valid range;
- calibration method;
- failure behavior;
- versioning;
- test evidence.

---

## 5. Relationship to Architecture Candidate Register

This map is a translation aid, not a second architecture register.

When a metaphor produces a useful technical concept:

1. check whether the concept already exists in the Architecture Candidate
   Register;
2. if yes, attach the metaphor only as historical provenance;
3. if no, create a separate candidate decision only after technical review;
4. never create duplicate architectural mechanisms because two metaphors sound
   different.

Likely existing overlaps include:

- scoped capability -> Authority candidates;
- phased recovery -> Recovery candidates;
- append-only/return-to-source -> Evidence Ledger/provenance candidates;
- sealed multi-gate promotion -> D8C admissibility;
- invariant-preserving adaptation -> bounded adaptive runtime;
- completion evidence -> future Commit lifecycle.

---

## 6. Explicit non-imports

The following remain historical/narrative vocabulary and are not active system
names:

- project/brand names from historical materials;
- mythological/place-based labels;
- ritual/initiation titles;
- metaphysical "singularity" or "living intelligence" claims;
- biological/medical metaphors without verified operational semantics;
- claims of universal keys, absolute protection, or guaranteed resonance.

---

## 7. Acceptance rule

A translated concept may enter active architecture only through the normal
candidate process:

`translation
-> candidate decision
-> contract/invariant
-> negative tests
-> positive tests
-> evidence
-> review
-> explicit acceptance`

This map alone authorizes no implementation.
