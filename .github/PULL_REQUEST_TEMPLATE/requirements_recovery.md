# Pull Request — Requirements Recovery / Traceability

> Use this template for work tied to the DO-178-style requirements-recovery
> effort (`RQ-*`/`VR-*` traceability, `DQ-*`/`DV-*`/`HZ-*` documentation
> safety cases) or otherwise safety-relevant changes. For most ordinary
> contributions — bug fixes, features, small docs/code changes — use the
> **General Contribution** template instead; it's shorter and doesn't require
> traceability IDs you likely don't have.

## Summary

- What behavior changed?
- Why was it needed?

## Traceability

- Parent lane/family issue(s):
- Child slice issue(s):
- Requirement IDs (`RQ-*`):
- Verification IDs (`VR-*`):
- Documentation requirement IDs (`DQ-*`), if docs changed:
- Documentation verification IDs (`DV-*`), if docs changed:
- Hazard linkage IDs (`HZ-*`), if docs changed:
- Source of truth (behavior/docs/clean-room/current Copperfin behavior):

## Verification

- Focused tests run:
- Broader validation run:
- Result summary:

## Evidence

- Commit(s):
- Workflow run(s):
- Artifacts/logs:

## Contribution Licensing And Provenance

- [ ] I have the right to submit this contribution and every included file.
- [ ] I license my contribution under GPL-3.0-only with the Copperfin Application, Runtime, and Toolchain Exception 1.0; I retain my copyright and make no copyright assignment.
- [ ] Every commit has the contributor's `Signed-off-by` trailer.
- [ ] Third-party material is identified with its source, copyright, and compatible license, or this change contains none.
- [ ] This change contains no secrets, signing material, personal/customer data, restricted source, or decompiled proprietary code.

## Documentation Safety Case (Required If Docs Changed)

- [ ] No documentation changes in this PR
- [ ] Documentation changes present and safety case completed

If documentation changed, provide:

- Affected procedures/pages:
- Procedural delta map (before/after operator actions):
- Misuse analysis (how users could misread/misapply):
- Severity assessment (none | low | medium | high):
- Independent reviewer and sign-off evidence:
- Simulation/walkthrough evidence:
- Rollback and field notification plan:

## Compatibility Delta

- [ ] No intentional compatibility delta
- [ ] Intentional compatibility delta documented below

Compatibility delta notes (if any):

## Safety/Security Impact

- Impact level: none | low | medium | high
- Rationale:

## Checklist

- [ ] All `RQ-*` items map to at least one `VR-*`
- [ ] Focused regression coverage added/updated where behavior changed
- [ ] For documentation changes, all `DQ-*` items map to `DV-*` and `HZ-*` (or explicit `HZ-none` rationale)
- [ ] For documentation changes, independent review and misuse analysis evidence is attached
- [ ] Changelog updated for shipped, lasting repo changes
- [ ] Unsupported or partial behavior is explicitly documented
- [ ] Machine-readable contracts remain compatible, or their intentional versioned change is documented and tested
- [ ] User-visible strings are localized and catalog parity is tested, or no user-visible strings changed
- [ ] Windows behavior and Linux/macOS seams were considered and applicable platform evidence is recorded
- [ ] Clean-room and security requirements were reviewed for the changed inputs and outputs
