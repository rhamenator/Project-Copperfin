# Agent Handoff

Canonical Copperfin continuation brief. Keep this file compact; do not rebuild a shipped-slice ledger here.

## Active Directive

- Start from live GitHub issue state plus the current repo guidance in `agents.md` and `docs/23-phase-a-dependency-breakdown.md`.
- Use the issue hierarchy: umbrella issues are planning/tracking units, parent/lane issues group work, and prompt-sized child issues are execution units.
- Pick or create one open prompt-sized child issue before implementation. Do not execute directly from an umbrella or parent/lane issue when child slices exist or can be created.
- Current active implementation lane is E3/#24, report/label designer fidelity, unless live GitHub state shows a higher-weight blocker. Localization/#2348 is now a standing architectural constraint for new user-facing text.
- Phase A, D1/#19, E1/#22, and old numbered runtime/planning sequences are historical/closed unless live regression evidence reopens them.
- Preserve deferred modernization and wishlist work as roadmap intent, not noise; do not close, ignore, or discard future-facing issues merely because they are not current blockers.
- Keep changes narrow, implementation-first, and validated. Update durable docs only when behavior or active guidance changes.

## Current State

- Last shipped E3 child: `#2367`, builder and wizard registry display title, description, launch validation, and launch catalog error localization for report/label designer-facing builder discovery paths.
- Last shipped localization child: `#2609`, the remaining `Runtime.Prg.Total.Error.*` diagnostics now have shipped-catalog parity in `en-US`, `es-419`, `pt-BR`, and `qps-ploc`, and focused localization coverage now proves translated prose, placeholder preservation, and pseudo-localized resolution without changing `TOTAL` command behavior, selected-work-area handling, alias/target resolution, runtime identifiers, or other machine-readable/runtime contracts.
- Current lane: continue prompt-sized localization cleanup under `#2348` until the obvious repo-wide user-facing string backlog is handled, then return to E3/#24 report/label designer fidelity and section-aware host surfaces.
- Next action: inspect live `#2348` children for the next prompt-sized localization slice, with the obvious shared-core keys, `copperfin_studio_host` usage/object-label/builder-parse/editor-action-parse/designer-parse/toolbox-palette-parse/toolbox-parse gaps, `copperfin_inspect` usage parity, and runtime-host/shared-runtime top-level/debug/bridge/manifest-verification/pause-session/watch/quit-prompt plus security-denial-audit, security-profile, non-profile security diagnostics, federation-planner-fallback/query-translator/aggregate command/SQL/report-output, database-profile, extensibility-profile, remaining native `Vfp.*`, shared `Runtime.IndexSeek.*`, shared `Runtime.XAsset.*`, shared `Runtime.PrgStaticAnalysis.*`, shared `Runtime.Package.Error.*`, and all shared `Runtime.Prg.*` gaps now closed; prefer the next coherent host/UI surface that still falls back to English, keep all new user-facing work localized by default once the backlog is closed, and if the remaining localization backlog stops blocking MVP release readiness, return to E3/#24 report/label designer fidelity.
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
