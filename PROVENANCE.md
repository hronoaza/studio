# HRONOAZA Studio — Provenance Policy

## Purpose

This document defines how material enters, changes within, and is accepted into the HRONOAZA Studio Current Baseline.

Its purpose is to preserve:

- source identity;
- authorship and contribution context;
- AI involvement;
- transformation history;
- verification status;
- sensitivity classification;
- decision history;
- traceability to exact repository revisions.

Provenance is part of the integrity model of the project.

## Core Principle

No material becomes trusted merely because it:

- is old;
- has been repeated many times;
- appears technically plausible;
- was produced by a capable AI model;
- existed in an earlier repository;
- appeared in a previous release;
- was accepted in an earlier conversation;
- was generated or reviewed by an external tool.

Trust must be tied to identifiable evidence, scope, transformation history, and verification.

## Standard Terms

**Current Baseline** — the active accepted project state.

**Accepted Material** — material explicitly accepted into the Current Baseline.

**Trusted Checkpoint** — a preserved project state approved as a recovery target for a defined scope.

**Quarantined Material** — material intentionally isolated from the Current Baseline pending assessment or remediation.

## Material Lifecycle

Typical progression:

`source-identified → imported → cleaned → reviewed → verified → accepted`

This progression is not strictly linear.

Material may also become:

`quarantined`

`rejected`

`revoked`

A previously accepted component may be quarantined or revoked if new evidence changes its trust status.

### source-identified

The origin is known well enough to begin assessment.

### imported

The material has entered a controlled migration workspace.

Import does not imply approval.

### cleaned

Approved migration cleanup has been performed.

Cleaning does not imply correctness.

### reviewed

The material has been examined for relevant provenance, terminology, dependencies, licensing, security, sensitivity, and intended role.

### verified

Relevant claims or behavior have been checked using defined evidence.

### accepted

The material has been explicitly included in the Current Baseline.

### quarantined

The material is isolated from the Current Baseline pending investigation or further review.

### rejected

The material has been assessed and not accepted for the intended scope.

### revoked

Previously accepted trust has been withdrawn because later evidence invalidated or materially changed the earlier decision.

## Provenance Record

For significant material, record where known:

- material identifier;
- current path;
- original source;
- original path or location;
- source repository or archive;
- source commit SHA, file hash, or timestamp;
- original author or contributor;
- creation or first-known date;
- import date;
- migration operator;
- transformation history;
- AI involvement;
- evidence basis;
- human review status;
- verification method;
- verification result;
- sensitivity classification;
- third-party dependencies;
- licensing considerations;
- accepted commit SHA;
- unresolved questions.

Unknown information must be marked as unknown rather than reconstructed without evidence.

## Human Source and Contribution

Human-created source should remain distinguishable from later transformations.

Where practical, record:

- original human author;
- editor or maintainer;
- migration operator;
- acceptance authority;
- whether the material predates this repository;
- whether legal ownership has been independently established.

Commit authorship alone is not complete evidence of intellectual authorship or legal ownership.

## AI Provenance

AI involvement should be recorded when a model materially influenced:

- source code;
- architecture;
- algorithms;
- documentation;
- technical conclusions;
- security analysis;
- terminology;
- data transformation;
- research synthesis;
- decision rationale.

Where known, record:

- provider;
- model name;
- model version or variant;
- interaction date;
- role of the model;
- source material supplied;
- transformation type;
- whether the model generated, edited, analyzed, summarized, translated, or reviewed;
- human review status;
- independent verification status;
- evidence basis.

Useful evidence-basis labels include:

`model-assertion`

`model-assisted-inference`

`human-judgment`

`test-derived-result`

`externally-verified-fact`

These labels describe the basis of a claim, not its importance.

AI output does not by itself establish correctness, originality, safety, ownership, or reliability.

## Model Separation

When different AI systems contribute to one artifact, their contributions should remain distinguishable where provenance matters.

Example:

`human source → model A analysis → model B implementation → human correction → model C review → test verification`

The final artifact may be accepted while preserving this development path.

## Transformation History

Migration and cleanup must not erase meaningful history.

Transformations may include:

- exact copy;
- format conversion;
- terminology replacement;
- structural refactor;
- dependency removal;
- security hardening;
- bug fix;
- code rewrite;
- documentation rewrite;
- AI-assisted transformation;
- manual reconstruction.

For sensitive or high-value material, preserve before/after hashes or exact references where practical.

## Terminology Integrity

Historical terminology and current HRONOAZA terminology must remain distinguishable.

Legacy terminology may remain in provenance records when required for historical identification while being prohibited from the Current Baseline.

Renaming does not change provenance.

## Sensitivity Classification

Baseline categories:

`ordinary`

`sensitive`

`dual-use-review`

`restricted`

Sensitivity may be increased immediately when credible risk is identified.

Sensitivity may be reduced only after explicit review.

Classification must be based on concrete content, handling risk, and foreseeable use rather than novelty or perceived importance alone.

## Claims and Evidence

Important claims must remain distinguishable from:

- hypotheses;
- interpretations;
- model-generated suggestions;
- experiments;
- verified observations;
- external claims;
- Accepted Material facts.

A successful test verifies only the tested scope, configuration, revision, environment, and measured behavior.

## Third-Party Material

Third-party material must retain its own provenance.

Before acceptance, identify where practical:

- source;
- upstream project;
- license;
- version;
- copyright notice;
- modifications;
- dependency chain;
- redistribution obligations.

Third-party material must not be represented as original HRONOAZA work.

## Historical Material

Historical repositories, archives, conversations, documents, experiments, and prototypes may be evidence.

However:

- historical presence does not imply current acceptance;
- historical terminology does not define current terminology automatically;
- historical code does not automatically enter the Current Baseline;
- historical AI output is not automatically verified;
- historical claims retain only the confidence supported by their evidence.

## Acceptance Rule

Material enters the Current Baseline only when:

1. source is sufficiently identified;
2. relevant transformations are documented;
3. material AI involvement is recorded;
4. terminology has been reviewed;
5. third-party rights and dependencies have been considered;
6. security and sensitivity have been assessed;
7. applicable verification has been completed;
8. unresolved limitations are documented;
9. acceptance is tied to an exact repository state.

## Decision Trail

High-impact provenance decisions should be append-only in procedure.

Corrections should add a new record or explicit amendment rather than silently replacing historical evidence.

## Evidence Preservation

Evidence may use:

- commit SHAs;
- hashes;
- signed tags;
- release records;
- archives;
- dated source files;
- test artifacts;
- decision records;
- migration records;
- trusted timestamps;
- signed statements.

No single external AI service, hosting provider, memory system, or development tool should be the sole holder of critical provenance evidence.
