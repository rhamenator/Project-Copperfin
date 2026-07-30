# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(SCRIPT "${SOURCE_DIR}/scripts/prepare-windows-launcher-trust.ps1")
set(VALIDATION_SCRIPT "${SOURCE_DIR}/scripts/run-windows-launcher-trust-validation.ps1")
set(WORKFLOW "${SOURCE_DIR}/.github/workflows/windows-launcher-trust-validation.yml")
foreach(REQUIRED_FILE IN ITEMS "${SCRIPT}" "${VALIDATION_SCRIPT}" "${WORKFLOW}")
    if(NOT EXISTS "${REQUIRED_FILE}")
        message(FATAL_ERROR "launcher trust provisioning file is missing: ${REQUIRED_FILE}")
    endif()
endforeach()

file(READ "${SCRIPT}" SCRIPT_CONTENT)
foreach(REQUIRED_TEXT IN ITEMS
    "RegistryHeaderPath"
    "SigningKeyPath"
    "SignerKeyId"
    "must be supplied outside the repository checkout"
    "kKnownLauncherInventoryTrustedKeys"
    "private_material_outside_repository"
    "windows-launcher-trust-provisioning"
    "signing_key_registry_binding"
)
    string(FIND "${SCRIPT_CONTENT}" "${REQUIRED_TEXT}" POSITION)
    if(POSITION EQUAL -1)
        message(FATAL_ERROR "launcher trust preflight is missing required contract text: ${REQUIRED_TEXT}")
    endif()
endforeach()

file(READ "${VALIDATION_SCRIPT}" VALIDATION_SCRIPT_CONTENT)
foreach(REQUIRED_TEXT IN ITEMS
    "valid-signed-launch"
    "modified-artifact"
    "removed-artifact"
    "removed-inventory-record"
    "duplicate-inventory-record"
    "case-ambiguous-inventory-record"
    "modified-signature-sidecar"
    "removed-signature-sidecar"
    "internal_apphost_started"
    "exit_code"
)
    string(FIND "${VALIDATION_SCRIPT_CONTENT}" "${REQUIRED_TEXT}" POSITION)
    if(POSITION EQUAL -1)
        message(FATAL_ERROR "launcher trust validation is missing required contract text: ${REQUIRED_TEXT}")
    endif()
endforeach()

file(READ "${WORKFLOW}" WORKFLOW_CONTENT)
foreach(REQUIRED_TEXT IN ITEMS
    "workflow_dispatch:"
    "secrets.COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER"
    "secrets.COPPERFIN_LAUNCHER_TRUST_SIGNING_KEY_PEM"
    "inputs.signer_key_id"
    "prepare-windows-launcher-trust.ps1"
    "-DCOPPERFIN_ENFORCE_LAUNCHER_TRUST=ON"
    "-DCOPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER"
    "run-windows-launcher-trust-validation.ps1"
    "sign-launcher-inventory.ps1"
    "test_windows_launcher_trust_fixture"
    "copperfin-launcher-trust-validation.json"
    "copperfin-windows-launcher-trust-provisioning"
)
    string(FIND "${WORKFLOW_CONTENT}" "${REQUIRED_TEXT}" POSITION)
    if(POSITION EQUAL -1)
        message(FATAL_ERROR "Windows launcher trust workflow is missing required contract text: ${REQUIRED_TEXT}")
    endif()
endforeach()

foreach(FORBIDDEN_TEXT IN ITEMS
    "-----BEGIN PRIVATE KEY-----"
    "-----BEGIN OPENSSH PRIVATE KEY-----"
    "push:"
    "pull_request:"
)
    string(FIND "${WORKFLOW_CONTENT}" "${FORBIDDEN_TEXT}" POSITION)
    if(NOT POSITION EQUAL -1)
        message(FATAL_ERROR "Windows launcher trust workflow contains forbidden text: ${FORBIDDEN_TEXT}")
    endif()
endforeach()
