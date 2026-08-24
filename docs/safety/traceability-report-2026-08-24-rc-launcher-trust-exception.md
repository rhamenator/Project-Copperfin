# RC Launcher-Trust Exception Traceability Report

## Scope

This report records the owner-approved boundary in `#5160`: private RC
evaluation bundles may be assembled without protected launcher-signing inputs,
but that state must be explicit and cannot be confused with release trust. It
uses Copperfin's DO-178C-inspired quality baseline; it does not claim
certification, formal compliance, or suitability for a safety-critical system.

## Requirement And Verification Map

| Requirement / derived constraint | Verification | Hazards |
| --- | --- | --- |
| `RQ-CF-REL-004`; `DQ-rc-launcher-trust-exception` — an unsigned private RC must use the closed `RC_TEST_EXCEPTION` value, never `PASS`, and the exception cannot meet the Release 1.0 launcher-trust gate | `DV-rc-launcher-trust-exception-contract`; assembler self-test; Draft 2020-12 schema validation; `test_rc_candidate_workflow_contract`; protected exact-tag RC execution pending | `HZ-system-failure-01`; `HZ-data-corruption-01`; `HZ-doc-command-01` |

## Boundary And Misuse Analysis

The exception applies only to a manifest whose `official_release` field is
false and whose immutable candidate tag follows the private RC format. The RC
workflow remains read-only and deliberately receives neither the protected
release environment nor signing secrets. Its machine-readable status is
therefore `RC_TEST_EXCEPTION`, rather than a successful trust claim or a
generic absence of evidence.

Treating that status as release trust would permit a package authenticity claim
without an approved registry, enforced launcher guard, signing sidecars, or
exact-artifact verification. The producer/schema pair reject that substitution:
the private RC manifest fixes the value to `RC_TEST_EXCEPTION`, while the
separate protected Windows launcher-trust workflow retains the required release
environment and fail-closed secrets path. This slice does not modify the
launcher, signing procedure, registry, package bytes, or any secret.

## Rollback And Residual Gaps

Before an immutable tag, revert the producer/schema/documentation together if
the evidence classification proves incorrect. After a tag is assembled, do not
rewrite it; withdraw the candidate, disclose the classification error, and use
the next sequential RC. Protected exact-tag RC execution remains necessary to
retain current workflow evidence. Final Release 1.0 signing, Authenticode,
Apple notarization, and Linux package signing remain separate work.

## Focused Verification

- `python3 scripts/assemble-rc-candidate.py --self-test` passed and rejected
  malformed self-test evidence while requiring the exact closed signing map.
- `python3 -m json.tool docs/contracts/rc-validation-manifest-v3.schema.json`
  passed.
- `cmake -DSOURCE_DIR:PATH=$PWD -P
  tests/run_rc_candidate_workflow_contract_check.cmake` passed.
- The adjacent package-signing and protected-launcher provisioning contracts
  passed without supplying protected material.

Protected exact-tag workflow evidence remains pending and is not inferred from
these local checks.
