# Retirement of `v1-development` — 2026-10-01

## Decision

On 2026-10-01 the repository owner (`rhamenator`) decided that `main` and
`v1-development` must always be the same line of history and that development
happens on `main`; the second branch is therefore retired. `main` was
fast-forwarded to `v1-development` (`fa527a2b2`) and later to the merge of
#6716 (`d68d7494b`) with plain fast-forward pushes (no force, no new commit
IDs), after which `v1-development` was brought level with `main`.

## What changed

- Pull requests target `main`; `main` is the default branch, so `Fixes #N`
  closes issues on merge (still verify).
- `main`'s ruleset `20356131` ("Protect main history") now also requires the 11
  status checks that ruleset `20582609` ("Protect v1 development history")
  required (strict, up to date with the base), so the merge gate is unchanged.
  Deletion and non-fast-forward remain blocked, pull requests and thread
  resolution remain required, 0 approvals, and the owner bypass is unchanged.
  The pre-change JSON of both rulesets is kept on the owner's workstation.
- Workflow triggers and their contract checks name `main` only.
- Historical records (older traceability reports, `docs/main-fast-forward-2026-09-30.md`,
  `docs/dco-historical-attestation-2026-09-29.md`, changelog entries) still say
  `v1-development` and are left as written.

## Retiring the branch

The branch and its ruleset are deleted after the owner has told every agent
working in this repository to branch from `main`, and after nothing open
targets `v1-development`. Anything still based on it can be rebased onto
`main`: the two are the same commit at the time of the final sync, so no
history differs.
