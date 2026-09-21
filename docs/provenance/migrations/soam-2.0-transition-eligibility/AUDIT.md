# Pre-Acceptance Audit — SOAM 2.0 Transition Eligibility (Phase C1)

## Repository state

- Base: `main`
- Base SHA: `ad99b9fbce1d03730939ac2d7c933f09e1003c55`
- Validated head: `dc2930b4a6bdc68d8d564122b6c8f4905ae3fe2c`
- Compare state at audit: ahead 5, behind 0
- PR: #5
- Mergeability at audit: mergeable

## Changed surface

Only the C1 eligibility surface is introduced:

- one eligibility contract header;
- one isolated contract test;
- one minimal CMake target;
- one dedicated sanitizer workflow;
- migration provenance.

No existing Current Baseline source file is modified.

## Trust-boundary audit

C1 can represent:

- requested transition direction;
- relationship identity;
- state version;
- transition class;
- five prerequisite evidence values;
- deterministic eligibility decision.

Its positive terminal state is only:

`eligible_for_authority_consideration`.

That state is not:

- authorization;
- permission grant;
- execution capability;
- mutation command;
- production commit.

## Construction boundary

The public API cannot directly construct the restricted identity/version/class/evidence values used by the evaluator.

The test translation unit defines the predeclared friend
`detail::ProductionTransitionConstructionAccess` solely to exercise the contract.

The preserved forward-declared
`detail::ProductionAuthorityDerivationAccess` seam has no implementation in C1.

## Validation

GitHub Actions run #1, ID `35644105465`:

- GNU 13.3.0 + ASan + UBSan: 1/1 PASS
- Clang 18.1.3 + ASan + UBSan: 1/1 PASS
- no sanitizer-reported error observed in reviewed logs

## Deferred surface

Still excluded:

- live runtime capture/binding;
- relationship-generation producer;
- state-version producer;
- live revalidation evaluator;
- BridgeTransitionAuthorization;
- production authority/capability derivation;
- production transition commit;
- K11/K12 runtime mutation integration.

## Audit disposition

Technical state: `verified`.

Acceptance state: `acceptance-pending`.

No merge is authorized by this audit.
