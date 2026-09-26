# D8B Baseline Alignment Review

## Status

- PR: #12
- D8B branch: `candidate/soam-2.0-d8b-provenance-envelope-design`
- Current Baseline: `379cd2e2cf17eb7181629945e670f7923ba662ce`
- Baseline source: accepted D8A Source Capture, merged from PR #8
- D8B executable changes: none
- Review disposition: **BASELINE ALIGNED / ACCEPTANCE REVIEW READY**

## 1. Divergence

At review time PR #12 is ahead of its original merge base by five
documentation-only commits and behind current `main` by one commit.

That one behind commit is the accepted D8A squash merge:

`379cd2e2cf17eb7181629945e670f7923ba662ce`

PR #12 itself changes only four D8B documentation files. It contains no runtime,
public header, test, CMake or workflow implementation.

Therefore the current divergence is an upstream-baseline relationship, not an
executable conflict.

## 2. Upstream assumptions now satisfied

The original D8B design required D8A to provide a restricted-origin capture-event
identity.

Current Baseline now provides:

- `ProductionRelationshipLocator`;
- `ProductionRelationshipSourceSnapshot`;
- `SourceCaptureId`;
- coherent relationship generation/state-version capture;
- raw bridge/endpoint facts;
- restricted-origin snapshot construction.

The accepted D8A capture ID is:

- 128-bit / 16 opaque bytes;
- OS-backed random;
- all-zero invalid;
- distinct for successful recaptures;
- preserved by copy/move;
- not authority/provenance/digest semantics.

This matches the D8B SourceBinding assumption.

## 3. D8B boundary after alignment

```text
accepted D8A SourceSnapshot + SourceCaptureId
-> D8B ProvenanceEnvelope
-> STOP
```

D8B still does not perform:

- provenance admissibility;
- semantic interpretation;
- transition eligibility;
- authority/capability/commit.

## 4. Resolved design-review findings

The first D8B design review identified four blocking issues.

They are now resolved at design level:

- capture identity ownership -> accepted D8A;
- integrity naming -> `CanonicalDigest`;
- source binding/retrieval conflation -> `SourceBinding` vs
  `SourceRecordReference`;
- ambiguous canonical preimage -> `CanonicalEnvelopeV1` with fixed vectors.

No new blocker was introduced by the D8A merge.

## 5. Remaining pre-implementation decisions

The remaining D8B-specific decisions are:

1. choose `ProvenanceItemId` generation and uniqueness domain;
2. fix production schema ID/version;
3. fix producer ID/version/revision semantics;
4. define the production dependency-kind registry;
5. accept the canonical wire format and normative vectors;
6. design restricted-origin C++ envelope construction and internal seams;
7. define compile-fail misuse gates.

These are D8B decisions, not unresolved D8A work.

## 6. Disposition

PR #12 is aligned with Current Baseline `379cd2e2cf17eb7181629945e670f7923ba662ce`.

It is ready for a D8B design acceptance review.

This document authorizes no implementation and no merge by itself.
