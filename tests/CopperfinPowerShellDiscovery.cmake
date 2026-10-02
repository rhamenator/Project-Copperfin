# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

# #5809: find_program() can return a PowerShell path that does not resolve to a
# launchable regular file (on hosted macOS runners, a duplicated app-bundle path
# such as /Applications/powershell.app/Contents/MacOS/Applications/...). The
# tests that use the host canonicalize it and require a regular file, so
# validate it during configuration instead of failing at run time.
#
# Selection: the discovered executable when it resolves to a regular file;
# otherwise, on macOS only, the first real binary in a known bundle or install
# directory; otherwise an empty value, which omits the PowerShell-dependent
# tests with a diagnostic. Containment and digest checks in the code under test
# are not touched: this only chooses which host the test hands them.
function(copperfin_powershell_resolves_to_file candidate output_variable)
    set(${output_variable} FALSE PARENT_SCOPE)
    if("${candidate}" STREQUAL "" OR "${candidate}" MATCHES "-NOTFOUND$")
        return()
    endif()
    if(NOT EXISTS "${candidate}")
        return()
    endif()
    file(REAL_PATH "${candidate}" resolved_candidate)
    if(EXISTS "${resolved_candidate}" AND NOT IS_DIRECTORY "${resolved_candidate}")
        set(${output_variable} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(copperfin_select_powershell_executable
         discovered_executable apple_platform output_variable)
    copperfin_powershell_resolves_to_file("${discovered_executable}" discovered_usable)
    if(discovered_usable)
        set(${output_variable} "${discovered_executable}" PARENT_SCOPE)
        return()
    endif()

    if(apple_platform)
        file(GLOB bundle_candidates
            "/Applications/PowerShell*.app/Contents/MacOS/pwsh"
            "/Applications/powershell*.app/Contents/MacOS/pwsh"
            "/usr/local/microsoft/powershell/*/pwsh"
            "/opt/homebrew/microsoft/powershell/*/pwsh")
        foreach(bundle_candidate IN LISTS bundle_candidates)
            copperfin_powershell_resolves_to_file("${bundle_candidate}" bundle_usable)
            if(bundle_usable)
                set(${output_variable} "${bundle_candidate}" PARENT_SCOPE)
                return()
            endif()
        endforeach()
    endif()

    if(NOT "${discovered_executable}" STREQUAL "" AND
       NOT "${discovered_executable}" MATCHES "-NOTFOUND$")
        message(STATUS
            "PowerShell host '${discovered_executable}' does not resolve to a regular file; "
            "omitting the PowerShell-dependent tests (#5809)")
    endif()
    set(${output_variable} "" PARENT_SCOPE)
endfunction()
