# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
    "registry key records do not match the declared array size"
    "duplicate signer key ID"
    "HashSet[string]"
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
    "Substring(\$inventoryPrefix.Length)"
    "modified-signature-sidecar"
    "removed-signature-sidecar"
    "internal_apphost_started"
    "exit_code"
    "package_relative_name"
    "sha256"
)
    string(FIND "${VALIDATION_SCRIPT_CONTENT}" "${REQUIRED_TEXT}" POSITION)
    if(POSITION EQUAL -1)
        message(FATAL_ERROR "launcher trust validation is missing required contract text: ${REQUIRED_TEXT}")
    endif()
endforeach()

file(READ "${WORKFLOW}" WORKFLOW_CONTENT)
foreach(REQUIRED_TEXT IN ITEMS
    "workflow_dispatch:"
    "    environment: release"
    "permissions:\n  contents: read\n\njobs:"
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

string(REGEX MATCHALL "contents:[ ]*read" CONTENTS_READ_PERMISSIONS
    "${WORKFLOW_CONTENT}")
list(LENGTH CONTENTS_READ_PERMISSIONS CONTENTS_READ_PERMISSION_COUNT)
if(NOT CONTENTS_READ_PERMISSION_COUNT EQUAL 1)
    message(FATAL_ERROR
        "Windows launcher trust workflow must retain exactly one contents: read permission")
endif()

string(REGEX MATCHALL "environment:[ ]*release" RELEASE_ENVIRONMENT_BINDINGS
    "${WORKFLOW_CONTENT}")
list(LENGTH RELEASE_ENVIRONMENT_BINDINGS RELEASE_ENVIRONMENT_BINDING_COUNT)
if(NOT RELEASE_ENVIRONMENT_BINDING_COUNT EQUAL 1)
    message(FATAL_ERROR
        "Windows launcher trust workflow must bind exactly one job to the fixed release environment")
endif()

string(REGEX MATCH "environment:[^\n]*\\$\\{\\{" DYNAMIC_ENVIRONMENT_BINDING
    "${WORKFLOW_CONTENT}")
if(DYNAMIC_ENVIRONMENT_BINDING)
    message(FATAL_ERROR
        "Windows launcher trust workflow must not select its protected environment dynamically")
endif()

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

string(FIND "${WORKFLOW_CONTENT}"
    "COPPERFIN_TRUST_SIGNER_KEY_ID=\$env:COPPERFIN_TRUST_SIGNER_KEY_ID"
    UNVALIDATED_SIGNER_ENV_PUBLICATION)
if(NOT UNVALIDATED_SIGNER_ENV_PUBLICATION EQUAL -1)
    message(FATAL_ERROR
        "Windows launcher trust workflow must not publish signer input before preflight validation")
endif()
