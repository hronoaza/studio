# CI Validation — SOAM 2.0 Transition Eligibility (Phase C1)

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `migration/soam-2.0-transition-eligibility`
- Validated head: `dc2930b4a6bdc68d8d564122b6c8f4905ae3fe2c`
- Workflow: `SOAM 2.0 Transition Eligibility Validation`
- Run: `#1`
- Run ID: `35644105465`
- Conclusion: `success`

## Compiler matrix

| Job | Compiler | Sanitizers | CTest |
|---|---|---|---|
| gcc-sanitizers | GNU 13.3.0 | ASan + UBSan | 1/1 PASS |
| clang-sanitizers | Clang 18.1.3 | ASan + UBSan | 1/1 PASS |

Reviewed logs contain no AddressSanitizer error or UBSan runtime error.

## Exercised contract

The test validates:

- restricted public construction of identity/version/class/binding/evidence types;
- complete five-prerequisite success path;
- missing-prerequisite fail-closed behavior;
- binding mismatch rejection before prerequisite-value evaluation;
- deterministic rejection precedence:
  permission → invariant → resilience → freshness → revalidation;
- stable repeated evaluation;
- terminal positive result:
  `eligible_for_authority_consideration`.

## Scope boundary

The validated Phase C1 contract does not:

- read live runtime state;
- construct production request evidence in production code;
- authorize a transition;
- issue an execution capability;
- mutate runtime state;
- commit K11/K12 production transitions.

This evidence applies only to the exact validated head above.
