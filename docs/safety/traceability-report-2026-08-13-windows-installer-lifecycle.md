# Windows Installer Lifecycle Traceability Report

## Scope And Assurance Boundary

This report covers `RQ-CF-REL-002`: direct Windows NSIS fresh-install,
installed-command, maintenance-reinstall, uninstall, and residue evidence. It
uses Copperfin's DO-178C-inspired general-purpose quality baseline; it is not a
claim of DO-178C compliance, certification, an assigned software level, or
suitability for a safety-critical deployment.

## Requirement And Verification Map

| Requirement | Verification | Controlled hazards |
| --- | --- | --- |
| `RQ-CF-REL-002`; `DQ-windows-installer-lifecycle-scope` — lifecycle evidence must be direct, bounded, exact-artifact-bound, and must not overstate same-version maintenance as upgrade | `DV-windows-installer-lifecycle-contract`; assembler self-test; corrected exact-head hosted Windows execution pending | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01` |

Reverse traceability is carried by `.github/workflows/build-installers.yml`,
`scripts/test-windows-installer-lifecycle.ps1`,
`scripts/assemble-rc-candidate.py`, the schema-v3 contract, and the durable
matrix row. The RC evaluation guide and focused lifecycle contract also carry
the governing requirement, derived requirement, verification, and hazard
identifiers; the focused check enforces those reverse links.

## Hazard, Misuse, And Boundary Analysis

- A package that merely builds can still fail to install, omit required files,
  fail when launched from its installed layout, or leave broken residue.
- The lifecycle root is a unique, explicit path under the hosted runner's
  temporary directory. The script refuses a pre-existing root and does not
  recursively delete residue, preventing a false clean result and limiting the
  destructive boundary.
- Installer, installed-command, and uninstaller child processes have bounded
  waits and timed-out process-tree termination.
- Installed Studio and locale contracts verify the package layout; the
  installed inspection CLI proves executable/catalog resolution from that
  layout.
- The maintenance pass repeats the same installer and requires identical file
  paths and hashes plus exactly one matching uninstall registration. It is not
  a previous-version upgrade and is machine-reported separately as `NOT_RUN`.
- Silent uninstall must remove the installation root and the registration whose
  install location exactly matches that root. The verifier does not alter other
  installations or registrations.
- Registry inventory is heterogeneous: unrelated uninstall entries may omit
  `InstallLocation`. Optional values are read through the PowerShell property
  collection so strict mode treats absence as a non-match rather than aborting
  before the Copperfin installer runs. The caller also array-wraps the complete
  query before counting, so zero matches remain the integer `0` rather than a
  strict-mode `$null.Count` failure.
- CPack NSIS records the quoted exact uninstaller path but does not require an
  `InstallLocation` value in its uninstall entry. Registration matching accepts
  either an exact normalized install location or the exact normalized
  `<root>\Uninstall.exe` path, and rejects command arguments or sibling paths.
- Registry enumeration skips only an absent uninstall base. Existing bases and
  child keys are read with terminating errors, so an unavailable hive or entry
  cannot be misreported as zero residue. The configured CPack uninstall-key
  identity is generated into the build contract and passed explicitly to the
  verifier; an exact key remains residue even when all of its values were
  cleared.

Potential severity is **high** because false lifecycle evidence could admit an
installer that cannot be safely deployed or removed. No user project, legacy
asset, profile, or customer data is placed in the test root.

## Verification And Residual Gaps

Local portable verification covers workflow/script structure, timeout and
root boundaries, honest upgrade classification, retained result shape, and RC
assembler ingestion. Exact-head hosted run `31698722081` passed the Windows
fresh-install, installed-tree/catalog, installed-CLI, same-version reinstall,
silent-uninstall, and filesystem/registry-residue stages. The retained result
records one uninstall registration after install and 21 installed files. Its
installer SHA-256, `832cc4cb5b3c5f2df8d260103545e58c6847d3f729718c687a446ac76b48afbe`,
was independently recomputed from the downloaded NSIS executable and matches
the JSON evidence. Artifact `9180809446` has GitHub digest
`sha256:1c0344a9555b73d5bc8d5a89eabd36860658fbb2a63a7e107af152c2792cc153`
and expires 2026-11-11. Independent review then found that registry read errors
could be suppressed and a leftover exact CPack key with cleared values could
escape the old filter. That run is retained as discovery evidence but does not
verify the corrected implementation. `RQ-CF-REL-002` therefore remains `gap`
until the corrected exact-head hosted lifecycle passes and is retained.

macOS productbuild, Linux DEB/RPM, previous-version Windows upgrade, VSIX
lifecycle, human UI smoke, and platform signing are outside this slice and
remain separately disclosed.

## Rollback And Field Notification

Before a new candidate, revert the lifecycle workflow, script, schema-v3
producer, and documentation together. After a candidate uses the contract, do
not rewrite its tag or artifact: withdraw the candidate, disclose the failed
operation and affected installer digest, correct the verifier or package with
regression coverage, and issue the next sequential immutable RC.
