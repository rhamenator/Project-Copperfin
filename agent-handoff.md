# Agent Handoff

Canonical Copperfin continuation brief. Keep this file compact; do not rebuild a shipped-slice ledger here.

## Active Directive

- Start from live GitHub issue state plus the current repo guidance in `agents.md` and `docs/23-phase-a-dependency-breakdown.md`.
- Use the issue hierarchy: umbrella issues are planning/tracking units, parent/lane issues group work, and prompt-sized child issues are execution units.
- Pick or create one open prompt-sized child issue before implementation. Do not execute directly from an umbrella or parent/lane issue when child slices exist or can be created.
- Current active implementation lane is E3/#24, report/label designer fidelity, unless live GitHub state shows a higher-weight blocker. Localization/#2348 is a standing architectural constraint for new user-facing text, including after the current hard-coded backlog reaches zero.
- Treat the current shared WinForms shell chrome as provisional, not target UX. This is durable product direction, not optional polish: future shell work should keep shared designer behavior but move host-specific chrome into native hosts, with VSIX UI aligned to current Visual Studio 202x conventions, the VSIX Command window implemented as a bottom tabbed tool window in the same family as Terminal/Output panes, the standalone host owning a modern dockable shell with dockable Command and terminal panes, and each host following its platform UI standard directly: Microsoft Windows conventions for Windows, Apple conventions for macOS, and Linux conventions consistent with how Linux users expect tools such as VS Code to behave.
- Phase A, D1/#19, E1/#22, and old numbered runtime/planning sequences are historical/closed unless live regression evidence reopens them.
- Preserve deferred modernization and wishlist work as roadmap intent, not noise; do not close, ignore, or discard future-facing issues merely because they are not current blockers.
- Keep changes narrow, implementation-first, and validated. Update durable docs only when behavior or active guidance changes.

## Current State

- Last shipped E3 child: `#3046`, shared report/label snap-to-grid command parity; the shared VSIX/standalone asset editor now routes live FRX/LBX object snap-to-grid through the existing Studio host `--snap-object --snap-mode both` contract with invariant record-plus-target selectors and shared `GRIDH`/`GRIDV` settings-derived grid dimensions, preserves selection continuity across both host-backed snap and command-undo refreshes so the selected object stays active while snapped geometry is immediately reflected in the shared property grid, and focused designer smokes now prove invariant host invocation shape plus shared continuity on synthetic report/label shells and real `STYLE3V.FRX` + `STYLELBL.LBX` round trips without changing host JSON keys, invariant property names, or existing layout-action command contracts.
- Last shipped localization child: `#2959`, the shared VSIX/standalone `CopperfinDesignerSelection` path now routes the remaining form/class-library, menu, project, and generic property-grid display labels through the existing localization catalogs for `en-US`, `es-419`, `pt-BR`, and `qps-ploc` instead of hard-coded English text, while focused designer smokes prove those visible labels localize without changing invariant property names or machine-readable update targets such as `PROMPT`.
- Current lane: E3/#24 remains the active implementation lane for report/label designer fidelity; Localization/#2348 is now a standing engineering constraint for all new user-facing text and still needs fresh audits only when live repo evidence shows remaining hard-coded prose.
- Next action: continue live GitHub triage under E3/#24 by selecting the next prompt-sized child from the remaining report/label host/designer fidelity backlog; after `#3046` closed the shared live snap-to-grid seam on top of `#3045` shared horizontal-distribution command parity, `#3044` shared left-alignment command parity, `#3043` shared post-nudge property-grid geometry refresh, and `#3042` shared real-asset report/label nudge continuity coverage, treat the live shared layout-action authoring path as covered for left alignment, horizontal distribution, snap-to-grid, and nudging unless fresh evidence shows a broader section/object command gap, and prefer the next host-backed shared-model gap where the native contract already exists but the common editor still lacks parity, especially the remaining `--resize-object` resize-to-anchor seam, before doing more raw FRX/LBX corpus hunting or reopening already-covered visual-property continuity lanes.
- Validation note: on Linux, the full `mono vsix/Copperfin.DesignerSmokeTests/bin/Release/net472/Copperfin.DesignerSmokeTests.exe` run still hits a pre-existing late native Mono/libgdiplus font crash before the later real-asset lane in `Main`, so late-added real-asset smokes may require targeted invocation or Windows validation until that unrelated crash lane is fixed.
- E2/#23 remains open mostly for evidence-audit/closure cleanup. Do not open more E2 wrapper work unless new APIs or fresh regression evidence show a real implementation gap.
- Localization/release-readiness slices under `#2348`/`#113` have shipped through `#2352` and `#1856`; continue them only if live issue state makes them higher priority than E3/#24.
- Shell/UI backlog guidance is now captured in `#2996` (VSIX native shell standards), `#2997` (modern dockable standalone shell), and `#2998` (future curated standalone extension support without loading the Copperfin VSIX). Reuse those issues rather than opening duplicate UI-direction backlog items unless new scope appears beyond the existing host-shell, docking, terminal, or extension-compatibility boundaries.

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
