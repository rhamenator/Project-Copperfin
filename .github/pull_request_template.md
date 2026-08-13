# Pull Request

## Summary

- What behavior changed?
- Why was it needed?

## Traceability

- Parent lane/family issue(s):
- Child slice issue(s):
- Governing product/recovered/derived requirement IDs (`RQ-*`, `LLR-*`, or evidenced `GAP-*`):
- Parent requirement or hazard for each derived requirement:
- Verification IDs (`VR-*`):
- Documentation requirement IDs (`DQ-*`), if docs changed:
- Documentation verification IDs (`DV-*`), if docs changed:
- Hazard linkage IDs (`HZ-*`, for behavior or documentation; use `HZ-none` with rationale):
- Allowed requirement source (real VFP9 observation / shipped Microsoft-VFP documentation / explicit owner product policy / registered exception / identified parent requirement or hazard):
- Source evidence or recovery-gap rationale (current Copperfin code is not a requirement source):
- Architecture/code/test reverse-traceability location:

## Verification

- Focused tests run:
- Broader validation run:
- Boundary/misuse/rollback verification, when proportionally required:
- Result summary:

## Evidence

- Commit(s):
- Workflow run(s):
- Artifacts/logs:
- Retention/location and release-evidence disposition:

## Contribution Licensing And Provenance

- [ ] I have the right to submit this contribution and every included file.
- [ ] I license my contribution under GPL-3.0-only with the Copperfin Application, Runtime, and Toolchain Exception 1.0; I retain my copyright and make no copyright assignment.
- [ ] Every commit has the contributor's `Signed-off-by` trailer.
- [ ] Third-party material is identified with its source, copyright, and compatible license, or this change contains none.
- [ ] This change contains no secrets, signing material, personal/customer data, restricted source, or decompiled proprietary code.

## Proportional Assurance Case

- [ ] No behavior or safety-relevant documentation change in this PR
- [ ] Behavior or safety-relevant documentation changed and the proportional assurance case is complete

For data integrity, runtime containment, security/package trust, debugger or
recovery behavior, concurrency, external processes, generated code, plausible
safety-significant/large-population reach, or safety-relevant docs, provide:

- Affected procedures/pages:
- Procedural delta map (before/after operator actions):
- Misuse analysis (how users could misread/misapply):
- Boundary and failure analysis:
- Severity assessment (none | low | medium | high | catastrophic):
- Independent reviewer and sign-off evidence:
- Simulation/walkthrough evidence:
- Rollback and field notification plan:
- Residual limitations:

## Compatibility Delta

- [ ] No intentional compatibility delta
- [ ] Intentional compatibility delta documented below

Compatibility delta notes (if any):

## Safety/Security Impact

- Impact level: none | low | medium | high | catastrophic
- Rationale:

## Checklist

- [ ] All `RQ-*`/`LLR-*` items map to architecture/code, focused and broader verification, retained results, and reverse links; any `GAP-*` remains explicit
- [ ] Focused regression coverage added/updated where behavior changed
- [ ] Applicable `DQ-*` items map to `DV-*`, and behavior/docs map to `HZ-*` (or explicit `HZ-none` rationale)
- [ ] Proportionally required independent review, misuse/boundary analysis, rollback, and walkthrough evidence is attached
- [ ] Changelog updated for shipped, lasting repo changes
- [ ] Unsupported or partial behavior is explicitly documented
- [ ] Machine-readable contracts remain compatible, or their intentional versioned change is documented and tested
- [ ] User-visible strings are localized and catalog parity is tested, or no user-visible strings changed
- [ ] Windows behavior and Linux/macOS seams were considered and applicable platform evidence is recorded
- [ ] Clean-room and security requirements were reviewed for the changed inputs and outputs
- [ ] This PR makes no claim of DO-178C compliance, certification, an assigned software level, or suitability for a specific safety-critical deployment
