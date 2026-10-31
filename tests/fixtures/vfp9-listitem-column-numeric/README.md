# Selected ListItem supplied COLUMN numeric READ slice

2026-10-15 owner decision: emulate VFP Numeric metadata, propagation and display
as closely as possible, rewriting the Numeric representation where necessary;
likewise preserve native ListBox/ComboBox behavior. This selects the prerequisite
compatibility path, NOT the proposed canonical-double/residual-gap shortcut.
See `docs/03-compatibility-and-migration.md` and the current handoff. Existing
#6776 switch/safety policy remains. The earlier owner-choice blocker below is
historical. `metadata-foundation.prg` starts bounded literal/copy/display
recovery; it is not a completed propagation specification or production fix.

2026-10-15 diagnostic update: [the decimal-place descriptor is now established
as causal for the near-integer discrepancy](cause-established.md). Ordinary2.5
reads column2. The owner requested diagnosis, not implementation or automation
resumption; earlier unknown-cause statements below are historical.

Current continuation: the owner selected the metadata-compatible path under
[#7189](https://github.com/rhamenator/Project-Copperfin/issues/7189). The
retained factorial/settings audit now recovers the conversion formula. The
historical unused-helper checkpoint implemented it without caller wiring; the
later bounded caller is recorded in the local implementation checkpoint below.
See [unused-helper foundation](unused-helper-phase.md) and the
[frozen safe-public original phase](frozen-original-phase.md). The earlier choice and
selection/recovery checkpoints below are retained history.

Selection-only checkpoint from origin/main
b6d1a7634e56435ad3326d92f13efa3f4fe241d3, after PR7187 merge and owned cleanup.
OPEN owner-authored5611/6776/exact agent-approved revalidated before intake;
this bounded slice derives from their numeric-conversion inventory, not a new
defect hunt or authority from another issue/comment. No fresh defect or runtime
reproduction is claimed. Current code identifies the inventory site ONLY; it is
not its requirement oracle.

## Candidate bounded requirement (recovery gap)

RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR-5611-LISTITEM-COLUMN-NUMERIC-001/002 /
DQ-DV-5611-LISTITEM-COLUMN-NUMERIC-001, MEDIUM/planned, links unchanged active
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01.

Exactly TWO-argument native ListBox/ComboBox ListItem READ: supplied SECOND
nonNULL Numeric/int64/uint64 COLUMN, FIRST an ordinary finite integral Numeric
stable ID1..32767 already present in a coherent materialized-unbound control.
It is not row position, first-ID conversion, implicit-column1, assignment/write,
parser/raw-route expansion, absent-ID repair or shared row creation. Default/
exact RST0 and trimmed-empty binding are the candidate conservative boundary;
independent allowed recovery must corroborate supported configuration/routes
before helper/acceptance assertions or registration are authored.

The already-shipped one-argument RQ-CF-PRG-LISTITEM-NUMERIC-001 and all prior100
subjects remain unchanged. Existing-ID/first-argument handling, other types,
NULL/arity/binding/RST/lifecycle, shared lookup/state/return/fallback and separate
unadmitted7151/7186 gaps are excluded. Do not borrow AddListItem/AddItem's32745
write/preparation bounds, low16 effects, error policy, helper/table or current
Copperfin empty fallback as this READ consumer's requirement. Bounds/default/
error-vs-empty/explicit-VFP9 conversion policy are STILL GAPS, not chosen here.
Exact64/NaN/binary extensions need named owner/numeric/hazard parents, not a
native syntax oracle or a reason to skip the extension.

## Recovery progress (2026-10-14T22:15 heartbeat)

Three independent matching native pairs224/188/360 now retained; all six actual
invocations0, full byte-repeat/BRS/state/context/framing audits0. See
[recovery observations and remaining gaps](recovery-observations.md). Near-integer
promotion needs more recovery before conversion policy/helper/assertions; these
captures are NOT acceptance-table or implementation completion. The ten main
source/test/config inputs remain unchanged. No helper/assertion/build/wiring.
The selection-only absence-of-native statement below describes checkpoint7bb,
not the current recovery phase.

## Next concrete action and ordering

Independently recover shipped Microsoft ListItem documentation and fresh actual
installed-VFP9 observations for this explicit COLUMN READ. Retain full probe
source, actual type/binary value, class/configuration/session/call context,
before/after state, return/error, complete raw/status/runner/help hashes and
repeat/capture audits. Use another fixture only for capture framing, never its
vectors/results as an oracle. No decompilation, proprietary source copying,
credential logging or VM/backup-reset-completion assumption.

Complete the pre-code requirement/architecture/verification/DQ-DV/hazard and
safe-original/extension boundary map FIRST. Then distinct UNUSED helper and
independent literals, separate pre-wire fixed-only hostile vectors and explicit
mode/class/session coverage, while preserving prior100. Freeze all relevant
source/assertion/CMake/isolation inputs and require BOTH complete original
compiler suites/full failures/counters/built commands/hashes BEFORE wiring.
Then bounded caller only/BOTH full fixed/sanitizers/hostile/SAME-compiler modeled
preservation/contracts/delegated walkthrough, signed exact-head review/postpush/
strict required checks/resolved conversations/merge/owned cleanup. Do not reuse
partial, prior-head, synthetic tool records or unsafe old-public executions.

At this checkpoint selection-main-inputs.sha256 freezes ten selected main
source/test/config inputs. No helper, assertion, CMake/isolation change, caller
wiring, configure/build/native invocation, push or PR exists. No parent/hazard/
KBX/system/release closure or owner-input blocker. Earlier PR7187 fixture
manifests remain immutable historical records; its270 runtime inputs remain
unchanged, but a manifest containing global docs is not a new-slice current
documentation snapshot and must not be refrozen to hide the new selection.
Initial selection-manifest invocation named a nonexistent isolation JSON path
and emitted only nine entries. That partial file is retained as
selection-main-inputs.initial-nine.sha256, NOT a ten-input qualification. The
correct policy source is tests/CopperfinTestIsolation.cmake; the new ten-entry
manifest is independently checked against origin/main, with no source changes.

## Local implementation checkpoint (2026-10-31)

The historical selection/recovery ordering above is now fulfilled through the
bounded caller and hostile gates. Both complete original106 compiler suites ran
before wiring and failed only the four expected public shards. Both complete
fixed106 suites now pass all subjects, including four public shards at92 reads
each, with unchanged544 frozen inputs and no sanitizer diagnostics. Each
compiler's four pre-wire-authored fixed-only hostile shards also passes56 reads
with empty stderr and unchanged source/binary hashes. The caller changes only
the exact-two-argument existing-ID ListItem supplied-column Numeric/exact
selector; first-selector behavior and every other route remain outside this
slice. The frozen excluded-scope harness subsequently captured 256 rows across
eight contexts at the exact pre-wire commit and current candidate with both
GNU and Clang sanitizer builds. All four audits pass, diagnostics are empty,
and each SAME-compiler baseline/fixed pair is whole-byte identical at SHA-256
`90187666e02d52fcf6d9bc71d3ef4fe2ac4f3cd3f29985bb3bf4cb1b0b9e2611`;
frozen source and all nine library hashes reverify. See
[the preservation boundary and procedure](preservation-plan.md). Five
development contracts pass and the required repeat MEDIUM walkthrough is clean
at exact head `659a3f558`. A one-shot 612-input broader manifest now freezes the
current runtime/test sources and a 131-test GNU/Clang gate, but that gate has not
executed. Broader qualification, exact-head review and hosted integration remain
mandatory before merge. No parent, hazard, system or release closure is claimed.

## DQ/DV-5611-LISTITEM-COLUMN-NUMERIC-001 development walkthrough

Misuse severity is MEDIUM. The procedural delta is deliberately narrow: only
an exact-two-argument `ListItem(existingStableId, suppliedColumn)` read with an
already-resolved ordinary stable ID may route the second Numeric/exact-integer
selector through the recovered checked conversion. It does not reinterpret the
first argument as a row position, alter one-argument/other-arity calls, create
rows, change binding/RowSourceType behavior, or claim arbitrary expression,
column-capacity, allocation, persistence or lifecycle parity. Constant
arithmetic metadata hooks preserve only the native-evidenced bounded chain and
leave payloads unchanged; unsupported origins, descriptor shapes and values
remain fail-closed.

The misuse walkthrough checks that guidance cannot be read as blanket rounding,
a binary tolerance, a canonical-double shortcut, proof that every fractional
Numeric is valid, or authority to weaken missing/malformed-metadata rejection.
It also keeps safe-public original failures, fixed-only hostile corroboration
and modeled excluded-scope preservation as separate evidence layers. The first
delegated development walkthrough at exact head `d24e468c3` found missing
arithmetic-requirement mapping, this assurance plan, and stale pre-wire wording;
the first repair addressed those items. A repeat at `3811417a5` then found one
incorrect fixed106 attribution, a stale intermediate gate status and two stale
pre-wire source comments. Those repairs were confirmed by a clean read-only
repeat at exact head `659a3f558`, including the intentionally historical
fixed544 evidence and its two comment-only mismatches. This is maintainer
development review, not independent-human, system, release or hazard-closure
evidence.

Rollback is a reviewed revert of the bounded caller, arithmetic-metadata hooks,
tests and corresponding documentation while retaining native captures,
original/fixed/hostile/preservation failures and receipts for investigation.
After rollback, rerun the frozen original/fixed and preservation contracts so a
partial revert cannot silently restore unchecked conversion or stale claims.
Field notification identifies the exact affected head, Numeric mode, control
class, argument shape and evidence limitation in the pull request and admitted
issues; correct linked requirements/hazard guidance and do not claim family,
parent, KBX, hazard, system or release closure.

## Retained hosted qualification dependency

PR7187 optional full Windows native run38636352844 attempt1 failed: all610 tests
attempted once/609pass/numeric_behavior180.07s timeout. Watch64557 harvested1;
aggregate failed closed, NOT full platform qualification. #7188 records evidence
at current main; no cause guessed. One unchanged full-attempt retry requested0/
attempt2 samehead verified inprogress. Existing blocking retrywatch31376/log/
actualstatus live OUTSIDE this worktree in canonical
/home/rich/dev/Project-Copperfin/build/pr7187-late-native/. Attempt1 complete
evidence copied under late-windows/; no original failure erased. Harvest retry,
do not duplicate/modelpoll/mixattempts/raise timeout/omit tests. The uncancelled
native recovery and explicit requirement gaps above remain retained. Previous complete
1.1GB build/evidence is recoverable at Trash/addlistitem-new-column-numeric-5611-build;
scoped stash350dee5e/old stashes/foreign files/worktrees are preserved.
