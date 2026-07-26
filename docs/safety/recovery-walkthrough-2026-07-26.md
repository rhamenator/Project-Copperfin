# MVP Recovery Walkthrough Evidence

This record captures the focused, headless recovery-contract run for the
tested product source head `c0e40a54`, which is an ancestor of the current
synchronized documentation head. It supports the #4403 DQ/DV evidence
ledger; it does not replace hosted Windows/VFP9 validation or independent
review.

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

## Reproducible Recovery Sequence

The focused assertions provide the following bounded recovery sequence and
observed contracts:

1. Package materialization stages the startup source and project assets,
   emits both `app.cfmanifest` and `app.cfdebug`, and records breakpoint and
   step-debug support in the debug plan. The corresponding assertions are in
   `test_materialize_runtime_package`.
2. A PRG fault enters the debug error state without losing the paused
   runtime's stack, watch, and breakpoint state. A later `continue` resumes
   the same runtime process and reaches the post-fault path, as asserted by
   `test_runtime_host_preserves_debug_state_across_prg_fault`.
3. Executable xAsset, report/label, and unexpected-process fault paths are
   separately checked for containment by
   `test_runtime_host_contains_executable_xasset_action_faults`,
   `test_runtime_host_contains_report_label_action_faults`, and
   `test_runtime_host_contains_unexpected_process_fault`.
4. Failed xAsset bootstrap writes and failed manifest-pair promotions are
   cleaned or rolled back without leaving a partially published package. The
   relevant assertions are
   `test_runtime_host_cleans_failed_xasset_bootstrap_write`,
   `test_manifest_pair_finalization_rolls_back_failed_promotions`, and
   `test_manifest_pair_finalization_recovers_stale_transactions`.
5. Localized pause, fault, breakpoint, and watch diagnostics preserve their
   invariant command tokens, status fields, and pause reasons while the
   human-readable prose changes with the selected catalog. These contracts
   are covered by the runtime-host localization tests and the four locale
   reruns below.

This is a test-backed recovery sequence, not a claim that a human operated a
single interactive session through every step. The hosted Windows and
mounted-VFP9 walkthrough remains a separate release gate.

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

## Current-Tree Rerun

The focused checks were rerun against the current synchronized tree after
this walkthrough was expanded:

- `test_runtime_pipeline`: direct executable returned exit code 0 and
  printed `All tests passed.`; its expected bounded transaction diagnostics
  covered lock acquisition, parent identity, and owned-file status rejection
  cases.
- `test_localization`: passed independently for all four locales.
- `test_prg_engine_debugger`: passed 2/2 together with the default locale
  check.

This rerun confirms the evidence document itself did not change the tested
runtime behavior. The prior source-head reference remains the identity of the
original captured run; the current synchronized head contains only the
corresponding evidence and coordination updates after that product code.

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
