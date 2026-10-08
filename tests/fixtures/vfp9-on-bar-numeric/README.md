# ON BAR Numeric-literal identifiers (#5611/#6776/#5868)

RQ-CF-PRG-ON-BAR-NUMERIC-001 derives from owner-admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and independent installed VFP9 7423
observations. Mapped BEFORE changing the selected production conversion.
Only on_bar_activate_popup_command's parsed Numeric-literal identifier is
selected, not ON SELECTION, skip/mark/query, parser/type/expression admission,
positive missing-bar/target lookup, reserved/system-menu support or lifecycle.

- COPPERFIN: finite truncation toward zero, converted1..INT32_MAX; otherwise
  localized catchable167 before activation-target map insertion/replacement.
- VFP9: finite raw>=2^32 saturates INT32_MAX; lower operands use defined
  signed-low32 conversion. Positive results bind the corresponding key.
  Converted-1/-2 produce localized catchable1612 without binding a negative
  key in this supported user-popup lane; other nonpositive results reject167.
  This is NOT native system-menu/reserved-bar implementation or a claim to
  have repaired general positive missing-bar/target admission.
- Ordinary fractions truncate in BOTH modes. Negative wide aliases1/2 exist
  only in VFP9 mode; positive wide values bindMAX, not small aliases.
- The checked helper rejects nonfinite/NaN in BOTH modes by owner-derived
  containment policy, not a native nonfinite claim. The existing literal
  parser does NOT admit NaN/infinity/overflowed exponents; its unparsed return
  path is unchanged. Direct helper checks do not claim catchable167 for those
  unparsed spellings or fix broader parser/type admission under #6227.
  Invalid admitted Numeric operands raise167 (VFP9 -1/-2 raises1612).
  Preserve session/mode/cursor, existing bindings,
  prompts/count, event routing/precedence and cleanup on rejected conversion.

Native probe.prg creates RELATIVE parent/child popups, only ONE parent bar at
a time (1,2,MAX), one child bar, and NEVER activates either popup. This bounds
state at two bars and distinguishes conversion identities by independent
existing-bar admission versus error1612 for other valid identifiers. Native
converted0/<-2 rejects167, converted-1/-2 raises1612 in these user popups.
Source 30b3500cb00f6f590f5f2f92a008a12f4dfc4dc6d8fee743f613a79fba64abce;
complete125-line/40-operand-by-three-seeds output
8127255a56e8f67bd58d3316caf182ffe32840c16318722251b8131f2b0417ec.
Three final unchanged serial runs completed with matching source/output hashes.
Initial harness recovery attempts failed107 (VERSION(5) was Numeric) and36
(oversized single string literal); they are NOT passing evidence. Fixed source
uses VERSION() and split value strings. No timeout, GUI/VM/template/system/
backend/credential/environment maintenance or huge ordinal allocation.
Allowed shipped help ON BAR topic3eecfefb-7685-4d43-8cec-6812216bd05e states
item-to-submenu assignment. ON() topic421c6247-80e9-478a-a7a3-2f2bd4d816c0
only queries ERROR/ESCAPE/KEY/PAGE, NOT ON BAR; no invented query oracle.
The #5868 expectation to reject ordinary fractions is contradicted by native
observations and is not a requirement source. Its other consumers stay open.

VR-5611-ON-BAR-NUMERIC-001:70 independent direct constants and58 fresh both-mode
guarded PRG sessions; select seeded bars through the existing headless host
interface and check activation telemetry, not a correlated numeric query.
16 four-locale ERROR/AERROR/message cases, four opposite-origin-policy session2
cases and two old popup/menu neighbors. Explicit callback cleanup snapshots
are PUBLIC, both session policies reset, then test-owned sessions cancelled/
reaped. Callback return/event-loop termination is separate #4630 work.
VR-5611-ON-BAR-NUMERIC-002: original byte-identical dispatch negative evidence,
fixed GCC/Clang ASan/UBSan/float-cast-overflow, older Numeric/localization and
repository contracts. Final byte-identical original dispatcher fails128
selected assertions per compiler, zero unrelated failures/no diagnostics;
70 helper checks and both old neighbors pass. See baseline-audit.txt for
source/log hashes, reconstruction and excluded preliminary fixture failures.
Isolated checked dispatch/localized167/1612 is implemented and verified;
this bounded requirement is defined. Final GCC Debug11/11 passed472.21s:
two focused targets, older Numeric/localization and seven repository contracts.
Final Clang ASan/UBSan/float-cast-overflow focused2/2 passed1.20s with no
diagnostics (detect_leaks=0; not leak qualification). All runtime tests were
serial with private TMPDIR. Final fixed log SHA256:
GCC8cbf59fc721b21ad71ed45b4bccec78bea4c1a9e518cad7d16a24e22e369404a;
Clangea8abfa0f3c986d6a45b39f06df4184e063afd4af88562d7fb2db8919aef2490.
Fixed dispatcher SHA256c1d89abed48fae0eb68f9f611b3310df7b65bbf895fbef49bfaae59287dcda43;
helper/test hashes remain those retained in baseline-audit.txt. Changelog
assembler separately validates179 fragments after adding this slice's entry.
General positive missing-bar/target admission remains
unfinished under #6227; fresh native evidence comment6058963784 binds that
limitation to current main, not a new scope expansion or closure.

HZ-runtime-crash-01: checked conversion before any cast/llround.
HZ-data-corruption-01: invalid conversion cannot mutate bindings; seed1/2/MAX
guards distinguish erroneous rounded/aliased/saturated routing.
HZ-doc-command-01: explicit modes, bounds, nonfinite and unsupported lookup/
system-menu boundaries. Misuse severity medium. DQ-5611-ON-BAR-NUMERIC-001
requires these distinctions; DV-5611-ON-BAR-NUMERIC-001 completed development
maintainer self-review under owner authorization, with the automated results
above. This is NOT independent human review. Walkthrough:0.5 and2^31 default
reject167 without changing guarded routes;1.9 binds1, not2;4294967297 default
rejects but VFP9 bindsMAX, never1. Negative-wide1/2 aliases are legacy-only;
legacy converted-1/-2 rejects1612 while seeded targets survive. Opposite-origin
session2 tests prove selected-session policy; locale ERROR/AERROR/ErrorNo agree.
The callback releases popups/closes the cursor/resets both policies, then the
test explicitly cancels/reaps its waiting session without claiming #4630 fixed.
Self-review traced requirement-to-helper-to-one-dispatch-site-to-independent
constants/host telemetry, checked native sparse/noactivation bounds, unchanged
unparsed-token behavior, and the rollback/field-notice boundaries below.
Procedural delta: replace one llround map-key site;
no other command migration. Rollback: revert isolated helper/dispatch/tests/
docs; field notice names truncation, default167 and opt-in aliases/MAX/1612.
No independent-human/full-native-type/GUI/system-menu/leak/release claim.
