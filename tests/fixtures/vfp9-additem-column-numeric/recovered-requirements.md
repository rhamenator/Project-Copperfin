# AddItem supplied THIRD-column pre-code requirements

This supersedes the pending recovery statements in README.md and
recovery-observations.md, not their retained evidence. No helper, acceptance
assertions, registration/isolation, freeze or caller migration exists yet.
The seven main core inputs remain byte-identical to main
cd4489475cfb424a5c7fa48888b978d9a27eab2a. This is an implementation plan and
recovered/derived requirement, not completed verification or production parity.

## Allowed sources and independent corroboration

OPEN owner-authored/exact-agent-approved #5611 and #6776 were revalidated before
content intake. Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001 names the per-area policy;
#5611 expressly requires operation-specific allocation/count bounds. Completely
read shipped AddItem/ColumnCount/BINTOC help and unchanged installed VFP9 7423
privateWine runner provenance are in recovery-observations.md. No decompilation,
proprietary HTML copy, credentials, setup change, VM start/clone or backup-reset
completion assumption. Neither existing code nor a sibling's expectation table
is this requirement's source.

Four independent clean paired captures now retain full source, CRLF raw,
actual runner/comparison status, stderr and manifests:

- Initial1000: raw579059e92ab86b1d26e939ba9b1ee7a48c5d590286a02f97f5d53c2a43b86ef7;
  boundary432: rawf0af5005109dd1950944e7a030c9610ffe06a533068d1e5de5c7d4426e42e23f.
  Their failed initial runner1/empty stdout/full result and failed empty-stdout
  comparison remain explicit, not promoted to runner success.
- Configuration864: rawef0c7d61865363c353e519fb7c4b53a94493ce1bb5fcdee67bfc3658affc5998;
  sourcec53afc579c05bb473b4dea51aabfee4ee4e15883ba318199ef10e6ead25aaeb2.
  BOTH classes, columns0/2/8, sparse three-row controls, independently supplied
  ordinary positions1/2/4, default/explicitRST0/space binding/Sorted contexts,
  configuration BEFORE population, twelve independent third operands.
  Both runner0/cmp0/empty stderr/DONE864; audit144 unchanged11/720 inserts/0 failures.
- Projection432: raw92ea708fee7ff352f1feb2dc87f9f7fb0fd1b50c66fdad07e718faed034dadbe;
  sourced76bb638993753b5cef5694a8184f666b6f6f05529866a5c19b2d06909ada8a9.
  BOTH classes, columns0/2/8, empty/sparse controls, position1; precise signed32
  neighbors, additional signed cycles and actual binary large-range payloads.
  Both runner0/cmp0/empty stderr/DONE432; audit192 unchanged11/240 inserts/0 failures.
  The retained preliminary audit compared first to itself only: capture
  consistency, NOT an independently repeated pair. The final audit uses BOTH.

corroboration-evidence.sha256 verifies the new pair/audits; first-capture-
evidence.sha256 retains earlier attempts. Observation-only corroboration audit
groups72 configuration and216 projection contexts; observed admission, payload
and visible candidate columns match across all contexts, failures0. It does
not test a conversion helper or originate a Copperfin default policy.

Precise observations: truncation toward zero; positive2147483648.1/.5/.9 and
negative-2147483648.1/.5/.9 admit blank rows, while2147483649 and
-2147483648+0.5 reject11. BOTH-sign6442450944/.9 likewise admit blank;
positive6442450945 rejects and negative-6442450945 admits. These corroborate
the signed32 minimum sentinel exception separately from ordinary-positive
column admission.65536/.9 admit blank;65537/.9 and65538.9 reach visible1/2.
Wider BOTH-sign aliases corroborate visible placement. Admission does not
depend on configured ColumnCount, Grid255 or stable-ID32767.

Actual postcall BRS/type is retained. In particular source -2^64 captured
POSITIVE43F0000000000000, so that source is not a negative-payload oracle.
Do not infer a parser/FPU defect or change parsing. Native source near-one
evaluation and direct C++ binary neighbors remain distinct evidence classes.
No hidden-storage layout, arbitrary native allocation capacity, absolute
successful focus/text/top/generated-ID/return or sorted placement parity follows
from the sampled visible cells and count increment.

## RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001

Bounded supplied THIRD nonNULL Numeric/int64/uint64 argument with exactly three
arguments/nonNULL Character item/coherent materialized-unbound native
ListBox/ComboBox/default or exact typed RST0/trimmed-empty Character binding.
The second operand is an ordinary finite integral Numeric insertion position
in1..ListCount+1, not a second-position coercion/alias/type migration. Capture
positions1/2/4 corroborate that bounded context independently. Typed-zero,
exact64 and binary/NaN extensions derive from the named parent, not native CAST.

COPPERFIN default: finite Numeric truncates toward zero and admits1..65535;
exact64 retains its integral value without double roundtrip and admits that
same domain. Invalid/nonfinite/NaN raises existing localized catchable11 before
modeled insertion. The65535 ceiling is a DERIVED operation-specific host bound
from #5611, #6776 and HZ-runtime-crash-01/HZ-data-corruption-01: retain the
ordinary nonzero16-bit column repertoire, bound each newly prepared row to at
most65535 argument-driven cells, reject wrap/zero-sentinel aliases by default.
It is NOT a shipped documented native maximum, configured-column bound,
global list capacity, memory/performance benchmark or closure of shared
allocator/total-memory hazards. Existing configured control size and overall
process resource exhaustion remain residual limits outside this conversion.

Explicit VFP9 mode: defined signed-low32 projection of finite/truncated Numeric,
then admit projectedpositive or exactlyINT32_MIN; reject projected0 and other
negative values with11. Project admitted values to unsigned low16 for the
effective column0..65535. This generalized model is recovered from independent
measured cycles and precise sentinel boundaries, with the defined conversion
mechanics supplied by the named numeric parent; it is not a decompiled cause.
Zero is an ADMITTED blank-row sentinel, never an invalid default or a
requested_column-1 underflow. Ordinary positive65536 is a VFP9-only blank alias.
Nonfinite/NaN/outside signed64 Numeric has parent-defined low32zero and rejects11.
Exact signed/unsigned64 low32 projection is lossless parent-derived extension
behavior, not a native64CAST claim. It still admits only bounded effective
columns, so compatibility aliases cannot request billions of host cells.

On nonzero admission prepare the bounded row/candidate at effective column;
on admitted zero prepare a blank row without indexing column-1. Then retain
existing insertion resolver/allocator/successful state/return/sorting. Existing
materialization can precede this check; do not claim whole-call atomicity.
Other second operand forms, types/arity/NULL, binding/RST/lifecycle/parser and
unadmitted7176/7177/earlier gaps remain excluded/unrepaired.

## Architecture and verification map (planned, before helper/assertions)

Distinct checked_additem_column_argument in helpers.cpp/h returns
optional<size_t>, including an engaged zero for the admitted blank sentinel.
It remains UNUSED through BOTH complete original suites/full failures/hash
audits. After that gate, only the selected AddItem third-column arm moves the
existing materialized-unbound check before the old column conversion and
prepares the safe row without zero underflow. Preserve the already shipped
exact-two-argument position arm and all fallback/shared insertion code. Reverse
RQ/VR links are required in helper/header/method/tests/registration.

VR-5611-ADDITEM-COLUMN-NUMERIC-001/002: independent literal tables, direct
helper boundaries and four public control/mode shards; modes/sessions,
empty/dense/sparse/config/ordinary positions/locales/rejection-state/catchability,
finite/binary/signed32 cycles/sentinel/exact64/NaN as applicable. Preserve all
prior85 subjects. BEFORE wiring freeze source plus audited registration and
isolation, run BOTH full original compiler suites, retain every full multiline
failure/status/source hash, then BOTH fixed suites/sanitizers, NON-NATIVE
excluded-scope before-after comparisons, five documentation contracts and
actual clean exact-head external review/all11 required checks/resolved threads.

### Safe original public-suite boundary

No global allocator override, environment-dependent assertion skip, production
allocation change, timeout increase or artificial error11 oracle is planned.
Original public vectors are restricted to finite ordinary operands whose
absolute magnitude is at most65538, plus signed64-representable finite negative
wider aliases whose old selected rounding stays negative and cannot request a
positive huge row. Each positive
original preparation is therefore bounded at65538 cells; exact64 public
vectors use the same safe magnitudes. This still exposes rounding vs truncation,
default65536 rejection, VFP9 low16 blank/1/2 aliases and catchable errors.
Direct UNUSED-helper vectors independently cover ALL large/nonfinite/exact64
boundaries, including dangerous positive aliases and INT32MIN, without row
allocation. Full original public probes intentionally do NOT execute billions-
column/nonfinite/unbounded old resize requests. That is a verification reach
limit, not observed runtime safety or full public huge-argument qualification.
Source/dataflow audits must prove all selected paths use the bounded helper
and cannot bypass it. Before declaring completion, separately retain a fixed-
only public hostile-input corroboration source written before caller wiring;
its chronology/reach is explicit and never presented as an original-suite run.
The fixed-only corpus must not redefine the frozen helper/public expectations.

## DQ/DV-5611-ADDITEM-COLUMN-NUMERIC-001 / hazards

Preliminary bounded documentation misuse MEDIUM; linked active hazards retain
their high/catastrophic severities and statuses. Procedural delta: supplied
third column, not row/ID; finite truncation/bounded row vs unchecked rounding;
default rejection vs explicit VFP9 low32/low16/blank sentinel; source spelling
vs actual payload; ordinary valid second-position context and configuration
BEFORE population; original safe-public limits vs fixed-only hostile probes.
Misuse includes treating65535 as native global capacity, rejecting engagedzero,
indexing zero-1, claiming parser/hidden-storage/shared-success/allocator parity
or presenting self-comparison/failed runner as repeated success.

DV review/walkthrough is PLANNED, not completed: verify source/raw/status/hash
links and native/binary attribution, independent literal expectations, guard
reach/zero handling, both frozen originals/full failures, unchanged old85 and
scope comparisons, both fixed/sanitizer counters/fixed-only hostile evidence,
contracts and integration gates. Bounded delegated-development self-review
must not be called independent human/high-hazard qualification. Rollback wrong
guidance via scoped reviewed correction/revert, preserve superseding evidence,
notify owner and linked PR/issue consumers. HZ-doc-command-01, runtime/data
hazards and parent workstreams remain active/open; no persistence/global
atomicity/system/release/certification/KBX or hazard closure.
