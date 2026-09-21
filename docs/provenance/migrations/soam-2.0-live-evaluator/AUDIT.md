# Pre-Acceptance Audit — SOAM 2.0 Live Evaluator Binding (Phase C2)

## Repository state

- Base: `main`
- Base SHA: `400ccfa75f128644b648f36e179208923bad6eb5`
- Validated head: `1e5c17100cc768f653b2f6127808792ff940965e`
- Compare state at audit: ahead 11, behind 0
- PR: #6
- Mergeability at audit: mergeable

## Changed surface

Phase C2 changes only the runtime/evaluator integration surface:

- evaluator public contract;
- private lease/binding implementation;
- runtime relationship generation;
- conservative runtime state-version lineage;
- snapshot capture and revalidation;
- evaluator lifetime/concurrency tests;
- CMake and CI;
- migration provenance.

## Trust-boundary audit

C2 can:

- locate a live directed relationship;
- capture relationship generation and runtime state version;
- revalidate a captured snapshot;
- invalidate evaluator bindings during mesh destruction;
- drain active evaluator leases;
- return `no_request` or `not_eligible`.

C2 intentionally does not provide a production path that returns
`eligible_for_authority_consideration`.

## Deliberate semantic separation

`BridgeStatus` is not treated as a substitute for
`PersistentBridgeRecommendation`.

No transition direction is derived from bridge runtime status.

Therefore C2 cannot fabricate authority-relevant provenance from the runtime's
existing bridge-state machine.

## Lineage model

New bridge pairs receive a fresh relationship generation.

Runtime changes advance a conservative transition-state version.

Revalidation requires the relationship generation and state version captured by
the attempt to remain current.

This version domain is intentionally broader than necessary and may invalidate
on unrelated runtime changes; that is fail-closed behavior, not a positive
authorization shortcut.

## Lifetime boundary

Evaluator handles hold shared binding state rather than an owning/raw runtime
lifetime guarantee.

Mesh destruction:

1. stops accepting new evaluator leases;
2. waits for active leases to drain;
3. clears the owner pointer.

Subsequent evaluator use returns `not_eligible`.

## Validation

At validated head `1e5c17100cc768f653b2f6127808792ff940965e`:

- runtime workflow run `35645827357`: 4/4 PASS under ASan/UBSan and 4/4 PASS under TSan;
- live evaluator workflow run `35645827383`: 4/4 PASS under ASan/UBSan and 4/4 PASS under TSan;
- no sanitizer-reported error observed in reviewed logs.

## Deferred surface

Still excluded:

- production-native observation/confidence provenance binding;
- positive direction derivation;
- live construction of C1 prerequisite evidence;
- BridgeTransitionAuthorization;
- production authority/capability derivation;
- production commit;
- K11/K12 mutation authority.

## Audit disposition

Technical state: `verified`.

Acceptance state: `acceptance-pending`.

No merge is authorized by this audit.
