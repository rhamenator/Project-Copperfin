# R Polyglot Sidecar Template

This checked-in base-R program implements the small leaf capability
`samples.r.mean-v1` through Copperfin's versioned stdin/stdout JSON envelope.
FP/VFP remains the control plane; R source stays in a separate file and executes
outside the trusted runtime core.

Copperfin launches an explicitly admitted `Rscript` with `--vanilla` and an
absolute sidecar path. The host separately admits that script beneath an
explicit physical root, pins its lowercase SHA-256 digest and file identity,
binds it to its exact command-line argument position, and revalidates it before
each launch. The child receives a complete explicit environment instead of
ambient host or agent variables; that environment sets
`R_DEFAULT_PACKAGES=base` so optional default packages are not loaded.
On Windows, configure and admit `<R_HOME>/bin/x64/Rscript.exe`; the top-level
`<R_HOME>/bin/Rscript.exe` is a shell-based architecture dispatcher and is not
compatible with Copperfin's shell-free complete environment.

The template uses base R only, processes one bounded request, performs no
discovery, network access, package installation, callback into mutable runtime
state, or in-process extension loading, and emits only one matching success or
typed-error envelope. Its closed JSON reader rejects duplicate members,
malformed Unicode escapes, non-finite values, excess depth, and excess value
counts without loading a third-party package tree. A production sidecar must
receive the same admission treatment for every script, data file, package tree,
and other loose input it can load; changing any admitted file requires a new
digest and admission.
