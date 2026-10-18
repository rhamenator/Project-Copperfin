# Decimal-literal payload recovery

Tracking #7206, derived from admitted #5611/#6776/#7189 and the direct owner
Numeric compatibility policy. Requirement
`RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001` is a **recovery gap**.
Baseline `ef3ea326f78d640546363abe97bbe27b6208bf45`. No runtime implementation,
accepted conversion equation, native parity or new source grammar is claimed.

## Preserved dependency and exact original

The unfinished non-whole division slice is explicitly deferred, not cancelled:
branch `fix/numeric-nonwhole-division-5611`, signed/pushed head
`11bd096a6232a86aee10074e8e35f7daca19cbb6`. Its native/public freeze274a38635
preceded actual ORIGINALafc488bb0. Two bare literals already differ before
division (both modes/all tested settings):

| Literal | Unchanged Copperfin | Actual VFP9 |
| --- | --- | --- |
| `1.234567890123456789` | `3FF3C0CA428C59FB` | `3FF3C0CA428C59FC` |
| `0.9999999999999999` | `3FEFFFFFFFFFFFFF` | `3FF0000000000000` |

Standalone width/scale/storage passes. The independent original audit attributes
all2,280 payload failures to these two inputs;11,040 descriptor failures remain
division work, evaluation/control0. Do not correct quotients, drop spellings or
weaken the retained578-row native header/574 exact assertions. After this
prerequisite integrates, refresh that ORIGINAL, finish fractional native
callback/continuation timing, freeze stronger former-unknown controls, then
bounded division implementation/SAME and all integration gates. Other mandatory
Numeric/display/exact-conversion/ListItem work remains uncancelled.

The prior metadata-only literal contract preserved the existing parsed payload
without rounding to the display scale. That did not prove the existing source
conversion was native-compatible. New conversion recovery must distinguish that
earlier bounded metadata guarantee from a general Numeric payload requirement.
Do not globally change `platform::try_parse_invariant_double`, which serves
unrelated invariant-data parsing. #5879 leading-decimal grammar and #7114 binary
literals are not selected work.

## Pre-capture profile

`emit-grid.cjs` creates only native source/inventory, without an expected value
or Copperfin import.245 unique plain-decimal spellings, four pure contexts
(bare literal, lexical-minus, group-minus, IIF) ->980 complete expressions.
DECIMALS0/18 x FIXED OFF/ON x compiled/EVAL ->7,840 observations per run.
Controls vary adjacent integer coefficients around2^53 and long decimal
boundaries, decimal placement, leading/trailing source zeroes representing the
same rational, and coefficient/fraction lengths1..32. This is a bounded
requirements-recovery profile, not a claim that a width alone determines a
future implementation eligibility frontier. All rows remain in the inventory.

Native outputs capture typed public-FLL fields, independent BINTOC BRS bits and
display bytes. These are pure expressions replayed for the three observations;
there is no callback/single-evaluation claim. `audit-grid.cjs` validates complete
source/framing/inventory, actual typed fields, FLL/BINTOC agreement and repeated
records; unused native parameter slots are checked separately. Compiled/EVAL
differences are reported, not assumed away. Character display hex may be empty;
that is not a missing Numeric value. No expected metadata/payload comes from a
conversion hypothesis, current Copperfin or a separately evaluated folded child.

REA tools/CLI are unavailable. Supported ordinary-shell black-box observations
are used, not REA Evidence IDs; no configuration/tool installation, native
disassembly/decompilation or proprietary implementation copying. Installed
VFP9/existing public-API FLL/runner/source identities are retained with captures.
No VM or heavy build is needed for this evidence phase. Probe timing must be
coordinated with the audit/DAW chats and deferred when host contention returns;
no backup/upload action is in scope. Preparation itself is not native execution.

## Required next evidence

### First profile actual observations and rejected models

The first paired profile returned0/empty stderr, all7,840 values/no errors/run.
Raw byte-identical SHA-256
`589f03edf5a2d5a36c7a46bc84f6b5431c5aaa209dff6e73478eee01c1dcce94`;
independent typed/BINTOC/repeat audit0, compiled/EVAL differences0. Full raw,
statuses, source/own FXP, hashes, auditor and candidate results are retained.
The two native runs were launched only after coordinated pressure fell (full
avg10 about0.02% between runs); both chats were told the window was released.
No VM or heavy build was used. The owner's report of Chromium background I/O
is context, not proof of the cause of any earlier timeout; no browser or backup
action was taken.

Among245 bare spellings,60 native payloads differ from direct nearest-binary64
decimal parsing. Eight first structural hypotheses all fail (41..97 mismatches
for the alternatives,60 for direct parsing). The initial108 chunk/order models
also all fail (best27). Their complete counterexamples and original script are
retained, not replaced by a successful subset. The initial model manifest binds
that archived script version, not the later extended source.

One refined **candidate**, not an accepted general rule, matches all245 bare
values: separately accumulate whole and fractional digits with binary64 Horner
steps, build the fractional divisor by repeated multiplication by10, then add
whole + fraction/divisor. Replacing that divisor by a directly parsed power
of ten fails six long-fraction cases. No per-spelling correction, digit cutoff,
tolerance or metadata-as-payload-rounding rule is used. This is a clean-room
behavioral hypothesis, not a claim about proprietary implementation internals.

Same exact decimal rational/source-spelling controls are significant:
`3.9999999999999996` -> native`400FFFFFFFFFFFFF`, but
`3.999999999999999600` -> native`4010000000000000`;
`0.1234567890123456789` -> native`3FBF9ADD3746F661`, while its trailing-zero
spelling`0.12345678901234567890` -> native`3FBF9ADD3746F660`.
These repeat compiled/EVAL/all tested settings. Do not canonicalize the source
spelling first or rationalize away a native payload difference. These remain
the same7206 source-conversion prerequisite, not new independent defect scope.

### Pre-observation held-out profile

`emit-holdout.cjs` prepares213 bare source literals (198 new spellings,
15 explicitly marked calibration overlaps), including independently seeded
coefficients/decimal placements and length/scale/source-zero boundaries up to
256 bytes. No sign/wrapper expression-length extension is inferred at that
boundary. Long expression strings are concatenated from short literal chunks
without changing their bytes. The native compiled cases remain complete bare
Numeric literals. Profile count is1,704 observations/run under the same eight
compiled/EVAL/setting/FIXED contexts.

`holdout-predictions.json` is explicitly labelled **PRE-CAPTURE HYPOTHESIS, NOT
ACTUAL NATIVE EXPECTATIONS**. It and the candidate/source/inventory/auditor are
hashed before held-out execution. Never use that file to generate public native
expected tests; compare it against independently audited actual results only.
The held-out source, inventory and prediction bytes reproduce exactly from the
emitter. They must stay frozen while awaiting the resource window. Preparing
them does not establish that the candidate covers the accepted256-byte producer
domain or that any Copperfin runtime behavior has changed.

Run paired unchanged sources only in the coordinated low-contention window;
retain actual process status/stderr, complete native result, source and own FXP.
Then audit before analyzing conversion hypotheses. Exact agreement with a
candidate over this finite profile is not a universal rule; keep counterexamples
and recover missing boundaries rather than guessing a digit cap or epsilon.
Freeze independent native descriptor/payload public ORIGINAL tests before any
Numeric producer change, then SAME/broader/actual named hosted execution and
exact-head clean review/strict/resolved-conversation gates. A clean requirements
checkpoint is not a green implementation PR and does not close7206 or its parents.

Documentation assurance `DQ-DV-7206-DECIMAL-PAYLOAD-RECOVERY-001` (LOW): the
procedural delta is source-conversion recovery before changing arithmetic or
consumer behavior. Misuse would mistake display precision, a two-spelling match
or added metadata for exact payload parity. Development self-review walkthrough
tracks the bare-literal original -> independent native profile -> explicit gap
-> future frozen acceptance. This is not independent-human/release assurance.
Rollback preserves captures and the division re-entry while reverting an
unaccepted candidate; no shipped runtime changes exist here.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01, #6776 per-area modes,
checked conversions/finite/allocation limits and exact64/Currency separation
remain parent constraints, not altered by this profile.
