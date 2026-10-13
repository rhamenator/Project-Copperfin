## Observed CI gap (not admitted for implementation)

Current main revision: `6fe579e32dcf80e77e0c66e9b07845c908ebecef`.
Observed while validating numeric RemoveItem PR #7168 at exact signed head
`4c83738685eee01f3824e3b122fc3e336f38416b`.

macOS Native Validation [run38310572850/job115015006769](https://github.com/rhamenator/Project-Copperfin/actions/runs/38310572850/job/115015006769)
completed FAILURE2026-10-12T22:05:00Z. Full CTest selected579 in847.96s:
576 passed, one failed, two skipped. ONLY `test_polyglot_runtime_host`
failed (0.95s), with:

```
FAIL: fail-fast should preserve the existing candidate error identity and counts
```

`tests/test_polyglot_runtime_host.cpp:398` combines candidate-failed status,
`polyglot.adapter.nonzero_exit`, candidate selection/authority, zero native
and one candidate invocation, and no native fallback into one assertion.
The child `--host-error` mode returns42; the current raw failure does NOT
identify which condition differed. Timeout/exit-status loss/platform race
are not established causes. Do not classify this as a flake from one run.

The test, polyglot host, adapter and bounded-process code/headers are unchanged
between this PR head and the current main revision. All five RemoveItem
subjects passed on macOS. Separate same-head Linux native run38310572797/
job115015008307 reported100%/zero failures out579 selected in986.12s,
including the polyglot-host subject; its platform-marked skips are not executed
test acceptance. Neither platform's generated-launcher skip substitutes for
its separate required launcher check.
Non-mutating local GNU diagnostic repeated the unchanged host test10 times:
all10 PASS, overall8.00s/status0 completed22:17:32Z. This does not reproduce,
explain, repair or clear the macOS failure, and is not native macOS evidence.

Investigation-ready full raw evidence retained in the owning implementation
chat's `build/removeitem-native/oldhead-platforms/` with run metadata,
successful download statuses and source-delta checks. Full3702-line macOS
raw SHA256 `d6065519c7ed4a8a55b863fe33be01a23bc8c12caadbc4570823c57aac718425`.
Full3869-line Linux rawSHA `218c5987ba95a0bdb574c636bdd1636448aa8b015fe698fcc71a1a833f4afb59`.
Local repeat rawSHA `67da7a9765094c7c5f663bdc8cf00ac02c69a30ee296f20e6e19a916ea0056be`.
Evidence will be preserved with the slice cleanup record before its scratch
build is removed. No assertion relaxation, retry-disguised-as-original-pass,
production workaround, workflow setup change or VM action occurred.

This focused issue records a genuinely new CI problem discovered during
the admitted implementation workflow. It is NOT admitted for implementation
and does not expand PR7168's numeric scope. Future investigation should retain
the actual dispatch result fields on a failing macOS run and distinguish
fixture, child admission/launch, exit-status and host-result mapping boundaries
before choosing a repair. Revised PR-head macOS validation remains follow-up
evidence; an eventual pass must not rewrite this recorded failure as success.
