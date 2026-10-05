# Installed VFP9 FIELD Numeric-index observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine,
using the existing owner-installed /home/rich/bin/vfp9-probe runner unchanged.
field.prg is the complete clean-room source; field.out retains VERSION and
all 58 calls. Two fresh processes produced byte-identical output. The probe
creates only two temporary three-field cursors and closes both. No VM,
persistent table, installer, font/system change, overlay or binary inspection
was used. Generated FXP files are ignored.

The selected #5611/#6776 slice is only the existing FIELD() first-argument
Numeric/exact-integer llround/size_t conversion in prg_engine_expression.inl.
FSIZE(), SELECT(), optional FIELD flags, lookup/routing, arity and non-Numeric
type parity remain outside this slice. Production changes and regression
verification have not started; this is retained recovery evidence, not a
completion claim or a requirement inferred from current Copperfin code.

## Observed contract and limits

For the three-field cfprobe cursor, indices 1, 2 and 3 return ALPHA, BRAVO
and CHARLIE; zero, ordinary negatives and index 4 return empty Character.
Numeric fractions truncate toward zero: 0.5/0.9 remain empty, 1.5/1.9 select
ALPHA, 2.5/2.9 select BRAVO, and 3.9 selects CHARLIE.

Oversized conversion is operation-specific: positive 4294967297/8/9 return
empty rather than wrapping to fields 1/2/3, while negative -4294967295/4/3
return those fields. Negative -4294967294.9 returns BRAVO; positive
4294967298.9 remains empty. Unconditional signed-low-32 conversion therefore
does not reproduce FIELD. The huge/infinite observations return empty but do
not distinguish an exact converted zero from another invalid index. Native
VFP9 does not expose Copperfin's exact int64/uint64 kinds or the helper's NaN
input; their behavior must be documented as owner-policy derivation.

Three routing controls verify explicit alias and numeric work-area targets
without changing the selected CFPROBE alias. The five arity/type controls
retain their actual native results: FIELD() raises 1229; Character '2.5'
returns empty; Currency $2.5 selects BRAVO; Logical and NULL raise 11.
They do not authorize changing those separate existing Copperfin paths.

## Reproduction and identity

From the repository root, run the installed wrapper against the complete
source and compare its stdout with field.out:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-field-index-numeric-observation/field.prg
sha256sum tests/fixtures/vfp9-field-index-numeric-observation/field.prg tests/fixtures/vfp9-field-index-numeric-observation/field.out
```

Source SHA-256: b60e497a6c0b738770bcc5fc4501c0fd0b8dcda409a65e12bcd1e5b63614af44.
Output SHA-256: b98ad9190d3b2c7155d91fdc07101e63e5f4b6b90ae2da529b488347978d4f7a.
VR-5611-FIELD-NATIVE-001: both fresh runs completed, 59 output lines matched
byte-for-byte, all 58 calls retained CFPROBE as the selected alias.

DQ-5611-FIELD-RECOVERY-001 requires reproducible clean-room recovery without
claiming completion or changing system state. DV-5611-FIELD-RECOVERY-001:
the displayed command was executed twice using the unchanged wrapper; both
outputs and the exact source/output digests were checked. Procedural delta:
the new command runs only this bounded local cursor probe. Misuse severity is
medium (HZ-runtime-crash-01/HZ-data-corruption-01): do not use it as permission
for persistent-table, VM-maintenance or product-wide parity work. Review is
maintainer-authorized agent self-review plus these replay/hash checks, not
independent human verification. Walkthrough outcome: both temporary cursors
close and the wrapper exits successfully. Rollback: remove this recovery
fixture if its provenance is invalid, preserve the output for investigation,
and correct the handoff/release evidence before relying on it.
