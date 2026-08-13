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
| `RQ-CF-REL-002`; `DQ-windows-installer-lifecycle-scope` — lifecycle evidence must be direct, bounded, exact-artifact-bound, and must not overstate same-version maintenance as upgrade | `DV-windows-installer-lifecycle-contract`; assembler self-test; exact-head hosted Windows execution pending | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01` |

Reverse traceability is carried by `.github/workflows/build-installers.yml`,
`scripts/test-windows-installer-lifecycle.ps1`,
`scripts/assemble-rc-candidate.py`, the schema-v3 contract, and the durable
matrix row.

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
  before the Copperfin installer runs.

Potential severity is **high** because false lifecycle evidence could admit an
installer that cannot be safely deployed or removed. No user project, legacy
asset, profile, or customer data is placed in the test root.

## Verification And Residual Gaps

Local portable verification covers workflow/script structure, timeout and
root boundaries, honest upgrade classification, retained result shape, and RC
assembler ingestion. Direct Windows execution and independent review remain
pending, so `RQ-CF-REL-002` remains `gap`.

macOS productbuild, Linux DEB/RPM, previous-version Windows upgrade, VSIX
lifecycle, human UI smoke, and platform signing are outside this slice and
remain separately disclosed.

## Rollback And Field Notification

Before a new candidate, revert the lifecycle workflow, script, schema-v3
producer, and documentation together. After a candidate uses the contract, do
not rewrite its tag or artifact: withdraw the candidate, disclose the failed
operation and affected installer digest, correct the verifier or package with
regression coverage, and issue the next sequential immutable RC.
