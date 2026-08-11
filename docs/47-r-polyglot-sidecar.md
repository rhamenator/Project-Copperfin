# R Polyglot Sidecar

## Status

Copperfin has one implementation-complete R sidecar example for the leaf
capability `samples.r.mean-v1`. It proves the Windows/POSIX execution seam; it
is not an embedded R runtime, package manager, general data-frame bridge, or
permission to place R syntax in FP/VFP source files.

## Trust contract

The trusted host provides two distinct admissions:

1. The configured `Rscript` executable is authorized beneath an explicit
   allowed root and pinned to its exact lowercase SHA-256 digest and physical
   file identity.
2. `candidate.R` is authorized as a non-executable supporting artifact beneath
   its own explicit physical root. Its exact bytes, identity, regular-file
   status, direct non-symlink/non-reparse containment, and bounded size are
   pinned and revalidated.

The invocation binds the admitted script to argument index 1 in
`Rscript --vanilla <absolute-script-path>`. Missing, duplicate, out-of-range,
or substituted bindings fail before process launch. The command line uses the
admission result's `resolved_path()`, not a caller-supplied lexical alias.
Supporting artifacts are revalidated first and the executable last; after
executable revalidation the adapter performs no allocation, callback, policy
decision, or other user-controlled work before its owned launch call.

These checks remain path-based rather than handle-bound across process
creation. Deploy the R installation and admitted scripts in locations not
writable by untrusted principals, and readmit after every change. A production
sidecar that loads data files, package trees, or other loose inputs must admit
and bind those dependencies too.

On Windows, configure the architecture-specific
`<R_HOME>/bin/x64/Rscript.exe`, not the top-level
`<R_HOME>/bin/Rscript.exe` dispatcher. The top-level executable delegates
through `system()` and `cmd.exe`; Copperfin's shell-free, complete child
environment intentionally does not support that path. Automatic test discovery
replaces the dispatcher with the installed x64 front end and omits the R target
if that direct executable is absent. The direct executable is the artifact that
is hash/identity pinned and revalidated before launch.

## Runtime behavior

- FP/VFP stays in control through `CFPOLYGLOTDISPATCH()` and may supervise the
  call through the existing task APIs.
- R starts with `--vanilla`. Copperfin supplies a complete explicit child
  environment containing only `R_DEFAULT_PACKAGES=base`, rather than ambient
  host or agent variables.
- The checked-in sample uses base R only. It performs no package installation,
  package discovery, networking, or callback into mutable runtime state.
- Its closed JSON reader admits at most 1 MiB, 32 levels, and 4,096 values;
  rejects duplicate members, malformed escapes, invalid UTF-8, and non-finite
  numbers; and accepts exactly one invocation envelope.
- The sample accepts one to 1,024 finite numeric values with absolute value no
  greater than `1e12`, calculates a compensated arithmetic mean, and returns
  one strict success or typed-error envelope.
- The generic bounded-process contract owns timeout, cancellation, I/O
  ceilings, stderr separation, and process-tree cleanup.

## Toolchain and regression evidence

Ordinary developer builds discover `Rscript` when it is available and omit the
focused R target otherwise. Generated Launcher Validation provisions exact R
4.6.1 on Windows, Ubuntu, and macOS through
`r-lib/actions/setup-r@d3c5be51b12e724e68f33216ca3c148b66d5f0b6`.
The action uses an immutable commit; Rtools and public package-manager routing
are disabled because this base-R sample installs no packages.

`test_polyglot_r_sidecar` copies the checked-in sample beneath a process-unique
root, admits both artifacts, performs the strict mean round trip, verifies a
typed invalid-argument result, and proves ordinary PRG receives one
authoritative candidate result. It also proves that script substitution and
post-admission mutation prevent R from starting. Direct bounded-process cases
prove the base-R reader rejects duplicate members, an unknown top-level field,
an unpaired Unicode surrogate, and excessive nesting.
`test_rscript_discovery_contract` independently proves that Windows automatic
discovery selects the direct x64 front end, preserves an already-direct path,
leaves POSIX discovery unchanged, and rejects the shell dispatcher when its
direct companion is absent.

Local Release testing uses an isolated unpacked R 4.5.2 runtime without a
system installation; the focused R integration repeats successfully `10/10`,
and the regenerated focused, adjacent, and isolation set passes `8/8`.
At exact signed implementation head `70c68bcfd`, Generated Launcher Validation
`31518107199` passes the real R 4.6.1 sidecar target on Windows, Ubuntu, and
macOS. The Windows job selects and launches the admitted direct x64 front end,
closing the dispatcher incompatibility found by the preceding hosted run.
Windows DECLARE ABI Validation `31518107351`, Windows Environment and
Executable Path Validation `31518107306`, and GCC/Clang Executable Path
Validation `31518107204` also pass; all eleven protected PR checks are green at
that implementation head.
Independent review at coordination sequence 1576 exercised all four discovery
contract cases, deliberately broke the x64 lookup to prove the regression is
load-bearing, and found no defect. The reviewer could not execute R itself on
that Linux review host; the local real-R run and three hosted jobs provide that
separate runtime evidence.

## Remaining H3 work

This slice does not implement general R environments or package locks, a CLR
host, or live MCP/AI product adapters. The practical Python and R leaf examples
share one audited FP/VFP control-plane boundary; product MCP/AI hooks remain the
next distinct H3 implementation gap.
