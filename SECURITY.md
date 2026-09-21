# Security Policy

## Scope

This policy applies to the `hronoaza/studio` repository and all source code, configuration, build scripts, documentation, release artifacts, and automation committed to it.

## Reporting a Security Issue

Do **not** disclose suspected vulnerabilities, exposed credentials, private keys, access tokens, personal data, or other sensitive security information in a public issue, pull request, discussion, or commit message.

Report security-sensitive findings privately to the repository owner through an approved private channel.

A useful report should include, when available:

- affected component or file;
- affected version, commit SHA, or release;
- clear reproduction steps;
- expected and observed behavior;
- potential impact;
- relevant logs or screenshots with secrets removed;
- suggested mitigation, if known.

## Secrets and Credentials

Never commit:

- passwords;
- API keys;
- access tokens;
- private keys;
- signing keys;
- recovery codes;
- session cookies;
- production credentials;
- unredacted secret-bearing logs or configuration.

If a secret is committed or otherwise exposed, treat it as compromised. Revoke or rotate it first, then remove it from active repository content and document the remediation without republishing the secret.

## Provenance and Integrity

Security-relevant changes should preserve a clear audit trail.

Where practical:

- identify the source commit for release artifacts;
- record exact commit SHAs when validating behavior;
- avoid rewriting published history used as provenance evidence;
- use reproducible or independently verifiable build steps;
- verify downloaded dependencies and artifacts against trusted sources;
- prefer signed commits, signed tags, release attestations, or checksums when appropriate.

A successful build or test run is evidence about that specific revision only. It is not, by itself, proof that the entire system is secure.

## Dependency and Supply-Chain Risk

Before introducing or updating a dependency, review:

- its source and maintainership;
- requested permissions;
- release history;
- integrity or signature information where available;
- transitive dependencies;
- whether a smaller or built-in alternative can satisfy the requirement.

Avoid adding dependencies that are unnecessary for the documented functionality of the project.

## Repository Changes

Security-sensitive modifications should be:

1. reviewed against the exact diff;
2. tested against the exact commit being proposed;
3. kept minimal and auditable;
4. separated from unrelated refactors where practical.

Changes affecting authentication, authorization, cryptography, persistence, update mechanisms, release generation, network access, or executable downloads require additional scrutiny.

## Third-Party Apps and Integrations

GitHub Apps, CI services, external AI tools, package registries, deployment systems, and other integrations form part of the project trust boundary.

Grant only the permissions and repository access required for the intended task. Read-only access is preferred when write access is unnecessary.

External services must not be treated as the sole source of truth for project provenance. Canonical evidence should remain traceable to repository history, signed metadata, release records, or independently verifiable artifacts.

## Security Review Status

The presence of this policy does not imply that the repository, a commit, a release, or any third-party integration has completed a formal security audit.

Security claims should be tied to specific evidence, scope, revision, and date.

## Responsible Disclosure

Please allow reasonable time to investigate and remediate a valid security report before public disclosure. Coordinated disclosure is preferred when users or downstream systems could be affected.
