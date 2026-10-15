# Numeric literal metadata: selected creation prerequisite

RQ-CF-PRG-NUMERIC-LITERAL-METADATA-001 / VR-5611-NUMERIC-LITERAL-METADATA-001 /
DQ-DV-5611-NUMERIC-LITERAL-METADATA-001 derives from the direct owner Numeric
compatibility instruction2026-10-15 and RQ-CF-PRG-NUMERIC-METADATA-001 under
admitted #5611/#6776. Baseline origin/main7d07d72034c9b0c63fda18a3af5ca97105d3d94c
contains merged PR7191's optional carrier, not migrated producers/consumers.

Next bounded work: recover installed VFP9 literal-creation descriptors for
existing decimal/scientific syntax, distinguishing direct temporary from assigned
value, then implement a precisely evidenced producer subset with before/after
regressions. The initial probe covers48 fixed literals under DECIMALS2/6/14,
FIXED OFF:144 observations per invocation. Integer-interface results are
controls, not permission to interpret their unused decimal/real fields as N.
Direct versus assigned width/scale/type/payload/TRANSFORM observations are kept
separate. No complete lexical/exponent/assignment propagation formula is assumed.

The native-only FLL is the previously built published-interface diagnostic;
its SDK-linked binary remains host-only in recoverable completed-build storage:
/home/rich/.local/share/Trash/files/listitem-column-numeric-5611-build/listitem-column-native/cause/cause-probe.fll,
SHA256 ebb34d4e4f12c378e7ffd521d66915a249b45995535e6d37513deb711d5264a3.
First-party source/interface provenance is retained in the prior ListItem
fixture. No decompilation, SDK implementation copy, VM or installation change.

No parser/helper/display/ListItem implementation or acceptance expected table
is introduced at selection. Recover rules and complete the requirement-to-code-
verification/impact map before wiring. Arithmetic/function propagation,
assignment normalization, fields, other literal syntaxes and formatting/list
consumers remain explicit follow-ons, not silently declared compatible or
deferred as optional. #7189 and the retained ListItem column read stay open.
Existing safety/mode policy and all prior behavior tests remain requirements.

## Actual first recovery pair

Both installed VFP9 invocations returned0 with empty stderr, full144-row
framing/stdout matching and byte-identical complete raw captures. Structural
and documented-type-field repeat audit0; SHA256 for each raw file:
add4cfe708506969e60b309089dfa61edd311eca94db90199b67774cc72518f2.
The audit retains unused interface slots verbatim but does not interpret them
as Numeric data. Full source/raw/status/audit and own FXP are retained here.

Observed at DECIMALS2: explicit decimal literal1.00 has direct width4/scale2
and assigned width13/scale2. Leading-zero literal001.2300 has direct width8/
scale4. The 19-fractional-digit literal0.0000000000000000001 still has width21
but scale18; its payload remains nonzero. Unary plus12.3400 has width7, unlike
its8-character source spelling; minus12.3400 has width8. Scientific1.00E2
crosses the interface as Integer, whereas1E-300 is Numeric with width40/scale18.
These observations rule out blindly using source length/fraction count for
every syntax or interpreting decimal metadata as rounding the payload.
They do NOT recover the complete literal/assignment/unary/exponent algorithm.
Next: complete the bounded producer contract and regression map, extending
native width/scale boundary evidence where needed, before changing parse_number.

DQ-DV selection-only procedural delta: begin native creation recovery after
carrier integration, not a completed Numeric rewrite. Misuse severity LOW for
this documentation step; walkthrough must distinguish observation from inferred
algorithm and native type-N fields from unused type-I slots. Keep raw failures
and source/status/hash records. Rollback removes this selection/probe only and
retains the uncancelled owner objective. Implementing-agent self-review is not
independent-human/system/release assurance; hazard severities/status unchanged.
