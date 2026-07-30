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
- `unknown_signer`: the signer ID is well-formed but absent from the launcher trust registry;
- `ambiguous_signer`: more than one registry entry claims the selected signer ID; and
- `invalid_signature`: the approved key exists but the signature does not match the canonical bytes.

The Windows launcher guard now checks present trust sidecars before any managed apphost process starts and keeps exit code `4` for all trust or inventory failures. Human-readable explanations use the active catalog; status codes, signer IDs, envelope keys, role values, and package paths remain invariant. Development builds preserve unsigned fallback when neither sidecar is present. A release build must set `COPPERFIN_ENFORCE_LAUNCHER_TRUST=ON` and configure `COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER` with an approved public-key registry outside the checkout; an empty registry is fail-closed.

The manually dispatched `Windows Launcher Trust Validation` workflow consumes
the protected `COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER` and
`COPPERFIN_LAUNCHER_TRUST_SIGNING_KEY_PEM` secrets plus an explicit non-secret
approved signer-key ID. It materializes protected inputs only under the
runner's temporary directory, proves that the signer ID is present in the
registry, rejects record-count mismatches and duplicate signer IDs, configures
the enforced native guard, and removes those inputs even when validation
fails. The native verifier independently rejects multiple registry entries for
the selected signer before signature acceptance. The workflow deliberately has
no push or pull-request
trigger, so ordinary development and unsigned installer validation remain
unchanged.

### Protected Release Environment

The signing/guard job is bound to the fixed GitHub Actions environment
`release`; the environment name is not a dispatch input or expression. Before
any protected run, a repository administrator must:

1. create the `release` environment and configure one or more required
   reviewers;
2. enable prevention of self-review so the dispatcher cannot approve the same
   run;
3. restrict deployment branches to `main`;
4. create environment-scoped
   `COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER` and
   `COPPERFIN_LAUNCHER_TRUST_SIGNING_KEY_PEM` secrets; and
5. confirm the same secret names are not being supplied as repository-level
   substitutes for the release ceremony.

An environment that is absent, has no required reviewer, permits self-review,
or admits branches other than `main` is not protected release evidence. The
job must remain pending until the configured reviewer approves it, and secret
material must not become available to the runner before that approval. The
repository intentionally does not create the environment, select its reviewer,
or provision its secrets in source control; those are external #4409 release
authority.

The protected job derives a canonical `app.cftrust` from an exact finalized
fixture inventory, signs it with the external key reference through the native
PowerShell signer, and invokes the actual enforced launcher guard. A valid
signed package must start the internal apphost. Modified and removed artifacts,
removed/duplicate/case-ambiguous inventory records, and modified or removed
signature sidecars must all return exit code `4` without starting that apphost.
Only the signer ID, commit/run identity, finalized direct artifact names/roles
and SHA-256 digests, invariant case results, and non-secret provisioning facts
are uploaded. This workflow path is implemented under
#4894 and its fixed release-environment binding is enforced under #4895, but it
is release evidence only after an externally approved registry and key execute
it successfully through the configured environment approval.

## Platform Policy

For an enforced release build, Windows generated-launcher packages require both trust sidecars and reject unsigned or unknown-signer inventories before managed apphost startup. The guard retains the existing containment, regular-file, physical-identity, and SHA-256 checks after signature verification. Until an approved release signer and registry are provisioned, ordinary development packages intentionally use the unsigned fallback and must not be presented as meeting the Windows release trust boundary.

POSIX and macOS do not claim this Windows trust boundary yet. Until platform-specific release signing and verification are approved, they retain the current unsigned inventory behavior and report the trust capability as unsupported rather than treating a recomputed unsigned inventory as authenticated.

## Fixture Evidence

`test_package_launcher_inventory_trust` covers canonical ordering, traversal
rejection, unknown signer IDs, invalid signatures, strict textual
signature-sidecar parsing and signer matching, and an RFC 8032 Ed25519 public
verification vector. The Windows `test_generated_launcher_process` regression
also proves malformed trust sidecars fail before managed startup. The #4894
fixture and protected orchestrator cover the external signer-to-guard path and
record whether the internal apphost started for every case. No private or
machine-specific key is embedded. The protected workflow must still run with
the approved registry/key pair before the release trust boundary is claimed.

Implementation evidence is current at head `f0c9e06e2`. Windows native run
`30559930672` built `test_windows_launcher_trust_fixture` with MSVC and passed
`315/315`; Linux `30559930560` and macOS `30559930719` passed `316/316`.
Generated-launcher run `30559417230` passed at the exact duplicate-signer head
`3968fabff`, and independent review confirmed the signer, registry, evidence,
and cleanup boundaries. Permissive safety run `30559107092` produced artifact
`8766084663`. These results validate the implementation contract only; the
approved protected execution required by #4409 remains outstanding.
