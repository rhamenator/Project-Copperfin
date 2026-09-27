# Active Completion Roadmap

## Purpose

This is the execution-facing companion to
[the defect-cluster roadmap](81-defect-cluster-and-completion-roadmap.md).
It deliberately excludes closed issues and completed cluster detail. It answers
which unfinished work remains, what should be worked first, and which closed
work still constrains an open slice.

This is a live-work snapshot, not an authority for issue admission. Before
starting an item, verify that it remains open, repository-owner authored, and
carries `agent-approved` on GitHub. Refresh its input with:

```sh
gh issue list --repo rhamenator/Project-Copperfin --state open \
  --label agent-approved --limit 1000 --json number,title,labels,author,createdAt,url
```

The live query is a candidate list only: inspect each item’s `author.login` and
state before admitting it to unattended work.

## Snapshot and completion rule

- **Snapshot:** 2026-09-27.
- **Active approved issues:** 976 in the live query above.
- **Completion:** the issue backlog is not complete merely because a cluster
  has a code change. An item closes only after its accepted behavior,
  requirements traceability, focused regression evidence, and required review
  and validation have landed. The release-level gates remain in
  [docs/05-roadmap.md](05-roadmap.md) and
  [RELEASE-READINESS-REVIEW.md](RELEASE-READINESS-REVIEW.md).

## Immediate active queue

These active defects and decisions have the strongest current leverage because
they concern data integrity at public file boundaries. Work the common DBF
field-decoding and interchange helpers once, then test every sibling exporter
and importer before closing a slice.

| Priority | Active work | Why it is next | Dependency / prior completion that still matters |
| --- | --- | --- | --- |
| P1 | #6651 — Varbinary `Q` in DIF/SYLK | Native VFP9 writes a binary representation while Copperfin routes it through text processing, risking byte corruption. | The completed Character whitespace fix #6646 established the shared serializer seam; it does not cover binary values. |
| P1 | #6650 — Varchar trailing spaces | The DBF read and write paths strip significant Varchar payload spaces before any consumer receives them. | #6646 fixes only fixed-width `C` handling; its formatters exposed the missing `V` rule. |
| P1 | #6639 — data-loss preview, rollback, and batch policy | Cross-cutting product decision for loss-risk operations: cancel/rollback, skip-record, proceed, apply-to-remaining, and inspect loss. | #6640 supplies the COPY TO omission-warning slice; completed format fixes are evidence inputs, not a substitute for the policy. |
| P2 | #6648 — non-null Logical DIF/SYLK cells | Copperfin emits generic display text rather than VFP-compatible Logical cells. | #6649 is the active implementation PR; nullable Logical behavior stays in #6636. |
| P1/P2 | #6628 and #6636 — Date/DateTime and nullable DIF/SYLK values | Format-specific typed cells still differ from native VFP9. | Do not regress #6646 Character formatting or #6648 Logical formatting. |
| P1 | #6614, #6612, #6609, #6630, #6623 | SDF and delimited type fidelity: variable binary/text alignment, Autoincrement, Memo compatibility mode, Blob omission, and blank Numeric import. | #6644/#6645 closed the Memo omission export slices; import policy and other variable-length types remain open. |
| P1 evidence then P2 | #6572 and #6573 | Establish numeric-overflow policy, then round to declared scale before width validation. | The policy must provide an actionable conversion remedy and feed #6639’s loss-risk UX. |

## Active clusters

The table names only unfinished clusters. Issue ranges and historical closure
proof remain in [docs/81-defect-cluster-and-completion-roadmap.md](81-defect-cluster-and-completion-roadmap.md);
this page avoids repeating closed tickets.

| Cluster | Remaining outcome |
| --- | --- |
| Session/task shutdown cleanup | Finish scoped cursor handling, non-`DO` `ON SHUTDOWN`, Destroy-during-QUIT cleanup, and the selectable compatibility architecture (#6194, #6197, #6199, #6454). |
| Reentrant cursor replacement and cancellation | Prove cursor replacement is safe when filters, predicates, or traversal state reenter work (#6576, #6577, #6578), then close the cancellation/lock sibling (#6499). |
| SQL federation and foundational SELECT/buffering | Correct cross-backend translation, NULL/date/coercion behavior, joins, and buffering semantics. |
| DBF, memo, and index integrity | Fail closed for malformed DBF/FPT/index input; preserve field-type and concurrent-mutation integrity. |
| Native objects, COM/OLE, events, and reflection | Complete object semantics, release/reentrancy safety, event order, and Automation lifecycle. |
| COPY/REPORT and interchange formats | Preserve fields and typed values across DBF, SDF, CSV, DELIMITED, DIF, SYLK, XLS, JSON, and database export/import; make loss-risk outcomes explicit. |
| Menus, screens, and interactive I/O | Replace no-op legacy menu, popup, designer, input, browse, screen, and window behavior. |
| SET command and built-in-function parity | Finish state isolation, option semantics, type coercion, three-valued NULL logic, locale/codepage, and boundary errors. |
| Access/Jet migration and Studio format parity | Recover tables, saved queries, forms/reports/VBA metadata, and real fixture behavior. |
| Filesystem, process, security, and resource bounds | Address wildcard/path behavior, TOCTOU, temporary files, subprocesses, malformed input, allocations, cancellation, and denial-of-service limits. |
| Polyglot, packaging, localization, and CI | Finish .NET/C# and other interop, installers/package lifecycle, platform/localization behavior, and qualification infrastructure. |
| Architecture proposals and runtime/class-system residuals | Turn active cross-cutting designs and umbrella residuals into bounded, verified implementation slices. |

## Completed work retained only for continuity

The following is intentionally short. It records completed work only where an
open item could otherwise be mis-scoped or reintroduce a regression.

| Completed work | Open work it constrains |
| --- | --- |
| Reentrant cursor-closure UAF cluster | Do not reopen the closed command slices. Continue the independently open cursor-replacement edges (#6576, #6577, #6578) and cancellation/lock behavior (#6499), or a newly evidenced bypass. |
| #6644 and #6645 Memo omission exports | They cover non-table Memo omission only. They do not decide text-to-Memo import compatibility (#6609), Blob handling (#6630), or the user-visible loss policy (#6639/#6640). |
| #6646 Character whitespace | It establishes `C` behavior for DIF/SYLK. Varchar #6650 and Varbinary #6651 have different on-disk length and byte contracts. |
| #6647 merge / #6649 active PR | The Character change is complete. Keep Logical type-cell work isolated to #6648 and nullable values to #6636. |
| Closed shutdown slices (#6183, #6193, #6195, #6196, #6198, #6263) | Remaining shutdown work is exactly #6194, #6197, #6199, and #6454; do not treat the older cluster text as a ten-issue open queue. |

## Operating order

1. Select an active admitted issue from the immediate queue or the active
   cluster with the highest shared-helper leverage.
2. Trace every source-to-consumer edge, including import/export siblings and
   error, cancellation, rollback, and retry paths.
3. Use VFP9 documentation and a native VFP9 observation where compatibility
   matters. Preserve exact bytes and settings.
4. Add focused regression coverage, requirements traceability, and a
   changelog fragment with the implementation.
5. Merge only after required reviews and validation pass. For documentation-only
   changes, run the applicable documentation/lint checks rather than the full
   product CI suite.
6. Refresh this page and the cluster roadmap after a material backlog sweep or
   a cluster changes state.
