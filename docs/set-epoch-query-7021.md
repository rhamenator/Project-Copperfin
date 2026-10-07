# SET EPOCH raw-year query prerequisite (#7021)

## Requirement and authority

RQ-CF-PRG-SET-EPOCH-QUERY-001 derives from the repository owner's direct
2026-10-06 instruction to retain SET EPOCH alongside SET CENTURY, using EPOCH
as a simpler start-year interface with independent CENTURY ON/OFF display,
and to use judgment for extension-only design/verification. Implementation
authority is the admitted owner workstream #3698, related to #5611/#6776.
#7021 records the focused dependency found during the numeric slice; its
creation does not manufacture an admission label or widen that numeric PR.

SET('EPOCH') shall return the stored year text unchanged, not apply Boolean
normalization to year 1. Date/time consumers must therefore receive that same
window start. This is intentional Copperfin extension behavior in both Numeric
modes, not native VFP EPOCH syntax or query parity. Native VFP's CENTURY/ROLLOVER
interface remains required in its separate bounded slice.

## Architecture and delta

src/runtime/prg_engine.cpp's runtime-surface SET callback includes EPOCH in
the existing raw-setting return path beside FDOW/FWEEK/DECIMALS. No setter,
numeric conversion, default, localization string, CENTURY implementation,
calendar-admission algorithm, table or backend changes in this prerequisite.

The existing epoch_year consumer in
src/runtime/prg_engine_date_time_functions.cpp parses the callback result.
Previously stored year 1 was reported as ON and silently fell back to 1950.
The correction preserves its numeric window. Parent HZ-data-corruption-01
requires avoiding silently misdated values; HZ-runtime-crash-01 remains the
numeric slice's checked-conversion parent, not a new mitigation claim here.

## Focused verification

tests/test_prg_engine_date_time_functions.cpp reverse-links the requirement.
Sixteen added assertions cover year-1 raw readback, two-digit 01=>1 and 00=>100,
CTOD/DTOC and CTOT/TTOC, four- versus two-digit display, COPPERFIN and VFP9 modes,
fresh session default, restoration, omitted reset and its 2001 consumer.
Existing assertions retain ordinary 1975 windows and related settings.

Against exact main 0c7b209ebbfce42a02d935fffa9e8e8cd7b749d8, the initial fourteen
assertions fail ten checks: readback is ON, year 01 is 2001 rather than 1,
year 00 is 2000 rather than 100, and the corresponding sorted date/time,
four-digit display and restored-session year use the wrong window. The
two-mode supplement adds desired coverage after that baseline, not a claim
that it ran in the original fourteen-check baseline.

VR-7021-EPOCH-QUERY-001: final normal GCC date/time, Numeric behavior, relations
and database lifecycle pass 4/4 serially (9.94s); the supplemented sixteen
assertions all pass. Four catalog/changelog/channel contracts pass (0.54s),
149 fragments validate, channel integrity and git diff checks pass. Broad
hosted validation and exact-head review remain PR merge gates. This small
raw-query routing change adds no integer conversion; no local sanitizer, VM,
installed-product or independent-human review result is claimed for this PR.

## Documentation assurance

DQ-7021-EPOCH-QUERY-001 requires distinguishing intentional extension semantics
from native behavior and explaining the query/consumer dependency without
silently weakening the numeric slice's desired year-1 regressions.
Procedural delta: bypass only EPOCH Boolean normalization; restore the selected
numeric window to existing consumers. Misuse severity is medium because
incorrect window interpretation silently misdates values. This small read-only
query correction adds no argument-sized allocation or persistence boundary.

Walkthrough: in a fresh session, SET EPOCH TO 1 and query it; expect 1. YEAR of
CTOD('01/02/01') is 1 in both display modes and Numeric modes, while year 00
becomes 100. Switch to a fresh session and see default 1950; return and retain
1. Omitted SET EPOCH TO restores 1950 and year 01 becomes 2001. Restore 1975
and verify the existing ordinary-window regression results.

DV-7021-EPOCH-QUERY-001: maintainer-authorized agent self-review on 2026-10-06
compared the ten-failure exact-main baseline with the final sixteen desired
assertions and passing four-suite/contract results, inspected the EPOCH-only
raw-query delta, and walked through the ON/OFF/mode/session/reset results
above. This is completed development assurance, not independent-human review;
PR check and exact-head review gates remain separate.
Rollback reverts only this prerequisite through version control while retaining
regressions and the numeric checkpoint. Reversion reintroduces the silent
1950 fallback and must be communicated, not presented as safer behavior.
If guidance proves incorrect, notify maintainers with exact commit/test results,
correct requirement, tests and documentation together, then revalidate.

## Retained work and re-entry

The numeric conversion branch fix/set-epoch-numeric-5611 remains a separate
signed/DCO local checkpoint ce8df4493128814e10074e1566c030503e6e39a6.
Its desired tests are not weakened to accept ON. After this prerequisite
merges, incorporate origin/main there and rerun its normal/sanitizer suites and
contracts before its own PR. Complete real CENTURY/ROLLOVER semantics under
#3698 afterward; neither workstream is marked complete by this query fix.
