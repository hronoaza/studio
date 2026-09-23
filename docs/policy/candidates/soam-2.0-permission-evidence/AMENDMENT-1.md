# Amendment-1 to SOAM 2.0 Permission Evidence Design

## 0. Amendment identity

- amendment_id:                AMENDMENT-1
- base:                        design baseline v0
- base_artifact:               DESIGN_CONTRACT.md
- base_location:               docs/policy/candidates/soam-2.0-permission-evidence/
- base_origin_baseline:        aa3a88267cdcb3aa72a462059e0dd23990bd6b4d
- base_artifact_blob:          a518293224a7a2afc13216ebb481819923513a38
- reason:                      successor state after D1/D3 cryptographic
                               verification slice implementation and
                               acceptance
- affected_contract_sections:  §§1–71 (see §3 classification)
- resulting_disposition:       toward v1 ratification (see §6)

## 1. Purpose

This amendment records the successor state of the permission-evidence
design after the initial D1/D3 cryptographic verification slice was
implemented and accepted.

It does not rewrite the v0 baseline.

It does not constitute v1 ratification.

It establishes the disposition that a v1 ratification event would apply,
including the treatment of the embedded D→C1 conversion boundary.

## 2. What changed since baseline v0

Since baseline v0 acceptance, the following state changes have occurred:

- D1/D3 cryptographic verification slice implemented and accepted.
  Merge: 41627df13777aefd625e85003ca45ff6f2cc50e9
  (runtime PR #33 merge into main).

- Full ProductionPermissionVerifier / evaluator: NOT implemented.

- D2 production issuer ceremony / provisioning: NOT performed.

- D→C1 adapter: NOT implemented.

- Production activation: absent.

No baseline v0 normative section is amended by this document.

## 3. Classification of baseline sections

This amendment does not rewrite any baseline section.

The classification of baseline §§1–71 is recorded here only to make
explicit which sections are normative and which are chronology, so
that future readers do not mistake chronology for unratified norm.

  §§56–66  normative — preserved unchanged
  §§67–68  gate / review chronology — preserved unchanged
  §§69–70  normative governance semantics — preserved unchanged
  §71      freeze / implementation chronology — preserved unchanged

No baseline section is superseded, redefined, or silently edited by this
amendment.

## 4. Successor executable status

The baseline v0 top status block recorded:

  Executable implementation: none

That statement was correct at baseline v0 acceptance time.

As of this amendment, the executable status is:

  D1/D3 cryptographic verification slice:
    implemented and accepted.

  Complete ProductionPermissionVerifier / evaluator:
    not implemented.

  D2 production issuer ceremony / provisioning:
    not performed.

  D→C1 adapter:
    not implemented.

  Production activation:
    absent.

This successor status does not retroactively rewrite the baseline v0
top status. The baseline v0 status remains as historical record of the
state at baseline acceptance.

## 5. Vertical-slice / split disposition for D→C1

For V1, D remains a vertical-slice design contract.

The D→C1 conversion defined in baseline §57 remains inside the D
contract and is not split into a separate design contract for V1.

However, the D→C1 boundary is separately auditable.

Its implementation must independently demonstrate:

  - total, non-decision-bearing conversion;
  - adapter-specific narrow production construction access;
  - friend authority limited to PermissionPrerequisiteEvidence;
  - no reuse of ProductionTransitionConstructionAccess in production;
  - no eligibility, authority, or execution semantics;
  - consistency with the accepted B1/C per-adapter narrow seam
    convention.

This is a structural compliance requirement.

It is not semantic inheritance from B1 or C.

A future split of the D→C1 boundary into a standalone
permission-C1 adapter contract would require an explicit amendment.
Such a split is not required for V1.

## 6. Ratification disposition toward v1

The disposition established by this amendment is:

  v1 ratification may cover the evaluator design, the trusted record
  design, and the embedded D→C1 conversion design as one ratification
  event, subject to a separate audit gate for the §57 boundary.

Implementation acceptance remains separately staged:

  D1/D3 cryptographic verification slice
    != complete ProductionPermissionVerifier
    != D→C1 adapter implementation
    != D2 provisioning / production activation.

This amendment does not perform v1 ratification.

It records the state that v1 ratification would formalize.

## 7. Non-effects

This amendment does not:

  - modify DESIGN_CONTRACT.md (baseline v0);
  - modify the accepted C1 transition-eligibility contract;
  - modify the accepted transition-pipeline contract;
  - modify the accepted B1 invariant-C1 adapter contract;
  - modify the accepted C resilience design contract;
  - modify the accepted C→C1 adapter contract;
  - constitute v1 ratification of the permission-evidence design;
  - constitute implementation acceptance of any D component;
  - constitute D2 provisioning;
  - constitute production activation.

## 8. Governance provenance

Per baseline §69–§70:

  - governance home: accepted main history;
  - governance event identified by merge commit SHA and its
    first-parent position on protected main;
  - ratification record: reviewed artifact + reviewed amendment
    + accepted merge into protected main.

The governance event created by this amendment is the protected-main
merge of this AMENDMENT-1.md into the accepted main history, not the
mere existence of the file on a feature branch.

The disposition of this amendment after v1 ratification — whether
AMENDMENT-1.md travels with the ratified v1 artifact into accepted/,
or remains in candidates/ as historical amendment provenance — is
decided at v1 ratification time and is not fixed by this document.

## 9. STOP boundary of this amendment

This amendment stops at the disposition recorded in §6.

It does not perform v1 ratification.

It does not alter baseline sections.

It does not alter accepted contracts.

It records successor state.

Amendment-1 ends here.
