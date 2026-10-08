# MRKBAR original-callback baseline audit

RQ-CF-PRG-MRKBAR-NUMERIC-001; #5611/#6776/#5868/#4618.
Base main 1a7dd8a2a6fa9c62a50d7b46356c008776ac7b55.
Both fresh configurations built the new helper/direct tests and frozen guarded
tests against the UNCHANGED original callback before production migration.
Original prg_engine.cpp blob3422390b36ade981970a53e35df7d3ed7bb2b165,
SHA256 db8b272b9e29e95435f3003b1140bbc4d0c87a7bed8fe26018718352fdab5dde.
TestSHA256 5a6eb2d9fde734ae1a9bbdb137f2819144aad0490d48dbff7589ef8f2dc872bb.
HelperSHA256 efcc9b7bdc03b222f4c2a3fa6df260bc24e4addd06a08531764a10cabc46b4a1.

GCC15.2 Debug: 1/2 failed, new numeric4.46s / neighbors0.11s, total4.59s;
exit8; logSHA256 c170b127ceeef3d815a3e5a921ac8a122cecad652a953725a52568d8323421d7.
Clang21.1.8 -O1 ASan/UBSan/float-cast-overflow: 1/2 failed, numeric4.61s /
neighbors0.09s, total4.72s; exit8;
logSHA256 ccfef93db34c8e98e35459af5de2f6183acf333d1a3c1227b31b577b610234d8.
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1;
UBSAN_OPTIONS=halt_on_error=0:print_stacktrace=1.
Serial CTest, separate private TMPDIRs; no overlapping runtime/native tests.
Sanitizer baseline has zero diagnostics, not evidence of leaks qualification.

Each compiler reported EXACTLY55 query-identity failures, with the same
sorted failure-line SHA256
b0c48ba04ad012ed8567b8daa8749b6b904b3ee3db4488bcbfe68b0afe8df33d.
Zero other failures: all96 direct helper checks, continuation/mark mutation
and events/skip/callback/prompt/count/cursor/session/cleanup guards, and
three original neighbors pass. No assertion removed or weakened to hide a
harness failure. These are Copperfin identity baselines, not native lookup
error parity: existing false fallback remains separate #7095.

All55 failures retained below (identical on GCC and Clang):

```text
FAIL: MRKBAR query identity COPPERFIN 1.5 seed1 session1 UNMARKED got true
FAIL: MRKBAR query identity COPPERFIN 1.9 seed1 session1 UNMARKED got true
FAIL: MRKBAR query identity COPPERFIN 2.9 seed1 session1 UNMARKED got false
FAIL: MRKBAR query identity COPPERFIN 2147483647.9 seed1 session1 UNMARKED got false
FAIL: MRKBAR query identity COPPERFIN 0.5 seed2 session1 UNMARKED got true
FAIL: MRKBAR query identity COPPERFIN 0.9 seed2 session1 UNMARKED got true
FAIL: MRKBAR query identity COPPERFIN 1.5 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity COPPERFIN 1.9 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity COPPERFIN 2147483647.9 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity COPPERFIN 0.5 seed2147483647 session1 UNMARKED got true
FAIL: MRKBAR query identity COPPERFIN 0.9 seed2147483647 session1 UNMARKED got true
FAIL: MRKBAR query identity COPPERFIN 2.9 seed2147483647 session1 UNMARKED got false
FAIL: MRKBAR query identity COPPERFIN 0.5 seed1 session1 MARKED got true
FAIL: MRKBAR query identity COPPERFIN 0.9 seed1 session1 MARKED got true
FAIL: MRKBAR query identity COPPERFIN 1.5 seed1 session1 MARKED got false
FAIL: MRKBAR query identity COPPERFIN 1.9 seed1 session1 MARKED got false
FAIL: MRKBAR query identity COPPERFIN 1.5 seed2 session1 MARKED got true
FAIL: MRKBAR query identity COPPERFIN 1.9 seed2 session1 MARKED got true
FAIL: MRKBAR query identity COPPERFIN 2.9 seed2 session1 MARKED got false
FAIL: MRKBAR query identity COPPERFIN 2147483647.9 seed2147483647 session1 MARKED got false
FAIL: MRKBAR query identity VFP9 1.5 seed1 session1 UNMARKED got true
FAIL: MRKBAR query identity VFP9 1.9 seed1 session1 UNMARKED got true
FAIL: MRKBAR query identity VFP9 2.9 seed1 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 2147483647.9 seed1 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 -4294967294 seed1 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 0.5 seed2 session1 UNMARKED got true
FAIL: MRKBAR query identity VFP9 0.9 seed2 session1 UNMARKED got true
FAIL: MRKBAR query identity VFP9 1.5 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 1.9 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 2147483647.9 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 4294967297 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 -4294967295 seed2 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 4294967297 seed2 session2 UNMARKED got false
FAIL: MRKBAR query identity VFP9 -4294967295 seed2 session2 UNMARKED got false
FAIL: MRKBAR query identity VFP9 0.5 seed2147483647 session1 UNMARKED got true
FAIL: MRKBAR query identity VFP9 0.9 seed2147483647 session1 UNMARKED got true
FAIL: MRKBAR query identity VFP9 2.9 seed2147483647 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 4294967297 seed2147483647 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 -4294967295 seed2147483647 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 -4294967294 seed2147483647 session1 UNMARKED got false
FAIL: MRKBAR query identity VFP9 4294967297 seed2147483647 session2 UNMARKED got false
FAIL: MRKBAR query identity VFP9 -4294967295 seed2147483647 session2 UNMARKED got false
FAIL: MRKBAR query identity VFP9 0.5 seed1 session1 MARKED got true
FAIL: MRKBAR query identity VFP9 0.9 seed1 session1 MARKED got true
FAIL: MRKBAR query identity VFP9 1.5 seed1 session1 MARKED got false
FAIL: MRKBAR query identity VFP9 1.9 seed1 session1 MARKED got false
FAIL: MRKBAR query identity VFP9 4294967297 seed1 session1 MARKED got false
FAIL: MRKBAR query identity VFP9 -4294967295 seed1 session1 MARKED got false
FAIL: MRKBAR query identity VFP9 4294967297 seed1 session2 MARKED got false
FAIL: MRKBAR query identity VFP9 -4294967295 seed1 session2 MARKED got false
FAIL: MRKBAR query identity VFP9 1.5 seed2 session1 MARKED got true
FAIL: MRKBAR query identity VFP9 1.9 seed2 session1 MARKED got true
FAIL: MRKBAR query identity VFP9 2.9 seed2 session1 MARKED got false
FAIL: MRKBAR query identity VFP9 -4294967294 seed2 session1 MARKED got false
FAIL: MRKBAR query identity VFP9 2147483647.9 seed2147483647 session1 MARKED got false
```

Fixed-callback acceptance and completed development DQ/DV are retained in
[the fixture README](README.md). The frozen test/helper hashes above remain
unchanged; only the selected production callback was migrated.
