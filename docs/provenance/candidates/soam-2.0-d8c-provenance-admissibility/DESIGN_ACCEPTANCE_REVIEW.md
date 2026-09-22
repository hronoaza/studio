# SOAM 2.0 D8C — Design Acceptance Review

## Review target

- PR: #15
- Base Current Baseline:
  `09b531bc2950a1e219872b7f6291212bdb3b80f3`
- Runtime changes: none
- Scope: D8C architecture/design
- Disposition: **ACCEPTED — IMPLEMENTATION AUTHORIZED**

---

## 1. Layer boundary

The design preserves:

```text
D8B well-formed provenance
-> D8C admissibility
-> D8D interpretation
-> later eligibility/authority
```

D8C cannot manufacture interpretation or authority.

PASS.

---

## 2. Independent re-derivability

The revised design forbids envelope self-reconstruction as proof of
`ReDerivable(x)`.

Retained source evidence is produced independently from the authentic D8A
snapshot through a restricted-origin retention path.

PASS.

---

## 3. Metadata consistency

The D8B metadata-view amendment requires typed metadata and canonical bytes to
originate from one structured producer input.

D8C verifies CanonicalDigest before relying on metadata for a positive result.

PASS at design level.

---

## 4. Policy coherence

One decision uses one immutable
`ProvenanceAdmissibilityPolicySnapshot`.

Producer/schema/dependency policy cannot drift independently during one
evaluation.

PASS.

---

## 5. Required dependency semantics

The policy snapshot distinguishes:

- every present dependency being acceptable;
- every required dependency being present.

This closes a fail-open omission case.

PASS.

---

## 6. Producer revision policy

D8C v1 requires an exact recognized implementation revision.

No wildcard revision is accepted in the production policy candidate.

This matches D8B's implementation-revision binding.

PASS.

---

## 7. Source comparison

Retained source facts are compared bit-exactly.

No floating tolerance can convert byte-different source evidence into a positive
re-derivation result.

PASS.

---

## 8. Decision identity/result semantics

Accepted and rejected completed decisions both carry one D8C-owned
`AdmissibilityDecisionId`.

Infrastructure failure is kept distinct from a policy rejection.

PASS.

---

## 9. Resolver side-effect bound

Production evaluation performs at most one source resolver call and does not
invoke it after a terminal local rejection.

PASS.

---

## 10. C2 freshness separation

D8C source-lineage consistency does not claim current-live transition freshness.

C2 retains responsibility for live revalidation.

PASS.

---

## 11. Remaining implementation dependencies

Before positive D8C runtime can be accepted, implementation must provide:

1. D8B `ProvenanceMetadataView` amendment;
2. retained source evidence recorder/store/resolver;
3. immutable policy snapshot types/registry;
4. decision identity/result API;
5. D8C evaluator;
6. exact-head conformance/failure/compile-fail/sanitizer tests.

These are implementation tasks, not unresolved design semantics.

---

## 12. Final disposition

```text
admissibility decomposition: PASS
independent re-derivability: PASS
metadata consistency model: PASS
policy snapshot coherence: PASS
dependency omission handling: PASS
decision/result API: PASS
layer separation: PASS
overall design: READY FOR EXPLICIT DESIGN ACCEPTANCE
implementation authorization: pending explicit acceptance
merge authorization: not granted by this review
```

If explicitly accepted, implementation should proceed on a separate candidate
branch from Current Baseline, with PR #15 retained as the design artifact.


---

## Explicit acceptance record

The Root Operator explicitly accepted the D8C v1 design after this review.

Recorded consequence:

```text
D8C design: accepted
supporting-contract implementation: authorized
D8C runtime implementation: authorized on controlled candidate branch
PR #15 merge: not implied by design acceptance
Current Baseline: unchanged by this acceptance record
```

Implementation proceeds separately from Current Baseline.
