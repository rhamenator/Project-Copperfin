# VFP9 `DESCENDING()` real-environment observation (issue #5358)

Retained evidence from running `tests/fixtures/vfp9_descending_observation.prg`
in a real, installed Visual FoxPro 9 environment, per `docs/07-clean-room-rules.md`
and `docs/32-recovered-requirements-traceability.md`'s allowed-evidence rules.

## Environment

- Visual FoxPro 9, version `9.0.00.7423` (SP2 + Hotfix 3, the "latest fully
  patched version" per the VFPX `VFPInstallers` README) -- `VERSION()` output
  confirmed via `vfp9.exe`'s own file version resource before this run.
- Windows 11 Pro, on a local libvirt/KVM VM built for this purpose.
- Run 2026-09-04, launched non-interactively via `vfp9.exe -c<config.fpw>`
  with `COMMAND=DO run_and_quit` (a thin wrapper that calls the observation
  script then `QUIT`), rather than through the interactive IDE.

## Files

- `descending-observation.tsv` -- the fixture's own recorded observations.
- `descending.dbf`, `descending.CDX`, `descending.idx` -- the exact generated
  table and index files the observations were captured against.

## What was captured vs. not

8 of the fixture's 9 planned observation cases completed and were recorded.
The final case (`active-idx-runtime-descending`, a runtime `DESCENDING`
override applied to a single-tag `.idx` file via
`SET INDEX TO ... ORDER ... DESCENDING`) did not complete in this run, and a
follow-up isolated retry (wrapping just that command in `TRY/CATCH`) also did
not produce output before the process exited non-interactively without
writing its result. Given no interactive terminal was available to observe
what actually happened (dialog, silent abort, or something else), this case
is recorded here as **not captured** rather than guessed -- retry with an
interactive session before relying on any assumption about single-IDX
runtime-override behavior.

## Observed contract (from the 8 completed cases)

Reading `descending-observation.tsv`:

| Case | `DESCENDING()` | Interpretation |
| --- | --- | --- |
| `no-active-order` | `F` | No active order -> `.F.`, not an error. |
| `active-ascending-tag` | `F` | Active tag created without `DESCENDING` -> `.F.`. |
| `active-persisted-descending-tag` | `T` | Active tag created *with* `DESCENDING` -> `.T.`. This is the core persisted-direction contract issue #5358 exists to recover. |
| `active-ascending-tag-runtime-descending` | `T` | A tag persisted *ascending*, but with `SET ORDER TO TAG ... DESCENDING` applied at runtime -> `.T.`. The runtime override wins over the persisted direction. |
| `active-persisted-descending-tag-runtime-ascending` | `F` | A tag persisted *descending*, with a runtime `SET ORDER TO TAG ... ASCENDING` override applied -> `.F.`. Runtime override wins here too, in the opposite direction. This override was **accepted without error** -- notable, since the shipped VFP9 help topic (`DESCENDING( ) Function`) documents the `DESCENDING` runtime override but is silent on whether an inverse `ASCENDING` override is legal; it evidently is. |

So the zero-argument `DESCENDING()` contract is well-evidenced: no active
order is `.F.`; otherwise the *runtime override* (if one was applied to the
active order) wins, and absent an override the *persisted creation
direction* of the active tag applies.

The two-argument form (`DESCENDING(cCDXFileName, nTagNumber)`, probing a
tag's direction without making it the active order) was also exercised in
this run (`persisted-tag-1/2/3` cases). This README originally recorded that
data as uninterpretable -- `TAGCOUNT()` reported 3 tags where only 2 CDX tags
were explicitly created, and the paired `TAG(nTagNumber)` labels didn't line
up against the `DESCENDING(lcCdx, nTagNumber)` values. PR #5488 review
(`chatgpt-codex-connector`) correctly identified why: the fixture's loop uses
one shared `lnTag` variable for two functions with *different* ordinal
scopes. `TAG(nTagNumber)` uses the global open-index ordinal from
`RQ-CF-PRG-010` (all open `.idx` files before `.cdx` tags) -- the fixture
also opens `descending.idx`, so `TAGCOUNT()=3` is exactly the `.idx` +
`AscTag` + `DescTag` count, and `TAG(1)` correctly returns `"DESCENDING"` as
the uppercase base name of `descending.idx`, not a tag literally named that.
`DESCENDING(lcCdx, nTagNumber)`, by contrast, is scoped to just that CDX's
own two tags. Read that way, the three recorded values are fully consistent
and informative:

| `nTagNumber` | `DESCENDING(lcCdx, nTagNumber)` | Interpretation |
| --- | --- | --- |
| 1 | `F` | CDX's own tag 1 (`AscTag`) -- ascending, as created. |
| 2 | `T` | CDX's own tag 2 (`DescTag`) -- descending, as created. |
| 3 | `F` | Out of range for a 2-tag CDX -- safe non-error default, not an error. |

So the two-argument form's contract is now evidenced too: it reports the
persisted creation direction of the `nTagNumber`-th tag *within that CDX
file*, independent of any open `.idx` files' own global ordinal position,
and returns `.F.` rather than erroring for an out-of-range tag number.

## Implementation status

**Not yet implemented.** The observed contract above requires reading a
tag's *persisted* creation direction out of the CDX tag header for the
non-override case, and Copperfin's current CDX descriptor model does not
retain that bit (`runtime order state begins with descending=false for
loaded CDX tags`, per issue #5358). Attempting to empirically locate that
bit against `descending.CDX` in this session was abandoned: `copperfin_inspect`'s
existing heuristic tag-name/offset detection produced garbled, overlapping
results against this specific (very small, 3-record) fixture, which is not
a safe basis for pinning down a byte offset -- doing so risked exactly the
"guessed encoding" outcome `docs/32`'s rules prohibit.

Next steps before implementation:
1. Generate a larger, more realistic fixture (more records, more varied key
   data) so `copperfin_inspect`'s tag-header heuristics resolve cleanly, and
   re-run this same correlation with a trustworthy hex-level reference.
2. Independently create additional known-direction tags (single ascending,
   single descending, in isolation, one per CDX) to isolate exactly which
   header bits differ between them, rather than reading a 2-tag file's
   heuristically-detected offsets.
3. Only then implement the persisted-direction read path, with a
   regression test asserting it against this real fixture data.
4. Retry the single-IDX runtime-override case (see above) with an
   interactive session to see what's actually happening there.
