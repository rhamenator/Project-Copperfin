# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/code_page.h")
set(source_path "${SOURCE_DIR}/src/platform/code_page.cpp")
set(locale_runtime_path "${SOURCE_DIR}/src/runtime/prg_engine_locale_code_page.cpp")
set(surface_runtime_path "${SOURCE_DIR}/src/runtime/prg_engine_runtime_surface_functions.cpp")
set(surface_helpers_path "${SOURCE_DIR}/src/runtime/prg_engine_runtime_surface_platform_helpers.inl")
set(root_build_path "${SOURCE_DIR}/CMakeLists.txt")
set(workflow_path "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml")
foreach(path IN ITEMS
        "${header_path}"
        "${source_path}"
        "${locale_runtime_path}"
        "${surface_runtime_path}"
        "${surface_helpers_path}"
        "${root_build_path}"
        "${workflow_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Code-page boundary input is missing: ${path}")
    endif()
endforeach()

file(READ "${header_path}" header_text)
file(READ "${source_path}" source_text)
file(READ "${locale_runtime_path}" locale_runtime_text)
file(READ "${surface_runtime_path}" surface_runtime_text)
file(READ "${surface_helpers_path}" surface_helpers_text)
file(READ "${root_build_path}" root_build_text)
file(READ "${workflow_path}" workflow_text)
set(runtime_text
    "${locale_runtime_text}\n${surface_runtime_text}\n${surface_helpers_text}")

function(require_text contents expected description)
    string(FIND "${contents}" "${expected}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "Missing ${description}: ${expected}")
    endif()
endfunction()

function(forbid_text contents forbidden description)
    string(FIND "${contents}" "${forbidden}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR "Forbidden ${description}: ${forbidden}")
    endif()
endfunction()

foreach(token IN ITEMS
        "_WIN32"
        "windows.h"
        "iconv.h"
        "langinfo.h"
        "UINT"
        "iconv_t"
        "CODESET"
        "GetACP"
        "GetOEMCP"
        "MultiByteToWideChar"
        "WideCharToMultiByte")
    forbid_text("${header_text}" "${token}" "native token in portable code-page API")
endforeach()

foreach(token IN ITEMS
        "#include <iconv.h>"
        "#include <langinfo.h>"
        "GetACP("
        "GetOEMCP("
        "MultiByteToWideChar("
        "WideCharToMultiByte("
        "nl_langinfo("
        "iconv("
        "iconv_open("
        "iconv_close(")
    forbid_text("${runtime_text}" "${token}" "native code-page implementation in PRG runtime")
endforeach()

foreach(token IN ITEMS
        "parse_posix_locale_code_page("
        "resolve_posix_host_code_page("
        "host_code_page();"
        "host_oem_code_page();"
        "convert_code_page_bytes(")
    require_text("${header_text}" "${token}" "portable code-page contract")
endforeach()

foreach(token IN ITEMS
        "#if defined(_WIN32)"
        "#include <windows.h>"
        "#include <iconv.h>"
        "#include <langinfo.h>"
        "::GetACP()"
        "::GetOEMCP()"
        "::MultiByteToWideChar("
        "::WideCharToMultiByte("
        "nl_langinfo(CODESET)"
        "iconv("
        "iconv_open("
        "iconv_close(")
    require_text("${source_text}" "${token}" "private cross-platform code-page implementation")
endforeach()

foreach(token IN ITEMS
        "copperfin::platform::parse_posix_locale_code_page("
        "copperfin::platform::resolve_posix_host_code_page("
        "copperfin::platform::host_code_page();"
        "copperfin::platform::host_oem_code_page();")
    require_text("${locale_runtime_text}" "${token}" "portable locale/code-page delegation")
endforeach()
require_text("${surface_helpers_text}"
    "copperfin::platform::convert_code_page_bytes("
    "portable CPCONVERT byte-conversion delegation")
require_text("${root_build_text}"
    "src/platform/code_page.cpp"
    "platform-support code-page source registration")
require_text("${root_build_text}"
    "target_link_libraries(cf_platform_support PRIVATE Iconv::Iconv)"
    "private POSIX iconv linkage")
require_text("${workflow_text}"
    "tests/run_platform_code_page_boundary_contract_check.cmake"
    "hosted code-page boundary path trigger")
require_text("${workflow_text}"
    "test_platform_code_page_boundary_contract"
    "hosted code-page boundary scheduling")
require_text("${workflow_text}"
    "test_prg_engine_locale_code_page"
    "hosted code-page behavior scheduling")

message(STATUS "Portable code-page boundary contract passed")
