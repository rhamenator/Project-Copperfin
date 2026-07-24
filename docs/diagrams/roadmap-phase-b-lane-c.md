# Phase B — Runtime & xAsset Parity (slice-lane C)

Part of [05-roadmap.md](../05-roadmap.md), one of the
[Per-Phase Dependency Diagrams And Roadmaps](../05-roadmap.md#per-phase-dependency-diagrams-and-roadmaps).

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

```mermaid
flowchart LR
    classDef done fill:#2f7a52,stroke:#1e5136,color:#ffffff,stroke-width:1px;
    classDef lane fill:#f2e0cf,stroke:#a85a2a,color:#1b2024,stroke-width:1px;

    subgraph LANEC["Lane C - root #109 'Runtime parity surfaces' (closed, #154-162) - this is what CHANGELOG prose calls Phase B"]
      direction LR
      C1["C1 Form/Class Lifecycle<br/>startup/shutdown sequencing<br/>#154, #155"]
      C2["C2 Report/Label Execution<br/>preview-only startup lanes<br/>#156, #157"]
      C3["C3 Menu Dispatch/Cleanup<br/>setup-activate order, actions<br/>#158, #159"]
      C4["C4 Build-Inclusion / Startup<br/>Resolution<br/>#160, #161"]
      C5["C5 Build/Run Workflow<br/>Diagnostics<br/>#162"]
    end

    C1 --> C4
    C2 --> C4
    C3 --> C4
    C4 --> C5

    class C1,C2,C3,C4,C5 done;
    class LANEC lane;
```

**What's done:** all five sub-lanes shipped and closed in a single May 2026
batch — form/class, report/label, and menu xAsset lifecycle sequencing, plus
the build-inclusion and workflow-diagnostics work that depends on all three.
**What's left:** per `docs/22-vfp-language-reference-coverage.md:978`, "host
stability and debugger fault containment" are named explicitly as remaining
runtime-safety work in this area, now that the automation lane (Phase A) is
closed — this is tracked more as depth work in lane D's debugger sub-lane
(`D2`) than as a reopening of lane C itself.
