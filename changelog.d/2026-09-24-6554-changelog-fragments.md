- 2026-09-24: Changelog entries now go in `changelog.d/` fragment files
  (one per pull request) instead of the top of `CHANGELOG.md`, so
  concurrent pull requests no longer conflict on it (#6554).
  `scripts/assemble_changelog.py` validates fragments (`--check`, also run by
  ctest) and folds them into `CHANGELOG.md` at release time. `agents.md`,
  the PR templates, the Copilot agent profile, `CLAUDE.md`,
  `scripts/drive-codex.ps1`, and the roadmap docs now point at both.
