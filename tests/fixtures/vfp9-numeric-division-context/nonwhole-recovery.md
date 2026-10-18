# Non-whole division recovery checkpoint

Parents: owner-directed #5611/#6776/#7189. Baseline:
`ef3ea326f78d640546363abe97bbe27b6208bf45` (PR7204 merged).
Requirement `RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001` is a
**recovery gap**, not completed compatibility. This checkpoint changes no
production behavior. The public regression and its exact native header must be
frozen before integration. No formula inferred from Copperfin is a requirement.

## Allowed evidence and scope

Installed VFP9 `09.00.0000.7423`, black-box PRG execution plus the previously
retained public-API FLL descriptor probe; no decompilation/disassembly. REA tools
and CLI were unavailable, so these are shell-produced captures, not REA Evidence
IDs. The runner, VFP executable and FLL identities are in
`nonwhole-captures/pre-capture.sha256`. Host raw evidence is also retained in the
slice's `build/nonwhole-native/`. No Windows VM was started or used.

`emit-nonwhole-grid.cjs` emits 68 core trees/eight wrappers, 18 literal controls,
12 runtime mixtures and four explicit outside-scope controls: 578 complete
expressions x five DECIMALS settings (0/2/6/14/18), FIXED OFF/ON and compiled/EVAL
= 11,560 observations/run. Signs, source widths/scales, nested division,
grouped unary operations, IIF and EVALUATE are explicit. Divisor magnitude is
at least one in the selected core trees. No claim covers small divisors, wider
unknown producers, other operators, general macro precision, display migration,
exact conversion or ListItem supplied-column reads. Those obligations remain.

Two raw runs each exited0, empty stderr, 11,560 successful values/no caught
errors; they are byte-identical, SHA-256
`1b523f2923e52e9a4109ffaf2e805708910449edd5a38bfb9036f7f821238e1c`.
`audit-nonwhole-grid.cjs` independently verifies complete framing/inventory,
source bindings, native Integer versus real payload fields, FLL/BINTOC bits,
FPU control word, compiled/EVAL and repeated typed records. It imports no
Copperfin helper. This pair has no unused-slot differences; that is not a
general guarantee about VFP parameter padding.

## Candidate, not an accepted algorithm

`analyze-nonwhole-candidate.cjs` is explicitly a post-capture hypothesis, not an
expected-value generator. With *captured native literal payloads* as its inputs,
the extension of the previous intrinsic width/scale/storage model and sequential
binary64 division agrees with all 10,880 core observations (zero descriptor or
payload differences). It does not model runtime mixtures, single-evaluation
effects or paused continuation. Separately evaluated child division descriptors
are never treated as the descriptor of a folded child in its enclosing tree.
Continuation/timing evidence is still required before accepting any new hook.

Two standalone literals differ from independent Node decimal-to-binary64 parsing:

| Literal | Native actual payload | Node nearest binary64 |
| --- | --- | --- |
| `1.234567890123456789` | `3FF3C0CA428C59FC` | `3FF3C0CA428C59FB` |
| `0.9999999999999999` | `3FF0000000000000` | `3FEFFFFFFFFFFFFF` |

Both native literal records are invariant across all 20 routes and repeat
byte-for-byte. This is a producer discrepancy, not evidence for correcting
quotients or choosing a tolerance. Copperfin's actual `parse_number()` routes
through `platform::try_parse_invariant_double`, which uses `std::from_chars` on
this host; source inspection alone is not an actual product reproduction.
The frozen public test must establish the actual original behavior. Do not
change the shared invariant parser (also used outside Numeric literals), discard
these operands, hardcode their spellings or claim full non-whole parity.
If confirmed, file a focused issue against the baseline and retain a bounded
literal-payload prerequisite separately from division integration.

## Frozen public verification

`emit-nonwhole-public-header.cjs` reconstructs all 578 rows/five settings only
from audited native records. Both discrepant literal controls remain admitted;
574 rows assert exact metadata **and** bits. Four outside-scope rows retain
native expectations but only check the current unknown-metadata boundary, with
no parity claim. `--check` is byte-exact. No candidate/product code generates
expectations. Metadata and bits are independent assertions.

`test_prg_engine_numeric_nonwhole_division.cpp` uses a real paused runtime,
both NUMERICBEHAVIOR modes, five DECIMALS settings and both FIXED settings,
checking all 578 expressions/context, plus NULL/type/zero/Currency/suppression
and debugger-state controls. It reports literal-bit failures separately.
Exclusive owned working/temp paths and trusted source overrides; no network or
children, serial process-environment initialization. CMake/isolation and exact
sanitizer target/selector coverage are frozen with the public source. No product
hook or public ORIGINAL result is claimed yet.

## Diagnostics preserved, not qualifying numerical failures

The initial audit assumed display hex was nonempty and exited1. The retained
scientific outside-scope control `1E-14/2` really has Character display length0
at DECIMALS0/FIXED ON (compiled and EVAL). A separate native VARTYPE/LEN/quoted
TRANSFORM calibration confirms this; the unchanged calibration rerun exited0.
Only the auditor's valid-empty-Character encoding was corrected, with an exact
two-row count assertion; numeric source/expectations/captures were unchanged.
The original auditor, failed status/stderr and successful raw runs are retained.
The first candidate launch read the failed auditor's empty JSON and exited1
before modeling; this is a harness-order diagnostic, not a counterexample.

The first display-calibration wrapper exited1 during cleanup (communicate
timeout then ProcessLookupError), despite a complete 171-byte native result.
Retain that original result/traceback separately. The unchanged-source rerun
returned0/empty stderr and the same four native rows. No timeout or runner
change was made; the first wrapper failure is not qualifying process success.
Raw CRLF/display bytes are never normalized. The initial pre-capture manifest
intentionally binds the initial auditor; its archived version, not the later
empty-display correction, satisfies that old hash. Later manifests bind the
current auditor. Native capture/producer failures must not be hidden by a new
manifest or weakened assertion.

## Traceability and re-entry

Owner Numeric compatibility policy -> native source/captures/auditor -> frozen
native header -> public ORIGINAL -> independent bounded implementation -> SAME
pass -> broader/hosted named execution/exact-head clean review/strict checks.
The last four stages are pending. Numeric metadata/payload loss is a parent
compatibility hazard; checked conversions, finite/allocation limits and per-area
mode policy must remain unchanged. No release, runtime-parity or display claim.

Documentation assurance `DQ-DV-5611-NONWHOLE-RECOVERY-001` (LOW): procedural
delta is evidence interpretation and the pre-code acceptance sequence only,
not guest/host administration. Misuse would mistake a conditional model match
or metadata-only change for payload parity. The walkthrough explicitly follows
literal controls -> actual original public bits -> prerequisite if confirmed ->
unchanged native expectations/SAME verification. Development self-review checked
the native-only emitter, diagnostic preservation and exact selector coverage;
it is not independent-human assurance. Rollback: revert this recovery note/test
checkpoint, preserve raw observations and reopen the gap; no released behavior
has changed. HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 remain
parent links, with no new host operation or weakened limit.

PR7204 acceptance (already completed, not restarted): exact head
`3bcb24d32e50aa3607ce8f6971edeca31d964206`, merge `ef3ea326f`, normal strict11
green checks, all conversations resolved, verified exact-head Codex clean review
after Claude-first request, actual hosted sanitizer run39086065459 with 30
unique executed tests/no failures/errors/skips, both named new tests. Hosted
ASan/UBSan/no-recovery verified; no explicit hosted float-cast-overflow claim.
Acceptance comment6244388045 and canonical `build/pr7204-completed/` retain the
actual review, artifacts, original/SAME and safe-cleanup evidence.
