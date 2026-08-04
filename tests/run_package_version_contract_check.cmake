# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

foreach(required_variable IN ITEMS COPPERFIN_SOURCE_DIR COPPERFIN_BINARY_DIR)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

set(version_file "${COPPERFIN_BINARY_DIR}/CopperfinPackageVersion.txt")
if(NOT EXISTS "${version_file}")
    message(FATAL_ERROR "Generated package version file does not exist: ${version_file}")
endif()
file(READ "${version_file}" package_version)
string(STRIP "${package_version}" package_version)
if(NOT package_version MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Generated package version is invalid: '${package_version}'")
endif()

foreach(manifest_path IN ITEMS
        "${COPPERFIN_SOURCE_DIR}/vsix/Copperfin.VisualStudio/source.extension.vsixmanifest"
        "${COPPERFIN_SOURCE_DIR}/vsix/Copperfin.VisualStudio/extension.vsixmanifest")
    if(NOT EXISTS "${manifest_path}")
        message(FATAL_ERROR "VSIX manifest is missing: ${manifest_path}")
    endif()
    file(READ "${manifest_path}" manifest_text)
    string(REGEX MATCH
        "<Identity[ \\t]+Id=\"Copperfin\\.VisualStudio\"[ \\t]+Version=\"[^\"]+\""
        identity_match "${manifest_text}")
    if(NOT identity_match)
        message(FATAL_ERROR "Copperfin VSIX identity is missing: ${manifest_path}")
    endif()
    string(REGEX REPLACE ".*Version=\"([^\"]+)\".*" "\\1" manifest_version "${identity_match}")
    if(NOT manifest_version STREQUAL package_version)
        message(FATAL_ERROR
            "VSIX version '${manifest_version}' in ${manifest_path} does not match '${package_version}'")
    endif()
endforeach()

set(package_source "${COPPERFIN_SOURCE_DIR}/vsix/Copperfin.VisualStudio/CopperfinPackage.cs")
if(NOT EXISTS "${package_source}")
    message(FATAL_ERROR "Visual Studio package source is missing: ${package_source}")
endif()

set(vsix_project "${COPPERFIN_SOURCE_DIR}/vsix/Copperfin.VisualStudio/Copperfin.VisualStudio.csproj")
if(NOT EXISTS "${vsix_project}")
    message(FATAL_ERROR "Visual Studio project is missing: ${vsix_project}")
endif()
file(READ "${vsix_project}" vsix_project_text)
string(REGEX MATCH
    "<UseCodebase>[ \t]*true[ \t]*</UseCodebase>"
    use_codebase_match "${vsix_project_text}")
if(NOT use_codebase_match)
    message(FATAL_ERROR
        "Visual Studio project must enable UseCodebase so the generated pkgdef registers the package DLL by CodeBase")
endif()

file(READ "${package_source}" package_text)
set(package_guids_source "${COPPERFIN_SOURCE_DIR}/vsix/Copperfin.VisualStudio/PackageGuids.cs")
if(NOT EXISTS "${package_guids_source}")
    message(FATAL_ERROR "Visual Studio package GUID source is missing: ${package_guids_source}")
endif()
file(READ "${package_guids_source}" package_guids_text)
string(FIND "${package_text}" "\"${package_version}\")]" registration_version_offset)
if(registration_version_offset EQUAL -1)
    message(FATAL_ERROR
        "InstalledProductRegistration version in ${package_source} does not match '${package_version}'")
endif()

string(REGEX MATCH "EditorDefaultPriority[ \t]*=[ \t]*([0-9]+)" editor_priority_match "${package_text}")
if(NOT editor_priority_match)
    string(REGEX MATCH "EditorDefaultPriority[ \t]*=[ \t]*([0-9]+)" editor_priority_match "${package_guids_text}")
endif()
if(NOT editor_priority_match OR NOT CMAKE_MATCH_1 STREQUAL "100")
    message(FATAL_ERROR
        "Copperfin editor default priority must remain 100 (> 0x60) so Visual Studio selects it for MVP asset extensions")
endif()

string(REGEX MATCH
    "DesignerLogicalViewString[ \t]*=[ \t]*\"\\{7651A702-06E5-11D1-8EBD-00A0C90F26EA\\}\""
    designer_view_constant_match "${package_guids_text}")
if(NOT designer_view_constant_match)
    message(FATAL_ERROR
        "Copperfin editor must retain the Visual Studio Designer logical-view GUID")
endif()

string(FIND "${package_text}" "PackageGuids.EditorDefaultPriority" editor_priority_reference)
if(editor_priority_reference EQUAL -1)
    message(FATAL_ERROR
        "Copperfin editor extensions must use the shared default-priority constant")
endif()

string(FIND "${package_text}" "ProvideEditorLogicalView" trusted_view_attribute)
string(FIND "${package_text}" "PackageGuids.DesignerLogicalViewString" trusted_view_reference)
string(FIND "${package_text}" "IsTrusted = true" trusted_view_flag)
if(trusted_view_attribute EQUAL -1 OR trusted_view_reference EQUAL -1 OR trusted_view_flag EQUAL -1)
    message(FATAL_ERROR
        "Copperfin package must register the trusted Designer logical view for hosted/automation opens")
endif()

string(REGEX MATCHALL "ProvideEditorExtension" editor_extension_matches "${package_text}")
list(LENGTH editor_extension_matches editor_extension_count)
if(NOT editor_extension_count EQUAL 6)
    message(FATAL_ERROR
        "Copperfin package must register exactly six MVP asset editor extensions; found ${editor_extension_count}")
endif()

message(STATUS "Copperfin package version contract passed: ${package_version}")
