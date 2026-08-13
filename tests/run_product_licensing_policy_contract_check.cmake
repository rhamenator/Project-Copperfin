# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED COPPERFIN_SOURCE_ROOT)
    message(FATAL_ERROR "COPPERFIN_SOURCE_ROOT is required")
endif()

function(require_file relative_path)
    if(NOT EXISTS "${COPPERFIN_SOURCE_ROOT}/${relative_path}")
        message(FATAL_ERROR "Required licensing-policy file is missing: ${relative_path}")
    endif()
endfunction()

function(require_text relative_path expected_text)
    require_file("${relative_path}")
    file(READ "${COPPERFIN_SOURCE_ROOT}/${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing required text: ${expected_text}")
    endif()
endfunction()

function(require_text_before relative_path earlier_text later_text)
    require_file("${relative_path}")
    file(READ "${COPPERFIN_SOURCE_ROOT}/${relative_path}" contents)
    string(FIND "${contents}" "${earlier_text}" earlier_offset)
    string(FIND "${contents}" "${later_text}" later_offset)
    if(earlier_offset EQUAL -1 OR later_offset EQUAL -1 OR
       NOT earlier_offset LESS later_offset)
        message(FATAL_ERROR
            "${relative_path} must place ${earlier_text} before ${later_text}")
    endif()
endfunction()

require_text("LICENSE" "GNU GENERAL PUBLIC LICENSE")
require_text("LICENSE" "COPPERFIN APPLICATION, RUNTIME, AND TOOLCHAIN EXCEPTION")
require_text("LICENSE" "additional permission is granted under section 7")
require_text("LICENSE" "link an Independent Work with Copperfin Code, statically or")
require_text("LICENSE" "Permitted Output under terms of")
require_text("LICENSE" "Standard Support Material is Permitted Output only")
require_text("LICENSE" "This rule does not withdraw paragraph 2(c)'s permission")
require_text("LICENSE" "GPL-3.0-only for the Copperfin Code")
require_text("LICENSE.md" "GPL-3.0-only")
require_text("LICENSE.md" "place that work under the GPL")
require_text("LICENSE.md" "link statically or")
require_text("LICENSE.md" "including proprietary terms")
require_text("LICENSE.md" "GPLv3 section 2")
require_text("LICENSE.md" "GPLv3 section 7")
require_text("LICENSE.md" "downstream distributor to remove an additional")
require_text("LICENSE.md" "Project Copperfin cannot place their")
require_text("README.md" "GNU General Public")
require_text("README.md" "GPL-3.0-only")
require_text("README.md" "Copperfin Application,")
require_text("README.md" "proprietary terms")
require_text("docs/05-roadmap.md" "GPLv3 section 7 exception")
require_text("tools/package-signer/README.md" "static or dynamic linking")
require_text("docs/33-application-runtime-license-exception.md" "VFP9 Offline Evidence")
require_text("docs/33-application-runtime-license-exception.md" "GPLv3 section 7")
require_text("docs/33-application-runtime-license-exception.md" "qualified legal review")
require_text("docs/34-human-authorship-and-assisted-development.md" "Human-Control Evidence")
require_text("SOURCE.md" "Corresponding Source")
require_text("THIRD_PARTY_NOTICES.md" "Vendored Ed25519 Verification Code")
require_file("LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt")
require_file("docs/contracts/release-license-metadata.json")
require_text("docs/archive/commercial-licensing-2026/README.md" "inactive historical material")

foreach(archived_name IN ITEMS
        LICENSE.md
        SOURCE_AVAILABLE_LICENSE.md
        COMMERCIAL_LICENSE.md
        CLA.md
        MIGRATION_NOTICE.md
        LEGAL_FAQ.md)
    require_file("docs/archive/commercial-licensing-2026/${archived_name}")
endforeach()

foreach(inactive_root_name IN ITEMS
        SOURCE_AVAILABLE_LICENSE.md
        COMMERCIAL_LICENSE.md
        CLA.md
        MIGRATION_NOTICE.md
        LEGAL_FAQ.md
        LICENSE-GPL-3.0-HISTORICAL.md)
    if(EXISTS "${COPPERFIN_SOURCE_ROOT}/${inactive_root_name}")
        message(FATAL_ERROR "Inactive licensing document remains operative at repository root: ${inactive_root_name}")
    endif()
endforeach()

require_text("CMakeLists.txt" "option(COPPERFIN_ENABLE_PRODUCT_LICENSING")
require_text("CMakeLists.txt" "Enable the archived commercial product-license loader and status model\"\n    OFF)")
require_text("include/copperfin/licensing/license_status.h" "kProductLicensingEnabled")
require_text("src/licensing/license_status.cpp" "#if !COPPERFIN_ENABLE_PRODUCT_LICENSING")
require_text("src/licensing/license_status.cpp" "return {};")
require_text("apps/copperfin_build_host/main.cpp" "if constexpr (copperfin::licensing::kProductLicensingEnabled)")
require_text("apps/copperfin_studio_host/studio_host_main_shared_cli_infra.cpp" "if constexpr (!copperfin::licensing::kProductLicensingEnabled)")
require_text_before("apps/copperfin_studio_host/main.cpp"
    "try_handle_workspace_agent_policy(catalog, args)"
    "try_handle_license_status(catalog, args)")
require_text("vsix/Copperfin.VisualStudio/CopperfinAssetEditorControl.cs" "if (!string.IsNullOrEmpty(snapshot.LicenseProfile.State))")

# Product licensing and release trust deliberately share low-level Ed25519
# primitives, but the package-trust target and regression test remain present
# and independent of the product-license activation policy.
require_text("CMakeLists.txt" "add_library(cf_package_trust")
require_text("CMakeLists.txt" "set(COPPERFIN_PACKAGE_LICENSE_DOCUMENT LICENSE)")
require_text("tests/CMakeLists.txt" "test_package_launcher_inventory_trust")
require_text("tools/package-signer/README.md" "provenance and integrity")
require_text("docs/29-package-trust-contract.md" "archived product-license signer")

file(GLOB_RECURSE policy_sources
    LIST_DIRECTORIES false
    "${COPPERFIN_SOURCE_ROOT}/apps/*.cpp"
    "${COPPERFIN_SOURCE_ROOT}/apps/*.h"
    "${COPPERFIN_SOURCE_ROOT}/include/*.h"
    "${COPPERFIN_SOURCE_ROOT}/scripts/*.ps1"
    "${COPPERFIN_SOURCE_ROOT}/scripts/*.psm1"
    "${COPPERFIN_SOURCE_ROOT}/scripts/*.py"
    "${COPPERFIN_SOURCE_ROOT}/scripts/*.sh"
    "${COPPERFIN_SOURCE_ROOT}/src/*.cpp"
    "${COPPERFIN_SOURCE_ROOT}/src/*.h"
    "${COPPERFIN_SOURCE_ROOT}/src/*.inl"
    "${COPPERFIN_SOURCE_ROOT}/tests/*.cmake"
    "${COPPERFIN_SOURCE_ROOT}/tests/*.cpp"
    "${COPPERFIN_SOURCE_ROOT}/tests/*.h"
    "${COPPERFIN_SOURCE_ROOT}/tests/*.ps1"
    "${COPPERFIN_SOURCE_ROOT}/tools/*.ps1"
    "${COPPERFIN_SOURCE_ROOT}/tools/*.sh"
    "${COPPERFIN_SOURCE_ROOT}/vsix/*.cs")
foreach(source_path IN LISTS policy_sources)
    if(IS_DIRECTORY "${source_path}")
        continue()
    endif()
    if(source_path STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
        continue()
    endif()
    file(READ "${source_path}" source_contents)
    string(FIND "${source_contents}" "Project Copperfin Source-Available License or" stale_header_offset)
    if(NOT stale_header_offset EQUAL -1)
        message(FATAL_ERROR "Stale source-available header remains in ${source_path}")
    endif()
    string(FIND "${source_contents}" "Copyright © 2026 Richard M. Hamilton." first_party_offset)
    if(NOT first_party_offset EQUAL -1)
        string(FIND "${source_contents}"
            "Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE."
            exception_notice_offset)
        if(exception_notice_offset EQUAL -1)
            message(FATAL_ERROR "First-party source omits the Copperfin exception notice: ${source_path}")
        endif()
    endif()
endforeach()

file(GLOB_RECURSE active_policy_docs
    LIST_DIRECTORIES false
    "${COPPERFIN_SOURCE_ROOT}/*.md")
foreach(doc_path IN LISTS active_policy_docs)
    if(doc_path MATCHES "/docs/archive/commercial-licensing-2026/")
        continue()
    endif()
    file(READ "${doc_path}" doc_contents)
    string(FIND "${doc_contents}" "commercial-license signer" stale_signer_offset)
    if(NOT stale_signer_offset EQUAL -1)
        message(FATAL_ERROR "Stale commercial-license signer wording remains in ${doc_path}")
    endif()
endforeach()

message(STATUS "GPL/product-licensing policy contract passed")
