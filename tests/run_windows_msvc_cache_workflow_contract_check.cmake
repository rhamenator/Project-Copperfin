# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(read_contract_file relative_path output_variable)
    set(path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Required compiler-cache contract file is missing: ${relative_path}")
    endif()
    file(READ "${path}" contents)
    string(REPLACE "\r\n" "\n" contents "${contents}")
    set(${output_variable} "${contents}" PARENT_SCOPE)
endfunction()

function(require_text relative_path expected_text description)
    read_contract_file("${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing ${description}")
    endif()
endfunction()

function(forbid_text relative_path forbidden_text description)
    read_contract_file("${relative_path}" contents)
    string(FIND "${contents}" "${forbidden_text}" match_index)
    if(NOT match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} contains forbidden ${description}")
    endif()
endfunction()

function(require_text_count relative_path expected_text expected_count description)
    read_contract_file("${relative_path}" contents)
    string(LENGTH "${expected_text}" expected_length)
    string(LENGTH "${contents}" original_length)
    string(REPLACE "${expected_text}" "" stripped "${contents}")
    string(LENGTH "${stripped}" stripped_length)
    math(EXPR actual_count "(${original_length} - ${stripped_length}) / ${expected_length}")
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR
            "${relative_path} must contain ${expected_count} ${description}; found ${actual_count}")
    endif()
endfunction()

function(require_text_order relative_path first_text second_text description)
    read_contract_file("${relative_path}" contents)
    string(FIND "${contents}" "${first_text}" first_index)
    string(FIND "${contents}" "${second_text}" second_index)
    if(first_index EQUAL -1 OR second_index EQUAL -1 OR first_index GREATER second_index)
        message(FATAL_ERROR "${relative_path} has invalid ${description}")
    endif()
endfunction()

set(workflow ".github/workflows/windows-msvc-cache-evaluation.yml")
set(provenance_script "scripts/verify-windows-sccache.ps1")
set(environment_script "scripts/initialize-windows-msvc-environment.ps1")
set(stats_script "scripts/capture-windows-sccache-stats.ps1")
set(probe_script "scripts/test-windows-sccache-contract.ps1")
set(comparison_script "scripts/compare-windows-sccache-evidence.ps1")

require_text("${workflow}" "name: Windows MSVC Compiler Cache Evaluation"
    "stable workflow identity")
require_text("${workflow}" "on:\n  workflow_dispatch:"
    "manual-only trigger")
forbid_text("${workflow}" "\n  push:" "automatic push trigger")
forbid_text("${workflow}" "\n  pull_request:" "automatic pull-request trigger")
require_text("${workflow}" "permissions:\n  contents: read"
    "read-only repository permission")
require_text("${workflow}" "timeout-minutes: 120"
    "bounded hosted timeout")
require_text("${workflow}"
    "uses: actions/checkout@de0fac2e4500dabe0009e67214ff5f5447ce83dd # v6.0.2"
    "immutable checkout action")
require_text("${workflow}"
    "uses: actions/setup-dotnet@26b0ec14cb23fa6904739307f278c14f94c95bf1 # v5.4.0"
    "immutable setup-dotnet action")
require_text("${workflow}"
    "uses: mozilla-actions/sccache-action@9e7fa8a12102821edf02ca5dbea1acd0f89a2696 # v0.0.10"
    "immutable sccache setup action")
require_text("${workflow}"
    "uses: actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a # v7.0.1"
    "immutable evidence uploader")
require_text_count("${workflow}" "\n        uses: " 4 "third-party action references")

require_text("${workflow}" "version: v0.15.0" "fixed sccache version")
require_text("${workflow}"
    "SCCACHE_GHA_VERSION: copperfin-sccache-v0.15.0-msvc-x64-release-\${{ github.sha }}-epoch-\${{ inputs.cache_epoch }}"
    "source- and epoch-scoped cache namespace")
require_text("${workflow}" "SCCACHE_BASEDIRS: \${{ github.workspace }}"
    "workspace path normalization")
require_text("${workflow}" "SCCACHE_GHA_RW_MODE: READ_WRITE"
    "manual evaluation write mode")
require_text("${workflow}" "-G', 'Ninja'" "Ninja generator required by compiler launchers")
require_text_count("${workflow}" "-DCMAKE_C_COMPILER_LAUNCHER=\$env:SCCACHE_PATH" 2
    "cold and warm C launcher arguments")
require_text_count("${workflow}" "-DCMAKE_CXX_COMPILER_LAUNCHER=\$env:SCCACHE_PATH" 2
    "cold and warm CXX launcher arguments")
require_text_count("${workflow}" "-DCMAKE_POLICY_DEFAULT_CMP0141=NEW" 2
    "MSVC debug-format policy arguments")
require_text_count("${workflow}" "-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded" 2
    "cache-compatible MSVC debug-format arguments")
require_text_count("${workflow}" "-DCMAKE_CXX_FLAGS=/DWIN32 /D_WINDOWS" 2
    "MSVC platform flags without a global /EH option")
require_text_count("${workflow}" "-DCOPPERFIN_MSVC_EXPLICIT_EXCEPTION_HANDLING=ON" 2
    "per-target MSVC exception-mode arguments")
require_text("CMakeLists.txt"
    "target_compile_options(cf_xbase_runtime PRIVATE /W4 /permissive- /bigobj /we4530)"
    "target-scoped MSVC large-object and unwind-semantics enforcement")
require_text("CMakeLists.txt" "COPPERFIN_MANAGED_CPP"
    "native-only explicit /EHsc generator expression")
require_text("tests/CMakeLists.txt" "COPPERFIN_MANAGED_CPP TRUE"
    "managed C++ exception-mode exclusion")
forbid_text("tests/CMakeLists.txt" "/EHa"
    "redundant /EH option on the /clr target")
require_text("${workflow}" "build-sccache-cold" "separate cold build tree")
require_text("${workflow}" "build-sccache-warm" "separate warm build tree")
require_text("${workflow}" "Reset cold-build cache statistics"
    "cold configure/build statistics boundary")
require_text("${workflow}" "Reset warm-build cache statistics"
    "warm configure/build statistics boundary")
require_text_count("${workflow}" "-MaximumWriteErrors 1" 2
    "bounded cold and warm write-availability allowances")
require_text("${workflow}"
    "-CommandArguments @('--test-dir', 'build-sccache-warm', '--output-on-failure')"
    "full unfiltered warm CTest invocation")
require_text_order("${workflow}"
    "-Name 'Test compiler-cache invalidation and malformed entries'"
    "-Name 'Build cold native targets'"
    "early malformed-cache probe and cold-build ordering")
require_text_order("${workflow}"
    "-Name 'Test compiler-cache invalidation and malformed entries'"
    "-Name 'Run warm native test suite'"
    "malformed-cache probe and authoritative CTest ordering")
require_text("${workflow}" "-Category other"
    "supported invalidation-probe metric category")
forbid_text("${workflow}" "if: \${{ !cancelled() }}"
    "CTest execution before a skipped warm build")
forbid_text("${workflow}" "-Category cache-validation"
    "unsupported validation metric category")
forbid_text("${workflow}" "'-R'" "CTest filtering")
forbid_text("${workflow}" "--tests-regex" "CTest filtering")
require_text("${workflow}" "-MinimumImprovementPercent 25"
    "material repeat-build threshold")
require_text("${workflow}" "-MinimumColdMissPercent 90"
    "cold cache-population threshold")
require_text("${workflow}" "-MinimumWarmHitPercent 90"
    "warm cache-hit threshold")
require_text("${workflow}" "path: artifacts/windows-sccache-evaluation/*.json"
    "JSON-only evidence upload")
forbid_text("${workflow}" "path: build-sccache" "build-tree upload")
forbid_text("${workflow}" "actions/cache" "opaque build-output archive cache")

require_text("${provenance_script}"
    "e68e38e5b548f015dfc47c76d6cfbe67a610034408961f2b8693828b728999f8"
    "reviewed executable SHA-256")
require_text("${provenance_script}"
    "b0b257a164bf438b2dea134ca7ded41c100f59a64b3bf275a202f1e8102ab217"
    "reviewed release-archive SHA-256")
require_text("${provenance_script}"
    "setup_action_commit = \"9e7fa8a12102821edf02ca5dbea1acd0f89a2696\""
    "setup-action provenance evidence")
require_text("${environment_script}"
    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
    "required MSVC component lookup")
require_text("${environment_script}" "compiler_sha256"
    "compiler identity digest")
require_text("${stats_script}" "cache_read_errors"
    "cache read-error accounting")
require_text("${stats_script}" "cache_write_errors"
    "cache write-error accounting")
require_text("${stats_script}" "cacheable_requests"
    "cacheable-request accounting")
require_text("${stats_script}" "cache_read_errors"
    "separate fail-closed cache read-error accounting")
require_text("${stats_script}" "MaximumWriteErrors"
    "bounded cache write-error policy")
foreach(invalidation_marker IN ITEMS
        "source change"
        "generated-header change"
        "compiler-flag change"
        "build-configuration change"
        "target-architecture change"
        "compiler-identity change"
        "malformed cache object")
    require_text("${probe_script}" "${invalidation_marker}"
        "focused ${invalidation_marker} contract")
endforeach()
require_text("${probe_script}" "/Brepro" "deterministic malformed-object comparison")
require_text("${probe_script}" "\$env:llvmX64"
    "Visual Studio-owned compiler-identity probe")
require_text("${probe_script}"
    "function Stop-SccacheServer {\n    param([switch]\$Cleanup)\n\n    \$output = (& \$env:SCCACHE_PATH --stop-server 2>&1 | Out-String).Trim()\n    if (\$LASTEXITCODE -ne 0)"
    "stop-server-specific fail-closed shutdown check")
require_text("${probe_script}" "sccache could not stop its server cleanly"
    "actionable cache-server shutdown failure")
require_text("${probe_script}"
    "(?m)^sccache: error: couldn't connect to server\\r?\$"
    "exact already-stopped cache-server handling")
require_text("${probe_script}" "Stop-SccacheServer -Cleanup"
    "non-masking final cache-server cleanup")
require_text("${probe_script}" "\$malformedHits -eq 0 -and \$malformedMisses -eq 1"
    "malformed-entry cache-miss fallback requirement")
require_text("${probe_script}" "\$actualObjectSha256 -eq \$expectedObjectSha256"
    "malformed-entry deterministic output verification")
foreach(malformed_evidence_key IN ITEMS
        cache_hits
        cache_misses
        cache_read_errors
        expected_object_sha256
        actual_object_sha256)
    require_text("${probe_script}" "${malformed_evidence_key} ="
        "malformed-entry ${malformed_evidence_key} evidence")
endforeach()
require_text("${comparison_script}" "materially_faster"
    "durable material-improvement result")
require_text("${comparison_script}" "MinimumWarmHitPercent"
    "material cache-attribution threshold")

foreach(required_lane IN ITEMS
        .github/actions/native-validation/action.yml
        .github/workflows/native-validation-windows.yml
        .github/workflows/native-release-readiness.yml)
    forbid_text("${required_lane}" "sccache" "unproven compiler-cache adoption")
    forbid_text("${required_lane}" "build-sccache" "evaluation build tree")
endforeach()

message(STATUS "Windows MSVC compiler-cache workflow contract check passed")
