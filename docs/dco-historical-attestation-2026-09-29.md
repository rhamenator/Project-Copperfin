# DCO historical attestation — 2026-09-29

## Decision

On 2026-09-29 the repository owner (`rhamenator`) directed that the historical
commits that would otherwise fail the contributor sign-off gate be recorded as
signed off by the owner, that the gate itself remain enforced, and that the
`main` and `v1-development` histories be joined. This record is the audit trail
for that decision.

## Why it was needed

`scripts/check-contributor-signoffs.py` requires a `Signed-off-by` trailer for
the author and every `Co-authored-by` identity of each commit in a pull-request
range. Pull requests normally contain only their own commits, so the gate never
saw two classes of older commit:

- 576 merge commits created by GitHub (authored under the owner's GitHub
  noreply identity, committed by GitHub), and
- 134 direct commits by the owner, chiefly agent-channel maintenance made
  before the gate applied, 67 of which also carry a `Co-authored-by` trailer
  for Claude.

Joining the `main` and `v1-development` histories places those commits inside a
pull-request range for the first time, so the gate failed on them. 710 commits
were affected. All 710 are authored under the owner's identities.

## What was done

- `.github/dco-historical-attestations.json` lists the 710 exact commit IDs.
  The owner attests to them; the list is the recorded sign-off.
- `scripts/check-contributor-signoffs.py` reads the manifest from the **trusted
  base revision** of the range under test, never from the range itself, so a
  pull request cannot define its own exemptions; a manifest change takes effect
  only once it has been reviewed and merged. It accepts a commit as signed off
  only when **all** of these hold: its full ID is listed; its author is one of the
  recorded owner identities; and it is an ancestor of a recorded boundary
  commit (the `main` and `v1-development` tips at the time of the decision).
  New work is never an ancestor of a boundary, so it can never be covered.
- The self-test (`test_contributor_signoff_contract`) covers acceptance of an
  attested commit under the base policy and rejection of: new work after the
  boundary, a range that edits the manifest to exempt its own unsigned commit,
  an attestation outside every boundary, and an attestation for an unrecorded
  author.
- Bootstrap: the check runs from the base branch, so the mechanism and the
  manifest land in their own pull request (only newly signed commits), and the
  history-joining merge follows in a second pull request that the merged base
  can validate. Independent review of this design caught, and this version
  fixes, an earlier draft that read the manifest from the pull-request head.
- No history was rewritten and no branch was force-pushed. Original commit IDs
  are unchanged.

## What did not change

- The gate is unchanged for every commit not listed here: each new commit still
  needs its own `Signed-off-by` for its author and each co-author.
- The attestation does not extend to commits by anyone other than the recorded
  identities, and does not extend to future commits.

## Verifying

```sh
python3 scripts/check-contributor-signoffs.py --self-test
python3 scripts/check-contributor-signoffs.py --base <base-sha> --head <head-sha>   # manifest read from <base-sha>
git log --format=%H <boundary> | grep -c -F -f <(python3 -c "import json;print('\n'.join(json.load(open('.github/dco-historical-attestations.json'))['commits']))")
```

Any change to the manifest is visible in review and is subject to the same
required checks as any other change.
