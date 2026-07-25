# Copyright © 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(read_contract_file relative_path output_variable)
    set(workflow_path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${workflow_path}")
        message(FATAL_ERROR "Required native validation workflow is missing: ${relative_path}")
    endif()

    file(READ "${workflow_path}" contents)
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

function(require_regex relative_path expected_regex description)
    read_contract_file("${relative_path}" contents)
    string(REGEX MATCH "${expected_regex}" match "${contents}")
    if("${match}" STREQUAL "")
        message(FATAL_ERROR "${relative_path} is missing ${description}")
    endif()
endfunction()

function(forbid_regex relative_path forbidden_regex description)
    read_contract_file("${relative_path}" contents)
    string(REGEX MATCH "${forbidden_regex}" match "${contents}")
    if(NOT "${match}" STREQUAL "")
        message(FATAL_ERROR "${relative_path} contains forbidden ${description}: ${match}")
    endif()
endfunction()

function(require_regex_count relative_path expected_regex expected_count description)
    read_contract_file("${relative_path}" contents)
    string(REGEX MATCHALL "${expected_regex}" matches "${contents}")
    list(LENGTH matches actual_count)
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR
            "${relative_path} must contain ${expected_count} ${description}; found ${actual_count}")
    endif()
endfunction()

function(require_text_count relative_path expected_text expected_count description)
    read_contract_file("${relative_path}" contents)
    string(LENGTH "${expected_text}" expected_length)
    if(expected_length EQUAL 0)
        message(FATAL_ERROR "Cannot count an empty contract string")
    endif()

    string(LENGTH "${contents}" original_length)
    string(REPLACE "${expected_text}" "" stripped_contents "${contents}")
    string(LENGTH "${stripped_contents}" stripped_length)
    math(EXPR removed_length "${original_length} - ${stripped_length}")
    math(EXPR actual_count "${removed_length} / ${expected_length}")
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR
            "${relative_path} must contain ${expected_count} ${description}; found ${actual_count}")
    endif()
endfunction()

function(validate_artifact_upload_paths relative_path)
    set(workflow_path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${workflow_path}")
        message(FATAL_ERROR "Required artifact workflow is missing: ${relative_path}")
    endif()
    file(STRINGS "${workflow_path}" lines)
    set(in_path_block FALSE)
    set(path_block_indent 0)
    set(allowed_build_artifact
        [=[build/${{ inputs.build_configuration }}/*.exe]=])
    foreach(line IN LISTS lines)
        set(candidate "")
        if(line MATCHES "^([ \t]*)path:[ \t]*(.*)$")
            string(LENGTH "${CMAKE_MATCH_1}" path_block_indent)
            set(candidate "${CMAKE_MATCH_2}")
            string(STRIP "${candidate}" candidate)
            if(candidate STREQUAL "|")
                set(in_path_block TRUE)
                continue()
            endif()
            set(in_path_block FALSE)
        elseif(in_path_block)
            if(line MATCHES "^([ \t]*)(.*)$")
                string(LENGTH "${CMAKE_MATCH_1}" line_indent)
                set(candidate "${CMAKE_MATCH_2}")
                string(STRIP "${candidate}" candidate)
                if(candidate STREQUAL "")
                    continue()
                endif()
                if(line_indent LESS_EQUAL path_block_indent)
                    set(in_path_block FALSE)
                    continue()
                endif()
            endif()
        else()
            continue()
        endif()

        if(candidate MATCHES "CMakeCache\\.txt" OR candidate MATCHES "CMakeFiles")
            message(FATAL_ERROR
                "${relative_path} publishes CMake build metadata: ${candidate}")
        endif()
        if(candidate STREQUAL "build" OR candidate STREQUAL "build/" OR
                (candidate MATCHES "^build/" AND
                 NOT candidate STREQUAL "${allowed_build_artifact}" AND
                 NOT candidate MATCHES "^build/package/copperfin-[^/]+\\.(exe|zip|pkg|deb|rpm|tar\\.gz)$"))
            message(FATAL_ERROR
                "${relative_path} publishes a reusable CMake build tree: ${candidate}")
        endif()
    endforeach()
endfunction()

function(check_caller relative_path workflow_name platform runner check_name)
    set(trigger_contract [=[on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  workflow_dispatch:]=])
    if(ARGC GREATER 5)
        set(trigger_contract "${ARGV5}")
    endif()

    require_regex("${relative_path}"
        "^name:[ \t]*${workflow_name}[ \t]*\n"
        "the stable top-level name '${workflow_name}'")

    require_text("${relative_path}" "${trigger_contract}"
        "push/pull_request main and workflow_dispatch triggers")
    require_text("${relative_path}" [=[permissions:
  contents: read]=]
        "top-level read-only contents permission")
    require_regex("${relative_path}"
        "\nconcurrency:\n  group:[^\n]*${platform}[^\n]*github\\.event_name[^\n]*\n  cancel-in-progress:[ \t]*true([ \t]*\n|$)"
        "event- and platform-scoped concurrency with cancellation")

    require_regex_count("${relative_path}"
        "\n[ \t]+uses:[ \t]*\\./\\.github/actions/native-validation[ \t]*\n"
        1 "local shared-action reference")
    require_regex_count("${relative_path}" "\n[ \t]+uses:[ \t]*" 2 "uses entries")
    require_text_count("${relative_path}"
        "uses: actions/checkout@de0fac2e4500dabe0009e67214ff5f5447ce83dd # v6.0.2"
        1 "immutable checkout action")

    require_regex("${relative_path}"
        "\n  [A-Za-z_][A-Za-z0-9_-]*:\n    name: ${check_name}\n    runs-on: ${runner}\n    timeout-minutes: 120\n    steps:"
        "stable '${check_name}' job identity, runner, and timeout")
    require_regex("${relative_path}"
        "\n        uses: \\./\\.github/actions/native-validation\n        with:\n          platform: ${platform}([ \t]*\n|$)"
        "exact shared-action input for ${platform}")

    forbid_regex("${relative_path}" "\n[ \t]+run:[ \t]*" "caller shell commands")
    forbid_text("${relative_path}" "cmake " "duplicated CMake command")
    forbid_text("${relative_path}" "ctest " "duplicated CTest command")
endfunction()

set(shared_action ".github/actions/native-validation/action.yml")
set(legacy_workflow "${SOURCE_DIR}/.github/workflows/native-validation.yml")
set(legacy_reusable_workflow
    "${SOURCE_DIR}/.github/workflows/native-validation-reusable.yml")

if(EXISTS "${legacy_workflow}")
    message(FATAL_ERROR
        ".github/workflows/native-validation.yml must be removed after splitting native validation")
endif()
if(EXISTS "${legacy_reusable_workflow}")
    message(FATAL_ERROR
        "The reusable-workflow form changes required-check identities; use the shared composite action")
endif()

check_caller(
    ".github/workflows/native-validation-linux.yml"
    "Linux Native Validation"
    "linux"
    "ubuntu-latest"
    "Linux GCC")
check_caller(
    ".github/workflows/native-validation-macos.yml"
    "macOS Native Validation"
    "macos"
    "macos-latest"
    "macOS Clang")
check_caller(
    ".github/workflows/native-validation-windows.yml"
    "Windows Native Validation"
    "windows"
    "windows-latest"
    "Windows MSVC"
    [=[on:
  push:
    branches: [main]
    paths-ignore:
      - ".agent-channel/**"
      - "docs/**"
      - "**/*.md"
      - "**/*.txt"
  pull_request:
    branches: [main]
    paths-ignore:
      - ".agent-channel/**"
      - "docs/**"
      - "**/*.md"
      - "**/*.txt"
  workflow_dispatch:]=])

set(release_workflow ".github/workflows/native-release-readiness.yml")
require_text("${release_workflow}" [=[name: Native Release Readiness

on:
  workflow_dispatch:]=]
    "manual-only native release trigger")
require_text("${release_workflow}" [=[permissions:
  contents: read]=]
    "read-only native release permission")
foreach(release_contract IN ITEMS
        "linux-native-validation|Release Linux GCC|ubuntu-latest|linux"
        "macos-native-validation|Release macOS Clang|macos-latest|macos"
        "windows-native-validation|Release Windows MSVC|windows-latest|windows")
    string(REPLACE "|" ";" release_fields "${release_contract}")
    list(GET release_fields 0 release_job)
    list(GET release_fields 1 release_name)
    list(GET release_fields 2 release_runner)
    list(GET release_fields 3 release_platform)
    require_text("${release_workflow}"
        "  ${release_job}:\n    name: ${release_name}\n    runs-on: ${release_runner}\n    timeout-minutes: 120"
        "release job '${release_name}'")
    require_text("${release_workflow}"
        "uses: ./.github/actions/native-validation\n        with:\n          platform: ${release_platform}"
        "shared release contract for ${release_platform}")
endforeach()
require_text_count("${release_workflow}"
    "uses: actions/checkout@de0fac2e4500dabe0009e67214ff5f5447ce83dd # v6.0.2"
    3 "immutable release checkout actions")
require_text("${release_workflow}" [=[  native-release-readiness:
    name: Native Release Readiness
    needs:
      - linux-native-validation
      - macos-native-validation
      - windows-native-validation]=]
    "final gate dependent on all three native platforms")
forbid_text("${release_workflow}" "actions/upload-artifact" "release build-tree upload")
forbid_text("${release_workflow}" "actions/download-artifact" "release build-tree reuse")
forbid_text("${release_workflow}" "cmake " "duplicated release CMake command")
forbid_text("${release_workflow}" "ctest " "duplicated release CTest command")

foreach(preserved_workflow IN ITEMS
        .github/workflows/windows-environment-validation.yml
        .github/workflows/build-vsix.yml
        .github/workflows/build-installers.yml
        .github/workflows/windows-deep-validation.yml
        .github/workflows/windows-x86-declare-validation.yml)
    if(NOT EXISTS "${SOURCE_DIR}/${preserved_workflow}")
        message(FATAL_ERROR
            "Focused product/platform workflow must remain separate: ${preserved_workflow}")
    endif()
endforeach()

require_text(".github/workflows/windows-environment-validation.yml"
    "name: Windows Environment and Executable Path Validation"
    "focused Windows environment workflow identity")
require_text(".github/workflows/windows-environment-validation.yml"
    "--target test_platform_environment test_localization test_licensing_status test_build_host_output test_runtime_host_implicit_path_launch test_tool_license_path_launch test_studio_host_report_section_selection_diagnostics"
    "focused Windows environment target inventory")
require_text(".github/workflows/windows-environment-validation.yml"
    [=[cmake -S . -B "$env:RUNNER_TEMP/copperfin-environment-build" -DCOPPERFIN_BUILD_TESTS=ON]=]
    "out-of-tree configure command")
require_text(".github/workflows/windows-environment-validation.yml"
    [=[ctest --test-dir "$env:RUNNER_TEMP/copperfin-environment-build" -C Release --output-on-failure -R "^(test_platform_environment|test_localization|test_licensing_status|test_build_host_output|test_runtime_host_implicit_path_launch|test_tool_license_path_launch|test_studio_host_report_section_selection_diagnostics)$"]=]
    "out-of-tree localization, runtime-host, and Studio-host CTest contract")
require_text(".github/workflows/build-vsix.yml"
    "name: Build Visual Studio VSIX"
    "focused VSIX workflow identity")
require_text(".github/workflows/build-vsix.yml"
    "path: vsix/Copperfin.VisualStudio/bin/Release/net472/*.vsix"
    "intended VSIX artifact path")
require_text(".github/workflows/build-installers.yml"
    "windows-installer:"
    "Windows installer job")
require_text(".github/workflows/build-installers.yml"
    "macos-installer:"
    "macOS installer job")
require_text(".github/workflows/build-installers.yml"
    "linux-deb-rpm-installers:"
    "Linux installer job")
require_text(".github/workflows/build-installers.yml"
    "--retry-count=3 --retry-delay=5"
    "NSIS package-feed retry contract")
require_text(".github/workflows/build-installers.yml"
    "https://downloads.sourceforge.net/nsis/nsis-3.12.zip"
    "checksum-pinned portable NSIS fallback URI")
require_text(".github/workflows/build-installers.yml"
    "56581f90db321581c5381193d796fffcf2d24b2f8fed2160a6c6a3baa67f2c4f"
    "portable NSIS fallback SHA-256 pin")
require_text(".github/workflows/build-installers.yml"
    "Get-FileHash -LiteralPath $nsisFallbackArchive -Algorithm SHA256"
    "portable NSIS fallback checksum verification")
require_text(".github/workflows/build-installers.yml"
    "Expand-Archive -LiteralPath $nsisFallbackArchive -DestinationPath $env:RUNNER_TEMP -Force"
    "portable NSIS fallback extraction")
require_text(".github/workflows/build-installers.yml"
    "Test-Path -LiteralPath $makensis -PathType Leaf"
    "NSIS compiler availability contract")
require_text(".github/workflows/build-installers.yml"
    "NSIS compiler verification failed with exit code"
    "NSIS compiler execution verification")
require_text(".github/workflows/build-installers.yml"
    "cpack --config build/CPackConfig.cmake -B build/package -C Release -G \"NSIS;ZIP\""
    "Windows CPack generator inventory")
require_text(".github/workflows/build-installers.yml"
    "cpack --config build/CPackConfig.cmake -B build/package -G \"productbuild;TGZ\""
    "macOS CPack generator inventory")
foreach(installer_path IN ITEMS
        "build/package/copperfin-*-Windows.exe"
        "build/package/copperfin-*-Windows.zip"
        "build/package/copperfin-*-Darwin.pkg"
        "build/package/copperfin-*-Darwin.tar.gz"
        "build/package/copperfin-*-Linux.deb"
        "build/package/copperfin-*-Linux.rpm"
        "build/package/copperfin-*-Linux.tar.gz")
    require_text(".github/workflows/build-installers.yml"
        "${installer_path}"
        "exact installer artifact path '${installer_path}'")
endforeach()
require_text(".github/workflows/build-installers.yml"
    "tests/run_cpack_artifact_contract_check.cmake"
    "CPack artifact ownership verifier")
require_text_count(".github/workflows/build-installers.yml"
    "-DCOPPERFIN_ARTIFACT_DIR:PATH=build/package"
    3
    "namespaced CPack artifact directories")
require_text_count(".github/workflows/build-installers.yml"
    "-DCOPPERFIN_VERSION_FILE:FILEPATH=build/CopperfinPackageVersion.txt"
    3
    "namespaced CPack version-file paths")
require_text_count(".github/workflows/build-installers.yml"
    "-DCOPPERFIN_EXPECTED_ARTIFACT_SUFFIXES="
    3
    "namespaced CPack artifact suffixes")
forbid_text(".github/workflows/build-installers.yml"
    "-DVERSION_FILE=build/CopperfinPackageVersion.txt"
    "generic CPack version-file variable")
require_text_count(".github/workflows/build-installers.yml"
    "tests/run_locale_catalog_install_contract_check.cmake"
    3
    "locale catalog install contract checks")
require_text_count(".github/workflows/build-installers.yml"
    "cmake --install build --prefix build/install"
    2
    "POSIX installer install-tree materialization")
require_text(".github/workflows/build-installers.yml"
    "-DINSTALL_ROOT=build/standalone-studio-install -P tests/run_locale_catalog_install_contract_check.cmake"
    "Windows installer locale install contract invocation")
require_text_count(".github/workflows/build-installers.yml"
    "tests/run_package_version_contract_check.cmake"
    3
    "package version contract checks")
require_text_count(".github/workflows/build-installers.yml"
    "-DCOPPERFIN_SOURCE_DIR:PATH=\${{ github.workspace }} -DCOPPERFIN_BINARY_DIR:PATH=\${{ github.workspace }}/build"
    3
    "absolute package version verifier paths")
forbid_text(".github/workflows/build-installers.yml"
    "-DSOURCE_DIR=. -DBINARY_DIR=build -P tests/run_package_version_contract_check.cmake"
    "reserved package version verifier variables")
forbid_text(".github/workflows/build-installers.yml"
    "0.1.0"
    "duplicated installer version literal")
require_text_count(".github/workflows/build-installers.yml"
    "if-no-files-found: error"
    3
    "fail-closed artifact upload settings")
require_text_count(".github/workflows/build-installers.yml"
    "cmake -E remove_directory build/package/_CPack_Packages"
    3
    "CPack internal staging cleanup")
require_text(".github/workflows/build-installers.yml"
    "Build standalone Studio shell"
    "Windows installer standalone Studio build")
require_text(".github/workflows/build-installers.yml"
    "cmake -S . -B build -A x64 -DCOPPERFIN_BUILD_TESTS=OFF -DCOPPERFIN_REQUIRE_X64=ON"
    "Windows installer x64 configure")
require_text(".github/workflows/build-installers.yml"
    "tests/run_studio_install_contract_check.cmake"
    "Windows standalone Studio install contract")
require_text(".github/workflows/build-installers.yml"
    "timeout-minutes: 120"
    "bounded Windows installer job timeout")
require_text(".github/workflows/build-installers.yml"
    "copperfin_inspect --parallel 2"
    "bounded Windows installer native build parallelism")
require_text(".github/workflows/build-installers.yml"
    "Copperfin.Studio.csproj /restore /t:Rebuild /m:2"
    "bounded Windows installer managed build parallelism")
require_text("CMakeLists.txt"
    "copperfin_studio_managed"
    "Windows managed Studio install target")
require_text("CMakeLists.txt"
    "MAP_IMPORTED_CONFIG_MINSIZEREL"
    "Windows managed Studio MinSizeRel mapping")
require_text("CMakeLists.txt"
    "CMAKE_SIZEOF_VOID_P must be 8"
    "Windows x64 pointer-size failure")
require_text("CMakeLists.txt"
    "COPPERFIN_NATIVE_POINTER_SIZE"
    "Persisted native pointer-size contract")
require_text("CMakeLists.txt"
    "COPPERFIN_REQUIRE_X64"
    "Opt-in Windows x64 packaging contract")
foreach(stale_pattern IN ITEMS
        "copperfin-*.exe"
        "copperfin-*.zip"
        "copperfin-*.pkg"
        "copperfin-*.deb"
        "copperfin-*.rpm"
        "copperfin-*.tar.gz")
    forbid_text(".github/workflows/build-installers.yml"
        "${stale_pattern}"
        "workspace-root installer glob '${stale_pattern}'")
endforeach()
require_text(".github/workflows/windows-deep-validation.yml"
    "name: Windows Deep Validation"
    "Windows deep-validation workflow identity")
require_text(".github/workflows/windows-deep-validation.yml" [=[      test_jobs:
        description: Bounded native CTest concurrency.
        required: true
        default: '2'
        type: choice
        options:
          - '1'
          - '2']=]
    "bounded Windows CTest input")
require_text(".github/workflows/windows-deep-validation.yml"
    [=[-CommandArguments @('--test-dir', 'build', '-C', '${{ inputs.build_configuration }}', '--output-on-failure', '--parallel', '${{ inputs.test_jobs }}')]=]
    "bounded Windows CTest command")
require_text(".github/workflows/windows-deep-validation.yml"
    [=[name: copperfin-windows-deep-validation-${{ inputs.build_configuration }}-build-${{ inputs.build_jobs }}-test-${{ inputs.test_jobs }}]=]
    "Windows deep-validation build/test job artifact identity")
require_text(".github/workflows/windows-deep-validation.yml"
    [=[      require_vfp9_samples:
        description: Fail closed unless the installed VFP9 sample corpus is available.
        required: true
        default: false
        type: boolean
      vfp9_root:]=]
    "explicit VFP9 prerequisite inputs")
require_text(".github/workflows/windows-deep-validation.yml"
    [=[-OutputPath artifacts/windows-deep-validation-metrics/vfp9-sample-prerequisite.json]=]
    "machine-readable VFP9 prerequisite output")
require_text(".github/workflows/windows-deep-validation.yml"
    [=[-Require:$${{ inputs.require_vfp9_samples }}]=]
    "fail-closed VFP9 prerequisite mode")
require_text("scripts/check-windows-vfp9-samples.ps1"
    "windows-vfp9-sample-prerequisite"
    "VFP9 prerequisite result identity")
require_text("scripts/check-windows-vfp9-samples.ps1"
    "no VFP9 sample coverage is claimed"
    "explicit missing-sample coverage warning")
require_text_count(".github/workflows/windows-deep-validation.yml"
    [=[if: ${{ steps.vfp9_samples.outputs.available == 'true' }}]=]
    4
    "VFP9-dependent smoke guards")
foreach(deep_step IN ITEMS
        "Build Visual Studio extension"
        "Build standalone Studio shell"
        "Build designer smoke tests"
        "Run designer smoke tests"
        "Run runtime package smoke test"
        "Run PRG debugger smoke test"
        "Run xAsset bootstrap smoke test"
        "Run report xAsset smoke test"
        "Run menu xAsset smoke test"
        "Run native CTest suite")
    require_text(".github/workflows/windows-deep-validation.yml"
        "${deep_step}"
        "Windows deep-validation step '${deep_step}'")
endforeach()
forbid_text(".github/workflows/windows-deep-validation.yml"
    "run_designer_smoke_tests"
    "conditionally disabled designer smoke input")
foreach(deep_smoke_stage IN ITEMS RuntimePackage PrgDebugger XAsset Report Menu)
    require_text(".github/workflows/windows-deep-validation.yml"
        "-Stage', '${deep_smoke_stage}'"
        "Windows deep-validation smoke stage '${deep_smoke_stage}'")
endforeach()
require_text("scripts/run-windows-deep-smoke.ps1"
    "[ValidateSet(\"RuntimePackage\", \"PrgDebugger\", \"XAsset\", \"Report\", \"Menu\")]"
    "shared Windows deep-smoke stage inventory")
require_text("scripts/run-windows-deep-smoke.ps1"
    "COPPERFIN_SECURITY_ROLE = \"build-engineer\""
    "secure runtime-package smoke build role")
require_text("scripts/run-windows-deep-smoke.ps1"
    "function Stage-SmokeAsset"
    "staged VFP asset smoke helper")
require_text("scripts/run-windows-deep-smoke.ps1"
    "ChangeExtension($SourcePath, $sidecarExtension)"
    "VFP asset memo-sidecar staging")
require_text("scripts/run-required-designer-smoke.ps1"
    "fixture-dependent skip(s)"
    "fixture-aware designer smoke launcher")
require_text(".github/workflows/windows-x86-declare-validation.yml"
    "name: Windows DECLARE ABI Validation"
    "focused DECLARE workflow identity")
require_text(".github/workflows/windows-x86-declare-validation.yml"
    "name: Windows \${{ matrix.platform }} DECLARE"
    "focused DECLARE check identity")
require_text(".github/workflows/windows-x86-declare-validation.yml"
    "--target test_prg_engine_seek_index test_prg_engine_dotnet_dispatch test_prg_engine_parser_classes test_localization"
    "focused DECLARE target inventory")

require_text("README.md"
    "Release readiness requires successful `Linux GCC`, `macOS Clang`, and `Windows MSVC` checks."
    "three-platform release-readiness guidance")
require_text("README.md"
    "Manual `.github/workflows/native-release-readiness.yml` runs all three shared contracts and exposes a final dependent gate."
    "manual release-gate guidance")

require_text("${shared_action}" "runs:\n  using: composite\n  steps:"
    "composite-action execution contract")
require_regex("${shared_action}"
    "\ninputs:\n  platform:\n    description:[^\n]*\n    required:[ \t]*true([ \t]*\n|$)"
    "required platform input")
require_regex_count("${shared_action}"
    "\n  [A-Za-z_][A-Za-z0-9_-]*:\n    description:[^\n]*\n    required:[ \t]*true([ \t]*\n|$)"
    1 "composite-action input declaration")
require_text_count("${shared_action}"
    "uses: actions/setup-dotnet@26b0ec14cb23fa6904739307f278c14f94c95bf1 # v5.4.0"
    1 "immutable setup-dotnet action")
require_text("${shared_action}" [=[uses: actions/setup-dotnet@26b0ec14cb23fa6904739307f278c14f94c95bf1 # v5.4.0
      with:
        dotnet-version: 10.0.x]=]
    ".NET 10 SDK setup")

require_text("${shared_action}" [=[if: ${{ inputs.platform != 'windows' }}
      shell: bash
      run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCOPPERFIN_BUILD_TESTS=ON]=]
    "non-Windows configure command")
require_text("${shared_action}" [=[if: ${{ inputs.platform != 'windows' }}
      shell: bash
      run: cmake --build build --parallel 2]=]
    "bounded non-Windows build command")
require_text("${shared_action}" [=[if: ${{ inputs.platform != 'windows' }}
      shell: bash
      run: ctest --test-dir build --output-on-failure --timeout 180]=]
    "full non-Windows CTest command")
require_text("${shared_action}"
    "-Name 'Configure native build'"
    "measured Windows configure phase")
require_text("${shared_action}"
    "-CommandArguments @('-S', '.', '-B', 'build', '-A', 'x64', '-DCOPPERFIN_BUILD_TESTS=ON')"
    "Windows x64 configure command")
require_text("${shared_action}"
    "-Name 'Build native targets'"
    "measured Windows native-build phase")
require_text("${shared_action}"
    [=[-CommandArguments @('--build', 'build', '--config', 'Release', '--parallel', '${{ inputs.build_jobs }}')]=]
    "bounded Windows build command")
require_text("${shared_action}"
    "-Name 'Run native test suite'"
    "measured Windows CTest phase")
require_text("${shared_action}"
    "-CommandArguments @('--test-dir', 'build', '-C', 'Release', '--output-on-failure', '--timeout', '180', '--parallel', '2')"
    "bounded full Windows CTest command")
require_text("${shared_action}"
    "-Mode Finalize"
    "Windows metric finalization")
require_text("scripts/validate-windows.ps1"
    [=[[int]$TestJobs = 2]=]
    "bounded local Windows CTest default")
require_text("scripts/validate-windows.ps1"
    [=["--timeout", "180",]=]
    "bounded local Windows CTest timeout")
require_text("scripts/validate-windows.ps1"
    [=["--parallel", "$TestJobs"]=]
    "bounded local Windows CTest command")
require_text("tests/CMakeLists.txt"
    "set_tests_properties(test_studio_host_json PROPERTIES TIMEOUT 600)"
    "extended timeout for the known aggregate Studio host JSON test")

require_regex_count("${shared_action}" "\n[ \t]+run:[ \t]*cmake -S "
    1 "direct non-Windows configure command")
require_regex_count("${shared_action}" "\n[ \t]+run:[ \t]*cmake --build "
    1 "direct non-Windows build command")
require_regex_count("${shared_action}" "\n[ \t]+run:[ \t]*ctest "
    1 "direct non-Windows CTest command")
require_text_count("${shared_action}" [=[if: ${{ inputs.platform != 'windows' }}]=]
    3 "non-Windows platform conditions")
require_text_count("${shared_action}" [=[if: ${{ inputs.platform == 'windows' }}]=]
    4 "Windows platform conditions")

forbid_text("${shared_action}" "upload-artifact" "artifact upload")
forbid_regex("${shared_action}" "ctest[^\n]*[ \t]-R([ \t=]|\n|$)" "CTest -R filtering")
forbid_text("${shared_action}" "--tests-regex" "CTest regex filtering")

foreach(native_workflow IN ITEMS
        .github/actions/native-validation/action.yml
        .github/workflows/native-validation-linux.yml
        .github/workflows/native-validation-macos.yml
        .github/workflows/native-validation-windows.yml
        .github/workflows/native-release-readiness.yml)
    forbid_text("${native_workflow}" "actions/upload-artifact" "artifact upload action")
    forbid_text("${native_workflow}" "actions/download-artifact" "artifact download action")
    forbid_regex("${native_workflow}"
        "path:[^\n]*(CMakeCache\\.txt|CMakeFiles|(^|[/\\\\])build([/\\\\]|$))"
        "CMake build-tree artifact path")
endforeach()

foreach(artifact_workflow IN ITEMS
        .github/workflows/build-vsix.yml
        .github/workflows/build-installers.yml
        .github/workflows/windows-deep-validation.yml)
    forbid_text("${artifact_workflow}" "CMakeCache.txt" "CMake cache artifact")
    forbid_text("${artifact_workflow}" "CMakeFiles" "CMake metadata artifact")
    validate_artifact_upload_paths("${artifact_workflow}")
endforeach()

message(STATUS "Native platform workflow contract check passed")
