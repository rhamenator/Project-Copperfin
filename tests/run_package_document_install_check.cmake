# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

foreach(required_variable IN ITEMS SOURCE_DIR BINARY_DIR TEST_ROOT_BASE)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef test_suffix)
set(install_root "${TEST_ROOT_BASE}-${test_suffix}")
set(install_command
    "${CMAKE_COMMAND}"
    --install "${BINARY_DIR}"
    --prefix "${install_root}"
    --component Documentation
)
if(DEFINED TEST_CONFIGURATION AND NOT "${TEST_CONFIGURATION}" STREQUAL "")
    list(APPEND install_command --config "${TEST_CONFIGURATION}")
endif()

execute_process(
    COMMAND ${install_command}
    RESULT_VARIABLE install_result
    OUTPUT_VARIABLE install_output
    ERROR_VARIABLE install_error
)
if(NOT install_result EQUAL 0)
    file(REMOVE_RECURSE "${install_root}")
    message(FATAL_ERROR
        "Documentation component install failed (${install_result}).\n"
        "stdout:\n${install_output}\n"
        "stderr:\n${install_error}")
endif()

set(source_document_names
    README.md
    LICENSE
    SECURITY.md
    SOURCE.md
    THIRD_PARTY_NOTICES.md
    remaining-work.md
    LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt
    docs/contracts/release-license-metadata.json)
set(installed_document_names
    README.md
    LICENSE
    SECURITY.md
    SOURCE.md
    THIRD_PARTY_NOTICES.md
    remaining-work.md
    licenses/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt
    contracts/release-license-metadata.json)
list(LENGTH source_document_names document_count)
math(EXPR last_document_index "${document_count} - 1")

foreach(document_index RANGE ${last_document_index})
    list(GET source_document_names ${document_index} source_document_name)
    list(GET installed_document_names ${document_index} installed_document_name)
    set(source_path "${SOURCE_DIR}/${source_document_name}")
    set(installed_path "${install_root}/share/copperfin/${installed_document_name}")
    if(NOT EXISTS "${installed_path}")
        file(REMOVE_RECURSE "${install_root}")
        message(FATAL_ERROR "Documentation component omitted ${installed_document_name}")
    endif()

    file(SHA256 "${source_path}" source_hash)
    file(SHA256 "${installed_path}" installed_hash)
    if(NOT source_hash STREQUAL installed_hash)
        file(REMOVE_RECURSE "${install_root}")
        message(FATAL_ERROR
            "Installed ${installed_document_name} differs from ${source_document_name}")
    endif()
endforeach()

if(EXISTS "${install_root}/share/copperfin/LICENSE.md")
    file(REMOVE_RECURSE "${install_root}")
    message(FATAL_ERROR "Documentation component duplicated the license as LICENSE.md")
endif()

file(REMOVE_RECURSE "${install_root}")
