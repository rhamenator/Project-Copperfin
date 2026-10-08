# ERROR command Numeric conversion (#5611/#6776)

## Retained requirement recovery and pre-implementation plan

RQ-CF-PRG-ERROR-COMMAND-NUMERIC-001 derives from admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner extension intent, and
HZ-runtime-crash-01/HZ-data-corruption-01. Only ERROR's first-operand
llround/int site in prg_engine_dispatch.inl is selected. This section records
the plan before production changes, baseline or acceptance; completed evidence
is recorded separately below, not retroactively claimed by the planning record.

Three final strictly serial installed VFP9 09.00.0000.7423 runs match all
66 lines: 32 Numeric operands in bare and parameterized forms, VERSION and
cleanup. Fractions truncate; zero/negative/huge/infinite invalid requests
raise catchable 1941. Negative low32 alias -4294967295 raises error1, while
positive 4294967297 rejects with1941. After-ERROR statements never run and
DATASESSION remains1. These observations do not independently establish
every converted catalog index or a complete valid-error-number catalog.

The shared Wine wrapper is unchanged. One preliminary serial observation
was followed by an accidentally overlapping three-probe batch, one of which
failed with X11 BadWindow and no VFP output. That entire overlapping batch
is discarded as acceptance evidence. All processes finished before the three
fresh final serial matching runs above. No VM, external table, SQL connection,
network call, decompiled binary or system change was involved.

Three final serial controls.prg runs match all38 lines: the same32 Numeric
operands with a counting parameter UDF, four Currency controls and VERSION/
cleanup. Conversion-stage invalid -1, signed32 endpoints outside the admitted
nonnegative domain, large positive and positive infinity suppress parameter
evaluation. Truncated0, invalid catalog entries such as99/9999, and negative
huge/infinite indefinite-zero aliases evaluate the parameter once before1941.
Negative -4294967295 raises1 after one call. Currency operands raise9 before
parameter evaluation. An early control source's incorrectly formed negative
Currency literal produced parser1988 and is excluded; final source uses
-$11.5 and all four Currency rows raise9. No fall-through/session change.

Planned derived conversion policy: default finite Numeric truncation with
checked nonnegative signed32 conversion-stage admission, including0; exact
int64/uint64 without double loss. Explicit VFP9 retains defined negative low32
aliases including indefinite0, rejects negative converted signed32 results,
and rejects positive out-of-range/non-finite operands. Evaluate the optional
parameter only after conversion-stage admission; then reject zero with1941
before publishing requested-error metadata. Positive supported/derived codes
continue to use the existing message path. Thus conversion rejection precedes
parameter evaluation, while the observed zero-domain rejection follows it.
Default rejection of huge negative values is owner-directed safe policy;
only VFP9 preserves their zero aliases. NaN/exact integers are derived direct
boundaries, not native script observations.

Use safe localized original-operand rejection text. Currency's existing
coercion will be checked/preserved separately rather than claiming native9
parity or silently redefining it from Numeric-only conversion observations.
This type-admission discrepancy is retained with the unresolved ERROR gap.

Allowed Numeric conversion does not establish catalog membership: VFP ERROR99
raises1941 whereas the current source/test expects99 and documents an incomplete
standard error-message catalog. Recovery gap #7079 is filed against exact main
with native/source/test evidence. Guarded current-main reproduction was pending
at recovery; it is now retained in baseline-audit.txt and comment6051872181.
Full catalog membership/message/type/arity,
synchronous-UDF/resumable ERROR handling, ON ERROR precedence and error-state
lifecycle remain separate. Base main is
70bc9233d2e5bf1a4a1b4d11568a00f27b9e17d2. The gap has no implementation
admission label; do not expand this conversion slice.

Verification plan: independent direct Numeric/exact/NaN/infinite/adjacent-range
constants in both modes; guarded bare/parameter scripts with error number,
semantic diagnostic, fall-through suppression, ERROR()/AERROR metadata,
session/mode/reset and String/Currency controls. Use unchanged production
first for negative evidence; then GCC and focused Clang ASan/UBSan/
float-cast-overflow, strictly serial with private TMPDIR, plus older Numeric,
focused error-handling neighbors and repository contracts. Native exact
integers/NaN, full catalog parity, byte-identical native messages and platform/
release qualification are not claimed. No argument-sized allocation or loop
is planned. No concurrent runtime/native invocations are supported.

DQ-ERRORNUMBER-001: distinguish native conversion observations, derived
extension/domain rules, and unresolved catalog/type boundaries, with reverse
requirement/code/test links. DQ-ERRORNUMBER-002: retain baseline/acceptance
identities, serial reproduction, cleanup, reviewed slice-only rollback and
changelog/PR field notice. Medium misuse: assuming every admitted signed32
number is a supported native catalog entry can misroute recovery.
Procedural delta will replace unchecked half-away/narrow conversion only.
DV self-review, guarded walkthrough, rollback walkthrough and automated VR
were unfinished at recovery; completed development evidence follows.

## Implementing boundary and original-production evidence

checked_error_number_argument in helpers.h/.cpp implements only the recovered
conversion stage. The selected dispatcher rejects missing conversions with
PrgCompatibilityError1941 and safe localized original-operand text before the
optional expression, rejects converted0 after that expression, and retains
positive-code message/metadata publishing and the character path. Exact types
never pass through double; Currency retains checked existing half-away coercion.
No whole-catalog, native Currency type9 or resumable-expression fix is implied.

baseline-audit.txt retains original source/test/raw-log identities and actual
catalog/type controls. With dispatcher blob53fcaac496e823ac92fd5f727dd8f75bae09eca7
byte-identical to base70bc9233d2e5bf1a4a1b4d11568a00f27b9e17d2 and the helper
linked but unused, GCC15.2 and Clang21.1.8 each fail exactly90 selected guarded
rows, with zero direct-helper/state-suffix/setup/reset/cleanup/neighbor failures.
GCC new0.70s/combined0.81s; Clang new2.91s/combined3.32s, no sanitizer diagnostic.
An initial TRANSFORM grouping/logical test-format mismatch was corrected before
these final counts and is excluded as regression evidence. No older Numeric
baseline or leak qualification is claimed. Audit SHA256:
94ed0cb3425f732abef27934bd9241379081139e8b95eeee0434d81504ecf9c4.

Native source/output identities (probe then controls), SHA256:
722fdfdd00769d3a9ae45d80f962d4312ab29d1db1cfe6ae36fa67a42916a6d8
e896517a9c3ad5ce577b5100f51f77774b7997f3854c72484ee08e35b7d3967a
c791c803d99a98842a4306468428eab36ecd79fd66e46c36213305b0bcab6e90
9a5d98cf55dfe1e5691606aeb1096ef38da178de68fdf2c6ca2979ebaa61c194.

## Completed development documentation review

DV-ERRORNUMBER-001: Codex in the owner-authorized maintainer workflow completed
development self-review of the32 native Numeric rows, conversion versus catalog
inference, independent exact/NaN/adjacent constants, signed32 range-before-narrow
paths and negative-only VFP9 aliases. Review confirms safe original-operand
formatting avoids value_as_string's unchecked Numeric llround; Currency and
exact integer formatting are safe. Zero admission is not successful ERROR0.
This is development self-review, not independent or human sign-off.

DV-ERRORNUMBER-002: completed guarded walkthrough seeds prior error12 metadata,
attempts bare/parameter requests, captures caught code/message, ERROR()/AERROR(),
counting-UDF calls and no fall-through, and checks cursor pointer/count/payload,
session1 and mode. Conversion failures suppress the UDF; truncated/aliased0
calls it once before1941; valid positive codes retain their parameter message.
All four runs close their guard cursor and restore COPPERFIN. Eight fresh
both-mode locale cases verify1941 and original -1 text in all four catalogs.
Seven existing ERROR regressions cover numeric/parameter/character/ON ERROR,
malformed arity, missing operand and logical/NULL rejection without redefining
their broader lifecycle policy. Full control_flow/task/concurrency is not run.

DV-ERRORNUMBER-003: completed rollback walkthrough specifies a reviewed revert
limited to this slice, repeats the named conversion/Numeric/ERROR regressions
and contracts, and republishes corrected guidance through changelog/PR. No
destructive reset or broad deletion. Procedural delta: unchecked half-away/int
Numeric conversion becomes checked truncation with explicit1941 and the
observed two-stage parameter order; Currency stays checked half-away. Medium
misuse remains assuming an admitted positive code implies native catalog
membership; #7079 is explicitly unfinished. Native message bytes, complete
catalog/type/arity, resumable handling, state lifecycle, platform/release/
integrator qualification remain separate. No argument-sized loop/allocation,
external database, ODBC/network, VM or installed-system change is added.

## Local verification and reproduction

VR-ERRORNUMBER-001: 118 independent direct calls pass in both modes, including
exact int64/uint64 precision/extrema, NaN/infinities, adjacent signed32/signed64
limits and Currency/other-type controls. Exact/NaN are direct derived boundaries,
not native/script observations.
VR-ERRORNUMBER-002: 148 guarded bare/parameter both-mode cases and eight localized
fresh cases pass, plus all seven existing ERROR neighbors. Metadata assertions
cover caught/error()/AERROR code and message agreement, not every AERROR field
or general nested-error lifecycle. English rejection text is semantic Copperfin
diagnostic evidence, not byte-identical native wording.
Focused GCC passes2/2 in1.84s (new1.73s,neighbors0.09s); focused Clang ASan/UBSan/
float-cast-overflow passes2/2 in5.13s (new4.73s,neighbors0.39s), no diagnostics,
detect_leaks=0. VR-ERRORNUMBER-003: full GCC selection passes3/3 in87.56s
(older Numeric86.45s,new1.00s,neighbors0.10s). Clang excludes full older Numeric.
Six final repository contracts pass6/6 in5.24s: issue intake, signoff,
changelog assembler/validity, locale installation and native test isolation.
All173 fragments validate. The additional long safety-workflow negative-fixture
contract also passes in359.21s (its six-contract selection6/6 in364.25s).
No merge/release completion is claimed. DQ-ERRORNUMBER-001 maps to DV-ERRORNUMBER-001/-002 and
VR-ERRORNUMBER-001/-002; DQ-ERRORNUMBER-002 maps to DV-ERRORNUMBER-003 and
VR-ERRORNUMBER-003. These records implement the docs32 reverse mapping.

Final GCC acceptance log SHA256:
966d070735dea1d4fd1b7c857da3dd43b31620222271cf8c61e6f5e82f1bfbf1.
Final focused Clang log SHA256:
7f545bbdec8ea7dec81d58b7eb68fd4b54e4bf5d807532d0923d25f02314d7a5.

Build named targets test_prg_engine_error_command_numeric,
test_prg_engine_error_command_neighbors and (GCC) test_prg_engine_numeric_behavior
in owned /home/rich/temp/copperfin-error-command-5611-build (GCC15.2 Debug
-O0 -g1) or /home/rich/temp/copperfin-error-command-5611-sanitize (Clang21.1.8
with -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all
-fno-omit-frame-pointer). Use a fresh mktemp -d TMPDIR inside each build,
ASAN_OPTIONS=detect_leaks=0/UBSAN_OPTIONS=halt_on_error=1 for Clang, and ctest
-j1 --output-on-failure; regex ^test_prg_engine_(error_command_numeric|error_command_neighbors|numeric_behavior)$
(Clang excludes older Numeric). Runtime/native invocations stay strictly serial
across builds; duplicate concurrent runs unsupported. Source-backed isolation:
test-owned mode/locale scratch roots, scoped/restored process locale, no child,
network, source sample or shared resource. Unchanged vfp9-probe runs each native
source strictly serial. After merge, only owned builds/FXP move to recoverable
trash and the validated slice branch/worktree is removed; tracked evidence,
unrelated files and all stashes are preserved. Hosted required checks, clean
exact-head Claude/authorized Codex review and resolved conversations remain
merge gates. Existing missing-field/unused-capture/function warnings remain.
