# RC Evidence Truthfulness Traceability Report

## Scope And Assurance Boundary

This report covers `RQ-CF-REL-001`: the private release-candidate validation
manifest must distinguish evidence levels rather than treating artifact
construction as installer or Visual Studio extension lifecycle proof. It uses
Copperfin's DO-178C-inspired project quality baseline; it is not a claim of
DO-178C compliance, certification, an assigned software level, or suitability
for a safety-critical deployment.

## Requirement And Verification Map

| Documentation / product requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `RQ-CF-REL-001`; `DQ-rc-evidence-v2-scope-separation` — use a versioned schema and closed status vocabulary; distinguish build/static checks, lifecycle execution, signing, linguistic review, and real installed-VFP9 evidence | `DV-rc-evidence-v2-assembly-self-test`; `DV-rc-evidence-v2-schema-validation`; `DV-rc-evidence-v2-workflow-contract`; exact-head protected execution pending | `HZ-system-failure-01`; `HZ-doc-command-01` |

Reverse traceability is carried by
`scripts/assemble-rc-candidate.py`,
`docs/contracts/rc-validation-manifest-v3.schema.json`,
`.github/workflows/rc-candidate-assembly.yml`,
`docs/35-rc1-evaluation-guide.md`, and the `RQ-CF-REL-001` matrix row.
The focused `tests/run_rc_candidate_workflow_contract_check.cmake` check also
anchors the governing requirement, derived requirement, verification claim,
and hazards directly in executable verification.

## Procedural Delta And Misuse Analysis

Before this correction, successful producer jobs generated the unqualified
fields `installers: passed` and `visual_studio_vsix: passed`. A reviewer could
reasonably misread those fields as proof that installation, launch, upgrade,
disablement, uninstall, or residue checks had run. The producer workflows in
fact build packages, verify selected staged/static contracts, and upload the
artifacts; they do not execute those lifecycle operations.

Schema v2 first replaced the ambiguous fields with separate build/static-check
and lifecycle fields. The active schema v3 retains that evidence-level
separation and adds the bounded Windows installer and VSIX lifecycle fields.
Unperformed operations remain `NOT_RUN`; signing, qualified linguistic review,
and real installed-VFP9 evidence remain separate. The owner-approved unsigned
Windows launcher state in a private RC is not unperformed evidence: it is the
closed `RC_TEST_EXCEPTION` status under `RQ-CF-REL-004`, which cannot satisfy
the Release 1.0 launcher-trust gate. Historical v1/v2 candidate artifacts are
retained as historical evidence and are not reinterpreted.
The likely effect of misuse is an unjustified release or evaluation decision,
so the potential severity is **high**.

## Boundaries And Residual Gaps

- This slice changes evidence classification only. It does not install,
  launch, upgrade, repair, disable, or uninstall any artifact.
- It does not run the protected Windows launcher-trust workflow and does not
  claim Authenticode, VSIX signing, Apple signing/notarization, or Linux
  package/repository signing.
- It does not supply qualified Spanish/Portuguese review or current execution
  against a real installed VFP9 environment.
- Existing immutable RC1 and RC2 tags and artifacts remain unchanged. Their
  schema-v1 fields must not be reinterpreted as lifecycle evidence.

## Verification Results

At the current local implementation state:

- `python3 scripts/assemble-rc-candidate.py --self-test` passes and validates
  the generated schema-v3 manifest against the exact bundled schema. It also
  proves the separate Windows installer/VSIX lifecycle mapping, absence of the
  old ambiguous keys, bundle layout, byte identity of the bundled schema,
  rejection of missing governing fields, and rejection of a malformed workflow
  URL without optional format assertions.
- `python3 -m json.tool docs/contracts/rc-validation-manifest-v3.schema.json`
  passes.
- `jsonschema.Draft202012Validator.check_schema(...)` accepts the schema.
- `cmake -DSOURCE_DIR="$PWD" -P
  tests/run_rc_candidate_workflow_contract_check.cmake` passes.
- `git diff --check` passes.

Independent review and protected exact-head workflow evidence remain pending;
therefore `RQ-CF-REL-001` remains `gap` rather than `defined`.

## Rollback And Field Notification

Before a new RC tag, rollback is a normal revert of the schema-v3 producer and
guide. After a candidate uses schema v3, do not rewrite its tag or artifact;
withdraw the affected candidate from evaluation, disclose the incorrect field
or schema, correct the producer with regression coverage, and create the next
sequential immutable RC. Never delete user projects, settings, or evidence to
make a lifecycle result appear clean.
