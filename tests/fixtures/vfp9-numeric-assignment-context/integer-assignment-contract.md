# Frozen bounded Integer scalar-assignment contract

RQ-CF-PRG-NUMERIC-INTEGER-ASSIGNMENT-001 / VR-5611-NUMERIC-INTEGER-ASSIGNMENT-001.
Parent RQ-CF-PRG-NUMERIC-ASSIGNMENT-CONTEXT-001, Numeric compatibility owner
decision and admitted #5611/#6776/#7189. Native source/raw/status/type-aware
audits precede this contract; contract precedes helper/tests/production wiring.

For a non-NULL ordinary Number with known Integer-storage metadata, an
integral finite payload in[-2147483647,2147483647], and a scalar memory-variable
`=` assignment, normalize descriptor width to10 except values<=-1000000000
use11. Preserve Number kind, exact payload/storage; decimal placeholder0 is
ignored, not the native unused decimal slot. Descriptor source lexical width
does not constrain assignment width: assignment may grow or shrink it.
LOCAL/PRIVATE/PUBLIC and copying an assigned value obey the same native rule,
independent of DECIMALS2/14 and FIXED OFF/ON.

Architecture: pure optional-metadata helper validates Integer identity,
finiteness/integrality/range BEFORE deriving width; no float-to-integer cast,
epsilon, formatting, allocation or PRG callbacks. Unknown/real descriptors and
unsupported payloads stay unchanged, not fabricated native identities.
Two scalar `=` dispatcher routes (plain identifier and m. namespace) call a
local storage lambda AFTER array/property routing. Only the existing presence
of assignment-expression text activates normalization; STORE/other internal
assign_variable users, FOR/function binding/return/fields/arrays/properties
are not implicitly migrated. Reference routing still uses existing assign_variable.
No expression re-evaluation: paused watch, macro/suspension and assignment RHS
evaluation must keep their prior effects/count/continuation semantics.

Independent public expected table is transcribed from the recorded native
Integer rows, not derived by the production helper. First run unchanged
runtime test and freeze failing output before hooks. Exercise both modes/
settings/scopes, copies, plain/m. paths and continuation; keep direct-expression,
real Numeric, unknown, NULL/type/exact64/Currency controls. Separate pure helper
coverage includes unsupported kinds/payloads, nonfinite/fraction/range boundaries
and maximal source descriptor width to rule out width arithmetic overflow.
Local focused regression and actual named hosted sanitizer execution are
required evidence, not CTest selection-only claims.

MEDIUM DQ-DV-5611-NUMERIC-INTEGER-ASSIGNMENT-001 procedural delta: old scalar
descriptor copy -> validated Integer-only normalization -> same storage routing.
Misuse: broad assign_variable hook changing unobserved loop/parameter/STORE
semantics, payload coercion, extrapolating this rule to Numeric, or claiming
complete Numeric/ListItem parity. Hazards HZ-data-corruption-01,
HZ-runtime-crash-01 and HZ-doc-command-01. Walkthrough must map frozen native
rows to ORIGINAL descriptor-only failures then unchanged expected tests passing,
plus controls/side effects. Development implementing-agent self-review and
automation are not independent-human/system/release qualification.
Rollback removes the dispatcher hooks/helper; prior typed literal/unary behavior
remains. Real Numeric normalization, Integer minimum/larger/longer/function/
field/arithmetic producers, STORE/transfers, display/conversion and retained
ListItem READ stay mandatory. #6776 modes/checks/limits remain unchanged.
