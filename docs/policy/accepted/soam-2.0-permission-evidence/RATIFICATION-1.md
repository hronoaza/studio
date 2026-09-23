# RATIFICATION-1 — SOAM 2.0 Permission Evidence, v1

## Identity

ratifies:  soam-2.0-permission-evidence
record:    RATIFICATION-1
status:    append-only; immutable once merged into protected main

This document is the reviewed human-readable ratification record for
the v1 design set. It does not claim to instantiate or implement a
runtime `ProductionPermissionPolicyRatificationRecord` type; no such
type schema is asserted here.

## Design invariant

This record is non-self-referential. It does not name its own blob
SHA, merge SHA, PR number, or review provenance. Such facts live in
protected-main git history and PR metadata, which together constitute
the canonical governance event.

Division of roles:

  RATIFICATION-1.md           — proposition: what is being ratified
  protected-main merge commit — event: by whom, and with what authority

The `status` line above is itself a proposition about intent; it is
realized by the protected-main ruleset and the resulting history, not
enforced by this artifact.

## Ratified set

ratified_set := DESIGN_CONTRACT.md + AMENDMENT-1.md
relationship: baseline + successor disposition

## predecessor_artifact_schema

record_schema: 1

Each predecessor entry uses:
  role
  source_path
  source_sha
  blob_sha
  ratified_path
  transfer

The tuple (source_path, source_sha) must resolve to blob_sha.
For byte-for-byte transfer, the ratified path at the ratifying
governance state must resolve to the same blob_sha.

## predecessor_artifacts

- role:          frozen v0 baseline
  source_path:   docs/policy/candidates/soam-2.0-permission-evidence/DESIGN_CONTRACT.md
  source_sha:    aa3a88267cdcb3aa72a462059e0dd23990bd6b4d
  blob_sha:      a518293224a7a2afc13216ebb481819923513a38
  ratified_path: docs/policy/accepted/soam-2.0-permission-evidence/DESIGN_CONTRACT.md
  transfer:      byte-for-byte

- role:          successor disposition
  source_path:   docs/policy/candidates/soam-2.0-permission-evidence/AMENDMENT-1.md
  source_sha:    479f75f86d5403417c1d17190429a5b302fe3629
  blob_sha:      7a6194dc9cfde588b206d9be437ead2704f19dd8
  ratified_path: docs/policy/accepted/soam-2.0-permission-evidence/AMENDMENT-1.md
  transfer:      byte-for-byte

## ratification_method

explicit pull-request merge commit into protected main
merge method: merge
squash: not permitted by current ruleset
rebase / fast-forward acceptance: not the ratification method

The provenance guarantee derives from the protected-main ruleset
(no force-push, no history rewrite, merge-commit-only) together with
the resulting history. It is not a property of this artifact.

## §57_audit_basis

Ratification requires a separate audit of the §57 D→C1 boundary.

The normative audit basis is:
  - DESIGN_CONTRACT.md §57   (mapping / STOP semantics)
  - AMENDMENT-1.md §5        (hybrid / separately-auditable boundary;
                              narrow-seam requirements)
  - AMENDMENT-1.md §6        (separate audit gate required at v1
                              ratification)

Satisfaction of this gate is established by review provenance outside
this non-self-referential artifact and becomes effective only with
the ratifying merge into protected main.

## implementation_state_reference

artifact: AMENDMENT-1.md
blob:     7a6194dc9cfde588b206d9be437ead2704f19dd8
section:  §4

Ratification-time provenance reference, not a live runtime-status
registry. The pointer is resolvable at ratification time and remains
a valid historical fact thereafter, even if AMENDMENT-1.md is
superseded by a future amendment.

## excluded_from_ratification

- item:  new evaluator implementation acceptance
  basis:
    - AMENDMENT-1.md §4
    - AMENDMENT-1.md §6
    - AMENDMENT-1.md §7

- item:  D→C1 adapter implementation acceptance
  basis:
    - DESIGN_CONTRACT.md §57
    - AMENDMENT-1.md §4
    - AMENDMENT-1.md §6
    - AMENDMENT-1.md §7

- item:  D2 production issuer ceremony / provisioning
  basis:
    - DESIGN_CONTRACT.md §65
    - DESIGN_CONTRACT.md §67
    - AMENDMENT-1.md §4
    - AMENDMENT-1.md §7

- item:  production activation
  basis:
    - DESIGN_CONTRACT.md §71
    - AMENDMENT-1.md §4
    - AMENDMENT-1.md §7

## supersedes

null

## superseded_by

null
