# D8C Supporting Contract — Retained Source Evidence

## Status

- Purpose: independent source re-derivability input for D8C
- Runtime implementation: none
- Current Baseline effect: none
- Acceptance status: candidate

---

## 1. Goal

Provide an immutable retained source record that is independent of the D8B
envelope being evaluated.

This supports the executable meaning of:

`ReDerivable(x)`

without treating envelope self-comparison as re-derivation.

---

## 2. Origin

The only accepted v1 source of a retained record is a restricted-origin D8A:

`ProductionRelationshipSourceSnapshot`

Conceptual chain:

```text
D8A source snapshot
-> SourceEvidenceRecorder
-> RetainedSourceEvidenceRecord
-> SourceEvidenceStore
```

The recorder MUST NOT create the record from:

- ProductionProvenanceEnvelope;
- canonical envelope bytes;
- D8C metadata;
- caller-supplied raw source fields.

---

## 3. Record contents

Candidate immutable record:

```text
RetainedSourceEvidenceRecord
├── SourceCaptureId
├── sourceNodeId
├── targetNodeId
├── relationshipGeneration
├── stateVersion
├── distance
├── orientationWeight
├── capacity
├── BridgeStatus
├── sourceState
├── targetState
├── sourceHealth
└── targetHealth
```

No interpretation/confidence/authority fields are permitted.

---

## 4. Identity rule

`SourceCaptureId` is the lookup key.

Required invariant:

```text
one accepted SourceCaptureId
-> at most one retained source record
```

A second byte-different record claiming the same SourceCaptureId is a
source-evidence integrity conflict.

It must not overwrite the first record.

---

## 5. Publication semantics

Preferred sequence:

```text
accepted immutable D8A snapshot
-> prepare detached retained record
-> validate exact copied fields
-> atomically publish into store keyed by SourceCaptureId
```

Failure:

- publishes no partial record;
- does not mutate D8A snapshot;
- does not mutate mesh/runtime state;
- does not fabricate a replacement SourceCaptureId.

---

## 6. Relationship to D8B

D8B envelope production and source retention are sibling consumers of the same
D8A snapshot.

```text
                 -> D8B envelope
D8A snapshot ----|
                 -> retained source record
```

Neither must be derived from the other.

D8B may succeed while retention fails.

In that case the envelope remains a valid D8B envelope but D8C v1 cannot
positively establish `SourceReDerivable`.

---

## 7. SourceEvidenceResolver

Candidate semantic interface:

```cpp
class SourceEvidenceResolver {
public:
    [[nodiscard]]
    SourceResolutionResult resolve(
        const SourceCaptureId& sourceCaptureId) const;
};
```

Candidate result states:

```text
found
not_found
resolver_failure
integrity_conflict
```

A result of `found` yields one immutable retained record.

The resolver performs no D8C policy decision itself.

---

## 8. Store semantics

V1 store requirements:

- immutable published records;
- lookup by exact 16-byte SourceCaptureId;
- no overwrite;
- no implicit synthesis on miss;
- no fallback to D8B envelope;
- deterministic conflict detection when duplicate key insertion is attempted.

Persistence across process restart is not automatically required for an
in-memory test implementation, but production admissibility claims must state
the retention scope actually provided.

---

## 9. Re-derivation comparison

D8C compares retained record against D8B envelope source fields.

Exact comparisons:

- IDs: exact;
- generation/state version: exact;
- enum: exact semantic tag/value;
- floating values: exact IEEE-754 binary64 bits.

No tolerance/epsilon comparison.

Required property:

```text
bit_difference in any retained source fact
-> SourceReDerivable fails
```

---

## 10. What this proves and does not prove

Matching retained source evidence proves:

> the D8B envelope's source fields can be independently reproduced from a
> separately retained copy of the authentic D8A capture event.

It does not prove:

- that the physical/world source was true;
- that producer identity is cryptographically authenticated;
- that the evidence is admissible under policy;
- that the relationship is still current/live.

---

## 11. Concurrency

Concurrent insertion of the same SourceCaptureId must result in exactly one of:

- identical duplicate recognized as already retained, if policy explicitly
  permits idempotent identical insertion;
- integrity conflict for differing content.

Preferred v1 rule:

- identical repeated insertion -> idempotent success;
- differing repeated insertion -> integrity conflict.

No last-writer-wins semantics.

---

## 12. Required tests

Before D8C implementation acceptance:

1. record can be created from authentic D8A snapshot;
2. caller cannot construct record from arbitrary raw fields;
3. exact SourceCaptureId is preserved;
4. all source facts are copied bit-exactly;
5. identical duplicate insertion is idempotent;
6. conflicting duplicate insertion is rejected;
7. lookup returns immutable record;
8. missing lookup returns not_found;
9. injected store/resolver failure is distinguishable from not_found;
10. D8B envelope cannot be used to create a retained record;
11. store failure mutates no live runtime state;
12. concurrent conflicting insertion is deterministic/fail-closed.

---

## 13. Non-goals

The source-evidence layer does not perform:

- provenance canonicalization;
- D8B digest construction;
- admissibility;
- interpretation;
- live freshness;
- authority;
- execution.

---

## 14. Acceptance gate

Before a positive D8C runtime path exists:

- record origin restriction must be implemented;
- store/resolver contract must be implemented;
- independent source comparison must be tested;
- no fallback to envelope self-reconstruction may exist.
