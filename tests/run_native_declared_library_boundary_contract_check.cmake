# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(read_source relative_path output_variable)
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    set(${output_variable} "${contents}" PARENT_SCOPE)
endfunction()

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

read_source("src/runtime/native_declared_library.h" boundary_header)
read_source("src/runtime/native_declared_library.cpp" windows_implementation)
read_source("src/runtime/prg_engine.cpp" interpreter_source)
read_source("src/runtime/prg_engine_dispatch.inl" dispatch_source)
read_source("src/runtime/prg_engine_session_shutdown_and_debug.inl" shutdown_source)
read_source("CMakeLists.txt" root_build)

foreach(forbidden_token IN ITEMS
        "_WIN32"
        "windows.h"
        "HMODULE"
        "FARPROC"
        "LoadLibrary"
        "FreeLibrary"
        "GetProcAddress"
        "DWORD")
    forbid_text("${boundary_header}" "${forbidden_token}"
        "Windows loader token in the portable native DECLARE boundary")
endforeach()

foreach(required_token IN ITEMS
        "enum class NativeDeclaredLibraryKind"
        "struct NativeDeclaredLibraryLoadResult"
        "std::uintptr_t module_handle = 0U;"
        "std::uintptr_t function_address = 0U;"
        "NativeDeclaredLibraryLoadResult load_native_declared_library("
        "void release_native_declared_library(std::uintptr_t module_handle) noexcept;")
    require_text("${boundary_header}" "${required_token}"
        "portable native DECLARE loader declaration")
endforeach()

foreach(required_token IN ITEMS
        "#include <windows.h>"
        "class ModuleOwner final"
        "FreeLibrary(module_);"
        "constexpr std::array<const wchar_t *, 5U> win32api_modules{\n                L\"Kernel32.dll\",\n                L\"Gdi32.dll\",\n                L\"User32.dll\",\n                L\"Mpr.dll\",\n                L\"Advapi32.dll\",\n            };"
        "const std::array<std::string, 2U> export_names{function_name, function_name + \"A\"};"
        "declared_dll_x86_stdcall_stack_bytes(parameter_types)"
        "GetSystemDirectoryW("
        "LoadLibraryW("
        "GetProcAddress("
        "SearchPathW("
        "GetModuleFileNameW("
        "FormatMessageA("
        "inspect_portable_executable(inspection_path)"
        "FreeLibrary(reinterpret_cast<HMODULE>(module_handle));")
    require_text("${windows_implementation}" "${required_token}"
        "private Windows native DECLARE loader implementation")
endforeach()

foreach(forbidden_token IN ITEMS
        "HMODULE hmodule"
        "FARPROC proc_address")
    forbid_text("${interpreter_source}" "${forbidden_token}"
        "native loader handle type in interpreter state")
endforeach()
foreach(required_token IN ITEMS
        "std::uintptr_t native_module_handle = 0U;"
        "std::uintptr_t native_function_address = 0U;")
    require_text("${interpreter_source}" "${required_token}"
        "portable native DECLARE state")
endforeach()

foreach(forbidden_token IN ITEMS
        "LoadLibraryW("
        "FreeLibrary("
        "GetProcAddress("
        "SearchPathW("
        "GetSystemDirectoryW("
        "GetModuleFileNameW("
        "FormatMessageA(")
    forbid_text("${dispatch_source}" "${forbidden_token}"
        "Windows loader operation in interpreter dispatch")
endforeach()
foreach(required_token IN ITEMS
        "const NativeDeclaredLibraryLoadResult load_result = load_native_declared_library("
        "declfn.native_module_handle = load_result.module_handle;"
        "declfn.native_function_address = load_result.function_address;"
        "release_native_declared_library(existing->second.native_module_handle);")
    require_text("${dispatch_source}" "${required_token}"
        "portable native DECLARE loader result use")
endforeach()

forbid_text("${shutdown_source}" "FreeLibrary("
    "Windows module release in interpreter shutdown")
require_text("${shutdown_source}"
    "release_native_declared_library(declfn.native_module_handle);"
    "portable native DECLARE lifetime release")
require_text("${root_build}"
    "src/runtime/native_declared_library.cpp"
    "private Windows native DECLARE loader build source")
