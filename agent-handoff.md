# Agent Handoff

Canonical Copperfin continuation brief. Keep this file compact; do not rebuild a shipped-slice ledger here.

## Active Directive

- Start from live GitHub issue state plus the current repo guidance in `agents.md` and `docs/23-phase-a-dependency-breakdown.md`.
- Use the issue hierarchy: umbrella issues are planning/tracking units, parent/lane issues group work, and prompt-sized child issues are execution units.
- Pick or create one open prompt-sized child issue before implementation. Do not execute directly from an umbrella or parent/lane issue when child slices exist or can be created.
- Current active implementation lane is E3/#24, report/label designer fidelity, unless live GitHub state shows a higher-weight blocker. Localization/#2348 is a standing architectural constraint for new user-facing text, including after the current hard-coded backlog reaches zero.
- Phase A, D1/#19, E1/#22, and old numbered runtime/planning sequences are historical/closed unless live regression evidence reopens them.
- Preserve deferred modernization and wishlist work as roadmap intent, not noise; do not close, ignore, or discard future-facing issues merely because they are not current blockers.
- Keep changes narrow, implementation-first, and validated. Update durable docs only when behavior or active guidance changes.

## Current State

- Last shipped E3 child: `#2861`, report schema-fallback evidence; the new focused `test_studio_host_report_schema_fallbacks` host regression now proves FRX/LBX missing section `OBJCODE`, missing section `EXPR`, missing section geometry, missing object `OBJCODE`, missing object `EXPR`, and missing object title fallback flows preserve stable `UNIQUEID`-backed section ids, deleted placed-object containing-section metadata, zero-geometry preview accounting, and report/label parity without changing production behavior, and those schema-fallback calls are now retired from the broad `test_studio_host_json` driver.
- Last shipped localization child: `#2726`, `src/runtime/runtime_pipeline.cpp` now emits locale-aware transpiled C# helpers that resolve unsupported-statement and xAsset manual-port `NotSupportedException` text from embedded `Runtime.Package.Transpilation.Error.*` catalog-backed values for `en-US`, `es-419`, `pt-BR`, and `qps-ploc`, honoring `--locale` and `COPPERFIN_LOCALE` at runtime without per-language transpiled artifacts; focused runtime-pipeline coverage now proves the generated PRG and xAsset C# output preserves statement-text and method-identity placeholders while routing the exception prose through localization instead of raw English literals.
- Current lane: E3/#24 remains the active implementation lane for report/label designer fidelity; Localization/#2348 is now a standing engineering constraint for all new user-facing text and still needs fresh audits only when live repo evidence shows remaining hard-coded prose.
- Next action: continue live GitHub triage under E3/#24 by taking `#2862`, the deleted FRX/LBX object/section host-smoke drift exposed after `#2861`; the focused schema-fallback target is now green, `test_studio_host_json` still fails on a separate stale-contract cluster around deleted placed-object containing-section metadata, deleted preview accounting, and deleted section/object summary expectations, and that broader cleanup should stay isolated from the finished schema-fallback extraction while keeping focused host regressions localized.
- E2/#23 remains open mostly for evidence-audit/closure cleanup. Do not open more E2 wrapper work unless new APIs or fresh regression evidence show a real implementation gap.
- Localization/release-readiness slices under `#2348`/`#113` have shipped through `#2352` and `#1856`; continue them only if live issue state makes them higher priority than E3/#24.

## Required Reading

Read only what is needed for the chosen slice:

- `agents.md` for operating rules and safety traceability.
- `docs/23-phase-a-dependency-breakdown.md` for current lane status and historical dependency evidence.
- `docs/22-vfp-language-reference-coverage.md` when the slice touches VFP/runtime language coverage.
- `docs/safety/triage-rubric.md` for feedback triage.
- `docs/safety/test-coverage-gap-analysis.md` when adding tests.
- Runtime, designer, or host files directly relevant to the selected child issue.

`remaining-work.md` is intentionally deprecated as an active planning source. `issues.txt` is a dated local snapshot, not planning authority; verify live GitHub state before choosing work.

## Validation And Updates

- Run focused tests for the selected slice plus `git diff --check`.
- For safety-relevant documentation, keep `DQ-*`, `DV-*`, and `HZ-*` traceability linked to `docs/safety/hazard-register.md`.
- Before a release tag, run `scripts/validate-safety-traceability.ps1` or the Safety Traceability Gate workflow against the intended release issue set and archive the report.
- Update `agent-handoff.md` only when the last shipped slice, current lane, or next action changes.
- Update `CHANGELOG.md` for lasting repo changes or material tracked-documentation changes.
