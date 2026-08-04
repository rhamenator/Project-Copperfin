# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED INSTALL_ROOT OR "${INSTALL_ROOT}" STREQUAL "")
    message(FATAL_ERROR "INSTALL_ROOT is required")
endif()

set(locale_install_root "${INSTALL_ROOT}/share/copperfin/locales")
foreach(locale IN ITEMS en-US es-419 pt-BR qps-ploc)
    set(catalog_path "${locale_install_root}/${locale}/strings.json")
    if(NOT EXISTS "${catalog_path}" OR IS_DIRECTORY "${catalog_path}" OR IS_SYMLINK "${catalog_path}")
        message(FATAL_ERROR "Installed locale catalog is missing or not a regular file: ${catalog_path}")
    endif()
    file(SIZE "${catalog_path}" catalog_size)
    if(catalog_size LESS_EQUAL 0)
        message(FATAL_ERROR "Installed locale catalog is empty: ${catalog_path}")
    endif()
endforeach()

message(STATUS "Installed locale catalog contract passed: ${locale_install_root}")
