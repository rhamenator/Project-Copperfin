# GO/GOTO explicit record-number conversion (#5611/#6776)

## Retained recovery before implementation

RQ-CF-PRG-GO-RECORD-NUMERIC-001 derives from admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner extension/default-mode intent, and
HZ-runtime-crash-01/HZ-data-corruption-01. Only go_command's explicit
record-number llround conversion in prg_engine_dispatch.inl is selected.
Base main: 5a1eeadd21ee97b7544e1cc336ee6d860256aebe.

Three strictly serial installed VFP9 09.00.0000.7423 probe.prg runs match all
82 lines: GO and GOTO with40 Numeric/Currency/type operands each, VERSION and
cleanup. Fractions1.49/1.5/1.9 and2.5/2.9 truncate. Negative overflow
-4294967295 aliases positive record1; positive4294967297 rejects5.
Ordinary cursor0/negative/absent record4 requests reject5 without movement;
Currency/Character/Boolean/NULL reject10. No statement after an error runs.
Position/BOF/EOF/count/session are retained per row.

Three fresh serial pending.prg runs match all34 lines. Buffering4/5 supports
valid pending negative identities -1/-2; negative fractions truncate toward
zero. Original out-of-range negative operands whose low32 is negative
(-4294967297/-4294967298) reject5 even with pending rows available.
-4294967295 still selects positive1. Invalid requests preserve pending -2 and
its payload. Temporary cursors revert/close, MULTILOCKS returnsOFF and session1.
No decompiled binary, VM, external DBF, ODBC/network or system changes.
The existing Wine wrapper is unchanged. No overlapped or failed probe batch.

Native source/output SHA256:
- probe.prg: 11eb452a2a8b3f09362e5b11cdda870017c110834b5fa2d012e3961dad665238
- probe.out: 1f34b6935fd40e778b7fd6dc7f601644f1ba80fa2943efec8392f2540d828d9e
- pending.prg: eeb2e691dd0b3fcf3df9bb9dbb5ad87f2b247c562d653334e0ede669eb4953b3
- pending.out: cae386d20ab101ec9f6b4ab0e8fb3c4c2f72855b3a41b5a49b533db41744306a

## Derived bounded conversion plan

Default finite Numeric truncates into checked signed32; exact int64/uint64
are compared without double loss. Zero and signed negative in-range targets
remain conversion-stage values, not proof that a record exists. Preserve
valid table-buffer negative identities in both modes. Explicit VFP9 alone
admits out-of-range negative low32 aliases when the converted result is
strictly positive; do not synthesize negative pending identities from huge
negative operands or infer positive-overflow wrapping. Reject non-finite,
NaN, positive overflow and all other overflow with catchable localized5,
safe original-operand text, before move_cursor_to, buffer commits, relation
synchronization and successful GO events. Signed32 endpoints and exact/NaN
extension boundaries are derived safety policy, not native existence evidence.

Non-Numeric existing value_as_number half-away coercions will be checked and
preserved; do not silently implement native type10 parity. General record
existence/type/pre-mutation admission is unfinished recovery gap #7081.
Current source move_cursor_to clamps invalid nonpositive/absent physical
targets and may commit row buffers before navigation. Actual unchanged-main
execution controls are pending; source/native disagreement alone is not a
runtime acceptance result at this pre-implementation recovery point. Actual
unchanged-dispatch controls and fixed acceptance follow below.
GO TOP/BOTTOM, SKIP, UNLOCK RECORD, aggregate scopes,
shared move_cursor_to and filter/relation/session/generation semantics stay
outside production scope. Preserve existing generation revalidation after
resumable operand evaluation. No argument-sized allocation or navigation loop.

Planned verification: independent direct bounds/exact/NaN cases in both modes;
guarded GO/GOTO with main and explicit-IN targets, unbuffered and buffering4/5
pending rows; position/payload/count/BOF/EOF/selected-alias/session/mode,
success-event suppression, cleanup and safe localized original operands.
Retain unchanged-dispatch fail-before proof and independent gap controls;
then GCC and Clang ASan/UBSan/float-cast-overflow runs strictly serial across
build directories with privateTMPDIR. Reuse focused navigation lifetime and
negative-buffer regressions; run older Numeric and repository contracts.

DQ-GORECORD-001: distinguish conversion-stage admission, native observations,
derived boundaries and unfinished record/type parity. DQ-GORECORD-002: retain
reverse RQ/code/test links, exact baseline/acceptance identities, serial
reproduction, cleanup, rollback and PR/changelog field notice.
Misuse severity medium: rejecting every negative target breaks correct
buffered programs; assuming conversion admission validates record existence
can cause pointer/buffer changes. Procedural delta replaces only unchecked
half-away conversion; keep general navigation recovery separate.
At initial recovery, VR implementation/baseline/acceptance and DV development
self-review/guarded walkthrough/slice-only rollback were pending. Completed
development evidence follows; no independent-human,
complete native/type/message, platform, leak or release qualification claim.

An initial unchanged-dispatch Clang test run had a harness setup mistake:
creating observer without SELECT0 replaced the selected guard cursor. It also
assumed a blank qualified-field read at BOF rather than retaining the actual
unchanged-navigation control. That preliminary run is excluded as negative
acceptance evidence; both harness expectations were corrected without changing
production. Its four dirty-row controls did show persistence, but final
unchanged-dispatch reproduction must independently confirm them.
The next preliminary harness incorrectly reported physical4/5 as positive
RECNO after selecting pending rows. Qualified-field and public-negative-recno
preservation controls are corrected to the actual unchanged behavior; neither
preliminary failure count is used as conversion negative evidence.

To repeat native observations, execute each command to completion before
the next, including across build trees:
`/home/rich/bin/vfp9-probe /absolute/slice/tests/fixtures/vfp9-go-record-numeric-observation/probe.prg`
and then pending.prg. Ignore owned generated FXP; retain source/output above.

## Implementing boundary and retained baseline

checked_go_record_argument in helpers.h/.cpp implements the selected signed32
conversion; go_command rejects missing results with PrgCompatibilityError5
after resumable operand evaluation and generation revalidation, before
move_cursor_to. No shared navigation, type routing or record-existence change.
Four locale catalogs provide safe original-operand diagnostics. Numeric uses
format_round_trip_decimal rather than value_as_string's unchecked llround;
exact/Currency formatting remains safe. Rendering1E300 produces301 decimal
digits, not scientific notation; this is Copperfin semantic diagnostic
evidence, not byte-identical native wording.

baseline-audit.txt retains original dispatcher blob a06d6738bce7e95ff13507d3c77d39445f33195f
and hashes of dispatcher/test/raw logs. On unchanged main5a1eeadd both GCC and
Clang fail exactly340 selected assertions:316 guarded conversion rows,
12 guarded successful-event counts and four each dirty-pointer/disk/event
checks. No setup/completion/cleanup/count/alias/session/mode or existing
neighbor failure. Full owned logs persist until merge. Actual baseline
controls establish #7081: unbuffered0/-1 move to BOF, absent4 to EOF and
Currency/String coercion succeeds, unlike native rejection. Buffered valid
negative identities are preserved controls. Direct/eight locale tests were
added after baseline; do not claim their negative execution.

## Completed development documentation review

DV-GORECORD-001: Codex in the owner-authorized maintainer workflow completed
development self-review of the native Numeric/pending observations,
independent signed32/signed64/exact/NaN constants, range-before-narrowing,
positive-only VFP9 negative aliases and pending identity preservation.
No inferred native record existence/type behavior is used as a requirement
for retained coercions. No argument-sized allocation/navigation loop or
external table/network/VM/system changes are introduced. This is development
self-review, not independent-human sign-off or integrator qualification.

DV-GORECORD-002: completed guarded walkthrough uses both modes/GO/GOTO and
unbuffered/table-buffer4/5 explicit-IN targets with a separate selected
observer. Conversion failures retain pointer/payload/BOF/EOF/count/alias/
session/mode, suppress fall-through and successful navigation events.
Four row-buffer2/3 walks reject1E300 while preserving the dirty current row;
after revert/close actual disk record2 remains originaltwo. Eight fresh locale
cases retain error5/ERROR()/AERROR message agreement and original operand.
Four existing neighbors cover reentrant cursor replacement/filter closure
and negative pending identities/partial update failure; not full navigation.
Cleanup closes owned cursors, reverts pending edits, restores COPPERFIN and
the scoped locale; temporary roots are privateTMPDIR-owned, runs serial.

DV-GORECORD-003: completed rollback walkthrough specifies a reviewed revert
limited to this slice's helper/dispatch/catalog/test/doc changes, repeats the
named Numeric/GO/neighbor/contracts verification and republishes corrected
guidance through changelog/PR. Never destructive reset, broad deletion or
rollback of external buffered edits. Medium misuse remains conflating valid
conversion with record existence (#7081), or rejecting all negative pending
identities. Procedural delta replaces unchecked llround/narrowing with checked
Numeric truncation and explicit5, retains safe non-Numeric half-away coercion.
Full type/message/record parity, broader filter/relation/session behavior,
platform/release/leak qualification and independent review stay separate.

## Local verification and reproduction

VR-GORECORD-001:122 independent helper calls pass, including exact64 precision/
extrema, signed32-adjacent doubles, signed64 bounds, NaN/infinities, Currency
and other-type preservation controls. Exact/NaN boundaries are derived, not
native script observations. VR-GORECORD-002:588 guarded rows, four dirty-row
disk sessions/eight locale sessions and four existing neighbors pass.
VR-GORECORD-003: full GCC15.2 Debug-O0-g1 passes3/3 in89.66s, older Numeric87.59s,
new1.76s/neighbors0.30s. Focused Clang21.1.8 ASan/UBSan/float-cast-overflow
passes2/2 in8.47s, new7.30s/neighbors1.15s, no diagnostics, detect_leaks=0.
Clang excludes full older Numeric; no leak/platform/release claim.
Six repository contracts pass6/6 in7.22s: issue intake, contributor signoff,
changelog assembler/validity, locale installation and native test isolation.
All174 fragments validate. Additional long safety-workflow negative-fixture
contract passes1/1 in358.49s (total358.50s).
Initial fixed GCC had eight expectation-only mismatches assuming scientific
notation; independent301-digit expected text corrected without production
changes before these passing runs. Those preliminary failures are excluded.
DQ-GORECORD-001 maps DV-001/-002 and VR-001/-002; DQ-GORECORD-002 maps
DV-003/VR-003. The docs32 requirement row carries reverse links.

Acceptance log SHA256, GCC then Clang:
00d8478f1d3126bacac35ce539c2a20f2f638a8fd6d468610f95fcc6955d60fc
69060c15906e8c41747debc16ba340f90c407c65c02888221cafc5bfd3acf2ee.
Final numeric-test SHA256:06ce2ff9f86ee415d22a23a71ab3cbec4b20ede4b0c9a6d0dbeed80199a6f0b8.
Baseline audit SHA256:3561d334a3ba6c519892a30135f7ebee7d70a2bd6a5a3e41ddad0149b9b1519f.

Build named targets test_prg_engine_go_record_numeric,
test_prg_engine_go_record_neighbors and (GCC) test_prg_engine_numeric_behavior
in owned /home/rich/temp/copperfin-go-record-5611-build or ...-sanitize.
Clang flags:-fsanitize=address,undefined,float-cast-overflow
-fno-sanitize-recover=all -fno-omit-frame-pointer; scoped
ASAN_OPTIONS=detect_leaks=0 and UBSAN_OPTIONS=halt_on_error=1.
Use fresh mktemp -d TMPDIR inside the build and ctest-j1 with
^test_prg_engine_(go_record_(numeric|neighbors)|numeric_behavior)$;
Clang excludes older Numeric. Never overlap native/runtime invocations,
including across builds. After merge move only owned builds/FXP to
recoverable trash and remove the validated slice branch/worktree;
retain tracked evidence, unrelated files and all stashes. Required hosted
checks, clean exact-head Claude/authorized Codex review and resolved
conversations remain merge gates. No merge or release completion is claimed.
