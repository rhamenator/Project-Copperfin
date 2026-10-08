# UNLOCK RECORD numeric-conversion recovery (#5611/#6776)

RQ-CF-PRG-UNLOCK-RECORD-NUMERIC-001 derives from
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and the admitted owner-authored #5611/#6776.
This is only explicit UNLOCK RECORD operand conversion, not a complete lock,
record-existence, type-admission, buffering or filesystem compatibility claim.

## Installed native oracle

Unchanged installed Wine VFP9 09.00.0000.7423, run through the existing
/home/rich/bin/vfp9-probe wrapper. Three strictly serial successful executions
of probe.prg produced identical 74-line output in probe.out: version,
36 operands in each selected/explicit-IN form, and cleanup.
The program owns its disposable unlock-probe-owned.dbf, opens it SHARED,
sets MULTILOCKS ON, verifies all three record locks before EVERY operand,
and records each lock after the command, record pointer, selected alias,
data session and continuation status. It unlocks/closes/deletes that owned
table and closes the observer/reset MULTILOCKS OFF at completion.
No external database, OS lock backend, VM or configuration was modified.

Source SHA256: 6a2bdb4db1011c97b72bf5d026761be2f9b3ae31f668c2c38c3e84f7d19ca1a0.
Output SHA256: 8b79636164173c373e1d2145d9a40310f71dd06e2d727ce80cc68236cc8fc81e.
An earlier cursor-only preliminary probe could not observe native locks:
RLOCK returned true but ISRLOCKED was false even before release. One first
shared-table setup attempt timed out after25s with no retained native failure
diagnostic; its source used the wrong alias after CREATE TABLE. Neither is
counted as acceptance or used to infer a runtime defect or timeout root cause.

Recovered facts (both selected/IN forms; pointer2/alias/session1 preserved):

- Numeric fractions truncate:1.5/1.9 release record1,2.5 releases2,3.9 releases3.
- -1.9 and -2147483648 raise10 without releasing a lock.
- -0.9/0/0.5/0.9 succeed without releasing any lock.
- Positive2147483648/4294967295 raise10, no release.
- Positive4294967296/4294967297/4294967298 succeed, NO small-row alias.
- Negative-2147483649 succeeds with no small-row release; -4294967295
  releases1. -4294967296/-4294967297 reject10.
- Positive2^53,2^63-adjacent,1E20,1E300 and overflowing positive
  multiplication succeed without releasing any lock. Corresponding sampled
  huge negatives reject10. This is NOT SKIP's symmetric low32 rule.
- Positive4/2147483647 (nonexistent targets) succeed without release.
- Currency1.5, String1.5, true and NULL raise10 without release.

## Bounded requirement / architecture / verification mapping

Recovered/derived contract mapped before dispatcher implementation:

- Default: Numeric truncation/exact-integer comparison into0..INT32_MAX;
  reject nonfinite/NaN/out-of-domain input before integral conversion.
- Explicit VFP9: retain sampled negative-to-positive low32 aliases ONLY;
  low32 zero/negative rejects. Positive2^31..2^32-1 rejects. Positive>=2^32
  retains its wide non-record target when within the checked shared int64
  model, otherwise uses zero. Never turn a positive oversized target into a
  small existing row. Exact uint64 preserves its original wide target.
  Values outside sampled native points and exact integer kinds are owner-
  derived defined extensions, not independently measured native guarantees.
- NaN rejects both modes. Other already accepted safe coercions retain checked
  half-away rounding and the default signed32 domain in both modes; native
  type parity remains a separately recorded gap.
- Missing conversion raises catchable localized10 BEFORE lock release and
  successful runtime.unlock RECORD event. Format the original Number through
  the existing safe decimal formatter, not an unchecked integral formatter.
- Preserve resumed evaluation/generation validation. Use the captured target
  data session's NUMERICBEHAVIOR during conversion and restore current session
  afterwards. Pass only a range-admitted size_t to the unchanged lock helper.
- Existing zero/nonexistent-record error policy, UNLOCK/ALL, file locks,
  ownership, pointers, dirty buffers and unrelated conversions stay unchanged.
  Native harmless no-op differs from that existing post-conversion policy;
  do not silently implement a broader lock/existence redesign here.

Code mapping: checked_unlock_record_argument in prg_engine_helpers.h/.cpp,
only unlock_command explicit-record conversion in prg_engine_dispatch.inl,
four locale catalogs; unchanged unlock_cursor_record_lock storage/release.
Reverse requirement links are in helper/dispatcher/test source.

VR-5611-UNLOCK-NUMERIC-001: independent helper constants, both-mode
selected/IN lock/pointer/session/setup/cleanup guards, original dispatcher
negative execution, fixed focused GCC and Clang ASan/UBSan/float-cast-overflow.
VR-5611-UNLOCK-NUMERIC-002: error10/ERROR/AERROR and safe four-locale message,
resumable target-session mode, existing navigation/replacement and lock
neighbors, older Numeric, localization and repository contracts.
Separate unchanged-main gaps: #7085 tracks harmless missing-record no-op,
native type admission and inherited explicit-error metadata on subsequent
generic failures; #7086 tracks two-argument ISRLOCKED alias routing. Neither
is admitted or implemented by this slice. Guarded tests capture caller alias
before SELECTing the target for one-argument lock observations and use a
fresh runtime session per operand. They do not establish native type,
post-conversion error lifecycle, alias-query or OS-lock compatibility.

## Retained negative and fixed verification

baseline-audit.txt binds exact base main, byte-identical original dispatcher,
provisional helper, then-current grouped test source and raw execution hashes.
Both original GCC/Clang runs fail exactly76 selected assertions (74 guarded
rows/two successful-event counts), zero unrelated failures and no sanitizer
diagnostics.82 direct helper calls and four then-selected navigation/buffering
neighbors pass. The fixed source subsequently adds independent fresh-session
isolation, locale/metadata, target-session, dirty-row/disk and two old lock
neighbors; these additions are NOT relabeled as original baseline evidence.

Clang21.1.8 Debug -O0 -g0 fixed two focused suites pass2/2,8.80s
(new7.60s/neighbors1.19s), with ASan/UBSan/float-cast-overflow,
ASAN detect_leaks=0:abort_on_error=1 and UBSAN halt_on_error=1:print_stacktrace=1.
No sanitizer diagnostics; no leak-verification claim. Fixed log SHA256
aee762c24d2253885ac90f7a853fd71f375500f2486fd81f6eefa229ce166992.
Tests cover82 independent helper calls,124 fresh guarded selected/IN operands,
twelve four-locale metadata cases, two resumed target-session cases, four
dirty-row2/3 disk-preservation cases and six existing navigation/lock neighbors.
Final test source SHA256 (after explanatory-only isolation comment):
beb3bb9e180eafec6a14565a8618f2496c615d18b043b79ebb66ba396e8e8ed4.
GCC15.2 Debug -O0 -g0 passes4/4,151.88s: new2.31s/neighbors0.32s,
full older Numeric134.25s and localization14.99s. Fixed GCC log SHA256
568903872f3d9a8cba10f9af886105c3897391e447279af237868bf961e25661.
Seven repository contracts pass7/7,360.35s (safety workflow348.84s): locale
install, agent intake, DCO policy, changelog assembler/176 valid fragments,
native isolation and safety traceability workflow. Focused sanitizer evidence
is not full regression or hosted platform qualification.

Retained actual fixed controls (code|continued|locks1/2/3|pointer|payload|
caller|session|mode|all-locks setup), with separate existence/type gaps visible:

```text
COPPERFIN 1.5: 0|T|F|T|T|2|two|UNLOCKGUARD|1|COPPERFIN|T
COPPERFIN -4294967295: 10|F|T|T|T|2|two|UNLOCKGUARD|1|COPPERFIN|T
VFP9 -4294967295: 0|T|F|T|T|2|two|UNLOCKGUARD|1|VFP9|T
VFP9 4294967297 IN: 1|F|T|T|T|2|two|OBSERVER|1|VFP9|T
VFP9 $1.5: 0|T|T|F|T|2|two|UNLOCKGUARD|1|VFP9|T
```

## Hazard / documentation assurance scope

HZ-runtime-crash-01: no unchecked floating-to-integer conversion.
HZ-data-corruption-01: no unintended lock release from a rounded fraction
or positive overflow alias; invalid conversion precedes mutation/event.
HZ-doc-command-01: distinguish conversion admission from missing-record/type
gaps and in-process versus OS locking. Plausible misuse severity medium.
DQ-5611-UNLOCK-NUMERIC-001: instructions must identify mode, target-session
scope, exact extension/no-alias rule, error metadata and retained gaps.
DV-5611-UNLOCK-NUMERIC-001 completed development maintainer self-review:
compared the retained74-line oracle and all36 sampled operands across both
forms against the helper's checked domains; inspected every cast for prior
admission, restored scoped session and post-evaluation generation check.
Walkthrough confirmed fresh default -4294967295 rejects10 retaining all locks,
explicit VFP9 releases only row1; positive4294967297 never releases row1 in
either mode; Numeric1.5 releases only row1 without moving pointer2. Four
locales retain error10/ERROR/AERROR and safe original huge operand; resumed
session callback runs once and restores session2 while using origin's policy.
Dirty-row cases retain pointer/lock/edit and disk two; cleanup closes owned
cursors and resets mode/MULTILOCKS. Native zero/nonexistent success differs
from fresh Copperfin code1 and native Currency/String rejection differs from
safe retained coercion; those were explicitly traced to7085, not silently
approved as compatibility. Alias-query discrepancy is7086. Grouped stale
error metadata is recorded, final per-case isolation is not a lifecycle fix.
These completed GCC/Clang checks and source/output walkthrough are maintainer
development self-review under owner authorization, NOT independent human
verification, full native equivalence or a certification assertion.
Procedural delta: checked conversion replaces llround and preserves later
existence checks; existing valid fractions now follow native truncation.
Rollback: revert only the isolated PR/helper/catalog/tests/docs changes.
Field notice: explain changed fraction/domain behavior and VFP9 opt-in,
never recommend opting in as a cure for untrusted numeric input.
No release/platform/leak/tool-qualification/certification claim.
