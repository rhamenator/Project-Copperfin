# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED BINARY_DIR OR "${BINARY_DIR}" STREQUAL "")
    message(FATAL_ERROR "BINARY_DIR is required")
endif()
if(NOT DEFINED DISCOVERY_MODULE OR "${DISCOVERY_MODULE}" STREQUAL "")
    message(FATAL_ERROR "DISCOVERY_MODULE is required")
endif()

include("${DISCOVERY_MODULE}")

set(fixture_root "${BINARY_DIR}/powershell-discovery-contract")
file(REMOVE_RECURSE "${fixture_root}")
file(MAKE_DIRECTORY "${fixture_root}/bin")
file(MAKE_DIRECTORY "${fixture_root}/bundle/powershell.app/Contents/MacOS")
file(WRITE "${fixture_root}/bin/pwsh" "regular host fixture")
file(WRITE "${fixture_root}/bundle/powershell.app/Contents/MacOS/PowerShell.sh" "launcher fixture")

# A regular file is kept as discovered, on every platform.
copperfin_select_powershell_executable("${fixture_root}/bin/pwsh" FALSE selected)
if(NOT "${selected}" STREQUAL "${fixture_root}/bin/pwsh")
    message(FATAL_ERROR "a regular PowerShell host was not preserved: ${selected}")
endif()

# A symlink to a regular file is kept (the test canonicalizes it itself).
if(NOT CMAKE_HOST_WIN32)
    file(CREATE_LINK "${fixture_root}/bin/pwsh" "${fixture_root}/bin/pwsh-link" SYMBOLIC)
    copperfin_select_powershell_executable("${fixture_root}/bin/pwsh-link" FALSE selected)
    if(NOT "${selected}" STREQUAL "${fixture_root}/bin/pwsh-link")
        message(FATAL_ERROR "a symlinked PowerShell host was not preserved: ${selected}")
    endif()

    # A dangling symlink is rejected and, with nothing to fall back to, omitted.
    file(CREATE_LINK "${fixture_root}/does-not-exist" "${fixture_root}/bin/pwsh-dangling" SYMBOLIC)
    copperfin_select_powershell_executable("${fixture_root}/bin/pwsh-dangling" FALSE selected)
    if(NOT "${selected}" STREQUAL "")
        message(FATAL_ERROR "a dangling PowerShell symlink was admitted: ${selected}")
    endif()
endif()

# The duplicated, nonexistent app-bundle path from #5809 is rejected.
set(duplicated
    "${fixture_root}/bundle/powershell.app/Contents/MacOS${fixture_root}/bundle/powershell.app/Contents/MacOS/PowerShell.sh")
copperfin_select_powershell_executable("${duplicated}" FALSE selected)
if(NOT "${selected}" STREQUAL "")
    message(FATAL_ERROR "the duplicated app-bundle path was admitted: ${selected}")
endif()
copperfin_select_powershell_executable("${duplicated}" TRUE selected)
# On macOS a bundle binary may be substituted; whatever is chosen must be real.
if(NOT "${selected}" STREQUAL "")
    copperfin_powershell_resolves_to_file("${selected}" selected_usable)
    if(NOT selected_usable)
        message(FATAL_ERROR "macOS fallback chose an unusable PowerShell host: ${selected}")
    endif()
endif()

# Nothing discovered stays empty.
copperfin_select_powershell_executable("pwsh-NOTFOUND" TRUE selected)
if(NOT "${selected}" STREQUAL "" AND
   NOT "${selected}" MATCHES "^/(Applications|usr/local|opt/homebrew)/")
    message(FATAL_ERROR "an undiscovered PowerShell host produced a non-bundle result: ${selected}")
endif()

file(REMOVE_RECURSE "${fixture_root}")
message(STATUS "PowerShell discovery contract passed")
