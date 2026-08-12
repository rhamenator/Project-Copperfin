# Portable Printer and Shell Boundary

`APRINTERS()` now reaches host printer discovery through the standard-C++
`copperfin::platform::enumerate_printer_names()` contract. The interpreter
does not select a host, invoke a shell, parse command syntax, or own native
printer handles.

On Windows, the private implementation calls Unicode `EnumPrintersW` for local
and connected queues and links `winspool` privately. On macOS and Linux it
resolves `lpstat` to an executable path and invokes it directly through the
existing bounded-process facility with fixed arguments, time, output, and
process-tree limits. No shell, redirection syntax, or ambient child environment
is used. An unavailable printer service produces the established deterministic
`(none)` APRINTERS entry.

Queue deduplication folds ASCII letters only, preserving every non-ASCII UTF-8
byte and avoiding locale-sensitive keys. POSIX discovery also uses a
non-throwing working-directory query and returns no queues when the process has
an invalid working directory, preserving APRINTERS' fail-closed behavior.

`RuntimeSessionOptions::printer_enumeration_callback` lets an embedding host
provide printer names without exposing platform mechanics and makes the PRG
contract deterministic in regression tests. The source contract rejects native
tokens in the public boundary and rejects `_popen`, `popen`, `system`, close-pipe
calls, and shell-redirection strings in both the platform implementation and
runtime path. Generated Launcher Validation builds the APRINTERS regression and
runs it plus the boundary contract on Windows, Ubuntu, and macOS.

This closes the bounded APRINTERS shell/printing seam. It does not claim that
report rendering, print dialogs, spool-job lifecycle, OLE/COM automation, or the
J2/J3 host ports are complete.
