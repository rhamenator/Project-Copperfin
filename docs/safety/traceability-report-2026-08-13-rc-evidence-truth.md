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
`docs/contracts/rc-validation-manifest-v2.schema.json`,
`docs/35-rc1-evaluation-guide.md`, and the `RQ-CF-REL-001` matrix row.

## Procedural Delta And Misuse Analysis

Before this correction, successful producer jobs generated the unqualified
fields `installers: passed` and `visual_studio_vsix: passed`. A reviewer could
reasonably misread those fields as proof that installation, launch, upgrade,
disablement, uninstall, or residue checks had run. The producer workflows in
fact build packages, verify selected staged/static contracts, and upload the
artifacts; they do not execute those lifecycle operations.

Schema v2 replaces the ambiguous fields with separate build/static-check and
lifecycle fields. Lifecycle fields are fixed at `NOT_RUN` until a future
workflow ingests actual target-platform evidence. Signing, qualified
linguistic review, and real installed-VFP9 evidence are likewise separate.
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

- `python3 scripts/assemble-rc-candidate.py --self-test` passes and proves the
  exact schema-v2 field mapping, absence of the old ambiguous keys, bundle
  layout, and byte identity of the bundled schema.
- `python3 -m json.tool docs/contracts/rc-validation-manifest-v2.schema.json`
  passes.
- `jsonschema.Draft202012Validator.check_schema(...)` accepts the schema.
- `cmake -DSOURCE_DIR="$PWD" -P
  tests/run_rc_candidate_workflow_contract_check.cmake` passes.
- `git diff --check` passes.

Independent review and protected exact-head workflow evidence remain pending;
therefore `RQ-CF-REL-001` remains `gap` rather than `defined`.

## Rollback And Field Notification

Before a new RC tag, rollback is a normal revert of the schema-v2 producer and
guide. After a candidate uses schema v2, do not rewrite its tag or artifact;
withdraw the affected candidate from evaluation, disclose the incorrect field
or schema, correct the producer with regression coverage, and create the next
sequential immutable RC. Never delete user projects, settings, or evidence to
make a lifecycle result appear clean.
