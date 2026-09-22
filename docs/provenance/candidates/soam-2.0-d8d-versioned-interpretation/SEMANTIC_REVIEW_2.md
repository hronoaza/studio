# SOAM 2.0 D8D — Semantic Review 2

## Review target

- accepted runtime field semantics;
- accepted domain output semantics;
- proposed InterpretationPolicyV1.

## Findings

### SR2-001 — effective coupling is an accepted runtime quantity

The runtime already defines:

```text
capacity
* (1 / (1 + 0.1 * distance))
* orientationWeight
```

as effective coupling.

Its inputs are validated so the value is bounded in [0,1].

Therefore reusing that exact function avoids inventing a second geometry/capacity
normalization.

PASS as a technical basis.

### SR2-002 — compatibility equivalence is a new policy decision

The runtime does not state that effective coupling and semantic compatibility
are identical concepts.

Therefore:

```text
compatibility := effectiveCoupling
```

must be accepted as a new D8D policy decision, not described as an existing
runtime truth.

PASS with that provenance qualification.

### SR2-003 — health is bounded but confidence mapping is new

Runtime health is accepted in [0,1].

No accepted source maps health to BridgeConfidence.

The candidate rule:

```text
confidence := min(sourceHealth, targetHealth)
```

is therefore a new conservative interpretation policy.

Advantages:

- no additional constants;
- symmetric;
- bounded;
- weakest endpoint limits confidence.

No stronger claim is justified.

PASS as candidate policy semantics.

### SR2-004 — raw state is not reproducibly normalizable

State meaning depends on node baseline/maxEpsilon.

Those invariant parameters are not in D8A/D8B provenance.

Therefore raw state is excluded from v1 semantic scoring.

PASS.

### SR2-005 — BridgeStatus exclusion avoids command leakage/double count

BridgeStatus is excluded.

Capacity already reflects the bridge update mechanism, while D7 prohibits
status-to-command substitution.

PASS.

### SR2-006 — directionality is explicit

orientationWeight is directional by runtime definition.

Thus v1 is intentionally directional.

PASS if accepted as policy semantics; tests required.

## Disposition

```text
source field semantics: sufficient
compatibility candidate: defined
confidence candidate: defined
BridgeStatus handling: defined
state handling: defined
directionality: defined
semantic policy provenance: explicit
overall: READY FOR FINAL DESIGN REVIEW
```

This review does not itself accept the policy.
