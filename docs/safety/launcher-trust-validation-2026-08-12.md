# Windows Launcher-Trust Validation Evidence — 2026-08-12

## Scope

This record links the protected external signer procedure to its safety and
security evidence without recording private key material. It covers only the
Windows pre-execution launcher guard. Authenticode, Apple signing/notarization,
Linux package signing, and ordinary unsigned development builds remain
separate limitations.

## Traceability

| Requirement | Verification | Hazards |
| --- | --- | --- |
| `DQ-MVP-4894-protected-launcher-signing-procedure` | `DV-MVP-4894-workflow-contract`; `DV-MVP-4894-enforced-guard-walkthrough`; `DV-MVP-4894-independent-review` | `HZ-system-failure-01`; `HZ-doc-command-01` |
| `DQ-MVP-4894-non-secret-evidence-boundary` | `DV-MVP-4894-workflow-contract`; `DV-MVP-4894-independent-review` | `HZ-system-failure-01`; `HZ-doc-command-01` |

## First Protected Execution

Run `31623671192` used the non-secret signer ID
`copperfin-launcher-2026-01` at `main` commit
`b458c8794369cb65811e3c5041f8d3804a678e9d`. The `release` environment was
restricted to `main` and supplied the private signing key and matching public
registry through environment-scoped secrets. The log did not disclose either
secret.

The run passed protected-input materialization, signer/registry preflight, the
enforced native build, and the package-trust verifier. The guard walkthrough
then executed the valid signed launch and the modified/removed artifact,
removed/duplicate/case-ambiguous inventory, and modified/removed signature
cases. Expected negative cases returned invariant exit code `4` before the
internal apphost started.

The job nevertheless failed after the matrix because the orchestration script
left the last expected guard exit code in PowerShell's global `LASTEXITCODE`.
GitHub's wrapper propagated that stale value and skipped the non-secret
evidence upload. This is a procedural result-state defect; it does not show a
signature, registry, verifier, or guard bypass.

## Corrected Protected Execution

The correction clears the expected-negative process state only after every
case assertion, evidence write, and protected-input cleanup completes. The
workflow source contract requires exactly one reset at the end of the script.
Removing, duplicating, or moving the reset must fail the focused contract.

Corrected protected run `31630819119` passed at exact `main` merge commit
`111fb67d09df1413221beeebce9b684f47097053`. The run used signer ID
`copperfin-launcher-2026-01`, a valid one-entry external registry, and the
fixed `release` environment restricted to `main`. Every workflow step passed,
including protected-input cleanup.

Artifact `9155061757`,
`copperfin-windows-launcher-trust-provisioning`, has GitHub archive digest
`sha256:c66fd93daab64d3c2abee12291648987f0ebff701ca36f625bfa7d4f207582eb`.
An independent download produced the same archive digest. Its two non-secret
JSON files have these SHA-256 digests:

- provisioning: `7adcc4ff91d5319b2969caa1e30523236d69918049481a80576472780ebc8cb7`;
- validation: `845459b1f83e587002aebc51d2f98a8db03edce9f1e9dc74bac3ab3a56563fa9`.

The validation report records exactly five finalized package artifacts and
eight unique cases. `valid-signed-launch` returned `0`, started the internal
apphost, and passed. `modified-artifact`, `removed-artifact`,
`removed-inventory-record`, `duplicate-inventory-record`,
`case-ambiguous-inventory-record`, `modified-signature-sidecar`, and
`removed-signature-sidecar` each returned `4`, did not start the internal
apphost, and passed. Machine assertions independently verified the schema,
signer, commit/run identity, unique case and artifact counts, 64-character
lowercase SHA-256 values, and all exit/start/status invariants. A marker scan
found no private-key or secret material in the downloaded evidence.

The exact PR head passed all 17 hosted checks before merge, including Linux,
Windows, and macOS native validation, both executable-path compilers, Windows
environment/path and DECLARE lanes, managed UI, all installers, VSIX,
security/SBOM, DCO, and socket checks. Independent source review verified that
the end-of-script process-state reset cannot mask a thrown failure and that
the focused contract rejects a missing, duplicated, or misplaced reset.

`DV-MVP-4894-enforced-guard-walkthrough` and the non-secret evidence boundary
are therefore satisfied. This evidence closes the Windows launcher-inventory
trust scopes in #4894, #4409, #4387, and #4041. It does not authorize a public
release or satisfy the separate Authenticode, Apple signing/notarization,
Linux package signing, independent safety-review, or localization-review gates.

## Superseding Distinct-Payload Execution

Independent review of the corrected artifact found that the dependency and
runtime-configuration fixture sidecars used identical minimal JSON content.
The signed inventory still bound each path, role, and digest independently, so
this was not a verifier bypass. The fixture was nevertheless made more
representative by assigning valid, role-specific JSON payloads and by adding a
source contract that rejects missing or swapped bindings.

Protected run `31635868978` passed every step at exact `main` merge commit
`477035ca2df6d0eb688d58e83aee80805e932d38`, including protected-input
materialization, signer/registry preflight, enforced build, package contract,
guard walkthrough, evidence upload, and protected-input cleanup. Artifact
`9156951212` has GitHub archive digest
`sha256:9385656df3fbbcf0408d8e0c68bb42504dc7a9f1333272f4f05b8b4cbfa29dec`;
an independent raw download reproduced that digest. The provisioning JSON
retains digest
`7adcc4ff91d5319b2969caa1e30523236d69918049481a80576472780ebc8cb7`, and
the superseding validation JSON has digest
`9a475a239c492f9bf0243261506c1e00a3a93f42a68b078157b7b1e6ce78bba3`.

Machine assertions verified the exact signer, run, and commit identities;
five unique artifact paths; five distinct lowercase SHA-256 values; eight
unique cases; the valid launch's exit `0`, apphost start, and passing status;
and all seven negative cases' exit `4`, absent apphost start, and passing
status. The dependency and runtime-configuration fixture digests are now
distinct:

- dependency sidecar: `40831f6757929bc14af6185082bccaadbd188d2246eb324b486e6b59792190e0`;
- runtime-configuration sidecar: `6616eda4480368fb2615c9496a572bd52ac3b1f102d252cbdb6edca338a775d1`.

A refined scan found no PEM private-key block, passphrase, signing-key secret
name, or registry-secret name in either downloaded JSON file. This execution
supersedes the earlier fixture-content evidence while preserving its valid
trust and fail-closed conclusions.

## Release Authority Limitation

The repository currently has one owner and no second trusted maintainer.
The corrected run therefore uses a recorded owner-only manual dispatch rather
than claiming independent environment approval. The environment remains
`main`-only and the secrets remain environment-scoped. Required independent
review and prevention of self-review become mandatory when another trusted
maintainer is admitted.
