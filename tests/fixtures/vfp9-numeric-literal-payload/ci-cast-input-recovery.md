# Hosted CAST input distinction and bounded test repair

RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001 / existing
RQ-CF-PRG-CAST-BYTE-NUMERIC-001 / RQ-CF-PRG-CAST-UINT64-NUMERIC-001.
PR7207 exact head b8c1325c7b466237ed58eeac5ac0c8d2daaa7f37; main ef3ea326f.
This is a CI input-assumption repair inside the selected literal prerequisite,
not new production conversion authority or native support for extension syntax.

## Actual failure and independent requirement basis

Hosted macOS run39094031076/job117576589900 attempt1 completes with eight failing
tests of629, all Byte/UInt64 public modes/sessions. Exactly68 assertion messages
name ONLY `-0.9999999999999999`: selected error/value/kind, plus Byte's successful
message checks. Actual error11 and sentinel12345 indicate safe rejection, not an
unsafe cast, wrong assignment or changed continuation/session behavior.
Full actual failed-step log, run/job identities and failed source head are retained
under host build/literal-payload-native/pr7207/, with lossless fixture archive.

Signed pre-code e2e59d020 native-only profile:50 ALREADY establishes this PRG
source spelling as BFF0000000000000, EXACT -1, in all eight native settings/
compiled-EVAL routes, repeated identically. Adjacent profile:49 positive source
is exact +1. It is not the neighboring C++ direct binary64 value
BFEFFFFFFFFFFFFF, independently asserted with std::bit_cast/static_assert.
The old CAST tests shared one expectation between those distinct producers.

The owner-derived extension requirements retained in
../vfp9-cast-byte-numeric/README.md and ../vfp9-cast-uint64-numeric/README.md
require finite truncation into the unsigned range: an actual Numeric strictly
between -1 and0 truncates0, but exact -1 rejects catchable11 before assignment.
Installed VFP does NOT support BYTE/UINT64 target syntax; do not claim a native
extension CAST result. Existing signed native source bits plus independent
binary64/integer mathematics, not Copperfin's changed output, determine both
expected routes. No new native probe/VM or production arithmetic change needed.

## Exact repair and assertion preservation

All direct Numeric rows/raw values/expected values remain unchanged (Byte140,
UInt6462). One row in each family now independently names its public bit-carrier
expression. CTOBIN of explicit big-endian BFEFFFFFFFFFFFFF bytes supplies the
original intended predecessor-of-minus-one input; BINTOC BRS verifies those bytes
inside an IIF guard, selecting exact -1 (must reject) if they differ. BS decode/
BRS encode follows shipped binary conversion policy and retained installed-VFP
bit-preserving roundtrip evidence in the ListItem fixture. No epsilon, source
canonicalization or production special case.

The OLD public neighbor-to-zero assertion remains. EACH alias additionally tests
the ORIGINAL decimal source spelling with error11/sentinel/kind/continuation/
session/locale controls. No spelling is dropped. Byte fresh public count126->128
per mode/session (two aliases); UInt64159->162 (three aliases). All other input/
expectation/timeout/control logic remains. The original1,193-row native public
literal oracle/helper/contract and ALL production code stay byte-identical to
b8c1325. Existing old CAST migration raw originals stay unchanged; this is a new
producer-input distinction, not a rerun or acceptance claim for old unsafe code.

Widen PR sanitizer build selection by exactly these two existing executables
and actual filter by their direct plus eight mode/session names. No existing
name is removed. New exact-name selector test passes4/4 and checks all ten names
and prefix/suffix exclusion. Registration is not actual hosted execution.

## Verification and gate state

Repair inputs are frozen before incremental compilation. Fresh local original
comparison may reuse the old b8c1325 source against unchanged current runtime;
the actual hosted failure above is the pre-repair negative evidence. Require
fixed two-family ten-name execution, unchanged literal public/helper and expanded
tooling checks; preserve exact direct/public counts and all source hashes.
Any compile/SAME result must be recorded as actual execution, not inferred.
Fresh exact-head review and hosted sanitizer/Windows/macOS gates are required
AFTER pushing the repair; the clean b8c1325 review does not cover new commits.

At the pre-compile checkpoint no local compiler/test was launched: audit
independently measured sustained full I/O pressure (~25% then37% avg10), and DAW
has no reservation but also defers heavy work. No foreign process/VM touched.
After pressure settled, BOTH audit and DAW cleared that narrow window.

## Actual GNU negative replay and fixed execution

Old b8c1325 CAST test source was compiled from Git through compiler stdin,
with existing shared test-support object and the nine retained GNU runtime
archives (not rebuilt or changed). Both compiles0, both direct shards0 with
140/62 counters. All eight public shards actually fail1, exactly68 assertions
on ONLY the same decimal spelling, Byte126/UInt64159 public counts unchanged.
This independently repeats the hosted macOS result; not an old unsafe-converter
baseline or a new runtime rebuild. Full statuses/stdout/stderr are retained.

Then normal CMake incremental build compiles ONLY the two repaired CAST TUs and
links them, one nice10 job; build0. CTest executes14 unique names,14/14 PASS,
14.71s, no failure/error/skip. Both direct counts stay140/62; EACH both-mode/
session public counter is128/162, all aliases/control paths complete. Original
literal public/helper also pass unchanged, all1,193x8=9,544 observations and458
helper inputs; selector/isolation pass. Pre-compile12-file hashes still verify0,
production/oracle diff empty, runtime archive SHA remains
c65ac027a74f755520fcc76eb9a395dff91fe13416b0d76b592e14dddaa03904.
Independent actual JUnit/status/count audits pass; `audit-cast-repair.py` supports
both host originals and lossless archived receipts. No values are manufactured
from product output. Actual local repair evidence in ci-cast-evidence/, host
build/literal-payload-native/cast-repair/. Both chats received window release;
no VM/native probe/external intervention. Full PSI ended0.11%/1.03%.

New-head actual hosted sanitizer/Windows/macOS and clean exact-head review still
required. The b8c1325 clean review and old-head CI passes do not qualify the fix.

DQ-DV-7206-CAST-INPUT-REPAIR-001, MEDIUM; parent HZ-data-corruption-01 /
HZ-runtime-crash-01 / HZ-doc-command-01. Delta is split independently proven raw
and source inputs, retain all old range assertions, add source-error assertions,
widen actual CI selection. Misuse is deleting failures, treating C++ parsing as
native source truth, changing production bounds or misclaiming native unsigned
CAST syntax. Development self-review walkthrough traces native profile:50 and
exact neighbor bytes to both derived range outcomes; selector4/4 actual checks
pass. Not independent-human/system/release assurance. Coherent rollback follows
the literal contract, preserving failure/native evidence and not weakening CAST
safety to accommodate a source-parser assumption. No parent/issue closure.
