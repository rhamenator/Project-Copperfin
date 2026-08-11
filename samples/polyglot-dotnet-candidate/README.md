# .NET Polyglot Candidate Template

This separate C# program is Copperfin's first executable-language candidate for
the artifact-first polyglot migration path. It implements one deliberately
small leaf capability, `samples.dotnet.add-v1`, by accepting the versioned
invocation envelope on standard input and returning one matching success or
error envelope on standard output. FP/VFP source remains the control plane and
does not contain C# source.

Publish on the target operating system and architecture with the .NET 10 SDK:

```text
dotnet publish samples/polyglot-dotnet-candidate/Copperfin.PolyglotCandidate.csproj --configuration Release --runtime <RID> --self-contained true
```

Use `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, or
`osx-arm64` as appropriate. Native AOT publishing produces one
platform-specific executable that contains the candidate and its required .NET
runtime. Copperfin must admit
that exact executable under an explicit allowed root and lowercase SHA-256
before a trusted host binds it to the capability. Rebuild, rehash, and readmit
after any source, SDK, runtime-pack, or configuration change.

The candidate inherits no ambient environment through Copperfin's adapter,
performs no discovery, network access, package restore, dynamic assembly load,
or callback into mutable runtime state, and processes exactly one bounded
request before exiting. It is a template for coarse-grained migrated leaf
logic, not a general CLR host, dependency resolver, or authorization bypass.
