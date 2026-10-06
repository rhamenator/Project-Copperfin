# Installed VFP9 FSIZE Numeric-admission observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the existing owner-installed /home/rich/bin/vfp9-probe runner unchanged.
fsize.prg is the complete clean-room source; fsize.out retains VERSION and
all 64 calls. Two fresh processes produced byte-identical output. The probe
creates only temporary cfprobe/cfother cursors and closes both. No VM,
persistent table, installer, overlay, system change or binary inspection was
used. Generated FXP files are ignored.

The selected #5611/#6776 slice is only the existing FSIZE first-argument
Numeric/exact-integer llround/size_t site in prg_engine_expression.inl.
RQ-CF-PRG-FSIZE-INDEX-NUMERIC-001 follows the owner's consistent-native-behavior
rule in #6776. It does not inherit FIELD's switchable index contract.
Character field-name lookup, alias/work-area routing, omitted arguments and
other type admission remain separate. SET COMPATIBLE/file-size behavior
tracked by #6014 is not implemented or claimed here.

## Recovered requirement and limitations

All 50 Numeric boundary/fractional/huge/infinite expressions and three
Numeric-with-designator controls raise catchable error 11 in installed VFP9.
This is rejection, not a recovered converted index or wrapping operation.
Seven Character controls return expected sizes: ALPHA C(5), BRAVO N(4),
CHARLIE L(1), DELTA C(7) in cfother and ECHO N(6) in its numeric work area;
missing names and Character '2.5' return zero. All 64 calls retain CFPROBE.

Numeric arguments therefore reject before any integral conversion or lookup
in both COPPERFIN and VFP9 modes. Exact int64/uint64 and NaN follow the derived
same-kind safety boundary; native VFP9 does not expose those Copperfin inputs.
Other existing non-Character coercions retain checked half-away rounding
through the shared field-index helper, with signed-int64/size_t validation.
No argument-sized allocation/loop is added.

The native omitted/Currency/Logical/NULL controls are separate: FSIZE() raises
1229; Currency, Logical and NULL raise 11. Preserving the current Copperfin
zero/rounded-index results for those paths is not native type/arity parity.
Focused issue #6990 retains that gap against main 6716b1872; it is not newly
admitted or fixed by this Numeric-only slice.
Only Numeric/exact-integer rejection is selected in this slice.

## Reproduction and identity

From the repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-fsize-index-numeric-observation/fsize.prg
sha256sum tests/fixtures/vfp9-fsize-index-numeric-observation/fsize.prg tests/fixtures/vfp9-fsize-index-numeric-observation/fsize.out
```

Source SHA-256: 17a677f93b3a44f248a71192e80912bf69ff1cc80c754153b50a00618ad172a8.
Output SHA-256: a7ea4eb58369afe006faf5e31082bfee7c3b4bfb0a0a58f02885604a49802de4.
VR-5611-FSIZE-NATIVE-001: both fresh runs complete with 65 matching output
lines, 64 calls and unchanged selected alias, using the displayed command.

## Focused verification

Independent coverage adds 81 direct calls: the 50 native Numeric values plus
NaN, adjacent doubles, signed-int64 boundaries and exact int64/uint64 all
reject; eight existing coercion controls keep checked rounding.
151 public PRG rows cover both modes, catchable errors with unchanged
alias/schema, Numeric routing rejection, Character name/routing preservation,
existing arity/type controls, mode reset and closed cursors. The test explicitly
SELECTs 0 before its second cursor (#6988) and normalizes alias case only in
assertions. It does not change cursor allocation or schema case behavior.

Two old table-structure/SQL-cursor tests incorrectly treated FSIZE(2) as native
field inspection. They now use the correct Character field name for the same
width assertions and independently assert catchable Numeric error 11 for
local and synthetic SQL cursors. SQL result widths are Copperfin regression
preservation, not independently recovered native SQL-provider output.

Portable reproduction (run the two runtime ctest commands serially across
directories, because the numeric suite owns a common temporary fixture path):

```sh
cmake -S . -B /home/rich/temp/copperfin-fsize-index-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-fsize-index-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions test_prg_engine_table_structure test_prg_engine_sql_cursors_mutation -j4
ctest --test-dir /home/rich/temp/copperfin-fsize-index-5611-build -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions|table_structure|sql_cursors_mutation)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-fsize-index-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-fsize-index-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions test_prg_engine_table_structure test_prg_engine_sql_cursors_mutation -j4
ctest --test-dir /home/rich/temp/copperfin-fsize-index-5611-sanitize -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions|table_structure|sql_cursors_mutation)$' --output-on-failure
```

VR-5611-FSIZE-REGRESSION-001: old dispatch fails 125 public PRG assertions plus
one local-table and one SQL-cursor Numeric error assertion (127 total, 8.28s);
named-field and other type/arity preservation controls pass. No old-dispatch
sanitizer report is claimed. All five focused suites pass after the change
under GCC 15.2 Debug (8.52s).
VR-5611-FSIZE-SANITIZER-001: all five focused suites pass under Clang 21.1.8
ASan/UBSan/float-cast-overflow (38.95s), without diagnostics. The changelog
assembler, fragment validity, contributor-signoff and agent-channel contracts
pass (4/4, 4.57s); all 140 fragments validate and git diff --check is clean.
Hosted exact-head checks and review remain PR gates;
local evidence is not installed-GUI, independent-human or product-wide parity.

## Documentation and assurance boundary

DQ-5611-FSIZE-CONVERSION-001 requires the documented consistent Numeric
rejection, preserved paths, serial verification, recovery limits and rollback
to match the checked implementation. DV-5611-FSIZE-CONVERSION-001:
maintainer-authorized agent self-review checks source/output identities,
independent error expectations, checked coercions and error-before-lookup
walkthrough; completed automated results are recorded above before shipping.
This is not independent human verification.

Procedural delta: the native command runs only this two-cursor probe; the
build/test command sets verify the selected boundary and five neighboring
suites without installation or VM/system changes. Misuse severity is medium
under HZ-runtime-crash-01/HZ-data-corruption-01: do not infer Numeric index
wrapping from FIELD, full type/arity parity, persistent-table authority or
native SQL-provider output from these tests. Walkthrough outcome: rejection
leaves selected alias and schema intact, named field queries still work and
temporary cursors close.
Rollback reverts only this slice while retaining regression cases that expose
the unchecked/invalid Numeric path. If documentation is wrong, correct the
matrix, handoff and PR/release evidence and notify the owner before reuse.
