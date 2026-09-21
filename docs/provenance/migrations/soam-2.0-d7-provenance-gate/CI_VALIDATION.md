# CI Validation — SOAM 2.0 D7 Provenance Gate

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `migration/soam-2.0-d7-provenance-gate`
- Validated head: `fb2d63a3f40249e3197df625315e2bbcbcc2f4d0`

Three runtime-relevant workflows completed successfully at this exact head:

### D7-specific workflow

- Workflow: `SOAM 2.0 D7 Provenance Gate Validation`
- Run number: `#1`
- Run ID: `35646956852`
- Conclusion: `success`

### Live evaluator regression workflow

- Workflow: `SOAM 2.0 Live Evaluator Validation`
- Run number: `#3`
- Run ID: `35646956772`
- Conclusion: `success`

### Runtime regression workflow

- Workflow: `SOAM 2.0 Runtime Validation`
- Run number: `#6`
- Run ID: `35646956777`
- Conclusion: `success`

## D7-specific job results

The D7 workflow completed the same six-test runtime suite under both sanitizer jobs:

| Job | Result |
|---|---|
| Debug + ASan + UBSan | 6/6 PASS |
| TSan, assertions enabled | 6/6 PASS |

The six tests include:

1. runtime behavior;
2. worker-pool lifecycle;
3. topology transaction boundary;
4. live evaluator binding/lifetime;
5. D7 provenance fail-closed runtime gate;
6. compile-fail rejection of public observation/confidence injection.

The compile-fail test produced the expected harness result:

`D7 provenance-injection misuse correctly rejected by compiler`

Reviewed D7 logs contain no AddressSanitizer error, UBSan runtime error, or
ThreadSanitizer warning/error.

## D7 contract validated

At the validated head:

- callers cannot default-construct `ProductionTransitionEvaluator`;
- the public live evaluator has no overload accepting caller-supplied
  `InteractionObservation` / `BridgeConfidence`;
- an attempted synthetic provenance-injection program fails compilation;
- repeated runtime evolution does not produce
  `eligible_for_authority_consideration`;
- an existing relationship remains fail-closed at `not_eligible`.

## Scope boundary

This evidence validates a **negative provenance gate**.

It does not establish or claim:

- a production observation producer;
- a production confidence producer;
- adaptive production evidence lineage;
- production-native direction derivation;
- positive live eligibility;
- authority/capability issuance;
- production commit.
