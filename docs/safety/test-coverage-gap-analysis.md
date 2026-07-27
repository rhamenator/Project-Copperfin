# Test Suite Edge Case Coverage Gap Analysis

**Version:** 1.0  
**Date:** 2026-05-03  
**Analyst:** Automated review against DO-178C structural coverage concepts (adapted for C++ runtime, no formal certification claimed)  
**Hazard register:** [docs/safety/hazard-register.md](hazard-register.md)  
**Scope:** All test files under `tests/` — 31 files, approximately 300 named test functions

---

## 1. Executive Summary

The current test suite has broad functional coverage of the happy path and several important robustness cases (runtime guardrails, DBF malformed-input rejection, staged-write rollback, ON ERROR/TRY/CATCH semantics, call-depth limits). Several categories of edge case relevant to DO-178C structural coverage and hazard containment remain absent or thin. This document enumerates those gaps, maps them to the hazard register, and proposes the minimal additional tests that would most meaningfully reduce certification risk.

**Overall assessment:** Functional coverage is strong. DBF parser/write error management now has focused coverage for the highest-risk malformed-header and rollback cases. Remaining boundary-value, fault-injection, concurrency, and security-subsystem coverage is still uneven.

---

## 2. Coverage Strengths (Do Not Regress)

These areas are well-covered and must not be broken by future slices:

| Area | Key Tests |
| --- | --- |
| DBF field type round-trips | `test_create_dbf_table_file_round_trips`, `test_integer_field_*`, `test_currency_and_datetime_*`, `test_double_field_*`, `test_varchar_and_varbinary_*`, `test_memo_field_*` |
| DBF truncation rejection | `test_parse_dbf_table_rejects_truncated_visual_asset` |
| DBF malformed-header rejection | `test_dbf_header_record_count_exceeds_file_size_is_rejected`, `test_dbf_field_descriptor_count_exceeds_header_size_is_rejected`, `test_dbf_record_width_mismatch_field_sum_is_rejected`, `test_dbf_with_zero_record_length_is_rejected`, `test_dbf_with_header_shorter_than_minimum_is_rejected`, `test_dbf_header_claim_beyond_file_size_is_rejected` |
| DBF write failure rollback | `test_replace_write_failure_leaves_original_dbf_intact`, `test_memo_sidecar_write_failure_leaves_dbf_header_consistent`, `test_staged_write_rollback_removes_temp_and_preserves_original` |
| Staged write cleanup | `test_staged_write_temp_artifacts_are_cleaned_up` |
| Runtime guardrails | `test_runtime_guardrail_limits_call_depth_without_crashing_host`, `test_runtime_guardrail_limits_statement_budget_without_crashing_host`, `test_static_diagnostic_flags_likely_infinite_do_while_loop` |
| Error recovery semantics | `test_on_error_do_handler_dispatches_routine`, `test_try_catch_finally_handles_runtime_errors`, `test_retry_reexecutes_faulting_statement`, `test_resume_next_continues_after_fault` |
| Variable scope isolation | `test_private_declaration_masks_caller_variable`, `test_release_private_restores_saved_binding_immediately`, `test_release_local_restores_visible_outer_global` |
| SCATTER/GATHER round-trips | Extensive suite in `test_prg_engine_data_io.cpp` (25+ cases) |
| Null token handling | `test_replace_field_value_accepts_null_token_for_supported_types` |
| Shutdown sequence | `test_quit_closes_open_database_and_runtime_handles`, `test_on_shutdown_*` |

---

## 3. Coverage Gaps

### GAP-01 — Numeric Boundary and Overflow Conditions

**Hazard linkage:** HZ-data-corruption-01 (if overflow silently truncates a persisted field value)

**What is missing:**

- DBF currency max/min boundary values are covered by `test_currency_field_boundary_values`.
- DBF double NaN/+INF/-INF write/read survivability is covered by `test_nan_inf_in_double_field_round_trip_behavior`.
- Division by zero in a PRG expression is covered by `test_prg_engine_control_flow` regression coverage and dispatches a runtime error rather than crashing.
- Numeric field overflow via `REPLACE` is covered by runtime diagnostics/regression coverage proving the write is diagnosed or safely bounded.
- Remaining thin spot: floating-point NaN/INF propagation through longer PRG expression chains and denormalized DBF double inputs are not separately exercised.

**Priority:** High. A silently truncated field value is a data corruption scenario (HZ-data-corruption-01). Division by zero crashing the host is a runtime crash scenario (HZ-runtime-crash-01).

**Suggested new tests:**

```text
test_denormal_double_field_round_trip_behavior
test_nan_inf_prg_expression_chain_behavior
```

---

### GAP-02 — Malformed / Adversarial DBF Header Inputs

**Hazard linkage:** HZ-data-corruption-01, HZ-runtime-crash-01

**What is missing:**

- Raw `.dbf` record-count claims beyond available record storage are covered by `test_dbf_header_record_count_exceeds_file_size_is_rejected`.
- Declared header bounds for descriptor parsing are covered by `test_dbf_field_descriptor_count_exceeds_header_size_is_rejected`.
- Malformed memo sidecar block metadata is covered by `test_memo_sidecar_version_mismatch_is_diagnosed`.
- Record-size/field-layout mismatches are covered by `test_dbf_record_width_mismatch_field_sum_is_rejected`.
- Full-width field names without null terminators are covered by `test_dbf_field_name_without_null_terminator_is_tolerated`.
- Additional malformed header bounds are covered by `test_dbf_with_zero_record_length_is_rejected`, `test_dbf_with_header_shorter_than_minimum_is_rejected`, and `test_dbf_header_claim_beyond_file_size_is_rejected`.
- Remaining thin spot: asset inspection reports validation findings for some malformed DBF/memo sidecar inputs, but parser-level coverage does not yet assert structured validation issue codes for every malformed raw DBF case.

**Priority:** High. Any of these could produce an out-of-bounds memory read if the parser trusts header metadata without validation. `test_visual_asset_memo_sidecar_repair_round_trip` and `test_memo_replace_recovers_directory_sidecar_path` show that repair paths exist, but they are exercised only in the "recoverable" case, not the "unrecoverable malformed input" case.

**Suggested new tests:**

```text
test_inspect_asset_reports_record_width_mismatch_issue_code
test_inspect_asset_reports_descriptor_header_bounds_issue_code
```

---

### GAP-03 — Disk I/O Failure Injection

**Hazard linkage:** HZ-data-corruption-01, HZ-system-failure-01

**What is missing:**

- Injected DBF staged-promote failures are covered by `test_replace_write_failure_leaves_original_dbf_intact`.
- Injected memo sidecar write failures are covered by `test_memo_sidecar_write_failure_leaves_dbf_header_consistent`.
- Generic staged rollback artifact cleanup is covered by `test_staged_write_rollback_removes_temp_and_preserves_original`.
- Remaining thin spot: tests use deterministic test-only write failure checkpoints, not an OS-level disk-full or permission-denied integration environment.

**Priority:** High (S0 for HZ-data-corruption-01). These are exactly the scenarios that cause real data loss in production environments with power interruptions or full disks.

**Note:** Simulating disk full on Windows requires either a quota-limited volume or injecting a write failure at the file API level. A pragmatic alternative is to verify the staged-write contract at the unit level by testing the rollback function directly, without requiring a real disk-full scenario.

**Suggested new tests:**

```text
test_replace_permission_denied_leaves_original_dbf_intact
test_memo_sidecar_os_write_error_preserves_consistency
```

---

### GAP-04 — Empty / Degenerate Table Inputs

**Hazard linkage:** None direct, but contributes to HZ-runtime-crash-01 if a zero-record table causes a null dereference

**What is missing:**

- No test for `SCATTER MEMVAR` on a zero-record table (should produce all-empty variables without error).
- No test for `SCAN` on an empty table (body should not execute; `RECCOUNT()` should return 0).
- No test for `COUNT`, `SUM`, `AVERAGE` on an empty table (should return 0 without crashing).
- No test for `COPY TO` from an empty table (should produce a valid empty DBF, not a zero-byte file).
- No test for `LOCATE FOR` on an empty table (should set EOF() and produce a "not found" state, not crash).
- No test for `GO TOP` or `GO BOTTOM` on an empty table.

**Priority:** Medium. These are "degenerate but valid" inputs. VFP's documented behavior for all of them is well-defined; the runtime must match it without crashing.

**Suggested new tests:**

```text
test_scan_on_empty_table_does_not_execute_body
test_aggregate_commands_on_empty_table_return_zero
test_locate_on_empty_table_sets_eof
test_go_top_bottom_on_empty_table_does_not_crash
test_copy_to_from_empty_table_produces_valid_empty_dbf
```

---

### GAP-05 — String / Character Field Boundary Conditions

**Hazard linkage:** HZ-data-corruption-01 (silent truncation of longer-than-field-width values is a data integrity concern)

**What is missing:**

- No test for `REPLACE` of a `C` field with a value longer than the declared field width. VFP truncates silently; the runtime must do the same (or diagnose it, if that is the intended parity behavior).
- No test for a `C` field at its maximum declared width (254 bytes) with exact-fit data.
- No test for `REPLACE` of a memo field with an empty string (zero-byte memo block).
- No test for a field name at the 10-character limit (11th byte is null terminator).

**Priority:** Medium. The truncation case is important for parity accuracy; an off-by-one could cause systematic data loss.

**Suggested new tests:**

```text
test_replace_character_field_truncates_to_field_width
test_character_field_at_maximum_width_round_trips
test_memo_field_replace_with_empty_string
```

---

### GAP-06 — Security Subsystem Boundary Conditions

**Hazard linkage:** HZ-runtime-debug-01 (if authorization misconfiguration exposes debug state)

**What is missing:**

- `test_authorization` exercises one positive permission check and one negative. There is no test for:
  - Unknown role name (should return `false`, not throw or crash).
  - Empty permission ID (should return `false`).
  - Role with no permissions (should return `false` for all checks).
- `test_secret_provider` covers the `env:` prefix with a set environment variable. There is no test for:
  - A missing environment variable (the `env:` reference resolves to nothing).
  - An empty environment variable value.
  - A non-`env:` prefix that looks like a provider reference but uses an unsupported scheme.
- `test_audit_stream_chain` verifies that two events are persisted, but does not verify the hash chain. There is no test for:
  - Detecting that a log line has been tampered with (modified hash).
  - Detecting that a log line has been deleted (chain broken).
  - Appending to a read-only audit log path (should fail gracefully, not crash).

**Priority:** Medium. The audit stream tamper test is most important if the stream is intended to be forensically defensible (which the `append_immutable_audit_event` name implies).

**Suggested new tests:**

```text
test_authorization_unknown_role_returns_false
test_authorization_empty_permission_returns_false
test_secret_provider_missing_env_var_returns_not_ok
test_audit_stream_tamper_detection
test_audit_stream_append_to_readonly_path_fails_gracefully
```

---

### GAP-07 — Guardrail Boundary Precision

**Hazard linkage:** HZ-runtime-crash-01 (if the limit is off-by-one and the stack overflows before the guard fires)

**What is missing:**

- `test_runtime_guardrail_limits_call_depth_without_crashing_host` verifies that exceeding the limit does not crash, but does not verify that exactly-at-limit calls succeed and exactly-one-over-limit calls fail.
- `test_runtime_guardrail_limits_statement_budget_without_crashing_host` has the same gap.
- No test for `test_config_fpw_overrides_runtime_limits` verifying that the override takes effect at the new limit boundary, not just that the override parses.

**Priority:** Low-Medium. The existing tests are "does not crash" tests. Boundary-precision tests ("limit - 1 succeeds, limit + 1 fails, limit equals exact-limit behavior") are more relevant to DO-178C MC/DC coverage than functional correctness.

**Suggested new tests:**

```text
test_runtime_guardrail_exactly_at_call_depth_limit_succeeds
test_runtime_guardrail_one_over_call_depth_limit_fails
test_config_fpw_custom_limit_is_enforced_at_boundary
```

---

### GAP-08 — Multi-Session / Work Area Isolation

**Hazard linkage:** HZ-data-corruption-01 (if two open work areas on the same table see inconsistent state)

**What is missing:**

- No test opening the same `.dbf` file in two different work areas simultaneously and verifying that a REPLACE in one work area is correctly reflected (or correctly isolated, depending on data session semantics) in the other.
- `test_work_area_and_data_session_compatibility` exists in `test_prg_engine.cpp` but its details were not reviewed; if it covers this scenario, this gap may be partially closed.
- No test for a transaction (BEGIN TRANSACTION / ROLLBACK) leaving the table in its pre-transaction state after rollback.

**Priority:** Medium. Multi-session isolation is a correctness concern that becomes safety-relevant if two concurrent callers can corrupt each other's data.

**Suggested new tests:**

```text
test_two_work_areas_on_same_table_see_consistent_mutations
test_transaction_rollback_leaves_table_unchanged
```

---

### GAP-09 — Diagnostic and Error Reporting Completeness

**Hazard linkage:** HZ-runtime-debug-01

**What is missing:**

- No test verifying that a fault raised inside a deeply nested TRY/CATCH stack correctly surfaces the innermost faulting line (versus the outermost catch site).
- `test_nested_routine_faults_report_faulting_stack_frame_line` and `test_aerror_populates_structured_runtime_error_array` cover parts of this, but there is no test that verifies AERROR row 5 (the line number) is the line inside the nested call, not the line of the CATCH.
- No test for AERROR content when the error originates from a SQL pass-through function versus a pure VFP expression fault.

**Priority:** Low. Diagnostic accuracy is important for operator confidence but not directly safety-critical unless a misleading error message causes an operator to take the wrong recovery action (HZ-runtime-debug-01).

---

## 4. Gap Priority Summary

| Gap ID | Area | Hazard(s) | Priority | Suggested Tests |
| --- | --- | --- | --- | --- |
| GAP-01 | Numeric overflow / NaN / zero-divide | HZ-data-corruption-01, HZ-runtime-crash-01 | **High** | 4 |
| GAP-02 | Malformed DBF header inputs | HZ-data-corruption-01, HZ-runtime-crash-01 | **High** | 5 |
| GAP-03 | Disk I/O failure injection | HZ-data-corruption-01, HZ-system-failure-01 | **High** | 3 |
| GAP-04 | Empty / degenerate table inputs | HZ-runtime-crash-01 | **Medium** | 5 |
| GAP-05 | String field boundary conditions | HZ-data-corruption-01 | **Medium** | 3 |
| GAP-06 | Security subsystem boundary | HZ-runtime-debug-01 | **Medium** | 5 |
| GAP-07 | Guardrail boundary precision | HZ-runtime-crash-01 | **Low-Medium** | 3 |
| GAP-08 | Multi-session / work area isolation | HZ-data-corruption-01 | **Medium** | 2 |
| GAP-09 | Diagnostic error reporting depth | HZ-runtime-debug-01 | **Low** | 1 |

**Total suggested new tests:** 31 (across all gaps)

---

## 5. Recommended Implementation Order

Highest-risk gaps first (following the HZ-data-corruption-01 priority from the hazard register):

1. **GAP-03** (disk I/O failure) — most likely to cause real data loss in production; implement staged-write rollback unit test first as it does not require OS-level disk-full simulation.
2. **GAP-01** (numeric overflow / zero-divide) — crash and silent-truncation risk; straightforward to implement in the PRG engine test harness.
3. **GAP-02** (malformed DBF header) — requires crafting adversarial byte arrays; builds on existing pattern in `test_dbf_table.cpp`.
4. **GAP-04** (empty table inputs) — low implementation cost; high VFP parity value.
5. **GAP-06** (security boundary) — important for audit forensic claim; moderate cost.
6. **GAP-05** (string boundary) — VFP parity; low cost.
7. **GAP-08** (multi-session isolation) — higher setup cost; depends on work area session model completeness.
8. **GAP-07** (guardrail boundary precision) — low value relative to cost; defer until other gaps are closed.
9. **GAP-09** (diagnostic depth) — low value; defer.

---

## 6. DO-178C Structural Coverage Notes (Advisory Only)

This project is not pursuing formal DO-178C certification. However, the following observations are offered for any future certification effort:

- **MC/DC coverage** (Modified Condition/Decision Coverage, required for DO-178C Level A/B) would require that every boolean condition in the runtime individually affect the overall decision outcome. The current test suite exercises most decision paths but does not verify that each individual condition was exercised independently. A formal MC/DC analysis would require instrumentation (e.g., GCOV with branch coverage reporting).
- **Statement coverage** is likely high for the PRG engine command dispatch paths given the breadth of the functional test suite, but has not been measured. Running `cmake --build build --target RUN_TESTS` with GCOV enabled would give a baseline measurement.
- **Data flow coverage** (def-use pairs) is not explicitly addressed. The SCATTER/GATHER round-trip tests provide implicit def-use coverage for field access paths, but complex expression chains are not systematically exercised at the def-use level.
- **Deactivated code** — the codebase uses `#if defined(_WIN32)` guards. The non-Windows paths are compiled in CI but may not be exercised by all test cases. Any certification effort should verify that CI runs tests on at least one non-Windows target.

---

## 7. Traceability

This section was refreshed against the current main tree on 2026-07-21. The original gap descriptions remain useful as hazard-oriented test intent, but the old checklist was stale: focused regressions have since been added across the DBF, PRG, security, and runtime-host targets.

### Current focused evidence

| Gap | Current evidence | Status and limitation |
| --- | --- | --- |
| GAP-01 | `tests/test_dbf_table.cpp` covers currency and NaN/INF boundaries; `tests/test_prg_engine_control_flow_error_handling_and_faults.cpp` covers divide-by-zero recovery and numeric-field overflow. | Focused regression coverage is present. This is not a formal numeric-boundary completeness claim. |
| GAP-02 | `tests/test_dbf_table.cpp` covers adversarial record counts, descriptor/header bounds, record-width mismatches, malformed memo metadata, and full-width field names. | Focused malformed-input coverage is present. Fuzzing and exhaustive format coverage remain out of scope. |
| GAP-03 | `tests/test_dbf_table.cpp` covers injected DBF promotion failures, memo-sidecar failures, rollback, and staged-artifact cleanup. | Deterministic failure-injection coverage is present; physical disk-full and power-loss simulation are not claimed. |
| GAP-04 | `tests/test_prg_engine_control_flow.cpp` and the aggregate/table-mutation test sources cover empty-table navigation, scans, aggregates, and zero-record copy/append paths. | Focused empty/degenerate cases are present; the full command surface is not exhaustive. |
| GAP-05 | `tests/test_dbf_table.cpp` covers character-width, memo, currency, and numeric field boundaries; PRG string and replacement tests cover runtime string boundaries. | Representative persistence and runtime coverage is present; broader code-page combinations remain a future parity slice. |
| GAP-06 | `tests/test_security_controls.cpp` covers secret, audit, and external-process policy boundaries; the `tests/test_runtime_host_audit_*.cpp` shard family covers runtime audit containment and malformed security metadata. | Focused security-boundary evidence is present; hosted Windows ACL and process-policy validation remains a release gate. |
| GAP-07 | `tests/test_prg_engine_control_flow_runtime_guardrails_and_lifecycle.cpp` covers the configured call-depth boundary and failure path. | The stack-frugal iterative frame-machine contract is tested at the guardrail, not formally proven for every nested feature. |
| GAP-08 | `tests/test_prg_engine_work_areas.cpp` covers multi-alias/session visibility and mutation persistence. | Representative work-area isolation coverage is present; broader concurrent-session analysis remains advisory. |
| GAP-09 | PRG error-handling, runtime-host debug-output, localization, and audit tests cover stable diagnostic identity, localized prose, and recovery output. | Diagnostic contracts are covered for shipped surfaces; no claim is made that every future diagnostic has been inventoried. |

The focused evidence above does not replace the required cross-platform validation matrix or the safety traceability workflow. Before a release tag, run `scripts/validate-safety-traceability.ps1` or the Safety Traceability Gate workflow against the intended release issue set and archive its report.

Remaining checklist:

- [x] GAP-01 through GAP-09 have current focused evidence references
- [ ] Coverage measurement baseline established (GCOV or equivalent)
- [x] This document refreshed to reflect current focused evidence
- [ ] Hazard register reviewed for any new hazards introduced by runtime additions since this refresh
