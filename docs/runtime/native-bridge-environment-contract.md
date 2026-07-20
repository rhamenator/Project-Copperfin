# Native Bridge Environment Contract

Generated POSIX runtime bridges construct the child environment immediately before `execve`. The snapshot is bounded by a bridge-local mutex, so concurrent bridge launches from the same generated module cannot interleave environment capture or override application.

Code that directly calls `setenv`, `unsetenv`, or `putenv` must provide the same external process-wide serialization around those calls and generated-bridge launches. The C++ environment helpers in `include/copperfin/platform/environment.h` serialize Copperfin callers; they cannot automatically serialize unrelated library calls that manipulate libc process-global state.

The POSIX bridge contract is:

- inherited `NAME=value` entries are copied as UTF-8 byte strings;
- duplicate inherited names collapse to one entry, with the last inherited entry winning while preserving the first entry's position;
- explicit overrides replace the matching inherited entry, or append when absent, and the last override wins;
- working-directory, argument, `execve`, and child exit-code behavior remain unchanged;
- the Windows `GetEnvironmentStringsW` and `CreateProcessW` path is independent and unchanged.

The generated bridge test seam is compiled only by focused native tests and is not part of shipped package output. It executes `/usr/bin/env -0` through the generated POSIX launch path to verify inherited UTF-8 bytes, override behavior, duplicate-key normalization, and the reported child exit code without relying on timing-sensitive concurrency assertions.
