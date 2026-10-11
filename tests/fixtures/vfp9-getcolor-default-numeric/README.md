# GETCOLOR Numeric default-color requirement recovery

Pre-helper/pre-test mapping recorded 2026-10-10T22:51Z against admitted
#5611/#6776 and main `7a97ac526d0d9b684fac9c08835b267a3c659fd9`.
Only optional first-argument Numeric/exact conversion is selected. No helper,
runtime assertion, CMake change or production migration existed at recovery.
Existing Copperfin code/tests are verification context, never requirements.

## Independent allowed evidence

Installed Visual FoxPro 09.00.0000.7423 under Wine was observed on a task-owned
private Xvfb display, without starting a VM or changing fonts/system/credentials.
`probe.prg`, `probe.fpw` and `observe.sh` retain the clean-room invocation.
The observer sends the declared Return/Escape only to Color dialogs on that
private display, waits for dialog readiness and bounds repeated key attempts.
All native workloads were harvested; none remains suspended.

Serial complete attempts4/5 each produce106 lines/52 calls including VERSION
and DONE. `native-1.out` and `native-2.out` are unedited copies verified with
cmp and identical SHA-256
`780bcb175588b2178f9e03caf4f70e969b48dae0574b95c9562f220959154ffa`.
PRG SHA-256 `16bcdd3c755a840b96b653b70affba6716787f65583eb072161591cb95e9d5c3`.
Full observer logs retain warnings/repeated-key attempts, not warning-free
qualification. SOURCE/DIALOG records tie the two PNGs to255.9 and the negative
red alias. Returned native numbers, not screenshot interpretation, are the
fractional/low-bit oracle.

Native Return yields255 for255.49/.5/.9;256 for256.9;16777215 for16777215.9.
Negative sub-unit values yield0. Negative-1/-1.49/-1.5/-1.9 yield16777215.
16777216 and +/-2147483648 yield0;2147483647/-2147483649 yield16777215.
4294967295 and its.1/.5/.9 fractions yield16777215;4294967296 yields0.
-4294967041 yields255 and-4294967040.5 yields256. Huge finite +/-1E20,
+/-1E300 and +/-EXP(1000) yield0. Omitted input yields0; Escape yields-1.
Both observations support truncation followed by low24 RGB projection,
not half-away rounding or GETFONT's positive raw admission ceiling.
Native Logical/NULL/Character controls error11; Currency255.9 yields255.
Those are retained out-of-scope type evidence, not authority to migrate them.
Native NaN/exact extended kinds are not expressible/distinguished here.

Shipped VFP9 SP2 Microsoft GETCOLOR help was read completely from
`/home/rich/temp/vfp9-help-len/html/532d427d-c647-402b-9854-60f24d70adfe.htm`,
SHA-256 `61c6c8cd5ae44f5243fb6e24e467ec34e05e75dd475221010e2d138baa94e100`.
It documents Numeric chosen-color output, omitted black, initial color and
first-color fallback, and Cancel/Escape -1. It does not specify fractional or
integer-conversion mechanics; observations above provide that evidence.

Excluded attempts retain raw outputs/observers/source/configuration in this
fixture, with additional diagnostic scratch in build/getcolor-native: attempt1
SCREEN=OFF put Color off-screen and timed out124 (two-line partial output,
first-offscreen source/config/stderr and black diagnostic image). Attempt2
was accidentally started before attempt1 was harvested, despite serial intent;
its complete106-line output is excluded due to overlap, not clean evidence.
The attempted exact-owned-PID termination found attempt2 already completed0;
no foreign process was terminated. Attempt3 with SCREEN=ON stalled after a
too-early Return at-1 and timed out124. Sources/observer/partials are retained.
Before any runtime assertion existed, the observer was changed to wait0.15s
and retry at most five times only while the last output line is BEGIN.
Clean serial4/5 then completed0 with unchanged PRG and matching outputs.
Excluded overlap output happens to have the same hash; identical output does
not cure its non-serial provenance. Generated probe.FXP is scratch, not source
or a decompilation input, and is preserved only under the ignored build tree.

## Governing bounded requirement and extension boundary

`RQ-CF-PRG-GETCOLOR-DEFAULT-NUMERIC-001` derives from
`RQ-CF-PRG-NUMERIC-BEHAVIOR-001`, owner-approved #5611/#6776, native/help
evidence above and direct owner extension intent2026-10-06.

Default COPPERFIN truncates finite Numeric input toward zero and admits only
an RGB integer0..16777215. Thus fractions in(-1,0) normalize0, but -1 or
16777216 and non-finite values raise existing localized catchable11. The
range is a derived unsigned24 RGB safety boundary: modern mode must not
silently alias oversized/negative colors, not a native admission limit claim.
Exact int64/uint64 must be checked directly0..16777215 without a Double
round trip. Explicit VFP9 uses the parent's defined signed-low32 conversion,
then masks low24 RGB bits; huge/non-finite/NaN Numeric indefinite becomes0.
Infinity/huge and negative wrapping are observed; NaN and exact-kind low bits
are explicitly parent-derived, not invented native GETCOLOR observations.

The checked RGB initial request supplies both the Numeric headless result and
the default/result event fields. No native dialog or simulated Cancel is
implemented: the headless extension returns the checked initial color, not a
claim of host selection or complete dialog/palette parity. This boundary is
derived from owner extension intent, not inferred as a requirement from old
llround output. Rejection precedes event publication/destination assignment;
no allocation, argument-sized iteration, cursor/setting change occurs.
Omission remains black0; title extension and existing non-Numeric/Logical/
NULL/Character/Currency/arity behavior remain unchanged. No GETFONT/shared
conversion/other consumer migration or new KBX exception is selected.

## Architecture and verification mapping before implementation

Plan distinct `checked_getcolor_default_numeric_argument` in helpers.h/.cpp,
UNUSED by production through BOTH complete frozen-original compiler results.
Only then gate Numeric/int64/uint64 first input in the GETCOLOR dispatcher;
preserve other argument paths and error machinery. Source/test comments and
the durable matrix link this RQ back to native/help/parent evidence.

`VR-5611-GETCOLOR-DEFAULT-NUMERIC-001`: independent literal direct boundaries,
nextafter/subnormal/NaN/exact64 including precision-loss aliases, both modes;
do not derive expectations from this helper or current runtime.
`VR-5611-GETCOLOR-DEFAULT-NUMERIC-002`: fresh both-mode/session public shards,
literal result/kind/complete event, error11/sentinel/no-event atomicity,
continuation, cursor and neighbor-policy/reset, four localized catalogs;
omitted/title/non-Numeric controls. Unchanged GETFONT/exact/numeric/isolation
neighbors, full raw original/fixed GNU and Clang ASan/UBSan/float-cast-overflow
results; broader unchanged runtime/localization/contracts and hosted full
regression remain required. Freeze before configuration. Any input/assertion
repair after freeze requires explicit refreeze and BOTH originals again.
No original sanitizer diagnostic is inferred from uninstrumented llround.

## Documentation assurance and planned walkthrough

`DQ-5611-GETCOLOR-DEFAULT-NUMERIC-001` requires documenting mode/range, exact
types, atomic errors, headless/native and unchanged-type boundaries plus safe
reproduction/rollback. `DV-5611-GETCOLOR-DEFAULT-NUMERIC-001` verifies those
procedures against retained evidence and development walkthrough results.
Links: active HZ-runtime-crash-01, HZ-data-corruption-01, HZ-doc-command-01;
registered severities/status are not changed. Bounded documentation misuse
severity medium: treating a wrapped color or headless result as native selected
color may mislead application output, without changing persisted data here.

Procedural delta: modern255.9 normalizes255;16777216/-1/nonfinite become
catchable11 before events. Only explicit VFP9 retains modulo color aliases.
Planned walkthrough verifies255.9=>255 both;16777216 modern11/legacy0;
-4294967041 modern11/legacy255;Infinity modern11/legacy0, public atomicity,
session/reset and four locale messages. Development maintainer recovery
self-review and both-compiler automated bounded walkthrough completed; signed-
head hosted/review acceptance remains gated, with no parent closure claim.
This is not independent qualified-human/high-hazard sign-off or release
qualification. Rollback: revert the bounded slice after reproducing a false
boundary; notify affected callers of the exact mode/input/revision and retest
saved scripts, rather than silently enabling legacy mode or editing assertions.

Reproduction requires installed VFP9 and this task-owned private display;
adapt absolute source/config/output paths only, preserve raws before repeats,
harvest one workload before starting another. Never run observer on the user's
desktop or start/recreate VMs while backup-reset completion is unconfirmed.

## Current verification state (2026-10-10T23:34Z)

The [frozen input audit](baseline-audit.md) records all five pre-configuration
hashes, original/fixed full raw hashes, exclusions and scheduling chronology.
Complete original GNU and Clang each9/13 with758 selected GETCOLOR assertions
failing/zero other failures and all174 direct/four80 public counters complete
were retained BEFORE Numeric-only production migration23:21Z. Original Clang
has no sanitizer report; its full warnings remain in the raw.
Fixed GNU13/13 PASS with complete counters verifies the bounded walkthrough,
atomicity/catalog/session controls and untouched GETFONT/exact/numeric/isolation
neighbors. No assertion/input/helper/config repair or refreeze occurred.
Fixed Clang halt1 also13/13 PASS, complete counters and no sanitizer reports;
leaks are disabled, so this is not leak qualification. The five documentation
contracts have combined completed results: initial4/5, sole missing-date
fragment-format failure retained; only fragment text repaired, unchanged
assembler/fragment checks2/2 PASS. No invented single5/5 run or frozen-input
repair. Applicable development DQ/DV walkthrough verifies documented RGB
boundaries, failure atomicity and rollback/misuse limits, not independent-human
or release acceptance. Signed-head/hosted regression and actual exact-head
external review remain required. All owned workloads are harvested.
Foreign builds/tests started after owned idle-inventory launches; overlap is
disclosed in the audit, no timing/idle-performance claim is made, and no foreign
process was modified. All raw original/fixed/failed/repaired logs remain full.
