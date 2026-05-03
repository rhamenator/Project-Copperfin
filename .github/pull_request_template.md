# Pull Request

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
