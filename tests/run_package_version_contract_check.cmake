# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

foreach(required_variable IN ITEMS SOURCE_DIR BINARY_DIR)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

set(version_file "${BINARY_DIR}/CopperfinPackageVersion.txt")
if(NOT EXISTS "${version_file}")
    message(FATAL_ERROR "Generated package version file does not exist: ${version_file}")
endif()
file(READ "${version_file}" package_version)
string(STRIP "${package_version}" package_version)
if(NOT package_version MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Generated package version is invalid: '${package_version}'")
endif()

foreach(manifest_path IN ITEMS
        "${SOURCE_DIR}/vsix/Copperfin.VisualStudio/source.extension.vsixmanifest"
        "${SOURCE_DIR}/vsix/Copperfin.VisualStudio/extension.vsixmanifest")
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

set(package_source "${SOURCE_DIR}/vsix/Copperfin.VisualStudio/CopperfinPackage.cs")
if(NOT EXISTS "${package_source}")
    message(FATAL_ERROR "Visual Studio package source is missing: ${package_source}")
endif()
file(READ "${package_source}" package_text)
string(FIND "${package_text}" "\"${package_version}\")]" registration_version_offset)
if(registration_version_offset EQUAL -1)
    message(FATAL_ERROR
        "InstalledProductRegistration version in ${package_source} does not match '${package_version}'")
endif()

message(STATUS "Copperfin package version contract passed: ${package_version}")
