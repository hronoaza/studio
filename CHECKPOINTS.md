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

### HRZ-CP-20260923-001 (completed)

**checkpoint_id:** HRZ-CP-20260923-001  
**predecessor:** none — first concrete checkpoint record in this ledger  
**phase:** 2 of 2 — completion

**timestamp_utc:** 2026-09-23T12:56:24Z — GitHub PR #36 created_at  
**timestamp_source:** GitHub phase 2 PR `created_at`, chosen for external immutability and verifiability rather than a local commit timestamp

**repository:** hronoaza/studio  
**branch:** main

**recorded_on_main_sha:** 53610c65caaf8579db376771e9bcc0014a97f3f5 — phase 1 merge commit; protection boundary  
**recorded_on_tree_sha:** fef8354933ed802639debdfad17bd1e61259b522

**creating_operator:** Mykola Bezruchko <hronoaza00@gmail.com>

**checkpoint_status:** verified  
**provenance_status:** documented  
**verification_status:** passed

**sensitivity:** ordinary

**restrictions:** documentation-only; no runtime / source / build / test changes

**unresolved_issues:** none for this checkpoint record

**recovery_suitability:** not applicable — governance-state checkpoint; no runtime state to recover to

---

**baseline_reference_commit:** a8b11dd4f1547de79ce25870cac792a757aa27d5 — last commit before this governance workstream began

**protection_boundary_commit:** 53610c65caaf8579db376771e9bcc0014a97f3f5 — last commit in main recorded under unprotected branch state

**protection_enabled_at_utc:** 2026-09-23T12:51:43.130Z  
**protection_timestamp_source:** GitHub repository ruleset metadata, ruleset id 23879954

**enforcement_mechanism:** repository ruleset, not classic branch protection. The classic branch-protection API surface may report its classic-rule state separately; enforcement for this checkpoint is established by repository ruleset 23879954.

- ruleset id: 23879954
- name: main
- enforcement: active
- target: default branch (main)
- bypass list: empty
- current_user_can_bypass: never
- deletion blocked: yes
- force-push blocked: yes — non-fast-forward updates rejected
- pull request required: yes
- required approvals: 0
- allowed merge methods: merge only
- required status checks: none initially
- review-thread resolution enforced: no

**observed_policy:** require PR before merge; no approvals; no bypass; no force-push; no deletion; merge-only; no required checks initially. Merge-only is intentional to preserve first-parent governance ordering. Review-thread resolution is not enforced by the current protection policy.

**operational_consequence:** with an empty bypass list and `current_user_can_bypass=never`, the operator cannot bypass the protected path, cannot force-push to main, cannot amend protected main history, and cannot merge to main without a pull request. Any correction requires a new commit through a new pull request.

**provenance_note:** phase 1 merge commit 53610c65caaf8579db376771e9bcc0014a97f3f5 is GitHub-signature-verified (`verified=true`, `reason=valid`). This confirms creation through GitHub merge infrastructure; it does not extend trust to the repository state as a whole.

**first_protected_change:** this phase 2 PR

**first_protected_commit:** see the GitHub merge record for this PR; canonical repository metadata; intentionally not self-embedded.
