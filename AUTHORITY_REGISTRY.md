# HRONOAZA Studio — Authority Registry

## Purpose

This registry records current operational authority assignments.

Role definitions remain in `ROLES.md` and `OPERATOR_AUTHORITY.md`.

## Registry Revision

**Registry version:** v0.1

Every change to active authority assignments should create a new repository revision.

Historical authority state should remain recoverable through Git history.

## Root Operator

**Status:** Active

**Authority scope:** Project-wide

**Current holder:** Controlled identifier pending explicit repository entry

Root Operator authority includes:

- Stop;
- Freeze;
- rollback authorization;
- Quarantine authorization;
- baseline acceptance control;
- integration restriction or revocation;
- governance approval;
- delegation and revocation.

## Delegation Record

Use:

**Delegation ID:** `HRZ-AUTH-YYYYMMDD-NNN`

**Role:**  
Defined role

**Holder:**  
Controlled identifier

**Scope:**  
Bounded operational scope

**Permitted actions:**  
Explicit list

**Prohibited actions:**  
Explicit list

**Effective from:**  
Timestamp

**Expires:**  
Timestamp, condition, or `until revoked`

**Delegated by:**  
Authorized delegating authority

**Status:**  
active | suspended | expired | revoked

**Evidence reference:**  
Commit SHA, signed record, decision record, or other controlled reference

## Authority Constraints

Delegated authority follows least privilege.

Delegation does not automatically include:

- Root Operator authority;
- project-wide Freeze removal;
- unrestricted rollback;
- unrestricted publication;
- unrestricted access to restricted material;
- further delegation.

## Technical Permission vs Authority

Technical access does not equal governance authority.

GitHub admin, push, API, infrastructure, cloud, CI/CD, deployment, or storage access must not be interpreted as a governance role unless explicitly assigned here or by an authorized delegation record.

## Temporary Authority

Temporary authority may support:

- incident response;
- migration;
- verification;
- release preparation;
- recovery testing;
- security review.

It should expire automatically where practical.

## Suspension

Suspension preserves historical assignment but disables current operational authority.

## Revocation

Revocation ends delegated authority while preserving the historical record.

Record:

- delegation ID;
- timestamp;
- revoking authority;
- scope;
- reason category where appropriate.

## Emergency Succession

No emergency succession exists unless explicitly recorded.

If adopted, record:

- trigger;
- temporary holder;
- transferred powers;
- excluded powers;
- activation time;
- expiry or restoration condition.

## AI and Automation

AI, bots, agents, CI services, and automation are not governance-role holders by default.

Technical write access does not make an automated system a Maintainer, Reviewer, acceptance authority, or Root Operator.

## Registry Integrity

Registry changes should be:

- explicit;
- attributable;
- timestamped through repository history;
- minimal;
- reviewable.

Historical authority states should not be silently rewritten.

## Current Registry State

Root Operator: assigned privately / controlled identifier pending explicit repository entry

Active delegations: none recorded

Suspended delegations: none recorded

Revoked delegations: none recorded

Emergency succession: not configured

## Core Principle

For any high-impact action, it should be possible to determine:

1. who held authority;
2. what authority existed;
3. scope;
4. validity period;
5. delegating authority;
6. whether the authority was valid when the action occurred.
