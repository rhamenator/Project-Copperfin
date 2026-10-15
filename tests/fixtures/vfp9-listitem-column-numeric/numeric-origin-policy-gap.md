# Owner choice required: ListItem Numeric origin

RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR001+002 / DQ-DV001,
admitted parents5611/6776; focused tracking [#7189](https://github.com/rhamenator/Project-Copperfin/issues/7189)
at current main b6d1a7634e56435ad3326d92f13efa3f4fe241d3. This is a new
requirement/representation gap revealed inside the selected bounded slice,
not authority from the new issue, an independently discovered production defect,
or a Copperfin runtime reproduction. No helper, conversion expected table,
runtime assertion/configuration/build or caller change has been introduced.

## Repeatable counterexample

Fresh installedVFP9 7423 ListBox/ComboBox, unbound RST0, empty binding,
nine populated columns, existing stable IDs11/22 (display rows1/2). Every
capture retains the actual VARTYPE N, BINTOC BRS bytes, invocation phase,
return/error, before/after sampled control state and workarea/alias/session.

| Evaluated column operand | Same captured eight-byte value | Native ListItem(11,value) |
| --- | --- | --- |
|1-1E-14|3FEFFFFFFFFFFFA6|error31 Invalid subscript reference|
|1-90*2^(-53)|3FEFFFFFFFFFFFA6|Character id11c1|

All four class-ID contexts reproduce the difference, with complete sampled
control state and context equal. At the same captured value near2, decimal
origin reads cell1 and power-expression origin reads cell2; near9 reads cell8
versus9. The original near-integer observations must NOT be interpreted as
a general binary64 tolerance rule. The new720/612 adjacent-value pairs show
power-expression inputs up to128 representable steps below an integer still
reading that integer's cell. No threshold algorithm is inferred from that.

The documented double reconstruction
CTOBIN(BINTOC(EVALUATE(operand),'BRS'),'BS') preserves all120 actual Numeric
types/eight-byte values and complete sampled states/contexts, yet changes32
outcomes from expression-shape-probe.prg. The two near1 examples then both
raise31; near2 reads cell1 and near9 cell8. audit-numeric-origin.cjs independently
checks the full comparison and all four direct counterexamples; this is
observation evidence, not a chosen implementation expectation.

Shipped External Library Parameters help describes additional native Numeric
display-width and decimal-place fields alongside a double. Numeric/INT/FLOOR/
CEILING/CTOBIN help was read completely (HTML text extraction where raw output
was truncated); new exact hashes are in precision-allowed-sources.sha256.
These documents provide context, NOT proof of the internal cause of the
ListItem distinction. No tolerance, precision metadata or hidden-state cause
is asserted. No FLL injection, decompilation, proprietary implementation copy,
VM/installation change or native nonfinite oracle was used.

Copperfin's inspected PrgValue architecture has number kind plus double
number_value, not Numeric expression/display-precision provenance. That is
architecture evidence only, never a requirement source. A helper keyed solely
by this same kind/eight bytes cannot reproduce both observed outcomes. This is
a legacy compatibility choice, not merely absence of native syntax for an
extension; the earlier delegated extension-boundary judgment does not itself
choose between the two contracts below.

## Specific owner decision

1. Authorize an explicitly documented canonical-binary64 boundary for this
   bounded VFP9-mode column consumer, using the documented native double
   reconstruction for equivalent values. Retain native origin-dependent
   Numeric behavior as a named residual compatibility gap. This permits a
   small stateless helper/caller migration, but cannot claim full compatibility
   for all legacy expression origins. Default and exact64/nonfinite extensions
   still require their named owner/numeric/hazard parents and verification.
2. Require prerequisite investigation/representation work to preserve the
   native Numeric distinctions before this consumer migrates. This prioritizes
   broader legacy parity but expands beyond the bounded stateless-helper/
   caller slice. The necessary attributes and actual cause still need recovery;
   adding decimal metadata alone is NOT claimed sufficient.

Neither choice has been made. No KBX exception or agent-approved label was
invented. The pre-code requirement map, helper/assertion authorship, both
original suites, wiring/fixed verification and PR are blocked on this choice.
The active slice is uncancelled. Preserve both checkpoints7bb0112897 and
538e7c526 and the new raw/source/status/FXP evidence; no frozen assertion or
production input is changed. On explicit owner direction, resume at the
chosen requirement map, then retain all original-before-wire/fixed/sanitizer/
preservation/contracts/exact-head-review/check/merge gates.

## Evidence and unqualified attempt chronology

| Fresh complete pair | Calls per invocation | Complete raw SHA-256 |
| --- | --- | --- |
|precision|720|ae993d6c727fe6845f913cce26c3488e7a85e63f8de4e56192625f62d803c8ae|
|precision-threshold|612|8617c98bc7694f5c1578d4d3f3de8ae8ae3b40c9d1b2977260f7fe70f50db4c8|
|expression-shape|120|444682956f4ebdc40ab78bf5d748553b3544bb9c571e8cf8a61321b0039f83c1|
|binary-roundtrip-v2|120|b78198cd391d57cad64525be2e7cd0c7da59640678585cbf771f6a2c221b521f|

Each first/repeat pair is full byte-identical, actual native statuses0 with
empty captured native stderr; capture-only framing/BRS/state/context audits0.
Both own FXPs are retained. Different probes may have overlapped in the same
existing Wine prefix during tool dispatch; they use distinct source/result
paths, fresh controls and scoped runner groups. Repeat comparison and complete
capture checks are retained, not a claim of system/prefix isolation.

The initial binary-roundtrip-probe.prg used BRS for BOTH functions, which
actually byte-swapped the Numeric value: e.g.0.9 becomes BRS CDCCCCCCCCCCEC3F.
Its complete120-row matching pair/status0/capture audit0 is preserved under
captures/binary-roundtrip/, but it is NOT a bit-preserving roundtrip, numeric
conversion oracle or native rejection evidence for the original operands.
Version2 changes only the documented CTOBIN flags to BS and a distinct output
path/title; full actual BRS equality to expression-shape is independently
verified. No wrong-endian original is overwritten or relabeled a pass.

All10 selected and270 previous fixed runtime/test inputs remain unchanged.
The original136-entry recovery manifest remains an immutable historical
538e7c526 checkpoint, not refrozen to hide the README's current blocker pointer.
All its other135 inputs remain unchanged; recover the exact old README from
that signed commit. New phase manifest preserves the exact current evidence.
No broad runtime/development assurance or release completion is claimed.

## Separate retained CI dependency

#7188 remains the earlier Windows180.07s numeric_behavior timeout, not the
cause of this native representation gap. Unchanged all-three attempt2 of
run38636352844 at source head8968c625a84512394ca3604a9b77d7b7baca62b4 is
still pending. Existing blocking watch31376 remains in canonical
/home/rich/dev/Project-Copperfin/build/pr7187-late-native/retry-watch.log;
actual retry-watch.status appears on completion. Do not duplicate/model-poll,
mix attempts or claim attempt1's609/610 as full qualification. Preserve this
dependency when resuming after owner input. No VM/reset-completion assumption.
