# CI Validation — Adaptive Mesh 1.1

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `migration/adaptive-mesh-1.1`
- Validated branch head: `9e073b0ba7c52153ba98b02231500789935f9934`
- GitHub Actions workflow: `Adaptive Mesh Validation`
- Workflow run: `#1`
- Run ID: `35635986219`
- Run conclusion: `success`
- Runner OS: Ubuntu 24.04.5 LTS
- Compiler: GCC 13.3.0

## Jobs

| Job | Result |
|---|---|
| Debug + ASan + UBSan | PASS |
| TSan, assertions enabled | PASS |

Both jobs configured, built, and executed the registered CTest target `run_mesh_tests`.

The test reached and completed `simulationStepAsync()` after the geometric test oracle was corrected from radius `3.0` to `3.5`. The source algorithm was not changed for that correction.

## Debug sanitizer evidence

The Debug job compiled with:

- `-fsanitize=address,undefined`
- `-fno-omit-frame-pointer`
- warnings enabled and promoted to errors

CTest result:

- 1/1 tests passed
- 0 failed
- process completed successfully

This establishes that the tested path completed under ASan + UBSan in this environment without a sanitizer-reported failure.

## Thread sanitizer evidence

The TSan job compiled with:

- `-fsanitize=thread`
- `-fno-omit-frame-pointer`
- `-UNDEBUG`
- `-O1 -g`

Runtime used:

`TSAN_OPTIONS=halt_on_error=1`

CTest result:

- 1/1 tests passed
- 0 failed
- process completed successfully

This establishes that the tested path completed under TSan in this environment without a sanitizer-reported data race.

## Historical comparison

The supplied validation bundle had previously recorded Debug and TSan runs that terminated before `simulationStepAsync()` because the test expected nodes at distance `sqrt(12) ≈ 3.464` to connect using radius `3.0`.

The migration corrected only that test assumption:

`autoConnectNearbyNodes(3.0)` → `autoConnectNearbyNodes(3.5)`

The new CI evidence therefore supersedes the old execution limitation while preserving the old evidence as historical provenance.

## Scope and limitations

This evidence is specific to:

- the exact validated commit above;
- the included unit-test path;
- GCC 13.3.0 on the GitHub-hosted Ubuntu environment;
- the exercised mesh configuration.

It does not by itself prove absence of all memory errors, undefined behavior, data races, deadlocks, starvation, numerical instability, or correctness for arbitrary graph sizes and workloads.

Final acceptance into the Current Baseline remains a separate operator decision.


## Expanded invariant and stress validation

A broader test surface was added without changing the production header.

Validated code/workflow head:

`105fe83137a61f21473612b9a008989489f386e6`

GitHub Actions:

- Workflow: `Adaptive Mesh Validation`
- Run: `#4`
- Run ID: `35637241950`
- Conclusion: `success`

Both sanitizer jobs executed two registered CTest targets:

1. `run_mesh_tests` — deterministic invariant and behavioral checks.
2. `run_mesh_stress_tests` — 32 nodes, 100 asynchronous simulation steps, periodic external shocks, finite-state and health-bound assertions.

Observed CI evidence:

| Job | CTest | Invariant suite | Stress suite | Sanitizer-reported error |
|---|---|---|---|---|
| Debug + ASan + UBSan | 2/2 PASS | PASS | PASS | none observed |
| TSan, assertions enabled | 2/2 PASS | PASS | PASS | none observed |

The expanded tests cover geometry, identity bounds, meta-evaluation categories, bridge state transitions, local reflex clamping, nearby-node connection behavior, duplicate protection in auto-connect, state/health finiteness, and repeated asynchronous simulation.

The production file `apps/adaptive-mesh/include/system_architecture.hpp` was unchanged by this test expansion.

### Interpretation boundary

Run #4 materially increases confidence in the exercised behavior, but it is not a proof for every graph topology, schedule interleaving, compiler, platform, or untested API call. In particular, direct `connectNodes()` input validation and arbitrary concurrent mutation of mesh topology are outside the exercised contract.
