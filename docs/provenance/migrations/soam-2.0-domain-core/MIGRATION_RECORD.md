# Migration Record — SOAM 2.0 Domain Core

## Status

- Migration stage: `candidate`
- Acceptance status: not accepted into Current Baseline
- Source archive: `adaptive-mesh-main (2).zip`
- Source archive SHA-256: `f14900ddb77852dbf3935562fdb5f73a051338ade1bb60a84f4d1fde59577c03`
- Target repository: `hronoaza/studio`
- Migration branch: `migration/soam-2.0-domain-core`
- Base: Current Baseline after Adaptive Mesh 1.1 acceptance

## Phase A boundary

This migration intentionally contains only the non-mutating domain chain:

`InteractionObservation → BridgeConfidence → BridgePolicyEvidence → BridgePersistence`

Included source headers:

- `interaction_observation.hpp`
- `bridge_confidence.hpp`
- `adaptive_bridge_policy.hpp`
- `bridge_persistence.hpp`

The four source headers are imported from the archive without algorithmic modification.

## Explicitly deferred

Not imported in Phase A:

- `SpatialAdaptiveMesh` runtime
- worker-pool/runtime lifecycle implementation
- `BridgeTransitionAuthorization`
- production transition eligibility/evaluator
- K11/K12 live transition machinery
- production authority/capability types
- experiments, including historical KIKO material
- repository-level license/publication metadata
- DOI/IP/patent claims

## Reserved seam

`BridgePersistence` contains a forward declaration and friendship for
`detail::ProductionPersistenceAccess`. Phase A does not define or exercise
that production access class. The seam is preserved as source provenance and
must not be interpreted as granting production authority.

## Validation strategy

Phase A tests only the domain contracts:

- bounded and finite observation/confidence values;
- explicit construction;
- non-forgeable `BridgePolicyEvidence` from a raw `double`;
- evidence symmetry, monotonic confidence response, and bounded range;
- persistence activation/release hysteresis;
- direction reversal;
- reset behavior;
- independence of separate persistence instances.

No runtime mutation capability exists in this phase.

Final acceptance remains a separate operator decision.
