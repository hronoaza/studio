# HRONOAZA Studio — Root Operator Authority

## Purpose

This document defines emergency and governance authority for HRONOAZA Studio.

Its purpose is to preserve the ability to stop, isolate, reverse, or recover project state when risk or uncertainty requires it.

## Root Operator

The Root Operator is the designated human authority for final operational decisions concerning the Current Baseline.

The Root Operator may:

- stop progression;
- issue a Freeze;
- suspend release, deployment, publication, or migration;
- restrict or revoke integrations;
- place material into Quarantine;
- reject proposed baseline changes;
- restore a Trusted Checkpoint;
- require renewed verification;
- approve governance changes;
- delegate limited authority.

AI systems, automation, CI/CD services, external tools, and contributors do not override Root Operator authority.

## Technical Permission vs Governance Authority

Technical access and governance authority are separate.

GitHub admin, push, merge, deployment, or infrastructure permissions do not automatically grant Root Operator authority.

Likewise, governance authority does not automatically provide technical credentials or access to a system.

Both must be established independently.

## Stop Authority

The Root Operator may immediately halt:

- commits;
- merges;
- releases;
- deployments;
- automated workflows;
- external integrations;
- model-assisted processing;
- exports;
- publication;
- dependency updates;
- migration activity.

Emergency stopping does not require completion of ordinary review first.

## Freeze

A Freeze prevents progression while preserving evidence.

During a Freeze:

- no new baseline acceptance should occur;
- releases and publication should remain suspended;
- sensitive migration activity should stop;
- relevant external write integrations should be reviewed;
- the current state should be identified exactly.

A Freeze is removed only by authorized decision.

## Checkpoints

A checkpoint is a preserved project state usable for verification, comparison, or recovery.

Important checkpoints should identify:

- exact commit SHA;
- verification status;
- provenance status;
- dependency state;
- relevant hashes;
- security review status;
- sensitivity restrictions;
- unresolved issues.

## Trusted Checkpoint

A Trusted Checkpoint is trusted only for the scope actually verified.

Trust may later be revoked.

## Rollback Authority

The Root Operator may restore operation to any preserved Trusted Checkpoint.

Rollback depth has no fixed numerical limit.

A rollback may cross one, ten, one hundred, or more later transitions if the target checkpoint and required recovery evidence remain available.

## Evidence-Preserving Rollback

Operational state may move backward.

Evidentiary history should remain preserved and append-only where practical.

Rollback must not be treated as permission to erase relevant provenance, security, authority, or incident evidence.

## Rollback Types

### Soft Rollback

Move the Current Baseline to an earlier trusted state while preserving later history.

### Hard Operational Rollback

Restore affected services, configurations, releases, or environments to an earlier known state.

### Containment Rollback

Disable or isolate affected functionality while investigation continues.

## Quarantine

Material may enter Quarantine when:

- provenance is uncertain;
- licensing is unresolved;
- AI contribution history is unclear;
- verification failed;
- unsafe behavior is suspected;
- sensitive information entered an inappropriate workflow;
- dual-use review is required.

Quarantined Material is outside the active Current Baseline.

Quarantine does not imply deletion.

## Restricted Material Handling

Sensitive or restricted material should not be copied unnecessarily into:

- public issues;
- public pull requests;
- public CI logs;
- external AI services;
- third-party memory systems;
- analytics platforms;
- unrestricted storage;
- broadly distributed chat transcripts.

Hashes, internal identifiers, controlled references, and redacted metadata are preferred where sufficient.

## Integration Control

The Root Operator may suspend, revoke, or reduce permissions for:

- GitHub Apps;
- CI/CD systems;
- AI assistants;
- MCP servers;
- registries;
- deployment systems;
- cloud platforms;
- external storage;
- monitoring services.

Least privilege is preferred.

## AI Override Principle

AI may propose, analyze, compare, verify within defined scope, and recommend containment or rollback.

AI may not independently:

- approve its own output into the Current Baseline;
- remove a Freeze;
- publish restricted material;
- erase provenance evidence;
- override Root Operator decisions;
- silently redefine governance authority.

## Recovery Verification

Rollback is incomplete until the recovered state is verified.

Verification may include:

- exact target revision confirmation;
- file integrity checks;
- dependency verification;
- configuration verification;
- security-sensitive diff review;
- tests;
- secret exposure review;
- integration review.

## Governance Change Rule

Governance changes apply prospectively unless an explicit corrective amendment states otherwise.

A governance change must not silently rewrite the authority, provenance, or evidence state that existed at the time of a historical event.

## Secrets and Credential Exposure

If rollback follows credential exposure:

1. revoke or rotate the exposed secret;
2. stop affected workflows as necessary;
3. isolate the exposure source;
4. restore an appropriate state;
5. verify that active systems no longer depend on the exposed credential;
6. document the event without republishing the secret.

Rollback alone does not make an exposed credential safe again.

## Escalation Rule

Preferred order:

`detect → stop → freeze → preserve evidence → quarantine → assess → select checkpoint → rollback or remediate → verify → resume`

Emergency containment may shorten this sequence.

## Resume Authority

Project-wide progression after emergency Freeze or rollback requires explicit authorized resumption.

## Delegation

Delegation should define:

- scope;
- duration;
- permitted actions;
- prohibited actions;
- affected systems.

Delegated authority does not automatically include further delegation.

## Non-Destructive Governance Principle

Prefer:

- revert over destructive history rewrite;
- quarantine over deletion;
- evidence preservation over concealment;
- explicit correction over silent replacement;
- checkpoint restoration over undocumented reconstruction.
