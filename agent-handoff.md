# Agent Handoff

Canonical Copperfin continuation brief. Keep this file compact; do not rebuild a shipped-slice ledger here.

## Active Directive

- Start from live GitHub issue state plus the current repo guidance in `agents.md` and `docs/23-phase-a-dependency-breakdown.md`.
- Use the issue hierarchy: umbrella issues are planning/tracking units, parent/lane issues group work, and prompt-sized child issues are execution units.
- Pick or create one open prompt-sized child issue before implementation. Do not execute directly from an umbrella or parent/lane issue when child slices exist or can be created.
- Current active implementation lane is E3/#24, report/label designer fidelity, unless live GitHub state shows a higher-weight blocker. Localization/#2348 is a standing architectural constraint for new user-facing text, including after the current hard-coded backlog reaches zero.
- Treat the current shared WinForms shell chrome as provisional, not target UX. This is durable product direction, not optional polish: future shell work should keep shared designer behavior but move host-specific chrome into native hosts, with VSIX UI aligned to current Visual Studio 202x conventions, the VSIX Command window implemented as a bottom tabbed tool window in the same family as Terminal/Output panes, the standalone host owning a modern dockable shell with dockable Command and terminal panes, and each host following its platform UI standard directly: Microsoft Windows conventions for Windows, Apple conventions for macOS, and Linux conventions consistent with how Linux users expect tools such as VS Code to behave. Keep new visible shell text localized while those host-shell upgrades land.
- Phase A, D1/#19, E1/#22, and old numbered runtime/planning sequences are historical/closed unless live regression evidence reopens them.
- Preserve deferred modernization and wishlist work as roadmap intent, not noise; do not close, ignore, or discard future-facing issues merely because they are not current blockers.
- Keep changes narrow, implementation-first, and validated. Update durable docs only when behavior or active guidance changes.

## Current State

- Last shipped E3 child: `#3061`, grouped real FRX section-height continuity evidence; shared host-backed and shared-editor real-asset section `HEIGHT` smokes now prove grouped multi-object `bandedm.FRX` `Group Header` height edits preserve all eight contained object geometries plus the untouched `Group Footer` sibling row and grouping linkage metadata across update, reload, and undo, without changing host JSON field names, invariant property names, or machine-readable contracts.
- Last shipped localization child: `#2959`, the shared VSIX/standalone `CopperfinDesignerSelection` path now routes the remaining form/class-library, menu, project, and generic property-grid display labels through the existing localization catalogs for `en-US`, `es-419`, `pt-BR`, and `qps-ploc` instead of hard-coded English text, while focused designer smokes prove those visible labels localize without changing invariant property names or machine-readable update targets such as `PROMPT`.
- Current lane: E3/#24 remains the active implementation lane for report/label designer fidelity; Localization/#2348 is now a standing engineering constraint for all new user-facing text and still needs fresh audits only when live repo evidence shows remaining hard-coded prose.
- Next action: continue live GitHub triage under E3/#24 by selecting or creating the next prompt-sized child from the remaining report/label host/designer fidelity backlog; after `#3061`, `#3060`, `#3059`, `#3058`, and `#3057` closed the grouped multi-object section-height, real section-sibling, grouped section-top, and non-top section continuity seams on top of `#3056` label axis-specific snap, `#3055` vertical snap, `#3054` horizontal snap, `#3053` vertical distribution, `#3052` top-alignment, `#3049` resize-to-anchor height, `#3048` resize-to-anchor width, `#3047` resize-to-anchor size, `#3046` snap-to-grid, `#3045` horizontal distribution, `#3044` left-alignment, `#3043` post-nudge property-grid geometry refresh, and `#3042` real-asset report/label nudge continuity coverage, treat the shared live layout-action path as covered for those command families unless fresh evidence shows a broader section/object authoring gap; prefer the next unshipped host-backed shared-model or real-asset round-trip seam around remaining section-aware edit fidelity, report/label settings parity, or unsupported-property preservation rather than reopening already-covered continuity lanes.
- Validation note: on Linux, the full `mono vsix/Copperfin.DesignerSmokeTests/bin/Release/net472/Copperfin.DesignerSmokeTests.exe` run is still not a clean release gate because the unmanaged Mono/libgdiplus tail remains unstable and the suite lacks targeted filtering; this slice used a release `dotnet build` of `vsix/Copperfin.DesignerSmokeTests`, direct host JSON repro for the grouped untouched sibling row, and a focused temporary `net472` reflection runner to invoke just the new host-backed and shared-editor `bandedm.FRX` grouped-section `HEIGHT` smokes, which completed with `TARGETED_FAILURES=0`.
- E2/#23 remains open mostly for evidence-audit/closure cleanup. Do not open more E2 wrapper work unless new APIs or fresh regression evidence show a real implementation gap.
- Localization/release-readiness slices under `#2348`/`#113` have shipped through `#2352` and `#1856`; continue them only if live issue state makes them higher priority than E3/#24.
- Shell/UI backlog guidance is now captured in `#3050` (platform UI standards for native hosts), `#3051` (dockable Command and terminal surfaces, including VSIX bottom-tabbed Command window behavior), and `#2998` (future curated standalone extension support without loading the Copperfin VSIX). Reuse those issues rather than opening duplicate UI-direction backlog items unless new scope appears beyond the existing host-shell, docking, terminal, extension-compatibility, or localization-ready shell-text boundaries.

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
