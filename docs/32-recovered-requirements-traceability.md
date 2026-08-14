# Recovered Requirements Traceability

## Purpose

This document is the durable low-level requirement (LLR) to code to test matrix
for requirements recovered after implementation. It implements the Requirements
Recovery Principle in `docs/01-product-charter.md` without treating Copperfin's
existing behavior as its own requirement source.

Allowed requirement evidence is limited to observed behavior from a real,
installed VFP9 environment, shipped Microsoft/VFP documentation, explicit
repository-owner product policy, and registered known-bug or crash exceptions
in `docs/27-known-vfp9-bug-exceptions.md`.
Decompiled or disassembled VFP binaries are prohibited inputs under
`docs/07-clean-room-rules.md`.

## Status Values

- `recovered`: the requirement has allowed source evidence and mapped code and
  test evidence.
- `gap`: allowed source evidence exists, but code or test evidence is missing or
  contradicts the requirement. Track implementation in a separate prompt-sized
  issue.
- `exception`: the behavior is intentionally different and has an applied
  `KBX-*` entry in `docs/27-known-vfp9-bug-exceptions.md`.

## Traceability Matrix

| LLR ID | Recovered low-level requirement | Allowed source evidence | Code | Tests | Verification | Status | Issue |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `LLR-VFP-NUM-001` | PRG numeric source and calculation text shall use a period as the decimal point independently of the host C/C++ locale or the VFP display-point setting. Comma-decimal text shall not be silently accepted as the same numeric literal. Machine numeric text routed back into parser/runtime contracts shall remain period-decimal and ungrouped. | Shipped VFP9 SP2 Help: [Numeric Data Type](https://vfphelp.com/vfp9/html/f0945c58-08e4-46fc-a15b-e1714a064d91.htm) defines numeric values using digits, an optional sign, and a decimal point. [SET POINT Command](https://www.vfphelp.com/help/html/ab6ea03e-d7f8-4ddb-b2e2-56755efd8857.htm) states that `SET POINT` changes display but calculations must use a period. | `include/copperfin/platform/invariant_numeric.h`; `src/platform/invariant_numeric.cpp`; `src/runtime/prg_engine_expression.inl` (`parse_number`) | `tests/test_prg_engine_parser_classes.cpp` (`test_invariant_numeric_parser_preserves_vfp_decimal_contract`, comma-decimal preprocessor locale test); `tests/test_prg_engine_data_io_save_restore.cpp` (`test_restore_from_parses_numeric_values_invariantly`) | macOS: both CTest targets under `C`, `en_US.UTF-8`, `pt_BR.UTF-8`, and `de_DE.UTF-8` (8/8). Linux seq1419: source/evidence mapping pass plus both targets under default, `C`, and `en_US.utf8` (6/6); `pt_BR`/`de_DE` were not installed and macOS supplies those executions. | `recovered` | closed `#4896` |
| `LLR-CF-ASSURANCE-001` | During solo-maintainer development, safety-documentation changes classified `none`, `low`, or `medium` shall accept explicit maintainer self-review plus applicable automated verification without claiming independence. Review verification and automation must be meaningful completed evidence. Changes classified `high` or `catastrophic` shall require a second qualified human reviewer before closure, evidenced by exactly one rendered structured sign-off section in a comment authored by that reviewer's GitHub account and bound to the exact reviewed issue-body SHA-256. Code blocks and HTML comments are not evidence; any top-level raw-HTML construct makes the sign-off comment fail closed, and ATX heading markers require following whitespace. The latest applicable sign-off by that reviewer for the same body digest shall supersede earlier sign-offs, including withdrawal of approval. Completed-project or first-stable-release readiness retains its independent-human-review gate. | Explicit repository-owner policy, 2026-08-14; `docs/DO-178C-ASSURANCE-POLICY.md`; `docs/RELEASE-READINESS-REVIEW.md` | `agents.md`; `.github/ISSUE_TEMPLATE/safety-critical-documentation-change.yml`; `scripts/validate-safety-traceability.ps1`; `docs/safety/triage-rubric.md` | `tests/run_safety_traceability_workflow_contract_check.cmake`; self-review, approved-independent, withdrawn-independent, duplicate-sign-off-section, non-rendered-sign-off, ATX-heading, same-author, unattested, incomplete, negated-result, placeholder-reviewer, issue-form-heading, stale-body-digest, placeholder/negated/unavailable sign-off, placeholder/failed/outcome-failure self-review, affirmative-`does not`/`never` guarantee, and live-snapshot source-contract cases; legacy independent-review fixtures | Focused safety-traceability workflow contract proves structured low-severity self-review; rejects high-severity self-review, author-as-independent-reviewer, unattested login claims, stale body digests, later reviewer withdrawal, malformed ATX headings, duplicate or non-rendered sign-off sections, punctuation-separated status negation, placeholder/negated/unavailable qualification or verification, placeholder/failed/outcome-failure self-review verification or automation, negated legacy results, and incomplete evidence; retains affirmative `does not`/`never` safety guarantees and legitimate failure-boundary scope prose; requires a post-comment stable live issue snapshot; accepts approved high-severity review only with a distinct reviewer-authored sign-off carrying meaningful qualification and verification evidence bound to the exact current issue body; preserves GitHub issue-form and authenticated legacy evidence compatibility; and retains mapping and hostile-input validation | `recovered` | repository-owner directive, 2026-08-14 |

## LLR-VFP-NUM-001 Evidence Boundary

The shipped help directly establishes two facts: VFP numeric/calculation syntax
uses a decimal point, and changing the displayed point does not change the
period required in calculations. The host-locale isolation clause is the
portable implementation consequence of those facts: Copperfin must not let the
embedding C/C++ locale reinterpret the VFP token grammar.

This row does **not** require displayed numeric text to ignore `SET POINT`,
`SET SYSFORMATS`, or other VFP formatting state. It also does not claim that all
string-to-number functions share source-literal syntax. Those surfaces require
separate recovered requirements and evidence.

The shared double parser requires full input consumption, rejects leading
whitespace, malformed signs, comma decimals, trailing text, overflow, and
nonfinite values unless a consuming binary-field contract explicitly opts in.
On Apple libc++, where floating-point `std::from_chars` may be unavailable, the
fallback stream is explicitly imbued with `std::locale::classic()` and uses
`std::noskipws`, preserving the same invariant contract.

Independent Linux review at channel sequence 1419 fetched and checked both
shipped-help pages, verified every mapped code and test location, and ran the
actual `test_prg_engine_parser_classes` and `test_prg_engine_data_io` targets.
The latter is the CMake target containing
`test_prg_engine_data_io_save_restore.cpp`; the source shard is not a standalone
test target. The Linux host's missing `pt_BR` and `de_DE` locales are disclosed
rather than inferred as executions. Linux source review confirmed the mapped
comma-decimal parser test constructs its locale facet in process, while the
macOS matrix supplies both installed locale runs.
