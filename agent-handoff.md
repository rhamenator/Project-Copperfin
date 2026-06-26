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

- Last shipped E3 child: `#2680`, nested report/label group ordering fidelity; focused `test_report_layout` and Studio host JSON coverage now prove FRX/LBX layouts preserve nested group-header/group-footer ordering, distinct group expressions, selected inner-group section metadata, full preview bounds, and label identity without changing machine-readable designer contracts.
- Last shipped localization child: `#2628`, the remaining `StudioHost.LaunchParse.ObjectCommand.*` labels now have shipped-catalog parity in `en-US`, `es-419`, `pt-BR`, and `qps-ploc`, and focused studio-host coverage now proves translated and pseudo-localized object-command labels without changing CLI flag names, placeholder option tokens, launch-parse machine behavior, JSON contract fields, or other machine-readable studio-host contracts.
- Current lane: E3/#24 report/label designer fidelity. Localization/#2348 is now a standing architectural constraint for any new user-facing text rather than the default implementation lane.
- Next action: inspect live `#24` children or create the next prompt-sized E3 slice from current real gaps, with remaining likely seams around report/label grouping or sorting metadata beyond single-surface ordering evidence, shared host/designer round-trip fidelity, report/label runtime-preview hooks, and any other FRX/LBX behavior the live issue tree still shows as unproven.
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
