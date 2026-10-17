# Runtime division metadata intervention checkpoint

2026-10-17; RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001;
VR-5611-NUMERIC-DIVISION-RUNTIME-GRID-001. Admitted owner-authored, open,
`agent-approved` #5611/#6776/#7189 and direct owner resumption govern this
necessary prerequisite. Baseline main b37ddf7241af01fe7d6b1a5f5e41131642591972.
The retained Integer and real-assignment foundations are integrated; they are
not restarted. This checkpoint changes no Copperfin production code or tests.

## Observed scope and controls

`runtime-grid.prg` contains an explicitly enumerated 9-by-9 operand matrix.
Every numerator has payload 5, every divisor 2, and each result has exact
binary64 2.5 (`4004000000000000`). The first four variants are native Integer
with widths 1,4,10,25. The remaining five are real Numeric with W/D pairs
4/2,13/2,25/0,25/14,40/18. Published-interface clone/width/decimal interventions
force runtime evaluation; they are not a source of Copperfin implementation.
Actual left/right descriptors are recorded for every result. The standalone
auditor checks their declared types, widths, relevant scales and payloads;
it does not infer Integer identity from a double. Integer's unused decimal
and real slots are retained but not interpreted.

The 81 expressions run at DECIMALS 0,2,6,14,18, FIXED OFF/ON and direct compiled
PRG/EVALUATE: 1,620 values per invocation. Two invocations actually return 0,
empty stderr, all 1,620 values, no caught errors, exact raw repeat and zero
compiled-versus-EVALUATE differences. `audit-runtime-grid.cjs` independently
checks framing, unique/complete coverage, source-to-compiled expressions,
operand controls, result fields, display bytes and repeat identity. It imports
no Copperfin helper/runtime. Both raw SHA-256 values are
`b6e15c11840fa090aaeee3c5f3390aebbf6740da0e8c750534156491e50864b0`.
The separately retained pilot has no operand columns; it is not substituted
for the qualifying input-verified pair.

## Bounded recovered candidate, not a universal algorithm

For these positive 5/2 runtime interventions, let S be SET DECIMALS, WL/WR
the observed widths and DL/DR the observed scales (zero for Integer; ignore
its unused native decimal slot). Every captured result agrees with:

    D = min(18, max(S + 2, DL, DR))
    W = min(40, WL + WR - DL - DR + D - point)
    point = 1 when DL > 0 or DR > 0, otherwise 0

An initially plausible alternative subtracting the point merely because
either storage type is Numeric has 64 counterexamples per run. In particular,
real Numeric with D0 must not be equated with a decimal-point contribution.
Those rejected-candidate mismatches are reproduced and counted by the auditor;
they are not silently discarded. The corrected candidate matches 1,620/1,620
rows in each capture, including width40 and scale18 saturation observations.
FIXED changes observed display, not the recovered descriptor candidate here.

This is **not** evidence for arbitrary payloads, signs, negative zero, magnitude,
nonfinite/zero-divisor handling, all intermediate scales, malformed metadata,
constant folding, runtime/constant subexpression boundaries or nested arithmetic.
Earlier constant-only counterexamples remain controlling; the runtime candidate
must not be wired indiscriminately into `parse_multiplicative`. Next recover
payload/sign and evaluation-context controls, then freeze a narrow normalized
contract and independent ORIGINAL-fail tests before any arithmetic hook. No
epsilon, rounded canonical-double shortcut, general division/display/conversion
parity, ListItem read completion or parent issue closure is claimed.

## Native-only diagnostic and attempt chronology

The old recorded Trash path for the host-only FLL no longer exists. The private
runner and installed VFP executable hashes still equal prior provenance. The
retained first-party `../vfp9-listitem-column-numeric/cause-probe.c` was rebuilt
against the owner's existing published-interface support library. No SDK header,
library, object, import library or linked FLL is redistributed here or linked
into Copperfin. No VM was started, recreated, reset or installed; no backup or
foreign process was modified. Current host binaries stay under the slice's
`build/division-recovery-native/diagnostic/`; hashes are in the capture folder.

Actual failed rebuild attempts are retained: initial `rust-lld -flavor lib`
is unsupported (both import statuses1); an overlapping first link ran before
inputs existed (status1); the next link used an import library without the
required x86 stdcall decoration (status1). `llvm-dlltool-21 -m i386 -k` produces
the kernel32 stdcall import library; final link0 uses the existing Rust LLD
link flavor, `/dll /machine:x86 /entry:CauseEntry@12 /nodefaultlib`, own object,
installed `winapims.lib`, kernel32 stdcall and msvcrt imports. Existing SDK
debug-section warnings are retained, not a warning-free claim. Compile0,
final link0 and both complete native capture statuses0 precede this checkpoint.
The native runner is unchanged, including its existing scoped25-second limit.

Initial staged whitespace check returned2 for final blank lines in the two
generated native PRG sources. The exact source/FXP/capture chronology is kept;
narrow fixture attributes exempt only those native-source blank EOFs. No
production code, runtime test, assertion, native source or raw capture changed.

Re-audit retained captures without VFP:

    node tests/fixtures/vfp9-numeric-division-context/audit-runtime-grid.cjs \
      tests/fixtures/vfp9-numeric-division-context/runtime-captures

## DQ/DV, misuse and rollback

LOW DQ-DV-5611-NUMERIC-DIVISION-RUNTIME-GRID-001 procedural delta: retained
648 mixed-context observations -> controlled runtime metadata interventions ->
capture actual operands as well as results -> standalone coverage/type/repeat
and rejected-candidate audit -> retain bounded observations and open boundaries.
Misuse: treating this fixed-payload equation as a universal compiler/runtime
rule could create incorrect Numeric propagation or unsafe downstream selectors.
Mitigation: explicit non-acceptance of production/general payloads, frozen raw
counterexamples, actual input controls and mandatory later ORIGINAL-fail tests.
Existing HZ-data-corruption-01, HZ-runtime-crash-01 and HZ-doc-command-01 apply.
Checked conversion/allocation limits and #6776 per-area compatibility policy
remain unchanged. Walkthrough: the actual auditor0 validates both qualified
captures and64 rejected-candidate mismatches without importing implementation.
Review is implementing-agent development self-review and automated cross-check,
not independent-human, system or release qualification. Rollback removes this
recovery-only increment while retaining prior captures and the unfinished owner
objective. No runtime field notification is needed because no runtime changed.
