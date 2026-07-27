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

## Resource Layout

The portable C++ catalog lives under `resources/locales/<locale>/strings.json` and installs to `share/copperfin/locales`. The first C++ surface routed through it is `copperfin_inspect` usage text; machine-readable inspection fields remain invariant.

The .NET UI catalog is compiled into `vsix/Copperfin.VisualStudio/CopperfinLocalization.cs` for fallback and synchronized into the shared `resources/locales/<locale>/strings.json` catalogs by `tools/Copperfin.LocalizationCatalogGenerator`. Standalone Studio loads the installed shared catalogs first and falls back to the compiled catalog when a file is unavailable or a key is absent. `COPPERFIN_UI_LOCALE` takes precedence over `COPPERFIN_LOCALE`; `--locale` takes precedence over both in standalone Studio; and `COPPERFIN_LOCALE_DIR` overrides catalog discovery. The managed catalog parity check runs from `tests/run_managed_compile_check.cmake`, rejects duplicate keys in each managed locale section, and prevents new managed keys from silently bypassing the shared resource set. The catalog covers the standalone Studio shell strings, catalog-backed asset-kind display labels for project, form, class library, report, label, menu, and generic assets, plus the embedded VSIX asset editor title/subtitle/guidance/Open/Reveal/Refresh chrome, host-mode subtitles, project workspace tab labels, Hide project records object-browser option, project command buttons, debugger controls, initial status/guidance strings, project workspace placeholder pane text, explorer list column headers, static asset-family guidance text, snapshot unavailable/loaded status text, undo labels and status text, undo-available suffixes, property update status text, static launch/workflow dialog text, and native `--license-status` parser/classifier diagnostics. It is linked into:

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
