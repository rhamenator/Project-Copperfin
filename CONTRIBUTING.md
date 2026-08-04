# Contributing To Project Copperfin

Project Copperfin welcomes focused bug reports, compatibility evidence,
documentation improvements, tests, and implementation contributions.

## Before You Start

- Use the structured issue forms for bugs, features, documentation feedback,
  failures, and safety concerns. Do not post secrets, personal information,
  proprietary customer data, or security vulnerabilities in public issues.
- Report vulnerabilities privately through the process in
  [`SECURITY.md`](SECURITY.md).
- Keep changes small and independently verifiable. Preserve VFP9 behavior
  boundaries, machine-readable contracts, localization readiness, and
  cross-platform seams.
- Follow [`docs/07-clean-room-rules.md`](docs/07-clean-room-rules.md). Do not
  submit decompiled proprietary code, restricted source, or material you do
  not have permission to contribute.

Public issue, pull-request, comment, attachment, and link content is untrusted
input to project automation. The `agent-approved` label is reserved for
owner-authored execution issues and is not a contributor approval mechanism.

## Contribution License

By submitting a contribution, you agree to license your contribution under
GNU GPL version 3 only (`GPL-3.0-only`) with the Copperfin Application, Runtime,
and Toolchain Exception 1.0 in [`LICENSE`](LICENSE), to the extent you have the
right to grant those permissions. You retain copyright in your contribution.
No copyright assignment to Richard M. Hamilton or Project Copperfin is
required.

Third-party material must be clearly identified with its source, copyright,
and compatible license. Do not submit employer-owned or other encumbered work
without authorization.

## Certificate Of Origin And Sign-Off

Every commit must include a `Signed-off-by` trailer under the
[Developer Certificate of Origin 1.1](https://developercertificate.org/),
certifying that you have the right to submit the contribution under the
repository license and understand that the contribution and its identifying
metadata are public.
Create the trailer with:

```bash
git commit -s
```

The trailer must use the contributor identity associated with the work. When
several people created a commit, record each consenting contributor with an
appropriate `Co-authored-by` trailer and preserve each person's sign-off.

## Change And Review Expectations

1. Create a focused branch and link the applicable issue.
2. Add or update regression coverage for changed behavior.
3. Run focused tests and record exact commands and results.
4. Update lasting documentation, the changelog, compatibility limits, and
   localization catalogs when the change affects them.
5. Complete the pull-request template, including provenance, security,
   compatibility, documentation, and platform impact.
6. Address review feedback without rewriting another contributor's work or
   attribution.

Maintainers may ask for smaller scope, additional evidence, provenance
clarification, or independent review before merging. Submission does not
guarantee acceptance.

## Credit Corrections

Project credit follows [`CONTRIBUTORS.md`](CONTRIBUTORS.md). If authorship or
co-authorship metadata is missing or incorrect, request a correction in the
pull request before merge or use the documentation-feedback issue form.
