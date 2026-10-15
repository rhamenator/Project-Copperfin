# Bounded plain-decimal producer: development evidence

RQ-CF-PRG-NUMERIC-LITERAL-METADATA-001 / VR-5611-NUMERIC-LITERAL-METADATA-001 /
DQ-DV-5611-NUMERIC-LITERAL-METADATA-001. Parent: owner-directed Numeric
compatibility, RQ-CF-PRG-NUMERIC-METADATA-001, admitted #5611/#6776.
The [pre-code contract](recovered-requirements.md) is deliberately narrower
than complete Numeric creation, propagation, formatting or list compatibility.

## Recovery and original-fail sequence

The initial repeated 144-row native grid remains unchanged. A further repeated
31-case plain-boundary probe completes with 30 values and one recorded native
error36 for the 257-byte input. Both complete captures return0/empty stderr,
are byte-identical (SHA256 de7a4c57f30c7efb2d01351b36de552a9ffc24dbf97f4d32eec3bc5dbef57c9d),
and pass `audit-plain-boundaries.cjs`. That audit interprets documented type-N
versus type-I fields separately. The first boundary-probe attempt aborted on
a 302-byte EVALUATE input; its source, partial result, actual1 and diagnostic
are retained, not counted as a completed acceptance grid. These observations
do not establish a universal compiler/EVALUATE syntax ceiling.

Before producer wiring, add and test the UNUSED pure classifier. The first
GNU Werror helper compile failed on a copying range-loop test warning; only
that loop was corrected to a const reference, and the corrected compile and
execution passed0. Original warning/status remain in `producer-evidence/`.
The original runtime library was built from unchanged main production sources.
An independent public `PrgRuntimeSession` paused-watch regression then failed1
with exactly56 absent-metadata expectations (14 native-backed expressions,
two modes, two DECIMALS settings). Exact native double bits, explicit STR,
arithmetic payload, malformed-input rejection and paused-state controls did
not fail. Freeze hashes of the tests, helper, registration/isolation and
original production/library are in `original-before-wire.sha256`. The helper
test was already passing while the public runtime regression failed.

## Implementation and verification

`src/runtime/prg_numeric_literal_metadata.h` classifies only digit-leading
unsigned plain decimal lexemes with a digit on both sides of one point and
at most256 bytes. It checks bounds/shape before bounded uint32 casts, allocates
nothing and returns optional attributes: width min(40, lexeme length), decimal
places min(18, fractional digits). It neither parses nor rounds the payload.
`parse_number()` in `prg_engine_expression.inl` returns its existing number
value with those attributes attached only after the existing invariant parser
succeeds. Its scanner, token advancement and invalid-input diagnostic are
unchanged. The single primary call now returns that value directly; its only
new include is in `prg_engine.cpp`.

The frozen public regression and helper/carrier CTests pass3/3 after this
connection (GNU Debug). A separate Clang helper compile/run passes0 under
ASan/UBSan/float-cast-overflow with no recovery. This standalone check is NOT
instrumented runtime evidence. Both new tests are explicitly added to the
existing hosted PR sanitizer selection for full instrumented-runtime execution;
hosted results, review and strict gates remain required and are not preclaimed.
Focused Numeric/equality/string-math and isolation checks are separately
retained in `producer-evidence/`; actual outcomes, not invocation, are evidence.

The pure helper has filesystem/environment/process/network/sample-free
parallel isolation. The public test owns an exclusively created directory
below its test working directory, supplies trusted in-memory source and a
private runtime-temp directory (no shared transaction recovery), and deletes
only that owned directory after sessions end. It has no PRG file I/O or host
callbacks. Runtime initialization reads process environment/config, so it is
conservatively serial with scoped-process environment and test-owned filesystem.
No existing test assertion or timeout is relaxed; no conversion/allocation
limit or compatibility-mode policy is changed.

## Mandatory remaining work and impact walkthrough

This first producer is NOT full Numeric parity. Integer/trailing-point,
scientific, unary-minus and overlong literal metadata remain unspecified;
unsupported shapes are not newly rejected by the runtime. In particular, the
257-byte native error is not copied into Copperfin syntax by this classifier.
Native assignment width normalization (1.00 direct width4 versus assigned13),
arithmetic/function propagation, fields, default display and conversion still
require recovery/migration. Ordinary value copies may carry the new attributes;
this is not evidence that assigned/returned descriptors are native-normalized.
Consumers still ignore metadata, so user-visible display and the retained
ListItem supplied-column behavior are not claimed fixed. The owner objective,
#7189, ListBox/ComboBox and #5611/#6776 remain open; these follow-ons are not
optional residual-gap waivers.

Implementing-agent MEDIUM DQ-DV walkthrough: source `0.0000000000000000001`
has scale18 but retains nonzero bits3BFD83C94FB6D2AC; long plain literals cap
width40, not payload magnitude; attributes must never become unchecked buffer
sizes. Pure unsupported/NUL/large-input guards do not change accepted runtime
syntax. Public tests distinguish direct temporaries from assignment and show
watch evaluation does not resume/mutate execution state. Hazard
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 severity/status remain
unchanged. This is developer self-review plus automated/native development
evidence, not independent-human/system/release assurance. Rollback removes the
single hookup/helper/test selection coherently, rebuilds all native consumers
and preserves the owner objective/evidence; notify the owner and update this
record if these documented boundaries prove wrong.

## Prior carrier's late Windows result

PR7191 was merged with all strict gates green; its optional Windows run was
pending then. It has now completed SUCCESS at exact c572e55126a7a38cfe5e74bf616921720136f613:
run38772071102, three successful shards204/204,204/204,203/203 and successful
inventory aggregate611 unique/full611. `producer-evidence/pr7191-late-windows-audit.json`
records exact jobs, full-log hashes and the aggregate receipt; downloaded full
job logs remain host-side in the owned slice build until recoverable cleanup.
This establishes prior carrier full Windows execution, not this new head or
retroactive success for the separate old #7188 failures.
