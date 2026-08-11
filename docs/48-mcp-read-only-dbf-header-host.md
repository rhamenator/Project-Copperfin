# Read-Only MCP DBF Header Host

Copperfin ships a local, model-independent MCP server executable named
`copperfin_mcp_host`. It exposes one useful but deliberately narrow tool:
`copperfin.parse_dbf_header`. The caller supplies exactly the 32-byte DBF
header as 64 hexadecimal characters and receives deterministic, invariant JSON
describing the header.

This is an opt-in inspection boundary. Starting the executable opts into the
MCP session; ordinary Copperfin execution never starts it. The process also
requires the existing `ai.mcp` permission. The default `developer` role is
denied; set `COPPERFIN_SECURITY_ROLE=runtime-operator` or use another future
profile role that explicitly grants that permission. The host does not
select or contact an AI provider, access a network, read a caller-named file,
run a shell, load an extension, or mutate project/runtime state. The MCP client
chooses and reads any source data, then passes only the bounded header bytes.

## Transport And Protocol

The executable uses local standard input/output transport: one UTF-8 JSON-RPC
message per line on stdin and one response per line on stdout. Diagnostics go
only to stderr, input lines are capped at 64 KiB, processing is sequential, and
EOF terminates the process. These properties follow the official
[MCP stdio transport](https://modelcontextprotocol.io/specification/2026-07-28/basic/transports/stdio).

The host supports:

- current stateless protocol `2026-07-28`, including mandatory
  `server/discover`, per-request protocol metadata, and typed complete results;
- initialization-era protocols `2025-11-25` and `2025-06-18`, including the
  `initialize` / `notifications/initialized` lifecycle.

Other versions on the current per-request path fail closed with JSON-RPC error
`-32022` and the supported version list. An initialization-era client that
requests an unsupported legacy version receives the newest supported legacy
version and can disconnect if it cannot use it, as that era's lifecycle
requires. Protocol support is explicit because MCP uses date-based
[version negotiation](https://modelcontextprotocol.io/specification/2026-07-28/basic/versioning).

## Tool Contract

`copperfin.parse_dbf_header` accepts:

```json
{"headerHex":"037e010201000000200001000000000000000000000000000000000001030000"}
```

The object must contain only `headerHex`; its value must contain exactly 64
ASCII hexadecimal characters. Success returns schema version 1 plus DBF
version, last-update date, record count and sizes, flags, code-page mark and
resolved code page, and memo/index/database-container indicators. Invalid
arguments, hexadecimal text, or DBF values return `isError: true` with a
stable machine-readable `errorCode`. The text content is the same JSON value
serialized for clients that do not consume `structuredContent`.

The tool declaration marks the operation read-only, non-destructive,
idempotent, and closed-world. Tool input and all JSON messages use the existing
strict bounded parser, including duplicate-member rejection.

Every `tools/call` attempt emits a content-free `ai.mcp_invoked` audit line on
stderr with only the known/unknown tool classification and success/rejected
outcome. Request bytes and DBF values are never logged.

## Client Configuration

Configure an MCP client to launch the installed executable directly, with no
arguments. A generic configuration shape is:

```json
{
  "mcpServers": {
    "copperfin": {
      "command": "/absolute/path/to/copperfin_mcp_host",
      "args": [],
      "env": {
        "COPPERFIN_SECURITY_ROLE": "runtime-operator"
      }
    }
  }
}
```

On Windows, use the installed `copperfin_mcp_host.exe` path. On macOS and
Linux, use `copperfin_mcp_host`. Client configuration formats differ, so the
client's own documentation remains authoritative.

## Deliberate Limits

This slice is not a general AI adapter, model host, prompt service, filesystem
browser, or mutable development agent. It proves a portable product MCP
surface with one bounded VFP-aware operation. Additional tools require separate
threat analysis, explicit authorization rules, deterministic contracts, and
focused cross-platform evidence.

Regression evidence lives in `test_mcp_host` and `test_mcp_host_stdio`.
They cover both protocol eras, discovery, listing, successful DBF parsing,
strict rejection paths, notification silence, exact JSON-RPC identifiers, and
the newline-delimited process boundary.

## Validation Evidence

At exact signed/DCO implementation head `25c545907`, Generated Launcher
Validation run `31525167620` built and exercised the host and both focused MCP
tests on Windows, Ubuntu, and macOS. Executable Path Validation
`31525167466`, Windows DECLARE ABI Validation `31525167468`, and Windows
Environment and Executable Path Validation `31525167519` also passed; all
eleven protected PR checks were green.

Independent review at that same head rebuilt the host with GCC 15 and
ASan/UBSan, reran both focused tests, and exercised the installed stdio process
with permission-denial, stdout-isolation, audit-redaction, oversized-message,
duplicate-key, exact-member, hexadecimal-boundary, unknown-tool,
version-negotiation, and malformed-envelope probes. No defect or sanitizer
finding was reproduced. The review host was Linux and did not repeat Windows
or macOS execution; the hosted three-platform run above supplies that evidence.
The process is deliberately single-threaded, so no TSan run was claimed. The
underlying unchanged DBF parser was outside this review's implementation diff.
