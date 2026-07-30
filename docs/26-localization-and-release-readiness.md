# Localization And Release Readiness

Copperfin localization is a release-readiness track for shipping locale-ready builds without changing machine-readable contracts. The first portable C++ locales are:

- `en-US`: source-of-truth/default English fallback.
- `es-419`: Latin American Spanish placeholder catalog; not production-ready until separately reviewed.
- `pt-BR`: Brazilian Portuguese placeholder catalog; not production-ready until separately reviewed.
- `qps-ploc`: pseudo-locale for testing string routing, expansion, Unicode handling, and hard-coded text assumptions.

See [localization.md](localization.md) for the portable C++ catalog contract.

## Contract

Localized text must be separated from stable protocol values:

- UI labels, menu text, dialog titles, status messages, release notes, installer text, and human diagnostics are localizable.
- JSON property names, enum-like values, command-line switches, exit codes, log field names, issue IDs, telemetry field names, and generated artifact IDs are not localizable.
- Error handling must keep machine-stable codes or keys available even when the human message is localized.
- Missing locale catalogs fall back to English.
- Missing keys fall back to the stable key instead of returning blank text.
- Locale selection must be deterministic and testable. Portable C++ surfaces use `--locale <tag>` where practical and honor `COPPERFIN_LOCALE`; existing .NET Studio surfaces retain their documented host-specific locale selection until migrated.

## Current Validation Evidence

The latest broad implementation baseline is product head `93d44395f`; the
latest product implementation head is `531bbec70`, while subsequent
documentation-only coordination commits may advance the branch. Hosted Linux
Native `30329037575` and
macOS Native `30329037567` passed their native
matrices, Linux managed UI `30329037628`, VSIX `30329037609`, installers
`30329037566`, and the security supply-chain gate `30329037556` passed, and
Windows Native `30329037587` passed `315/315` CTest cases. Windows Deep
Validation `30329053296` passed native CTest `315/315`, VSIX and managed
checks, the net472 process-runner fixture, standalone Studio/designer builds,
designer smoke assertions, and PRG debugger smoke. That deep run deliberately
used `require_vfp9_samples=false`, so it does not claim the sample-dependent
RuntimePackage, xAsset, Report, or Menu stages.

The hosted Windows reconciliation recorded under closed issue #4621 supplies
the remaining accepted evidence at its validated heads: installed VFP9
discovery, real/copy `invoice.frx` editing and reload, report/label workflows,
clean VSIX lifecycle and SHA-256 identity, standalone Studio, and hosted
Visual Studio Command and Terminal pane observations. Windows Codex also ran
the four installed-VFP9 sample-dependent equivalents twice at implementation
head `93d44395f`; the artifacts are recorded under
`E:\Project-Copperfin\artifacts\windows-mounted-vfp9-validation\`. These
results close the hosted Windows/VFP9 evidence issue, but do not close safety
traceability or protected package signing.

The current focused work-area boundary correction is implemented at product
head `ca7889efa` under #4861. Linux Native (`30511406972`) and macOS Native
(`30511406973`) passed the exact corrected source/test head; the focused
regression fills all 32,767 bounded work areas and verifies that failed
allocation preserves the prior selection and alias map. The corresponding
Windows Native workflow (`30511406940`) passed `315/315` CTest cases, including
the focused work-area target in `79.23s` under the per-test `180s` timeout.
Issue #4861 is closed as implementation-complete. The MVP RC gate remains open
for the separately tracked safety, protected signing, and remaining hosted
product evidence; the product head is distinct from documentation-only
coordination commits.

Subsequent invariant-machine-metadata corrections preserve the localization
boundary. #4862 is closed after Linux, macOS/AppleClang, and Windows/MSVC each
passed the transaction-journal target under default, `pt_BR.UTF-8`, and
`de_DE.UTF-8`. #4863 at product head `f4e480ea4` adds no user-facing text or
catalog keys: portable cursor XML attribute names and runtime event categories
remain invariant, malformed field widths/decimal counts use the existing
localized parse-warning path, and the focused runtime-surface target passes
the same three locale environments on Linux, macOS/AppleClang, and
Windows/MSVC. Issue #4863 is closed; this focused acceptance is not broad
release evidence.

#4864 at product head `d21415cbf` likewise adds no user-facing text or catalog
keys. It treats the opaque runtime object-reference spelling as invariant
machine identity and rejects malformed numeric suffixes before classification
or lookup, while preserving existing programmatic identifiers and localized
reflection fallback behavior. The focused runtime-surface target passes under
default, `pt_BR.UTF-8`, and `de_DE.UTF-8` on Linux, macOS/AppleClang, and
Windows/MSVC. Issue #4864 is closed.

#4865 at product head `6b226ba33` adds no user-facing text or catalog keys.
Windows-message handler strings remain invariant host-return data and now
require a complete signed pointer-sized integer before numeric coercion. The
focused runtime-surface target passes under default, `pt_BR.UTF-8`, and
`de_DE.UTF-8` on Linux, macOS/AppleClang, and Windows/MSVC. Issue #4865 is
closed; exact-head hosted native evidence remains separately nonterminal.

#4866 at product head `416e8d123` adds no user-facing text or catalog keys.
DBF Currency (`Y`) input remains invariant machine/storage text, and boundary
rejection uses the existing localized invalid-currency diagnostic. The focused
DBF target passes under default, `pt_BR.UTF-8`, and `de_DE.UTF-8` on Linux,
macOS/AppleClang, and Windows/MSVC. Issue #4866 is closed; exact-head hosted
evidence remains separately nonterminal.

#4867 at product head `a2a64427d` adds no user-facing text or catalog keys.
FRX/LBX numeric metadata remains invariant storage text; malformed values stay
visible through existing raw provenance without becoming localized display
fields or fabricated layout state. Both focused layout/classification targets
pass under default, `pt_BR.UTF-8`, and `de_DE.UTF-8` on Linux and
macOS/AppleClang and Windows/MSVC. Issue #4867 is closed; exact-head hosted
release evidence remains separate.

#4868 at product head `858e56929` adds no user-facing text or catalog keys.
Generated DLL/OCX and FLL API manifest arities remain invariant machine
metadata and now explicitly use the classic locale. The focused
runtime-pipeline target passes under default, `pt_BR.UTF-8`, and `de_DE.UTF-8`
on Linux, macOS/AppleClang, and Windows/MSVC. Issue #4868 is closed; exact-head
hosted release evidence remains separate.

#4869 at product head `c86be275f` also adds no user-facing text or catalog
keys. Generated wrapper source lines and parameter counts are invariant C++
numeric literals, now explicitly isolated from the host locale. The focused
runtime-pipeline target passes under default, `pt_BR.UTF-8`, and
`de_DE.UTF-8` on Linux, macOS/AppleClang, and Windows/MSVC. Issue #4869 is
closed; exact-head hosted release evidence remains separate.

#4870 at product head `531bbec70` adds no user-facing text or catalog keys.
DIF `VECTORS`/`TUPLES` dimensions are invariant interchange metadata and now
explicitly use the classic locale. The focused data-I/O target passes under
default, `pt_BR.UTF-8`, and `de_DE.UTF-8` on Linux and Windows/MSVC;
AppleClang acceptance and exact-head hosted evidence remain pending.

#4871 at product head `a55a35868` adds no user-facing text or catalog keys.
APP archive byte payloads are invariant lowercase hexadecimal machine data and
now explicitly use the classic locale. The focused runtime-pipeline target
passes under default, `pt_BR.UTF-8`, and `de_DE.UTF-8` on Linux; AppleClang/MSVC
acceptance and exact-head hosted evidence remain pending.

Subsequent focused runtime/package slices are independently tracked rather
than silently folded into the broad baseline. #4750 is closed after its
Windows path/package contract validation. #4757-#4769 are closed with their
focused implementation and hosted evidence. The current product correction
head is `f9f44a786`: #4770 now routes both strict table snapshot materialization
and cursor-order inspection through the ambiguity-safe database-byte helper,
and #4771 includes disabled-button filtering plus focused CommandButton ENTER
precedence when a host supplies a target handle. The three focused Linux
targets pass `3/3`; exact-head Windows validation seq778 closed #4770 after
passing those targets `3/3`, including the prior folded-only CDX failure.
Claude and Windows reviewed #4771, and exact-head macOS Native Validation
`30353802078` passed at `f9f44a786`; the child is closed as implementation-
complete. Its full RC matrix and UI rendering boundaries remain separate.
The #4772 strict PRG source-admission correction is closed at product head
`71ac05cac`; it also restores fail-closed behavior for unadmitted includes
when same-named disk files exist. Its focused local PRG, dynamic-xAsset
security, and runtime-surface targets pass `3/3`; Claude's independent repro
and exact-head native Windows validation seq785 corroborate the correction;
macOS Native Validation `30356725909` also passed the full native matrix at
the exact corrected product commit. Remaining release-matrix evidence is
separate.
Runtime child #4773 under #3217 is implemented at product head `d2c6d525e`:
default forward Tab traversal now uses deduplicated `TabIndex` order, skips
non-tab-stop/invisible/disabled direct controls, wraps, preserves the existing
focus transition semantics, and honors `KeyPress NODEFAULT`. The focused
runtime-surface CTest passes `1/1` on Linux; Claude review seq790 and exact-head
Windows validation seq789 also pass, so #4773 is implementation-complete.
The remaining macOS/full-RC and keyboard/input/UI boundaries are separate
release evidence.
Runtime child #4774 is also closed as implementation-complete at product head
`29391e3d2`: default forward Tab traversal now descends through nested
`Container` descendants iteratively, propagates ancestor visibility/enabled
state, and preserves the existing ordering and focus contracts. Local CTest,
Claude seq795, and Windows seq794 pass; PageFrame/OptionGroup/CommandGroup,
reverse Shift+Tab, and full macOS/RC validation remain separate evidence.
The focused localization test also passes `1/1` locally across the four
catalogs. The strict safety traceability gate remains open only because #4403
is not closed and requires genuinely arm's-length reviewer sign-off; #4409's
protected external package-signing registry remains a separate release gate.
Runtime child #4775 is implementation-complete at product head `bdd033690`:
default forward Tab traversal now descends through only the active native
`PageFrame` page, excludes inactive pages and `Page` candidates, and preserves
ancestor visibility/enabled filtering, TabStop filtering, deterministic order,
and focus transitions. Local focused CTest, Claude seq797, and exact-head
Windows validation seq799 pass. The macOS job for the broader exact-head
matrix remains in progress and is not claimed as slice evidence.
Runtime child #4776 is closed at product head `73408bfff`; CommandGroup
children use group-parent plus child-local TabIndex ordering and preserve the
existing filtering and focus contracts. Runtime child #4778 is closed at
product head `b642f81c5`; Container and active PageFrame parent TabIndex values
now prefix descendant sort paths, with the sibling-bypass regression covered.
Linux, Claude, and exact-head Windows focused validation pass for both slices.
Runtime child #4777 is implemented at product head `5b43e7a26`: OptionGroup
is one parent-level Tab stop and Windows-compatible arrow selection updates
eligible child buttons and group/child Value state. Its focused Linux test
passes, with Claude and Windows focused behavior corroborated; #4777 is
implementation-complete. Full current-head Windows/macOS workflows remain
separate RC evidence. None of these focused slices closes the broader RC
matrix, safety gate, signing registry, or hosted pixel-UI evidence.

The latest focused compatibility slices are recorded at implementation head
`30ff4ec38`. Runtime children #4793 and #4794 add indexed `PROGRAM(nLevel)` /
`SYS(16,nLevel)` stack lookup and VFP procedure-context formatting; Windows
seq861 and the focused Linux target pass, and both issues are closed as
implementation-complete. Managed language-service child #4795 recognizes
`LPARAMETER` alongside `LPARAMETERS` and `PARAMETERS` while rejecting bare
`PARAMETER`; its focused Release suite and independent Claude validation seq866
pass, and the issue is closed. These slices preserve invariant machine
contracts and do not replace the broader RC matrix.

Local CPack at `c95cf269d` generated the Linux DEB, RPM, and TGZ artifacts, and
the package/document/install contract subset passed `4/4`. Current-head
installer workflow `30317113970` passed its Linux, Windows, and macOS jobs and
uploaded the expected artifacts, including all four locale catalogs in the
install trees. Installer resources and package names remain machine-stable
across locales; protected signing evidence remains separate.

## Resource Layout

The portable C++ catalog lives under `resources/locales/<locale>/strings.json` and installs to `share/copperfin/locales`. The first C++ surface routed through it is `copperfin_inspect` usage text; machine-readable inspection fields remain invariant.

The .NET UI catalog is compiled into `vsix/Copperfin.VisualStudio/CopperfinLocalization.cs` for fallback and synchronized into the shared `resources/locales/<locale>/strings.json` catalogs by `tools/Copperfin.LocalizationCatalogGenerator`. Standalone Studio loads the installed shared catalogs first and falls back to the compiled catalog when a file is unavailable or a key is absent. `COPPERFIN_UI_LOCALE` takes precedence over `COPPERFIN_LOCALE`; `--locale` takes precedence over both in standalone Studio; and `COPPERFIN_LOCALE_DIR` overrides catalog discovery. The managed catalog parity check runs from `tests/run_managed_compile_check.cmake`, rejects duplicate keys in each managed locale section, and prevents new managed keys from silently bypassing the shared resource set. The catalog covers the standalone Studio shell strings, catalog-backed asset-kind display labels for project, form, class library, report, label, menu, and generic assets, plus the embedded VSIX asset editor title/subtitle/guidance/Open/Reveal/Refresh chrome, host-mode subtitles, project workspace tab labels, Hide project records object-browser option, project command buttons, debugger controls, initial status/guidance strings, project workspace placeholder pane text, explorer list column headers, static asset-family guidance text, snapshot unavailable/loaded status text, undo labels and status text, undo-available suffixes, property update status text, static launch/workflow dialog text, and native `--license-status` parser/classifier diagnostics. It is linked into:

Native runtime hosts pass their selected catalog into `PrgRuntimeSession`, so session messages, runtime faults, watch diagnostics, and step/entry status text use the explicit host locale. Direct library/test sessions retain the legacy `COPPERFIN_LOCALE` fallback, and a scoped thread-local context restores nested sessions without changing machine-readable fields.

- `vsix/Copperfin.Studio`: standalone Studio shell.
- `vsix/Copperfin.DesignerSmokeTests`: UI smoke test project.
- `vsix/Copperfin.LanguageServiceTests`: portable catalog tests.

Future surfaces should reuse an existing catalog where practical or add equivalent resource catalogs with the same locale normalization and fallback rules:

- CLI/native diagnostics: keep command switches and JSON fields stable; localize human summaries behind locale-aware lookup.
- Studio host JSON: keep JSON contracts stable; localize optional display text only when a consumer requests it.
- VSIX UI: continue moving dialogs, deeper pane body text, and dynamic project/debugger detail text into shared resources while preserving VS command IDs.
- Installer text: ship en-US, es-419, pt-BR, and qps-ploc resources with packaging smoke tests; every Windows, macOS, and Linux installer job must materialize an install tree and verify each catalog is present, non-empty, and a regular file.
- Docs/help and generated templates: version translated text separately from generated file identifiers and code symbols.

## Installer Artifact Contract

Each platform installer job creates a fresh `build/package` directory, runs CPack with the platform's expected generator set, and removes CPack's internal `_CPack_Packages` staging tree before verification. CMake generates `build/CopperfinPackageVersion.txt` from the project version, and `tests/run_package_version_contract_check.cmake` validates that version against the VSIX manifests and Visual Studio package registration. `tests/run_cpack_artifact_contract_check.cmake` then derives the exact current package filenames from that generated version, rejects directories, symlinks, empty files, unexpected files, and missing files, and only after that allows artifact upload. Upload steps use version-independent package-name patterns with `if-no-files-found: error`; the preceding exact verifier prevents stale or extra artifacts from satisfying release publication.

The package filenames are machine-readable release identifiers and remain invariant across locales: `copperfin-<project-version>-Windows.exe`, `copperfin-<project-version>-Windows.zip`, `copperfin-<project-version>-Darwin.pkg`, `copperfin-<project-version>-Darwin.tar.gz`, `copperfin-<project-version>-Linux.deb`, `copperfin-<project-version>-Linux.rpm`, and `copperfin-<project-version>-Linux.tar.gz`. The native workflow contract test and package-version contract must remain green whenever these paths or the package workflow changes.

## GitHub Actions Dependency Procedure

Treat every external action reference as executable supply-chain code. When updating one, resolve the intended release tag to its reviewed full 40-character commit SHA, retain the human-readable release tag in an inline comment, and review the upstream release notes and diff before merging. Run the repository-wide GitHub Actions contract test plus the affected workflow contract and hosted workflow. Keep local reusable actions on relative paths, keep workflow `GITHUB_TOKEN` permissions explicit and read-only unless a documented step demonstrably requires more, and update the pin and comment together when a dependency is intentionally advanced.

## Release Checklist

Before a localized production release:

- Execute and archive the [MVP runtime recovery walkthrough](20-runtime-build-and-debug-pipeline.md#mvp-recovery-walkthrough) alongside the platform validation and safety-traceability evidence; the walkthrough must keep localized prose separate from invariant runtime/debug contracts.
- Run `test_localization` through CTest. Its catalog gate proves English fallback, Spanish lookup, Portuguese lookup, catalog key parity, nonblank localized values, exact placeholder parity with `en-US`, and pseudo-locale expansion with replacement values preserved.
- Smoke the standalone Studio shell with `--locale es-419` and `--locale pt-BR`, plus an installed `COPPERFIN_LOCALE_DIR` override and pseudo-locale fallback.
- Smoke VSIX and installer packaging with localized resources included.
- Review Spanish and Portuguese terminology for consistency across Studio, VSIX, CLI diagnostics, docs/help, and templates.
- Verify screenshots or UI smoke captures for clipped text in Spanish and Portuguese.
- Verify JSON and CLI machine-readable contracts are unchanged across locales.
- Keep the managed catalog generator and `qps-ploc` catalog check in the routine compile gate; pseudo-localized values must remain nonblank and decorated while production locale entries retain exact managed text.
- Record translation source, reviewer, locale, and release version in the release checklist.

The automated gate is necessary but not sufficient for a production language pack. `es-419` and `pt-BR` remain placeholder/review-pending catalogs until a qualified reviewer records accuracy, regional usage, formality, euphemism/directness, terminology consistency, and the absence of awkward or offensive wording. `qps-ploc` is test-only and must never be presented as a production language option.
