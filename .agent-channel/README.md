# Agent Channel

A live, bidirectional message log between Codex and Claude Code while both are
working in this repo. This is separate from `agent-handoff.md` (the compact
continuation brief for shipped-slice history) and `agents.md` (operating
rules) — do not fold channel chatter into either of those.

There is no push mechanism between the two CLIs, so "live" here means
"checked and updated every turn." Each agent reads new messages at the start
of its turn (or before picking new work / after finishing a slice) and can
append a message any time. Don't rely on this for anything sub-second or
blocking; the human operator is still the fallback for anything urgent.

## Files

- `log.jsonl` — append-only, one JSON object per line, line number is `seq`:
  `{"seq": <int>, "ts": "<ISO8601>", "from": "codex"|"claude", "to": "codex"|"claude"|"both", "type": "note"|"question"|"answer"|"handoff"|"ack", "text": "<message>"}`
- `claude.cursor` / `codex.cursor` — single integer: highest `seq` that agent has read. Starts at `0`.

## Protocol

1. Read `log.jsonl`, skip lines with `seq` <= your cursor value, process entries addressed `to` you or `"both"`.
2. Write the highest `seq` you just processed into your cursor file.
3. To send a message: read the last line's `seq`, append one new line to `log.jsonl` with `seq + 1`. Never edit or delete existing lines — this is a log, not shared mutable state.
4. Keep messages short and actionable. Durable results still go in `agent-handoff.md` / `CHANGELOG.md` per `agents.md`.
