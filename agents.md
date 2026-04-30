# Agent Workflow

This repo should not be advanced by chasing whichever adjacent runtime slice looks easiest.

Use the dependency model in:

- `remaining-work.md`
- `docs/22-vfp-language-reference-coverage.md`
- `docs/23-phase-a-dependency-breakdown.md`
- `agent-handoff.md`

## Operating Rule

Pick work by dependency weight first, not by local adjacency.

That means:

1. follow the critical path in `docs/23-phase-a-dependency-breakdown.md`
2. prefer issue blockers with the highest fan-out
3. close parent-path prerequisites before polishing downstream command surfaces
4. only leave the critical path when a narrow side slice is nearly free and does not delay it

## Current Priority Order

For Phase A, the recommended order is:

1. `#92` residual order/collation/search parity
2. `#97`, `#98`, `#99` macro/eval/runtime-state and memory/assignment foundations
3. `#94` structural table-operation residuals
4. `#100` field-transfer and macro-target parity
5. `#101` headless interaction macro/eval fidelity
6. `#93` remote/result-cursor parity
7. `#95` aggregate/view/helper parity
8. `#96` DBC/container fidelity
9. `#10`, `#11`, `#12` automation depth after the runtime foundation is quieter

## Slice Selection Rules

- Start with the highest-priority open issue that unblocks later work.
- Prefer slices that improve shared runtime behavior used by multiple commands.
- Avoid reopening recently-deepened lanes unless there is a concrete parity bug.
- Keep implementation narrow, add focused regression coverage, and update the backlog/docs after validation.

## Handoff Rules

- `agent-handoff.md` is the canonical continuation brief.
- Update it whenever a shipped slice changes the recommended next target.
- Do not create extra prompt files unless explicitly requested.
- If a temporary planning note is created, fold any lasting guidance back into the tracked docs and delete the throwaway note.
