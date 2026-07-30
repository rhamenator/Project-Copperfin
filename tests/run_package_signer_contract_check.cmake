# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md.

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(SIGNER "${SOURCE_DIR}/tools/package-signer/sign-launcher-inventory.sh")
set(WINDOWS_SIGNER "${SOURCE_DIR}/tools/package-signer/sign-launcher-inventory.ps1")
foreach(REQUIRED_SIGNER IN ITEMS "${SIGNER}" "${WINDOWS_SIGNER}")
    if(NOT EXISTS "${REQUIRED_SIGNER}")
        message(FATAL_ERROR "package signer is missing: ${REQUIRED_SIGNER}")
    endif()
endforeach()

file(READ "${SIGNER}" CONTENT)
foreach(REQUIRED_TEXT IN ITEMS
    "--input"
    "--output"
    "--key-ref"
    "openssl pkeyutl -sign -rawin"
    "launcher_signature_version=1"
    "signature_algorithm=ed25519"
    "input envelope must end with a single LF byte"
    "refusing a signing key inside the repository checkout"
)
    string(FIND "${CONTENT}" "${REQUIRED_TEXT}" POSITION)
    if(POSITION EQUAL -1)
        message(FATAL_ERROR "package signer is missing required contract text: ${REQUIRED_TEXT}")
    endif()
endforeach()

if(CONTENT MATCHES "private_key|private key material")
    message(FATAL_ERROR "package signer must not embed private key material")
endif()

file(READ "${WINDOWS_SIGNER}" WINDOWS_CONTENT)
foreach(REQUIRED_TEXT IN ITEMS
    "InputPath"
    "OutputPath"
    "KeyRef"
    "openssl pkeyutl -sign -rawin"
    "launcher_signature_version=1"
    "signature_algorithm=ed25519"
    "canonical LF line endings"
    "Refusing a signing key inside the repository checkout"
)
    string(FIND "${WINDOWS_CONTENT}" "${REQUIRED_TEXT}" POSITION)
    if(POSITION EQUAL -1)
        message(FATAL_ERROR "Windows package signer is missing required contract text: ${REQUIRED_TEXT}")
    endif()
endforeach()

if(WINDOWS_CONTENT MATCHES "-----BEGIN.*PRIVATE KEY-----")
    message(FATAL_ERROR "Windows package signer must not embed private key material")
endif()
