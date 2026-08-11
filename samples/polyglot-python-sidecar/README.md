# Python Polyglot Sidecar Template

This checked-in Python program implements the small leaf capability
`samples.python.add-v1` through Copperfin's versioned stdin/stdout JSON
envelope. FP/VFP remains the control plane; Python source stays in a separate
file and executes outside the trusted runtime core.

Copperfin launches an explicitly admitted Python interpreter with `-I -S` and
an absolute sidecar path. The host separately admits that script beneath an
explicit physical root, pins its lowercase SHA-256 digest and file identity,
binds it to its exact command-line argument position, and revalidates it before
each launch. The child receives a complete explicit environment instead of
ambient host or agent variables.

The template uses only the Python standard library, processes one bounded
request, performs no discovery, network access, package installation, callback
into mutable runtime state, or in-process extension loading, and emits only one
matching success or typed-error envelope. A production sidecar must receive the
same admission treatment for every script, data file, and other loose input it
can load; changing any admitted file requires a new digest and admission.
