# GETFONT Numeric font-size requirement recovery

Pre-helper/pre-test mapping recorded 2026-10-10T19:26Z against admitted
#5611/#6776 and main `14e6188406a8e4289eccee5b6fe50614eba07792`.
Only the optional second argument's Numeric/exact conversion is selected.
No helper, assertion, CMake change or production migration existed when these
requirements were recovered. Existing Copperfin `llround` and smoke tests
are verification context, never the requirement source.

## Independent allowed evidence

Installed Visual FoxPro 09.00.0000.7423 under Wine was run in a task-owned
isolated Xvfb display. No VM, font installation, system maintenance, table or
credentials was used or changed. `probe.fpw`
and the complete clean-room PRGs retain the invocation. `observe.sh` sends
only the declared Return/Escape action to exact visible Font dialogs on that
private display. Its EXIT trap terminates only the owned VFP process.
All native sessions were bounded and harvested; none is suspended.

`initial-boundary.prg` has 28 calls. `native-1.out` and `native-2.out` are
identical complete 59-line observations, including VERSION, FONT and DONE;
SHA-256 `6ec310d8e895e67155a9694e121f1a96b0ee4fa9f0accbd45d5e7c00ae70f1a1`.
`probe.prg` adds 19 controls. `supplement-1.out` and `supplement-2.out`
are identical complete 97-line/47-call observations;
SHA-256 `d84990084a74e870b334ecd6d4b6043857cbb7fc6f27127ce12d01c5f83eb9ec`.
PRG SHA-256 `ccfaf61a570e3ae8822324f9a9fe96b2ab473f20b3e17588e5c17dff840dfa74`.
Full observer logs retain their XGetInputFocus warnings, not a warning-free
qualification claim. Native output was copied unedited and compared with cmp.

The selected installed non-vertical font is Adwaita Sans. OK independently
returns size12 for 12, 12.49, 12.5 and12.9; size4/127 controls return4/127.
Zero, -1, -0.5, -0.9, -1E300 and -EXP(1000) return size10. The wrapped
negative aliases -4294967284 and -4294967283.5 return12 and13, respectively.
Positive4294967295 is admitted; every observed positive fractional value
above that inclusive ceiling, 4294967296, 1E20, 1E300 and EXP(1000) raises11
without a dialog. Negative oversized values are admitted. Cancel returns
empty Character; admission alone does not reveal the converted integer.

The retained `subunit-0.5.png`, `positive-wrap-2147483648.5.png` and
`positive-uint32-max.png`, tied to SOURCE/DIALOG entries in both supplemental
observer logs, show the native dialog's size10 initial field. The small
positive sub-unit defaults to10; positive signed32 overflow can also default
to10 after legacy wrapping. Other enormous displayed sizes reflect native
dialog/backend arithmetic; their Cancel output is NOT proof of an exact
converted point count. No full Font-dialog parity is inferred.

The shipped Microsoft VFP9 SP2 GETFONT reference was read completely from
`/home/rich/temp/vfp9-help-len/html/53a11bb9-6a3b-4539-a343-ffd75ae45dbe.htm`
(SHA-256 `9262f4d19fb4daf30c58010cfa1c3d4130166548a3817b01b2892137cfb46cc2`).
It documents native interactive selection, name/size/style return, empty
Cancel, nonpositive/omitted size default10, selectable sizes4..127, and
admission of other caller-specified values with validation on OK. It does
not prescribe a fractional rounding rule or integer representation. Native
observations above supply fractional and argument-admission evidence.

## Governing bounded requirement

`RQ-CF-PRG-GETFONT-SIZE-NUMERIC-001` derives from
`RQ-CF-PRG-NUMERIC-BEHAVIOR-001`, owner-approved #5611/#6776, the allowed
native/help evidence above, and the owner's 2026-10-06 extension intent.

In default COPPERFIN, reject non-finite Numeric values with localized,
catchable error11. Finite nonpositive values normalize to documented10
without an integer cast, including huge negative values. Positive values
truncate toward zero; truncated zero normalizes to10. Admit a positive
truncated value only if it fits signed32: binary64 input must be strictly
below2147483648. Exact signed/unsigned64 positives must be at most2147483647,
tested directly without a Double round trip. An oversized positive request
is a catchable11, not an accidental negative/default font size. This signed32
request domain is a derived modern safety boundary, not a native limit claim.

Only explicit VFP9 mode retains the parent's defined signed-low32 conversion
model, followed by default10 if the converted value is nonpositive. Preserve
GETFONT's independently observed positive admission ceiling BEFORE conversion:
positive binary64 values above4294967295 (even when truncation would fit)
raise11; exact positive integers above4294967295 raise11 directly. Negative
finite inputs use the defined legacy conversion; -4294967284/-4294967283.5
must produce12/13. Negative infinity and NaN use the parent's defined
indefinite-to-zero model, hence10. Negative infinity is independently observed;
NaN and exact extended-kind behavior are explicitly parent-derived because
this installed VFP probe does not distinguish/express them. Do not invent a
GETFONT-specific NaN observation or infer low bits from an empty Cancel result.

Both modes must convert exact extended integers directly. Rejection occurs
before `runtime.getfont` callback or destination assignment; no selection,
allocation, cursor/setting mutation or argument-sized loop occurs. Ordinary
positive fractional inputs follow recovered truncation, not llround.

Copperfin's existing headless name result and event are an intentional
extension boundary. The event records the checked initial request size;
it does not claim native selected-point/backend arithmetic parity. Omitted
second argument keeps its no-size-field behavior. Current non-Numeric,
Logical, NULL, Character, Currency, name/style/charset/arity behavior and
other consumers remain unchanged. Native .T./NULL error11 observations are
retained as out-of-scope type evidence, not permission to alter those paths.
No GETCOLOR/dialog implementation/font availability change is selected.

## Architecture and verification mapping before implementation

Plan a distinct `checked_getfont_size_numeric_argument` in
`src/runtime/prg_engine_helpers.{h,cpp}`. It remains UNUSED by GETFONT through
BOTH full original GNU and Clang runs. Only afterwards migrate Numeric
second arguments in `prg_engine_runtime_surface_dispatch_general.inl` to the
optional checked result and existing localized error11 machinery.

`VR-5611-GETFONT-SIZE-NUMERIC-001`: independent direct helper literals for both
modes, positive/negative fractions, native aliases, signed32/uint32 edges,
adjacent binary64 values, huge/nonfinite values, exact kinds above2^53 and
64-bit extrema. Assert independent literal totals, not helper-derived results.
`VR-5611-GETFONT-SIZE-NUMERIC-002`: fresh public sessions in both modes,
literal event-size expectations, catch11 with sentinel/no-event preservation,
continuation, locale/session/cursor/reset isolation, omitted argument and
unchanged non-Numeric/name/style controls. Retain current GETFONT smoke and
neighboring runtime/numeric/localization contracts without assertion repair.
Freeze helpers/header/tests/CMake/isolation configuration before configuration;
retain BOTH complete original results before migration. Any assertion/input
repair requires explicit refreeze and repeat of BOTH originals. Fixed focused,
broader and sanitizer results, signed exact-head hosted checks and clean
Claude-first/authorized-Codex review remain required. They are not yet claimed.

## Documentation and hazard boundary

`DQ-5611-GETFONT-SIZE-NUMERIC-001` requires separating input admission,
conversion, native UI validation, extension event/request semantics and actual
acceptance evidence. `DV-5611-GETFONT-SIZE-NUMERIC-001` maps to the complete
native-source/output/help comparison, independent boundary assertions, fixed
error/continuation walkthrough and exact-head gates. Pre-code recovery review
is complete; fixed implementation/verification walkthrough remains outstanding.
No independent-human/high-hazard qualification or release acceptance is claimed.

Linked active hazards: HZ-runtime-crash-01 (undefined conversion/host failure),
HZ-data-corruption-01 (silent wrong request/event or partial assignment),
HZ-doc-command-01 (mistaking headless admission or Cancel for UI parity).
Registered severities/status are unchanged. Bounded documentation misuse is
medium. Procedural delta: validate positive sizes; catch11; use VFP9 only for
intentional legacy aliases; never infer native dialog parity from the setting.
Walkthrough targets: 12.9 becomes12; -4294967284 is10 by default and12 inVFP9;
4294967295.1 fails both before event/assignment; -Infinity fails default but
is10 only under the explicit legacy mode. Rollback is reverting this slice
while retaining exposing regressions/evidence; corrections go in changelog
and release notes. No parent/family/type/VM/package/release issue closure.

## Implemented boundary and retained focused verification

The pre-code requirement above is unchanged. The helper, independent literal
tests and isolation/CMake inputs were frozen before configuration, and BOTH
complete GNU/Clang originals were retained BEFORE the sole Numeric/exact
second-argument dispatcher migration. Each original passes9/13 registered
tests with486 selected public assertion failures and zero other failures.
All172 direct/four79 public and unchanged INT16162/four152 counters complete.
No original sanitizer diagnostic is inferred from the uninstrumented llround.

Fixed GNU and Clang each pass13/13 with identical complete literal counters,
no assertion repair/retry and five frozen input hashes unchanged. The Clang
ASan/UBSan/float-cast-overflow halt1 run has no sanitizer reports. Full original
and fixed configure/build/CTest raws, copy/cmp/hashes and chronology are in
[baseline-audit.md](baseline-audit.md), not filtered success-only excerpts.

The bounded development DQ/DV walkthrough now confirms12.9=>12,
-4294967284=>10/default and12/VFP9, positive4294967295.1=>catch11 in both,
and negativeInfinity=>catch11/default versus10/VFP9. Every public check
validates event/assignment atomicity, continuation, cursor/policy/reset and
the unchanged name/style/omitted/non-Numeric paths; four catalogs check the
literal localized error11. This is development recovery/self-review plus
automated verification, NOT independent qualified-human/high-hazard sign-off.
Broader unchanged GETFONT smoke/localization/contracts, hosted full regression,
signed exact-head Claude-first/authorized-Codex review, all required checks
and resolved conversations remain integration gates. No such pending gate,
full native UI parity or release/family/parent acceptance is claimed here.

## Excluded native attempts and reproduction precautions

`first-fallback.prg/.out` and its observer log retain the first12-call run
with unavailable Arial, which selected @Droid Sans Fallback size6 regardless
of requested ordinary size. It is NOT a fractional-size oracle. Attempts2/3
were blocked by VFP STRTOFILE's overwrite confirmation before new output;
their timeout/prompt screenshot is retained under build/getfont-native.
Attempt4 accidentally overlapped attempt3 and was terminated (both owned
sessions harvested143); it is explicitly excluded, not clean native evidence.
SET SAFETY OFF was then added only to the isolated probe session; clean
serial attempts5/6 yielded the28-call pair. Attempt7 reached an invalid-size
OK alert on -2147483649 and timed out; its incomplete output/source/log and
screenshots are retained, not completion evidence. That probe action was
changed to Escape before the independent complete47-call pair8/9. No frozen
runtime assertion existed or was changed by these native harness repairs.

For reproduction, adapt only absolute probe/config/output paths to an owned
scratch directory and installed VFP9. Use a private Xvfb display and bounded
observer; preserve raw source/output/logs before repeating. Do not run this
observer on the user's desktop or install fonts to force matching results.
Run native observations serially, wait for completion and verify no owned
Wine/Xvfb remains. VM cleanup/backup-reset completion remains unconfirmed;
this recipe does not authorize VM starts or system maintenance.
