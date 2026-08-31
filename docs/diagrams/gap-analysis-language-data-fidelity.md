# Language & Data Fidelity — Detailed Gap Diagram

Part of [31-specification-compliance-gap-analysis.md](../31-specification-compliance-gap-analysis.md).

This diagram is kept in its own file because GitHub's Mermaid renderer only
reliably renders the first diagram on a page; a page with several diagrams
tends to render only the first and leave the rest blank.

```mermaid
flowchart TB
    classDef have fill:#2f7a52,stroke:#1e5136,color:#ffffff,stroke-width:1px;
    classDef gap fill:#8a3a3a,stroke:#5c2626,color:#ffffff,stroke-width:1px;
    classDef spec fill:#33475b,stroke:#1f2c38,color:#ffffff,stroke-width:1px;

    KBX_SPEC["docs/27 requires: every Copperfin/VFP9\nbehavioral difference must be either\na cataloged known-bug/crash exception,\nor it is a parity defect to fix"]
    KBX_REAL["REALITY: registry has ZERO entries\n(scaffold only) - so today, EVERY\nundocumented behavioral difference is\ntechnically an uncataloged parity gap"]
    KBX_NEED["WHAT IT TAKES: an audit pass over\nknown Copperfin/VFP9 divergences,\neach evidenced against real VFP9 or\nshipped docs, classified and filed\nas KBX-NNN before it can be called\nan intentional exception"]
    KBX_SPEC --> KBX_REAL --> KBX_NEED

    LANG_SPEC["docs/22 official VFP9 surface:\n429 commands + 413 functions +\n323 properties + 83 methods +\n72 system vars + 69 events +\n22 objects + 4 preprocessor directives +\n3 operators = 1,418 documented items"]
    LANG_REAL["REALITY: coverage is tracked per-symbol\nin hundreds of individual runtime-surface\nentries; doc's own backlog section states\nthe official inventory is much larger\nthan the current runtime"]
    LANG_NEED["WHAT IT TAKES: close the backlog\nagainst open issues #7/#8/#10/#11/#13/\n#14/#22/#24/#30/#31, expanding coverage\nsymbol-by-symbol with real-VFP9 or\nshipped-doc evidence per docs/07"]
    LANG_SPEC --> LANG_REAL --> LANG_NEED

    IDX_SPEC["docs/13 requires: named-collation\nmapping for IDX/NDX sort markers,\nplus index WRITE fidelity"]
    IDX_REAL["REALITY: CDX/DCX/IDX/NDX/MDX are\nread/inspect only; sort markers are\nheuristic, not mapped to named\ncollations; MDX write fidelity is\nexplicitly out of scope"]
    IDX_NEED["WHAT IT TAKES: a dedicated index-write\nfidelity slice (separate from read/\ninspection parsing) plus a named-collation\nmapping table validated against format\ndocs, not decompiled binaries"]
    IDX_SPEC --> IDX_REAL --> IDX_NEED

    class KBX_SPEC,LANG_SPEC,IDX_SPEC spec;
    class KBX_REAL,LANG_REAL,IDX_REAL gap;
    class KBX_NEED,LANG_NEED,IDX_NEED have;
```
