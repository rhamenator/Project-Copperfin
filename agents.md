# Agent Workflow

This repo should not be advanced by chasing whichever adjacent runtime slice looks easiest.

Use the dependency model in:

- `remaining-work.md`
- `docs/22-vfp-language-reference-coverage.md`
- `docs/23-phase-a-dependency-breakdown.md`
- `agent-handoff.md`

## Operating Rule

Pick work by dependency weight first, not by local adjacency.

Pick a concrete child issue first whenever one exists. Do not treat broad lane issues or umbrella issues as execution units when native slice issues are already open beneath them.

That means:

1. follow the critical path in `docs/23-phase-a-dependency-breakdown.md`
2. prefer issue blockers with the highest fan-out
3. close parent-path prerequisites before polishing downstream command surfaces
4. only leave the critical path when a narrow side slice is nearly free and does not delay it

## Current Priority Order

For Phase A, the recommended order is:

1. `#150`, then `#151` under `#13`
2. `#152`, then `#153` under `#14`
3. only after those open runtime-safety/diagnostics slices are exhausted, take the highest-fan-out next slice under `#93` or `#94`
4. after the active Phase A runtime/data path is materially quieter, move outward to the already-split later-phase child queues under `#15` and beyond

## Slice Selection Rules

- Start with the highest-priority open child issue that unblocks later work.
- Prefer slices that improve shared runtime behavior used by multiple commands.
- Avoid reopening recently-deepened lanes unless there is a concrete parity bug.
- Keep implementation narrow, add focused regression coverage, and update the backlog/docs after validation.

## Tree Rules

- Use umbrella issues only for planning, dependency tracking, and closure.
- Use lane issues for grouping related prompt-sized work.
- Use child issues as the normal execution unit.
- If a child issue still feels too large for one prompt, split it again before coding.
- If a lane has no open child issues, create the next native child issue before starting implementation.

## Handoff Rules

- `agent-handoff.md` is the canonical continuation brief.
- Update it whenever a shipped slice changes the recommended next target.
- Update `CHANGELOG.md` whenever a turn ships lasting repo changes or materially updates tracked documentation.
- Do not create extra prompt files unless explicitly requested.
- If a temporary planning note is created, fold any lasting guidance back into the tracked docs and delete the throwaway note.
