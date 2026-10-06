# Installed VFP9 SELECT Numeric-selector observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the owner-installed /home/rich/bin/vfp9-probe runner unchanged.
select.prg is the complete clean-room source; select.out retains VERSION and
all 60 calls. Two fresh processes produced byte-identical output. Only two
temporary cursors are created and closed. No VM, installer, persistent table,
overlay, system change or binary inspection was used. Generated FXP is ignored.

## Selected requirement and boundary

RQ-CF-PRG-SELECT-SELECTOR-NUMERIC-001 derives from #5611/#6776 and
HZ-runtime-crash-01/HZ-data-corruption-01. Only SELECT()'s first-argument
llround/int site is selected. Numeric fractions truncate toward zero;
the admitted converted selector domain is 0..32767. Invalid conversion or
domain raises localized catchable error 17 before existing query callbacks.
Default COPPERFIN requires a finite checked signed-int32 value before domain
admission. Explicit VFP9 retains shared signed-low-32 aliases and
huge/infinite-to-zero behavior. Exact int64/uint64 avoid double rounding;
NaN rejects both modes as a derived safety boundary, not a native NaN claim.
Existing other coercions retain checked half-away rounding, without native
type/arity parity.

The 50 native Numeric calls independently distinguish truncation, +/-2^32
aliases, valid 32767 versus invalid 32768, error 17 and huge/infinite zero
behavior. Native current-area zero, unused-area one and alias-two outputs
distinguish those converted selectors. Unused-area 3/4 and 32767 return zero
in this two-cursor fixture; their exact converted values are shared-model
policy rather than independently distinguished native indices.

Important separate gap #6013: SELECT(0), SELECT(1) and omitted COMPATIBLE
routing are already wrong in Copperfin. This slice preserves those callbacks
and compares converted calls with canonical SELECT values to isolate
conversion; it does not claim native current/unused-work-area result parity.
SELECT commands, alias lookup, data-session/allocator state, omitted arguments,
COMPATIBLE behavior and non-Numeric admission are not production changes.
No argument-sized allocation or loop is added.

Native controls record omitted SELECT() and Character alias/missing-name
queries, Currency and Logical/NULL differences. The non-Numeric admission gap
is filed separately as #6994 against main e15a654ec03f85692e996042688bd28046c0db95.
Those observations do not expand this Numeric conversion slice.

## Reproduction and identity

From the repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-select-selector-numeric-observation/select.prg
sha256sum tests/fixtures/vfp9-select-selector-numeric-observation/select.prg tests/fixtures/vfp9-select-selector-numeric-observation/select.out
```

Source SHA-256: 9226e04ecea01554ef0b592b43931da6ee77ed53d0d0a99d9922e14092bb4d78.
Output SHA-256: bf16778d28314774aac4b26e26a18db6d4180b5bbc4b4ff976a69f1391994a9c.
VR-5611-SELECT-NATIVE-001: two fresh runs match all 61 output lines,
including 60 calls and unchanged selected CFPROBE.

## Focused verification

164 direct calls cover the independent native table, adjacent doubles at
0/32768, signed-int64 and exact extended integer boundaries, NaN and checked
Currency/Logical/NULL preservation in both modes.
126 PRG rows cover canonical conversion comparisons, catchability with
unchanged alias/schema, name lookup, preserved coercions, mode reset and
closed cursors. Test cursors use explicit areas 1/2 to isolate #6988.

Run the runtime ctest commands serially across build directories because
the numeric suite owns a common temporary fixture path:

```sh
cmake -S . -B /home/rich/temp/copperfin-select-selector-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-select-selector-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions test_prg_engine_work_areas -j4
ctest --test-dir /home/rich/temp/copperfin-select-selector-5611-build -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions|work_areas)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-select-selector-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-select-selector-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions test_prg_engine_work_areas -j4
ctest --test-dir /home/rich/temp/copperfin-select-selector-5611-sanitize -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions|work_areas)$' --output-on-failure
```

VR-5611-SELECT-REGRESSION-001: the original SELECT dispatch fails 68 semantic
assertions (numeric suite 6.08s; four-suite baseline 35.01s). All 164 direct
helper calls and non-Numeric/alias/state controls pass. An earlier run with
incorrect Logical-output formatting is excluded from this evidence.
The checked dispatch passes all four GCC suites (35.55s; numeric 6.15s).
The four changelog/signoff/channel contracts pass (3.91s), with 141 valid
changelog fragments; git diff --check and fixture SHA-256 identities pass.
VR-5611-SELECT-SANITIZER-001: all four Clang ASan/UBSan/float-cast-overflow
suites pass (137.19s; numeric 27.08s), without sanitizer diagnostics.
Hosted exact-head checks and review are separate PR gates. This is not
independent-human, installed-GUI or full SELECT/product parity evidence.

## Documentation assurance

DQ-5611-SELECT-CONVERSION-001 requires documented conversion/domain admission,
preserved query boundaries, serial reproduction, evidence limits and rollback
to match the checked implementation.
DV-5611-SELECT-CONVERSION-001: maintainer-authorized agent self-review checks
source/output identities, independent expectations, shared checked conversion
and error-before-callback walkthrough; completed automated results above are
required before shipping. No independent-human verification is claimed.

Procedural delta: this native command runs only the temporary two-cursor probe;
focused normal/sanitizer commands validate this boundary and three neighboring
suites without installation, VM or system changes.
Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01:
do not infer repaired current/unused-area or COMPATIBLE routing, native
non-Numeric parity or allocator correctness from canonical comparisons.
Walkthrough outcome: rejected Numeric selectors preserve selected alias/schema;
name lookup continues and temporary cursors close.
Rollback reverts only this conversion while retaining independent unchecked-
path regressions. If documentation is wrong, correct the matrix, handoff and
release/PR evidence and notify the owner before reuse.
