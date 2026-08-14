# Feedback Triage Rubric

**Version:** 1.0  
**Date:** 2026-05-03  
**Scope:** All issues submitted via `.github/ISSUE_TEMPLATE/` feedback templates  
**Hazard register:** [docs/safety/hazard-register.md](hazard-register.md)

---

## Purpose

This rubric maps each issue template type to an expected initial response time, triage ownership, escalation path, and hazard register linkage. It exists so that any maintainer opening the issue queue can determine the correct handling without prior context.

---

## Severity Definitions

| Severity | Meaning |
| --- | --- |
| **S0 – Critical** | Data corruption, loss of operator records, or safety-relevant behavior deviation. Linked to `HZ-*` hazards rated `catastrophic` or `high`. |
| **S1 – High** | Runtime crash, hang, or unrecoverable failure that blocks legitimate use. |
| **S2 – Medium** | Functional defect with a known workaround; parity gap vs. VFP reference behavior. |
| **S3 – Low** | Cosmetic, documentation gap, performance, or enhancement with no safety implication. |

---

## Template Triage Map

### 1. `bug-report.yml` — Standard Bug Report

| Property | Value |
| --- | --- |
| **Default severity** | S2 – Medium |
| **Override to S1** | If the "Impact" field is checked as "Data not saved or incorrect" or "Application crash" |
| **Override to S0** | If the "Impact" field indicates corrupted or permanently lost data |
| **Initial response target** | 5 business days (S2), 2 business days (S1), 1 business day (S0) |
| **Triage owner** | Runtime maintainer |
| **Escalation path** | If S0: immediately open a linked `data-corruption-report.yml` issue and cross-link; notify maintainer lead same day |
| **Hazard linkage** | None by default; maintainer adds `HZ-*` if investigation reveals a hazard connection |
| **Required before close** | Reproduction confirmed or waived with rationale; fix or `wont-fix` decision documented |

---

### 2. `runtime-crash-report.yml` — Crash / Hang / Failure

| Property | Value |
| --- | --- |
| **Default severity** | S1 – High |
| **Override to S0** | If crash results in partial write to a DBF/memo that leaves the file in an inconsistent state |
| **Initial response target** | 2 business days (S1), 1 business day (S0) |
| **Triage owner** | Runtime maintainer |
| **Escalation path** | If crash is reproducible and involves file mutation: escalate to `data-corruption-report.yml` flow; link `HZ-runtime-crash-01` and `HZ-data-corruption-01` |
| **Hazard linkage** | HZ-runtime-crash-01 (primary); HZ-data-corruption-01 if I/O involved |
| **Required before close** | Stack trace or reproduction script attached; root cause identified; regression test added or deferred with issue link |

---

### 3. `data-corruption-report.yml` — Data Integrity Issue

| Property | Value |
| --- | --- |
| **Default severity** | S0 – Critical |
| **Initial response target** | 1 business day |
| **Triage owner** | Maintainer lead (not delegable without explicit sign-off) |
| **Escalation path** | Day 1: verify corruption is reproducible; preserve affected files for forensics. Day 2: root cause hypothesis. Day 3: fix or mitigation plan. If no reproduction in 5 days, document and move to S1. |
| **Hazard linkage** | HZ-data-corruption-01 (mandatory). Also link HZ-system-failure-01 if corruption causes runtime abort. |
| **Required before close** | Full investigation report with: reproduction steps, root cause, file format analysis, fix description, regression test(s), and independent reviewer sign-off (DV-*). All five evidence fields in the issue body must be completed. |

---

### 4. `safety-incident-feedback.yml` — Safety Incident or Near-Miss

| Property | Value |
| --- | --- |
| **Default severity** | S0 – Critical |
| **Initial response target** | Same business day if received before 14:00 local maintainer time; next business day otherwise |
| **Triage owner** | Maintainer lead |
| **Escalation path** | Immediate: assign the maintainer lead; create a private draft investigation note. Within 24 hours: confirm whether a `HZ-*` entry exists for the reported behavior; if not, draft a new `HZ-*` entry in hazard-register.md. Within 48 hours: confirm or refute safety relevance. If confirmed safety-relevant: block any pending release tag. |
| **Hazard linkage** | Determined by initial triage; must reference at least one `HZ-*` entry before closure |
| **Required before close** | All seven evidence sections must be non-empty (procedural delta map, misuse analysis, severity classification, review evidence, simulation walkthrough, rollback plan, field notification plan). Because an S0 safety incident is high/catastrophic by definition, a second qualified human reviewer must sign off. Run `scripts/validate-safety-traceability.ps1` against the closing issue JSON and attach the report artifact. |
| **Release gate** | Any open S0 safety incident blocks all release tags until closed or explicitly deferred with documented rationale. |

---

### 5. `documentation-feedback.yml` — Documentation Quality

| Property | Value |
| --- | --- |
| **Default severity** | S3 – Low |
| **Override to S2** | If the "Safety Relevance" field is "Unclear / possibly safety-relevant" |
| **Override to S0** | If the "Safety Relevance" field is "Yes – could cause operator error" |
| **Initial response target** | 10 business days (S3), 5 business days (S2), 2 business days (S0) |
| **Triage owner** | Documentation maintainer (S3/S2); Maintainer lead (S0) |
| **Escalation path** | If S0: immediately create a linked `safety-critical-documentation-change.yml` issue with full DQ-*/DV-*/HZ-* traceability |
| **Hazard linkage** | HZ-doc-command-01 if the documentation describes a runtime command, data manipulation, or recovery procedure; None otherwise |
| **Required before close** | Updated documentation committed or deferred with issue link; if S0: all safety-critical documentation change requirements satisfied |

---

### 6. `safety-critical-documentation-change.yml` — Safety-Relevant Docs Change

| Property | Value |
| --- | --- |
| **Default severity** | S0 – Critical |
| **Initial response target** | 2 business days for initial triage; full closure requires all evidence sections |
| **Triage owner** | Maintainer lead |
| **Escalation path** | Before any merge: verify `DQ-*`, `DV-*`, and `HZ-*` identifiers are present and misuse analysis classifies severity. `none`/`low`/`medium` may use documented maintainer self-review plus applicable automation; `high`/`catastrophic` require a second qualified human reviewer named separately from the author and an approved structured sign-off comment authored by that reviewer account for the exact current issue-body SHA-256. |
| **Hazard linkage** | At least one `HZ-*` entry required. If the documentation change affects a behavior linked to `HZ-data-corruption-01`, `HZ-runtime-crash-01`, or `HZ-system-failure-01`, the linked hazard entry must be updated to reference this change. |
| **Required before close** | Structured rendered sections must occur exactly once when present; historical rendered marker text is accepted only when its corresponding structured heading is absent. Current Review Evidence and legacy Independent Review Evidence schemas are mutually exclusive, with schema-heading presence counted independently from valid section extraction so duplicates cannot fall through to another schema. Raw HTML blocks are excluded from issue evidence without blanking unrelated rendered content; Markdown autolinks remain ordinary rendered content. `scripts/validate-safety-traceability.ps1` must pass. Severity must come from exactly one rendered severity section containing exactly one severity token, or exactly one legacy severity field only when no rendered section exists. Hidden values, duplicated tokens or sections, and mixed severity forms fail closed. Review evidence must identify its mode, meaningful verification scope, and exactly one approved overall `result`/`status`. Scope, basis, and evidence prose describe what was examined; machine-readable outcome is carried only by the structured fields. Maintainer self-review must record exactly one `verification result`/`verification status` equal to `passed`, meaningful automated evidence, and exactly one `automated evidence result`/`automated evidence status` equal to `passed`. `high`/`catastrophic` changes require a reviewer-authored comment containing exactly one rendered structured sign-off section that identifies the reviewer, meaningful qualification basis and verification scope, exactly one `qualification result`/`qualification status` equal to `qualified`, exactly one `verification result`/`verification status` equal to `passed`, the exact current issue-body SHA-256, and exactly one approved overall `result`/`status`. ATX heading markers require following whitespace and may use a whitespace-separated closing hash sequence. Headings or fields inside code blocks or HTML comments are not evidence; comments outside fenced, indented, and inline code are masked without changing line or column positions, and a sign-off comment containing any top-level raw-HTML construct fails closed. Missing, non-passing, or duplicated sections or outcome aliases fail closed. Any later issue-body edit requires renewed sign-off; for the same reviewer and digest, the latest applicable sign-off supersedes earlier approval and may withdraw it. Simulation or walkthrough evidence must be attached or linked. |
| **Release gate** | Unresolved open safety-critical documentation issues block release tagging. |

---

### 7. `feature-request.yml` — Enhancement Request

| Property | Value |
| --- | --- |
| **Default severity** | S3 – Low |
| **Initial response target** | 15 business days |
| **Triage owner** | Runtime maintainer |
| **Escalation path** | If the feature is safety-relevant (e.g., changes audit stream behavior, modifies error recovery semantics, alters data integrity guarantees): escalate to maintainer lead and require `DQ-*`/`HZ-*` linkage before any implementation begins |
| **Hazard linkage** | None by default; add if implementation would touch a hazard-adjacent subsystem |
| **Required before close** | Accepted or declined decision documented; if accepted, linked to an implementation slice issue |

---

### 8. `general-feedback.yml` — General / Catch-all

| Property | Value |
| --- | --- |
| **Default severity** | S3 – Low |
| **Initial response target** | 15 business days |
| **Triage owner** | Any maintainer |
| **Escalation path** | If feedback describes symptoms consistent with a bug, crash, data corruption, or safety incident: close this issue and ask the reporter to resubmit using the appropriate template. Link the new issue from this one. |
| **Hazard linkage** | None; do not assign `HZ-*` identifiers to general feedback issues |
| **Required before close** | Brief acknowledgment; re-routed to correct template if applicable |

---

## Response Time Summary

| Template | Default Severity | Response Target |
| --- | --- | --- |
| bug-report | S2 | 5 business days |
| runtime-crash-report | S1 | 2 business days |
| data-corruption-report | S0 | 1 business day |
| safety-incident-feedback | S0 | Same day (if before 14:00) |
| documentation-feedback | S3 | 10 business days |
| safety-critical-documentation-change | S0 | 2 business days (triage) |
| feature-request | S3 | 15 business days |
| general-feedback | S3 | 15 business days |

---

## Escalation Decision Tree

```text
Issue received
│
├─ Template: data-corruption-report or safety-incident-feedback?
│   └─ YES → S0, maintainer lead, same-day triage, release gate active
│
├─ Template: runtime-crash-report?
│   └─ YES → S1, runtime maintainer, 2-day triage
│       └─ Crash touches DBF I/O? → escalate to S0, add HZ-data-corruption-01
│
├─ Template: bug-report?
│   └─ Impact = "Data not saved or incorrect"? → S1 or S0
│   └─ Otherwise → S2, 5-day triage
│
├─ Template: documentation-feedback?
│   └─ Safety relevant = Yes? → S0, create safety-critical-documentation-change issue
│   └─ Possibly? → S2, 5-day triage
│   └─ No → S3, 10-day triage
│
├─ Template: safety-critical-documentation-change?
│   └─ S0, maintainer lead, release gate active
│
├─ Template: feature-request?
│   └─ S3, 15-day response
│
└─ Template: general-feedback?
    └─ S3, re-route if symptoms match a higher template
```

---

## Hazard Register Cross-Reference

| Hazard ID | Short Name | Templates That May Reference It |
| --- | --- | --- |
| HZ-data-corruption-01 | DBF/memo data corruption | data-corruption-report, runtime-crash-report, safety-incident-feedback, safety-critical-documentation-change |
| HZ-system-failure-01 | Unrecoverable runtime failure | runtime-crash-report, safety-incident-feedback |
| HZ-runtime-crash-01 | Runtime crash during I/O | runtime-crash-report, bug-report (if S0 override) |
| HZ-runtime-debug-01 | Debug session state exposure | safety-incident-feedback |
| HZ-doc-command-01 | Operator error from doc misuse | documentation-feedback, safety-critical-documentation-change |

Full hazard register: [docs/safety/hazard-register.md](hazard-register.md)
