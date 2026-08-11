# Python Polyglot Sidecar

## Status

Copperfin has one implementation-complete Python sidecar example for the leaf
capability `samples.python.add-v1`. It proves the Windows/POSIX execution seam;
it is not an embedded Python runtime, package manager, module-discovery system,
or permission to place Python syntax in FP/VFP source files.

## Trust contract

The trusted host must provide two distinct admissions:

1. The configured Python interpreter is authorized as an external executable
   beneath an explicit allowed root and pinned to its exact lowercase SHA-256
   digest and physical file identity.
2. `candidate.py` is authorized as a non-executable supporting artifact beneath
   its own explicit physical root. Its exact bytes, identity, regular-file
   status, and direct (non-symlink/non-reparse) containment are pinned.
   Admission also requires an explicit bounded size (one MiB for the sample;
   the generic API has a hard sixteen-MiB ceiling). The same ceiling is
   enforced inside the opened-file read loop, including if a file grows after
   its initial identity inspection.

The invocation binds the admitted script to argument index 2 in
`python -I -S <absolute-script-path>`. Missing, duplicate, out-of-range, or
substituted bindings fail before process launch. Supporting artifacts are
revalidated first and the executable last; after executable revalidation the
adapter performs no allocation, callback, policy decision, or other
user-controlled work before its owned launch call.

The path checks remain path-based rather than handle-bound across process
creation. They minimize and test the substitution window but cannot make an
interpreter open atomic with Copperfin's prior file read. Deploy admitted files
in a directory not writable by untrusted principals, and readmit after every
change.

The command line must use the admission result's `resolved_path()`, not the
caller-supplied lexical spelling. This matters where a temporary or deployment
path has a different physical spelling (for example macOS `/var` resolving
beneath `/private/var`); the host intentionally rejects the lexical alias as a
substitution.

## Runtime behavior

- FP/VFP stays in control through `CFPOLYGLOTDISPATCH()`; it may supervise the
  call through the existing task APIs and inspect the JSON result.
- Python starts with isolated mode (`-I`) and without `site` initialization
  (`-S`). Copperfin supplies a complete explicit child environment rather than
  inheriting ambient host or agent variables.
- The sample uses only the standard library, reads one bounded request from
  stdin, writes one strict response to stdout, and performs no discovery,
  networking, installation, or callback into mutable runtime state.
- The generic bounded-process contract owns timeout, cancellation, I/O
  ceilings, stderr separation, and process-tree cleanup.

## Regression evidence

`test_polyglot_python_sidecar` resolves the configured interpreter, copies the
checked-in sample under a process-unique root, admits both artifacts, performs
the strict envelope round trip, and proves ordinary PRG receives one
authoritative candidate result. It also proves that substituting the script
argument or modifying the admitted script prevents Python from starting and
revokes the changed supporting token. Adapter, route-execution, runtime-host,
and isolation tests cover the adjacent contracts.

At exact implementation head `648c02b1b`, hosted Generated Launcher Validation
run `31509182243` passes the Python target on Windows, Ubuntu, and macOS.
Windows DECLARE ABI run `31509182280`, Windows Environment and Executable Path
Validation run `31509182221`, and GCC/Clang Executable Path Validation run
`31509182231` also pass. Together with DCO and both package-security checks,
all eleven protected pull-request checks are green.

## Remaining H3 work

This slice does not implement R, general Python environments or dependency
locks, a CLR host, or live MCP/AI product adapters. Those remain separately
reviewable H3 work and must preserve the same FP/VFP control-plane and
fail-closed external-capability boundary.
