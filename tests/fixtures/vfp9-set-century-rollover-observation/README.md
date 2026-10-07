# Installed VFP9 CENTURY/ROLLOVER observations (#3698)

This is retained requirements-recovery evidence for the admitted CENTURY
follow-on. Native observations and derived safety/extension policies below
are distinct sources; neither is invented from existing Copperfin code. Both
environments run installed Microsoft VFP9 `09.00.0000.7423`; no decompiled
source is used. The owner explicitly retains SET EPOCH as a simpler extension
and keeps CENTURY display policy independent of the parsing window.

## Observations retained on 2026-10-07 UTC

`century.prg` has 33 independently reset macro-command rows, seven numeric
query variants, and a private-session round trip. Two fresh Wine processes
match all 79 output lines. The TEXT command-list echo is retained verbatim,
not counted as a command result. `CLOCK|2026` binds time-dependent defaults.

- ON/OFF does not change the century/rollover window. Explicit 20/25 parses
  24 as 2124 and 25 as 2025.
- TO with only a century preserves the active rollover. Bare TO restores
  19/76 at this clock, without changing the OFF display setting.
- Century 1 and 99, rollover 0 and 99, and truncating Numeric/Currency
  fractions are accepted. Invalid ranges raise 11; Character, Logical and
  NULL operands raise 9. Missing/malformed forms have separate syntax errors.
- Native low-32 aliases are observed for both operands. Huge and infinite
  rollover values alias zero; the corresponding zero century is rejected.
- Native invalid rollover -1 or 100 can partially change the century to 20
  before error 11. These are observations, not authority to weaken #3698's
  explicit failure-atomic acceptance or Copperfin's checked default policy.
- SET('CENTURY',1/2) returns Numeric century/rollover; selectors outside 1..3
raise 11. Wine returns Numeric -1 for selector 3, which is not evidence for
  the Windows system-regional-calendar value.

`queries.prg` retains 26 evaluated query expressions and their typed results
in 53 lines (including TEXT source echo). Two fresh repeat outputs exactly
match. Numeric and Currency selectors truncate; both-sign low-32 selector
aliases are observed. Character selectors, including '1'/'2'/'3', Logical,
NULL and out-of-domain numeric selectors raise 11. A third argument raises
1230 even when the second is otherwise invalid. These type/arity results are
independent observations, not inferred from the setter's conversion model.

`windows-query.ps1` runs low-risk COM observations on the original
`copperfin-vfp9-win11` VM. Two fresh COM objects match all 12 lines of the
final output. It verifies the equivalent OFF/ON 20/25 parsing, bare TO reset,
and private-session isolation. The automation object's initial display is ON;
a fresh private Session is OFF. Both have century/rollover 19/76 at this clock.
Selector 3 returns **2049**, independently of the active rollover **76**.
Do not substitute a year-window component or Wine's unsupported -1 for this
Windows regional-calendar observation. Other regional-calendar configurations
were not modified or tested; no universal calendar-value claim is made.

## Reproduction and identities

From the dedicated slice checkout:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-century-rollover-observation/century.prg
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-century-rollover-observation/queries.prg
```

Read the local VM policy before Windows testing. Copy the checked-in PS1 to
a verified unused guest path and run it through the authorized SSH alias:

```sh
ssh copperfin-vfp9 powershell.exe -NoProfile -ExecutionPolicy Bypass -File C:/Users/claude/codex-century-3698-20261007-query.ps1
```

Bypass is process-local; no persistent execution-policy, registry, calendar,
installer or VM-disk change is performed. Each COM object is quit/released in
finally. The owned guest script is removed after testing. The VM was already
running on entry and is left running; this observation does not authorize
interrupting another user's work. Credentials are never copied or printed.
Output is retained with LF line endings; Windows SSH warning text is not VFP
output. Restore no calendar/system setting because none is changed.

| File | SHA-256 |
| --- | --- |
| century.prg | be47176dc287c6abe17fba1df4ada62e05924ff5d212336ba7affc5e89da758f |
| century.out | 2e8ca13d6c2f78274705aaaa2d2a66d0b09b03a8fa1ed21a969453288152b641 |
| windows-query.ps1 | c3756f1a30d0753f006b4344af47ebca05482815cf2f6836c7b4d65e4eb5e1be |
| windows-query.out | 1cc4193915916eccc69e737cf9709643daa262c8ddcc26eaa188d99b9051af29 |
| queries.prg | de2cd822adbcf6ab9638fab5479c422c0c1c187c0214450160a0172de0d07040 |
| queries.out | dd3bbd83b5229212ddbf5a5e089376e580460ee5aba4b82de3e57b190574e14a |

## Recovered and derived contract

RQ-CF-PRG-SET-CENTURY-WINDOW-001: `SET CENTURY ON/OFF` changes display only;
`SET CENTURY TO nCentury [ROLLOVER nYear]` selects the same parsing window
as `SET EPOCH TO nCentury*100+nYear`. Century-only TO preserves rollover.
Bare TO resets to the native current-local-year-minus-50 window, preserving
display. Initial sessions use that window; the default automation-style
session displays ON and fresh private sessions display OFF, without inheriting
either setting. Setting/restoring a session retains its selected window.
EPOCH's explicit omitted-TO reset remains **fixed 1950** by intentional
extension policy; its initial readback now reflects the native shared default.
Clock-century transitions and clamping an unrepresentable host year to 1..9999
are derived boundaries, not observations made by changing the original VM clock.

RQ-CF-PRG-SET-CENTURY-NUMERIC-001: century truncates into 1..99, rollover
into 0..99; Numeric/Currency operands are admitted, other types raise 9.
Default COPPERFIN rejects non-finite/out-of-range inputs with error 11.
Explicit VFP9 retains observed low-32 aliases/indefinite zero, then validates
the same domains. Exact signed/unsigned integers and NaN tests are derived
#6776/HZ-runtime-crash-01 policy, not native extended-type evidence. Currency
truncates its exact scaled integer without double rounding. **Both modes are
failure-atomic**, deliberately unlike native partial century mutation when
rollover is invalid, as #3698 explicitly requires. No clamping, state mutation
or success event is permitted on rejected command admission. Bare decimal
parser range failures also reject; expression errors propagate instead of
silently resetting. User-expression side effects are not a database transaction.
Malformed CENTURY forms raise 10; missing century before ROLLOVER raises 12;
missing rollover operand raises 67. Error messages are localized in four catalogs.

RQ-CF-PRG-SET-CENTURY-QUERY-001: `SET('CENTURY')` is Character ON/OFF;
Numeric variants 1/2 return century/rollover from the shared window, and 3
reads the Windows user-default calendar's two-digit-year maximum without
changing it. Numeric/Currency selector truncation and mode-specific aliases
precede the 1..3 domain check; invalid type/value raises 11 and extra arguments
raise 1230. On non-Windows or failed Windows observation, 3 returns Numeric -1.
This absence sentinel is a derived portable policy, not a universal VFP/locale
value. Only the retained Windows configuration is native-tested; do not infer
all-calendar parity. The Windows regression compares against the host API.

EPOCH year 1 legitimately exposes century 0/rollover 1 through readback; native
SET CENTURY TO 0 still rejects. High windows can expand a short year beyond
9999, so selecting a valid window does not promise every calendar value exists.
CTOD/CTOT, DTOC/TTOC, DTOT/TTOD and full-year inputs keep the existing calendar
admission; TTOS consistency is derived Copperfin-only behavior. No host setting,
table backend, installer or package boundary is changed.

## Verification and documentation assurance

Before production edits, the initial 218 fresh-session cases in
`test_prg_engine_century_rollover` fail **693 assertions** against main
22f1ac91ba1ba2e4d89dba22699822b5a3310f28. The existing five neighbor suites
pass (16.56s; six-suite baseline including expected failure totals 17.07s).
The final suite supplements those cases with four locale walkthroughs,
156 direct conversion/domain checks and eight derived clock boundaries.
Its 96 consumer cases use the independent arrays from the retained native
1950/1975/2025 window fixture, in both display and Numeric modes. Existing
date/time tests now independently read `YEAR(DATE())-50` for fresh defaults;
their explicit fixed-1950 consumer checks still execute EPOCH's omitted reset.
Error numbers use STR rather than TRANSFORM's unrelated display grouping.

Architecture/reverse links: helpers own checked conversion/default calculation;
dispatch admits the complete CENTURY command before updating EPOCH; session
state owns defaults; SET callback and runtime-surface dispatch implement typed
queries. Existing date/time consumers need no calendar algorithm change.
The new test's owned scratch root and scoped/restored locale are audited;
duplicate invocations remain serial. Exact local final results are recorded
below; hosted exact-head checks and
clean review remain independent merge gates, not implied by local tests.

DQ-3698-CENTURY-001 requires documenting shared parsing state separately from
display and host regional calendar, native facts separately from safety and
extension policy, and native versus EPOCH reset behavior. Procedural delta:
replace the old Boolean-only CENTURY TO path with real window syntax; change
fresh/default session years to recovered clock-based values and private display
to OFF; preserve explicit EPOCH reset. Misuse severity is **medium**: a wrong
window can silently misdate records (HZ-data-corruption-01), unchecked input
can escape runtime containment (HZ-runtime-crash-01), and ambiguous syntax or
reset guidance creates HZ-doc-command-01 exposure. This slice selects/queries
state and does not write tables or change the Windows regional calendar.

Walkthrough: choose EPOCH 2025 with CENTURY OFF; short 24 maps to 2124 and 25
to 2025. CENTURY ON changes only DTOC width. Choose CENTURY 19/75 and observe
EPOCH 1975. Invalid rollover 100 raises 11 and retains the full 1975/OFF state,
with no success event. Bare CENTURY TO restores the clock-based native window;
bare EPOCH TO restores 1950. A fresh private session has its own native window
and OFF display; returning restores 2025. Query 3 remains the host calendar
maximum, not either selected window. Four localized runtime walkthroughs verify
the catchable diagnostic and retained state. Source/output identities above
bind native evidence; original Windows VM/settings remain unchanged.

Rollback/field notification: revert this isolated slice using an ordinary
reviewed revert PR while retaining fixtures/regressions. Restoring the old
initial window or Boolean-only path is a behavior regression, not a safe fix.
Notify consumers through the dated fragment and corrected coverage/matrix
before relying on different historical dates. Never rewrite persisted dates
merely because a session's parsing policy changes. No certification,
independent-human review, installed Copperfin product or all-locale claim.

VR-3698-CENTURY-001: final GCC six-suite run passes 6/6 in **17.70s**
(CENTURY 0.80s). Clang 21 ASan/UBSan/float-cast-overflow with abort-on-error
and default leak detection passes the same six suites serially in **77.44s**
(CENTURY 3.12s), without diagnostics. GCC Debug uses -O0 -g1; sanitizer uses
the same with -fsanitize=address,undefined,float-cast-overflow and
-fno-sanitize-recover=all. Five locale-install/signoff/changelog/channel/
native-isolation contracts pass 5/5 in **4.93s**; 151 fragments validate and
channel integrity/diff checks pass. The baseline is not a sanitizer failure
claim. Hosted full-platform/native checks and exact-head clean review are
required separately before merging.

DV-3698-CENTURY-001: maintainer-authorized agent self-review on 2026-10-07
compares the retained native setter/query/Windows results, 693-failure
baseline, unchanged desired consumer arrays and final 222 fresh-session,
156 direct-domain/eight clock-boundary checks. It inspects the conversion-
before-mutation path, exact integer/Currency arithmetic, independent display/
regional calendar, four localized failure-atomic walkthroughs, five passing
contracts and clean normal/sanitizer results against the procedural delta.
This is completed local self-review plus automated evidence allowed for this
medium-severity documentation delta, **not independent-human review**.

Reproduce the focused verification after building all six named targets:

```sh
ctest --test-dir /home/rich/temp/copperfin-set-century-3698-build --output-on-failure -R '^test_prg_engine_(century_rollover|numeric_behavior|date_time_functions|relations|database_lifecycle|runtime_surface_functions)$' -j1
ctest --test-dir /home/rich/temp/copperfin-set-century-3698-sanitize --output-on-failure -R '^test_prg_engine_(century_rollover|numeric_behavior|date_time_functions|relations|database_lifecycle|runtime_surface_functions)$' -j1
```

Run the two builds' tests serially across builds: neighboring suites share
existing scratch roots. Remove only these owned build directories after the
PR merges/closes; do not remove an unrelated checkout or VM/template.

Microsoft-licensed VFP9 help corroborates command/session/display separation
and query meanings, but installed observations govern contradictory defaults:
[SET CENTURY](https://vfphelp.com/vfp9/html/d376a010-d478-439f-8aa7-517bcd617558.htm)
and [SET function](https://www.vfphelp.com/help/_5wn12ps82.htm).
