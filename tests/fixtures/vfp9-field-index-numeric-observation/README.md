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
type parity remain outside this slice. RQ-CF-PRG-FIELD-INDEX-NUMERIC-001 maps
the recovered Numeric behavior and explicitly derived safety boundaries to
the checked helper, FIELD dispatch and independent direct/public PRG tests.
The separate arity/type gap is #6987; cursor allocation is #6988.

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

## Implemented conversion and scope

Default COPPERFIN truncates Numeric values toward zero behind finite signed
int64 admission, preserves exact int64/uint64 without double rounding, clamps
admitted negatives to the existing empty-field index zero, and checks the
positive destination size_t bound before casting. Non-finite, out-of-int64
or out-of-size_t values raise localized catchable error 11 before lookup.
The default does not introduce an int32 ceiling: representable positive
indices above that ceiling still perform the existing empty lookup.

Explicit VFP9 applies the shared signed-low-32 model to Numeric/exact values
except that original positive values above INT32_MAX produce an empty lookup.
Negative aliases can therefore expose fields 1/2/3 while positive oversized
values cannot. Zero used for that positive empty lookup is an implementation
sentinel, not an independently recovered exact native converted index.
Huge/infinite native outputs are empty; exact huge/infinite conversion follows
the derived shared model. NaN is rejected in both modes as derived safety
policy. Native VFP9 cannot supply Copperfin's exact extended integer kinds.
Other existing coercions retain half-away rounding behind checked signed-int64
and size_t admission; this is preservation, not native type parity.

No argument-sized allocation/loop is added. Existing lookup, alias/numeric
work-area routing, third flags, omitted arguments and cursor/session state
remain unchanged. The Copperfin fixture explicitly SELECTs 0 before its second
CREATE CURSOR (#6988), uses uppercase schema names, and case-normalizes ALIAS
only in assertions. It does not silently repair cursor allocation or case
parity. Its first setup-failure run is discarded as invalid conversion
evidence. The corrected old-dispatch regression is recorded below.

## Focused verification

The independent table covers all 50 Numeric native expressions in both modes.
Direct coverage adds exact int64/uint64, NaN, adjacent-double, signed-int64 and
destination size_t boundaries: 196 direct calls total. The public runtime adds
141 rows for actual field names, both modes, catchable rejection with unchanged
alias/schema, routing, existing coercions/arity/third flags, mode reset and
closed cursors. It does not compute expected results with the production helper.

Portable reproduction from the repository root (run the two ctest commands
serially because the numeric suite owns a common temporary fixture directory):

```sh
cmake -S . -B /home/rich/temp/copperfin-field-index-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-field-index-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions test_prg_engine_table_structure -j4
ctest --test-dir /home/rich/temp/copperfin-field-index-5611-build -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions|table_structure)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-field-index-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-field-index-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions test_prg_engine_table_structure -j4
ctest --test-dir /home/rich/temp/copperfin-field-index-5611-sanitize -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions|table_structure)$' --output-on-failure
```

VR-5611-FIELD-REGRESSION-001: the corrected old FIELD dispatch fails 41
semantic assertions (5.57s); no old-dispatch sanitizer report is claimed.
Normal GCC 15.2 verification passes all four focused suites (7.71s).
VR-5611-FIELD-SANITIZER-001: Clang 21.1.8 ASan/UBSan/float-cast-overflow
passes the same four suites (34.02s) without diagnostics. Changelog assembler,
139-fragment validation, contributor-signoff and agent-channel contracts
also pass 4/4 (3.96s). Hosted exact-head PR checks remain a gate;
this local evidence is not installed-GUI, independent-human or product-wide
parity verification.

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
claiming product-wide completion or changing system state. DV-5611-FIELD-RECOVERY-001:
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

DQ-5611-FIELD-CONVERSION-001 requires the documented mode-specific bounds,
localized catchability, preservation limits, serial verification and rollback
to match the implementation. DV-5611-FIELD-CONVERSION-001: maintainer-authorized
agent self-review checks the source/output identities, independent expected
tables, checked casts and error-before-lookup walkthrough; final focused runs
are recorded above. Misuse severity is medium under the same hazards: native
empty output does not imply exact converted zero or permission to change type
admission. Rejected conversions leave the selected alias and three-field schema
intact. Rollback reverts only this conversion slice, keeping regression rows
that expose the restored unchecked path; these newly filed gaps are not admitted
follow-up production work. Procedural delta: the two build/test command sets
verify this conversion and its four neighboring suites; they do not install
software or change VM/system state. If documentation is wrong, correct the
matrix, handoff and PR/release evidence and notify the owner before reuse.
