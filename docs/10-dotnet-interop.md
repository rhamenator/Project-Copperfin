# .NET Interoperability Strategy

## Goal

Copperfin must fit into the .NET ecosystem well enough that teams can:

- call managed .NET code from Copperfin applications
- expose Copperfin-built business logic to .NET applications
- package Copperfin functionality as .NET-consumable outputs
- modernize legacy systems without throwing away .NET investments

Current maturity:

- Windows builds provide a bounded in-process .NET Framework v4 `DECLARE ... IN <assembly>` path for public static methods named as `Namespace.Type.Method`. The native host loads the exact resolved assembly through `Assembly.LoadFrom`, resolves overloads through the CLR binder, and currently marshals the supported scalar `DECLARE` values through `VARIANT`/`SAFEARRAY` boundaries.
- The managed `DECLARE` path is tested on Win32 and x64 for absolute, explicit-relative, and parentless loader-resolved paths, sibling dependency resolution, integer/floating/string values, repeat success/failure, localized failures, and mixed-mode assemblies whose native export must take precedence. It is not a general managed object, event, callback, or by-reference interop surface.
- The build pipeline can also generate a C# launcher/stub that is invoked as a child process by the native runtime pipeline in `src/runtime/runtime_pipeline.cpp`.
- The portable artifact-bridge foundation can now serialize a deterministic, versioned invocation request and admit its matching structured response. It does not yet connect that exchange to the runtime host or expose PRG dispatch, asynchronous status, cancellation, or result retrieval.
- Generated C# transpilation output is currently an emitted artifact, not code executed by the runtime host.
- macOS and Linux builds do not compile or host this Windows .NET Framework `DECLARE` path; their native runtime and packaging paths remain guarded from the Windows-only implementation. Broader cross-platform CLR/.NET hosting, managed wrappers, object lifetime, callbacks, policy enforcement, and generated strongly typed bindings remain v1 work.

## Why This Matters

A plausible reason VFP became harder for Microsoft to justify was that it did not align cleanly with the company's 64-bit and .NET-centered strategic direction.

This is an inference from product direction, not a quoted Microsoft statement.

Copperfin should answer that pressure directly instead of fighting it.

## Core Design

The product core remains native and performance-focused.

FP/VFP source remains FP/VFP source. Polyglot use should be exposed through a
PRG-callable capability boundary rather than embedded foreign-language blocks.
For asynchronous work, PRG code should retain orchestration through a bounded
operation handle with status, cancellation, and result retrieval. Foreign
runtime threads must not call directly into mutable Copperfin runtime state;
completion must return through a controlled host/scheduler boundary.

The native `CFJSONVALID()`, `CFJSONTYPE()`, and `CFJSONGET()` facade gives PRG
code a bounded way to inspect immutable structured results from that boundary.
It preserves exact non-string JSON bytes, including large numbers, and does not
load managed code or authorize an artifact. See
`docs/39-prg-json-control-plane.md`.

The sibling native safe-regex facade lets the same PRG control plane validate
and extract bounded immutable text through a documented non-backtracking,
byte-oriented subset. It does not load managed code or authorize an artifact;
unsupported advanced regex features remain eligible for a separately approved
external route. See `docs/40-prg-safe-regex-control-plane.md`.

A dedicated interop layer provides:

- CLR hosting
- managed assembly loading
- type marshaling
- object lifetime coordination
- .NET SDK generation
- executable and library wrapper generation

## Interop Modes

### 1. Copperfin Calls .NET

Use cases:

- call business services written in C#
- reuse modern authentication libraries
- invoke enterprise APIs
- incorporate reporting, PDF, or domain logic already written for .NET

Mechanisms:

- in-process CLR hosting where safe
- out-of-process managed service bridge where isolation is preferred
- generated strongly-typed proxy bindings

### 2. .NET Calls Copperfin

Use cases:

- consume Copperfin runtime services inside a .NET app
- reuse Copperfin query/report/data components from C#
- embed Copperfin business modules in a broader .NET estate

Mechanisms:

- native library exports with managed wrappers
- NuGet-packaged interop assemblies
- generated source or metadata for strongly-typed bindings

### 3. Copperfin Generates .NET-Consumable Outputs

Potential outputs:

- .NET class libraries wrapping Copperfin modules
- self-hosted .NET executables that call into native Copperfin runtime components
- AOT-friendly wrappers where feasible

## 64-Bit Position

Copperfin should be x64-first.

Rules:

- no architectural dependency on 32-bit-only process assumptions
- interop story must work cleanly in 64-bit processes
- packaging should prefer x64 targets and only support x86 when required for migration scenarios

## Packaging Targets

Short-term:

- native desktop app
- native CLI tools
- native shared libraries

Medium-term:

- NuGet packages for Copperfin interop
- .NET host templates for Copperfin-powered services and apps
- generated managed wrappers for reusable modules

## Type System Boundaries

The interop layer needs stable representations for:

- strings and memo/blob values
- numeric/decimal types
- dates and times
- nullability and empty-value distinctions
- result sets and cursors
- object references and event callbacks

## Security Considerations

- managed code invocation should respect Copperfin Shield policies
- assembly loading must be explicit and auditable
- trust boundaries for plugins and managed extensions must be enforced
- out-of-process hosting should be available for untrusted or mixed-trust scenarios

## Recommended Early Deliverables

1. native-to-managed string, decimal, date, and cursor marshaling prototype
2. CLR host proof-of-concept
3. simple C# sample that calls Copperfin native APIs
4. simple Copperfin sample that invokes a C# assembly
5. draft NuGet packaging strategy
