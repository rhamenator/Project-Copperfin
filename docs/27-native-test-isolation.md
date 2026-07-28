# Native Test Isolation

This document defines the CTest isolation evidence required before Copperfin runs native tests concurrently. Build parallelism is governed separately; this policy applies only after all native targets have been built.

## Inventory

`tests/CopperfinTestIsolation.cmake` classifies every test registered in the current platform configuration. Configuration writes `native-test-isolation.tsv` at the build root, including SHA-256 identities for the classification and test-registration sources. `test_native_test_isolation_contract` rejects stale inventory, verifies its schema, unique test identities, required risk axes, allowed values, and agreement between the schedule label and CTest `RUN_SERIAL` property.

Every test has exactly one value for each axis:

- `filesystem`: no access, read-only access, an owned or unique mutable root, or a shared/fixed mutable root
- `environment`: no mutation, process-scoped mutation, child-only mutation, or unverified behavior
- `child-processes`: none, bounded/reaped children, or unverified behavior
- `network`: none, deliberately disabled probes, possible package restore, or unverified behavior
- `samples`: none, read-only samples, an owned copy, or unverified behavior
- `platform`: portable or an explicit platform/tool dependency
- `resources`: no shared global resource, an explicit CTest lock, or unverified behavior
- `audit`: `complete` only after source-backed review of all preceding axes; otherwise `pending`
- `schedule`: `parallel-safe` or `serial`

CTest labels use the stable `copperfin-isolation:<axis>=<value>` form. These are test-policy identities, not product strings or runtime contracts.

The current audit explicitly covers the pure polyglot decision/telemetry tests, the read-only polyglot contract checks, and the unique-temporary-root `test_prg_engine_relations` suite. Those targets are classified as complete and parallel-safe; the inventory contract remains fail-closed for any future registration that has not received the same source-backed review.

## Fail-Closed Default

A newly registered or unaudited test receives `unverified` risk values, `audit=pending`, `schedule=serial`, and `RUN_SERIAL=TRUE`. A pending test therefore runs alone even when the caller later uses bounded `ctest --parallel` execution.

Promotion to `parallel-safe` requires source evidence that:

1. every mutable filesystem root is unique among concurrently eligible CTest names or protected by a named CTest resource lock
2. any child process is awaited or terminated and cannot retain inherited output or files
3. environment and current-directory mutation is process-local and restored on every exit path
4. source samples are read-only or copied before mutation
5. no fixed port, global service, build output, package cache, or other cross-process resource can collide
6. failure and cleanup paths preserve the same isolation as success paths

Process-local environment mutation is not a cross-CTest collision by itself because each CTest command runs in a separate process. A fixed temporary root may be safe among distinct test names when the audited roots are unique, but it does not prove that two concurrent instances of the same test are safe. Tests expected to support duplicate concurrent instances should use process-owned roots.

## Known Serial Boundaries

- `test_managed_compile` writes shared managed `bin`/`obj` trees and may restore packages.
- `test_build_parallelism_contract` uses one fixed build-tree metrics root.
- `test_studio_host_json` is the monolithic Studio-host suite and reuses the same function-owned roots as its specialized CTest executables, so the monolith runs alone.
- Specialized Studio-host targets that compile shared fixture sources containing fixed roots use the `copperfin-studio-host-shared-fixtures` resource lock. They remain parallel-eligible against unrelated tests but cannot overlap each other.
- `test_visual_asset_editor_code_page` and `test_studio_host_code_page` invoke the same executable against the same fixed root, so both use the `copperfin-visual-asset-code-page-root` resource lock.
- `test_prg_engine` writes the fixed compatibility-corpus build roots and launches the PowerShell corpus exporter.
- `test_generated_launcher_process` and `test_runtime_pipeline` launch managed/native toolchains; they remain serial while cleanup and package-restore behavior are deliberately conservative.
- `test_security_controls` combines fixed roots, environment mutation, and internal concurrency; `test_vfp_assets` combines fixed roots with optional sample probing. Both remain serial.
- Tests with colliding fixed mutable roots remain serial until those roots become process-owned or a narrower resource lock is justified. A fixed root unique to one CTest identity may be classified `test-owned-unique`, but duplicate concurrent invocations of that identity are not supported.

The current audit found no native test that owns a fixed TCP/UDP port. Adding one requires an explicit network classification and either process-owned allocation or a CTest resource lock.

## Bounded Trials

Do not replace serial CTest with bare or unbounded parallel execution. Windows validation uses an explicit two-job cap and retains the complete test inventory, `--output-on-failure`, and phase timing. The manual Windows Deep Validation workflow permits one or two test jobs and defaults to the adopted two-job policy.

The adoption evidence consists of three successful hosted Windows Deep Validation runs at commit `84c8c2b3`. All passed 282/282 tests. Native CTest elapsed times were 342.27, 350.47, and 313.44 seconds versus the 449.70-second serial baseline, for a 335.39-second mean (25.42% reduction) and 342.27-second median (23.89% reduction). Minimum free memory remained above 12 GiB and peak tracked working set remained below 1 GiB. Linux and macOS remain serial until each platform has independent evidence.

Windows validation also passes CTest `--timeout 180`. This is a fail-fast diagnostic boundary for a hung or deadlocked individual test; it does not remove tests, reduce the native inventory, or change the bounded two-job policy. The same timeout is required by the manual Windows Deep Validation workflow contract.
