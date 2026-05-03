# Test Suite Edge Case Coverage Gap Analysis

**Version:** 1.0  
**Date:** 2026-05-03  
**Analyst:** Automated review against DO-178C structural coverage concepts (adapted for C++ runtime, no formal certification claimed)  
**Hazard register:** [docs/safety/hazard-register.md](hazard-register.md)  
**Scope:** All test files under `tests/` — 31 files, approximately 300 named test functions

---

## 1. Executive Summary

The current test suite has broad functional coverage of the happy path and several important robustness cases (runtime guardrails, truncated DBF rejection, ON ERROR/TRY/CATCH semantics, call-depth limits). However, several categories of edge case relevant to DO-178C structural coverage and hazard containment are absent or thin. This document enumerates those gaps, maps them to the hazard register, and proposes the minimal additional tests that would most meaningfully reduce certification risk.

**Overall assessment:** Functional coverage is strong. Boundary-value, fault-injection, and concurrency coverage are weak. Security subsystem coverage is shallow.

---

## 2. Coverage Strengths (Do Not Regress)

These areas are well-covered and must not be broken by future slices:

| Area | Key Tests |
| --- | --- |
| DBF field type round-trips | `test_create_dbf_table_file_round_trips`, `test_integer_field_*`, `test_currency_and_datetime_*`, `test_double_field_*`, `test_varchar_and_varbinary_*`, `test_memo_field_*` |
| DBF truncation rejection | `test_parse_dbf_table_rejects_truncated_visual_asset` |
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

- No test for integer overflow when the result of a REPLACE or CALCULATE expression exceeds the field's declared width (e.g., writing 100 to a 2-digit N field).
- No test for floating-point NaN or INF propagation through a chain of numeric expressions. VFP's behavior on NaN is documented but not exercised.
- No test for division by zero in a PRG expression (`x = 1 / 0`). The guardrail should produce a runtime error, not a crash.
- No test for CURRENCY field at maximum and minimum representable values (±$922,337,203,685,477.5807).
- No test for DOUBLE field with +INF, -INF, or denormalized input when writing and re-reading from a DBF.

**Priority:** High. A silently truncated field value is a data corruption scenario (HZ-data-corruption-01). Division by zero crashing the host is a runtime crash scenario (HZ-runtime-crash-01).

**Suggested new tests:**

```text
test_numeric_field_overflow_is_diagnosed_not_silently_truncated
test_division_by_zero_dispatches_runtime_error
test_nan_inf_in_double_field_round_trip_behavior
test_currency_field_boundary_values
```

---

### GAP-02 — Malformed / Adversarial DBF Header Inputs

**Hazard linkage:** HZ-data-corruption-01, HZ-runtime-crash-01

**What is missing:**

- `test_parse_dbf_table_rejects_truncated_visual_asset` covers one truncation scenario, but only for visual asset (.scx) files. There is no test for a raw `.dbf` file whose header claims N records but the file is too small to contain them (could cause an out-of-bounds read).
- No test for a header where the field descriptor count (computed from header size) exceeds the actual byte length.
- No test for mismatched `.dbf`/`.fpt` (memo sidecar) version bytes — the sidecar claims a different block size than the header expects.
- No test for a DBF where record size in the header does not match the sum of field widths.
- No test for a field descriptor with a name that is not null-terminated within its 11-byte slot.

**Priority:** High. Any of these could produce an out-of-bounds memory read if the parser trusts header metadata without validation. `test_visual_asset_memo_sidecar_repair_round_trip` and `test_memo_replace_recovers_directory_sidecar_path` show that repair paths exist, but they are exercised only in the "recoverable" case, not the "unrecoverable malformed input" case.

**Suggested new tests:**

```text
test_dbf_header_record_count_exceeds_file_size_is_rejected
test_dbf_field_descriptor_count_exceeds_header_size_is_rejected
test_dbf_record_width_mismatch_field_sum_is_rejected
test_memo_sidecar_version_mismatch_is_diagnosed
test_dbf_field_name_without_null_terminator_is_tolerated
```

---

### GAP-03 — Disk I/O Failure Injection

**Hazard linkage:** HZ-data-corruption-01, HZ-system-failure-01

**What is missing:**

- No test for write failure mid-REPLACE (simulated by writing to a read-only path or a file that becomes unwritable after the staged temp file is created). The critical question is whether the original `.dbf` is left intact if the temp-to-final rename fails.
- No test for partial memo sidecar write. If the DBF header write succeeds but the memo `.fpt` write fails, the table should not be left in a state where the DBF header points to a memo block that does not exist.
- `test_staged_write_temp_artifacts_are_cleaned_up` verifies that temp files do not leak on success, but does not test the abort/failure case.

**Priority:** High (S0 for HZ-data-corruption-01). These are exactly the scenarios that cause real data loss in production environments with power interruptions or full disks.

**Note:** Simulating disk full on Windows requires either a quota-limited volume or injecting a write failure at the file API level. A pragmatic alternative is to verify the staged-write contract at the unit level by testing the rollback function directly, without requiring a real disk-full scenario.

**Suggested new tests:**

```text
test_replace_write_failure_leaves_original_dbf_intact
test_memo_sidecar_write_failure_leaves_dbf_header_consistent
test_staged_write_rollback_removes_temp_and_preserves_original
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

```
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

```
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

```
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

```
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

This gap analysis was produced against commit state as of 2026-05-03. It should be re-run after any significant addition to the test suite or after a hazard register update changes the severity of an active hazard.

Re-run checklist:

- [ ] All GAP-01 through GAP-03 tests implemented
- [ ] GAP-04 through GAP-06 tests implemented
- [ ] Coverage measurement baseline established (GCOV or equivalent)
- [ ] This document updated to reflect closed gaps
- [ ] Hazard register reviewed for any new hazards introduced by runtime additions since this analysis
