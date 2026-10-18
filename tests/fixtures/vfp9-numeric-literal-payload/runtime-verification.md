# Producer hookup staged; runtime verification deferred

`RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001`, #7206, relates to admitted
#5611/#6776/#7189. This is an **unfinished development checkpoint**, not runtime
acceptance, a green PR, or whole-Numeric/division/ListItem completion.

Chronology: native-only public freeze e2e59d020 -> actual qualified ORIGINAL
f530857ff -> unused helper/expanded registration freeze b7fb2b23f -> one include
and one successful `parse_number` payload connection. See [contract](contract.md),
[actual ORIGINAL](original-verification.md), [unused support](unused-support.md).
No frozen public/helper assertion, native expected row, contract or registration
was changed for the hookup. The existing shared invariant parser remains intact.
Metadata, grammar/error paths, Currency/exact64 and other producers are unchanged.

`plain_decimal_literal_payload(literal).value_or(*parsed)` is called only after
the old successful invariant parse. Unsupported shapes/environments retain the
old parsed payload; these are mandatory recovery obligations, not parity claims.
Display metadata never truncates the source digits. The exact intentional two-
file hook diff and unchanged expanded-input checks are retained in
`runtime-deferred-evidence/`. Checking the entire pre-hook manifest reports the
TWO intended producer-file differences; excluding exactly those two files
verifies all ten remaining frozen inputs. A separate diff of the frozen public/
helper/header/contract/registrations is empty. These checks do not prove SAME.

## Actual configured work and resource stop

Audit and DAW both cleared one low-priority GNU Debug `-O0 -g0` Ninja two-job
focused build plus selected tests. CMake configure0. Initial target selection
failed1 before compiling because two requested names did not exist; the exact
diagnostic is retained. Correct target names were then read from CMake and a
focused eighteen-target build began. This is not a product compile failure.

While it ran, the configured sanitizer-selector and native-isolation contract
tests actually executed: CTest0, two unique passed/no failures/errors/skips.
The new public SAME and the broader runtime tests have **NOT** run.

Full I/O PSI avg10 rose from0.00% to16.72%, then22.72%/23.64%; audit independently
confirmed sustained contention and the agreed stop condition. Verified exact
owned Ninja PID443577, parent443576 and build-directory cwd, then sent SIGINT
ONLY to that Ninja. It exited130 (`interrupted by user`) after125/230 scheduled
build steps; owned compiler/Ninja/CMake processes exited. Partial100MiB build
tree remains reusable at `build/literal-payload-fixed/`. No external process,
browser, backup/upload or VM was changed. Both chats received window-release/
deferral notices. Pressure is resource evidence, not an attributed root cause.

Configuration/build/stdout/stderr/statuses, actual tooling JUnit/logs, I/O
samples, source hashes and frozen-input checks are retained losslessly under
`runtime-deferred-evidence/`; host originals are under
`build/literal-payload-native/fixed-public/`. No runtime build-success claim.

## Exact next action

Coordinate a fresh low-contention window with BOTH audit and DAW before resuming
the same `build/literal-payload-fixed` eighteen-target build (two low-priority
jobs). Preserve this interrupted attempt's receipts before any new logs. Then
execute the unchanged public SAME/helper tests and prior Numeric/metadata/
assignment/division/Currency/exact64/macro/control-flow coverage. Keep all1,193
native rows and every control/assertion unchanged; diagnose real failures.
Only after actual SAME/broader success, open the bounded PR, request Claude
first, require clean exact-head review, actual named hosted execution, strict
green checks and resolved conversations. No approval bypass or PR yet.

`DQ-DV-7206-DECIMAL-PAYLOAD-CONTRACT-001` remains MEDIUM. Development self-review
walkthrough confirms pre-code evidence/freeze, one scoped hook, no altered oracle,
and graceful resource deferral; not independent-human/system/release assurance.
Rollback removes the single connection coherently while preserving all native/
ORIGINAL evidence. Parent hazards/checked limits/mode policy remain unchanged.
Retained non-whole division re-entry follows this prerequisite's integration,
not this unfinished checkpoint; its578-row header/574 exact assertions remain.
