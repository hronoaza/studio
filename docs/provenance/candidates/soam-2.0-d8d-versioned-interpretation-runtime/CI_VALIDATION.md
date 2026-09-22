# CI Validation — SOAM 2.0 D8D Versioned Interpretation

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `candidate/soam-2.0-d8d-versioned-interpretation-runtime`
- Base Current Baseline: `391536d444eeac8f7284f30f4e6241b1d944560c`
- Exact validated implementation/test head:
  `f128203036c9bb6031ea753c821bdcba233d9d73`

## Successful workflows on exact head

| Workflow | Run | Result |
|---|---:|---|
| Runtime Validation | #55 / `35713247314` | success |
| D8A Source Capture Validation | #41 / `35713247321` | success |
| D8B Provenance Envelope Validation | #14 / `35713247305` | success |
| D8C Provenance Admissibility Validation | #6 / `35713247352` | success |
| D8D Versioned Interpretation Validation | #4 / `35713247312` | success |
| Live Evaluator Validation | #52 / `35713247350` | success |
| D7 Provenance Gate Validation | #50 / `35713247396` | success |

Every workflow executed:

- Debug + AddressSanitizer + UndefinedBehaviorSanitizer;
- ThreadSanitizer with assertions enabled.

All fourteen reviewed jobs report:

`100% tests passed, 0 tests failed out of 30`

No reviewed job reports an ASan, UBSan or TSan failure.

---

## D8D suite additions

The 30-test suite includes all previously accepted tests plus:

- D8D deterministic/runtime interpretation test;
- InterpretationDecisionId default-construction compile-fail;
- InterpretationDecisionId arbitrary-byte construction compile-fail;
- VersionedProductionInterpretation default-construction compile-fail;
- arbitrary observation/confidence trusted-construction compile-fail;
- InterpretationPolicySnapshot default-construction compile-fail.

---

## Normative vector evidence

D8D tests independently exercise the accepted v1 equations:

```text
attenuation =
    1 / (1 + 0.1 * distance)

compatibility =
    capacity * attenuation * orientationWeight

confidence =
    min(sourceHealth, targetHealth)
```

The suite verifies fixed IEEE-754 binary64 fixtures including:

- full coupling/full health;
- zero orientation;
- exact neutral coupling at distance 10;
- zero-confidence suppression;
- directional capacity-0.8 fixture;
- invalid/non-finite input rejection.

The runtime implementation does not provide the test oracle.

---

## End-to-end evidence

The D8D test exercises:

```text
D8A source snapshot
-> retained source evidence
-> D8B provenance envelope
-> D8C admissibility
-> D8D interpretation
```

The resulting D8D success preserves:

- SourceCaptureId;
- ProvenanceItemId;
- AdmissibilityDecisionId;
- D8C PolicySnapshotId.

Repeated interpretation under the same policy preserves semantic output and
creates a distinct InterpretationDecisionId.

---

## Directionality evidence

The end-to-end fixture uses a directed z-axis relationship.

Forward and reverse captures have different accepted runtime
`orientationWeight` values.

D8D produces different compatibility values as required by the accepted
directional InterpretationPolicyV1.

This is intentional policy behavior, not accidental source/target ordering.

---

## Policy rejection versus infrastructure failure

The final validated implementation distinguishes:

- unrecognized interpretation policy revision -> typed
  `ProductionInterpretationRejection`;
- D8D decision-ID generation failure -> outer infrastructure failure
  (`nullopt`).

The earlier implementation that returned `nullopt` for policy mismatch was
corrected before the cited exact-head validation.

---

## Implementation-revision binding

Production D8D policy descriptor uses:

`implementationRevisionKind = 1`

The revision digest is SHA-256 over a deterministic implementation manifest
containing hashes of:

- `include/versioned_interpretation.hpp`;
- `src/detail/versioned_interpretation_internal.hpp`;
- `src/versioned_interpretation.cpp`.

The manifest is independent of documentation-only commits and build-directory
paths.

---

## Validation history

Initial D8D head `f702ac4...` failed to build because the runtime target did
not expose the public domain-header include path required by the new D8D public
API.

That failed head is retained as evidence and is not treated as validation.

Head `cfb3c267...` passed the full matrix after the include dependency was
corrected.

A subsequent API review identified a result-channel mismatch for policy
rejection. The implementation was corrected and revalidated at exact head
`f128203036c9bb6031ea753c821bdcba233d9d73`.

Only `f128203036c9bb6031ea753c821bdcba233d9d73` is the final implementation/test evidence point for
pre-acceptance.

---

## Evidence limits

This CI establishes behavior on cited Linux GitHub-hosted runners.

Windows BCrypt and Apple/BSD D8D opaque-ID paths are present but are not
independently executed by these runs.

The evidence does not establish:

- physical/world truth of compatibility;
- producer cryptographic authentication;
- D8C source retention across restart;
- transition eligibility;
- current-live freshness after interpretation;
- authority/capability/commit.
