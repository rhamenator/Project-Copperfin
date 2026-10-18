# Default Numeric TRANSFORM recovery — #5611/#6776/#7189, tracking #7210

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

### Native public-arithmetic discrimination — unrun

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

## Traceability and development-document walkthrough

`VR-5611-NUMERIC-DEFAULT-TRANSFORM-001`: native qualification, native-only
expectation emitter, original-fail/SAME, focused/broader/actual named hosted
execution, exact-head review, strict checks and resolved conversations are
pending except the six bounded native pairs qualified above. No compatibility,
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
