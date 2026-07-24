# Remaining Work

This file is intentionally deprecated as an active planning source.

Historical shipped-slice ledgers that used to live here made Codex prompts expensive and occasionally contradicted live issue state. Do not use this file to choose the next implementation slice.

Use this hierarchy instead:

1. Live GitHub issue state.
2. `agents.md` for operating rules and issue hierarchy.
3. `agent-handoff.md` for the compact continuation brief.
4. `docs/05-roadmap.md` for the durable workstream tree and phase/topic map.
5. `docs/23-phase-a-dependency-breakdown.md` for issue-linked progress and historical dependency evidence.
6. `docs/22-vfp-language-reference-coverage.md` when the chosen slice touches VFP/runtime language coverage.

There is no permanently active implementation lane. Select the highest-value unfinished subgoal from the workstream tree in `docs/05-roadmap.md` using live GitHub state and current evidence. Completed subgoals remain closed unless a regression, new compatibility evidence, or release-validation failure creates a new gap. Historical Phase A, D1, E1, and old issue sequences remain closed unless live regression evidence reopens them.

Wishlist and future-facing modernization issues remain valid deferred roadmap work. They should wait behind important/basic compatibility, usability, release-readiness, and prerequisite architecture unless live GitHub state shows they have become the highest-weight blocker.

`issues.txt` is only a local snapshot of open issue rows at the time it was exported. It is not a planning authority.
