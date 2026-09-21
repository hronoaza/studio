# CI Validation — SOAM 2.0 Compiled Runtime

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `migration/soam-2.0-runtime`
- Validated head: `45c9c5a490b4f73b4c125d3a5950c60dc1d33a25`
- Workflow: `SOAM 2.0 Runtime Validation`
- Run: `#1`
- Run ID: `35643399828`
- Conclusion: `success`

## Jobs

| Job | Result |
|---|---|
| Debug + ASan + UBSan | 3/3 CTest PASS |
| TSan, assertions enabled | 3/3 CTest PASS |

Both jobs executed all three Phase B tests:

- runtime behavior;
- worker-pool lifecycle;
- topology transaction boundary.

Reviewed logs contain no AddressSanitizer error, UBSan runtime error, or ThreadSanitizer warning/error.

## Exercised runtime contracts

The validation covers:

- compiled runtime construction/destruction;
- persistent worker-pool lifecycle across repeated mesh instances;
- fixed worker counts 1, 2 and 4;
- topology growth after an initial simulation step;
- validated node IDs and direct connection indices;
- self-connection rejection;
- duplicate direct-connection rejection;
- batch connection with pre-publication validation;
- no partial batch publication when a later requested edge is invalid;
- automatic nearby-node connection;
- finite states and bounded health across repeated simulation steps;
- blocking compatibility wrapper `simulationStepAsync()`.

## Scope boundary

This evidence covers only the Phase B runtime/topology surface at the exact validated head.

It does not validate later production transition authorization, eligibility, authority/capability issuance, K11/K12 integration, or production commit semantics.
