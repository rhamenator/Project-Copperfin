# Project Copperfin RC1 Evaluation Guide

Project Copperfin `v0.1.0-rc.1` is a private-evaluation release candidate. It
is not an official Project Copperfin release and is not distributed through a
published GitHub Release. The bundle is produced by a manually dispatched,
fail-closed GitHub Actions workflow from the exact tagged commit.

## Verify Before Testing

1. Download the single `copperfin-v0.1.0-rc.1-evaluation-bundle` workflow
   artifact while signed in to GitHub.
2. Extract it into a new directory.
3. Compare the tag and 40-character revision in
   `rc-validation-manifest.json` with the workflow page.
4. Verify every entry in `SHA256SUMS.txt` before opening or installing a
   payload. On Windows, use `Get-FileHash -Algorithm SHA256`; on macOS or
   Linux, use `shasum -a 256` or `sha256sum`.
5. Retain the artifact name and digest with the test report. GitHub workflow
   artifacts expire; this bundle requests 90-day retention, which remains
   subject to repository and GitHub retention policy.

## Payloads

- `installers/windows/` contains the NSIS installer and portable ZIP.
- `installers/macos/` contains the productbuild package and portable TGZ.
- `installers/linux/` contains DEB, RPM, and portable TGZ packages.
- `ide/visual-studio/` contains the Visual Studio VSIX.
- `source/` contains the exact Corresponding Source archive for the tagged
  revision.
- `sbom/` contains the CycloneDX software bill of materials.
- `licensing/` contains the GPLv3 license, Copperfin exception, plain-language
  explanation, Corresponding Source contract, release metadata, and
  third-party notices.

Choose only the installer or archive appropriate to the test operating system.
Use disposable or backed-up test environments for legacy applications and
data. Do not submit customer, employer, proprietary, personal, credential, or
other restricted material to Project Copperfin or to a public issue.

## Signing And Review Limits

This evaluation workflow does not claim completion of the protected Windows
launcher release-trust approval. Unless that separate protected process has
been completed and directly evidenced, treat generated Windows launchers as
evaluation output rather than approved release-trusted inventory. The Windows
installer also does not claim Authenticode signing. Project Copperfin does not
currently support Apple platform signing/notarization or Linux distribution
package signing.

English is the authoritative reviewed documentation and UI source for RC1.
Shipped non-English and pseudo-localized catalogs remain subject to the
documented machine-translation and human-review limits in
`docs/26-localization-and-release-readiness.md` from the Corresponding Source.

## What To Exercise

Prioritize real Visual FoxPro projects and representative copies of their
assets: project open/build/run/debug flows, PRG compatibility, reports and
labels, menus, xAssets, runtime packages, database/index behavior, Visual
Studio integration, standalone Studio workflows, diagnostics, and migration
outputs. Keep originals and test copies separate.

## Report A Finding

A useful report identifies:

- RC tag and exact commit;
- operating system and version;
- artifact filename and SHA-256 digest;
- concise reproduction steps;
- expected and actual behavior;
- whether customer or proprietary material was involved, without attaching
  that material; and
- logs or a minimal non-confidential reproducer when safe.

Security vulnerabilities and suspected secret exposure must follow
`SECURITY.md`; do not place them in a public issue. Ordinary findings may be
reported through the repository's structured issue forms after removing
restricted content.
