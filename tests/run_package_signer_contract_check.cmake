# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md.

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(SIGNER "${SOURCE_DIR}/tools/package-signer/sign-launcher-inventory.sh")
set(WINDOWS_SIGNER "${SOURCE_DIR}/tools/package-signer/sign-launcher-inventory.ps1")
set(KEY_GENERATOR
    "${SOURCE_DIR}/tools/package-signer/generate-launcher-signing-key.sh")
foreach(REQUIRED_SIGNER IN ITEMS "${SIGNER}" "${WINDOWS_SIGNER}" "${KEY_GENERATOR}")
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

file(READ "${KEY_GENERATOR}" KEY_GENERATOR_CONTENT)
foreach(REQUIRED_TEXT IN ITEMS
    "--key-id"
    "--output-dir"
    "openssl genpkey -algorithm ED25519"
    "refusing to generate launcher signing material inside the repository checkout"
    "refusing to overwrite existing launcher identity output"
    "kKnownLauncherInventoryTrustedKeys"
    "COPPERFIN_LAUNCHER_TRUST_SIGNING_KEY_PEM"
    "COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER"
    "it does not sign macOS or Linux artifacts"
)
    string(FIND "${KEY_GENERATOR_CONTENT}" "${REQUIRED_TEXT}" POSITION)
    if(POSITION EQUAL -1)
        message(FATAL_ERROR
            "launcher key generator is missing required contract text: ${REQUIRED_TEXT}")
    endif()
endforeach()

if(UNIX)
    find_program(OPENSSL_EXECUTABLE openssl REQUIRED)
    find_program(SH_EXECUTABLE sh REQUIRED)
    find_program(CXX_EXECUTABLE c++ REQUIRED)
    if(DEFINED ENV{TMPDIR} AND NOT "$ENV{TMPDIR}" STREQUAL "")
        set(TEMP_BASE "$ENV{TMPDIR}")
    else()
        set(TEMP_BASE "/tmp")
    endif()
    string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef TEST_SUFFIX)
    set(TEST_ROOT "${TEMP_BASE}/copperfin-launcher-keygen-${TEST_SUFFIX}")
    set(OUTPUT_DIR "${TEST_ROOT}/identity")
    set(KEY_ID "copperfin-launcher-contract-${TEST_SUFFIX}")
    set(PRIVATE_KEY "${OUTPUT_DIR}/${KEY_ID}_launcher_private.pem")
    set(PUBLIC_KEY "${OUTPUT_DIR}/${KEY_ID}_launcher_public.pem")
    set(REGISTRY_HEADER "${OUTPUT_DIR}/${KEY_ID}_launcher_registry.h")
    set(METADATA_FILE "${OUTPUT_DIR}/${KEY_ID}_launcher_metadata.json")
    file(MAKE_DIRECTORY "${TEST_ROOT}")

    execute_process(
        COMMAND "${SH_EXECUTABLE}" "${KEY_GENERATOR}"
            --key-id "${KEY_ID}"
            --output-dir "${OUTPUT_DIR}"
        RESULT_VARIABLE GENERATOR_RESULT
        OUTPUT_VARIABLE GENERATOR_OUTPUT
        ERROR_VARIABLE GENERATOR_ERROR)
    if(NOT GENERATOR_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "launcher key generator failed: ${GENERATOR_OUTPUT}${GENERATOR_ERROR}")
    endif()
    foreach(EXPECTED_OUTPUT IN ITEMS
        "${PRIVATE_KEY}"
        "${PUBLIC_KEY}"
        "${REGISTRY_HEADER}"
        "${METADATA_FILE}")
        if(NOT EXISTS "${EXPECTED_OUTPUT}")
            file(REMOVE_RECURSE "${TEST_ROOT}")
            message(FATAL_ERROR
                "launcher key generator omitted expected output: ${EXPECTED_OUTPUT}")
        endif()
    endforeach()

    file(READ "${REGISTRY_HEADER}" GENERATED_REGISTRY)
    foreach(REQUIRED_REGISTRY_TEXT IN ITEMS
        "std::array<LauncherInventoryTrustedKey, 1>"
        "kKnownLauncherInventoryTrustedKeys"
        "\"${KEY_ID}\"")
        string(FIND "${GENERATED_REGISTRY}" "${REQUIRED_REGISTRY_TEXT}" POSITION)
        if(POSITION EQUAL -1)
            file(REMOVE_RECURSE "${TEST_ROOT}")
            message(FATAL_ERROR
                "generated launcher registry is missing: ${REQUIRED_REGISTRY_TEXT}")
        endif()
    endforeach()
    file(READ "${METADATA_FILE}" GENERATED_METADATA)
    foreach(REQUIRED_METADATA_TEXT IN ITEMS
        "\"kind\": \"copperfin-launcher-release-signing-identity\""
        "\"signature_algorithm\": \"ed25519\""
        "\"signer_key_id\": \"${KEY_ID}\""
        "\"public_key_der_sha256\":")
        string(FIND "${GENERATED_METADATA}" "${REQUIRED_METADATA_TEXT}" POSITION)
        if(POSITION EQUAL -1)
            file(REMOVE_RECURSE "${TEST_ROOT}")
            message(FATAL_ERROR
                "generated launcher metadata is missing: ${REQUIRED_METADATA_TEXT}")
        endif()
    endforeach()

    set(DERIVED_PUBLIC "${TEST_ROOT}/derived-public.pem")
    execute_process(
        COMMAND "${OPENSSL_EXECUTABLE}" pkey
            -in "${PRIVATE_KEY}"
            -pubout
            -out "${DERIVED_PUBLIC}"
        RESULT_VARIABLE DERIVE_RESULT
        OUTPUT_QUIET
        ERROR_VARIABLE DERIVE_ERROR)
    if(NOT DERIVE_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "generated private key is not a usable Ed25519 key: ${DERIVE_ERROR}")
    endif()
    file(SHA256 "${PUBLIC_KEY}" PUBLIC_PEM_SHA256)
    file(SHA256 "${DERIVED_PUBLIC}" DERIVED_PEM_SHA256)
    if(NOT PUBLIC_PEM_SHA256 STREQUAL DERIVED_PEM_SHA256)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "generated launcher public key does not match the private key")
    endif()

    set(COMPILE_PROBE "${TEST_ROOT}/registry-probe.cpp")
    file(WRITE "${COMPILE_PROBE}"
        "#include \"${KEY_ID}_launcher_registry.h\"\n"
        "static_assert(copperfin::package_trust::"
        "kKnownLauncherInventoryTrustedKeys.size() == 1);\n"
        "int main() { return 0; }\n")
    execute_process(
        COMMAND "${CXX_EXECUTABLE}" -std=c++20
            "-I${SOURCE_DIR}/include"
            "-I${OUTPUT_DIR}"
            -fsyntax-only "${COMPILE_PROBE}"
        RESULT_VARIABLE COMPILE_RESULT
        OUTPUT_VARIABLE COMPILE_OUTPUT
        ERROR_VARIABLE COMPILE_ERROR)
    if(NOT COMPILE_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "generated launcher registry does not compile: "
            "${COMPILE_OUTPUT}${COMPILE_ERROR}")
    endif()

    set(ENVELOPE "${TEST_ROOT}/app.cftrust")
    set(SIDECAR "${TEST_ROOT}/app.cftrust.sig")
    file(WRITE "${ENVELOPE}"
        "launcher_inventory_version=1\n"
        "hash_algorithm=sha256\n"
        "signature_algorithm=ed25519\n"
        "signer_key_id=${KEY_ID}\n"
        "artifact=public_apphost|CopperfinApp.exe|"
        "0000000000000000000000000000000000000000000000000000000000000000\n")
    execute_process(
        COMMAND "${SH_EXECUTABLE}" "${SIGNER}"
            --input "${ENVELOPE}"
            --output "${SIDECAR}"
            --key-ref "${PRIVATE_KEY}"
        RESULT_VARIABLE SIGN_RESULT
        OUTPUT_VARIABLE SIGN_OUTPUT
        ERROR_VARIABLE SIGN_ERROR)
    if(NOT SIGN_RESULT EQUAL 0 OR NOT EXISTS "${SIDECAR}")
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "generated launcher key failed signing smoke: ${SIGN_OUTPUT}${SIGN_ERROR}")
    endif()
    file(READ "${SIDECAR}" GENERATED_SIDECAR)
    string(REGEX MATCH
        "signature_base64=([A-Za-z0-9+/=]+)"
        SIGNATURE_MATCH
        "${GENERATED_SIDECAR}")
    if(NOT SIGNATURE_MATCH)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "generated launcher signature sidecar omitted canonical base64")
    endif()
    set(SIGNATURE_BASE64 "${CMAKE_MATCH_1}")
    set(SIGNATURE_BASE64_FILE "${TEST_ROOT}/signature.b64")
    set(SIGNATURE_RAW_FILE "${TEST_ROOT}/signature.raw")
    file(WRITE "${SIGNATURE_BASE64_FILE}" "${SIGNATURE_BASE64}")
    execute_process(
        COMMAND "${OPENSSL_EXECUTABLE}" base64 -d -A
            -in "${SIGNATURE_BASE64_FILE}"
            -out "${SIGNATURE_RAW_FILE}"
        RESULT_VARIABLE DECODE_RESULT
        OUTPUT_QUIET
        ERROR_VARIABLE DECODE_ERROR)
    execute_process(
        COMMAND "${OPENSSL_EXECUTABLE}" pkeyutl
            -verify
            -rawin
            -pubin
            -inkey "${PUBLIC_KEY}"
            -in "${ENVELOPE}"
            -sigfile "${SIGNATURE_RAW_FILE}"
        RESULT_VARIABLE VERIFY_RESULT
        OUTPUT_QUIET
        ERROR_VARIABLE VERIFY_ERROR)
    if(NOT DECODE_RESULT EQUAL 0 OR NOT VERIFY_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "generated launcher signature did not verify: "
            "${DECODE_ERROR}${VERIFY_ERROR}")
    endif()

    find_program(PWSH_EXECUTABLE pwsh)
    if(PWSH_EXECUTABLE)
        set(PROVISIONING_REPORT "${TEST_ROOT}/provisioning.json")
        execute_process(
            COMMAND "${PWSH_EXECUTABLE}" -NoProfile -File
                "${SOURCE_DIR}/scripts/prepare-windows-launcher-trust.ps1"
                -RegistryHeaderPath "${REGISTRY_HEADER}"
                -SigningKeyPath "${PRIVATE_KEY}"
                -SignerKeyId "${KEY_ID}"
                -RepositoryRoot "${SOURCE_DIR}"
                -OutputPath "${PROVISIONING_REPORT}"
            RESULT_VARIABLE PROVISION_RESULT
            OUTPUT_VARIABLE PROVISION_OUTPUT
            ERROR_VARIABLE PROVISION_ERROR)
        if(NOT PROVISION_RESULT EQUAL 0 OR NOT EXISTS "${PROVISIONING_REPORT}")
            file(REMOVE_RECURSE "${TEST_ROOT}")
            message(FATAL_ERROR
                "generated identity failed launcher provisioning preflight: "
                "${PROVISION_OUTPUT}${PROVISION_ERROR}")
        endif()
    endif()

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        execute_process(
            COMMAND stat -c "%a" "${PRIVATE_KEY}"
            RESULT_VARIABLE MODE_RESULT
            OUTPUT_VARIABLE PRIVATE_MODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_VARIABLE MODE_ERROR)
        if(NOT MODE_RESULT EQUAL 0 OR NOT PRIVATE_MODE STREQUAL "600")
            file(REMOVE_RECURSE "${TEST_ROOT}")
            message(FATAL_ERROR
                "generated private key mode is not 600: ${PRIVATE_MODE}${MODE_ERROR}")
        endif()
    endif()

    execute_process(
        COMMAND "${SH_EXECUTABLE}" "${KEY_GENERATOR}"
            --key-id "${KEY_ID}"
            --output-dir "${OUTPUT_DIR}"
        RESULT_VARIABLE OVERWRITE_RESULT
        OUTPUT_QUIET
        ERROR_QUIET)
    if(OVERWRITE_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR "launcher key generator allowed overwrite")
    endif()
    execute_process(
        COMMAND "${SH_EXECUTABLE}" "${KEY_GENERATOR}"
            --key-id "invalid/key"
            --output-dir "${TEST_ROOT}/invalid"
        RESULT_VARIABLE INVALID_ID_RESULT
        OUTPUT_QUIET
        ERROR_QUIET)
    if(INVALID_ID_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR "launcher key generator accepted an invalid key ID")
    endif()
    set(UNSAFE_OUTPUT_DIR "${TEST_ROOT}/unsafe")
    file(MAKE_DIRECTORY "${UNSAFE_OUTPUT_DIR}")
    file(CHMOD "${UNSAFE_OUTPUT_DIR}"
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    execute_process(
        COMMAND "${SH_EXECUTABLE}" "${KEY_GENERATOR}"
            --key-id "unsafe-output-${TEST_SUFFIX}"
            --output-dir "${UNSAFE_OUTPUT_DIR}"
        RESULT_VARIABLE UNSAFE_OUTPUT_RESULT
        OUTPUT_QUIET
        ERROR_QUIET)
    if(UNSAFE_OUTPUT_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "launcher key generator accepted an unsafe output directory")
    endif()
    execute_process(
        COMMAND "${SH_EXECUTABLE}" "${KEY_GENERATOR}"
            --key-id "inside-checkout-${TEST_SUFFIX}"
            --output-dir "${SOURCE_DIR}"
        RESULT_VARIABLE INSIDE_CHECKOUT_RESULT
        OUTPUT_QUIET
        ERROR_QUIET)
    if(INSIDE_CHECKOUT_RESULT EQUAL 0)
        file(REMOVE_RECURSE "${TEST_ROOT}")
        message(FATAL_ERROR
            "launcher key generator allowed output inside the checkout")
    endif()

    file(REMOVE_RECURSE "${TEST_ROOT}")
endif()
