# Frozen BITNOT input and baseline audit

RQ-CF-PRG-BITNOT-NUMERIC-001; requirement mapped before helper/tests/migration.
Freeze2026-10-09 before first configuration. Planned104 direct checks,
156 fresh public cases (both modes/selected sessions1/2), two unchanged
old-neighbor functions. Public expectations independently retain native literal
complements and derived error11, plus checked preserved other coercions;
cursor/mode/setup/continuation/reset guards never derive expected state from
the conversion helper. Exact2^53+1 is built from two exact64 seeds, not a
rounded Numeric literal. Binary/type/arity policy and other bit functions are
not migrated. Native source/output hashes and pre-retention corrections in README.

## Frozen SHA-256 (unchanged through original and fixed runs)

- helpers.h: e6777bc0ed749c9de0fa98aa0b6e88041e299e57b4cb1655c6c09150d711f05a
- helpers.cpp: 68ec9560c2503cc14dbf653e197a2d450e9a34e19452326b749ec2c9a97a2df6
- test_prg_engine_bitnot_numeric.cpp: 7c9985a78096da6aaac7aa3f1345b027cc30514e9bd76c0c93be70731762adcd
- test_prg_engine_bitnot_neighbors.cpp: 50788cdf7297fbf3c806375170ff35947cf34fb64cac161af468d35f72b021e2
- tests/CMakeLists.txt: 2e73c6349defd99ff33cd60eddc00e294bd9132b2c672a16998a3d8e92b73b12
- tests/CopperfinTestIsolation.cmake: bc8baee8b12b9ea0e2706783e8f1e0523a250fbccd55e74bdb444188a619ff40

Original dispatcher SHA823db63f03c985de289a10063cc8fa97ea04a9d2fef6d86f5a03ac26cda0e6d1
and shared platform helper SHA639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe
match origin/main419390ac7. New checked helper is intentionally UNUSED during
both original baselines. The platform helper must also stay original after
BITNOT migration; other bit functions remain separate.

## Verification plan

GNU15.2 Debug-g0 one-job build; Clang21.1.8 Debug-O1-g0 with ASan/UBSan/
float-cast-overflow/frame-pointer and matching C/CXX/EXE/SHARED link flags.
Strictly serial compiler builds/tests; nice10/ionice2:7 and private TMPDIR,
180s focused per-test cap. Leak checking disabled for sanitizer runtime,
halt/abort on actual diagnostics. Retain full original logs, count selected
versus unrelated assertions and diagnostics, then BOTH baselines must be
complete before dispatcher migration. No fixed result claimed yet.
No helper/assertion/test/CMake repair/removal/weakening after this freeze unless
explicitly documented and both original baselines repeated with new hashes.
Broader/older Numeric/localization/contracts results follow below;
postpush exact-head verification and external review/CI remain gates.

## Actual unchanged-original GNU baseline (2026-10-09)

GNU15.2 configuration and single-job Debug-g0 build completed successfully.
All six frozen input hashes and both original dispatcher/platform-helper
hashes above remain identical. The new helper is still UNUSED by BITNOT.
Focused serial CTest completed with exit8: numeric test failed1.13s, unchanged
neighbor test passed0.11s, total real1.25s. All104 direct checks and156 fresh
public cases executed. Exactly110 assertion failures, all selected error or
complement assertions:84 COPPERFIN,26 VFP9;30 error-number,80 complement.
Zero direct or unrelated assertion failures, and no sanitizer run is implied
by this GNU result. The public context/continuation/reset controls passed.
Full unedited CTest output is retained as original-gcc.log (136 lines), SHA-256
203c0e3331bec6d211f5ea4cce6ae91a857b62cb9f2e9263c4952f5b593e9975.
Private TMPDIR, nice10/ionice2:7, -j1 and180s per-test cap were used.

## Actual unchanged-original Clang baseline (2026-10-09)

Clang21.1.8 configuration and single-job build completed with the planned
matching sanitizer flags. All frozen input and original dispatcher/platform
hashes remain identical. Focused serial CTest completed with exit8: numeric
test failed1.81s, unchanged neighbor test passed0.10s, total real1.92s.
All104 direct checks and156 fresh public cases executed. The110 FAIL lines
are byte-identical to GNU (84 COPPERFIN/26 VFP9;30 error/80 complement),
zero direct/unrelated failures; context/continuation/reset controls passed.
No ASan/UBSan/float-cast-overflow diagnostic, with leak checking disabled,
abort_on_error=1 and halt_on_error=1:print_stacktrace=1. Full unedited136-line
CTest output retained as original-clang.log, SHA-256
046a6c23f65580af95f904e925208a912afb6e3eb165d309b1d86a59928988c6.
Private TMPDIR/nice10/ionice2:7/-j1/180s cap used, strictly after GNU completion.

BOTH original runs and full logs are retained before BITNOT-only migration.
No frozen helper/assertion/test/CMake/isolation repair, removal or weakening.
The dispatcher is still original at this recording point; fixed checks pending.

## BITNOT-only migration and first fixed GNU check

After both original logs were retained, only the BITNOT branch of
prg_engine_runtime_surface_dispatch_general.inl was migrated. The checked
operand uses numeric_behavior(set_callback); failed admission raises existing
localized catchable11. The signed32 complement is calculated as -1-int64(x),
avoiding implementation-defined unsigned-to-signed narrowing and preserving
the result kind. Global NULL/arity routing is untouched (source inspection,
not newly recovered full-arity parity). Shared platform helper SHA is unchanged.
Migrated dispatcher SHA-256
f8dee07b722c2faeaf61a68a7ea9d2738198c6fa135b5c52f49601b7f5e10314.

An initial fixed-build command named nonexistent target
test_prg_engine_menu_queries; Ninja rejected it before compiling. Corrected
build command only, with no source/assertion/frozen-input repair. GNU build
then completed. Focused serial private-TMPDIR CTest passed2/2: numeric1.12s,
neighbor0.13s, total1.26s, exit0. Full CTest console output retained as
fixed-focused-gcc.log, SHA-256
5eaf2c0abf1d9d49fa9500a94a8e7ccf45b9ebb2189907d27e1bc787ae7b9f02.
This non-verbose success log does not print the test's case counter; verbose
broader verification will retain that counter. Clang fixed/broader checks
and integration gates were still pending at that point; the frozen six inputs
are unchanged.

## Fixed Clang focused/shared verification

Fixed Clang build completed with matching sanitizer flags and no input repair.
Verbose serial private-TMPDIR CTest passed5/5, exit0, total106.62s:
NULL semantics0.12s, older Numeric behavior102.46s, BITNOT numeric1.74s,
unchanged BITNOT neighbors0.09s, Collection selector numeric2.19s.
The full79-line fixed-clang.log prints actual BITNOT counters104direct/156public
and Collection counters116direct/140public/26hidden; no assertion failure or
ASan/UBSan/float-cast-overflow diagnostic. Leak checking remains disabled;
abort/halt/stacktrace flags and180s default cap were used, strictly serial.
SHA-256 4fcf20de278fdbe69ee1e69b60011027b82b32593c74ed50d2ea85423015843a.
Older Numeric timing is retained without a performance or cause claim.

## Fixed GNU broader verification

GNU verbose focused/shared/localization/seven-contract CTest completed strictly
after Clang:13/13 PASS, exit0, total412.64s. BITNOT numeric1.15s/neighbor0.11s,
older Numeric65.83s, NULL0.13s, Collection1.84s, localization14.39s;
safety workflow contract327.29s. Seven contracts were intake, both changelog
checks, agent channel, native isolation, safety workflow and focused-path filters.
Actual BITNOT104direct/156public and Collection116direct/140public/26hidden
counters retained in full171-line fixed-gcc.log, SHA-256
5414581dd9575ce38356b45bf036d781278db25aeb847eb78ecf097eb2a73071.
No assertion failure; serial/private TMPDIR/nice10/ionice2:7. CLI default600s;
safety contract keeps registered1200s, not a workflow timeout change. Timings
retained without a performance/root-cause claim. Raw verbose log trailing
spaces after CTest's Environment variables heading are preserved byte-for-byte,
not source whitespace errors. Six frozen hashes and shared platform hash remain
unchanged through originals/fixed runs; no helper/test/CMake repair/weakening.

Bounded DQ/DV walkthrough and implementation-agent development self-review
completed in README; not independent human/high-hazard/release qualification.
No parent/residual issue closure. Postpush exact-head proof, actual clean
Claude-FIRST/authorized Codex review, required CI and resolved conversations
remain integration gates.
