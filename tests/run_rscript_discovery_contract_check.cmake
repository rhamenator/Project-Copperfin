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

set(fixture_root "${BINARY_DIR}/rscript-discovery-contract")
file(REMOVE_RECURSE "${fixture_root}")
file(MAKE_DIRECTORY "${fixture_root}/bin/x64")
file(WRITE "${fixture_root}/bin/Rscript.exe" "dispatcher fixture")
file(WRITE "${fixture_root}/bin/x64/Rscript.exe" "direct fixture")

set(dispatcher "${fixture_root}/bin/Rscript.exe")
set(direct "${fixture_root}/bin/x64/Rscript.exe")
copperfin_select_rscript_executable("${dispatcher}" TRUE selected)
if(NOT "${selected}" STREQUAL "${direct}")
    message(FATAL_ERROR
        "Windows Rscript discovery did not replace the shell dispatcher: ${selected}")
endif()

copperfin_select_rscript_executable("${direct}" TRUE selected)
if(NOT "${selected}" STREQUAL "${direct}")
    message(FATAL_ERROR
        "Windows Rscript discovery did not preserve the direct x64 front end")
endif()

copperfin_select_rscript_executable("${dispatcher}" FALSE selected)
if(NOT "${selected}" STREQUAL "${dispatcher}")
    message(FATAL_ERROR "POSIX Rscript discovery unexpectedly rewrote its executable")
endif()

file(REMOVE "${direct}")
copperfin_select_rscript_executable("${dispatcher}" TRUE selected)
if(NOT "${selected}" STREQUAL "")
    message(FATAL_ERROR
        "Windows Rscript discovery admitted the shell dispatcher without a direct front end")
endif()

file(REMOVE_RECURSE "${fixture_root}")
message(STATUS "Rscript discovery contract passed")
