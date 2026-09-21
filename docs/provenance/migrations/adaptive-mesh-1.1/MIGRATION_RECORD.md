# Migration Record — Adaptive Mesh 1.1

## Status

- Migration stage: `source-identified`
- Acceptance status: not accepted into Current Baseline
- Target repository: `hronoaza/studio`
- Migration branch: `migration/adaptive-mesh-1.1`
- Base branch: `main`
- Base commit: `31d3df9433e33372e3b8cb1a619b532f36ea3e1b`

## Evidence bundle

Uploaded validation bundle:

- File: `adaptive-mesh-cpp-validation(1).zip`
- SHA-256: `c1751307d7458dcc017a2dc50ff67f2e1bf101bd23427d617f75184eb3cad6ae`
- File count: 31
- Manifest verification: all entries in `SHA256SUMS.json` matched the extracted evidence files.

The bundle records an earlier validated source archive with SHA-256:

`10087b027b0d8a97fb76c317f184246f18de1f44e144321876d31d04e2659ee7`

These hashes identify different artifacts: the first is the current evidence bundle; the second is the source archive referenced by the evidence.

## Historical validation finding

The supplied evidence does not establish a fully passing sanitizer run.

- Debug configuration: configure/build succeeded; CTest stopped on an assertion before `simulationStepAsync()`.
- Release configuration: CTest returned success, but assertions were disabled by `-DNDEBUG`.
- TSan configuration: configure/build succeeded; CTest stopped on the same assertion before the asynchronous simulation step.

The reproduced test mismatch is geometric: nodes at `(0,0,0)` and `(2,2,2)` are separated by `sqrt(12) ≈ 3.464`, while the test calls `autoConnectNearbyNodes(3.0)` and then expects a new 0↔2 bridge.

This evidence is preserved as historical input. It must not be rewritten to imply that sanitizer coverage passed.

## Source candidate

The evidence bundle contains a C++20 header-only candidate implementation with:

- `IdentityInvariant`
- `MetaEvaluator`
- `SignalCategory`
- `SpatialBridge`
- `AutopoieticNode`
- `SpatialAdaptiveMesh`
- local reflex filtering
- adaptive bridge capacity
- isolation/recovery states
- asynchronous simulation via `std::jthread`

Source import and any test correction are separate migration steps from historical evidence preservation.

## License/provenance boundary

The source header declares GNU AGPLv3 or a commercial license upon request. This migration record preserves that declaration; it does not choose or alter the repository's project license.

## Acceptance gate

Before acceptance into Current Baseline:

1. preserve the evidence bundle identity and historical failures;
2. import the source candidate separately from evidence;
3. correct or replace the invalid geometric test assumption without changing the algorithm merely to satisfy the old assertion;
4. run Debug with ASan+UBSan through `simulationStepAsync()`;
5. run an assertion-enabled TSan path through `simulationStepAsync()`;
6. distinguish sanitizer evidence from ordinary Release success;
7. review license and provenance boundaries;
8. review the exact final migration head;
9. require explicit Root Operator acceptance before merge.
