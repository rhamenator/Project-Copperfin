# Non-whole ORIGINAL result and upstream prerequisite

`RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001`, admitted
#5611/#6776/#7189; [frozen recovery contract](nonwhole-recovery.md).
Signed/DCO checkpoint `274a38635a463db154b4b9e0c78c502d6722c53c` froze
the actual native inputs, public source/header and registrations **before** this
run. Production `src/` and `include/` are byte-identical to current main
`ef3ea326f78d640546363abe97bbe27b6208bf45`; no runtime hook is present.

## Actual execution

The owner explicitly requested coordination with the audit and DAW chats before
heavy builds/VMs. Both chats cleared this one low-priority GNU Debug `-O0 -g0`
Ninja `--parallel 2` focused build. DAW held heavy/VM work. Configure and build
returned0; worker completed at2026-10-18T02:28:37Z. Sampled full I/O pressure
avg10 stayed near zero (start0.14%, end0.00%), ample memory, no VM started or
used. Both chats were notified that the heavy-work window was released.
This is not backup/upload qualification or permission to overlap another build.

CTest ran the new public regression and the two registration/isolation contracts:
three unique actually executed tests, two passed, one expected ORIGINAL failure,
no errors/skips, elapsed2.09s. CTest returned8; the public program returned1.
Independent XML inspection confirms exactly the named public test failed.
`audit-nonwhole-original.cjs` diagnoses the full verbose log independently of
product helpers or the post-capture metadata candidate:

| Category | Actual failing assertions |
| --- | ---: |
| Descriptor | 11,040 |
| Payload bits | 2,280 |
| Of payload bits: standalone literals | 40 |
| Evaluation | 0 |
| Safety/debugger/boundary controls | 0 |
| Total (literal bits are a subset) | 13,320 |

These are repeated assertions over contexts, **not** thousands of separate
defects. All frozen pre-hook source/header/contract/registration hashes verify0.
Actual logs, statuses, JUnit, audit summaries, production-diff receipt and binary
identities are retained in `nonwhole-original-evidence/`. Full raw verbose log is
preserved even where an XML producer limits inline failed-test text. The unused
initial binary-hash receipt named nonexistent `libcf_platform.a` and failed1;
its partial receipt remains host-only. The corrected receipt uses the actual
Ninja-linked `libcf_platform_support.a`, hash command0. No build/test was rerun
or changed to correct that receipt path.

## Cause established at the producer boundary

The bare literals (no division or other arithmetic) fail in **all20** public
contexts: both NUMERICBEHAVIOR modes, all five DECIMALS and both FIXED settings.

| Literal | Copperfin actual | Native actual |
| --- | --- | --- |
| `1.234567890123456789` | `3FF3C0CA428C59FB` | `3FF3C0CA428C59FC` |
| `0.9999999999999999` | `3FEFFFFFFFFFFFFF` | `3FF0000000000000` |

The standalone descriptors pass; adding or repairing division metadata cannot
repair these payloads. Native records are repeated and independently cross-check
FLL typed bits against BINTOC, compiled versus EVAL, and settings. The current
source path is `ExpressionParser::parse_number()` ->
`platform::try_parse_invariant_double()` -> host `std::from_chars`; literal
metadata attachment does not change the returned payload. Thus the actual
source-literal conversion policy already diverges upstream of division.
The exact VFP conversion algorithm remains **unrecovered**; do not infer a
15/16-digit rule, tolerance, arbitrary decimal rounding or spelling exception.

The independent diagnostic counterfactual uses the manifest's exact trees and
standard binary64 literal parsing, not Copperfin code. It accounts for all2,280
recorded bit failures:112 core rows x20 contexts, plus two bare literals x20.
Each recorded actual payload matches that counterfactual; each expected payload
matches actual native evidence. The native-input arithmetic candidate matched
all10,880 core observations earlier. Together these localize the observed
payload discrepancy to the inputs, without proving a general division or
decimal-conversion algorithm. The 11,040 descriptor failures are a separate,
still-unimplemented division metadata gap. This is not direct ListItem acceptance.

## Explicit deferral and re-entry

Do not widen the active division hook or mask/exclude the two literals.
Defer non-whole integration on a **separate bounded decimal-literal payload
recovery prerequisite**, [focused issue #7206](https://github.com/rhamenator/Project-Copperfin/issues/7206),
derived from the admitted Numeric workstream and filed against the exact main
revision above. No approval label was added; authority remains with the admitted
parents and direct owner policy. Start its own evidence/
contract/source freeze from main; do not alter the shared invariant parser used
by unrelated persisted data. Native source conversion, width/scale/storage and
propagation must be recovered together where evidence requires them.

Re-enter this retained division slice after that prerequisite has native-backed
public acceptance and is integrated. Keep the 578-row native header and exact
574-row assertions unchanged; refresh ORIGINAL on the new main. Then complete
non-whole callback/continuation timing recovery, replace the older constant
test's three now-admitted unknown assertions with stronger native expectations
**before** its next source freeze, integrate only the bounded division rule,
and require SAME pass plus broader/hosted named execution, exact-head clean
Claude/Codex review, strict checks and resolved conversations before merge.
Small divisors, wider producers, other operators, display/exact conversion and
ListItem supplied-column reads remain mandatory, not residual acceptance.

Documentation assurance delta `DQ-DV-5611-NONWHOLE-RECOVERY-001` remains LOW:
this note records actual failure and prevents a misleading parity claim; no
operational VM/backup procedure or shipped runtime changed. Walkthrough and
development self-review compare bare-literal controls, independently diagnosed
descendant failures, frozen hashes and explicit re-entry. Rollback retains raw
evidence and reopens the gap, not altered expectations. Parent
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 and #6776 checked
conversion/allocation/mode limits are unchanged. No owner decision is needed
to continue the admitted prerequisite; the automation need not pause.
