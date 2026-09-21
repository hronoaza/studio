# CI Validation — HRONOAZA Audio Studio 1.1

## Evidence identity

- Repository: `hronoaza/studio`
- Branch: `migration/audio-studio-1.1`
- Validated branch head: `d9f1f38f54ea9cd0a8216f805daa9d415edc7b8d`
- GitHub Actions workflow: `Audio Studio Validation`
- Workflow run: `#8`
- Run ID: `35627033315`
- Job ID: `106423804380`
- Run conclusion: `success`
- Browser user agent: `HeadlessChrome/151.0.7922.34`
- Artifact: `audio-studio-validation`
- Artifact ID: `10651948718`
- Artifact digest: `sha256:2240a52c348e38c53923297f0f6e6f3a7c7f2e9b7186ad2b1ef124e870f950d6`

## Executed checks

| Check | Result |
|---|---|
| Node.js lifecycle suite | PASS, 13/13 |
| Chromium installation in clean GitHub-hosted runner | PASS |
| Browser / OfflineAudioContext DSP and async suite | PASS, 26/26 |
| Headless UI checks | PASS |
| Validation artifact upload | PASS |

The browser suite executed in a clean GitHub-hosted Ubuntu runner with Node.js 22 and Playwright Chromium.

## Key measured results

The successful browser suite included:

- shared production graph checks for M1, M2 and M3 at 44.1 kHz and 48 kHz;
- analytical AM waveform comparison;
- phase-dependent DC checks;
- fade-in and fade-out automation checks;
- mode-frequency ramp phase integration;
- primitive GainNode and OscillatorNode automation checks;
- compressor transient measurements;
- peak-meter threshold checks;
- selected async lifecycle regressions.

Representative measured values from run #8:

- M1 44.1 kHz analytical AM maximum error: `0.00008055627663373921`
- M2 44.1 kHz analytical AM maximum error: `0.00008682223109264531`
- M3 44.1 kHz analytical AM maximum error: `0.00008658369495768634`
- 44.1 kHz fade maximum error: `0.000013517214614677808`
- 44.1 kHz mode-ramp maximum error: `0.000145134492236354`
- 44.1 kHz actual OfflineAudioContext intervention time: `2.0027210884353743 s`

The 44.1 kHz intervention time reflects OfflineAudioContext render-quantum alignment. The test oracle was adjusted to compare against the actual audio-clock intervention time rather than an assumed exact 2.000 s boundary.

## Implementation note

During validation, the stop fade was made explicit and deterministic by anchoring the current envelope value with `cancelScheduledValues()` + `setValueAtTime()` before the existing 0.5 s linear ramp to zero.

This did not change the declared carrier frequency, modulation frequencies, base gain, modulation depth, compressor settings, start-ramp duration, stop-ramp duration, or mode-ramp duration.

## Scope and limitations

This evidence establishes that the tested implementation passed the repository's Node lifecycle checks and the Chromium-based OfflineAudioContext / headless validation suite for the exact commit above.

It does not by itself establish:

- hearing safety;
- physical SPL;
- true-peak or intersample-peak certification;
- behavior in Firefox or Safari;
- behavior on all mobile devices;
- scientific or therapeutic efficacy;
- final acceptance into the Current Baseline.

Final baseline acceptance remains a separate explicit operator decision.


## Locked dependency revalidation

A later CI run revalidated the migration after committing the verified npm lockfile and switching dependency installation to `npm ci`.

- Validated branch head: `b5d3148cb5d7a32508db87dc56ee3b854f34dbf0`
- GitHub Actions run: `#14`
- Run ID: `35627944407`
- Job ID: `106426821177`
- Conclusion: `success`
- Dependency installation: `npm ci --ignore-scripts --no-audit --no-fund`
- Browser / OfflineAudioContext validation: PASS
- Lifecycle suite: PASS
- Artifact ID: `10652994157`
- Artifact digest: `sha256:75405ec542ea2f816f1344df279926302e0ed5318e27e10434430ca12cee893d`

This run confirms that the committed dependency lockfile is usable and that the validated browser/DSP and lifecycle behavior still passes with the frozen npm dependency graph.
