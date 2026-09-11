# Access `SaveAsText` Export Helper

A PowerShell reference implementation of the automation-helper half of
#5477/#5478's documented architecture (`docs/78-access-forms-reports-
vba-storage-reconnaissance.md`'s "Implications for implementation
architecture" section): it drives Microsoft Access via COM automation
to export every Form, Report, and standalone Module in a database as
`Application.SaveAsText` output, which
`copperfin::vfp::parse_access_saveastext_design()`
(`src/vfp/access_saveastext_design.cpp`, #5477) then parses into a
structured control-hierarchy tree.

## Status

**Not wired into Copperfin's own runtime or any `IMPORT DATABASE`
command surface.** This is a tested, working reference script, not a
finished, security-admitted component. Copperfin's own existing
external-process invocations (e.g. `samples/polyglot-python-sidecar/`)
go through an explicit admission model -- a pinned SHA-256 digest, an
admitted physical root, a fixed command-line argument position,
revalidation before each launch, and a complete explicit child
environment instead of ambient host/agent variables. Before this script
(or a C++ equivalent of it) is actually invoked from Copperfin's own
code, it needs the same treatment; that work has not been done here.

Verified end-to-end (2026-09-11) against a real fixture on a licensed
Access 365 installation (`copperfin-access365-win11`, see
`project_copperfin_windows_vm` memory): exported all 21 objects (15
forms, 6 reports, 1 standalone module) from a real Jet3 `.mdb`, and
`parse_access_saveastext_design()` parsed every one of the resulting 20
form/report text files successfully.

## Requirements

- Windows with a licensed Microsoft Access installation (COM automation
  requires the real product; there is no documented headless/CLI
  equivalent).
- PowerShell.

## Usage

```powershell
powershell -ExecutionPolicy Bypass -File export_access_design.ps1 `
    -DatabasePath 'C:\path\to\database.mdb' `
    -OutputDirectory 'C:\path\to\output'
```

Writes one `<Kind>_<Name>.txt` file per exported object (`Form_`/
`Report_`/`Module_` prefix) plus a `manifest.json` listing every
attempted export and whether it succeeded. Exits non-zero if any object
failed to export (the manifest still lists every attempt either way, so
a caller can decide whether a partial result is acceptable).

## Security

Sets `Application.AutomationSecurity =
msoAutomationSecurityForceDisable` (3) **before** opening the database.
`OpenCurrentDatabase()` against a database containing an `AutoExec`
macro, a startup form, or other active content can otherwise execute
that content under the importing user's own authority before
`SaveAsText` ever runs -- turning a read-only inspection tool into
arbitrary code execution for an untrusted input file, and conflicting
with `docs/04-security-model.md`'s Runtime Boundary (which explicitly
protects both "COM/interop access" and "macro/eval execution"). This
mitigation was directly re-verified to not change `SaveAsText`'s own
output (byte-for-byte identical export with and without the property
set) -- see `docs/78`'s own security section for the evidence.

## What this does not do

- Does not decode the exported text itself -- that is
  `access_saveastext_design.cpp`'s job.
- Does not handle macros, queries, or any object kind other than
  Forms/Reports/Modules -- out of this script's and #5477/#5478's own
  scope.
- Does not attempt ACE/`.accdb` sources any differently from Jet/`.mdb`
  -- both should work identically via the same COM automation surface,
  but this was only directly verified against a Jet3 `.mdb` fixture.
