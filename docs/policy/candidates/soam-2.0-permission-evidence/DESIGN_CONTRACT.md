# SOAM Permission Evidence Design Contract

> **status:** design baseline v0  
> **ratification:** not performed  
> **next:** v0 -> amendment -> v1 ratification  
> **predecessor:** none  
> **supersedes:** none

## Baseline immutability and amendments

This file, once accepted into `main` as design baseline v0, is a stable
reference and MUST NOT be silently rewritten to change v0 semantics.

Any semantic change after v0 acceptance MUST be introduced as an explicit
amendment artifact or explicitly delimited amendment section that records:

- amendment identifier (`amendment-N`);
- base = `design baseline v0`;
- reason for the amendment;
- exact affected contract sections;
- resulting disposition toward v1 ratification.

An amendment does not retroactively redefine v0. It creates a visible
successor step in the chain:

```text
design baseline v0
-> amendment-N
-> ...
-> v1 ratification
```

Editorial corrections that can change interpretation MUST be treated as
amendments rather than silent edits.

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


## 56. Normative record-construction boundary

`ProductionPermissionPrerequisiteRecord` is a trusted immutable value object.

Its trust property is enforced by the type system, not by convention.

Required production shape:

```cpp
class ProductionPermissionPrerequisiteRecord final {
public:
    ProductionPermissionPrerequisiteRecord(
        const ProductionPermissionPrerequisiteRecord&) = default;
    ProductionPermissionPrerequisiteRecord& operator=(
        const ProductionPermissionPrerequisiteRecord&) = default;

    const PermissionDecisionId& decisionId() const noexcept;
    const ProductionTransitionRequestBinding& binding() const noexcept;
    const PermissionAttestationId& attestationId() const noexcept;
    const PermissionIssuerId& issuerId() const noexcept;
    const PermissionPolicySnapshotId& policySnapshotId() const noexcept;
    bool satisfied() const noexcept;

private:
    ProductionPermissionPrerequisiteRecord(/* trusted fields */);

    friend class ProductionPermissionVerifier;
};
```

Normative requirements:

- no public default constructor;
- no public raw-field constructor;
- no aggregate initialization;
- no public setters;
- class is `final`;
- all externally observable state is read-only;
- arbitrary caller code cannot synthesize a trusted record.

Compile-fail tests must prove these properties.

## 57. D -> C1 conversion is total and non-decision-bearing

The D -> C1 conversion exists because the trusted D record carries richer
provenance than the intentionally narrow C1 permission evidence type.

Therefore the two types are not aliases.

The conversion is nevertheless a total mapping:

```cpp
class ProductionPermissionC1Adapter final {
public:
    [[nodiscard]]
    ProductionPermissionC1Prerequisite
    convert(
        const ProductionPermissionPrerequisiteRecord& record) const noexcept;
};
```

Normative mapping:

```text
record.binding()   -> PermissionPrerequisiteEvidence.context()
record.satisfied() -> PermissionPrerequisiteEvidence.satisfied()
```

The adapter MUST NOT:

- return optional/variant/rejection;
- verify the signature again;
- inspect issuer trust;
- inspect policy currency;
- inspect live state;
- inspect freshness/revalidation;
- inspect invariant/resilience;
- evaluate eligibility;
- invoke authority or execution logic.

If any future requirement makes conversion conditional, that change creates a
new decision layer and must not be added silently to this adapter.

The phrase "conversion MUST NOT fail" is therefore a design property of the
well-formed trusted input type plus this total mapping, not a claim that C++ can
survive arbitrary memory corruption or undefined behavior.

## 58. Policy snapshot identity is provenance, not post-verification enforcement

For an already-created `ProductionPermissionPrerequisiteRecord`,
`PermissionPolicySnapshotId` records which accepted verification policy
established the decision.

It is provenance.

It is not a dynamic "must still be current" enforcement gate.

Therefore:

```text
record created under accepted policy snapshot P
+ later policy rotation to P2
!= automatic invalidation of record
```

Future attestation verification uses the then-active accepted permission policy.

An already-created trusted record remains evidence of the completed verification
decision for its exact request and policy provenance.

D does not perform a later "policy must still be current" check.

Request epoch freshness remains a separate A1 concern.

Key/policy rotation is expressed through a new permission-policy revision and
affects future verification decisions.

## 59. Exact canonical byte layout

PermissionAttestationV1 signed payload is a fixed-size byte sequence.

The domain separator is exactly the following 31 bytes:

```text
ASCII("SOAM:PERMISSION-ATTESTATION:V1") || 0x00
```

There is no C-string interpretation at the cryptographic boundary.

The complete V1 layout is exactly:

```text
offset  size  field
0       31    domainSeparator
31       2    formatMajor               u16 little-endian
33       2    formatMinor               u16 little-endian
35      16    attestationId             raw bytes
51      16    issuerId                  raw bytes
67      16    requestDecisionId         raw bytes
83       8    sourceNodeId              u64 little-endian
91       8    targetNodeId              u64 little-endian
99       8    relationshipGeneration    u64 little-endian
107      1    direction                 u8
108      8    transitionClass           u64 little-endian
116      8    stateVersion              u64 little-endian
124      1    decision                  u8
125     16    permissionPolicyId         raw bytes
141      2    permissionPolicyMajor      u16 little-endian
143      2    permissionPolicyMinor      u16 little-endian
```

Total signed payload length:

```text
145 bytes exactly
```

No padding.
No host-endian encoding.
No native struct serialization.
No JSON.
No UTF-8 payload fields in V1.
No optional fields.
No trailing extension area.

## 60. Exact-size parser rule

The V1 parser is exact-size.

Normative rule:

```text
input length == 145 bytes
```

Anything else is malformed.

Therefore:

- 144 bytes or fewer => `AttestationMalformed`;
- 146 bytes or more => `AttestationMalformed`;
- trailing bytes are rejected, never ignored;
- missing bytes are rejected, never default-filled;
- unknown enum discriminants are rejected;
- wrong domain separator bytes are rejected.

The signature bytes are transported separately from the 145-byte signed payload
or in a higher-level fixed envelope whose exact size is independently specified.

## 61. Frozen interoperability vectors are mandatory

D V1 is not implementation-complete without frozen cryptographic test vectors.

At minimum the accepted policy/test provenance must include one positive vector
containing:

- every logical field value;
- exact 145-byte canonical payload hex;
- exact 32-byte Ed25519 test public key;
- exact 64-byte Ed25519 signature;
- expected verification result = true.

The key used for interoperability vectors MUST be an explicitly designated test
key and MUST NOT be the production Root Operator permission key.

Required negative vectors include at least:

- one changed stateVersion byte -> verification false;
- changed direction -> verification false;
- changed issuerId -> verification false;
- changed domain separator byte -> verification false;
- changed signature byte -> verification false;
- truncated payload -> parse failure;
- payload with trailing byte -> parse failure.

These vectors define cross-implementation compatibility, not merely unit-test
convenience.

## 62. STOP D contract

D STOP means exactly:

```text
permission verification completed
-> trusted request-bound ProductionPermissionPrerequisiteRecord exists
-> ready only for total D -> C1 conversion
```

D STOP does NOT mean:

- ready for authority;
- ready for execution;
- ready for a human approval step;
- eligible for authority consideration.

The only permitted downstream semantic transition from D is through the narrow
D -> C1 conversion into `PermissionPrerequisiteEvidence`.

## 63. STOP C1 contract

C1 already produces an immutable
`ProductionTransitionEligibilityDecision` through a private constructor
controlled by `ProductionTransitionEligibilityEvaluator`.

That pattern is normative for the C1 boundary.

Authority must consume the trusted C1 decision artifact, not independently
reassemble or re-evaluate the five prerequisite channels.

Only:

```text
ProductionTransitionEligibilityDecision {
    eligibility = eligible_for_authority_consideration,
    exact request binding
}
```

may proceed to a future Authority layer.

C1 STOP does NOT mean ready for execution.

It means only:

```text
all accepted prerequisite channels passed
-> request is eligible to be considered by explicit Authority
```

## 64. Eligibility artifacts are immutable, not consumable

`ProductionTransitionEligibilityDecision` is an immutable decision artifact.

It is not the replay-consumption object.

Likewise, D permission records are immutable and replayable as evidence for the
same exact request.

Single-use semantics belong later:

```text
C1 eligibility decision
-> authority decision
-> single-use execution capability
-> atomic execution transaction
```

The eventual execution boundary must atomically bind:

```text
current stateVersion validation
+ exact authority/request lineage validation
+ capability not-consumed check
+ transition mutation
+ publication of new stateVersion
+ capability consumption
```

as one commit context.

No intermediate durable state may expose "consumed but mutation not committed"
or "mutation committed but capability still reusable".

This is a future Authority/Execution contract and is not implemented by D.

## 65. Ceremony document and key-generation ordering

The Root Operator issuer ceremony is a separate security artifact from the
permission verification policy.

The ceremony procedure MUST exist before production key generation.

The procedure specifies prospectively:

- approved generation environment;
- approved generation software/version;
- entropy/source requirements;
- verification steps;
- handling and transfer rules;
- private-key storage model;
- backup/recovery rules;
- compromise response;
- rotation process;
- assignment process for stable `PermissionIssuerId`.

The procedure does not contain a production public key that does not yet exist.

After generation, a separate policy/provenance record binds:

```text
PermissionIssuerId
+
generated Ed25519 public key
+
IssuerCeremonyDocumentDigestAlgorithm
+
IssuerCeremonyDocumentDigest
```

This two-artifact structure demonstrates that the accepted key is claimed to
have been generated under a pre-existing procedure.

The ceremony-document digest provides document identity/integrity binding.

It does not, by itself, prove temporal ordering. Strong temporal proof would
require an external timestamp/notarization/transparency mechanism and is not
claimed by V1.

## 66. Issuer identity remains independent from key material

`PermissionIssuerId` is assigned, not derived from the Ed25519 public key.

Normative invariant:

```text
PermissionIssuerId != hash(publicKey)
```

and no deterministic public-key derivation rule defines issuer identity.

This permits:

```text
same logical issuer
+ rotated credential
+ new permission-policy revision
```

without changing logical issuer identity.

## 67. Remaining normative gates

After the preceding review, the remaining independent production choices are:

### D1/D3 combined gate

Accept or reject:

```text
signature profile = Ed25519
verification provider = libsodium
linkage = PRIVATE to soam_transition_permission
production module = verifier-only
```

### D2 ceremony gate

Before any production key generation:

- accept Root Operator Issuer Ceremony V1;
- assign stable opaque PermissionIssuerId;
- approve private-key custody model;
- then generate keypair externally;
- then bind public key + ceremony document digest into a new accepted permission
  policy provenance record.

No production key is to be generated as part of PR #32.

## 68. Final pre-merge design condition

PR #32 may become design-mergeable once the following are all explicit in the
review record:

- unforgeable immutable D record;
- total non-decision-bearing D -> C1 conversion;
- policySnapshotId provenance semantics;
- exact 145-byte canonical payload;
- exact-size parser;
- mandatory frozen interoperability vectors;
- exact STOP D contract;
- exact STOP C1 contract;
- Authority/Execution consumption deferred to an atomic later layer;
- two-artifact ceremony/key provenance model.

Executable D remains blocked until D1/D3 and D2 are separately accepted.


## 69. Governance home V1

For V1, permission-policy governance records live in the accepted `main`
history of this repository.

The governance ordering is the first-parent ancestry of accepted merge commits
on `main`.

A governance event is identified by its exact merge commit SHA plus its
first-parent position relative to other accepted governance events.

This V1 model requires the governance branch to be protected against force-push
and history rewrite.

No separate ledger service and no independent monotonic counter are introduced
for V1.

If repository governance can no longer guarantee append-only accepted history,
this V1 governance-sequence model is invalid and must be revised before new
production policy admission.

## 70. Ratification record trust V1

V1 ratification records do not introduce a second signature PKI.

A `ProductionPermissionPolicyRatificationRecord` derives its trust from:

```text
reviewed policy artifact
+ reviewed ratification record
+ accepted merge into protected main
+ exact immutable git provenance
```

CI may validate syntax, digests, canonical encodings, and consistency, but CI is
not the Policy Ratifier.

The permission issuer Ed25519 credential does not sign or self-ratify the policy
that trusts that issuer.

Adding cryptographic signatures to governance records in a later version is a
separate governance-policy change and must define what the new signature means.

## 71. Design freeze disposition

With sections 69 and 70, the D design baseline has no remaining blocker for the
initial D1/D3 implementation slice.

The first D1/D3 implementation is intentionally limited to:

- strict 145-byte PermissionAttestationV1 parser;
- fixed-size parsed payload/signature types;
- one Ed25519 verification primitive backed by libsodium;
- frozen positive/negative interoperability vectors;
- test-only trust fixtures;
- no production issuer key;
- no D2 ceremony artifact;
- no production activation path;
- no D -> C1 adapter yet.

Further governance elaboration is deferred until required by a concrete
implementation or D2 ceremony.
