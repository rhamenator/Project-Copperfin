# Interop, Federation, Trust, and Security — Detailed Gap Diagram

Part of [31-specification-compliance-gap-analysis.md](../31-specification-compliance-gap-analysis.md).

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

```mermaid
flowchart LR
    classDef have fill:#2f7a52,stroke:#1e5136,color:#ffffff,stroke-width:1px;
    classDef gap fill:#8a3a3a,stroke:#5c2626,color:#ffffff,stroke-width:1px;
    classDef lane fill:#f2e0cf,stroke:#a85a2a,color:#1b2024,stroke-width:1px;

    subgraph DOTNET[".NET Bridge (docs/11) vs Reality"]
      direction TB
      D_HAVE["HAVE: cf_runtime_pipeline<br/>_csharp_and_launcher<br/>generates a C# launcher stub,<br/>spawned as a child process"]
      D_GAP1["GAP: no cf_dotnet_host<br/>(x64 CLR hosting)"]
      D_GAP2["GAP: no cf_dotnet_marshaler<br/>(typed marshaling)"]
      D_GAP3["GAP: no cf_dotnet_policy<br/>gateway (allow/fallback/reject)"]
      D_GAP4["GAP: no .NET parity matrix<br/>(exact/adapted/not_supported)"]
      D_GAP5["GAP: transpiled C# output is<br/>emitted, never executed"]
      D_HAVE --> D_GAP1 --> D_GAP2 --> D_GAP3 --> D_GAP4
      D_HAVE --> D_GAP5
    end

    subgraph FED["Database Federation (docs/21) vs Reality"]
      direction TB
      F_HAVE["HAVE: cf_platform_profile<br/>deterministic Fox-SQL to backend<br/>translator + execution-planning lane"]
      F_GAP1["GAP: no live connector execution<br/>for sqlite/postgres/sqlserver/oracle"]
      F_GAP2["GAP: no provider/session/cursor<br/>capability contracts"]
      F_HAVE --> F_GAP1 --> F_GAP2
    end

    subgraph TRUST["Package Trust (docs/29) vs Reality"]
      direction TB
      T_HAVE["HAVE: app.cftrust envelope +\napp.cftrust.sig format fully spec'd;\ncopperfin::package_trust verifier exists"]
      T_GAP1["GAP: unsigned fallback still\nthe DEFAULT dev posture"]
      T_GAP2["GAP: no approved signer +\npublic-key registry provisioned"]
      T_GAP3["GAP: POSIX/macOS report the\ntrust capability unsupported"]
      T_HAVE --> T_GAP1 --> T_GAP2
      T_HAVE --> T_GAP3
    end

    subgraph SECURITY["Security Model (docs/04) vs RBAC Baseline (docs/18)"]
      direction TB
      S_HAVE["HAVE: docs/18 claims a native\nRBAC baseline - 5 roles, hardening\ntiers, surfaced in Studio host JSON"]
      S_GAP1["GAP: docs/04's Entra ID / OIDC /\nSAML identity providers - unclaimed"]
      S_GAP2["GAP: docs/04's Copperfin Shield\n(9 capabilities) - no dedicated\ncf_shield code beyond cf_security"]
      S_GAP3["GAP: audit-event enum (11 items)\nnot verified against actual events"]
      S_HAVE --> S_GAP1
      S_HAVE --> S_GAP2
      S_HAVE --> S_GAP3
    end

    class D_HAVE,F_HAVE,T_HAVE,S_HAVE have;
    class D_GAP1,D_GAP2,D_GAP3,D_GAP4,D_GAP5,F_GAP1,F_GAP2,T_GAP1,T_GAP2,T_GAP3,S_GAP1,S_GAP2,S_GAP3 gap;
    class DOTNET,FED,TRUST,SECURITY lane;
```
