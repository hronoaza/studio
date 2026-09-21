# Migration Record — SOAM 2.0 Transition Eligibility (Phase C1)

## Status

- Migration stage: `candidate`
- Acceptance status: not accepted into Current Baseline
- Source archive: `adaptive-mesh-main (2).zip`
- Source archive SHA-256: `f14900ddb77852dbf3935562fdb5f73a051338ade1bb60a84f4d1fde59577c03`
- Migration branch: `migration/soam-2.0-transition-eligibility`
- Base: canonical Current Baseline after Phase B

## Boundary

Phase C1 introduces only the typed transition-eligibility contract:

`request binding + five prerequisite evidence values -> eligibility decision`

Terminal positive result:

`eligible_for_authority_consideration -> STOP`

It is not an authorization, capability, credential, intent, execution command,
or production mutation request.

## Included

- relationship identity;
- state version;
- transition class ID;
- requested direction;
- request binding;
- permission prerequisite evidence;
- invariant prerequisite evidence;
- resilience prerequisite evidence;
- freshness prerequisite evidence;
- revalidation prerequisite evidence;
- deterministic fail-closed eligibility evaluator;
- rejection-precedence contract tests.

## Restricted-origin construction

Public construction of request identity/version/class/evidence remains closed.

Tests define the already-declared friend
`detail::ProductionTransitionConstructionAccess` only inside the test
translation unit to exercise the contract. No production construction owner is
introduced in Phase C1.

The header also preserves the archive's forward-declared
`detail::ProductionAuthorityDerivationAccess` friendship for state version
and transition class ID. No such authority implementation exists in this
phase; the seam is inactive and does not grant runtime authority.

## Explicitly deferred

- live runtime state capture;
- relationship generation/state-version producers;
- production transition evaluator live binding;
- BridgeTransitionAuthorization;
- authority context/policy/types;
- capability issuance;
- transition commit;
- K11/K12 live mutation integration.

## Preflight

Local C++20 Debug preflight with ASan + UBSan: 1/1 PASS.

Final acceptance remains an explicit Root Operator decision.
