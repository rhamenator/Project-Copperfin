# Fast-forward of `main` to `v1-development` — 2026-09-30

## Decision

On 2026-09-30 the repository owner (`rhamenator`) approved fast-forwarding
`main` to the tip of `v1-development` so that the two branches are the same
line of history while v1 is being finished, and approved the method below.
This record is the audit trail. It is committed to `v1-development` *before*
the move so that `main` contains it once moved.

## Starting state

- `main` was at `31da60b66` (2026-08-23), 458 commits that `v1-development`
  lacked, and 1,740 commits behind it.
- The two histories were joined by #6692 (merge `5fa8714e1`), so `main`'s tip
  became an ancestor of `v1-development`; a fast-forward is therefore possible
  and rewrites nothing.
- The contributor sign-off gate's owner attestation for historical commits
  (`docs/dco-historical-attestation-2026-09-29.md`, #6691) covers the older
  commits; see that record.

## Method

`git push origin <v1-development-tip>:refs/heads/main`, a plain
fast-forward (no force, no merge commit, no new commit IDs).

`main` is protected by ruleset `20356131` ("Protect main history": no
deletion, no non-fast-forward, pull request required, review-thread
resolution required, 0 required approvals). That ruleset already lists the
owner account (`User` id `51587867`) as a bypass actor with mode `always`, so
the push needs **no change to the ruleset**. GitHub records the push as a
ruleset bypass in the repository's rule insights. An assessment earlier in
the same session that the ruleset had no bypass actors was wrong (it read the
list endpoint, which omits them); the full ruleset was read before acting.

The full pre-change ruleset JSON was saved locally by the owner's
workstation (`ruleset-20356131-before.json` in the reconciliation backup
folder, not committed); its material fields are reproduced here:

```json
{
  "id": 20356131,
  "name": "Protect main history",
  "target": "branch",
  "enforcement": "active",
  "bypass_actors": [{"actor_id": 51587867, "actor_type": "User", "bypass_mode": "always"}],
  "conditions": {"ref_name": {"include": ["~DEFAULT_BRANCH"], "exclude": []}},
  "rules": ["deletion", "non_fast_forward", "pull_request (0 approvals, thread resolution required)"]
}
```

## Alternatives considered

- **PR from `v1-development` into `main`.** Rejected: the DCO check runs the
  base branch's script, which does not yet have the historical attestation, and
  every PR merged into `v1-development` adds a new unsigned GitHub merge commit
  that no fixed manifest can pre-list, so attestation cannot catch up with a
  moving branch. Merging anyway would leave a failed check on the record for the
  gate the owner asked to keep.
- **Leave `main` behind until the release-candidate cut.** Viable, but leaves
  two branches to reason about.
- **Force-reset `main` to `v1-development`.** Rejected: unnecessary, and
  rewriting a protected branch.

## What is not changed

- No commit is rewritten; every commit ID is unchanged, so the CI results and
  reviews already attached to `v1-development` commits apply to `main`.
- The ruleset is not modified. The contributor sign-off gate is unchanged for
  new commits.

## Verification (recorded in the tracking issue after execution)

```sh
git fetch origin
git rev-parse origin/main origin/v1-development      # equal at the moment of the move
git merge-base --is-ancestor <old-main-tip> origin/main   # 31da60b66 preserved
git merge-base --is-ancestor origin/v1-development origin/main
gh api repos/rhamenator/Project-Copperfin/rulesets/20356131   # unchanged vs. snapshot
gh run list --branch main                                # push-triggered workflows
```

## Afterwards

`v1-development` keeps moving through pull requests, so `main` will trail it
again until the next deliberate synchronization (planned at the release-
candidate cut). This record's execution results are posted to issue #6676.
