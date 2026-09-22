# SOAM 2.0 — Permission Evidence Design Contract

## Status

- Layer: D — permission prerequisite evidence
- Document class: design-contract candidate
- Design origin baseline: `f244fde6ddb7b97028f8f42106fd90c5bacf680e`
- Accepted upstream:
  - Transition Request derivation
  - A1 Freshness/Revalidation
  - A2 Freshness/Revalidation -> C1 adapter
  - B1 Invariant evidence
  - C Resilience evidence
- Downstream contract: C1 `PermissionPrerequisiteEvidence`
- Acceptance status: design-review pending
- Executable implementation: none
- Authority effect: none

## 1. Boundary

D defines only the permission prerequisite channel:

```text
ProductionDerivedTransitionRequest
+ trusted permission attestation
+ trusted/versioned permission-verification policy
-> ProductionPermissionPrerequisiteRecord
   OR typed permission rejection
-> STOP
```

D does not produce:

- InvariantPrerequisiteEvidence;
- ResiliencePrerequisiteEvidence;
- FreshnessPrerequisiteEvidence;
- RevalidationPrerequisiteEvidence;
- ProductionTransitionPrerequisiteSet;
- ProductionTransitionEligibilityDecision;
- transition authority;
- execution capability;
- mutation.

## 2. Principal separation

```text
permission attestation
!= eligibility
!= authority
!= capability
!= execution
```

A valid permission attestation means only:

> an accepted permission issuer explicitly permitted this exact transition
> request to proceed to prerequisite/authority consideration.

It does not mean:

- the transition is safe;
- the transition is resilient;
- the request is fresh;
- all prerequisites are complete;
- authority has been granted;
- execution is possible or allowed.

## 3. No implicit permission

Permission must not be inferred from:

- GitHub repository write access;
- ChatGPT connector permissions;
- process ownership;
- operating-system user identity;
- CI success;
- request direction;
- transition class alone;
- D9 recommendation;
- invariant=true;
- resilience=true;
- freshness=true;
- revalidation=true;
- possession of executable code.

Therefore:

```text
technical capability != permission
```

and:

```text
permission != authority
```

## 4. Why D needs an independent trust source

The current repository has no production permission identity or attestation
model.

If the transition runtime could freely construct a trusted permission record,
the permission prerequisite would collapse into a self-asserted boolean.

D therefore requires a trust source independent from the evaluator that consumes
the permission.

Candidate V1 trust model:

```text
external permission issuer
-> signed immutable attestation
-> in-process verifier with public trust anchor only
-> ProductionPermissionPrerequisiteRecord
```

The transition runtime must not possess issuer signing secrets.

## 5. Candidate V1 attestation semantics

A permission attestation is bound to one exact derived request.

Candidate logical payload:

```text
PermissionAttestationV1 {
    attestationId
    issuerId
    requestDecisionId
    sourceNodeId
    targetNodeId
    relationshipGeneration
    requestedDirection
    transitionClass
    stateVersion
    decision = PERMIT | DENY
    permissionPolicyId
    permissionPolicyMajor
    permissionPolicyMinor
}
```

The signature covers the canonical serialization of every field above.

No field is caller-omittable.

## 6. Exact request binding

The verifier must require exact equality between attestation claims and the
trusted `ProductionDerivedTransitionRequest`:

- request decision identity;
- source node;
- target node;
- relationship generation;
- direction;
- transition class;
- stateVersion.

Therefore a permission for one request cannot be reused for:

- another relationship;
- another generation;
- another direction;
- another stateVersion;
- another request decision;
- another transition class.

## 7. No time semantics in V1

Candidate V1 intentionally has no wall-clock issuance/expiry field.

Reason:

- exact request stateVersion already scopes the attestation to one runtime epoch;
- A1 Freshness independently determines whether that epoch is still current;
- adding time introduces clock trust, skew, expiry policy, and another freshness
  semantic before such policy exists.

A future permission-policy version may add temporal validity through a separate
design gate.

## 8. Permit and deny are both explicit

Candidate attestation decision:

```cpp
enum class PermissionAttestationDecision : std::uint8_t {
    deny,
    permit
};
```

A cryptographically valid `DENY` is a valid permission decision record with:

```text
satisfied=false
```

It is not a verifier rejection.

This preserves:

```text
trusted negative decision != invalid evidence
```

## 9. Invalid attestation vs denied permission

Examples producing a valid record:

- valid trusted PERMIT -> `satisfied=true`;
- valid trusted DENY -> `satisfied=false`.

Examples producing rejection:

- signature invalid;
- issuer unrecognized;
- malformed canonical encoding;
- request binding mismatch;
- unsupported attestation version;
- unsupported verification-policy revision;
- internal verification failure.

## 10. Candidate cryptographic profile

Preferred V1 profile:

```text
signature algorithm: Ed25519
public key size: 32 bytes
signature size: 64 bytes
canonical payload: fixed binary encoding
```

Rationale:

- asymmetric verification keeps signing authority outside runtime;
- verifier needs only public keys;
- fixed-size keys/signatures simplify canonical parsing;
- deterministic signing avoids nonce-management classes of failure;
- mature standard implementations exist.

This is a candidate until explicitly accepted.

## 11. Rejected shared-secret model

HMAC/shared-secret verification is not preferred for V1.

If runtime holds the verification secret, runtime also possesses enough material
to forge permission attestations.

That collapses issuer/verifier separation.

Therefore:

```text
shared verifier/signing secret
=> rejected for production D V1
```

unless a later design introduces a hardware/remote trust boundary that prevents
forgery.

## 12. Issuer identity

Candidate issuer identity:

```cpp
class PermissionIssuerId final {
public:
    using Bytes = std::array<std::uint8_t,16>;
    ...
};
```

The issuer ID is not self-authenticating.

It is trusted only when paired with a public key present in the accepted
permission-verification policy snapshot.

## 13. Trusted issuer registry

Candidate V1 permission policy includes a fixed immutable set of trusted issuer
descriptors:

```text
TrustedPermissionIssuer {
    PermissionIssuerId issuerId
    Ed25519PublicKey publicKey
}
```

The set is:

- versioned;
- immutable within one policy snapshot;
- not caller configurable;
- reproducibly bound into implementation/policy provenance.

No dynamic network key discovery is required for V1.

## 14. Policy identity

Candidate policy family:

```text
PermissionVerificationPolicyV1
major = 1
minor = 0
attestationFormat = PermissionAttestationV1
signatureProfile = Ed25519
trustedIssuerSet = versioned immutable registry
transitionClass = BridgeCouplingAdjustmentV1
```

The policy snapshot must include:

- opaque policy snapshot ID;
- semantic policy ID;
- major/minor version;
- implementation revision kind;
- reproducible implementation digest;
- signature profile identifier;
- trusted issuer descriptors;
- supported transition class.

## 15. Permission issuer is not transition authority

The entity that signs permission attestations is called a permission issuer,
not a transition authority.

Its signature states only:

```text
this exact request is permitted to continue through prerequisite evaluation
```

The later authority layer remains free to deny execution even when permission,
invariant, resilience, freshness, and revalidation all succeed.

## 16. No signing API in runtime

The production D module must not expose:

- private key loading;
- attestation signing;
- issuer secret generation;
- `permit(request)` that directly fabricates trusted permission;
- operator bypass flags.

Production runtime is verifier-only.

Test-only helpers may create deterministic fixtures in private test surfaces,
but must not be linked/exported as production signing APIs.

## 17. Attestation origin

The signed attestation may arrive from:

- an operator-facing external signing tool;
- another trusted service;
- an offline approval workflow;
- a hardware-backed signer.

D V1 does not prescribe the user interface or transport.

The verifier consumes bytes plus trusted request/policy context.

## 18. Candidate canonical encoding

V1 should use a fixed-width binary canonical representation.

Candidate ordering:

```text
domainSeparator
formatMajor
formatMinor
attestationId[16]
issuerId[16]
requestDecisionId[16]
sourceNodeId[u64-be]
targetNodeId[u64-be]
relationshipGeneration[u64-be]
direction[u8]
transitionClass[u64-be]
stateVersion[u64-be]
decision[u8]
permissionPolicyId[16]
permissionPolicyMajor[u16-be]
permissionPolicyMinor[u16-be]
```

No JSON/text canonicalization is recommended for the signed production payload.

Exact byte layout remains a critical-review item.

## 19. Domain separation

The signed payload must begin with a fixed domain separator unique to this
attestation family.

Candidate:

```text
"SOAM:PERMISSION-ATTESTATION:V1\0"
```

This prevents the same signature from being interpreted as another SOAM object
family.

## 20. Request identity and lineage

D consumes only a trusted `ProductionDerivedTransitionRequest`.

The evaluator must validate basic request lineage consistency before accepting a
permission attestation.

It must not accept a loose
`ProductionTransitionRequestBinding` plus caller-supplied IDs as equivalent
input.

## 21. Candidate evidence record

D should introduce a restricted-origin record distinct from C1 evidence:

```cpp
class PermissionDecisionId final;

class ProductionPermissionPrerequisiteRecord final {
public:
    const PermissionDecisionId& decisionId() const noexcept;
    const ProductionTransitionRequestBinding& binding() const noexcept;
    const PermissionAttestationId& attestationId() const noexcept;
    const PermissionIssuerId& issuerId() const noexcept;
    const PermissionPolicySnapshotId& policySnapshotId() const noexcept;
    bool satisfied() const noexcept;

private:
    // verifier/evaluator-only construction
};
```

This record is trusted evidence that a verified attestation produced a permit or
deny result for the exact request.

## 22. Decision identity

Each completed permission verification decision should have one opaque non-zero
128-bit `PermissionDecisionId`.

Properties:

- no public default construction;
- no raw-byte public construction;
- production-origin only;
- copyable after creation;
- OS-backed generation consistent with existing SOAM decision identities.

Identity-generation failure before a decision exists returns `std::nullopt`.

## 23. Candidate attestation representation

Suggested public immutable value type:

```cpp
class ProductionPermissionAttestation final {
public:
    // read-only accessors for parsed claims/signature
private:
    // no public trusted constructor from claims
};
```

Important distinction:

A byte blob from external transport is untrusted input.

Parsing a blob may produce an untrusted parsed envelope.

Only signature + policy + request verification may produce
`ProductionPermissionPrerequisiteRecord`.

## 24. Parsing boundary

Recommended split:

```text
untrusted bytes
-> strict PermissionAttestationV1 parser
-> parsed untrusted attestation
-> cryptographic/request/policy verifier
-> trusted permission record OR rejection
```

Parser success alone does not create trusted permission evidence.

## 25. Fail-closed parser requirements

The V1 parser must reject:

- wrong total length;
- unknown format version;
- invalid enum discriminants;
- all-zero required identifiers;
- non-canonical integer encoding;
- trailing bytes;
- unsupported signature profile;
- malformed key/signature lengths.

No partial/default claim filling is permitted.

## 26. Candidate rejection reasons

```cpp
enum class TransitionPermissionReason : std::uint8_t {
    RequestLineageInconsistent,
    TransitionClassUnsupported,
    PolicyRevisionUnrecognized,
    AttestationMalformed,
    IssuerUnrecognized,
    SignatureInvalid,
    RequestBindingMismatch,
    InternalVerificationFailure
};
```

Exact precedence remains a critical-review item.

## 27. Request mismatch semantics

A correctly signed attestation for a different request is not a valid deny.

It is a rejection:

```text
RequestBindingMismatch
```

This prevents a signed permit/deny for request A from being interpreted as a
decision about request B.

## 28. Policy mismatch semantics

An attestation explicitly names the permission policy family/version it was
issued under.

If the verifier policy does not match those claims:

```text
PolicyRevisionUnrecognized
```

or a more specific policy-binding rejection may be introduced in review.

The verifier must not silently reinterpret old attestations under new policy.

## 29. Transition class scope

D V1 applies only to:

```text
BridgeCouplingAdjustmentV1
```

A narrow class-identity seam analogous to B1/C may be used.

It must not grant construction access to:

- request bindings;
- PermissionPrerequisiteEvidence;
- other C1 evidence classes;
- eligibility decisions;
- authority;
- capabilities;
- execution.

## 30. No runtime-state read required

Permission verification does not need to read live mesh state.

It verifies:

- trusted request identity/binding;
- signed attestation;
- trusted permission policy.

Freshness/revalidation remain separate prerequisites.

Therefore D V1 should not depend on `SpatialAdaptiveMesh` or topology snapshot
sources.

## 31. No invariant/resilience duplication

D does not inspect:

- IdentityInvariant;
- bridge capacity/status;
- alternate paths;
- node health.

Those belong to B1/C.

A valid permission may coexist with invariant/resilience failure.

## 32. No eligibility call

D must not instantiate or invoke:

```text
ProductionTransitionEligibilityEvaluator
```

D stops before full prerequisite assembly.

## 33. Restricted C1 conversion seam

D itself stops at `ProductionPermissionPrerequisiteRecord`.

A later narrow adapter may convert a verified record into:

```text
PermissionPrerequisiteEvidence
```

That adapter may receive friendship only for
`PermissionPrerequisiteEvidence`.

It must not gain access to:

- InvariantPrerequisiteEvidence;
- ResiliencePrerequisiteEvidence;
- FreshnessPrerequisiteEvidence;
- RevalidationPrerequisiteEvidence;
- request binding constructors;
- eligibility decisions;
- authority;
- capability;
- execution.

## 34. Revocation

V1 has no online revocation lookup.

Reason:

- no accepted external permission service exists;
- network availability would become part of permission validity;
- request/stateVersion binding makes attestations narrow-lived in practice.

If revocation is required, it needs a separate versioned design with explicit
availability/failure semantics.

This is an open policy question, not an implicit guarantee.

## 35. Replay

Reusing the same valid attestation against the exact same trusted request may
produce another observational verification record.

It does not grant execution and does not consume the attestation.

Cross-request replay fails because requestDecisionId and full binding are signed.

Single-use consumption belongs to a later authority/execution transaction layer
if required.

## 36. Auditability

A trusted permission record should preserve:

- attestation identity;
- issuer identity;
- request binding;
- permission-policy snapshot identity;
- permit/deny outcome;
- independent permission decision identity.

It should not expose secret key material.

## 37. Dependency placement

Candidate module:

```text
apps/soam-transition-permission/
```

Target:

```text
soam_transition_permission
AdaptiveMesh::soam_transition_permission
```

Candidate dependencies:

- AdaptiveMesh::soam_transition_request;
- AdaptiveMesh::soam_transition;
- a narrowly selected cryptographic verification dependency, if required.

It should not depend on runtime topology/simulation.

## 38. Compile-fail requirements

Before D implementation acceptance, negative tests must prove callers cannot:

1. default/raw-byte construct PermissionDecisionId;
2. fabricate a trusted ProductionPermissionPrerequisiteRecord;
3. construct trusted policy snapshots with arbitrary issuer keys;
4. call a production signing API in D;
5. directly construct C1 PermissionPrerequisiteEvidence;
6. construct request bindings through D;
7. construct Invariant/Resilience/Freshness/Revalidation evidence through D;
8. invoke C1 eligibility through the permission verifier;
9. obtain authority/capability/execution objects;
10. access private signing-key material from the verifier.

## 39. Positive verification requirements

Implementation acceptance must prove:

- canonical parser rejects malformed/trailing/unsupported input;
- valid trusted PERMIT -> record `satisfied=true`;
- valid trusted DENY -> record `satisfied=false`;
- signature tampering -> rejection;
- issuer substitution -> rejection;
- unknown issuer -> rejection;
- requestDecisionId mismatch -> rejection;
- relationship/direction/class/stateVersion mismatch -> rejection;
- permission-policy mismatch -> rejection;
- exact binding preserved in trusted record;
- no live mesh read occurs;
- no C1/eligibility/authority/execution output exists.

## 40. Security property

Even when:

```text
permission=true
&& invariant=true
&& resilience=true
&& freshness=true
&& revalidation=true
```

the result is still only sufficient input for:

```text
C1 eligibility evaluation
```

and, if eligible:

```text
eligible_for_authority_consideration
```

It is not authority and is not execution.

## 41. Critical design questions

Before D implementation begins, review must decide:

1. Is asymmetric signed attestation the accepted V1 trust model?
2. Is Ed25519 the accepted signature profile?
3. What exact issuer public key(s) are trusted in V1?
4. How is `PermissionIssuerId` assigned to those keys?
5. Exact canonical binary payload layout.
6. Exact domain separator.
7. Is no-time/no-expiry correct for V1?
8. Is no online revocation correct for V1?
9. Is a valid signed DENY represented as `satisfied=false`?
10. Exact rejection precedence.
11. Whether policy ID/version claims belong inside the signed payload exactly as
    proposed.
12. Whether the V1 verifier may depend on a system crypto library or must use a
    vendored/internal implementation.

## 42. Initial design disposition

The key non-negotiable property is independent trust origin:

```text
the component that verifies permission
must not be able to mint permission
```

Asymmetric signed attestations satisfy that property cleanly.

However D is not implementation-ready until the signature profile, trust anchor,
canonical payload, and policy/revocation semantics are explicitly accepted.

## 43. STOP

This is design only.

No permission attestation signer is implemented.
No trusted PermissionPrerequisiteEvidence is constructed.
No prerequisite set is assembled.
No eligibility decision is produced.
No authority, capability, or execution boundary is crossed.


## 44. Critical review — repository crypto baseline

Repository review found no existing production dependency on:

- OpenSSL;
- libsodium;
- an Ed25519 verification library;
- EVP_PKEY Ed25519 APIs;
- a repository-local asymmetric-signature implementation.

Existing platform randomness support does not constitute a signature-verification
stack.

Therefore D V1 must not pretend that asymmetric verification is already an
accepted dependency.

Introducing crypto verification is an explicit architectural dependency decision.

## 45. Critical review — canonical payload disposition

The V1 payload should remain fixed-width binary and fully deterministic.

Reviewed candidate layout:

```text
domainSeparator[30]
formatMajor[u16-be]
formatMinor[u16-be]
attestationId[16]
issuerId[16]
requestDecisionId[16]
sourceNodeId[u64-be]
targetNodeId[u64-be]
relationshipGeneration[u64-be]
direction[u8]
transitionClass[u64-be]
stateVersion[u64-be]
decision[u8]
permissionPolicyId[16]
permissionPolicyMajor[u16-be]
permissionPolicyMinor[u16-be]
```

No optional fields.
No padding.
No native-endian encoding.
No trailing bytes.
No text serialization.

The exact domain separator remains tied to the final V1 signature-profile
acceptance.

## 46. Critical review — no-time V1

The no-time/no-expiry design remains coherent for V1 because the attestation is
bound to:

- requestDecisionId;
- exact relationship generation;
- exact stateVersion.

A1 Freshness independently determines whether the request epoch is still
current.

Therefore D does not need a second wall-clock freshness channel in V1.

This does not claim the attestation is permanently valid. It is meaningful only
for the exact request object it signs.

## 47. Critical review — no online revocation V1

No online revocation lookup is retained for V1.

Adding revocation would require accepted semantics for:

- network availability;
- stale revocation data;
- fail-open vs fail-closed behavior;
- cache lifetime;
- issuer rotation.

Those are not currently defined.

V1 instead uses a narrow request-bound attestation and versioned trusted issuer
set.

Issuer/key rotation requires a new accepted permission-policy revision.

## 48. Critical review — signed DENY

A correctly signed trusted `DENY` is confirmed as a valid negative permission
decision:

```text
signature valid
issuer trusted
request binding exact
decision = DENY
=> ProductionPermissionPrerequisiteRecord{satisfied=false}
```

This is not a verifier rejection.

The distinction is required so C1 can later distinguish:

```text
permission explicitly denied
```

from:

```text
permission evidence malformed/untrusted/unavailable
```.

## 49. Critical review — policy claims are signed

The permission policy ID and major/minor version remain inside the signed
payload.

This prevents a valid signature issued under one permission-policy context from
being silently replayed under another policy revision.

The verifier requires exact policy-claim equality with the accepted policy
snapshot.

## 50. Critical review — rejection precedence

Candidate D V1 precedence after decision-ID establishment:

1. RequestLineageInconsistent
2. TransitionClassUnsupported
3. PolicyRevisionUnrecognized
4. AttestationMalformed
5. IssuerUnrecognized
6. SignatureInvalid
7. RequestBindingMismatch
8. InternalVerificationFailure

A valid signed DENY is never a rejection.

## 51. Critical review — trust-anchor shape

For V1, the safest trust-anchor shape is a small immutable issuer registry in
the accepted permission policy snapshot.

The registry should not be populated from arbitrary caller input.

Preferred production shape:

```text
PermissionVerificationPolicyV1
-> fixed trusted issuer descriptors
-> each descriptor = issuerId + publicKey
```

The exact issuer ID and public key are not yet defined and must not be invented
by implementation.

## 52. Critical review — signer separation

The production D module remains verifier-only.

No production target in `apps/soam-transition-permission/` may:

- generate a signing key;
- load a private signing key;
- sign an attestation;
- expose a helper that returns trusted permission records without signature
  verification.

If a signing utility is later needed, it must be a separate operator/tooling
surface with separate provenance and must not be linked into the verifier
library.

## 53. Critical review — unresolved normative gates

D V1 is not implementation-ready until three independent choices are accepted:

### Gate D1 — signature profile

Candidate:

```text
Ed25519
public key = 32 bytes
signature = 64 bytes
```

### Gate D2 — trust anchor

Must provide:

```text
PermissionIssuerId
+
exact trusted Ed25519 public key
```

The implementation must not fabricate these values.

### Gate D3 — verification dependency

A concrete production verification implementation must be selected.

Acceptable categories:

- system/library dependency with mature Ed25519 verification;
- narrowly vendored audited implementation;
- platform cryptographic provider where semantics are portable enough for the
  supported build matrix.

A home-grown Ed25519 implementation is rejected.

## 54. Implementation readiness

Until D1, D2, and D3 are accepted:

```text
D runtime implementation = BLOCKED
```

The rest of the design is sufficiently specified to proceed immediately once
those trust decisions exist.

## 55. Review STOP

No executable permission verifier is added by this review.
No signer is added.
No PermissionPrerequisiteEvidence is constructed.
No prerequisite set is assembled.
No eligibility decision is produced.
No authority, capability, or execution surface is opened.
