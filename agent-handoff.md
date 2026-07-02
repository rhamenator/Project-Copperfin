# Agent Handoff

Canonical Copperfin continuation brief. Keep this file compact; do not rebuild a shipped-slice ledger here.

## Active Directive

- Start from live GitHub issue state plus the current repo guidance in `agents.md` and `docs/23-phase-a-dependency-breakdown.md`.
- Use the issue hierarchy: umbrella issues are planning/tracking units, parent/lane issues group work, and prompt-sized child issues are execution units.
- Pick or create one open prompt-sized child issue before implementation. Do not execute directly from an umbrella or parent/lane issue when child slices exist or can be created.
- Current active implementation lane is E3/#24, report/label designer fidelity, unless live GitHub state shows a higher-weight blocker. Localization/#2348 is a standing architectural constraint for new user-facing text, including after the current hard-coded backlog reaches zero.
- Treat the current shared WinForms shell chrome as provisional, not target UX. This is durable product direction, not optional polish: future shell work should keep shared designer behavior but move host-specific chrome into native hosts, with VSIX UI aligned to current Visual Studio 202x conventions, the VSIX Command window implemented by default as a bottom tabbed tool window in the same family as Terminal/Output panes, the standalone host owning a modern dockable shell with dockable Command and terminal panes, and Windows standalone work aiming for contemporary Visual Studio-grade polish without forcing a literal clone onto macOS or Linux. Each host should follow its platform UI standard directly: Microsoft Windows conventions for Windows, Apple conventions for macOS, and Linux conventions consistent with how Linux users expect tools such as VS Code to behave. Keep new visible shell text localized while those host-shell upgrades land.
- Phase A, D1/#19, E1/#22, and old numbered runtime/planning sequences are historical/closed unless live regression evidence reopens them.
- Preserve deferred modernization and wishlist work as roadmap intent, not noise; do not close, ignore, or discard future-facing issues merely because they are not current blockers.
- Keep changes narrow, implementation-first, and validated. Update durable docs only when behavior or active guidance changes.

## Current State

- Last shipped E3 child: `#3198`, the shared real `STYLE2V.FRX` host-backed live reorder smoke now captures the source preview-bounds geometry and fails if `PREVIEWBOUNDS` drift during reorder or reload, so live reorder coverage can no longer accept host-JSON preview regressions hidden behind correct UNIQUEID, section-count, and record-order behavior alone.
- Last shipped localization child: `#3172`, `FoxProIntelliSenseCatalog` now routes built-in Quick Info, completion metadata, signature-help documentation, and source-derived project symbol prose through the shared localization catalogs for `en-US`, `es-419`, `pt-BR`, and `qps-ploc` instead of hard-coded English strings, while preserving invariant token names, symbol kinds, and signature content.
- Current lane: E3/#24 remains the active implementation lane for report/label designer fidelity; Localization/#2348 is now a standing engineering constraint for all new user-facing text and still needs fresh audits only when live repo evidence shows remaining hard-coded prose.
- Next action: continue live GitHub triage under E3/#24 by selecting or creating the next prompt-sized child from the remaining report/label host/designer fidelity backlog. After `#3198`, the host-backed real preview-metadata seams for deleted-object workflows plus live duplicate/rename/reorder now have explicit real-asset evidence, so the next worthwhile slice should be whichever remaining E3 child still lacks real-asset evidence across report/label editing, shared host JSON fidelity, or project-startup integration beyond the already-covered VFP9/VFPSource startup paths, remaining live-object property/batch/placement preview-metadata seams, fake-host layout-action families, and recent real-sample packaging-path/look-up fixes.
- Validation note: on Linux, the full `mono vsix/Copperfin.DesignerSmokeTests/bin/Release/net472/Copperfin.DesignerSmokeTests.exe` run is still not a clean release gate because the unmanaged Mono/libgdiplus tail remains unstable and the suite lacks targeted filtering; the latest E3 host-backed live reorder preview-metadata hardening slice used `git diff --check`, `dotnet build vsix/Copperfin.DesignerSmokeTests/Copperfin.DesignerSmokeTests.csproj -c Release`, and a focused repo-local net472/Mono reflection probe that invoked `SmokeRealAssetHostBackedReorderRoundTrip` under the resolved local toolchain against real `STYLE2V.FRX`, proving source `PREVIEWBOUNDS` geometry stays stable through live reorder and reload in the current native snapshot contract. Earlier focused validation remains as described below for the shared designer/runtime workflow families.
- E2/#23 remains open mostly for evidence-audit/closure cleanup. Do not open more E2 wrapper work unless new APIs or fresh regression evidence show a real implementation gap.
- Localization/release-readiness slices under `#2348`/`#113` have shipped through `#2352` and `#1856`; continue them only if live issue state makes them higher priority than E3/#24.
- Shell/UI backlog guidance is now captured in `#3050` (platform UI standards for native hosts), `#3051` (dockable Command and terminal surfaces, including the default bottom-tabbed VSIX Command window behavior), and `#2998` (future curated standalone extension support without loading the Copperfin VSIX). Those three issues already cover the current durable UI direction; reuse them rather than opening duplicate backlog items unless new scope appears beyond the existing host-shell, docking, terminal, extension-compatibility, or localization-ready shell-text boundaries.

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
