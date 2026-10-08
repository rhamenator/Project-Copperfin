# SKPBAR original-callback baseline audit

RQ-CF-PRG-SKPBAR-NUMERIC-001; #5611/#6776/#5868/#4617.
Base main89fd9ee5b6dcc14f2183cd893b2425340e49c243.
Both fresh configurations built the new helper/direct tests and frozen guarded
tests against the UNCHANGED original callback before production migration.
Original prg_engine.cpp blob4d2f4727e16c532a7d8c74c533fe725177c1a384,
SHA25638d0a63fe03ae1415a4d3b63be3341f1f2d786d9e9d869b877f959fd6eeaa6a7.
TestSHA256c39704e731ed00eeeed02a26fed70ff78514436502f402b5adc4312fcb668306.
HelperSHA256b7d3a07c4e9c68209e3ac0ae758b412f3ffd70cab28c42b579237a712bf52005.

GCC15.2 Debug:1/2 failed, numeric4.34s/neighbors0.10s,total4.46s;exit8;
logSHA2566132cc5afdc73cdf364885f24b9589d0bfe102b796640ad4d82548488dd3ff95.
Clang21.1.8 -O1 ASan/UBSan/float-cast-overflow:1/2 failed, numeric4.65s/
neighbors0.09s,total4.76s;exit8;
logSHA256f70e201286502683269278fb5f341852b9e5985dbf561ea903aa7b24dcd29886.
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1;
UBSAN_OPTIONS=halt_on_error=0:print_stacktrace=1.
Serial CTest, separate private TMPDIRs; no overlapping runtime/native tests.
No runtime sanitizer diagnostics; this does not qualify leak behavior.
Additional GCC shared-helper MRKBAR baseline1/1 PASS4.57s(numeric4.56s),
logSHA256ce66eac61f2faa43e84a44db97352367c56eeae559fdae92ba777b033c5dbd54.

Each compiler reported EXACTLY55 query-identity failures with the same sorted
failure-line SHA25671c87c4632d9bd68120622161f8bb5eb0a7a3d98508c2b159bac7d6da0257254.
Zero other failures: all96 direct helper checks, continuation/skip mutation
and events/mark/callback/prompt/count/cursor/session/cleanup guards, and
three original neighbors pass. No assertion was removed or weakened to hide
a harness failure. This is identity evidence, not native error parity:
first-pass false fallback remains separate from native1612 gap#7098.

All55 failures retained below(identical on GCC and Clang):

```text
FAIL: SKPBAR query identity COPPERFIN 1.5 seed1 session1 ENABLED got true
FAIL: SKPBAR query identity COPPERFIN 1.9 seed1 session1 ENABLED got true
FAIL: SKPBAR query identity COPPERFIN 2.9 seed1 session1 ENABLED got false
FAIL: SKPBAR query identity COPPERFIN 2147483647.9 seed1 session1 ENABLED got false
FAIL: SKPBAR query identity COPPERFIN 0.5 seed2 session1 ENABLED got true
FAIL: SKPBAR query identity COPPERFIN 0.9 seed2 session1 ENABLED got true
FAIL: SKPBAR query identity COPPERFIN 1.5 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity COPPERFIN 1.9 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity COPPERFIN 2147483647.9 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity COPPERFIN 0.5 seed2147483647 session1 ENABLED got true
FAIL: SKPBAR query identity COPPERFIN 0.9 seed2147483647 session1 ENABLED got true
FAIL: SKPBAR query identity COPPERFIN 2.9 seed2147483647 session1 ENABLED got false
FAIL: SKPBAR query identity COPPERFIN 0.5 seed1 session1 DISABLED got true
FAIL: SKPBAR query identity COPPERFIN 0.9 seed1 session1 DISABLED got true
FAIL: SKPBAR query identity COPPERFIN 1.5 seed1 session1 DISABLED got false
FAIL: SKPBAR query identity COPPERFIN 1.9 seed1 session1 DISABLED got false
FAIL: SKPBAR query identity COPPERFIN 1.5 seed2 session1 DISABLED got true
FAIL: SKPBAR query identity COPPERFIN 1.9 seed2 session1 DISABLED got true
FAIL: SKPBAR query identity COPPERFIN 2.9 seed2 session1 DISABLED got false
FAIL: SKPBAR query identity COPPERFIN 2147483647.9 seed2147483647 session1 DISABLED got false
FAIL: SKPBAR query identity VFP9 1.5 seed1 session1 ENABLED got true
FAIL: SKPBAR query identity VFP9 1.9 seed1 session1 ENABLED got true
FAIL: SKPBAR query identity VFP9 2.9 seed1 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 2147483647.9 seed1 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 -4294967294 seed1 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 0.5 seed2 session1 ENABLED got true
FAIL: SKPBAR query identity VFP9 0.9 seed2 session1 ENABLED got true
FAIL: SKPBAR query identity VFP9 1.5 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 1.9 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 2147483647.9 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 4294967297 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 -4294967295 seed2 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 4294967297 seed2 session2 ENABLED got false
FAIL: SKPBAR query identity VFP9 -4294967295 seed2 session2 ENABLED got false
FAIL: SKPBAR query identity VFP9 0.5 seed2147483647 session1 ENABLED got true
FAIL: SKPBAR query identity VFP9 0.9 seed2147483647 session1 ENABLED got true
FAIL: SKPBAR query identity VFP9 2.9 seed2147483647 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 4294967297 seed2147483647 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 -4294967295 seed2147483647 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 -4294967294 seed2147483647 session1 ENABLED got false
FAIL: SKPBAR query identity VFP9 4294967297 seed2147483647 session2 ENABLED got false
FAIL: SKPBAR query identity VFP9 -4294967295 seed2147483647 session2 ENABLED got false
FAIL: SKPBAR query identity VFP9 0.5 seed1 session1 DISABLED got true
FAIL: SKPBAR query identity VFP9 0.9 seed1 session1 DISABLED got true
FAIL: SKPBAR query identity VFP9 1.5 seed1 session1 DISABLED got false
FAIL: SKPBAR query identity VFP9 1.9 seed1 session1 DISABLED got false
FAIL: SKPBAR query identity VFP9 4294967297 seed1 session1 DISABLED got false
FAIL: SKPBAR query identity VFP9 -4294967295 seed1 session1 DISABLED got false
FAIL: SKPBAR query identity VFP9 4294967297 seed1 session2 DISABLED got false
FAIL: SKPBAR query identity VFP9 -4294967295 seed1 session2 DISABLED got false
FAIL: SKPBAR query identity VFP9 1.5 seed2 session1 DISABLED got true
FAIL: SKPBAR query identity VFP9 1.9 seed2 session1 DISABLED got true
FAIL: SKPBAR query identity VFP9 2.9 seed2 session1 DISABLED got false
FAIL: SKPBAR query identity VFP9 -4294967294 seed2 session1 DISABLED got false
FAIL: SKPBAR query identity VFP9 2147483647.9 seed2147483647 session1 DISABLED got false
```

Fixed-callback acceptance and completed development DQ/DV are retained in
[the fixture README](README.md). The frozen test/helper hashes above remain
unchanged; only the selected production callback is migrated.
