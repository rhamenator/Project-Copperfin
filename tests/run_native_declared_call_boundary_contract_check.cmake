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

read_source("src/runtime/native_declared_call.h" boundary_header)
read_source("src/runtime/native_declared_call.cpp" windows_implementation)
read_source("src/runtime/prg_engine.cpp" interpreter_source)
read_source("src/runtime/prg_engine_dll.inl" declared_call_source)
read_source("CMakeLists.txt" root_build)

foreach(forbidden_token IN ITEMS
        "_WIN32"
        "_WIN64"
        "windows.h"
        "oleauto.h"
        "HRESULT"
        "VARIANT"
        "VARTYPE"
        "CALLCONV"
        "HMODULE"
        "FARPROC"
        "ULONG_PTR"
        "DispCallFunc"
        "__int64")
    forbid_text("${boundary_header}" "${forbidden_token}"
        "Windows ABI token in the portable native-call boundary")
endforeach()

foreach(required_token IN ITEMS
        "enum class NativeDeclaredArgumentKind"
        "enum class NativeDeclaredReturnKind"
        "struct NativeDeclaredCallRequest"
        "struct NativeDeclaredCallResult"
        "std::uintptr_t function_address = 0U;"
        "std::int32_t compatible_error_code = 0;"
        "std::vector<NativeDeclaredArgument> arguments;"
        "NativeDeclaredCallResult invoke_native_declared_function(")
    require_text("${boundary_header}" "${required_token}"
        "portable native-call declaration")
endforeach()

foreach(required_token IN ITEMS
        "#include <windows.h>"
        "#include <oleauto.h>"
        "#include \"win64_native_call.h\""
        "std::vector<VARTYPE> argument_types(arguments.size());"
        "std::vector<VARIANTARG> argument_values(arguments.size());"
        "const HRESULT invoke_result = DispCallFunc("
        "request.use_cdecl ? CC_CDECL : CC_STDCALL"
        "copy_by_reference_results(request, storage, result);"
        "detail::invoke_win64_native_function(")
    require_text("${windows_implementation}" "${required_token}"
        "private Windows native-call implementation")
endforeach()

foreach(forbidden_token IN ITEMS
        "#include \"win64_native_call.h\""
        "#include <oleauto.h>")
    forbid_text("${interpreter_source}" "${forbidden_token}"
        "native-call ABI dependency in the interpreter translation unit")
endforeach()
require_text("${interpreter_source}" "#include \"native_declared_call.h\""
    "portable native-call boundary include")

foreach(forbidden_token IN ITEMS
        "__int64"
        "VARTYPE"
        "VARIANTARG"
        "VariantInit"
        "CALLCONV"
        "CC_CDECL"
        "CC_STDCALL"
        "DispCallFunc"
        "invoke_win64_native_function"
        "reinterpret_cast<__int64>")
    forbid_text("${declared_call_source}" "${forbidden_token}"
        "Windows ABI marshaling in the interpreter DECLARE path")
endforeach()
foreach(required_token IN ITEMS
        "NativeDeclaredCallRequest request;"
        "request.function_address = declfn.native_function_address;"
        "request.use_cdecl = declfn.native_cdecl;"
        "request.arguments.push_back(std::move(argument));"
        "const NativeDeclaredCallResult native_result ="
        "invoke_native_declared_function(request);"
        "if (!native_result.succeeded)"
        "native_result.compatible_error_code"
        "assign_variable("
        "return make_string_value(native_result.string_value);"
        "return make_int64_value(native_result.signed_integer_value);")
    require_text("${declared_call_source}" "${required_token}"
        "portable interpreter-to-native-call boundary use")
endforeach()

require_text("${root_build}" "src/runtime/native_declared_call.cpp"
    "private Windows native-call build source")
