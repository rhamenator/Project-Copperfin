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

read_source("src/runtime/managed_declared_call.h" boundary_header)
read_source("src/runtime/managed_declared_call.cpp" windows_implementation)
read_source("src/runtime/prg_engine.cpp" interpreter_source)
read_source("src/runtime/prg_engine_dll.inl" declared_call_source)
read_source("tests/test_prg_engine_dotnet_dispatch.cpp" managed_dispatch_test)
read_source("CMakeLists.txt" root_build)
read_source("tests/CMakeLists.txt" test_build)

foreach(obsolete_path IN ITEMS
        "src/runtime/dispatch_exception_info.h"
        "src/runtime/dispatch_exception_info.cpp")
    if(EXISTS "${SOURCE_DIR}/${obsolete_path}")
        message(FATAL_ERROR "Obsolete IDispatch exception owner returned: ${obsolete_path}")
    endif()
endforeach()

require_text("${managed_dispatch_test}"
    "#include <windows.h>\n#include <oleauto.h>"
    "explicit Windows SDK prerequisite before Automation in the managed dispatch test")

foreach(build_source IN ITEMS "${root_build}" "${test_build}")
    foreach(obsolete_token IN ITEMS
            "dispatch_exception_info"
            "COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS")
        forbid_text("${build_source}" "${obsolete_token}"
            "obsolete IDispatch exception-owner build wiring")
    endforeach()
endforeach()

foreach(forbidden_token IN ITEMS
        "_WIN32"
        "windows.h"
        "oleauto.h"
        "metahost.h"
        "mscorlib"
        "HRESULT"
        "VARIANT"
        "BSTR"
        "SAFEARRAY"
        "DISP_E_")
    forbid_text("${boundary_header}" "${forbidden_token}"
        "native CLR/COM token in the portable boundary header")
endforeach()

foreach(required_token IN ITEMS
        "enum class ManagedDeclaredArgumentKind"
        "enum class ManagedDeclaredValueKind"
        "std::int32_t compatible_error_code = 0;"
        "const std::vector<ManagedDeclaredArgument> &arguments);")
    require_text("${boundary_header}" "${required_token}"
        "portable managed-call declaration")
endforeach()

foreach(required_token IN ITEMS
        "#include <windows.h>"
        "#include <oleauto.h>"
        "#include <metahost.h>"
        "#pragma comment(lib, \"mscoree.lib\")"
        "#import \"mscorlib.tlb\""
        "HRESULT populate_argument_variant("
        "ManagedDeclaredValue portable_return_value(const VARIANT &value)"
        "return managed_failure(hr, ManagedInvocationStage::invoke_method);")
    require_text("${windows_implementation}" "${required_token}"
        "private Windows CLR-host implementation")
endforeach()

foreach(forbidden_token IN ITEMS
        "#include <metahost.h>"
        "#include <comdef.h>"
        "#pragma comment(lib, \"mscoree.lib\")")
    forbid_text("${interpreter_source}" "${forbidden_token}"
        "CLR-host dependency in the interpreter translation unit")
endforeach()
require_text("${interpreter_source}" "#include \"managed_declared_call.h\""
    "portable CLR-host boundary include")

foreach(forbidden_token IN ITEMS
        "to_variant"
        "from_variant"
        "DispatchExceptionInfo"
        "EXCEPINFO"
        "std::vector<VARIANT> managed_arguments"
        "VariantClear(&return_value)"
        "invocation.hresult")
    forbid_text("${declared_call_source}" "${forbidden_token}"
        "managed COM marshaling in the interpreter DECLARE path")
endforeach()
foreach(required_token IN ITEMS
        "std::vector<ManagedDeclaredArgument> managed_arguments;"
        "to_managed_argument(args[index], param_type_at(index))"
        "if (!invocation.succeeded)"
        "invocation.compatible_error_code"
        "from_managed_value(invocation.value)")
    require_text("${declared_call_source}" "${required_token}"
        "portable interpreter-to-CLR boundary use")
endforeach()
