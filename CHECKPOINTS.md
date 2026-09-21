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
