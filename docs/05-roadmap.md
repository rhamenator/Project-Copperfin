# Roadmap

The Copperfin roadmap is dependency-first: complete deep runtime and data-engine behavior before broad surface expansion.

Current execution authority is:

- live GitHub issue state
- `agents.md`
- `agent-handoff.md`
- `docs/23-phase-a-dependency-breakdown.md`

`remaining-work.md` is deprecated as active planning input; use it only for its deprecation notice.

## Phase A: Core Data And Compatibility Engine

Goal:

- full behavioral compatibility for core DBF/CDX/DBC data paths, work areas/data sessions, command/expression runtime semantics, and automation containment foundations

Current state:

- critical-path closure is complete
- adjacent work now proceeds from the post-D1/E1 designer queue instead of the old runtime-safety gate

Active issue lanes:

- critical-path Phase A lanes are closed: `#92`, `#93`, `#94`, `#97`, `#98`, `#99`, `#100`, `#101`, `#150`, `#151`, `#152`, `#153`
- live execution is currently centered on E3/#24 report/label designer fidelity; latest shipped child `#1916` covers record-selected report/label section top clear preview metadata; E2/#23 remains open mainly for evidence-backed audit/closure cleanup of already implemented designer host slices

Execution rule:

- do not reopen closed Phase A lanes without a concrete regression; prefer the current E3/#24 directive in `docs/23-phase-a-dependency-breakdown.md` unless live GitHub state shows a higher-weight blocker

## Phase B: Runtime Safety And Diagnostic Fidelity

Goal:

- deterministic fault isolation and repeatable debug metadata across pause/resume/retry flows

Current state:

- `#13` is closed and the current prompt-sized `#14` child queue is exhausted through `#273`
- any new work under this phase should start with a fresh child issue instead of reusing the closed safety queue

## Phase C: Runtime Parity Surfaces

Goal:

- forms/classes, reports/labels, menus, and project startup/build behavior parity

Current state:

- first-pass runtime parity lanes are closed (`#15`-`#18`, `#154`-`#161`)
- follow-on work here should reopen only when a new parity gap is identified or a deeper child issue is created

## Phase D: Build, Compiler, And Debug Pipeline

Goal:

- deterministic package/manifest/build/debug contracts and robust host/runtime launch fidelity

Current state:

- baseline shipped
- current prompt-sized child queues are closed on this branch (`#162`-`#177` shipped for build/debug; `#178`-`#183`, `#395`, `#396`, `#397`, `#398`, `#399`, `#400`, and `#401` shipped for language service)
- routine validation now also includes a managed compile gate over `Copperfin.LanguageServiceTests`, `Copperfin.Studio`, and `Copperfin.DesignerSmokeTests`

## Phase E: Designers And IDE Parity

Goal:

- shared design model, designer interaction fidelity, and IDE shell parity

Current state:

- `#22` is closed; `#23` and `#24` remain open as designer lane parents.
- `#23` still has many open evidence-audit child rows; close completed children only with specific issue/test/validation evidence, and keep parent `#23` open until root-level closure evidence is explicit.
- `#24` remains the active E3 report/label lane; latest shipped child is `#1910`, so inspect live GitHub state and create or pick the next prompt-sized E3 child before more report/label implementation unless a higher-weight blocker is open.
- `#27`-`#29` currently have no downstream open native slice queue in the tracked handoff (`#178`-`#183`, `#395`-`#401` shipped); re-audit those parent issues before opening new language-service children.

## Phase F: Federation, Interop, And Modern Platform

Goal:

- relational/document federation, modern interop outputs, and runtime bridge contracts

Current state:

- historical native slice queues under `#30`-`#32`, `#57`, and `#91` were exhausted after `#200`-`#203` shipped; re-audit live GitHub parent issues before opening new federation/interop children.

## Phase G: Security And Policy

Goal:

- runtime/project policy depth and extension/host policy hardening

Current state:

- parent issues `#33` and `#34` remain open for deeper policy work, but the tracked prompt-sized child queue `#190`-`#193` is closed
- create a fresh child issue before new implementation work under this phase

## Phase H: Portability

Goal:

- portable core boundary and standalone/core host support on macOS and Linux

Current state:

- downstream open queue under `#35`-`#37` (`#194`-`#199`); do not prioritize it ahead of E3/#24 without live higher-weight blocker evidence
- remains downstream of Windows-first runtime stabilization priorities

## Delivery Discipline

- execute one prompt-sized issue slice at a time
- ship code with focused regression coverage
- update changelog + handoff + backlog docs with each durable slice
- prefer critical-path blockers before adjacent depth work
