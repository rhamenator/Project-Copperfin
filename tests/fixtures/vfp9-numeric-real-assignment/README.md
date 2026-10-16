# Real Numeric assignment recovery

RQ-CF-PRG-NUMERIC-REAL-ASSIGNMENT-001 / VR-5611-NUMERIC-REAL-ASSIGNMENT-001,
child of assignment-context recovery and owner Numeric policy #5611/#6776/#7189.
Baseline origin/main ffbce9d20db6ee11cc375517049eb6527eb76a15. Pending division
precision/display/conversion/ListItem supplied-column READ are not cancelled.

The first-party probe records48 expressions x DECIMALS2/14 x FIXED OFF/ON x
7 contexts: direct EVALUATE/compiled, LOCAL EVALUATE/compiled assignment,
LOCAL copy, PRIVATE and PUBLIC compiled assignment. Payloads go directly to
the host-only published-interface observer, BINTOC and TRANSFORM, never through
a PRG observer parameter. Captures retain native tagged slots; the auditor
interprets only documented active fields (not Numeric's unused Integer slot).

Both actual runner exits0, empty stderr,1344 Numeric values/no errors per run;
structural/type-aware audit0, byte-identical repeat SHA256
7d2100a1b4a168946882acd6750a0588cd32e9b2f56b1e74c32b93331c0dc6eb.
No compiled/EVALUATE, assigned copy or scope differences. Descriptor values
are independent of the selected display settings. Source, own compiled FXP,
paired raw/status/stderr, provenance hashes and audit are retained here.
Replay: node audit-real-assignment.cjs captures.

Observed signed1 at every scale1..18: assigned positive W=11+D, negative
W=12+D. Positive and negative zero have W=11+D, but preserve their distinct
payload sign bits. Tiny +/-1e-18 and +/-9999999999.0 obey the same rule.
Leading-zero lexical widths shrink: long5.00 -> W13D2, negative -> W14D2.

Important counterexample: +/-9999999999.999999999 round to payload magnitude
10000000000, assigned W20/W21,D9, not evidence for a general magnitude formula.
These are retained EXCLUDED boundary controls. The bounded child contract
accepts only finite magnitude<1e10, known real-storage scale1..18. Decimal0,
larger magnitudes and other producers still require recovery; no epsilon,
digit-count assumption, tolerance or full-parity claim follows from this grid.

Private installed VFP9 09.00.0000.7423, unchanged private Wine runner; no VM
started/recreated and no backup-reset completion assumed. Host provenance
hashes identify runner/native exe/own published-interface FLL; no proprietary
implementation, decompilation, SDK or native product binary enters Copperfin.
The retained FXP is compiled from our own probe, not Microsoft source.

LOW DQ-DV-5611-NUMERIC-REAL-RECOVERY-001 procedural delta: missing real-scale
assignment evidence -> paired native recovery -> frozen bounded contract.
Misuse is extrapolating the rounded boundary or treating unused slots as
semantics. Automated audit and implementing-agent development self-review,
not independent-human/system/release qualification. Walkthrough: complete
1344 framing, exact repeat,48 typed observations and excluded boundary above.
Existing HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 apply.
Rollback removes this recovery only; owner compatibility objective remains.
