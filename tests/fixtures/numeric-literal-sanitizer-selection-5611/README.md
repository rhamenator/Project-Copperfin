# Sanitizer selection repair / correction to PR7192 evidence

Governing contract and pre-code verification/impact map: [requirements.md](requirements.md).
Refs #7195, #5611/#6776 and the owner Numeric compatibility instruction.
Main baseline ff41cfa99aa048014578211022f32979fe09b1ad.

PR7192's hosted sanitizer job116599363489 / run38786393919 was SUCCESS at
head9384204d27676c787f010b6e00e239608f32a3e3 (tested merge88e80b2950, parents
7d07d7203 and9384204d27). However, its driver compiled the two literal test
targets but omitted both from its separate PR CTest regex. Its PASS must NOT
be cited as execution evidence for those tests. The previously published
instrumented-runtime execution claim is withdrawn; PR7192 correction comment
6200492012 and issue7195 retain this explicitly. Historical frozen evidence
manifests/documents remain unchanged; this is the correcting record.

Before touching the driver, the new independent Python AST regression examines
the actual `command.extend(['-R', ...])` argument without importing/executing
the driver. Original exit1, four failing assertions across three test methods:
both built targets are omitted and both required literal names fail selection.
Original driver/test/requirement hashes and full output retained in evidence/.
After only selection/receipt wiring changes, the SAME three methods pass0.
It also checks all built sanitizer targets are executable under the PR filter
and rejects prefixed/suffixed names. CMake registration plus isolation and
changelog contracts pass4/4. This is selector verification, NOT a claimed
instrumented runtime run.

Architecture: add the two existing literal tests and this Python contract to
the existing bounded PR CTest regex. Keep every previous test, sanitizer flag,
parallelism, timeout and assertion. Sanitizer runs additionally produce CTest
JUnit and retain metadata/ctest.log/JUnit14 days even on success, under a
run/attempt-specific artifact identity. Other lanes do not acquire this new
receipt requirement. This prevents compilation or a generic wrapper PASS
from substituting for named execution evidence. The read-only AST test is
portable/parallel-safe, with no child processes, samples or network.

Acceptance still required: download the new exact-head hosted artifact,
validate matching run/head/tested merge parents and successful NON-skipped
testcase records for BOTH literal tests, plus the selector contract; then
actual clean Claude-first/authorized fallback review, fresh all11 strict
contexts and resolved conversations. Neither local selector PASS nor the old
sanitizer job closes7195. No Numeric algorithm/metadata consumer is changed.

MEDIUM DQ-DV walkthrough/developer self-review: original four failures expose
build-versus-execution drift; fixed unchanged tests verify exact-name selection;
the named hosted receipt is a separate mandatory acceptance step. A missing,
skipped or failed testcase is not acceptance. No independent-human/system/
release claim; HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01
severities/statuses stay unchanged. Rollback coherently removes the repair and
reopens the gap, never restoring the withdrawn execution claim. Notify the
owner and correct these records if receipt interpretation proves wrong.

The uncancelled unary-recovery slice is preserved in its separate worktree,
no production changes: first native probe1/missing result and repeat124/no
result remain unqualified, not behavioral evidence. Resume source/probe
diagnosis and repeated recovery only after this repair. Assignment, arithmetic,
fields, display, ListItem and full Numeric/#7189 compatibility remain open.
