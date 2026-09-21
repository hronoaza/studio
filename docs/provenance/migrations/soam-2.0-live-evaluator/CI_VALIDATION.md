# CI Validation — SOAM 2.0 Live Evaluator Binding (Phase C2)

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `migration/soam-2.0-live-evaluator`
- Validated head: `1e5c17100cc768f653b2f6127808792ff940965e`

Two workflows exercised the changed runtime surface at this exact head:

### Existing runtime workflow

- Workflow: `SOAM 2.0 Runtime Validation`
- Run number: `#4`
- Run ID: `35645827357`
- Conclusion: `success`

### C2-specific workflow

- Workflow: `SOAM 2.0 Live Evaluator Validation`
- Run number: `#1`
- Run ID: `35645827383`
- Conclusion: `success`

## Sanitizer matrix

Both workflows completed:

| Job | CTest |
|---|---|
| Debug + ASan + UBSan | 4/4 PASS |
| TSan, assertions enabled | 4/4 PASS |

The test set includes:

- runtime behavior;
- worker-pool lifecycle;
- topology transaction boundary;
- live evaluator binding/lifetime/concurrency.

Reviewed logs contain no AddressSanitizer error, UBSan runtime error, or ThreadSanitizer warning/error.

## C2-specific behavior exercised

The live evaluator tests cover:

- no relationship -> `no_request`;
- existing relationship -> fail-closed `not_eligible`;
- invalid/self/out-of-range locators -> `not_eligible`;
- evaluator use after owning mesh destruction -> `not_eligible`;
- concurrent live evaluation while runtime state changes;
- no observed promotion to `eligible_for_authority_consideration` in Phase C2.

## Scope boundary

This evidence validates the C2 live-binding and conservative lineage/revalidation implementation only.

It does not validate:

- positive live direction derivation;
- C1 prerequisite evidence materialization from live provenance;
- transition authorization;
- authority/capability issuance;
- production transition commit;
- K11/K12 mutation authority.
