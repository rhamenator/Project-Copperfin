# SLEEP explicit-duration numeric boundary (#5611/#6776)

RQ-CF-PRG-SLEEP-DURATION-NUMERIC-001 derives from admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, direct owner extension intent and
docs/25-engine-concurrency-policy.md. The extension remains available in both
modes. No native low32 alias is invented for a command lacking a usable native
equivalent. This is duration admission only, not scheduler, cancellation,
critical-section, wall-clock accuracy or task-lifecycle redesign.

Requirements mapped before dispatcher changes:

- Explicit duration is milliseconds; retain ordinary half-away quantization.
- Reject raw negative/nonfinite/NaN and rounded values outside the intersection
  of nonnegative signed64 and size_t BEFORE an integral cast, yield/wait,
  blocking-policy event or successful runtime.sleep event. Catchable localized
  error11 is a derived extension contract, not a native error-number claim.
- Exact int64/uint64 compare without first rounding through double. Safe other
  currently admitted coercions retain checked half-away behavior.
- Both NUMERICBEHAVIOR modes share defined extension policy; no VFP9 wrap.
- Preserve absent-duration scheduler_yield_sleep_ms, zero cooperative yield,
  one-time resumed expression evaluation, current-session selection, cancellation
  checkpoint and positive-duration critical-section guard after admission.
- Domain admission is not a maximum-duration/service-latency guarantee: a very
  long representable wait is still intentional and must remain cancelable.

Code: checked_sleep_duration_argument in helpers.h/.cpp; only explicit
sleep_command conversion in dispatch.inl; existing SleepInvalidDuration key.
VR-5611-SLEEP-NUMERIC-001: independent helper boundaries/exact values;
34 independent direct checks; 68 fresh guarded PRG operands in both modes,
eight four-locale metadata cases, two default/zero-resumed cases and four
additional valid/invalid resumed data-session callbacks; deterministic critical-section
containment for large admitted operands so no huge physical waits; errors,
metadata, state/event ordering, default and continuation controls.
VR-5611-SLEEP-NUMERIC-002: original-dispatch negative execution, fixed GCC/
Clang ASan/UBSan/float-cast-overflow, old sleep/critical/cancellation neighbors,
older Numeric/localization and repository contracts. Clang focused2/2 21.53s
passes ASan/UBSan/float-cast-overflow (detect_leaks0, halt_on_error1), no
diagnostics. Fixed GCC4/4 263.77s passes: new1.09s/neighbors0.70s/full older
Numeric245.99s/localization15.97s. Seven repository contracts pass7/7,380.80s
(safety workflow376.17s): locale installation, agent intake, DCO, changelog
assembler/177 valid fragments, native isolation and safety traceability.

Installed VFP9 7423 fixture uses only zero-duration attempts and recognized
INT(1.9)==1/session1 controls. Both EXECSCRIPT SLEEP0 forms raise10 without
continuation; those errors provide NO duration/type/rounding/upper-limit
oracle. No unsafe huge native wait, task/OS configuration, VM or backend use.
Retain complete repeated source/output; do not claim full native absence from
two rejected spellings alone. Governing extension policy is owner-derived.

Three identical five-line installed runs were completed before dispatcher
changes. Source SHA-256:
599a68008ff4500181bbe6916362e6ec6ae0090862297d6adbeb16a5a1e5f2b3.
Output SHA-256:
e705d7c713670fa84c8be317c55b14e9469e8b55310997eadab37245b9e6093b.

HZ-runtime-crash-01: checked floating/integral conversion.
HZ-data-corruption-01: rejected delay cannot become a spurious zero/small wait
or reach coordination mutation while caller believes its duration was valid.
HZ-doc-command-01: distinguish conversion domain from intentional long waits,
cancelability and critical-section fast failure. Misuse severity medium.
DQ-5611-SLEEP-NUMERIC-001: mode/domain/unit/error/order/extension limits explicit.
DV-5611-SLEEP-NUMERIC-001: maintainer development self-review and walkthrough
with automated verification; not independent human evidence. Completion evidence
is recorded below with the executed focused and broader verification.
Procedural delta: checked admission replaces llround-to-size_t; safe ordinary
rounding and zero/default waiting policy remain unchanged.
Rollback: revert isolated helper/dispatch/test/docs changes. Field notice:
unsafe duration now raises11 before waiting in either mode; opting into VFP9
does not restore invented extension aliases or guarantee bounded latency.
No certification, leak, clock precision, hosted-platform or release claim.

Development maintainer self-review and walkthrough (not independent human
review): reviewed only the explicit-duration dispatch delta, exact integer
branches and finite/rounded bounds against the parent requirements; cancellation,
critical policy, scheduler default and wait loop remain unchanged. Confirmed
all four existing localized catalogs were reused without a new message key.
For both modes the executed focused walkthrough verifies 0.49 produces one
zero-duration success event; 0.5 and 1.5 produce one 1/2ms event outside CRITICAL;
negative and 1E300 reject11 without continuation or a blocking/success event;
an admitted positive duration inside CRITICAL retains existing blocking failure
and no successful sleep event. Cursor/pointer/payload/mode/session/reset remain
intact. Callback-selected session2 remains selected, its mode differs from
origin1, and valid half-away/invalid duration expressions execute exactly once.
Existing zero/default, deep resumability and cooperative task cancellation
controls passed under the focused sanitizer. These are development checks, not
independent human assurance or a timing/service-latency bound. GCC broader
verification above confirms Numeric and four-locale catalog controls. Repository
contracts are additionally run before committing/pushing; PR checks/review and
merge evidence are recorded in GitHub/handoff. No all-platform or independent
release qualification follows from these local checks. Parents #5611/#6776 still contain
unfinished conversion sites.

Fixed source SHA-256: dispatcher
84bb0cf8cc875afd2d46c3d76f23b7f89ed2037a8ae0ed51992bbd41fb5db70b;
helper a8db2990ea956b72e089f41b8a1f903cf4d0b4867e43e739b8bafe1a937088b0;
test 927fa25b36c47a3c7c53f3aaabec83ff5352fe122b8adf72cb7412a60eb8f721.
Clang fixed CTest log SHA-256:
36ecdc9b02fe76ea8d674c88a6cfef9152434ec0063d7b350787f34e00488397.
GCC fixed CTest log SHA-256:
172b8153bcb1d2a840ffa72f332529b610f9e5fb95e2a860146b13f94e87cdaa.
See baseline-audit.txt for unchanged-main failure counts, source/log identities
and the assertion-only pseudo-locale correction; no preliminary failure is
presented as passing verification.
