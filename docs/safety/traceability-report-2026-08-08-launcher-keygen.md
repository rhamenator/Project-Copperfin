# Launcher Key Generation Safety Traceability Report

## Scope

- Repository: `rhamenator/Project-Copperfin`
- Issue: `#4898` under protected launcher-trust gate `#4409`
- Product/tool implementation: `tools/package-signer/generate-launcher-signing-key.sh`
- Safety boundary: dedicated Ed25519 launcher identity generation and handling
- Non-claim: this report does not complete #4409, create a GitHub `release`
  environment, provision its secrets, or execute the protected Windows guard.

## DQ/DV/HZ Mapping

| Documentation requirement | Verification evidence | Controlled hazards |
| --- | --- | --- |
| `DQ-MVP-launcher-keygen-dedicated-identity` | `DV-MVP-launcher-keygen-contract`; `DV-MVP-launcher-keygen-walkthrough`; `DV-MVP-launcher-keygen-independent-review` | `HZ-system-failure-01`; `HZ-doc-command-01` |
| `DQ-MVP-launcher-keygen-secret-boundary` | `DV-MVP-launcher-keygen-contract`; `DV-MVP-launcher-keygen-walkthrough`; `DV-MVP-launcher-keygen-independent-review` | `HZ-system-failure-01`; `HZ-doc-command-01` |

## Verification Evidence

### DV-MVP-launcher-keygen-contract

`test_package_signer_contract` generates a fresh identity outside the checkout,
derives and compares its public key, compiles the generated registry, signs and
verifies a canonical launcher inventory, checks mode `0600`, performs Windows
provisioning preflight when PowerShell is present, and rejects overwrite,
invalid identifiers, unsafe directory permissions, and checkout-contained
output. The launcher-trust provisioning and native test-isolation contracts
provide adjacent coverage. No generated private material is retained.

### DV-MVP-launcher-keygen-walkthrough

The repository owner ran the documented Linux ceremony on 2026-08-08 with
signer ID `copperfin-launcher-2026-01`. The private PEM reported mode `0600`.
The non-secret metadata identified Ed25519 and public-key DER SHA-256
`de4e9c062bbad4f591031da08b4b76ebac6fb1319f6f82eea521737b6c72241a`.
The generated private key, registry contents, backup location, and any future
secret values are intentionally absent from this report.

### DV-MVP-launcher-keygen-independent-review

The read-only review recorded at coordination sequence 1423 generated an
independent temporary identity, recomputed the DER fingerprint and raw public
key with OpenSSL, compared them with the metadata and registry, ran the real
contract, and exercised overwrite, unsafe identifier, traversal, symlink,
checkout, and permission-negative cases. It found no tool defect and identified
the missing durable DQ/DV/HZ record corrected by this report.

## Hazard Controls And Residual Gate

The dedicated-identity requirement prevents accidental reuse of the archived
product-licensing key. The secret-boundary requirement keeps private material
outside Git, packages, logs, and generated evidence while retaining a public
fingerprint for identity checks. Fail-closed identifier, path, permission,
algorithm, public/private-match, and overwrite checks control
`HZ-system-failure-01` and `HZ-doc-command-01`.

The `release` GitHub environment and its two protected secrets remain external
operator prerequisites. Only a successful protected Windows Launcher Trust
Validation run with the approved signer ID can satisfy #4409.

