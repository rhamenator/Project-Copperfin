# Whole-System Architecture

Part of [05-roadmap.md](../05-roadmap.md). This map shows the real
native/managed build graph together with the aspirational `copperfin-*`
module taxonomy from `docs/02-architecture.md`'s "Top-Level Product Map," in
one diagram, so the distance between what exists and what is planned is
visible at a glance. It mirrors the ground-truth class diagram in
[24-system-uml.md](../24-system-uml.md) but in flowchart form rather than
UML, and folds in the aspirational layer that document deliberately keeps
separate.

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

Refreshed 2026-09-26 from `CMakeLists.txt`. The map now separates foundation,
data/interop, runtime/build, host, and managed-client dependencies; includes
`cf_migration` and `cf_mcp_host`; and preserves the target-state taxonomy as
dashed relationships. See [24-system-uml.md](../24-system-uml.md) and
[28-repository-ontology.md](../28-repository-ontology.md) for the companion
UML and source-backed ontology.

```mermaid
flowchart TB
    classDef foundation fill:#33475b,stroke:#1f2c38,color:#fff;
    classDef data fill:#0f766e,stroke:#115e59,color:#fff;
    classDef runtime fill:#7c3aed,stroke:#5b21b6,color:#fff;
    classDef studio fill:#a85a2a,stroke:#7c2d12,color:#fff;
    classDef security fill:#991b1b,stroke:#7f1d1d,color:#fff;
    classDef executable fill:#4b5563,stroke:#2f353b,color:#fff;
    classDef managed fill:#1d4ed8,stroke:#1e3a8a,color:#fff;
    classDef aspirational fill:#a8790c,stroke:#6e4f07,color:#fff;

    subgraph FOUNDATION[Foundation Libraries]
      direction LR
      PS["cf_platform_support<br/>process, paths, files, JSON, encoding, PE"]
      LOC["cf_localization<br/>catalogs, locale, translation"]
      LIC["cf_licensing<br/>payload, status, Ed25519"]
      TRUST["cf_package_trust<br/>launcher inventory trust"]
      SEC["cf_security<br/>authorization, audit, containment,<br/>external-process and workspace-agent policy"]
    end

    subgraph DATA[Data, Interop, and Migration]
      direction LR
      VFP["cf_vfp_assets<br/>DBF/FPT, CDX, Access, xAsset,<br/>staged import, editor/undo"]
      PROFILE["cf_platform_profile<br/>database model, federation, query translation,<br/>polyglot routes, telemetry"]
      SQLITE["cf_sqlite_connector<br/>read-only plan execution / fallback"]
      MCP["cf_mcp_host<br/>stdio protocol + read-only DBF header"]
      MIG["cf_migration<br/>project inventory"]
    end

    subgraph EXECUTION[Runtime and Build]
      direction LR
      RTEXT["cf_runtime_text<br/>runtime localized diagnostics"]
      ANALYSIS["cf_prg_analysis<br/>static PRG analysis"]
      DESIGN["cf_design_model<br/>workspace, designers, builders, reports,<br/>toolbox, VS launch contract"]
      XBASE["cf_xbase_runtime<br/>parser/expression, dispatch/flow,<br/>cursor/session, commands, builtins,<br/>SQL/index, objects/COM/DLL/polyglot"]
      PIPE["cf_runtime_pipeline<br/>classification, manifests, library export,<br/>host bridge, C# launcher, packaging"]
    end

    subgraph HOSTS[Native Hosts]
      direction LR
      INSPECT["copperfin_inspect"]
      MCPX["copperfin_mcp_host"]
      STUDIOH["copperfin_studio_host"]
      RUNTIMEH["copperfin_runtime_host"]
      BUILDH["copperfin_build_host"]
      GUARD["copperfin_launcher_guard<br/>Windows only"]
    end

    subgraph MANAGED[Managed Clients]
      direction LR
      VSIX["Copperfin.VisualStudio<br/>editor, IntelliSense, design/debug clients"]
      WINFORMS["Copperfin.Studio<br/>standalone designer shell"]
    end

    subgraph TARGET[Target-State Module Taxonomy]
      direction LR
      CORE["copperfin-core"]
      CDATA["copperfin-data"]
      CONN["copperfin-connectors"]
      CRUNTIME["copperfin-runtime"]
      CDESIGN["copperfin-designer"]
      CREPORTS["copperfin-reports"]
      CMIG["copperfin-migrator"]
      CDOTNET["copperfin-dotnet"]
      CGATEWAY["copperfin-gateway"]
      CSHIELD["copperfin-shield"]
      CCLI["copperfin-cli"]
      CVSIX["copperfin-vsix"]
    end

    LOC --> PS
    LIC --> PS
    SEC --> LOC
    SEC --> PS
    TRUST --> LIC
    PROFILE --> LOC
    PROFILE --> SEC
    SQLITE --> PROFILE
    SQLITE --> SEC
    VFP --> LOC
    VFP --> PS
    VFP --> SEC
    VFP --> PROFILE
    MCP --> PS
    MCP --> VFP

    RTEXT --> LOC
    ANALYSIS --> RTEXT
    DESIGN --> VFP
    DESIGN --> ANALYSIS
    DESIGN --> LOC
    XBASE --> ANALYSIS
    XBASE --> RTEXT
    XBASE --> DESIGN
    XBASE --> VFP
    XBASE --> PROFILE
    XBASE --> SEC
    PIPE --> DESIGN
    PIPE --> XBASE
    PIPE --> PROFILE
    PIPE --> SEC
    PIPE --> LIC
    PIPE --> PS

    INSPECT --> VFP
    INSPECT --> SEC
    INSPECT --> LIC
    INSPECT --> LOC
    MCPX --> MCP
    MCPX --> SEC
    STUDIOH --> DESIGN
    STUDIOH --> PROFILE
    STUDIOH --> SEC
    STUDIOH --> LIC
    RUNTIMEH --> XBASE
    RUNTIMEH --> SQLITE
    RUNTIMEH --> PROFILE
    RUNTIMEH --> SEC
    RUNTIMEH --> LIC
    RUNTIMEH --> LOC
    BUILDH --> PIPE
    BUILDH --> LOC
    BUILDH -.stages.-> GUARD
    GUARD --> TRUST
    GUARD --> SEC
    GUARD --> LOC
    GUARD --> PS
    VSIX -.design JSON.-> STUDIOH
    VSIX -.debug protocol.-> RUNTIMEH
    WINFORMS -.design JSON.-> STUDIOH
    WINFORMS -.debug protocol.-> RUNTIMEH

    CORE -.extracts foundation.-> LOC
    CDATA -.extracts format layer.-> VFP
    CONN -.deepens federation.-> PROFILE
    CRUNTIME -.extracts runtime.-> XBASE
    CDESIGN -.extracts design model.-> DESIGN
    CREPORTS -.new render/export layer.-> DESIGN
    CMIG -.new migration layer.-> VFP
    CDOTNET -.deepens bridge.-> PIPE
    CGATEWAY -.new gateway.-> SEC
    CSHIELD -.deepens policy.-> SEC
    CCLI -.unifies hosts.-> PIPE
    CVSIX -.implemented by.-> VSIX
    CDATA -.requires.-> CORE
    CONN -.requires.-> CDATA
    CRUNTIME -.requires.-> CDATA
    CDESIGN -.requires.-> CDATA
    CREPORTS -.requires.-> CRUNTIME
    CMIG -.requires.-> CDESIGN
    CDOTNET -.requires.-> CRUNTIME
    CGATEWAY -.requires.-> CDOTNET

    class PS,LOC,LIC foundation;
    class SEC,TRUST security;
    class VFP,PROFILE,SQLITE,MCP,MIG data;
    class RTEXT,ANALYSIS,XBASE,PIPE runtime;
    class DESIGN studio;
    class INSPECT,MCPX,STUDIOH,RUNTIMEH,BUILDH,GUARD executable;
    class VSIX,WINFORMS managed;
    class CORE,CDATA,CONN,CRUNTIME,CDESIGN,CREPORTS,CMIG,CDOTNET,CGATEWAY,CSHIELD,CCLI,CVSIX aspirational;
```
