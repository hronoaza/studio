# SOAM 2.0 D8A — SourceCaptureId Generation Decision v1

## Status

- Related design: `DESIGN_AMENDMENT.md`
- Decision class: identity-generation candidate
- Runtime implementation: none
- Current Baseline effect: none
- Acceptance status: candidate
- Applies to: D8A `SourceCaptureId`

---

## 1. Selected generation family

Candidate v1 generation family:

```text
opaque random 128-bit identifier
generated from an operating-system-backed CSPRNG
```

The identifier is exactly 16 bytes.

It has no textual UUID semantics, no embedded timestamp, no node ID, no process
ID and no authority meaning.

The wire representation remains the 16 opaque bytes already reserved by
`CanonicalEnvelopeV1`.

---

## 2. Why this family is selected

The D8A capture identity must remain unique across:

- concurrent captures;
- multiple runtime instances;
- process restarts;
- machine restarts;
- implementation-version transitions.

A process-local monotonic counter cannot satisfy that domain without persistent
coordination.

A composite runtime-instance/counter design still requires a separately unique
runtime-instance identity and therefore shifts rather than removes the same
problem.

A persistent monotonic namespace adds durable state, locking, recovery and
allocation failure semantics to the source-capture path before those mechanisms
are otherwise required.

A random 128-bit opaque identifier provides the narrowest D8A mechanism:

- no durable counter;
- no clock dependency;
- no global coordinator;
- no dependency on relationship or state values;
- no semantic information leakage through the ID.

---

## 3. Required randomness source

The implementation MUST use an operating-system-backed cryptographic random
source suitable for generating 128 unpredictable bits.

This requirement is about randomness quality and collision resistance.

`SourceCaptureId` is not secret and unpredictability is not used as an
authorization property.

The implementation MUST NOT generate IDs from:

- `std::rand`;
- a deterministic default-seeded PRNG;
- current timestamp alone;
- thread ID;
- process ID;
- relationship IDs;
- state version;
- relationship generation;
- hashes of source measurements.

---

## 4. Uniqueness domain

The declared uniqueness domain is:

> all `SourceCaptureId` values emitted by all compliant D8A runtime instances
> that may contribute evidence to the same provenance/evidence ecosystem,
> including across process and machine restarts.

The design provides probabilistic uniqueness, not a mathematical no-collision
proof.

For uniformly random 128-bit values, the birthday-bound collision probability
after `n` captures is approximately:

```text
p ≈ n² / (2 * 2^128)
```

Illustrative bounds:

- at `10^9` captures: approximately `1.47e-21`;
- at `10^12` captures: approximately `1.47e-15`.

These are architecture-scale risk estimates, not runtime guarantees.

---

## 5. Collision semantics

A collision must never be intentionally tolerated as "same capture".

Required semantic rule:

```text
same SourceCaptureId => same capture identity claim
```

Therefore an implementation that detects an ID collision within any locally
known active/recent identity set MUST:

- reject the candidate ID;
- generate a new candidate;
- never publish two distinct snapshots with the detected same ID.

A global forever-history collision registry is not required by D8A v1.

Long-term duplicate detection, if later required across persisted evidence,
belongs to the evidence/provenance storage layer.

---

## 6. Generation failure

If the OS random source is unavailable or fails:

```text
no valid SourceCaptureId
-> no source snapshot publication
```

The implementation MUST NOT silently fall back to:

- a weak PRNG;
- timestamp;
- zero/default ID;
- a counter with a different uniqueness domain.

Failure is fail-closed.

---

## 7. Reserved invalid value

Candidate invalid/sentinel representation:

```text
00000000000000000000000000000000
```

All-zero `SourceCaptureId` is reserved and MUST NOT be published as a valid
capture identity.

If the random source returns all-zero bytes, the implementation retries.

This gives tests and default/uninitialized representations a stable invalid
value without conflating it with a real capture.

---

## 8. Retry policy

Generation conceptually follows:

```text
repeat:
    candidate = OS_CSPRNG_128()
    if candidate == all_zero:
        continue
    if locally_detected_collision(candidate):
        continue
    return candidate
```

A production implementation MUST impose a finite retry/failure policy so a
broken randomness provider cannot create an infinite loop.

Candidate limit:

`8 attempts`

If no valid ID is obtained within the limit, capture fails closed.

The exact limit remains parameter-reviewable but must be finite.

---

## 9. Thread safety

The random-source integration must support concurrent D8A captures.

Required observable property:

```text
concurrent successful captures
-> distinct valid SourceCaptureId values
```

No global topology lock may be held while invoking an external randomness
provider if that provider can block or call external code.

Preferred structure:

```text
prepare SourceCaptureId
-> acquire coherent capture lock
-> validate/collect source facts
-> construct snapshot
-> publish
```

or another ordering that preserves the same capture-event binding and failure
atomicity.

The implementation design must prove that an ID prepared before lock
acquisition cannot become bound to facts from more than one capture event.

---

## 10. Capture-event binding

Preparing an ID before reading source facts is permitted only as an internal
reservation.

The identity becomes an accepted `SourceCaptureId` only when the corresponding
snapshot is successfully constructed/published.

An abandoned reserved ID is never reused.

Therefore:

```text
reserved ID != published capture identity
```

until snapshot publication succeeds.

---

## 11. Copy/move behavior

The random generator is invoked only for a new capture operation.

It is NOT invoked for:

- copying a snapshot;
- moving a snapshot;
- D8B envelope construction;
- canonical serialization;
- digest recomputation.

Those operations preserve the existing 16-byte `SourceCaptureId`.

---

## 12. D8B wire compatibility

The selected D8A representation maps directly to:

```text
CanonicalEnvelopeV1.SourceBinding.SourceCaptureId
= 16 opaque bytes
```

No textual formatting or UUID byte-order conversion is permitted.

The bytes generated by D8A are the bytes serialized by D8B.

---

## 13. Security boundary

A random `SourceCaptureId` provides only collision-resistant identity.

It does not provide:

- producer authentication;
- authorization;
- confidentiality;
- tamper resistance;
- proof of source truth;
- replay authorization;
- capability semantics.

Required distinction:

```text
random_capture_id != capability
random_capture_id != authenticated_provenance
```

---

## 14. Required tests

Implementation acceptance should include:

1. valid IDs are exactly 16 bytes;
2. all-zero ID is never published;
3. repeated unchanged captures receive distinct IDs;
4. concurrent captures receive distinct IDs;
5. copy/move preserves the same ID;
6. deterministic/failing fake randomness source can exercise retry paths;
7. all-zero candidate triggers retry;
8. injected duplicate candidate triggers retry when locally detectable;
9. eight consecutive invalid candidates produce capture failure;
10. random-source failure produces no snapshot;
11. failure does not mutate live mesh state;
12. caller cannot choose or replace the ID;
13. D8B serialization preserves all 16 bytes exactly.

Tests must use an injectable test-only randomness seam rather than depending on
astronomically unlikely real collisions.

---

## 15. Public API constraint

The randomness provider itself MUST NOT become part of the ordinary public
runtime API.

Production callers must not be able to inject a chosen ID provider into the
normal D8A capture path.

Any deterministic provider seam is test/internal-only and must not alter the
canonical public class definition across build modes.

This specifically avoids repeating the configuration-dependent public API/ODR
problem identified in the older independent audit.

---

## 16. Decision summary

Candidate v1 decision:

```text
SourceCaptureId:
    width: 128 bits / 16 opaque bytes
    generator: OS-backed CSPRNG
    all-zero: reserved invalid
    generation scope: every new successful D8A capture event
    retry: finite, candidate 8 attempts
    persistence: none required
    timestamp embedding: none
    authority semantics: none
```

---

## 17. Remaining gate

If this generation decision is accepted, the D8A amendment becomes precise
enough for implementation design.

The next step is not implementation yet.

First define:

- internal C++ representation of the 16-byte ID;
- test-only randomness seam without public API drift;
- exact capture ordering relative to topology lock;
- compile-fail misuse cases.

Only after that should PR #8 be amended and fully revalidated.
