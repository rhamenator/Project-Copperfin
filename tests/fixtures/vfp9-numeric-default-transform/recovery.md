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
