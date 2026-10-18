# Constant-division public verification chronology

Governing requirement: RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-MATERIALIZATION-001,
derived from the intrinsic/typed division parent and admitted #5611/#6776/#7189.
Native boundary/timing contract and846-row independent header were signed in
5ef64f9357bc3d3657666c9da7d230d429601532 before public test/parser integration.
That checkpoint has NO parser hook; the intrinsic helper is still unused.

`test_prg_engine_numeric_constant_division.cpp` checks798 native producer-domain
rows at five DECIMALS settings, FIXED OFF/ON and both NUMERICBEHAVIOR modes:
15,960 descriptor/bit pairs. All48 further native rows remain in the header;
the test explicitly verifies their existing unknown metadata in each context,
not compatibility. Additional unsupported non-whole/small-divisor/other-operator,
NULL/type/zero-divisor/Currency/suppression/watch-state controls are fail-closed.
Three callback expressions per mode pause inside a UDF which changes DECIMALS
14->0: grouped, ungrouped and chained prefixes. Require quotient1.25, retained
native scale14, final setting0, actual pause/resume and exactly1 callback.
Normal scalar assignment can normalize width, so this callback check does NOT
claim to observe the raw expression width.

Ungrouped timing is independently established by `constant-continuation.prg`:
paired80 values/run, actual0/empty stderr/no caught errors, all callbacks1,
source/framing/typed/repeat/compiler equivalence0 and raw byte equality. Exact
SHA5779e236c74ee68cd8cfa926bebaf356959365c387a75b8cfc6ff57e9d15975b.
The source/ownFXP/cases and lossless captures are retained. The single-evaluation
auditor now accepts either the original16-case effects inventory or this exact
four-case inventory; original320-case captures still audit0 unchanged. This
reveals an architecture constraint: save completed division-node precision as
well as primary checkpoints, otherwise an ungrouped prefix may re-query the
setting after a callback on replay. No implementation acceptance is claimed.

Before the ORIGINAL baseline, the prior runtime division test's six explicit
unknown assertions for newly admitted constant routes are replaced by the
stronger native expectations above; the other three unknown controls remain.
No runtime arithmetic, expected native fields, tolerance, assertion weakening,
timeout or allocation/conversion boundary is changed. Register the two named
tests in CMake, explicit filesystem/environment isolation and BOTH sanitizer
build/filter paths; the exact-name selector inspection passes3/3. Registration
alone is not hosted execution evidence.

Pre-hook source/hash freeze is retained at host
`build/constant-division-native/pre-hook-public.sha256`. Earlier source before
adding the ungrouped/chained callback controls and its hash list also remain on
the host; no ORIGINAL result preceded that strengthening. Configure is actual0
using GNU Debug -O0 -g0, dedicated `build/constant-division-original`, parallel2.
Public test/registrations were signed c7e287ba1f0abcbbbdcd0b78226b5bc59418eb84
BEFORE runtime hooks. Actual configure/build/final no-work build all0. ORIGINAL
CTest exits8: public constant test fails; prior runtime division, intrinsic
helper, exact-name sanitizer-selection and native isolation contracts pass4/4.
Public summary:15,966 descriptor failures (15,960 table pairs plus6 callback
precision assertions),660 native-bit failures, evaluation0/control0, total16,626.
All660 expected bit mismatches are signed zeros:440 native positive zero and220
native negative zero. No nonzero expected bits fail. Actual callback pauses,
once-only counts, quotient, final setting,48 retained gaps, unsupported routes
and watch/resume controls pass. Raw log and JUnit are retained losslessly in
`constant-public-evidence/`; audit classifies, never changes native constants.
The full pre-hook source/header/contract/registration SHA list still verifies0
after baseline. Next require SAME frozen tests passing after bounded integration;
do not revise constants to fit code.
All broad/local/hosted/exact-head-review/strict/resolved-conversation gates and
remaining producer/non-whole/display/conversion/ListItem obligations remain.

MEDIUM DQ/DV delta and hazards/rollback follow the materialization contract.
This is development self-review/automation, not independent qualification.
