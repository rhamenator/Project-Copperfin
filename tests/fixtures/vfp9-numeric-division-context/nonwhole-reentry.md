# Non-whole division re-entry after the literal prerequisite

`RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001`, admitted
#5611/#6776/#7189. PR7207 merged as
`4b6cc734fb1b70cf7cbcdd1f083fafa1317fa4f9` with its bounded native-backed
source-decimal producer. Its exact-head review/hosted acceptance is linked in
the requirements matrix; that does not establish non-whole division metadata,
callback timing, general Numeric or ListItem parity.

This branch merges that main revision without rewriting its signed native
freeze274a38635, actual ORIGINALafc488bb0 and deferral11bd096a6 chronology.
Resolve overlapping registration additions by retaining **both** the non-whole
test and every literal/helper/CAST sanitizer name. Both requirement/evidence
histories remain. No product code is authored by the merge resolution; the
merged runtime must be byte-identical to that main revision.

Keep `nonwhole-public-expected.h` (578 native rows,574 exact admitted assertions)
and `test_prg_engine_numeric_nonwhole_division.cpp` byte-identical to11bd096a6.
Refresh actual ORIGINAL against the integrated producer before new division
support, preserving previous logs separately. Payload/descriptor/evaluation and
boundary counters must be diagnosed independently; do not predeclare a result
from source inspection or a candidate model.

Then recover native fractional callback/continuation timing, retaining exact
operands and one-evaluation/state controls. Strengthen the older constant test's
three newly admitted unknown assertions **before** its next freeze. The existing
non-whole candidate is only a post-capture hypothesis; separately evaluated
child descriptors are not folded-child descriptors. Follow independent native
contract -> unused support -> ORIGINAL -> bounded hook -> SAME/broader/actual
hosted execution -> clean exact-head review/strict checks/resolved conversations.
No tolerance, spelling exception, quotient correction, suppressed callback
replay, source exclusion, or accepted residual-gap shortcut.

Audit and DAW both clear source-only preparation and report no heavy/VM
reservation. Elevated I/O pressure initially defers any rebuild/native/Wine
execution. A new narrow resource window plus fresh pressure sampling is required
first. No local VM, backup/upload action, reboot or date-based action is involved.
Four retained outside-scope rows, small divisors, wider producers/operators,
propagation/display/exact conversion and supplied-column ListItem reads remain
mandatory work. Parent issues and #7206 stay open.

DQ-DV-5611-NONWHOLE-RECOVERY-001 remains LOW: delta is evidence sequencing and
merge/registration preservation, not VM administration. Walkthrough compares
the signed source/header and merged main runtime, then requires actual refreshed
ORIGINAL before a new hook. Development self-review and static selector checks
are not independent-human/system/release qualification. Roll back this merge
checkpoint if incorrect, retaining original observations/results. Parent
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01, checked conversions,
allocation limits and #6776 mode policy remain unchanged.
