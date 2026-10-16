# Frozen bounded real Numeric scalar-assignment contract

RQ-CF-PRG-NUMERIC-REAL-ASSIGNMENT-001 / VR-5611-NUMERIC-REAL-ASSIGNMENT-001.
Derived from recorded native assignment-context/real-scale recovery and owner
Numeric policy #5611/#6776/#7189, not existing Copperfin behavior. This contract
and native observations precede helper, public tests and production wiring.

Only ordinary non-NULL Number scalar memory-variable '=' assignment with
known real-storage descriptor, finite abs(payload)<10000000000 and decimals
1..18 is migrated. Set width to11+decimals+(payload<0 ?1:0). Preserve payload
bits, decimals and storage. In particular negative zero retains sign but has
no extra sign width. Source lexical width may grow or shrink. LOCAL/PRIVATE/
PUBLIC/copy rules are independent of DECIMALS2/14 and FIXED display settings.
Native rounded +/-1e10,D9 boundary controls are excluded, not rationalized.

Architecture: pure optional-descriptor helper checks identity, range and
scale BEFORE bounded unsigned arithmetic (maximum width30). No cast, epsilon,
rounding, formatting, allocation, callback or expression re-evaluation.
Unknown/Integer/Decimal0/out-of-range/nonfinite descriptors are unchanged.
Compose this with existing Integer normalization only in the two existing
plain/m. scalar '=' routes after array/property routing; do not change general
assign_variable, STORE, loops, parameter/return/field/property/array transfers.
Do not copy unrelated/already-normalized values just to normalize metadata.

Before hooks, author independent public expected constants from native rows
1..46, audit exact descriptors/bits, freeze inputs, run ORIGINAL runtime and
retain descriptor-only failures. Keep the existing18 Integer expected rows
unchanged. Its prior staged real-not-migrated control for assigned -0.0 must
intentionally evolve from W4D1 to native W12D1 BEFORE original-runtime testing;
this is the selected admitted follow-on, not weakening an Integer assertion.
Direct-expression -0.0 remains W4D1. Test both modes/settings/scopes/copies/
namespaces, NULL/unknown/Integer/type/exact64/Currency controls, paused watch
state and continuation. Pure helper covers scale/magnitude boundaries,
nonfinite values, negative zero, unknown/Integer and maximum old width.
Actual named hosted sanitizer execution is required; selection is not execution.

MEDIUM DQ-DV-5611-NUMERIC-REAL-ASSIGNMENT-001 delta: Integer-only scalar
normalization -> validated bounded real normalization -> same storage route.
Misuse: broad transfer hook, extrapolated magnitude formula, lost zero sign,
RHS replay or claiming Numeric/ListItem parity. Hazards HZ-data-corruption-01,
HZ-runtime-crash-01/HZ-doc-command-01. Walkthrough must link frozen native rows
to ORIGINAL descriptor failures then SAME frozen tests passing, with Integer
table unchanged and controls intact. Development implementing-agent self-review
and automation are not independent-human/system/release qualification.
Rollback removes only real helper/composition, restoring the previous staged
real control; keep Integer normalization/native evidence. #6776 modes/checked
conversions/allocation limits unchanged. Larger/scaleless real assignment,
other producers/transfers, division precision/display/exact conversion and
ListItem supplied-column READ remain mandatory re-entry work, not waived gaps.
