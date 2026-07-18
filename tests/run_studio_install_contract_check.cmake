# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

foreach(required_variable IN ITEMS BINARY_DIR INSTALL_ROOT)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

if(NOT WIN32)
    message(STATUS "Standalone Studio install contract is Windows-only; skipped")
    return()
endif()

set(required_files
    "bin/studio/Copperfin.Studio.exe"
    "bin/studio/Copperfin.Studio.exe.config"
    "bin/copperfin_studio_host.exe"
)
foreach(relative_path IN LISTS required_files)
    set(installed_path "${INSTALL_ROOT}/${relative_path}")
    if(NOT EXISTS "${installed_path}" OR IS_DIRECTORY "${installed_path}" OR IS_SYMLINK "${installed_path}")
        message(FATAL_ERROR "Standalone Studio install contract is missing ${installed_path}")
    endif()
    file(SIZE "${installed_path}" installed_size)
    if(installed_size LESS_EQUAL 0)
        message(FATAL_ERROR "Standalone Studio install contract found an empty file: ${installed_path}")
    endif()
endforeach()

file(GLOB studio_entries RELATIVE "${INSTALL_ROOT}/bin/studio" "${INSTALL_ROOT}/bin/studio/*")
list(SORT studio_entries)
if(NOT studio_entries STREQUAL "Copperfin.Studio.exe;Copperfin.Studio.exe.config")
    message(FATAL_ERROR
        "Standalone Studio install directory contains unexpected files: ${studio_entries}")
endif()

message(STATUS "Standalone Studio install contract passed: ${INSTALL_ROOT}")
