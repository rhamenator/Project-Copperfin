# Generated Launcher Package Trust Contract

This document defines the prerequisite contract for the Windows generated-launcher inventory trust work in #4387/#4397. It is deliberately separate from license status, license payloads, and the existing `launcher_artifact=` provenance fields.

## Scope

The release package may contain an additive trust envelope named `app.cftrust` and a detached signature named `app.cftrust.sig`. The envelope authenticates only the files that execute before the managed launcher starts:

- the public apphost;
- the required internal apphost and generated runtime sidecars; and
- the optional generated launcher PDB.

Entries contain package-relative direct file names only. Source paths, debug source paths, user data paths, license fields, and localized display text must never enter this envelope.

The existing `app.cfmanifest`, `app.cfdebug`, `launcher_artifact=` lines, and their version fields remain unchanged. Their hashes continue to support inventory and post-launch checks; they are not authenticated by this document until #4387 consumes the signed envelope.

## Canonical Envelope

The signed bytes are UTF-8 text with LF line endings and a final LF. There is no BOM, trailing whitespace, or blank line. Records are emitted in this exact order:

```text
launcher_inventory_version=1
hash_algorithm=sha256
signature_algorithm=ed25519
signer_key_id=<approved-key-id>
artifact=<role>|<direct-package-file-name>|<lowercase-sha256>
```

`artifact` records are sorted by role (`public_apphost`, `runtime_required`, `debug_optional`) and then by bytewise package file name. Duplicate role/name pairs are invalid. Hashes are exactly 64 lowercase hexadecimal characters. Key IDs contain only ASCII letters, digits, `.`, `_`, and `-`. File names cannot contain `/`, `\\`, `|`, control characters, `.` or `..` path components, or the `..` sequence.

The detached signature sidecar is UTF-8 text with LF line endings and a final LF:

```text
launcher_signature_version=1
signature_algorithm=ed25519
signer_key_id=<same-id-as-envelope>
signature_base64=< exactly 64 raw-signature bytes encoded as base64 >
```

The signer ID is covered by the signed envelope and repeated in the sidecar for deterministic key lookup. A mismatch is malformed, not a request to try another key.

## Signing And Provisioning

Release finalization receives an external signing-key reference, not private-key material in a command line, package, or checkout. The approved release workflow must:

1. build and validate the package;
2. derive `app.cftrust` from the finalized package-relative inventory;
3. resolve the approved key reference from the release secret store or a file outside the checkout;
4. sign the exact canonical bytes with the approved Ed25519 signing tool;
5. write only `app.cftrust` and `app.cftrust.sig` into the package; and
6. verify the detached signature with the release public-key registry before publishing.

The signer interface is intentionally a release-tool contract rather than a product API:

```text
copperfin-package-signer sign-launcher-inventory \
  --input app.cftrust \
  --output app.cftrust.sig \
  --key-ref <external-approved-key-reference>
```

`--key-ref` may identify a CI secret-manager object or an external protected file. It must not accept a private key embedded in an issue, source file, package, or generated test fixture. The repository may contain public test vectors and signatures, but never production or machine-specific private keys.

## Verification API And Failure States

The native verifier is `copperfin::package_trust`, not `copperfin::licensing`. It may reuse the verify-only Ed25519 primitive, but it owns its envelope parser, canonicalization, launcher trust-key registry, and result statuses. It must not call license parsing or make license-state decisions.

The verifier distinguishes:

- `valid`: canonical envelope, approved signer ID, and detached signature all verify;
- `malformed_envelope`: version, fields, ordering, path, digest, sidecar, or signer-ID syntax is invalid;
- `unknown_signer`: the signer ID is well-formed but absent from the launcher trust registry; and
- `invalid_signature`: the approved key exists but the signature does not match the canonical bytes.

The Windows launcher guard will keep exit code `4` for all trust or inventory failures. Human-readable explanations use the active catalog; status codes, signer IDs, envelope keys, role values, and package paths remain invariant.

## Platform Policy

When #4387 is implemented, Windows generated-launcher packages require both trust sidecars and reject unsigned or unknown-signer inventories before managed apphost startup. Existing containment, regular-file, physical-identity, and SHA-256 checks remain necessary after signature verification.

POSIX and macOS do not claim this Windows trust boundary yet. Until platform-specific release signing and verification are approved, they retain the current unsigned inventory behavior and report the trust capability as unsupported rather than treating a recomputed unsigned inventory as authenticated.

## Fixture Evidence

`test_package_launcher_inventory_trust` covers canonical ordering, traversal rejection, unknown signer IDs, invalid signatures, and an RFC 8032 Ed25519 public verification vector. No private or machine-specific key is embedded. A future release-signing integration test must add a detached signature over the canonical envelope using an external key reference and must exercise modified, removed, duplicate, and ambiguous artifacts through the Windows guard.
