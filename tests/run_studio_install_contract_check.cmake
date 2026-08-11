# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

foreach(required_variable IN ITEMS BINARY_DIR INSTALL_ROOT)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

get_filename_component(binary_dir "${BINARY_DIR}" ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
get_filename_component(install_root "${INSTALL_ROOT}" ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

if(NOT WIN32)
    message(STATUS "Standalone Studio install contract is Windows-only; skipped")
    return()
endif()

if(NOT EXISTS "${binary_dir}/CMakeCache.txt")
    message(FATAL_ERROR "Windows architecture contract is missing CMakeCache.txt")
endif()

file(STRINGS "${binary_dir}/CMakeCache.txt" pointer_size_lines
    REGEX "^COPPERFIN_NATIVE_POINTER_SIZE:INTERNAL=8$")
if(NOT pointer_size_lines)
    message(FATAL_ERROR "Windows installer native build is not configured for an x64 pointer size")
endif()
file(STRINGS "${binary_dir}/CMakeCache.txt" generator_platform_lines
    REGEX "^CMAKE_GENERATOR_PLATFORM:INTERNAL=x64$")
if(NOT generator_platform_lines)
    message(FATAL_ERROR "Windows installer generator platform is not pinned to x64")
endif()

foreach(managed_project IN ITEMS
        "${CMAKE_CURRENT_LIST_DIR}/../vsix/Copperfin.Studio/Copperfin.Studio.csproj"
        "${CMAKE_CURRENT_LIST_DIR}/../vsix/Copperfin.VisualStudio/Copperfin.VisualStudio.csproj")
    if(NOT EXISTS "${managed_project}")
        message(FATAL_ERROR "Managed x64 architecture contract is missing ${managed_project}")
    endif()
    file(STRINGS "${managed_project}" managed_platform_lines
        REGEX "^[ \\t]*<PlatformTarget>x64</PlatformTarget>[ \\t]*$")
    if(NOT managed_platform_lines)
        message(FATAL_ERROR "Managed project is not pinned to x64: ${managed_project}")
    endif()
endforeach()

set(required_files
    "bin/studio/Copperfin.Studio.exe"
    "bin/studio/Copperfin.Studio.exe.config"
    "bin/copperfin_studio_host.exe"
    "bin/copperfin_mcp_host.exe"
    "share/copperfin/locales/en-US/strings.json"
    "share/copperfin/locales/es-419/strings.json"
    "share/copperfin/locales/pt-BR/strings.json"
    "share/copperfin/locales/qps-ploc/strings.json"
)
foreach(relative_path IN LISTS required_files)
    set(installed_path "${install_root}/${relative_path}")
    if(NOT EXISTS "${installed_path}" OR IS_DIRECTORY "${installed_path}" OR IS_SYMLINK "${installed_path}")
        message(FATAL_ERROR "Standalone Studio install contract is missing ${installed_path}")
    endif()
    file(SIZE "${installed_path}" installed_size)
    if(installed_size LESS_EQUAL 0)
        message(FATAL_ERROR "Standalone Studio install contract found an empty file: ${installed_path}")
    endif()
endforeach()

file(GLOB studio_entries RELATIVE "${install_root}/bin/studio" "${install_root}/bin/studio/*")
list(SORT studio_entries)
if(NOT studio_entries STREQUAL "Copperfin.Studio.exe;Copperfin.Studio.exe.config")
    message(FATAL_ERROR
        "Standalone Studio install directory contains unexpected files: ${studio_entries}")
endif()

message(STATUS "Standalone Studio install contract passed: ${install_root}")
