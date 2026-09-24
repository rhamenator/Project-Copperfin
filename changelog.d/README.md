# Changelog fragments

Each pull request that ships lasting repo changes or materially updates
tracked documentation adds **one fragment file here** instead of editing
the top of `CHANGELOG.md`. Separate files never conflict, so concurrent pull
requests don't need a rebase just for the changelog (#6554).

## Format

- File name: `YYYY-MM-DD-<issue-or-slug>.md`, lowercase letters, digits and
  dashes only, e.g. `2026-09-24-6554-changelog-fragments.md`.
- Content: exactly one `CHANGELOG.md` bullet. The first line starts with
  `- YYYY-MM-DD:` (the same date as the file name). Continuation lines are
  indented by two spaces, with no blank lines.

## Validating and assembling

```sh
python3 scripts/assemble_changelog.py --check   # validate only (also run by ctest)
python3 scripts/assemble_changelog.py           # fold into CHANGELOG.md, newest first, and delete the fragments
```

Fragments are assembled at release/RC time, or whenever a consolidated view
is wanted. Until then, **the unreleased history is `CHANGELOG.md` plus the
fragments in this directory**; read both when you need the latest status.
