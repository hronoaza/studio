# Third-Party Dependency Record — HRONOAZA Audio Studio 1.1

## Declared dependency

The migrated application declares one development dependency:

- package: `playwright`
- version: `1.62.1`
- scope: `devDependency`
- runtime purpose: headless browser validation only
- production browser application does not import Playwright

## License status

Playwright is distributed by Microsoft under the MIT License.

This record documents the dependency used for repository validation. It does not change or select the license for HRONOAZA Studio itself.

## Reproducibility status

The package version is pinned exactly to `1.62.1`.

At the time of this record, `apps/audio-studio/package-lock.json` is not committed. GitHub Actions therefore installs the exact direct Playwright version through `npm install`, but the npm dependency tree is not additionally frozen by a repository lockfile.

This is a reproducibility limitation, not a browser/DSP validation failure. The successful CI evidence remains tied to the exact validated commit and GitHub Actions run recorded in `CI_VALIDATION.md`.

## Acceptance implication

Before a production-quality dependency baseline is considered fully frozen, a verified lockfile should be generated and reviewed. No automatic project-license choice is made by this record.
