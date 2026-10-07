# SET EPOCH extension and installed VFP9 date-window oracle

## Governing policy and bounded slice

The repository owner directly instructed on 2026-10-06 to retain both SET EPOCH
and SET CENTURY, explaining that EPOCH was created to avoid CENTURY's quirky
century-plus-rollover interface. EPOCH names the start of the two-digit-year
window; CENTURY ON/OFF controls display independently. The owner explicitly
delegates judgment for missing extension design and verification. This
supersedes the old remove-or-extension-mode condition in admitted #3698.
Native VFP has no EPOCH command/query: this is an intentional Copperfin
extension in both COPPERFIN and VFP9 numeric modes, not a native-parity claim.

RQ-CF-PRG-SET-EPOCH-CONTRACT-001 records that disposition under admitted
#5611/#6776. The active slice implements EPOCH numeric conversion and verifies
display-independent consumers. VFP-compatible CENTURY TO ... ROLLOVER,
query variants, native defaults and session behavior remain the next bounded
admitted #3698 slice. Both interfaces are required; #3698 is not complete.

## Derived extension contract

RQ-CF-PRG-SET-EPOCH-NUMERIC-001 derives from the owner's retention/judgment
policy, RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01:

- Numeric operands truncate toward zero; exact signed/unsigned integers do not
  pass through double. Admit converted years 1..9999, reject non-finite or
  out-of-domain results without wrapping, clamping or changing the setting.
- These rules are identical in both numeric modes: VFP9 mode does not fabricate
  native integer aliases for a command VFP does not implement.
- Reject with localized catchable error 10 before mutation or a successful SET
  event. This error choice is extension policy, not native EPOCH observation.
- Omitted SET EPOCH TO resets to fixed 1950. A stable convenience default keeps
  existing ordinary Copperfin use; native CENTURY reset is separately recovered.
- The 1..9999 domain follows the four-digit calendar-year envelope, not a
  conversion of the century prefix. Truncation follows the project's Numeric
  convention. The upper windows do not promise that every resulting date is
  representable; calendar admission remains a separate parser boundary.
- Complete bare decimal literals retain Numeric rejection if finite parsing
  overflows/underflows. Quoted/malformed Character tokens and expression
  evaluation fallback retain the existing policy. Ordinary non-Numeric
  rounding/clamping/fallback is made defined, not claimed native type parity.

RQ-CF-PRG-SET-EPOCH-WINDOW-001 derives from the owner policy and the installed
VFP9 equivalent CENTURY/ROLLOVER oracle below. The century starts at
floor(EPOCH/100)*100; two-digit years below EPOCH's remainder use the following
century. Toggling display cannot alter that mapping. Explicit full years are
not shifted. CTOD/CTOT and their formatting/DTOT/TTOD consumers must agree;
Copperfin-only TTOS shares the window as a derived extension consistency rule.

## Installed VFP9 provenance

Recovered from installed Visual FoxPro 09.00.0000.7423 under Wine with the
unchanged owner-installed /home/rich/bin/vfp9-probe runner. Complete clean-room
sources and outputs are retained. No decompilation, table/backend/network,
VM, installer or system change was used. Generated FXP is ignored local scratch.

epoch.prg / epoch.out retain the original ten-line rejection/control evidence:
three macro EPOCH forms raise 36; the same EXECSCRIPT forms raise 10 in that
execution context; SET('EPOCH') raises 231. Actual CENTURY 19 ROLLOVER 50
reports 19/50 and CTOD years 2049/1950. The initial discarded direct-command
program failed compilation and supplied no numeric-domain matrix. These
observations remain provenance; they no longer block the owner-retained
extension or justify pretending EPOCH is native.

century-window.prg / century-window.out retain the equivalent native consumer
oracle for windows 1950, 1975 and 2025, each with CENTURY OFF and ON, and years
00/24/25/49/50/74/75/99. All 103 lines include native state, YEAR/CTOD and
YEAR/TTOD/CTOT, ordinary and sortable DTOC, sortable TTOC and DTOT. The same
window maps years identically in both display modes; only ordinary DTOC's
year width changes. TTOS is not presented as a native VFP function.

## Reproduction and verification

From the slice repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-epoch-numeric-observation/epoch.prg
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-epoch-numeric-observation/century-window.prg
sha256sum tests/fixtures/vfp9-set-epoch-numeric-observation/*.prg tests/fixtures/vfp9-set-epoch-numeric-observation/*.out
```

The original epoch source/output identities remain
a01e1cc70e6c9e23a8ac5b0d07e5d5e1c556c019c50b225f5f2c56b65b3d2c99 /
59e68b19e9b1e8c9690e869a378b046fe68d2ec6a9f23d0ad60a10063500e759.

Implementation reverse links: checked_set_epoch_argument in
src/runtime/prg_engine_helpers.h/.cpp; EPOCH-only dispatch in
src/runtime/prg_engine_dispatch.inl; existing epoch_year /
expand_two_digit_year_for_set in src/runtime/prg_engine_date_time_functions.cpp;
four resources/locales catalogs. Tests reverse-link the Numeric and WINDOW
requirements in test_set_epoch_numeric_boundaries and
test_set_epoch_display_independent_native_window in
tests/test_prg_engine_numeric_behavior.cpp. Existing date/time regressions
retain fresh-session default, session isolation/restoration and related SETs.

VR-5611-SET-EPOCH-WINDOW-001: two fresh final native runs match all 103 lines.
Window source SHA-256: 65e3968a5647ba88f3b3fec8d99c15f0ecce43755517a94bed00433bdf7d3d25.
Window output SHA-256: 6e4c632bf6f64caafc1083824e6a53213737c3a5664bfae3d6969bc6060ac174.
VR-5611-SET-EPOCH-NUMERIC-001: original dispatch fails 210 assertions; initial
checked GCC run passes four of five suites but reports 12 year-1 readback
failures. Separate current-main issue #7021 records the pre-existing raw-query
normalization of EPOCH 1 to ON, which also makes its parser consumer fall back
to 1950. Do not bless ON as the extension requirement or claim this slice is
complete. The obsolete, now-uncalled unchecked integer-setting lambda was
removed after that run; the re-entry validation must rebuild.

Re-entry condition: merge the separate bounded #3698-derived #7021 raw-query
prerequisite, incorporate origin/main here, then rerun the unchanged desired
regressions plus normal/sanitizer and contracts. Verification is in progress;
this checkpoint is not a passing sanitizer, exact-head review or merge claim.

## Documentation assurance and residual scope

DQ-5611-SET-EPOCH-CONTRACT-001 requires retaining native rejection provenance
without misclassifying an owner-requested extension as absent work.
DQ-5611-SET-EPOCH-NUMERIC-001 requires distinguishing delegated extension
choices from native observations, explaining failure atomicity and independent
display, and retaining the unfinished CENTURY/ROLLOVER slice.

DV-5611-SET-EPOCH-CONTRACT-001: maintainer-authorized agent self-review on
2026-10-06 compared the two complete native window outputs and hashes, retained
the native rejection provenance, checked the desired regression failures
against the scoped query callback and recorded #7021 against exact main.
Numeric completion remains gated on its separate prerequisite and revalidation.

Procedural delta: remove the owner-policy pause after the direct instruction;
replace unchecked round/narrow/clamp of Numeric operands with a checked
extension contract; verify consumers against equivalent native windows.
Misuse severity is medium: silently changing year windows can misdate data.
HZ-runtime-crash-01 and HZ-data-corruption-01 link checked conversion, localized
catchable failure and preserved setting/display. No argument-sized allocation
or new backend is introduced. This assurance is maintainer-authorized agent
self-review, not independent-human review or certification.

Walkthrough: set EPOCH 1975 while CENTURY OFF; verify 74 becomes 2074 and 75
becomes 1975. Switch CENTURY ON and verify only ordinary DTOC's width changes.
Attempt huge/invalid Numeric EPOCH inside TRY/CATCH; verify error 10, the old
window and no success event. Reset with omitted TO and verify 1950.
Rollback through version control restores only this bounded slice; retain
regressions and provenance. Reverting checked conversion reintroduces the
hazard. Notify maintainers with exact head/fixture identities if policy or
evidence changes, correct requirements/tests/catalogs together and rerun
native plus normal/sanitizer checks before merging.
