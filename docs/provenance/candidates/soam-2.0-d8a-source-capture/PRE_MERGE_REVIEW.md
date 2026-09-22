# Final Pre-Merge Review — SOAM 2.0 D8A Source Capture

## Review state

- PR: #8
- Branch: `candidate/soam-2.0-d8a-native-provenance`
- Current PR head:
  `a283be80ce422f04d492752173bc7a63858db7d6`
- Validated implementation/test head:
  `a7ed4edcf83b5787852265f99fc77ef4b57471d4`
- Current canonical `main`:
  `f7477ca28cbfe1d94470533d7ae44a7ac99d9b60`
- Mergeable at review: yes
- Review disposition: **READY FOR EXPLICIT ACCEPTANCE**
- Merge authorization: not granted by this document

---

## 1. Exact-head validation

The amended runtime/test implementation was validated on exact head:

`a7ed4edcf83b5787852265f99fc77ef4b57471d4`

Successful workflows:

- Runtime Validation #38 / `35702949626`;
- D8A Source Capture Validation #24 / `35702949664`;
- Live Evaluator Validation #35 / `35702949681`;
- D7 Provenance Gate Validation #33 / `35702949673`.

Every reviewed Debug ASan/UBSan and TSan job reports:

`100% tests passed, 0 tests failed out of 11`

No reviewed log contains an ASan, UBSan or TSan failure.

---

## 2. Changes after validated implementation head

Comparison:

```text
a7ed4edcf83b5787852265f99fc77ef4b57471d4
..
a283be80ce422f04d492752173bc7a63858db7d6
```

contains exactly three commits and only these files:

- `docs/provenance/candidates/soam-2.0-d8a-source-capture/CI_VALIDATION.md`
- `docs/provenance/candidates/soam-2.0-d8a-source-capture/AUDIT.md`
- `docs/provenance/candidates/soam-2.0-d8a-source-capture/CANDIDATE_RECORD.md`

No runtime source, header, test, CMake or workflow definition changed after the
validated implementation/test head.

Therefore the cited exact-head CI remains applicable to the current executable
delta.

---

## 3. Divergence from current main

Comparison of current PR head against current `main`:

- status: diverged;
- PR ahead: 51 commits;
- PR behind: 1 commit;
- mergeable: yes.

The one commit present on `main` after the PR merge-base adds only:

`docs/architecture/SOAM_PROVENANCE_REFINEMENT_MAP.md`

No runtime/source/test file is changed by that behind-main commit.

Therefore the branch divergence does not introduce a known executable conflict
with D8A.

---

## 4. Runtime scope reviewed

The PR executable delta introduces or modifies:

- neutral `ProductionRelationshipLocator`;
- restricted-origin `ProductionRelationshipSourceSnapshot`;
- immutable 16-byte `SourceCaptureId`;
- internal OS-backed capture-ID generator;
- source-capture integration;
- live evaluator migration to the neutral locator;
- SourceCaptureId lifecycle/concurrency tests;
- compile-fail misuse gates;
- D8A validation workflow/CMake registration.

No D8B provenance-envelope implementation is included.

No D8C admissibility implementation is included.

No D8D interpretation implementation is included.

No authority/capability/commit path is included.

---

## 5. SourceCaptureId boundary

Verified candidate properties:

```text
one successful recapture event -> distinct SourceCaptureId

copy/move -> preserves SourceCaptureId

SourceCaptureId
!= relationship generation
!= runtime state version
!= provenance item identity
!= digest
!= authentication
!= authority capability
```

Generation is outside the topology lock.

Source facts remain captured under the coherent topology/state shared lock.

Random-source failure fails closed and publishes no snapshot.

---

## 6. Negative-contract evidence

Compile-fail gates verify ordinary public callers cannot:

- inject D7 provenance evidence;
- forge a D8A source snapshot;
- default-construct `SourceCaptureId`;
- construct `SourceCaptureId` from arbitrary bytes;
- mutate the capture ID exposed by a snapshot.

These gates passed in both reviewed sanitizer jobs of the D8A workflow.

---

## 7. Evidence limitations

The current validation evidence establishes behavior on the cited Linux GitHub
Actions runners.

The implementation contains:

- Windows BCrypt randomness path;
- Apple/BSD `arc4random_buf` path.

Those platform-specific branches were not exercised by the cited Linux runs.

No cross-platform validation claim is made for them.

The review also does not claim:

- formal refinement proof between C++ and TLA+;
- authenticated provenance;
- cryptographic producer identity;
- D8B/D8C/D8D readiness;
- production authority readiness.

---

## 8. Documentation consistency

The current:

- `CI_VALIDATION.md`;
- `AUDIT.md`;
- `CANDIDATE_RECORD.md`;
- PR body

all identify:

`a7ed4edcf83b5787852265f99fc77ef4b57471d4`

as the validated implementation/test head and do not reuse the former
`39ca8ff...` validation as evidence for amended runtime code.

---

## 9. Final review disposition

No blocking inconsistency was found between:

- the current D8A architecture boundary;
- the amended SourceCaptureId semantics;
- the exact validated implementation/test head;
- the documentation-only post-validation commits;
- the one documentation-only commit by which the branch is behind `main`.

Disposition:

```text
technical evidence: sufficient for candidate acceptance on cited Linux CI
architecture boundary: coherent
mergeability: yes
acceptance: requires explicit Root Operator decision
merge: not authorized by this review
```

If explicit acceptance is granted, PR #8 may proceed to merge into Current
Baseline.
