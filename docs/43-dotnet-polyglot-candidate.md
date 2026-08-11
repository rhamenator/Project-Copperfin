# .NET Polyglot Leaf Candidate

## Purpose

Copperfin includes one real external .NET target for the artifact-first
polyglot bridge. `samples/polyglot-dotnet-candidate` implements the fixed
capability `samples.dotnet.add-v1`: it adds two signed 64-bit integers and
returns the sum in the versioned polyglot response contract.

FP/VFP source remains the control plane. It calls the capability through
`CFPOLYGLOTDISPATCH()` and inspects the immutable result through `CFJSONGET()`;
it does not contain C# source. A caller that needs nonblocking supervision can
place the dispatch in `SPAWN` and use the existing `CFTASK*` lifecycle.

## Publish And Artifact Identity

The project targets .NET 10 Native AOT and has no package references. Publish
on the target operating system and architecture:

```text
dotnet publish samples/polyglot-dotnet-candidate/Copperfin.PolyglotCandidate.csproj --configuration Release --runtime <RID> --self-contained true
```

Supported repository test RIDs are `win-x64`, `win-arm64`, `linux-x64`,
`linux-arm64`, `osx-x64`, and `osx-arm64`. Win32 is intentionally excluded
because the adopted .NET Native AOT target matrix does not provide `win-x86`.

Native AOT produces a platform-specific executable containing the candidate
and its required .NET runtime. Optional `.pdb`, `.dbg`, or `.dSYM` output is
debug information, not a runtime dependency. Copperfin admits only the exact
executable under an explicit allowed root and lowercase SHA-256. Any change to
source, SDK, runtime pack, RID, compiler, or publish options requires a rebuild,
new hash, and new admission.

For platform behavior and deployment constraints, see the official
[Native AOT deployment documentation](https://learn.microsoft.com/dotnet/core/deploying/native-aot/)
and [single-file deployment overview](https://learn.microsoft.com/dotnet/core/deploying/single-file/overview).

## Machine Contract

The candidate reads at most 1 MiB from standard input and accepts exactly these
unique top-level fields:

- `envelope_version`: `1.0`
- `kind`: `invocation`
- `capability_id`: `samples.dotnet.add-v1`
- nonempty `correlation_id`
- `protocol_version`: `1.0.0`
- `arguments`: an object containing only signed 64-bit integer `left` and
  `right` fields

Wrong identity, missing/duplicate/unknown fields, malformed JSON, or oversized
input exits with code 2 and emits no reflected request data. A valid identity
with invalid arguments returns `sample.dotnet.invalid_arguments`; checked
addition overflow returns `sample.dotnet.overflow`. Both are nonretryable typed
error envelopes. Success returns `{"sum":<signed-int64>}`. The generic adapter
validates exact response identity and maps a candidate error to its established
bounded host error; candidate error text is not made authoritative PRG state.

## Trusted Host Composition

The integration test publishes the executable for the current supported host,
hashes it, admits it under the publish directory, and supplies the opaque token
to `PolyglotRuntimeHost`. The route is explicitly `on`, the bridge is
fail-fast, and the process receives an empty environment, fixed working root,
one attempt, bounded input/output/error streams, and a five-second timeout.
Ordinary PRG then invokes the C# leaf and reads the authoritative sum without a
foreign thread entering mutable runtime state.

The test also proves quiet rejection of wrong identity, duplicate identity
fields, malformed JSON, and input over the candidate's own limit; strict
overflow handling; exact invocation counts; no loose managed/runtime sidecar;
and telemetry redaction of arguments and correlation bytes.

## Security Boundary And Nonclaims

The candidate performs one request and exits. It performs no ambient runtime
discovery, package installation, reflection, arbitrary assembly loading,
network access, shell invocation, or callback into Copperfin. The production
host retains its documented immediate path-based revalidation boundary; this
sample does not claim atomic handle-bound launch.

This implementation proves one coarse-grained external .NET leaf target. It is
not:

- inline C# or mixed-language PRG syntax
- an in-process or general CLR host
- an arbitrary DLL/assembly loader or dependency resolver
- execution of Copperfin's emitted C# transpilation artifact
- a new task/thread lifecycle
- a Python, R, AI, or MCP runtime hook

The Native AOT executable is platform-specific. Each distributed platform and
architecture needs its own published, hashed, admitted artifact and applicable
platform signing/notarization treatment. Copperfin's Ed25519 launcher-inventory
identity is a separate trust contract and does not replace operating-system
code signing.

## Evidence

The focused native target is `test_polyglot_dotnet_candidate`. The related
workflow builds it with the generated-launcher process tests on hosted Windows,
Linux, and macOS. Local Release focused and adjacent tests pass `7/7`, the
focused target repeats `20/20`, Clang 21 ASan/UBSan passes with leak detection,
and managed formatting/analyzers plus focused C++ static analysis are clean.
Exact product/test candidate `4e66813d2` passes Linux Native `31459407580` and
macOS Native `31459407582` at `337/337`, Windows Native `31459407816` at
`336/336`, the focused regression on every host, and the macOS four-locale
matrix at `8/8`. Generated Launcher Validation `31459366674` independently
passes the candidate with its process regression on Windows, Ubuntu, and macOS.
All eleven protected PR checks pass at documentation head `d0c7ef408`.
