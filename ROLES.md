# HRONOAZA Studio — Roles and Authority

## Purpose

This document defines operational roles.

Roles define responsibility and governance authority.

They do not establish copyright ownership, employment status, or legal ownership.

## Root Operator

The Root Operator holds final operational authority over the Current Baseline.

Authority includes:

- Freeze;
- Stop;
- rollback authorization;
- Quarantine authorization;
- baseline acceptance control;
- integration restriction or revocation;
- governance change approval;
- delegation and revocation.

## Maintainer

A Maintainer may manage normal development within explicitly granted authority.

Typical responsibilities:

- reviewing changes;
- maintaining branches;
- preparing pull requests;
- documentation maintenance;
- coordinating verification;
- handling routine changes within scope.

A Maintainer does not automatically gain Root Operator authority.

## Reviewer

A Reviewer evaluates material without automatically receiving acceptance authority.

Review may cover:

- code;
- provenance;
- security;
- terminology;
- dependencies;
- licensing;
- verification evidence.

Review and acceptance are distinct.

## Verification Operator

A Verification Operator executes or supervises defined verification procedures.

Verification records should identify:

- tested revision;
- environment;
- procedure;
- result;
- limitations.

Verification does not automatically authorize release or acceptance.

## Migration Operator

A Migration Operator prepares external or historical material for controlled entry.

Responsibilities may include:

- source provenance identification;
- hashing;
- source path recording;
- terminology cleanup;
- third-party separation;
- migration diff preparation.

Migration authority does not imply verification or acceptance authority.

## Security Reviewer

A Security Reviewer may recommend:

- Freeze;
- Quarantine;
- key rotation;
- rollback;
- dependency removal;
- access reduction;
- additional verification.

Recommendations do not override Root Operator authority.

## AI Systems

AI systems are tools, not governance authorities.

AI may assist with:

- analysis;
- drafting;
- code generation;
- review;
- comparison;
- classification;
- testing support;
- anomaly identification.

AI may not independently:

- accept material;
- remove a Freeze;
- approve its own output;
- publish restricted material;
- erase provenance evidence;
- grant itself authority.

## Technical Permission Principle

Technical permission and governance authority are independent.

Repository admin rights, infrastructure credentials, deployment access, or API tokens do not automatically assign a governance role.

Governance roles must be assigned explicitly.

## Delegation

Delegation should identify:

- delegating authority;
- holder;
- role;
- scope;
- permitted actions;
- prohibited actions;
- effective date;
- expiration or revocation condition.

Least privilege applies.

## Emergency Succession

No emergency succession exists unless explicitly adopted and recorded.

Any succession policy must identify:

- activation condition;
- temporary holder;
- transferred powers;
- excluded powers;
- duration;
- restoration mechanism.

## Role Separation Principle

Where practical:

`creation → review → verification → acceptance`

The same person may perform multiple roles when necessary, but records should make that explicit.

## Authority Registry

Current assignments belong in `AUTHORITY_REGISTRY.md`.

This document defines roles, not current holders.
