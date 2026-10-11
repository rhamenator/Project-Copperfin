# List Numeric display-row read requirements recovery

Pre-code map recorded 2026-10-11T16:21Z BEFORE helper/assertions/configuration
or production migration. Admitted OPEN owner-authored agent-approved #5611/#6776;
base origin/main f77021174fe86210e8c71bf0c74b3413c7b15e47. Bounded slice:
native ComboBox/ListBox ONE-ARGUMENT List Numeric/exact display-row READ,
implicit column1. Explicit columns/multiargument forms, writes, row/ID
creation/removal/lifecycle, types/NULL/Currency/arity, other consumers, textual
GETPEM, generic reflection/parser and RowSource/callback atomicity excluded.

## Independent allowed evidence

Fresh installed Visual FoxPro 09.00.0000.7423 in existing private Wine prefix
/home/rich/copperfin-wine-vfp9, not a VM. Unchanged /home/rich/bin/vfp9-probe
SHA1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06;
full source, snapshot, unedited runner logs and matching native pair retained.
Both native-first.out and native-repeat.out have302lines/300calls, SHA
87b64de6a995fd6b427afd790df06c9cb9dbf3e3967cec05f1b675fdad50124a.
Fresh object per expression, both controls, sparse IDs11/22/33 and dense1/2/3,
three rows, selectedrow2, payloads101/202/303, bitsfalse/true/false and actual
item identities remain unchanged on every successful or rejected read.
No credentials, decompilation, local VM or backup-reset completion assumption.

Independently observed List fractions truncate: 1.9=>Character first,
2.9=>second,3.9=>third IN BOTH identity shapes. Sparse22.9=>empty Character,
not second: List addresses display positions, not item IDs. Zero, fractions
(-1,1) and absent nonnegative positions return empty Character. Ordinary
converted negative values -1/-1.9 error31. Positive raw values aboveINT32_MAX,
including2147483647.1/.5/.9,4294967297,1e300,+Infinity, error31.
Negative signed-low32 aliases independently observed in BOTH shapes:
-4294967295/-.9=>first,-4294967294.5=>second,-4294967293=>third.
-4294967285/-4294967274.9/-4294967263=>empty, not sparse ID lookup.
Converted negative lowbits -2147483648/-.9,-4294967297 reject31;
-2147483649/-.9,-4294967296/-.5,negative huge/-Infinity=>empty Character.
Absent outputs alone do not recover every intermediate integer; exact64 and
adjacent-boundary expectations below are explicit parent/arithmetic derivations.
LogicalTRUE/NULL/Character'2.9'/Currency$2.9 error31 natively; type admission is
not migrated. Existing unsupported coercions are preservation controls only.
The complete original GNU preservation controls independently disagree with
native Logical/NULL admission. Focused [#7153](https://github.com/rhamenator/Project-Copperfin/issues/7153)
records this against exact current mainf77021174; separate/unadmitted, no type
migration or label change. Full report/raw/source retained alongside this map.

Shipped Microsoft VFP9SP2 List Property help completely read:
/home/rich/temp/vfp9-help-len/html/a1d6a6ef-aca6-4624-9be9-012697f29b2c.htm
SHAc256b772afdf5e6299679028fc9cd84781155d0e60805399e2486dea932db1e4.
It defines a Character array property for both controls, nRow in display order,
omitted nCol firstcolumn, and enumeration1..ListCount. ListItem is a distinct
item-ID property. No sibling code, tests or requirement used as semantic oracle.

## Pre-code requirement and architecture

RQ-CF-PRG-LIST-INDEX-NUMERIC-001 derives from #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and active HZ-runtime-crash-01/
HZ-data-corruption-01. COPPERFIN truncates finite Numeric then checks converted
0..INT32_MAX; rejected values raise localized catchable31. Deliberate default
2147483647.9=>absent empty is parent-derived checked-domain policy, unlike
native raw ceiling. Default negative aliases/nonfinite reject31. Explicit VFP9
first rejects positiveRAW aboveINT32_MAX, then defined signed-low32/nonnegative
admission; negative huge/-Infinity=>0. Accepted displayrow1..ListCount returns
first-column Character; zero/missing returns empty. NaN rejects both by parent
containment, not a native NaN syntax claim. Exact signed/unsigned64 comparisons
and low32 projection without a Double roundtrip are parent-derived extension
boundaries: INT64_MIN+2 default31/VFP9 second in BOTH identity shapes;
positive exact aliases reject both. Nextafter/subnormal literals derive from
independent arithmetic, not helper outputs or old rounding.

Distinct checked_list_index_numeric_argument stays UNUSED through BOTH complete
original GNU and Clang sanitizer suites. Only then wire a narrow
read_native_list_control_cell_numeric adapter into the one-argument List arm
and raw-property route BEFORE legacy List cell resolution. Existing expression
routes already forward one raw PrgValue for ordinary/postfix parentheses and
brackets; no parser/signature widening. Explicit columns, textual GETPEM and
unsupported types retain old paths. Keep visibility/_Access/delegate and
current-session policy ordering. Materialization precedes read conversion;
stable RowSourceType0 only, not arbitrary callbacks or whole-call atomicity.
Bounded materialized-row lookup, no input-sized allocation or iteration.

VR-5611-LIST-INDEX-NUMERIC-001: freeze independently enumerated both-mode direct
Numeric/exact64/NaN/Infinity/nextafter/subnormal and unsupported-helper guards.
VR-5611-LIST-INDEX-NUMERIC-002: both classes/modes/two sessions/sparse+dense,
four one-argument routes, Character kind/empty/error/locales/sentinel/
continuation/cursor/mode/reset and stable rows/count/data/bits/IDs/ListIndex/
ListItemID. Retain all39 completed numeric subjects and six unchanged list-
control preservation groups. Both full originals and raw counts/status/hashes
BEFORE migration; any frozen repair requires explicit refreeze and BOTH full
originals repeated. Identical fixed suites, sanitizers, preservation groups,
documentation contracts, signed exact-head CI/review are separate later gates.

## Documentation assurance before code

DQ/DV-5611-LIST-INDEX-NUMERIC-001 links runtime/data/HZ-doc-command-01 without
changing registered severity/status. Bounded documentation misuse medium:
misaddressed Character data can misdirect downstream actions; READ conversion
does not persist data or qualify high-hazard applications.
Procedural delta/walkthrough: sparse2.9=>second (not rounded third or missing
ID),22.9=>empty (not second),-1=>31,zero=>empty; positive4294967297 both31,
negative-4294967294.5 default31/VFP9 second; positiveInfinity both31,
negativeInfinity default31/VFP9 empty; directNaN both31. Walk all four routes,
both sessions/modes/catalogs and preserved state. Explicitly review deliberate
default-upper-fraction extension and lossless exact64. Retain delegated
development maintainer self-review and automated contracts AFTER verification,
not independent-human or release/high-hazard qualification. Rollback only
bounded adapter/helper and rerun frozen neighbors; notify integrators through
changelog before re-release. No parent/KBX/family/hazard closure.

Reproduce only after inspecting probe.prg's exact task-owned output path;
use retained unchanged runner with --result for that path. Preserve full raw
source and logs. Do not change assertions to make an original pass.

## Phase checkpoint

[Full phase audit](baseline-audit.md) retains the five-input16:23:42Z freeze,
both complete44-subject originals and unedited source/output/configure/compile
logs. GNU40/44/594.32s and Clang40/44/1107.45s have identical3552 selected
failed assertions, only four new public subjects failing; all39 unchanged
subjects and248direct/four944public counters complete. No sanitizer report,
leaks disabled; no original result is acceptance. Both full originals were
reviewed and retained before17:17Z bounded method/raw-property migration.
All five frozen inputs and eleven neighbors remain unchanged, no refreeze.
Identical full fixed GNU44/44/594.91s and Clang sanitizers44/44/1068.61s PASS,
all counters complete and zero failed assertions/sanitizer reports; each
compiler's six unchanged preservation groups PASS. Five documentation contracts
5/5/347.85s PASS. Boundedmedium delegated development DQ/DV walkthrough and
self-review complete in the audit, not independent-human qualification. Frozen
inputs unchanged; signed exact-head CI/external review gates remain pending.
No whole-call, family, parent, hazard or release completion is claimed.
