# Product Charter

## Working Name

Copperfin Studio

Related product names:

- Copperfin Runtime
- Copperfin Designer
- Copperfin Reports
- Copperfin Migrator
- Copperfin Gateway
- Copperfin Shield

## Mission

Build a modern business application platform that lets organizations keep using legacy FoxPro/xBase-era data and application patterns while gaining modern security, deployment, tooling, and maintainability.

Implementation stance:

- security and speed drive implementation choices
- native C++ is preferred for the trusted core and performance-critical path
- other stacks are allowed only for outer tooling and services when they do not weaken those goals
- .NET compatibility is a product requirement, not an optional add-on
- Rust is acceptable for selected subsystems where memory safety and performance both matter

Version target:

- Version 1 is tested as a Visual FoxPro 9-compatible target. VFP 6, VFP 7, and VFP 8 files may work through shared DBF/FPT/CDX/DBC readers, but they are best-effort and untested rather than separate compile-time targets. Older Fox Software and xBase-family assets can be tracked as wishlist interpretation/inspection targets, but Copperfin should not expose a version-selection dial until the project intentionally funds and tests differentiated behavior.

## Problem

Legacy FoxPro systems still run critical line-of-business workflows, but teams are boxed in by:

- aging runtimes and deployment methods
- shrinking maintainer pools
- weak built-in security assumptions
- difficult source control and CI/CD practices
- fragile installer/runtime dependencies
- poor interoperability with modern identity, APIs, and observability stacks
- pressure to move data into mainstream SQL platforms without a safe bridge

## Product Thesis

The winning successor is not just an interpreter.

It must be a full platform that combines:

- data compatibility
- multi-database connectivity
- language/runtime compatibility
- first-class .NET interoperability
- visual/business-app productivity
- reporting
- scaffolding
- migration tooling
- security by default
- native performance and native desktop control

## Decision Drivers

Ordered by importance:

1. Security
2. Speed
3. Compatibility
4. .NET ecosystem fit
5. Maintainability
6. Developer ergonomics

Language and framework choices should be judged against these, not the other way around.

## Primary Users

- organizations running legacy FoxPro/VFP business systems
- consultants maintaining xBase/FoxPro estates
- internal IT teams modernizing line-of-business software
- developers who want rapid desktop/data-driven business app creation

## Non-Goals For Version 1

- bug-for-bug compatibility with VFP9's known, catalogued defects (see the known-bug exception registry below) — these are intentionally not reproduced
- reproducing VFP9's crash behavior on inputs that crash real VFP9 — Copperfin must not crash on these inputs (see Compatibility Fidelity Rule below)
- pixel-perfect recreation of the original IDE shell
- support for every third-party ActiveX control on day one
- full cloud-native rewrite of every legacy app automatically
- producing binaries for FoxBASE, FoxBASE+, FoxPro 1.x/2.x, dBASE, or Clipper

## Compatibility Fidelity Rule

Copperfin's parity target is **exact duplication of VFP9 behavior, including undocumented edge-case behavior**, with exactly two carved-out exceptions:

1. **Known VFP9 bugs.** Where real VFP9's behavior is a catalogued defect rather than intended behavior, Copperfin intentionally does not reproduce it. Every such exception must be recorded in the known-bug exception registry (`docs/27-known-vfp9-bug-exceptions.md`) with the observed VFP9 behavior, the classification, the evidence it was checked against, and Copperfin's intentional alternate behavior.
2. **Inputs where real VFP9 crashes.** Copperfin must not crash on these inputs. The documented default is to raise a catchable runtime error through the existing `TRY`/`CATCH`/`ON ERROR` machinery instead, so the failure is recoverable rather than fatal. A registry entry should record the crashing input and confirm the non-crash fallback was applied.

Outside of these two exception categories, an undocumented edge case is in scope for exact parity, not excluded by default. Edge-case behavior should be validated against real, installed VFP9 (observed product behavior) or shipped documentation per `docs/07-clean-room-rules.md` — never against decompiled VFP9 binaries, which remain a restricted input.

## Runtime Safety Requirement

For PRG-style execution, Copperfin must remain stack-frugal rather than reproducing the native-stack failure profile of the original `VFP.exe`.

- routine and expression evaluation work should prefer heap-backed or otherwise bounded execution state over unbounded C++ call-stack growth
- deeply nested or recursive PRG workloads must fail through catchable runtime faults before native stack exhaustion becomes possible
- future runtime-parity work, including user-defined function calls inside expressions and native class/method execution, must preserve this constraint rather than reintroducing stack-overflow behavior as an accidental side effect of parity work

This is a product requirement, not an optimization detail.

## Requirements Recovery Principle

Copperfin did not begin with a complete up-front requirements set. The project
therefore uses DO-178C-inspired development-assurance discipline, adapted to a
general-purpose C++/.NET platform, as its quality baseline. This is not a claim
of formal avionics certification, DO-178C compliance, an assigned software
level, or suitability for any particular safety-critical deployment.

Requirements recovery and bidirectional traceability are continuous,
load-bearing work rather than a later documentation pass. The durable matrix
must connect product and compatibility requirements, recovered high- and
low-level requirements, derived requirements and safety constraints,
architecture and implementing code, focused and broader tests, retained
verification results, known-bug/crash exceptions, hazards and mitigations, and
release evidence. Every behavior-changing slice must identify its governing
requirement or record a properly evidenced recovery gap; code and tests alone
do not establish that the intended behavior is known.

Recovered requirements must be derived only from:

- observed behavior from a real installed VFP9 environment
- shipped Microsoft/VFP documentation
- explicit repository-owner product policy
- Copperfin's registered compatibility exceptions for known VFP9 bugs and crash cases

Existing Copperfin implementation or behavior is verification evidence only;
it must never be used as its own requirement source. Derived requirements must
identify their parent product requirement or hazard. Recovered requirements
must not be derived from decompiled VFP binaries or other restricted clean-room
inputs. When code and allowed evidence disagree, record a gap or exception
instead of changing the requirement to rationalize the code. The compatibility
fidelity rule above remains the source of truth for deciding when an observed
edge case is a requirement, a known-bug exception, or a non-crash safety
fallback.

Assurance rigor is proportional to hazard and reach. Data corruption, runtime
containment, security boundaries, package trust, debugger or recovery behavior,
concurrency, external processes, generated code, and plausible use in
safety-significant or large-population systems require explicit hazard, misuse,
boundary, rollback, and verification analysis. Release evidence must identify
the applicable requirements and retained verification results and must disclose
unresolved traceability gaps or exceptions.

A future integrator remains responsible for its system safety assessment,
assurance level, target-specific requirements, independent verification, tool
qualification where applicable, configuration control, operational environment,
and domain certification. Copperfin documentation must not claim that the
project is certified, automatically certifiable, or safe for a specific
safety-critical deployment.

## Success Criteria

- open legacy DBF/FPT/CDX/DBC assets safely and accurately
- connect to SQLite, PostgreSQL, SQL Server, and Oracle through a consistent data access layer
- run a meaningful subset of FoxPro-style business logic with tests
- duplicate VFP9 edge-case behavior exactly per the Compatibility Fidelity Rule, outside the known-bug and crash exceptions
- recover and maintain traceable low-level compatibility requirements from validated VFP9 behavior, shipped docs, and documented exception registries
- host and call .NET components from Copperfin applications cleanly
- generate .NET-consumable assemblies or executables so Copperfin-built logic can be reused in .NET applications
- ship as a 64-bit-first platform with a modernization story stronger than late-stage VFP had
- provide modern capabilities beyond VFP9 using VFP-like phraseology/syntax: threading/concurrency primitives, deeper .NET capabilities, polyglot interoperability, and transpilation to other platforms
- import or map common forms/reports/projects into a modern workspace
- package apps without brittle shared-machine setup
- enforce modern authn, authz, audit, secrets, and policy controls

Interop maturity note:

- Current .NET and polyglot capabilities are not blanket execution guarantees. The current .NET path can publish a generated C# launcher/stub that is invoked as a child process by the native runtime pipeline; generated C# transpilation output is an emitted artifact rather than code executed by the runtime host; Python support has no runtime hook today. Exposing .NET or polyglot behavior to users should require an explicit modernization target selection until those paths are implemented and tested as first-class runtime surfaces.

## Product Principles

1. Compatibility where it preserves business value.
2. Modern defaults where legacy behavior is unsafe.
3. Clean-room implementation only.
4. Tooling first, not just runtime first.
5. Documentation and migration guidance are product features.
6. Database choice should be a deployment decision, not a rewrite trigger.
