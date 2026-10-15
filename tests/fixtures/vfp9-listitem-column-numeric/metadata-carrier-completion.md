# Numeric metadata carrier: development results and remaining gates

RQ-CF-PRG-NUMERIC-METADATA-CARRIER-001 / VR-5611-NUMERIC-METADATA-CARRIER-001 /
DQ-DV-5611-NUMERIC-METADATA-CARRIER-001. See the pre-code map in
metadata-carrier-requirements.md and the native metadata-foundation/cause
evidence. This is the first representation increment, NOT full Numeric parity.

## Actual implementation

`PrgNumericMetadata` stores normalized unsigned32 width and decimal-place count.
`PrgValue` appends an optional value-owned instance; absence means unspecified.
Existing aggregate member order is retained, but C++ layout changes and all
native consumers must rebuild. The descriptor never rounds the double, creates
an allocation, or performs a conversion. No parser/arithmetic/assignment-width/
field/display/index/ListItem producer or consumer is wired in this slice.
The existing formatter's historical comment now acknowledges an unconsumed
carrier rather than falsely claiming no metadata field exists; formatter
executable statements are unchanged.

The header-only regression covers all existing kind defaults, native-observed
same-bits descriptors (near-one, literal2.5 versus5/2, direct versus assigned
1.00), independent copy/assignment/move/container/by-value parameter/return
ownership, metadata reset, and untouched finite/subnormal/signed-zero/nonfinite
host payloads. Maximum metadata fields are storage-only robustness, not an
unchecked allocation or native-compatibility promise. Literal/arithmetic
creation and actual runtime propagation remain mandatory follow-ons.

## Actual focused verification

- Before the header change, the newly authored/frozen test compilation returned
  1 for the absent type/member. Original header, input hashes and compiler log
  are retained. This is compile-fail then compile/pass representation evidence,
  NOT a claim of a pre-fix behavioral runtime failure.
- GNU: `g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -I include
  tests/test_prg_numeric_metadata.cpp -o build/numeric-metadata-carrier/gnu-test`:
  compile0, execution0, empty compile diagnostics.
- Clang: same C++20/warning flags plus
  `-fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer -g`:
  compile0; execution0 under ASAN_OPTIONS=abort_on_error=1 and
  UBSAN_OPTIONS=halt_on_error=1; empty compile/sanitizer diagnostics.
- Ordinary Debug CMake configure0, new target build0, CTest1/1 PASS. The test
  has no runtime library linkage and no file/environment/process/network/sample
  use; explicit portable parallel-safe isolation passes the isolation contract.
- Focused CTest6/6 PASS: new carrier, release licensing, native platform workflow,
  GitHub Actions, agent channel and native test isolation contracts. Changelog
  fragment validation passes. Frozen pre-code test/requirements hashes remain
  unchanged after the implementation; no expected output was revised to pass.

Source/log/status evidence is retained in `build/numeric-metadata-carrier/` and
the first-party retained `metadata-carrier-evidence/` archive. This is focused
development evidence only. Hosted native rebuilds, strict required checks,
exact-head Claude-first/authorized-Codex review and resolved conversations
remain merge gates. No local full runtime/sanitizer suite or independent-human
approval is claimed. Existing ListItem consumer code/expectations are untouched;
global test CMake/isolation and the public header intentionally change.

## Walkthrough and rollback

The regression stores identical near-one payloads with separate width25/scale14
and width13/scale2 descriptors and copies each independently. It does not inspect
source spelling or change the bits. Future native-backed consumers can therefore
distinguish values which the old representation collapsed. At present no consumer
consults the carrier, so no native formatting/index result is claimed repaired.
Rollback removes the added member/type/test registration and rebuilds consumers;
keep the native evidence and broader owner requirement open. Implementing-agent
self-review of the bounded DQ-DV walkthrough is not independent-human review.
No hazard severity/status, Numeric/list family, system/release or #7189 closure.

## Separate historical CI finding, not this change's result

PR7187 optional Windows run38636352844 attempt2 (exact head8968c625a8) completed
FAILURE. Shards1/3 and3/3 succeeded; shard2/3 logged test_dbf_table timeout196.62s,
202/203 passing. The original numeric_behavior timeout did not recur in that
attempt. This is neither a pass for attempt1 nor a Numeric-metadata regression:
the run predates this carrier and tests its own unchanged head. Exact job logs
and metadata are retained separately under the build directory. Root cause of
the different timeout is not established; no timeout increase, omitted assertion,
production repair or additional retry is hidden in this slice. #7188 remains
the tracking dependency, not authority to expand Numeric production changes.
