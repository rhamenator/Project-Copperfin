# Frozen BITSET input and baseline audit

RQ-CF-PRG-BITSET-NUMERIC-001 mapped before helpers/tests/migration.
Freeze2026-10-09 BEFORE configuration:206 direct checks and792 fresh public
cases in both modes/selected sessions1/2, two unchanged expression neighbors.
184 independent native calls (186 lines) initial plus3repeats byte-identical.
Literal set-bit0/1 observations recover value bits; zero/-1 independently expose
position conversion/late validation; all32 bits, mixed, exact64/NaN/nextafter,
assignment/error/continuation/cursor/mode/reset and safe-coercion controls.
Exact2^53+1 uses two exact64 seeds, not rounded Numeric. Other-type unsafe
position containment is parent-derived, not native parity.

## Frozen source/native hashes

```
027981df554cbb75ae92684ca2feac973aaa0635b321b63b0173b27dc0651903  src/runtime/prg_engine_helpers.h
fbd06f52387148c7941ea303a13fc0cd5a866b100e99c40b315817e1cffd9ed3  src/runtime/prg_engine_helpers.cpp
d5fa711d9634adc52265346e769c18336bbd5025a765a8ffdd9ea71da2a9732f  tests/test_prg_engine_bitset_numeric.cpp
50788cdf7297fbf3c806375170ff35947cf34fb64cac161af468d35f72b021e2  tests/test_prg_engine_bitset_neighbors.cpp
43360672bd1af12af7ef11020a071066043a446f0970103444f0ac05c4ba0957  tests/CMakeLists.txt
be28189e0daa4ed7eab0c0457deb67be06780be9109b9a320f05fffa3159e70a  tests/CopperfinTestIsolation.cmake
21bf6be3e0e4245e8653c1a1dc1011d254e66d98c6d44b3aa8795f53d7b96667  src/runtime/prg_engine_runtime_surface_dispatch_general.inl
639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe  src/runtime/prg_engine_runtime_surface_platform_helpers.inl
9e52514f506d85acf1f05b4977ffe1431fb3d7daaef7614c80624c91adfdd531  tests/fixtures/vfp9-bitset-numeric/probe.prg
da4f79c50e984a5ea7ba96979c902157970c1725357db2b978fd3deee10cb837  tests/fixtures/vfp9-bitset-numeric/probe.out
```

Dispatcher/platform match original main8b2a28ac4a07b5bb85c45f2fbc0d5af8279e7b06.
New value/position checked helpers unused by BITSET through BOTH originals.
No frozen helper/header/assertion/test/CMake/isolation repair/removal/weakening
without explicit refreeze and BOTH original repeats.

## Execution boundary

GNU Debug-g0, then Clang Debug-O1-g0 ASan/UBSan/float-cast-overflow with
matching C/CXX/EXE/SHARED flags/framepointers; leaks disabled, abort/halt/
stacktrace. Serial nice10/ionice2:7 one-job build/test, private TMPDIR and local
CLI600s focused allowance; no workflow/test-definition timeout change.
Complete BOTH original runs, full raw logs and actual counters before dispatcher
migration. No original/fixed/broader/assurance/hosted/review result yet claimed.

## Incomplete disk-backed GNU attempt

Original CTest46846 was interrupted with SIGINT to its exact owned CTest PID
1579319 after observing test PID1579321 DN/jbd2_log_wait_commit at2m16s.
Exit130, no final counters: INCOMPLETE, not all-scenario baseline evidence.
Full original-gcc-disk-interrupted.log SHA-256
b106f1f4f99193be990dcb6c22396c3610c46dac298799321610bb69ee107b37 retained.
No assertion/helper/flags/dispatcher change; repeat uses private RAM-backed
TMPDIR /tmp/copperfin-bitset-5611-zlhE15, reducing test fixture disk writes. Both complete original
compilers and fixed runs use this same private location; no persistence/disk
qualification or diagnosis of prior VSIX/timeouts is claimed.

## Complete unchanged GNU original

GNU15.2 Debug-g0 build succeeded. Same binary/frozen inputs, CTest97300 exit8,
Numeric FAILED15.02s/neighbor PASS0.12s/total15.15s, actual206direct/792public.
540 exclusively selected expectation failures:246error/266result/28result-kind;
zero direct/context/continuation/cursor/reset/neighbor assertions. Full
original-gcc.log SHA-256
ca05949ddc312143649faa2d7b2c1c1fdf4d80b5f37c45d668623c51c32cf6fb.
All frozen source/original dispatcher/platform hashes unchanged. Complete Clang
sanitizer original remains mandatory BEFORE any BITSET migration.

## Complete unchanged Clang original BEFORE migration

Clang21.1.8 matching C/CXX/EXE/SHARED sanitizer flags built successfully.
CTest9957 exit8: Numeric FAILED27.24s/neighbor PASS2.74s/total30.00s,
actual206direct/792public. All540 selected FAIL lines byte-identical to GNU
(246error/266result/28kind;398COPPERFIN/142VFP9); zero unrelated/direct/
context/continuation/reset/neighbor failures or sanitizer diagnostics.
Leaking not tested: detect_leaks0. Full original-clang.log SHA-256
237fdd28471d182c51e859e34732a0f5a09e7de6d1c03621d28774507bcf7e1c.
All six frozen helper/header/test/CMake/isolation and original dispatcher/
platform hashes match BEFORE migration. No frozen-input changes or weakening.
Both complete originals/raw logs are now retained; interrupted GNU attempt
remains separate and incomplete. Timings are observations, not qualification.

## BITSET-only migration and fixed GNU focused result

After BOTH complete originals, ONLY BITSET dispatcher migrated: selected mode
once, checked value AND position even after-1, localized catchable11 before
assignment/shift, uint32 shift/OR and defined signed32 arithmetic in int64.
Fixed dispatcher SHA-256
2973e2b67f5045d3be3e213f025b33ca9a950c6b91ce9f14b6d80c44a5767940.
Shared platform and all six frozen inputs remain unchanged; siblings/global
NULL/arity/type/Currency/binary policy untouched. No argument-driven work.

One-job GNU rebuild succeeded; focused CTest96022 exit0,2/2 PASS14.88s
(Numeric14.75s/neighbors0.11s), actual206direct/792public, no assertion failures.
Full fixed-focused-gcc.log SHA-256
7c8f8b70bc678219a4f0c9cfe71ac553d111332b34563fb657668a247573abcb.
Broader GNU/fixed Clang/documentation assurance and integration gates pending.

## Fixed GNU broader complete

One-job builds succeeded; serial CTest52805 exit0,17/17 PASS503.65s.
Includes BITSET/neighbors (actual206direct/792public), older Numeric71.82s,
NULL, BITXOR/BITOR/BITAND/BITNOT/Collection, localization14.93s and seven
repository contracts. Safety contract374.19s retained its existing1200s cap;
other tests CLI600s. Full unedited216-line fixed-broader-gcc.log SHA-256
2405a30c0d97d0ac92b32e81451734e94d68fa3cb04772a104b5921233eb1de5.
No frozen changes,
unrelated assertion failures or platform/release qualification claimed.
Fixed Clang one-job rebuild now follows, not concurrent with GNU verification.

## Fixed Clang complete

One-job rebuild succeeded; serial CTest42314 exit0,9/9 PASS167.39s.
Numeric102.49s, BITSET27.71s/neighbors0.10s, NULL and BITXOR/BITOR/BITAND/
BITNOT/Collection all PASS; actual206direct/792public and sibling counters in
full unedited123-line fixed-clang.log SHA-256
a92cf664a600bca810320ecbedfa81731eacf5444aa101d9f0e9e6c011851023.
No assertion or ASan/UBSan/float-cast-overflow diagnostics, leaks disabled.
All six frozen inputs/shared platform remain unchanged; fixed dispatcher
2973e2b67f5045d3be3e213f025b33ca9a950c6b91ce9f14b6d80c44a5767940.
No assertion/helper/test repair/removal/weakening or refreeze occurred.
Bounded local VR-5611-BITSET-NUMERIC-001/002 is complete; signed exact-head
verification/review, hosted gates and resolved conversations remain.
