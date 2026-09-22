# SOAM 2.0 D8D — Versioned Interpretation Design Contract

## Status

- Layer: D8D — Versioned Interpretation
- Document class: design-contract candidate
- Executable implementation: none
- Current Baseline effect: none
- Upstream baseline: accepted D8C Provenance Admissibility
  (`391536d444eeac8f7284f30f4e6241b1d944560c`)
- Downstream consumer: future interpretation-to-policy evidence path
- Acceptance status: design-review pending
- Authority effect: none

D8D is the first layer allowed to assign domain semantics to admissible
provenance.

It is not an authority layer.

---

## 1. Boundary

Input:

`AdmissibleProductionProvenance`

Output:

`VersionedProductionInterpretation`

Conceptual chain:

```text
accepted D8C AdmissibleProductionProvenance
-> D8D VersionedInterpretationPolicy
-> VersionedProductionInterpretation
-> STOP
```

A raw D8B `ProductionProvenanceEnvelope` is not a valid D8D input.

A D8C rejection is not a valid D8D input.

---

## 2. Primary invariant

```text
admissible provenance
!= interpretation
!= policy recommendation
!= transition eligibility
!= authority
```

D8D may interpret admissible evidence.

D8D MUST NOT:

- issue transition capability;
- make a production commit;
- bypass C1/C2;
- invent provenance;
- weaken D8C admissibility;
- convert a D8C rejection into success.

---

## 3. Required lineage preservation

Every D8D result must preserve the identities that established its upstream
evidence lineage:

- `SourceCaptureId`;
- `ProvenanceItemId`;
- `AdmissibilityDecisionId`;
- `PolicySnapshotId`;
- D8C admissibility policy ID/version;
- relationship generation;
- state version.

D8D adds its own interpretation identity/version; it does not replace upstream
identity.

Required invariant:

```text
interpretation lineage
= source capture lineage
+ provenance lineage
+ admissibility lineage
+ interpretation policy lineage
```

---

## 4. Interpretation policy identity

Candidate types:

`InterpretationPolicyId`

`InterpretationPolicyDescriptor`

Candidate descriptor:

```text
InterpretationPolicyDescriptor
- policyId
- majorVersion
- minorVersion
- implementationRevisionKind
- implementationRevision
```

Policy identity is independent from D8C policy identity.

The active D8D policy must be immutable for one interpretation event.

---

## 5. Interpretation event identity

Candidate type:

`InterpretationDecisionId`

Semantics:

> identity of one completed D8D interpretation event.

Candidate generation:

- 128-bit opaque bytes;
- OS-backed random generation family;
- all-zero invalid;
- restricted-origin;
- not caller-selectable.

Distinct from:

- SourceCaptureId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- PolicySnapshotId;
- CanonicalDigest.

Repeated interpretation of the same admissible provenance under the same
interpretation policy creates a new decision identity.

---

## 6. Candidate semantic outputs

D8D may produce the domain-semantic values needed by later policy layers.

Candidate v1 output:

```text
VersionedProductionInterpretation
├── InterpretationDecisionId
├── InterpretationPolicyDescriptor
├── upstream lineage identities
├── InteractionObservation
├── BridgeConfidence
└── InterpretationTrace
```

No `RequestedTransitionDirection` is produced directly by D8D.

No `eligible_for_authority_consideration` is produced directly by D8D.

Those remain downstream policy/eligibility responsibilities.

---

## 7. InteractionObservation

D8D may construct `InteractionObservation` only from explicitly documented
source facts in the accepted admissible provenance.

The interpretation policy must enumerate every source field used.

No hidden use of:

- `BridgeStatus` as a recommendation;
- current live runtime state outside the admissible input;
- caller-provided synthetic values;
- global mutable policy state

is permitted.

---

## 8. BridgeConfidence

`BridgeConfidence` is a semantic interpretation output, not provenance
integrity and not authority confidence.

Required distinction:

```text
BridgeConfidence
!= CanonicalDigest validity
!= source truth probability
!= authority confidence
!= transition eligibility
```

Its range, normalization and meaning must be fixed by the versioned
interpretation policy.

---

## 9. No BridgeStatus substitution

D7 established the negative boundary that `BridgeStatus` alone must not become
a requested production transition.

D8D therefore MUST NOT define mappings such as:

```text
NORMAL -> increase
DAMPING -> decrease
RECOVERY -> increase
ISOLATED -> disconnect
```

unless a future explicitly reviewed interpretation policy defines a larger
evidence function in which BridgeStatus is only one declared input and the
result is still not authority.

For v1, BridgeStatus may be used only as an explicitly named contextual feature,
not as a direct command mapping.

---

## 10. Deterministic interpretation policy

For fixed:

- admissible provenance;
- interpretation policy descriptor/revision;
- policy-owned constants;

the semantic output must be deterministic.

Required property:

```text
same admissible input
+ same interpretation policy revision
-> same InteractionObservation
+ same BridgeConfidence
+ same interpretation trace
```

Only `InterpretationDecisionId` differs between repeated evaluation events.

---

## 11. Policy-owned constants

Any normalization constants, thresholds, weights or transforms used by D8D are
part of the interpretation policy revision.

They MUST NOT be:

- hidden magic values;
- environment-variable overrides;
- caller-supplied per evaluation;
- mutable global runtime settings.

Changing a policy-owned constant requires a new policy revision.

---

## 12. Input-field declaration

Every D8D policy revision must publish an explicit field-usage manifest.

Candidate manifest entries:

```text
InterpretationInputField
- source field identity
- semantic role
- normalization rule
- required/optional status
```

Candidate v1 source fields may include:

- distance;
- orientation weight;
- capacity;
- source state;
- target state;
- source health;
- target health;
- BridgeStatus;
- relationship generation/state version only for lineage/context, not numeric
  semantic scoring unless explicitly justified.

No source field may be consumed without appearing in the manifest.

---

## 13. Normalization

Normalization must be versioned and deterministic.

For each numerical input, the policy must define:

- accepted domain;
- finite/non-finite handling;
- clipping policy, if any;
- transform;
- output range.

Fail closed on unsupported numeric states.

D8D must not silently coerce NaN/Infinity into ordinary confidence values.

---

## 14. Directionality

The interpretation policy must explicitly declare whether semantics are:

- symmetric with respect to source/target reversal; or
- directional.

If directional, reversing:

`source -> target`

to:

`target -> source`

may produce different observations/confidence, but this must be intentional,
versioned and tested.

No accidental directional dependence from field ordering is permitted.

---

## 15. Invalid interpretation result

Candidate failure type:

`ProductionInterpretationRejection`

Candidate reasons:

- unsupported interpretation policy;
- unsupported producer/schema lineage;
- unsupported numeric value;
- required source field unavailable;
- normalization failure;
- policy invariant violation;
- upstream lineage inconsistency;
- internal deterministic-evaluation failure.

Interpretation rejection is distinct from D8C rejection.

The admissible provenance remains admissible evidence even if a specific D8D
policy cannot interpret it.

---

## 16. Result carrier

Candidate public model:

```cpp
using ProductionInterpretationResult =
    std::variant<
        VersionedProductionInterpretation,
        ProductionInterpretationRejection>;
```

Candidate evaluator:

```cpp
class VersionedProductionInterpreter final {
public:
    [[nodiscard]]
    std::optional<ProductionInterpretationResult> interpret(
        const AdmissibleProductionProvenance& provenance,
        const InterpretationPolicySnapshot& policy) const;
};
```

Outer failure means infrastructure failure before a completed interpretation
decision exists.

Completed policy rejection is a typed rejection variant.

---

## 17. Restricted-origin result

`VersionedProductionInterpretation` must be restricted-origin.

Ordinary callers cannot:

- default construct it;
- construct it from raw `ProductionProvenanceEnvelope`;
- construct it from arbitrary observation/confidence values;
- replace upstream lineage;
- replace policy identity;
- inject a fake interpretation decision ID.

Only the D8D interpreter may construct production results.

---

## 18. Interpretation policy snapshot

One interpretation event must use one immutable
`InterpretationPolicySnapshot`.

Candidate contents:

```text
InterpretationPolicySnapshot
├── snapshot identity
├── InterpretationPolicyDescriptor
├── field-usage manifest
├── normalization constants
├── directional semantics
└── output invariants
```

No mutable global lookup during one interpretation event.

---

## 19. Output invariants

The D8D policy must define invariant ranges for semantic outputs.

Candidate requirements:

- `InteractionObservation` must satisfy the accepted domain type invariants;
- `BridgeConfidence` must be finite;
- confidence range must be explicitly fixed by policy;
- no output may encode authority or requested transition direction.

If output invariants fail, publish rejection, not a partially valid
interpretation.

---

## 20. InterpretationTrace

Candidate immutable trace contains:

- policy snapshot identity;
- policy descriptor/revision;
- declared input fields actually consumed;
- normalized intermediate values required for independent reproduction;
- final observation/confidence values.

The trace is evidence for reproducibility, not an execution command.

No secret/internal authority data belongs in the trace.

---

## 21. Reproducibility

The D8D layer must support independent reproduction of a semantic result from:

- the accepted `AdmissibleProductionProvenance`;
- the exact interpretation policy revision;
- documented policy constants.

Required claim:

```text
same admitted evidence
+ same policy revision
-> reproducible semantic output
```

This is separate from D8C source re-derivability.

---

## 22. Relationship to C1/C2

D8D produces semantics, not transition eligibility.

A future downstream chain may use:

`VersionedProductionInterpretation`

to construct policy evidence.

C1/C2 remain responsible for eligibility and live revalidation.

Required distinction:

```text
interpretable
!= eligible
!= live/current
!= authorized
```

---

## 23. Relationship to authority

No D8D type carries:

- grant;
- nonce;
- expiry;
- capability scope;
- revocation state;
- commit authorization.

D8D cannot authorize a production transition.

---

## 24. Candidate v1 policy question

Before implementation, one semantic issue must be selected explicitly:

> What exact deterministic function maps accepted source facts to
> `InteractionObservation` and `BridgeConfidence`?

D8D design must not invent this mapping from convenient runtime fields merely to
make tests pass.

The mapping should be justified by an accepted domain contract or prior accepted
behavioral semantics.

Until that function is fixed, D8D remains design-only.

---

## 25. Verification plan

Before implementation acceptance, tests should include:

1. raw D8B envelope cannot enter D8D;
2. D8C rejection cannot enter D8D;
3. admissible provenance produces deterministic semantic output under fixed
   policy;
4. repeated interpretation has distinct InterpretationDecisionId;
5. repeated interpretation preserves identical semantic output;
6. SourceCaptureId preserved;
7. ProvenanceItemId preserved;
8. AdmissibilityDecisionId preserved;
9. PolicySnapshotId preserved;
10. interpretation policy identity/revision preserved;
11. undeclared input field cannot affect output;
12. unsupported numeric value fails closed;
13. reverse-direction behavior matches declared symmetry/asymmetry;
14. policy constant change requires revision change;
15. output invariant failure rejects;
16. caller cannot forge VersionedProductionInterpretation;
17. caller cannot inject InteractionObservation/BridgeConfidence as trusted
    interpretation;
18. no RequestedTransitionDirection generated by D8D;
19. no eligibility/authority type generated by D8D;
20. all D7/D8A/D8B/D8C/C2 regression suites remain green.

---

## 26. Known design blocker

The accepted architecture defines the D8D boundary but does not yet provide an
accepted numerical/semantic interpretation function.

Therefore:

```text
D8D structural contract: definable now
D8D semantic mapping: acceptance-pending
D8D runtime implementation: blocked
```

This is intentional.

A fabricated mapping would violate the evidence-first architecture.

---

## 27. Next gates

Before D8D implementation:

1. review this structural contract;
2. identify the accepted domain semantics for `InteractionObservation`;
3. identify the accepted semantics/range for `BridgeConfidence`;
4. define exact field-usage manifest;
5. define normalization and directional behavior;
6. define interpretation policy identity/revision;
7. perform final design acceptance review;
8. only then implement D8D on a separate candidate branch.
