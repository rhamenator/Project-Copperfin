# Numeric metadata carrier: pre-code bounded slice

RQ-CF-PRG-NUMERIC-METADATA-CARRIER-001 derives from the direct owner instruction
2026-10-15 and RQ-CF-PRG-NUMERIC-METADATA-001 under #5611/#6776. Native sources
are the published FLL Numeric width/decimal descriptor and the retained cause
and metadata-foundation captures, not current Copperfin behavior.

## Contract and architecture

Introduce a normalized, value-owned Numeric metadata carrier in PrgValue,
containing display width and decimal-place count separately from its double.
Presence is explicit: an existing/default/host-created value without supplied
metadata remains unspecified, NOT silently assigned SET DECIMALS or zero places.
Copies/moves through normal C++ value containers and call boundaries preserve
supplied metadata and exact payload bits, including negative zero. A metadata
change does not implicitly round the double. Distinct metadata can accompany
identical double bits, as required by the native same-bits observations.

Store attributes as unsigned fixed-width semantic values, not the proprietary
FLL memory layout or opaque unused slots. A carrier is not an allocation size
or permission to cast; future consumers must validate attributes and recover
operation-specific rules before use. No global registry or shared mutable
metadata keyed by double bits: two equal doubles may have different attributes.

Append the optional carrier to PrgValue, preserving existing member ordering
and aggregate initialization. This changes the C++ value layout; all native
consumers must rebuild together. No stable binary PrgValue layout is promised
here. A scoped source search found no sizeof/memcpy/reinterpret serialization
of PrgValue in include/src/tests; that is impact evidence, not a general ABI
proof. Existing value kind, NULL, exact integers, Currency, date/string/object
provenance and numeric payloads are unchanged.

This first slice intentionally does NOT populate parser literals, infer
arithmetic precision, alter assignment widths, field reading, formatting,
Numeric-to-index conversion, ListItem or any other list/combo caller. Those
are mandatory follow-on slices, not declared residual exceptions or completed
compatibility. Existing constructors remain metadata-absent until migrated
with native-backed producer contracts. No global display/conversion behavior
can change from an unconsumed optional field.

## Verification and impact boundary

VR-5611-NUMERIC-METADATA-CARRIER-001: a header-only standalone test checks
absence for all existing kinds, same-bits/native-observed descriptors, independent
copy/assignment/move/vector/map/by-value parameter/return ownership, overwritten
value clearing, and payload preservation for finite/subnormal/nonfinite host
extensions. Host nonfinite/maximum-attribute vectors are derived robustness
tests, not claims of native syntax/output or acceptance by future consumers.
Compile the new test against the original header before modifying that header;
the expected failure is missing type/member compilation, NOT a behavioral
runtime fail-then-pass claim. Then compile/run with GNU and Clang sanitizers,
register ordinary CMake/CTest, audit isolation and run applicable contracts.
Hosted native suites must rebuild against the changed header; no prior binary
or synthetic result substitutes for that gate. Original ListItem converter
acceptance suites remain frozen; their migration has not started.

DQ-DV-5611-NUMERIC-METADATA-CARRIER-001: procedural delta is an incremental
unconsumed value carrier before producer/consumer migration. Misuse severity
LOW for the documented step; misleadingly claiming full Numeric parity would
be the primary misuse. Walkthrough must show identical near-one bits with
separate 2/14 descriptors and unchanged checked conversions/formatting source.
Rollback removes the optional member/type/test registration and rebuilds all
consumers; preserve native evidence and keep the owner objective open. This
is implementing-agent self-review, not independent human approval.

HZ-data-corruption-01 and HZ-runtime-crash-01 apply to future consumers and
the rebuild/layout impact; this slice adds no allocation, cast, I/O or format
interpretation. HZ-doc-command-01 requires the explicit not-yet-wired boundary.
All hazard severity/status, broad Numeric/list family acceptance and release
qualification remain unchanged. Exact-head review and required CI remain gates.
