# Architecture Candidate Register v1

## Status

- Document class: architecture candidate register
- Implementation effect: none
- Current Baseline effect: none
- Acceptance status: candidate only
- Intended use: normalize historical/descriptive material into independently
  reviewable architecture decisions
- This register does not modify or broaden D8A.

## 1. Source-handling rule

The reviewed material mixes:

- architectural descriptions;
- parameter proposals;
- metaphors and narrative analogies;
- self-assigned audit scores;
- future/test scenarios;
- claims of guarantees or readiness.

This register does not treat those categories as equivalent.

A statement is retained only after translation into one of:

- typed contract;
- invariant;
- state-machine transition;
- recovery rule;
- authority rule;
- audit criterion;
- explicit parameter hypothesis.

Historical names and narrative labels are not adopted as active runtime/API
terminology.

## 2. Classification vocabulary

### KEEP

Architectural concept is useful and should be carried forward into design work,
subject to implementation and independent verification.

### EXPERIMENT

Plausible design hypothesis that should be compared against alternatives before
acceptance.

### PARAMETER-CANDIDATE

Numeric value exists in source material, but does not become a system default
until its semantics, units, calibration method and failure behavior are defined.

### EVIDENCE-ONLY

Useful as provenance of prior reasoning, but not sufficient to establish a
current implementation property.

### DROP

Narrative, branding or metaphor with no necessary executable meaning.

---

## 3. Candidate register

### ACR-001 — Human-governed authority root

**Classification:** KEEP

**Normalized decision**

A machine component may evaluate, recommend, prepare or execute only within an
explicitly delegated capability. It may not create, widen or restore its own
authority scope.

**Required invariants**

- automation cannot mint its own authority capability;
- automation cannot widen capability scope;
- a revoked capability cannot be reactivated by its holder;
- absence of valid authority must fail closed;
- governance authority remains external to the decision-producing path.

**Target layer**

Future Authority Capability Layer.

**Relation to current stack**

C1 remains only
`eligible_for_authority_consideration`.
D8A remains below authority.

---

### ACR-002 — Revocation and stop path independent of decision path

**Classification:** KEEP

**Normalized decision**

A stop/revoke path should not depend on the same semantic decision machinery
that it is capable of stopping.

**Design implication**

Separate:

`decision/evaluation path`

from:

`revocation/termination path`.

For software this means an independently reachable fail-closed control path.
For physical deployments an out-of-band channel is a separate implementation
question requiring independent evidence.

**Target layer**

Authority / Operations.

---

### ACR-003 — Fail-closed safe-state transition

**Classification:** KEEP

**Normalized decision**

Failure of any mandatory precondition must produce no authoritative commit.

Candidate triggers include:

- missing/stale authority;
- failed integrity;
- stale lineage;
- invariant failure;
- failed revalidation;
- expired/revoked capability;
- loss of required supervision signal.

**Invariant candidate**

`mandatory_check_failed => no_authoritative_state_commit`

**Target layer**

D8C, C1, Authority, Commit.

---

### ACR-004 — Versioned checkpoints and bounded rollback

**Classification:** KEEP

**Normalized decision**

Recovery should use explicit versioned checkpoints rather than an untyped
"history buffer".

Candidate checkpoint identity:

- `CheckpointId`;
- `StateVersion`;
- parent checkpoint;
- integrity digest;
- capture reason;
- effect/commit boundary;
- retention metadata.

Rollback must additionally respect an irreversible-effect barrier.

**Important non-adoption**

Historical `100 steps` is not accepted as a core invariant.

**Target layer**

Recovery / State History.

---

### ACR-005 — Checkpoint retention depth

**Classification:** PARAMETER-CANDIDATE

**Historical value**

`100 steps`

**Required before adoption**

Define whether retention is bounded by:

- count;
- bytes;
- time;
- external storage policy;
- or a combination.

Also define:

- minimum recoverable horizon;
- eviction semantics;
- integrity behavior after compaction;
- treatment of irreversible effects.

**Target layer**

Recovery policy.

---

### ACR-006 — Append-only causal evidence ledger

**Classification:** KEEP

**Normalized decision**

Accepted evidence should extend prior evidence history rather than silently
rewrite it.

Candidate causal chain:

`source identity`
→ `capture identity`
→ `provenance envelope`
→ `admissibility verdict`
→ `interpretation policy/version`
→ `policy evidence`
→ `transition request`
→ `eligibility result`
→ `authority decision`
→ `execution result`.

**Target layer**

D8B/D8C plus later Evidence Ledger.

**Formal relation**

Compatible with the already reviewed monotonic-ledger model, but no executable
ledger invariant is claimed here.

---

### ACR-007 — Provenance integrity binding

**Classification:** KEEP

**Normalized decision**

A provenance envelope should bind a canonical payload to identity and lineage
metadata.

Candidate envelope fields include:

- provenance item identity;
- producer identity/version;
- schema identity/version;
- relationship generation;
- state/baseline version;
- capture identity;
- canonical payload digest;
- dependency manifest;
- derivation/source reference.

**Target layer**

D8B.

---

### ACR-008 — Explicit provenance admissibility

**Classification:** KEEP

**Normalized decision**

Raw capture does not become usable evidence merely because it originated from
the runtime.

Candidate admissibility checks:

- known producer/source;
- integrity valid;
- schema compatible;
- dependencies current;
- no retired-invariant dependency;
- no superseded interpretation dependency;
- re-derivable;
- lineage current.

**Target layer**

D8C.

---

### ACR-009 — Versioned deterministic interpretation context

**Classification:** KEEP

**Normalized decision**

Interpretation must expose the policy/version and all inputs that materially
affect its result.

Candidate property:

`same admissible evidence + same explicit interpretation context
=> same interpretation result`

within the defined deterministic scope.

**Target layer**

D8D.

---

### ACR-010 — Uncertainty as explicit policy evidence

**Classification:** KEEP concept / PARAMETER-CANDIDATE values

**Historical values**

- threshold: `0.50`;
- penalty weight: `0.75`.

**Normalized decision**

Uncertainty/confidence must have a defined operational meaning and must not be
used as an unexplained global score.

Before any threshold is accepted, define:

- what is being estimated;
- calibration population/domain;
- units/range;
- aggregation rule;
- behavior at threshold;
- missing-confidence behavior;
- invalidation and re-calibration.

**Target layer**

Primarily D8D; potentially higher policy layers.

**Status of numeric values**

`0.50` and `0.75` remain parameter candidates only.

---

### ACR-011 — Alternative-generation policy

**Classification:** EXPERIMENT

**Historical value**

Diversity weight `0.85`, with a narrative preference for 2–3 alternatives.

**Normalized hypothesis**

When a question admits materially different valid interpretations or actions,
the system may generate multiple bounded alternatives rather than silently
collapse to one path.

Potential activation conditions:

- ambiguity is material;
- consequences are substantial;
- uncertainty is above a defined threshold;
- operator explicitly asks for alternatives.

**Non-goal**

Alternative count is not a core invariant.

**Target layer**

Decision-support / interpretation presentation, not D8A-D8C.

---

### ACR-012 — Architecture / Behavior / Governance audit decomposition

**Classification:** EXPERIMENT

**Historical review weights**

- Architecture: `0.50`;
- Behavior: `0.30`;
- Governance: `0.20`.

**Normalized decision**

The three-way decomposition may be useful as an audit dashboard, but the
weights must not grant runtime authority.

Future scores should be derived from objective evidence where possible, for
example:

- CI/test result;
- formal invariant result;
- negative misuse gate;
- dependency/security review;
- manual authority review;
- external audit evidence.

**Status of 0.50/0.30/0.20**

Parameter candidates for an audit rubric only.

---

### ACR-013 — Historical readiness score and deployment threshold

**Classification:** EVIDENCE-ONLY

**Historical values**

- aggregate score: approximately `97.73%`;
- deployment threshold: `65%`.

These values document a prior self-assessment model.

They do not establish:

- production readiness;
- security;
- correctness;
- deployment authorization.

No runtime or governance gate should consume them unless a future acceptance
process independently defines and validates such a metric.

---

### ACR-014 — Recursive hypothesis/evidence/correction loop

**Classification:** KEEP concept

**Normalized loop**

`question/context`
→ `hypothesis`
→ `evidence acquisition`
→ `evaluation`
→ `result`
→ `correction/new hypothesis`.

This is useful as a research/analysis workflow.

**Boundary**

The loop may generate candidates and evidence, but does not grant itself
authority to commit production state.

**Target layer**

Analysis/research tooling.

---

### ACR-015 — Five-plane architecture extracted from the historical layered chain

**Classification:** KEEP as simplification

The historical multi-level narrative is normalized into five technical planes.

#### Physical Control Plane

External/manual termination and operational controls.

#### Input & Capture Plane

Addressing, triggering and raw source capture.

Current example:

`ProductionRelationshipLocator
-> ProductionRelationshipSourceSnapshot`.

#### Evidence & State Plane

Provenance, checkpoints, integrity and append-only lineage.

#### Interpretation & Decision Plane

Admissibility, interpretation, confidence, alternatives and policy evidence.

#### Authority & Commit Plane

Eligibility, capability, veto/revocation, bounded execution and commit.

This plane model is a candidate architectural map, not a forced runtime class
hierarchy.

---

### ACR-016 — Physical/quantum/thermodynamic metaphors

**Classification:** DROP from executable specification / EVIDENCE-ONLY historically

Terms such as:

- entropy/negentropy as generic cognitive claims;
- quantum superposition as a generic decision metaphor;
- wave-function collapse as operator choice;
- material writing media as mandatory system layers

are not imported into executable architecture unless a future design defines a
specific measurable mechanism requiring them.

---

### ACR-017 — "Hardened", "100% isolated", "100% resistant" claims

**Classification:** EVIDENCE-ONLY / not accepted as guarantees

Such labels require independent evidence tied to:

- exact implementation version;
- threat model;
- test method;
- environment;
- bounds;
- result artifacts.

Narrative or self-scored declarations are insufficient for Current Baseline
guarantees.

---

### ACR-018 — Out-of-band physical termination interface

**Classification:** KEEP as interface concept

For deployments controlling a physical process, the architecture may require a
termination channel outside the main decision/execution path.

This register does not claim that any existing hardware implementation provides
that property.

Required future evidence would include:

- wiring/topology;
- failure mode;
- latency bound;
- independence assumptions;
- bypass analysis;
- verification under fault.

**Target layer**

Deployment-specific Operations / Safety.

---

## 4. Parameter register

| Parameter | Historical value | Status | Candidate owner |
|---|---:|---|---|
| Architecture audit weight | 0.50 | PARAMETER-CANDIDATE | Audit rubric |
| Behavior audit weight | 0.30 | PARAMETER-CANDIDATE | Audit rubric |
| Governance audit weight | 0.20 | PARAMETER-CANDIDATE | Audit rubric |
| Alternative-generation weight | 0.85 | PARAMETER-CANDIDATE | Decision-support policy |
| Uncertainty threshold | 0.50 | PARAMETER-CANDIDATE | D8D/policy |
| Uncertainty penalty weight | 0.75 | PARAMETER-CANDIDATE | D8D/policy |
| Checkpoint retention | 100 steps | PARAMETER-CANDIDATE | Recovery policy |
| Historical readiness threshold | 0.65 | EVIDENCE-ONLY | Historical audit model |
| Historical aggregate readiness | ~0.9773 | EVIDENCE-ONLY | Historical audit model |

No value in this table is a Current Baseline runtime default.

---

## 5. Mapping to current and future layers

| Candidate area | Current/future layer |
|---|---|
| Neutral relationship addressing | accepted D8A candidate design |
| Raw coherent source capture | D8A |
| Provenance envelope / integrity | D8B |
| Provenance admissibility | D8C |
| Versioned interpretation / confidence | D8D |
| Policy evidence / persistence | Phase A extension/binding |
| Transition eligibility | C1 |
| Live relationship revalidation | C2 |
| Negative provenance boundary | D7 |
| Capability issuance/revocation | future Authority layer |
| Bounded commit | future Commit layer |
| Checkpoints / rollback | future Recovery layer |
| Causal append-only record | future Evidence Ledger |
| Audit score/dashboard | governance tooling only |

---

## 6. Explicit non-imports

The following are intentionally not imported into active naming or contracts:

- historical project branding;
- narrative agent names;
- narrative command names;
- literary labels;
- "quantum" or "negentropy" claims without measurable implementation semantics;
- self-certified security/readiness labels;
- fixed numeric thresholds without calibration evidence.

Historical source documents may retain their original terminology as evidence.

---

## 7. Decision sequence

Recommended review order:

1. keep D8A unchanged;
2. review ACR-007 and ACR-006 for D8B;
3. review ACR-008 for D8C;
4. review ACR-009 and ACR-010 for D8D;
5. independently design ACR-001/002/003 for Authority;
6. independently design ACR-004/005 for Recovery;
7. treat audit/alternative-generation parameters as experiments until calibrated.

No candidate in this register authorizes implementation by itself.
