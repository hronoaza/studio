# CI Validation — SOAM 2.0 Domain Core

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `migration/soam-2.0-domain-core`
- Validated head: `3e4825eeb7560b8fff49f1853b571f1fee081fbd`
- Workflow: `SOAM 2.0 Domain Core Validation`
- Run: `#1`
- Run ID: `35642257137`
- Conclusion: `success`

## Compiler matrix

| Job | Compiler | Sanitizers | CTest |
|---|---|---|---|
| gcc-sanitizers | GNU 13.3.0 | ASan + UBSan | 1/1 PASS |
| clang-sanitizers | Clang 18.1.3 | ASan + UBSan | 1/1 PASS |

No AddressSanitizer error or UBSan runtime error was observed in the reviewed logs.

## Exercised domain contracts

The isolated test covers:

- finite and bounded `InteractionObservation`;
- finite and bounded `BridgeConfidence`;
- explicit construction boundaries;
- non-forgeable raw construction of `BridgePolicyEvidence`;
- bounded evidence in `[-1,1]`;
- symmetry around neutral compatibility;
- confidence monotonicity;
- zero-confidence suppression;
- persistence activation;
- release hysteresis;
- direction reversal;
- reset semantics;
- independent persistence instances.

## Scope boundary

The validated phase contains no topology mutation, worker pool, runtime execution authority,
K11/K12 transition commit path, or production capability issuance.

This evidence validates only the Phase A domain-core contracts at the exact validated head.
