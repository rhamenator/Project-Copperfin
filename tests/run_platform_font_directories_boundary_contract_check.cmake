# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()
set(header_path "${SOURCE_DIR}/include/copperfin/platform/font_directories.h")
set(source_path "${SOURCE_DIR}/src/platform/font_directories.cpp")
set(runtime_path "${SOURCE_DIR}/src/runtime/prg_engine_variables.inl")
set(runtime_owner_path "${SOURCE_DIR}/src/runtime/prg_engine.cpp")
set(root_build_path "${SOURCE_DIR}/CMakeLists.txt")
set(test_build_path "${SOURCE_DIR}/tests/CMakeLists.txt")
set(workflow_path "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml")
foreach(path IN ITEMS "${header_path}" "${source_path}" "${runtime_path}"
        "${runtime_owner_path}" "${root_build_path}" "${test_build_path}" "${workflow_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Font-directory boundary input is missing: ${path}")
    endif()
endforeach()
foreach(binding IN ITEMS header source runtime runtime_owner root_build test_build workflow)
    file(READ "${${binding}_path}" ${binding}_text)
endforeach()
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
foreach(token IN ITEMS "_WIN32" "__APPLE__" "WINDIR" "HOME"
        "C:\\Windows\\Fonts" "/System/Library/Fonts" "/usr/share/fonts")
    forbid_text("${header_text}" "${token}" "host token in portable font-directory API")
    forbid_text("${runtime_text}" "${token}" "host font-root policy in PRG runtime")
endforeach()
foreach(token IN ITEMS "#if defined(_WIN32)" "#elif defined(__APPLE__)"
        "read_environment_variable(\"WINDIR\")" "read_environment_variable(\"HOME\")"
        "C:\\\\Windows\\\\Fonts" "/System/Library/Fonts" "/usr/share/fonts")
    require_text("${source_text}" "${token}" "private host font-directory selection")
endforeach()
require_text("${header_text}" "std::vector<std::filesystem::path> font_search_directories();"
    "portable font-directory contract")
require_text("${runtime_owner_text}" "#include \"copperfin/platform/font_directories.h\""
    "runtime portable font-directory include")
require_text("${runtime_text}" "platform::font_search_directories();" "AFONT platform delegation")
require_text("${root_build_text}" "src/platform/font_directories.cpp" "platform source registration")
require_text("${test_build_text}" "test_platform_font_directories cf_platform_support"
    "font-directory behavior test registration")
require_text("${workflow_text}" "test_platform_font_directories_boundary_contract"
    "hosted font-directory boundary scheduling")
require_text("${workflow_text}" "test_platform_font_directories" "hosted behavior scheduling")
require_text("${workflow_text}" "test_prg_engine_arrays" "hosted AFONT consumer scheduling")
message(STATUS "Portable font-directory boundary contract passed")
