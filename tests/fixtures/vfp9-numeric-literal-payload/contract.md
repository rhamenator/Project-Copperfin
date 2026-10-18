# Bounded plain-decimal Numeric payload contract

Requirement `RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001`; direct owner
Numeric compatibility policy, admitted #5611/#6776/#7189 and derived #7206.
Baseline main`ef3ea326f78d640546363abe97bbe27b6208bf45`. This pre-code contract
recovers a bounded compatible algorithm from black-box native observations,
not proprietary implementation internals or Copperfin as its own requirement.
It is not a whole-Numeric, display, division or ListItem completion claim.

## Native basis and independent prediction test

The signed4ff7d0d8b checkpoint froze the structural hypothesis, held-out source,
213-case inventory and explicitly non-native prediction JSON BEFORE held-out
execution. Both unchanged held-out runs returned0/empty stderr, all1,704
values/no errors; raw byte-identical SHA
`afc73f60f03eb9bacd182bcf4c3710945f92adc9bee664422be479477f52ace0`.
Independent typed/source/BINTOC/repeat audit0 and compiled/EVAL differences0.
All1,704 frozen predictions matched (1,584 new-spelling observations,120
calibration observations), including lengths/scales through256 source bytes.
The earlier245-spelling/four-context profile adds7,840 values/run and rejects
eight simple plus initial108 structural alternatives. All raw/counterexample/
status/own FXP evidence remains retained. No source, prediction, auditor
expectation, timeout or operand was changed to make held-out predictions pass.

`emit-native-header.cjs` builds1,193 exact expression/metadata/bit rows ONLY
from the independently audited actual profiles. It imports no hypothesis,
prediction or Copperfin conversion helper. Public tests shall check EVERY row,
including all calibration overlaps, long/source-zero cases, signs and IIF.
The `plain_literal` flag identifies unsigned direct inputs for a future pure
helper test; it never excludes a public assertion. Native settings0/18 and both
FIXED modes/compiled-EVAL are retained; public verification uses both ordinary
NUMERICBEHAVIOR modes under the existing source-literal policy.

## Recovered bounded behavior

For the existing known plain-decimal producer domain: ASCII digits, one point,
at least one digit on each side, length3..256 inclusive. Preserve every source
digit and zero; do not canonicalize equal decimal rationals. The compatible
payload computation uses separately rounded binary64 steps:

1. Accumulate the whole digits by multiply-by10, then add digit.
2. Separately accumulate fractional digits by the same steps, while building a
   divisor by a rounded multiply-by10 for each fractional digit.
3. Round fraction/divisor, then round whole + that ratio.

Each multiply/add/divide boundary is significant: fused/reassociated operations
or a directly parsed `10^scale` are not interchangeable. Preserve explicit
binary64 rounding boundaries; validate with GNU/Clang/sanitizers and hosted
Windows/macOS. The native observations use recorded control word027F. This
contract does not recover arbitrary non-default host floating environments;
do not alter the process rounding mode to implement it.

The optional width/scale/storage attributes keep the prior native contract
(width min40/source length, decimals min18/fraction length, native real storage;
existing unary/materialization rules separately apply). Those display caps
must NOT truncate digits used in payload computation or round the payload to
the metadata scale. This corrects source conversion, not decimal arithmetic.

## Architecture and safety boundary

First freeze native/public ORIGINAL tests and registrations BEFORE support or
hooks. Proposed nonallocating pure helper validates the complete existing
bounded unsigned plain shape before floating work and returns unspecified for
other shapes. All accumulators/divisors are bounded by at most255 digits, far
inside binary64 finite range; still reject nonfinite results fail-closed. No
unchecked size/number conversion or allocation derived from metadata.

Only after actual ORIGINAL failure, wire the ordinary successful
`ExpressionParser::parse_number()` literal producer to the recovered payload
and existing metadata. Retain existing invariant parse success/error grammar,
NULL/kind, suppression, modes and source advancement. Do NOT change shared
`platform::try_parse_invariant_double` (persisted-data consumers), scientific/
integer/trailing-point/omitted-leading-zero grammar, Currency/exact64 paths,
assignment normalization, other operators, general macros, formatting or list
consumers. A corrected literal value can naturally affect downstream arithmetic;
run prior Numeric/control-flow/exact64/Currency regressions, not just the helper.

Beyond this established producer domain and non-default floating environments
remain explicit recovery obligations, not a canonical-double residual shortcut
or exemption for unusual values. Do not label this increment full Numeric parity.
Other producers/conversions and the retained non-whole/ListItem slices remain.

## Verification and rollback

`VR-7206-DECIMAL-LITERAL-PAYLOAD-001`: native-only header byte check; exact public
metadata/payload, malformed/NULL/Currency/scientific/integer/control-flow/state
controls; ORIGINAL failing before helper/hook; SAME frozen assertions passing;
pure helper unsupported/long/NUL/rounding-boundary vectors; GNU warning-clean
and Clang ASan/UBSan/float-cast-overflow/no-recovery helper evidence. Register
both named tests/isolation and both hosted sanitizer build/filter selection;
actual hosted execution is a later gate, not inferred from registration.

For the ORIGINAL public test, reuse retained unchanged-main static libraries
only after proving production source/include equality, compiler/ABI flags,
archive identities and original successful build receipt. Record exact manual
compile/link/run separately: it is not a new runtime rebuild or CTest run.
Later normal CMake build/SAME, broader tests, actual named hosted execution,
exact-head clean Claude/Codex review, strict checks and resolved conversations
are required before merge. No acceptance is granted by this document alone.

Rollback removes the single literal producer hookup/helper coherently, restores
the previous parser behavior, preserves native/raw/ORIGINAL evidence and reopens
the gap; it never rewrites native expected payloads. Re-enter division only after
this prerequisite integrates, with its unchanged1193-independent-literal evidence
and retained578-row division header/574 exact assertions refreshed on new main.

`DQ-DV-7206-DECIMAL-PAYLOAD-CONTRACT-001` (MEDIUM): procedural delta is native
recovery -> pre-code public ORIGINAL -> unused support -> single literal hook
-> SAME/broader/hosted review gates. Misuse is rounding to display decimals,
fusing operations, changing shared parsers or treating a helper as complete
Numeric/list parity. Development self-review walkthrough checks repeated-source
zeros,19+fraction digits,256-byte boundaries and rejected alternatives against
actual captures; automation cross-checks native fields/hashes. Not independent-
human/system/release assurance. HZ-data-corruption-01/HZ-runtime-crash-01/
HZ-doc-command-01, checked finite/conversion/allocation limits, #6776 per-area
switches and Currency/exact64 separation retain their existing parent status.
