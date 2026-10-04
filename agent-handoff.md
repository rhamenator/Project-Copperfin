# Agent Handoff

## Branching (changed 2026-10-01)

Develop on `main`: branch from `origin/main` and open pull requests with
`--base main`. `v1-development` is retired (see
`docs/v1-development-retirement-2026-10-01.md`); older text below that says a
slice "merged into `v1-development`" is historical.

## Last shipped slice

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
umbrella and reopened #6776 is the design/checklist reference. The active
bounded follow-up is `STR()` width/decimals validation on branch
`fix/str-bounds-5611` in
`~/.codex/worktrees/str-bounds-5611/Project-Copperfin`, based on `origin/main`
at `2c56331d5`. A fresh installed-VFP9 probe established the exact width
ceiling as 237 with error 1908 at 238, confirmed decimals 0 through 18 and
error 1908 for -1, and recovered the same negative low-32-bit conversion used
elsewhere in explicit VFP9 mode (-4294967295 becomes 1 and -4294967296 becomes
0). The probe source and complete output are retained under
`tests/fixtures/vfp9-str-argument-bounds-observation/`. Production validation,
focused script rows, catchable-error coverage, docs/22, docs/32, and the
changelog fragment are complete. Both focused targets pass normally and under
Clang AddressSanitizer, UndefinedBehaviorSanitizer, and float-cast-overflow
instrumentation. Signed implementation commit `051f061ad` is pushed and PR
#6929 targets `main`. Remaining: exact-head review, conversation resolution,
green exact-head checks, and merge.

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
   - Active: the bounded `STR()` width/decimals follow-up. Fresh installed-VFP9
     evidence fixes the exact width ceiling at 237 with error 1908 and confirms
     the explicit-mode negative low-32-bit quirk.
   - Remaining after it: the ~150 `llround(value_as_number(...))` sites in
     other modules (#5611 umbrella).
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
