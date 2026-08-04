# Package Signer

Package and launcher signatures authenticate release provenance and integrity.
They are independent of the archived product-license system and do not create
activation or entitlement rights.

## Generate A Dedicated Launcher-Release Identity On Linux

Do not reuse any archived product-license signing key. Generate one dedicated
launcher-release Ed25519 identity in a protected directory outside this
checkout:

```sh
tools/package-signer/generate-launcher-signing-key.sh \
  --key-id copperfin-launcher-rc1-2026 \
  --output-dir /home/rich/copperfin-launcher-release-keys
```

The output directory must be owned by the current user, grant no group or
other permissions, and live outside the repository. The command refuses
invalid signer IDs and existing output files. It writes:

- `copperfin-launcher-rc1-2026_launcher_private.pem`: PKCS#8 Ed25519 private
  key, mode `0600`. Store its complete contents only in the protected GitHub
  `release` environment secret
  `COPPERFIN_LAUNCHER_TRUST_SIGNING_KEY_PEM`.
- `copperfin-launcher-rc1-2026_launcher_registry.h`: matching single-key
  `kKnownLauncherInventoryTrustedKeys` registry. Store its complete contents
  in the `release` environment secret
  `COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER`.
- `copperfin-launcher-rc1-2026_launcher_public.pem`: non-secret public-key
  backup.
- `copperfin-launcher-rc1-2026_launcher_metadata.json`: non-secret signer ID,
  public-key fingerprint, and output-name record suitable for the release
  evidence ledger.

Back up the private PEM in a separate protected secret store before depending
on the identity. Never commit it, paste it into an issue or chat, or store it
as a repository-level GitHub secret. The `Windows Launcher Trust Validation`
workflow's `signer_key_id` input must exactly match the `--key-id` value. The
same generated files are used by the existing Windows PowerShell signer; no
Windows key-generation script or second key pair is required.

This identity authenticates the Windows generated-launcher inventory. It does
not Authenticode-sign Windows binaries and does not sign macOS or Linux
artifacts. Those RC artifact validations retain their existing platform
contracts.

`sign-launcher-inventory.sh` creates the detached `app.cftrust.sig` sidecar for a finalized canonical `app.cftrust` envelope. It uses OpenSSL's Ed25519 signer and accepts only a key reference to a PEM file outside the repository checkout.

`sign-launcher-inventory.ps1` provides the same fail-closed contract for the
protected Windows release workflow. Both implementations validate the
canonical envelope boundary and write the detached sidecar atomically without
printing private material.

```sh
tools/package-signer/sign-launcher-inventory.sh \
  --input app.cftrust \
  --output app.cftrust.sig \
  --key-ref /protected/release-keys/copperfin-windows-test_private.pem
```

The input envelope must already have been generated from the finalized package inventory. The tool does not generate or alter `app.cfmanifest`, `app.cfdebug`, or package files. It writes only the versioned UTF-8/LF detached-signature sidecar and uses an atomic replacement within the output directory.

Private keys must remain in a secret manager or protected location outside the checkout. The public key registry is configured separately through `COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER`; this tool does not embed, publish, or select trusted keys.

The repository workflow uses the fixed GitHub Actions `release` environment.
Configure required reviewers, prevent self-review, restrict it to `main`, and
store both launcher-trust secrets at environment scope before dispatch. Merely
creating an unprotected environment or repository-level secrets is not release
evidence.
