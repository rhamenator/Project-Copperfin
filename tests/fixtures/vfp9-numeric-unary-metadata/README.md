# Numeric unary-sign metadata: recovery checkpoint

RQ-CF-PRG-NUMERIC-UNARY-METADATA-001 / VR-5611-NUMERIC-UNARY-METADATA-001 /
DQ-DV-5611-NUMERIC-UNARY-METADATA-001 derives from the owner's Numeric
compatibility decision and RQ-CF-PRG-NUMERIC-METADATA-001 under admitted
#5611/#6776. Recovery began at origin/main
ff41cfa99aa048014578211022f32979fe09b1ad; the branch is now fast-forwarded to
995b1a88f37665fdc4e93d5ff7a96f8d76a36ed7 after verification repair PR7196.
Both contain the bounded plain-decimal producer, not complete propagation.

Selected bounded prerequisite: recover installed-VFP9 direct unary-sign metadata
for plain-decimal literals and already-assigned Numeric values, including
parentheses, whitespace, repeated signs, signed zero, width/scale boundaries,
DECIMALS2/14 and FIXED OFF/ON. Integer/scientific/division expressions are
controls, not permission to migrate their producers without evidence. Direct
temporary metadata, assigned metadata, payload and default display remain
separate. Do not assume that every minus merely adds one to the operand width
or that metadata scale rounds the double. Recover the rule before architecture,
independent expected tests/original-fail and production wiring.

Use the existing unchanged installed VFP9 private Wine probe runner, not a VM.
The first-party published-interface diagnostic remains host-only at
/home/rich/.local/share/Trash/files/listitem-column-numeric-5611-build/listitem-column-native/cause/cause-probe.fll,
SHA256 ebb34d4e4f12c378e7ffd521d66915a249b45995535e6d37513deb711d5264a3.
No SDK binary/library/header or proprietary implementation enters the product.
Retain raw/status/source/hash provenance and repeat documented type-aware
fields; unused type-I real/decimal or type-N integer slots are not Numeric
semantics. Native syntax errors are recorded per case, not silently dropped
or copied as new Copperfin parser restrictions.

See [observations, limitations and next action](observed-requirements.md).
Each capture directory contains its exact source, own compiled FXP, paired raw
results/stdout, exit statuses, stderr and generated structural/type-aware audit.
Run `node audit-unary.cjs captures-240 captures-240/unary-probe.prg` from this
directory; use the matching source argument for captures-160 and captures-192.
The audit is not an implementation oracle or proof of a general unary formula.
Native CRLF raw bytes are retained. A default Git whitespace check reports
those carriage returns; `git -c core.whitespace=cr-at-eol diff --cached --check`
checks the preserved capture lines without normalizing their evidence bytes.

DQ-DV procedural delta: plain producer is integrated; retain repeated native
unary observations and distinguish unresolved rules before wiring, not full
Numeric/ListItem closure. Misuse severity LOW for this
recovery record: a walkthrough must distinguish observations from inferred
algorithms, lexical sign from runtime negation and documented fields from
unused slots. Preserve failed attempts. Rollback removes this recovery-only
selection/probe while preserving the owner objective and prior implementation.
Walkthrough/self-review: all three audit statuses are0 with matching frame/
case counts; the two failed invocations remain unqualified, and literal versus
variable signed-zero counterexamples prevent an invented universal rule.
Implementing-agent development self-review is not independent-human/system/
release assurance. HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01
severity/status, #6776 mode policy and all conversion/allocation limits stay
unchanged. Assignment/display/ListItem and broader Numeric parity remain
mandatory unfinished work; #7189 remains open.
