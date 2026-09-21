# Pre-Acceptance Audit — SOAM 2.0 Compiled Runtime

## Repository state

- Base branch: `main`
- Base SHA: `fde04915eb80796a2672f4d7c57f9d45c3b8cb19`
- Validated head: `45c9c5a490b4f73b4c125d3a5950c60dc1d33a25`
- Compare state at audit: ahead 8, behind 0
- PR: #4
- Mergeability at audit: mergeable

## Changed surface

Only the Phase B runtime surface is introduced:

- compiled static runtime library;
- runtime public interface;
- runtime implementation;
- runtime behavior test;
- topology transaction test;
- worker-pool lifecycle test;
- dedicated sanitizer workflow;
- migration provenance.

No existing Current Baseline source file is modified.

## Runtime trust boundary

Phase B may mutate only its own in-memory mesh runtime through its public API.

It has no:

- production execution capability type;
- authority derivation policy;
- transition eligibility evaluator;
- K11/K12 transition commit path;
- network or external-system integration;
- filesystem mutation API;
- shell/process execution surface.

## Adaptation from source archive

The source archive combines runtime implementation with later production-authority/K12 machinery.
That mixed translation unit was not imported wholesale.

The migrated runtime preserves the relevant Phase B mechanisms:

- compiled PIMPL boundary;
- persistent worker pool;
- topology validation;
- transactional batch edge preparation before publication;
- buffered parallel simulation before state publication;
- shared/exclusive topology synchronization.

Later authority and production-transition mechanisms remain excluded.

## Semantic boundaries

`simulationStepAsync()` is a compatibility name only in Phase B and is blocking.
Parallel computation occurs internally through the persistent worker pool.

Node IDs are intentionally constrained to insertion indices in this phase.

Direct duplicate connections are rejected rather than silently duplicated.

## Validation

GitHub Actions run #1, ID `35643399828`:

- Debug + ASan + UBSan: 3/3 PASS
- assertion-enabled TSan: 3/3 PASS
- no sanitizer-reported error observed in reviewed logs

## Audit disposition

Technical state: `verified`.

Acceptance state: `acceptance-pending`.

No merge is authorized by this audit.
