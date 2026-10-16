# Numeric memory-assignment recovery

RQ-CF-PRG-NUMERIC-ASSIGNMENT-CONTEXT-001 derives from owner Numeric policy,
the retained type-aware division recovery and admitted #5611/#6776/#7189.
Baseline origin/main c5583c4ce4fbf98f3b13bb69fd988b7dc4e3d954.
This recovery does not cancel the pending ListItem supplied-column READ.

Two independently authored24-expression grids, each672 observations/run:
DECIMALS2/14 x FIXED OFF/ON x direct EVALUATE/compiled expression, LOCAL
EVALUATE/compiled assignment, LOCAL copy, PRIVATE and PUBLIC compiled assignment.
No numeric payload is routed through a PRG observer parameter: CFNMETA/BINTOC/
TRANSFORM consume the expression or stored variable directly. This avoids
silently measuring a parameter-transfer descriptor instead of assignment.
All four runs actual0, empty stderr,672 values/no errors each. Structural/
type-aware audits0; fields/bits/display and raw bytes repeat exactly for each
pair. No direct compiled/EVALUATE, assignment compiled/EVALUATE, copy or scope
differences in either grid. Unused tagged-interface union slots are retained
raw but never interpreted as semantics.

First pair SHA256 f3d1d51c26ba35b22c084d42b79701e4dfaa2e0463fbd624ebd067549f5b80f5.
Boundary pair12cd5d70fa2f0fbc92cafd666c98a3e44f97dc6f3feb1b7d53ca3256fe532545.
Private installed VFP9 09.00.0000.7423 runner unchanged, no local VM started.
Runner SHA1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06,
native executable SHAcdc159a1e07e5208a0def267a94b8b3b007160641b442cb453aba8ecc4138560,
host-only own published-interface FLL SHAebb34d4e4f12c378e7ffd521d66915a249b45995535e6d37513deb711d5264a3.
No proprietary implementation/decompilation or SDK binary enters the product.
Run audit-assignment.cjs against captures/assignment or captures/boundary,
passing the corresponding retained probe source as the second argument.

Observed Integer assignments:0,-0,+/-1,+/-5,leading zeros -> W10;
-999999999 W10; -1000000000/-2147483646/-2147483647 W11;
positive1000000000/2147483647 and double-negated maximum -> W10.
Even a20-digit leading-zero direct Integer W20 normalizes to assigned W10;
its literal producer remains separately unmigrated. No settings/scope/copy
differences. Payload/storage are unchanged, Integer decimal slot ignored.

Numeric assignment is NOT the same rule:5.00 W4D2 -> W13D2,
-5.00 W5D2 -> W14D2; negative Numeric zero retains negative bits but assigned
W12D1, not W13D1. Wider magnitudes/scales in boundary observations are retained.
The original identical-bit counterexamples still differ in assigned decimals:
at DECIMALS2,1-1E-14 W25D14 versus1-90*2^(-53) W13D2; at DECIMALS14 both W25D14.
No general real Numeric assignment/arithmetic/conversion rule is accepted here.

Selected necessary child is the [pre-code Integer assignment contract](integer-assignment-contract.md).
LOW DQ-DV-5611-NUMERIC-ASSIGNMENT-CONTEXT-001 delta: missing assignment operand
descriptors -> independent paired native grids -> narrow contract/test BEFORE
wiring. Misuse: interpreting unused slots, assuming all assigned widths10,
or treating recovery as full parity. Walkthrough: both type-aware audits0,
672 records/pair/settings/scopes/copies, explicit Numeric counterexamples.
Implementing-agent development self-review plus automation, not independent
human/system/release evidence. Rollback removes this recovery fixture while
retaining owner objective and existing integrated behavior. Existing
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 apply; #6776 per-area
policy, checked conversions and allocation limits unchanged.
