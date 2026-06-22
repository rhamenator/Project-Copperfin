# Localization And Release Readiness

Copperfin localization is a release-readiness track for shipping Spanish and Portuguese builds without changing machine-readable contracts. The first supported product locales are:

- `en`: default English fallback.
- `es-419`: Spanish for Latin America. Neutral `es` and regional `es-*` requests normalize here until a region-specific catalog is justified.
- `pt-BR`: Brazilian Portuguese. Neutral `pt` and regional `pt-*` requests normalize here until another Portuguese catalog is justified.

## Contract

Localized text must be separated from stable protocol values:

- UI labels, menu text, dialog titles, status messages, release notes, installer text, and human diagnostics are localizable.
- JSON property names, enum-like values, command-line switches, exit codes, log field names, issue IDs, telemetry field names, and generated artifact IDs are not localizable.
- Error handling must keep machine-stable codes or keys available even when the human message is localized.
- Missing locale catalogs fall back to English.
- Missing keys fall back to the stable key instead of returning blank text.
- Locale selection must be deterministic and testable. Standalone Studio accepts `--locale <tag>` and also honors `COPPERFIN_UI_LOCALE`.

## Resource Layout

The initial .NET UI catalog lives in `vsix/Copperfin.VisualStudio/CopperfinLocalization.cs`. It currently covers the standalone Studio shell strings, catalog-backed asset-kind display labels for project, form, class library, report, label, menu, and generic assets, plus the embedded VSIX asset editor title/subtitle/guidance/Open/Reveal/Refresh chrome, project workspace tab labels, Hide project records object-browser option, project command buttons, debugger controls, initial status/guidance strings, project workspace placeholder pane text, explorer list column headers, and static asset-family guidance text. It is linked into:

- `vsix/Copperfin.Studio`: standalone Studio shell.
- `vsix/Copperfin.DesignerSmokeTests`: UI smoke test project.
- `vsix/Copperfin.LanguageServiceTests`: portable catalog tests.

Future surfaces should either reuse this catalog directly or add equivalent resource catalogs with the same locale normalization and fallback rules:

- CLI/native diagnostics: keep command switches and JSON fields stable; localize human summaries behind locale-aware lookup.
- Studio host JSON: keep JSON contracts stable; localize optional display text only when a consumer requests it.
- VSIX UI: continue moving dialogs, deeper pane body text, dynamic project/debugger detail text, and remaining status text into shared resources while preserving VS command IDs.
- Installer text: ship `en`, `es-419`, and `pt-BR` resources with packaging smoke tests.
- Docs/help and generated templates: version translated text separately from generated file identifiers and code symbols.

## Release Checklist

Before a localized production release:

- Run catalog tests proving English fallback, Spanish lookup, Portuguese lookup, and missing-key fallback.
- Smoke the standalone Studio shell with `--locale es-419` and `--locale pt-BR`.
- Smoke VSIX and installer packaging with localized resources included.
- Review Spanish and Portuguese terminology for consistency across Studio, VSIX, CLI diagnostics, docs/help, and templates.
- Verify screenshots or UI smoke captures for clipped text in Spanish and Portuguese.
- Verify JSON and CLI machine-readable contracts are unchanged across locales.
- Record translation source, reviewer, locale, and release version in the release checklist.
