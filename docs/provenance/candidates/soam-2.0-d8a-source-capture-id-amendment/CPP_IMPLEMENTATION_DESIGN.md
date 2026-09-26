# SOAM 2.0 D8A — SourceCaptureId C++ Implementation Design

## Status

- Related amendment: `DESIGN_AMENDMENT.md`
- Generation decision: `GENERATION_DECISION_V1.md`
- Artifact class: implementation design
- Runtime implementation: none
- Current Baseline effect: none
- PR #8 effect: none
- Acceptance status: candidate

This document fixes the proposed C++ shape and internal boundaries required
before PR #8 may be amended.

---

## 1. Public value type

Candidate public type:

```cpp
#pragma once

#include <array>
#include <cstdint>

namespace AdaptiveMesh {

class SpatialAdaptiveMesh;

class SourceCaptureId final {
public:
    using Bytes = std::array<std::uint8_t, 16>;

    SourceCaptureId(const SourceCaptureId&) noexcept = default;
    SourceCaptureId& operator=(const SourceCaptureId&) noexcept = default;
    SourceCaptureId(SourceCaptureId&&) noexcept = default;
    SourceCaptureId& operator=(SourceCaptureId&&) noexcept = default;

    [[nodiscard]] const Bytes& bytes() const noexcept {
        return bytes_;
    }

    friend bool operator==(
        const SourceCaptureId&,
        const SourceCaptureId&) noexcept = default;

private:
    explicit SourceCaptureId(Bytes bytes) noexcept
        : bytes_(bytes) {}

    Bytes bytes_;

    friend class SpatialAdaptiveMesh;
};

} // namespace AdaptiveMesh
```

The exact header name is candidate:

`source_capture_id.hpp`

### Required properties

- exact logical width: 16 bytes;
- public copy/move allowed;
- no public default constructor;
- no public constructor from arbitrary bytes;
- no public mutation;
- byte access is read-only;
- equality compares all 16 bytes;
- no ordering semantics are required in v1.

---

## 2. Why no public default constructor

The all-zero value is reserved as invalid.

A public default constructor would make this possible:

```cpp
SourceCaptureId id{};
```

and would create an ordinary public object representing the reserved invalid
state.

D8A does not need such a state in its public API.

Therefore the public type is constructible only by copying/moving an already
valid ID.

---

## 3. Snapshot amendment

Candidate addition to
`ProductionRelationshipSourceSnapshot`:

```cpp
[[nodiscard]] const SourceCaptureId&
sourceCaptureId() const noexcept {
    return sourceCaptureId_;
}
```

Private construction becomes conceptually:

```cpp
ProductionRelationshipSourceSnapshot(
    SourceCaptureId sourceCaptureId,
    std::size_t sourceNodeId,
    std::size_t targetNodeId,
    std::uint64_t relationshipGeneration,
    std::uint64_t stateVersion,
    ...
) noexcept;
```

The snapshot stores:

```cpp
SourceCaptureId sourceCaptureId_;
```

The constructor remains private.

No caller-selectable capture ID enters the public capture API.

---

## 4. Internal generator boundary

The randomness provider must remain outside the public headers.

Candidate internal interface:

```cpp
namespace AdaptiveMesh::detail {

class SourceCaptureIdRandomSource {
public:
    virtual ~SourceCaptureIdRandomSource() = default;

    virtual bool fill(
        std::array<std::uint8_t, 16>& out) noexcept = 0;
};

}
```

This interface is internal-only and must not be installed/exported as part of
the supported public API.

The public definition of `SpatialAdaptiveMesh` and `SourceCaptureId` must not
change between normal/test/profile/scenario builds.

---

## 5. Production random source

The production implementation owns one internal OS-backed random source.

The design requirement is:

```text
fill(16 bytes)
-> OS-backed CSPRNG
-> success/failure only
```

No fallback generator is permitted.

Platform bindings are implementation details and must be isolated behind the
internal random-source implementation.

The D8A contract does not depend on a specific OS API name.

---

## 6. Test-only seam

Tests need deterministic failure/collision injection.

The seam must be internal, not conditional public API.

Preferred structure:

```text
SpatialAdaptiveMesh public class
    -> private Impl
        -> internal SourceCaptureIdRandomSource
```

Tests may construct/use a private/internal harness that owns the same internal
implementation or substitutes the random source through an internal factory.

Acceptable approaches include:

- an internal constructor/function in a non-installed header;
- a private implementation factory linked only into test targets;
- a test-only source file with access to an internal dependency-injection
  point.

Not acceptable:

- `#ifdef TEST` changing a public class declaration;
- a public constructor that accepts an ID generator;
- a public setter for the random provider;
- a scenario/profile-only friend in the canonical public header.

This preserves configuration-invariant public token definitions.

---

## 7. ID generation helper

Candidate internal helper contract:

```cpp
std::optional<SourceCaptureId>
tryGenerateSourceCaptureId(
    SourceCaptureIdRandomSource& source) noexcept;
```

Semantics:

```text
for attempt in [1..8]:
    bytes = randomSource.fill()
    if fill failed:
        return nullopt
    if bytes == all-zero:
        continue
    if locallyKnownCollision(bytes):
        continue
    return valid SourceCaptureId(bytes)

return nullopt
```

A random-source I/O failure fails immediately.

All-zero/collision candidates consume retry budget.

The exact local collision tracker is an implementation detail.

---

## 8. Collision tracking scope

D8A v1 does not maintain a permanent global history.

The implementation may maintain a bounded process-local set of IDs that are
currently live or recently issued, sufficient to make deterministic injected
collision tests observable.

This local tracker is defense-in-depth only.

The architecture-wide uniqueness property still comes from the 128-bit random
generation model.

The tracker must not become a provenance ledger.

---

## 9. Capture ordering

The preferred ordering is:

```text
1. Generate/reserve SourceCaptureId outside topology lock.
2. If generation fails -> return no snapshot.
3. Acquire topology/state shared lock.
4. Validate locator and relationship existence.
5. Read all D8A source facts coherently.
6. Construct immutable snapshot using the reserved SourceCaptureId.
7. Release lock.
8. Return/publish snapshot.
```

This avoids calling an OS randomness provider while holding the topology lock.

---

## 10. Abandoned reservations

Because the ID is generated before relationship validation under lock, some
reserved IDs may never be published.

Example:

```text
reserve ID
-> acquire lock
-> relationship missing
-> no snapshot
```

This is allowed.

Required semantics:

- the reserved ID is never observable;
- it is never reused;
- gaps have no meaning;
- D8A does not promise contiguous identity allocation.

Therefore:

```text
reserved ID != successful capture identity
```

until snapshot construction succeeds.

---

## 11. Why generation is outside the topology lock

The OS randomness call can have platform-dependent latency/failure behavior.

Holding the topology lock around it would unnecessarily extend the critical
section and would couple mesh liveness to the entropy provider.

The source facts remain coherent because the ID contains no source facts.

The binding becomes authoritative only when the snapshot is constructed under
the coherent capture lock.

---

## 12. Alternative ordering rejected for v1

Rejected:

```text
acquire topology lock
-> read facts
-> release lock
-> generate ID
-> construct snapshot
```

Reason:

This creates an intermediate detached fact set without its final capture
identity and complicates the proof that the identity and facts are published as
one immutable source-capture object.

Also rejected:

```text
acquire topology lock
-> call external OS/provider abstraction
-> generate ID
-> capture
```

Reason:

Potentially blocking/external work occurs inside the topology critical section.

---

## 13. Exception model

The preferred v1 capture-ID generation path is non-throwing.

Internal random source:

`fill(...) noexcept -> bool`

Generation helper:

`tryGenerateSourceCaptureId(...) noexcept -> optional<SourceCaptureId>`

The existing source snapshot constructor remains `noexcept`.

If an implementation requires allocation for collision tracking, that work must
not be able to create a partially published snapshot.

A simpler fixed/bounded tracker is preferred over adding throwing allocation to
the capture path.

---

## 14. Public API compile-fail requirements

The following must fail to compile from ordinary public headers:

### Default construction

```cpp
SourceCaptureId id{};
```

### Arbitrary-byte construction

```cpp
SourceCaptureId id{{0x01, ...}};
```

### Snapshot forgery with attacker ID

```cpp
ProductionRelationshipSourceSnapshot forged{
    attackerChosenId,
    ...
};
```

### Mutation

```cpp
snapshot.sourceCaptureId().bytes()[0] = 0x42;
```

The read-only byte accessor may be used by D8B serialization.

---

## 15. Runtime tests

Required D8A tests after implementation:

1. successful snapshot has 16-byte non-zero ID;
2. repeated capture without runtime mutation yields:
   - same relationship generation;
   - same state version;
   - distinct capture IDs;
3. copied snapshot preserves capture ID;
4. moved snapshot preserves capture event identity;
5. state mutation + recapture gives new capture ID;
6. relationship recreation + recapture gives new capture ID;
7. reverse-direction capture gets distinct capture ID;
8. concurrent successful captures produce distinct IDs;
9. invalid locator returns no snapshot and no ID is observable;
10. missing relationship returns no snapshot;
11. fake all-zero source retries;
12. fake duplicate source retries;
13. random-source hard failure returns no snapshot;
14. eight invalid candidates fail closed;
15. runtime state/topology are unchanged by ID-generation failure.

---

## 16. Regression requirements

The amendment must preserve:

- D7 negative provenance gate;
- C2 live evaluator behavior;
- neutral `ProductionRelationshipLocator`;
- source snapshot restricted-origin construction;
- no interpretation/confidence production;
- no authority/capability semantics.

The test suite count may increase, but all existing tests must continue to pass.

---

## 17. Sanitizer/concurrency validation

The amended exact implementation head must pass:

- Debug ASan/UBSan;
- TSan;
- ordinary runtime regression;
- source-snapshot tests;
- D7 compile-fail gate;
- D8A forged-snapshot compile-fail gate;
- new SourceCaptureId compile-fail misuse gates.

No prior exact-head validation may be reused as proof for the amended code.

---

## 18. D8B handoff

D8B reads:

```cpp
snapshot.sourceCaptureId().bytes()
```

and serializes the exact 16 bytes into:

```text
CanonicalEnvelopeV1.SourceBinding.SourceCaptureId
```

D8B never regenerates, normalizes or textualizes the ID.

---

## 19. Candidate implementation file split

Suggested structure:

```text
apps/soam-runtime/include/source_capture_id.hpp
apps/soam-runtime/include/production_relationship_source_snapshot.hpp
apps/soam-runtime/src/source_capture_id_internal.hpp
apps/soam-runtime/src/source_capture_id.cpp
apps/soam-runtime/src/system_architecture.cpp
apps/soam-runtime/tests/source_snapshot_tests.cpp
apps/soam-runtime/tests/compile_fail/d8a_default_construct_capture_id.cpp
apps/soam-runtime/tests/compile_fail/d8a_construct_capture_id_from_bytes.cpp
apps/soam-runtime/tests/compile_fail/d8a_mutate_capture_id.cpp
```

The exact internal filenames may change.

No internal random-source header should be installed as public API.

---

## 20. Implementation gate

PR #8 must not be amended until this implementation design is accepted.

If accepted, the next action is:

```text
amend PR #8 code
-> add SourceCaptureId
-> add internal OS-backed generator
-> add deterministic internal test seam
-> extend tests/compile-fail gates
-> run full exact-head CI
-> update D8A audit/evidence
-> return to explicit acceptance gate
```

No D8B runtime implementation should begin before that amended D8A boundary is
validated.
