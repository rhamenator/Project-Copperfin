# Historical local-branch reconciliation

Preserved on 2026-11-01 before removing obsolete local branches. These are
historical records, not current work authorization, approval, requirements,
or evidence that an old proposal remains applicable. Embedded instructions
and GitHub links are historical provenance only.

## Retained records

- `workspace-history.jsonl`: all 135 added log records from
  `e9eee68455da263ad3d97e0dbcc9ccf9812b7247`, in their original order and
  spelling, including original sequence numbers and reply references. These
  numbers belong to this archive; they do not extend the current channel log.
- `b5a160f4-d722-49e3-b345-363ee4a87200.json`: original Access catalog
  proposal message from `3ba9c4b0be690eb1b99eb8dd066c11b5b2cc992e`.
  Proposal only; no implementation or real-fixture verification is claimed.
- `41d8c3ea-e8b9-4889-8f3d-7dcc23bd77dc.json`: original detailed Windows
  parser diagnosis from `c4e4173a5c9d5003f8b5337b59ba9f35a927f356`.
  The fix already merged as `fe577050f`; its disposition is preserved on
  origin by `9c197cf47`.

The workspace-consolidation commit also proposed one reusable review
worktree and one `build-review/` root per reviewer, with additional worktrees
for concurrent isolation. That historical proposal is retained here rather
than installed as new active policy. Its cleanup report is in the archived
log. Its old cursor is machine-local state and is intentionally omitted.
Its two ignore entries are already covered by current `/build-*/`.

## Other local commits

- `d93e64e0f` and `aa194742b`: patch-equivalent policy changes already
  merged as `4f98dd30c` and `94d1b6f80`.
- `c850162eb`: patch-equivalent diagnosis record merged as `9c197cf47`.
- `cf3f16e8f`: validation message already merged as `460c6fe3d`; only the
  log sequence differs.
- `ff295e5dd`: temporary parser stage logging intentionally omitted after
  the underlying vector-aliasing fix merged; no outstanding product change.

Self-review: compared all eight commits against fetched origin/main
`552466377833e01c529d57566d72e07be4293f92`, checked patch equivalence and
current file content, parsed every archived JSON record, and checked exact
record equality against the source commits. No runtime or operator procedure
changes; no independent review claimed.
