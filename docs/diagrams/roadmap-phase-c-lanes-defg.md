# Phase C — Designer, IDE & Delivery Pipeline (slice-lanes D + E + F + G)

Part of [05-roadmap.md](../05-roadmap.md), one of the
[Per-Phase Dependency Diagrams And Roadmaps](../05-roadmap.md#per-phase-dependency-diagrams-and-roadmaps).

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

```mermaid
flowchart TB
    classDef done fill:#2f7a52,stroke:#1e5136,color:#ffffff,stroke-width:1px;
    classDef partial fill:#a8790c,stroke:#6e4f07,color:#ffffff,stroke-width:1px;
    classDef lane fill:#f2e0cf,stroke:#a85a2a,color:#1b2024,stroke-width:1px;

    subgraph LANED["Lane D - root #110 'Build/compiler/debug pipeline'"]
      direction LR
      D1["D1 Packaging Pipeline<br/>DLL/FLL/OCX/APP/FXP outputs,<br/>AST/IR/.transpiled.cs contracts<br/>root #19, recorded slices closed"]
      D2["D2 Debugger<br/>breakpoints, watch/locals,<br/>step, xAsset action-id mapping<br/>root #20, recorded slices closed"]
    end

    subgraph LANEE["Lane E - root #111 'Shared design model and designer fidelity' (closed 2026-07-24)"]
      direction LR
      E1["E1 Shared Design Model<br/>memo-heavy round-trip<br/>preservation groundwork<br/>root #22, closed"]
      E2["E2 Designer Interactions<br/>builders, context-aware<br/>editors<br/>root #23, closed, to #1749"]
      E3["E3 Report/Label Fidelity<br/>FRX/LBX field-by-field parity<br/>root #24, closed, to #4543+<br/>(largest lane in the repo)"]
      E1 --> E2
      E1 --> E3
    end

    subgraph LANEF["Lane F - root #112 'IDE and editor parity' (shared root with G)"]
      direction LR
      F1["F1 VS Extension Parity<br/>+ Utility Panes<br/>root #25, #174-175, #1714"]
      F2["F2 Standalone Studio<br/>-> Full IDE<br/>root #26, #176-177"]
      F1 --> F2
    end

    subgraph LANEG["Lane G - root #112 'IDE and editor parity' (shared root with F) - IntelliSense"]
      direction LR
      G1["G1 Semantic Resolution,<br/>Signature Help, Completions<br/>root #27, recorded slices closed"]
      G2["G2 Navigation, References,<br/>Refactoring<br/>root #28, recorded slices closed"]
      G3["G3 Richer IntelliSense<br/>Metadata Inputs<br/>root #29, recorded slices closed"]
      G1 --> G2
      G1 --> G3
    end

    E2 --> F1
    E3 --> F1
    E2 --> D2
    E3 --> D1
    F1 --> G1

    class D1,D2 partial;
    class E1,E2,E3 done;
    class F1,F2 partial;
    class G1,G2,G3 partial;
    class LANED,LANEE,LANEF,LANEG lane;
```

**What's done:** lane E (shared design model, designer interactions, and
report/label fidelity) closed on 2026-07-24 — the newest closure in the whole
repo. The cited D1/D2 and most G1/G2/G3 slices are also closed in live GitHub state,
while lane F has substantial shipped host and utility-pane slices. F2 now also
has localized standalone Build/Run/Debug menu routing at #4888 and focus-scoped
Edit/Undo routing at #4889, while the broader full-IDE parent stays open. G2
child #4887 is reclosed at corrected combined head `9f8f425cf` after array-
subscript and `TEXT ... ENDTEXT` reference fixes.
**What's left:** hosted Windows,
mounted-VFP9, and Visual Studio validation remain
release-evidence gates for the completed implementation slices (per
`agent-handoff.md`); the broader D, F, and G umbrella scopes remain open under
the current MVP subgoal tree and must not be inferred complete from the cited
slice closures.
