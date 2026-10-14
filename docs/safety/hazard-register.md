# Hazard Register

RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001 / DQ-DV-5611-ADDITEM-COLUMN-NUMERIC-001
links active HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 to the
[pre-code supplied-third-column recovery/map](../../tests/fixtures/vfp9-additem-column-numeric/recovered-requirements.md).
Independent installedVFP9 pairs corroborate ordinary truncation, admission
beyond configured ColumnCount and signed32-minimum/low16 blank-row quirks.
Default1..65535 allocation bound/exact64/NaN derive from admitted5611/6776 and
the named numeric parent, not native global capacity. Engaged zero must not
be rejected in VFP9 mode or indexed as zero-1. Original public probes remain
bounded to avoid unguarded billions-column allocations; full scalar helper
boundaries/source dataflow and pre-wire-authored fixed-only hostile probes are
separate verification layers. Bounded documentation misuse MEDIUM: conflate
column/row/ID or default/legacy, overclaim hidden storage/shared success/native
capacity, or misstate failed/self-comparison captures as repeated success.
Distinct UNUSED helper/frozen inputs/BOTH complete originals/full failure-hash
audit before wiring, fixed/sanitizer/preservation/contracts/review walkthrough
remain PLANNED. No review/verification completion, independent-human/high-
hazard/system/parent/release closure; all hazard severities/statuses unchanged.
The [frozen original phase](../../tests/fixtures/vfp9-additem-column-numeric/frozen-original-phase.md)
now records the UNUSED helper/independent literals and five serial subjects;
planned276 direct/four1224 safe public/fixed-only542 counters, not measured
acceptance. Native literal audit2528 rows/97 captured of98 literals/failures0;
uncaptured integer65538 is explicitly generalized-model-derived. All prior85
and caller/selector remain unchanged. BOTH complete original90 executions/full
failure-hash audits are in progress and remain gates BEFORE wiring; preliminary
source syntax0 or driver progress is not a pass or completed DV walkthrough.
The [BOTH-original gate](../../tests/fixtures/vfp9-additem-column-numeric/both-originals-before-wire.md)
now supersedes that pending status: both90 complete/identical6968 full failures/
allprior85-new276direct PASS/onlyfourNEW1224 safe-public fail/all255 hashes0.
Only afterwards the [bounded caller phase](../../tests/fixtures/vfp9-additem-column-numeric/caller-wiring-fixed-phase.md)
wires the supplied third operand/current session/localized11 and admitted-zero
blank row without zero-1 indexing. Original predicate, shipped position arm,
shared insertion/allocator/state/sort/return and fallback body remain unchanged.
Both fixed90/sanitizers/fixed-only542/both296 raw comparisons/contracts/bounded
medium review/walkthrough/integration gates remain pending, not completed DV or
independent-human/high-hazard/parent/hazard/release qualification.

The [current local completion](../../tests/fixtures/vfp9-additem-column-numeric/local-completion.md)
supersedes those pending verification statements: BOTH fixed90 PASS/all276-four1224/
each four fixed-only542/both NON-NATIVE296 fullcmp/source hashes, capture/literal/
structural audits0 and five contracts5of5 PASS329.53s. BoundedMEDIUM delegated
development DQ-DV self-review/procedural delta/misuse/automated walkthrough and
rollback/field-notification plan are complete, not independent human or personal
human sign-off. Original safe-public reach and separately fixed-only hostile
corroboration remain distinct. Signed exact-head postpush/actual Claude-FIRST or
authorized fallback review/required11/resolved conversations remain integration
gates. No hazard severity/status change, parent/KBX/high-hazard/system/release
closure, global allocator/whole-call atomicity/leak/warningfree qualification.

Controlled hazard linkage register for safety-relevant documentation and behavior changes.

Purpose:

- provide stable `HZ-*` identifiers
- support investigation-grade traceability
- connect product behavior and documentation procedures to hazard analysis

This register supports Copperfin's DO-178C-inspired development-assurance
baseline for a general-purpose platform. It is not a claim of DO-178C
compliance, certification, an assigned software level, or suitability for a
specific safety-critical deployment. Hazards and controls are applied
proportionally to reach and severity; integrators remain responsible for their
own system and domain assurance.

## Usage Rules

1. Use a unique immutable `HZ-*` id for each hazard.
2. Never recycle retired ids; mark them `retired` instead.
3. Link each safety-relevant behavior or documentation change to one or more
   `HZ-*` ids and to its governing requirement in
   `docs/32-recovered-requirements-traceability.md`.
4. If no hazard applies, record `HZ-none` with explicit rationale in the issue.
5. Update this register in the same change set where new hazard ids are introduced.

## Severity Scale

- `none`: no credible safety impact
- `low`: minor operational disruption, no expected injury
- `medium`: serious operational error possible, injury unlikely but credible
- `high`: severe operational error possible, serious injury credible
- `catastrophic`: loss-of-life or loss-of-platform hazard

## Hazard Entries

| Hazard ID | Title | Scope | Trigger / Misuse Condition | Potential Effect | Severity | Mitigation / Control | Verification Evidence | Owner | Status | Last Updated |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| HZ-data-corruption-01 | Data corruption through unsafe workflow or guidance | Data integrity and persistence | Operator or integrator follows incomplete or incorrect data-handling procedure | Silent record corruption, invalid downstream decisions, potential safety event | catastrophic | Enforce explicit backup/validation/recovery procedure steps in docs and reviews | DV walkthrough plus corruption-detection regression evidence | TBD | active | 2026-05-03 |
| HZ-system-failure-01 | System-level operational failure | Runtime and host operations | Procedure omits preconditions, recovery boundaries, or safe fallback actions | Service outage or unsafe degraded mode | high | Procedure precondition checks, fail-safe boundaries, and rollback/notification controls | DV evidence with independent review and simulation | TBD | active | 2026-05-03 |
| HZ-runtime-crash-01 | Runtime crash under operational use | Runtime execution and debugging | Misapplied runtime/debug command sequence or unsupported-state operation | Abrupt process termination and possible state loss | high | Crash-safe operational guidance, incident capture, and post-crash recovery procedure | DV evidence with crash-path simulation/walkthrough | TBD | active | 2026-05-03 |
| HZ-runtime-debug-01 | Incorrect fault recovery procedure | Runtime debug operations | Operator executes incorrect continue/retry sequence after runtime fault | Runtime resumes in unexpected state; downstream unsafe decisions possible | high | Require explicit pause-state inspection and validated recovery steps in docs | Linked DV evidence in issue and walkthrough artifact | TBD | active | 2026-05-03 |
| HZ-doc-command-01 | Ambiguous command guidance | Operator command documentation | Documentation omits critical preconditions or target context for commands | User executes destructive or incorrect command sequence | high | Procedural delta review, misuse analysis, and independent review before publish | Linked DV evidence in issue | TBD | active | 2026-05-03 |

## Bounded numeric-conversion linkage

RQ-CF-PRG-ADDLISTITEM-ID-NUMERIC-001 / DQ-DV-5611-ADDLISTITEM-ID-NUMERIC-001
links active HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01
unchanged to the [independent pre-code map](../../tests/fixtures/vfp9-addlistitem-numeric/README.md)
and [completed bounded development walkthrough](../../tests/fixtures/vfp9-addlistitem-numeric/local-completion.md).
Medium documentation misuse: confuse stable IDs/rows, fractional truncation,
default/exact policy/explicit VFP9 low32 aliases, fuzzy comparisons/binary
identity, configuration-before-population or excluded duplicate-caption behavior.
Distinct helper remained unused through BOTH complete original80/failure/hash
audits before one bounded second-ID admission. Both fixed80 GNU580.86s/Clang
sanitizer3516.31s and both272 NON-NATIVE fullraw comparisons/all251 hashes/
initialfive unchanged/five contracts5/5 PASS330.25s. Delegated development
self-review/automated walkthrough complete, not independent-human/high-hazard
qualification. No successful shared-state/return, setter/callback/persistence,
whole-call atomicity, parent/hazard/KBX or release closure. Separate unadmitted
7173 caption update/7174 old precision note remain excluded; exact-head signed
postpush/review/hosted integration gates remain separate.

RQ-CF-PRG-REMOVEITEM-ROW-NUMERIC-001 / DQ-DV-5611-REMOVEITEM-ROW-NUMERIC-001
links active HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01
unchanged to the [independent RemoveItem map](../../tests/fixtures/vfp9-removeitem-numeric/README.md).
Bounded documentation misuse medium: remove the wrong display row/identity,
misstate rejected-state preservation, or claim native successful focus/text/
top/return despite7163 or type parity despite7164. Both-sign VFP9 projection,
finite/exact default and NaN containment derive from measured/named parents;
argument-domain11 and admitted-absent1234 are independently observed.
Review4252485330 trimmed-empty binding boundary is independently confirmed
by paired36-call native observations; separate supplemental72 literals froze
before BOTH120-failure pre-fix runs and one-line guard correction. Initial
five frozen and old assertions unchanged. Revised GNU70/70/516.71s and Clang
sanitizer70/70/3213.58s/supplemental72/both144cmp/bothsixgroups/all34hashes/
fivecontracts5/5/345.35s PASS; bounded delegated DQ-DV renewed inREADME, not
independenthuman/high-hazard qualification. Failed relative-root driver attempts
retained, only nonfrozen argument normalized; initial incorrect assessment
explicitly superseded. Signed-fix exact-head integration gates remain;7169
old-head Mac polyglot failure is unadmitted/not repaired, no scope expansion.
Second review4253248168 exact-zero config boundary derives named numeric
parent, native explicitRST0 paired36 source supports equivalent consumers.
First NEW72 scaffold errored on CATCH error objects: incomplete migration
stopped/restored, only new scaffold corrected/refrozen/BOTH120-failure repeats
before typedzero guard; initialfive/first72 unchanged. Revised GNU70/70
PASS514.10s/Clang sanitizer70/70 PASS3202.31s/both144cmp/both72supplementals/
sixgroups/all35hashes/contracts5of5 PASS339.04s. Bounded delegated DQ-DV
renewed01:17:02Z inREADME, not independenthuman/high-hazard qualification.
Signed-fix/freshactual exactheadreview/postpush70/CI still integration gates.
Audit correction01:24Z: original explicitRST0 paired36 operated on EMPTY
native controls, not populated removal evidence. Corrected fresh pair configures
before population/all36before3/rawc92fe8e1, LATE corroboration of original
pre-code literals. Bounded DQ-DV renewed with explicit setup order and separate
unadmitted7171 lifecycle gap; no frozen/source weakening or native setter
parity claim. Current-document contracts repeat remains an integration gate.
Distinct UNUSED helper/frozen tests/BOTH complete original70 suites preceded
one bounded admission arm; shared remover/types/arity/other routes unchanged.
Full fixed70/sanitizers/both144-call NON-NATIVE scope comparisons/contracts/
bounded delegated development walkthrough completed; signed exact-head
post-push/hosted checks/actual clean external review remain integration gates.
No independenthuman/high-hazard/system/parent/hazard/KBX/
release closure or whole-call/callback atomicity claim.
Current documented walkthrough/self-review is complete under the owner's
bounded-medium delegated development allowance: full GNU70/70/521.24s,
both144 scope comparisons/both six groups/all33hashes/fivecontracts5/5/339.63s
and independent native/parent boundary checks retained. It is not independent
human/high-hazard qualification. Subsequent full fixedClang70/70 PASS3230.17s
completed20:50:11Z/all198-four556 counters/all33hashes/no diagnostic (leaks
disabled). Signed exact-head integration gates remain, registered hazards
active/unchanged; separate7163/7164 unadmitted, no shared-state/type repair.

RQ-CF-PRG-TOPITEMID-NUMERIC-001 / DQ-DV-5611-TOPITEMID-NUMERIC-001 links active
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 unchanged to the
[independent pre-code TopItemID map](../../tests/fixtures/vfp9-topitemid-numeric/README.md).
Bounded documentation misuse medium: confuse unique identity/top row/focus,
clear instead of absent no-op, claim empty absolute parity despite7157 or
unsupported native CAST syntax. Checked/default/exact/NaN containment derived
from named parents; raw native ceiling/no-op/state requirements measured.
Distinct unused helper/frozen literal tests/BOTH complete65 original suites
before migration/fixed sanitizer/preservation/contracts/review gates required.
Both identical fixed65 GNU1371.49s/Clangsanitizers2731.66s, both144 excluded-scope
comparisons/six preservation groups/all31 hashes and fivecontracts5/5/324.11s
PASS; initial fivefrozen unchanged. Linked bounded delegated development
self-review/walkthrough completed, not independenthuman/high-hazard/
constructor/parent/KBX/hazard/release closure. Signed/external gates remain.

RQ-CF-PRG-TOPINDEX-NUMERIC-001 / DQ-DV-5611-TOPINDEX-NUMERIC-001 links
active HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 unchanged to
[independent TopIndex map](../../tests/fixtures/vfp9-topindex-numeric/README.md).
Relative top-row/no-op/focus preservation/readonly contract recovered before
code; checked32/default31/native46/exact64/NaN derivations prevent unchecked
conversion. Bounded documentation misuse medium includes falsely claiming
absolute empty-state compatibility despite7157. Distinct unused helper/frozen
regressions/BOTHcomplete60 originals before migration/fullfixed/sixgroups/
contracts/explicit review walkthrough/exacthead gates required. Bounded local
verification completed BOTH fixed60/sixgroups/all30hashes/fivecontracts and
delegated development self-review/automated walkthrough; signed/external
integration gates remain. No independenthuman/high-hazard/constructor/
default-state/parent/KBX/hazard closure.

RQ-CF-PRG-LISTITEMID-NUMERIC-001 / DQ-DV-5611-LISTITEMID-NUMERIC-001 links
active HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 to
[independent pre-code ListItemID requirements](../../tests/fixtures/vfp9-listitemid-numeric/README.md).
Fresh native pairs/help recover ID addressing, absent0 BOTH classes, no
ComboBox sentinel and accepted MultiSelect clearing ALL bits. Rejection46
preserves full state; checked default signed32/31, exact64/NaN are explicit
parent derivations. Bounded documentation misuse medium does not change hazard
severity/status. Distinct unused helper/frozen assertions, BOTH full55 originals
and repeated fixed/sanitizer/preservation/contracts/exact-head integration
evidence required before completion. BOTH fixed55 suites PASS GNU994.77s/
Clangsanitizers1825.00s; all284/four1216/both sixgroups/all27hashes complete,
initialfivefrozen unchanged. Five contracts PASS337.08s/210fragments;
linked boundedmedium delegated development self-review/walkthrough complete,
not independent-human/high-hazard signoff. Signed exact-head external gates
remain; no parent/KBX/hazard/release qualification claim.

RQ-CF-PRG-LISTINDEX-NUMERIC-001 / DQ-DV-5611-LISTINDEX-NUMERIC-001 links
active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01 to the
[pre-code ListIndex assignment map](../../tests/fixtures/vfp9-listindex-numeric/README.md).
Independent VFP9/help evidence distinguishes focus, absent-row sentinel,
MultiSelect bits, UINT32 raw ceiling and signed-boundary aliases; default
checked admission and exact64/NaN are derived before helper/assertions/config.
Bounded documentation misuse medium does not change registered severity/status.
Both complete49-subject originals were retained BEFORE bounded migration;
GNU45/49/759.68s and Clangsanitizer45/49/1393.60s, onlyfournewpublicfail/
identical16704 assertions, all frozen inputs and original production unchanged.
Adapter now wired; fixed attempt2 GNU45/49 retained with direct cases passing
and SETPEM-only failures. Selected Numeric/exact SETPEM callback routing now
repaired with frozen expectations unchanged, full suites repeated. Identical
fixed GNU49/49 PASS760.09s and Clang ASan/UBSan/float-cast-overflow49/49
PASS1409.70s/all counters complete; both six preservationgroups PASS,
frozen/source/neighbor hashes checked. Boundedmedium delegated development
DQ/DV self-review and automated walkthrough complete; five contracts PASS332.88s. No
independent-human, persistence, callback atomicity, high-hazard, parent/KBX/hazard
closure or release qualification claim.

Head967 hosted full Linux/macOS failure is isolated to five obsolete
non-frozen empty-control smoke expectations; corrections use the pre-code
native ComboBox1/ListBox0 map. Both unchanged before diagnostics reproduce
five failures, both after pass; no assertions/setup/production/frozen inputs
removed or changed. Expanded50-subject GNU/Clang verification and integration
gates subsequently passed. This does not change hazard classification or closure.
Expanded GNU50/50 PASS769.56s and Clang sanitizers50/50 PASS1445.84s,
full runtime_surface/all counts/both sixgroups/all25 hashes checked; repeated
contracts5/5 PASS332.66s. Development self-review rechecked without procedural,
production, setup/assertion or original-freeze change. PR7155 mergedd8005f994
at signed1d6f22dbf after postpush50/50/765.73s/all25hashes, all11required
uniquelySUCCESS, actual clean verified Codex6115825243 after Claude-FIRST/
fallback and threads0. Optional fullplatform pending atmerge, not retroactive
green, independent-human/high-hazard qualification or hazard closure.

RQ-CF-PRG-LIST-INDEX-NUMERIC-001 / DQ-DV-5611-LIST-INDEX-NUMERIC-001 links
active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01 to the
[pre-code one-argument display-row map](../../tests/fixtures/vfp9-list-index-numeric/README.md).
Independent dense/sparse observations distinguish display rows from item IDs;
default checked nonnegative32 admission and explicit VFP9 low32/raw-ceiling
policy are mapped BEFORE helper/assertions/configuration. Bounded documentation
misuse medium does not alter registered severity/status. BOTH full originals
were retained before bounded migration, each40/44 with identical3552 selected
failures and all39 unchanged subjects passing. Identical fixed GNU44/44 and
Clangsanitizer44/44 PASS/all counts complete/zeroFAIL/no sanitizerreport;
both six unchanged preservationgroups and fivecontracts PASS. Boundedmedium
delegated development walkthrough/self-review complete in retained phase audit,
not independent-human signoff. PR7154 merged after all11required SUCCESS,
actual clean exact-head Codex6112438131/head2b8bb142ea, resolved conversations,
and signed-head postpush44/44 PASS; full Linux/macOS554/554 PASS, optional
Windows pending atmerge, later exact-head555/555 PASS1838.02s/raw/hash retained
and completedwatcher recoverably cleaned. No independent-human,
persistence, callback atomicity, high-hazard,
parent/KBX/hazard closure or release qualification claim.

RQ-CF-PRG-LISTITEM-NUMERIC-001 / DQ-DV-5611-LISTITEM-NUMERIC-001 links
active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01 to the
[pre-code one-argument item-ID map](../../tests/fixtures/vfp9-listitem-numeric/README.md).
Default checked nonnegative signed32 admission avoids unintended aliases;
native positive raw ceiling/negative low32 aliases are independently observed
explicit VFP9 policy. Sparse identity controls distinguish item IDs from rows.
Bounded documentation misuse medium does not alter registered severity/status.
Both full originals were retained before migration; fixed GNU39/39 and Clang
sanitizer39/39 plus both six unchanged preservation groups substantiate the
completed development walkthrough/delegated maintainer self-review, not
independent-human sign-off. Documentation contracts pass5/5/333.19s with full
raw/hash retained. PR7152 merged after signed-head postpush39/39/all11required/
actualcleanexact-headCodex6110506059/resolvedconversations; optional fullWindows
pending atmerge. Type gap#7151 is
separate/unadmitted. No persistence,
whole-call callback/materialization atomicity, high-hazard qualification,
parent/KBX/hazard closure or release qualification claim.

RQ-CF-PRG-INDEXTOITEMID-NUMERIC-001 / DQ-DV-5611-INDEXTOITEMID-NUMERIC-001
links active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01
to the [pre-code IndexToItemID row mapping](../../tests/fixtures/vfp9-indextoitemid-numeric/README.md).
Default checked signed32 avoids unintended aliases without changing ordinary
negative/missing-row zero semantics. Both positive and negative aliases are
independently observed explicit VFP9 policy, not SelectedID's ceiling domain.
Sparse IDs distinguish row position from identity. Medium bounded documentation
misuse does not alter registered severity/status; both complete originals are
retained before bounded migration. Fixed GNU29/29 and Clang sanitizer29/29 plus
both six unchanged preservation groups complete the bounded development
walkthrough/self-review. Five documentation contracts PASS335.83s with full
raw/hash retained; exact-head integration remains pending. No persistence/whole-call materialization,
independent-human/high-hazard or release qualification claim.

RQ-CF-PRG-SELECTEDID-NUMERIC-001 / DQ-DV-5611-SELECTEDID-NUMERIC-001
links active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01
to the [pre-code SelectedID item-ID mapping](../../tests/fixtures/vfp9-selectedid-numeric/README.md).
Default checked admission avoids unintended aliases; independently observed
negative aliases are explicit VFP9 policy. Sparse identity controls prevent a
row-index requirement from being substituted for an item-ID requirement.
Bounded documentation misuse medium does not alter registered severity/status.
Both full originals retained before migration; fixed GNU23/23 and Clang
sanitizer23/23 complete the bounded walkthrough with frozen inputs unchanged.
Six unchanged preservation groups pass both builds; delegated development
self-review complete, not independent-human signoff. Documentation contracts
pass5/5 at the retained checkpoint; exact-head external integration remains
gated. No whole-call callback/materialization atomicity,
persistence, independent-human/high-hazard or release qualification claim.

RQ-CF-PRG-SELECTED-INDEX-NUMERIC-001 / DQ-DV-5611-SELECTED-INDEX-NUMERIC-001
links active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01
to the [pre-code Selected read-index mapping](../../tests/fixtures/vfp9-selected-index-numeric/README.md).
Independently observed VFP9 raw ceiling/negative aliases are available only in
explicit compatibility mode; default checked admission is parent-derived.
Bounded documentation misuse medium does not alter registered severity/status.
Both full originals retained before migration; fixed GNU18/18 and Clang
sanitizer18/18 complete the bounded walkthrough with frozen inputs unchanged.
Delegated development self-review is not independent-human signoff.
Documentation contracts5/5 pass at the retained precommit checkpoint;
exact-head hosted/review evidence remains pending. No whole-call callback/
materialization atomicity, persistence, independent-human or release claim.

RQ-CF-PRG-ITEMDATA-INDEX-NUMERIC-001 / DQ-DV-5611-ITEMDATA-INDEX-NUMERIC-001
links active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01
to the [pre-code read-index mapping](../../tests/fixtures/vfp9-itemdata-index-numeric/README.md).
Default checked nonnegative signed32 admission avoids aliases; explicit VFP9
positive raw ceiling/negative low32 aliases are observed independently.
Bounded documentation misuse medium does not alter registered severity/status.
Both final originals retained before migration; GNU13/13 and Clang sanitizer
13/13 verify the bounded walkthrough, not arbitrary callback/materialization
atomicity. Documentation contracts5/5 pass with checkpoint limitations in the
audit; hosted exact-head regression/review gates remain pending;
no independent-human/high-hazard/release or parent acceptance claim.

RQ-CF-PRG-GETCOLOR-DEFAULT-NUMERIC-001 / DQ-DV-5611-GETCOLOR-DEFAULT-NUMERIC-001
links active HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01
to the [pre-code RGB/headless/native boundary](../../tests/fixtures/vfp9-getcolor-default-numeric/README.md).
Checked default unsigned24 RGB admission avoids accidental aliases; explicit
VFP9 low24 projection is independently observed and parent-defined for exact
extended kinds/NaN. Medium bounded documentation misuse does not alter the
registered severities/status. Both full originals were retained before
Numeric-only migration; fixed GNU/Clang bounded walkthroughs pass, all frozen
inputs unchanged. Documentation contracts pass with disclosed fragment-format
failure/repair and retained full raws. Exact-head hosted/review integration
gates remain pending; no independent-human/high-hazard or release qualification
is claimed.

RQ-CF-PRG-GETFONT-SIZE-NUMERIC-001 / DQ-DV-5611-GETFONT-SIZE-NUMERIC-001 links
HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01 to the
[pre-code GETFONT input/extension boundary](../../tests/fixtures/vfp9-getfont-size-numeric/README.md).
Default checked positive signed32 request admission prevents accidental wraps;
legacy aliases and native positive admission ceiling are explicit VFP9 policy.
Headless request events are not native Font-dialog/backend parity. Bounded
documentation misuse is medium; registered severities/status remain unchanged.
Both frozen original/fixed compiler results and the completed bounded
development walkthrough are retained in the fixture. Broader/exact-head
integration remains gated; no independent-human/high-hazard or release
qualification claim.

RQ-CF-PRG-CAST-INT16-NUMERIC-001 / DQ-DV-5611-CAST-INT16-NUMERIC-001 links
HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01 to the
[signed16 extension boundary and verification plan](../../tests/fixtures/vfp9-cast-int16-numeric/README.md).
Both modes use checked Numeric/exact signed16 admission and localized atomic11;
native target absence is not a wrapping oracle. Bounded documentation misuse
is medium; registered severities/status remain unchanged. Full original/fixed
focused/broader evidence and completed development walkthrough are retained
in the fixture; exact-head integration remains gated, no
independent-human/high-hazard qualification claim.

RQ-CF-PRG-CAST-BYTE-NUMERIC-001 / DQ-DV-5611-CAST-BYTE-NUMERIC-001 links
HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01 to the
[unsigned8 extension boundary and verification plan](../../tests/fixtures/vfp9-cast-byte-numeric/README.md).
Numeric/exact range admission prevents undefined conversion/silent low-bit
aliases; both modes agree because native VFP lacks these aliases. Bounded
documentation misuse is medium; registered hazard severities/status remain
unchanged. Development walkthrough and full original/fixed/broader evidence
are retained in the fixture; exact-head integration remains gated. This is
not independent-human/high-hazard qualification.

## Investigation Checklist

When investigating an incident potentially related to product behavior or documentation:

1. Identify all linked `HZ-*`, `DQ-*`, and `DV-*` ids.
2. Retrieve issue evidence ledger (commits, reviewers, artifacts, walkthrough results).
3. Compare deployed documentation revision against verified procedural delta map.
4. Confirm rollback and field-notification actions were triggered when needed.
5. Record findings back into the closing issue and changelog entry when applicable.

## Release Gate Use

Run `scripts/validate-safety-traceability.ps1` before release tagging (or run the `Safety Traceability Gate` workflow) against the intended documentation-traceability issue set to verify its current DQ/DV contract. Separately verify that every release-affecting behavior change has requirement/code/test/result traceability in the durable matrix and that unresolved gaps are disclosed. The current script verifies:

1. `DQ-*`, `DV-*`, and `HZ-*` presence for safety-relevant documentation issues
2. hazard ids resolve in this register
3. primary hazard coverage includes at least one of `HZ-data-corruption-01`, `HZ-system-failure-01`, or `HZ-runtime-crash-01`
4. investigation-ready evidence sections are present

Do not pass every issue carrying the broad `safety` label. That label also covers implementation and test lanes; the validator selects documentation issues through the `documentation` label, a `[docs-safety]` title, or `DQ-*` identifiers in the issue body.

## Bounded Windows native CI scheduling linkage

RQ-CF-CI-WINDOWS-NATIVE-BUDGET-001 / DQ-DV-CI-7181-001 links active
HZ-system-failure-01/HZ-doc-command-01 unchanged to the
[budget-repair requirement/evidence/walkthrough](../../tests/fixtures/windows-native-test-shards-7181/README.md).
Documentation misuse MEDIUM: report partial/one-shard/synthetic/inventory-only
results as full native/release qualification, omit the aggregate or erase
previous failures. Retain test timeouts/assertions/serial policy; require
matching full inventories and successful exact-once execution. Local mutation
and actual tiny CTest/aggregate checks are tool evidence, not hosted/native
product acceptance. Hosted3+aggregate/exact-head review/integration remain
gates. Delegated development self-review is not personal owner/independent
human sign-off; no hazard severity/status or system/release assurance closure.
