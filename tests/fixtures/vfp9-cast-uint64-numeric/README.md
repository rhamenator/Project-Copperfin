# CAST unsigned64 Numeric/exact conversion

RQ-CF-PRG-CAST-UINT64-NUMERIC-001, bounded admitted #5611/#6776 slice from
origin/main e2c16c3c22bbd08bece2c137af9c7dc9f0d8e5d6. This requirement,
architecture and verification mapping precedes new helper/tests/migration.

## Independent native boundary and extension derivation

Installed Wine VFP9 SP2 09.00.0000.7423 and shipped Microsoft CAST help
55c599e9-18d1-4ac6-b311-e6677dc3b0b7.htm establish ordinary INTEGER (I/INT),
not UINT64/ULONGLONG/UBIGINT. Shipped-help SHA-256:
8bf6b2a5a06b705090f6597d63e14b359e91146ebe255120923ac0a11404a21d.
No binary inspection, disassembly or decompilation was used.

Four fresh bounded25s Wine observations retain identical38-line/36-call output:
18 ordinary INTEGER/INT/I controls pass (including -0.9=>0, -1.9=>-1 and
signed32 endpoints); all18 unsigned64 alias calls raise11, including zero.
This is an unsupported target-type boundary, not a native unsigned64 range rule.
The first observation overlapped another chat's unrelated local build; it is
retained transparently, not performance evidence. After that build ended, three
fresh serial repeats used nice10/ionice2:7, explicit exit0, and empty stderr.
No VM started and no other chat's process was changed.

Probe SHA-256 fed82671468bead3e2466917546f6700bbb3b8bd1b9937d51f61d1f111bdd8dc.
Each native-1.out through native-4.out SHA-256:
13054cbb9cd2a34515d01c170a87812236151c6f28c24013ecf46cf7ee30b75f.

Owner extension intent2026-10-06 permits meaningful modern capability despite
absence of native syntax. Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001/#5611/#6776
requires defined operation-specific conversion and exact64 safety. Independent
integer/binary64 mathematics supplies [0,18446744073709551615]: the Numeric just
below2^64 is18446744073709549568;2^64 and its next neighbor18446744073709555712
are outside. A Numeric spelling UINT64_MAX rounds to2^64 and must reject, while
exact UINT64_MAX succeeds without a Double round trip. Independent integer sums
18446744073709549568+2047 and9007199254740992+1 yield the literal exact test
values, not values obtained from Copperfin. Python math.nextafter/trunc confirms
the lower Numeric neighbor -0.9999999999999999 truncates to0, while -1 and its
outer neighbor -1.0000000000000002 truncate to-1. Ordinary native INTEGER
fractions support truncation only; they are not native unsigned64 semantics.

Derived contract, identical in COPPERFIN and VFP9 modes:

- Numeric: finite, truncate toward zero, require truncated result in[0,2^64),
  then produce an exact uint64. Thus negative fractions strictly between-1 and0
  become0; -1 and more negative values reject. This is representable truncation,
  not integer modulo, wrap, clamp or invented native behavior.
- Exact uint64: identity over the full range, including UINT64_MAX and low bits
  above2^53. Exact int64: admit nonnegative values exactly; reject negative ones
  before unsigned conversion, including INT64_MIN, with no wrap.
- Invalid Numeric/exact64 domain: localized catchable11 before assignment.
  Error11 is the parent operation-specific conversion policy, not exact
  arithmetic's overflow39. Neither mode invents native64 low32 aliases.
- Ordinary other coercions and global NULL handling are preservation controls;
  no new native type/Currency/binary/arity policy is claimed.

Only the unsigned64 dispatcher branch and distinct checked_cast_uint64_argument
are selected. Signed64 CAST, other target types, parser, arithmetic, DECLARE,
shared coercions and session settings remain unchanged neighbors.

## Verification plan and assurance boundary

VR-5611-CAST-UINT64-NUMERIC-001: independent literal direct helper cases for
fractions, unsigned64 endpoints/nextafter/huge/nonfinite/NaN and exact kinds.
VR-5611-CAST-UINT64-NUMERIC-002: all3aliases in fresh public expressions, both
modes and sessions1/2, exact result kind/value, catchable11, sentinel assignment
retention, subsequent control, cursor/context/reset and four literal catalogs.
Unchanged signed64 CAST and exact-expression tests are neighbors, not oracles.

Freeze helper/header/tests/CMake/isolation before configuration. Keep the new
helper unused by CAST through BOTH complete original GNU and Clang baselines.
Original Clang uses ASan/UBSan/float-cast-overflow with explicit recovery and
halt_on_error=0 to retain complete negative evidence; diagnostic-bearing runs
are NOT clean sanitizer acceptance. Fixed verification uses halt_on_error=1.
Retain full raw logs/counts/hashes/chronology and incomplete attempts. Any
assertion change requires explicit refreeze and BOTH original repeats.
Freeze03:49Z:62 direct and4x159 fresh public cases, five separate CTest
processes; exact count derivation and hashes are in baseline-audit.md.
Complete GNU original: all62/4x159 counters, unchanged signed64 five shards
and exact-expression neighbor complete;960 selected assertions/zero other
failures,15.24s. Complete Clang original: identical960 selected/zero other,
all counters complete,21.74s, four actual unsafe-cast reports at original
unsigned branch677. Both full unedited originals retained BEFORE migration;
diagnostic-bearing originals are NOT clean sanitizer acceptance.
Only then the unsigned64 branch migrated. Fixed GNU11/11PASS15.93s, all
62/4x159 new and56/4x141 old counters complete/exact-expression PASS.
Full194-line raw fixed log and hashes in baseline-audit.md; frozen tests/helper
unchanged. Fixed Clang11/11PASS19.71s with ASan/UBSan/float-cast-overflow,
halt1, no sanitizer diagnostics/leaks disabled; full195-line raw log retained,
same62/4x159 new and56/4x141 old counters complete/exact-expression PASS.
Broader GNU12/12PASS425.47s: four older runtime/localization and all seven
selected contracts. Broader Clang4/4PASS94.61s with the same sanitizer flags,
halt1/no sanitizer diagnostics/leaks disabled. Full294/123-line raw logs and
hashes are retained; expected missing-catalog negative subprocess diagnostics
are not unexpected CTest failures. Integration gates remain pending.

HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain active at
registered severities. Bounded medium documentation misuse includes mistaking a
rounded Numeric for an exact identifier, assuming negative exact integers wrap,
or assuming VFP9 mode provides a nonexistent native64 type. No new critical
operator procedure, argument-sized allocation, external process or persistence
mutation is introduced; no KBX applies.

DQ-5611-CAST-UINT64-NUMERIC-001 procedural delta: undefined/wrapped Numeric and
negative exact conversions become error11; exact unsigned identity preserves
low bits; negative fractional truncation to0 stays defined; both modes agree.
DV-5611-CAST-UINT64-NUMERIC-001 requires a completed bounded delegated
development self-review and automated GNU/Clang walkthrough: -0.9=>0; -1
rejects11 without replacing sentinel; exact UINT64_MAX succeeds; Numeric2^64
rejects11; low bits, context, cleanup and catalogs remain correct. This is not
independent-human/high-hazard qualification. The bounded development
self-review, automated GNU/Clang walkthrough and rollback plan are complete;
the explicit source/expected-outcome/result record is in baseline-audit.md.
This does not close registered hazards or qualify release/full-platform use.

Rollback: revert only this slice and preserve native/original/fixed evidence
and boundary tests; correct guidance and notify affected integrators if
published guidance is wrong. Hosted exact-head checks/clean Claude or authorized
Codex review/resolved conversations remain gates. Full native unsigned64/type/
Currency/arity/leak/platform/persistence/release/family/parent acceptance separate.
