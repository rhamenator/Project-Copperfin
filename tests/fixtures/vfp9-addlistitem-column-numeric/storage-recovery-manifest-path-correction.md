# Documentary phase preservation / supplemental manifest path correction

Extending recovery-observations.md initially changed a document already named
by the original first-recovery-evidence.sha256. The current full check returned
1 solely for that document; the other35 entries matched. A utility assertion
expecting0 failed, and no aggregate success is claimed. Raw captures, native
sources, statuses and audits were unchanged.

Preserve the supplemental combined document byte-for-byte as
storage-recovery-observations.md. Restore recovery-observations.md to the exact
original phase, independently confirmed SHA256
b1702fddcd8054d56a70df13f8978e55ac00ed174ff937f49c1c0e09c22013d1.
The complete original36-entry manifest then checks0, with the earlier mismatch
log/status1 and successful restoration log/status0 retained.

The supplemental79-entry storage-recovery-evidence.sha256 had already checked0
against the expanded document. Preserve that original manifest unchanged;
after restoration its old document path intentionally checks1. Add
storage-recovery-evidence-v2.sha256 with ONLY that document path changed to
storage-recovery-observations.md; all79 original hashes remain identical.
Full v2 check0. Both old-path mismatch1 and v2 check0 logs/statuses retained.
This is documentary phase separation/path-only correction, not refrozen or
weakened acceptance assertions, a changed oracle, native rerun or code change.

Use storage-recovery-observations.md and the path-corrected v2 manifest for the
additional32/50/216/288/144 pairs. The preserved original document and original
manifest remain the first972/60 phase. Both leave requirements/default-policy
recovery incomplete; no helper, acceptance assertions or caller is changed.
