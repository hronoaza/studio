# SOAM 2.0 — Transition Pipeline Composition Module

## 0. Status

- Layer: E — top-level composition of transition-eligibility pipeline
- Document class: design-contract candidate
- Scope: composition of request derivation, prerequisite channel evaluation, and C1 eligibility invocation into one coherent pipeline outcome
- Acceptance status: design-review pending
- Authority effect: none
- Module placement: `apps/soam-transition-pipeline/`; target `soam_transition_pipeline` (provisional; subject to ratification)
- Origin baseline: main @ `87dabeed795294c63a32867668f1fd6052e52b96`

## 1. Scope

This contract governs the composition layer that:

- obtains one ProductionDerivedTransitionRequest by invoking the accepted upstream derivation chain;
- invokes the accepted prerequisite channels (A1/A2, B1, C, D) against that request;
- materializes a ProductionTransitionPrerequisiteSet from the results;
- invokes the accepted C1 eligibility evaluator;
- returns a coherent pipeline outcome to its caller.

This contract does not define new semantic rules for any of the layers it composes. It does not re-derive, re-validate, or re-interpret the outputs of upstream accepted layers.

## 2. Non-scope

This contract does NOT:

- define new rejection taxonomies;
- create new decision identities;
- construct or forge C1 prerequisite evidence;
- construct or forge any restricted-origin type of a composed layer;
- acquire mesh topology locks directly;
- read live SpatialAdaptiveMesh state;
- amend, weaken, or extend C1 precedence semantics;
- substitute satisfied=false for missing prerequisite evidence;
- perform any form of retry, backoff, or pipeline restart;
- perform mutation, execution, or commit-time revalidation;
- accept an externally pre-derived ProductionDerivedTransitionRequest as an input alternative.

## 3. Layer role

Composition layer.

Not a decision layer.
Not a trust boundary.
Not a construction authority.

The layer's authority is limited to:

- invoking accepted evaluators in a defined order;
- materializing already-constructed evidence objects into the public ProductionTransitionPrerequisiteSet aggregate;
- returning the terminal outcome to the caller.

The layer holds no state across calls.

## 4. Dependency direction

`soam_transition_pipeline` depends on:

- `soam_transition_eligibility` — C1 contract;
- `soam_transition_request` — request derivation;
- `soam_transition_live_validity` — A1;
- `soam_transition_live_validity_c1` — A2;
- `soam_transition_invariant` — B1;
- `soam_transition_resilience` — C;
- `soam_transition_permission` — D;
- `soam_runtime` — upstream evaluators / sources.

No module in the list above may depend on `soam_transition_pipeline`. This preserves the existing repository DAG.

## 5. Input

The pipeline entrypoint is canonical and singular.

It accepts only the upstream context required to obtain a ProductionDerivedTransitionRequest through the accepted request-derivation chain.

The pipeline does not expose a second API surface that accepts an already-derived request. External callers that already hold a valid ProductionDerivedTransitionRequest are out of scope for this contract.

## 6. Outcome algebra

```text
variant<
  PreRequestTerminal,
  ProductionTransitionEligibilityDecision
>
```

No optional wrapper is introduced in v0. A composition-local infrastructure failure mode has not been established as a distinct terminal class, so the contract does not pre-declare one.

Where:

`PreRequestTerminal` is the typed terminal of the accepted request-derivation chain, preserved as the accepted type without translation. It means that a valid ProductionDerivedTransitionRequest did not arise.

`ProductionTransitionEligibilityDecision` is the terminal decision produced by C1 after a valid request existed.

The pipeline has exactly two terminal classes and no others.

## 7. Composition semantics

The pipeline executes in ordered stages:

Stage 1 — obtain request

Invoke the accepted request-derivation chain. If a valid ProductionDerivedTransitionRequest arises, proceed to Stage 2. If the derivation terminates without a valid request, return PreRequestTerminal and stop.

Stage 2 — invoke prerequisite channels

Invoke each accepted channel evaluator against the request.

A channel that produces prerequisite evidence retains that evidence for Stage 3.

A channel that does not produce prerequisite evidence is retained only for the purpose of determining that the corresponding C1 prerequisite slot is absent. It does not become a pipeline terminal outcome and is not translated into synthetic evidence.

Stage 3 — materialize prerequisite set

Construct a ProductionTransitionPrerequisiteSet:

- successful evidence -> placed in its corresponding slot;
- channel without evidence -> corresponding slot remains std::nullopt.

No synthetic evidence is produced.

Stage 4 — invoke C1

Call `ProductionTransitionEligibilityEvaluator::evaluate(request.binding(), set)`.

Return its ProductionTransitionEligibilityDecision unchanged.

Once Stage 1 has produced a valid request, the pipeline does not short-circuit.

## 8. No synthetic evidence rule

This is normative.

A missing prerequisite channel MUST remain std::nullopt in the materialized set. It MUST NOT be replaced by a `{ satisfied = false }` evidence object.

C1 distinguishes, by contract and by test:

```text
absence of evidence    -> missing_prerequisite
present evidence false -> <channel>_not_satisfied
```

Collapsing absence into false would erase C1's diagnostic distinction and constitute a semantic rewrite of C1 outcome by the pipeline.

## 9. Partial-set semantics

The pipeline MUST invoke C1 with a partial set when one or more channels did not produce evidence.

The pipeline MUST NOT:

- abort early because one channel has rejected;
- skip remaining channels after a channel rejection;
- substitute any channel result with a derived outcome.

C1 evaluation order is:

```text
missing_prerequisite
  -> binding_mismatch
  -> permission_not_satisfied
  -> invariant_not_satisfied
  -> resilience_not_satisfied
  -> freshness_not_satisfied
  -> revalidation_failed
  -> eligible_for_authority_consideration
```

`missing_prerequisite` is not a per-channel precedence value. If any one or more slots are absent, C1 returns a single `missing_prerequisite` reason. There is no ordering among missing channels and no diagnostic distinction by which channel is absent.

## 10. C1 diagnostic ownership

After a valid request exists, C1 is the sole producer of the terminal eligibility decision, including all rejection reasons defined in its contract.

The pipeline does not produce rejection reasons of its own. The pipeline does not translate, aggregate, or reorder reasons.

Channel-specific post-request rejection details are not promoted into pipeline terminal semantics in v0. The corresponding C1 slot is absent, and C1 owns the terminal eligibility diagnosis.

Callers that need channel-level post-request diagnostics must obtain them from the respective channel's own accepted API, outside this pipeline contract.

## 11. Observability limitation

Pipeline callers receive exactly one of:

- PreRequestTerminal, produced before a valid request exists; or
- ProductionTransitionEligibilityDecision, produced by C1 after a valid request exists.

Post-request channel rejection details are not observable through the pipeline outcome in v0.

A caller that requires channel-specific post-request diagnostics must obtain them from the respective channel's accepted API outside this pipeline contract.

This limitation is intentional and follows from C1 diagnostic ownership defined in §10. It is not a loss of C1 terminal semantics: absence of channel evidence is represented to C1 as an absent prerequisite slot and is diagnosed by C1 as defined by its contract.

## 12. Execution ordering and order-independence invariant

Stage order is normative:

```text
request -> channels -> set materialization -> C1
```

Within Stage 2, the order of channel invocation is not mandated by this contract.

Order-independence invariant:

The observable pipeline outcome MUST be independent of the order in which Stage 2 channels are invoked. Channel evaluators are independent evaluators against an immutable request, and the materialized set, the C1 decision, and the returned outcome must not depend on invocation order.

Implementations may choose any order; they must not depend on cross-channel state, shared mutable state, or invocation side effects.

## 13. Provenance preservation

The pipeline preserves, without semantic modification:

- the ProductionDerivedTransitionRequest supplied to channels;
- the exact ProductionTransitionRequestBinding used to invoke C1;
- every successfully produced C1 prerequisite evidence object placed into the prerequisite set;
- the ProductionTransitionEligibilityDecision returned by C1;
- a PreRequestTerminal returned before request creation.

The composition outcome variant discriminates which accepted terminal occurred; it does not reinterpret either terminal.

Channel rejection details produced post-request are retained only as internal provenance sufficient to determine slot absence and are not part of the pipeline outcome.

## 14. STOP boundary

STOP: PipelineOutcome.

PipelineOutcome is the variant defined in §6: either PreRequestTerminal or ProductionTransitionEligibilityDecision.

The pipeline does not produce:

- authority;
- capability;
- execution instruction;
- commit-time validation.

Authority and execution remain outside the scope of this contract, as recorded in Evidence Inventory v1 §19.

## 15. Failure semantics

No composition-local infrastructure failure class is introduced in v0.

If a concrete composition-local failure mode is later identified that does not belong to any composed layer, it requires an explicit amendment to this contract before it can appear in the outcome algebra.

Until then, the pipeline has no nullopt terminal and no third terminal class.

## 16. Structural patterns reused from A2 (non-semantic)

The module may reuse, without implying semantic inheritance:

- sibling module organization under `apps/`;
- private constructor + friend construction seam where the module constructs a composition-local wrapper, if any;
- compile-fail boundary tests where the module exposes public construction surfaces;
- positive and negative runtime tests.

Semantic choices — including lossy vs lossless, container vs decision identity, and typed rejection vs pass-through at the module's own boundaries — remain independent design decisions for this contract and are not inherited from A2 or from any other adapter.

## 17. Explicit scope limitation

This contract does not define a generic pipeline framework.

It does not prescribe composition semantics for any future pipeline beyond the one described in §1.

It does not amend Evidence Inventory v1.

Structural implementation patterns may be reused without implying semantic inheritance.

## 18. Not defined in v0

The following remain open and are subject to amendment:

- handling of channel evaluator re-entry or repetition;
- concurrency model for channel invocation;
- cancellation and partial cleanup;
- metrics, logging, or observability surfaces beyond §11;
- pipeline-level timeout;
- module naming ratification;
- repository placement of this document itself;
- composition-local infrastructure failure taxonomy.
