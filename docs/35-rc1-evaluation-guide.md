# Project Copperfin RC Evaluation Guide

Traceability: `RQ-CF-REL-001`, `DQ-rc-evidence-v2-scope-separation`,
`DV-rc-evidence-v2-assembly-self-test`, `DV-rc-evidence-v2-schema-validation`,
`DV-rc-evidence-v2-workflow-contract`, `HZ-system-failure-01`, and
`HZ-doc-command-01` govern the release-evidence distinctions and checks in
this guide. `RQ-CF-REL-002`, `DQ-windows-installer-lifecycle-scope`,
`DV-windows-installer-lifecycle-contract`, and `HZ-data-corruption-01` govern
the direct Windows installer lifecycle evidence and its fail-closed boundaries.
`RQ-CF-REL-003`, `DQ-windows-vsix-lifecycle-scope`, and
`DV-windows-vsix-lifecycle-contract` govern the corresponding Windows VSIX
lifecycle boundary.

Each Project Copperfin `v0.1.0-rc.N` candidate is a private-evaluation release
candidate. It is not an official Project Copperfin release and is not
distributed through a published GitHub Release. The bundle is produced by a
manually dispatched, fail-closed GitHub Actions workflow from the exact tagged
commit. A failed or superseded RC tag remains immutable; corrections use the
next sequential RC number.

## Verify Before Testing

1. Download the single `copperfin-<candidate-tag>-evaluation-bundle` workflow
   artifact while signed in to GitHub. For example, RC2 uses
   `copperfin-v0.1.0-rc.2-evaluation-bundle`.
2. Extract it into a new directory.
3. Compare the tag and 40-character revision in
   `rc-validation-manifest.json` with the workflow page.
   Validate that manifest against the bundled
   `rc-validation-manifest.schema.json`. Schema version 3 separates package
   construction/static checks from each platform's actual installer and VSIX
   lifecycle execution. The Windows NSIS fields distinguish fresh install,
   installed-command smoke, same-version maintenance reinstall, prior-version
   upgrade, silent uninstall, and residue checks. `NOT_RUN` means the bundle
   workflow did not perform that exact operation; it must not be read as a
   failure or a pass.
   The signing, linguistic-review, and real-installed-VFP9 fields use the same
   vocabulary and describe only this exact candidate workflow. Evidence from a
   separate protected or human run must not be inferred into this manifest.
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
- `evidence/windows-installer-lifecycle.json` records the Windows NSIS
  lifecycle result and binds it to the installer's SHA-256 digest. A successful
  maintenance reinstall is not evidence of upgrade from an older version.
- `evidence/windows-vsix-lifecycle.json` records exact-instance VSIX install,
  identity/version, package load, runner-owned PRG/command smoke, uninstall,
  and residue results bound to the VSIX SHA-256. It separately reports
  same-version reinstall, previous-version upgrade, and disablement as
  `NOT_RUN` until those operations are directly exercised.
- `rc-validation-manifest.schema.json` defines the invariant status vocabulary
  and exact machine-readable validation fields used by the candidate manifest.
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

English is the authoritative reviewed documentation and UI source for the RC.
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
