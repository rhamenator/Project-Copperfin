# ON PAGE Configuration Traceability

## Scope

This low-reach runtime slice records `RQ-CF-PRG-004`, recovered from the
installed VFP9 `ON PAGE Command` help topic
`html/e5f99c20-85da-4b5d-b2be-031d41bd0f90.htm` and the `ON()` Function topic
`html/421c6247-80e9-478a-a7a3-2f2bd4d816c0.htm`.

## Derived Requirement And Boundary

`DQ-prg-on-page-001`: `ON PAGE AT LINE <nLineNumber> <command>` shall retain
the command and line expression, `ON('PAGE')` shall return that exact command,
and bare `ON PAGE` shall clear it. Assignment and querying shall not execute
the command.

`DQ-prg-on-page-002`: a headless `EJECT PAGE` shall execute one retained static
non-macro `ON PAGE` command through the existing bounded iterative frame path
and record deterministic event evidence. A cleared, macro-backed, malformed,
or unavailable action must not partially execute.

The installed documentation says execution also occurs when `_PLINENO` exceeds
the configured line. Copperfin deliberately implements only the `EJECT PAGE`
handler trigger: it has no printer/alternate-file output, `_PADVANCE`,
`_PLENGTH`, `_PLINENO`, `_PAGENO`, report/label pagination, or line-threshold
dispatch. It must not substitute output-row count for the VFP page model.

## Hazard, Misuse, And Rollback

`HZ-system-failure-01` applies proportionally: treating configuration as a
page event could cause unexpected program execution. The action is admitted
only as a static non-macro source-program command, parsed into an owned routine,
and executed through the existing call-depth guard and iterative frame path.

Rollback is coherent: remove the parser/dispatcher/runtime-surface state, the
dedicated test, this report, and the corresponding matrix, coverage, handoff,
and changelog entries together. Existing report output behavior is unchanged.

## Verification

`DV-prg-on-page-001` and `DV-prg-on-page-002` use the dedicated portable CTest target
`test_prg_engine_on_page`. It verifies parenthesized compound line-expression
capture, exact command readback, EJECT-triggered static-command dispatch, bare
clear, and macro-action non-dispatch. Its
machine-readable isolation is portable, parallel-safe, test-owned filesystem
only, and has no environment, child-process, network, or sample dependency.

This is development-assurance evidence only; it is not a claim of formal
DO-178C compliance, certification, safety-critical suitability, or report
pagination completion.
