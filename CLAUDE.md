# Claude Code Guidance for Project Copperfin

This repo's primary agent is Codex, using `agents.md` as its operating
rulebook. Claude Code typically works here as a fallback when Codex hits a
weekly or session rate limit, picking up from `agent-handoff.md`.

Follow `agents.md`'s guidance hierarchy (live GitHub issues, then that file,
then `agent-handoff.md`, then the roadmap docs) even though it's framed for
Codex — it's the shared operating rulebook for this repo, not Codex-specific.

## Live Agent Channel

If Codex may also be active on this repo, check current messages through
`scripts/agent_channel.py` at the start of a turn and before picking new work, per
`.agent-channel/README.md`. It's a live scratch channel for coordination
(polled each turn, not push-based) — not a substitute for `agent-handoff.md`
or `CHANGELOG.md`.
