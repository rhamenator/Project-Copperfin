# Unary Numeric recovery: observed, not implemented

RQ-CF-PRG-NUMERIC-UNARY-METADATA-001 /
VR-5611-NUMERIC-UNARY-METADATA-001 / DQ-DV-5611-NUMERIC-UNARY-METADATA-001.
Parent: owner Numeric compatibility decision, RQ-CF-PRG-NUMERIC-METADATA-001,
admitted #5611/#6776. This checkpoint changes no production code and closes
neither the unary slice nor #7189 nor the retained ListItem column read.

## Evidence and failed attempts

Installed Visual FoxPro09.00.0000.7423, existing private Wine runner, no VM.
The first40-expression program wrote `build/numeric-unary-native/results.txt`,
but the runner was incorrectly given `first-results.txt`: actual exit1 and
missing-result diagnostic. The second invocation requested `repeat-results.txt`
and exited124 at the runner's unchanged25-second limit; its timeout cause is
not established. A raw160-case results file existed, but is retained only as
`failed-attempts/wrong-result-argument-unqualified.txt`, not acceptance evidence.
Failed stdout/stderr/status are retained alongside it. No assertion/timeout
weakening, VM start, environment repair, SDK copying or proprietary inspection.

Correct invocations use `--result build/numeric-unary-native/results.txt` from
the dedicated unary worktree. Each successful pair has actual0, empty stderr,
stdout equal to the result file, complete frames, unique expected cases and
matching documented type-aware fields. Four settings per expression:
DECIMALS2/14 crossed with FIXED OFF/ON. Default display and assigned descriptors
are captured separately, never inferred from the direct descriptor.

| Captures | Expressions | Values / native errors per run | Byte-identical raw SHA256 |
| --- | --- | --- | --- |
| captures-160 |40|148 /12|e1465ad341841c1bbfb1fd3bdbcc67a231475f96354e6db918a08b126073744c|
| captures-192 |48|180 /12|c7f2db161b4e122742968f8422ced0c5e3a1b86f296fd74b94c9790b74924b75|
| captures-240 |60|228 /12|8dccd0b4ffb9266898baf24885d84df2963b27b3c63b6820fd680493522bdb76|

`audit-unary.cjs` independently enumerates the source expressions/settings and
checks the framing, count, identity, type, payload bits and repeat. Unused
Integer real/decimal slots and Numeric integer slots are NOT interpreted.
The three saved audit exit statuses are0. Error cases remain observations,
not discarded rows or authorization to change Copperfin syntax.

## Recovered examples and unresolved semantics

Values below are direct descriptors, not assigned width. `D` means associated
decimal places; hexadecimal bits distinguish zero signs even when display does
not. Examples occur in every captured setting unless explicitly qualified.

| Expression | Direct result |
| --- | --- |
| `1.00` | Numeric W4 D2, positive1 |
| `-1.00`, `- 1.00`, `-(1.00)` | Numeric W5 D2, negative1 |
| `+1.00`, `+ (1.00)` | Numeric W4 D2, positive1 |
| `-(-1.00)`, `-(-(-1.00))` | Numeric W6/W7 D2, positive/negative1 |
| minus applied to `1.` followed by38zeros | Numeric W41 D18, not width-capped40 |
| nested minus of that negative operand | Numeric W42 D18 |
| `-nSource`, `-(-nSource)` where `nSource=1.00` | Numeric W14/W15 D2, unlike the literal's W5/W6 |
| `-0.00`, `-(0.00)`, `-(-0.00)`, `-(-(-0.00))` | negative-zero bits8000000000000000; widths5/5/6/7, D2 |
| `-nZero`, `-(-nZero)` where `nZero=0.00` | positive-zero bits0000000000000000; widths14/15, D2 |
| `-nMinusZero`, `-(-nMinusZero)` where `nMinusZero=-0.00` | negative-zero bits8000000000000000; widths14/15, D2 |
| `-(0.00-0.00)` versus `-(nZero-nZero)` | negative-zero W7 versus positive-zero W15, both D2 |
| `-0000000000.00` versus `-nZero` | same W14 D2 but negative-zero versus positive-zero |

These counterexamples rule out a universal IEEE sign toggle, a universal
zero-sign-preserving rule, and descriptor width alone as sufficient context.
A compile-time/constant-expression distinction is a hypothesis, not recovered
VFP implementation knowledge or permission to invent a provenance flag.
Repeated signs without grouping report1231 (`--1.00`, `- -1.00`) or152
(`++1.00`). This checkpoint has not established a new Copperfin syntax defect
through its public interface and does not change its parser grammar.

Controls also show `-5/2` at DECIMALS2 has W5 D2, but `-(5/2)` has W2 D0,
both payload-2.5; the latter default TRANSFORM is `-3`. Scientific and Integer
controls, assignment-width normalization and display/constant folding require
their own recovered contracts. Metadata is not merely a display label, and
unchanged raw double does not imply equivalent downstream behavior.

## Architecture and next implementation action

Current `src/runtime/prg_engine_expression.inl::parse_unary()` returns the
operand unchanged for plus, but uses `make_number_value(-value_as_number(...))`
for ordinary Numeric minus. That drops attached metadata and applies ordinary
IEEE negation. This code is evidence of the migration point, not a requirement.
The existing optional value-owned descriptor can carry the recovered widths,
but does not encode expression context or assignment normalization. Merely
copying it, globally clamping width40, or guessing an epsilon establishes no
parity. Exact integers/Currency/NULL/type checks, suppressed evaluation, switch
policy and checked conversion/allocation limits remain untouched.

Next: separate lexical signed-literal creation from evaluated unary behavior
with bounded native controls for returned/conditional/function-created zero,
constant versus variable arithmetic, and direct compiled versus EVALUATE
execution. Recover a testable contract for that boundary before choosing a
normalized architecture. Then freeze independent expected tests and establish
original-fail evidence before connecting production code; full unary and
consumer acceptance remain open. No request for another owner policy decision.

## Provenance and documentation verification

Clean-room input is installed behavior through the existing first-party
published-interface diagnostic; no proprietary implementation was read.
Host dependencies (not added to source/product):

- `/home/rich/bin/vfp9-probe` SHA256
  1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06.
- `/home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe` SHA256
  cdc159a1e07e5208a0def267a94b8b3b007160641b442cb453aba8ecc4138560.
- Published-interface `cause-probe.fll`, host-only path in README, SHA256
  ebb34d4e4f12c378e7ffd521d66915a249b45995535e6d37513deb711d5264a3.

Own generated FXPs are probe artifacts only. `SHA256SUMS` covers retained
source/raw/status/audit artifacts. DQ/DV procedural delta, LOW misuse,
walkthrough/self-review, rollback and unchanged hazard linkage are in README.
This is implementing-agent development verification, not independent-human
assurance, full Numeric acceptance, system qualification or release clearance.
