# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

function(copperfin_select_rscript_executable
         discovered_executable windows_platform output_variable)
    if("${discovered_executable}" STREQUAL "" OR
       "${discovered_executable}" MATCHES "-NOTFOUND$")
        set(${output_variable} "" PARENT_SCOPE)
        return()
    endif()

    if(NOT windows_platform)
        set(${output_variable} "${discovered_executable}" PARENT_SCOPE)
        return()
    endif()

    get_filename_component(rscript_directory "${discovered_executable}" DIRECTORY)
    get_filename_component(rscript_directory_name "${rscript_directory}" NAME)
    string(TOLOWER "${rscript_directory_name}" rscript_directory_name_lower)
    if(rscript_directory_name_lower STREQUAL "x64")
        set(${output_variable} "${discovered_executable}" PARENT_SCOPE)
        return()
    endif()

    set(architecture_executable "${rscript_directory}/x64/Rscript.exe")
    if(EXISTS "${architecture_executable}" AND
       NOT IS_DIRECTORY "${architecture_executable}")
        set(${output_variable} "${architecture_executable}" PARENT_SCOPE)
    else()
        # The top-level Windows dispatcher invokes cmd.exe through system().
        # It is deliberately not admitted by Copperfin's shell-free boundary.
        set(${output_variable} "" PARENT_SCOPE)
    endif()
endfunction()
