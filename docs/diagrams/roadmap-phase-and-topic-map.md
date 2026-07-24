# Phase And Topic Map

Part of [05-roadmap.md](../05-roadmap.md). This map is a durable view of the
project tree by workstream topic. It describes relationships and completion
gates, not a queue or a promise that every topic is implemented. It predates
the lettered-lane reconstruction and uses workstream names rather than lane
letters; see
[roadmap-full-phase-dependency.md](roadmap-full-phase-dependency.md) for the
same shape using the real lane identities.

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

```mermaid
flowchart TD
    MVP["MVP implementation-complete RC"]

    subgraph Compatibility["Compatibility And Authoring"]
        RPT["Report and label fidelity"]
        IDE["IDE and designer workflows"]
        RTL["Runtime and language compatibility"]
    end

    subgraph Foundation["Foundation And Delivery"]
        LOC["Localization"]
        PKG["Build, package, and debug contracts"]
        SEC["Security and platform seams"]
    end

    subgraph Evidence["Acceptance Evidence"]
        TEST["Native, managed, and Windows validation"]
        SAMPLE["Real VFP9 sample coverage"]
        DOCS["Safety traceability and known limitations"]
    end

    RPT --> IDE
    RTL --> RPT
    LOC --> IDE
    LOC --> PKG
    PKG --> IDE
    SEC --> PKG
    IDE --> TEST
    RPT --> SAMPLE
    RTL --> SAMPLE
    PKG --> TEST
    TEST --> MVP
    SAMPLE --> MVP
    DOCS --> MVP

    MVP --> V1["v1 transition"]
    V1 --> PARITY["Broader runtime parity"]
    V1 --> DESIGN["Full designer and IDE parity"]
    V1 --> MODERN["Federation, planning, and interop"]
    V1 --> SECURITY["Security depth"]
    V1 --> PORT["Portability"]
    V1 --> TRACE["Requirements traceability"]
```
