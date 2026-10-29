# Default Numeric TRANSFORM recovery — #5611/#6776/#7189, tracking #7210

## Exact binary rounding diagnostic — 2026-10-27

### Next bounded native discriminator: ambient setting independence

Native capture result (2026-10-29): the exact first-run gate passed before the
repeat was launched. Both runs retained36 records/144 observations and were
byte-identical at SHA256
`76562c8a088e371ccea64a95798b11fabbbfe604395f62982b5397efbc6f3099`;
all status receipts are0, stderr is empty, both prefix checks are idle and both
full I/O PSI avg10/avg60 readings are below1%. The96 exact comparisons between
ambient DECIMALS0 and13/18 found zero public STR mismatches for all six
sign-paired payloads, FIXED OFF/ON and explicit counts15..18. This supports
ambient-setting independence only for the observed grid; it does not identify
an internal formatter, precision cap or production algorithm.

`analyze-str-ambient.cjs` does not assume equality: it retains every exact
hex mismatch with payload, sign, FIXED mode, explicit count and both ambient
settings. The qualified run's complete mismatch list is empty. Its synthetic
test proves a changed count17 observation is retained rather than normalized.
`str-ambient-captures1` preserves67 gzip entries including both raw runs, the
complete44-field observations, first/paired audit reports, pressure/prefix/
hash receipts, frozen source and analyzer sources. Every entry passed compressed
and decoded size/hash verification against the host bytes; manifest SHA256 is
`1304c58f52eca5e2e7a30a31490c993e853aaebb2d41f635b7d3797f0facc50d`.
Ordinary shell/Wine evidence remains public-observation evidence, not REA atomic
mapped-image or internal-call proof. No VM, compiler, new FLL, tolerance,
ORIGINAL assertion or production change was used.

The zero ambient mismatches reject ambient SET DECIMALS as the discriminator
for this observed explicit-STR grid. The next bounded conversion discriminator
is the installed MSVCR71 public `sprintf`/`_snprintf` fixed-format route, which
was not covered by the completed `_fcvt`, `_ecvt` and `_gcvt` comparisons.
Read-only PE export inspection confirms both symbols in the same retained
module SHA256
`8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe`.
`plan-sprintf-discriminator.cjs` binds the byte-qualified STRmiddle audit and
selects all264 payloads, counts15..18 and both FIXED control rows:1,056 bounded
CRT calls compared against2,112 exact retained VFP strings. It preserves the
full expected hex, requires OFF/ON equality before deduplicating CRT calls and
uses only `%.*f` with a2,048-byte owned buffer. Eight synthetic rejection checks
cover missing controls and changed FIXED text. A guarded source-only replay had
idle Wine prefix, unchanged parent/module/source hashes, empty stderr/status0;
`sprintf-discriminator-plan-captures1` retains17 gzip source/output/gate/hash/
export receipts with compressed and decoded hashes. This is source-only selection:
no compiler/FLL/native call has run, and public CRT agreement would not prove
VFP internal call attribution, formatter acceptance or a production algorithm.

The first caller foundation is now source-frozen without invoking that native
route. `sprintf-boundary.h` admits only finite binary64 values and exact counts
15..18, passes the literal `%.*f` to both installed-export-shaped cdecl
interfaces and owns independent2,048-byte guarded buffers. It validates return
length, NUL termination and capacity before copying into384-byte result fields.
Crucially, unequal `sprintf` and `_snprintf` strings and non-fixed-grammar text
are retained with explicit equality/grammar flags rather than rejected. The direct
stub test passes45 boundary checks including unequal-output preservation,
nonfinite/count rejection, each return/length/termination/grammar/canary failure
and null parameters. Seventeen frozen-source assertions require exact call order,
literal format/capacity/counts, guard-before-second-call and prohibit DLL loads,
allocation and floating-control writes. This is still source/stub evidence only:
the FLL handler, strict native reader and worker remain to be authored before a
fresh resource gate can authorize compilation or a native run.

The FLL handler source now binds the existing loaded MSVCR71 module through the
retained path/header/hash gate, resolves both exports without loading a DLL and
calls the guarded comparison boundary as `CFNSPRTF(value,count)`. Its bounded
4096-byte response retains payload bits, count, control words, exact module
identity, both public output strings, both grammar flags and equality, including
false flags and unequal text. It does not modify the completed `_fcvt`, `_ecvt`
or `_gcvt` handlers. The expanded frozen-source check passes27 assertions and
the same45 stub boundary checks still pass. No freestanding/native compile or
Wine execution has occurred; the generated VFP stimulus, strict reader and
first-run-gated worker remain pending.

The generated stimulus is now frozen source-only. `emit-sprintf.cjs` revalidates
the byte-qualified STRmiddle audit and derives exactly264 finite payloads and
their original DECIMALS settings without embedding expected strings in the PRG.
The generated run is1,056 records (four counts15..18 per payload), FIXED OFF,
with inline width40/scale18 consumers and before/after metadata, BRS bits, fresh
STR text and `_fcvt` controls around every `CFNSPRTF` call. Its separate inventory
retains both OFF/ON expected VFP hex values for2,112 later comparisons. Eighteen
exact generator/frozen-byte assertions pass; generated source and inventory are
18,460 and136,099 bytes respectively. This step did not compile or execute the
FLL or Wine. Host full I/O PSI was about25% at selection, so the bounded wake
remained lightweight; strict reader and first-run-gated worker are still next.

The strict per-record reader foundation now verifies all18 fields, inline
metadata/BRS/STR and `_fcvt` before/after controls, applied DECIMALS/FIXED state,
exact loaded-module identity and every `CFNSPRTF` structural field. It
independently recomputes equality and count-specific fixed grammar. Both export
strings remain raw, and every difference against both retained OFF/ON VFP values
is materialized with export, mode, expected/actual hex, raw text and over-width
status; non-grammar and over-width output are observations, not discarded
records. The inventory now carries file size, PE identity and required runtime
environment from the retained CRT cases. Twenty-five synthetic parser checks
and all18 generator checks pass. This is record-level source verification only;
complete run framing/repeat qualification and the gated worker remain pending.

Complete-run qualification is now source-frozen. `audit-sprintf.cjs` requires
status0/empty stderr, exact runtime environment, stdout/results identity, VFP9
version framing, all1,056 ordered records and DONE count. Its `--single` result
is the mandatory first-run receipt and includes the complete mismatch array;
paired qualification binds that exact receipt before reading the repeat, then
requires observation and mismatch identity while allowing only validated
inactive Numeric integer-slot differences. Eleven complete-reader synthetic
checks cover the passing first/pair, status/stderr/environment/framing/record
rejection, receipt binding, repeat divergence, positive mismatch preservation
and the sole inactive-slot exception. Together with25 record-parser and18
generator checks, this remains source-only. The capture worker/build binding is
the remaining harness step before a fresh resource-gated compile/native run.

Stimulus/reader source checkpoint (prepared before the native capture):
`emit-str-ambient.cjs` derives a36-record/144-observation public stimulus from
the selected six payloads, with originalD15 controls before/after each ambient
window. Inspection added direct ambient metadata/BRS and after-setting receipts;
the frozen record has44 fields, not the initially proposed38. Source SHA256 is
`7031945d2b9ac4dafc4d31a2640fe05363b4e2ad2e50aa123b6b64b8baacd945`.
`read-str-ambient-record.cjs`, `read-str-ambient-run.cjs` and
`audit-str-ambient.cjs` require all controls, exact source/environment/status/
stdout/raw binding, complete order/framing and a first-structure gate before
paired qualification. Paired differences allow only inactive integer metadata
slots after active metadata validation; raw equality/differences remain visible.

Six guarded offline stages passed:20 generated-source,44 record,13 full-run and
13 audit synthetic checks (90 total), generated-source/repeat byte-equality;
each child had a15-second bound, prefix-idle/full I/O PSI below1%/nice10,
empty stderr/status0 and immutable inputs before/after. Synthetic audit records
are harness tests, never native ambient results. Initial audit tests exceeded
30 seconds while constructing oversized Buffer assertion diagnostics; a bounded
five-second trace localized this. Exact Buffer.equals predicates with bounded
messages replace diagnostic-heavy deepEqual; all13 rejection checks then passed
within10 seconds and in the full suite. The earlier process outcome was not
recoverable and was never claimed as a pass. No native or production defect is
inferred. `str-ambient-source-captures1` retains47 gzip source/receipt entries
with compressed/decoded size/hash and exact host-byte verification; eight source
versions remained unchanged. Frozen generated PRG is retained in that archive.

LOW DQ-DV001 developer walkthrough checks loop/field order, ambient before/after
metadata, first-error stop, malformed/status/source rejection, first-run gate,
bounded failure diagnostics and archive bytes. Not independent human assurance;
existing hazards/rollback apply. The later native result above is the capture
acceptance; source readiness alone was not treated as acceptance.

Selection checkpoint: `select-str-ambient-witnesses.cjs` binds the qualified
STRmiddle raw hash, actual full OFF/ON controls and N40/18 metadata. Ten focused
rejection checks pass. Four guarded offline stages passed (tests, strict native
reader replay, selection/repeat byte-equality), then independent parent lookup
verified six payloads,12 original controls, sign-bit pairing and144 planned
observations. Original inputs remained unchanged before/after each child;
prefix-idle, both full I/O PSI averages below1%, nice10 and empty stderr/status0
apply. The later reader is separately bound, not retroactively included in the
original execution manifest. `str-ambient-selection-captures1` retains37 gzip
source/receipt entries verified compressed/decoded size/hash and exact host
bytes, with three source versions stable across archival. This qualifies
stimulus selection only: no new native experiment or output acceptance occurred.
LOW DQ-DV001 developer walkthrough checks the documented index correction,
full parent controls, rejection boundaries, independent sign-pair lookup and
archive bytes; not independent human assurance. Existing hazards/rollback apply.

Pre-observation specification, not executed or accepted: the retained STRmiddle
grid binds each payload to its original SET DECIMALS; it explicitly is not a
settings Cartesian experiment. Existing CRT precision offsets change conversion
counts, not that payload-to-setting binding. Therefore count-step arithmetic
alone cannot separate ambient-setting dependence from explicit STR-count
behavior. Do not repeat those aggregate analyses as a new discriminator.

Select actual source indices184,185,186 and their negative witnesses190,191,192
from the hash-qualified STRmiddle inventory, retaining their exact bits and
original setting. Before execution verify those are the intended signed
witnesses rather than inferring pair relationships from index proximity. Source
inspection corrected the initial proposed223..225: those belong to different
setting18 payloads, not sign counterparts. Actual positive bits are
3CC1FF130772E184/185/186; negative190..192 differ only in sign bit and all six
original settings are15. Preserve this selection correction; no run used the
incorrect proposal. Use
the same ORIGINAL INLINE N40/18 route and width40, explicit STR counts15..18,
FIXED OFF/ON, and independently applied ambient DECIMALS0,13,18:144 public STR
observations. Counts/settings are interventions, not a precision-cap model.
Retain original-setting control calls before/after, actual SET readbacks,
metadata/BRS, raw output type/width/hex and installed-runtime sampled identity/
control-word provenance. Require strict first-run validation before repeat,
exact pair readback and complete mismatch retention. No generated expected
strings, hidden normalization, tolerance or production changes are authorized
by this experiment. Native/compiler execution still requires current resource
coordination and the existing fresh pressure/prefix gates; this specification
does not reserve or start a VM or Wine process.

### Signed residual characterization

#### Adjacent requested-count relationships

Arithmetic step relations are retained in `exact-str-step-relation-captures1`:
all1,584 pairs, both exact-nearest modes,72 changing outcomes and unchanged
controls. Thirteen synthetic checks and four guarded stages pass; separate
independent endpoint arithmetic verifies every category,12 groups and the full
changing list. Archive38 gzip entries passed compressed/decoded size/hash and
exact host-byte checks with four sources stable. Later reader binding remains
separate. Existing prefix/PSI/nice10/immutable-input controls apply.

For OFF and either nearest mode,15->16 has160 both-zero,92 cancelled and12
mixed steps.16->17 has184 both-zero,68 cancelled and12 residual-only;
17->18 has168 both-zero,84 cancelled and12 residual-only. No changing step
equals the exact-nearest step in these OFF groups. Here residual-only means
nearestStep0 and observedStep nonzero; cancelled means observedStep0 but nearest
and residual steps cancel. These are arithmetic identities, NOT causal or
internal implementation attribution. Complete ON groups and all endpoints are
retained rather than inferred from OFF. No rounding repair/tolerance follows.
LOW DQ-DV001 developer walkthrough checks conservation, negative/zero/bigint
boundaries, independent raw-endpoint categories and archive bytes; not
independent human assurance. Existing hazards/rollback apply; ORIGINAL unchanged.
NEXT use the retained changing payload/count witnesses to specify a bounded
native discriminator for conversion precision (not another aggregate relabeling),
checking existing captures first and preserving exact count/source provenance.

The completed context partition (`exact-str-changes-captures1`) binds the
count-report compressed SHA256
`d007657a0fc003559ea9b9e4b08d9e23c6060f23108cfdd4b9a23b2780e2c46e`
and decoded manifest. All528 contexts retain their complete three pairs and
shared endpoints. Twelve synthetic checks and four guarded stages passed;
separate independent lookup recomputes the changing classification directly
from observed integers, validates every endpoint, both partitions and eight
grouped index lists. Archive38 gzip entries passed compressed/decoded size/hash
and exact host-byte verification, with four source versions unchanged. Later
reader binding is separate; existing prefix/PSI/input-hash/nice10 controls apply.

There are72 changing contexts and456 unchanged. Each FIXED mode has36 changing
contexts, all changing on exactly one adjacent-count step; none changes twice
or three times. Both modes have the same changing indices:184..186,190..219,
223..225. Complete payload/settings and nearest residuals remain attached;
this is not evidence of a universal cap or causal conversion mechanism.
LOW DQ-DV001 developer walkthrough checks parent binding, complete-grid and
shared-endpoint rejection, independent observed-integer classification and
archived bytes. Not independent human assurance; existing hazards/rollback
apply. No production, tolerance or ORIGINAL assertion changes.
NEXT compare each changing step's exact nearest-rounding transition to its
observed transition and retain all72 outcomes, distinguishing changes explained
by exact rounding from residual-only changes before choosing new native stimuli.

`join-exact-str-counts.cjs` requires each original index/FIXED context's complete
unique15..18 count grid and unchanged payload/setting. All1,584 adjacent pairs
retain both full endpoints and exact observed, nearest and residual steps,
asserting observedStep=nearestStep+residualStep. Twelve synthetic join checks
pass, including malformed/incomplete/duplicate/mixed-context rejection.
Four guarded stages passed (tests, parent readback, analysis, byte-equal repeat),
followed by separate independent endpoint lookup and verification of all pairs,
six histograms and complete nonzero list. Prefix-idle, full I/O PSI below1%,
nice10, empty stderr/status0 and input hashes before/after apply. The later
independent source is separately bound, not retrobound. Archive
`exact-str-counts-captures1` retains38 gzip entries with compressed/decoded
size/hash and exact host-byte checks; four source versions remained unchanged.

For each FIXED mode and each15->16/16->17/17->18 step,252 of264 observations
have observedStep0. The remaining12 split six negative/six positive, with
step sizes respectively10,100,1000 units of the higher requested decimal count.
All72 nonzero pairs and all1,584 endpoints remain retained. Thus these retained
STR values mostly append precision without changing numeric value, but not
universally; neither a fixed decimal cap nor a universal append-only conversion
is established. No tolerance or ORIGINAL assertion change is accepted.

LOW DQ-DV001 developer walkthrough traces pinned parent manifest, complete-grid
rejection tests, independent endpoint/conservation/group checks and archive
bytes. Not independent human assurance; existing hazards/rollback apply.
NEXT isolate the72 changing pairs and bind their exact payload/settings to
existing nearest residuals to distinguish count-driven changes from unchanged
payload groups, retaining all counterexamples before choosing new stimulus.

`exact-str-residual.cjs` retains observed-minus-nearest integer residuals in
units10^-count and orients magnitude residuals using the raw payload sign,
including rounded zero. Fifteen synthetic checks pass. The analyzer pins the
parent compressed report SHA256
`f95a4192f2a4889b50f6bb9e9b3d238124583eb89e07516b2df2ed1ac3df106f`,
decoded manifest hash and native source hash. Every original row is preserved.
Five guarded stages (syntax, tests, parent readback, analysis/repeat) passed;
separate independent readback verifies all2,112 parent identities, both signed
residuals, all16 histograms and complete mismatch selection. Later reader source
is separately bound, not retroactively inserted into the old execution manifest.
Prefix-idle, both full I/O PSI averages below1%, nice10, empty stderr/status0,
and input hashes before/after apply. Archive `exact-str-residual-captures1`
contains49 gzip entries verified compressed/decoded sizes/hashes/exact host bytes;
four source versions were unchanged across archival.

For OFF nearest-even, count15 has216 zero residuals and48 at+1 magnitude unit.
Count16 ranges-5..+5 with160 zero; count17 ranges-50..+50 with132 zero;
count18 ranges-500..+500 with112 zero. These are finite observations, not bounds
for other inputs or a fitted correction. Complete histograms for both modes and
FIXED policies and every counterexample remain archived. A constant single-unit
adjustment cannot explain the observed higher-count residuals. No tolerance,
production conversion rule or ORIGINAL assertion change is accepted.

LOW DQ-DV001 developer self-review traces parent hashes/raw sign, exact delta,
independent per-row recomputation, histograms and archive bytes. This is not
independent human assurance; existing hazards/rollback apply. NEXT investigate
count-to-count residual relationships on identical payload/FIXED contexts using
retained raw observations, without guessing a decimal cap or erasing mismatches.

Owner-directed diagnostic tests pass28 checks: known BRS payload byte order,
signed zero, subnormal/minimum-normal/maximum-finite boundaries, exact positive
and negative ties, neighboring payloads, sub-binary64 decimal distinctions and
invalid inputs. Exact BigInt binary rational scaling produces truncation,
nearest-even and nearest-away hypotheses, not native expected strings.

The hash-bound retained STRmiddle source supplies all2,112 cells (528 original
contexts at counts15..18). Five guarded stages passed:28 tests, strict native
reader replay, analysis, byte-equal repeat and independently implemented binary
bit-string/rational readback of every row, group and full mismatch selection.
Each child had prefix-idle status1, both full I/O PSI averages below1%, nice10,
empty stderr/status0 and immutable input hashes checked before/after. No native
execution occurred. Receipts and four source versions are retained in
`exact-binary-str-captures1`, with35 gzip entries verified compressed/decoded
sizes/SHA256 and exact host bytes; host root is `build/exact-binary-str1`.

Per FIXED mode, count15/16/17/18 truncation value mismatches are190/208/214/220;
nearest-even and nearest-away each have48/104/132/152. Raw zero-sign differences
are3/9/15/21. All2,112 raw observations and1,682 rows with any hypothesis-value
or zero-sign mismatch are retained. Signed zero is compared separately from
integer numeric equality; assertions were not weakened. These counterexamples
reject all three hypotheses as universal replacements for the observed STR
conversion. No general formatter or internal-call rule is accepted.

LOW DQ-DV001 developer self-review walkthrough: trace pinned source/raw reader
to payload/count/text, check exact tie and negative-zero boundaries, independent
readback and complete mismatch list, then archived bytes. Not independent human
assurance. Existing hazards/rollback apply; production and ORIGINAL assertions
are unchanged. NEXT characterize exact signed residuals of the nearest-rounding
counterexamples, retaining count/payload/source context; do not fit tolerances
or discard mismatches.

## Retained STR/CRT coverage inventory — 2026-10-27

### Exact same-count comparison of available cells

`analyze-str-crt-cells.cjs` compares all384 available cells, retaining raw STR
text, CRT digits/point/sign, context, source controls and all1,728 missing cells.
`compare-str-crt-cell.cjs` uses exact BigInt coefficient/exponent comparison;
empty native returns remain unavailable numeric values, never invented zero.
Twenty-one synthetic metric tests cover signed zero, empty returns, exponents,
negative decimal points, sub-binary64-resolution differences and malformed data.
Fourteen guarded stages passed, empty stderr, immutable inputs before/after,
prefix-idle and both full I/O PSI averages below1%; repeated output is byte-equal.
Independent rational cross-multiplication readback binds every row to raw source
controls and verifies exact-value/zero-sign flags and all groups without sharing
the metric normalizer. That later reader is separately hash-bound, not claimed
to have existed in the original execution manifest.

For each FIXED mode there are48 contexts per count-setting offset. Offset0 has
12 empty returns and36 exact-value differences; offset1 has0 empty and36 value
differences; offsets2 and3 each have0 empty and24 value differences. All groups
have0 zero-sign differences. Offset0 repeats the earlier96-cell control and is
not new evidence. These finite counterexamples reject universal same-count CRT
value equivalence for these consumers; they do not establish a replacement
conversion rule, internal call graph, metadata parity or production formatter.
Every mismatch remains in `str-crt-coverage-captures3/cell-analysis.out.gz`.

Archive3 retains137 gzip source/receipt entries, with compressed and decoded
sizes/SHA256 and exact captured host-byte verification; eleven current sources
were unchanged across archival. No native/VM/compiler execution or ORIGINAL
assertion change occurred. LOW DQ-DV001 developer self-review walkthrough checks
raw source binding, empty-return separation, exact arithmetic and complete
counterexamples against independent readback and archive bytes; this is not
independent human assurance. Existing hazards and rollback boundaries apply.
NEXT recover the conversion discriminator behind the nonempty value gaps;
do not synthesize missing native observations or fit a tolerance to the totals.

### Additional retained count coverage

The separately qualified ORIGINAL-only D+1/+2/+3 CRT experiment has pinned raw
SHA256 `ba189979b2d7cd67ab15f1d719af586f815f28f98427d30344629760f1fbabd3`.
The combined loader validates original payload/argument/setting/display/STR and
all eleven sampled runtime/control-word fields before combining coverage. Its
reader separately validates metadata and all 1,584 records. This does not turn
sampled module controls into atomic image binding or establish digit equality.
The experimental count bound reaches21; requested STR cells remain15..18.

Combined coverage has 2,112 requested cells,384 available,1,728 missing. Full
cell and missing lists remain in `str-crt-coverage-captures2/inventory.out.gz`.
Nineteen synthetic coverage, twelve loader and twenty-four cross-source checks
pass. Ten guarded offline stages passed with empty stderr, prefix-idle status1,
both full I/O PSI averages below1%, nice10 and input hashes before/after. The
precision reader matched its frozen source. Independent nested-source lookup
verified every cell and missing-list entry; repeated inventory was byte-equal.

Independent readback source changed after the execution manifest was frozen.
The initial prehash correctly failed; retain `independent.inputs-before.txt.gz`.
Only that source was then explicitly excluded from the stable input manifest
and separately hash-bound before/after readback. No old execution manifest was
rewritten or used to retroqualify the new source. Archive2 contains102 gzip
source/receipt files; compressed/decoded sizes and hashes and exact host bytes
were verified. Seven current source hashes bound archival inputs before/after;
host `build/default-transform-str-crt-coverage2` retains postarchive verification.
Archive1 and its source versions remain unchanged.

LOW DQ-DV001 maintainer walkthrough: inspect the complete context key and
cross-source rejection tests, frozen-reader identity, sampled runtime limits,
all requested/missing rows, explicit source-version transition, independent
lookup and archive byte checks. This is developer self-review, not independent
human/release assurance. Existing hazards/rollback apply. No native execution,
digit-generation candidate, tolerance or ORIGINAL assertion change occurred.
NEXT select a bounded digit-conversion discriminator from the384 available
cells, retaining complete counterexamples, before deciding whether any of the
1,728 missing cells require new native stimulus. Missing observations must not
be synthesized from other counts or rounded outputs.

`inventory-str-crt-coverage.cjs` and `load-str-crt-coverage.cjs` inventory the
requested STR15/16/17/18 cells of the retained 528-context public consumer grid.
Availability requires the same original index, FIXED mode, payload, setting and
conversion count in the native-only CRT capture. SHIFTED and E-kind records
cannot fill F-kind cells. This inventory is not a metadata/runtime equivalence
join or a digit-conversion comparison; empty CRT returns prove availability
only, never numeric equality. No native expected text is generated.

The retained grid requests 2,112 cells; 96 have corresponding native F-count
observations and 2,016 do not. Every requested and missing cell is retained in
`str-crt-coverage-captures1/inventory.out.gz`. The earlier accepted 96-context
comparison is not new evidence and cannot stand in for missing count/payload
coverage. The completed broader528 rendering relation still takes native STR
digits as input; it is not a recovered conversion algorithm.

Six source files and receipts are retained in `str-crt-coverage-captures1`:
82 gzip files, manifest compressed/decoded byte counts and SHA256 verified
against original host bytes. Host `build/default-transform-str-crt-coverage1`
retains the post-archive source-hash check. Six source hashes bound archival
inputs, including the later independent-readback source. The original execution
manifest predates that readback source; do not claim it bound that later source.
Both native reader files were byte-compared to their frozen archive sources.
Eight offline stages passed, empty stderr, fresh prefix-idle status1, both full
I/O PSI averages below1%, nice10, immutable inputs before/after each child.
Independent nested source lookup checked all cells and the complete missing
list; inventory repeat was byte-identical. Sixteen synthetic coverage and twelve
synthetic loader checks passed. A first loader-test attempt used an incorrect
in-memory path slice and failed before native evidence access; corrected from
slice2 to slice3. No production defect is inferred from that harness failure.

LOW DQ-DV001 maintainer self-review/walkthrough: trace source hashes and raw
reader flags to the retained archives, inspect duplicate rejection, route/kind/
payload/count separation, input immutability and empty-return non-equality,
then independently reconstruct every cell and verify archive bytes. This is
developer self-review, not independent human or release assurance. Existing
hazards, rollback and notification boundaries remain unchanged. Roll back only
this diagnostic inventory if its selection is wrong; never rewrite native
observations or relax ORIGINAL assertions to match it.

NEXT inventory the additional retained D+1/+2/+3 count evidence against missing
cells, preserving distinct metadata/runtime provenance before any combined
conversion conclusion. Do not assume those captures fill cells merely because
counts coincide. No new native stimulus, production formatter, tolerance,
conversion acceptance or division/ListItem completion is claimed here.

## Pre-observation contract

`RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001` derives from owner Numeric
compatibility policy and parent `RQ-CF-PRG-NUMERIC-METADATA-001`.
Recover the default Numeric-to-Character display consumer's use of associated
width/decimal metadata and SET DECIMALS/FIXED. Allowed requirement sources are
installed VFP9 public behavior and owner policy, not the existing formatter.
The #6776 per-area compatibility-switch policy, checked conversions and
allocation limits remain unchanged. Neither Numeric metadata fields alone nor
an approximate/tolerance result establish display parity.

Unchanged main240ac51cb and the deferred small-divisor branch already reproduce
70 literal-only display mismatches while all300 literal metadata/payloads match:
signed checkpoint03c3a4520446765125101be7052a05bf7e6f0546 and issue
comment6249598072. At D0/FIXED OFF, current0.5 prints `0`, native `0.5`;
current2.50 prints `2`, native `2.50`, both NUMERICBEHAVIOR modes. This is
reproduction, not a proven algorithm or product acceptance.

## Frozen native stimulus (not expected results)

`emit-probe.cjs` produces the checked-in `probe.prg` and `cases.json`
without importing Copperfin, candidate logic or expected data. It declares
58 literal forms and24 group/unary/IIF controls, DECIMALS0/1/2/6/14/18,
FIXED OFF/ON, and four distinct routes: direct EVALUATE, direct compiled,
EVALUATE then local assignment, compiled then local assignment.3936 records
per invocation. All expressions are pure; repeated evaluations for metadata,
BRS and default TRANSFORM cannot repeat a user callback.

Each observation retains public-ABI CFNMETA type/width/decimals/active payload
and FPU control word, independent BRS payload, exact Character bytes and
VARTYPE. Reuse the already-authored documented public-ABI observer
`build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll` on the host.
No binary decompilation, disassembly, patching, proprietary implementation
reuse, new installation or VM is involved. The installed VFP9 Wine executable,
observer and runner must be hash-bound before/after execution.

`audit-probe.cjs` is an independent strict raw reader. It requires both actual
statuses0, empty stderr, complete unique inventory coverage, finite typed
payload/BRS agreement, Character output, equivalent compiled/EVALUATE routes
and exact typed repeat. Inactive ABI slots are not semantic equality.
No expectation emitter, candidate, test or production code exists yet.

## Qualification and bounds

Freeze source/inventory/reader in a signed DCO commit BEFORE invocation.
Obtain fresh audit/DAW chat clearance and require full I/O PSI avg10 AND avg60
below1% immediately before EACH serialized nice10 invocation. Preserve all
failed/guarded attempts. Record REA Evidence, actual statuses, raw files,
hashes, pressure, root exit versus descendant settlement, cleanup, truncation
and limitations. The runner's25-second bound and inherited environment remain
explicit; process sampling is not syscall coverage or network containment.

Once qualified, native observations may define independent regression
expectations, then actual unchanged ORIGINAL failure before an unused bounded
support helper/direct tests, then separate consumer wiring and unchanged SAME.
Inspect architecture only as architecture, never as the requirement source.
Keep TRANSFORM pictures, Currency, locale changes, general display dispatch,
nonfinite policy, other Numeric producers and the deferred division helper out
of this slice unless additional native evidence explicitly makes one a required
prerequisite. Stored routes materialize at CURRENT settings; they do not prove
behavior of a variable assigned under another setting. That boundary remains
unknown pending a separately frozen probe if needed.

## First qualified native pair

Signed/DCO source freeze7a45097eb872d64f2e37357dee1b804fa33a358d precedes
both actual invocations. Fresh audit/DAW clearance obtained. Both native
statuses0/empty stderr; all3936 typed records and exact Character bytes repeat,
equivalent compiled/EVALUATE routes agree. Source/runner/VFP9/FLL before/after
hash checks0. Full PSI before first0.00%/0.03%, before repeat0.00%/0.02%.
Both chats received RELEASED; prefix process postcheck empty/exit1.

First raw SHA2569908c60cbc39b1585b463182550e07fae68868ce8de8f21a98c75d00e4fde4d9;
repeat7e2d5030fdea536a4ee9cdce292adbc8aa2000989e15184c20da0faca327d14a.
They are NOT raw-byte identical: one unused real slot of type-I differs.
Initial reader0/report/source are preserved. Strengthened reader explicitly
checks every raw difference is confined to inactive ABI slots, returns0, and
the archived readback report compares byte-identical to the host report.
No native rerun or expected-result change for this strengthening.

REA ev_b5c385b50babd0d46ce575ce70612afc29509d23ec249752ecddbb2eeef6fe67:
root exit0, descendants quiesced, owned process group cleanup verified,
temporary root removed, untruncated. Full structured record in captures/;
prelaunch executable binding is bash, not an atomic VFP9 image binding.
Native/runner/FLL hashes bind sampled files separately. Short-lived descendants
can escape sampling, snapshots are not syscall traces, inherited environment
is not recorded; no network-containment/security-sandbox claim.

Observed, not a general formula:

- FIXED OFF direct2.50 and its assigned copy both display2.50, but1.00
  displays1; settings0 through18 do not change those literal displays.
- FIXED ON changes0.5 to1 at D0,0.50 at D2, and18 fractional places at D18.
  This is half-tie evidence, not authority for all rounding boundaries.
- Direct1E-14 displays .00000000000001 (no leading zero), assigned copy
  displays0.00000000000001; payload/scale match but width differs15 versus25.
  At D0/FIXED ON that direct display is empty while assigned copy displays0.
- Negative nonzero tiny literal -0.000000000000000001 displays -0 with FIXED
  OFF, whereas true signed zero -0.00 displays0. At D0/FIXED ON both display0.
- The18-scale1.234567890123456789 displays1.234567890123457000; simply
  streaming18 exact binary64 fractional digits is not the native requirement.
- Grouping may change associated metadata (already a separate completed
  producer concern). No producer correction is admitted by this display probe.
- Large/exponent controls retain exponent/asterisk outputs, not a newly
  authorized conversion or overflow policy.

Unknowns before implementation: exact finite formatting/rounding boundary,
width/scale causality versus correlated literal syntax, FIXED width handling,
and display after settings change without re-assignment. NEXT separately
freeze controlled public-ABI metadata interventions plus cross-setting stored
values and rounding controls, recover those boundaries, then native-only
regression/actual ORIGINAL before support/wiring. No production change yet.

## Follow-up intervention attempt and narrower source freeze

Signed/DCO source f07ab8b0c9edfbfacf61d71248a3b316f519675a froze384 GRID
expressions and12 STORED expressions,2736 observations per run. Eight finite
real seeds have baseline and bit/field-preserving CFNCLONE controls plus42
width/scale interventions each (widths1/2/3/4/12/20/40,
scales0/1/2/14/15/18). CFNWIDTH/CFNDEC use the existing
authored public-ABI observer source in vfp9-listitem-column-numeric/cause-probe.c;
no new FLL, arithmetic or implementation logic. Read back requested metadata,
unchanged BRS payload and FPU word before accepting any causal observation.
The32 additional finite rounding controls discriminate binary precision/ties
and integer-digit magnitudes. GRID uses EVALUATE only: no compiled-equivalence
claim for this intervention grid.

Twelve private values are assigned explicit compiled expressions once under
each birth DECIMALS0/2/18 and FIXED OFF/ON. Subsequent nested display settings
read the SAME variables without reassignment; observer function references
private variables rather than passing them through a value parameter. It also
captures explicit STR(value,40,current DECIMALS) as diagnostic native bytes,
NOT as a model or requirement for TRANSFORM. Reader reports any metadata/payload
changes without assuming they are zero. Callback timing is not in this grid.

Actual attempt1, after fresh BOTH-chat clearance: first runner0/empty stderr
but ONLY387 observations/no DONE footer, so INCOMPLETE/UNQUALIFIED. Independent
frozen reader1 rejects the missing DONE|2736 footer. Last emitted observation:
D0/FIXED ON GRID3 CFNDEC(CFNWIDTH(0.50,1),0); next requested GRID4
CFNDEC(CFNWIDTH(0.50,1),1) was not emitted. No proven native crash, cause or
Copperfin defect is inferred from this abrupt ending. Initial release messages
reported runner0, then both chats received the explicit missing-footer
correction. No complete native-grid or test pass is claimed.

Full I/O PSI0.00%/0.00% before first invocation;19.34%/4.49% before repeat.
Pressure guard75 prevented second native invocation; no repeat status exists.
First source/VFP9/runner/FLL hashes unchanged. Raw first SHA256
b8b7abdaa151097a36af0559355757caca0e89db9a39843cf43513587690e0a0.
Lossless intervention-attempt1/ retains original source/inventory/reader, all
raw data, statuses, hash checks, guard, pressure, script and full REA record.
REA ev_c690dbad800eaa574d67879bffb95c1bbbc95a987a9491688466ae32eae0a944:
root75/quiesced/owned group verified/untruncated, BUT14 sampled processes have
EACCES environ ownership unknowns. Minimal readback retains root/system process
names/parents; it neither proves causation nor authorizes touching them. No
global cleanup claim. Same image-binding/sampling/environment limits as above.
Both chats RELEASED, prefix postcheck empty; no build/native/Wine/VM reserved.

Narrower current stimulus deliberately omits synthetic width<=scale pairs:
240 GRID +12 STORED expressions,1872 observations/run,24 interventions/seed.
This is an experimental safety restriction after the incomplete attempt, NOT
a recovered production validity rule or an explanation of the missing footer.
The natural width=scale+1 boundary (e.g.1E-14) remains covered. Original
f07ab8b0c source and partial capture remain immutable in Git and the archive.
Source/inventory/reader must be signed BEFORE renewed paired execution. Current
full PSI59.23%/43.38% disallows retry; no waiting resource window retained.
NEXT renewed BOTH-chat clearance plus low-pressure gates for this narrower
pair. Do not retrofit conclusions into the initial82-case stimulus or frozen
division tests. Production, expected emitter, model and helper remain absent.
The next source uses separate host directory build/default-transform-intervention2
so it cannot overwrite attempt1. Reader --single must validate completeness,
metadata interventions, payload/BRS and clone controls after FIRST capture
BEFORE allowing a second invocation. Its status0 means first-structure-only,
not repeat qualification or product parity. Preserve any first-structure failure
and stop; never treat runner0 alone as successful native completion.

### Narrower pair qualified

Signed/DCO source acaf2f67cd795dc06ce15e5018b3545533900e5a precedes the
renewed run after fresh audit/DAW clearance. Both actual native statuses0/
empty stderr/complete DONE|1872. First structural gate0 before repeat;
final independent paired reader0. All1872 records repeat RAW-BYTE-IDENTICALLY,
SHA25645d1e318ba670f980f191c574a83d7d14206069468f35a53827537469018bea2.
Requested metadata readbacks exact, every intervention payload unchanged,
cloned controls equal; stored metadata/payload changes0 across display settings.
Archived independent readback0/byte compare0. Both full PSI windows0.00/0.00
before EACH invocation; all source/runner/VFP9/FLL hash checks0. Both chats
RELEASED; prefix postcheck empty/1. No build, Wine, native or VM reservation.

Full intervention-captures/ REA
ev_d750f514647fb048ce40c9fe290bdf782421ca5a52662b3c19ed0d84bce08915:
root0/quiesced/owned cleanup verified/untruncated, no sampled EACCES unknowns
in THIS capture. Older failed attempt/unknowns remain retained, not reclassified.
Image-binding, process-sampling, filesystem-snapshot and inherited-environment
limitations remain; no sandbox/global cleanup claim.

Observed intervention witnesses (D2/FIXED OFF): same0.50 bits/scale1 display
`.5` at width2, `0.5` at widths3/4; fixed width20 with scale1 versus2 displays
`0.5` versus `0.50`. Width and scale independently affect exact output without
changing payload. Negative0.50 at width3/scale1 displays `-.5`, width4 `-0.5`;
width2 gives `.*`. The clone control is unchanged. Stored-value output changes
with FIXED/DECIMALS while all observed value metadata/payloads remain unchanged.

Rounding witnesses: D2/FIXED ON2.675 ->2.68,1.005 ->1.01,9.995 ->*.** at its
literal width, while explicit STR(...,40,2) is10.00. FIXED OFF18-scale
1.234567890123456789 ->1.234567890123457000; 999999999.123456789 ->
999999999.123456800;12345678901234.5625 ->12345678901234.5600. These suggest
a magnitude-dependent significant-digit boundary, but do NOT establish its
algorithm. Near-half0.499999999999999 produces1 in STR(...,40,0), so a simple
exact-binary half-away rule is contradicted. Do not guess a tolerance from this.
NEXT independently frozen adjacent-binary64 boundary recovery using observed
native centers and documented CTOBIN/BRS identity, BEFORE formatter/test model.

### Adjacent-binary64 source freeze and qualified pair

`emit-rounding-boundary.cjs` reads only the qualified archived native report
(bound to SHA45d1e318...) and stimulus inventory to select eight native payload
centers. It enumerates1000 exact neighboring positive finite binary64 payloads:
0.5 offsets-512..32 and seven other centers offsets-32..32. The centers are
1.005,2.675,9.995,5E-16,1.2345678901234565,999999999.123456789 and
12345678901234.5625; critical DECIMALS0/2/15/7 are explicitly inventoried.
Both FIXED modes give2000 observations/run. Hex payload arithmetic defines
stimulus ONLY, not a display/rounding expectation.

The authored PRG reconstructs bytes with documented CTOBIN(...,'BS') and
applies the already-verified public-ABI width40/scale18 clone without assigning
that returned descriptor to a normalizing variable. Strict reader requires
every metadata field and BRS payload match the declared stimulus, complete
framing/unique coverage, exact typed repeat and inactive-slot-only raw changes.
First --single structure must pass BEFORE repeat. No runtime model, tolerance,
compiled-equivalence or general-format proof is encoded. Separate host directory
build/default-transform-rounding preserves all prior attempts. Sign source/
inventory/reader before fresh BOTH-chat/low-PSI paired capture.
Signed source freeze8f38313540a702af0e0b3bad858a56ff3017fb7c pushed. No run
yet: latest full PSI0.00%/3.70% fails the avg60 gate. DAW cleared this separate
window; audit initially returned no text, which is NOT clearance, then explicitly
said HOLD at full0.00%/3.46%. Do not inherit a released window, treat HOLD as
clearance or start merely because one pressure window is zero. No reservation
retained. NEXT fresh pressure readback and BOTH-chat clearance, not a retry of
the already-qualified1872-record grid. That earlier resource HOLD is historical;
it did not authorize the following separately coordinated run.

Fresh BOTH-chat explicit CLEAR and low-pressure gates subsequently allowed
the UNCHANGED signed8f3831354 source. Both native statuses0/empty stderr,
first structure0 BEFORE repeat, independent paired reader0, complete2000/run,
RAW-BYTE-IDENTICAL SHA256
3c6601c1d769cd9bf662adf3068da1486a4f48e9993fb2b13524dfabcf627980.
Every CTOBIN/BRS payload and width40/scale18 readback exact. Full PSI0.00/0.00
before both invocations; all source/runner/VFP9/FLL hash checks0.
rounding-captures/ retains lossless records/statuses/pressure/script, archived
reader0/host-byte-compare0 and full REA
ev_25f3b664d91b97d1c3b9be6055c6a65987f22542c87f4039a110992173d81106:
root0/quiesced/owned cleanup verified/untruncated/no sampled ownership unknowns.
The inherited-environment/sampling/snapshot/image-binding limits remain.
Both chats RELEASED; prefix postcheck empty/1; no resource reservation.

`analyze-rounding-boundary.cjs` emits observed equal-text intervals ONLY;
it is not a formatter/model. Its complete result/status/stderr is retained.
For FIXED ON, all sampled offsets-512..32 around native0.5 still display1;
all offsets-32..32 around1.005,2.675,9.995 and5E-16 likewise remain in ONE
observed text interval at the declared critical setting. These windows do NOT
bracket those transitions. FIXED OFF still distinguishes payloads through
18-scale text. The other three centers have16/65/13 observed FIXED ON
intervals. A positive finite neighborhood and its endpoint strings do not
establish a general tolerance, negative behavior or a complete conversion rule.

### Wider signed magnitude-offset ladder — qualified

The next source/inventory/reader is emit-rounding-ladder.cjs,
rounding-ladder.prg, rounding-ladder-cases.json and audit-rounding-ladder.cjs.
It derives eight centers from the qualified native declarations, bound to
the paired2000-record receipt. Each center has both sign bits and121 exact
magnitude offsets:0/1/-1 plus neighbors of negative powers of two through2^40.
There are1936 finite inputs/3872 observations per run, same width40/scale18,
critical DECIMALS and both FIXED modes. New source reuses only our authored
PRG template; no Copperfin/candidate/expected-string input.

The purpose is to bracket unobserved transitions, not declare an epsilon or
fit a rounding tolerance. Strict complete/typed/BRS/paired audits and first-
structure gate remain required. Separate build/default-transform-rounding-ladder
preserves all prior evidence. At the source-freeze checkpoint no invocation
had occurred. Audit subsequently reported
renewed full PSI20.05/3.97 and stopped its own probes, so no additional native
job is reserved. NEXT fresh BOTH-chat CLEAR and immediate low-full-PSI gates
for this signed frozen ladder; then exact boundary recovery before any model,
native-only expected regression/actual ORIGINAL/support/wiring.

Fresh audit and DAW explicit CLEAR subsequently allowed the unchanged signed
c25498cd955e33a94b1dd9b85908edb687f4e631 ladder. Both native statuses0/empty
stderr/DONE3872, first structural0 BEFORE repeat, paired0. All3872 records
RAW-BYTE-IDENTICAL, SHA256
e7cbfd0a45549b69bbfcebc2d924dc20bb7f895e1405ad95136f4359b4eb134f.
Every width40/scale18/CTOBIN/BRS readback exact. Archived independent reader0/
host byte comparison0. Full PSI0.00/0.00 immediately before both invocations;
all source/runner/VFP9/FLL hash checks0. Both chats RELEASED, prefix postcheck
empty/1. No build/native/Wine/VM reservation remains.

rounding-ladder-captures/ retains all lossless host receipts and complete REA
ev_380da287e63482c2aefe25f101d20ad4fbeea20048d5bda34e3b0cc0d28d37a3:
root0/quiesced/owned-group cleanup verified/untruncated. Ten sampled unrelated
process-ownership EACCES unknowns remain; they were untouched, with minimal
PID/user/comm readback retained. No claim of global cleanup or causal attribution.
Image-binding/process-sampling/filesystem-snapshot/inherited-environment/network
limits remain. No native implementation or proprietary binary copied.

analyze-rounding-ladder.cjs groups ACTUAL sampled text and differing-neighbor
brackets, not an output formula or continuous-interval proof. At critical D2/
FIXED ON, BOTH signs show magnitude-offset brackets:1.005 between-34359738367
and-17179869185 (1.00 to1.01),2.675 between-17179869183 and-8589934593
(2.67 to2.68),9.995 between-4294967295 and-2147483649 (9.99 to10.00),
with corresponding negative strings. At D0, BOTH signs of0.5 still have one
observed segment across-1099511627777..1; D15/BOTH signs of5E-16 likewise.
The smallest sampled positive0.5 neighbor is0.49993896484374994 and STILL
prints1 in BOTH default TRANSFORM and explicit STR(...,40,0). These facts
rule out treating the earlier adjacent-only window as an exact boundary.
They do not establish monotonicity, uniqueness or any numeric tolerance.

### Native-feedback exact-edge stimulus — qualified

emit-rounding-edge.cjs, rounding-edge.prg, rounding-edge-cases.json and
audit-rounding-edge.cjs freeze the next bounded native-only recovery. Five
near-half centers/both signs, endpoints offset0 and-2^52, width40/scale18,
critical DECIMALS. Both FIXED observations are emitted for EVERY query;
only the actual native ON text chooses a midpoint branch. Up to54 exact
integer-offset queries/group (1080 observations/run maximum), with every
input reconstructed from hex and checked against independent host BigInt.
No Copperfin or candidate formatter supplies strings or decisions.

The reader independently replays every queried midpoint, requires distinct
initial ON outputs and an adjacent-payload terminal EDGE with distinct ON
outputs for all10 groups, exact metadata/BRS, framing/totals and paired typed
repeat. An unbracketed endpoint, bad constructor, incomplete run or path
mismatch fails BEFORE repeat. Even a qualified result establishes only the
local witnessed edge, NOT monotonicity or a general conversion algorithm.
At source freeze no native invocation or VFP-syntax acceptance was claimed.
Source regeneration and Node syntax checks passed; no expected regression/
helper/model/hook was introduced.

Separate build/default-transform-rounding-edge will preserve earlier evidence.
Renewed full PSI55.61/26.01 after ladder release blocks further native work.
NEXT sign/push this source freeze, then obtain NEW explicit BOTH-chat CLEAR
and immediate full avg10 AND avg60<1%/idle-prefix guards for each invocation.
Do not duplicate the qualified3936/1872/2000/3872 pairs. Exact-rule recovery
still precedes independent expectations/actual ORIGINAL/support/wiring.

Fresh explicit audit/DAW CLEAR subsequently allowed the unchanged signed
4f8040d1c9eac65516a9b11e84b88355a82e1a1a source. Both actual native statuses0/
empty stderr/DONE1080; first structural gate0 BEFORE repeat, paired reader0,
RAW-BYTE-IDENTICAL SHA256
b6e43abea8d3290d46e231a72e0ec6807c25551b266cfa156d91f393f4baacc2.
All10 signed groups completed54 exact queries and reached adjacent payloads
with different ON strings. Every input/path/metadata/BRS check exact; archived
independent reader0/host byte compare0. Full PSI0.00/0.00 before both runs,
all input/runner/VFP9/FLL hash checks0. Both chats RELEASED/prefix empty1.

rounding-edge-captures/ retains all host receipts and full REA
ev_a3f65d05d1c73c2be15c40f366ecd85dd9b9465e0e36c56bc277d9431a4e6515:
root0/quiesced/owned-group verified/untruncated/no sampled ownership unknowns
in THIS capture. Image-binding/sampling/snapshot/inherited-environment/network
limits remain; no global cleanup claim. No build, VM or reservation retained.

Positive local ON edges (negative groups have the SAME magnitude bits and
opposite strings, including negative zero):

| Center / D | Lower payload | Upper adjacent payload | Lower / upper text |
| --- | --- | --- | --- |
| 0.5 / 0 | 3FDFF7CED9168729 | 3FDFF7CED916872A | 0 / 1 |
| 1.005 / 2 | 3FF01475A31A4BDB | 3FF01475A31A4BDC | 1.00 / 1.01 |
| 2.675 / 2 | 40056663C74FB549 | 40056663C74FB54A | 2.67 / 2.68 |
| 9.995 / 2 | 4023FD6FFC115DF6 | 4023FD6FFC115DF7 | 9.99 / 10.00 |
| 5E-16 / 15 | 3CC1FF130772E183 | 3CC1FF130772E184 | 0.000000000000000 / 0.000000000000001 |

These are witnessed local edges, NOT general monotonicity or uniqueness.
reject-rounding-hypotheses.cjs compares only the540 ON magnitudes against
three explicit HOST-BINARY64 hypotheses, all REJECTED: straightforward
floor(abs(x)*10^D+0.5)/10^D has198 mismatches; replacing0.5 with0.5005 has4;
rounding three extra decimal places first has4. Exact raw witnesses retained.
The two apparently close hypotheses fail at the0.5 and5E-16 edge payloads,
BOTH signs. Numeric comparison excludes padding/negative-zero/sign semantics
and says nothing about an x87 implementation. No tolerated mismatch, inferred
epsilon or fitted formatter is accepted as the requirement.

### DECIMALS0..18 exact-edge stimulus — first-only UNQUALIFIED

emit-rounding-decimals.cjs, rounding-decimals.prg,
rounding-decimals-cases.json and audit-rounding-decimals.cjs freeze the next
native-only experiment. Nineteen explicit finite binary64 stimuli near half
steps, DECIMALS0..18/both signs (38 groups), width40/scale18, BOTH FIXED
observations per query;54 queries/group maximum4104 observations/run.
Host Number selects explicit input BITS only, never an expected output.
The proven query/reader protocol is reused; every constructor/readback/path,
adjacent differing-output edge and paired repeat must qualify independently.
This probes setting-dependent behavior, not all magnitudes/descriptors or a
general algorithm. Separate build/default-transform-rounding-decimals.
No invocation yet; sign/push BEFORE renewed BOTH-chat CLEAR and immediate
full avg10 AND avg60<1%/idle-prefix gates. Existing qualified pairs need no
replay. Exact-rule recovery still precedes native-only regression/actual
ORIGINAL/unused support/direct verification/wiring and acceptance.

The unchanged signed c73d70876 source subsequently ran once after NEW explicit
audit/DAW CLEAR and immediate full PSI0.00/0.00/idle prefix. Actual native0/
empty stderr/complete DONE3480, BUT strict first structural reader1: the first
sixteen settings/both signs emitted32 local EDGE records, while D16..18/both
signs emitted six UNBRACKETED records with equal endpoint text. The repeat
was correctly NOT invoked. This is NOT a qualified paired capture, despite
the successful native process exit. First raw SHA256
8a2f03fb12b0e25a430163284dac832378f2f42b3ea61c1709d5898735a0a5fa.
The frozen strict reader and its failure remain unchanged.

rounding-decimals-attempt1/ retains ALL exact host receipts, failed strict
reader output/status and full REA
ev_b87e000140c3a68e610bbec850334a09d80de02fa3e03723b38431b6558b8298:
root1/quiesced/owned-group verified/untruncated/no sampled ownership unknowns
in THIS capture. Standard image-binding/sampling/snapshot/environment/network
limits remain. Input/runner/VFP9/FLL hashes unchanged; both chats RELEASED,
prefix empty1, no native/build/Wine/VM reservation.

audit-decimals-attempt.cjs is a separate FORENSIC diagnostic, not a replacement
qualification reader. Its archived readback0 verifies3480 complete observations,
32 local edges and exactly six known unbracketed controls, every actual query
path/constructor/metadata/BRS field. It explicitly reports
originalQualificationFailed:true/pairedQualified:false. It does NOT retroactively
qualify the first run or infer applied settings not captured in that source.

### Refined DECIMALS sweep with applied settings — first-only UNQUALIFIED

emit-rounding-decimals2.cjs, rounding-decimals2.prg,
rounding-decimals2-cases.json, audit-rounding-decimals2.cjs freeze the bounded
refinement, preserving the original source/inventory/reader/failure intact.
D0..15 retain their declared bits; D16..18 instead reuse the already QUALIFIED
5E-16 input bits3CC203AF9EE75616 from the1080 paired edge witness. This merely
selects a new stimulus; it is NOT a minimum-fractional-precision rule. Still
38 groups/both signs/width40/scale18/max54 queries/group and4104 rows/run.

Each record additionally reads SET('DECIMALS') and SET('FIXED'); the new reader
requires actual applied values equal the requested group setting and mode.
Every query/metadata/BRS/control/framing/adjacent differing-output edge and
paired repeat must independently qualify, with first structural gate BEFORE
repeat. No output synthesis or guessed tolerance. Separate host directory
build/default-transform-rounding-decimals2. Regeneration/Node syntax/bash-n
checks precede signing; no native invocation yet. NEW explicit BOTH-chat CLEAR
and immediate low-full-PSI/idle-prefix gates remain mandatory for EACH invocation.
No owner-input blocker. Exact-rule recovery remains before native-only
regression/actual ORIGINAL/unused helper/direct checks/wiring and acceptance.

The unchanged signed0722c6860 source subsequently ran once after NEW explicit
audit/DAW CLEAR, full PSI0.00/0.03 and idle prefix. Actual native0/empty stderr/
DONE4104, BUT all4104 observation rows are ERROR11 (Function argument value,
type, or count is invalid) from the authored combined setting-readback
expression. The strict first reader1 correctly rejects five-field ERROR rows
instead of required12 fields; repeat NOT invoked.38 feedback EDGE rows are
not enough to establish input/path/metadata/BRS/applied settings. No qualified
pair or Copperfin defect is inferred. The exact failing subexpression/SET
return type was NOT isolated by this capture. Raw SHA256
3410b6884d72f4cf17823226dca2479eba01142cbfaddacbc1ac3138190d64b7.

rounding-decimals2-attempt1/ retains every exact host receipt, failed strict
reader, failure summary and full REA
ev_2e775fabbc279bea57b4966bebc9c6174ddb50036137475fa6d931367cac0067:
root1/quiesced/owned-group verified/untruncated/no sampled ownership unknowns
in THIS capture. Standard image-binding/sampling/snapshot/environment/network
limitations remain. Input/runner/VFP9/FLL hash checks0, both chats RELEASED,
prefix empty1/no resource reservation. Earlier failed/qualified receipts intact.

### Type-aware setting-readback refinement — qualified

emit-rounding-decimals3.cjs, rounding-decimals3.prg,
rounding-decimals3-cases.json and audit-rounding-decimals3.cjs freeze the next
source/inventory/reader. Same38 groups/bits/query bounds/width40/scale18/BOTH
FIXED/max4104 records. Setting readback no longer passes an unknown return
type to ALLTRIM: VARTYPE plus hex TRANSFORM text record BOTH native SET values.
The reader requires a complete numeric DECIMALS value equal to the requested
count, actual FIXED text equal to the requested mode, and exact type/text repeat;
then every constructor/path/metadata/BRS/adjacent edge/completeness gate still
applies. It is not a SET product change or presumed native type rule.

Separate build/default-transform-rounding-decimals3. Source regeneration,
Node syntax/worker bash-n and applied-count regex checks precede source signing.
No native invocation yet. NEW explicit BOTH-chat CLEAR and immediate BOTH
full PSI windows<1%/idle prefix before EACH invocation. Exact conversion-rule
recovery remains before expected regressions/actual ORIGINAL/support/wiring.

Type-aware source freeze395f3829ffee866ff9e2483fc9e5e0aa6bec8392 is signed/
DCO/pushed. NEW explicit audit/DAW CLEAR at full0.00/0.00 preceded the worker,
but its immediate pre-FIRST snapshot was full38.92/9.91. Guard75 prevented
BOTH native invocations. No first native status/output exists; no VFP syntax
or type-aware setting-readback acceptance is claimed. Prefix empty1/both chats
RELEASED; no standing reservation or owner-input blocker.

rounding-decimals3-guard1/ retains the full guard receipts and REA
ev_4f4b4529244ebb5144f453177aabe83554f38d91e7400344993cc77da33b9403:
root75/quiesced/owned-group verified/untruncated/no sampled ownership unknowns;
standard observation/image/environment/network limits remain. The host guard
directory was moved, not overwritten, to
build/default-transform-rounding-decimals3-guarded1; its archived worker must
NOT be run. Original build/default-transform-rounding-decimals3 was recreated
with ONLY the identical prepared worker so the unchanged signed PRG output
path can be used in a NEW capture window without overwriting guard evidence.
NEXT fresh explicit BOTH-chat CLEAR and immediate low-full-PSI/idle-prefix
guards; no retry in this pressure episode. All earlier attempts stay intact.

The unchanged signed395f3829f native source subsequently ran at clean signed
headacd08561f after NEW explicit audit/DAW CLEAR. Both actual native0/empty
stderr/DONE4104; first structural0 BEFORE repeat, paired reader0. All4104
records RAW-BYTE-IDENTICAL SHA256
6972cb22232ddd776ec25491bb461b051e88c297d5e98d1bf9eea5bd519e1a3f.
All38 signed groups reach adjacent different-ON-output payloads; every
constructor/query path/width40/scale18/BRS/applied setting exact. Native SET
DECIMALS returns typeN, SET FIXED typeC in every captured row. Archived
independent reader0/host-byte-compare0; all source/runner/VFP9/FLL hash checks0.
Full PSI0.00/0.00 immediately before EACH native run. Both chats RELEASED,
prefix empty1/no resource reservation. Older failures and guard remain unchanged.

rounding-decimals3-captures/ retains ALL host receipts and full REA
ev_c2b005708e8f209f2bd8529cead52a6ad3c7ea5393045ad225ad038e9773b453:
root0/quiesced/owned-group verified/untruncated/no sampled ownership unknowns
in THIS capture; standard image-binding/sampling/snapshot/environment/network
limits remain. No global cleanup or implementation parity claim.

Observed positive local edges (negative groups have the same magnitude bits
with opposite strings, including negative zero):

| Applied D | Lower payload | Upper adjacent payload | Upper ON text |
| --- | --- | --- | --- |
| 0 | 3FDFF7CED9168729 | 3FDFF7CED916872A | 1 |
| 15 | 3CC1FF130772E183 | 3CC1FF130772E184 | 0.000000000000001 |
| 16 | 3CC203398FC21739 | 3CC203398FC2173A | 0.0000000000000010 |
| 17 | 3CC203A3D096CFCB | 3CC203A3D096CFCC | 0.00000000000000100 |
| 18 | 3CC203AE70AC1573 | 3CC203AE70AC1574 | 0.000000000000001000 |

These D15..18 edges are DIFFERENT despite their similar padded upper values;
the settings were not simply ignored. This is NOT a general precision cap.
reject-preadd-hypothesis.cjs rejects a further post-hoc HOST-BINARY64 magnitude
hypothesis: pre-add5*10^(-D-4), then scale/half-round at MIN(D,15).16 of2052
ON observations disagree. All raw counterexamples retained; no tolerated
mismatch, guessed epsilon or accepted formatter follows from the close fit.
Numeric magnitude comparison is not exact sign/padding/text verification.

### Native public-arithmetic discrimination — qualified528

emit-rounding-arithmetic.cjs, rounding-arithmetic.prg,
rounding-arithmetic-cases.json and audit-rounding-arithmetic.cjs freeze the
next bounded experiment. Six explicit payloads around each of44 QUALIFIED
signed local edges (38 from this pair, six D2 edges near1.005/2.675/9.995
from the1080 pair):264 inputs/528 observations per run, width40/scale18,
both FIXED modes and type-aware applied-setting readback.

The public-language provisional arithmetic graph records BRS payloads of
10^(-D), its product with0.0005, ABS(input)+that product, MIN(D,15),10^q,
shifted*10^q, that value+0.5 and INT(result), plus their VARTYPE values.
Scalar assignments are explicitly part of this experiment. It is a
DISCRIMINATOR for a rejected hypothesis, not an accepted formatter, cap,
tolerance, arithmetic-order requirement or expected-string producer. Actual
TRANSFORM/STR observations remain independent. The reader requires every
original payload/descriptor/applied setting, all finite arithmetic bits,
complete unique coverage and typed repeat; it does NOT synthesize or approve
the candidate outputs. First structural gate BEFORE repeat remains mandatory.

Source freeze1cf234454 is signed/pushed and regeneration/bash-n checks pass.
After NEW explicit audit and DAW CLEAR, the immediate first invocation guard
observed full PSI55.32/24.52 and exited75 BEFORE VFP. No first/repeat native
status, stdout, stderr or results exists; this is resource evidence only,
not a failed native probe or qualified arithmetic observation.
rounding-arithmetic-guard1/ retains ALL host receipts/full REA
ev_230e66f1ba7243ed95db75a3e3863d3b9c4b59c6b4c1595da5f963f9ea7aaa8a:
root75/quiesced/owned-group verified/untruncated, normal image-binding,
sampling, filesystem-snapshot and inherited-environment limitations retained.
Independent archived readback verifies every compressed receipt byte-equal to
the host, exact guard/source status, unchanged source/native hashes and empty
prefix checks1 before/after; archive manifest0. Both chats RELEASED promptly.
Original host receipts moved intact to build/default-transform-rounding-arithmetic-guarded1;
fresh original directory contains ONLY the identical prepared worker, preserving
the frozen PRG's hardcoded output path without overwriting receipts. Do not run
the archived worker. NO native invocation/reservation/owner-input blocker.
NEXT reuse unchanged signed source1cf234454, NEW explicit BOTH-chat CLEAR and immediate BOTH full PSI
windows<1%/idle prefix before EACH invocation. Exact-rule recovery still precedes
native-only expectations/actual ORIGINAL/unused support/direct checks/wiring.

The preceding paragraph records the guard-only checkpoint, NOT current run
status. On a NEW explicitly cleared audit/DAW window at signed/DCO clean head
962594cf0297b91ef3121d1ba42c55517fc17008, unchanged source1cf234454 completed
BOTH actual runs0/empty stderr/DONE528. Immediate full PSI0.00/0.00 and prefix
empty1 before EACH run. First structural reader0 BEFORE repeat; paired0,
all528 raw records byte-identical SHA
2917ff9167e4a2752520614776580d08dda8fa4ba1520615154912bd937bd1c9.
Original input/descriptor/BRS/applied settings exact; all eight finite
intermediates numeric and equal across FIXED modes/typed repeats. Archived
independent reader0/host-audit compare0/all receipt byte-compares0/manifest0.
Both chats RELEASED promptly, post-prefix empty1; no build/compiler/VM/new FLL.
rounding-arithmetic-captures/ retains ALL host receipts and complete REA
ev_91adec73e74ffd0b13dc8004895fe694d2878feb3ccb7c170a5f5c8bcb33f510:
root0/quiesced/owned verified/untruncated/no sampled ownership unknowns;
image-binding/sampling/snapshot/inherited-environment and unobserved-network limitations remain.
An initial inline archive byte-check had a host JavaScript syntax error;
its corrected independent check actually passed. No native capture was replayed
or qualification reader weakened.

analyze-rounding-arithmetic.cjs rejects the OBSERVED native scalar graph:
18/264 ON magnitudes disagree with actual TRANSFORM. Comparison uses exact
decimal integer units, not a Number(string) approximation. Raw strings and
all counterexamples remain retained; sign/padding/negative-zero/text equality
are NOT claimed. Host/native powDown and bias bits differ36 times each, but
all later recorded steps coincide in this finite set; power differences ALONE
do not explain the formatter discrepancy. No cap, epsilon, operation order,
internal precision, call attribution or production formatter is established.

### Next bounded conversion-boundary experiment — source freeze, not built/run

Do not replay the seven qualified pairs. Prepare an OWN native-only FLL wrapper
for installed32-bit MSVCR71 _fcvt/_ecvt, using documented C ABI and the existing
published FLL interface, NOT guessed VFP DECLARE calling convention. Observe
original and already-observed shifted payloads, actual input bits/count/sign/
decimal-point/digit buffer and control word before/after, alongside independent
TRANSFORM/STR. Bounds: the same264 finite inputs, applied count1..18 for _fcvt (omit0),
positive15/16/17 significant counts for _ecvt; copy static return buffer
immediately, bound reads, reject NULL/error/overflow and preserve failures.
The following source-only checkpoint now implements this bounded experiment,
but no compiler/linker/native invocation or binary exists yet.

Public signatures/count meanings/static-buffer lifetime are documented by
[Microsoft _fcvt](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fcvt?view=msvc-170)
and [Microsoft _ecvt](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/ecvt?view=msvc-170).
These current interface documents are NOT version-specific VFP rounding
evidence. The bounded read-only PE export inventory, retained as
crt-inventory.txt.gz, finds _fcvt/_ecvt/_gcvt in installed32-bit
syswow64/msvcr71.dll SHA8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe.
This proves neither that VFP calls them nor that 64-bit Wine's separate
system32 DLL is equivalent; do not use that other DLL. Bind runtime module
identity in the future wrapper capture. No decompilation/internal attribution.
Freeze/sign native-only sources/readers and request NEW BOTH-chat compiler/
native clearance with immediate BOTH full PSI<1%/idle-prefix gates; no VM.
At the qualified528 checkpoint, full48.97/21.69 failed, so no additional build/native invocation followed.
If these functions disagree, preserve rejection and recover the next actual
conversion boundary rather than accepting a close fit. Exact-rule recovery
still precedes expectation freeze/actual ORIGINAL/support/direct/wiring/SAME.

crt-probe.c/crt-kernel32.def implement the OWN32-bit FLL wrapper using the same
published interface/SDK entry pattern as cause-probe.c. GetModuleHandleA,
GetModuleFileNameA and GetProcAddress query the already-loaded MSVCR71; no DLL
load/replacement/CW changes. Correct C calling convention is explicit. Reject
nonfinite/wrong type/count before conversion, require bounded integral counts,
copy at most127 digits immediately from the CRT static buffer, reject NULL/
invalid characters/truncation, and retain exact count/input bits/sign/decimal/
CW before+after/module path/digit bytes. No proprietary binary enters the repo.

emit-crt.cjs/crt.prg/crt-cases.json freeze264 original and already-observed
shifted inputs from the qualified528 pair, NOT recomputed candidate arithmetic.
Both FIXED modes; _fcvt applied D1..18 only, _ecvt15/16/17;4176 rows/run.
Independent TRANSFORM/STR, original/requested metadata/BRS/settings, and
post-call metadata/BRS/display controls accompany every CRT observation.
audit-crt.cjs requires actual0/empty stderr/full unique coverage, exact
arguments/CW/module path/unchanged controls/typed repeat; NO digits are
generated or compared against a model. First strict --single BEFORE repeat.
Unexpected runtime module path remains a qualification failure to investigate,
not permission to accept64-bit Wine's different file or silently relax identity.

crt-build.sh/crt-capture.sh isolate build/default-transform-crt, reject
attempt-directory overwrite and dirty sources, retain source/tool/SDK/input/
binary hashes, native results/statuses/stderrs and per-stage pressure/prefix
guards. NEW explicit BOTH-chat clearance remains external prerequisite for
EACH window, with immediate full avg10 AND avg60<1%/idle prefix before EACH
import-generation/compiler/link/native invocation. No compiler syntax check
has run yet. Node syntax/source+inventory regeneration/cmp and bash-n returned0.
Do NOT archive/distribute generated SDK-containing FLL/object/import libraries;
retain them host-only, archive first-party source/FXP and plain receipts only.
SDK debug-section linker warnings, if any, must remain recorded.

LOW DQ-DV001 developer self-review walkthrough: inspected the count/type/finite
and buffer bounds, no-load/no-CW-write module lookup, strict argument/control
and module checks,4176 enumeration and fail-before-repeat flow. Source-only
automated checks above passed; ABI/build/native execution remain unverified.
No independent-human/system/release assurance, known-bug exception or closure.
Source-freeze resource check full0.64/21.27 fails avg60; no compiler/native
window requested or job started, no reservation or owner-input blocker.

### First CRT build attempt — import0, compile1, no native invocation

NEW explicit audit/DAW CLEAR obtained at unchanged signed sourceaccdbe40f.
Immediate full0.00/0.01 and prefix empty1 before BOTH import and compiler.
Import-generation0; Clang1 at two historical generic-function-pointer casts
under -Werror,-Wcast-function-type-mismatch. No linker/native invocation,
build-finished marker or wrapper FLL exists. No rounding observation follows.
Both chats RELEASED promptly, post-prefix empty1.
crt-build-attempt1/ retains ALL plain host receipts, failed first-party C/
build source and complete REA
ev_f58bc2e20aabe068adc2b9de0ddc073cd5475234a6427fac253d6714c5d6c5cc:
root1/quiesced/owned verified/untruncated; standard sampling/image-binding/
snapshot/inherited-environment/network limitations remain. Independent archived
byte readback0 verifies import0/compile1 and absence of link/native/FLL.
The import library remains HOST-ONLY; original host directory moved intact to
build/default-transform-crt-build-attempt1. Original build path is now absent.

Correct only the OWN dispatch-pointer C type to the actual void fastcall
handler and remove the incompatible casts; retain -Wall/-Wextra/-Werror and
add packed PublicInfo size14/function-address offset4 assertions. No public
table bytes/caller/CRT experiment/reader/production semantics are changed.
This corrected source is UNBUILT; freeze/sign/push it BEFORE NEW compiler/native
window and fresh BOTH-chat CLEAR/guards. Do not replay the failed source or
overwrite its receipts. LOW developer walkthrough confirms the correction
keeps the packed code-address layout and strengthens rather than skips compile
validation; execution remains pending, not a verified ABI or parity claim.

### Corrected CRT build0; first native4176 UNQUALIFIED on module identity

NEW explicit audit/DAW CLEAR at unchanged signed/DCO clean7d706418f.
Import/Clang/link all actually0; -Wall/-Wextra/-Werror retained, compile stderr
empty. Four installed SDK .debug$S linker warnings remain recorded; not a
warning-free build. Result HOST-ONLY32-bit FLL SHA
318adabbf8f235d2596ae28c6ddcc5c8eb44198c5d7ec2871b2264a0e0d92af6.
Build input hash-check0. Full PSI0.00/0.00 and idle prefix before all three
stages. Full REA build record
ev_c41f80a8d204ee0da213e47d487ce9012cf09f920891e0bada52faa070b7ba73.

First native actual0/empty stderr/full DONE4176, likewise immediate full
PSI0.00/0.00 and idle prefix. BUT strict --single returned1: runtime
GetModuleFileNameA reports C:\windows\system32\MSVCR71.dll, not frozen
syswow64 path. Repeat correctly NOT invoked. No pair/ABI/formatter/conversion
qualification; do NOT simply admit the different path. Host system32 contains
the separately inventoried64-bit Wine DLL while syswow64 is the32-bit file;
the actual runtime loaded image/backing-file mapping still needs direct
read-only evidence rather than inference from the path string.
Raw SHA98cf75e53306f441f2359b63fb84e19134c8aee8fe97889794b851d395134c96.
Separate framing/module diagnostic verifies4176 non-error20-field rows and
that one observed module path, explicitly qualified:false; it does NOT replace
the failed original reader or qualify metadata/controls/conversion semantics.
First native source/binary/hash checks0. Both chats RELEASED/prefix idle1.

crt-native-attempt1/ retains ALL plain receipts, first-party caller/FXP/
wrapper/reader/inventory and complete native REA
ev_619d964368741cc71b9868ea74871296d035a6a3a3bb54340be07c2fdbaa07cb.
Both REA roots report owned-group verified/quiesced/untruncated; all original
sampling/image-binding/filesystem-snapshot/environment/network limitations
and residual unknowns retained. Archive bytes compared against host0/manifest0.
SDK-containing FLL/object/import libraries remain HOST-ONLY in intact
build/default-transform-crt-native-attempt1; original build path absent.
Earlier compile1 attempt remains untouched. No replay of qualified/failed
probes, no weakened reader, no tolerance or internal VFP call attribution.

NEXT bounded recovery: strengthen the OWN observer with read-only loaded-image
PE32 header identity plus a bounded SHA256 of the file opened through the
reported path INSIDE the32-bit process. Cross-check the hash implementation
with standard known-answer/direct host checks before trusting it; bind the
digest and loaded PE header facts against the frozen32-bit artifact, retaining
atomic image-binding limitations. Consult authoritative Win32 file/module
and SHA256 documentation while preparing this change. No guessed path rewrite,
silent module allow-list expansion, proprietary file export or DLL/CW changes.
This binding enhancement/caller/reader protocol is NOT implemented/frozen yet;
sign before NEW BOTH-chat-cleared compilation/native capture. No owner input
blocker. Exact-rule/expectation/ORIGINAL/support/wiring gates remain unchanged.
LOW developer walkthrough verified strict module failure prevents repeat and
receipts remain unqualified; next binding requires its own direct checks,
not retroactive acceptance of this capture.

### Runtime-path binding enhancement — source freeze; UNBUILT/UNRUN

crt-binding.h is OWN freestanding SHA256/header support, not proprietary
implementation or a formatter model. Algorithm provenance:
[NIST FIPS180-4](https://doi.org/10.6028/NIST.FIPS.180-4) sections4.1.2/4.2.2/5/6.2;
[NIST SHA256 examples](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHA256.pdf)
supply abc and56-byte known answers. Empty/abc/56-byte streaming self-tests run
inside the FLL before hashing. Unsigned32-bit words, bounded8MiB input, defined
rotations/length padding; no allocation/CRT hashing provider/new DLL load.
No FIPS validation/security certification claimed.

Read-only module/file protocol follows public Microsoft
[module path](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamea),
[PE format](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format),
[VirtualQuery](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualquery),
[explicit32-bit memory information](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-memory_basic_information),
[CreateFileA](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea),
[GetFileSizeEx](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesizeex),
[ReadFile](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile) and
[CloseHandle](https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle).
[WOW64 redirector](https://learn.microsoft.com/en-us/windows/win32/winprog64/file-system-redirector)
documents Windows behavior only, not evidence of this Wine mapping.

Observer requires committed/readable/unguarded MEM_IMAGE header memory at the
queried module allocation; parses at most4096 header bytes, validates MZ/PE32/
machine014C and bounded offsets. Opens the REPORTED path verbatim from INSIDE
the32-bit process, GENERIC_READ/FILE_SHARE_READ/OPEN_EXISTING. Bounds file4096..
8MiB, reads4096-byte chunks/at most4096 successful reads, checks expected size
and EOF, closes the handle on every opened path, compares file/loaded public
machine/magic/timestamp/SizeOfImage/checksum and hashes all file bytes. Emits
no proprietary bytes. Caches that sample ONCE per FLL lifetime; every later
call rechecks module/path/loaded headers. No claim of atomic mapped-content/
file binding, per-conversion rehash, protection from host writes or internal
VFP formatter call attribution. No DLL load/replacement/CW modification.

The NEW composite reader requires the previously OBSERVED system32 path PLUS
exact in-process file digest8094af5e.../size348160 AND matching loaded/file PE32
machine332/magic267/timestamp1045831340/imageSize352256/checksum389609, frozen
from read-only host headers of the32-bit syswow64 artifact. It does not accept
the host system32 Wine64 file, infer equivalence from a path, or retroqualify
crt-native-attempt1 (which lacks every new identity field). Old source/reader/
inventory/raw failure remain immutable there. Same264 original/qualified
shifted payloads/4176 rows, count bounds, independent display/settings/metadata/
BRS controls and strict-first-before-repeat; digits never generated by a model.

crt-binding-check.c/cjs directly test the EXACT shared header before FLL build:
GNU and Clang ASan/UBSan, three known answers, deterministic binary messages at
padding/block boundaries0/1/3/55/56/63/64/65/119/120/127/128/129/4095/4096/4097,
million-a known answer, chunk sizes1/55/64/4096, installed32-bit CRT digest vs
independent Node crypto, and malformed/truncated PE-header rejection.72 test
invocations/compiler, EACH with immediate full avg10 AND avg60<1%/idle prefix
and recorded status/stdout/stderr. Build stages retain the same guards plus
NEW explicit BOTH-chat clearance BEFORE requesting execution. No host compiler,
test executable, cross-build or native invocation has yet run this source.
Node syntax/emitter PRG+inventory cmp/bash-n/diff-check all0; these are source
checks only. SDK-linked outputs remain HOST-ONLY, failures preserved unchanged.

LOW DQ-DV001 developer self-review walkthrough: checked file-handle cleanup,
read/hash/header/output bounds and explicit cache limitations, old failed
capture rejection, hash-before-native sequencing and first-only failure gate.
Current full PSI1.00/1.55 fails; no compiler/native window requested, no standing
reservation or owner-input blocker. NEXT sign/push this native-only checkpoint,
then fresh BOTH-chat clearance/direct checks/build/native FIRST gate/repeat.
Do not resume production/expectation/ORIGINAL work before exact-rule recovery.

### Binding build attempt1 — direct checks0, cross-compile0, link1; no native

At signed/DCO clean176224a01 after NEW explicit BOTH-chat CLEAR, GNU/Clang
ASan/UBSan each compiled0 and each actually executed all72 hash/header checks0/
empty stderr. Every independent digest (including installed CRT) and known
answer matched; PE bounds tests passed.144 individual test prefix/pressure/
status/stdout/stderr receipts retained. Import-generation0 and32-bit Clang0/
empty stderr; link1: undefined _memcpy referenced by bind_module's structure
assignment. Four installed SDK .debug$S warnings remain. No FLL/build-finished/
native invocation. All stage and individual test immediate full avg10/avg60
guards<1%/prefix empty1. Source/tool/native hashes unchanged0; BOTH chats
RELEASED promptly, no standing reservation.

crt-binding-build-attempt1/ archives105 byte-compared generated receipts/OWN
sources/test inputs and complete REA
ev_3e83f3df48e30e8ac01d60ae3e95aa0e375f9c5281330c60f46b809534ae5863.
Archive independent readback0/manifest0. The first manifest CLI call used the
wrong working directory and returned1 (no file); corrected exact archive cwd
returned0, no content changed. Host directory moved intact to
build/default-transform-crt-binding-build-attempt1; original path absent.
SDK-containing object/import libraries and host executables stay HOST-ONLY.

REA root1/quiesced/owned group verified, terminal/filesystem untruncated BUT
process sample_limit60 reached: overall truncated:true, process sampling
INCOMPLETE. Three unrelated /proc environment ownership checks EACCES; those
processes left untouched. Original process/image-binding/snapshot/environment/
network limitations and unknowns retained. Do not claim full descendant
coverage or recapture this completed failure to erase the limitation.

Correct only OWN cached CrtPe assignment to five explicit unsigned scalar
copies, removing the compiler-generated freestanding memcpy dependency without
new imports/providers, ABI/output/reader/hash algorithm or production changes.
This correction is UNBUILT; sign/push BEFORE NEW BOTH-chat compiler/native
clearance/guards. The direct checks above apply to unchanged shared hash/header
source, not proof of the corrected FLL link or runtime binding. LOW developer
walkthrough confirms exact field coverage and preserved failed receipts; no
formatter/native qualification, exception or independent-human acceptance.

### Corrected binding build0; first native TIMEOUT/UNQUALIFIED, no repeat

At unchanged signed/DCO clean7c58dcbf8, a NEW BOTH-chat compiler-only CLEAR and
all immediate guards preceded repeated GNU/Clang ASan/UBSan compilation and
72 actual checks each0/empty stderr. Import/Clang32/link now all0; four SDK
debug-section warnings retained. HOST-ONLY FLL SHA
d86e224a7fdda75625838296c89eed8691517975cc17f8f83e7ddaec40ce9610.
Source/tool/native/binary hash checks0. Compiler window explicitly RELEASED.
Build REA ev_4f7c967ddb570d6ea8ae683fd492a0dcde37e971fc3aad8f21b51aa153018c6d:
root0/quiesced/owned verified; terminal/filesystem untruncated but process limit60
reached => overall truncated:true/incomplete process sampling. One unrelated
EACCES ownership unknown left untouched; all original limits/unknowns retained.

Separate NEW explicit BOTH-chat native-only CLEAR followed. Immediate full
avg10/avg60<1% guard exit0 and idle-prefix exit1/empty passed; first wrapper
returned actual124 after its25sec timeout. stdout empty, stderr70 bytes records
timeout. Result file partial703242 bytes/1626 complete20-field rows, ALL
ERROR:MODULEBINDING, NO DONE. Raw SHA
23d814cc7b2caf2b9f1ada4bf9530935e3fcbd88fc9a06c3d5e24ca1aa3af640.
Strict first reader and repeat were NOT invoked because actual native status
already failed. No ABI/conversion/module-file/display qualification and no
retroqualification of the earlier4176 attempt. Generic failure does NOT reveal
which binding check failed; do not guess path redirection, image header or
hash fault from this receipt. Failed binding is attempted per call because
only success is cached, so do not replay this full4176 worker.

Native REA ev_118f96a67432dd223a2e334bdf2031e916a527238a5f35cb94716fa2e82d327c:
root124/quiesced/owned verified/untruncated; no sampled ownership unknowns in
THIS run. The explicit native capture snapshot budget160 files/40MB covers
extended identity protocol/build+native receipts, not additional test scope;
other bounds unchanged. All original process-sampling/atomic-image-binding/
filesystem-snapshot/inherited-environment/network limits remain. Both chats
RELEASED promptly; post-prefix empty1. No standing reservation/VM/owner blocker.

crt-binding-native-attempt1/ retains134 byte-compared generated receipts,
OWN sources/caller/FXP and BOTH complete REA records. SDK-containing object/
import/export libraries/FLL and host executables stay HOST-ONLY in intact
build/default-transform-crt-binding-native-attempt1; original build path absent.
All source/tool/native/binary/post-attempt input checks0. Separate
audit-crt-binding-attempt.cjs host+archived forensic reader0/manifest0 verifies
partial framing/errors/status and direct host receipts only, explicitly
qualified:false/pairedQualified:false. It is NOT the skipped strict reader or
proof of any successful runtime binding/conversion semantics.

NEXT bounded recovery, NOT implemented yet: prepare a ONE-CALL native-only
binding diagnostic, exposing bounded public path/VirtualQuery/PE facts and
distinct operation/status errors for size/read/header/digest, without exporting
proprietary bytes or weakening any identity requirement. Stop the PRG at the
first observer error; never spend another4176 calls repeating a failed binding.
Freeze/sign source/reader first, then fresh BOTH-chat compiler/native windows
and immediate guards. Do not run the unchanged failed worker or reuse attempted
directories. Only after one-call exact identity/control proof may a separately
frozen full4176 caller run; first strict gate still precedes repeat. Preserve
failures and cache semantics explicitly. No production/hash-provider/CW/DLL
replacement, guessed path acceptance, epsilon, cap or formatter-model adoption.

LOW DQ-DV001 developer walkthrough: checked actual124 prevents strict/repeat,
all1626 rows remain unqualified, original source/build/native receipts immutable,
host-only binary exclusion, next single-call-before-full recovery ordering.
Forensic verification above actually passed; no independent-human/system or
release acceptance, hazard/compatibility/parent closure.

### ONE-CALL staged binding diagnostic — source freeze, UNBUILT/UNRUN

CFNBIND is an OWN zero-argument diagnostic entry in the same published packed
FLL table (new third entry; original conversion entries unchanged). It permits
only one diagnostic per FLL lifetime, refuses reuse/parameters/already-cached
binding, calls no CRT conversion and changes no control word. It records the
FIRST failed binding operation/predicate plus immediate
[GetLastError](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror)
only where the API documents failure error state. Zero error on a predicate
failure is explicit, not an API success claim. Close outcome/error is separate;
an earlier read failure is preserved even if cleanup also fails.

Stages: MODULE_API/PATH_API/PATH_BOUND, QUERY_API/QUERY_SIZE/QUERY_PREDICATE,
IMAGE_HEADER, CACHE_IMAGE/CACHE_PATH, SHA_SELFTEST, OPEN_API/SIZE_API/SIZE_BOUND,
READ_LIMIT/READ_API/READ_BOUND, EOF_API/EOF_BOUND, CLOSE_API/READ_SIZE,
FILE_HEADER/HEADER_EQUAL, or BOUND. Public facts: bounded path, returned query
byte count/base+allocation match booleans/region size/state/protection/type,
validity-tagged loaded/file PE fields, self-test outcome,64-bit file size words,
bytes/read count/close result, validity-tagged file SHA256 and CW before/after.
No process pointers, proprietary bytes, instructions or disassembly are emitted.
Output bound2048 (max518 path hex + fixed protocol less1600), header/read/hash
bounds unchanged. No aggregate struct copies/new provider or DLL loading.

Every ORIGINAL binding predicate remains mandatory. Digest now finalizes after
complete bounded read/EOF/close but BEFORE header equality, allowing a failed
HEADER_EQUAL diagnostic to retain the sampled file hash; hash alone never sets
bound. BOUND still requires loaded/header/file checks and cache field copies.
Shared crt-binding.h/hash/header algorithm remains unchanged from the actual
GNU+Clang72-check receipts. New FLL/table/diagnostic execution is unverified.

crt-binding-diagnostic.prg invokes CFNBIND exactly ONCE, no conversion/full4176
loop, and writes VERSION/one BIND record/DONE1 then returns regardless of bound
status. DONE1 means diagnostic completion, NOT identity success. Independent
pre/post Numeric0.5 width40/scale18 metadata/BRS/display/CW controls accompany
the one call. audit-crt-binding-diagnostic.cjs separates complete framing/
unchanged-control diagnosis from --require-bound acceptance. Failure details
are retained with bindingEstablished:false; --require-bound returns2. A claimed
BOUND must also pass exact frozen32-bit path/digest/size/header facts and every
memory/read/control predicate. No CRT/display/formatter/pair qualification
follows even if this one-call check succeeds. No path-only bypass or old-capture
retroqualification; prior strict reader and failed sources remain immutable.

crt-binding-diagnostic.sh --build and --capture use a NEW isolated directory,
reject attempted modes BEFORE overwriting receipts, require clean signed source,
retain all tool/source/native/binary hashes/actual statuses/stderr and immediate
full avg10 AND avg60<1%/idle prefix before each import/compiler/link/native
invocation. Each mode needs separate fresh explicit BOTH-chat CLEAR; script
arguments do not grant clearance. No repeat/full caller is reachable. Native
failure exits before reader; reader statuses are always recorded. Do not invoke
crt-build.sh/crt-capture.sh or attempted workers as a fallback. SDK-linked
object/libs/FLL remain HOST-ONLY. No VM/session/setup/install/config change.

Node syntax/bash-n/diff-check0; no compiler/test executable/native invocation
yet. LOW DQ-DV001 procedural delta/walkthrough: traced each failure through
close/record/return, one-call guard, separate structural-vs-binding gates and
fresh-directory guards. Developer self-review only, not independent-human or
acceptance evidence. Misuse mitigated: DONE/diagnostic0 cannot be reported as
successful binding or a formatter oracle. Rollback: exact diagnostic-source
revert, keep all failure receipts and notify owner if evidence is invalidated.
Current full41.16/12.65 fails; no window requested/reservation/owner-input blocker.
NEXT sign/push source, then only if pressure recovers request fresh BOTH-chat
compiler-only CLEAR; after build0/release, separate NEW native-only CLEAR.

### ONE-CALL pre-import guard1 — no import/compiler/link/native invocation

Unchanged clean signed/DCO be3ebe9693abd29615c08553827fe1b3c038fc14 after
NEW explicit BOTH-chat compiler-only CLEAR. Pressure had recovered0.00/0.79
before request; immediate pre-import full8.41/2.04 failed the mandatory guard75.
Prefix idle exit1/empty. NO import command, compilation, link, FLL or native
invocation occurred. BOTH chats RELEASED promptly; no standing reservation.
The diagnostic source remains UNBUILT/UNRUN, not a failed compiler/native test.

crt-binding-diagnostic-guard1/ retains17 byte-compared plain guard/OWN source
receipts and complete REA
ev_d97dfeca257dd96dfca09d8adb47cd1e8042e7d051535e930d034751bca32b35:
root75/quiesced/owned verified/untruncated/no sampled ownership unknowns. All
original process-sampling/atomic-image-binding/snapshot/inherited-environment/
network limitations remain; root75 is not compiler or native evidence.
Archive readback0/manifest0. Original host directory moved intact to
build/default-transform-crt-binding-diagnostic-guarded1; original build path
absent. No SDK binary/object/import library was generated; installed inputs
unchanged. Do NOT invoke archived/guarded worker or overwrite attempted mode.
LOW DQ-DV001 developer walkthrough/readback verifies guard prevents the FIRST
import and preserves all attempted state, with no identity or resource-policy
relaxation. NEXT same frozen source only after NEW BOTH-chat compiler-only
CLEAR/immediate guards at a fresh root; build0/release precedes a separate new
native-only window. No VM or owner-input blocker; recurring work stays active.

### ONE-CALL build0/diagnosis0; HEADER_EQUAL binding2 — provenance gap exposed

Unchanged signed/DCO clean27a1c66e9 (observer sourcebe3ebe969), NEW BOTH-chat
compiler-only CLEAR/full guards/idle prefix: import/Clang32/link all0, compile
stderr empty, four installed SDK .debug$S warnings retained. HOST-ONLY FLL SHA
789ab0c0860692fe87e3087a31cbc3a10b700d0ce500a10f895cb9444c68766c.
Compiler window RELEASED; separate NEW BOTH-chat native-only CLEAR/guard then
ONE CFNBIND call: actual VFP0/empty stderr/DONE1, structural/control reader0,
but exact --require-bound2 at HEADER_EQUAL, error0/bound0. No CRT conversion,
full4176 caller or repeat. Raw SHA
26d2694c98da1a87bd787d1ff3ae59841d11f6beab046206dbdd847753c8b214.

Observed loaded PE32 machine014C/magic010B/header timestamp69C2DC77,
SizeOfImage00273000/checksum002B0A4C; committed/readable/unguarded MEM_IMAGE
region4096 at matching base/allocation, query28 bytes. In-process opening of
reported C:\windows\system32\MSVCR71.dll read348160 bytes in85 reads/EOF/close0
and self-tests passed; file SHA8094af5e... is the frozen Microsoft32 artifact,
but its timestamp3E561EAC/imageSize00056000/checksum0005F1E9 differ. CW027F and
Numeric control metadata/BRS/display unchanged. Gate failure is NOT waived.

Read-only crt-provenance.cjs compares these public facts with installed files:
/usr/lib/x86_64-linux-gnu/wine/i386-windows/msvcr71.dll SHA
ba16ba7ccf14544b6cbce9f143153c81f00c4ed92802bd761c2463ffc4880b28,
size2780966, matches ALL loaded header facts. Wine64 artifact remains distinct
SHA271f2b72.../machine8664/magic020B. Installed libwine:amd64
10.0~repack-12ubuntu1 supplies both architectures (libwine:i386 is not installed).
This is a Wine-builtin provenance candidate supported by exact header match,
NOT atomic mapped instruction/file identity. No proprietary bytes/disassembly
copied. Local DLL override environment was unset and selected registry override
section empty; do not claim complete historical environment reconstruction.

PROVENANCE CORRECTION: seven prior paired phases retain their raw/repeat/typed/
control observations and failed hypotheses IN THEIR CAPTURED ENVIRONMENT.
They did not record loaded Microsoft-CRT identity; it cannot be assumed or
retroactively established from backing-file hashes. They are NOT by themselves
proof of Microsoft-CRT/Windows formatter parity. Do not turn their thresholds/
rejected arithmetic fits into production expectations until an explicitly
native-bound discriminator recovers the relevant boundary. This does not prove
a shipped regression or authorize unrelated product changes/replaying every
completed slice; select a bounded decisive comparison after identity proof.

crt-binding-diagnostic-attempt1/ retains78 byte-compared generated receipts/
OWN source/caller/FXP/full build+native REA and offline provenance.json.
Host+archived diagnostic/provenance readers0/manifest0; exact binding remains2.
All source/tool/native/binary hashes checked0. SDK-containing FLL/object/
import/export libs stay HOST-ONLY in intact
build/default-transform-crt-binding-diagnostic-attempt1; original path absent.
Both chats RELEASED promptly/prefix idle1, no reservation/VM/owner blocker.
Build REA ev_f791b2be140ca80a1d1cc1e84350ebe983973a66b74b26dafe86954b3d1b1b30
root0; native REA ev_8969fe0cd5a361e1245602b4e82a816bb63b97ccac5441edc7e1bb6155342b17
root2. BOTH quiesced/owned verified/untruncated/no sampled ownership unknowns;
all original process-sampling/atomic-binding/snapshot/environment/network limits
remain. No old failed capture is retroqualified.

### Native-only load-order discriminator — source freeze, NOT run

The installed wine-stable.1 manpage and
[Wine10 primary manpage](https://github.com/wine-mirror/wine/blob/wine-10.0/loader/wine.man.in)
document WINEDLLOVERRIDES and native-only n (no builtin fallback). The runner
source copies its inherited environment, then sets WINEPREFIX/WINEDEBUG, so
the selected override reaches Wine. No global helper edit is needed.

Change ONLY the diagnostic worker's native invocation to process-local
env WINEDLLOVERRIDES=msvcr71=n; record the supplied value and inherited override,
fail if a nonempty override already exists rather than silently replacing it.
No registry/installed DLL/loader/search-path/system/VM maintenance, DLL copying,
global configuration or load from inside the observer. This is a declared
test-input change, not accepting a Wine module as Microsoft. Same ONE CFNBIND
caller/control/observer/strict digest+header+path predicates, unchanged C/hash/
reader and no full4176/repeat. Native-only loading may fail; preserve actual
failure. Even BOUND success covers one sampled module diagnostic under Wine,
not internal VFP conversion attribution, formatter parity or Windows validation.

Build/capture requires clean signed NEW source at a fresh root after prior
attempt preservation. Fresh separate BOTH-chat compiler/native CLEAR and all
immediate guards unchanged. Bash-n/Node syntax/diff-check0. Override not run,
new worker unbuilt; old FLL is evidence only, not a bypass of source manifests.
Current full45.86/44.60 fails; no further window requested/reservation.
LOW DQ-DV001 developer walkthrough checked first failure/close/digest fields,
offline header equality, preserved exact failed gate, uncertainty correction,
process-only env scope/unset guard/recorded override/no fallback, and rollback
by exact worker revert while retaining receipts. Readers/readbacks above
actually0; no independent-human/system/release acceptance or hazard closure.

### Native-only ONE-CALL exact binding0 — observed; capture cleanup caveat

Unchanged signed/DCO cleanc1d63a83c after NEW explicit BOTH-chat compiler-only
CLEAR/guards: import/Clang32/link all0, compile stderr empty/four SDK debug
warnings retained. HOST-ONLY FLL SHA
396374787866ab66dcb86b9054c70697a07772e813b8449029bc71d35affe9cd.
Observer/header/caller/inventory/strict reader SOURCE BYTES match prior failed
diagnostic. Rebuild artifacts have different hashes; binary equality is not
claimed. Compiler window RELEASED, separate NEW BOTH native-only CLEAR/guard.
Inherited override recorded UNSET; supplied process-only msvcr71=n recorded.
No registry/global helper/DLL/VM change or builtin fallback was made.

Actual VFP0/empty stderr/DONE1, structural/control0 AND exact --require-bound0:
BOUND/error0. Loaded/file machine014C/magic010B/timestamp3E561EAC/imageSize00056000/
checksum0005F1E9 match frozen Microsoft32 artifact;348160 bytes/85 reads/EOF/
close0/file SHA8094af5e..., self-test/memory predicates/CW027F/Numeric metadata/
BRS/display controls pass. Raw1053 bytes SHA
fe7146266291fe113dedafae02946464e8dc63d7ffab78ace921e98a5967a108.
This is ONE sampled module-binding/control result under Wine, no CRT conversion,
formatter rule, Windows execution or paired qualification. Native-only loading
corrects this observed identity boundary; old failures remain failed and prior
captured environment/provenance limitations are NOT erased.

Build REA ev_fa42d0af7c3e00e07020d6c04ef3006a88f44d61d28c2366729fd33f52132bc3
root0/quiesced/owned verified/untruncated/no sampled ownership unknowns.
Native REA returned top-level cleanup_incomplete, NO Evidence ID. Full raw tool
response, parsed error and partial capture retained, NOT discarded/replayed.
Partial capture root0/quiesced/untruncated BUT cleanup_outcome:failed due EACCES
ownership token on live candidate PID1245895. Read-only ps identified unrelated
Edge renderer/parent938012/UID1000; left untouched/no signals. Later ps found
PID absent and Wine-prefix scan exit1. That follow-up is not retroactive proof
of REA ownership/cleanup; do NOT claim owned-group verified for THIS native run.
Original sampling/atomic-binding/snapshot/inherited-environment/network limits
remain. Initial orchestration projection assumed normal result shape and threw
after the full error was saved; corrected extraction preserved the same partial
record, no native rerun. Actual worker/readers were already0 independently.

crt-binding-nativeonly-capture1/ retains81 byte-compared plain receipts/OWN
source/FXP/full error+partial capture/cleanup-followup and build REA. Host+archive
strict --require-bound reader0/manifest0, all source/tool/native/binary hash
checks0. SDK-containing FLL/obj/import/export libs HOST-ONLY in intact
build/default-transform-crt-binding-nativeonly-capture1; original diagnostic
build path absent. Both chats RELEASED/prefix idle1/no reservation or VM.

NEXT bounded recovery (NOT source-frozen/implemented yet): adapt the full CRT
experiment to this explicit native-only load-order input and retain the exact
binding predicate on EVERY process. Same264 original/historical shifted payloads
are EXPLICIT STIMULI, not accepted thresholds, current-runtime arithmetic
results or expected formatter strings. Change emitter/caller to stop and retain
the FIRST observer/runtime error, no thousands of repeated failures. Worker
must record supplied/inherited environment and reject nonempty overrides, hash
all changed sources, and first strict gate BEFORE repeat. Freeze/sign source
before fresh separate BOTH-chat compiler/native windows and immediate guards.
Do not simply invoke the old full4176 worker (no native-only env/first-error
stop), replay seven prior pairs, reuse attempted paths or relax identity.
Recover the exact conversion/display boundary under the corrected input before
native-only expectations/actual ORIGINAL/support/direct/wiring/SAME/hosted.
No production/model/tolerance/cap/call-attribution or compatibility closure.

LOW DQ-DV001 developer walkthrough: inspected independent actual0/strict0/raw
and hash receipts, unchanged source bytes, validity-tagged native identity,
one-call scope and full unmodified cleanup failure; verified archive readback/
manifest/reader0. The compact handoff redirects historical chronology here,
preserving uncancelled division ORIGINAL/tests and all failures/unknowns.
No independent-human/system/release assurance. No owner-input blocker or pause.

### Native-bound full CRT recovery — source freeze, NOT built/run

emit-crt.cjs/crt.prg/crt-cases.json now explicitly classify the264 original/
historical shifted payloads as stimuli ONLY. Old arithmetic-capture bits remain
immutable; not interpreted as new-runtime arithmetic, threshold expectations or
accepted formatter strings. Enumeration remains4176 rows/run: both FIXED modes,
F at applied D1..18 (D0 omitted), E15/16/17, both payload routes. Same independent
TRANSFORM/STR/settings/metadata/BRS/pre+post controls and bounded observer C/hash
source. First observer ERROR: return or caught runtime error is written ONCE,
then caller unloads libraries/returns with NO completion footer. Never continue
thousands of failed calls. Normal20-field row layout/coverage stays unchanged.

crt-capture.sh supplies process-local native-only msvcr71=n on EACH invocation,
records supplied/inherited override, rejects nonempty inherited overrides.
audit-crt.cjs requires those exact records plus the explicit-stimulus declaration,
all existing exact32-bit digest/path/loaded-file header/CW/argument controls,
actual0/empty/full coverage, strict first gate BEFORE repeat/typed repeat.
No old reader/archive is edited or retroqualified. No expected CRT digits or
production formatter rule is synthesized.

Source Node syntax/emitter PRG+inventory regeneration/cmp/bash-n/diff-check
all0 under set-e/pipefail. These are source checks, NOT compiler/native proof.
crt-build.sh still performs guarded GNU+Clang exact shared hash/header direct
checks before cross-building the source-frozen wrapper. No changes to observer,
hash algorithm, compatibility-switch/checked-conversion/allocation policy,
registry/global helper/DLL/VM configuration or production code.

NEXT clean signed source at fresh build/default-transform-crt; request NEW BOTH
compiler-only CLEAR, all immediate avg10 AND avg60<1%/idle-prefix gates per
host test/import/compiler/link. Build0/release -> separate NEW BOTH native-only
window, each invocation gated. Preserve every guard/failure/full tool error and
actual status; never overwrite prior paths. For full paired native receipts,
declare snapshot budget160 files/40MB (not extra test scope) so extended protocol/
raw/stdout/audit/source/build receipts can be sampled without hiding omissions.
Keep SDK-linked binaries/objects/libs HOST-ONLY; full REA and any limitations
remain evidence alongside actual statuses/readers, not replacements for them.
No full experiment has run this native-only/first-error source yet.

LOW DQ-DV001 procedural delta/developer walkthrough: traced observer and runtime
failure branches through one retained error/no DONE, unchanged normal row count,
both environment guards and exact module reader, old observation/stimulus versus
expectation boundary, fresh separate windows/strict-first-before-repeat. Source
checks above actually0; rollback exact source revert while retaining all raw
receipts/provenance correction, notify owner if evidence is invalidated. No
independent-human/system/release assurance, known-bug exception or closure.

### Full CRT build capture lifecycle failure — no worker evidence

At signed/DCO source8ce84ed596e1b65c51084188515a31fafe7a2fa0, fresh explicit
BOTH compiler-only CLEAR was obtained. The declared crt-build.sh REA call
returned ONLY cleanup_incomplete (retryable:false), no Evidence ID, root exit,
partial capture or worker receipts. No build.started.utc/build.finished.utc or
compiler/test/import/link status existed. Therefore no completed build or test
is established; absence of receipts is NOT complete process observation.

Full unmodified tool response is retained. Ownership-token reads failed with
EACCES for candidates1303366/1305485/1305502. Read-only ps showed msedge,
parent938012/UID1000; left untouched, NO signals sent. This is not verified
REA cleanup or evidence that the target completed. Both windows RELEASED
immediately. Follow-up full I/O PSI41.72/17.74 exceeded the gate, prefix idle1;
no native reservation or VM operation. No repeated producer or diagnostic run.

crt-full-rea-lifecycle-attempt1/ archives13 byte-compared plain receipts/OWN
source copies; SHA256SUMS verified0. Host attempt retained intact at
build/default-transform-crt-rea-lifecycle-attempt1; original worker path absent.
No SDK-containing artifacts archived and no new FLL evidenced.

NEXT use the existing frozen worker through an ordinary shell capture after
fresh BOTH compiler-only CLEAR and all unchanged immediate per-invocation
pressure/prefix gates. Do not retry identical failed REA lifecycle while its
blocker persists, modify REA configuration, or signal unrelated processes.
Ordinary shell receipts must be labelled as such, NOT REA Evidence/verified
descendant cleanup. Only actual build0 allows a separate fresh BOTH native-only
window. Preserve every guard/failure, strict first-before-repeat and exact
module binding. Full CRT experiment remains UNBUILT/UNRUN; no accepted rule.

LOW DQ-DV001 developer walkthrough: compared all13 archived bytes to original
receipts/source and verified manifest0; inspected error-only/absent execution
evidence versus prior one-call partial capture distinction. This procedural
delta changes capture fallback only, not probe/guards/identity/acceptance.
Rollback retains all raw errors and restores previous capture route only after
its blocker resolves; notify owner if evidence invalidated. No independent or
release assurance, production change, owner-input blocker or automation pause.

### Native-only full CRT pair — captured and qualified, no accepted rule

At clean signed/DCO f1be513a3a041f35aca8bb4ba8374a5b8fa4c718 (unchanged
experiment source8ce84ed59), NEW explicit BOTH compiler-only CLEAR and all
immediate full avg10/avg60<1%/idle-prefix gates: GNU+Clang ASan/UBSan72 direct
shared SHA/header checks EACH0, import/Clang32 compile/link0. Compiler stderr
empty; four installed SDK .debug$S linker warnings retained. HOST-ONLY FLL
0c683d16a8bb7637398ad91d9451d53cdb4bdb13d9e3e276274e3738a3ecb686.
Ordinary shell worker/session completion0, NOT REA Evidence/descendant-cleanup
attestation. Previous REA error-only attempt remains intact, not retroqualified.

Compiler window RELEASED, separate NEW BOTH native-only CLEAR. EACH invocation
guard0/full PSI0.00/0.00/prefix idle1. Inherited override UNSET/supplied
WINEDLLOVERRIDES=msvcr71=n recorded; no global registry/helper/DLL/VM change.
First actual0/empty stderr/DONE4176/strict first0 BEFORE repeat. Repeat actual0/
empty stderr/DONE4176, paired reader0. All20 fields RAW BYTE IDENTICAL:
SHA60a1b217bbe305ccb61012cb4324d7bb944d9e413bd920803debd17e36e8c54d
for BOTH raw reports. Same264 explicit original/historical shifted stimuli,
both FIXED modes, F at applied D1..18 and E15/16/17. Metadata/BRS/argument/
settings/control-word027F/control displays exact, module path/digest/PE32
identity predicates exact on each observation; sampled file once per FLL,
later loaded-image/path rechecks. This is NOT atomic mapped-code proof,
Windows execution or VFP internal-call attribution. No proprietary decompilation.

crt-nativeonly-captures1/ retains157 byte-compared receipts/OWN sources/FXP/
generated hash inputs/full plain-shell results/strict audit/offline comparison.
SHA256SUMS and archived strict+analysis readers0 under set-e/pipefail. Source/
tool/native/binary hash checks0 before relocation. SDK-containing FLL/obj/libs
and host test executables stay HOST-ONLY in intact
build/default-transform-crt-nativeonly-captures1; original worker path absent.
Both chats immediately RELEASED/prefix idle1; no standing job/reservation/VM.

Offline analyze-crt.cjs checks raw report digests/qualified-reader flags and
produces comparison.json with complete counterexamples, NOT expected strings
or production logic. All528 original-route payload/settings display contexts
match the earlier arithmetic capture exactly. ONLY this finite overlap was
compared; no retroqualification of earlier phases/Windows formatter provenance.

Public conversion-output tuple sufficiency check groups by fixed/D/width40/
scale18/original sign/kind/count/returned decimal/sign/digits, deliberately NOT
input bits. Distinct original display strings share the same tuple:

| Route | F at D: conflicting/keys (rows) | E15 | E16 | E17 |
| --- | --- | --- | --- | --- |
| ORIGINAL | 50/84 (504) | 52/88 (528) | 18/444 (528) | 0/528 (528) |
| historical SHIFTED | 42/156 (504) | 52/88 (528) | 12/436 (528) | 0/524 (528) |

Example ORIGINAL indices13 and16, FIXED ON/D1/positive: same F1 empty digits,
decimal FFFFFFFF/sign0, but VFP original display0.0 versus0.1. Therefore those
public tuples alone do NOT determine original display in the captured sample.
No inference that VFP calls that API at that count. Zero E17 conflicts does NOT
establish a formatter rule: original keys are unique, so the test provides no
nontrivial equivalence for them. SHIFTED bits remain explicit old magnitude
stimuli, not newly observed native arithmetic or an accepted bias/operation.

NEXT bounded native-only recovery: source-freeze an ORIGINAL-only public
_fcvt count discriminator at applied D+1,D+2,D+3 (D0..18, counts1..21), same
264 payloads/width40scale18/both FIXED modes,1584 rows/run. Explicitly extend
only the experimental wrapper count bound before testing; no production or
implicit18 cap. Keep finite/count validation, immediate bounded copy of CRT
static output, exact identity/controls, first-error stop, fresh separate BOTH
windows/all guards and strict-first-before-repeat. These counts are diagnostic
inputs, NOT a presumed VFP extra-digit algorithm. Source NOT yet written/frozen.
Primary documented API contract (not legacy-runtime behavior proof):
https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fcvt?view=msvc-170
describes decimal-place count, decimal/sign results, bounded static output
overwritten by subsequent conversions; keep count0 excluded. Exact behavior
must come from this installed/bound native CRT, not modern documentation alone.

LOW DQ-DV001 developer walkthrough: inspected actual stage/direct-check logs,
both strict/raw/hash proofs, archive byte comparisons/manifest/readers and
complete tuple counterexamples. Delta is qualified plain-shell evidence plus
an offline insufficiency test; NOT a fitted formatter or expected regression.
Existing hazards/rollback retained. No independent-human/release assurance,
parent closure, owner-input blocker or automation pause. Next remains exact
conversion/display recovery -> expectation freeze/actual ORIGINAL -> unused
support/direct -> wiring/SAME/broader/hosted/exact-head review/merge.

### ORIGINAL-only higher-count discriminator — source freeze, UNBUILT/UNRUN

emit-crt-precision.cjs validates both parent native raw hashes60a1b217bbe305cc...
and selects exactly264 ORIGINAL/ON/E17 records, checking identical index/D/bits
against prior immutable input inventory. Emits crt-precision.prg/inventory with
no SHIFTED route, both FIXED modes, F at D+1/+2/+3 for D0..18/counts1..21:
1584 rows/run. Same20-field controls/independent metadata40/18/BRS/TRANSFORM/
STR/applied settings/pre+post controls and first-error/no-footer stop. No
expected text, decimal rounding, threshold/bias or format rule is generated.

crt-probe.c now has explicit compile-time experimental CRT_FIXED_COUNT_MAX,
static-asserted18 or21, default18 (prior experiment domain unchanged). ONLY
new crt-precision-build.sh opts into21. Finite/positive/integral count checks,
E15/16/17 restriction, argument/header/digest/path binding, immediate127-digit
static-buffer copy/NULL/character/bounds/CW checks remain. No internal calls,
global override/configuration/VM or production/model changes. Old exact sources
and captures remain archived unchanged; do not rerun them to requalify this delta.

New dedicated build/default-transform-crt-precision worker path; compiler worker
retains guarded GNU+Clang72 direct hash/header tests each, import/compile/link
and source/tool/SDK/CRT hashes. Capture worker records inherited/supplied native
override, rejects inherited nonempty overrides, all source/native/binary hashes,
fresh guard EACH invocation, actual first0 then strict --single0 BEFORE repeat,
paired strict0. audit-crt-precision.cjs requires full1584 unique keys/exact
counts1..21/ORIGINAL-only inputs, all prior identity/typed/raw/control predicates.
No old reader or archive acceptance is weakened. Ordinary shell capture is NOT
REA Evidence/verified descendant cleanup; retain earlier tool errors separately.

Node syntax/emitter source+inventory regeneration/cmp/bash-n/diff-check0 under
set-e/pipefail. These are source checks ONLY, NOT compiler or native execution.
NEXT clean signed source -> NEW BOTH compiler-only CLEAR/all per-invocation
full avg10 AND avg60<1%/idle-prefix gates -> actual build0/release -> separate
NEW BOTH native-only CLEAR, same strict-first-before-repeat. Preserve every
guard/failure/actual status/full ordinary-shell result. No attempted path reuse,
SDK binary distribution, old completed capture replay or guess of production
precision. Exact conversion/display recovery and entire implementation ladder
remain pending; no owner-input blocker/pause.

LOW DQ-DV001 source walkthrough: traced264*2*3 count coverage/max21/default18,
unchanged C binding/buffer/parameter gates, new reader source/actual-env/first
gate and separate worker paths. Source checks0 above; no independent-human,
compiler/native or release assurance. Misuse risk is treating diagnostic counts
as accepted VFP extra digits; mitigated by explicit no-model inventory/controls
and required native observation. Existing HZ-data-corruption-01/HZ-runtime-crash-01/
HZ-doc-command-01 retained. Rollback exact new source delta while preserving all
raw receipts/provenance correction; notify owner if evidence invalidated.

### ORIGINAL-only precision build0 / pre-native guard75 — UNRUN

At clean signed/DCO f4fdf81a9571bc86071b9e621b98029fbbf050a6, NEW BOTH
compiler-only CLEAR and all immediate guards: GNU+Clang ASan/UBSan72 direct
shared SHA/header checks EACH0, import/Clang32 opt-in21/link0. Compiler stderr
empty/four installed SDK .debug$S warnings retained. Ordinary-shell worker and
completion0, NOT REA Evidence/verified descendant cleanup. HOST-ONLY FLL SHA
a4d38aaecf67837c06b0d640746bc44b56b126d697cd0519c1b25c1b7934b090.

Released compiler window, separate NEW BOTH native-only CLEAR. First immediate
guard failed75 at full PSI19.91/4.54 (avg10/avg60), prefix idle1, BEFORE invoking
VFP. Plain-shell root75, no first native status/out/stderr/results/strict/repeat.
Both windows immediately RELEASED; later prefix idle1/full PSI42.07/14.59.
This is resource guard evidence ONLY, not native observation or accepted pair.
No VM/global config/production change, old capture replay or same-window retry.

crt-precision-guard1/ retains130 byte-compared files:129 successful-build/
guard/OWN-source/generated-input/full-shell receipts plus build-reuse receipt.
Manifest0 and host+archived audit-crt-precision-guard.cjs forensic reader0.
Reader verifies actual build0/seven stages/all144 direct checks/guards and
absence of native receipts, explicitly pairedQualified:false. Input/tool/native/
binary hash checks0 before relocation. SDK-containing FLL/obj/libs and host
test executables stay HOST-ONLY in intact
build/default-transform-crt-precision-guard1; no proprietary binaries archived.

Successful build is NOT repeated just to retry a pre-invocation resource gate.
An explicit106-file allowlist (seven successful stage receipts, both direct-check
logs/generated hash inputs, build/source/tool/hash/identity receipts, local
compiled outputs) was byte-copied into new EMPTY build/default-transform-crt-precision.
Source guard attempt untouched; NO native/guard attempt receipts copied.
restored-build.json records exact source/destination/source commit/all106 sizes+
SHA256 and purpose, byte comparison0/3,064,649bytes; input+binary manifests check0
at restored paths. This mechanical host-only copy executes NO compiler, test or
native program, and is not new build qualification. Restored host executables
need not be executable: only the proven FLL is needed for next native capture.

NEXT after unchanged source/hashes/clean signed state, request NEW BOTH
native-only CLEAR and invoke crt-precision-capture.sh against the restored
successful build. Every invocation still full avg10 AND avg60<1%/idle prefix;
first actual0/strict0 BEFORE repeat. Preserve new guards/failures separately,
never overwrite original guard attempt or rerun completed full4176/one-call
captures. If source changes invalidate a build input, explicitly rebuild after
fresh compiler clearance instead of reusing stale artifacts. No source/module
acceptance or successful native run exists for counts19..21 yet.

LOW DQ-DV001 developer walkthrough: examined all144 actual direct-test statuses,
compiler/link logs, exact first guard before native call, host/archive forensic
reader0/manifest/byte comparisons, preserved attempt and106 allowlisted build
copy hashes/no-native-receipt condition. Delta reuses completed unchanged build
only; no weakened pressure/identity/strict gates or production precision policy.
Existing hazards/rollback retained; no independent-human/release assurance,
owner-input blocker or pause. Full expected/ORIGINAL/implementation ladder open.

### ORIGINAL-only higher-count native pair — qualified, tuple rule rejected

At clean signed/DCO f69a8bc7090e59c37f13be402d15448a7510389b, unchanged
sourcef4fdf81a9/restored successful build a4d38aae... verified: all106 file
sizes+SHA256/source+binary manifests0, no native receipt in new root. No compiler
or completed direct test repeated. NEW explicit BOTH native-only CLEAR, EACH
immediate guard0/full PSI0.00/0.00/prefix idle1. Inherited override UNSET,
supplied process-local WINEDLLOVERRIDES=msvcr71=n, no global configuration/VM.

First actual0/empty stderr/DONE1584/strict --single0 BEFORE repeat; repeat
actual0/empty stderr/DONE1584/paired0. All20 raw fields IDENTICAL/inactive-slot
differences0; both raw SHA
ba189979b2d7cd67ab15f1d719af586f815f28f98427d30344629760f1fbabd3.
All264 ORIGINAL payloads/both FIXED modes/D+1,+2,+3/counts1..21 present once,
metadata40/18/BRS/settings/CW027F/display controls exact, exact Microsoft32
CRT path/digest/PE32 binding predicates pass. File sampled once perFLL with
later image/path checks; NOT atomic image proof/Windows execution/internal
call attribution. Ordinary shell receipts NOT REA Evidence/descendant cleanup.
Both chats immediately RELEASED/prefix idle1/no reservation or owner blocker.

crt-precision-captures1/161 byte-compared receipts/OWN sources/FXP/generated
hash inputs/full shell progress+completion/restored-build provenance/raw/strict
audit/offline comparison. Manifest+archived strict reader0, offline analysis
byte-exact recomputation0, source/tool/native/binary hashes0 before relocation.
SDK-linked FLL/obj/libs/host binaries HOST-ONLY in intact
build/default-transform-crt-precision-captures1; original working path absent.
Original guard1/130 receipts and source host attempt remain unchanged. No
retroqualification or replacement of prior guard/tool failures.

analyze-crt-precision.cjs is offline only, added AFTER native capture. It checks
both raw digests/qualified-reader flags and bound-parent raw hashes, verifies
all528 input/settings contexts and three-offset control consistency. Original
TRANSFORM AND STR outputs match previous bound4176 pair in all528 contexts.
This finite overlap is not global/earlier-phase/Windows retroqualification.

Sufficiency key is fixed/D/width40/scale18 and CRT count/decimal/sign/digits,
deliberately not input bits. Complete conflict tuples/counterexamples retained:

| Count offset | FIXED OFF conflicts/keys (264 contexts) | FIXED ON conflicts/keys (264 contexts) |
| --- | --- | --- |
| D+1 | 8/44 | 44/44 |
| D+2 | 8/44 | 44/44 |
| D+3 | 14/88 | 30/88 |
| combined D+1,+2,+3 | 14/88 | 30/88 |

Example D0/ON indices3 and4: SAME tuples F1 digits5/F2 digits50/F3 digits499,
decimal0/sign0, but original VFP displays0 versus1. More precision at these
three diagnostic counts cannot by itself determine the original display in
this sample. This does NOT disprove other CRT roles, identify internal calls,
establish precision/tolerance/cap/operation order or accept a formatter model.

NEXT bounded recovery: compare public VFP ROUND(payload,D), D0..18 on SAME264
ORIGINAL payloads/metadata40/18/both FIXED modes, recording typed result
metadata/BRS and independent original TRANSFORM/STR/settings/pre+post controls,
under process-local bound native CRT identity. No need for new observer C or
compiler: reuse already proven host-only observer with exact source/binary
hashes and explicit new caller/reader/worker freeze; per-observation existing
CFNFCVT(payload,17) control can retain module/CW binding without count0.
Public ROUND count0 is not CRT count0. Stop first native/error/identity failure,
strict first BEFORE repeat, fresh BOTH native-only CLEAR/all immediate gates.
This is an independent public-function discriminator, NOT an assumption that
TRANSFORM calls ROUND or authorization to change production ROUND separately.
Source NOT written/frozen yet; do not rerun completed CRT precision samples.

Published Microsoft VFP9 SP2 Help distinguishes public numeric rounding from
display settings and permits numeric precision arguments. Mirrored primary
help topic: https://vfphelp.com/vfp9/html/099a505d-54f5-4d5a-aa35-f100494d142f.htm
and STR cross-reference:
https://www.vfphelp.com/vfp9/html/970442f9-ac4f-401f-a027-03aa9f978054.htm
These contracts motivate the comparison, NOT expected bits/strings or legacy
implementation details; only installed native observations can establish them.
No external page/comment instructions are work authority.

LOW DQ-DV001 developer walkthrough: verified unchanged build reuse, actual0/
strict-first/repeat/raw equality/identity controls, complete conflict groups,
all161 archive byte checks/manifest/strict reader/offline reproducibility0.
Delta adds qualified public-conversion evidence and rejected tuple sufficiency,
NOT production precision. Existing hazards/rollback/field notification retained;
no independent-human/release/parent closure. Exact recovery -> expectations/
ORIGINAL -> unused support/direct -> wiring/SAME/broader/hosted/review remains.

### Public ROUND typed-result discriminator — source freeze, UNRUN

emit-round-result.cjs checks BOTH raw parent hashesba189979b2d7cd67... and
selects identical264 ORIGINAL input/D/index tuples. round-result.prg/inventory
record ROUND(argument,D), argument explicitly Numeric40/18, both FIXED modes,
D0..18/528 rows/run. Independent original metadata/BRS/TRANSFORM/STR/settings
are bracketed by CFNFCVT(argument,17) controls with exact native binding/CW.
Result metadata/BRS/TRANSFORM/STR are sampled before+after the control call.
First binding/runtime error emits ONE failure/no DONE and returns. No expected
numeric bits, display text, tolerance, cap or assumed internal call generated.

audit-round-result.cjs requires26 fields/full528 unique keys, all input
descriptors/BRS/settings/parent original display+STR exact, before+after CRT
identity/argument/CW/output equal, finite typed result/BRS/metadata/display/STR
stable and typed repeat. Result values are observations, not assumed ROUND
arithmetic. Only documented inactive slots are normalized: Numeric.integer,
Integer.real/decimals. Public VARTYPE must be N for BOTH carriers; raw published
Value.type selects meaningful fields. Primary Microsoft VFP9 SP2 Help mirror:
https://vfphelp.com/vfp9/html/b05ef476-980c-492f-81f5-1be0eb7ef8d9.htm
Pre-freeze walkthrough corrected the initial draft mapping that conflated
VARTYPE with the raw carrier tag; no failed native capture/reader was rewritten.

round-result-capture.sh uses new build/default-transform-round-result, refuses
an attempted/dirty root, records inherited/supplied native-only environment,
checks reused FLL SHAa4d38aaecf67837c06b0d640746bc44b56b126d697cd0519c1b25c1b7934b090
from host build/default-transform-crt-precision-captures1, compares actual
observer C/header bytes to archived proven sources and hashes ALL current
caller/reader/worker/observer/native inputs. No compiler or new observer needed.
Each native invocation still requires fresh BOTH CLEAR/full avg10 AND avg60<1%/
idle prefix, actual first0/strict0 BEFORE repeat and complete paired strict0.
Ordinary shell NOT REA Evidence/descendant-cleanup proof; no prior capture replay,
global DLL/registry/helper changes or VM. SDK-containing binaries HOST-ONLY.

Node syntax/emitter PRG+inventory regeneration/cmp/bash-n/diff-check0 under
set-e/pipefail. Source checks ONLY, NOT native execution or ROUND/display parity.
NEXT sign/freeze/clean source, verify unchanged observer hash/source, NEW BOTH
native-only CLEAR and immediate guards, invoke new worker. Preserve every
guard/error/full actual status, never overwrite old attempts or weaken first
gate. No new production ROUND work or assumed TRANSFORM implementation.

LOW DQ-DV001 developer walkthrough: traced264*2 keys,26 field indices, independent
parent controls/CRT17 before+after, Numeric-vs-Integer inactive-slot treatment,
first-error/no-footer and strict-first-before-repeat flow. Source checks0 above,
not compiler/native/independent-human/release assurance. Delta is an independent
public function comparison; misuse as an accepted formatting rule would risk
silent data corruption, mitigated by explicit observation-only inventory and
remaining exact-recovery/ORIGINAL/implementation ladder. Existing hazards/
rollback/field notification retained; no owner-input blocker/pause.

### ROUND pre-launch pressure guard — no native execution

At clean signed/DCO ec4e26cff57cee88254a4f75b1365a6287f6615c, a NEW
explicit BOTH-chat native-only CLEAR was obtained. Worker immediately checked
the unchanged observer SHA/proven C/header, source cleanliness/input hashes,
native-only environment and idle prefix1, then stopped75 before first VFP:
full PSI avg10=11.94/avg60=3.26. No first native status/stdout/stderr/result,
strict acceptance/repeat/FXP/footer exists. This is a distinct actual worker
guard, not the prior audit HOLD before worker start. Both windows RELEASED.

round-result-guard1/26 byte-compared compressed plain receipts/OWN sources,
manifest0 and host+archived audit-round-guard.cjs forensic0. Source/tool/native/
observer inputs sha256sum0 before relocation. Intact original host attempt at
build/default-transform-round-result-guard1; original working root ABSENT.
No new compiler/native execution or SDK binary redistribution. Full ordinary
shell result retained, NOT REA Evidence or descendant-cleanup attestation.
Guard checker added AFTER attempt and explicitly reports pairedQualified:false.

NEXT: unchanged ROUND caller/reader/worker and existing observer need no rebuild.
New clean source checkpoint plus NEW BOTH native-only CLEAR/all immediate gates
permits a fresh root, never overwrite/replay the guarded attempt. Original
source freeze and both prior failures remain. No owner-input blocker/pause.

LOW DQ-DV001 delta walkthrough: checked actual pre-launch control flow against
all26 archived source/receipt bytes, missing-native predicates and manifest;
forensic reader0 is not semantic/native qualification. Misuse would turn a
resource stop into false compatibility evidence; explicit no-invocation fields
and unchanged native/strict-first gates mitigate that risk. Existing hazards,
rollback/notification and development self-review limits apply.

### ROUND first-only argument-width rejection and inline correction

NEW BOTH native-only CLEAR at clean signed/DCO35bd9b24bbecfb4f9503170c5909375463393425.
Immediate full PSI0.00/0.02/idle prefix1, reused observer/source/native input
hashes0. First native actual0/empty stderr/DONE528, then FROZEN strict reader1
at required width40 (0028), correctly preventing repeat. Raw SHA
e86d9b43a656896a2dd9ed3965b5d4fe77273aeca765dfe89d3977511e492468.
All argument pre/post widths already29 or30 (264 each); descriptor width does
not change across ROUND, so this is not evidence of ROUND mutating input.
Caller stored CFNDEC(CFNWIDTH(payload,40),18) into local argument before
observation, unlike the prior inline width40 captures. This route did not
preserve the required input descriptor. No semantic ROUND result accepted.

round-result-attempt1/39 byte-compared plain/source/OWN FXP receipts retain
actual first/strict diagnostic/full ordinary-shell results. Manifest0 and
host+archived audit-round-attempt.cjs forensic0 explicitly argumentControlQualified:
false/pairedQualified:false. Original reader/source archived unchanged; this
forensic check added AFTER failure is NOT strict acceptance. Intact host at
build/default-transform-round-result-attempt1, original working path absent.
Both chats RELEASED/prefix idle1. No compiler/SDK binary copies/VM/REA Evidence/
descendant-cleanup attestation; previous guard1 remains unchanged.

Correction: emit the already-used inline CFNDEC(CFNWIDTH(payload,40),18) at
EVERY original metadata/BRS/display/STR/CRT17 consumer and public ROUND input,
without local argument assignment. Same264 input/D keys,26fields/528rows,
original parent controls, before/after CRT identity and width40 strict check
are unchanged. Reader additionally verifies the explicit input-route source
contract. The result remains deliberately assigned to local rounded, so result
metadata/display evidence covers that STORED result only, not inline result
metadata. Inventory distinguishes both routes. No predicted bits/strings or
relaxed predicates, changed observer/compiler, production ROUND/formatter fix.

Node syntax/regeneration+cmp/bash-n/diff-check0; next signed clean source freeze,
NEW BOTH native-only CLEAR/all immediate gates and fresh root. Earlier failure
stays unqualified, no retroactive repeat. Strict first BEFORE repeat unchanged.

LOW DQ-DV001 delta walkthrough: traced the failing field4 assertion and both
argument descriptor samples against528 rows, compared prior inline caller and
own public-ABI clone setter source, checked each corrected consumer route,
stored-result boundary, raw archive39 byte checks/manifest/forensic0 and source
checks above. Developer self-review, not independent-human/release assurance.
Misuse could mistake assignment-normalized input for width40 evidence; unchanged
strict metadata gate and preserved rejection mitigate silent display/data risk.
Existing hazards/rollback/notification and exact-recovery ladder remain open.

### Corrected inline-input/stored-ROUND-result pair — qualified528

At clean signed/DCO9e9bf0c15cf07374a987d49d5a18115f18ee4502, NEW explicit
BOTH native-only CLEAR. No compiler/new observer. Each immediate guard0/prefix
idle1; full PSI first0.15/0.55, repeat0.10/0.51 (avg10/avg60, each<1%). Inherited
override UNSET/process-local native-onlymsvcr71=n, exact reused observer/source/
native input hashes0. First actual0/empty stderr/DONE528, strict --single0
BEFORE repeat; repeat actual0/empty/DONE528/paired0. All26 raw fields identical,
inactive differences0, both raw SHA
0be810176af4c28eca3864eae70dc8adbfe409986ca227cdde8ccaa7c8013607.

Same264 ORIGINAL inputs/D0..18/both FIXED; original INLINE width40/scale18,
BRS/settings/original TRANSFORM+STR/CRT17 before-after identity/argument/CW027F
controls exact. Stored ROUND result type/active descriptor/BRS/display/STR
stable across control call and repeat. File sampled once perFLL/later loaded
image/path checks, NOT atomic mapped-code proof/Windows execution/internal-call
attribution. Ordinary shell NOT REA Evidence/descendant-cleanup attestation.
Both chats RELEASED/prefix idle1/no reservation. Previous first-only rejection
and pre-launch guard remain intact and unqualified.

round-result-captures1/55 byte-compared compressed plain/source/OWN FXP/full
shell receipts/raw/strict/offline comparison; manifest+archived strict0,
host/archive audit byte-exact0, offline comparison recomputed byte-exact0,
input/tool/native/observer hashes0 before relocation. Intact host at
build/default-transform-round-result-captures1; original working path ABSENT.
Existing SDK-linked observer and metadata FLL remain HOST-ONLY, not copied.

analyze-round-result.cjs added AFTER capture, offline only: verifies both raw
digests/strict flags, bound-parent raw digests and all528 original display+STR
contexts unchanged. This overlap ONLY, no broader retroqualification.

| Stored-result comparison | FIXED OFF (264) | FIXED ON (264) |
| --- | --- | --- |
| Result payload differs from original | 264 | 264 |
| Result TRANSFORM differs from original | 252 | 180 |
| Result STR(...,40,D) differs from original | 132 | 132 |
| Conflicting stored-result tuples / distinct tuples | 8/44 | 44/44 |

Tuple includes FIXED/D/original40/18/result carrier/width/decimals/payload,
deliberately NOT original bits. Full tuples and all comparisons/counterexamples
retained. D0/ON indices3 and4 both stored Numeric width10/scale0/result+0, yet
original displays0 and1. Thus the stored public ROUND tuple alone cannot
determine original default display in this finite set. Assignment is part of
this route: this does NOT establish inline ROUND descriptor behavior, disprove
all internal rounding roles, identify calls, or accept a formatter rule.

NEXT bounded recovery: observe INLINE ROUND(CFNDEC(CFNWIDTH(payload,40),18),D)
descriptor/BRS/TRANSFORM/STR at each consumer, alongside SAME stored-result and
original controls. This isolates the still-unobserved result-assignment boundary
without weakening any original width40/parent/identity checks. Existing observer
can be reused, no compiler needed. Freeze a NEW bounded caller/inventory/reader/
worker before fresh BOTH native-only CLEAR/all immediate gates; do not replay
this completed pair or accept an inferred tolerance/cap/internal-call shortcut.
New inline-result sources NOT written/frozen yet.

LOW DQ-DV001 delta walkthrough: checked strict-first-before-repeat statuses,
all528 key/descriptor/BRS/settings/identity controls, raw equality,55 byte checks/
manifest/archived strict/offline reproducibility0, finite tuple collision keys
and explicit stored-result boundary. Misuse as a universal formatting rule would
silently lose distinctions; recorded counterexamples and next inline-result
boundary preserve the recovery gap. Developer self-review only, not independent
human/system/release acceptance; existing hazards/rollback/notification retained.

### INLINE ROUND result-assignment boundary — source freeze, UNRUN

emit-round-inline.cjs validates BOTH stored-parent raw digests0be810176af4c28e...
and exact264 input/D/index inventory, derives a NEW round-inline.prg and
round-inline-cases.json from the retained source (completed pair NOT replayed).
Same264 ORIGINAL/bothFIXED/D0..18/528rows,35fields (original26 + inline9).
All inputs remain INLINE width40/18. Public ROUND is evaluated inline for
VARTYPE/CFNMETA/BINTOC/TRANSFORM/STR separately, only resulting characters
assigned. Snapshot fields26..30 precede stored ROUND assignment; fresh inline
meta/BRS/display/STR fields31..34 follow the after-CRT control. These are separate
evaluations, NOT a claim of one retained numeric object. Stored result and all
original/settings/CRT identity controls remain independent.

audit-round-inline.cjs requires full528 unique keys/footer/actual0/empty stderr/
stdout==raw, unchanged original width40/18/BRS/settings/display+STR, exact CRT17
module/CW/argument before+after, and stored result descriptor/payload/display/
STR/CRT tuple identical to qualified parent. INLINE result type/active descriptor/
finite BRS/display/STR must be stable across control call and repeat, but are
NOT assumed equal to stored result or expected arithmetic. Only published
inactive Numeric.integer/Integer.real+decimals slots normalized; public
VARTYPE=N for both carriers. Per-field raw repeat differences retained/countable.

round-inline-capture.sh uses NEW build/default-transform-round-inline, refuses
dirty/attempted source/root, exact reused observer hash+archived C/header,
generation-input/parent/source/native/tool hashes and process-local native-only
msvcr71=n. Each native invocation still requires NEW BOTH CLEAR and immediate
full PSI avg10 AND avg60<1%/idle prefix; first strict --single BEFORE repeat.
First binding/runtime error stops without footer. Ordinary-shell fallback,
NOT REA Evidence/descendant-cleanup attestation; no compiler/VM/global config.

Node syntax, emitter PRG+inventory regeneration/cmp, emitted35-field separator
count, bash-n and diff-check0. Source checks ONLY; new worker/root/native not
started. Observer C/header/binary unchanged, HOST-ONLY reused FLL SHA
a4d38aaecf67837c06b0d640746bc44b56b126d697cd0519c1b25c1b7934b090.
NEXT sign/push clean freeze, NEW BOTH native-only CLEAR/all immediate guards.
No inferred internal TRANSFORM call, accepted formatter, tolerance/cap or
production ROUND change. Exact recovery/expectations/ORIGINAL ladder remains.

LOW DQ-DV001 walkthrough: traced26 retained+9 new fields, inline versus stored
numeric assignment routes, each input40/18 consumer, first-error/no-footer and
strict-first-before-repeat flow, unchanged identity/parent controls and active
slot normalization. Completed source checks above, developer self-review only,
not native/independent-human/release acceptance. Mistaking repeated expressions
for a retained object or a formatter algorithm could silently corrupt display;
explicit route contracts/independent parent/raw controls mitigate that misuse.
Existing hazards/rollback/notification apply; no owner-input blocker/pause.

### INLINE ROUND result pair — qualified528, assignment boundary observed

Clean signed/DCO41a425fb464ed11b5991bfa906ce87c1f22ecf6e, NEW explicit BOTH
native-only CLEAR; immediate each guard0/prefix idle1/full PSI0.00/0.01
(avg10/avg60). Existing observer/source/native/generation-parent hashes0,
inherited override UNSET/process-localmsvcr71=n. No compiler/new FLL/VM.
First actual0/empty stderr/DONE528/strict --single0 BEFORE repeat; repeat
actual0/empty/DONE528/paired0. All35 raw fields identical/inactive differences0,
both raw SHA
254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873.

All264/bothFIXED/D0..18 keys/original40/18/BRS/settings/parent original+stored
result/CRT17 before-after module/argument/CW027F controls exact. Inline result
active descriptor/BRS/display/STR stable across control call/repeat. File
sampled once perFLL/later loaded-image/path rechecks, NOT atomic mapped-code
proof/Windows execution/internal-call attribution. Ordinary shell NOT REA
Evidence/descendant-cleanup attestation. Both chats RELEASED/prefix idle1.

round-inline-captures1/57 byte-compared plain/source/OWN FXP/full shell/raw/
strict/offline comparison receipts; manifest+archived strict0/host-archive
audit byte-exact0/offline byte-exact recomputation0/input-tool-native-observer-
parent hashes0 before relocation. Intact host
build/default-transform-round-inline-captures1; working root ABSENT. SDK-linked
observer and metadata FLL remain HOST-ONLY, not redistributed. Prior captures/
failed first-only attempt/guard remain unchanged; no completed pair replay.

analyze-round-inline.cjs added AFTER capture/offline only checks both raw
digests/qualified flags/stored-parent raw hashes/all528 original+stored controls
unchanged. This finite overlap ONLY, not broad/Windows retroqualification.

For each FIXED mode264 contexts: inline-vs-stored payload/display/STR differences0,
active metadata differs ONLY width264. Inline width40 retains input width;
stored width follows previously observed normalized descriptor. This is a
bounded observed ROUND route result, NOT universal assignment behavior.
Inline-vs-original TRANSFORM mismatches252OFF/180ON and STR132each. Inline
descriptor/payload tuples (including FIXED/D/original40/18/result carrier/width/
decimals/payload, deliberately not input bits) still conflict8/44OFF (48contexts),
44/44ON (264contexts), same as stored. Full tuples/all comparisons retained.
D0/ON idx3vs4 now both INLINE Numeric width40/scale0/+0, yet original0vs1.
Result assignment does not explain these display distinctions. No accepted
formatter, tolerance/cap/operation order/internal-call or parity follows.

NEXT bounded recovery: isolate the remaining public-arithmetic expression
evaluation route, not replay the completed scalar graph. The earlier REJECTED
graph used numeric local assignments between powDown/bias/shifted/powUp/scaled/
raised/INT stages. Observe the SAME graph's stages as INLINE expressions with
no numeric intermediate assignment, on SAME264 inputs/D/bothFIXED, alongside
original40/18/settings/BRS/display/STR and exact before-after CRT17 controls.
Prior constants and MIN(D,15) remain REJECTED-hypothesis stimulus ONLY, never
requirements or proposed tolerance/cap. This is a route/precision discriminator,
not permission to implement the graph or call it TRANSFORM internals. Existing
observer can be reused/no compiler. Freeze NEW bounded source/inventory/reader/
worker, fresh BOTH native-only CLEAR/all immediate gates, strict first BEFORE
repeat. Sources NOT written yet. Do not repeat completed scalar/CRT/ROUND pairs.

LOW DQ-DV001 walkthrough: checked actual first/strict/repeat statuses,35 fields/
528 key controls, raw equality/identity,57 archive byte checks/manifest/strict/
offline reproducibility0, differing active metadata fields and complete tuple
collisions. Explicitly distinguished expression evaluations from object identity
and finite route behavior from a general requirement. Developer self-review
only, not independent-human/system/release acceptance. Misuse of rejected graph
constants as a formatter rule risks silent data corruption; observation-only
contracts/independent original controls/exact recovery ladder remain mitigation.
Existing hazards/rollback/notification retained, no owner-input blocker/pause.

### Inline rejected-graph arithmetic route — source freeze, UNRUN

emit-arithmetic-inline.cjs verifies BOTH bound inline-ROUND-parent raw SHA
254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873,
same264 input/index/D tuples against original scalar inventory, same8 stage
names. NEW arithmetic-inline.prg/inventory evaluate prior REJECTED graph's
powDown/bias/shifted/effectiveCount/powUp/scaled/raised/quantized as INLINE
expressions, not numeric locals. Only VARTYPE/CFNMETA/BINTOC character results
are stored. Numeric input/control loop indices remain ordinary variables;
stage arithmetic uses ABS(payload), matching prior graph, while independent
original formatting controls use inline width40/18. No guessed new constants.
Prior0.0005/0.5/MIN(D,15) remain rejected diagnostic stimulus, NEVER requirement,
proposed tolerance/cap/internal TRANSFORM algorithm or authorized production fix.

Same264/bothFIXED/D0..18/528rows/66fields:26 original+storedROUND+CRT controls
and8*5 stage fields (VARTYPE/meta/BRS BEFORE; meta/BRS AFTER). Each expression
evaluation independent; no retained-object claim. Appended records split across
source lines (maximum715 characters), delimiter count65, all8 expression
parentheses balanced/source has no named numeric stage assignment.

audit-arithmetic-inline.cjs requires full528 keys/footer/actual0/empty stderr/
stdout==raw, exact original40/18/BRS/settings/display+STR, stored-ROUND descriptor/
payload/text and CRT17 tuple against bound parent, exact before-after runtime
identity/argument/CW027F. Every stage finite typed result/meta/BRS stable before/
after/repeat, no predicted stage bits/accepted arithmetic. Public VARTYPE=N for
N/I carriers, raw Value tag selects active slots; only published inactive slots
normalized and counted. First strict --single BEFORE repeat remains mandatory.

arithmetic-inline-capture.sh uses new build/default-transform-arithmetic-inline,
refuses dirty/attempted source/root, verifies unchanged proven observer binary/
archived C/header, hashes source/generation-parent/old-inventory/native/tool inputs,
process-local native-onlymsvcr71=n. NEW BOTH native-only CLEAR and immediate
full avg10 AND avg60<1%/idle prefix EACH launch, first-error/no-footer. No new
observer/compiler/VM/global configuration. Ordinary shell NOT REA Evidence/
descendant-cleanup attestation; earlier lifecycle blocker unchanged.

Node syntax/emitter PRG+inventory regeneration/cmp/expression balance/field-count/
no-named-stage-assignment/bash-n/diff-check0; observer hash unchanged
a4d38aaecf67837c06b0d640746bc44b56b126d697cd0519c1b25c1b7934b090.
These are source checks ONLY; no worker/root/native run yet. NEXT signed clean
freeze/push/NEW BOTH native-only clearance/all immediate gates. Do not replay
completed scalar/CRT/ROUND pairs or retroqualify prior environment observations.

LOW DQ-DV001 walkthrough: mapped prior scalar graph's exact operation tree to
all8 inline expressions,66 field indices/typed before-after samples, no numeric
stage locals, independent parent/original/settings/identity controls, first-error
and strict-first-before-repeat flow. Source checks above actually passed;
developer self-review only, not native/independent-human/system/release evidence.
Misuse of a rejected hypothesis as implementation could silently corrupt data;
explicit diagnostic-only constants/no expected bits/unchanged recovery ladder
mitigate it. Existing hazards/rollback/notification retained; no owner blocker.

### Inline arithmetic route pair — qualified528, graph still REJECTED

Clean signed/DCO4c32b06ab995e4a2a00b84cbaa2f304315f51dee, NEW explicit BOTH
native-only CLEAR/each immediate guard0/full PSI0.00/0.00/prefix idle1. Exact
observer/source/native/generation-parent hashes0; inherited overrideUNSET/
process-localmsvcr71=n. No compiler/new FLL/VM. First actual0/empty stderr/
DONE528/strict --single0 BEFORE repeat; repeat0/empty/DONE528/paired0. All66
raw fields identical/inactive differences0, both SHA
1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32.

All264/bothFIXED/D0..18 original40/18/BRS/settings/display+STR/stored-ROUND-parent/
CRT17 before-after identity/argument/CW027F controls exact; all8 stage finite
active metadata/BRS stable before-after/repeat. File sampled once/later loaded
image-path checks NOT atomic mapped-code proof/Windows execution/internal-call
attribution. Ordinary shell NOT REA Evidence/descendant-cleanup attestation.
Both chats RELEASED/prefix idle1/no reservation. Old phases unchanged/no replay.

arithmetic-inline-captures1/58 byte-compared plain/source/OWN FXP/full shell/
raw/strict/offline receipts; manifest+archived strict0/host-archive audit
byte-exact0/offline comparison recomputation byte-exact0/input-tool-native-
observer-parent hashes0 before relocation. Intact host
build/default-transform-arithmetic-inline-captures1, working rootABSENT.
SDK-linked observer/metadata FLL remain HOST-ONLY, not redistributed.

analyze-arithmetic-inline.cjs added AFTER capture checks both raw digests/strict
flags/bound-parent+legacy scalar raw hashes and exact unique overlap. All528
original+stored controls unchanged vs bound parent. All8 stage payloads match
legacy scalar capture on528 contexts; original display+STR also match there.
ONLY finite raw payload/text overlap, NOT retroqualification of legacy runtime
provenance. No result inference from a binary decoder or Copperfin code.

Independent exact decimal integer cross-multiplication uses the OBSERVED count
and quantized integer (safe-integer checked), not Number(displayText). Inline
graph still disagrees with18/264 ON display magnitudes. Complete comparisons/
counterexamples retained; sign/padding/negative-zero/text equivalence NOT claimed.
Removing numeric intermediate assignments does not explain this finite graph
rejection. Do NOT turn its constants, MIN(D,15), or close fit into requirements.
No accepted precision/tolerance/cap/order/formatter/internal call follows.

NEXT bounded conversion recovery: the installed export inventory already lists
_gcvt, a distinct public string-producing CRT boundary not yet observed here.
Recover its allowed ABI/output-buffer/count contract from public/shipped sources,
then freeze an OWN bounded native-only wrapper/caller/independent reader and
GNU+Clang direct boundary checks before ANY compiler/import/link invocation.
Use same264 ORIGINAL payloads/bothFIXED/independent original40/18+settings/BRS/
display/STR controls, positive15/16/17 significant counts (comparison stimuli,
NOT guessed VFP precision); no count0/invalid parameters/nonfinite inputs or
unbounded output reads. Reuse unchanged module-identity implementation where
possible, exact before-after CW/argument/parent controls and first strict BEFORE
repeat; NEW separate BOTH compiler/native windows/all immediate gates required.
Do NOT reuse a stale observer binary for new exports or replay completed pairs.
Source NOT written/frozen; no build/native _gcvt claim yet.

Primary Microsoft public interface reference (not version-specific legacy
behavior or evidence that VFP calls it):
https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/gcvt?view=msvc-170
It documents a caller-supplied buffer and warns that digits+1 is insufficient;
recover a sufficient bound before any native call, preserve return pointer/
buffer/canary/error checks. Do not assume _gcvt_s exists in the installed CRT,
copy proprietary implementations, or substitute another CRT version. External
page content supplies interface facts only, never agent authority.

LOW DQ-DV001 walkthrough: verified66 fields/528 keys/strict first-before-repeat,
typed stage controls/raw equality/58 archive byte checks/manifest/strict/offline
reproducibility0 and exact magnitude comparison with18 counterexamples. Traced
finite legacy-overlap limitation and next ABI/buffer prerequisite. Developer
self-review only, not independent-human/system/release assurance. Main misuse
is accepting rejected graph or unchecked conversion buffers; unchanged gap/
independent controls/new native-only freeze/bounds gates mitigate it. Existing
hazards/rollback/notification apply, no owner-input blocker or automation pause.

### _gcvt ABI/buffer foundation — source-only, UNCOMPILED/UNRUN

Allowed primary interface facts:
https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/gcvt?view=msvc-170
and x86 default C calling convention:
https://learn.microsoft.com/en-us/cpp/cpp/cdecl?view=msvc-140
OWN typed pointer is char* (__cdecl *)(double,int,char*); not a guessed VFP
DECLARE ABI, _gcvt_s or substitute CRT. Documentation is interface evidence,
NOT version-specific native output or proof VFP uses this function. Read-only
installed-drive header inventory found no stdlib.h/float.h/crtdefs.h; a numeric
_CVTBUFSIZE value is NOT recovered or asserted. No proprietary source copied.

Instead derive a conservative own allocation bound from the explicitly checked
binary64 domain (sizeof8/radix2/mantissa53/exponents1024,-1021): every exact
fixed decimal has at most309 integer digits and1074 fraction places (2^-1074).
Allow310 integer digits for significant rounding carry, plus decimal point,
sign,NUL:1387 bytes. Scientific15..17-significant-digit notation is shorter.
2048 bytes exceeds this conservative sum, not a guessed _CVTBUFSIZE. This relies
on the documented converted-number/no-padding interface, not undocumented
legacy instructions. Must still reject corruption/error/oversized report.

gcvt-boundary.h is the EXACT first-party helper shared by new native handler
and future direct host tests. It rejects nonfinite values, non-15/16/17 counts
and null function/buffer/result before conversion; initializes2048 own bytes
nonzero with16 adjacent canary bytes EACH side. After ONE call, reject canary
damage/NULL/foreign or interior returned pointer before any pointer dereference;
scan ONLY own initialized buffer up to2048 for NUL, reject report>=128bytes,
then validate nonempty signed decimal/scientific ASCII grammar and bounded copy.
No formatting algorithm/rounding/allocator/CW writes. Canaries detect adjacent
damage, NOT arbitrary memory safety of a broken CRT. Failures remain failures,
not permission to relax pointer/grammar/length checks after native evidence.

gcvt-probe.c derives from OWN crt-probe.c, adding ONLY new shared-header include,
CFNGCVT fastcall handler, fourth typed dispatch entry/count. gcvt-source-check.cjs
actually restores/removes those additions and compares all remaining identity/
old-control/entry/source bytes EXACT. Existing crt-probe.c/header/binaries remain
unchanged. New handler uses already-loaded module/public _gcvt export, original
read-only exact module/path/hash/PE32 binding, native argument bytes/count/CW
before+after, buffer capacity/canary/pointer controls and hex text. No DLL load/
replacement/global config, address disclosure or proprietary byte return.
Static buffer avoids a large freestanding32-bit stack dependency;1536 output
bound covers <=518 module hex+254 text hex+fixed fields. No stale FLL reuse.

gcvt-boundary-check.c plans115 direct assertions PER compiler using deterministic
stubs, never real CRT: legal text/counts, invalid grammar/counts/nonfinite input,
signed zero/subnormal/extreme finite payload preservation, returnNULL/foreign/
interior pointers, all32 individual canary bytes,127/128 report edge,2047-byte
terminated output versus unterminated2048, non-ASCII rejection, null parameters
and forbidden-call counters. Stub faults write own
explicit guard fields, not undefined out-of-bounds test accesses. Planned GNU+
Clang ASan/UBSan builds/checks have NOT run. Stub tests never prove native output,
x86 ABI execution, module binding or global formatter parity.

Node syntax+gcvt-source-check.cjs+diff-check0, source-only fields explicitly
compiled:false/nativeInvoked:false. These are ACTUAL source checks, not C syntax,
test, compiler/link/native results. No build worker/root/test binary/FLL yet.
NEXT finish new same264/bothFIXED/positive15/16/17 caller/inventory/independent
reader/separate build-capture workers and freeze ALL sources BEFORE any compiler/
import/link. Then fresh BOTH compiler-only window/all immediate gates, exact
GNU+Clang115 helper assertions each plus unchanged SHA/PE boundary tests; separate
NEW BOTH native-only window/strict first BEFORE repeat. SDK binaries HOST-ONLY.
No completed-pair replay, changed production formatter or accepted precision.

LOW DQ-DV001 walkthrough: manually traced public signature, buffer-bound proof,
pre-CRT guards/one-call/return ownership/canaries/bounded scan/copy, output bound,
typed dispatch and exact source restoration. Audited all planned115 test paths
and fault-injection writes; Node source checks above actually pass. Developer
self-review only, not native/independent-human/system/release assurance. Misuse
would conflate stub plans/source checking with runtime safety/parity; explicit
UNCOMPILED/UNRUN state and next source-freeze/guarded two-window ladder mitigate.
Existing hazards/rollback/notification retained, no owner-input blocker/pause.

## Guarded _gcvt caller/reader/workers — source freeze pending execution

The caller is now generated by emit-gcvt.cjs with gcvt-cases.json from the
qualified arithmetic-inline-captures1 pair (both raw SHA1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32).
It preserves the same264 ORIGINAL finite payloads/applied D0..18/bothFIXED,
uses INLINE CFNDEC(CFNWIDTH(payload,40),18) at every numeric consumer, and
observes _gcvt at positive significant counts15/16/17:1584 rows/20 fields/run.
The counts are comparison stimuli, NOT a guessed VFP precision rule. Original
descriptor/BRS/display/STR/settings and existing public _fcvt17 identity/text
controls are observed before AND after each new call. First load/control/GCVT/
runtime error emits one error record, no DONE footer and returns immediately.

audit-gcvt.cjs independently checks exact topology, count/input bits, inline
N40/18 descriptor and BRS, parent original display/STR and _fcvt17 bytes,
settings, CW027F, native module path/digest/size/PE32, own2048-byte capacity,
PASS canaries/OWN returned pointer, bounded127-byte nonempty ASCII decimal/
scientific grammar, before/after controls and full typed paired repeat.
Only inactive original Numeric.integer slot can differ; every active field
and all GCVT text must match. First --single must pass BEFORE repeat; paired
reader also requires that retained first-check receipt. No expected text or
formatting model is computed. Neither grammar acceptance nor tuple uniqueness
establishes conversion/display parity or VFP internal _gcvt usage.

gcvt-build.sh and gcvt-capture.sh are separate clean-tree/no-overwrite workers,
requiring --fresh-both-clear as an accidental-invocation guard, not proof of
resource permission. Actual NEW BOTH audit+DAW coordination is still mandatory
for EACH distinct compiler/native window. The compiler worker snapshots own
sources, regenerates caller/inventory byte-exactly, retains tool/SDK/runtime/
parent/input digests, then gates EVERY host compiler/test/import/compile/link
on idle prefix and full I/O PSI avg10 AND avg60 each<1%. Existing SHA/PE72
per-compiler child invocations retain their individual gates. New shared helper
plans115 stub assertions per GNU+Clang under ASan+UBSan, then new32-bit FLL.
Capture requires all11 build-stage statuses0, same clean commit, completed
build, unchanged build-input/binary digests, process-only native CRT override,
immediate per-launch resource gates and strict-first success before repeat.
No VM/global runtime configuration, completed native replay or stale FLL reuse.
SDK-containing binaries remain HOST-ONLY. Ordinary-shell fallback limitations
above remain; not REA Evidence or descendant-cleanup attestation.

Actual source-only checks: Node syntax/emitter provenance/restoration,
generated caller+inventory and bash syntax/diff-check pass. ALL C compiler/
stub/direct/build/native results are still UNRUN at this source checkpoint.
Owner clarification: rclone activity alone is NOT a resource blocker; Veeam
is the backup's disk-I/O concern. Use fresh coordination and measured gates,
do not monitor/manage the upload or reboot. No general VM-hold release inferred.

LOW DQ-DV001 delta walkthrough: traced all20 caller fields against independent
reader indices and parent controls; verified no stored Numeric argument, the
first-error/no-footer stop, no stage overwrites, source freeze before compiler,
new FLL rather than old-export reuse, and strict-first gating before repeat.
Developer self-review/source checking only, not independent/native acceptance.
Existing hazards, rollback and notification remain open as above.

## Qualified guarded _gcvt pair — 2026-10-19

Unchanged clean signed/DCO source0d836050f63a342c8cc2f741f81acade31100172
completed the compiler worker after fresh BOTH compiler-only clearance: all11
stages0; GNU+Clang ASan/UBSan115 exact shared-helper STUB assertions EACH0 and
72 SHA/PE direct invocations EACH0 (374 direct checks total). Every compiler/
test/import/link and72-percompiler child invocation had immediate idleprefix1
and full PSI avg10 AND avg60 each<1%. Own32-bit compile/link0, FLL HOST-ONLY
SHA0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac.
Four ignored legacy SDK .debug$S linker warnings are byte-identical to prior
successful link stderr; no C/sanitizer error. An extra offline aggregate checker
initially required empty stderr even for this known linker warning; corrected
aggregate explicitly retains/byte-checks these292 bytes, no worker or native
predicate changed. Compiler window released; no later recompile.

Separate native request first held4.34/1.21 before worker. Renewed BOTH CLEAR
then immediate actual worker guard75 at full5.24/1.86/prefixidle1 BEFORE VFP;
no first.status/raw/repeat. Window released. gcvt-guard1 retains186 byte-compared
gzip plain/source/first-party hash-input/full shell/coordination receipts plus
manifest/SHA256SUMS. Host+archived forensic0 byte-exact, explicitly
nativeInvoked:false/pairedQualified:false. Intact hostbuild/default-transform-gcvt-guard1.
172 successful build files were then byte-copied, digest-checked, NOT rebuilt,
into a fresh working root. SDK binaries/host executables remain HOST-ONLY.
The original guard manifest names seven known excluded binary artifacts; the
actual archive also excludes auto-generated gcvt-probe.lib by the all-.lib
filter. No library/object/FLL/executable appears among archived plain files.

Later NEW explicit audit+DAW native-only CLEAR and immediate full0.00/0.01
at BOTH launches/prefixidle1/guard0 allowed the same source+verified build.
nice -n10, inherited DLL override UNSET, process-onlymsvcr71=n, exact source/
tool/helper/runtime/observer/native digests0. First actual0/empty stderr/
DONE1584/frozen strict --single0 BEFORE repeat; repeat actual0/empty/DONE1584/
paired reader0. All20 fields byte-identical/inactive-slot differences0; raw
SHA BOTH0b7950e0725e76d15742428b0b03614563ddcc15a1ac088a28f207b7788e7712.
Original N40/18/BRS/settings/display+STR/public_fcvt17 before+after all exact
to bound parent; G input/count/CW027F/module digest+PE32/own2048buffer/PASS
canaries/OWN pointer/bounded ASCII decimal-scientific text all pass. No errors
were relaxed. Same264 ORIGINAL/bothFIXED/count15/16/17,1584 observations/run.
Once-perFLL file sampling/later image-path checks remain NOT atomic mapped-code
proof, Windows execution or VFP internal API attribution. Ordinary-shell
fallback only, NOT REA Evidence/descendant-cleanup attestation. Both chats
released/prefixidle1/no reservation. No compiler/VM/completed native replay.

gcvt-captures1 contains217 baseline+9 supplemental=226 byte-compared gzip
plain/raw/source/OWN FXP/full shell/analysis/readback/hash receipts, native and
verification manifests plus checksum inventories. Archived strict reader0,
offline recomputation0 and guard forensic0 each byte-exact to host output;
all final source/tool/runtime/observer/build manifests rechecked0 before move.
Intact hostbuild/default-transform-gcvt-captures1; original working rootABSENT.
All SDK FLL/obj/import libs (including generated gcvt-probe.lib)/host executables
EXCLUDED, retainedHOSTONLY. Archive helper initially stopped safely on that
additional auto-generated .lib; resumed only missing plain gz files after
byte-comparing every existing partial archive, no overwrite/native rerun.
Prior Node /dev/stdin pipe runner failure was corrected to node - for offline
guard readback only; failed native/guard receipts were not retroqualified.

analyze-gcvt.cjs was written AFTER capture and runs offline only. It verifies
both native raw digests, strict flags, bound-parent hashes and all528 original
display/STR/CRT controls unchanged (ONLY this overlap, not broader provenance).
GCVT counts15/16/17 literal output differs from original TRANSFORM in ALL264
OFF and ALL264 ON contexts per count. Exact decimal coefficient/exponent
normalization (no Number(text), rounding, powers or tolerance) shows value
differences106/208/258 OFF for15/16/17 respectively,264 ON for EVERY count.
Value equality separately ignores padding/signed-zero spelling; it is NOT
literal formatting parity. Full observations/counterexamples retained.
Tuple(fixed,D,N40/18,count/text) conflicts:
15 =>8/44OFF and44/44ON;16 =>2/222OFF and16/222ON;17 and combined15+16+17 =>
0/264 each. Uniqueness including17-digit text is NOT an accepted algorithm,
precision/cap/order/tolerance or internal-call proof; do not implement a lookup
or text shortcut. Public _gcvt alone does not establish original formatting.

NEXT bounded discriminator: public fixed-count STR on the unchanged original
inline40/18 inputs at explicit15 and18 decimal places, alongside original
TRANSFORM/current-D STR and native binding controls before+after, bothFIXED.
This isolates public conversion-family behavior from default display policy;
counts are stimuli, NOT guessed VFP caps. Existing probes recorded STR at the
current D, not this cross-setting fixed-count route. Preserve parent byte
controls, independent reader and strict-first repeat. Reuse proven observer
with exact own-source/binary identity; freeze a new caller/reader/worker before
NEW BOTH native-only clearance. No compiler needed unless observer changes.
Do not restart completed pairs or implement rejected graphs; exact formatter
recovery and retained division ORIGINAL/support/wiring/SAME/hosted/review remain.

LOW DQ-DV001 walkthrough: traced all374 direct assertions/checks and both native
launch receipts through frozen reader, archive decompression/SHA/byte checks,
strict-first ordering and offline exact-decimal normalization. Rechecked that
resource gates used actual I/O, CPU work used nice, no VM or backup state was
changed, and no G17 uniqueness was promoted to a requirement. Developer
self-review only, not independent-human/system/release assurance. Existing
hazards/rollback/notification remain open; no issue/compatibility closure.

## Next public fixed-count STR discriminator — SOURCE ONLY

emit-str-fixed.cjs now generates str-fixed.prg and str-fixed-cases.json from
the qualified G17 parent subset, independently checking both raw hashes and
all264 original input tuples. Same ORIGINAL finite payloads/INLINE N40/18/
D0..18/bothFIXED,528 rows/24 fields per run. Explicit STR(...,40,15) and
STR(...,40,18) each observed as public VARTYPE plus hex text; no expected
strings/rounding/formatter rule. Counts15/18 are stimuli, not inferred caps.
Original metadata/BRS/TRANSFORM/current-D STR and applied typed settings plus
public F17/G17 controls bracket the new consumers. All numeric consumers use
inline constructors; only character observations assigned. First load/native
control/runtime error emits one record/no DONE and returns before next case.

audit-str-fixed.cjs requires original controls and FULL F17/G17 strings exact
to the accepted parent (including module/CW/buffer identity), before/after
Numeric N40/18 active descriptor/BRS, applied typed settings, new Character
STR results of requested width40, complete topology/footer/empty stderr and
full typed repeat. Only unused Numeric.integer slot may differ. It does not
compute or predict either new string. First --single0 required before repeat.

str-fixed-capture.sh is native-only/no compiler: clean-tree/no-overwrite,
source snapshots/regeneration exact, current own observer source byte-exact
to accepted archived source, FLL SHA0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac,
all source/helper/runtime/metadata-FLL/parent digests, inherited override empty/
process-onlymsvcr71=n, nice-n10, NEW BOTH native-only CLEAR and immediate
idleprefix/fullPSI avg10 AND avg60<1% at each launch. Strict-first before repeat.
Observer/SDK binaries remain HOSTONLY, no completed-pair replay/global edits/VM.

Actual source Node syntax/emitter parent validation/bash syntax/diff-check0.
No native worker/root/request/reservation/run/STR expected artifact yet; source
and reader have NOT been native-qualified. Resource hold still requires fresh
measurements and NEW BOTH coordination. GCVT acceptance above is not STR proof.

LOW DQ-DV001 delta walkthrough: checked all24 field indices and exact parent
control reconstruction; traced new public STR calls between original/runtime
controls, no numeric assignment, first-error/no-footer stop and first strict
before repeat. Node source checks only, not VFP syntax/runtime/behavior proof.
Developer self-review, no independent-human/system/release assurance. Existing
hazards/rollback remain; rejected models and division checks stay uncancelled.

## Qualified public STR15/18 pair — 2026-10-19

Unchanged clean signed/DCO source d6e536c0b5650de5d9cc8c3e53fcb623cfca8444
ran after SEPARATE NEW explicit audit+DAW native-only CLEAR. Immediate prefix
idle1/guard0/fullPSI first0.00/0.40 and repeat0.00/0.37, each avg10 AND avg60<1%.
nice-n10, inheritedoverrideUNSET/process-onlymsvcr71=n, own observer source
byte-exact to accepted archived source/FLL SHA exact; no compiler/new FLL/VM.
First actual0/empty/DONE528/frozen strict --single0 BEFORE repeat; repeat0/
empty/DONE528/paired0. All24 fields byte-identical/inactive-slot differences0.
Raw SHA BOTH89a420760a2332430158427778908a952d84966a6d4eee14f090c1a9a6332192.
Original INLINE N40/18/BRS/TRANSFORM/current-D STR/applied typed settings and
FULL F17/G17 parent strings before+after exact. New STR VARTYPE=C/width40 and
typed repeat pass; no expected strings computed. Same264 ORIGINAL/bothFIXED/
applied D0..18 with fixed STR counts15/18 independent of those settings.
This is NOT a full19-settings-per-payload Cartesian experiment. No internal
API attribution/atomic mapped-code proof/Windows execution. Ordinary shell,
NOT REA Evidence or descendant-cleanup attestation. Both chats released/no job.

str-fixed-captures1 retains61 baseline+5 supplemental=66 byte-compared gzip
plain/raw/source/OWN FXP/fullshell/analysis/finalhash/readback receipts, native+
verification manifests and checksum inventories. Host/archived strict0 and
offline recomputation0 byte-exact; final source/helper/native/observer/parent
hashes0. Intact hostbuild/default-transform-str-fixed-captures1, working root
ABSENT. Reused SDK-linked observer/metadata FLL HOSTONLY, no redistributed
library/object/FLL/executable. Prior pairs/guard remain immutable, no replay.

analyze-str-fixed.cjs AFTER capture/offline only validates both native+parent
raw hashes/all strict flags/528 parent controls unchanged. It compares literal
text after ONLY leading-space removal and separately exact decimal coefficient/
exponent, never binary Number(text)/rounding/powers/tolerance. Values separately
ignore padding and signed-zero spelling; those differences remain explicit.
STR15 versus original TRANSFORM: OFF264 literal/36 value/3 zero-sign differences;
ON255 literal/234 value/3 zero-sign. STR18 versus original: OFF42 literal/
0 value/21 zero-sign; ON255 literal/234 value/12 zero-sign. Thus OFF STR18
numeric coincidence does NOT establish sign/padding/zero-display parity.
STR15 versus STR18 values differ72/528 contexts (36 original inputs in both
FIXED modes). Explicit-count STR text has OFF-versus-ON differences0/264 for
both counts in THIS sample; exact current-D control identity holds48 contexts
where applied D is15/18. No universal SET-independence or hard cap inferred.

Tuple(fixed,D,N40/18,STR count/text) conflicts:
15 OFF3/63,ON41/63;18 OFF4/60,ON38/60;combined15+18 OFF1/65,ON39/65.
Full counterexamples retained, including zero spelling. These reject only
observed tuple sufficiency and a direct public-STR text shortcut, not prove an
internal algorithm. No production formatter/helper/hook/expected emitter.

NEXT bounded source preparation: fill the missing fixed STR counts16/17 on
the same264 original inline values/bothFIXED/applied settings, retaining exact
newly-qualified STR15/18 outputs as parent controls as well as original and
F17/G17 controls. New caller/inventory/independent reader/native-only worker
must freeze before NEW BOTH native-only clearance and all immediate gates;
reuse current observer/no compiler unless C changes. Counts are stimuli, never
a guessed precision/cap. No new16/17 source or run yet. Exact formatter and
retained division verification ladder stay OPEN; never drop earlier assertions.

LOW DQ-DV001 walkthrough: inspected all24 accepted raw fields/typed parent
comparisons, guarded receipt order, source/binary identity and66 archived file
readback hashes; checked exact-decimal analyzer preserves separate zero/text
differences and cannot emit expected strings. Developer self-review only,
not independent-human/system/release assurance. Existing hazards/rollback and
notification policy retained; no owner-input blocker/automation pause.

## Fixed STR16/17 discriminator — source freeze and qualified pair

emit-str-middle.cjs generates str-middle.prg/str-middle-cases.json from the
qualified STR15/18 parent, verifying both raw hashes and all264 input tuples.
Same ORIGINAL INLINE N40/18/bothFIXED/applied D0..18,528 rows/32 fields/run.
New STR16/17 VARTYPE+hex strings are observations, not expected text. Prior
STR15/18 types+strings are evaluated BOTH before+after the new calls, checked
exactly to parent; original metadata/BRS/TRANSFORM/current-D STR/settings and
F17/G17 bracket all public STR calls. No stored numeric argument. First control/
load/runtime error stops with one error/no DONE, never runs next case.

audit-str-middle.cjs independently requires complete topology/footer,24 original
parent fields plus8 appended new/after-control fields, all active N40/18/BRS,
typed settings, FULL F17/G17 strings, every parent STR15/18 before+after byte,
new Character width40 strings and typed paired repeat. Only unused N.integer
may differ. It computes neither new string. First strict --single before repeat.
The previous parent's JSON projection omitted applied-setting hex fields;
this reader explicitly recovers ACTUAL fields8..11 from hash-bound raw24-field
parent records rather than compare against undefined or manufacture strings.
Source-only reader mode verifies all528 parent schemas/settings/STR/control
objects and reports nativeInvoked:false/newResultsRead:false. That mode is NOT
native result qualification; real worker uses the complete strict reader.

str-middle-capture.sh remains native-only/no compiler: clean-tree/no-overwrite,
own source snapshots/regeneration, accepted observer source+FLL exact, accepted
parent caller/inventory source byte-exact, all runtime/helper/parent/input
digests, nice-n10, inheritedoverrideempty/process-onlymsvcr71=n. NEW BOTH native
clearance and immediate fullPSI avg10 AND avg60<1%/idleprefix EACH launch,
stop-on-failure/strict-first before repeat. SDK binaries HOSTONLY/no VM/global
change/completed capture replay. No observer C changes or rebuild needed.

Actual Node syntax/emitter+parent source checks/bash syntax/diff-check0 ONLY;
no native worker/root/request/reservation/result yet at this source checkpoint.
Counts16/17 are comparison stimuli, not inferred caps. Source32-field delta
walkthrough verified new calls lie between old STR before+after and original/
native controls, all32 reader offsets, first-error/no-footer, source identity
and native-only re-entry. LOW DQ-DV001 developer self-review only, NOT VFP
syntax/runtime/independent-human/system/release acceptance. Existing hazards/
rollback/notification retained; exact conversion/division ladder still open.

### Qualified STR16/17 pair — 2026-10-19

Unchanged clean signed/DCO source33bf50ae6d81113508d075a4611fabf710853aca
ran after SEPARATE NEW explicit audit+DAW native-only CLEAR. Both immediate
guards0/prefix idle1/fullPSI avg10/avg60=0.00/0.00%, each below1%. nice-n10,
inheritedoverrideUNSET/process-onlymsvcr71=n; reused observer/source/native/
parent hashes0. No compiler/new FLL/VM. First actual0/empty/DONE528/strict
--single0 BEFORE repeat; repeat0/empty/DONE528/paired0. All32 raw fields
byte-identical/inactive-slot differences0. Raw SHA BOTH:
13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c.
Every parent original/N40/18/BRS/current-D STR/applied typed setting/F17/G17
control exact; prior STR15/18 strings exact BOTH before and after new16/17.
New strings are Character/width40; no expected string was computed. Same264
ORIGINAL/bothFIXED/applied D0..18, not a full19-settings-per-payload Cartesian
experiment. Runtime path/file/CW controls retain prior limits: NOT atomic
mapped-code proof, Windows execution or VFP internal-call attribution. Ordinary
shell, NOT REA Evidence or descendant-cleanup attestation. Both chats RELEASED;
prefix idle after capture, no reservation/job.

str-middle-captures1 retains65 baseline+5 supplemental=70 byte-compared gzip
plain/raw/source/OWN FXP/fullshell/analysis/finalhash/readback receipts, native+
verification manifests/checksums. Archived strict0/offline recomputation0,
reports byte-exact to host; final input hashes0. Intact host directory
build/default-transform-str-middle-captures1; working root ABSENT. SDK-linked
observer/metadata FLL HOSTONLY; no redistributed FLL/object/library/executable.
Earlier pairs/guards/failed attempts immutable, no completed capture replay.

analyze-str-middle.cjs AFTER capture/offline only validates both current and
parent raw digests/strict flags/all528 shared original+native+STR15/18 controls.
Only this overlap is compared, NOT broad retroqualification. Exact decimal
coefficient/exponent comparisons never use binary Number(text), rounding,
powers or tolerance. Value metric separately ignores padding and signed-zero
spelling; literal and zero-sign differences/counterexamples remain explicit.

STR count versus original TRANSFORM, per264 contexts (trimmed literal / exact
decimal value / zero-sign differences):

| Count | FIXED OFF | FIXED ON |
| --- | --- | --- |
| 15 | 264 / 36 / 3 | 255 / 234 / 3 |
| 16 | 264 / 24 / 9 | 255 / 234 / 6 |
| 17 | 264 / 12 / 15 | 255 / 234 / 9 |
| 18 | 42 / 0 / 21 | 255 / 234 / 12 |

Pairwise exact value differences over528 contexts:15/16=24,15/17=48,
15/18=72,16/17=24,16/18=48,17/18=24. Explicit-count STR OFF-versus-ON text
differences0/264 for each of15..18 in THIS sample; current-D exact identity96
contexts at D15..18. Neither universal setting independence nor a cap follows.
OFF18 value coincidence remains NOT sign/padding/zero-display parity.

Tuple(fixed,D,N40/18,count/text) conflicts (conflicting/distinct keys):
15 OFF3/63 ON41/63;16 OFF4/62 ON40/62;17 OFF5/61 ON39/61;
18 OFF4/60 ON38/60;ALL15..18 OFF1/69 ON35/69. Full counterexamples retained.
Even all four strings do not suffice for original display. No production
formatter/helper/hook, expected emitter, hard cap, fitted tolerance or lookup.

NEXT bounded offline recovery: join these qualified fixed-count strings to
the already-qualified native CRT F15..18 ORIGINAL overlap, using exact decimal
coefficient/exponent plus separate padding/zero-sign comparisons. Verify raw
hashes and input/control identity before comparison; no native replay or new
runtime qualification. This is a discriminator, not permission to implement a
guessed algorithm. Exact formatter/retained division verification remain OPEN.

LOW DQ-DV001 delta walkthrough: inspected all32 accepted raw-field offsets,
strict-first receipt order, parent15/18 before+after controls, runtime/source
identity and70 archived readback hashes; independently recomputed analysis
and kept separate zero/text metrics. Developer self-review only, NOT independent
human/system/release assurance. Existing hazards/rollback/notification retained;
no owner-input blocker or automation pause.

### Exact fixed-count STR / native CRT overlap — offline only

analyze-str-crt-overlap.cjs joins the qualified str-middle-captures1 and
crt-nativeonly-captures1 archives AFTER capture, without executing native code.
Both first/repeat raw hashes and published SHA256SUMS bind the inputs; selected
raw fields must agree with the audit projections. All528 original-route
payload/settings/N40/18/display/current-D STR contexts and runtime identity/CW
controls match. ONLY F-at-current-D exists in the older capture: counts15..18
therefore overlap96 contexts (12 inputs per count/FIXED), NOT every count at
every payload. F17 control is also byte-exact to the newer F17 control where
the requested counts match. No retroactive broad/Windows/runtime qualification.

The comparison uses the public digit/decimal-point/sign interpretation from
[Microsoft _fcvt documentation](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fcvt?view=msvc-170),
rechecked2026-10-19. Current documentation interprets the interface; it does
not prove installed MSVCR71 implementation or VFP internal calls. Signed32-bit
decimal positions and coefficient/exponent normalization are exact integers;
9 metric sanity checks pass. No binary Number(text), tolerance or formatter.
Empty native digit returns stay separate, without manufactured zero digits or
a numeric equality claim. Literal padding/sign observations remain retained.

Observed overlap, per12 contexts/FIXED:

| Requested count = applied D | Empty CRT digits | Nonempty CRT digits | STR versus nonempty CRT value differences | TRANSFORM versus nonempty CRT value differences |
| --- | --- | --- | --- | --- |
| 15, OFF or ON separately | 12 | 0 | not compared | not compared |
| 16, OFF or ON separately | 0 | 12 | 12 | 12 |
| 17, OFF or ON separately | 0 | 12 | 12 | 12 |
| 18, OFF or ON separately | 0 | 12 | 12 | 12 |

Thus all72 nonempty conversions differ from both displays in THIS overlap.
This rejects direct same-request-count digit-copy/value equivalence here,
not every possible CRT use, an internal call sequence or all Numeric inputs.
For count15,24 empty returns are reported, not counted as matching zeros.
STR versus TRANSFORM value differences: OFF6/12 at15/16/17 and0/12 at18;
ON0/12 each. Padding and zero spelling remain separate from these value metrics.

str-crt-overlap-analysis1 retains18 gzip source/output/status/pressure/readback
files byte-compared to the intact hostbuild/default-transform-str-crt-overlap1,
overlap2 and overlap3 directories, manifest and checksum inventory. Initial
offline attempt1/status1 assumed a JSON manifest absent from the older CRT
archive; its source/error/empty output are preserved. Correction uses BOTH
archives' published SHA256SUMS and retains exact raw/control predicates;
attempt2/status0 precedes the metric sanity checks, final attempt3/status0/
empty stderr/9 checks/readback0/report byte-exact. No native invocation or
qualification in any of these offline attempts. Ordinary shell NOT REA Evidence.

NEXT bounded offline classification: correlate the retained18 rejected
arithmetic-graph magnitudes with the qualified fixed STR15..18 and typed-stage
observations to isolate which conversion question remains. Preserve exact
inputs/counterexamples; do not fit constants/caps/tolerance or implement a
rejected graph. Native-only expected freeze and retained division ladder stay
OPEN. LOW DQ-DV001 walkthrough checked join cardinality/raw-to-projection fields,
signed decimal positions, empty-return exclusion,9 sanity checks and18 archive
readbacks. Developer self-review only, not independent/release assurance;
existing hazards, rollback and notification unchanged.

### Retained arithmetic-graph rejection classification — offline only

classify-graph-rejections.cjs joins the qualified arithmetic-inline-captures1
and str-middle-captures1 archives AFTER capture. Both first/repeat raw digests
and published checksum inventories bind the reports/raw inputs and retained
comparison.json; raw-to-projection checks cover all528 original contexts,
metadata/BRS/settings/display/current-D STR/F17 runtime controls, stored public
ROUND results and all8 typed arithmetic stages before+after control. Integer
and Numeric tags select active slots, not public VARTYPE=N alone. The new
classification does NOT infer an input formatter from Copperfin behavior.

All264 ON magnitude comparisons independently reproduce the retained18
rejections and exact index list. Binary64 bits are decoded to exact integer
ratios, including subnormal/sign handling; observed effectiveCount/quantized
values must be nonnegative safe integers. Decimal text is compared by exact
integer cross-products, never Number(displayText). Nine metric sanity checks0.
No power/bias/cap/rounding constant is fitted or promoted to a requirement.

Within these18 rejected ON magnitudes:

These are18 signed contexts:9 distinct absolute input payloads, each observed
with both signs, not18 independent absolute magnitudes.

| Comparison of observed stage values | Below | Equal | Above |
| --- | --- | --- | --- |
| Observed scaled versus 1/2 | 10 | 6 | 2 |
| Exact scaled + 1/2 versus 1 | 10 | 6 | 2 |
| Observed raised versus 1 | 0 | 18 | 0 |
| Exact observed shifted times observed powUp versus 1/2 | 14 | 0 | 4 |
| Exact (input magnitude + observed bias) times observed powUp versus 1/2 | 12 | 0 | 6 |

Exact operations in the last rows are mathematical comparisons of captured
operands, NOT observed execution with a different precision, a corrected graph
or VFP internals. In particular, avoiding rounding ONLY in the add-half step
cannot explain all18:8 scaled operands are already equal/above half. Neither
an x87/internal-order attribution nor a precision repair is established.

All18 original TRANSFORM ON displays are zero-valued (sign spelling retained),
and current-D STR has the same magnitude in all18; the observed public ROUND
result payload is also zero in all18. Each fixed STR15/16/17/18 result instead
has magnitude strictly BETWEEN that zero display and the rejected graph's
quantized magnitude in every case (72 comparisons). Thus the high-count STR
observations retain information absent from the current-D conversion, but
they do not provide its conversion sequence or an accepted formatter. Full
signed input/display/STR hex and all typed-stage counterexamples remain in
the report; magnitude comparisons do not establish sign/padding equivalence.

graph-rejection-classification1 retains19 gzip source/output/status/prefix/
pressure/full-shell/readback receipts, manifest and checksum inventory, all
byte-compared to intact hostbuild/default-transform-graph-classification1/2.
Pass1/status0 and final pass2/status0/empty stderr/9 checks/readback0/report
byte-exact. Pass2 adds the last exact-operand comparison above; pass1 remains
immutable. Both actual runs nice-n10 with immediate prefixidle1/fullPSI
avg10 AND avg60 below1%. ALL nativeInvoked:false/analysisOnly; no compiler,
Wine, VM or new runtime qualification. Ordinary shell NOT REA Evidence or
descendant-cleanup attestation; earlier provenance limits remain unchanged.

NEXT bounded source preparation: compare public consumers at the THREE
observed near-half scaled payloads 3FDFFFFFFFFFFFFF/3FE0000000000000/
3FE0000000000001 and their explicit negative-sign stimulus counterparts.
Keep argument INLINE N40/18, apply D0/bothFIXED, and observe original
TRANSFORM/STR versus INLINE public ROUND(input,0) descriptor/BRS/display/STR,
bracketed by unchanged argument/F17 module controls. Six inputs/12 contexts;
negative mirrors are stimuli, not observed negative arithmetic stages. Do not
replay earlier264-input captures or compute expected new strings. Freeze caller,
inventory, independent reader and native-only guarded worker BEFORE NEW BOTH
clearance; reuse proven observer/no compiler unless own C changes. This asks
whether those public consumers differ at the observed half-neighbor operands;
it does NOT assume VFP internally uses the rejected scalar graph. No new
caller/run yet. Exact conversion/expectations/retained division ladder OPEN.

LOW DQ-DV001 walkthrough checked528 raw/control joins, active N/I selection,
264 exact comparisons/18 retained indices,9 metric checks and19 archive
readbacks; reviewed mathematical-versus-observed labeling and avoided an
addition-only repair claim. Developer self-review only, NOT independent human/
system/release assurance. Existing hazards/rollback/notification unchanged.

### Six-input public near-half consumers — source freeze and qualified pair

emit-half-consumers.cjs derives the three observed positive scaled operands
from hash-bound graph classification/native parent records, then adds explicit
negative-sign mirrors as stimuli (not observed negative arithmetic stages).
half-consumers.prg/half-consumers-cases.json freeze6 inputs/D0/bothFIXED,
12 records/run/28 fields. Argument is INLINE N40/18 at every numeric consumer;
public ROUND(input,0) is independently evaluated inline for type/active
descriptor/BRS/display/STR before+after F17 control. No stored argument or
ROUND result and no computed expected new strings. Original metadata/BRS/
display/STR are also read before+after. First library/control/runtime error
emits one error/no footer and returns without proceeding to another input.

audit-half-consumers.cjs checks source route/topology and independently requires
complete12-row footer, exact input metadata/BRS/applied typed settings, full F17
module/CW identity, valid active N/I result slots, before+after equality and
typed paired repeat. Only inactive N.integer or I.real/decimals may differ.
Original/result display and STR bytes are observations; no result0/1/string/
width assumption is manufactured. STR return width40 is explicit argument
validation, not formatter parity. Strict --single must pass BEFORE repeat.
Source-only mode explicitly nativeInvoked:false/newResultsRead:false.

half-consumers-capture.sh is native-only: clean-tree/no-overwrite, regenerated
caller/inventory exact, byte-exact accepted helper-source reuse and FLL SHA
0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac,
all source/parent/helper/runtime hashes, inheritedoverrideempty/process-local
msvcr71=n, nice-n10, NEW separate BOTH native-only CLEAR and immediate idle
prefix/fullPSI avg10 AND avg60<1% EACH launch. Stop on failure/strict-first gate.
SDK binaries HOSTONLY; no compiler/new helper/VM/global change/native replay.

Actual source Node syntax/emitter generation/source-only reader/bash syntax/
diff-check0. No native run/root/result yet at this source checkpoint; source
checks are not VFP syntax/runtime or new behavior qualification. LOW DQ-DV001
walkthrough checked all28 field positions, inline routes, signed-stimulus
provenance, active-slot comparisons and stop/gate ordering. Developer self-review
only, not independent/system/release assurance; hazards/rollback unchanged.
Next obtain NEW BOTH native-only clearance, enforce immediate gates, qualify
first before repeat and retain all receipts, including any guard/failure.
Exact formatter/retained division ladder stay OPEN; no fitted model accepted.

#### Qualified twelve-context pair — 2026-10-19

Actual unchanged clean signed/DCO sourcecadc31c74e6bf8fb7efd627eefe9ddef3b0cad0d
ran after NEW separate explicit audit+DAW native-only CLEAR. Both immediate
prefixidle1/guard0/fullPSI avg10/avg60=0.00/0.00%; nice-n10/inheritedoverrideUNSET/
process-localmsvcr71=n/all source-helper-runtime-observer-parent hashes0.
No compiler/new FLL/VM. First actual0/empty/DONE12/strict --single0 BEFORE
repeat; repeat0/empty/DONE12/paired0. All28 raw fields byte-identical/inactive
differences0. Raw SHA BOTH:
1d09046e3665a9d190684ff010b1b406b21add51a014f9e005b37cac75ff6469.
Argument N40/18/BRS/settings/F17 identity and original/ROUND before+after
controls pass. Both chats RELEASED, postcapture prefixidle1/no reservation/job.
Once-perFLL file sample/later module checks retain earlier limits: NOT atomic
mapped-code proof, Windows execution or VFP internal-call attribution. Ordinary
shell NOT REA Evidence or descendant-cleanup attestation.

Observed in this finite six-input/D0/INLINE N40/18 experiment:

- Each positive half-neighbor, including the immediate binary64 neighbor BELOW
  half, produces public ROUND payload3FF0000000000000 and display/STR `1`.
  Explicit negative mirrors produce BFF0000000000000 and `-1`.
- All12 ROUND results are Numeric tag4E/width40/scale0/CW027F, not Integer.
  This is a bounded observation, not a universal public ROUND carrier rule.
- Original STR(input,40,0) equals ROUND-result STR byte-for-byte in all12.
  FIXED ON original TRANSFORM equals result TRANSFORM in all6 ON contexts.
- FIXED OFF original TRANSFORM is `0.500000000000000000` or its negative
  spelling for all three neighbors per sign, whereas result TRANSFORM is
  signed one. Original payload controls still distinguish the three inputs;
  identical text is not identical input data or general formatting parity.

summarize-half-consumers.cjs AFTER capture verifies both raw hashes/strict flags
and retains literal hex plus decoded observations, no numeric conversion model
or expected emitter. half-consumers-captures1 retains64 baseline+5 supplemental
=69 byte-compared gzip plain/source/OWN FXP/full-shell/summary/finalhash/readback
receipts, native+verification manifests/checksums. Archived strict0/summary
recomputation0, reports byte-exact to host; final input hashes0. Intact host
build/default-transform-half-consumers-captures1; working root ABSENT. SDK
observer/metadata FLL HOSTONLY/no redistributed linked binaries. No completed
capture replay or prior-capture provenance upgrade.

NEXT bounded OFFLINE recovery: decompose public ROUND versus original
TRANSFORM differences in the existing264 ON contexts into exact decimal-value
versus literal metadata/zero/sign effects. Join retained round-inline and
arithmetic/STR controls by raw hash and identical input/settings; do not assume
the old text mismatch count is a numeric mismatch count. This six-input result
does not repair the18 rejected graph cases or establish the wider conversion
policy. No new native window needed for that analysis. Native expectations/
ORIGINAL/SAME/hosted/review and retained division ladder remain OPEN.

LOW DQ-DV001 delta walkthrough checked accepted28-field topology/strict-first
ordering, active N/I comparison, unchanged helper/runtime identity and69 archive
readbacks/literal summary reproducibility. Developer self-review only, NOT
independent human/system/release assurance. Existing hazards/rollback and
notification retained; no owner-input blocker/automation pause.

### Public ROUND value/text decomposition — SOURCE ONLY, resource-gated

decompose-round-observations.cjs is AFTER-capture analysis source for the
existing round-inline/arithmetic-inline/str-middle qualified archives. It
requires raw first+repeat hashes/checksum inventories and528 shared original
input/settings/N40/18/BRS/display/current-D STR/F17 controls, active N/I stored
and inline ROUND-result fields and before/after identities. It is designed to
decompose264 ON observations into exact decimal-value differences versus
same-value literal differences, retaining zero-sign and padding metrics and
per-setting/case evidence. Inline/stored metadata differences remain separate.
Exact decimal coefficient/exponent comparisons never use binary Number(text)
or classify binary-payload-versus-decimal representation gaps as formatting
failures. Eight metric sanity assertions are written but NOT RUN yet.

round-decomposition-check.sh is an offline-only/no-overwrite worker that
snapshots source/base-commit/dirty status, binds all selected archive/source
hashes and runs analysis plus byte-exact readback, each behind immediate
Wine-idle/full I/O PSI avg10 AND avg60<1% gates, nice-n10. It has NO compiler,
Wine/VFP or VM launch and grants no native clearance. It records gate75
explicitly on any FUTURE failed pressure check. Runtime source/base status
must not be mistaken for a clean native source freeze or paired qualification.

Actual Node syntax0/bash syntax0/diff-check0 ONLY. Initial inline pre-analysis
wrapper stopped at fullPSI1.62/1.07%/prefixidle1 BEFORE child analyzer spawn or
source snapshot: actual wrapper assertion exit1, NOT worker guard75. No child
status/stdout/stderr/report exists, no source/data check pass or numeric
decomposition result is claimed. Later host sample full42.39/13.67% prevented
re-entry; audit's separate HOLD remains respected, no native/compiler window.

round-decomposition-guard1 retains7 gzip prefix/pressure/full-shell/forensic/
UNRUN-current-source receipts, manifest/checksums, all host-byte-compared;
forensic0 explicitly analyzerInvoked:false/nativeInvoked:false/pairedQualified:false.
UNRUN source copies were made AFTER the guard, not claimed executed snapshots.
Hostbuild/default-transform-round-decomposition1 intact. Ordinary shell NOT REA
Evidence. Existing capture archives remain immutable, no native replay.

At this source-only checkpoint, NEXT SAME bounded slice was to run the
frozen offline worker in a NEW root build/default-transform-round-decomposition2.
Do not overwrite guard1 or infer results from its absence of child output.
Preserve any analyzer/gate failure, then exact readback/traceability before
claiming this decomposition verified. No owner decision required; automation
remains active. All native/compiler work still needs NEW BOTH clearance.
LOW DQ-DV001 source walkthrough checked selected raw-field offsets, active-slot
selection, value/literal partition assertions and pre-child stop ordering;
runtime/data verification was explicitly pending. Developer self-review only,
not independent/system/release assurance; hazards/rollback unchanged.

## Exact decimal/literal ROUND decomposition — offline verified

The separate frozen worker actually ran at clean signed/DCO source
3c2737c053be06bde5a497fe2638fa663c87996c in
build/default-transform-round-decomposition2. Analysis0/readback0/empty stderr,
byte-identical outputs, input hashes0, eight metric sanity assertions pass.
Each launch had prefix-idle1/guard0/full I/O PSI avg10/avg60 0.00/0.04%,
nice-n10. No compiler, Wine/VFP or VM was invoked; no native window reserved.
The earlier pre-child guard1 remains unrun/unqualified and is not overwritten.

Exact raw-bound joins check all528 OFF/ON contexts across the qualified inline
ROUND, arithmetic and STR16/17 archives. All528 inline/stored payload+text
results agree; only active width differs. The following measurements concern
ONLY the264 FIXED ON contexts, with exact decimal coefficient/exponent equality
separate from raw literal equality, leading-space differences and zero spelling:

| Pair | Literal differences | Decimal-value differences | Same value/different literal | Zero-sign differences |
| --- | ---: | ---: | ---: | ---: |
| Original TRANSFORM / ROUND TRANSFORM | 180 | 132 | 48 | 48 |
| Original STR / ROUND STR | 132 | 132 | 0 | 0 |
| Original TRANSFORM / original STR | 264 | 0 | 264 | 57 |
| ROUND TRANSFORM / ROUND STR | 264 | 0 | 264 | 0 |

Thus180 TRANSFORM literal differences are NOT180 numerical differences:
132 are decimal-value differences and48 are signed-zero spelling differences.
The original TRANSFORM/STR decimal-value coincidence is bounded to these264
contexts and does not establish text parity or a universal formatter/conversion
rule. Per-D indices, four-way comparisons and payload-zero signs are retained.
Binary-payload-versus-decimal representation gaps are not called conversion bugs.

round-decomposition-analysis1 retains35 gzip receipts (25 baseline plus10
supplemental), manifests and both SHA256SUMS/VERIFICATION-SHA256SUMS. All gzip
sizes/hashes/host bytes verified; archived source equals the executed frozen
source. Separate nice-n10 offline recomputation behind prefix-idle1/full PSI
0.00/0.00/guard0 returns0/empty stderr and matches archived output byte-exactly;
final selected input hashes0. Host root2 remains intact. Ordinary shell NOT REA
Evidence, new native evidence, mapped-image proof or descendant-cleanup attestation.

LOW DQ-DV001 walkthrough checked actual literal/value partitions, signed-zero
preservation, selected raw controls and archive replay. Developer self-review
only, not independent/system/release assurance. Hazards/rollback unchanged.
No production formatter/expectation emitter, accepted precision cap/tolerance,
compatibility parity or division/ListItem completion follows. Next bounded
question is whether observed D16..18 value gaps depend on value-associated
decimals: inspect actual result metadata before defining same-payload/changed-
scale interventions. Any new native caller must be independently read/frozen
and obtain NEW BOTH resource clearance; completed captures must not be replayed.

## Same-payload scale intervention — source foundation, native UNRUN

emit-scale-intervention.cjs, scale-intervention.prg/cases.json,
audit-scale-intervention.cjs and scale-intervention-capture.sh define a new
bounded discriminator, not a completed native replay or expected formatter.
Six retained signed parent inputs (indices193/199,205/211,217/223) come from
the hash-bound qualified round-inline archive. D16/17 each reconstruct original
and observed ROUND payload at scale18 versus scaleD, width40/bothFIXED; D18
already has scale18 on both routes, so only two unchanged-scale control routes
are emitted. Total20 routes/40 contexts/42 fields, applied D16..18.

Every target consumer receives INLINE CFNDEC(CFNWIDTH(targetPayload,40),scale),
never a locally assigned Numeric argument/result. Original N40/18 and actual
INLINE ROUND(original,D) are independently sampled BEFORE and AFTER target
observations, with exact bound-parent metadata/BRS/TRANSFORM/STR/F17/settings
controls. Target metadata/BRS/TRANSFORM/STR/F17/type are observed before/after.
Reconstructed ROUND payload's NEW display is deliberately NOT required to equal
the parent ROUND expression: metadata/payload equality alone is not presumed
sufficient. No new output strings, precision cap, tolerance or causal rule is
calculated. Reader tolerates only the inactive Numeric integer slot, preserving
all active metadata, literals, signed-zero spelling and exact repeat checks.

Source-only Node syntax/emitter regeneration and byte comparisons, independent
reader's12 raw-bound parent contexts, bash syntax and diff-check all return0.
check-scale-reader.cjs passes44 SYNTHETIC reader checks (valid first/paired
structure; rejection of metadata/BRS/settings/runtime/text/footer/status changes;
inactive Numeric integer-slot acceptance). Arbitrary synthetic target text is
reader-test input, NOT native behavior or compatibility expected strings.
All nine successful source-check stages have immediate prefix-idle1/full I/O
PSI avg10 AND avg60<1%/guard0, nice-n10; not VFP syntax/behavior qualification.

Earlier synthetic attempt1 failed because a purported changed original display
was already byte30. Diagnostic attempt2 explicitly identifies inert column10.
The test correction uses a different literal and asserts each mutation actually
changes its field; actual attempt3 passes. This was a test-stimulus error, not
a newly discovered product defect. Both failed results/source copies are kept,
not called native failures or retroqualified by the later pass.
scale-intervention-source-checks1 retains132 gzip receipts for all three
attempts, manifest/checksums and exact host-byte/gunzip comparisons. First two
source copies were made after execution; attempt3 snapshots precede checks.
Host build/default-transform-scale-source-check1/2/3 remain intact. No native
capture root, compiler/new FLL, Wine/VFP or VM invocation; existing SDK-linked
observers remain HOST-ONLY. Ordinary shell NOT REA Evidence or cleanup proof.

LOW DQ-DV001 procedure delta: replace a proposed probe with a fixed40-context
caller/independent42-field reader/no-overwrite native worker; keep NEW BOTH
resource clearance, immediate per-launch idle/full-PSI gates and strict-first
BEFORE repeat. Walkthrough checked all42 field offsets, parent/target route
separation, control bracketing, fail-before-repeat path and44 synthetic checks.
Developer-maintainer self-review only, not independent/system/release assurance.
Misuse risk is treating a reconstructed result as an already equivalent native
route, or treating source/synthetic checks as native evidence. Mitigations are
separate target observations, exact parent controls, retained failures and
explicit UNRUN state. Hazards/rollback/field-notification policy unchanged.

At the source checkpoint, native worker remained UNRUN. Audit released its separate job then reported
resource HOLD fullPSI19.78/3.90%; no native/compiler window is reserved here.
After NEW explicit audit AND DAW native-only CLEAR, require immediate prefix-idle
and BOTH fullPSI avg10/avg60<1% EACH launch. Reuse verified observer/no rebuild,
strict first40-row read before repeat, retain any failure without weakening
controls, then archive independent readback before interpreting outcomes.
No owner-input blocker or automation pause; conversion/display recovery and
the retained division/ListItem implementation ladder remain open.

## Same-payload scale pair — qualified40, bounded metadata effect

Actual clean signed/DCO source1a933a5beb097dcd0ad38b4d2adac57f24742b43.
NEW explicit audit AND DAW native-only CLEAR, both immediate prefix-idle1/
guard0/full I/O PSI avg10/avg60 0.00/0.00%, nice-n10, verified observer reuse/
unchanged own C sources/no compiler/new FLL/VM. Inherited override UNSET,
process-local WINEDLLOVERRIDES=msvcr71=n. First actual0/empty stderr/DONE40/
frozen strict-single0 BEFORE repeat; repeat0/empty/DONE40/paired0. All42 raw
fields repeat-identical, inactive-slot repeat differences0, raw SHA BOTH:
d6fef944a8f3fd2732703b7f8a81190a5743be78dcad758341cdd97702cb0fc2.
Original and actual ROUND parent controls, target N40/scale/BRS, before-after
settings/F17/CW027F/exact Microsoft32 path+digest+PE32 controls pass.
Once-per-FLL backing-file sample/later image-path checks NOT atomic mapped-code,
Windows execution/internal-call or descendant-cleanup proof. Ordinary shell
NOT REA Evidence. Native window RELEASED to both chats; post-prefix idle1.

scale-intervention-captures1 retains99 gzip receipts (63baseline+36supplemental),
manifest/SHA256SUMS and verification-manifest/VERIFICATION-SHA256SUMS, all
gunzip sizes/hashes/host bytes verified. Archived source equals frozen source;
archived strict reader0/empty stderr/report byte-exact, final input hashes0.
Intact hostbuild/default-transform-scale-intervention-captures1; workingroot
ABSENT. SDK-linked observer/metadata binaries remain HOST-ONLY, no redistribution.

analyze-scale-intervention.cjs was added AFTER capture. An initial offline
analysis1 incorrectly demanded raw before/after equality of the inactive Numeric
integer slot; it failed1 and was retained with its source/stderr/full shell
receipt. Correction normalizes ONLY that inactive slot while retaining type,
width, decimals, real payload and CW. Frozen native reader/qualification was
unchanged; no native replay or product issue. Separate analysis2/readback both0/
empty/byte-identical, eight exact-decimal metric checks pass/all40 raw projection
and control joins verified. Both immediate prefix/PSI gates pass, nice-n10.
Hostbuild/default-transform-scale-analysis1/2 retained; both archived above.

| Bounded comparison | Count | TRANSFORM literal gaps | STR literal gaps | Decimal-value gaps (either consumer) |
| --- | ---: | ---: | ---: | ---: |
| Same payload, scale18 versus scaleD at D16/17 | 16 | 4 | 0 | 0 |
| Original reconstruction scale18 versus parent original | 12 | 0 | 0 | 0 |
| ROUND-payload reconstruction scaleD versus actual parent ROUND | 12 | 0 | 0 | 0 |

The four scale-dependent TRANSFORM strings are ROUND-kind/FIXED OFF at
D16/17, both signs: only trailing zeros differ, exact value +/-1E-15 unchanged.
No zero-sign changes occur in those16 intervention pairs. D18 remains
unchanged-scale controls, not an independent metadata intervention.
Across all40 target TRANSFORM/STR pairs:40 raw literal gaps,19 after leading-space
removal,0 decimal-value gaps,10 zero-sign spelling gaps. Decimal coefficient/
exponent comparisons preserve these spelling/padding distinctions separately;
binary-payload-versus-decimal representation gaps are not called defects.

In THESE contexts, changing scale18 to16/17 does not explain the earlier
original-versus-ROUND decimal-value gap. Baseline route coincidence is not a
universal reconstruction/metadata-sufficiency/hidden-state-absence rule.
No formatter, precision cap/order/tolerance or expected-output model is accepted.
LOW DQ-DV001 actual walkthrough checked strict-first ordering, all42 offsets,
exact archived replay, inactive-slot correction and bounded comparison counts.
Developer self-review only; existing hazards/rollback/field-notification unchanged.
NEXT bounded offline question: inspect prior exact graph analyses before testing
true decimal half-grid quantization of retained payloads versus observed displays.
Keep any candidate cap/rounding arithmetic explicitly hypothesis-only, use exact
integer ratios rather than fitted tolerance, preserve counterexamples, and do
not repeat completed native captures. Exact conversion/display and retained
division/ListItem implementation ladder remain open; no owner-input blocker.

## True decimal half-grid hypotheses — all four REJECTED

test-decimal-half-grid.cjs is an AFTER-capture mathematical discriminator, NOT
a formatter/expected emitter. Inspection of classify-graph-rejections.cjs and
analyze-arithmetic-inline.cjs confirmed those comparisons use observed binary
bias/powers/stages, not true decimal half-grid quantization of the input.
Four explicit candidates are now compared with264 retained N40/18 FIXED ON
original inputs: q=D or q=min(D,15), each with no bias or exact decimal prebias
1/(2000*10^D). Candidate magnitude is floor((abs(input)+bias)*10^q+1/2)/10^q,
all operations using exact BigInt ratios. The cap15 and prebias originate in
REJECTED diagnostic stimuli, not recovered requirements, fitted parameters or
permission to implement them. No binary Number(displayText) or tolerance.

The source reuses the unchanged, archived-source-bound decomposition validator:
its actual fresh output equals the retained output byte-exactly, validating all
528 raw ROUND/arithmetic/STR controls (eight validator checks). Independent new
binary-ratio/decimal/half-grid metric checks14 pass, including adjacent half
payloads, subnormal/negative-zero, exact prebias ties, count18/cap15 distinction,
scientific decimal equality and nonfinite rejection. Prior graph classification
is manifest/hash bound and all264 indices/bits/settings/display joins match.
Public ROUND payloads are retained for traceability but NOT compared to decimal
text as a supposed conversion-defect metric. This is a magnitude-only model
comparison, not a literal/padding/signed-zero implementation.

| Candidate | Original TRANSFORM differences | Original STR differences | ROUND TRANSFORM differences | ROUND STR differences |
| --- | ---: | ---: | ---: | ---: |
| Unbiased, q=D | 150 | 150 | 36 | 36 |
| Unbiased, q=min(D,15) | 132 | 132 | 36 | 36 |
| Exact decimal prebias, q=D | 64 | 64 | 134 | 134 |
| Exact decimal prebias, q=min(D,15) | 34 | 34 | 122 | 122 |

Each column has264 comparisons; raw literals/zero spelling remain attached to
every row without predicting text. All four candidates are rejected. The exact
decimal-prebias/cap15 candidate fails six of the prior graph's18 failures and
introduces28 others (34 total), so merely substituting exact arithmetic does
not repair the graph. No global cap, internal call/order/precision or formatter
rule is accepted, even where one candidate happens to match a finite subset.

Actual analysis0/readback0/empty stderr/byte-identical, all14 new and8 reused
checks pass behind immediate prefix-idle1/full I/O PSI avg10 AND avg60<1%/guard0,
nice-n10. No compiler/Wine/VFP/VM/native invocation/window. Host root
build/default-transform-decimal-half-grid1 intact; decimal-half-grid-analysis1
retains20 gzip receipts/source/results/gates/full shell/input inventory/checks,
manifest/SHA256SUMS, all gunzip sizes/hashes/host bytes verified. Source-base and
dirty-status snapshots are explicitly POST-check, not a clean native freeze.

At the prior checkpoint, separate executable archived-source recomputation was NOT yet performed:
audit then reported HOLD17.16/3.29 and31.91/8.51, with local fullPSI52.89/32.41.
No replay was attempted and this is NOT worker guard75/native failure. NEXT
SAME slice: after fresh immediate prefix-idle/full avg10 AND avg60<1% gates,
verify archived source/input hashes, recompute to a new receipt file, compare
byte-exact and append supplemental verification receipts without overwriting
the20-file baseline. Then inspect qualified raw264 inputs for any actually
bracketed adjacent display transitions before selecting another discriminator;
do not infer new thresholds from fitted tolerance or use old-runtime captures
as retroqualified native evidence. No owner-input blocker/automation pause.

LOW DQ-DV001 delta/walkthrough: replace the proposed exact-half-grid question
with executed, explicitly rejected candidates; checked magnitude/sign handling,
exact prebias formula,264 joins/counts,14 metric checks and fresh pressure stop
before further execution. Misuse risk is treating a near-fit as an accepted
formatter; retain all counterexamples and no production hook. Developer self-
review only, not independent/system/release assurance. Hazards/rollback unchanged;
exact conversion/display and retained division/ListItem ladder remain open.

The deferred half-grid archive check is now COMPLETE: all20 baseline file
sizes/hashes/host bytes and archived-source equality verified; actual independent
offline recomputation0/empty stderr/byte-exact to both archived analysis/readback,
input hashes0 BEFORE and AFTER,14 new/eight reused metric checks. Immediate
prefix-idle1/full PSI0.00/0.00/guard0/nice-n10. Ten supplemental gzip receipts,
verification-manifest/VERIFICATION-SHA256SUMS bring decimal-half-grid-analysis1
to30 byte-compared gzip receipts without overwriting baseline. Hostroot1 intact.
Ordinary shell NOT REA Evidence or native replay. All four candidates remain
rejected; completion of archival verification does not accept any formatter.

## Current-capture adjacent transitions — inventory0, archive replay guarded

inventory-adjacent-transitions.cjs derives only actually adjacent observations
from the qualified CURRENT264 ORIGINAL N40/18 FIXED ON inputs, same applied D
and sign. It reuses the unchanged archived-source-bound raw validator (fresh528
controls/eight checks/output byte-exact), explicitly checks original N40/18/
real/CW descriptors and raw projection, and orders finite payloads by magnitude
using BigInt bits. Eight new rank/sign/adjacency/nonfinite checks pass. No old
runtime captures are used to qualify a new transition, and no uncaptured gap is
interpolated. Conflicting coincident payloads would remain unqualified rather
than being discarded. This is a retained-observation inventory, not a search
for a fitted threshold/tolerance or another native capture.

Actual inventory0/readback0/empty stderr/byte-identical, each behind fresh
prefix-idle1/fullPSI avg10 AND avg60<1%/guard0/nice-n10. The264 distinct payload
nodes form38 setting/sign groups,44 contiguous components and220 adjacent pairs;
six nonadjacent gaps are excluded. There are44 observed TRANSFORM decimal-value
changes and44 STR changes,0 consumer-boundary disagreements,0 coincident-payload
conflicts, and no component lacking an adjacent observed value change. Literal,
signed-zero and both endpoint bytes/indices remain attached to every pair.
One raw payload step is NOT evidence of global monotonicity/unique threshold,
metadata sufficiency, internal operation order or a production conversion rule.

adjacent-transition-inventory1 retains20 baseline gzip receipts/source/results/
gates/full shell/input hashes, manifest/SHA256SUMS, all gunzip sizes/hashes/host
bytes verified. Host build/default-transform-adjacent-inventory1 intact; source-
base/dirty snapshots explicitly POST-check, not a clean native execution freeze.
A separate archival replay then stopped at actual pressure guard75/fullPSI
60.39/45.28%/prefix-idle1 BEFORE child analyzer. No archive-recompute status/
stdout/stderr/report exists. Five supplemental forensic gzip receipts plus
verification-guard-manifest/GUARD-SHA256SUMS retain that guard;25 total gzip
receipts byte-compared, archivedRecomputeAnalyzerInvoked:false. This does not
invalidate the earlier actual inventory/readback, nor qualify the missing
archival replay. No native/compiler/Wine/VM invocation or reserved window.

NEXT SAME slice after fresh offline idle-prefix/fullPSI avg10 AND avg60<1%
gates: use NEW receipt names/root to verify archived source and inputs, execute
byte-equivalent source, compare exact to baseline inventory/readback, append
verification receipts without overwriting guard/baseline. Then join the rejected
exact-half-grid candidates to these observed adjacent endpoints, keeping all
counterexamples and no fitted/interpolated thresholds. Any new native work
requires NEW BOTH resource clearance. No owner-input blocker/automation pause.
LOW DQ-DV001 walkthrough checked same-context grouping, all264 input accounting,
finite bit-rank assertions, exclusions and pre-child guard ordering; actual
eight new/eight reused checks and initial readback support this source delta.
Developer self-review only, not independent/system/release assurance. Existing
hazards/rollback unchanged; exact conversion/display/division/ListItem remain open.

## 2026-10-19 adjacent archive replay and rejected-candidate endpoint join

The preceding NEXT archival replay is now completed, not retroactively qualified:
NEW archive-recompute2 names preserve the original pre-child guard75 untouched.
Baseline20/prior guard5 manifests, checksum inventories and gunzip size/hash/host
bytes pass before execution; archived source equals executed current source.
Fresh prefixidle1/fullPSI avg10/avg60=0.14/0.21%, guard0/nice-n10; actual
recompute0/empty stderr/byte-exact to BOTH archived inventory/readback, input
hashes before+after0. Ten supplemental gzip receipts plus verification-manifest/
VERIFICATION-SHA256SUMS give35 inventory receipts total; all bytes verified.
Host build/default-transform-adjacent-inventory1 remains intact. No native launch.

Derived same-slice AFTER-capture question: do the four rejected candidates change
on the actual observed adjacent endpoint pairs, rather than merely producing a
similar aggregate count? join-half-grid-adjacency.cjs binds/replays BOTH archived
candidate and inventory sources/results, including the raw validator, checks
source digests and all264 unique endpoint index/D/bits/public-ROUND/TF/STR joins,
and retains all220 adjacent pairs per candidate. Four category truth-table checks
plus replayed14 candidate/eight inventory/eight shared raw-validator checks pass.
No new model, interpolation, expectation or formatter is introduced.

| Already rejected candidate | Endpoint magnitude gaps | Coincident changes only | Missed observed changes | Invented changes on observed plateaus | Shared plateaus |
| --- | ---: | ---: | ---: | ---: | ---: |
| unbiased_at_D | 150 | 0 | 44 | 0 | 176 |
| unbiased_cap15 | 132 | 0 | 44 | 0 | 176 |
| decimal_prebias_at_D | 64 | 14 | 30 | 24 | 152 |
| decimal_prebias_cap15 | 34 | 14 | 30 | 30 | 146 |

All candidates compare220 pairs, including44 observed native TF/STR value
changes. The cap15+prebias candidate's44 predicted changes match only14 of those
observed pairs: equal total counts are not boundary or endpoint agreement.
Coincident change alone is not correct magnitude; both endpoint magnitudes match
on76/88/138/156 pairs respectively. Six uncaptured gaps remain excluded; no
global monotonicity/unique threshold claim. Literal padding/zero signs remain
observations, not predictions. These mathematical candidates stay REJECTED.

Actual analysis0/readback0/empty/byte-identical, each immediate prefixidle1/
fullPSI0.09/0.21%/guard0/nice-n10, input hashes before+after each0. Baseline23
gzip receipts in half-grid-adjacency-analysis1 include BEFORE source snapshot,
input inventory, gates, reports and full actual shell result. Source-base/dirty
snapshots are explicitly POST-check (basefb86f6f86), not native clean freezes.
Separate archive-source recomputation0/empty/byte-exact to both reports, fresh
prefixidle1/fullPSI0.00/0.13%/guard0/nice-n10/input hashes before+after0. Ten
supplemental receipts/verification-manifest/VERIFICATION-SHA256SUMS yield33 gzip
receipts; all compressed hashes/gunzip sizes/hashes/host bytes verified. Host
build/default-transform-half-grid-adjacency1 remains intact. Ordinary shell,
NOT REA Evidence/native/Windows/internal-call/atomic-image/cleanup proof.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 delta/walkthrough: checked four
truth-table outcomes, same-context exhaustive joins, raw adjacency, all rejected
endpoints and excluded gaps. Misuse would promote equal aggregate counts to an
accepted formatter; the exact pair ledger and retained rejections prevent that
interpretation. Developer-maintainer self-review plus the actual automated
checks above, not independent/system/release assurance. Existing
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 and rollback unchanged.

NEXT bounded same-slice question: join these observed endpoint changes/plateaus
to retained public ROUND and diagnostic arithmetic-stage payloads to identify
which observable stages collapse distinct display endpoints. This is an offline
discriminator, not an inference of VFP's internal call/operation order. Fresh
idle-prefix/fullPSI avg10 AND avg60<1%/nice-n10 each offline launch; NEW BOTH
resource clearance before any native/compiler work. Exact display conversion,
native-only expectations, unchanged division ORIGINAL/SAME and full acceptance
ladder remain OPEN; no production formatter/helper/hook or compatibility closure.

## 2026-10-19 observable-stage / adjacent endpoint join

Completed the preceding bounded NEXT discriminator with
join-observable-stage-adjacency.cjs. This replays/binds archived inventory
source/results (including the raw ROUND validator) and classification pass2
source/results (including EVERY arithmetic-stage raw field in BOTH captures).
Source digests and all264 endpoint index/D/bits/public-ROUND/TF/STR joins checked.
The eight diagnostic expressions are independent inline evaluations of a
REJECTED graph, not observed formatter internals or a recovered operation order.
All220 actually adjacent pairs and264 endpoints retained, six uncaptured gaps
excluded. D/sign held equal; original metadata N40/18 validated by the inventory.

Payload equality and full ACTIVE tuple equality are distinct metrics, including
type/width/active scale/value/CW. Inactive union slots are excluded ONLY by the
already raw-validated type rules; signed-zero payloads are not normalized away.
Nine new metric checks cover key order, transition/plateau classification,
same-payload/different-width distinction, signed zero, N/I distinction and
nonfinite rejection. Replayed inventory8/classification9/shared raw validator8
checks pass; 528 raw/control contexts joined,264 ON endpoints/220 pairs.

| Observable expression/consumer | Equal active tuple across44 observed changes | Different tuple across44 changes | Equal tuple on176 observed plateaus | Different tuple on176 plateaus |
| --- | ---: | ---: | ---: | ---: |
| shifted | 0 | 44 | 2 | 174 |
| scaled | 2 | 42 | 12 | 164 |
| raised | 12 | 32 | 92 | 84 |
| quantized | 12 | 32 | 166 | 10 |
| public inline ROUND | 44 | 0 | 176 | 0 |
| public stored ROUND | 44 | 0 | 176 | 0 |

The payload-equal counts across changes equal the tuple-equal counts in this
sample. Inline and stored ROUND are EACH constant across all220 pairs; that is
not a claim that their widths equal each other. D-only powDown/bias/effectiveCount/
powUp controls are constant across all220 pairs by construction, not a discovery
about the formatter. Equal ROUND tuples on44 display transitions are explicit
counterexamples to ROUND tuple sufficiency for original display in this sample.

The two scaled-collapse witnesses are D13 pairs159->160 and165->166 (signs),
bits3D2C1E8DBBA38061->3D2C1E8DBBA38062 and corresponding BD payloads. The
shifted tuples differ but scaled tuples coincide; twelve raised/quantized
collapse pairs are111->112,117->118,123->124,129->130,135->136,141->142,
147->148,153->154,159->160,165->166,171->172,177->178. These observations
do not prove which operations VFP uses internally, or that changing precision
would establish parity. No candidate accepted, interpolation/fitted threshold,
expected emitter or production formatter.

Actual analysis0/readback0/empty/byte-identical and input hashes before+after
EACH0. BEFORE source snapshot; basea42cbbf31+dirty snapshots explicitly POST-check,
not a native clean freeze. Each immediate prefixidle1/fullPSI avg10/avg60
0.00/0.00%/guard0/nice-n10. Hostbuild/default-transform-stage-adjacency1 intact.
stage-adjacency-analysis1 retains23 baseline gzip receipts/source/reports/gates/
inputs/full actual shell result; baseline manifest/compressed hashes/gunzip
sizes/hashes/host bytes checked. Separate byte-equivalent archived-source replay0/
empty/byte-exact to BOTH reports/input hashes before+after0, NEW immediate
prefixidle1/fullPSI0.00/0.00%/guard0/nice-n10. Ten supplemental receipts and
verification-manifest/VERIFICATION-SHA256SUMS ->33 verified gzip receipts.
Ordinary shell NOT REA Evidence/native/Windows/internal-call/atomic-image or
cleanup proof. No compiler/native/Wine/VM invocation/reservation.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 delta/walkthrough: examined the two
D13 witness endpoints, separate payload/metadata metrics, all220 pair accounting,
independent-expression warning, D-only controls and nine synthetic truth checks.
Misuse risk: treating a diagnostic-stage collision as the actual formatter
sequence or an accepted extra-precision repair. Retained raw tuples/counterexamples
and explicit sufficiency-only interpretation mitigate this. Actual developer-
maintainer self-review/automated checks, not independent/system/release assurance.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01/rollback unchanged.

NEXT bounded discriminator: exact-rational comparison of already observed
shifted/powUp and scaled/+half operands for these collapsed endpoint pairs,
retaining their distinguishability relative to the observed quantization
boundary. This is offline analysis of measured operands only, not a fitted
conversion rule or proposed internal precision. Fresh idle prefix/fullPSI avg10
AND avg60<1%/nice-n10 EACH offline launch; NEW BOTH explicit resource CLEAR before
any compiler/native work. Full recovery/expectation/ORIGINAL/SAME/hosted/review
ladder remains OPEN, including uncancelled division/ListItem prerequisites.

## 2026-10-19 exact measured-operand discriminator

Completed the preceding NEXT with compare-collapsed-stage-operands.cjs. It
binds/replays the archived stage-join source/results, unchanged raw validators,
and264 original input tuples to the native-frozen arithmetic cases (gzip digest
in the original SHA256SUMS, source bytes exact). Diagnostic expressions explicitly
name independent scaled/raised/INT evaluations;0.5, bias and cap remain REJECTED
graph stimuli, not requirements. All264 nodes/220 pairs/176 plateaus retained,
six uncaptured gaps excluded. Exact reduced BigInt fractions preserve the
measured product shifted*powUp and sum scaled+1/2 without binary64 re-rounding.
Observed nonnegative integer intervals [quantized,quantized+1) are retained per
endpoint; comparisons use shared intervals only when observed integers coincide.
Neither the interval nor a coincident integer decision predicts original display.

First analysis1 FAILED1 before readback: own analyzer incorrectly assumed all
observed quantized integers were0/1. Retained data also includes100/101/267/268/
999/1000; native evidence/reader unchanged. BEFORE failing source/inputs, actual
empty stdout/stderr/status/gates/full shell result retained in host
build/default-transform-collapsed-operands1 and11 pass1 gzip receipts. No after
hash check/readback is claimed. This is an AFTER-capture analyzer defect, not a
product issue/native failure. Source corrected to arbitrary nonnegative observed
integer intervals, including a non-unit-boundary metric check. Separate host
build/default-transform-collapsed-operands2 preserves corrected execution.

Corrected pass2:14 new metric checks cover exact signed/subnormal conversion,
fraction reduction/zero, addition/multiplication/boundaries/decisions/nonfinite
rejection. Replayed stage9/inventory8/classification9/shared raw8 checks pass.
Fresh528 raw/control joins, all input tuples and both native arithmetic field
captures unchanged. Analysis0/readback0/empty/byte-identical, input hashes
before+after EACH0. BEFORE pass2 source snapshot; base7dce8dfc7+dirty snapshots
explicitly POST-check, not a clean native freeze.

Finite measured-operand results (integer decisions belong ONLY to the rejected
diagnostic graph; changes alone are not correct original decimal magnitudes):

| Pair set | Pairs | Distinct exact measured products | Product+half decisions differ | Exact measured-scaled+half decisions differ | Measured raised/INT decisions differ |
| --- | ---: | ---: | ---: | ---: | ---: |
| Original display changes | 44 | 44 | 16 | 18 | 32 |
| Original display plateaus | 176 | 174 | 28 | 26 | 10 |
| Scaled tuple collapses across display changes | 2 | 2 | 0 | 0 | 0 |
| Raised/quantized tuple collapses across display changes | 12 | 12 | 10 | 6 | 0 |

Among twelve raised/quantized collapse pairs (observed integer1), ten exact
products straddle the diagnostic half boundary; two D13 products are BOTH above
it, despite distinct exact values and an original-display transition. Exact
scaled+half falls less/equal on six pairs, equal/greater on four, greater/greater
on two; measured raised is equal/equal at1 on all twelve. The two scaled-collapse
witnesses remain159->160/165->166, both D13/signs. Thus a product-precision-only
or add-half-only substitution is not established by these observations; across
all44 changes the exact product decision misses28 and changes on28 plateaus.
No fitted boundary/tolerance/cap or internal operation/precision attribution.

collapsed-operands-analysis1 retains34 baseline gzip receipts (11 failed pass1,
23 corrected pass2), manifest/SHA256SUMS; all gunzip sizes/hashes/host bytes
verified. Separate archived pass2 source equals executed source; recomputation0/
empty/exact to BOTH pass2 reports/input hashes BEFORE+AFTER0. Ten supplemental
gzip receipts/verification-manifest/VERIFICATION-SHA256SUMS ->44 total verified.
Each launch (including failed pass1) fresh prefixidle1/fullPSI avg10 AND avg60
0.00/0.00%/guard0/nice-n10. Ordinary shell NOT REA Evidence/native/Windows/
internal-call/atomic-image/cleanup proof; no native/compiler/Wine/VM reservation.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 delta/walkthrough: checked explicit
non-unit integer intervals and fourteen metric cases, retained failed assumption,
all pair accounting and the signed D13 above-boundary counterexamples. Misuse:
promoting preserved exact distinctions to an accepted precision repair. Mitigation:
keep incorrect/coincident decisions and plateau counterexamples alongside every
exact fraction; no production code/expectation changed. Actual developer-
maintainer self-review/automated checks, not independent/system/release assurance.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01/rollback unchanged.

NEXT bounded native-evidence preparation: signed D13 original-versus-observed-
shifted-payload consumer discriminator, using the two witness pairs and exact
original/public-ROUND before+after controls. Reconstructed shifted payloads are
a NEW route observation, never assumed equivalent to the independently evaluated
expression or original display. Freeze source/contract/strict reader and focused
synthetic checks BEFORE execution. Reuse the verified observer, no new FLL/VM;
NEW explicit BOTH audit+DAW native-only CLEAR and immediate idle-prefix/fullPSI
avg10 AND avg60<1%/nice-n10 EACH native launch. Exact formatter recovery and
expectations/unchanged division ORIGINAL-SAME/hosted/review ladder remain OPEN.

## 2026-10-19 signed D13 shifted-consumer source preparation — checks pending

Derived bounded contract under RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001 and the
preceding NEXT: parent159/160/165/166 atD13, original/shifted target kinds,
both FIXED ->8 routes/16 records/42 fields. Files emit-shifted-consumer.cjs,
shifted-consumer.prg, shifted-consumer-cases.json, audit-shifted-consumer.cjs,
check-shifted-reader.cjs, shifted-consumer-capture.sh. No production change.

Original bits3D2C1E8DBBA38061/62 and BD counterparts; measured positive ABS-shift
bits3D2C25C268497683/84. Sign-copy from original is a fixture-derived NEW route,
not observed signed expression arithmetic. Native measured shift metadata was
N32/18; target deliberately N40/18 for every TRANSFORM/STR/CFNMETA/BRS/F17
consumer. No local assignment of the wrapped target/ROUND result. Reconstructed
route/metadata normalization are NOT assumed equivalent to the original inline
expression or display, and no expected new target strings are emitted.

Caller independently brackets targets with original N40/18 and actual inline
ROUND(original,D) controls before+after; settings/type/BRS/CW/exact Microsoft32
binding checked by the strict reader. Emitter replays archived stage/raw checks
to derive stimuli. Reader uses a separate literal witness table and binds BOTH
retained arithmetic runs/raw shift fields36..40 and exact original/public-ROUND
parent controls; no emitter import/target formatter. Planned synthetic reader
tests exercise arbitrary valid target text, rejected mutations, runtime/status/
footer and inactive Numeric-integer-slot exclusions, not compatibility strings.

Guarded worker reuses the verified observer; no new FLL/compiler/VM. Requires
clean source and NEW explicit BOTH native-only clearance; no overwrite of an
existing attempt root. Native-only process override/inherited override guard,
observer/source/input hashes, first native/strict single reader BEFORE repeat,
paired reader/all42 repeat-field audit remain required. Shell pipefail protects
the combined prior/nested raw-input inventory. None of this worker is executed.

Actual source/cases bootstrap0/empty, immediate prefixidle1/fullPSI avg10/avg60
0.00/0.00%/guard0/nice-n10 each; outputs equal current draft byte-for-byte.
The subsequent ten-stage source-check pipeline ACTUALLY stopped guard75 before
its FIRST syntax child at fullPSI63.42/33.67%/prefixidle1. There is NO syntax
status/stdout/stderr, reader run, synthetic-test count, PRG compilation, native
run or clean-native execution freeze to qualify. Do not describe the draft as
tested merely because its emitter completed. Input hashes BEFORE guard passed;
no after-check pipeline result claimed.

Hostbuild/default-transform-shifted-source-checks1 and archive
shifted-consumer-source-checks1 retain30 gzip files: BEFORE source snapshots,
bootstrap outputs/status/gates/full actual shell, input inventory, pre-child
guard/full shell, POST-guard base9e90410f1/dirty snapshots. Manifest/SHA256SUMS,
all gunzip sizes/hashes/host bytes checked; current draft equals snapshots.
Post-guard state is NOT a native freeze. Ordinary shell NOT REA Evidence/native/
Windows/internal-call/atomic-image/cleanup proof. No resource reservation.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 procedural delta/manual walkthrough:
reviewed8-route/16-record construction, signed stimuli, deliberate N32->N40
boundary, original/ROUND bracketing and fail-before-repeat ordering. Misuse risk
is promoting sign-copy/reconstruction or unrun reader tests to parity evidence;
separate source-only/guard metadata and no expected target strings prevent that
claim. Actual developer self-review plus bootstrap/byte-preservation only; planned
automated reader validation remains OPEN, not development acceptance/independent/
system/release assurance. HZ-data-corruption-01/HZ-runtime-crash-01/
HZ-doc-command-01/rollback unchanged. No parent/division/ListItem closure.

NEXT SAME source unit: after fresh immediate idle-prefix/fullPSI avg10 AND
avg60<1%, use NEW build/default-transform-shifted-source-checks2 and receipt
names; preserve checks1. Verify archived draft/input hashes, then run all ten
source stages (JS syntax3, shell syntax1, regeneration2, byte comparisons2,
source-only reader1, synthetic reader1) at nice-n10 with fresh gates. Source
freeze/sign and full reader evidence precede any native execution; request NEW
explicit BOTH native-only resource CLEAR and apply all immediate native gates.
No owner-input blocker/automation pause. Exact conversion recovery remains open.

## 2026-10-19 D13 source validation — complete, native pair still pending

Continued the SAME source unit after the preserved checks1 pre-child I/O guard;
no completed native capture was restarted. Verified all30 prior compressed
receipts/gunzip sizes/hashes/host bytes and exact six draft-source snapshots
before NEW hostbuild/default-transform-shifted-source-checks2 execution. Source
baseb6c59e169 and status were captured BEFORE checks (clean). First nine stages0:
JS syntax3/shell syntax1/regeneration2/byte comparisons2/source-only reader1.
The reader independently checked8 original/ROUND parent contexts and8 measured
shift contexts,16 rows/42 fields, newResultsRead:false/nativeInvoked:false.

Synthetic stage FAILED1: its column14 mutation supplied zero, but the first
parent ROUND control was already zero. The test's explicit non-inert assertion
stopped before that mutation was submitted to the reader. Full original source,
input inventory/gates/status/stdout/stderr/actual shell results are preserved;
no full synthetic-test pass or post-failure input hash is claimed for checks2.
This is a synthetic-fixture defect, not native/strict-reader/product failure.

Only check-shifted-reader.cjs changed: column14 now mutates to binary64 one
(3FF0000000000000), still explicitly asserted different from the baseline.
Caller, cases, strict native reader and worker remained byte-identical. Separate
hostbuild/default-transform-shifted-source-checks3 snapshots precede execution;
baseb6c59e169+dirty checker-only delta recorded, NOT clean native execution freeze.
All ten stages0/empty stderr, input hashes BEFORE+AFTER EACH0. Source-only reader
controls8+8 pass; syntheticReaderSelfTests44 passes with arbitrary target text,
rejected metadata/payload/settings/runtime/status/footer/repeat mutations, and
only inactive Numeric integer-slot repeat differences accepted. These44 tests
are structural/control validation, NOT native compatibility expected strings.

shifted-consumer-source-checks2 archive retains199 baseline gzip receipts
(99 failed checks2+100 corrected checks3), manifest/SHA256SUMS. Separate byte-
equivalent archived-source replay verifies regeneration2/source-reader/synthetic
reader: four actual stages0/empty/byte-exact to retained outputs,44 tests again,
input hashes BEFORE+AFTER EACH0. Thirty-seven supplemental gzip receipts plus
verification-manifest/VERIFICATION-SHA256SUMS ->236 total; all compressed hashes,
gunzip sizes/hashes/exact host bytes checked. Prior30-file guard archive untouched.
All24 major source/replay invocations had immediate prefixidle1/guard0/fullPSI
avg10 AND avg60<1% (maxima0.18/0.03%, separate maxima, not one sample), nice-n10.
No VFP/PRG compiler/native/Wine/VM invocation or resource reservation. Ordinary
shell NOT REA Evidence/Windows/internal-call/atomic-image/cleanup proof.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 delta/walkthrough now includes actual
ten-stage and44 mutation evidence plus archive replay. Examined the failed inert
mutation/correction, original control preservation, strict-single-before-repeat
ordering, deliberate sign-copy/N32->N40 boundary and synthetic-only text. Misuse:
promoting source/reader checks to native result equivalence. The unrun native
state and absence of new expected strings remain explicit. Actual developer-
maintainer self-review/automated development evidence, not independent/system/
release assurance. Existing HZ-data-corruption-01/HZ-runtime-crash-01/
HZ-doc-command-01/rollback unchanged; no parent/division/ListItem closure.

NEXT source freeze/sign and NEW explicit BOTH audit+DAW native-only CLEAR for
shifted-consumer-capture.sh --fresh-both-clear. Source checks are complete, but
the16-row native pair is UNRUN. Reuse exact observer, no new compiler/FLL/VM;
clean branch, absent capture root, observer/input/native-only-runtime guards and
idle-prefix/fullPSI avg10 AND avg60<1% EACH native launch. First native0/strict
single0 BEFORE repeat, then paired/all42 raw-field validation; preserve all
guards/failures. Any target equivalence is an observation to recover, not an
assumption. Full exact-conversion/expectations/ORIGINAL/SAME/hosted/review ladder
remains OPEN. No owner-input blocker or automation pause.

## 2026-10-19 signed D13 shifted-consumer native pair — QUALIFIED16

Completed the preceding NEXT from actual clean signed/DCO source
1b248f7a474e92fe24c13392fd6c95fe0b154828 after NEW explicit audit AND DAW
native-only CLEAR. The clearance snapshots/turn identities are retained, not
inferred from prior RELEASE messages. No host compiler/new FLL/VM; reused exact
observer0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac.
Inherited override UNSET; process-local WINEDLLOVERRIDES=msvcr71=n. Both immediate
prefixidle1/fullPSI avg10 AND avg60=0.00/0.00%/guard0/nice-n10. Both windows now
RELEASED; post-prefix idle1, no standing native/compiler/Wine/VM reservation.

Actual first native0/empty/DONE16/strict-single0 BEFORE repeat; repeat0/empty/
DONE16/paired reader0. All42 raw fields repeat-identical/16 records, inactive
slot differences0; exact original/actual inline-ROUND before+after/settings/
N40/18 target/BRS/F17/CW027F/Microsoft32 controls pass. BOTH raw digest:
764bc67a4feedce531033ac8b1fa52adcca9bd5844af00c4d18575f0140667e5.
Once-per-FLL backing-file sample/later image-path checks remain NOT atomic
mapped-code, Windows execution, internal-call or descendant-cleanup proof.
Ordinary-shell fallback is NOT REA Evidence or cleanup attestation.

Completed hostbuild/default-transform-shifted-consumer-captures1 retains exact
native/source/status/input/clearance/release receipts; working root
build/default-transform-shifted-consumer is ABSENT after validated move. Observer
and proprietary SDK-linked FLLs stay HOST-ONLY; no FLL included in the archive.
shifted-consumer-captures1 initially65 gzip receipts, manifest/SHA256SUMS; every
compressed hash/gunzip size/hash/host byte verified. Native qualification is
independent of the later AFTER-capture analyzer source.

analyze-shifted-consumer.cjs binds the qualified source/raw digest, projects all16
raw42-field records, normalizes ONLY inactive Numeric integer slots for active
metadata comparison, and compares exact decimal text separately from padding/
signed-zero spelling. Eight decimal metric checks pass. Archived FROZEN strict
reader replay0/empty/report byte-exact; analysis0/readback0/empty/byte-identical,
input hashes before+after EACH0, immediate prefixidle1/fullPSI both windows<1%/
nice-n10. Across two native+three offline launches maxima0.14/0.03% (separate
maxima, not one sample). BEFORE analyzer snapshot/base1b248f7a4+dirty POST-native
analysis snapshots are NOT native source freezes.

Finite observations (no reconstructed route equivalence was presumed):

- Original target reconstruction: all8 TF AND STR strings exactly equal their
  original parent controls, including both FIXED modes/signs in this sample.
- Original->shifted N40/18 payload interventions:8 comparisons; TF changes2
  decimal values (lower positive/negative endpoints, FIXED ON), STR changes4
  (those lower endpoints, both FIXED). All other respective strings unchanged.
- Signed adjacent original target pairs: TF changes2/2 ON,0/2 OFF; STR changes2/2
  in BOTH FIXED modes. Shifted adjacent target pairs change0/2 for either consumer
  or FIXED mode. The shifted route removes these observed transitions, not a
  recovered original-display algorithm or accepted precision/bias repair.
- Across16 target TF/STR comparisons:16 literal gaps,9 trimmed-literal gaps,
  8 exact decimal-value gaps (all OFF),1 both-zero sign-spelling gap. Retained
  signs/literals are not collapsed into the magnitude metric.

Hostbuild/default-transform-shifted-analysis1 intact. Thirty-three supplemental
gzip source/analysis/reader/readback/input/gate/full-shell receipts, verification-
manifest/VERIFICATION-SHA256SUMS ->98 total native+verification gzip receipts;
both inventories and all gunzip sizes/hashes/host bytes checked. No native rerun
for analysis. Original30-file source pressure guard and236-file source-test
failure/correction/qualification archive remain intact and historically scoped.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 walkthrough: checked first-strict-
before-repeat ordering, exact source/runtime controls, all16 raw projections,
eight original baselines and separate value/sign/literal comparisons. Misuse
risk: treating shifted-route success or N40 normalization as original expression
or universal conversion equivalence. Native-only controls and explicit finite
counterexamples/metadata boundary retain that gap. Actual developer-maintainer
self-review plus native/reader/metric/archive checks, not independent/system/
release assurance. Existing HZ-data-corruption-01/HZ-runtime-crash-01/
HZ-doc-command-01/rollback unchanged; no parent/division/ListItem closure.

NEXT bounded prerequisite: prepare same signed shifted payloads at their native
measured width32 versus controlled width40, both scale18/D13/FIXED, with original/
public-ROUND before+after controls. This tests the explicit N32->N40 normalization
boundary, not a guessed formatter/tolerance/internal sequence. Source/contract/
independent strict reader/synthetic checks/freeze BEFORE any new native capture;
NEW BOTH resource clearance and all immediate gates. Do NOT replay the completed
16-row original/shifted pair. Exact conversion/expectations/unchanged division
ORIGINAL-SAME/hosted/exact-head review ladder remains OPEN; no owner-input blocker.

## D13 shifted width32/width40 source qualification and deferred archive replay

Under the same admitted Numeric prerequisite, the next bounded discriminator
holds four signed shifted payloads, scale18/D13 and both FIXED modes constant,
varying only CFNWIDTH 32 versus40. Source/contract: emit-shifted-width-consumer.cjs,
shifted-width-consumer.prg and shifted-width-consumer-cases.json; independent
audit-shifted-width-consumer.cjs; check-shifted-width-reader.cjs; native worker
shifted-width-consumer-capture.sh and offline shifted-width-source-check.sh.
Eight routes/16 records/42 fields. Width is explicit in the contract/array,
independently checked via literal witnesses and both before/after target metadata.
Original N40/18 and actual inline public ROUND controls remain before/after;
parent raw8 and measured positive ABS-shift raw8 controls are independently read.
Sign-copy/reconstruction remain NEW routes, not signed-expression equivalence.
No target display prediction, fitted bias/tolerance or production change.

Host build/default-transform-shifted-width-generate1: two generation stages0.
Source files were applied from those generated bytes. Host source-checks1 (same
prefix) all11 stages0/empty stderr/input hashes BEFORE+AFTER EACH0, initial46
synthetic tests pass. Expanded source-checks2 all11 stages0/empty/hashes0 and48
tests pass: both wrong-width directions (32->40 and40->32), independently in
before/after metadata, now reject. Contract widths[32,40] also checked. This is
strengthened test coverage, not a native/product failure; initial source/result
snapshots remain retained. Each of24 stage launches has immediate prefixidle1,
fullPSI avg10/avg60<1%, nice-n10. All were source-only, not native launches.
BEFORE source snapshots/base96cd0a4d2+dirty additions are NOT clean nativefreeze.

Archive shifted-width-source-checks1 has251 primary gzip receipts/manifests/
SHA256SUMS. Includes both successful source-check passes, generation, BEFORE
sources and actual completion receipts. First packaging invocation failed during
Node parsing because of shell quoting, BEFORE producer execution; exact failure
and script retained. Corrected quoting packaged successfully; no source/native
change or rerun was needed. All compressed hashes/gunzip sizes/hashes/exact
hostbytes verified, separately from pending archived-source recomputation.

Host build/default-transform-shifted-width-archive-replay1: first immediate gate
prefixidle1/fullPSI3.80/0.71%, guard75 BEFORE archived-emitter-source child. No
child .status/.stdout/.stderr or post-child hashes claimed. All four archived
source replays and byte comparisons remain UNRUN. Later direct observation
45.93/12.12% confirms HOLD, not standing permission or a native/reader failure.
Ten supplemental forensic receipts/guard-manifest/GUARD-SHA256SUMS ->261 total,
all compressed/gunzip/host bytes verified. No native worker root, reservation,
clearance request, compiler/new FLL or VM. Small forensic packaging uses
ionice-c3/nice-n10; ordinary shell is NOT REA Evidence or cleanup attestation.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 delta/walkthrough: checked explicit
width32/40 arrays and inline consumer paths against independent reader/table,
both-direction metadata mutation rejection, unchanged parent controls, resource
guard and absence of child status after stop. Misuse risk: mistaking synthetic
target strings or source qualification for native observations/parity. Mitigation:
no compatibility strings, no target equality assumed; archived replay, clean
freeze and new paired native qualification remain required. Developer-maintainer
self-review only, not independent/system/release assurance. Existing hazards,
rollback, uncancelled division/ListItem and expectation/ORIGINAL-SAME ladder stay.

NEXT SAME: after fresh immediate prefix/fullPSI avg10 AND avg60<1% gates, replay
the FOUR archived expanded sources into a NEW host replay2 root; compare all four
reports byte-exact, retain input hashes BEFORE+AFTER EACH and guard1 history.
Then freeze clean signed source and request NEW BOTH audit+DAW native-only CLEAR;
each native launch needs its own immediate gates/nice-n10. Do not replay the
completed original/shifted16 pair, run the new width route only. No owner-input
blocker or automation pause; pressure is an execution gate, not product steering.

## Width discriminator archived-source replay2 completed

NEW build/default-transform-shifted-width-archive-replay2, from clean signed/DCO
04645b1c90f711f914484b62194e2a6f53adab3e, runs the SAME four archived expanded
sources after resolved I/O pressure. Archived emitter source/cases, strict reader
source-only and synthetic checker all0/empty stderr; all four reports byte-exact,
parent8+shifted8 controls and48 synthetic checks pass again. Inputs hashes BEFORE
and AFTER EACH0. Every immediate prefixidle1/guard0/nice-n10/fullPSI avg10/avg60
<1%; maximum avg10=0.00, avg60=0.15%. BEFORE clean source/status snapshots retained.
No native invocation or native-compatibility target string is inferred.

Forty-one supplemental source/gate/hash/report/actual full-shell receipts plus
verification-manifest/VERIFICATION-SHA256SUMS bring shifted-width-source-checks1
to302 gzip receipts. All three inventories, compressed hashes/gunzip sizes/hashes/
exact hostbytes verified. Prior guard1/fullPSI3.80/0.71 and packaging parse failure
remain immutable; source/reader/caller/native worker unchanged. LOW DQ-DV001
walkthrough checks SAME archived inputs and byte comparisons, rather than treating
prior stop as a pass. Existing hazards/rollback/non-REA limitations unchanged.

NEXT: clean signed source freeze and NEW BOTH native-only clearance; absent
build/default-transform-shifted-width-consumer root, exact reused observer,
first0/strict-single0 BEFORE repeat/paired0, each immediate idle/PSI gate/nice10.
No compiler/new FLL/VM; no repeat of completed original/shifted16 pair.

## Width32/40 native pair qualified; offline readback held by fresh pressure

Clean signed/DCO/G native source freeze:
0ec07074a6d6140bc25a876bf7f940bd426c5f86. NEW explicit BOTH audit+DAW native-only
CLEAR received and actual source/request/clearance receipts retained. Fresh
pre-request fullPSI0.00/0.03%, prefixidle. Run the frozen worker ONCE: first
native0, strict-single0 BEFORE repeat, repeat0, paired reader0, all stderrs empty.
Sixteen records/all42 fields repeat-identical; inactive-slot differences0. Both
raw SHA b3f9e1bb7b5aa25bd61edbcaa733e779635422b9cb055e432be470fd21b9a139.
Each immediate native prefixidle1/guard0/fullPSI0.00/0.01%/nice-n10. Existing
observer reused, process-local Microsoft32 override/inheritedUNSET; no host
compiler/new FLL/VM. BOTH windows promptly RELEASED; post-worker prefixidle.

Validated/moved completed host root to build/default-transform-shifted-width-
consumer-captures1; original working root ABSENT. Native/source/runtime/control/
status/input/actual shell/clearance/release receipts and own compiled FXP retained.
No FLL/proprietary SDK binary included. Archive shifted-width-consumer-captures1
has63 native gzip receipts/manifest/SHA256SUMS, compressed hashes/gunzip sizes/
hashes/exact hostbytes verified. Identity/binding still NOT atomic mapped-code,
Windows execution, formatter internal-call or descendant-cleanup proof; ordinary
shell is NOT REA Evidence or cleanup attestation.

Direct inspection of the qualified native audit shows identical target TF and
STR Character bytes for all8 width32/40 comparisons; both widths are actually
observed and independently checked, not inferred from strings. This is finite
source-qualified native observation, not accepted universal width-independence,
original expression equivalence, precision repair or formatter rule. Formal
archived-reader replay/exact-decimal decomposition is still pending below.

AFTER capture, add analyze-shifted-width-consumer.cjs: binds exact source/raw
digest and all16 raw projections, compares width interventions and signed
adjacent pairs, preserving decimal magnitude, literal padding and signed-zero
metrics separately. Its source is NOT part of native freeze or accepted evidence
of its own correctness. Audit's independent reference window was respected;
offline test launches held until its explicit RELEASE. On resumption NEW host
build/default-transform-shifted-width-analysis1 stops at first immediate gate:
prefixidle1/fullPSI19.43/4.73%/guard75 BEFORE archived-reader child. No childstatus,
stderr/stdout/report or post-child hashes claimed. Archive-reader replay, analyzer,
metric self-checks and readback all UNRUN; no failure of native/reader/analyzer.
Later direct fullPSI37.11/14.41% confirms HOLD. BEFORE analyzer/source-base0ec+
dirty AFTER-native snapshots and full actual shell/guard receipts retained.

Thirteen supplemental forensic gzip receipts/guard-manifest/GUARD-SHA256SUMS
bring native archive to76 total; both inventories and all compressed/gunzip
sizes/hashes/host bytes verified. Small forensic packaging ionice3/nice10 only;
no new native run or resource reservation. Source archive302 unchanged; its
earlier source-replay pressure stop and packaging failure remain historical.

LOW DQ-DV001 procedural delta/walkthrough: inspected strict-first-before-repeat,
explicit width metadata and8 paired native string equalities; verified no child
result after offline guard75. Misuse risk: upgrading finite equalities into a
formatter or claiming unrun metric checks. Retain gaps and next exact re-entry;
developer-maintainer self-review only, not independent/system/release assurance.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 and rollback unchanged.

NEXT SAME after fresh idle/fullPSI avg10 AND avg60<1%: run THREE offline stages
from retained guard-shell-completion.json.gz into NEW width-analysis2 root,
archived FROZEN strict reader/report byte match, analyzer/readback byte match,
input hashes BEFORE+AFTER EACH0/nice10. Do NOT rerun completed native pair.
Only after that result, next bounded source discriminator is positive D13 actual
inline ABS(payload)+(0.0005*(10^(-D))) from the retained REJECTED diagnostic graph
versus reconstruction of the SAME measured N32/18 shifted payload. This tests a
route boundary, not a proposed formatter bias/formula; retain original/public
ROUND controls and independent metadata/payload checks. Source qualification/
clean freeze/NEW BOTH CLEAR before any new native run. Exact conversion/division/
ListItem remain OPEN; no owner-input blocker/automation pause.

## Width discriminator offline analysis2 completed without native replay

Resolved pressure prerequisite only; NEW build/default-transform-shifted-width-
analysis2 from clean signed/DCO52838d1a28d87e6a0246af412940b546de7cb062 runs the
SAME retained three offline stages. Archived FROZEN strict reader0/empty stderr/
report byte-exact against native audit; analyzer0/readback0/empty/byte-identical.
Inputs hashes BEFORE+AFTER EACH0; all immediate prefixidle1/fullPSI0.00/0.01%/
guard0/nice-n10. BEFORE analyzer source and clean source/status snapshots retained;
this AFTER-native analysis base is NOT native source freeze0ec07074a. No native
invocation/window/compiler/new FLL/VM. Prior analysis1 guard75 remains immutable.

Exact-decimal metric8 self-checks pass; all16 raw42-field records/projected report
tuples and before/after controls checked. Eight width32->40 interventions: zero
TF or STR literal, trimmed-literal, exact decimal-value or zero-sign differences.
Signed adjacent shifted pairs have no TF/STR differences under either width and
FIXED setting (2 comparisons per width/FIXED). Across16 TF-versus-STR pairs:
16 literal gaps/8 trimmed-literal gaps/8 decimal-value gaps (all FIXED OFF), zero
both-zero sign-spelling gaps. This qualifies the earlier direct audit inspection,
not universal width independence, actual expression equivalence, a bias/precision
rule, formatter parity or a relaxation of any original regression.

Thirty-three supplemental source/report/gate/hash/full actual-shell receipts plus
verification-manifest/VERIFICATION-SHA256SUMS bring shifted-width-consumer-captures1
to109 gzip receipts (63 native+13 guard+33 verification). All three inventories,
compressed hashes/gunzip sizes/hashes/exact hostbytes verified. Host analysis2 and
all earlier native/source/guard roots intact; runtime/FLL identity limitations and
ordinary-shell NOT REA Evidence/cleanup-attestation limits unchanged.

LOW DQ-DV001 actual walkthrough: replayed the exact pre-child-held command into a
new root; checked archived reader/result equality, independent width metadata,
all16 projections,8 metric checks, finite value/sign/literal comparisons and
before/after input inventories. Developer-maintainer self-review, not independent
human/system/release assurance. Misuse risk and mitigation unchanged: do not
promote finite width equality to actual-expression or universal formatter policy.
Existing hazards/rollback stay; no compatibility, division or ListItem closure.

NEXT bounded source preparation: positive D13 actual inline shifted expression
ABS(payload)+(0.0005*(10^(-D))) from the retained REJECTED diagnostic graph versus
reconstructed SAME measured N32/18 payload for parent159/160, both FIXED. Preserve
original/public ROUND controls and exact independent metadata/payload checks;
no target display predictions. This is a route-boundary test, NOT a proposed
formatter bias/formula or new production change. Independent reader/synthetic
source qualification and clean freeze BEFORE new BOTH clearance/each immediate
native gate. Do not replay either completed16-row pair. Exact conversion and
unchanged division ORIGINAL/SAME/hosted/review ladder remain OPEN.

## Positive D13 actual-expression/reconstruction source qualified; native UNRUN

NEXT bounded prerequisite prepared, without replaying either completed native
pair: emit-shifted-expression-consumer.cjs, shifted-expression-consumer.prg/cases,
audit-shifted-expression-consumer.cjs, check-shifted-expression-reader.cjs,
shifted-expression-consumer-capture.sh and shifted-expression-source-check.sh.
Parent159/160 positive originals;4 input routes/8 records/42 fields, both FIXED.
Compare actual inline ABS(originalPayload)+(0.0005*(10^(-decimalCount))) against
CFNDEC(CFNWIDTH(targetPayload,32),18), reconstructing measured shifted bits
3D2C25C268497683/84 at N32/18. No sign-copy. This expression is retained diagnostic
stimulus from the REJECTED graph, NOT a proposed formatter rule, bias, tolerance,
operation sequence or general compatibility requirement. No display expectation
or route equality is assumed. Independent parent4+measured shifted4 raw controls
bind the stimuli; output metadata/payload requirements are measured, not fitted.

Use explicit IF/ELSE source branches around all Character observations, not IIF
or an assigned Numeric expression/result that could normalize the route. Original
N40/18 and actual inline public ROUND remain before/after controls. Independent
reader isolates uniquely marked complete branches and verifies each literal
consumer path, runtime identity/FPU controls, metadata/BRS and before/after output
stability. Synthetic48 checks include wrong-width40 rejection independently for
both routes/before+after, framing/status/settings/control mutations and permitted
inactive Numeric integer-slot variation. Arbitrary valid target test strings are
structural checks only, NOT native compatibility expectations.

Host build/default-transform-shifted-expression-generate1 two generation stages0;
source-checks1 all11 stages0/empty stderr/input hashes BEFORE+AFTER EACH0/48 tests.
New archive-replay1 all4 archived-source stages0/empty/four reports byte-exact/
hashes BEFORE+AFTER EACH0/48 tests again. All17 immediate prefixidle1/fullPSI
avg10/avg60=0.00/0.00%/guard0/nice-n10; no native/compiler/new FLL/VM/window.
BEFORE snapshots/base d8ea75d34+dirty new-source additions are NOT native clean
freeze. Source files are unchanged between source tests and archive recomputation.

Archive shifted-expression-source-checks1 has139 primary+41 verification=180
gzip source/contract/status/gate/hash/report/full actual-shell receipts, both
manifests/checksum inventories; all compressed hashes/gunzip sizes/hashes/exact
hostbytes verified. All three host roots retained. Ordinary shell NOT REA
Evidence/native/Windows/internal-call/atomic-image/cleanup proof. No earlier
guard, failure, source freeze or finite native result erased or upgraded.

LOW DQ-DV001 procedural delta/actual walkthrough: inspected source branches and
native-stimulus provenance, original/public ROUND before+after controls, independent
strict reader paths and48 mutation tests, exact generation/archive readbacks and
input inventories. Misuse risk: treating expected metadata or the diagnostic
constants as a predicted display/accepted formatter. Mitigation: no target text
predictions, new paired native qualification still mandatory. Developer-maintainer
self-review only, not independent human/system/release assurance. Existing
hazards/rollback and unchanged division/ListItem ladder retained.

NEXT SAME: clean signed source freeze, fresh safe prefix/fullPSI observation then
NEW BOTH explicit audit+DAW native-only CLEAR. Absent new capture root/exact reused
observer/no host compiler/new FLL/VM. Frozen worker first8-record native0 -> strict
single0 BEFORE repeat -> repeat0/paired0; each immediate prefixidle and fullPSI
avg10 AND avg60<1%/nice10. Preserve any stop and release promptly. Native pair is
UNRUN, no reservation/window requested; exact conversion/formatter/parent and
division/ListItem acceptance stay OPEN. No owner-input blocker/automation pause.

## Positive D13 actual-expression/reconstruction native pair qualified

Clean signed/DCO/G source freeze964d3aae606634b4f7593dcaa012d1fc46aadd99, unchanged
from source qualification180 receipts. Fresh15:15:34UTC prefixidle/fullPSI0.00/
0.01% then NEW BOTH explicit audit+DAW native-only CLEAR; exact requests and actual
clearance snapshots retained. Frozen worker ran ONCE: first native0, strict-single0
BEFORE repeat, repeat0, paired reader0; all stderrs empty. Eight records/all42
raw fields repeat-identical; inactive Numeric integer-slot differences0. Both raw
SHA321cf441f436db9c77de2aa872f6c5b52c3cc9d22cb7b8211399062006470355.
Immediate native gates first prefix1/fullPSI0.86/0.29%/guard0 and repeat prefix1/
fullPSI0.88/0.31%/guard0, each nice-n10. All are below BOTH1% thresholds, not a
waiver of the gate. Exact observer reused/native-only Microsoft32 override/
inheritedUNSET; no host compiler/new FLL/VM. BOTH promptly RELEASED/post-prefixidle.

Validated/moved completed host root to build/default-transform-shifted-expression-
consumer-captures1; original worker root ABSENT. Native/source/runtime/status/
input/control/actual shell/inspection/clearance/release receipts and own FXP
retained; no FLL/proprietary SDK binary included. The combined direct-inspection
shell exit1 comes from its final idle-prefix pgrep, NOT native/reader failure;
actual four native/reader status files0 and inspection stdout retained verbatim.
Archive shifted-expression-consumer-captures1 initially64 native gzip receipts,
manifest/SHA256SUMS; compressed hashes/gunzip sizes/hashes/exact hostbytes verified.
Ordinary shell is NOT REA Evidence/Windows/internal-call/atomic mapped-code or
descendant-cleanup proof; prior runtime identity limitations unchanged.

AFTER-capture analyze-shifted-expression-consumer.cjs binds the exact source/raw
digest and all8 raw42-field projections/active metadata, before/after controls;
compares actual expression versus reconstruction and the adjacent positive pair,
keeping exact decimal values/literals/padding/zero signs separate. Eight metric
self-checks pass. Host build/default-transform-shifted-expression-analysis1:
archived FROZEN reader0/empty/report byte-exact, analysis0/readback0/empty/byte-
identical, input hashes BEFORE+AFTER EACH0, every immediate prefix1/fullPSI0.00/
0.04%/guard0/nice-n10. BEFORE analyzer snapshot/base964+dirty AFTER-native source/
status is NOT native clean freeze. Cosmetic inherited shell completion word
"width" is preserved; actual paths/source/raw digest/eight contexts identify the
expression-route analysis, not a rerun or mislabeled qualification of width data.

Finite observations: all4 actual-expression/reconstructed pairs have identical
observed active N32/18 tuples AND TF/STR Character strings; literal/trimmed/value/
zero-sign differences0 for both consumers. Parent159->160 shifted adjacent pair
has no TF/STR change in either route/FIXED mode (one comparison per group).
TF-versus-STR across8 contexts:8 literal/4 trimmed/4 exact-value gaps (all OFF),
zero both-zero sign gaps. In this sample no expression/reconstruction difference
explains the diagnostic graph mismatch. This does NOT establish universal route
equivalence, internal formatter sequence/precision, accepted bias/formatter or
parity, nor rescue the REJECTED graph or original regression.

Thirty-three supplemental source/reader/report/gate/hash/actual-shell receipts,
verification-manifest/VERIFICATION-SHA256SUMS bring native archive to97 gzip
receipts. Both inventories and all compressed/gunzip sizes/hashes/hostbytes
verified. All host roots/older guard and failure receipts retained; no replay of
either completed16-row pair. LOW DQ-DV001 actual walkthrough checked first-strict-
before-repeat, explicit actual branch versus reconstruction, exact native/control
tuples, frozen reader,8 metric tests and finite comparison coverage. Developer-
maintainer self-review only, not independent human/system/release assurance.
Existing HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01/rollback stay.

NEXT bounded OFFLINE discriminator, not another native replay or the completed
measured-shifted-product calculation: compare exact pre-addition operands
ABS(original)+measured native bias with the measured rounded shifted payload,
then use observed native powUp. Cover retained264 ON endpoints/220 adjacent pairs
and all plateaus, preserving source/raw controls and rejected cases. Determine
whether precision lost AT the addition can distinguish actual display boundaries;
do not assume any internal operation/precision or fit a tolerance. Prior measured
shifted*powUp and exact decimal-bias candidates remain rejected/unchanged. This
is a hypothesis discriminator, not formatter acceptance. Exact conversion/division
ORIGINAL-SAME/hosted/review/ListItem remain OPEN; no owner-input blocker/pause.

## Traceability and development-document walkthrough

`VR-5611-NUMERIC-DEFAULT-TRANSFORM-001`: native qualification, native-only
expectation emitter, original-fail/SAME, focused/broader/actual named hosted
execution, exact-head review, strict checks and resolved conversations are
pending except the seven earlier bounded captured pairs and this native-bound
CRT pair qualified above, each retaining its provenance limits. No compatibility,
parent, hazard or release closure.

`DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001`, LOW procedure delta: preserve the
uncancelled division source/test freeze3b7a51636 and complete ORIGINAL031d790ab;
work only in separate display WT from origin/main240ac. Walkthrough confirms
there is no formatter/hook change and no instruction to drop display assertions.
Misuse risk: mistaking a stimulus or fitted model for the oracle would permit
silent display/data corruption. Mitigation: immutable raw native paired capture,
independent expected reader, actual original fail, unchanged SAME and exact-head
review. HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 unchanged.
Rollback is the exact display slice revert, retaining raw receipts; notify the
owner if evidence or shipped formatting is invalidated. Developer-maintainer
self-review only, not independent-human, system or release assurance.

2026-10-18 arithmetic checkpoint walkthrough: inspected signed stimulus and
strict reader against all528 retained rows, verified archived readback/byte
checks and18 explicit rejected-model counterexamples. The next procedure keeps
count0/unchecked buffers/guessed DECLARE ABI out of execution, requires a new
source freeze and resource clearance, and leaves proprietary binaries host-only.
This is LOW developer self-review of the evidence/continuation procedure,
not verification of an as-yet-unwritten wrapper or independent acceptance.

## 2026-10-19 — exact pre-addition discriminator prepared; checks pressure-blocked

`compare-preaddition-operands.cjs` extends the measured-operand analysis without
replaying native VFP. It retains exact rational ABS(original) and the MEASURED
binary bias separately through their sum, then multiplies by measured powUp.
It compares that with the already analyzed rounded-shifted product for all264 ON
endpoints/220 adjacent pairs/all176 plateaus, excluding the same six uncaptured
gaps. Full prior nodes/pairs, active metadata, Character literals/signs,528 raw
OFF/ON control joins and18 rejected diagnostic-graph contexts remain visible.
The proposed output distinguishes endpoint addition-loss signs, hypothetical
half-up integer decisions and original decimal-magnitude gaps, plus transition
and plateau decisions separately. No new outcome is known yet.

This is NOT the completed exact rounded-shifted product comparison, the rejected
true-decimal-bias candidate, a formatter, expected emitter or production repair.
Its exact-arithmetic/half-up/count assumptions are diagnostic hypotheses, NOT
observed native formatter internals, sequence, precision or accepted requirements.
The source binds and replays the frozen measured-operand/stage/inventory/raw
validators and separately the graph classification. Nineteen metric self-checks
are authored, but UNRUN. No syntax, analysis, readback or source replay pass is
claimed for this newly prepared source.

`preaddition-check.sh` snapshots source/base/status BEFORE checks, hashes all
bound source/archives, checks input hashes BEFORE+AFTER EACH successful stage,
requires idle prefix and BOTH full I/O PSI avg10/avg60 below1% immediately before
each child, and runs each at nice-n10. Planned stages: Node syntax, bash syntax,
analysis, readback, immutable BEFORE-source snapshot replay; reports must be
byte-identical. It does not grant a compiler/native/VM resource window.

Actual command: `bash tests/fixtures/vfp9-numeric-default-transform/preaddition-check.sh
build/default-transform-preaddition1`. Shell status75; idle prefix1, full I/O
PSI42.12/19.70%; syntax.guard.status75 BEFORE Node, no syntax.status/report or
subsequent check invoked. Source/base51052ed39+dirty BEFORE snapshots, complete
input inventory/BEFORE hash check, PSI/prefix/guard and actual shell completion
are retained in host build/default-transform-preaddition1 and12 gzip receipts at
preaddition-guard1/. Manifest/SHA256SUMS and all compressed+decoded sizes/hashes/
exact host bytes verified during packaging. This is a pressure guard, not a
syntax/analyzer/native/product failure. There is deliberately no AFTER-input
check or completed-test assertion. Subsequent read-only PSI observations also
remained above gate; no attempt to identify/monitor a backup or bypass the gate.

LOW `DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001` actual walkthrough: inspected exact
binary decoding, sign/magnitude and reduced-rational handling, generalized
integer decisions, both original consumer value controls, rejected rows, full
pair/plateau coverage and transitive source/raw replay checks. Reviewed worker
fail-closed gate, fresh roots, BEFORE snapshots and strict non-success labeling.
This is developer-maintainer source/procedure self-review only; authored metric
checks and runtime behavior remain unverified, not independent/system/release
assurance. Existing hazards/mitigations/rollback above remain unchanged.

NEXT: with a fresh idle/PSI observation passing BOTH limits, run SAME unchanged
source with preaddition-check.sh at NEW build/default-transform-preaddition2;
retain guard1 intact. Preserve any new failure and its exact source before
correction; interpret only actual successful reports, archive/report replay and
byte-check receipts, then update this chronology/matrix/changelog/handoff. No
new native run, compiler or VM is needed for this bounded question. No accepted
boundary/parity, parent closure or unchanged division ORIGINAL/SAME relaxation.

### Fresh resource gates pass — preaddition2 qualification, same unchanged source

Following the preserved guard, both full I/O thresholds cleared. SAME source
at clean signed/DCO8494e9d1a was run ONCE at NEW host
build/default-transform-preaddition2: Node syntax0/bash syntax0/analysis0/
readback0/BEFORE-source snapshot replay0, all stderr empty. Reports byte-identical;
input hashes BEFORE+AFTER EACH0. Every immediate prefix1/fullPSI avg10=0.00%,
avg60=0.90..0.93%/guard0, every child nice-n10. All19 new metric checks pass,
with unchanged bound14 measured-operand/9 stage/8 inventory/9 classification/
8 raw-validator checks. The clean BEFORE source/base snapshot is an offline
analysis freeze, NOT a native source freeze or qualification of old runtimes.

Exact addition sums differ from the rounded shifted payload at all264 nodes:
130 below/134 above/zero equal. Hypothetical pre-addition versus post-addition
half-up decisions differ at12 endpoints; magnitude gaps against observed
TF+STR are34 versus30. Among18 retained graph rejections, the two hypothetical
routes have6 versus4 magnitude gaps; neither recovers the actual formatter.
All literal/zero-sign/active metadata fields remain observations in full nodes,
not generated outputs or collapsed into this magnitude-only comparison.

Across44 actual display transitions, preserving the sum distinguishes14 (misses30),
versus16 using the rounded shifted product. It invents30 changes on176 observed
plateaus, versus28 after addition rounding. Addition rounding changes boundary
decisions on10 transition pairs and14 plateau pairs. Among12 raised-tuple
collapses the pre-addition decisions differ4, versus10 post-addition. Both D13
scaled-collapse witnesses159->160/165->166 still give1 at BOTH endpoints in
BOTH routes: distinct exact sums/products are NOT sufficient display boundaries.
This rejects retaining addition precision alone as a repair of this diagnostic
graph; it does not identify native internal operation order/precision or rule
out precision loss elsewhere. No accepted formatter/expected strings result.

Successful source/base/status/gates/inputs/reports/actual shell completion are
53 gzip receipts in preaddition-analysis2. NEW host
build/default-transform-preaddition-archive1 replays the GZIP-archived source at
its original module path: status0/empty stderr/byte-exact report, original and
archive inputs BEFORE+AFTER0, immediate idleprefix1/fullPSI0.00/0.30%/guard0/
nice-n10. Fifteen supplemental receipts with actual command/completion and
verification-manifest/VERIFICATION-SHA256SUMS bring total68. Both inventories,
all compressed and decoded sizes/hashes and exact host bytes verified. All
guard/source/host roots remain intact. No compiler/native/VM/reservation used.

LOW DQ-DV001 actual walkthrough now checks executed19 tests, dependency replays,
all264 nodes/220 pairs/176 plateaus/18 rejections, D13 counterexamples,
literal/magnitude separation, clean BEFORE source identity, full I/O/idle/nice
gates and archived byte-exact rerun. Developer-maintainer self-review only;
existing hazards/mitigations/rollback above unchanged. No independent/system/
release assurance, original/SAME relaxation or parent/division/ListItem closure.

NEXT bounded OFFLINE join: compare these pre-addition endpoint/pair decisions
BY INDEX with the retained exact-decimal `decimal_prebias_cap15` candidate in
decimal-half-grid-analysis1/half-grid-adjacency-analysis1 (sources
test-decimal-half-grid.cjs/join-half-grid-adjacency.cjs). Both aggregates are34 magnitude gaps,
14 coincident/30 missed/30 invented changes; aggregate equality is NOT evidence
of identical rows. Bind/replay frozen sources/raw and verify endpoint decisions,
pair membership and counterexample identity on all264/220, retaining every
plateau/sign/rejected row. Distinguish measured BINARY bias/powUp effects from
true DECIMAL bias/powers without fitting a new candidate, assuming formatter
internals or repeating native runs. Select a new stimulus only from the actual
unexplained counterexamples, never from a tolerance or guessed precision rule.

## 2026-10-19 — index-exact pre-addition/decimal candidate join qualified

`join-preaddition-decimal.cjs` binds/replays the frozen preaddition-analysis2
source/report and half-grid-adjacency-analysis1 source/report, including their
transitive native raw/control validators. It joins all264 ON nodes and220 actual
adjacent pairs BY INDEX, checking payload, setting, sign, literals, exact decimal
observations and native effective-count bytes. Full prior metadata and18 graph
rejections remain in the report; six uncaptured gaps remain excluded. This is a
comparison of two already REJECTED hypothetical quantizers, not a new model.

Result: zero endpoint integer-decision differences, zero magnitude-gap membership
differences, zero pair-category or pair endpoint-decision differences. The SAME34
failed endpoint indices occur in both candidates (SAME6 among18 graph rejections).
Every pair-category set matches:14 coincident changes,30 missed changes,
30 invented changes on176 observed plateaus,146 shared plateaus. All19 setting
groups independently have zero endpoint differences. D13 missed pairs159->160/
165->166 and invented157->158/163->164 remain explicit counterexamples, not
accepted boundaries. Equality applies to these decisions/membership ONLY, not
exact operands, global equivalence, formatter algorithm/order or precision.
Measured binary bias/powUp rather than true decimal counterparts does not
explain the remaining gaps in THIS sample under the hypothetical half-up graph.

`preaddition-decimal-check.sh` is a separate fail-closed offline worker: BEFORE
source/base/status snapshots, input hashes BEFORE+AFTER EACH stage, idle prefix,
full I/O PSI avg10 AND avg60<1%, nice-n10 EACH child, fresh roots only. Actual
host build/default-transform-preaddition-decimal1 from base3c452cace+dirty new
source snapshots: Node syntax0/bash syntax0/analysis0/readback0/BEFORE-source
snapshot replay0, empty stderr, reports byte-identical, all input checks0.
Thirteen new metric tests pass, including all four pair categories, canonical
unit strings, equal-count/different-membership witness, order-independent sets,
duplicate rejection and indexed lookup. Bound prior19 pre-addition/4 decimal
join/14 decimal candidate/14 measured operand/9 stage/8 inventory/9 classification/
8 raw-validator checks remain unchanged. Immediate prefix1/fullPSI0.00/0.04%/
guard0 for allfive stages; no compiler, native, VM or reservation.

NEW host build/default-transform-preaddition-decimal-archive1 replays gzip-
archived source at its original module path:0/empty stderr/byte-exact report,
original and archive input hashes BEFORE+AFTER0; prefix1/fullPSI0.00/0.01%/
guard0/nice10. Archive preaddition-decimal-analysis1 contains53 original+15
verification=68 gzip receipts with both manifests/checksum inventories. All
compressed/decoded sizes/hashes/exact host bytes checked. BEFORE snapshots are
offline source evidence, NOT a clean native freeze. All older guard roots,
captures, raw/runtime provenance and rejected hypotheses remain intact.

LOW `DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001` procedural delta/walkthrough:
replace an aggregate coincidence with index-exact endpoint/pair/set comparison;
inspect every join key, native count, preserved signed literal/active metadata,
13 executed self-checks, transitive source/raw replays, per-child resource gates,
all reports and archived byte-exact recomputation. Developer-maintainer self-
review only, not independent/system/release assurance. Existing hazards,
mitigations, rollback and field notification above unchanged. No formatter,
native expected emitter, fitted constant/tolerance, production change, parent
closure or unchanged division ORIGINAL/SAME/assertion relaxation.

NEXT bounded discriminator: prepare ORIGINAL D13 payload width32 versus width40
routes at SAME decimals18 for signed parent159/160/165/166, both FIXED modes,
with original/public ROUND controls before+after and independent strict reader.
The completed width test used SHIFTED payloads, not these original adjacent
display-transition inputs; do not replay it or generalize its finite null result.
Use emit-shifted-width-consumer.cjs and its qualified reader/worker as a template,
not as new native evidence. Preserve payload/scale and vary width ONLY; no target
width32 strings assumed. Parent N40/18 strings/metadata/payloads must retain raw
controls. Qualify generated source and synthetic reader coverage, archive/source
replays and clean signed freeze BEFORE any native run. Then NEW BOTH explicit
native-only CLEAR plus immediate idle/fullPSI gates, reused observer, nice10.
No compiler/new FLL/VM needed unless later evidence demonstrates otherwise.

## 2026-10-19 — ORIGINAL D13 width source prepared; archive replay pressure-blocked

New emit-original-width-consumer.cjs/generated original-width-consumer.prg/
original-width-consumer-cases.json hold four signed ORIGINAL payloads from
parent159/160/165/166, scale18 and DECIMALS13 fixed; vary width32 versus40,
both FIXED modes:8 input routes/16 records/42 fields. Positive bits
3D2C1E8DBBA38061/62 and negative BD counterparts come from original parent
raw evidence, NOT shifted payloads or fixture sign-copy. No bias/power/quantizer,
target Character predictions or assumed universal width rule is introduced.
Inline CFNDEC(CFNWIDTH(targetPayload,inputWidth[index]),inputScale[index]) is
supplied afresh to EVERY consumer; no Numeric argument/result assignment.
Original N40/18 and public inline ROUND controls surround target observations.

Independent audit-original-width-consumer.cjs uses its own four literal seeds,
validates all8 selected OFF/ON parent controls against both retained raw runs,
then requires exact input order/coverage, VARTYPE, active Numeric metadata,
BRS, settings, parent before/after Character strings and public ROUND controls,
target before/after agreement, strict runtime path/digest/PE/control state,
first-only gate BEFORE repeat and paired active-field agreement. Inactive
Numeric integer slots remain the only repeat exclusions, explicitly counted.
No target string (including the new width40 route) is fabricated or assumed.
This is source/reader preparation, NOT newly observed native output.

original-width-consumer-capture.sh reuses the existing observer with its exact
source/binary digest checks; rejects inherited nonempty DLL overrides and records
process-local msvcr71=n. NEW BOTH native-only CLEAR plus Request Guard release/
coordination required; the acknowledgment flag itself grants no clearance.
Unlike the prior template, EVERY preparatory Node/strict-reader/native launch
has idle prefix/full I/O PSI avg10 AND avg60<1%/nice-n10 and input checks BEFORE/
AFTER. Nonzero child status/empty-stderr requirements fail closed; any partial
native report and original child status are retained. No compiler/new FLL/VM.
Native worker/root build/default-transform-original-width-consumer remain UNRUN/
ABSENT. Source freeze/native qualification still require the archive replay below.

Actual offline stages: original-width-source-check.sh generate at NEW
build/default-transform-original-width-generate1 completes2 stages0/empty.
Generated outputs applied to source without shell source writes; initial
build/default-transform-original-width-source-checks1 completes11 stages0/empty,
48 synthetic reader checks. Expanded pass2 at NEW
build/default-transform-original-width-source-checks2 completes13 stages0/empty,
48 reader +13 worker checks; generation/parent source contract byte-exact/input
hashes BEFORE+AFTER EACH0, immediate idleprefix1/fullPSI0.00/0.00%/guard0/nice10.
BEFORE source/base07cad3c5c+dirty snapshots retained for EACH pass; not a native
clean freeze. Source-only reader confirms8 parent controls/newResultsRead=false.

check-original-width-worker.cjs extracts ONLY the checked() function, uses
synthetic pgrep/pressure data and harmless shell children in its own temporary
root; never executes capture.sh or a native path. Executed checks cover clear
gate, busy/error prefix, equality and exceedance of EACH PSI limit, missing full
record, child7/retained output, stderr98, input mutation, partial-result/status
retention and native/strict-reader ordering in source. These finite tests are
NOT actual host clearance, native execution or descendant-cleanup proof.
Reader tests cover every active field/control/runtime/width and repeat mutation,
valid width swaps, malformed framing/errors and inactive-slot-only acceptance;
synthetic target text is deliberately arbitrary, NOT compatibility expectations.

Base source archive original-width-source-checks1 contains266 gzip receipts from
generation/initial/expanded passes. All compressed and decoded sizes/hashes and
exact host bytes verified. NEW archived-source replay command at
build/default-transform-original-width-source-replay1 plans seven stages
(emitter source/cases, source reader, reader+worker synthetic tests, both shell
syntaxes), but stops shell75 BEFORE its FIRST Node child: idleprefix1/fullPSI
48.79/25.77%/guard75. Original+archive BEFORE hash checks0; no child status,
report, AFTER hash check or successful archived-source execution claimed.
Exact full command/actual shell completion and10 guard/source/hash receipts
are preserved (11 supplementary gzip total); verification-manifest and
VERIFICATION-SHA256SUMS bring archive to277 receipts, all compressed/decoded/
host bytes verified. Later PSI remained above gate; this is a pressure stop,
not syntax/analyzer/native/product failure. No backup monitoring or resource
window reservation was used. The Request Guard window remains a separate hold.

LOW DQ-DV001 procedural delta/walkthrough: original rather than shifted target
bits; independent literal-seed/raw-parent checks; new EVERY-child resource gate;
actual2/11/13-stage receipts and48+13 synthetic results; explicit pre-child guard
and UNRUN archive/native labels. Developer-maintainer self-review only, not
independent/system/release assurance. Existing hazards/mitigations/rollback and
notification policy above remain unchanged. No formatter/expected emitter,
fitted tolerance/production change/parent closure/division SAME relaxation.

NEXT SAME unchanged source: once fresh idle/fullPSI avg10 AND avg60<1%, replay
the exact archived-source command retained in replay1-shell-completion.json.gz
at NEW build/default-transform-original-width-source-replay2 (update ONLY the
root name; never overwrite replay1). Archive/review the seven actual results,
byte-equal comparisons, input checks and gates. Only then take clean signed
source freeze and request NEW BOTH native-only CLEAR plus Request Guard release/
coordination; immediate EACH-launch gates remain mandatory. Do not run either
completed shifted-value16 pair or the completed original/shifted pair again.

### Original-width archived-source replay2 passes — unchanged source

After fresh gates cleared, the EXACT retained replay1 command was run with ONLY
the root changed to build/default-transform-original-width-source-replay2.
Clean offline base4438af550/BEFORE status empty; no caller/reader/worker/source
change. Allseven archived-source stages0/empty stderr: source/cases regenerate
byte-exact, strict source-only parent reader byte-exact,48 synthetic reader and
13 synthetic worker tests repeat byte-exact, both archived shell syntaxes pass.
Original and archive input hashes BEFORE+AFTER EACH0; every immediate prefix1/
fullPSI0.00/0.05%/guard0/nice-n10. Old replay1 pre-child75 remains intact.

Eighty-two supplemental receipts (exact command/actual shell completion, clean
BEFORE source/status, inventories, seven full stage gates/logs/results/checks)
at replay2-* with replay2-manifest/REPLAY2-SHA256SUMS bring source archive to359
gzip receipts. Allthree inventories and compressed/decoded sizes/hashes/exact
host bytes verified. This finishes source qualification, not native execution.
LOW DQ-DV001 actual walkthrough extends the prior source review with immutable
source byte identity, allseven execution receipts and strict original/archive
hashes, not a new formatter expectation or native/internal/Windows proof.
Existing hazards/mitigations/rollback and assertion preservation unchanged.

NEXT: clean signed source freeze, NEW explicit audit+DAW native-only CLEAR and
Request Guard release/coordination; fresh EACH-child idle/fullPSI gates. Run
original-width-consumer-capture.sh ONCE only if all coordination/gates pass,
strict single BEFORE repeat, preserve all raw/status/partial results, release
the window promptly. Reuse exact observer; no compiler/newFLL/VM. The completed
shifted-value/native pairs are not part of this new ORIGINAL-width capture.

## 2026-10-19 — ORIGINAL D13 width native pair qualified; offline replay guarded

Clean signed/DCO source freeze0b381796591c5ef074e8bec185a3bd8728fb27aa,
same qualified caller/strict reader/every-child guarded worker, NEW explicit
audit+DAW native-only CLEAR and Request Guard VM-window release/no conflict.
Actual coordination prompts/responses/pre-request shell and release are retained.
No compiler/new FLL/VM. Exact observer reused; process-only msvcr71=n, inherited
override UNSET, prior Microsoft32 binding/ABI/once-file-sampling limits unchanged.

Worker ONCE: first native0, strict first-only reader0 BEFORE repeat, repeat0,
paired reader0; every preparatory/native/reader child0/empty stderr. All16 records/
42 raw fields repeat-identical, inactive Numeric integer-slot differences0.
Both raw SHA2c8edbd66a1030613e6bf39d203fac3bfcb7bb221e614205bf4a244e12a6924f.
Every immediate launch prefix1/fullPSI0.00/0.00%/guard0/nice-n10; native first/
repeat gates and all input hashes BEFORE+AFTER EACH0 retained. Window promptly
RELEASED to allthree coordinators; post-prefix idle/no reservation/job.
Combined inspection shell exit1 is its FINAL idle-prefix pgrep, NOT native/
reader failure; separate actual child status files are all0 and stderr empty.

Validated and moved completed root to build/default-transform-original-width-
consumer-captures1; worker root build/default-transform-original-width-consumer
ABSENT. Native/source/runtime/FXP/control/gate/status/hash/actual-shell/inspection/
coordination/release receipts initially99 gzip files at original-width-consumer-
captures1 with manifest/SHA256SUMS. All compressed/decoded sizes/hashes/exact
host bytes verified. Own authored PRG/FXP retained, no proprietary SDK/FLL binary.
Ordinary shell is NOT REA Evidence/Windows parity/internal-call/atomic mapped-code
or descendant-cleanup attestation. Prior provenance limitations are not upgraded.

Direct audit inspection (not yet the AFTER-capture exact-decimal analyzer):
all8 paired width32/40 target TF+STR Character strings are identical; the signed
original adjacent display transitions remain at both widths. This is a finite
null width intervention on these four ORIGINAL D13 payloads, not a universal
width rule or formatter/precision repair. All original/public ROUND controls
remain raw-exact before+after. The completed shifted-width and expression pairs
were NOT rerun; no target Character strings were used as expected inputs.

New AFTER-capture analyze-original-width-consumer.cjs binds source freeze/raw
digest, every raw42-field/active metadata/control projection and both runs;
retains literal/padding/exact decimal value/zero-sign distinctions and original
width interventions/adjacent pairs. Eight metric checks authored, UNRUN.
First offline command at build/default-transform-original-width-analysis1 stops
shell75 BEFORE frozen-reader replay: idleprefix1/fullPSI48.41/13.75%/guard75.
No reader/syntax/analyzer/readback child status or AFTER check exists; successful
native qualification above is unaffected. BEFORE source/base0b381+dirty
AFTER-capture snapshots, full exact command/actual shell, input-before/hash/gate
receipts retained. Ten supplementary gzip files plus verification-manifest/
VERIFICATION-SHA256SUMS bring archive109; both inventories/all compressed and
decoded sizes/hashes/exact host bytes verified. Later PSI remained high; no
native retry, backup monitoring, owner-input blocker or automation pause.

LOW DQ-DV001 actual walkthrough extends source qualification with fresh clearance,
strict-first-before-repeat, all16/42 raw field identity, every-child gate/hash/
status, preserved runtime limits, prompt release and exact pre-child verification
stop. Developer-maintainer self-review only, not independent/system/release
assurance. Existing hazards/mitigations/rollback/assertion preservation unchanged.
No formatter/expected emitter, tolerance/cap, compatibility or parent closure.

NEXT SAME AFTER-capture analyzer: exact command in analysis1-shell-completion.json.gz
at NEW build/default-transform-original-width-analysis2 (change ONLY root) after
fresh idle/fullPSI avg10 AND avg60<1%. Frozen reader/syntax/analysis/readback must
all actually pass, reports byte-exact, eight metric checks, hashes/gates retained;
archive/review before interpreting metrics or choosing the next metadata stimulus.
No new native run/compiler/VM needed for this step; native source freeze remains
0b3817965, distinct from the AFTER-capture analyzer/evidence commit.

### Original-width offline analysis2 qualified — SAME source, no native rerun

After fresh I/O gates cleared, the EXACT retained analysis1 command ran with
ONLY root changed to build/default-transform-original-width-analysis2. Clean
AFTER-capture basecbaba4ac3/BEFORE status empty, native freeze0b3817965 unchanged.
Frozen native-source reader0/empty/report byte-exact, analyzer syntax0,
analysis0/readback0/empty/byte-identical; allfour stages input hashes BEFORE+
AFTER EACH0, immediate prefix1/fullPSI0.00/0.60%/guard0/nice-n10. All8 metric
self-checks pass and all16 raw42-field/active metadata/control projections bind
to the unchanged paired native digest. Earlier analysis1 pre-child75 retained.

Finite results: all8 width32/40 interventions have zero TF+STR literal/trimmed/
exact-value/zero-sign differences. At EACH width, both signed adjacent pairs
change STR value under OFF and ON (2/2 per group); TF changes2/2 ON,0/2 OFF.
TF-versus-STR across16 contexts:16 literal differences,10 trimmed differences,
8 exact decimal-value differences (all OFF) and2 both-zero sign-spelling gaps
(negative lower endpoint ON, both widths). Literal/padding/sign distinctions
are not collapsed into a guessed binary-versus-decimal defect metric.
The original D13 transitions persist regardless of tested width; finite null
interventions do NOT imply universal width independence or repair/validate the
rejected arithmetic graph, precision/order, propagation or accepted formatter.

Additional NEW build/default-transform-original-width-analysis-replay1 executes
the gzip-archived AFTER-capture analyzer at its original module path:0/empty/
byte-exact report,8 metric checks, hashes BEFORE+AFTER0, immediate prefix1/fullPSI
0.00/0.25%/guard0/nice10. Frozen/current source bytes explicitly matched; this
is source-bound offline recomputation, not new runtime qualification. Fifty-five
supplemental source/base/status/full command/actual shell/report/gate/hash receipts
and analysis2-manifest/ANALYSIS2-SHA256SUMS bring native archive to164 gzip files
(99 native+10 old guard+55 verification). Allthree inventories and compressed/
decoded sizes/hashes/exact host bytes verified. No native/compiler/VM/reservation.

LOW DQ-DV001 actual walkthrough: executed frozen-reader and exact raw projection
checks, literal/value/zero-sign metrics and8 self-checks, both widths/settings/
signs/all8 interventions/adjacent groups, preserved failure, clean AFTER-capture
source identity and archived byte-exact recomputation. Developer-maintainer
self-review only; existing hazards/mitigations/rollback/notification unchanged,
no independent/system/release assurance or original/SAME relaxation.

NEXT bounded source discriminator: ORIGINAL D13 decimal metadata13 versus18,
width40 fixed, SAME four signed payloads159/160/165/166 and both FIXED modes.
Vary decimal metadata ONLY through inline CFNDEC(CFNWIDTH(targetPayload,40),scale),
preserving payload bits, D13, parent N40/18 and public ROUND controls before+
after, independent strict reader and no target-string predictions. This does
NOT repeat the completed scale pair: emit-scale-intervention.cjs selected
parents193/199/205/211/217/223 at D16/17/18, not these D13 adjacent witnesses.
Use the qualified original-width source/reader/every-child guarded worker and
the old scale source as templates, not evidence for new target text. Qualify
generation/source/synthetic reader+worker/archive checks and clean signed freeze
BEFORE NEW resource coordination/native execution; no compiler/newFLL/VM needed.
No tolerance, guessed rounding precision, canonical-double shortcut or formatter
implementation/parent closure is justified by the null width result.

### Original D13 decimal-metadata13/18 source qualification — native UNRUN

Bounded derived RQ/VR-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001 discriminator:
emit-original-scale-consumer.cjs generates original-scale-consumer.prg/cases.json.
Exactly four signed ORIGINAL witnesses parent159/160/165/166, bits
3D2C1E8DBBA38061/62 and BD2C1E8DBBA38061/62, D13, width40, decimal metadata13/18,
both FIXED modes:8 routes/16 records/42 fields. Target consumers reconstruct
CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]) independently every time;
no numeric argument/result assignment, sign-copy, shifted arithmetic or target
Character predictions. Original N40/18 and public inline ROUND controls before+
after remain bound to eight independent raw parent observations/digest254b46da...
The completed D16/17/18 scale sample is distinct and was NOT rerun.

Independent audit-original-scale-consumer.cjs binds literal seeds, generated
source, all42 raw fields, settings, active metadata/payload/control/runtime
identity and before/after equality. check-original-scale-reader.cjs passes50
SYNTHETIC tests: baseline/single/paired, control and malformed field rejection,
both valid decimal-metadata swaps rejected at both target observations, wrong
width rejected, status/stderr/runtime/repeat mutation rejection and acceptance
of ONLY inactive Numeric integer-slot differences. Arbitrary synthetic target
text is not native compatibility evidence. check-original-scale-worker.cjs
passes13 SYNTHETIC tests using ONLY extracted checked() with fake pressure/prefix
functions and harmless children, never the whole native worker/native paths.
This covers both I/O windows at/either side of1%, missing full PSI, busy/error
prefix, status/stderr/hash failure and partial-result preservation/order.

original-scale-source-check.sh generate/check ran at NEW host roots
build/default-transform-original-scale-generate1 and original-scale-source-checks1:
2 generation+13 source stages0/empty; regeneration byte-exact, all input hashes
BEFORE+AFTER EACH0, immediate prefix1/fullPSI avg10=0.00/avg60<=0.02%/guard0/
nice-n10. BEFORE source base1bd7b3ed9 plus dirty source snapshots is NOT a clean
native freeze. New native worker original-scale-consumer-capture.sh retains
every-child gates/immutable inputs/strict-first-before-repeat/process-local
Microsoft32 override and refuses a dirty source tree. Flag is acknowledgement
of external coordination only, never a resource grant. Reuses observer; no
compiler/newFLL/VM required. Native worker has NOT executed in this checkpoint.

Source archive original-scale-source-checks1 initially158 gzip receipts plus
manifest/SHA256SUMS. Packaging first misclassified source-reader.guard.status
as a source snapshot and failed ENOENT; actual failure retained. Corrected
explicit source-name classification verified the partial gzip bytes without
overwriting any existing file, then completed all compressed/decoded hashes,
sizes and exact host-byte checks. This was a packaging failure, not a source,
reader, host-pressure, native or production failure; no defect issue inferred.

NEW build/default-transform-original-scale-source-replay1 compiles gzip-archived
sources at original module paths with explicit archived/current byte identity;
allseven stages0/empty/byte-exact generation/source-reader/synthetic readbacks,
50 reader+13 worker checks, original/archive hashes BEFORE+AFTER EACH0, immediate
prefix1/fullPSI0.00/0.00%/guard0/nice10. Shell worker replay is syntax ONLY.
83 supplemental receipts with full exact replay command/actual shell and
packaging correction bring total241 gzip files; both manifests/checksum lists,
compressed/decoded sizes/hashes/exact host bytes verified. All host roots intact;
no SDK/FLL binary archived. Ordinary shell is NOT REA Evidence, native result,
Windows/internal-call/atomic mapped-code or descendant-cleanup attestation.

LOW DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001 procedural delta: introduce only the
decimal-metadata discriminator, independent fail-closed reader and new-root
workers, preserving existing native controls/acceptance. Actual walkthrough:
source generation, field/source binding,50 reader mutations,13 harmless helper
tests, archived source replay and byte checks; developer-maintainer self-review
only, NOT independent human/system/release assurance. Existing HZ-data-corruption-
01/HZ-runtime-crash-01/HZ-doc-command-01 mitigations remain: never accept synthetic
strings, partial captures, metadata-only parity or guessed tolerance; do not
weaken ORIGINAL/SAME. Rollback is withdraw this diagnostic slice, preserve
receipts and correct the trace/handoff before another run; no runtime release
or field notification is needed for this source-only checkpoint.

NEXT: clean signed source freeze, then NEW explicit audit+DAW native-only CLEAR
and Request Guard release/no conflicting window, fresh idle/full I/O PSI avg10
AND avg60<1% before EVERY child. Audit18:11UTC HOLD0.00/2.93% is retained as
resource-state data, not authority; later passing offline gates do NOT grant a
native window. No reservation/native/compiler/Wine/VM job here. Run the frozen
worker once with strict-single BEFORE repeat, qualify paired raw fields, promptly
release, archive original receipts before AFTER-capture interpretation. Existing
width/shifted/expression captures stay complete, all parent Numeric/conversion/
division/ListItem implementation and acceptance remain OPEN. No owner-input
blocker, production formatter/helper/hook/expected emitter or PR created.

### Original D13 decimal-metadata13/18 native pair and offline analysis qualified

Clean signed/DCO/G source freeze26407441cae30774f2b75ee381edf60a3b4c6c0f
executed SAME frozen caller/reader/worker, ONCE, 2026-10-19T18:45:33..18:45:40Z.
NEW audit+DAW explicit native-only CLEAR, Request Guard released/no conflicting
heavy window; actual tool snapshots retained. Earlier HOLD is not erased or
treated as permission. No compiler/newFLL/VM: reused verified observer, inherited
overrides UNSET and process-local Microsoft32-only runtime binding.
First0/strict-single0 BEFORE repeat/repeat0/paired0, all stderr empty;16 records/
all42 raw fields byte-identical, inactive-slot differences0. Both raw SHA256
ec82060851762e0314f7b08434c170e89f2acbcbcf2b1fb9946d15bac812148d.
Allseven preparatory/native/reader children prefix1/fullPSIavg10=0.00/avg60<=0.06%/
guard0/nice-n10/input hashes BEFORE+AFTER EACH0. Original/public ROUND controls,
target active metadata/payload before+after and exact runtime controls pass.
Combined inspection shell exit1 is FINAL idle-prefix pgrep, not native/reader
failure (allseven actual child status files0). Promptly RELEASED allthree
coordinators, audit acknowledgement; no active native job/reservation.

Validated/moved root to build/default-transform-original-scale-consumer-captures1;
worker root build/default-transform-original-scale-consumer ABSENT. Archive
original-scale-consumer-captures1 initially100 native gzip receipts, includes
own authored PRG/FXP, frozen source/control/status/gate/hash/raw/runtime/actual
shell/inspection and clearance/release snapshots; no proprietary SDK/FLL binary.
Compressed/decoded hashes/sizes/exact host bytes verified before interpretation.
Ordinary shell is NOT REA Evidence/Windows parity/internal-call/atomic mapped-code
or descendant-cleanup attestation; no earlier environment is retroqualified.

AFTER-capture analyze-original-scale-consumer.cjs binds exact native freeze/raw
digest and all16 raw42-field/active metadata/control projections. Eight metric
self-checks keep padding/literal/exact decimal-value/zero-sign differences apart;
no native target strings inferred or used as expectations before capture.
NEW build/default-transform-original-scale-analysis1: frozen archived reader0/
empty/report byte-exact to native audit, syntax0/analysis0/readback0/empty/
byte-identical, eight metric tests; hashes BEFORE+AFTER EACH0/prefix1/fullPSI0/0/
guard0/nice10. BEFORE base26407441c+dirty AFTER-capture source/archive status is
NOT native freeze. NEW original-scale-analysis-replay1 compiles gzip-archived
analyzer at original module path after explicit archived/current source byte
identity:0/empty/byte-exact/eight tests/input hashes before+after0/prefix1/fullPSI
0/0/guard0/nice10.43 analysis+14 replay receipts bring total157 gzip files; all
three inventories/checksum lists/compressed+decoded sizes/hashes/host bytes pass.
All host roots and earlier source packaging failure/correction receipts intact.

FINITE results (four signed ORIGINAL D13 payloads, metadata13/18, width40):
8 metadata interventions change TF literal/trimmed/exact decimal value4 times,
all FIXED OFF; ON changes0 and zero-sign changes0. STR changes0/8 for every
metric. At metadata13 both signed adjacent pairs change TF+STR value2/2 under
OFF and ON. At metadata18 STR still changes2/2 both modes, TF changes0/2 OFF
and2/2 ON. Across16 target TFversusSTR:16 literal/8 trimmed/4 exact decimal-value
gaps (all OFF at metadata18) and3 both-zero sign-spelling gaps (negative lower
endpoint165:OFF+ON at metadata13, ON at metadata18). This measured intervention
supports value-associated decimal-metadata relevance to these default displays;
it does NOT establish a universal scale formula, exact formatter internals,
rounding/precision order, conversion/propagation rule, or general parity.

LOW DQ-DV001 walkthrough extends source qualification with actual fresh resource
clearance/strict-first order/all42 raw field equality/runtime binding, distinct
metadata-only comparisons and8 exact-decimal metric tests, frozen-reader and
archived-analyzer byte-exact replays, prompt release and preserved failures.
Developer-maintainer self-review only, NOT independent human/system/release
assurance. Existing hazards/mitigations/rollback/field-notification boundaries
unchanged. No expected emitter, tolerance/canonical-double shortcut, production
change, PR or acceptance/parent closure. Division ORIGINAL/SAME remains intact.

NEXT bounded SOURCE discriminator: intermediate decimal metadata14/15/16/17 at
fixedwidth40 on SAME ORIGINAL signed D13 witnesses159/160/165/166/bothFIXED,
16routes/32records/42fields. Existing13/18 captures remain complete, not rerun.
Fresh inline target reconstruction at every consumer, original N40/18/public
inline ROUND controls before+after, independent literal/raw-parent reader and
no target Character predictions. This brackets the observed metadata-sensitive
OFF display without assuming a formula or fitting a threshold. Qualify source/
generation/synthetic reader+guarded-worker/archive replays and clean signed
freeze BEFORE any NEW native resource coordination. No compiler/newFLL/VM needed.
Wider Numeric representation/conversion/display/division/ListItem acceptance OPEN.

### Intermediate D13 decimal metadata14..17 source qualification — native UNRUN

Bounded derived RQ/VR-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001 continuation of the
observed13/18 display intervention: emit-original-scale-middle-consumer.cjs
generates original-scale-middle-consumer.prg/cases.json. Four ORIGINAL signed
payloads parent159/160/165/166, D13 and width40 fixed, metadata14/15/16/17 ONLY,
bothFIXED:16 routes/32 records/42 fields. No13/18 or completed native rerun.
Every target consumer uses fresh inline CFNDEC(CFNWIDTH(targetPayload,40),scale);
no numeric argument/result assignment, shifted arithmetic or sign-copy. Original
N40/18 and public inline ROUND controls before+after remain independently bound
to eight parent raw observations/digest254b46da...; no target strings predicted.

Independent audit-original-scale-middle-consumer.cjs uses literal seeds and
binds source/all32 expected routes/42 raw fields/active metadata/payload/settings/
runtime/before+after/repeat controls. check-original-scale-middle-reader.cjs
passes54 SYNTHETIC tests (allfour valid target-scale swaps at both observations,
wrong width/record count, field/control/status/stderr/runtime/repeat mutations,
baseline/single/paired and inactive Numeric integer-slot-only acceptance).
These arbitrary synthetic target strings are NOT native expected text.
check-original-scale-middle-worker.cjs passes13 harmless SYNTHETIC extracted-
checked() tests, fake prefix/pressure and harmless children; never the whole
native worker or native paths, and not actual resource clearance evidence.
Native worker original-scale-middle-consumer-capture.sh preserves clean-tree/
new-root/override/source hashes/every-child idle/fullPSI/nice gates and strict
single-reader BEFORE repeat. Resource flag acknowledges external coordination,
never grants it. No compiler/newFLL/VM needed, existing observer reused later.

Actual NEW host original-scale-middle-generate1:2 stages0/empty. First
original-scale-middle-source-checks1:11 stages0/empty, synthetic-reader fails1
at stage12 because wrong-count mutation still searched for old DONE|16 in the
new DONE|32 baseline, changing nothing. Failure/stderr/BEFORE source and input
snapshots/actual shell retained; no failed-stage AFTER hashes or worker stage
claimed. This is an own test-porting error, not a native/production defect.
Correction uses DONE|32->DONE|31 plus explicit must-change assertion. NEW host
original-scale-middle-source-checks2:all13 stages0/empty/regeneration byte-exact,
54reader+13worker checks pass/input hashes BEFORE+AFTER EACH0. Generation and
initial11 successful stages also retain before+after checks; immediate prefix1/
fullPSIavg10<=0.19/avg60<=0.20%/guard0/nice-n10 across all attempted stages.
BEFORE base62b6a90b8+dirty snapshots are NOT native freeze; generation/failed
pass retain old test source intentionally, successful pass binds corrected source.

original-scale-middle-source-checks1 archive276 gzip plus manifest/SHA256SUMS
preserves allthree host roots and actual commands/completions. NEW host
original-scale-middle-source-replay1 compiles gzip-archived successful-pass
sources at original module paths with explicit archived/current byte identity:
seven stages0/empty/byte-exact generation/source-reader/54reader+13worker outputs,
original/archive hashes BEFORE+AFTER EACH0/prefix1/fullPSIavg10<=0.42/avg60<=0.21%/
guard0/nice10. Archived shell sources syntax ONLY.83 supplementary receipts
bring total359 gzip files; both inventories/checksum lists/compressed+decoded
sizes/hashes/exact host bytes verified. All host roots/failure snapshots intact;
no SDK/FLL binary. Ordinary shell is NOT REA Evidence/native results/Windows/
internal-call/atomic mapped-code or descendant-cleanup attestation.

LOW DQ-DV001 procedural delta: only new intermediate metadata stimuli/counts,
independent reader and new-root workers. Actual walkthrough: source generation/
regeneration/source-control binding, retained inert-mutation failure/correction,
54reader/13worker tests, byte-exact archived source replay and all receipt hashes.
Developer-maintainer self-review only, not independent human/system/release
assurance. HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 mitigations
unchanged: never mistake synthetic text or fields alone for parity, never weaken
ORIGINAL/SAME/control assertions or use guessed tolerance. Rollback: withdraw
this diagnostic source, retain receipts and correct trace/handoff before a run;
no runtime release or field notification needed for source-only checkpoint.

NEXT clean signed source freeze then NEW explicit audit+DAW native-only CLEAR,
Request Guard renewed two-VM window RELEASE/no conflict, fresh idle/fullPSIavg10
ANDavg60<1% EVERY child. Audit19:08UTC HOLD15.07/46.44% is resource-state data;
later passing small offline gates do NOT grant native permission. No native/
compiler/Wine/VM window requested or launched here, no reservation/job. SAME
frozen worker ONCE, strict-single BEFORE repeat, paired qualification, prompt
release and archive BEFORE AFTER-capture interpretation. No target display rule,
formatter/helper/hook/expected emitter, production change/PR/parent closure or
owner-input pause. Exact conversion/display/division/ListItem acceptance OPEN.

### Intermediate14..17 first native attempt — pressure stop, NOT qualified

Owner directly restarted the heartbeat on2026-10-20. Live admitted parents
#5611/#6776/#7189 still OPEN/owner/exact agent-approved; derived tracking#7210
unchanged. SAME clean signed/DCO/G source freeze
a8f46bbf80c83d8859353c8d0f81ec5af5130007, no source modification/native rerun
of any completed checkpoint. NEW audit+DAW explicit native-only CLEAR and
Request Guard released/no conflict, actual request/snapshot/message receipts
retained. Audit limited concurrent work to remote CI-log review; no local
conflicting launch was admitted. Reused observer/process-local Microsoft32
binding/inherited overridesUNSET; no compiler/newFLL/VM/reboot/backup monitoring.

Frozen original-scale-middle-consumer-capture.sh ran ONCE. Three preparatory
stages0/empty, first native0/empty; their immediate prefix1/fullPSI0.00/0.01%/
guard0/nice-n10/input hashes BEFORE+AFTER EACH0 retained. BEFORE first-structure
reader: prefix1 but full I/O PSIavg10/avg60=1.48/0.33%, guard75 and workerexit75.
The strict-single reader DID NOT launch; no reader status/JSON/AFTER hashes,
repeat or paired audit exists. No paired qualification, target comparison,
metadata conclusion or compatibility result is claimed from this attempt.
First raw report/out/result files match, observed terminator DONE|32, SHA256
aa6b7f0675cd237c89c0719abce44ea693e90efdc3fa9a92c292a7472a693a43.
Basic preservation/terminator checks are NOT the unrun independent reader.
The pressure guard is a resource stop, not a native/production defect or a
reason to weaken the gate, bypass first-reader order or invent expected text.

Promptly RELEASED allthree resource owners; audit acknowledged no capacity/
no paired qualification. Request Guard subsequently reported a modest local
test window; future native/compiler work must coordinate NEW availability.
Post-stop prefix idle; combined inspection shell exit1 is its FINAL idle-prefix
pgrep, not a failed native child. No active own native job/reservation.
After fresh packaging gates, validated/moved partial root to
build/default-transform-original-scale-middle-consumer-guard1; original worker
root now ABSENT, no report overwritten. New archive original-scale-middle-
consumer-guard1 contains73 gzip receipts plus manifest/SHA256SUMS: exact source/
PRG/ownFXP/raw/status/gates/hashes/actual shell/inspection/coordination/releases.
Every compressed/decoded size/hash/exact host byte verified. No SDK/FLL binary.
Ordinary shell is NOT REA Evidence, Windows/internal-call/atomic-image or
descendant-cleanup attestation. Earlier source359 receipts/test correction and
Oct19 coordination-only deferral root remain intact, not retroqualified.

LOW DQ-DV001 actual walkthrough: new explicit clearance, unchanged clean source,
three preparatory passes/native0, preserved first raw bytes, demonstrated
fail-closed75 BEFORE strict reader/repeat, prompt release, partial-not-qualified
manifest and byte-verified archive. Developer-maintainer self-review only,
not independent human/system/release assurance. Existing HZ-data-corruption-01/
HZ-runtime-crash-01/HZ-doc-command-01 mitigations, rollback/notification and
ORIGINAL/SAME preservation remain unchanged. No behavior shipped/parent closure.

NEXT only after pressure block actually clears AND NEW audit+DAW native-only
CLEAR plus Request Guard current no-conflict/release: fresh idle/full I/O PSI
avg10 ANDavg60<1% EACH child. SAME caller/reader/worker/source bytes (bind to
a8f46bbf8) at a new clean signed evidence head may run a NEW full pair at the
now-absent original working root. Preserve guard1 intact; the unqualified first
attempt is not a completed native pair. Strict-single MUST pass BEFORE repeat,
then paired raw qualification/prompt release/archive BEFORE AFTER-capture
analysis. Do not reuse old clearances or interpret this partial report instead.
No new compiler/FLL/VM needed. No owner-input blocker or automation pause.

### Intermediate14..17 NEW native pair qualified — earlier guard1 unchanged

After the pressure block cleared (fresh fullPSI0.00/0.00%), NEW explicit audit+
DAW native-only CLEAR and Request Guard current no-conflict CLEAR obtained.
Actual resource requests/snapshots retained; no prior clearance reused. SAME
worker executed ONCE18:14:25..18:14:32UTC2026-10-20 from clean signed/DCO/G
evidence head daf531078fccbed0208a563a2a5d869fc671649e. All nine snapshotted
caller/reader/helper/input source files explicitly compare byte-for-byte to
source freeze a8f46bbf80c83d8859353c8d0f81ec5af5130007 via git-show and current
files; exact SHA/source-byte identity receipt retained. No caller/reader/worker
source change, compiler/newFLL/VM/reboot/backup monitoring. Observer reused;
process-local Microsoft32 binding/inherited overridesUNSET retained.

First0/strict-single0 BEFORE repeat/repeat0/paired0; allseven preparatory/native/
reader children0/empty/prefix1/fullPSI0.00/0.00%/guard0/nice-n10/input hashes
BEFORE+AFTER EACH0.32 records/all42 raw fields repeat-identical; inactive Numeric
integer-slot differences0. Parent original/public inline ROUND controls, active
target metadata/payload/settings/runtime before+after checks pass. Both raw SHA
aa6b7f0675cd237c89c0719abce44ea693e90efdc3fa9a92c292a7472a693a43.
This happens to match the earlier partial first raw digest; guard1 remains
SEPARATELY UNQUALIFIED, with its pre-reader75/no-repeat state intact. No
environment, clearance, reader execution or pair is retroactively claimed.

Promptly RELEASED allthree resource owners, audit/Request Guard acknowledgements;
post-prefix idle/fullPSI0/0/no active own job/reservation. Combined inspection
shell exit1 is FINAL idle-prefix pgrep, not child failure. Validated/moved NEW
complete root to build/default-transform-original-scale-middle-consumer-captures1;
original worker root ABSENT, prior consumer-guard1 preserved separately.
Archive original-scale-middle-consumer-captures1 initially101 gzip files includes
own authored PRG/FXP/source/raw/status/runtime/gate/hash/full actual shell/
inspection/clearance/release/source-byte-identity receipts. No SDK/FLL binary.
All compressed/decoded sizes/hashes/exact host bytes verified before analysis.

NEW build/default-transform-original-scale-middle-reader-replay1 compiles the
gzip-archived frozen native reader at original module path with explicit current/
archived byte identity:0/empty/report byte-exact to captured native audit,
input hashes BEFORE+AFTER0/prefix1/fullPSI0/0/guard0/nice10. BEFORE base daf531078
plus dirty archive/status is an AFTER-capture verification snapshot, not native
freeze.15 reader-replay receipts bring total116 gzip files; both manifests/
checksum lists/compressed+decoded sizes/hashes/exact host bytes verified. Source
359 receipts/test correction, guard1's73 receipts and coordination1 remain intact.
Ordinary shell is NOT REA Evidence/Windows/internal-call/atomic mapped-code or
descendant-cleanup attestation. No full formatting/propagation/conversion parity.

LOW DQ-DV001 actual walkthrough: fresh resource coordination/clean unchanged
source, strict-single BEFORE repeat, all32/42 raw equality and independent
metadata/control/runtime checks, prompt release, separately preserved failure,
source-byte identity, frozen-reader replay and all receipt-byte checks.
Developer-maintainer self-review only, NOT independent human/system/release
assurance. Existing hazard/mitigation/rollback/notification and ORIGINAL/SAME
assertion-preservation boundaries unchanged. No production formatter/helper/
hook/expected emitter, target metrics, tolerance/canonical-double shortcut,
PR or parent acceptance/closure inferred from native qualification alone.

NEXT bounded AFTER-capture source: analyze-original-scale-middle-consumer.cjs,
bind NEW manifest source daf531078/source-byte freeze a8f46bbf8 and exact raw
digest, all32 raw42-field/control/active metadata projections. Compare24 adjacent
metadata14->15->16->17 interventions,16 signed adjacent-payload pairs across
bothFIXED/allfour scales,32 target TFversusSTR contexts. Keep literal/padding/
exact decimal value/zero-sign separate with independent metric self-checks; no
guessed target strings or universal formula. Frozen reader/syntax/analysis/
readback/gzip-archived analyzer replay and immutable input hashes/gates must
actually pass and archive before interpreting metrics/selecting another stimulus.
No native rerun/compiler/VM needed. Exact display/conversion/propagation,
division ORIGINAL/SAME/hosted/review and ListItem acceptance remain OPEN.

### Intermediate14..17 AFTER-capture analysis — 2026-10-20

Completed analyze-original-scale-middle-consumer.cjs from evidence HEAD
b3f3538557912af9d98b2db87ab496b8c6ed1eae (AFTER-capture dirty source snapshot,
not a native source freeze). It binds the daf531078 native manifest, a8f46bbf8
source-byte freeze, exact aa6b7f0675cd237c89c0719abce44ea693e90efdc3fa9a92c292a7472a693a43
raw digest, all32 records/42-field projections and active metadata/control tuples.
Host build/default-transform-original-scale-middle-analysis1: frozen archived
reader/syntax/analysis/independent readback all0/empty stderr, report byte-exact;
eight independent decimal-value/zero-sign metric checks. Input hashes BEFORE and
AFTER EACH0, immediate idleprefix1/full I/O PSI avg10+avg60=0/0, nice-n10.
Archived analyzer replay at original module path explicitly matches current
source bytes and produces byte-exact report, status0/empty/hashes before+after0,
same fresh gates. Its host root is original-scale-middle-analysis-replay1.
42 analysis+13 replay receipts supplement116 native/reader receipts:171 total
gzip receipts, with separate manifests/checksum lists. New compressed/decoded
sizes/hashes and exact host bytes checked; earlier source359/guard73 untouched.

Observed finite sample:24 adjacent metadata interventions14->15->16->17 change
12 TRANSFORM literal/trimmed strings but ZERO exact decimal values/zero signs;
STR changes0/24. Across16 signed adjacent payload pairs TRANSFORM distinguishes
both pairs under FIXED ON at every scale, neither under OFF; STR distinguishes
both under both modes at every scale.32 TF-versus-STR contexts have32 literal,
20 trimmed-literal,16 exact-value gaps and4 zero-sign gaps, separately counted.
No precision/cap/tolerance/formatter rule inferred from finite equalities.

LOW DQ-DV001 actual developer walkthrough: inspect all raw projections, enforce
metadata-only/payload adjacency invariants, independent exact-decimal metrics,
source-bound frozen-reader and analyzer replays, per-child gates/hash integrity,
byte-exact readback and archived/host bytes. Developer self-review only, NOT
independent human/system/release assurance. No native/compiler/VM invoked,
production formatter/helper/hook/expected emitter or compatibility acceptance.

NEXT bounded offline discriminator: index-exact join of completed original
metadata13/18 and14..17 captures for the SAME four payloads/D13/width40/bothFIXED.
Validate shared parent/control/runtime identities across captures BEFORE joining;
retain separate capture provenance, test all five consecutive metadata edges,
classify literal/padding/value/zero-sign changes without guessing a general rule.
No native rerun required. Exact conversion/display/propagation, unchanged division
ORIGINAL/SAME/hosted/review and ListItem acceptance remain OPEN.

### Broader528 retained spelling test — owner-directed, 2026-10-20

check-original-broad-spelling-hypothesis.cjs preserves the narrow candidate's
rendering body unchanged; only validation admits observed settings0..18 and
metadata18. Original str-fixed raw89a420760a2332430158427778908a952d84966a6d4eee14f090c1a9a6332192,
all528 source/index/FIXED/payload/N40/18/current-setting tuples validated.
OFF uses ACTUALstr18Hex; ON ACTUALoriginalStrHex/current-D. All528 exact native
TFhex comparisons retained, with complete observations and mismatch array.
Thirty independent boundary/rendering tests retained; no tolerance/cap or
weakened exact assertion. No original division assertion altered.

Host original-broad-spelling-hypothesis1: frozen native reader/source identity/
byte-exact audit and gcvt parent graph, syntax, analysis, independent readback,
gzip-frozen original-path source replay all0/empty/byte-exact. Immutable hashes
BEFORE+AFTER EACH0/idleprefix1/BOTH fullPSI<1%/nice10.52 source/status/gate/hash/
actualcommand/report gzip receipts at original-broad-spelling-hypothesis-captures1,
compressed/decoded sizes/hashes/exacthostbytes verified before interpretation.
Outcome528 exact matches/0 mismatches (264 OFF/264 ON). Existing48 and earlier
failed hypotheses untouched. Base62312ee35+dirty AFTER-capture, not native freeze.

This broader exact consumer/rendering relation still takes actual native STR
digits as INPUT. It is NOT an implemented numeric digit-conversion algorithm,
universal formatter/internal call graph, or production/compatibility acceptance.
Exact numeric conversion recovery and unchanged division ORIGINAL/SAME/hosted/
review/ListItem remain OPEN. LOW DQ-DV001 maintainer self-review/walkthrough:
inspect unchanged rendering body, broader validator-only delta, exact native
reader/parent identity, full528 coverage,30 tests, per-child gates/hash/status,
exacthex mismatch retention and archive bytes; not independent human/system
assurance. Existing hazards/rollback/notification remain unchanged.

Direct owner steering additionally admits CodeQL alert#1 remediation, tracking
issue#7214; Numeric checkpoint preserved and temporarily deferred while that
bounded fix proceeds. Dedicated branch fix/codeql-zip-slip-7214 from main240ac.
Numeric re-entry: recover actual public STR digit conversion against the retained
native-only CRT/count evidence; do not substitute these observed STR inputs for
a Copperfin implementation or relax ORIGINAL. No native/compiler/VM run here.

### Unwired selected-consumer spelling hypothesis — 2026-10-20

check-original-setting-spelling-hypothesis.cjs binds the complete48 selected
classification report by manifest digest/source/count/compound identities.
Diagnostic candidate only: trim observed STR leading padding; selected zero OFF
becomes bare0 with raw-input sign, zero ON keeps observed fractions/exponent
spelling with raw-input sign; nonzero observed STR body unchanged. Finite input
payload/sign/count/context/width validation, no binary floating conversion or
guessed target values. Thirty independent positive/negative zero/nonzero/
fraction/exponent and invalid text/nonfinite payload/context/count/width/sign
tests; candidate expected cases synthetic, NOT native compatibility assertions.
Complete48 exact observedTF versus candidate hex comparisons retain every
mismatch rather than discarding it or weakening ORIGINAL assertions.

Host original-setting-spelling-hypothesis1 five guarded stages0/empty: frozen
parent classifier/current-byte identity/byte-exact report, syntax, candidate
analysis, independent readback and gzip-frozen source original-path replay.
Reports byte-identical/immutable hashes BEFORE+AFTER EACH0/idleprefix1/fullPSI
0/0/nice-n10.52 gzip source/status/gate/hash/actualcommand/report receipts at
original-setting-spelling-hypothesis-captures1, compressed/decoded size/hash/
exacthostbytes verified before interpretation. Base79d50b749+dirty snapshot is
AFTER-capture, not native freeze. Prior inputs/failures untouched.

Finite outcome48 exactTFhex matches/0 mismatches. This establishes ONLY the
tested rendering relation over these already-observed public STR inputs, NOT
how Copperfin should compute numeric decimal digits, an internal native call
graph or general default formatter. No production helper/hook/expected emitter,
numeric-conversion algorithm, tolerance/cap or compatibility acceptance.

LOW DQ-DV001 actual developer walkthrough: validate native-source/context/count
identity,30 independent candidate/boundary tests, explicit raw sign for rounded
zero versus observed nonzero sign, full48 exacthex comparisons/mismatch retention,
per-child status/resource/hash gates, source/readback/archive bytes. Maintainer
self-review only, not independent human/system/release assurance; existing
hazards/rollback/notification/ORIGINAL boundaries retained. No native/compiler/VM.

NEXT bounded broader retained-oracle test: original528-context str-fixed pair,
original Numeric metadata18. OFF selects actual public STR18; ON selects actual
current-D originalStrHex. Bind current native raw/audit/runtime/meta identities
and original control graph before selection; reuse UNCHANGED raw-sign/zero-
rendering logic (only validation adapts to admitted broader observed counts).
All528 exact TF hex comparisons with complete per-index/count/source provenance,
retain every mismatch and do not fit a count cap or tolerance. First frozen
native reader/source/parent replay, independent metric/boundary tests, gated
analysis/readback/gzip-source replay and archives. This is broader value/payload/
setting evidence, not repeating48 or completed STR count-tuple sufficiency tests.
No new native stimulus required. Exact numeric conversion/default formatting/
propagation, division ORIGINAL/SAME/hosted/review and ListItem acceptance OPEN.

### Selected-consumer exact spelling classification — 2026-10-20

analyze-original-setting-spelling.cjs binds the144-comparison report by manifest
digest, validates all selector/source/count/payload identities and complete48
parent/FIXED/D/metadata coverage. Select ON same-setting STR and OFF metadata-
count STR; recompute all144 exact decimal/literal/trimmed/zero-sign metrics,
not merely the selected aggregate. Raw text/profiles preserve leading spaces,
sign/fraction/trailing-zero/exponent spellings and input payload sign. Fourteen
independent profile/metric tests and six duplicate/missing/damaged identity/count/
selector rejection tests; no target normalization or replacement strings.

Host original-setting-spelling1 five guarded stages0/empty: frozen parent cross-
consumer replay/current-source identity/byte-exact report, syntax, classification,
independent readback and gzip-frozen source original-path replay. Reports byte-
identical/input hashes BEFORE+AFTER EACH0/idleprefix1/fullPSI0/0/nice-n10.
52 gzip source/status/gate/hash/actualcommand/report receipts at original-setting-
spelling-captures1, compressed/decoded sizes/hashes/exacthostbytes verified before
interpreting. Basebec48f066+dirty AFTER-capture snapshot not native freeze.
Existing captures/failures intact; no native/compiler/VM/production behavior.

Observed48 selected pairs:36 padding-only,12 zero-spelling,0 different-value,
0 equal-nonzero-different-spelling. OFF24:18 padding-only/six zero-spelling/three
zero-sign gaps; ON24:18 padding-only/six zero-spelling/six zero-sign gaps. All
literal pairs differ due STR width40 padding; all12 trimmed differences are
zero. OFF TF is bare0 or-0 versus fixed fractional STR zero (six contexts);
ON TF retains fractional digits and negative input sign while STR zero is
unsigned (six contexts). Nine sign gaps correspond to negative input payloads;
three positive OFF zeros differ only by fractional zero spelling. This is raw
finite evidence, not a universal zero/rounding/default formatter rule.

LOW DQ-DV001 actual developer walkthrough: select one actual STR observation per
complete context, reject wrong counts/source identities/duplicates/missing rows,
recompute parent metrics, inspect raw12 zero cases and input sign, distinguish
padding/value/zero sign/fraction spellings, gate/hash/status/byte-exact source
readback and archive verification. Maintainer self-review only, not independent
human/system/release assurance. Existing hazards/rollback/notification and exact
ORIGINAL assertion boundaries retained; no compatibility acceptance.

NEXT bounded unwired spelling hypothesis test from observed48: remove public
STR leading padding; for a numerically zero selected result, OFF emits bare0
with raw-input negative sign, ON preserves its fractional zeros with raw-input
negative sign. Preserve nonzero observed STR body. Test explicit positive/
negative zero and nonzero, fraction/exponent spelling and invalid count/context
boundaries independently, then compare ALL48 exact TF hex strings with source/
count provenance; retain every mismatch. Do not treat selected public STR values
as a newly recovered numeric conversion algorithm, hook production, weaken exact
ORIGINAL tests or claim universal parity. Gated source/readback/gzip-replay/
archive first, then seek broader retained-oracle count/payload coverage before
any new native stimulus. No native rerun needed for this hypothesis test.
Exact propagation/conversion/formatter implementation, division acceptance and
ListItem supplied-column READ remain OPEN.

### Observed STR cross-consumer discriminator — 2026-10-20

compare-original-setting-str.cjs binds allthree source/raw+analysis manifests
and the completed D12/13/14 join report digest/projections. Same48-context grid,
each TF compared with actual retained public STR counts12/13/14 at SAME parent/
metadata/FIXED=144 comparisons. Shared original payload/active metadata/runtime
checked; across-D ROUND/control changes remain observations. STR provenance
retains capture/index/sourceD/publicCount, not fabricated target strings.
Eight metric+six join+two independent count-selector tests.48 same-setting and
48 metadata-count selections asserted, including their intentional overlap.

Host original-setting-str1 eight guarded stages0/empty: three frozen analyzers+
parent join byte-exact replays/current-byte identity, syntax, new analysis,
independent readback and gzip-frozen source replay at original module path.
Reports byte-identical/immutable hashes BEFORE+AFTER EACH0/idleprefix1/fullPSI
both thresholds pass/nice-n10.82 source/status/gate/hash/fullcommand+completion/
report gzip receipts at original-setting-str-captures1, all compressed/decoded
sizes/hashes/exacthostbytes verified before interpretation. Base7a14bff46+dirty
AFTER-capture snapshots not native freeze. Existing source/capture failures intact.

Finite observations: all144 comparisons144 literal/108 trimmed/82 exact-value/
16 zero-sign gaps. Same-setting count48 comparisons48literal/24trimmed/14value/
eight zero-sign gaps; metadata-count48 comparisons48literal/23trimmed/14value/
five zero-sign gaps. ByFIXED: OFF metadata-count24 comparisons0 value gaps but
six trimmed/three zero-sign gaps; ON same-setting24 comparisons0 value gaps but
six trimmed/six zero-sign gaps. The opposite count selectors each have14 value
gaps. Thus selecting OFF metadata-count/ON same-setting count has zero observed
decimal-value gaps across48, but twelve trimmed-spelling and nine zero-sign gaps
remain. This finite observed consumer relation is NOT an internal call graph,
general formatter/rounding/conversion policy or implementation acceptance.

LOW DQ-DV001 actual developer walkthrough: rebind source/parent-join digests,
compare complete compound-grid projections, verify public count provenance,
explicit selector overlap/count tests, exact decimal/zero metrics, reject damaged
shared identity while allowing across-D controls, actual per-child gates/hash/
status, source/report replay and archive bytes. Maintainer self-review only, not
independent human/system/release assurance. No native/compiler/VM/production
change; existing hazards/rollback/notification/ORIGINAL boundaries retained.

NEXT bounded offline selected-consumer spelling recovery: choose ON same-D STR
and OFF metadata-count STR from these actual144 comparisons, assert48-context
coverage and source/count identity, inspect/classify all twelve trimmed-text
differences separately from leading padding and nine zero-sign differences.
Preserve raw signed-zero/width/decimal spelling and exact decimal-value evidence;
do not guess a normalization from aggregates, relax exact ORIGINAL assertions,
or call finite numeric equality parity. Author source/metric/rejection tests,
gated analysis/readback/gzip-source replay and verified archive BEFORE adopting
any spelling hypothesis or choosing a further native stimulus. No native rerun
needed. Exact propagation/conversion/formatter implementation, division
ORIGINAL/SAME/hosted/review and ListItem acceptance remain OPEN.

### Index-exact D12/13/14 join — 2026-10-20

join-original-setting-consumers.cjs binds three distinct native source/raw and
AFTER-capture analysis manifests: original-scale D13/scale13, original-scale-
middle D13/scale14, original-setting D12+14/scales13+14. Original analyzers
validate every source observation BEFORE selection;48 compound parent/FIXED/D/
metadata keys join.48 same-D control and shared original-payload/N40/18/runtime/
target identity comparisons pass. Across-D original/ROUND strings/payloads are
retained as observations, NOT assumed equal.24 metadata edges/32 consecutive
D edges/24 signed adjacent pairs/48 TFvsSTR contexts; eight decimal metric+six
join misuse checks (duplicate/control/runtime/width/payload rejection, changed-D
control acceptance without relaxing shared identity).

Host original-setting-join1 seven guarded stages0/empty: allthree frozen source
analyzers/current-byte identity/byte-exact old reports, syntax, join, independent
readback and gzip-frozen join replay at original module path. Reports byte-
identical/input hashes BEFORE+AFTER EACH0/idleprefix1/BOTH fullPSI<1%/nice-n10.
72 gzip source/status/gates/hash/fullcommand+completion/report receipts at
original-setting-join-captures1; manifest/checksumlist/compressed+decoded sizes/
hashes/exact hostbytes verified BEFORE interpreting metrics. Basefb92a017c+
dirty AFTER-capture source snapshots NOT native freeze. Existing inputs intact.

Finite observations: eachD metadata13->14 changes4 TF values(allOFF)/0 STR.
D12->13:8 TF spelling/4 value changes,16 STR spelling/8 value changes;
D13->14:8 TF spelling/8 value changes,16 STR spelling/value changes. No edge
changes zero sign. Adjacent FIXED ON distinctions2/2 at bothmetadata ONLY D13,
0/2 at D12/14. OFF TF2/2 at metadata13 and0/2 at14 for allD; STR2/2 bothmetadata/
bothFIXED onlyD13,0/2 D12/14.48 TFvsSTR contexts48literal/24trimmed/14value/eight
zero-sign gaps separate. No fitted tolerance/cap or universal formatter rule.

LOW DQ-DV001 actual developer walkthrough: immutable per-source replay before
selection, full compound grid and source/raw provenance, same-D control versus
across-D observation policy, six independent join misuse checks, eight metric
tests, per-child status/resource/hash receipts, byte-exact archived-source
readback and receipt verification. Maintainer self-review only, NOT independent
human/system/release assurance. Existing hazard/rollback/notification and
unchanged ORIGINAL assertions retained; no native/compiler/VM/production change.

NEXT bounded offline cross-consumer discriminator using this SAME48-context
grid: for each TRANSFORM output, join actual retained public STR outputs at
counts12/13/14 for SAME parent/metadata/FIXED (144 comparisons). Preserve the
STR observation's sourceD/count provenance; check payload/active metadata and
shared runtime identity before comparing. Classify same-setting count versus
metadata-count exact values/spelling/padding/zero sign, without treating either
candidate as an internal call graph or universal formatting algorithm. This
uses observed cross-setting STR counts, not a new native probe or the already
completed same-D TFvsSTR comparisons alone. Source/replay/metric/hash gates and
archival must pass before interpreting. Exact conversion/display/propagation,
division ORIGINAL/SAME/hosted/review and ListItem acceptance remain OPEN.

### DECIMALS12/14 AFTER-capture analysis — 2026-10-20

analyze-original-setting-consumer.cjs binds native freeze886b8720d and exact
47232c78c28f7b7536b654b3c85fd94bd7a6eead587f719298f80762f02c2499
raw digest, all32raw42-field projections, including newly observed active
original/ROUND control tuple/metadata/payload. Unique parent/FIXED/D/scale grid,
16 metadata13->14 interventions,16 D12->14 interventions,16 signed adjacent
pairs and32 TFvsSTR contexts. Metadata comparisons require same-D controls;
setting comparisons preserve changed original/ROUND observations without
requiring control equality across D. Eight independent decimal/zero-sign tests.

Host original-setting-analysis1 frozen native-reader/syntax/analysis/independent
readback0/empty, audit/report byte-exact; original-setting-analysis-replay1 gzip-
frozen analyzer at original module path/current-byte identity0/empty/report
byte-exact. Immutable hashes BEFORE+AFTER EACH0/immediate idleprefix1/BOTH full
I/O PSI<1%/nice-n10.42+13 compressed/decoded-size/hash/exact-host-byte-verified
receipts supplement114 native/reader receipts ->169 total, with separate
manifests/checksumlists. Base256693f35+dirty AFTER-capture snapshots not native
freeze; earlier source100+283 and all failed/older captures unchanged.

Observed finite results: metadata changes affect8 TF literal/trimmed/value
comparisons,0 STR/zero-sign; setting changes affect8 TF literal/trimmed/value
and16 STR literal/trimmed/value comparisons,0 zero-sign. Signed adjacent TF
distinguishes2/2 OFF at scale13 at bothD,0/2 OFF at scale14 and0/2 ON at both
scales/D. STR distinguishes0/2 throughout.32 TFvsSTR contexts retain32 literal/
16 trimmed/10 exact-value/five zero-sign gaps separately. No general rounding/
metadata/setting formula, fitted cap/tolerance, formatter or parity acceptance.

LOW DQ-DV001 actual maintainer walkthrough: inspect complete compound-key grid,
all raw projections/active control observations, distinguish same-D control
invariance from across-D observation, exact metric tests, per-child status/gates/
hashes, byte-identical reader/analysis/replay and receipt bytes. Developer
self-review only, not independent human/system/release assurance. No native/
compiler/VM/production change; existing hazards/rollback/notification and
unchanged ORIGINAL assertion boundaries retained.

NEXT bounded offline index-exact join of retained D12/D14 setting capture with
D13 scale13 from original-scale and scale14 from original-scale-middle captures:
four payloads x3D x2metadata x2FIXED=48 contexts. Bind distinct source/raw
provenance and shared payload/active metadata/runtime; SAME-D controls must
match, across-D ROUND/control values need NOT. Classify D12->13/13->14 setting
edges, metadata13->14 and signed adjacent distinctions with separate spelling/
padding/value/zero-sign metrics. This is NOT a repeat of completed D13-only
metadata13..18 join and requires no native rerun. Exact recovery/implementation/
division ORIGINAL/SAME/hosted/review and ListItem acceptance remain OPEN.

### DECIMALS12/14 qualified native pair — 2026-10-20

At clean signed/DCO/G source freeze886b8720d56fa770847c79a9dacba4dbdf1d0a5f,
NEW audit+DAW explicit CLEAR and Request Guard no-conflict/release preceded ONE
worker invocation20:46:25..20:46:33UTC. Seven children0/empty: regenerate source/
cases, parent-source reader, first native, strict-single BEFORE repeat, repeat,
paired audit. Every child idleprefix1/full I/O PSI avg10+avg60=0/0/guard0/nice-n10,
immutable input hashes BEFORE+AFTER EACH0. No compiler/newFLL/VM; observer reused
and process-local Microsoft32 override/inheritedUNSET verified. Prompt RELEASE
to allthree after completion; no remaining own native/compiler/VM reservation.

32records/run/all42rawfields repeat-identical/inactive slots0, rawSHA both
47232c78c28f7b7536b654b3c85fd94bd7a6eead587f719298f80762f02c2499.
Original payload/N40/18/runtime and target metadata/payload/settings are exact;
new-setting before-after/repeat/cross-metadata parent/ROUND invariance passes.
Changed-setting original/ROUND tuple values remain newly observed, not copied
D13 expectations or formatter acceptance. No target comparison metrics yet.

Validated complete root moved to
build/default-transform-original-setting-consumer-captures1; workerroot absent.
100 source/ownPRG+FXP/raw/status/hash/gate/runtime/actual shell initial+completion/
fresh coordination+release gzip receipts at matching fixture archive; no SDK/FLL
binary. Manifest/checksumlist and compressed/decoded size/hash/exact host bytes
verified. Combined post-run inspection shell1 is final idle-prefix pgrep, NOT
native or reader failure. Frozen native-reader replay at original module path
with explicit archive/current source identity0/empty/report byte-exact to audit,
input hashes before+after0/prefix1/fullPSI0/0/nice10, host
original-setting-native-reader-replay1.14 verified supplemental receipts bring
native archive114; earlier source100+283 receipts intact. AFTER-capture dirty
archive snapshots are NOT native freeze. Ordinary shell NOT REA Evidence or
Windows/internal-call/atomic mapped-image/descendant-cleanup attestation.

LOW DQ-DV001 actual developer walkthrough: fresh three-party coordination,
unchanged signed source, per-child resource/hash gates, strict first-reader
before repeat, all42rawfields/control/runtime equality, prompt release, separate
native/AFTER-capture provenance, gzip-reader identity/readback and all receipt
byte checks. Maintainer self-review only, not independent human/system/release
assurance. Existing hazard/rollback/notification/ORIGINAL boundaries unchanged.

NEXT bounded AFTER-capture analyze-original-setting-consumer.cjs: bind source
freeze/digest/native manifest/all32 raw42 projections and observed control tuples.
Compare16 metadata13->14 interventions at SAME parent/D/FIXED,16 setting12->14
interventions at SAME parent/metadata/FIXED,16 signed adjacent payload pairs
(two pairs x2 D x2 metadata x2 FIXED),32 TFvsSTR contexts. Setting interventions
must NOT presume control/ROUND values unchanged across D. Separate literal/
padding/exact decimal value/zero-sign with independent metric tests; no guessed
strings/threshold/cap/universal formula. Frozen reader/syntax/analysis/readback/
gzip-analyzer replay and immutable inputs/gates must pass+archive BEFORE metrics
are interpreted. No native rerun/compiler/VM needed; formatter/conversion/
propagation/division ORIGINAL/SAME/hosted/review/ListItem acceptance remain OPEN.

### DECIMALS12/14 reader and guarded-worker qualification — 2026-10-20

audit-original-setting-consumer.cjs now independently validates the literal
16-route grid/32 records/all42 fields, old raw-parent8 payload/runtime provenance,
actual new settings and target active metadata/payload. New-setting original
display/STR and public ROUND active metadata/payload/display/STR are observed,
NOT copied D13 expectations or guessed constants. Original active N40/18/payload/
runtime remain anchored; newly observed ROUND is finite Numeric with matching
raw payload, without assuming its width/decimals/value. Before/after, per-context
repeat and SAME parent/setting/FIXED cross-metadata control equality are mandatory.
Raw observations retain active control tuples and ROUND metadata/payload. Only
the inactive Numeric integer slot is normalized; no payload/text differences
are tolerated across repeats. Strict-single reader runs BEFORE native repeat.

check-original-setting-reader.cjs passes61 SYNTHETIC tests: baseline single/pair,
bad order/fields/settings/status/runtime/control/payload/metadata/length/count,
active before-after damage, synchronized but cross-metadata control drift,
nonfinite ROUND, fractional DECIMALS and repeat mismatch rejected. Deliberately
arbitrary coherent finite ROUND tuples (including different width/decimals) are
accepted, demonstrating that synthetic text and D13 tuples are NOT expectations.
Inactive-integer-only difference is accepted and reported, never payload damage.

original-setting-consumer-capture.sh requires fresh external coordination flag,
clean source, inherited override unset, exact reused binary/source identities,
NEW output root and per-child immutable hashes/idleprefix/BOTH full I/O PSI<1%/
nice-n10 gates. It preserves partial native reports on failure, runs strict
first reader BEFORE repeat and paired audit, never silently reuses capture roots.
check-original-setting-worker.cjs extracts ONLY checked() and passes13 harmless
synthetic guard/status/hash/partial-report/source-order tests; fake PSI/prefix
are NOT actual host clearance. No whole native script/native paths executed.

Host original-setting-reader1: four stages0/empty, reader source-only and61 tests;
reader-replay1: two gzip-frozen source/test stages0/empty/byte-exact/current-source
identity. New original-setting-source-checks2: all14 stages0/empty, regenerated
source/cases byte-exact,168 independent source assertions,61 reader+13 worker
checks. New original-setting-source-replay2: eight archived-source stages0/empty,
six original-path/current-byte-identity JS replays plus two shell syntax-only
checks; exact regeneration/168+61+13 again. Immutable hashes BEFORE+AFTER EACH0,
idleprefix1/fullPSI avg10<=0.00/avg60<=0.03%/nice-n10.283 gzip receipts across
four roots at original-setting-reader-checks1, all manifests/checksumlists and
compressed/decoded sizes/hashes/exact host bytes checked; all nine qualification
source snapshots additionally match current files. Earlier100 source receipts
unchanged. Base43c7cb51f+dirty source/evidence snapshots NOT native capture.

LOW DQ-DV001 actual developer walkthrough: independently inspect source-route
grid and new-setting provenance boundary; run source/reader/guard misuse tests,
preserve new observations, source-byte identity, actual per-child gates/status/
hashes and archived readbacks. Maintainer self-review only, NOT independent
human/system/release assurance. Existing hazards/rollback/notification and
unchanged ORIGINAL assertion boundaries retained. No production behavior change.

NEXT clean signed/DCO source/evidence freeze, then NEW explicit audit+DAW CLEAR
and Request Guard no-conflict/release plus fresh per-child gates before ONE new
native pair. No native/compiler/VM/newFLL has run; prior D13 captures untouched.
No guessed new-setting control/target strings, formula/cap/tolerance or formatter
parity claim. Full recovery/implementation/division/ListItem acceptance OPEN.

### DECIMALS12/14 source preparation checkpoint — 2026-10-20

emit-original-setting-consumer.cjs generates original-setting-consumer.prg and
original-setting-consumer-cases.json: SAME four signed original D13 payloads,
SET DECIMALS12/14, metadata13/14, width40/bothFIXED,16routes/32records/42fields.
No numeric argument/result assignment; fresh inline reconstruction for every
consumer, original N40/18/public inline ROUND before+after. Independent literal
source checker check-original-setting-source.cjs checks168 inventory/source/
route/runtime assertions. It does NOT qualify a results reader or native worker.
Source-only ten guarded stages in build/default-transform-original-setting-source1
all0/empty: generation2, syntax2, source check, regeneration2, frozen gzip source/
cases/checker replays3. Regenerated/replayed source/cases/report byte-identical,
immutable hashes BEFORE+AFTER EACH0, idleprefix1/fullPSI0/0/nice-n10.100 receipts
at original-setting-source-checks1, compressed/decoded-size/hash/exact-host-byte
verified with manifest/SHA256SUMS. Base3f129dceb+dirty source is NOT native freeze.
No native/compiler/VM/newFLL or target/control display predictions.

Important new-setting boundary: old D13 original/ROUND display and ROUND payload
are NOT valid expectations at D12/14. Old raw seed evidence still establishes
original payload/provenance/runtime binding, not changed-setting controls. NEXT
independent results reader must validate original payload/active N40/18/runtime,
new settings and target metadata/payload, observe new-setting parent/ROUND
tuples without guessing their values, verify before/after invariance, per-context
repeat equality and cross-metadata control equality at SAME parent/setting/FIXED.
Do not silently copy D13 display/ROUND expectations or claim them independently
native-qualified. Reader must retain raw newly observed ROUND metadata/payload
and strings, with exact finite/active metadata/runtime checks and strict-single
BEFORE repeat. Synthetic tests must reject changed active controls and malformed
records while explicitly labeling arbitrary synthetic strings NOT expectations.
Then author fail-closed per-child guarded worker/synthetic helper tests, source
and archived-source qualification; only after clean signed freeze/NEW explicit
audit+DAW/Request Guard clearance may native execute. No worker authored/run yet.

LOW DQ-DV001 developer walkthrough: inspect independent literal16-route grid,
all repeated fresh consumer expressions,42-column layout, absence of numeric
result assignment, changed-setting nonprediction policy, actual168 checks,
byte-identical regeneration/replay and per-child gates/hash receipts. Maintainer
self-review only, not independent human/system/release assurance. Existing hazard/
rollback/notification/ORIGINAL assertion-preservation boundaries unchanged.
Full source qualification/native evidence/recovery/formatter acceptance remain
OPEN; no parent/division/ListItem completion or compatibility change claimed.

### Index-exact original metadata13..18 join — 2026-10-20

join-original-scale-consumers.cjs binds both distinct native manifests/source
commits/raw digests and both archived analysis manifests.48 compound keys
(parent/FIXED/scale), all raw42-field target projections, original/public ROUND
active control tuples/settings and CRT identity tuples are checked BEFORE
joining. Only inactive Numeric integer slots are removed.48 shared-control
comparisons pass; separate source/raw provenance retained, not same-session or
internal-call evidence.40 consecutive metadata edges/24 signed adjacent pairs/
48 target TF-versus-STR contexts, eight decimal metrics and five rejection
self-checks (duplicate key, altered control/runtime/width/payload).

Preserve build/default-transform-original-scale-join1: both frozen archived
analyzers+syntax0, then own analyzer1 from an incorrect global route-index
assumption (indices restart under each FIXED mode). No failed-stage AFTER,
readback/replay claimed. Corrected to raw compound identity and prior report
lookup by index+parent+FIXED+kind. NEW original-scale-join2 completes six guarded
stages0/empty: both frozen analyzers/source-byte identity/byte-exact old reports,
syntax, join, independent readback, gzip-frozen join at original module path.
Join/readback/replay byte-identical, immutable input hashes BEFORE+AFTER EACH0,
idleprefix1/full I/O PSI both thresholds pass/nice-n10. No native/compiler/VM.
104 gzip source/status/gates/hash/report/full-command/completion receipts across
both roots are compressed/decoded-size/hash and exact-host-byte verified under
original-scale-join-captures1 with manifest/SHA256SUMS. Existing captures untouched.

Observed finite results:13->14 changes four TF exact decimal values (all OFF),
while14->15/15->16/16->17/17->18 each changes four literal/trimmed spellings but
zero values. No metadata edge changes STR or zero sign. Across24 signed adjacent
payload pairs TF distinguishes14, STR24. TFvsSTR48 literal/28 trimmed/20 value/
seven zero-sign gaps remain separate. No universal metadata/rounding/precision
rule, tolerance/cap or accepted formatter inferred.

LOW DQ-DV001 developer walkthrough: validate compound-key uniqueness and complete
grid, exact per-capture projections, reject mutated cross-capture controls,
preserve failed assumption/source/status, replay immutable analyzers and new
source, inspect metric categories and byte-verification. Developer self-review
only, not independent human/system/release assurance; existing hazards/rollback/
notification/unchanged ORIGINAL assertion boundaries retained.

NEXT bounded source discriminator: SAME four original signed payloads, width40,
target metadata13/14, SET DECIMALS12/14, FIXED OFF/ON (16 routes/32 records).
Vary the previously fixed D13 setting around the observed metadata13->14 value
boundary, with fresh inline reconstruction per consumer and independent raw
parent/public ROUND controls. No guessed target strings/arithmetic/shift/signcopy
or repeated D13 native capture. Prepare/fail-closed reader and guarded-worker
source checks first; native only after signed source freeze and NEW explicit
audit+DAW/Request Guard resource coordination. Exact display/conversion/
propagation, division ORIGINAL/SAME/hosted/review and ListItem remain OPEN.
