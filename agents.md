# Agent Workflow

This is the short operating rulebook for Codex work in Project Copperfin.

## Active Guidance Hierarchy

Use these sources in order:

1. Live GitHub issue state.
2. This file for operating rules.
3. `agent-handoff.md` for the compact continuation brief.
4. `docs/05-roadmap.md` for the durable workstream tree and phase/topic map.
5. `docs/23-phase-a-dependency-breakdown.md` for issue-linked progress and historical dependency evidence.
6. `docs/22-vfp-language-reference-coverage.md` when a chosen slice touches VFP/runtime language coverage.
7. `docs/safety/hazard-register.md` and related safety docs when a change is safety-relevant.

`remaining-work.md` is intentionally deprecated as an active planning source. `issues.txt` is a local snapshot only; never use it instead of live GitHub state.

## Standing Goal: Requirements Recovery

Eventual goal, restated here because it keeps getting dropped from condensed guidance: build a DO-178-style LLR <-> code <-> test traceability matrix via requirements recovery, since Copperfin's requirements were never written before its code. See `docs/01-product-charter.md` (Compatibility Fidelity Rule) and `docs/27-known-vfp9-bug-exceptions.md` (bug/crash exception registry). A recovered requirement must validate against real installed VFP9 or shipped docs only, never decompiled binaries (`docs/07-clean-room-rules.md`). Select this subgoal when its evidence, dependencies, or release impact make it the highest-value unfinished work; do not let it block a more important MVP acceptance gap.

## Product And Roadmap Intent

Copperfin is not only a VFP 9 compatibility clone. The foundation is maintaining existing FoxPro/VFP codebases without forced rewrites, then enabling gradual modernization through .NET-facing commands, interop paths, safer host behavior, modern runtime capabilities, richer tooling, portability, generated outputs, host integration, and eventually the rapid application-building feel FoxPro/VFP provided.

Treat wishlist and future-facing issues as deferred roadmap work, not noise. Do not ignore, close, demote, or discard them merely because they are not current blockers. Work them after important/basic compatibility, usability, release-readiness, and prerequisite architecture are stable enough for that lane.

## Active Execution Rule

Choose work from live GitHub issue state and current repo guidance, not from old numbered ledgers.

- The implementation target is the complete MVP workstream tree in `docs/05-roadmap.md`, not one permanently active lane. Select the highest-value unfinished subgoal using live GitHub state, current tests, compatibility risk, blockers, and user-visible impact. Treat localization as a standing architectural constraint for new user-facing text.
- Mark a subgoal complete only when its implementation and acceptance evidence pass. Do not revisit completed subgoals unless a regression, new compatibility evidence, or release-validation failure creates a new gap.
- Prefer the open prompt-sized child issue that unblocks the most downstream work within the selected workstream.
- Create the next prompt-sized child under the selected workstream before coding when no existing child fits.
- Keep implementation narrow, add focused regression coverage, validate, and update durable docs only when behavior or active guidance changes. Use the roadmap for durable workstream structure and progress/handoff files for issue-linked evidence.

## Issue Hierarchy

- Umbrella issues are planning and tracking units.
- Parent/lane issues group related work.
- Prompt-sized child issues are execution units.
- Do not treat umbrella or parent/lane issues as execution units when child issues exist or can be created.
- If a child issue still feels too large for one prompt, split it again before coding.

## Historical Guidance

- Historical Phase A dependency tables and shipped-slice ledgers explain why old work closed; they are not active queues.
- Do not redirect to old Phase A/runtime issues such as `#150`-`#153`, `#92`-`#101`, or `#154`-`#203` unless the corresponding live issue has reopened with fresh regression evidence.
- Preserve evidence needed to understand old closure decisions, but keep recurring handoff text limited to the last shipped slice, selected workstream, and next action.

## Safety Documentation Traceability

Apply these rules whenever a change touches operator-facing or procedure-defining documentation, including README command guidance, runtime debug steps, and recovery procedures.

1. Treat documentation as safety-relevant when misuse could cause critical operator error.
2. Use issue-level identifiers for documentation traceability:
   - `DQ-*` for documentation requirements
   - `DV-*` for documentation verification
   - `HZ-*` for hazard linkage from `docs/safety/hazard-register.md`
3. Require a procedural delta map for each safety-relevant documentation change.
4. Require misuse analysis and severity classification: `none`, `low`, `medium`, `high`, or `catastrophic`.
5. Require independent verification evidence from a second qualified reviewer.
6. Require simulation or walkthrough evidence that validates the expected operator outcome.
7. Require rollback and field-notification planning for incorrect documentation.
8. Do not close a safety-relevant documentation issue without investigation-ready evidence auditable by a third party.

## Handoff Rules

- `agent-handoff.md` is the canonical continuation brief and should stay compact.
- Update `agent-handoff.md` only when a shipped slice changes the last shipped slice, selected workstream, or next action.
- Update `CHANGELOG.md` whenever a turn ships lasting repo changes or materially updates tracked documentation.
- Do not create extra prompt files unless explicitly requested.
- If a temporary planning note is created, fold any lasting guidance back into tracked docs and delete the throwaway note.
- Before cutting or approving a release tag, run `scripts/validate-safety-traceability.ps1` or the Safety Traceability Gate workflow against the intended release issue set and archive the report artifact.
