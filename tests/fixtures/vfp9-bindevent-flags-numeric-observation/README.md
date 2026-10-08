# BINDEVENT flags numeric-conversion slice (#5611/#6776)

## Governing requirement recorded before implementation

RQ-CF-PRG-BINDEVENT-FLAGS-NUMERIC-001 derives from owner-admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Installed VFP9 09.00.0000.7423 observes Numeric truncation, rejection of
negative converted flags with catchable error 11, and bits 0/1 controlling
after/before and suppression of simple-method delegates. Other positive bits
are ignored. Three fresh identical 49-line runs retain 43 operand observations,
four failed-rebind controls preserving the original after-handler, and native
routine-delegate syntax error 11. The initial VERSION(5) string concatenation
failed before any observation; it was repaired, not treated as runtime evidence.

COPPERFIN shall truncate Numeric flags and require 0..INT32_MAX before integral
conversion. Negative fractions whose truncation is zero remain valid. Explicit
VFP9 shall use the shared defined signed-low-32/indefinite-zero conversion,
reject negative converted flags, and expose only bits 0/1. Exact int64/uint64
extensions follow the same checked domain or exact-low-bit policy without
double precision loss. Native absence of exact integer/routine syntax is not
grounds to omit the extension: owner modernization intent derives identical
flag policy for the supported routine-delegate form.

Missing conversion shall raise localized catchable 11 before a binding insert,
duplicate replacement, ordinal consumption or successful bind event. No
argument-sized allocation or loop is added. Existing other-type half-away
coercions remain checked signed32 and masked; native type/Currency parity is
not claimed. Only the object-method fifth flag and routine fourth flag sites
are selected. Window handles/messages, UNBINDEVENTS, event lifetime, property
read behavior, routing, arity and general type policy remain separate.

Architecture: checked_bindevent_flags_argument in helpers.h/.cpp, then the two
flag branches in prg_engine_event_binding_dispatch.inl. Verification plan:
independent direct boundary table including exact extrema/non-finite values;
both forms/modes PRG setup/rebind/error/message/simple/raised/cleanup/state
and successful-event counts; fail-before/pass-after GCC and focused Clang
ASan/UBSan/float-cast-overflow; older Numeric and event neighbors. No behavior
completion from this plan alone or independent-human/platform/release qualification
is claimed; completed local acceptance is recorded below.

## Completed verification and evidence identities

VR-BINDEVENT-FLAGS-001: 132 direct checks cover Numeric fractions,
non-finite/signed32/signed64 adjacent boundaries, exact int64/uint64 extrema
and precision, and preserved NULL/Boolean/Character/Currency coercions.
VR-BINDEVENT-FLAGS-002: 176 script cases across both modes and both delegate
forms verify status/error 11 and English message, unassigned result on error,
preserved existing binding, before/after order and simple-call suppression,
unbinding count, cursor row/count/payload, active session and mode, reset,
and successful bind/delegate/raise/unbind event counts. Existing binding count
1 is a Copperfin preservation control, not native return-type qualification.
Ordinal failure atomicity additionally uses source review; no internal counter
introspection is claimed. Arguments are evaluated before dispatch; no claim
that flag rejection suppresses argument evaluation or source/target lookup.

Original dispatcher remained byte-identical to main
b00a4758b6fbd790cb381aa9505499b726cc5079 while the independent helper and
tests were linked. Both selected llround sites were unchanged. Original GCC
failed 98 selected assertions (90 rows,8 event counts) in 0.71s; Clang
ASan/UBSan/float-cast-overflow failed the same 98 in 3.08s, no diagnostic.
132 direct helper checks, setup/cleanup/reset and coercion controls passed;
zero cursor/session/mode suffix mismatches. Condensed audit retains counts,
dispatcher and raw-log hashes, not full raw logs or an invented cast diagnostic.
The final test additionally scopes/restores en-US explicitly; the original
observed messages were already English. No full Numeric sanitizer baseline
or floating-environment qualification is claimed.

VR-BINDEVENT-FLAGS-003: fixed GCC six suites pass 6/6 in 87.87s, including
full older Numeric (86.15s), new flags (0.73s), string/math, NULL, native
focus and native lifecycle neighbors. Clang sanitizer five suites pass 5/5
in 8.07s (flags4.01s), without diagnostics; this excludes full older Numeric.
ASAN detect_leaks=0, no leak qualification. Runtime checks ran serially across
both builds, each with a private TMPDIR. Six repository contracts pass 6/6
in 5.15s, including source-backed new-test isolation: test-owned roots,
scoped/restored locale, no child/network/samples/shared resources, serial.
Existing missing-field/unused-function/unused-capture warnings remain; this
is not warning-free/platform/release qualification.

Native source SHA256:
52adc7e10caac934f887d40e96b1a20211c6029265065e024dfe628a2ab19ee3
Native output SHA256:
a2190ff769b3422272e6b167d9f7c1905c8ca71dbe54fa70895fe36390d4a2ef
Condensed original audit SHA256:
884e7cb41369386d2783354964bd94ab76553856fa91f9fc0c3f87e76ca8af80

Native non-Numeric/Currency observations conflict with existing Copperfin
coercions; focused issue #7070 records the gap against the pinned main.
This slice does not implement it or acquire authority from that issue.
Window-message conversions and property-read/lifetime/routing/arity remain
separate; the routine extension remains supported despite native syntax error.

## Documentation safety and completed development review

DQ-BINDEVENT-FLAGS-001: distinguish native observations from derived default/
extension boundaries and preserved type/return controls; maintain requirement/
code/test links in docs32 and the coverage/changelog notice.
DQ-BINDEVENT-FLAGS-002: use serial, private scratch execution; preserve source,
output/audit identities and disclose failed baseline and verification limits.

Procedural delta: unsafe half-away llround/narrow/mask becomes Numeric
truncate/check/mask; opt-in VFP9 wraps through the existing checked model.
Callers should handle catchable 11 without assuming a rejected rebind changed
the prior binding. On resolved supported targets, validation precedes binding
insert/replacement, ordinal allocation and success events. Existing invalid
source/target lookup and evaluation precedence are not newly specified.
No user data, external event service, decompiled source or VM maintenance used.

Misuse severity medium: assuming acceptance or after/before order can route
callbacks incorrectly; treating native absence as removal would break the
intentional routine extension. HZ-runtime-crash-01/HZ-data-corruption-01 link
the numeric boundary, while domain-specific/integrator safety assessment and
independent verification remain external obligations.

DV-BINDEVENT-FLAGS-001: Codex acting in the owner-authorized maintainer workflow
completed development self-review of the native fixture/constants, exact
integer/no-cast-before-admission paths, both flag dispatches, ordinal ordering,
unchanged window-message branch and preserved coercions. This is not an
independent or human sign-off. Completed automated VR evidence above supports
the bounded medium-misuse documentation delta.
DV-BINDEVENT-FLAGS-002: walkthrough establishes initial after-handler, attempts
each rebind, catches 11 without changing it, checks simple/raised invocation,
removes exactly one binding and restores mode/cursor; original and fixed
outputs independently validate error versus acceptance. Success-event counts
show rejected registration is not reported successful.
DV-BINDEVENT-FLAGS-003: rollback walkthrough uses a reviewed revert of only
this slice, reruns named regressions and republishes the boundary notice; no
destructive reset of unrelated changes. PR/changelog notification must retain
#7070 and the extension distinction. Hosted CI and clean exact-head Claude or
authorized Codex review remain merge gates, not inferred from local passes.

## Reproduction

Build the named test targets with GCC15 Debug (-O0 -g1), or Clang21 Debug
with -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all
-fno-omit-frame-pointer. Scratch builds:
 /home/rich/temp/copperfin-bindevent-flags-5611-build
 /home/rich/temp/copperfin-bindevent-flags-5611-sanitize
Set TMPDIR to a fresh mktemp -d directory inside the chosen owned build.
Run ctest -j1 with the anchored names from VR-003; use
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 for Clang.
Do not overlap runtime execution across builds. Original reproduction uses
the pinned base dispatcher with these tests/helper linked but not dispatched,
without overwriting unrelated changes. The unchanged /home/rich/bin/vfp9-probe
wrapper executes probe.prg serially; three complete output runs match.
Generated FXP and scratch builds move to recoverable trash after merge;
tracked probe/audit evidence and all unrelated files/stashes remain preserved.

The exact anchored GCC selection is:
```sh
ctest --test-dir /home/rich/temp/copperfin-bindevent-flags-5611-build -j 1 \
  -R '^test_prg_engine_(bindevent_flags_numeric|numeric_behavior|string_math_functions|null_builtins|native_lifecycle_events|native_focus_move_events)$' \
  --output-on-failure
```
The Clang selection uses the sanitizer build with the same regex except
numeric_behavior is omitted; it is five suites, not full older Numeric.
Post-documentation contracts pass 6/6 in 4.28s; 171 fragments validate.
