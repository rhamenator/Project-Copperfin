# Unproduced intrinsic support verification

The signed pre-code checkpoint37a3acec5ca29a1603e415358bc9ad50f4446877 freezes
`constant-intrinsic-contract.md` and the640-row native expected header BEFORE
the unused helper. Pre-support hashes of the contract/header and existing
runtime CPP/expression/header all verify0 before and after helper verification.
The native expected fields, source, captures, assertions and timeouts were not
changed to fit this helper. No runtime hook or CMake/sanitizer-lane registration
is introduced in this support phase; there is no PR or integration claim yet.

RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-INTRINSIC-001 maps to
`src/runtime/prg_numeric_constant_division_intrinsic.h` and
`tests/test_prg_numeric_constant_division_intrinsic_helper.cpp`.
`constant_division_intrinsic` validates bounded typed operands and explicit
caller proof of a constant node; it does not infer provenance from metadata.
It returns intrinsic metadata and constant-zero normalization, without SET
DECIMALS/global state, allocation, conversion to integral C++ types, source
matching or operand replay. No existing constructor or public representation
changed. Integer's unused scale is ignored; unsupported input returns unknown.

Actual GNU warnings-as-errors compile0/run0 and Clang explicit ASan/UBSan/
float-cast-overflow/no-recovery compile0/run0: all640 frozen native metadata/
payload rows pass, including existing grouped-unary composition, and all
unknown/malformed/nonfinite/type/NULL/Currency/exact64/zero-divisor/small-divisor/
whole-domain/bounds controls pass. Inputs' descriptors and bits remain unchanged.
Initial compiles/runs passed; no failed assertion or hidden retry. Logs, actual
statuses, compiler versions and source hashes are retained in this evidence
directory. Node source syntax, structural/model/header audits0; changelog valid.

From repository root, reproduce the focused unused-helper verification:

    mkdir -p build/constant-division-native
    g++ -std=c++20 -Iinclude -O0 -g0 -Wall -Wextra -Werror \
      tests/test_prg_numeric_constant_division_intrinsic_helper.cpp \
      -o build/constant-division-native/intrinsic-helper-gnu
    build/constant-division-native/intrinsic-helper-gnu

Use an isolated caller-owned output directory for parallel runs; the test
itself has no filesystem/environment/process/network effects. The actual Clang
run additionally used `-O1 -g -fsanitize=address,undefined,float-cast-overflow
-fno-sanitize-recover=all`, `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. No local VM or SDK code.

MEDIUM DQ-DV-5611-NUMERIC-CONSTANT-DIVISION-INTRINSIC-001 walkthrough: independent
native/header checks -> signed pre-code contract -> pure unused implementation
-> GNU/Clang exact same native rows and unsupported-domain controls -> unchanged
runtime/input SHA checks. Existing corruption/runtime/doc hazards and #6776
mode/checked-conversion/allocation policy remain unchanged. This is development
self-review/automation, not independent-human/system/release qualification.
Rollback removes only unproduced support/test; no field notification is required
because no running product behavior changed.

Next: recover nested constant/materialization/function-boundary and continuation
semantics before selecting transient parser architecture. Freeze independent
ORIGINAL-fail public tests before hooks, then same-test pass and appropriate
broader/hosted/exact-head-review/strict/resolved-conversation gates. Small divisors,
non-whole operands, wider domains, other producers, display/exact conversion
and retained ListItem supplied-column READ stay mandatory. This is NOT full
constant folding, Numeric parity, parent closure or a canonical-double shortcut.
