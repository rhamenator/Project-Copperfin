# Pull Request — General Contribution

> Use this template for most contributions: bug fixes, features, tests,
> tooling, or docs that aren't part of the requirements-recovery effort. If
> your change is safety-relevant or tied to `RQ-*`/`VR-*` traceability, use
> the **Requirements Recovery / Traceability** template instead.

## Summary

- What does this change do, and why?
- Related issue(s), if any:

## Testing

- How did you verify this? (commands run, tests added/updated, manual steps)
- Result:

## Contribution Licensing And Provenance

- [ ] I have the right to submit this contribution and every included file.
- [ ] I license my contribution under GPL-3.0-only with the Copperfin Application, Runtime, and Toolchain Exception 1.0; I retain my copyright and make no copyright assignment.
- [ ] Every commit has my `Signed-off-by` trailer (see [CONTRIBUTING.md](../../CONTRIBUTING.md)).
- [ ] Third-party material is identified with its source, copyright, and compatible license, or this change contains none.
- [ ] This change contains no secrets, signing material, personal/customer data, restricted source, or decompiled proprietary code.

## Checklist

- [ ] Existing tests pass, and I added/updated tests if behavior changed
- [ ] User-visible strings are localized (or none changed) — see [docs/22-vfp-language-reference-coverage.md](../../docs/22-vfp-language-reference-coverage.md) if touching runtime language coverage
- [ ] I updated `CHANGELOG.md` if this is a lasting, user-visible change
- [ ] Unsupported or partial behavior is clearly documented

Not sure about any of the above? Say so in the PR description — a maintainer can help. See [CONTRIBUTING.md](../../CONTRIBUTING.md) for the full guide.
