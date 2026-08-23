# Agent Channel

A live, bidirectional message log between Codex and Claude Code while both are
working in this repo. This is separate from `agent-handoff.md` (the compact
continuation brief for shipped-slice history) and `agents.md` (operating
rules) — do not fold channel chatter into either of those.

There is no push mechanism between the two CLIs, so "live" here means
"checked and updated every turn." Each agent reads new messages at the start
of its turn (or before picking new work / after finishing a slice) and can
post a message any time. Don't rely on this for anything sub-second or
blocking; the human operator is still the fallback for anything urgent.

## Files

- `log.jsonl` — historic, read-only archive of the former shared-append protocol.
- `messages/<message_id>.json` — one immutable current message per file. Each
  message has a UUID `message_id`, UTC `ts`, `from`, `to` array, `type`, and
  `text`. The filename must equal its `message_id`.
- `cursors/` — optional local per-agent read state. It is ignored by Git; do
  not commit cursor state or write another agent's cursor.

## Protocol

1. Read and record new messages with `python3 scripts/agent_channel.py read --agent <agent> --only-unread --mark-read`; the tool owns that agent's local UUID cursor. Inspect the current cursor with `python3 scripts/agent_channel.py cursor --agent <agent>`. Never use a numeric sequence as a cursor or write another agent's cursor.
2. To send a message, run `python3 scripts/agent_channel.py post --from <agent> --to <agent-or-both> --type <type> --text <text>`. Stage and commit the newly created `messages/<message_id>.json` file with the related work.
3. Before committing, run `python3 scripts/agent_channel.py verify`. Do not edit or delete message files after publication. A correction is a new message that names the earlier `message_id` in its text.
4. Keep messages short and actionable. Durable results still go in `agent-handoff.md` / `CHANGELOG.md` per `agents.md`.

This protocol avoids the sequence collisions, lost updates, and cursor skips
caused when independent Git worktrees appended to the same JSONL file.
