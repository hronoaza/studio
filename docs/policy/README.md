# SOAM policy documents

## Directory layout

`candidates/` contains design contracts under review; they are not yet accepted.

`accepted/` contains design contracts accepted into the project baseline.

Acceptance does not by itself imply implementation. Implementation may be accepted separately, before or after design acceptance.

## Acceptance criteria

A design contract is accepted when:

- its scope is explicitly bounded — what it governs and what it does not;
- it is marked with an Acceptance status reflecting actual governance state;
- its location matches its status — accepted documents live under `accepted/`.

## Anti-recursion rule

Acceptance of a design contract does not constitute semantic precedent for other design contracts. Structural patterns may be reused without implying semantic inheritance.

## Not defined in v0

- handling of rejected candidates;
- withdrawal of an accepted design;
- amendment procedure for an accepted design.
