# Migration Record — SOAM 2.0 Compiled Runtime

## Status

- Migration stage: `candidate`
- Acceptance status: not accepted into Current Baseline
- Source archive: `adaptive-mesh-main (2).zip`
- Source archive SHA-256: `f14900ddb77852dbf3935562fdb5f73a051338ade1bb60a84f4d1fde59577c03`
- Target repository: `hronoaza/studio`
- Migration branch: `migration/soam-2.0-runtime`
- Base: canonical Current Baseline after SOAM 2.0 Phase A

## Phase B boundary

Phase B introduces a compiled runtime/topology layer only.

Included capabilities:

- compiled static runtime target;
- validated geometry and invariant operations;
- sequential node identity contract;
- direct and batch topology connection;
- duplicate/self/out-of-range rejection;
- automatic proximity connection;
- symmetric bridge pruning;
- bounded local shock filtering;
- buffered simulation computation followed by publication;
- persistent worker pool with explicit lifecycle;
- blocking compatibility wrapper for `simulationStepAsync()`;
- shared/exclusive topology synchronization;
- public state/health/bridge-count observation.

## Source relationship

The supplied 2.0 archive mixes compiled runtime implementation with later
production-authority and K11/K12 machinery in the same translation unit and
private implementation.

Phase B therefore does **not** copy that mixed file wholesale.

Instead, it extracts the runtime mechanisms needed for this layer while
preserving the archive's relevant runtime semantics:

- compiled PIMPL boundary;
- persistent worker pool;
- batch topology preparation before publication;
- explicit numeric/topology validation;
- buffered simulation state and bridge-state commit.

Production-authority lifecycle records, capability issuance, transition
eligibility, evaluator bindings and production commit paths are deliberately
excluded.

## Explicitly deferred

Not present in Phase B:

- `BridgeTransitionAuthorization`;
- production transition prerequisites/eligibility;
- production transition evaluator;
- production authority context/policy/types;
- capability issuance;
- production transition commit;
- K11/K12 live integration;
- experimental KIKO/shadow layers;
- root repository license/publication metadata.

## Preflight

A local C++20 Debug preflight with ASan + UBSan completed 3/3 tests successfully:

- runtime behavior;
- worker-pool lifecycle;
- topology transaction boundary.

GitHub CI is the acceptance evidence source for this migration.

## Semantic note

`simulationStepAsync()` is retained for compatibility but is intentionally
blocking in Phase B. The internal computation is parallelized by a persistent
worker pool. No API claim of non-blocking asynchronous completion is made.

Final acceptance remains an explicit Root Operator decision.
