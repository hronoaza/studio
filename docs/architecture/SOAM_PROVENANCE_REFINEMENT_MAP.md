# SOAM Provenance Refinement Map

## Status

- Document class: architecture/refinement candidate
- Executable implementation change: none
- Acceptance status: not accepted into Current Baseline
- Base: canonical Current Baseline after D7
- Related candidate: PR #8 / D8A native provenance
- Formal source package: `soam.rar`
- Formal source package SHA-256:
  `061129ef13db1d03caa5e2e3c4cb7fd325963b68852f87de7f71081210d42916`

This document defines a proposed correspondence between the verified abstract
SOAM provenance/governance model and the executable SOAM 2.0 layers already
accepted in this repository.

It is a **refinement map candidate**, not a proof that the executable system
refines the TLA+ model.

## 1. Why this map exists

The accepted executable stack currently contains:

- Phase A — domain evidence primitives;
- Phase B — compiled runtime;
- Phase C1 — typed transition eligibility;
- Phase C2 — live binding, relationship lineage and revalidation;
- D7 — verified negative provenance gate.

D7 established that the system must not fabricate a production transition
direction from `BridgeStatus`, caller-injected evidence, or synthetic test
hooks.

The formal SOAM provenance model adds a stronger requirement:

> provenance is not merely a runtime snapshot; evidence must also satisfy
> integrity, schema, semantic-independence and re-derivability constraints
> before it can participate in an admissible transition.

Therefore the next executable layers must distinguish:

`source facts -> provenance envelope -> admissibility -> interpretation`

rather than collapsing all four into one producer.

## 2. Formal model concepts

The formal package defines an A→B migration/governance state machine around
versioned baselines, provenance-bearing items, grants and commit readiness.

The relevant abstract concepts include:

- `ProvenanceKnown(x)`
- `IntegrityValid(x)`
- `SchemaCompatibleB(x)`
- `~DependsOnRetiredInvariant(x)`
- `~ContainsInterpretationA(x)`
- `ReDerivable(x)`
- `AdmissibleB(x)`
- current/baseline version binding
- snapshot capture and snapshot hash
- authorization grants bound to issuer/subject/expiry/nonce/actions/version
- used/revoked nonces
- append-only/monotonic ledger semantics
- probation states with no irreversible effects

The formal model operates at a broader migration/governance scale than one
runtime relationship transition. The correspondence below is therefore
structural, not one-to-one code identity.

## 3. Proposed refinement correspondence

| Formal concept | Executable/current concept | Refinement status |
|---|---|---|
| provenance-bearing source item | future D8A source-capture value | candidate only |
| item incarnation / source identity | C2 relationship generation | partial structural correspondence |
| baseline/version binding | C2 runtime transition-state version | partial structural correspondence |
| snapshot capture | C2/D8A coherent locked capture | partial structural correspondence |
| snapshot identity/hash | not yet implemented | missing |
| `ProvenanceKnown(x)` | producer-owned origin, not caller-forged | D7 boundary established; positive path missing |
| `IntegrityValid(x)` | no executable equivalent yet | missing |
| `SchemaCompatibleB(x)` | no versioned provenance schema gate yet | missing |
| no retired invariant dependency | no executable provenance dependency manifest yet | missing |
| no old interpretation semantics | D7 forbids `BridgeStatus` substitution | partial |
| `ReDerivable(x)` | no executable derivation recipe/reference yet | missing |
| `AdmissibleB(x)` | future D8C provenance admissibility result | missing |
| interpretation into B semantics | future D8D mapping to `InteractionObservation` / `BridgeConfidence` | missing |
| persistent recommendation | Phase A `BridgePersistence` | accepted domain primitive |
| candidate eligible for authority consideration | C1 `eligible_for_authority_consideration` | accepted typed terminal state |
| live freshness / revalidation | C2 snapshot generation + state-version revalidation | accepted |
| grant | future authority capability | deliberately absent |
| grant baseline/version binding | future Phase D authority context | deliberately absent |
| nonce / replay prevention | future Phase D authority boundary | deliberately absent |
| probation with no irreversible effects | candidate/verified/acceptance-pending workflow | governance analogue; not runtime proof |
| irreversible commit | future production transition commit | deliberately absent |
| monotonic ledger | additive provenance/evidence records | process convention; executable invariant not yet implemented |

## 4. Refinement gaps

The following gaps block any claim that the current executable implementation
refines the formal provenance model.

### G1 — Source identity

A runtime relationship generation identifies one relationship incarnation, but
D8 does not yet define a stable provenance-item identity or producer identity.

### G2 — Integrity

There is no digest/signature/hash binding the captured payload, schema identity,
producer identity and lineage metadata.

### G3 — Versioned schema

The current runtime has no explicit provenance schema identifier or compatibility
policy.

### G4 — Dependency lineage

A provenance item does not yet record which invariants, algorithms, policies or
interpretation versions contributed to it.

Therefore the executable system cannot yet prove that an item is independent of
a retired invariant or legacy interpretation.

### G5 — Re-derivability

The executable system does not yet retain a derivation recipe or source reference
sufficient to independently reproduce a provenance item.

### G6 — Admissibility

No executable equivalent of `AdmissibleB(x)` exists.

### G7 — Interpretation

No accepted policy exists for:

`admissible production provenance ->
InteractionObservation + BridgeConfidence`

This gap is intentional. D7 requires the system to remain fail-closed here.

### G8 — Formal refinement proof

No TLC run or theorem currently establishes a refinement relation between the
C++ implementation and the TLA+ state machine.

The TLA+ verification evidence must not be cited as proof of C++ correspondence.

## 5. Revised D8 decomposition

The previous D8A candidate used the name
`ProductionRelationshipProvenance` for a raw runtime snapshot.

That name is too strong if the formal provenance semantics are adopted.

The executable design should instead be decomposed as follows.

### D8A — Source Capture

Purpose:

`relationship locator -> coherent immutable source facts`

Candidate output name:

`ProductionRelationshipSourceSnapshot`

Required properties:

- runtime-produced, not caller-forged;
- one directed relationship incarnation;
- one runtime state version;
- capture under one coherent lock/snapshot domain;
- raw facts only;
- no compatibility/confidence semantics.

### D8B — Provenance Envelope

Purpose:

`source snapshot -> provenance-bearing immutable envelope`

Required metadata:

- provenance item identity;
- producer identity/version;
- schema identity/version;
- relationship generation;
- state/baseline version;
- capture identity;
- integrity digest over canonical payload;
- derivation/source reference;
- dependency manifest.

D8B still does not interpret compatibility or confidence.

### D8C — Provenance Admissibility

Purpose:

implement an executable analogue of:

`AdmissibleB(x)`

The result should be a restricted-origin typed value such as:

`AdmissibleProductionProvenance`

Only an admissibility evaluator may construct it.

Candidate checks:

- known producer/source;
- integrity valid;
- schema compatible;
- required dependencies active;
- no retired-invariant dependency;
- no legacy interpretation dependency;
- re-derivable;
- lineage current.

Fail closed on any missing or unverifiable condition.

### D8D — Versioned Interpretation Policy

Purpose:

`AdmissibleProductionProvenance
-> InteractionObservation + BridgeConfidence`

The policy must explicitly define:

- interpretation policy ID/version;
- fields used by compatibility;
- normalization;
- confidence semantics;
- directional symmetry/asymmetry;
- invalidation behavior;
- relationship/state-lineage binding;
- reproducibility requirements.

No hidden mapping from `BridgeStatus` is permitted.

## 6. Relationship-level refinement chain

The proposed executable chain becomes:

`runtime relationship`
→ `D8A ProductionRelationshipSourceSnapshot`
→ `D8B ProvenanceEnvelope`
→ `D8C AdmissibleProductionProvenance`
→ `D8D InteractionObservation + BridgeConfidence`
→ `AdaptiveBridgePolicy`
→ `BridgePolicyEvidence`
→ `BridgePersistence`
→ `PersistentBridgeRecommendation`
→ `RequestedTransitionDirection`
→ `C1 prerequisites`
→ `eligible_for_authority_consideration`
→ **STOP**

Authority remains outside this chain.

## 7. Authority refinement boundary

The formal package contains grant semantics, including identity, action scope,
version binding, expiry, nonce, revocation and consumption.

Those concepts belong to a later authority phase and must not be imported into
D8.

Future authority refinement may map:

- formal grant → typed execution capability;
- permitted actions → capability scope;
- baseline version → capability state-version binding;
- nonce → single-use/replay-prevention token;
- revoked nonce → explicit revocation state;
- expiry → temporal validity;
- irreversible transition → commit after successful authority validation.

The human Root Operator remains the governance authority. Machine capabilities
may only represent bounded, explicitly delegated execution rights.

## 8. Ledger refinement boundary

The formal model's monotonic ledger principle should eventually become an
executable invariant:

`new evidence state extends prior evidence state`

rather than mutable replacement of historical evidence.

Current repository provenance records already follow an additive workflow
convention, but that convention is not yet represented as a runtime/type-level
invariant.

This is a future evidence-layer task, not D8A.

## 9. Treatment of PR #8

PR #8 is intentionally left unmodified while this refinement map is reviewed.

Its current compile failure should not be repaired merely to obtain green CI.

Before D8A continues, PR #8 should be revised so that:

1. the raw value is renamed from the semantically strong
   `ProductionRelationshipProvenance`;
2. D8A is explicitly source capture only;
3. provenance semantics move to D8B/D8C;
4. the public-header dependency boundary is reconsidered;
5. tests distinguish raw source capture from admissible provenance.

## 10. Verification claim

This document makes no claim that:

- the TLA+ model is a complete model of the C++ runtime;
- TLC verification proves C++ correctness;
- this correspondence is a formal refinement proof;
- D8A/D8B/D8C/D8D are implemented;
- production authority is ready.

It defines the map that must be tested and, where feasible, formally checked
before such claims are allowed.

## 11. Next gate

Before changing PR #8:

1. review and accept/reject this refinement map;
2. if accepted, revise D8A into source capture only;
3. obtain green regression/sanitizer CI for revised D8A;
4. audit D8A independently;
5. only then begin D8B provenance-envelope design.
