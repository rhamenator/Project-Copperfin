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

- Last shipped E3 child: `#2992`, real deleted FRX/LBX rename continuity; focused host-backed and shared VSIX/standalone designer smokes now drive representative deleted rows from VFPSource `by_author.FRX` and `STYLELBL.LBX` through real rename, reload, command undo, and restore flows, proving deleted-object identity, containing-section continuity, and on-disk `UNIQUEID` round trips stay stable for supported deleted-row renames without changing host JSON keys, selectors, or localized contract names.
- Last shipped localization child: `#2959`, the shared VSIX/standalone `CopperfinDesignerSelection` path now routes the remaining form/class-library, menu, project, and generic property-grid display labels through the existing localization catalogs for `en-US`, `es-419`, `pt-BR`, and `qps-ploc` instead of hard-coded English text, while focused designer smokes prove those visible labels localize without changing invariant property names or machine-readable update targets such as `PROMPT`.
- Current lane: E3/#24 remains the active implementation lane for report/label designer fidelity; Localization/#2348 is now a standing engineering constraint for all new user-facing text and still needs fresh audits only when live repo evidence shows remaining hard-coded prose.
- Next action: continue live GitHub triage under E3/#24 by selecting the next prompt-sized child from the remaining report/label host/designer fidelity backlog; after `#2992` closed the real deleted rename continuity seam, `#2991` closed the real deleted property-edit continuity seam, `#2990` closed the real deleted section-member continuity seam, `#2989` closed the real FRX/LBX single-command batch-undo seam, `#2988` closed the shared batch-refresh seam, `#2987` closed the real section-property continuity seam, `#2986` closed the real FRX duplicate reopen seam, `#2985` closed the real unplaced FRX placement-transition seam, `#2984` closed the real unplaced FRX continuity seam, `#2983` closed the real memo-backed write-back seam, `#2982` closed the shared property-grid direct-field seam, `#2981` closed the host-level real reload+undo seam, and `#2980` closed non-Windows real-asset smoke fallback resolution, prefer the next real-asset gap that still lacks evidence such as deleted-object reorder or duplicate continuity against real assets, richer supported section/object property families beyond the current direct-field, geometry, lifecycle, and batch slices, or another report/label fidelity seam surfaced by live GitHub state, and treat the real-project debugger path as a separate Windows-first follow-up only if fresh evidence shows a real implementation seam rather than the current runtime build-execution environment limit.
- E2/#23 remains open mostly for evidence-audit/closure cleanup. Do not open more E2 wrapper work unless new APIs or fresh regression evidence show a real implementation gap.
- Localization/release-readiness slices under `#2348`/`#113` have shipped through `#2352` and `#1856`; continue them only if live issue state makes them higher priority than E3/#24.

## Required Reading

Read only what is needed for the chosen slice:

- `agents.md` for operating rules and safety traceability.
- `docs/23-phase-a-dependency-breakdown.md` for current lane status and historical dependency evidence.
- `docs/28-repository-ontology.md` when the slice needs current repo-structure grounding instead of aspirational architecture summaries.
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
