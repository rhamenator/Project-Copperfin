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

## Correction And Gate State

The correction clears the expected-negative process state only after every
case assertion, evidence write, and protected-input cleanup completes. The
workflow source contract requires exactly one reset at the end of the script.
Removing, duplicating, or moving the reset must fail the focused contract.

`DV-MVP-4894-enforced-guard-walkthrough` remains **pending** until a corrected
protected Windows run passes and its uploaded JSON is inspected. This record
must be amended with the exact corrected commit, run, artifact digest, eight
case results, and independent review before #4894/#4409 close or a new RC is
assembled.

## Release Authority Limitation

The repository currently has one owner and no second trusted maintainer. The
run therefore uses a recorded owner-only manual dispatch rather than claiming
independent environment approval. The environment remains `main`-only and the
secrets remain environment-scoped. Required independent review and prevention
of self-review become mandatory when another trusted maintainer is admitted.
