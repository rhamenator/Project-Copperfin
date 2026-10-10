# CAST signed32 Numeric/exact conversion

RQ-CF-PRG-CAST-INT32-NUMERIC-001, admitted #5611/#6776, from origin/main
8988225cf9bd34ac14b560a4b0edd44bb27fcb87. This requirement and architecture/
verification mapping precedes new helpers, tests, configuration and migration.
Existing Copperfin code/tests are comparison evidence, never requirement sources.

## Allowed evidence and recovered boundary

Installed Wine VFP9 SP2 09.00.0000.7423, not a clone VM, independently executes
the retained probe. Shipped Microsoft CAST help
55c599e9-18d1-4ac6-b311-e6677dc3b0b7.htm (SHA-256
8bf6b2a5a06b705090f6597d63e14b359e91146ebe255120923ac0a11404a21d)
names I/INT/INTEGER and describes potentially lossy Numeric-to-Integer
conversion. It does not specify INT32 or LONG. No decompilation/disassembly.

Three matching native-2.out/native-5.out/native-6.out observations contain
145 calls each plus version/DONE: 87 INTEGER/INT/I controls
and 58 unsupported INT32/LONG calls (error11, including zero). Controls recover
fractional truncation, both-sign low32 aliases beyond signed32, and huge/EXP
overflow results0, not a native extended target-type contract. A supplemental
12-call observation confirms both signed32 nextafter-side decimal expressions
and independently constructed (2^63-1024)=>-1024 / 2^63=>0 / outer=>0.
The large decimal spelling9223372036854774784 returns-2048 in native VFP,
whereas the constructed expression returns-1024. This records a native numeric
literal/formatting boundary, not permission to change Copperfin's parser or
assert identical source parsing. Supplement SOURCE uses STR(...,40,0), which
formats large values coarsely; it is NOT exact binary64 bit evidence.

Initial probe contained a too-long native string literal; it did not compile,
timed out25s (exit124), produced no output, and left a line5 compilation error.
Its original source and normalized diagnostic are retained, not acceptance.
Only the probe string was split before successful native observations; no
Copperfin helper/test existed. Native completion/stdout/stderr/hash chronology
is recorded in baseline-audit.md. No VM started; backup-reset completion is
unconfirmed, and no other chat's process/environment was changed.

## Testable contract derived BEFORE implementation

Parents RQ-CF-PRG-NUMERIC-BEHAVIOR-001, #5611/#6776 and
HZ-runtime-crash-01/HZ-data-corruption-01 require checked operation-specific
conversion and an explicit opt-in for native out-of-range quirks. Owner
modern-extension intent permits INT32/LONG despite absence of native syntax.

- COPPERFIN: finite Numeric, truncate toward zero, require truncated result
  in[-2147483648,2147483648). Fractional -2147483648.9 fits after truncation;
  2147483648 and -2147483649 do not. Exact signed64/unsigned64 must fit signed32
  without a Double round trip. Otherwise localized catchable11 before assignment.
- Explicit VFP9: finite Numeric inside the signed64 conversion domain truncates
  then uses signed low32; huge/outside signed64 and EXP-overflow consumers yield0
  with a defined pre-conversion check, not an unsafe C++ cast. NaN is rejected
  as parent-derived containment; direct infinity0 is a parent-derived mapping
  of the observed overflow consumer, not a claim STR proved native IEEE bits.
  Exact64 low32 preserves all bits, including2^53+1 and full unsigned64; this
  is a parent-derived modern exact-kind boundary, not native exact64 support.
- INT/INTEGER and Copperfin extension INT32/LONG share the above policy and
  produce exact signed32 values held in int64. These aliases do not introduce
  new SET COMPATIBLE or NUMERICBEHAVIOR state.
- Ordinary non-Numeric coercion and NULL dispatch remain unchanged preservation
  controls, not new type/Currency/Character/arity requirements. The Numeric-only
  adapter is not applied to those kinds. Existing Character coercion error1
  and assignment retention remain; no full coercion safety/parity is claimed.

Independent integer/binary64 mathematics supplies literal expectations:
INT32_MIN=-2147483648, MAX=2147483647; unsigned residue2^32-1 is signed-1,
2^32+1 is1 and its negative is-1. Exact2^53+1 residue is1 (negative is-1),
INT64_MAX residue-1, INT64_MIN residue0, UINT64_MAX residue-1. Numeric
nextafter(2^31,0) truncates2147483647; nextafter(2^31,+infinity) truncates
2147483648. Numeric nextafter(-2147483649,0) truncates-2147483648; its outer
neighbor truncates-2147483649. These expectations do not use Copperfin output.

Architecture: distinct checked_cast_int32_numeric_argument(value,behavior),
unused by CAST through BOTH original baselines; selected dispatcher first gates
number/int64/uint64 and reads the selected session's Numeric mode. Only then
replace the four selected aliases' Numeric path. Leave the original non-Numeric
path and INT16/SHORT/BYTE/other targets, shared conversions/arithmetic, DECLARE,
parser and settings unchanged. Signed64/unsigned64 CAST tests are neighbors,
not oracles. Reuse a defined low32 primitive only after this independent recovery.

## Verification and assurance plan

VR-5611-CAST-INT32-NUMERIC-001: independent direct literals, both modes,
fractions/endpoints/nextafter/huge/infinity/NaN and exact64 low-bit boundaries.
VR-5611-CAST-INT32-NUMERIC-002: four aliases, fresh public expressions in both
modes and sessions1/2, literal result/kind/error11, assignment sentinel,
continuation/control, cursor/context/reset and four locale catalogs. Include
unchanged signed64/unsigned64 CAST and exact-expression neighbors. Then broader
Numeric/NULL/typed-NULL/string-math/localization and all seven selected contracts.

Freeze header/helper/tests/CMake/isolation BEFORE configuring. Retain complete,
unedited original GNU and Clang raw logs/counters/hashes BEFORE migration.
Original Clang ASan/UBSan/float-cast-overflow uses explicit recovery/halt0 to
finish every case; diagnostic-bearing originals are NOT clean acceptance.
Fixed builds use the same flags and halt1/leaks disabled. Any assertion repair
requires explicit refreeze and repeating BOTH complete originals.
Serial nice10/ionice2:7/-j1/private RAM fixture storage; no local build/test/Wine
overlap. This is not persistence/disk/timeout-optimum qualification.
Pre-configuration freeze06:59Z:146 direct (both modes) and4x264 fresh public
cases; source/count derivation and frozen hashes are in baseline-audit.md.
Complete GNU original12/16PASS30.25s, all146/four264 and old-neighbor counters,
1216 selected assertions/zero other; full1431-line raw retained before Clang
configuration. Capacity-interrupted first attempt retained separately, never
acceptance. Complete Clang original12/16PASS42.47s, identical1216selected/zero
other, all counters and four actual recoverable float-cast-overflow reports.
Full1813-line raw retained/cmp-verified BEFORE the four-alias Numeric-only
dispatch migration; all frozen inputs remain unchanged. Fixed GNU16/16PASS
30.33s and Clang sanitizers16/16PASS38.16s, all146/4x264new plus unchanged
signed56/4x141 and unsigned62/4x159/exact-expression complete, no diagnostics.
Full unedited raw logs/counters/hashes retained/cmp-identical; fixed halt1,
leaks disabled. Broader GNU11/12 PASS with sole dated-fragment failure retained;
fragment repair/check+assembler2/2 PASS verifies all12 planned subjects across
runs, NOT a single12/12 invocation. Broader Clang4/4 PASS97.84s, no sanitizer
reports. Full raw/hashes retained; runner/capacity interruptions are separate,
never clean acceptance. Signed-head integration remains pending.

DQ-5611-CAST-INT32-NUMERIC-001 procedural delta: default rejects width overflow,
VFP9 opt-in intentionally wraps low32 (never use it for unvalidated identifiers),
exact kinds avoid2^53 rounding, INT32/LONG are modern aliases not native syntax.
Misuse severity medium for this bounded documentation delta: silent wrapped
identifiers, treating low32 as saturation, assuming exact/native source parsing
or all type/CAST safety coverage. Registered HZ-runtime-crash-01,
HZ-data-corruption-01 and HZ-doc-command-01 remain active at their existing
severities; no new critical operator procedure/allocation/persistence boundary
or KBX exception. DV-5611-CAST-INT32-NUMERIC-001 bounded delegated development
self-review and automated boundary/localization/session walkthrough are
complete in baseline-audit.md; no independent-human/high-hazard qualification.
Rollback the bounded code/docs
together, preserve native/raw records, notify the owner if guidance proves wrong.
Signed postpush proof, actual clean exact-head Claude-first/Codex-fallback review,
required hosted checks and resolved conversations gate integration. No full
native/type/Currency/arity/leak/platform/release/family/parent issue acceptance.
