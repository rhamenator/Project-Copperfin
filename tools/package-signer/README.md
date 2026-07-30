# Package Signer

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
