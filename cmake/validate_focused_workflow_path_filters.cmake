if(NOT DEFINED COPPERFIN_SOURCE_DIR)
    get_filename_component(COPPERFIN_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

function(require_path_filter workflow_name path_pattern)
    set(workflow_path "${COPPERFIN_SOURCE_DIR}/.github/workflows/${workflow_name}")
    if(NOT EXISTS "${workflow_path}")
        message(FATAL_ERROR "Missing focused workflow: ${workflow_path}")
    endif()

    file(READ "${workflow_path}" workflow_contents)
    string(FIND "${workflow_contents}" "  push:" push_start)
    string(FIND "${workflow_contents}" "  pull_request:" pull_request_start)
    string(FIND "${workflow_contents}" "\npermissions:" permissions_start)
    if(push_start EQUAL -1 OR pull_request_start EQUAL -1 OR permissions_start EQUAL -1)
        message(FATAL_ERROR "${workflow_name} must define push and pull_request path-filter sections")
    endif()

    math(EXPR push_length "${pull_request_start} - ${push_start}")
    math(EXPR pull_request_length "${permissions_start} - ${pull_request_start}")
    string(SUBSTRING "${workflow_contents}" ${push_start} ${push_length} push_section)
    string(SUBSTRING "${workflow_contents}" ${pull_request_start} ${pull_request_length} pull_request_section)
    set(path_line "      - \"${path_pattern}\"")
    string(FIND "${push_section}" "${path_line}" push_path_index)
    string(FIND "${pull_request_section}" "${path_line}" pull_request_path_index)
    if(push_path_index EQUAL -1 OR pull_request_path_index EQUAL -1)
        message(FATAL_ERROR
            "${workflow_name} must include ${path_line} in both push and pull_request path filters")
    endif()
endfunction()

function(require_path_filter_contract workflow_name)
    foreach(path_pattern IN LISTS ARGN)
        require_path_filter("${workflow_name}" "${path_pattern}")
    endforeach()
endfunction()

set(common_focused_inputs
    "CMakeLists.txt"
    "cmake/**"
    "include/**"
    "resources/locales/**"
    "src/**"
    "tests/**")
set(environment_host_inputs
    ".github/workflows/windows-environment-validation.yml"
    "apps/copperfin_build_host/**"
    "apps/copperfin_inspect/**"
    "apps/copperfin_runtime_host/**"
    ${common_focused_inputs})
set(executable_path_inputs
    ".github/workflows/executable-path-validation.yml"
    "apps/copperfin_build_host/**"
    "apps/copperfin_inspect/**"
    "apps/copperfin_runtime_host/**"
    ${common_focused_inputs})
set(declare_abi_inputs
    ".github/workflows/windows-x86-declare-validation.yml"
    ${common_focused_inputs})

require_path_filter_contract("windows-environment-validation.yml" ${environment_host_inputs})
require_path_filter_contract("executable-path-validation.yml" ${executable_path_inputs})
require_path_filter_contract("windows-x86-declare-validation.yml" ${declare_abi_inputs})
