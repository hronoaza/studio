# HRONOAZA Studio — Checkpoint Policy

## Purpose

A checkpoint is a preserved project state usable as:

- verification reference;
- comparison point;
- recovery target;
- rollback target;
- evidence anchor.

Checkpoints make recovery deterministic rather than reconstructive.

## Checkpoint Identity

Recommended identifier:

`HRZ-CP-YYYYMMDD-NNN`

The authoritative technical identity remains the exact repository revision and associated integrity evidence.

## Required Fields

Where applicable:

- checkpoint ID;
- timestamp;
- repository;
- branch;
- exact commit SHA;
- creating operator;
- checkpoint status;
- provenance status;
- verification status;
- sensitivity classification;
- restrictions;
- unresolved issues;
- recovery suitability.

## Integrity Fields

For important checkpoints, record where practical:

- Git commit SHA;
- Git tree SHA;
- hashes of critical artifacts;
- lockfile hashes;
- release artifact hashes;
- signed tag reference;
- attestation reference;
- independent archive reference.

## Checkpoint Status

### candidate

Preserved but not fully reviewed.

### reviewed

Metadata and scope inspected.

### verified

Defined verification completed successfully for recorded scope.

### trusted

Explicitly approved as a recovery target.

### superseded

Still historically valid and recoverable, but no longer the preferred recovery point.

### revoked

Must no longer be used as a trusted recovery target.

Revocation must preserve the record and reason.

## Verification Status

Suggested values:

`not-run`

`partial`

`passed`

`failed`

`passed-with-limitations`

## Provenance Status

Suggested values:

`unknown`

`partial`

`documented`

`reviewed`

`accepted`

## Sensitivity

Use:

`ordinary`

`sensitive`

`dual-use-review`

`restricted`

Sensitivity may increase immediately.

Sensitivity reduction requires review.

## Recovery Suitability

Record whether suitable for:

- source rollback;
- development rollback;
- release rollback;
- deployment rollback;
- configuration rollback;
- forensic comparison.

## Checkpoint Creation

Create checkpoints where practical before:

- major migration;
- destructive refactor;
- dependency transition;
- security-sensitive change;
- release preparation;
- integration change;
- large AI-assisted transformation;
- import of sensitive material;
- governance or provenance changes.

## Rollback Depth

No numerical rollback limit is imposed.

Any preserved Trusted Checkpoint may be used if its integrity and recovery requirements remain satisfiable.

## Rollback Record

Record:

- rollback timestamp;
- authorizing operator;
- source state;
- target checkpoint;
- affected scope;
- reason category;
- procedure;
- post-rollback verification.

Sensitive details should be minimized.

## Freeze Relationship

Freeze stops progression.

Checkpoint preserves recoverability.

Preferred sequence:

`Freeze → preserve evidence → identify Trusted Checkpoint → rollback or remediate → verify → resume`

## Quarantine Relationship

Rollback does not require deletion.

Later material may remain:

- in history;
- in Quarantine;
- on an isolated branch;
- in forensic storage;
- as evidence.

## Independent Preservation

Critical checkpoints should not depend exclusively on one repository.

Where justified, use:

- signed tags;
- offline Git bundles;
- cryptographic manifests;
- encrypted backups;
- release archives;
- trusted timestamps;
- independently controlled storage.

## Recovery Test

Important checkpoints should periodically be tested for actual recoverability.

A checkpoint that cannot practically be restored should not be treated as a strong recovery point merely because its commit exists.

---

## Recorded checkpoints

### HRZ-CP-20260923-001

**checkpoint_id:** HRZ-CP-20260923-001  
**predecessor:** none — first concrete checkpoint record in this ledger

**timestamp_utc:** 2026-09-23T11:37:23Z — GitHub PR #35 created_at  
**timestamp_source:** PR `created_at` from GitHub metadata, chosen for external immutability and verifiability rather than a local commit timestamp

**repository:** hronoaza/studio  
**branch:** main

**recorded_on_main_sha:** a8b11dd4f1547de79ce25870cac792a757aa27d5  
**recorded_on_tree_sha:** 09aa3e9f63eb3003ab0e74734e3d5d99872dc8ef

**creating_operator:** Mykola Bezruchko <hronoaza00@gmail.com>

**checkpoint_status:** candidate  
**provenance_status:** documented  
**verification_status:** passed-with-limitations  
**verification_limitations:** branch protection is not yet enabled; `protection_boundary_commit` and `first_protected_commit` are not yet established

**sensitivity:** ordinary

**restrictions:** documentation-only checkpoint record; no runtime / source / build / test changes

**unresolved_issues:**
- protection_boundary_commit not yet established;
- first_protected_commit not yet established;
- branch protection is not yet enabled.

**recovery_suitability:**
- source rollback: suitable for repository-content comparison/rollback if otherwise authorized;
- development rollback: suitable as a source-state reference;
- release rollback: not established;
- deployment rollback: not applicable;
- configuration rollback: not sufficient for GitHub branch-protection state;
- forensic comparison: suitable.

---

**baseline_reference_commit:** a8b11dd4f1547de79ce25870cac792a757aa27d5  
Last commit before this governance workstream began.

**pending_governance_action:** enable branch protection on main with:

- require pull request before merge: true;
- required approving reviews: 0;
- enforce administrators: true;
- allow force pushes: false;
- allow deletions: false;
- required status checks: none initially.

**governance_semantics:** protection will apply prospectively, not retroactively.

Once protection is enabled, main history cannot be rewritten through force-push, administrator bypass is disabled for the protected path, and any repository-content correction to main requires a new commit through a new pull request. This is intentional enforcement, not a side effect.

**protection_boundary_commit:** pending — will be the merge SHA of this PR and the last commit recorded under unprotected main.

**first_protected_commit:** pending — phase 2 will identify the completing checkpoint PR as the first protected change; the exact merge SHA will remain canonical in the GitHub merge record rather than being self-embedded.

**completion:** pending — after branch protection is enabled, a phase 2 checkpoint PR will complete this record by replacing `protection_boundary_commit` with the exact phase 1 merge SHA and replacing the phase-1 `first_protected_commit` placeholder with an explicit pointer to the phase 2 GitHub merge record.

**implementation_note:** with administrator enforcement enabled, the operator cannot force-push to main, cannot bypass the protected merge path as administrator, and cannot amend protected main history. Corrections must proceed through a new PR.

**single_operator_policy_rationale:** `required approving reviews = 0` together with administrator enforcement is intentional for the current single-operator repository. Requiring one approval while enforcing the rule on administrators would make the merge path non-functional because self-approval does not satisfy an independent-review requirement; allowing administrator bypass would weaken the boundary. The chosen configuration preserves a real PR-only boundary without inventing a reviewer who does not exist.

**provenance_note:** the signature-verification state of the phase 1 merge commit will be observed after merge and recorded as evidence in phase 2. No pre-merge claim is made that the future merge commit is already verified.
