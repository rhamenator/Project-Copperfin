# Roadmap

The Copperfin roadmap is dependency-first: complete deep runtime and data-engine behavior before broad surface expansion.

Current execution authority is:

- `remaining-work.md`
- `docs/23-phase-a-dependency-breakdown.md`
- `agent-handoff.md`

## Phase A: Core Data And Compatibility Engine

Goal:

- full behavioral compatibility for core DBF/CDX/DBC data paths, work areas/data sessions, command/expression runtime semantics, and automation containment foundations

Current state:

- active and near completion
- critical-path execution is currently anchored to the runtime safety/diagnostics gate and the remaining A3 closure chain

Active issue lanes:

- runtime safety/diagnostics gate: `#150`, `#151`, `#152`, `#153`
- A3 closure lanes: `#92`, `#97`, `#98`, `#99`, `#100`, `#101`, `#93`, `#94`

Execution rule:

- do not advance adjacent open branches (`#154`-`#203`) until `#94` is complete

## Phase B: Runtime Safety And Diagnostic Fidelity

Goal:

- deterministic fault isolation and repeatable debug metadata across pause/resume/retry flows

Current state:

- in active execution under `#13` and `#14`
- strict order: `#150` -> `#151` -> `#152` -> `#153`

## Phase C: Runtime Parity Surfaces

Goal:

- forms/classes, reports/labels, menus, and project startup/build behavior parity

Current state:

- lanes are active-open with focused slice queues (`#154`-`#161`)
- progress exists, but this phase is not treated as complete while Phase A/B critical-path lanes remain open

## Phase D: Build, Compiler, And Debug Pipeline

Goal:

- deterministic package/manifest/build/debug contracts and robust host/runtime launch fidelity

Current state:

- baseline shipped
- active-open follow-on queue exists (`#171`-`#172`; `#162`-`#170` shipped)

## Phase E: Designers And IDE Parity

Goal:

- shared design model, designer interaction fidelity, and IDE shell parity

Current state:

- active-open queues remain under `#22`-`#29` (`#168`-`#183`)

## Phase F: Federation, Interop, And Modern Platform

Goal:

- relational/document federation, modern interop outputs, and runtime bridge contracts

Current state:

- active-open queues under `#30`-`#32`, `#57`, `#91` (`#184`-`#189`, `#200`-`#203`)

## Phase G: Security And Policy

Goal:

- runtime/project policy depth and extension/host policy hardening

Current state:

- active-open queue under `#33` and `#34` (`#190`-`#193`)

## Phase H: Portability

Goal:

- portable core boundary and standalone/core host support on macOS and Linux

Current state:

- active-open queue under `#35`-`#37` (`#194`-`#199`)
- remains downstream of Windows-first runtime stabilization priorities

## Delivery Discipline

- execute one prompt-sized issue slice at a time
- ship code with focused regression coverage
- update changelog + handoff + backlog docs with each durable slice
- prefer critical-path blockers before adjacent depth work
