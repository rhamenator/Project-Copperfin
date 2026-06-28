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

- Last shipped E3 child: `#2691`, report/label layout font-option edit fidelity; focused Studio host JSON regressions now prove FRX/LBX layout-object `FONTSIZE` updates and `MODE` clears round-trip through record-selected, stable-selected, and deleted record-selected flows, and the older deleted layout-object font regressions now match current placed deleted-object containing-section semantics without changing machine-readable designer contracts.
- Last shipped localization child: `#2719`, the remaining OLE method-invocation and property-read fallback faults in `src/runtime/prg_engine.cpp` now route through shared `Runtime.Prg.Core.Error.*` catalog entries with shipped parity in `en-US`, `es-419`, `pt-BR`, and `qps-ploc`, focused localization coverage now proves placeholder-preserving catalog parity for the new OLE invocation/read keys, and focused PRG control-flow coverage now proves pseudo-localized missing-object and missing-member OLE invocation/read failures without changing AERROR metadata, automation bridge identifiers, object state handling, or other machine-readable runtime contracts.
- Current lane: Localization/#2348 remains active for the remaining runtime/host hard-coded user-facing prose; E3/#24 is still open, but new user-facing work must stay localized by default now, and after the remaining localization backlog is exhausted, future user-facing changes should continue landing localized at introduction time instead of reopening a hard-coded-string cleanup lane.
- Next action: continue live GitHub triage for `#2348` outside both `src/runtime/prg_engine_dispatch.inl` and the residual OLE fallback paths in `src/runtime/prg_engine.cpp`; prioritize any remaining runtime/host/native user-facing prose that still bypasses shipped catalogs, while keeping E3/#24 open for report/label fidelity and preserving localization as a standing requirement for every new user-facing surface.
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
