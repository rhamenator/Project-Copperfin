# Hazard Register

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
DQ/DV self-review and automated walkthrough complete; five contracts PASS332.88s;
signed
exact-head hosted/review/conversation gates remain integration gates. No
independent-human, persistence, callback atomicity, high-hazard, parent/KBX/hazard
closure or release qualification claim.

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
