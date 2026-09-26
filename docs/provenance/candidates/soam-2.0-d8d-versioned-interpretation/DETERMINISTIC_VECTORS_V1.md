# SOAM 2.0 D8D — Deterministic Vectors v1

## Status

- Policy: `SOAM.D8D.INTERPRETATION.COUPLING_HEALTH.V1`
- Version: 1.0
- Runtime implementation: none
- Vector role: normative semantic fixtures

Equations:

```text
attenuation = 1 / (1 + 0.1 * distance)

compatibility =
    capacity * attenuation * orientationWeight

confidence =
    min(sourceHealth, targetHealth)

downstreamEvidence =
    ((2 * compatibility) - 1) * confidence
```

`downstreamEvidence` is included only as a cross-check against the already
accepted AdaptiveBridgePolicy. It is not a D8D output.

---

## V1 — full coupling / full health

Input:

```text
distance = 0
orientationWeight = 1
capacity = 1
sourceHealth = 1
targetHealth = 1
```

Expected:

```text
attenuation = 1
compatibility = 1
confidence = 1
downstreamEvidence = +1
```

IEEE-754 binary64:

```text
compatibility = 3ff0000000000000
confidence    = 3ff0000000000000
```

---

## V2 — zero orientation

Input:

```text
distance = 5
orientationWeight = 0
capacity = 1
sourceHealth = 1
targetHealth = 1
```

Expected:

```text
attenuation = 2/3
compatibility = 0
confidence = 1
downstreamEvidence = -1
```

IEEE-754:

```text
compatibility = 0000000000000000
confidence    = 3ff0000000000000
```

---

## V3 — exact neutral coupling

Input:

```text
distance = 10
orientationWeight = 1
capacity = 1
sourceHealth = 1
targetHealth = 1
```

Expected:

```text
attenuation = 0.5
compatibility = 0.5
confidence = 1
downstreamEvidence = 0
```

IEEE-754:

```text
compatibility = 3fe0000000000000
confidence    = 3ff0000000000000
```

---

## V4 — zero-confidence suppression

Input:

```text
distance = 0
orientationWeight = 1
capacity = 1
sourceHealth = 0
targetHealth = 1
```

Expected:

```text
attenuation = 1
compatibility = 1
confidence = 0
downstreamEvidence = 0
```

IEEE-754:

```text
compatibility = 3ff0000000000000
confidence    = 0000000000000000
```

---

## V5 — directional example

Forward input:

```text
distance = 1
orientationWeight = 1
capacity = 0.8
sourceHealth = 0.9
targetHealth = 0.7
```

Expected forward:

```text
attenuation = 1 / 1.1
compatibility = 0.7272727272727273
confidence = 0.7
downstreamEvidence = 0.3181818181818182
```

IEEE-754:

```text
compatibility = 3fe745d1745d1746
confidence    = 3fe6666666666666
```

Reverse comparison fixture keeps distance/capacity/health values but uses:

```text
orientationWeight = 0
```

Expected reverse:

```text
compatibility = 0
confidence = 0.7
downstreamEvidence = -0.7
```

IEEE-754:

```text
compatibility = 0000000000000000
confidence    = 3fe6666666666666
```

This fixture demonstrates intentional directional semantics.

---

## Invalid vectors

The interpreter must reject rather than clamp:

- distance < 0;
- orientationWeight < 0 or > 1;
- capacity < 0 or > 1;
- sourceHealth < 0 or > 1;
- targetHealth < 0 or > 1;
- any consumed NaN;
- any consumed infinity;
- non-finite computed attenuation/compatibility/confidence.

---

## Independent recomputation record

The expected compatibility/confidence values and binary64 encodings above were
independently recomputed from the documented equations, not produced by a D8D
runtime implementation.

These vectors become the oracle for future implementation tests.
