# MVP Recovery Walkthrough Evidence

This record captures the focused, headless recovery-contract run for the
current source head `c0e40a54`. It supports the #4403 DQ/DV evidence ledger;
it does not replace hosted Windows/VFP9 validation or independent review.

## Environment

- Platform: Linux
- Build tree: `build2`
- Build type: existing `RelWithDebInfo` validation tree
- Network: disabled by the CTest isolation contract
- Source/debug machine fields: preserved by the existing runtime-pipeline
  tests; no package or debug contract keys were changed by this walkthrough

## Focused Recovery Contract

Command:

```sh
ctest --test-dir build2 --output-on-failure \
  -R 'test_runtime_pipeline|test_runtime_host_debug_output_formatting|test_prg_engine_debugger|test_localization'
```

Result: **4/4 passed** in 49.25 seconds.

The run covered package materialization and cleanup, runtime-host debug output,
PRG debugger pause/recovery state, and the default catalog contract. The
individual tests were:

- `test_localization`: passed in 1.20 seconds
- `test_runtime_host_debug_output_formatting`: passed in 4.15 seconds
- `test_runtime_pipeline`: passed in 43.87 seconds
- `test_prg_engine_debugger`: passed in 0.02 seconds

## Locale Contract

The dedicated localization target was then run independently for each
supported locale:

```sh
for locale in en-US es-419 pt-BR qps-ploc; do
  COPPERFIN_LOCALE="$locale" ctest --test-dir build2 \
    --output-on-failure -R '^test_localization$'
done
```

Results: `en-US`, `es-419`, `pt-BR`, and `qps-ploc` each passed 1/1.

Localized prose remains separate from invariant diagnostic codes, JSON keys,
debugger command names, parser tokens, and runtime identifiers. The Spanish
and Brazilian Portuguese catalogs still require qualified human language
review before being treated as production translations.

## Evidence Boundary

This walkthrough demonstrates the current Linux package/debug/recovery and
catalog contracts. It does not claim:

- hosted Windows or mounted-VFP9 sample validation
- Visual Studio package-load, theme, or docking behavior
- installer or VSIX installation evidence
- strict safety validation with `RequireClosedIssues=true`
- independent reviewer sign-off

Those gates remain tracked in the release evidence issues and must be recorded
separately before an MVP release tag.
