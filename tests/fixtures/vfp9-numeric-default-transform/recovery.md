# Default Numeric TRANSFORM recovery — #5611/#6776/#7189, tracking #7210

## Pre-observation contract

`RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001` derives from owner Numeric
compatibility policy and parent `RQ-CF-PRG-NUMERIC-METADATA-001`.
Recover the default Numeric-to-Character display consumer's use of associated
width/decimal metadata and SET DECIMALS/FIXED. Allowed requirement sources are
installed VFP9 public behavior and owner policy, not the existing formatter.
The #6776 per-area compatibility-switch policy, checked conversions and
allocation limits remain unchanged. Neither Numeric metadata fields alone nor
an approximate/tolerance result establish display parity.

Unchanged main240ac51cb and the deferred small-divisor branch already reproduce
70 literal-only display mismatches while all300 literal metadata/payloads match:
signed checkpoint03c3a4520446765125101be7052a05bf7e6f0546 and issue
comment6249598072. At D0/FIXED OFF, current0.5 prints `0`, native `0.5`;
current2.50 prints `2`, native `2.50`, both NUMERICBEHAVIOR modes. This is
reproduction, not a proven algorithm or product acceptance.

## Frozen native stimulus (not expected results)

`emit-probe.cjs` produces the checked-in `probe.prg` and `cases.json`
without importing Copperfin, candidate logic or expected data. It declares
58 literal forms and24 group/unary/IIF controls, DECIMALS0/1/2/6/14/18,
FIXED OFF/ON, and four distinct routes: direct EVALUATE, direct compiled,
EVALUATE then local assignment, compiled then local assignment.3936 records
per invocation. All expressions are pure; repeated evaluations for metadata,
BRS and default TRANSFORM cannot repeat a user callback.

Each observation retains public-ABI CFNMETA type/width/decimals/active payload
and FPU control word, independent BRS payload, exact Character bytes and
VARTYPE. Reuse the already-authored documented public-ABI observer
`build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll` on the host.
No binary decompilation, disassembly, patching, proprietary implementation
reuse, new installation or VM is involved. The installed VFP9 Wine executable,
observer and runner must be hash-bound before/after execution.

`audit-probe.cjs` is an independent strict raw reader. It requires both actual
statuses0, empty stderr, complete unique inventory coverage, finite typed
payload/BRS agreement, Character output, equivalent compiled/EVALUATE routes
and exact typed repeat. Inactive ABI slots are not semantic equality.
No expectation emitter, candidate, test or production code exists yet.

## Qualification and bounds

Freeze source/inventory/reader in a signed DCO commit BEFORE invocation.
Obtain fresh audit/DAW chat clearance and require full I/O PSI avg10 AND avg60
below1% immediately before EACH serialized nice10 invocation. Preserve all
failed/guarded attempts. Record REA Evidence, actual statuses, raw files,
hashes, pressure, root exit versus descendant settlement, cleanup, truncation
and limitations. The runner's25-second bound and inherited environment remain
explicit; process sampling is not syscall coverage or network containment.

Once qualified, native observations may define independent regression
expectations, then actual unchanged ORIGINAL failure before an unused bounded
support helper/direct tests, then separate consumer wiring and unchanged SAME.
Inspect architecture only as architecture, never as the requirement source.
Keep TRANSFORM pictures, Currency, locale changes, general display dispatch,
nonfinite policy, other Numeric producers and the deferred division helper out
of this slice unless additional native evidence explicitly makes one a required
prerequisite. Stored routes materialize at CURRENT settings; they do not prove
behavior of a variable assigned under another setting. That boundary remains
unknown pending a separately frozen probe if needed.

## Traceability and development-document walkthrough

`VR-5611-NUMERIC-DEFAULT-TRANSFORM-001`: native qualification, native-only
expectation emitter, original-fail/SAME, focused/broader/actual named hosted
execution, exact-head review, strict checks and resolved conversations are
pending. No compatibility, parent, hazard or release closure.

`DQ-DV-5611-NUMERIC-DEFAULT-TRANSFORM-001`, LOW procedure delta: preserve the
uncancelled division source/test freeze3b7a51636 and complete ORIGINAL031d790ab;
work only in separate display WT from origin/main240ac. Walkthrough confirms
there is no formatter/hook change and no instruction to drop display assertions.
Misuse risk: mistaking a stimulus or fitted model for the oracle would permit
silent display/data corruption. Mitigation: immutable raw native paired capture,
independent expected reader, actual original fail, unchanged SAME and exact-head
review. HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 unchanged.
Rollback is the exact display slice revert, retaining raw receipts; notify the
owner if evidence or shipped formatting is invalidated. Developer-maintainer
self-review only, not independent-human, system or release assurance.
