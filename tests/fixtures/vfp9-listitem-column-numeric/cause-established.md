# Native ListItem column coercion: decimal metadata is causal

2026-10-15, direct owner request: establish the cause and explain why the
near-one expressions concern ListItem; subsequently confirm column2.5.
This is diagnostic work only. It does not select a product-policy option,
resume the paused automation, implement a runtime fix or close the workstream.
RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR001+002 / DQ-DV001; #7189 remains
the earlier tracking reference, not new authority. The earlier documents'
"cause unproven" statements describe their retained pre-investigation phase.

## What the numbers index

The test call is ctrl.ListItem(existingItemID,columnOperand). The first
argument selects an existing stable item ID (11 or22, at display rows1 or2).
The disputed number is the SECOND argument: the one-based column selector,
not the element's stored value, caption, item ID or display-row position.
Fresh nine-column ListBox/ComboBox controls contain distinct strings such as
id11c1/id11c2, so a returned caption identifies the selected cell unambiguously.

Both near-one expressions evaluate to the same double just below1:
0.999999999999990007992778373591..., bytes3FEFFFFFFFFFFFA6. They deliberately
test the lower boundary: conversion to0 is invalid; conversion to1 selects
the first column. They are not examples of sensible application column
constants. Ordinary literal2.5 and computed5/2 select column2, not3; no
fractional column exists. The near-integer behavior is not ordinary rounding
to nearest:1.9 still selects1,2.9 selects2, and0.9 raises31 even when its
decimal-place descriptor is experimentally set to0.

## Established cause, not merely a correlation

VFP9 Numeric values include a decimal-place descriptor in addition to the
double. Shipped External Library Parameters help documents Numeric ev_length
as decimal places, ev_width as display width, and ev_real as double precision.
The diagnostic uses that published native FLL interface and installed SDK
support library, without decompiling VFP or copying its implementation into
Copperfin. It reads the actual native descriptor and x87 control word.

At the default SET DECIMALS2:

| Expression | Actual double bytes | Decimal places | Native read |
| --- | --- | --- | --- |
|1-1E-14|3FEFFFFFFFFFFFA6|14|error31 Invalid subscript reference|
|1-90*2^(-53)|3FEFFFFFFFFFFFA6|2|column1|

The decisive experiment returns copies of the SAME native Numeric descriptor,
changing only its decimal-place field before directly passing the temporary
to ListItem. With width held independently at25 or30, type N, identical
ev_real/BINTOC bytes and x87 control word027F:

- decimal-place count2 makes BOTH expressions select column1;
- decimal-place count14 makes BOTH expressions raise31;
- changing width alone does not change either original outcome;
- a field-preserving native clone does not change either original outcome.

Equivalent near2/near9 cases corroborate the same decimal-only intervention
at fixed widths. This is causal evidence for the role of the decimal-place
descriptor in the observed ListItem coercion, not an inference from a guessed
tolerance. The exact complete conversion formula for all values is NOT claimed
recovered; these experiments establish which extra input controls this case.
The positive fractional controls rule out describing it as blanket rounding
to the declared number of decimal places followed by integer conversion.

An independent SET DECIMALS experiment shows how the discrepancy arises:
setting0/2/6/14/18 BEFORE evaluating the power expression changes its attached
decimal-place count accordingly. The near-one result is admitted at0/2/6 and
rejected at14/18. Changing the global setting only AFTER evaluating the value
does not change its descriptor or outcome. The precision belongs to the value,
not merely the current global display setting when ListItem reads it.
The scientific-literal expression retains14 places under the default setting.

Thus the earlier eight-byte capture was accurate but incomplete as a native
Numeric identity: it omitted the decimal-place descriptor. The documented
double roundtrip preserved the double but changed Numeric metadata, explaining
why it changed the earlier result. This is not arbitrary dependence on source
spelling, a different list state, a display-width effect or a changed x87
rounding/precision control word in these controlled runs.

## Actual verification

cause-factorial.prg:11 operands x17 direct descriptor variants x2 classes x2
existing IDs =748 native reads per invocation. Two complete runs returned0,
empty native stderr and byte-identical full raw evidence. cause-settings.prg:
4 operands x10 evaluation/read-setting combinations x4 contexts =160 reads
per invocation, likewise two actual0/empty-stderr/full-byte-identical runs.

audit-cause.cjs actually returned0 and independently verifies complete framing,
unique rows, all original/argument/after bytes, documented metadata, unchanged
sampled list state/workarea/alias/session, exact intended interventions,
48 decimal-only outcome-flip pairs,88 width-only controls and44 clone controls.
It also verifies literal2.5 and computed5/2 read column2 across all tested
settings/classes/IDs. Native raw hashes:

- factorial:4b8f1fce9a8339eb755810d4ddb0d740086cd47a00e7b2a90387ee485d5f0a17
- settings:6e0beb9c2c05e7f8db39c068594dae6410c9976b3f89784440f78e4af9fcc656

All raw metadata includes the ev_long slot for transparency, but that slot is
unused for Numeric according to the shipped interface and is not interpreted
as meaningful Numeric data. No raw bytes are normalized away. The auditor
checks the documented Numeric fields, while retaining every original byte.
These are native diagnostics, not full Copperfin regression/acceptance,
system isolation, independent-human assurance or release qualification.

## Reproduction and provenance boundaries

The first-party cause-probe.c is a native-only32-bit FLL diagnostic using
published interface facts. cause-kernel32.def/cause-msvcrt.def provide ordinary
Windows API import names; Clang21 generates32-bit COFF and the installed Rust
LLD links against the owner's installed winapims.lib. No compiler, SDK, VM or
software installation was performed. No original VM/template was started.
The existing private Wine/Xvfb runner and its scoped25-second deadline are
unchanged. SDK header/library, runner, compiler/linker, resulting FLL and own
object hashes/paths are retained in cause-host-provenance.sha256.

The FLL and SDK-containing linked/object/import libraries remain HOST-ONLY at
build/listitem-column-native/cause/. They are not redistributed in this
fixture or linked into Copperfin. Retained repo evidence contains first-party
sources, own PRG/FXP files, raw observations, statuses, audits and build logs.
No proprietary SDK implementation/header/library is copied into the repo.

Attempt chronology is retained, not silently repaired:

- first link: GNU cross-linker lacked its32-bit script, actual1;
- second link: duplicate own/SDK DllMain, actual1;
- third link: missing32-bit Windows/CRT imports, actual1;
- final link: actual0 after documented import/entry setup; SDK debug-section
  magic warnings remain recorded, not a warning-free claim;
- initial intervention PRG used single-equals CASE comparisons, so VFP's
  SET EXACT OFF let dec1 match dec12/dec14, and intermediate assignment
  canonicalized display width. Its actual0 source/raw evidence is retained
  UNQUALIFIED for the intended high-decimal/width controls;
- the qualifying factorial uses exact == comparisons, direct temporary
  arguments, actual metadata verification and independent held-width pairs.

No product helper/assertion/CMake/isolation/caller has changed. The10 selected
and270 prior fixed production/test inputs remain unchanged. Implementation
still requires an explicit owner direction after this diagnostic discussion.
Broader Numeric precision propagation requirements have not been selected or
implemented, and no complete legacy-parity claim follows from this result.
