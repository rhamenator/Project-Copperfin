# Agent Handoff

## Branching (changed 2026-10-01)

Develop on `main`: branch from `origin/main` and open pull requests with
`--base main`. `v1-development` is retired (see
`docs/v1-development-retirement-2026-10-01.md`); older text below that says a
slice "merged into `v1-development`" is historical.

## Extension intent (direct owner steering, 2026-10-06)

VFP compatibility is the foundation, not the feature ceiling. Copperfin
extensions deliberately fix VFP problems and fill gaps with modern development
tools and platforms, borrowing suitable ideas from other languages/platforms
where useful; some gaps require substantial capabilities. Absence of native
VFP syntax or precedent is not grounds to skip or demote an extension. For the
selected admitted work, document derived extension requirements, boundaries,
interoperability and verification from owner product intent; use installed VFP
as the oracle for the equivalent legacy behavior, not as an oracle for an
extension it does not implement. This steering is additive: finish the retained
EPOCH query/numeric slices and then #3698 CENTURY/ROLLOVER. It does not admit
unrelated production work or waive acceptance/review evidence.

## Review routing (owner steering, 2026-10-06)

The owner reports GitHub review availability exhausted for the rest of
October 2026; the exhaustion start date is unknown. The owner will task Claude
with reviews. For new/revised slice heads through October 31, retain a Claude
or owner-authorized fallback Codex review tied to the exact commit and address
its findings before merging;
all 11 required checks and resolved GitHub conversations remain gates.
Unavailable/skipped GitHub bot reviews are not clean-review evidence. Preserve
actual historical review records without guessing when availability ended.
The owner's subsequent instruction authorizes requesting Claude review when
creating each PR. Use a verified Claude reviewer identity if available; the
official Claude Code Review integration documents a top-level @claude review
comment on an open, non-draft PR as its manual trigger. Do not assign an
unverified GitHub account named Claude or silently enable a paid integration.
The current CLI collaborator list contains only rhamenator, no Claude workflow
was found under .github, and app-installation lookup returned 401; installation
or quota status therefore remains unconfirmed, not known absent/exhausted.
The owner additionally authorizes Codex as the fallback when Claude is not
working. Try Claude first, then request Codex through the repository's verified
@codex review integration if Claude is unavailable or quota-blocked. Do not
assume reported monthly exhaustion has reset or treat quota notices as review.
Report explicit quota/setup failures with PR URL, exact head and message;
if neither service can review, wait for the owner's terminal Claude review.
No response is not a completed clean review. Do not independently launch
terminal Claude or create/message another chat. Request fresh exact-head review
after fixes; all required checks and conversation-resolution gates still apply.
The subsequent direct owner instruction resumes the EPOCH slice and retains
both EPOCH and CENTURY (see selected work below), without authorizing unrelated
production work. Reconfirm review availability/routing
after October rather than assuming quota restoration.

## Last shipped slice

BITOR Numeric/exact PR#7112 merged2026-10-09T11:16:33Z as
b4cd165e39ac64f5e474d88a141d991c206efcae from signed/DCO G head
d190d18b1a7684938bb086b96af76717d062196d.
https://github.com/rhamenator/Project-Copperfin/pull/7112
Exact-head merge gate6079783442: live ruleset20356131 all11 required contexts
SUCCESS/strict;35/36 totalSUCCESS atmerge, optional Windows Native37917231785/
job113776290595 stillrunning then. Final WindowsSUCCESS11:36:24Z/in1h13m33s;
all36/36SUCCESS exacthead, proof6080220178; watcher27083 harvestedexit0.
macOS Native succeeded10:52:49Z; no actionable hostedfailure/timingcauseclaim.
Actual clean Codex6079409420 at10:51:04Z reviewed d190d18b1a, following
Claude-FIRST6079004222 and authorized fallback6079385928. No explicit Claude
setup/quota failure known; Summary6079001325 alone NOT clean review evidence.
Paginated REST reviews/inline empty; Graphthreads0/closingrefs0/hasNextfalse.
Parents5611/6776/family6871 OPEN/rhamenator/agent-approved fresh11:15Z, no closure.

Independent installed WineVFP9 7423/shipped help; three matching108-line/106call
runs. Frozen104direct/472public plus2unchangedneighbors; BOTH originals before
BITOR-only migration372identicalselectedFAIL/zeroother/no diagnostics, GNU6.03s/
Clang10.80s. FixedGNU15/15PASS429.38s/Clangsanitizers7/7PASS131.91s,
leaksdisabled/no diagnostics. No frozeninput repairs. Source/native/original/
fixed hashes, RQ/VR and completed bounded medium-misuse development DQ/DV
self-review/walkthrough/rollback in tests/fixtures/vfp9-bitor-numeric/README.md
and baseline-audit.md. Not independenthuman/highhazard/fullnative/platform/leak/
release/parent acceptance. Signedpostpush12/12PASS16.82s proof6079022532,
rawlogSHA82395526bb56b419512b945ae5e92f0b1478839e7cc88fa97314c4d00ac3908f.
Shared helpers/siblings/type/Currency/binary/arity/NULL unchanged;
7111arity remains unadmitted/unimplemented.

Main FF synchronizedb4cd165e3; completed WT/local/remote branch removed.
410MB GNU/313MB Clang builds recoverablytrashed at
/home/rich/.local/share/Trash/files/bitor-gcc and bitor-clang; postpush log
preserved/hashverified there. Own pre-sync handoff pathstash
218036382bdcc290551ed62351e2fec5c7dc38e3 retained; olderstashes/foreignfiles/WTs
preserved. Livechannel rereadempty. NoVM or owner-input blocker.
Prior BITAND7110 all36SUCCESS proof6077724390/gate6077580073; actualclean6077084324;
trashbitand-gcc/-clang/stash86b65aeaed5d451d35b0a564ad3c3433053189eb retained.
BITNOT7107/Collection7105/GETBAR7103/PRMBAR7101 prior proof/stashes/trash retained.
EPOCH/CENTURY complete/#3698closed;#6879 after numeric. 7108/7106/7104 unadmitted.

## Selected active bounded slice

BITXOR Numeric/exact derived from admitted5611/6776/6871, metadata fresh13:02Z
OPEN/rhamenator/agent-approved. Baseorigin/main unchanged:
b4cd165e39ac64f5e474d88a141d991c206efcae. Branch fix/bitxor-numeric-5611;
dedicated gitworktree /home/rich/.codex/worktrees/bitxor-numeric-5611/Project-Copperfin
createdfromorigin/main. No newcommit/push/PR yet; no managedattachment assumption.

Independent installed WineVFP9 7423/shipped BITXOR help
b7858458-a38e-492c-b8b6-0d6c42324fb6.htm recovered BEFOREhelper/tests:
initial plus3matching112-line/110call native runs,36Numeric first/second
identity0, all26fractional slots,12mixed/cancellation controls. Exact64/NaN/
default safety parent-derived, not nativeextension syntax. Full source/outputs/
hashes in tests/fixtures/vfp9-bitxor-numeric/README.md and baseline-audit.md.
NoVM/credentials/binaryinspection; existing25sharness/nice10/ionice2:7/serial.
RQ-CF-PRG-BITXOR-NUMERIC-001 mapped docs32 BEFOREhelper/tests/migration.

Six helper/header/assertion/test/CMake/isolation inputs frozen BEFOREconfig,
newcheckedhelper UNUSED byBITXOR throughBOTHcomplete originals. No frozeninput
repair/removal/weakening. Both104direct/488public,388byte-identical selectedFAIL
(86error/302result;278COPPERFIN/110VFP9), zero unrelated/no sanitizerdiagnostics:
GNU16467 exit8,total8.13s; Clangrepeat56048 exit8,total365.45s.
Full439-line logs hashes c06ac791f68484c267c5c648c2e9418a9ef179a4dc6b6decedfc9cadd5c6849b
and a10fea1c2f9cf513a8f72c68244fd2ae68e15e0711f65201293485bae7c11eb2.
FirstClang50846 TIMEOUT180.38s/incomplete retained separately, SHA
6b413db0647be9a3a90c0414c928f9ebc08a17653d313da4fd745ec099cd3ff4;
not complete evidence/rootcause/optimum claim. SAMEbinary/flags/inputs repeat
CLI600s only; no workflow/test-definition timeout change.

THEN BITXOR-only selectedsession mode/everyoperandchecked/uint32XOR/
definedsigned32int64/localized11 migration. DispatcherSHA
21bf6be3e0e4245e8653c1a1dc1011d254e66d98c6d44b3aa8795f53d7b96667.
Sharedhelpers/siblings/type/Currency/binary/arity/NULL unchanged; safe other-type
controls NOTnativeparity. Allsix/sharedplatform hashes unchanged throughfixed.
FocusedGNU89562 exit0,2/2PASS348.05s/104direct488public; journalwaitsample
retained without timeoutcauseclaim. FullfixedfocusedlogSHA
1a85690d96e52d93e5438e477c3a014c30052cd152e53b50c178b919ed30b04d.
BroaderGNU63626 exit0,16/16PASS449.11s, olderNumeric/NULL/BITOR/BITAND/BITNOT/
Collection/localization/7contracts. Safety324.55s existing1200s cap unchanged.
Full204-line fixed-gcc.log SHA
be02a6656a7a3d1927609750fcd517fb273a0691d560722104e2f0bb7b52e053.
THEN strictlyserialClang73151 exit0,8/8PASS180.93s/no diagnostics/leaksdisabled,
actualBITXOR104/488/BITORandBITAND104/472/BITNOT104/156/Collection116/140/26.
Full112-line fixed-clang.log SHA
88fc6383f8e99f724c6f731d1d53dc7d0698c50f521cf922c80fe3d0b3142e58.
MatchingC/CXX/EXE/SHAREDASan/UBSan/float-cast-overflow/O1g0/framepointers,
privateTMPDIR/nice10/ionice2:7/-j1/CLI600s/abort/halt/stacktrace. No activejob.
DQ/DV boundedmedium-misuse developmentself-review/GNU-Clangwalkthrough/rollback
complete, not independenthuman/highhazard/fullnative/type/binary/arity/platform/
leak/release/family/parent acceptance. Docs22/32/fragment updated; source
whitespace/changelog/channel checks passed. #7115arity source/help mismatch
filedagainstmain, OPEN/owner/bug/runtime/NOagent-approved, unadmitted/
unimplemented; native26 verified, not27rejection/errornumber. No active scope expansion.

NEXT signed/DCOcommit/push/PRbase main, ClaudeFIRST; signed exact-head local
proof then actualcleanClaude or authorizedverifiedCodexfallback review,
all11requiredgreen and allconversationsresolved beforemerge. No parentclosure.
Completed reviewfixes require exactchangeverification beforeconversationresolution.
Main remainsb4cd165e3/foreignfiles/WTs/stashes preserved; noowner-input blocker.
PriorPR7112 all36SUCCESS proof6080220178; no activehostedwatcher.

## Retained owner steering / deferred work

Audit candidates7100 lifecycle output-drain timeout and7102 designer-smoke
descendant cleanup were OPEN/rhamenator/agent-approved at00:47Z; revalidate
before intake. Deferred until after bounded numeric work unless directowner
redirects or activeCI actionablefailure. Not explanations/fixes for7053;
timeout increase is not their acceptance. Audit chat owns discovery metadata
and scheduled Cloud Defect Hunt reviews; don't duplicate acknowledgments/edits.
Hosted VSIX40min owner policy shipped7099, not measured optimum/rootcause.
LocalVSIX timing still deferred until safe full independent clone and capacity.
VM policy: use briefly only when necessary, checkactiveVMs before starting;
avoid overlap/interruptingforeignVM; stop ours afterfocuseduse. Disk contention
required owner reboot. NoVM used thiscontinuation. Interrupted SKPBAR partial
clones recoverable/NEVER boot; originals/template/firmware/backups preserved.

PR #6926 (`fix/array-dimension-overflow-5594`) merged into `main` as
`2c56331d57c0ef459ae55d38236a012b77c129b1` on 2026-10-04 and closed #5594.
It completed the array-dimension safety slice under #5611/#6776 with checked
dimension conversion, checked size/offset arithmetic, a bounded host ceiling,
and failure-atomic declaration, resize, RESTORE, and native-class array paths
while preserving sequential multi-target visibility. All required checks were
green at exact head `1c2dfaa07`; exact-head review was clean and all review
conversations were resolved before merge.

PR #6923 (`fix/bintoc-ctobin-contract-5766`) merged into `main` as
`5d64d0e97b3a5a6baf9ed91119e954035e6a2a09` on 2026-10-04 and closed #5766.
It completed the `BINTOC()` / `CTOBIN()` numeric-conversion slice under
#5611/#6776 with bounded flag/width validation, canonical sortable and native
byte order, exact Currency handling, and localized catchable errors. All
required checks passed at exact head `a813944b0f`; all review conversations
were resolved before merge.

PR #6920 (`fix/declare-numeric-boundaries-6050`) merged into `main` as
`a2641cc9a9a03c2dfd5254fdad8f6d6be9f7994f` on 2026-10-03 and closed #6050.
It completed defined native/managed `DECLARE` integer conversion under
#5611/#6776, including low-32-bit VFP9 conversion, checked 64-bit admission,
native by-reference initial values, portable source contracts, and focused
Windows native/managed fixtures. All 33 checks passed at exact head
`8d8294ce4c9b72eda2b3bd53dd68f0d75f1da22e`, exact-head review was clean, and
the sole review conversation was resolved before merge. The review fixes made
finite out-of-int64 INTEGER conversion mode-aware and made the Windows-only
marshaler read `NUMERICBEHAVIOR` from the session SET state in scope.

PR #6917 (`fix/installer-artifact-policy-6905`) merged into `main` as
`6b64a176d680aab732e0772cd889a423ed3c993e` on 2026-10-03. Successful
pull-request installer packages/evidence/source now retain for 7 days,
successful non-pull-request outputs for 30 days, and exact failure packages
plus available diagnostics for 14 days; the immutable RC bundle remains a
separate 90-day artifact. All 36 checks passed, exact head `ca3bbd3cb` passed
Codex review, and all four review conversations were resolved before merge.

PR #6916 (`fix/linux-rpm-lifecycle-6905`) merged into `main` as
`ec556e17214266274508c430529a9f3912805e19` on 2026-10-03. It completed the
fourth #6905 installed-product slice with a distinct native RPM lifecycle in an
ephemeral digest-pinned Fedora `linux/amd64` container: exact package and
container-image identity, fresh native install, installed
tree/catalog/semantic command smoke, `rpm --verify`, same-version reinstall,
external-fixture preservation, erase, and RPM-database/filesystem residue
checks. Exact head `a5ca7e859` passed all 36 checks, including the native RPM
lifecycle; all four review conversations were resolved before merge.

PR #6910 (`fix/windows-installed-ui-6905`) merged into `main` as
`ce15bab04fc14db26c81081b1dc5f067b7fc6c88` on 2026-10-03. Hosted run
`37127435107`, job `111215503606`, passed the complete Windows installer and
installed-Studio UI lifecycle at implementation head `2806a4a31`; the
retained evidence binds installer SHA-256
`402a4dc2a399b92328ed768db8334590cefd34c7b174c62b761c62ef9d44763c` and
installed Studio SHA-256
`2d939edb402d31abd126322ed4567919be817dabbcc59b96ed31d8fb3a99b2f9`.
All four review conversations were resolved, exact-head review found no
further issue, and the final docs-only head passed all 33 checks. #6911 owns
the inherited hosted-runner authority gap and #6913 owns standard accessible
roles; human GUI remains `NOT_RUN`.

PR #6908 (`fix/macos-installer-lifecycle-6905`) merged into `main` as
`ed9a12512761197f7214b638bf43f413d0917f56` on 2026-10-03. Hosted run
`37112542106`, job `111173111329`, passed the exact lifecycle at implementation
head `354c35105`; retained evidence binds PKG SHA-256
`4e8f9556d1fd2a333a572e674fd24d37ba2a0e7ad2ffa96def14f0456fe07f38`.
Final docs-only head run `37114247412` also passed all installers; all review
conversations were resolved before merge.

Issue #6905's first installed-product slice (PR #6906, merge `3af2b9609`)
finished 2026-10-03: the exact generated Linux DEB now undergoes a disposable
hosted-runner fresh install, installed-tree/English-catalog/semantic CLI smoke,
same-version reinstall, external-fixture survival, purge, and residue checks.
Digest-bound evidence is admitted into the RC bundle; all six review threads
were resolved and the exact lifecycle plus required checks passed.

Earlier on 2026-10-03, PR #6894 merged into `main` as `5807fbedf`. It
completed the `CHR()` numeric-boundary slice under #5611/#6776, including
VFP9-mode modulo wrapping, Copperfin-mode out-of-range rejection, focused
sanitizer coverage, and the review-requested additional wrap cases. All review
conversations were resolved and all required checks passed.

2026-10-02 session (all merged into `main`): cluster 25 finished (#6035 via
#6769, plus #6755 and #6760); installer lifecycle CI fixed (#6761, #6696);
macOS lane made green (#6762 `VAL(-1E-308)`, #6764 PowerShell host discovery,
#6768 invalid-UTF-8 fixture); Windows UTF-8 test fixtures fixed (#6774, #6767);
string length bounds (#6775: #5595, #6003, #6004); `SET NUMERICBEHAVIOR` and the
`LEFT`/`RIGHT` family (#6777); `STUFF`/`STUFFC`/`SUBSTR`/`SUBSTRC` start
boundaries, `STR` decimals above 18, `GETWORDNUM`/`MLINE` index, and defined
saturating conversions across the string functions (#6778, #3704). #5598 was
fixed by #6769 and is pinned by script rows. Evidence for every VFP9 claim is
retained under `~/temp/vfp9-probes/` (see Probing VFP9 below).

Issue #6593 (PR #6683, merge `fe91230ea`) finished 2026-09-27: `APPEND FROM
... TYPE CSV` now discards BOM-free field-name rows when selected target names
appear in a different order, while subsequent values remain positional in both
local DBF and selected SQL/result cursor paths. Focused and hosted checks
passed; review completed without actionable conversations.

Issue #6598 (PR #6681, merge `af0a22dd3`) finished 2026-09-27: `APPEND FROM
... TYPE CSV` now appends blank physical data records after the header in both
local DBF and selected SQL/result cursor paths, while header-only sources
remain zero-row and DELIMITED behavior is unchanged. Focused and hosted checks
passed; the channel-mirror review finding was fixed and resolved.

Issue #6602 (PR #6680, merge `2bcc34c6c`) finished 2026-09-27: `APPEND FROM
... TYPE SDF` now recognizes CR-only physical records alongside CRLF and LF
without changing blank-record or Memo-target behavior. Focused and hosted
checks passed; review completed without actionable conversations.

Issue #6519 (PR #6678, merge `37a77a193`) finished 2026-09-27: unquoted
Character data in CSV and DELIMITED imports now preserves following literal
quote bytes while retaining each format's enclosed-field rules. Focused and
hosted checks passed; both review conversations were fixed and resolved.

Issue #6522 (PR #6670, merge `b3c1ed73d`) finished 2026-09-27: enclosed doubled
quotes in `APPEND FROM TYPE CSV` now preserve both VFP9-observed quote bytes for
fixed-width Character targets without changing DELIMITED, TAB, unquoted,
Varchar, or Memo behavior. Focused and hosted checks passed; review completed
without actionable conversations.

Earlier: issue #6665 (PR #6667, merge `d99c78ac2`) finished 2026-09-27:
enclosed doubled quotes in `APPEND FROM TYPE DELIMITED` now follow VFP9's
format-specific truncation rule without changing TAB or unquoted-field
handling. Focused and hosted checks passed; all review conversations were
resolved.

Earlier: issue #6604 (PR #6666, merge `780d6ac12`) finished 2026-09-27: SDF
Logical export now emits VFP-compatible uppercase `T`/`F` bytes while
preserving adjacent fixed-width Numeric formatting. Focused and hosted checks
passed; the two review documentation mismatches were fixed and their
conversations resolved.

Earlier: issue #6623 (PR #6664, merge `91c0f7165`) finished 2026-09-27: blank SDF
Numeric/Float cells now import as non-NULL zero at the target field's scale,
including nullable Numeric targets, without shifting later fields. Focused and
hosted checks passed; all review conversations were resolved.

Earlier: issue #6636 (PR #6663, merge `21ba05355`) finished 2026-09-27:
nullable DIF numeric, Character, and Logical cells now match VFP9, while SYLK
emits blank `K` cells without shifting later fields. Review added explicit
stale-physical-byte protection for nullable Character/Varchar fields. Focused
and hosted checks passed; all review conversations were resolved.

Earlier: issue #6609 (PR #6662, merge `c3e62e4e7`) finished 2026-09-27:
text-to-Memo SDF/CSV imports now use an explicit session-scoped VFP/Copperfin
policy, strict CSV enclosure preflight, localized pre-mutation diagnostics,
and focused DBF/FPT atomicity coverage. Focused and hosted checks passed; all
review conversations were resolved.

Earlier: issue #6612 (PR #6661, merge `9b028f5eb`) finished 2026-09-27: SDF
Autoincrement fields now use VFP's 11-character printable layout while
preserving genuine `0x31` metadata and raw bytes. Focused and hosted checks
passed; all review conversations were resolved.

Earlier: issue #6614 (PR #6660, merge `cf2b95187`) finished 2026-09-27: SDF now uses
Varchar/Varbinary payload widths, uppercase hexadecimal Varbinary text, and
validated case-insensitive hex import with command-atomic rejection of bad
input. Focused and hosted checks passed; the review diagnostic fix was
verified and its conversation resolved.

Earlier: issue #6628 (PR #6659, merge `59fe248ed`) finished 2026-09-27:
DIF/SYLK Date and DateTime export now uses VFP-compatible typed cells,
including blank DateTime handling and the Excel-1900 leap-day discontinuity.
Focused and hosted checks passed; all review conversations were resolved.

Earlier: issue #6630 (PR #6658, merge `0b2a52244`) finished 2026-09-27: CSV and
DELIMITED export/import now project Blob (`W`) fields with the same native
rules as General/Picture, including the parallel SQL-result import path.
Focused and hosted checks passed; the review conversation was resolved.

Earlier: issue #6640 (PR #6657, merge `e3b765653`) finished 2026-09-27:
successful lossy `COPY TO` interchange projections now emit one localized,
non-blocking warning with stable structured output-type and ordered omitted-
field metadata, without disclosing payloads or changing output bytes. Review
extended the event metadata through debugger, Visual Studio, and headless host
protocols and added actual `TYPE TAB` coverage. Focused and hosted checks
passed; all conversations were resolved.

Earlier: issue #6654 (PR #6656, merge `c0a003b43`) finished native
CR-delimited DIF/SYLK imports with Varbinary targets and atomic rollback.

Reentrant-cursor-closure cluster 1 (`docs/81`), cursor-lifetime part,
finished 2026-09-24: #6321 (PR #6521), #6322 (PR #6524), #6331 (PR #6526),
#6242 (PR #6529), #6241 (PR #6534), and #6240 (PR #6536, which also carries
the `docs/81` status update and the changelog catch-up). All
merged into `v1-development` with their issues closed manually. Shared
pattern: capture a `CursorGenerationReference` before any user-code
evaluation and re-resolve it afterwards (plus `current_data_session` where
the command must stay in its session); on loss, raise the catchable
`{command} target work area not found` error before touching the cursor.
Lessons from the review rounds: (a) positional rollback of a provisional
row is unsafe once callbacks can PACK the target, so evaluate against a
`RecordEvaluationOverride` instead (#6322); (b) a leading-`&` visibility
expression is evaluated twice, so the shared evaluator re-resolves between
passes; (c) a new loss flag on a shared helper must be threaded to every
caller (GO/SKIP/relation walking), not only the one under repair. Each fix
was verified fail-then-pass with a local Clang ASan/UBSan build; after
halt-on-error stops at the first failure, isolate newly added scenarios to
prove each one independently.

Earlier: PR #6505 fixed #6047 (nullable DBF writes silently losing NULL,
storage layer; `RQ-CF-PRG-059`), with #6506 filed for the `PrgValue` NULL
semantics redesign (not started). #6496 (PR #6516) and #6497 (PR #6517)
have since merged too.

Earlier: PR #6503 fixed #5567 (legacy DBF import silently reactivating
deleted source records, merged `565d66f32`) and PR #6502 fixed #5631
(JSON export of an unknown/NULL logical value as `false`, merged
`232b7e704`); both issues closed manually.

Earlier still: PR #6500 fixed #6495 (concurrency state-sequence hunt,
child of umbrella #6498, merged `bac49af30ba1e80e2707d6d1d1a8a7b4fa0d19e4`);
#6495 closed manually. Surfaced and filed #6499 (cancellation inside an
explicit `FLOCK()`/`RLOCK()` retry loop silently swallowed instead of
halting), deliberately not fixed as part of that coverage-only slice.

Earlier shipped slices (#6459/#6458/#6460/#6389/#6388/cloud-validation/#6251)
all merged; #5680 remains open (partial). PR #6486 was closed without merge
after review found a macOS clone destination-identity gap.

## Active slice

Open owner-authored, `agent-approved` issue #5611 is the implementation
umbrella and reopened #6776 is the design/checklist reference. The `RGB()`
component numeric-validation correction merged in PR #6939 at `69b3c1510`
and closed #6938. All 33 checks passed at exact head `54c2d3225`, exact-head
review was clean, and there were no review conversations. The Currency-first
`MOD()` Numeric-divisor conversion then merged in PR #6941 at `3f48b096f`.
All 33 checks passed at exact head `2136b52de`, exact-head review was clean,
and all six review conversations were resolved after their fixes were verified.
The completed worktree, branch, scratch probe, and build directories were
cleaned up after merge.

The `RAND()` seed numeric-conversion slice merged in PR #6942 at `7ba689c48`.
All 33 exact-head checks passed and its one review conversation was resolved
after fresh VFP9 evidence proved the intended truncation-first ceiling. The
completed worktree, branch, probe, and build directories were cleaned up.

The `FILE()` / `DIRECTORY()` visibility-flag numeric-conversion slice merged
in PR #6947 at `68178c648`. All required checks passed at exact head
`cde062b21`, optional installer/package checks were also green, exact-head
Copilot review had no findings, and there were no review conversations. The
completed worktree, branch, scratch probe, and build directories were cleaned
up after merge.

The `FDATE()` optional type-flag numeric-conversion slice merged in PR #6948
at `ae4fdc5ab`. All required exact-head checks passed at `f9611161e`, all
three review conversations were resolved after their fixes were verified, and
the completed worktree, branch, scratch probe, and build directories were
cleaned up after merge.

The `FOPEN()` optional numeric mode-conversion slice merged in PR #6950 at
`f8877f04d`. All required exact-head checks passed at `6097cb7a0`, automated
review reported no findings, and there were no review conversations. The
completed worktree, branch, scratch probe, and build directories were cleaned
up after merge.

The `FCREATE()` attribute-conversion slice merged in PR #6952 at
`2a99e37f9`. All 11 required checks passed for final commit `1ccee0e4c`,
review found no further issue, and both conversations were resolved before
merge. The completed worktree, branch, probe and build directories were cleaned
up. Optional Windows MSVC was still running at merge; all other checks passed.

The `FSEEK()` offset/origin numeric-conversion slice merged in PR #6957 at
`7d4d77867`. All 11 required checks passed at exact head `a16e76fdf`, review
reported no findings and there were no conversations. Optional Windows MSVC
was still running at merge; every other check passed. The completed worktree,
branch and build directories were cleaned up. Failed-seek return parity is
separate gap #6956.

Shared file-handle numeric conversion merged in PR #6960 at `cbd5ee9b6`.
All 11 required checks passed at final signed head `4b8ca7248`; exact-head
review was clean, and the sole conversation was resolved after the
nonzero-FERROR preservation test fix was pushed and verified. Optional Linux
GCC, macOS Clang and Windows MSVC were still running at merge; other checks
were green. Main was synchronized, and the completed worktree, branches and
build directories were cleaned up (builds moved to recoverable trash).

`FCHSIZE()` size conversion merged in PR #6962 at `146367518` with all
11 required checks green at final signed head `1e74778d1`, clean exact-head
review and the sole conversation resolved. Optional Windows MSVC was still
running at merge; every other check passed. Main was synchronized and the
slice worktree/branches removed; builds were moved to recoverable trash.
The 101-call installed-VFP9 fixture and 56-row native/verified two-mode matrix
remain retained. Both focused suites pass normally and under Clang
ASan/UBSan/float-cast-overflow; old dispatch reports 366 failures and sanitizer
stops at 1E300. An unrelated macOS .NET-candidate test failed once on the
documentation-only head and passed a same-head job retry; diagnostic follow-up
#6964 is open, with no .NET changes in this slice. Return gaps
#5912/#5913/#5914/#5887/#5911/#6956/#6959 remain separate.

HEX numeric conversion merged in PR #6965 at `44d399248` with all 11 required
checks green at signed head `8c41ea70a`, clean Codex/Copilot exact-head reviews
and no conversations. Optional Windows MSVC was still running; every other
check passed. Main was synchronized and the slice worktree/branches removed;
both builds moved to recoverable trash. The 62 direct calls and 14 PRG rows
pass normally and under Clang ASan/UBSan/float-cast-overflow; old dispatch
reports 36 failures and sanitizer stops at the signed-64-bit ceiling.
review and no conversations. Optional Linux GCC and Windows MSVC were still
running; every other check passed. Main was synchronized, the completed
worktree/branches removed, and builds moved to recoverable trash. The retained
24-row installed Numeric probe supports truncation, positive non-wrapping and
negative wrapping/clamping only; operation/return gaps #6945/#6966 remain
separate. Its 66 boundary calls, two operation controls and 11 PRG rows pass
normally and under sanitizers. Old dispatch fails 26 semantic assertions;
unsupported FE_INVALID assertions were removed after optimized macOS ARM64
showed non-strict FP speculation (fixture README retains the diagnosis).

CPCURRENT Numeric/exact-integer selector conversion merged in PR #6969 at
`09555b5502a517777642fe8bf67411a90f66164c` on 2026-10-05. All 11 required
checks passed at signed head `8adbd5fefe60db8e24f868dc96a3ce4525c4dfd2`;
Codex and Copilot exact-head reviews were clean and there were no conversations.
Optional Windows MSVC was still running; every other check passed. Main was
synchronized, worktree/local/remote branches removed, and builds moved to
recoverable trash. Its 80 direct boundary calls, six query controls and 16 PRG
rows pass normally and under sanitizers; old conversion fails 86 assertions.
The 32-row native table remains retained. Query/type parity #6968 is separate.

Shared RELATION/TARGET Numeric-index conversion merged in PR #6972 at
`52daa2f7f6b39bfe232de3c65556be5aaf0fcdd6` on 2026-10-05. All 11 required
checks passed at signed head `0d1bd0e2e56a2bf4c1b88c07fce660b36cdc2e50`;
exact-head reviews completed without findings and no conversations remained.
Optional Linux GCC and Windows MSVC were still running at merge; every other
check passed. Main was synchronized, worktree/local/remote branches removed,
and both builds moved to recoverable trash. Its 68-result Windows native
fixture, 148 direct calls and 40 PRG rows remain retained; all three focused
suites pass normally and under sanitizers, with 208 old-dispatch failures.
Additive-order gap #6971 remains separate. Access365 finished its low-risk
temporary-cursor COM probe and is shut down; no overlay/system changes.

SET(TEXTMERGE) Numeric query-variant conversion merged in PR #6975 at
`91467cdc9219d0f0d579d3ee2c7a05ff6e6187d7` on 2026-10-05. All 33 checks
passed at signed head `167a2a53a51b50002ee88f76fc0706ea94d084a9`, exact-head
Codex review was clean and the sole handoff conversation was verified and
resolved. Main was synchronized, worktree/local/remote branches removed, and
both builds moved to recoverable trash. Its 34-result native fixture, 80 direct
calls and 20 PRG rows remain retained; focused normal/sanitizer suites pass,
with 102 old-dispatch semantic failures. #6973/#6333 remain separate.

ALINES Numeric/exact-integer flag conversion merged in PR #6978 at
`2a0c811dc422480402d325c3d11afb18457eec0d` on 2026-10-05. All 11 required
checks passed at signed head `ab33d01d944ca5e047f465e2be9916d68963ee07`;
Codex and Copilot exact-head reviews were clean and there were no conversations.
Optional Windows MSVC finished successfully after merge; all 33 checks passed.
Main was
synchronized, worktree/local/remote branches removed, and both builds moved
to recoverable trash. Its 36-result native fixture, 138 direct calls and
92 PRG rows remain retained; all three focused suites pass normally and under
sanitizers, with 52 old-dispatch semantic failures. No VM was used.

ADIR Numeric/exact-integer display conversion merged in PR #6980 as
`3aa571eaad1dd169b97491c5c3bd260882a2d4a2` on 2026-10-05 at 21:46:21 UTC.
All 11 required checks passed at signed head
`c3a37e04956ad707af7448954bc1bcbc9609faff`; exact-head Codex review was clean,
and all five Copilot conversations were verified/resolved. Broad native,
managed UI, sanitizer/fuzz/stress/migration and installer checks also passed
after infrastructure retries. Optional dynamic CodeQL actions/C#/Python jobs
could not acquire runners and GitHub rejects their rerun; C/C++ analysis passed.
Main is synchronized; the completed worktree/local/remote branches were removed
and 420 MB/787 MB builds moved to recoverable trash. Its 46-result native
fixture, 170 direct calls and 114 PRG rows remain retained; all three focused
normal/sanitizer suites pass, with 60 old-dispatch semantic failures. No VM used.

AFONT Numeric/exact-integer third-argument size conversion completed in PR
#6983. Its retained 54-call installed VFP9 fixture, 198 direct checks and 137
PRG rows are in `tests/fixtures/vfp9-afont-size-numeric-observation/` and
`tests/test_prg_engine_numeric_behavior.cpp`. Old dispatch failed 40
assertions; fixed normal suites passed 3/3 (7.20s) and Clang ASan/UBSan/
float-cast-overflow passed 3/3 (30.63s) without diagnostics. A handoff review
fix was verified and resolved; its changelog indentation failure was corrected
in the final signed head, which passed all 138 fragment checks and focused
5/5 validation (7.54s). Linux/macOS broad suites and VSIX lifecycle then
passed at the exact final head. Existing #3252/#6955/#6958/#6963 remain
separate; font/result/type/charset parity was not expanded.

FIELD first-argument Numeric/exact-integer conversion completed in PR #6989.
Its complete 58-call native fixture, 196 direct calls and 141 PRG rows remain
retained with docs/32 and docs/22 reverse links. Corrected old dispatch failed
41 semantic assertions (5.57s); the initial cursor-setup failure is explicitly
discarded. Normal GCC suites passed 4/4 (7.71s), and Clang ASan/UBSan/
float-cast-overflow passed 4/4 (34.02s) without diagnostics. Changelog,
contributor-signoff and channel contracts passed 4/4 (3.96s), with 139 valid
fragments. All required exact-head CI/review gates passed before merge;
completed optional native, managed, security, sanitizer/fuzz/stress/migration
and installer lanes passed. Windows MSVC subsequently passed; all 33 checks
completed successfully. FIELD arity/type #6987 and cursor allocation #6988 remain
separate unadmitted production work.

FSIZE first-argument Numeric/exact-integer admission completed in PR #6992.
Its 64-call native fixture, 81 direct calls and 151 PRG rows remain retained
with docs/22/docs/32 and completed README VR/DQ/DV/reproduction evidence.
Old dispatch failed 127 assertions (8.28s); five suites passed normally
(8.52s) and under ASan/UBSan/float-cast-overflow (38.95s), without diagnostics.
All four documentation/signoff/channel contracts passed (4.57s), with 140
valid fragments. All 11 required exact-head checks passed and Codex review was
clean with no conversations before merge. All completed optional checks
passed; Windows MSVC was still running at merge. Branches, worktree and builds
are cleaned as recorded above. Other type/arity #6990 and SET COMPATIBLE/file
semantics #6014 remain separate. No VM or persistent/system changes were used.

SELECT() Numeric/exact-integer selector conversion completed in PR #6995.
The retained 60-call native fixture, 164 direct calls and 126 PRG rows map to
RQ-CF-PRG-SELECT-SELECTOR-NUMERIC-001 in docs/22/docs/32 and completed README
VR/DQ/DV/reproduction evidence. Old dispatch failed 68 semantic assertions
(numeric 6.08s; four-suite baseline 35.01s); formatting-confounded output was
excluded. Normal suites passed 4/4 (35.55s); sanitizer suites passed 4/4
(137.19s), without diagnostics. Four contracts passed (3.91s), 141 fragments
validated. All 11 required checks passed, exact-head review was clean and no
conversations remained before merge; all completed optional checks passed,
with Windows MSVC still running. Query routing #6013, non-Numeric admission
#6994 and cursor allocation #6988 remain separate. FSIZE's optional Windows
MSVC subsequently passed; all 33 FSIZE checks completed successfully.

SQLGETPROP first-argument Numeric/exact-integer handle conversion shipped in PR #6998
under #5611/#6776 on `fix/sqlgetprop-handle-numeric-5611`, from main
`369d5f41bb162c909c497fc097c982957289d574`, in
`/home/rich/.codex/worktrees/sqlgetprop-handle-numeric-5611/Project-Copperfin`.
Only its llround/int site in prg_engine_expression.inl is selected. Other SQL
callers, property-value conversion, backend/connection/session behavior and
non-Numeric admission remain separate.
The complete 48-call installed VFP9 09.00.0000.7423 Wine fixture is retained in
`tests/fixtures/vfp9-sqlgetprop-handle-numeric-observation/`; two fresh
connection-free queries match all 50 lines. Fractions near zero truncate;
huge positive handles reject with 1466, whereas explicit VFP9 negative
low-32 aliases and huge/infinite zero remain observable through handle zero.
Nonzero absent-handle outputs do not independently distinguish every converted
index; exact int64/uint64 and NaN follow documented derived safety policy.
The checked helper is wired into only this expression dispatch. 134 direct
calls/119 final PRG rows use the existing synthetic in-process connection, not an
ODBC/network connection. Original dispatch failed exactly 43 conversion
assertions in both GCC (numeric 6.13s; full run 7.80s) and Clang sanitizers
(numeric 27.68s; full run 34.34s); all direct helper and preserved controls plus
three neighbors passed. No baseline sanitizer diagnostic was produced.
After this conversion change, normal suites pass 4/4 (7.63s; numeric 5.94s),
and sanitizer suites pass 4/4 (34.59s; numeric 27.33s) without diagnostics.
Native default/invalid-query and other-type admission gaps are filed separately
as #6996 against exact main 369d5f41, without an implementation-admission label.
Final diagnostic walkthrough found the shared Numeric formatter's unchecked
huge-finite llround (#6997). Four message rows were added after the original
115-row baseline; two finite-huge messages fail (5.86s) while infinite controls
pass. Only SQLGETPROP's rejection diagnostic now uses the existing safe
round-trip decimal formatter; the shared formatter/other callers are untouched.
Final normal verification with all 119 rows passes 4/4 (7.83s; numeric 6.11s).
Final sanitizer verification passes 4/4 (34.19s; numeric 27.45s), including
safe diagnostic regressions, without sanitizer diagnostics.
Changelog/live-channel/full safety workflow contracts pass 4/4 (360.90s),
final fragment checks pass 2/2 (0.45s), and all 142 fragments validate.
Normal/sanitizer builds are
`/home/rich/temp/copperfin-sqlgetprop-handle-5611-build` and
`/home/rich/temp/copperfin-sqlgetprop-handle-5611-sanitize`.
Completed README VR/DQ/DV, fixture identities and docs/22/docs/32 traceability
retain the conversion/query boundary and recovery limits. No VM was used.
PR #6998 merged at e3bdb6458 after all 11 required checks and clean exact-head
review, with no conversations. The slice worktree/branches are removed and
both builds are in recoverable trash; paths above are historical reproduction
locations. Unrelated files are preserved. The live channel was re-read empty.

SQLSETPROP first-argument handle conversion is now selected under #5611/#6776
on `fix/sqlsetprop-handle-numeric-5611`, from exact main
`e3bdb6458f6b1c5f801f530c29d06ee8fff6bf29`, in
`/home/rich/.codex/worktrees/sqlsetprop-handle-numeric-5611/Project-Copperfin`.
Only its expression-dispatch llround/int site is selected. Separate property-
value conversions, setter/default/backend results, connection/session state,
non-Numeric admission and all other SQL callers remain outside scope.
The complete connection-free native fixture is retained in
`tests/fixtures/vfp9-sqlsetprop-handle-numeric-observation/`; three fresh runs
match 52 lines (48 setter calls, VERSION, unchanged default before/after and
unchanged session 1). Its conversion boundary independently matches SQLGETPROP.
The new operation-specific helper shares the verified conversion only.
134 direct calls and 118 PRG rows are added; setter rows reset the synthetic
connection's Boolean property and verify returned status plus mutation state.
No ODBC/network connection or VM is used. Normal and sanitizer builds are
`/home/rich/temp/copperfin-sqlsetprop-handle-5611-build` and
`/home/rich/temp/copperfin-sqlsetprop-handle-5611-sanitize`.
Original dispatch fails exactly 47 new conversion/caught-message assertions
in GCC (numeric 6.49s; full run 8.27s) and Clang sanitizers (numeric 29.68s;
full run 36.72s); direct helpers, preserved controls, SQLGETPROP rows and all
three neighbors pass. No baseline sanitizer diagnostic was produced.
Only SQLSETPROP's first-argument dispatch is now checked, with safe Numeric
rejection diagnostics; property-value conversion and backend callbacks are
unchanged. GCC passes 4/4 (7.64s; numeric 5.93s); Clang ASan/UBSan/float-cast-
overflow passes 4/4 (34.64s; numeric 27.80s) without diagnostics. Runtime
tests ran serially across build directories. Changelog/signoff/live-channel
contracts pass 4/4 (5.65s); all 143 fragments validate.
The encountered default/invalid-setter and non-Numeric admission gap is
filed separately as #6999 against exact main e3bdb6458 without implementation
admission. The general shared formatter gap #6997 remains unchanged.
Completed README VR/DQ/DV, fixture identities, error-before-setter/property-
reset walkthrough and docs/22/docs/32 traceability retain the bounded scope.
SQLSETPROP shipped as PR #7000 at signed head cb920f30a8 with all 11 required
checks green, clean exact-head Codex review and zero review conversations.
Merged 2026-10-06 05:16:38 UTC as 4aef3ae3052a3d004da5dbe48bbbb8189a810cd1.
Main is synchronized; its worktree/local/remote branches are removed and both
builds are in recoverable trash. All completed optional checks passed; Windows
MSVC remained in progress. Unrelated files/worktrees are preserved and the
live channel was re-read empty.

SQLDISCONNECT first-argument handle conversion is now selected under #5611/#6776
on `fix/sqldisconnect-handle-numeric-5611`, from exact main
`4aef3ae3052a3d004da5dbe48bbbb8189a810cd1`, in
`/home/rich/.codex/worktrees/sqldisconnect-handle-numeric-5611/Project-Copperfin`.
Only its expression-dispatch llround/int site is selected; disconnect-all,
absent-handle/type result parity, backend/session lifecycle and other SQL
callers stay separate. No VM or external SQL connection is used.
The complete connection-free native fixture is retained under
`tests/fixtures/vfp9-sqldisconnect-handle-numeric-observation/`; fresh repeated
50-line output contains VERSION, 48 calls and unchanged session 1. It independently
matches the selected negative-only zero aliases observed for GET/SETPROP.
134 direct calls and 300 fresh-session PRG rows are added. Original dispatch
fails exactly 41 new conversion/caught-message assertions in GCC (numeric
6.40s; full run 8.22s) and Clang sanitizers (numeric 29.95s; full run 36.92s).
Direct helpers, preserved callback/coercion controls, existing GET/SETPROP rows
and all three neighbors pass; no baseline sanitizer diagnostic occurred.
Only SQLDISCONNECT first-argument dispatch is now checked with safe original
Numeric rejection text; disconnect callbacks and successful events are unchanged.
Final GCC passes 4/4 (9.22s; numeric 7.23s); Clang ASan/UBSan/float-cast-overflow
passes 4/4 (38.78s; numeric 31.45s) without diagnostics. Runtime tests ran
serially across build directories. Changelog/signoff/live-channel contracts
pass 4/4 (4.21s); all 144 fragments validate. Normal and sanitizer builds:
`/home/rich/temp/copperfin-sqldisconnect-handle-5611-build` and
`/home/rich/temp/copperfin-sqldisconnect-handle-5611-sanitize`.
Completed README VR/DQ/DV, fixture hashes, error-before-disconnect/cleanup
walkthrough and docs/22/docs/32 traceability retain the bounded scope.
Shipped as PR #7002 with all 11 required checks green, clean exact-head Codex
review and zero review conversations. Merged 2026-10-06 06:17:15 UTC as
667b6fd61936a65b5a5944aa048f92e6947bf0d1. The optional macOS installer job
passed after a same-head, job-only retry of GitHub artifact-upload ENOTFOUND;
no source change was needed. Other completed optional checks passed; Windows
MSVC remained in progress. Main is synchronized; the worktree/local/remote
branches are removed and both builds plus generated FXP are in recoverable
trash. Unrelated files/worktrees are preserved and the live channel was empty.
The encountered
disconnect-all/absent-handle/
type gap is #7001, filed against exact main without implementation admission.
The shared Numeric formatter gap #6997 remains unchanged. No owner input needed.

SET DATASESSION TO Numeric/exact-integer selector conversion is selected next
from exact main 667b6fd61936a65b5a5944aa048f92e6947bf0d1 on
`fix/set-datasession-numeric-5611`, in
`/home/rich/.codex/worktrees/set-datasession-numeric-5611/Project-Copperfin`.
Complete clean-room native source/output retains 61 commands/64 lines in
`tests/fixtures/vfp9-set-datasession-numeric-observation/`; two fresh final
runs match byte-for-byte. Numeric fractions truncate and both-sign low-32
aliases select live native sessions 1/2. Invalid selection raises 1540 while
preserving the active session. Other positive indices are derived checked
domain policy, not native existence evidence. The encountered existence/type
gap is #7003, filed against exact main without implementation admission.
140 direct calls and 108 fresh synthetic cases plus two resumable/four locale
cases cover bounds, state/settings retention, successful-event suppression,
catchability, mode reset and cleanup. Original dispatch fails exactly 251
selected assertions in both toolchains; helpers/controls/existing numeric
rows and three neighbors pass. Final results and README/matrix evidence are
summarized above. Builds are `/home/rich/temp/copperfin-set-datasession-5611-build`
and `/home/rich/temp/copperfin-set-datasession-5611-sanitize`; numeric tests ran
serially across build directories. The unchanged broad control-flow baseline
was interrupted after sustained execution and is not claimed passed; focused
resumable operands and relations/database/date-time neighbors are verified.
Shipped as PR #7005 at signed head d3a825a86 with all 11 required checks green,
clean exact-head review and no conversations. Merged 2026-10-06 07:16:08 UTC
as d5b1031df8d010c2534a8cece03a193de937d7d3. Completed optional checks passed;
Windows MSVC remained running. Main is synchronized, branches/worktree removed
and both builds/FXP are in recoverable trash. SET DECIMALS is selected next.

The Linux installed-GUI sub-slice remains explicitly deferred until the
managed Studio is shipped in the Linux package; source-tree Mono/Xvfb smoke is
not installed-product evidence. Re-enter that slice when packaging exposes the
managed Studio.

The owner-directed #6879 extended-table sequence remains retained and
uncancelled after the current owner-directed numeric-conversion work.

## Retained owner-directed workstream: extended tables and indexes

After the current numeric-conversion work, open owner-authored,
`agent-approved` design #6879 and its 23 direct sub-issues remain the next retained workstream.
Their live metadata and all owner-authored comments were revalidated on
2026-10-03. Work in this order:

1. #6880: probe installed VFP9 on unknown table-header version bytes and choose
   a distinct Copperfin marker that VFP rejects. Format work waits for this.
2. #6888: first ship the bounded #6877 exact-integer index-key fix, preserving
   adjacent keys above 2^53 in ascending and descending order.
3. Independent starting slices: #6893 `SET TEXTBEHAVIOR` (default `VFP9`) and
   #6895 `SET WARNINGS ON|OFF|ERROR` (name approved; default `ON`).
4. Bitwise group: #6872 shared conversion helper, then #6871, #6873, #6874,
   #6875, and #6876 (`BITAND64` family naming approved).
5. After #6880: #6881 format/registry; types #6882-#6885, #6896-#6902 and
   #6743; #6886 limits; #6887 native index; #6889 `INDEX ON`; #6890 DBC;
   #6891 import/export/COPY; #6892 documentation. #6902 is design-first.

Owner decisions for this workstream:

- VFP-compatible remains the default. Any non-VFP column automatically gives
  the table the distinct Copperfin header byte and a `SET WARNINGS`-controlled
  warning; mixed VFP/non-VFP columns are allowed.
- Copperfin-table limits are 1,024 columns, 65,500 fixed in-row bytes, 3,072
  index-key bytes, 32 columns per key, and 8,000 in-row text bytes. Widths are
  bytes. A declaration over its limit is an error and never auto-converts to
  memo storage. Memo/sidecar storage is an explicitly declared type. Odd widths
  for a two-byte-unit Unicode encoding follow `SET WARNINGS`: ON/OFF round down
  with shown/suppressed warning; ERROR raises a catchable error.
- `SET TEXTBEHAVIOR TO VFP9|COPPERFIN` governs every over-long text write:
  default VFP9 truncates (UTF-8 at a character boundary), COPPERFIN raises a
  catchable error naming the setting.
- Bitwise numeric conversion wraps to the low 32 bits only in VFP9 mode;
  default Copperfin mode rejects out-of-range values with error 11 (#6873).
- Copperfin tables retain fixed-width records. The native index is one
  combination/multi-tag file per table, similar to but intentionally distinct
  from CDX; Codex chooses and documents its layout and extension. Do not apply
  external-engine key limits.
- Nothing in the candidate type list is deferred. VFP9 claims require retained
  installed-VFP9 evidence. Use one slice per PR, fail-before/pass-after coverage
  in applicable modes, sanitizer evidence, docs/32 traceability, a valid dated
  changelog fragment, signed/DCO commits, green required checks and resolved
  review threads.

The unfinished #5611/#6776 numeric-conversion work below is active under the
latest owner-directed implementation-loop instruction. Continue bounded
slices until redirected or the checklist is complete, then return to #6879.

## Retained workstreams

Owner-directed workstream order before the #6879 assignment was:

1. **The numeric-conversion group.** Work authority is the open,
   owner-authored, `agent-approved` umbrella #5611; #6776 is the design
   reference and checklist (it was auto-closed when #6777 merged and has been
   reopened, see the traps below). Slices 1 and 2 are merged (#6777, #6778). The
   remaining work, by function, with the VFP9 behavior probed in
   `~/temp/vfp9-probes/numconv-6776/probe1.txt` (14 boundary values per
   expression; `probe1.out` is the UTF-16 original):
   - Completed bounded slices: arrays (#6030, PR #6865), `BITLSHIFT`/
     `BITRSHIFT` (#5765, PR #6867), `GOMONTH`/`EOMONTH` (#5608, PR #6869),
     `SPACE` (#6775), `ROUND` (PR #6878), and `CHR` (this continuation slice).
   - Completed most recently: `AT`/`STRTRAN` occurrence validation and
     `GETWORDNUM` boundary coverage (PR #6918).
   - Completed after those: `SUBSTR`/`SUBSTRC` huge-positive starts (PR #6919)
     and native/managed `DECLARE` narrowing (#6050, PR #6920).
   - Completed after those: `BINTOC`/`CTOBIN` (#5766, PR #6923).
   - Completed after those: array dimensions (#5594, PR #6926).
   - Completed after those: bounded `STR()` width/decimals validation (PR
     #6929), including the exact 237 width ceiling and explicit-mode negative
     low-32-bit quirk.
   - Completed after those: `DATE()` / `DATETIME()` numeric component
     validation (PR #6930).
   - Completed after those: `DOW()` optional first-day numeric validation (PR
     #6935).
   - Completed after those: `WEEK()` option order, bounds, setting fallback,
     and year-boundary compatibility (#6933, PR #6936).
   - Completed after those: `RGB()` component numeric validation (#6938, PR
     #6939).
   - Completed after those: Currency-first `MOD()` Numeric-divisor conversion
     (PR #6941).
   - Completed after those: `RAND()` seed numeric conversion (PR #6942).
   - Completed after those: `FILE()` / `DIRECTORY()` visibility-flag numeric
     conversion (PR #6947).
   - Completed after those: `FDATE()` optional type-flag numeric conversion
     (PR #6948).
   - Completed after those: `FOPEN()` optional numeric mode conversion
     (PR #6950).
   - Completed after those: `FCREATE()` optional numeric file-attribute
     conversion (PR #6952).
   - Completed after those: `FSEEK()` offset/origin numeric conversion
     (PR #6957).
   - Completed after those: shared file-handle numeric conversion in ten
     low-level callers (PR #6960).
   - Completed after those: `FCHSIZE()` size numeric conversion (PR #6962)
     and HEX extension numeric conversion (PR #6965).
   - Completed after those: primary SYS Numeric-selector conversion (PR #6967).
   - Completed after those: CPCURRENT Numeric/exact-integer selector
     conversion (PR #6969); query-domain/type parity #6968 remains separate.
   - Completed after those: shared RELATION/TARGET Numeric-index conversion
     (PR #6972); additive relation-order gap #6971 remains separate.
   - Completed after those: SET(TEXTMERGE) Numeric query-variant conversion
     (PR #6975); query-result gap #6973 and routing #6333 remain separate.
   - Completed after those: ALINES Numeric/exact-integer optional flag
     conversion (PR #6978).
   - Completed after those: ADIR Numeric/exact-integer optional display-flag
     conversion (PR #6980).
   - Completed after those: AFONT Numeric/exact-integer third-argument size
     conversion (PR #6983).
   - Completed after those: FIELD Numeric/exact-integer first-argument index
     conversion (PR #6989).
   - Completed after those: FSIZE Numeric/exact-integer first-argument
     admission (PR #6992).
   - Completed after those: SELECT() Numeric/exact-integer selector conversion
     (PR #6995).
   - Completed after those: SQLGETPROP Numeric/exact-integer first-argument
     handle conversion (PR #6998).
   - Completed after those: SQLSETPROP Numeric/exact-integer first-argument
     handle conversion (PR #7000).
   - Completed after those: SQLDISCONNECT Numeric/exact-integer first-argument
     handle conversion (PR #7002).
   - Completed after those: SET DATASESSION TO Numeric/exact-integer selector
     conversion (PR #7005); existence/type gap #7003 remains separate.
   - Completed after those: SET DECIMALS TO Numeric/exact-integer setting
     conversion (PR #7007); other-type admission #7006 remains separate.
   - Completed after those: SET FDOW TO Numeric/exact-integer setting
     conversion (PR #7009); other-type admission #7008 remains separate.
   - Completed after those: SET FWEEK TO Numeric/exact-integer setting
     conversion (PR #7011); other-type admission #7010 remains separate.
   - Completed after those: SET EPOCH query prerequisite (#7022/#7021) and
     Numeric/exact-integer conversion (PR #7023), then admitted #3698
     CENTURY/ROLLOVER (PR #7024). Both share parsing, independent display.
   - Completed after those: SQLCANCEL handle conversion (PR #7026); native
     connected-index and callback/type gaps #7025 remain separate.
   - Completed after those: SQLCOMMIT handle conversion (PR #7029); native
     connected-index and callback/type gaps #7028 remain separate.
   - Completed after those: SQLROLLBACK handle conversion (PR #7031); native
     connected-index and callback/type gaps #7030 remain separate.
   - Completed after those: SQLTABLES handle conversion (PR #7034); native
     connected-index and callback/type gaps #7032 remain separate.
   - Completed after those: SQLDATABASES handle conversion (PR #7035).
   - Completed after those: SQLPRIMARYKEYS handle conversion (PR #7037).
   - Completed after those: SQLFOREIGNKEYS handle conversion (PR #7039).
   - Completed after those: SQLCOLUMNS handle conversion (PR #7041); native
     callback/type/connected-index gap #7040 remains separate.
   - Completed after those: SQLROWCOUNT handle conversion (PR #7043);
     native absence is an intentional supported extension boundary.
   - Completed after those: SQLPREPARE first-argument Numeric/exact-integer
     conversion (PR #7046);
     native callback/type/connected-index gap #7045 remains separate.
   - Completed after those: SQLEXEC first-argument Numeric/exact-integer
     conversion (PR #7048); native callback/type/connected-index gap #7047
     remains separate.
   - Completed after those: CALLFN first-argument Numeric/exact-integer
     conversion (PR #7051); native absent/type gap #7050 remains separate.
   - Completed after those: ALEN optional dimension Numeric/exact-integer
     conversion (PR #7056); #6302 dimensionality/#7055 type gap remain separate.
   - Completed after those: TAG ordinal Numeric/exact-integer conversion
     (PR #7059); omitted routing #7057/type admission #7058 remain separate.
   - Completed after those: KEY ordinal Numeric/exact-integer conversion
     (PR #7062); omitted routing #7060/type admission #7061 remain separate.
   - Completed after those: DESCENDING ordinal Numeric/exact-integer conversion
     (PR #7065); type #7063/alias routing #7064 remain separate.
   - Completed after those: ISLEAPYEAR Numeric/exact-integer conversion
     (PR #7066); retained intentional extension with derived mathematical bounds.
   - Completed after those: JTOD/JTOT shared Julian-day conversion (PR #7068).
   - Completed after those: FV/PV excess-arity conversion boundary (PR #7069);
     #5878 core financial behavior remains unfinished.
   - Completed after those: BINDEVENT optional object-event flags (PR #7076);
     routine extension retained, type/Currency gap #7070 remains separate.
   - Completed after those: CURSORSETPROP BUFFERING second-argument numeric
     conversion (PR #7078); general buffering/type gap #7077 remain separate.
   - Completed after those: ERROR first-operand numeric conversion (PR #7080);
     native catalog/type recovery gap #7079 remains separate.
   - Completed after those: GO/GOTO explicit record-number conversion (PR#7082),
     SKIP count conversion (PR#7084), UNLOCK RECORD conversion (PR#7087)
     and SLEEP explicit-duration conversion (PR#7088). General record/type
     gap#7081, SKIP type#7083 and UNLOCK gaps#7085/#7086 remain separate.
   - Completed after those: DEFINE BAR Numeric-literal identifiers (PR#7089).
   - Completed after it: ON BAR ... ACTIVATE POPUP Numeric identifiers (PR#7090).
   - Completed after it: ON SELECTION BAR DO/static-action identifiers (PR#7091).
   - Completed after it: SET SKIP OF BAR identifiers (PR#7092).
   - Completed after it: SET MARK OF BAR identifiers (PR#7094).
   - Completed after it: MRKBAR Numeric/exact-integer identifiers (PR#7097).
   - Completed after it: SKPBAR Numeric/exact-integer identifiers (PR#7099).
   - Completed after it: PRMBAR Numeric/exact-integer identifiers (PR#7101).
   - Active: GETBAR second-argument Numeric/exact-integer positions.
     Other bar consumers#5868, popup order/count#6152 and expression admission
     #6225/#6227 remain unfinished.
   - Remaining after it: the other `llround(value_as_number(...))` sites in
     this and other modules (#5611 umbrella).
2. **Remaining cluster 15 allocation issues** (`docs/81` cluster 15): `FILETOSTR`
   #5740, `XMLTOCURSOR` #5767, `AGETFILEVERSION` #5686/#5759, project inventory
   #5703, PRG include depth #5728, runtime PRG load #5731, directory
   enumeration and array materialization #5741, `SPAWN` quota #5782, audit log
   appends #5787, `RESTORE FROM` #5790, list control #5764, import buffering
   #5642, CDX/DCX/MDX probing #5615. Follow the pattern in #6775: one shared
   validator or bound, error numbers from VFP9 evidence, a script-rows test.

### Rules decided by the owner (2026-10-02)

- Inconsistent or quirky VFP9 behavior is emulated only under a per-area switch.
  `SET COMPATIBLE` keeps its real FoxBASE+/dBASE meaning (#6223) and is not that
  switch. The first per-area switch is `SET NUMERICBEHAVIOR TO COPPERFIN|VFP9`
  (default `COPPERFIN`, reported by `SET('NUMERICBEHAVIOR')`; the spelling is
  provisional). Design and rationale: #6776.
- Default rule for every area: ask whether a *correct* legacy VFP program could
  plausibly depend on the behavior. If yes (documented contract, error numbers,
  limits such as the 16,777,184-byte Character string ceiling, `QUIT` with
  `NODEFAULT`), the default is VFP9 behavior. If only a buggy program could
  depend on it (for example `LEFT('abc', 1E20)` returning empty while
  `SUBSTR('abc', 1E20)` returns the last character), the default is Copperfin's
  own defined, consistent behavior and the VFP9 quirk sits behind the switch.
- A VFP9 expectation in an issue or an old test is not authoritative until it
  matches a fresh probe: three pinned tests were wrong this session
  (`stuff_zero_start`, `stuff_negative_start`, `SUBSTRC`/`STUFFC` zero and
  negative starts) and were corrected to the probed results.

### Probing VFP9

- Quick local probes: `~/bin/vfp9-probe /path/to/probe.prg` (Wine, about one
  second, no GUI). The PRG prints with `?`; do **not** end it with `QUIT`, the
  wrapper runs it inside its own harness. Use `--result <file>` for a PRG that
  writes its own file with `STRTOFILE(..., 0)`. Wine VFP9 was repaired on
  2026-10-02 (the missing piece was `VFP9ENU.DLL`).
- Cross-check on the Windows VM `copperfin-access365-win11`: automate
  `New-Object -ComObject VisualFoxPro.Application` from PowerShell and call
  `$v.Eval(...)`; examples in `~/temp/vfp9-probes/numconv-6776/probe*.ps1`.
  Launch long VM jobs with `Invoke-CimMethod Win32_Process Create` (a process
  started from an SSH session dies at logout); `C:\src\vm-job.ps1` builds named
  targets, checks free space and always cleans up. Keep the VM disk tidy: do not
  resize it unless a real space constraint appears.

### Merge and CI traps learned

- `main` requires 11 checks: DCO (1), the two Socket checks (2), the two
  executable-paths checks (2), and six lanes: the three generated-launcher
  checks (Windows, Ubuntu, macOS), the two DECLARE checks (Win32 and x64), and
  `windows-environment-paths`. `macOS Clang`, `Windows
  MSVC` and `windows-installer` are not required. After the fixes above,
  `macOS Clang` and `windows-installer` were green on #6778, and the full
  `Windows MSVC` native validation passed on #6774's own run (#6767); on #6778
  it was still running when that PR merged, so check the first `main` run.
- Count the required checks that have reported. "Nothing pending" can mean the
  jobs are not scheduled yet. `BLOCKED` with every required check green means an
  unresolved review thread (bot threads included). `mergeStateStatus` becoming
  `UNSTABLE` or `CLEAN` with zero unresolved threads is the reliable signal.
- Two jobs can share a name (`Windows MSVC`); `gh pr edit` is broken by the
  Projects-classic deprecation, so patch PR bodies through the REST API; rebase,
  do not merge, to catch a branch up; every commit needs a `Signed-off-by` for
  each `Co-Authored-By` identity; only the first issue in a comma-separated
  `Fixes` list auto-closes, so close the rest by hand; and GitHub's closing
  keywords ignore negation, so a PR note saying "does not close #N" still
  closes #N (this closed #6776 early): write "relates to #N" instead.

## Workspace preservation

Preserve unrelated untracked user files in the main checkout:

- `AGENTS.md`
- `Z:\\home\\rich\\temp\\vfp9-probes\\empty-object-205\\vfp.out`

The defect-fix takeover ended at 17:00 America/Detroit on 2026-09-22. The
first focused cycle of the owner's second-pass discovery prompt found and
reproduced issue #6492: `PREVIEW` inside a quoted REPORT/LABEL `TO FILE` output
pathname incorrectly enters preview mode and creates no output. A temporary
Linux regression failed for both commands; the test edit was removed, and no
production fix was committed. Per the owner's subsequent direction, the
discovery heartbeat now runs hourly; the older
`continue-copperfin-issue-loop` heartbeat is paused. Next discovery cycle:
inspect fixes since this pass or shift to an independent invariant if newly
filed issues are being resolved.
