# v1 — Post-MVP Roadmap (slice-lanes H + I + J)

Part of [05-roadmap.md](../05-roadmap.md), one of the
[Per-Phase Dependency Diagrams And Roadmaps](../05-roadmap.md#per-phase-dependency-diagrams-and-roadmaps).

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

```mermaid
flowchart TB
    classDef partial fill:#a8790c,stroke:#6e4f07,color:#ffffff,stroke-width:1px;
    classDef planned fill:#6b7280,stroke:#484d54,color:#ffffff,stroke-width:1px;
    classDef lane fill:#f2e0cf,stroke:#a85a2a,color:#1b2024,stroke-width:1px;

    MVP["MVP implementation-complete RC<br/>(gate)"]

    subgraph V1["v1 Roadmap"]
      direction TB

      CONT1["#108 continuation<br/>Broader runtime parity<br/>(v1 item 1) - no dedicated<br/>letter; continues post-Phase-A work"]
      CONT2["#111 / #112 continuation<br/>Full authoring workflows<br/>(v1 item 2) - matures lanes<br/>E / F / G past 'reached green'"]

      subgraph LANEH["Lane H - root #113 'Modernization/outputs/interop/security' (shared root with I)"]
        direction LR
        H1["H1 Deterministic Relational<br/>Backend Translators<br/>root #30 - SEEDED:<br/>see docs/21, cf_platform_profile"]
        H2["H2 Document/Vector Backend<br/>+ AI-Assisted Planning<br/>root #31"]
        H3["H3 .NET Outputs, MCP/AI<br/>Hooks, Python/R Sidecars<br/>root #32 - SEEDED:<br/>see docs/11, docs/19, launcher stub only"]
        H1 --> H2 --> H3
      end

      subgraph LANEI["Lane I - root #113 'Modernization/outputs/interop/security' (shared root with H)"]
        direction LR
        I1["I1 Runtime/Project Security<br/>Depth + Opt-In Generated-<br/>App Controls<br/>root #33 - SEEDED: cf_security baseline"]
        I2["I2 Extension/Host/AI-MCP<br/>Security Boundary<br/>root #34"]
        I1 --> I2
      end

      subgraph LANEJ["Lane J - root #114 'Portability and portable core boundary' (still OPEN, zero shipped evidence)"]
        direction LR
        J1["J1 Preserve Portable Core<br/>Boundary<br/>root #35"]
        J2["J2 Port Standalone IDE<br/>+ Core to macOS<br/>root #36"]
        J3["J3 Port Standalone IDE<br/>+ Core to Linux<br/>root #37"]
        J1 --> J2
        J1 --> J3
      end

      TRACE["Requirements-to-code-to-test<br/>traceability matrix<br/>(v1 item 6) - no lettered root;<br/>standing deferred goal"]
    end

    MVP --> CONT1
    MVP --> CONT2
    MVP --> LANEH
    MVP --> LANEI
    MVP --> LANEJ
    CONT1 --> H1
    CONT2 --> H3
    H1 --> TRACE
    H3 --> TRACE
    I2 --> TRACE
    J1 --> TRACE

    class CONT1,CONT2 planned;
    class H1,H3,I1 partial;
    class H2,I2 planned;
    class J1,J2,J3 planned;
    class TRACE planned;
    class V1,LANEH,LANEI,LANEJ lane;
```

**What's done:** `H1`'s relational backend translators and `I1`'s security
baseline both have real seeds — `cf_platform_profile`'s deterministic Fox-SQL
translator/execution-planning lane, and `cf_security`'s RBAC/audit/secrets/
signing baseline, respectively. **What's left, and what it takes:** this is
covered in full, document-by-document, in
[31-specification-compliance-gap-analysis.md](../31-specification-compliance-gap-analysis.md) —
in particular its interop/federation/trust/security and language/data-fidelity
gap diagrams give the `H`/`I` gaps in the same level of detail as this diagram
gives their dependency shape. Lane `J` (portability) has no shipped evidence
at all yet and is the most clearly not-started item in the entire v1 list.
