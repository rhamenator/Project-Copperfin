function(copperfin_add_test TARGET_NAME LINK_LIBS)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} PRIVATE ${LINK_LIBS})

    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
endfunction()

copperfin_add_test(test_mcp_host "cf_mcp_host;cf_platform_support"
    test_mcp_host.cpp)
copperfin_add_test(test_project_inventory "cf_migration"
    test_project_inventory.cpp)
add_test(
    NAME test_mcp_host_stdio
    COMMAND ${CMAKE_COMMAND}
        -DMCP_HOST=$<TARGET_FILE:copperfin_mcp_host>
        -DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_mcp_host_stdio.cmake
)

function(copperfin_add_test_support_object TARGET_NAME SOURCE_FILE LINK_LIBS)
    add_library(${TARGET_NAME} OBJECT ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE ${LINK_LIBS})

    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

function(copperfin_reuse_test_support_object SOURCE_FILE OBJECT_TARGET)
    if(NOT TARGET ${OBJECT_TARGET})
        message(FATAL_ERROR "Unknown reusable test-support object target: ${OBJECT_TARGET}")
    endif()

    set(copperfin_reuse_count 0)
    get_property(copperfin_reuse_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(copperfin_reuse_target IN LISTS copperfin_reuse_targets)
        get_target_property(copperfin_reuse_type ${copperfin_reuse_target} TYPE)
        if(NOT copperfin_reuse_type STREQUAL "EXECUTABLE")
            continue()
        endif()

        get_target_property(copperfin_reuse_sources ${copperfin_reuse_target} SOURCES)
        if(NOT SOURCE_FILE IN_LIST copperfin_reuse_sources)
            continue()
        endif()

        list(REMOVE_ITEM copperfin_reuse_sources ${SOURCE_FILE})
        set_property(TARGET ${copperfin_reuse_target} PROPERTY SOURCES "${copperfin_reuse_sources}")
        target_sources(${copperfin_reuse_target} PRIVATE $<TARGET_OBJECTS:${OBJECT_TARGET}>)
        math(EXPR copperfin_reuse_count "${copperfin_reuse_count} + 1")
    endforeach()

    if(copperfin_reuse_count EQUAL 0)
        message(FATAL_ERROR "Reusable test-support source is not used by an executable: ${SOURCE_FILE}")
    endif()
endfunction()

function(copperfin_reuse_test_support_object_for_targets
         SOURCE_FILE OBJECT_TARGET EXPECTED_DEFINITIONS)
    if(NOT TARGET ${OBJECT_TARGET})
        message(FATAL_ERROR "Unknown targeted test-support object: ${OBJECT_TARGET}")
    endif()

    foreach(copperfin_reuse_target IN LISTS ARGN)
        if(NOT TARGET ${copperfin_reuse_target})
            message(FATAL_ERROR "Unknown targeted test executable: ${copperfin_reuse_target}")
        endif()

        get_target_property(copperfin_reuse_sources ${copperfin_reuse_target} SOURCES)
        if(NOT SOURCE_FILE IN_LIST copperfin_reuse_sources)
            message(FATAL_ERROR
                "Target ${copperfin_reuse_target} does not compile reusable source ${SOURCE_FILE}")
        endif()

        get_target_property(
            copperfin_reuse_definitions
            ${copperfin_reuse_target}
            COMPILE_DEFINITIONS
        )
        if(copperfin_reuse_definitions STREQUAL "copperfin_reuse_definitions-NOTFOUND")
            set(copperfin_reuse_definitions)
        endif()
        set(copperfin_expected_definitions ${EXPECTED_DEFINITIONS})
        list(SORT copperfin_reuse_definitions)
        list(SORT copperfin_expected_definitions)
        if(NOT "${copperfin_reuse_definitions}" STREQUAL "${copperfin_expected_definitions}")
            message(FATAL_ERROR
                "Target ${copperfin_reuse_target} compile definitions no longer match reusable object ${OBJECT_TARGET}")
        endif()

        list(REMOVE_ITEM copperfin_reuse_sources ${SOURCE_FILE})
        set_property(TARGET ${copperfin_reuse_target} PROPERTY SOURCES "${copperfin_reuse_sources}")
        target_sources(${copperfin_reuse_target} PRIVATE $<TARGET_OBJECTS:${OBJECT_TARGET}>)
    endforeach()
endfunction()

add_library(cf_test_process_capture_support STATIC
    test_process_capture_support.cpp
)
if(MSVC)
    target_compile_options(cf_test_process_capture_support PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_test_process_capture_support PRIVATE -Wall -Wextra -Wpedantic)
endif()

# These support translation units do not consume per-executable compile
# definitions. Compile them once and reuse their objects across test binaries.
copperfin_add_test_support_object(
    cf_prg_engine_test_support
    "${CMAKE_CURRENT_SOURCE_DIR}/prg_engine_test_support.cpp"
    cf_xbase_runtime
)
copperfin_add_test_support_object(
    cf_studio_host_json_shared_test_helpers
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_shared_test_helpers.cpp"
    cf_vfp_assets
)
target_compile_definitions(
    cf_studio_host_json_shared_test_helpers
    PRIVATE COPPERFIN_TEST_LOCALE_DIR="${PROJECT_SOURCE_DIR}/resources/locales"
)
copperfin_add_test_support_object(
    cf_studio_host_json_report_shared_fixtures
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_report_shared_fixtures.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_objects_dhf_shared_fixtures
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_objects_dhf_shared_fixtures.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_sections_dhf_shared_fixtures
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_sections_dhf_shared_fixtures.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_report_settings_shared_fixtures
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_report_settings_shared_fixtures.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_report_section_shared_fixtures
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_report_section_shared_fixtures.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_geometry_preview_helpers
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_geometry_preview_helpers.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_geometry_preview_helpers_with_unresolved
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_geometry_preview_bounds.cpp"
    cf_vfp_assets
)
target_compile_definitions(
    cf_studio_host_json_geometry_preview_helpers_with_unresolved
    PRIVATE
        COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
        COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
)
copperfin_add_test_support_object(
    cf_studio_host_json_objects_dhf_expressions_and_exposure
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_objects_dhf_expressions_and_exposure.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_objects_layout_and_expressions
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_objects_layout_and_expressions.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_report_object_fallbacks
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_report_object_fallbacks.cpp"
    cf_vfp_assets
)
copperfin_add_test_support_object(
    cf_studio_host_json_geometry_dimensions_with_unresolved
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_geometry_dimensions.cpp"
    cf_vfp_assets
)
target_compile_definitions(
    cf_studio_host_json_geometry_dimensions_with_unresolved
    PRIVATE
        COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
        COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
)
copperfin_add_test_support_object(
    cf_studio_host_json_sections_diagnostics_with_unresolved
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_sections_diagnostics_and_misc.cpp"
    cf_vfp_assets
)
target_compile_definitions(
    cf_studio_host_json_sections_diagnostics_with_unresolved
    PRIVATE
        COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
        COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
)
copperfin_add_test_support_object(
    cf_studio_host_json_settings_column_setup_helpers
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_settings_column_setup.cpp"
    cf_vfp_assets
)
target_compile_definitions(
    cf_studio_host_json_settings_column_setup_helpers
    PRIVATE COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)
copperfin_add_test_support_object(
    cf_studio_host_json_settings_page_layout_helpers
    "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_settings_page_layout_fields.cpp"
    cf_vfp_assets
)
target_sources(
    cf_studio_host_json_settings_page_layout_helpers
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/test_studio_host_json_settings_bottom_margin.cpp"
)
target_compile_definitions(
    cf_studio_host_json_settings_page_layout_helpers
    PRIVATE COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

find_program(DOTNET_EXECUTABLE NAMES dotnet)
if(DOTNET_EXECUTABLE)
    add_test(
        NAME test_managed_compile
        COMMAND ${CMAKE_COMMAND}
            -DDOTNET_EXECUTABLE=${DOTNET_EXECUTABLE}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DTEST_CONFIGURATION=$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_managed_compile_check.cmake
    )
endif()

add_test(
    NAME test_package_document_install
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DTEST_ROOT_BASE=${CMAKE_CURRENT_BINARY_DIR}/package-document-install
        -DTEST_CONFIGURATION=$<CONFIG>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_package_document_install_check.cmake
)

add_test(
    NAME test_locale_catalog_install_contract
    COMMAND ${CMAKE_COMMAND}
        -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/locale-catalog-install-contract
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_locale_catalog_install_contract_test.cmake
)

add_test(
    NAME test_studio_install_contract
    COMMAND ${CMAKE_COMMAND}
        -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/studio-install-contract
        -DTEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/studio-install-contract-binary
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_studio_install_contract_test.cmake
)

add_test(
    NAME test_cpack_artifact_contract
    COMMAND ${CMAKE_COMMAND}
        -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/cpack-artifact-contract
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_cpack_artifact_contract_test.cmake
)

add_test(
    NAME test_build_parallelism_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_build_parallelism_contract_check.cmake
)

add_test(
    NAME test_vfp9_descending_observation_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_vfp9_descending_observation_contract_check.cmake
)

add_test(
    NAME test_package_signer_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_package_signer_contract_check.cmake
)

find_program(POWERSHELL_EXECUTABLE NAMES pwsh pwsh.exe powershell powershell.exe)
if(POWERSHELL_EXECUTABLE)
    add_test(
        NAME test_agent_issue_intake
        COMMAND ${POWERSHELL_EXECUTABLE}
            -NoLogo
            -NoProfile
            -NonInteractive
            -File ${CMAKE_CURRENT_SOURCE_DIR}/test_agent_issue_intake.ps1
            -ModulePath ${CMAKE_SOURCE_DIR}/scripts/Copperfin.AgentIssueIntake.psm1
            -DriverPath ${CMAKE_SOURCE_DIR}/scripts/drive-codex.ps1
            -AgentGuidancePath ${CMAKE_SOURCE_DIR}/agents.md
            -RuntimeAgentPath ${CMAKE_SOURCE_DIR}/.github/agents/copperfin-vfp-runtime.agent.md
            -SafetyReportPath ${CMAKE_SOURCE_DIR}/docs/safety/traceability-report-2026-08-10-agent-intake-scope-approval.md
    )
endif()

add_test(
    NAME test_product_licensing_policy_contract
    COMMAND ${CMAKE_COMMAND}
        -DCOPPERFIN_SOURCE_ROOT=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_product_licensing_policy_contract_check.cmake
)

add_test(
    NAME test_repository_community_contract
    COMMAND ${CMAKE_COMMAND}
        -DCOPPERFIN_SOURCE_ROOT=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_repository_community_contract_check.cmake
)

add_test(
    NAME test_release_licensing_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_release_licensing_contract_check.cmake
)

add_test(
    NAME test_rc_candidate_workflow_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rc_candidate_workflow_contract_check.cmake
)

find_program(COPPERFIN_PYTHON3_EXECUTABLE NAMES python3 python)
find_program(COPPERFIN_RSCRIPT_EXECUTABLE NAMES Rscript Rscript.exe)
include(${CMAKE_CURRENT_SOURCE_DIR}/CopperfinRscriptDiscovery.cmake)
copperfin_select_rscript_executable(
    "${COPPERFIN_RSCRIPT_EXECUTABLE}"
    "${WIN32}"
    COPPERFIN_RSCRIPT_EXECUTABLE
)
if(COPPERFIN_PYTHON3_EXECUTABLE)
    add_test(
        NAME test_rc_candidate_assembly
        COMMAND ${COPPERFIN_PYTHON3_EXECUTABLE}
            ${CMAKE_SOURCE_DIR}/scripts/assemble-rc-candidate.py
            --self-test
    )

    add_test(
        NAME test_contributor_signoff_contract
        COMMAND ${COPPERFIN_PYTHON3_EXECUTABLE}
            ${CMAKE_SOURCE_DIR}/scripts/check-contributor-signoffs.py
            --self-test
    )
endif()

add_test(
    NAME test_launcher_trust_provisioning_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_launcher_trust_provisioning_contract_check.cmake
)

add_test(
    NAME test_rscript_discovery_contract
    COMMAND ${CMAKE_COMMAND}
        -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DDISCOVERY_MODULE=${CMAKE_CURRENT_SOURCE_DIR}/CopperfinRscriptDiscovery.cmake
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rscript_discovery_contract_check.cmake
)

add_test(
    NAME test_polyglot_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_polyglot_contract_check.cmake
)

if(UNIX)
    add_test(
        NAME test_validate_posix_build_type
        COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DBINARY_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_validate_posix_build_type_check.cmake
    )
endif()

add_test(
    NAME test_native_platform_workflow_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_native_platform_workflow_contract_check.cmake
)

add_test(
    NAME test_github_actions_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_github_actions_contract_check.cmake
)

add_test(
    NAME test_windows_msvc_cache_workflow_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_windows_msvc_cache_workflow_contract_check.cmake
)

add_test(
    NAME test_agent_channel_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_agent_channel_contract_check.cmake
)

add_test(
    NAME test_windows_installer_lifecycle_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_windows_installer_lifecycle_contract_check.cmake
)

add_test(
    NAME test_windows_vsix_lifecycle_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_windows_vsix_lifecycle_contract_check.cmake
)

add_test(
    NAME test_native_test_isolation_contract
    COMMAND ${CMAKE_COMMAND}
        -DINVENTORY_PATH=${CMAKE_BINARY_DIR}/native-test-isolation.tsv
        -DISOLATION_SOURCE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/CopperfinTestIsolation.cmake
        -DTEST_REGISTRATION_SOURCE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_native_test_isolation_contract_check.cmake
)

add_test(
    NAME test_security_supply_chain_workflow_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_security_supply_chain_workflow_contract_check.cmake
)

add_test(
    NAME test_safety_traceability_workflow_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_safety_traceability_workflow_contract_check.cmake
)
set_tests_properties(test_safety_traceability_workflow_contract PROPERTIES
    TIMEOUT 1200
)

add_test(
    NAME test_runtime_host_binding
    COMMAND ${CMAKE_COMMAND}
        -DRUNTIME_HOST_EXECUTABLE=$<TARGET_FILE:copperfin_runtime_host>
        -DLOCALE_ROOT=${CMAKE_SOURCE_DIR}/resources/locales
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_runtime_host_binding_check.cmake
)

add_executable(test_localization
    test_localization_catalog.cpp
    test_localization_runtime_session.cpp
    test_localization_runtime_dispatch.cpp
    test_localization_runtime_io.cpp
    test_localization_runtime_misc.cpp
    test_localization_host_inspect.cpp
    test_localization_main.cpp
    prg_engine_test_support.cpp
)
target_link_libraries(test_localization PRIVATE cf_localization cf_runtime_pipeline cf_xbase_runtime)
add_dependencies(test_localization copperfin_inspect)
target_compile_definitions(
    test_localization
    PRIVATE COPPERFIN_TEST_LOCALE_DIR="${PROJECT_SOURCE_DIR}/resources/locales"
)

if(MSVC)
    target_compile_options(test_localization PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_localization PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_localization
    COMMAND $<TARGET_FILE:test_localization> $<TARGET_FILE:copperfin_inspect>
)
# The product-catalog parity checks run from an out-of-tree build on Windows,
# so provide the checkout's catalog fixture explicitly. Discovery cases
# deliberately exercise an absent override and clear this scoped environment.
set_tests_properties(
    test_localization
    PROPERTIES ENVIRONMENT "COPPERFIN_LOCALE_DIR=${CMAKE_SOURCE_DIR}/resources/locales"
)

foreach(runtime_host_test_target IN ITEMS
        test_runtime_host_debug_output_formatting
        test_runtime_host_audit_stream)
    if(runtime_host_test_target STREQUAL "test_runtime_host_debug_output_formatting")
        set(runtime_host_test_sources
            test_runtime_host_debug_output_formatting_xasset.cpp
            test_runtime_host_debug_output_formatting_bridge.cpp
            test_runtime_host_debug_output_formatting_contract.cpp
            test_runtime_host_debug_output_localization.cpp
            test_runtime_host_debug_output_formatting_main.cpp
        )
    else()
        set(runtime_host_test_sources
            test_runtime_host_audit_snapshots.cpp
            test_runtime_host_audit_package.cpp
            test_runtime_host_audit_containment.cpp
            test_runtime_host_audit_contracts.cpp
            test_runtime_host_audit_main.cpp
        )
    endif()
    add_executable(${runtime_host_test_target} ${runtime_host_test_sources})
    target_link_libraries(${runtime_host_test_target} PRIVATE cf_design_model cf_xbase_runtime cf_vfp_assets)
    target_link_libraries(${runtime_host_test_target} PRIVATE cf_security)
    copperfin_apply_windows_stack_reserve(${runtime_host_test_target})
    if(runtime_host_test_target STREQUAL "test_runtime_host_debug_output_formatting")
        target_link_libraries(${runtime_host_test_target} PRIVATE cf_test_process_capture_support)
    endif()
    add_dependencies(${runtime_host_test_target} copperfin_runtime_host)

    if(MSVC)
        target_compile_options(${runtime_host_test_target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${runtime_host_test_target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(
        NAME ${runtime_host_test_target}
        COMMAND $<TARGET_FILE:${runtime_host_test_target}> $<TARGET_FILE:copperfin_runtime_host>
    )
endforeach()

add_executable(test_runtime_host_audit_containment
    test_runtime_host_audit_containment.cpp
    test_runtime_host_audit_containment_main.cpp
)
target_link_libraries(test_runtime_host_audit_containment PRIVATE cf_security cf_platform_support cf_vfp_assets)
add_dependencies(test_runtime_host_audit_containment copperfin_runtime_host)

if(MSVC)
    target_compile_options(test_runtime_host_audit_containment PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_runtime_host_audit_containment PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_runtime_host_audit_containment
    COMMAND
        $<TARGET_FILE:test_runtime_host_audit_containment>
        $<TARGET_FILE:copperfin_runtime_host>
        ${CMAKE_SOURCE_DIR}/resources/locales
)

add_executable(test_runtime_host_implicit_path_launch
    test_runtime_host_implicit_path_launch.cpp
)
target_link_libraries(test_runtime_host_implicit_path_launch PRIVATE cf_platform_support)
add_dependencies(test_runtime_host_implicit_path_launch copperfin_runtime_host)

if(MSVC)
    target_compile_options(test_runtime_host_implicit_path_launch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_runtime_host_implicit_path_launch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_runtime_host_implicit_path_launch
    COMMAND
        $<TARGET_FILE:test_runtime_host_implicit_path_launch>
        $<TARGET_FILE:copperfin_runtime_host>
)

add_executable(test_tool_license_path_launch
    test_tool_license_path_launch.cpp
)
target_link_libraries(test_tool_license_path_launch PRIVATE cf_platform_support)
add_dependencies(test_tool_license_path_launch copperfin_build_host copperfin_inspect)

if(MSVC)
    target_compile_options(test_tool_license_path_launch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_tool_license_path_launch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_tool_license_path_launch
    COMMAND
        $<TARGET_FILE:test_tool_license_path_launch>
        $<TARGET_FILE:copperfin_build_host>
        $<TARGET_FILE:copperfin_inspect>
)

add_executable(test_build_host_output
    test_build_host_output.cpp
)
target_link_libraries(test_build_host_output PRIVATE cf_vfp_assets ${CMAKE_DL_LIBS})
add_dependencies(test_build_host_output copperfin_build_host copperfin_runtime_host)
target_compile_definitions(
    test_build_host_output
    PRIVATE COPPERFIN_TEST_LOCALE_DIR="${PROJECT_SOURCE_DIR}/resources/locales"
)

if(MSVC)
    target_compile_options(test_build_host_output PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_build_host_output PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_build_host_output
    COMMAND $<TARGET_FILE:test_build_host_output> $<TARGET_FILE:copperfin_build_host>
)

add_executable(test_build_host_utf8_launcher_paths
    test_build_host_utf8_launcher_paths.cpp
)
target_link_libraries(test_build_host_utf8_launcher_paths PRIVATE
    cf_vfp_assets
    cf_test_process_capture_support
)
target_compile_definitions(
    test_build_host_utf8_launcher_paths
    PRIVATE COPPERFIN_TEST_LOCALE_DIR="${PROJECT_SOURCE_DIR}/resources/locales"
)
add_dependencies(test_build_host_utf8_launcher_paths copperfin_build_host copperfin_runtime_host)

if(MSVC)
    target_compile_options(test_build_host_utf8_launcher_paths PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_build_host_utf8_launcher_paths PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32 AND DOTNET_EXECUTABLE)
    add_test(
        NAME test_build_host_utf8_launcher_paths
        COMMAND $<TARGET_FILE:test_build_host_utf8_launcher_paths>
            $<TARGET_FILE:copperfin_build_host>
    )
else()
    add_test(
        NAME test_build_host_utf8_launcher_paths
        COMMAND $<TARGET_FILE:test_build_host_utf8_launcher_paths>
    )
endif()
set_tests_properties(test_build_host_utf8_launcher_paths PROPERTIES
    SKIP_RETURN_CODE 77
    TIMEOUT 600
)

add_executable(test_studio_host_shell_command
    test_studio_host_shell_command.cpp
    ../apps/copperfin_studio_host/studio_host_main_shell_command.cpp
)
target_link_libraries(test_studio_host_shell_command PRIVATE
    cf_platform_support
)
target_include_directories(test_studio_host_shell_command PRIVATE
    ${CMAKE_SOURCE_DIR}/apps/copperfin_studio_host
    ${CMAKE_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(test_studio_host_shell_command PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_shell_command PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_shell_command
    COMMAND $<TARGET_FILE:test_studio_host_shell_command>
)

add_executable(test_studio_host_utf8_arguments
    test_studio_host_utf8_arguments.cpp
)
target_link_libraries(test_studio_host_utf8_arguments PRIVATE
    cf_platform_support
    cf_test_process_capture_support
)
add_dependencies(test_studio_host_utf8_arguments copperfin_studio_host)

if(MSVC)
    target_compile_options(test_studio_host_utf8_arguments PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_utf8_arguments PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_utf8_arguments
    COMMAND
        $<TARGET_FILE:test_studio_host_utf8_arguments>
        $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_licensing_status
    test_licensing_status.cpp
)
target_link_libraries(test_licensing_status PRIVATE cf_licensing cf_localization)
target_include_directories(test_licensing_status PRIVATE ${CMAKE_SOURCE_DIR}/src/licensing)

if(MSVC)
    target_compile_options(test_licensing_status PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_licensing_status PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_licensing_status
    COMMAND $<TARGET_FILE:test_licensing_status>
)

add_executable(test_package_launcher_inventory_trust
    test_package_launcher_inventory_trust.cpp
)
target_link_libraries(test_package_launcher_inventory_trust PRIVATE cf_package_trust)
target_include_directories(test_package_launcher_inventory_trust PRIVATE ${CMAKE_SOURCE_DIR}/src/licensing)

if(MSVC)
    target_compile_options(test_package_launcher_inventory_trust PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_package_launcher_inventory_trust PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_package_launcher_inventory_trust
    COMMAND $<TARGET_FILE:test_package_launcher_inventory_trust>
)

add_executable(test_windows_launcher_trust_fixture
    test_windows_launcher_trust_fixture.cpp
)
target_link_libraries(test_windows_launcher_trust_fixture PRIVATE
    cf_package_trust
    cf_security
)
if(MSVC)
    target_compile_options(test_windows_launcher_trust_fixture PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_windows_launcher_trust_fixture PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_focused_workflow_path_filters
    COMMAND ${CMAKE_COMMAND}
        -DCOPPERFIN_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/validate_focused_workflow_path_filters.cmake
)

add_executable(test_process_success test_process_exit.cpp)
target_compile_definitions(test_process_success PRIVATE COPPERFIN_TEST_PROCESS_EXIT_CODE=0)

add_executable(test_process_failure test_process_exit.cpp)
target_compile_definitions(test_process_failure PRIVATE COPPERFIN_TEST_PROCESS_EXIT_CODE=1)

copperfin_add_test(test_process_capture cf_test_process_capture_support
    test_process_capture.cpp
)
target_link_libraries(test_process_capture PRIVATE cf_platform_support)

add_executable(test_studio_host_json
    test_studio_host_json.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_diagnostics_appearance_and_drawing.cpp
    test_studio_host_json_diagnostics_target_selectors.cpp
    test_studio_host_json_diagnostics_values.cpp
    test_studio_host_json_diagnostics_general.cpp
    test_studio_host_json_diagnostics_object_metadata.cpp
    test_studio_host_json_parse_diagnostics_toolbox.cpp
    test_studio_host_json_parse_diagnostics_visual.cpp
    test_studio_host_json_parse_diagnostics_core.cpp
    test_studio_host_json_dispatch_toolbox_catalog_and_admission.cpp
    test_studio_host_json_dispatch_toolbox_palette.cpp
    test_studio_host_json_dispatch_toolbox_object_creation_plans.cpp
    test_studio_host_json_dispatch_builder.cpp
    test_studio_host_json_dispatch_designer.cpp
    test_studio_host_json_rushmore_explain.cpp
    test_studio_host_json_dispatch_editor_action.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_behavior_window_chrome_01.cpp
    test_studio_host_json_behavior_window_chrome_02.cpp
    test_studio_host_json_behavior_window_chrome_03.cpp
    test_studio_host_json_behavior_window_chrome_04.cpp
    test_studio_host_json_behavior_window_chrome_05.cpp
    test_studio_host_json_behavior_window_chrome_06.cpp
    test_studio_host_json_behavior_grid_and_selection.cpp
    test_studio_host_json_behavior_data_and_control.cpp
    test_studio_host_json_behavior_general_state.cpp
    test_studio_host_json_appearance_colors.cpp
    test_studio_host_json_appearance_fonts.cpp
    test_studio_host_json_appearance_pictures_and_icons.cpp
    test_studio_host_json_appearance_borders_fill_draw.cpp
    test_studio_host_json_appearance_window_and_display.cpp
    test_studio_host_json_appearance_ole_and_drag.cpp
    test_studio_host_json_setters_data.cpp
    test_studio_host_json_setters_data_selection.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_report_section_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_band_exposure.cpp
    test_studio_host_json_objects_band_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_visual_editor_json_methods.cpp
    test_studio_host_json_visual_editor_json_objects.cpp
    test_studio_host_json_visual_editor_json_properties.cpp
    test_studio_host_json_form_class_startup_selection.cpp
)
target_link_libraries(test_studio_host_json PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_json PRIVATE
    COPPERFIN_TEST_SUCCESS_COMMAND="$<TARGET_FILE:test_process_success>"
    COPPERFIN_TEST_FAILURE_COMMAND="$<TARGET_FILE:test_process_failure>"
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_OBJECT_ACTION_LIFECYCLE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_REMAINING_OBJECT_ACTIONS
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_EXPRESSIONS
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_LIFECYCLE_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_EXPRESSIONS_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_LIFECYCLE_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_GEOMETRY_SKIP_STABLE
    COPPERFIN_REPORT_DIRECT_SETTING_FIELDS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_SETTINGS_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_UNRESOLVED_MEMO_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_SCHEMA_FALLBACK_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_DELETED_STATES_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HELPERS
    COPPERFIN_REPORT_OBJECT_FALLBACKS_SKIP_HELPERS
    COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_SKIP_HOST_SMOKE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
)
add_dependencies(test_studio_host_json test_process_success test_process_failure)

if(MSVC)
    target_compile_options(test_studio_host_json PRIVATE /W4 /permissive-)
    target_compile_options(test_studio_host_json PRIVATE /bigobj)
else()
    target_compile_options(test_studio_host_json PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_json
    COMMAND $<TARGET_FILE:test_studio_host_json> $<TARGET_FILE:copperfin_studio_host>
)
# This aggregate host contract exceeds the ordinary Windows test budget on the
# hosted VFP9 machine; keep the default bounded timeout for all other tests.
set_tests_properties(test_studio_host_json PROPERTIES TIMEOUT 1200)

add_executable(test_studio_host_toolbox_create_lifecycle
    test_studio_host_toolbox_create_lifecycle.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_toolbox_create_lifecycle PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_toolbox_create_lifecycle PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_ONLY_TOOLBOX_CREATE
)

if(MSVC)
    target_compile_options(test_studio_host_toolbox_create_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_toolbox_create_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_toolbox_create_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_toolbox_create_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_stable_selector_mutation
    test_studio_host_stable_selector_mutation.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_stable_selector_mutation PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_stable_selector_mutation PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_ONLY_STABLE_SELECTOR_MUTATION
)

if(MSVC)
    target_compile_options(test_studio_host_stable_selector_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_stable_selector_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_stable_selector_mutation
    COMMAND $<TARGET_FILE:test_studio_host_stable_selector_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_action_lifecycle
    test_studio_host_object_action_lifecycle.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_object_actions.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_object_action_lifecycle PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_object_action_lifecycle PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION
    COPPERFIN_OBJECT_LIFECYCLE_ONLY_OBJECT_ACTION_LIFECYCLE
)

if(MSVC)
    target_compile_options(test_studio_host_object_action_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_action_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_action_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_object_action_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_remaining_object_lifecycle
    test_studio_host_remaining_object_lifecycle.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_remaining_object_lifecycle PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_remaining_object_lifecycle PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_OBJECT_ACTION_LIFECYCLE
)

if(MSVC)
    target_compile_options(test_studio_host_remaining_object_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_remaining_object_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_remaining_object_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_remaining_object_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_expressions
    test_studio_host_detail_header_footer_section_expressions.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_expressions PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_section_expressions PRIVATE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_ONLY_EXPRESSIONS
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_expressions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_expressions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_expressions
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_expressions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_geometry
    test_studio_host_detail_header_footer_section_geometry.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_expressions_and_exposure.cpp
    test_studio_host_json_sections_dhf_heights.cpp
    test_studio_host_json_sections_dhf_tops.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_section_geometry PRIVATE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_EXPRESSIONS
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_geometry
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_lifecycle
    test_studio_host_detail_header_footer_section_lifecycle.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_lifecycle PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_geometry
    test_studio_host_detail_header_footer_object_geometry.cpp
    test_studio_host_json_geometry_dimensions.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_object_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_geometry
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_expressions
    test_studio_host_detail_header_footer_object_expressions.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_expressions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_expressions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_expressions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_expressions
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_expressions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_lifecycle
    test_studio_host_detail_header_footer_object_lifecycle.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_lifecycle PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_layout_actions
    test_studio_host_detail_header_footer_object_layout_actions.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_align.cpp
    test_studio_host_json_objects_dhf_distribution.cpp
    test_studio_host_json_objects_dhf_nudge.cpp
    test_studio_host_json_objects_dhf_snap.cpp
    test_studio_host_json_objects_dhf_resize.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_classifications
    test_studio_host_report_layout_classifications.cpp
    test_studio_host_json_report_layout_classifications.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_classifications PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_layout_classifications PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_classifications PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_classifications PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_classifications
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_classifications> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_direct_setting_fields
    test_studio_host_report_direct_setting_fields.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_column_setup.cpp
    test_studio_host_json_settings_direct_field_validation.cpp
    test_studio_host_json_settings_page_layout_fields.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_direct_setting_fields PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_direct_setting_fields PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_direct_setting_fields PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_direct_setting_fields PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_direct_setting_fields
    COMMAND $<TARGET_FILE:test_studio_host_report_direct_setting_fields> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_column_setup
    test_studio_host_report_column_setup.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_column_setup.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_column_setup PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_column_setup PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_column_setup PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_column_setup PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_column_setup
    COMMAND $<TARGET_FILE:test_studio_host_report_column_setup> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_page_setup_fields
    test_studio_host_report_page_setup_fields.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_page_layout_fields.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_page_setup_fields PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_page_setup_fields PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_page_setup_fields PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_page_setup_fields PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_page_setup_fields
    COMMAND $<TARGET_FILE:test_studio_host_report_page_setup_fields> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_column_width_fields
    test_studio_host_report_column_width_fields.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_geometry_column_width.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_column_width_fields PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_column_width_fields PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_column_width_fields PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_column_width_fields PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_column_width_fields
    COMMAND $<TARGET_FILE:test_studio_host_report_column_width_fields> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_label_layout_actions
    test_studio_host_label_layout_actions.cpp
    test_studio_host_json_objects_layout_and_expressions.cpp
    test_studio_host_json_report_labels.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_label_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_label_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_label_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_label_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_label_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_actions
    test_studio_host_report_layout_actions.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_objects_layout_and_expressions.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_font_metadata
    test_studio_host_report_font_metadata.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_geometry_font_metadata.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_font_metadata PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_font_metadata PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_font_metadata PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_font_metadata
    COMMAND $<TARGET_FILE:test_studio_host_report_font_metadata> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_placement
    test_studio_host_report_layout_placement.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_sections_move_and_placement.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_placement PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_placement PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_placement PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_placement
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_placement> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_settings_memo_parsing
    test_studio_host_report_settings_memo_parsing.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_memo_parsing.cpp
    test_studio_host_json_settings_expr_case.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_settings_memo_parsing PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_settings_memo_parsing PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_settings_memo_parsing PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_settings_memo_parsing PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_settings_memo_parsing
    COMMAND $<TARGET_FILE:test_studio_host_report_settings_memo_parsing> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_settings_diagnostics
    test_studio_host_report_settings_diagnostics.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_settings_diagnostics_and_misc.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_settings_diagnostics PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_settings_diagnostics PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_settings_diagnostics PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_settings_diagnostics PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_settings_diagnostics
    COMMAND $<TARGET_FILE:test_studio_host_report_settings_diagnostics> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_section_selection_diagnostics
    test_studio_host_report_section_selection_diagnostics.cpp
    test_studio_host_json_report_section_selection_diagnostics.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_section_selection_diagnostics PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_section_selection_diagnostics PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_section_selection_diagnostics PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_section_selection_diagnostics
    COMMAND $<TARGET_FILE:test_studio_host_report_section_selection_diagnostics> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_live_section_geometry
    test_studio_host_report_live_section_geometry.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_sections_geometry.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_section_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_live_section_geometry PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_live_section_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_live_section_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_live_section_geometry
    COMMAND $<TARGET_FILE:test_studio_host_report_live_section_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_unresolved_memo_placeholders
    test_studio_host_report_unresolved_memo_placeholders.cpp
    test_studio_host_json_geometry_dimensions.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_object_fallbacks.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_memo_and_placeholders.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_sections_diagnostics_and_misc.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_unresolved_memo_placeholders PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_unresolved_memo_placeholders PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_unresolved_memo_placeholders PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_unresolved_memo_placeholders PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_unresolved_memo_placeholders
    COMMAND $<TARGET_FILE:test_studio_host_report_unresolved_memo_placeholders> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_schema_fallbacks
    test_studio_host_report_schema_fallbacks.cpp
    test_studio_host_json_geometry_dimensions.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_object_fallbacks.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_sections_diagnostics_and_misc.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_schema_fallbacks PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_schema_fallbacks PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_schema_fallbacks PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_schema_fallbacks PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_schema_fallbacks
    COMMAND $<TARGET_FILE:test_studio_host_report_schema_fallbacks> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_round_trip
    test_studio_host_real_sample_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

copperfin_add_test(
    test_studio_host_real_sample_discovery
    cf_platform_support
    test_studio_host_real_sample_discovery.cpp
)

add_executable(test_studio_host_real_sample_sort_settings_round_trip
    test_studio_host_real_sample_sort_settings_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_sort_settings_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_sort_settings_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_sort_settings_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_sort_settings_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_sort_settings_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_settings_delete_restore_round_trip
    test_studio_host_real_sample_settings_delete_restore_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_settings_delete_restore_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_settings_delete_restore_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_settings_delete_restore_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_settings_delete_restore_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_settings_delete_restore_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_column_setup_round_trip
    test_studio_host_real_sample_column_setup_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_column_setup_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_column_setup_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_column_setup_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_column_setup_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_column_setup_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_paper_dimensions_round_trip
    test_studio_host_real_sample_paper_dimensions_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_paper_dimensions_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_paper_dimensions_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_paper_dimensions_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_paper_dimensions_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_paper_dimensions_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_left_margin_round_trip
    test_studio_host_real_sample_left_margin_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_left_margin_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_left_margin_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_left_margin_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_left_margin_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_left_margin_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_right_margin_round_trip
    test_studio_host_real_sample_right_margin_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_right_margin_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_right_margin_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_right_margin_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_right_margin_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_right_margin_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_driver_round_trip
    test_studio_host_real_sample_driver_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_driver_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_driver_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_driver_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_driver_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_driver_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_device_round_trip
    test_studio_host_real_sample_device_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_device_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_device_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_device_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_device_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_device_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_output_round_trip
    test_studio_host_real_sample_output_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_output_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_output_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_output_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_output_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_output_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_defaultsource_round_trip
    test_studio_host_real_sample_defaultsource_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_defaultsource_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_defaultsource_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_defaultsource_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_defaultsource_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_defaultsource_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_printquality_round_trip
    test_studio_host_real_sample_printquality_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_printquality_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_printquality_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_printquality_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_printquality_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_printquality_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_yresolution_round_trip
    test_studio_host_real_sample_yresolution_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_yresolution_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_yresolution_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_yresolution_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_yresolution_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_yresolution_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_ttoption_round_trip
    test_studio_host_real_sample_ttoption_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_ttoption_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_ttoption_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_ttoption_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_ttoption_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_ttoption_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_ascii_round_trip
    test_studio_host_real_sample_ascii_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_ascii_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_ascii_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_ascii_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_ascii_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_ascii_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_collate_round_trip
    test_studio_host_real_sample_collate_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_collate_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_collate_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_collate_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_collate_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_collate_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_copies_round_trip
    test_studio_host_real_sample_copies_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_copies_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_copies_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_copies_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_copies_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_copies_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_page_setup_settings_round_trip
    test_studio_host_real_sample_page_setup_settings_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_page_setup_settings_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_page_setup_settings_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_page_setup_settings_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_page_setup_settings_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_page_setup_settings_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_direct_margins_round_trip
    test_studio_host_real_sample_direct_margins_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_direct_margins_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_direct_margins_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_direct_margins_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_direct_margins_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_direct_margins_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_round_trip
    test_studio_host_real_sample_object_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_preview_bounds_round_trip
    test_studio_host_real_sample_object_preview_bounds_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_preview_bounds_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_preview_bounds_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_preview_bounds_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_preview_bounds_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_preview_bounds_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_nudge_round_trip
    test_studio_host_real_sample_object_nudge_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_nudge_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_nudge_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_nudge_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_nudge_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_nudge_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_align_round_trip
    test_studio_host_real_sample_object_align_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_align_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_align_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_align_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_align_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_align_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_distribute_round_trip
    test_studio_host_real_sample_object_distribute_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_distribute_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_distribute_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_distribute_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_distribute_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_distribute_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_resize_round_trip
    test_studio_host_real_sample_object_resize_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_resize_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_resize_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_resize_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_resize_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_resize_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_snap_round_trip
    test_studio_host_real_sample_object_snap_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_snap_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_snap_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_snap_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_snap_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_snap_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_duplicate_round_trip
    test_studio_host_real_sample_object_duplicate_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_duplicate_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_duplicate_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_duplicate_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_duplicate_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_duplicate_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_rename_round_trip
    test_studio_host_real_sample_object_rename_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_rename_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_rename_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_rename_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_rename_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_rename_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_reorder_round_trip
    test_studio_host_real_sample_object_reorder_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_reorder_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_reorder_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_reorder_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_reorder_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_reorder_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_memo_round_trip
    test_studio_host_real_sample_object_memo_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_memo_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_memo_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_memo_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_memo_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_memo_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_font_round_trip
    test_studio_host_real_sample_object_font_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_font_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_font_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_font_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_font_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_font_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_font_clear_round_trip
    test_studio_host_real_sample_object_font_clear_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_font_clear_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_font_clear_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_font_clear_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_font_clear_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_font_clear_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_delete_restore_round_trip
    test_studio_host_real_sample_object_delete_restore_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_delete_restore_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_delete_restore_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_delete_restore_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_delete_restore_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_delete_restore_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_deleted_preview_round_trip
    test_studio_host_real_sample_object_deleted_preview_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_deleted_preview_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_deleted_preview_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_deleted_preview_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_deleted_preview_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_deleted_preview_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_section_round_trip
    test_studio_host_real_sample_section_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_section_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_section_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_section_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_section_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_section_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_section_deleted_preview_round_trip
    test_studio_host_real_sample_section_deleted_preview_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_section_deleted_preview_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_section_deleted_preview_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_section_deleted_preview_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_section_deleted_preview_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_section_deleted_preview_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_section_delete_restore_round_trip
    test_studio_host_real_sample_section_delete_restore_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_section_delete_restore_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_section_delete_restore_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_section_delete_restore_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_section_delete_restore_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_section_delete_restore_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_grouping_round_trip
    test_studio_host_real_sample_grouping_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_grouping_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_grouping_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_grouping_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_grouping_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_grouping_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_grouping_deleted_preview_round_trip
    test_studio_host_real_sample_grouping_deleted_preview_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_grouping_deleted_preview_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_grouping_deleted_preview_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_grouping_deleted_preview_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_grouping_deleted_preview_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_grouping_deleted_preview_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_section_geometry
    test_studio_host_deleted_section_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_section_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_section_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_section_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_section_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_section_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_section_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_width_left_geometry
    test_studio_host_deleted_object_width_left_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_object_width_left_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_object_width_left_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_width_left_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_width_left_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_width_left_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_width_left_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_height_top_geometry
    test_studio_host_deleted_object_height_top_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_object_height_top_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_object_height_top_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_height_top_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_height_top_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_height_top_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_height_top_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_restore_geometry
    test_studio_host_deleted_object_restore_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_object_restore_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_object_restore_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_restore_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_restore_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_restore_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_restore_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_edited_object_delete_geometry
    test_studio_host_edited_object_delete_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_edited_object_delete_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_edited_object_delete_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_edited_object_delete_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_edited_object_delete_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_edited_object_delete_geometry
    COMMAND $<TARGET_FILE:test_studio_host_edited_object_delete_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_width_left_preview_bounds
    test_studio_host_report_width_left_preview_bounds.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_geometry_preview_object_lefts.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_report_width_left_preview_bounds PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_width_left_preview_bounds PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_width_left_preview_bounds PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_width_left_preview_bounds PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_width_left_preview_bounds
    COMMAND $<TARGET_FILE:test_studio_host_report_width_left_preview_bounds> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_height_top_preview_bounds
    test_studio_host_report_height_top_preview_bounds.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_geometry_preview_object_tops.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_report_height_top_preview_bounds PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_height_top_preview_bounds PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_height_top_preview_bounds PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_height_top_preview_bounds PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_height_top_preview_bounds
    COMMAND $<TARGET_FILE:test_studio_host_report_height_top_preview_bounds> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_preview_bounds
    test_studio_host_detail_header_footer_section_preview_bounds.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_geometry_preview_section_lifecycle.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_preview_bounds
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_preview_bounds> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_rejections
    test_studio_host_deleted_report_visual_property_rejections.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_visual_properties.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_rejections PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_rejections PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_rejections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_rejections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_rejections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_rejections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_clears
    test_studio_host_deleted_report_visual_property_clears.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_visual_properties.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_clears PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_clears PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_CLEARS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_clears PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_clears PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_clears
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_clears> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_copies
    test_studio_host_deleted_report_visual_property_copies.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_deleted_report_visual_property_copies.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_copies PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_copies PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_copies PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_copies PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_copies
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_copies> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_moves
    test_studio_host_deleted_report_visual_property_moves.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_visual_properties.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_moves PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_moves PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_CLEARS_SKIP_HOST_SMOKE
    COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_MOVES_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_moves PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_moves PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_moves
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_moves> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_geometry_defaults
    test_studio_host_report_geometry_defaults.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_report_geometry_defaults PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_geometry_defaults PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_geometry_defaults PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_geometry_defaults PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_geometry_defaults
    COMMAND $<TARGET_FILE:test_studio_host_report_geometry_defaults> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_deleted_states
    test_studio_host_report_deleted_states.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_objects_diagnostics_and_misc.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_deleted_states PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_deleted_states PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HELPERS
    COPPERFIN_REPORT_OBJECT_FALLBACKS_SKIP_HELPERS
    COPPERFIN_REPORT_SETTINGS_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_DELETED_STATES_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_report_deleted_states> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_diagnostics
    test_studio_host_report_layout_diagnostics.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_objects_diagnostics_and_misc.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_diagnostics PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_layout_diagnostics PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_REPORT_DELETED_STATES_SKIP_HELPERS
    COPPERFIN_REPORT_OBJECT_FALLBACKS_SKIP_HELPERS
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_diagnostics PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_diagnostics PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_diagnostics
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_diagnostics> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_none
    test_studio_host_report_selection_none.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_none PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_none PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_none PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_none
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_none> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_padded
    test_studio_host_report_selection_padded.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_padded PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_padded PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_padded PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_padded
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_padded> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_deep
    test_studio_host_report_selection_deep.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_deep PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_deep PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_deep PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_deep
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_deep> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_deep_section_settings
    test_studio_host_report_selection_deep_section_settings.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_deep_section_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_deep_section_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_deep_section_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_deep_section_settings
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_deep_section_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_exposure
    test_studio_host_report_grouping_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_exposure PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_exposure PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_exposure PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_exposure
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_exposure> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_nested_mutation
    test_studio_host_report_grouping_nested_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_nested_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_nested_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_nested_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_nested_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_nested_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_stable_mutation
    test_studio_host_report_grouping_stable_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_stable_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_stable_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_stable_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_stable_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_stable_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_record_mutation
    test_studio_host_report_grouping_record_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_record_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_record_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_record_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_record_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_record_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_deleted_exposure
    test_studio_host_report_grouping_deleted_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_deleted_exposure PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_deleted_exposure PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_deleted_exposure PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_deleted_exposure
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_deleted_exposure> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_deleted_mutation
    test_studio_host_report_grouping_deleted_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_deleted_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_deleted_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_deleted_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_deleted_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_deleted_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_remaining_exposure
    test_studio_host_report_grouping_remaining_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_remaining_exposure PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_remaining_exposure PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_remaining_exposure PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_remaining_exposure
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_remaining_exposure> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_layout_actions
    test_studio_host_layout_actions.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_geometry_live_edit_actions
    test_studio_host_geometry_live_edit_actions.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_geometry_live_edit_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_geometry_live_edit_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_geometry_live_edit_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_geometry_live_edit_actions
    COMMAND $<TARGET_FILE:test_studio_host_geometry_live_edit_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_section_ids
    test_studio_host_report_section_ids.cpp
)
target_link_libraries(test_studio_host_report_section_ids PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_section_ids PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_section_ids PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_section_ids
    COMMAND $<TARGET_FILE:test_studio_host_report_section_ids> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_band_object_sections
    test_studio_host_deleted_band_object_sections.cpp
)
target_link_libraries(test_studio_host_deleted_band_object_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_band_object_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_band_object_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_band_object_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_band_object_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_update_batch_sections
    test_studio_host_deleted_update_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_update_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_update_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_update_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_update_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_update_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_update_batch
    test_studio_host_deleted_update_batch.cpp
)
target_link_libraries(test_studio_host_deleted_update_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_update_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_update_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_update_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_update_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_update_batch_errors
    test_studio_host_deleted_update_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_update_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_update_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_update_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_update_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_update_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_update_batch
    test_studio_host_update_batch.cpp
)
target_link_libraries(test_studio_host_update_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_update_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_update_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_update_batch
    COMMAND $<TARGET_FILE:test_studio_host_update_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_update_batch_errors
    test_studio_host_update_batch_errors.cpp
)
target_link_libraries(test_studio_host_update_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_update_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_update_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_update_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_update_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_clear_sections
    test_studio_host_deleted_property_clear_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_clear_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_clear_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_clear_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_clear_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_clear_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_clear_batch_sections
    test_studio_host_deleted_property_clear_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_clear_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_clear_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_clear_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_clear_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_clear_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_copy_sections
    test_studio_host_deleted_property_copy_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_copy_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_copy_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_copy_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_copy_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_copy_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_copy_batch_sections
    test_studio_host_deleted_property_copy_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_copy_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_copy_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_copy_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_copy_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_copy_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_move_sections
    test_studio_host_deleted_property_move_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_move_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_move_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_move_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_move_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_move_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_move_batch_sections
    test_studio_host_deleted_property_move_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_move_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_move_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_move_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_move_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_move_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_rejection_sections
    test_studio_host_deleted_property_rejection_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_rejection_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_rejection_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_rejection_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_rejection_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_rejection_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_batch_rejection_sections
    test_studio_host_deleted_property_batch_rejection_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_batch_rejection_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_batch_rejection_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_batch_rejection_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_batch_rejection_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_batch_rejection_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_rename_batch_sections
    test_studio_host_deleted_object_rename_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_object_rename_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_rename_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_rename_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_rename_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_rename_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_rename_batch
    test_studio_host_deleted_object_rename_batch.cpp
)
target_link_libraries(test_studio_host_deleted_object_rename_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_rename_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_rename_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_rename_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_rename_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_duplicate_batch_sections
    test_studio_host_deleted_object_duplicate_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_object_duplicate_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_duplicate_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_duplicate_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_duplicate_batch
    test_studio_host_deleted_object_duplicate_batch.cpp
)
target_link_libraries(test_studio_host_deleted_object_duplicate_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_duplicate_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_duplicate_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_duplicate_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_duplicate_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_reorder_batch_sections
    test_studio_host_deleted_object_reorder_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_object_reorder_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_reorder_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_reorder_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_reorder_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_reorder_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_reorder_batch
    test_studio_host_deleted_object_reorder_batch.cpp
)
target_link_libraries(test_studio_host_deleted_object_reorder_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_reorder_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_reorder_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_reorder_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_reorder_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_reorder_batch_errors
    test_studio_host_deleted_object_reorder_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_object_reorder_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_reorder_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_reorder_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_reorder_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_reorder_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_duplicate_batch_errors
    test_studio_host_deleted_object_duplicate_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_object_duplicate_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_duplicate_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_duplicate_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_rename_batch_errors
    test_studio_host_deleted_object_rename_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_object_rename_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_rename_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_rename_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_rename_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_rename_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_reorder_batch_errors
    test_studio_host_object_reorder_batch_errors.cpp
)
target_link_libraries(test_studio_host_object_reorder_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_reorder_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_reorder_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_reorder_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_reorder_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_reorder_batch
    test_studio_host_object_reorder_batch.cpp
)
target_link_libraries(test_studio_host_object_reorder_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_reorder_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_reorder_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_reorder_batch
    COMMAND $<TARGET_FILE:test_studio_host_object_reorder_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_duplicate_batch_errors
    test_studio_host_object_duplicate_batch_errors.cpp
)
target_link_libraries(test_studio_host_object_duplicate_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_duplicate_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_duplicate_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_duplicate_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_duplicate_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_duplicate_batch
    test_studio_host_object_duplicate_batch.cpp
)
target_link_libraries(test_studio_host_object_duplicate_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_duplicate_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_duplicate_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_duplicate_batch
    COMMAND $<TARGET_FILE:test_studio_host_object_duplicate_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_rename_batch_errors
    test_studio_host_object_rename_batch_errors.cpp
)
target_link_libraries(test_studio_host_object_rename_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_rename_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_rename_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_rename_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_rename_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_rename_batch
    test_studio_host_object_rename_batch.cpp
)
target_link_libraries(test_studio_host_object_rename_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_rename_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_rename_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_rename_batch
    COMMAND $<TARGET_FILE:test_studio_host_object_rename_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_deleted_states_errors
    test_studio_host_object_deleted_states_errors.cpp
)
target_link_libraries(test_studio_host_object_deleted_states_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_deleted_states_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_deleted_states_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_deleted_states_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_deleted_states_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_deleted_states
    test_studio_host_object_deleted_states.cpp
)
target_link_libraries(test_studio_host_object_deleted_states PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_object_deleted_states> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_mixed_deleted_states_errors
    test_studio_host_mixed_deleted_states_errors.cpp
)
target_link_libraries(test_studio_host_mixed_deleted_states_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_mixed_deleted_states_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_mixed_deleted_states_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_mixed_deleted_states_errors
    COMMAND $<TARGET_FILE:test_studio_host_mixed_deleted_states_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_mixed_deleted_states
    test_studio_host_mixed_deleted_states.cpp
)
target_link_libraries(test_studio_host_mixed_deleted_states PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_mixed_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_mixed_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_mixed_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_mixed_deleted_states> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_section_deleted_states_errors
    test_studio_host_settings_section_deleted_states_errors.cpp
)
target_link_libraries(test_studio_host_settings_section_deleted_states_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_section_deleted_states_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_section_deleted_states_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_section_deleted_states_errors
    COMMAND $<TARGET_FILE:test_studio_host_settings_section_deleted_states_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_section_deleted_states
    test_studio_host_settings_section_deleted_states.cpp
)
target_link_libraries(test_studio_host_settings_section_deleted_states PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_section_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_section_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_section_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_settings_section_deleted_states> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_page_margin_stable_settings
    test_studio_host_page_margin_stable_settings.cpp
)
target_link_libraries(test_studio_host_page_margin_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_page_margin_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_page_margin_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_page_margin_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_page_margin_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_page_margin_record_settings
    test_studio_host_page_margin_record_settings.cpp
)
target_link_libraries(test_studio_host_page_margin_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_page_margin_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_page_margin_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_page_margin_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_page_margin_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_side_margin_record_settings
    test_studio_host_side_margin_record_settings.cpp
)
target_link_libraries(test_studio_host_side_margin_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_side_margin_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_side_margin_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_side_margin_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_side_margin_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_bottom_margin_stable_settings
    test_studio_host_bottom_margin_stable_settings.cpp
)
target_link_libraries(test_studio_host_bottom_margin_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_bottom_margin_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_bottom_margin_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_bottom_margin_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_bottom_margin_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_bottom_margin_record_settings
    test_studio_host_bottom_margin_record_settings.cpp
)
target_link_libraries(test_studio_host_bottom_margin_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_bottom_margin_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_bottom_margin_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_bottom_margin_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_bottom_margin_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_vertical_grid_stable_settings
    test_studio_host_vertical_grid_stable_settings.cpp
)
target_link_libraries(test_studio_host_vertical_grid_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_vertical_grid_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_vertical_grid_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_vertical_grid_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_vertical_grid_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_vertical_grid_record_settings
    test_studio_host_vertical_grid_record_settings.cpp
)
target_link_libraries(test_studio_host_vertical_grid_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_vertical_grid_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_vertical_grid_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_vertical_grid_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_vertical_grid_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_horizontal_grid_stable_settings
    test_studio_host_horizontal_grid_stable_settings.cpp
)
target_link_libraries(test_studio_host_horizontal_grid_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_horizontal_grid_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_horizontal_grid_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_horizontal_grid_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_horizontal_grid_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_horizontal_grid_record_settings
    test_studio_host_horizontal_grid_record_settings.cpp
)
target_link_libraries(test_studio_host_horizontal_grid_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_horizontal_grid_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_horizontal_grid_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_horizontal_grid_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_horizontal_grid_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_orientation_stable_settings
    test_studio_host_orientation_stable_settings.cpp
)
target_link_libraries(test_studio_host_orientation_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_orientation_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_orientation_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_orientation_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_orientation_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_orientation_record_settings
    test_studio_host_orientation_record_settings.cpp
)
target_link_libraries(test_studio_host_orientation_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_orientation_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_orientation_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_orientation_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_orientation_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_paper_size_stable_settings
    test_studio_host_paper_size_stable_settings.cpp
)
target_link_libraries(test_studio_host_paper_size_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_paper_size_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_paper_size_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_paper_size_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_paper_size_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_paper_size_record_settings
    test_studio_host_paper_size_record_settings.cpp
)
target_link_libraries(test_studio_host_paper_size_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_paper_size_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_paper_size_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_paper_size_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_paper_size_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_memo_stable
    test_studio_host_settings_memo_stable.cpp
)
target_link_libraries(test_studio_host_settings_memo_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_memo_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_memo_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_memo_stable
    COMMAND $<TARGET_FILE:test_studio_host_settings_memo_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_sort_settings_stable
    test_studio_host_report_sort_settings_stable.cpp
)
target_link_libraries(test_studio_host_report_sort_settings_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_sort_settings_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_sort_settings_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_sort_settings_stable
    COMMAND $<TARGET_FILE:test_studio_host_report_sort_settings_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_sort_settings_record
    test_studio_host_report_sort_settings_record.cpp
)
target_link_libraries(test_studio_host_report_sort_settings_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_sort_settings_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_sort_settings_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_sort_settings_record
    COMMAND $<TARGET_FILE:test_studio_host_report_sort_settings_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_delete_stable
    test_studio_host_settings_delete_stable.cpp
)
target_link_libraries(test_studio_host_settings_delete_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_delete_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_delete_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_delete_stable
    COMMAND $<TARGET_FILE:test_studio_host_settings_delete_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_delete_record
    test_studio_host_settings_delete_record.cpp
)
target_link_libraries(test_studio_host_settings_delete_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_delete_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_delete_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_delete_record
    COMMAND $<TARGET_FILE:test_studio_host_settings_delete_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_restore_record
    test_studio_host_settings_restore_record.cpp
)
target_link_libraries(test_studio_host_settings_restore_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_restore_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_restore_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_restore_record
    COMMAND $<TARGET_FILE:test_studio_host_settings_restore_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_settings_stable
    test_studio_host_selected_settings_stable.cpp
)
target_link_libraries(test_studio_host_selected_settings_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_settings_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_settings_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_settings_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_settings_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_sections_stable
    test_studio_host_selected_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_sections_stable
    test_studio_host_selected_page_header_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_header_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_objects_stable
    test_studio_host_selected_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_objects_stable
    test_studio_host_selected_summary_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_summary_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_header_objects_stable
    test_studio_host_selected_group_header_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_header_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_header_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_header_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_header_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_header_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_footer_objects_stable
    test_studio_host_selected_group_footer_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_footer_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_footer_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_footer_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_footer_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_footer_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_title_objects_stable
    test_studio_host_selected_title_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_title_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_title_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_title_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_title_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_title_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_footer_objects_stable
    test_studio_host_selected_page_footer_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_footer_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_footer_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_footer_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_footer_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_footer_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_header_objects_stable
    test_studio_host_selected_column_header_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_header_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_header_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_header_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_header_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_header_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_footer_objects_stable
    test_studio_host_selected_column_footer_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_footer_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_footer_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_footer_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_footer_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_footer_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_unplaced_objects_stable
    test_studio_host_selected_unplaced_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_unplaced_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_unplaced_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_unplaced_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_unplaced_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_unplaced_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_objects_stable
    test_studio_host_selected_page_header_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_header_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_objects_orphaned
    test_studio_host_selected_page_header_objects_orphaned.cpp
)
target_link_libraries(test_studio_host_selected_page_header_objects_orphaned PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_objects_orphaned PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_objects_orphaned PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_objects_orphaned
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_objects_orphaned> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_detail_objects_orphaned
    test_studio_host_selected_detail_objects_orphaned.cpp
)
target_link_libraries(test_studio_host_selected_detail_objects_orphaned PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_detail_objects_orphaned PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_detail_objects_orphaned PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_detail_objects_orphaned
    COMMAND $<TARGET_FILE:test_studio_host_selected_detail_objects_orphaned> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_objects_record
    test_studio_host_selected_summary_objects_record.cpp
)
target_link_libraries(test_studio_host_selected_summary_objects_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_objects_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_objects_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_objects_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_objects_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_sections_record
    test_studio_host_selected_summary_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_summary_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_title_sections_record
    test_studio_host_selected_title_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_title_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_title_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_title_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_title_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_title_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_header_sections_record
    test_studio_host_selected_column_header_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_column_header_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_header_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_header_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_header_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_header_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_footer_sections_record
    test_studio_host_selected_column_footer_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_column_footer_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_footer_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_footer_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_footer_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_footer_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_footer_sections_record
    test_studio_host_selected_page_footer_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_page_footer_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_footer_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_footer_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_footer_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_footer_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_sections_record
    test_studio_host_selected_page_header_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_page_header_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_header_sections_record
    test_studio_host_selected_group_header_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_group_header_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_header_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_header_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_header_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_header_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_footer_sections_record
    test_studio_host_selected_group_footer_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_group_footer_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_footer_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_footer_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_footer_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_footer_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_header_sections_stable
    test_studio_host_selected_group_header_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_header_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_header_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_header_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_header_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_header_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_footer_sections_stable
    test_studio_host_selected_group_footer_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_footer_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_footer_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_footer_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_footer_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_footer_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_nested_group_sections_record
    test_studio_host_selected_nested_group_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_nested_group_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_nested_group_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_nested_group_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_nested_group_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_nested_group_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_nested_group_sections_stable
    test_studio_host_selected_nested_group_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_nested_group_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_nested_group_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_nested_group_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_nested_group_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_nested_group_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_deleted_object_counts
    test_studio_host_section_deleted_object_counts.cpp
)
target_link_libraries(test_studio_host_section_deleted_object_counts PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_deleted_object_counts PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_deleted_object_counts PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_deleted_object_counts
    COMMAND $<TARGET_FILE:test_studio_host_section_deleted_object_counts> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_report_sections
    test_studio_host_selected_report_sections.cpp
)
target_link_libraries(test_studio_host_selected_report_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_report_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_report_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_report_sections
    COMMAND $<TARGET_FILE:test_studio_host_selected_report_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_report_objects
    test_studio_host_selected_report_objects.cpp
)
target_link_libraries(test_studio_host_selected_report_objects PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_report_objects PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_report_objects PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_report_objects
    COMMAND $<TARGET_FILE:test_studio_host_selected_report_objects> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_report_settings
    test_studio_host_selected_report_settings.cpp
)
target_link_libraries(test_studio_host_selected_report_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_report_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_report_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_report_settings
    COMMAND $<TARGET_FILE:test_studio_host_selected_report_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_label_objects
    test_studio_host_selected_label_objects.cpp
)
target_link_libraries(test_studio_host_selected_label_objects PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_label_objects PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_label_objects PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_label_objects
    COMMAND $<TARGET_FILE:test_studio_host_selected_label_objects> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_label_sections
    test_studio_host_selected_label_sections.cpp
)
target_link_libraries(test_studio_host_selected_label_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_label_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_label_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_label_sections
    COMMAND $<TARGET_FILE:test_studio_host_selected_label_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_label_settings
    test_studio_host_selected_label_settings.cpp
)
target_link_libraries(test_studio_host_selected_label_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_label_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_label_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_label_settings
    COMMAND $<TARGET_FILE:test_studio_host_selected_label_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_sections_stable
    test_studio_host_selected_summary_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_summary_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_title_sections_stable
    test_studio_host_selected_title_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_title_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_title_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_title_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_title_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_title_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_header_sections_stable
    test_studio_host_selected_column_header_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_header_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_header_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_header_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_header_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_header_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_footer_sections_stable
    test_studio_host_selected_column_footer_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_footer_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_footer_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_footer_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_footer_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_footer_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_footer_sections_stable
    test_studio_host_selected_page_footer_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_footer_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_footer_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_footer_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_footer_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_footer_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_restore_stable
    test_studio_host_settings_restore_stable.cpp
)
target_link_libraries(test_studio_host_settings_restore_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_restore_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_restore_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_restore_stable
    COMMAND $<TARGET_FILE:test_studio_host_settings_restore_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_memo_record
    test_studio_host_settings_memo_record.cpp
)
target_link_libraries(test_studio_host_settings_memo_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_memo_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_memo_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_memo_record
    COMMAND $<TARGET_FILE:test_studio_host_settings_memo_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_count_record_settings
    test_studio_host_column_count_record_settings.cpp
)
target_link_libraries(test_studio_host_column_count_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_count_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_count_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_count_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_count_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_width_record_settings
    test_studio_host_column_width_record_settings.cpp
)
target_link_libraries(test_studio_host_column_width_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_width_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_width_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_width_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_width_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_spacing_record_settings
    test_studio_host_column_spacing_record_settings.cpp
)
target_link_libraries(test_studio_host_column_spacing_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_spacing_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_spacing_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_spacing_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_spacing_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_spacing_stable_settings
    test_studio_host_column_spacing_stable_settings.cpp
)
target_link_libraries(test_studio_host_column_spacing_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_spacing_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_spacing_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_spacing_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_spacing_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_setup_record_settings
    test_studio_host_column_setup_record_settings.cpp
)
target_link_libraries(test_studio_host_column_setup_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_setup_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_setup_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_setup_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_setup_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_setup_stable_settings
    test_studio_host_column_setup_stable_settings.cpp
)
target_link_libraries(test_studio_host_column_setup_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_setup_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_setup_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_setup_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_setup_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_delete_record_selection
    test_studio_host_section_delete_record_selection.cpp
)
target_link_libraries(test_studio_host_section_delete_record_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_delete_record_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_delete_record_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_delete_record_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_delete_record_selection> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_delete_stable_selection
    test_studio_host_section_delete_stable_selection.cpp
)
target_link_libraries(test_studio_host_section_delete_stable_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_delete_stable_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_delete_stable_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_delete_stable_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_delete_stable_selection> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_restore_record_selection
    test_studio_host_section_restore_record_selection.cpp
)
target_link_libraries(test_studio_host_section_restore_record_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_restore_record_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_restore_record_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_restore_record_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_restore_record_selection> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_restore_stable_selection
    test_studio_host_section_restore_stable_selection.cpp
)
target_link_libraries(test_studio_host_section_restore_stable_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_restore_stable_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_restore_stable_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_restore_stable_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_restore_stable_selection> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_deleted_state_errors
    test_studio_host_subtree_deleted_state_errors.cpp
)
target_link_libraries(test_studio_host_subtree_deleted_state_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_deleted_state_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_deleted_state_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_deleted_state_errors
    COMMAND $<TARGET_FILE:test_studio_host_subtree_deleted_state_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_deleted_state
    test_studio_host_subtree_deleted_state.cpp
)
target_link_libraries(test_studio_host_subtree_deleted_state PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_deleted_state PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_deleted_state PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_deleted_state
    COMMAND $<TARGET_FILE:test_studio_host_subtree_deleted_state> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_duplicate_errors
    test_studio_host_subtree_duplicate_errors.cpp
)
target_link_libraries(test_studio_host_subtree_duplicate_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_duplicate_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_duplicate_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_duplicate_errors
    COMMAND $<TARGET_FILE:test_studio_host_subtree_duplicate_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_duplicate_collision_errors
    test_studio_host_subtree_duplicate_collision_errors.cpp
)
target_link_libraries(test_studio_host_subtree_duplicate_collision_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_duplicate_collision_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_duplicate_collision_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_duplicate_collision_errors
    COMMAND $<TARGET_FILE:test_studio_host_subtree_duplicate_collision_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_duplicate
    test_studio_host_subtree_duplicate.cpp
)
target_link_libraries(test_studio_host_subtree_duplicate PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_duplicate PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_duplicate PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_duplicate
    COMMAND $<TARGET_FILE:test_studio_host_subtree_duplicate> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_subtree_duplicate_errors
    test_studio_host_deleted_subtree_duplicate_errors.cpp
)
target_link_libraries(test_studio_host_deleted_subtree_duplicate_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_subtree_duplicate_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_subtree_duplicate_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_subtree_duplicate_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_subtree_duplicate_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_subtree_duplicate
    test_studio_host_deleted_subtree_duplicate.cpp
)
target_link_libraries(test_studio_host_deleted_subtree_duplicate PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_subtree_duplicate PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_subtree_duplicate PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_subtree_duplicate
    COMMAND $<TARGET_FILE:test_studio_host_deleted_subtree_duplicate> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_subtree_duplicate_collision_errors
    test_studio_host_deleted_subtree_duplicate_collision_errors.cpp
)
target_link_libraries(test_studio_host_deleted_subtree_duplicate_collision_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_subtree_duplicate_collision_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_subtree_duplicate_collision_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_subtree_duplicate_collision_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_subtree_duplicate_collision_errors> $<TARGET_FILE:copperfin_studio_host>
)

copperfin_add_test(test_vfp_assets cf_vfp_assets
    test_vfp_assets.cpp
)

copperfin_add_test(test_vfp_sidecar_path cf_design_model
    test_vfp_sidecar_path.cpp
)

copperfin_add_test(test_studio_host cf_design_model
    test_studio_host.cpp
    test_studio_host_shared_test_helpers.cpp
    test_studio_host_document_and_snapshot.cpp
    test_studio_host_diagnostics_and_session.cpp
    test_studio_host_selection_context.cpp
    test_studio_host_property_commands.cpp
    test_studio_host_object_lifecycle.cpp
    test_studio_host_layout_action_parse.cpp
    test_studio_host_behavior_window_chrome.cpp
    test_studio_host_behavior_grid_and_selection.cpp
    test_studio_host_behavior_data_and_control.cpp
    test_studio_host_behavior_general_state.cpp
    test_studio_host_appearance_colors.cpp
    test_studio_host_appearance_fonts.cpp
    test_studio_host_appearance_pictures_and_icons.cpp
    test_studio_host_appearance_borders_fill_draw.cpp
    test_studio_host_appearance_window_and_display.cpp
    test_studio_host_appearance_ole_and_drag.cpp
    test_studio_host_setters_data.cpp
)
copperfin_apply_windows_stack_reserve(test_studio_host)

copperfin_add_test(test_dbf_table cf_vfp_assets
    test_dbf_table.cpp
    test_dbf_table_support.cpp
    test_dbf_table_memo_lifecycle.cpp
    test_dbf_table_raw_memo_payloads.cpp
    test_dbf_table_memo_integrity.cpp
    test_dbf_table_schema_rewrite_memo.cpp
)

copperfin_add_test(test_dbf_text_encoding cf_vfp_assets
    test_dbf_text_encoding.cpp
)

# Bounded, deterministic defense-in-depth robustness coverage for the DBF
# header parser (docs/36-dbf-header-parser-robustness-testing.md). These
# targets exist only when explicitly requested. When the local compiler
# supports it, an additional sanitized
# variant compiles the parser itself with AddressSanitizer/
# UndefinedBehaviorSanitizer instrumentation; this is purely additive and
# never changes the normal or release build.
if(COPPERFIN_BUILD_ROBUSTNESS_TESTS)
    set(copperfin_dbf_header_robustness_work_dir
        "${CMAKE_CURRENT_BINARY_DIR}/dbf-header-robustness-normal")
    file(MAKE_DIRECTORY "${copperfin_dbf_header_robustness_work_dir}")
    copperfin_add_test(test_dbf_header_robustness cf_vfp_assets
        test_dbf_header_robustness.cpp
    )
    set_tests_properties(test_dbf_header_robustness PROPERTIES
        WORKING_DIRECTORY "${copperfin_dbf_header_robustness_work_dir}"
        TIMEOUT 60
    )

    if(NOT MSVC)
        include(CheckCXXSourceCompiles)
        include(CMakePushCheckState)
        # Sanitizer support requires a compile-and-link probe. Preserve any
        # required-check state established by the parent toolchain.
        cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_FLAGS
            "-fsanitize=address,undefined -fno-sanitize-recover=all")
        set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address,undefined")
        check_cxx_source_compiles("int main() { return 0; }" COPPERFIN_HAS_ASAN_UBSAN)
        cmake_pop_check_state()
        if(COPPERFIN_HAS_ASAN_UBSAN)
            set(copperfin_dbf_header_robustness_sanitized_work_dir
                "${CMAKE_CURRENT_BINARY_DIR}/dbf-header-robustness-sanitized")
            file(MAKE_DIRECTORY "${copperfin_dbf_header_robustness_sanitized_work_dir}")
            add_executable(test_dbf_header_robustness_sanitized
                test_dbf_header_robustness.cpp
                ${CMAKE_SOURCE_DIR}/src/vfp/dbf_header.cpp
            )
            target_include_directories(test_dbf_header_robustness_sanitized PRIVATE
                ${CMAKE_SOURCE_DIR}/include
            )
            target_link_libraries(test_dbf_header_robustness_sanitized PRIVATE cf_localization)
            target_compile_options(test_dbf_header_robustness_sanitized PRIVATE
                -Wall -Wextra -Wpedantic
                -fsanitize=address,undefined -fno-sanitize-recover=all -g
            )
            target_link_options(test_dbf_header_robustness_sanitized PRIVATE
                -fsanitize=address,undefined
            )
            add_test(
                NAME test_dbf_header_robustness_sanitized
                COMMAND $<TARGET_FILE:test_dbf_header_robustness_sanitized>
            )
            set_tests_properties(test_dbf_header_robustness_sanitized PROPERTIES
                WORKING_DIRECTORY "${copperfin_dbf_header_robustness_sanitized_work_dir}"
                TIMEOUT 60
            )
        endif()
    endif()
endif()

# Optional, open-ended libFuzzer coverage for the same DBF header parser
# (docs/65-dbf-header-parser-fuzzing.md), complementing the bounded,
# deterministic sweeps above. Absent from default and release builds; only
# built when the local compiler supports -fsanitize=fuzzer,address,undefined
# (libFuzzer is not available under MSVC).
if(COPPERFIN_BUILD_FUZZ_TESTS AND NOT MSVC)
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_FLAGS
        "-fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=fuzzer,address,undefined")
    check_cxx_source_compiles(
        "extern \"C\" int LLVMFuzzerTestOneInput(const unsigned char*, unsigned long) { return 0; }"
        COPPERFIN_HAS_LIBFUZZER)
    cmake_pop_check_state()
    if(COPPERFIN_HAS_LIBFUZZER)
        set(copperfin_dbf_header_fuzz_work_dir
            "${CMAKE_CURRENT_BINARY_DIR}/dbf-header-fuzz")
        set(copperfin_dbf_header_fuzz_corpus_dir
            "${copperfin_dbf_header_fuzz_work_dir}/corpus")
        set(copperfin_dbf_header_fuzz_seed_corpus_dir
            "${CMAKE_CURRENT_SOURCE_DIR}/fuzz/corpus/dbf_header")
        file(MAKE_DIRECTORY "${copperfin_dbf_header_fuzz_work_dir}")
        add_executable(fuzz_dbf_header
            fuzz_dbf_header.cpp
            ${CMAKE_SOURCE_DIR}/src/vfp/dbf_header.cpp
        )
        target_include_directories(fuzz_dbf_header PRIVATE
            ${CMAKE_SOURCE_DIR}/include
        )
        target_link_libraries(fuzz_dbf_header PRIVATE cf_localization)
        target_compile_options(fuzz_dbf_header PRIVATE
            -Wall -Wextra -Wpedantic
            -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all -g
        )
        target_link_options(fuzz_dbf_header PRIVATE
            -fsanitize=fuzzer,address,undefined
        )
        # Bounded, fixed-seed smoke run against only the checked-in synthetic
        # corpus -- reproducible for a given libFuzzer version/corpus, not an
        # open-ended search. Longer exploratory runs are a manual/local
        # activity; see docs/65-dbf-header-parser-fuzzing.md. libFuzzer treats
        # its corpus argument as read-write, adding every newly-covering input
        # it discovers, so the wrapper script resets the build-tree corpus
        # from the checked-in seeds immediately before every run -- a repeat
        # ctest invocation in the same build tree must not run against a
        # corpus already grown by a prior run.
        add_test(
            NAME test_dbf_header_fuzz_smoke
            COMMAND "${CMAKE_COMMAND}"
                "-DSEED_CORPUS_DIR=${copperfin_dbf_header_fuzz_seed_corpus_dir}"
                "-DCORPUS_DIR=${copperfin_dbf_header_fuzz_corpus_dir}"
                "-DFUZZ_EXECUTABLE=$<TARGET_FILE:fuzz_dbf_header>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/run_dbf_header_fuzz_smoke_check.cmake"
        )
        set_tests_properties(test_dbf_header_fuzz_smoke PROPERTIES
            WORKING_DIRECTORY "${copperfin_dbf_header_fuzz_work_dir}"
            TIMEOUT 60
        )
    endif()
endif()

copperfin_add_test(test_visual_asset_editor_code_page cf_vfp_assets
    test_visual_asset_editor_code_page.cpp
)
add_test(
    NAME test_studio_host_code_page
    COMMAND $<TARGET_FILE:test_visual_asset_editor_code_page> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_record_localization
    test_studio_host_deleted_record_localization.cpp
)
target_link_libraries(test_studio_host_deleted_record_localization PRIVATE
    cf_vfp_assets
    cf_test_process_capture_support
)
target_compile_definitions(test_studio_host_deleted_record_localization PRIVATE
    COPPERFIN_TEST_LOCALE_DIR="${PROJECT_SOURCE_DIR}/resources/locales"
)
add_test(
    NAME test_studio_host_deleted_record_localization
    COMMAND $<TARGET_FILE:test_studio_host_deleted_record_localization>
        $<TARGET_FILE:copperfin_studio_host>
)

add_test(
    NAME test_dbf_table_active_locale
    COMMAND $<TARGET_FILE:test_dbf_table> --active-locale-missing-field
)
set_tests_properties(
    test_dbf_table_active_locale
    PROPERTIES ENVIRONMENT
        "COPPERFIN_LOCALE=qps-ploc;COPPERFIN_LOCALE_DIR=${PROJECT_SOURCE_DIR}/resources/locales"
)

copperfin_add_test(test_visual_asset_editor cf_vfp_assets
    test_visual_asset_editor.cpp
    test_visual_asset_editor_shared_test_helpers.cpp
    test_visual_asset_editor_raw_preservation.cpp
    test_visual_asset_editor_object_lifecycle_properties_01.cpp
    test_visual_asset_editor_object_lifecycle_properties_02.cpp
    test_visual_asset_editor_object_lifecycle_create_duplicate.cpp
    test_visual_asset_editor_object_lifecycle_hierarchy.cpp
    test_visual_asset_editor_object_lifecycle_geometry_01.cpp
    test_visual_asset_editor_object_lifecycle_geometry_02.cpp
    test_visual_asset_editor_object_lifecycle_identity_01.cpp
    test_visual_asset_editor_object_lifecycle_identity_02.cpp
    test_visual_asset_editor_method_listing.cpp
    test_visual_asset_editor_property_blank_value.cpp
    test_visual_asset_editor_property_comment_lines.cpp
    test_visual_asset_editor_report_settings_bottom_margin.cpp
    test_visual_asset_editor_methods.cpp
    test_visual_asset_editor_properties.cpp
    test_visual_asset_editor_report_printer_settings.cpp
    test_visual_asset_editor_report_settings_case.cpp
    test_visual_asset_editor_hierarchy_and_listing.cpp
    test_visual_asset_editor_update_batch.cpp
    test_visual_asset_editor_setters_behavior.cpp
    test_visual_asset_editor_appearance_colors.cpp
    test_visual_asset_editor_appearance_fonts.cpp
    test_visual_asset_editor_appearance_pictures_and_icons.cpp
    test_visual_asset_editor_appearance_borders_fill_draw.cpp
    test_visual_asset_editor_appearance_window_and_display.cpp
    test_visual_asset_editor_appearance_ole_and_drag.cpp
    test_visual_asset_editor_setters_data.cpp
)
if(MSVC)
    target_compile_options(test_visual_asset_editor PRIVATE /bigobj)
endif()

copperfin_add_test(test_product_subsystems cf_design_model
    test_product_subsystems.cpp
)

copperfin_add_test(test_builder_registry cf_design_model
    test_builder_registry.cpp
)

copperfin_add_test(test_toolbox_palette cf_design_model
    test_toolbox_palette.cpp
)

copperfin_add_test(test_toolbox_creation cf_design_model
    test_toolbox_creation_baseline_01.cpp
    test_toolbox_creation_batch_01.cpp
    test_toolbox_creation_catalog_context_01.cpp
    test_toolbox_creation_catalog_context_02.cpp
    test_toolbox_creation_dispatch_01.cpp
    test_toolbox_creation_dispatch_02.cpp
    test_toolbox_creation_main.cpp
    test_toolbox_creation_planner_01.cpp
    test_toolbox_creation_planner_02.cpp
)

copperfin_add_test(test_context_editor_actions cf_design_model
    test_context_editor_actions.cpp
)

copperfin_add_test(test_designer_context cf_design_model
    test_designer_context.cpp
)

copperfin_add_test(test_report_layout cf_design_model
    test_report_layout.cpp
    test_report_layout_support.cpp
    test_report_layout_sections.cpp
    test_report_layout_image_objects.cpp
    test_report_layout_ruler_lines.cpp
    test_report_layout_offset.cpp
    test_report_layout_fillchar.cpp
    test_report_layout_total_type.cpp
    test_report_layout_reset_total.cpp
    test_report_layout_spacing.cpp
    test_report_layout_general.cpp
    test_report_layout_image_offset.cpp
    test_report_layout_tooltip.cpp
    test_report_layout_line_shape_style.cpp
    test_report_layout_font_charset.cpp
    test_report_layout_header_view_settings.cpp
    test_report_layout_header_ruler_lines.cpp
    test_report_layout_header_picture_overrides.cpp
    test_report_layout_header_add_alias.cpp
    test_report_layout_header_curpos.cpp
    test_report_layout_header_unique.cpp
    test_report_layout_grouping.cpp
    test_report_layout_summary.cpp
)

copperfin_add_test(test_report_layout_sort_settings cf_design_model
    test_report_layout_sort_settings.cpp
)

copperfin_add_test(test_report_layout_side_margin_settings cf_design_model
    test_report_layout_side_margin_settings.cpp
)

copperfin_add_test(test_project_workspace cf_design_model
    test_project_workspace.cpp
)

copperfin_add_test(test_platform_models "cf_security;cf_platform_profile"
    test_platform_models.cpp
)

copperfin_add_test(test_dotnet_interop_audit "cf_security;cf_platform_profile"
    test_dotnet_interop_audit.cpp
)

copperfin_add_test(test_polyglot_route_registry cf_platform_profile
    test_polyglot_route_registry.cpp
)
target_compile_definitions(
    test_polyglot_route_registry
    PRIVATE
        COPPERFIN_ROUTE_REGISTRY_EXAMPLE_PATH="${CMAKE_SOURCE_DIR}/docs/contracts/polyglot-route-registry-v1.json"
)

copperfin_add_test(test_polyglot_route_impact cf_platform_profile
    test_polyglot_route_impact.cpp
)

copperfin_add_test(test_polyglot_benchmark cf_platform_profile
    test_polyglot_benchmark.cpp
)

copperfin_add_test(test_polyglot_bridge_invocation cf_platform_profile
    test_polyglot_bridge_invocation.cpp
)

copperfin_add_test(test_polyglot_artifact_admission
    "cf_security;cf_platform_profile;cf_platform_support"
    test_polyglot_artifact_admission.cpp
)

copperfin_add_test(test_polyglot_artifact_adapter
    "cf_security;cf_platform_profile;cf_platform_support"
    test_polyglot_artifact_adapter.cpp
)

if(COPPERFIN_PYTHON3_EXECUTABLE)
    copperfin_add_test(test_polyglot_python_sidecar
        "cf_xbase_runtime;cf_security;cf_platform_profile;cf_platform_support"
        test_polyglot_python_sidecar.cpp
        prg_engine_test_support.cpp
    )
    target_compile_definitions(test_polyglot_python_sidecar PRIVATE
        COPPERFIN_POLYGLOT_PYTHON_EXECUTABLE="${COPPERFIN_PYTHON3_EXECUTABLE}"
        COPPERFIN_POLYGLOT_PYTHON_SIDECAR_PATH="${CMAKE_SOURCE_DIR}/samples/polyglot-python-sidecar/candidate.py"
        COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS=1
    )
    set_tests_properties(test_polyglot_python_sidecar PROPERTIES TIMEOUT 30)
endif()

if(COPPERFIN_RSCRIPT_EXECUTABLE)
    copperfin_add_test(test_polyglot_r_sidecar
        "cf_xbase_runtime;cf_security;cf_platform_profile;cf_platform_support"
        test_polyglot_r_sidecar.cpp
        prg_engine_test_support.cpp
    )
    target_compile_definitions(test_polyglot_r_sidecar PRIVATE
        COPPERFIN_POLYGLOT_RSCRIPT_EXECUTABLE="${COPPERFIN_RSCRIPT_EXECUTABLE}"
        COPPERFIN_POLYGLOT_R_SIDECAR_PATH="${CMAKE_SOURCE_DIR}/samples/polyglot-r-sidecar/candidate.R"
    )
    set_tests_properties(test_polyglot_r_sidecar PROPERTIES TIMEOUT 45)
endif()

copperfin_add_test(test_polyglot_route_execution
    "cf_security;cf_platform_profile;cf_platform_support"
    test_polyglot_route_execution.cpp
)

copperfin_add_test(test_polyglot_runtime_host
    "cf_xbase_runtime;cf_security;cf_platform_profile;cf_platform_support"
    test_polyglot_runtime_host.cpp
    prg_engine_test_support.cpp
)

set(COPPERFIN_POLYGLOT_DOTNET_RID "")
if(DOTNET_EXECUTABLE)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" COPPERFIN_SYSTEM_PROCESSOR_LOWER)
    if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        if(COPPERFIN_SYSTEM_PROCESSOR_LOWER MATCHES "arm64|aarch64")
            set(COPPERFIN_POLYGLOT_DOTNET_RID "win-arm64")
        else()
            set(COPPERFIN_POLYGLOT_DOTNET_RID "win-x64")
        endif()
    elseif(APPLE)
        if(COPPERFIN_SYSTEM_PROCESSOR_LOWER MATCHES "arm64|aarch64")
            set(COPPERFIN_POLYGLOT_DOTNET_RID "osx-arm64")
        else()
            set(COPPERFIN_POLYGLOT_DOTNET_RID "osx-x64")
        endif()
    elseif(UNIX)
        if(COPPERFIN_SYSTEM_PROCESSOR_LOWER MATCHES "arm64|aarch64")
            set(COPPERFIN_POLYGLOT_DOTNET_RID "linux-arm64")
        elseif(COPPERFIN_SYSTEM_PROCESSOR_LOWER MATCHES "x86_64|amd64")
            set(COPPERFIN_POLYGLOT_DOTNET_RID "linux-x64")
        endif()
    endif()
endif()

if(NOT COPPERFIN_POLYGLOT_DOTNET_RID STREQUAL "")
    set(COPPERFIN_POLYGLOT_DOTNET_PUBLISH_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/polyglot-dotnet-candidate")
    set(COPPERFIN_POLYGLOT_DOTNET_CANDIDATE_PATH
        "${COPPERFIN_POLYGLOT_DOTNET_PUBLISH_DIR}/Copperfin.PolyglotCandidate${CMAKE_EXECUTABLE_SUFFIX}")
    add_custom_command(
        OUTPUT "${COPPERFIN_POLYGLOT_DOTNET_CANDIDATE_PATH}"
        COMMAND "${CMAKE_COMMAND}" -E rm -rf
            "${COPPERFIN_POLYGLOT_DOTNET_PUBLISH_DIR}"
        COMMAND "${DOTNET_EXECUTABLE}" publish
            "${CMAKE_SOURCE_DIR}/samples/polyglot-dotnet-candidate/Copperfin.PolyglotCandidate.csproj"
            --configuration Release
            --runtime "${COPPERFIN_POLYGLOT_DOTNET_RID}"
            --self-contained true
            --output "${COPPERFIN_POLYGLOT_DOTNET_PUBLISH_DIR}"
            --nologo
            -v minimal
            -p:UseSharedCompilation=false
        DEPENDS
            "${CMAKE_SOURCE_DIR}/samples/polyglot-dotnet-candidate/Copperfin.PolyglotCandidate.csproj"
            "${CMAKE_SOURCE_DIR}/samples/polyglot-dotnet-candidate/Program.cs"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Publishing the self-contained Native AOT .NET polyglot candidate"
        VERBATIM
    )
    add_custom_target(copperfin_polyglot_dotnet_candidate
        DEPENDS "${COPPERFIN_POLYGLOT_DOTNET_CANDIDATE_PATH}"
    )
    copperfin_add_test(test_polyglot_dotnet_candidate
        "cf_xbase_runtime;cf_security;cf_platform_profile;cf_platform_support"
        test_polyglot_dotnet_candidate.cpp
        prg_engine_test_support.cpp
    )
    add_dependencies(
        test_polyglot_dotnet_candidate
        copperfin_polyglot_dotnet_candidate
    )
    target_compile_definitions(test_polyglot_dotnet_candidate PRIVATE
        COPPERFIN_POLYGLOT_DOTNET_CANDIDATE_PATH="${COPPERFIN_POLYGLOT_DOTNET_CANDIDATE_PATH}"
    )
    set_tests_properties(test_polyglot_dotnet_candidate PROPERTIES TIMEOUT 30)
endif()

copperfin_add_test(test_polyglot_interop_envelope cf_platform_profile
    test_polyglot_interop_envelope.cpp
)
target_compile_definitions(
    test_polyglot_interop_envelope
    PRIVATE
        COPPERFIN_POLYGLOT_SUCCESS_ENVELOPE_PATH="${CMAKE_SOURCE_DIR}/docs/contracts/polyglot-success-envelope-v1.json"
        COPPERFIN_POLYGLOT_ERROR_ENVELOPE_PATH="${CMAKE_SOURCE_DIR}/docs/contracts/polyglot-error-envelope-v1.json"
        COPPERFIN_POLYGLOT_REQUEST_ENVELOPE_PATH="${CMAKE_SOURCE_DIR}/docs/contracts/polyglot-invocation-request-v1.json"
)

copperfin_add_test(test_bounded_process cf_platform_profile
    test_bounded_process.cpp
)

copperfin_add_test(test_runtime_external_event_queue cf_xbase_runtime
    test_runtime_external_event_queue.cpp
)

copperfin_add_test(test_polyglot_parity_comparator cf_platform_profile
    test_polyglot_parity_comparator.cpp
)

copperfin_add_test(test_polyglot_migration_telemetry cf_platform_profile
    test_polyglot_migration_telemetry.cpp
)

add_test(
    NAME test_polyglot_route_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_polyglot_route_contract_check.cmake
)

add_test(
    NAME test_polyglot_benchmark_result_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_polyglot_benchmark_result_contract_check.cmake
)

copperfin_add_test(test_platform_path cf_platform_support
    test_platform_path.cpp
)

copperfin_add_test(test_bounded_wide_string cf_platform_support
    test_bounded_wide_string.cpp
)

copperfin_add_test(test_platform_disk_space cf_platform_support
    test_platform_disk_space.cpp
)

copperfin_add_test(test_platform_exclusive_file cf_platform_support
    test_platform_exclusive_file.cpp
)

copperfin_add_test(test_platform_file_stream cf_platform_support
    test_platform_file_stream.cpp
)

copperfin_add_test(test_platform_font_directories cf_platform_support
    test_platform_font_directories.cpp
)

add_test(
    NAME test_platform_path_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_path_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_environment_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_environment_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_executable_path_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_executable_path_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_file_version_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_file_version_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_code_page_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_code_page_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_disk_space_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_disk_space_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_exclusive_file_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_exclusive_file_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_private_directory_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_private_directory_boundary_contract_check.cmake
)

add_test(
    NAME test_workspace_agent_process_image_materialization_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_workspace_agent_process_image_materialization_contract_check.cmake
)

add_test(
    NAME test_platform_file_stream_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_file_stream_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_font_directories_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_font_directories_boundary_contract_check.cmake
)

add_test(
    NAME test_platform_sqlite_api_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_platform_sqlite_api_boundary_contract_check.cmake
)

add_test(
    NAME test_portable_clr_host_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_portable_clr_host_boundary_contract_check.cmake
)

add_test(
    NAME test_native_declared_library_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_native_declared_library_boundary_contract_check.cmake
)

add_test(
    NAME test_native_declared_call_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_native_declared_call_boundary_contract_check.cmake
)

add_test(
    NAME test_printer_shell_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_printer_shell_boundary_contract_check.cmake
)

add_executable(test_platform_environment
    test_platform_environment.cpp
)
target_link_libraries(test_platform_environment PRIVATE cf_localization)
add_dependencies(test_platform_environment copperfin_runtime_host)

if(MSVC)
    target_compile_options(test_platform_environment PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_platform_environment PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_platform_environment
    COMMAND $<TARGET_FILE:test_platform_environment> $<TARGET_FILE:copperfin_runtime_host>
)

copperfin_add_test(test_query_translator cf_platform_profile
    test_query_translator.cpp
)

copperfin_add_test(test_federation_execution cf_platform_profile
    test_federation_execution.cpp
)

copperfin_add_test(test_sqlite_federation_connector cf_sqlite_connector
    test_sqlite_federation_connector.cpp
)
target_include_directories(test_sqlite_federation_connector PRIVATE
    ${CMAKE_SOURCE_DIR}/src)

add_executable(test_runtime_host_sqlite_federation
    test_runtime_host_sqlite_federation.cpp
)
target_link_libraries(test_runtime_host_sqlite_federation PRIVATE
    cf_sqlite_connector
    cf_platform_support
    cf_test_process_capture_support)
target_include_directories(test_runtime_host_sqlite_federation PRIVATE
    ${CMAKE_SOURCE_DIR}/src)
add_dependencies(test_runtime_host_sqlite_federation copperfin_runtime_host)
if(MSVC)
    target_compile_options(test_runtime_host_sqlite_federation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_runtime_host_sqlite_federation PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(
    NAME test_runtime_host_sqlite_federation
    COMMAND
        $<TARGET_FILE:test_runtime_host_sqlite_federation>
        $<TARGET_FILE:copperfin_runtime_host>
)

copperfin_add_test(test_runtime_pipeline cf_runtime_pipeline_test
    test_runtime_pipeline.cpp
    test_runtime_pipeline_shared_test_helpers.cpp
    test_runtime_pipeline_fixture_isolation.cpp
    test_runtime_pipeline_output_packaging_support.h
    test_runtime_pipeline_output_packaging_library.cpp
    test_runtime_pipeline_output_packaging_library_locale.cpp
    test_runtime_pipeline_output_packaging_fll.cpp
    test_runtime_pipeline_output_packaging_fxp_app.cpp
    test_runtime_pipeline_output_packaging_compiler.cpp
    test_runtime_pipeline_output_packaging_contracts.cpp
    test_runtime_pipeline_rematerialization.cpp
    test_runtime_pipeline_startup_casefold.cpp
    test_runtime_pipeline_startup_and_staging.cpp
    test_runtime_pipeline_launcher_and_security.cpp
    test_runtime_pipeline_manifest_pair.cpp
    test_runtime_pipeline_content_containment.cpp
)
target_compile_definitions(test_runtime_pipeline PRIVATE
    COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS=1
    COPPERFIN_TEST_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)
target_include_directories(test_runtime_pipeline PRIVATE ${CMAKE_SOURCE_DIR}/src/runtime)

add_executable(test_generated_launcher_process
    test_generated_launcher_process.cpp
)
target_link_libraries(test_generated_launcher_process PRIVATE cf_runtime_pipeline)
add_dependencies(test_generated_launcher_process copperfin_build_host)

if(MSVC)
    target_compile_options(test_generated_launcher_process PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_generated_launcher_process PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(DOTNET_EXECUTABLE)
    add_test(
        NAME test_generated_launcher_process
        COMMAND $<TARGET_FILE:test_generated_launcher_process>
            ${DOTNET_EXECUTABLE}
            $<TARGET_FILE:copperfin_build_host>
    )
else()
    add_test(
        NAME test_generated_launcher_process
        COMMAND $<TARGET_FILE:test_generated_launcher_process>
    )
endif()
set_tests_properties(test_generated_launcher_process PROPERTIES
    SKIP_RETURN_CODE 77
    TIMEOUT 600
)

if(NOT WIN32 AND DOTNET_EXECUTABLE)
    add_executable(test_generated_launcher_posix_process
        test_generated_launcher_posix_process.cpp
    )
    target_link_libraries(test_generated_launcher_posix_process PRIVATE
        cf_runtime_pipeline
        cf_vfp_assets
        cf_test_process_capture_support
    )
    add_dependencies(test_generated_launcher_posix_process copperfin_build_host copperfin_runtime_host)
    if(MSVC)
        target_compile_options(test_generated_launcher_posix_process PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_generated_launcher_posix_process PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(
        NAME test_generated_launcher_posix_process
        COMMAND $<TARGET_FILE:test_generated_launcher_posix_process>
            ${DOTNET_EXECUTABLE}
            $<TARGET_FILE:copperfin_build_host>
    )
    set_tests_properties(test_generated_launcher_posix_process PROPERTIES
        SKIP_RETURN_CODE 77
        TIMEOUT 600
    )
endif()

copperfin_add_test(test_security_controls cf_security
    test_security_controls.cpp
)
target_compile_definitions(test_security_controls PRIVATE
    COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS=1
)

copperfin_add_test(test_workspace_agent_policy cf_security
    test_workspace_agent_policy.cpp
)

copperfin_add_test(test_workspace_agent_session cf_security
    test_workspace_agent_session.cpp
)
target_compile_definitions(test_workspace_agent_session PRIVATE
    COPPERFIN_ENABLE_WORKSPACE_AGENT_SESSION_TEST_HOOKS=1
)

copperfin_add_test(test_workspace_agent_tool_registry cf_security
    test_workspace_agent_tool_registry.cpp
)

copperfin_add_test(test_workspace_agent_target_containment cf_security
    test_workspace_agent_target_containment.cpp
)

copperfin_add_test(test_workspace_agent_process_containment cf_security
    test_workspace_agent_process_containment.cpp
)

copperfin_add_test(test_workspace_agent_process_parser cf_security
    test_workspace_agent_process_parser.cpp
)
target_compile_definitions(test_workspace_agent_process_parser PRIVATE
    COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS=1
)

add_test(
    NAME test_workspace_agent_process_parser_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_workspace_agent_process_parser_boundary_contract_check.cmake
)

copperfin_add_test(test_workspace_agent_process_invocation cf_security
    test_workspace_agent_process_invocation.cpp
)

if(WIN32)
    copperfin_add_test(test_windows_com_event_adapter cf_xbase_runtime
        test_windows_com_event_adapter.cpp
    )
    target_include_directories(test_windows_com_event_adapter PRIVATE
        ${CMAKE_SOURCE_DIR}/src/runtime
    )

    add_executable(workspace_agent_execution_fixture
        workspace_agent_execution_fixture.cpp
    )

    add_executable(workspace_agent_execution_probe
        workspace_agent_execution_probe.cpp
    )
    target_compile_options(workspace_agent_execution_probe PRIVATE
        /W4 /permissive-
        $<$<CONFIG:Debug>:/MTd>
        $<$<NOT:$<CONFIG:Debug>>:/MT>
    )
    target_compile_options(workspace_agent_execution_fixture PRIVATE
        /W4 /permissive-
        $<$<CONFIG:Debug>:/MTd>
        $<$<NOT:$<CONFIG:Debug>>:/MT>
    )

    add_executable(test_workspace_agent_isolated_environment
        test_workspace_agent_isolated_environment.cpp
    )
    target_link_libraries(test_workspace_agent_isolated_environment PRIVATE
        cf_security
    )
    target_compile_options(test_workspace_agent_isolated_environment PRIVATE
        /W4 /permissive-
    )
    add_dependencies(
        test_workspace_agent_isolated_environment
        workspace_agent_execution_fixture
        workspace_agent_execution_probe
    )
    add_test(
        NAME test_workspace_agent_isolated_environment
        COMMAND
            $<TARGET_FILE:test_workspace_agent_isolated_environment>
            $<TARGET_FILE:workspace_agent_execution_fixture>
            $<TARGET_FILE:workspace_agent_execution_probe>
    )
else()
    add_executable(workspace_agent_execution_fixture
        workspace_agent_execution_fixture.cpp
    )
    target_compile_options(workspace_agent_execution_fixture PRIVATE
        -Wall -Wextra -Wpedantic
    )

    add_executable(test_workspace_agent_isolated_environment
        test_workspace_agent_isolated_environment.cpp
    )
    target_link_libraries(test_workspace_agent_isolated_environment PRIVATE
        cf_security
    )
    target_compile_options(test_workspace_agent_isolated_environment PRIVATE
        -Wall -Wextra -Wpedantic
    )
    add_dependencies(
        test_workspace_agent_isolated_environment
        workspace_agent_execution_fixture
    )
    add_test(
        NAME test_workspace_agent_isolated_environment
        COMMAND
            $<TARGET_FILE:test_workspace_agent_isolated_environment>
            $<TARGET_FILE:workspace_agent_execution_fixture>
    )
endif()

copperfin_add_test(test_platform_private_directory cf_platform_support
    test_platform_private_directory.cpp
)

copperfin_add_test(test_process_environment_serialization cf_platform_support
    test_process_environment_serialization.cpp
)

copperfin_add_test(test_process_argument_serialization cf_platform_support
    test_process_argument_serialization.cpp
)

copperfin_add_test(test_windows_pe_image cf_platform_support
    test_windows_pe_image.cpp
)

add_test(
    NAME test_windows_pe_image_boundary_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_windows_pe_image_boundary_contract_check.cmake
)

copperfin_add_test(test_workspace_agent_audit_sink cf_security
    test_workspace_agent_audit_sink.cpp
)

add_executable(test_studio_host_workspace_agent_policy
    test_studio_host_workspace_agent_policy.cpp
)
target_link_libraries(test_studio_host_workspace_agent_policy PRIVATE
    cf_test_process_capture_support
    cf_platform_support
)
if(MSVC)
    target_compile_options(test_studio_host_workspace_agent_policy PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_workspace_agent_policy PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(
    NAME test_studio_host_workspace_agent_policy
    COMMAND $<TARGET_FILE:test_studio_host_workspace_agent_policy>
        $<TARGET_FILE:copperfin_studio_host>
)

copperfin_add_test(test_security_audit_concurrency "cf_security;cf_test_process_capture_support"
    test_security_audit_concurrency.cpp
)

copperfin_add_test(test_prg_engine cf_xbase_runtime
    test_prg_engine.cpp
    test_prg_engine_verified_source_localization.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_dynamic_xasset_security cf_xbase_runtime
    test_prg_engine_dynamic_xasset_security.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_report_security cf_xbase_runtime
    test_prg_engine_report_security.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_verified_dbf_security cf_xbase_runtime
    test_prg_engine_verified_dbf_security.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_debugger cf_xbase_runtime
    test_prg_engine_debugger.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_work_areas cf_xbase_runtime
    test_prg_engine_work_areas.cpp
    prg_engine_test_support.cpp
)
# Exceeds the default 180s Windows CI budget on a resource-constrained hosted
# machine (observed 202.50s); same rationale as test_studio_host_json above.
set_tests_properties(test_prg_engine_work_areas PROPERTIES TIMEOUT 300)

copperfin_add_test(test_prg_engine_relations cf_xbase_runtime
    test_prg_engine_relations.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_ordered_iteration cf_xbase_runtime
    test_prg_engine_ordered_iteration.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_scope_declarations cf_xbase_runtime
    test_prg_engine_scope_declarations.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_database_lifecycle cf_xbase_runtime
    test_prg_engine_database_lifecycle.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_seek_index cf_xbase_runtime
    test_prg_engine_seek_index_cursor_state_01.cpp
    test_prg_engine_seek_index_cursor_state_02.cpp
    test_prg_engine_seek_index_declared_dll_01.cpp
    test_prg_engine_seek_index_declared_dll_02.cpp
    test_prg_engine_seek_index_declared_dll_03.cpp
    test_prg_engine_seek_index_main.cpp
    test_prg_engine_seek_index_seek_01.cpp
    test_prg_engine_seek_index_seek_02.cpp
    test_prg_engine_seek_index_seek_03.cpp
    prg_engine_test_support.cpp
)
if(WIN32)
    add_library(copperfin_declared_dll_fixture SHARED
        fixtures/declared_dll_fixture.cpp
    )
    target_compile_definitions(test_prg_engine_seek_index PRIVATE
        COPPERFIN_DECLARED_DLL_FIXTURE_NAME="$<TARGET_FILE_NAME:copperfin_declared_dll_fixture>"
    )
    add_dependencies(test_prg_engine_seek_index copperfin_declared_dll_fixture)

    if(MSVC)
        add_library(copperfin_mixed_mode_declared_dll_fixture SHARED
            fixtures/mixed_mode_declared_dll_fixture.cpp
        )
        set_target_properties(copperfin_mixed_mode_declared_dll_fixture PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            COPPERFIN_DISABLE_CFG TRUE
            COPPERFIN_MANAGED_CPP TRUE
        )
        target_compile_options(copperfin_mixed_mode_declared_dll_fixture PRIVATE
            /guard:cf-
            /clr
        )
        target_link_options(copperfin_mixed_mode_declared_dll_fixture PRIVATE /GUARD:NO)
    endif()

    find_program(COPPERFIN_FRAMEWORK_CSC
        NAMES csc.exe
        HINTS
            "$ENV{WINDIR}/Microsoft.NET/Framework/v4.0.30319"
            "$ENV{WINDIR}/Microsoft.NET/Framework64/v4.0.30319"
    )
    if(COPPERFIN_FRAMEWORK_CSC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM x64)
        else()
            set(COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM x86)
        endif()
        set(COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareFixture.dll")
        set(COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareFixture.x86.dll")
        set(COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareFixture.x64.dll")
        set(COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareDependency.dll")
        set(COPPERFIN_MANAGED_DECLARE_MISSING_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareMissing.dll")
        set(COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareRelative.dll")
        set(COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareParentless.dll")
        add_custom_command(
            OUTPUT
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/fixtures"
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:anycpu
                /out:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                managed_declare_dependency.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:x86
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:x64
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}
                managed_declare_fixture.cs
            DEPENDS
                "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/managed_declare_dependency.cs"
                "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/managed_declare_fixture.cs"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
            VERBATIM
        )
        add_custom_target(copperfin_managed_declare_fixture
            DEPENDS
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}"
        )
        copperfin_add_test(test_prg_engine_dotnet_dispatch cf_xbase_runtime
            test_prg_engine_dotnet_dispatch.cpp
            prg_engine_test_support.cpp
        )
        target_include_directories(test_prg_engine_dotnet_dispatch PRIVATE
            "${CMAKE_SOURCE_DIR}/src/runtime"
        )
        target_compile_definitions(test_prg_engine_dotnet_dispatch PRIVATE
            COPPERFIN_DECLARED_DLL_FIXTURE_PATH="$<TARGET_FILE:copperfin_declared_dll_fixture>"
            COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH="${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}"
            COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH="${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}"
            COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH="${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}"
            COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH="${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}"
            COPPERFIN_MANAGED_DECLARE_MISSING_PATH="${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}"
            COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH="${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}"
            COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH="${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}"
        )
        add_dependencies(
            test_prg_engine_dotnet_dispatch
            copperfin_declared_dll_fixture
            copperfin_managed_declare_fixture
        )
        if(TARGET copperfin_mixed_mode_declared_dll_fixture)
            target_compile_definitions(test_prg_engine_dotnet_dispatch PRIVATE
                COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH="$<TARGET_FILE:copperfin_mixed_mode_declared_dll_fixture>"
            )
            add_dependencies(
                test_prg_engine_dotnet_dispatch
                copperfin_mixed_mode_declared_dll_fixture
            )
        endif()
    else()
        message(STATUS "Legacy .NET Framework csc.exe not found; managed DECLARE runtime test is unavailable")
    endif()
endif()

copperfin_add_test(test_prg_engine_index_seek_optimization cf_xbase_runtime
    test_prg_engine_index_seek_optimization.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_rushmore_optimization cf_xbase_runtime
    test_prg_engine_rushmore_optimization.cpp
    test_prg_engine_rushmore_planning_contracts.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_control_flow cf_xbase_runtime
    test_prg_engine_control_flow.cpp
    test_prg_engine_control_flow_shared_test_helpers.cpp
    test_prg_engine_control_flow_control_flow_basics.cpp
    test_prg_engine_control_flow_basic_constructs.cpp
    test_prg_engine_control_flow_for_each.cpp
    test_prg_engine_control_flow_array_copy.cpp
    test_prg_engine_control_flow_implicit_scope.cpp
    test_prg_engine_control_flow_variables_and_scope.cpp
    test_prg_engine_control_flow_aggregate_and_total_commands.cpp
    test_prg_engine_select_query_commands.cpp
    test_prg_engine_control_flow_concurrency_and_critical_sections.cpp
    test_prg_engine_control_flow_critical_sections.cpp
    test_prg_engine_control_flow_task_supervision.cpp
    test_prg_engine_control_flow_yield_event.cpp
    test_prg_engine_control_flow_yield_critical_policy.cpp
    test_prg_engine_control_flow_yield_critical_sections.cpp
    test_prg_engine_control_flow_event_loop.cpp
    test_prg_engine_control_flow_error_handler_commands.cpp
    test_prg_engine_control_flow_bare_null.cpp
    test_prg_engine_control_flow_on_error_dispatch.cpp
    test_prg_engine_control_flow_aerror_runtime_metadata.cpp
    test_prg_engine_control_flow_aerror_sql_ole_metadata.cpp
    test_prg_engine_control_flow_aerror_innermost_line.cpp
    test_prg_engine_control_flow_error_handling_and_faults.cpp
    test_prg_engine_control_flow_catch_predicate_fallthrough.cpp
    test_prg_engine_control_flow_throw_exception_chain.cpp
    test_prg_engine_control_flow_bare_throw_rethrow.cpp
    test_prg_engine_control_flow_localized_faults.cpp
    test_prg_engine_control_flow_recovery_state.cpp
    test_prg_engine_control_flow_normalized_diagnostics.cpp
    test_prg_engine_control_flow_retry_resume.cpp
    test_prg_engine_control_flow_bare_throw_default.cpp
    test_prg_engine_control_flow_localized_errors.cpp
    test_prg_engine_control_flow_fault_recovery.cpp
    test_prg_engine_control_flow_runtime_guardrails_and_lifecycle.cpp
    test_prg_engine_control_flow_routine_calls_parameters.cpp
    test_prg_engine_control_flow_routine_calls_continuations_01.cpp
    test_prg_engine_control_flow_routine_calls_continuations_02.cpp
    test_prg_engine_control_flow_routine_calls_array_parameters.cpp
    test_prg_engine_control_flow_routine_calls_predicates_01.cpp
    test_prg_engine_control_flow_routine_calls_predicates_02.cpp
    test_prg_engine_control_flow_routine_calls_set_procedure.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_parser_classes cf_xbase_runtime
    test_prg_engine_parser_classes.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_macro_expressions cf_xbase_runtime
    test_prg_engine_macro_expressions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_json_facade cf_xbase_runtime
    test_prg_engine_json_facade.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_polyglot_dispatch cf_xbase_runtime
    test_prg_engine_polyglot_dispatch.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_platform_safe_regex cf_platform_support
    test_platform_safe_regex.cpp
)

copperfin_add_test(test_prg_engine_regex_facade cf_xbase_runtime
    test_prg_engine_regex_facade.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_payload_crypto cf_security
    test_payload_crypto.cpp
)

copperfin_add_test(test_prg_engine_payload_crypto_facade cf_xbase_runtime
    test_prg_engine_payload_crypto_facade.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_data_io cf_xbase_runtime
    test_prg_engine_data_io.cpp
    test_prg_engine_data_io_shared_test_helpers.cpp
    test_prg_engine_data_io_scatter_gather.cpp
    test_prg_engine_data_io_import_export.cpp
    test_prg_engine_data_io_save_restore.cpp
    test_prg_engine_data_io_display_and_list.cpp
    test_prg_engine_data_io_interactive_io_commands.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_on_key_label cf_xbase_runtime
    test_prg_engine_on_key_label.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_on_function cf_xbase_runtime
    test_prg_engine_on_function.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_on_escape cf_xbase_runtime
    test_prg_engine_on_escape.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_on_page cf_xbase_runtime
    test_prg_engine_on_page.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_arrays cf_xbase_runtime
    test_prg_engine_arrays.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_functions cf_xbase_runtime
    test_prg_engine_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_runtime_surface_functions cf_xbase_runtime
    test_prg_engine_runtime_surface_functions_collections_01.cpp
    test_prg_engine_runtime_surface_functions_collections_02.cpp
    test_prg_engine_runtime_surface_functions_controls_01.cpp
    test_prg_engine_runtime_surface_functions_controls_02.cpp
    test_prg_engine_runtime_surface_functions_controls_03.cpp
    test_prg_engine_runtime_surface_functions_controls_04.cpp
    test_prg_engine_runtime_surface_functions_default_actions_01.cpp
    test_prg_engine_runtime_surface_functions_default_actions_02.cpp
    test_prg_engine_runtime_surface_functions_default_actions_03.cpp
    test_prg_engine_runtime_surface_functions_default_actions_04.cpp
    test_prg_engine_runtime_surface_functions_events_01.cpp
    test_prg_engine_runtime_surface_functions_events_02.cpp
    test_prg_engine_runtime_surface_functions_events_03.cpp
    test_prg_engine_runtime_surface_functions_expressions_01.cpp
    test_prg_engine_runtime_surface_functions_sys16.cpp
    test_prg_engine_runtime_surface_functions_filesystem_sys.cpp
    test_prg_engine_runtime_surface_functions_sys2012.cpp
    test_prg_engine_runtime_surface_functions_sys2030.cpp
    test_prg_engine_runtime_surface_functions_sys2021.cpp
    test_prg_engine_runtime_surface_functions_expressions_02.cpp
    test_prg_engine_runtime_surface_functions_forms_01.cpp
    test_prg_engine_runtime_surface_functions_forms_02.cpp
    test_prg_engine_runtime_surface_functions_inheritance_01.cpp
    test_prg_engine_runtime_surface_functions_inheritance_02.cpp
    test_prg_engine_runtime_surface_functions_inheritance_03.cpp
    test_prg_engine_runtime_surface_functions_inheritance_04.cpp
    test_prg_engine_runtime_surface_functions_inheritance_05.cpp
    test_prg_engine_runtime_surface_functions_inheritance_06.cpp
    test_prg_engine_runtime_surface_functions_inheritance_07.cpp
    test_prg_engine_runtime_surface_functions_inheritance_08.cpp
    test_prg_engine_runtime_surface_functions_inheritance_09.cpp
    test_prg_engine_runtime_surface_functions_instantiation_01.cpp
    test_prg_engine_runtime_surface_functions_interop_01.cpp
    test_prg_engine_runtime_surface_functions_interop_02.cpp
    test_prg_engine_runtime_surface_functions_keypress.cpp
    test_prg_engine_runtime_surface_functions_interactivechange.cpp
    test_prg_engine_runtime_surface_functions_keypreview.cpp
    test_prg_engine_runtime_surface_functions_click.cpp
    test_prg_engine_runtime_surface_functions_mouse.cpp
    test_prg_engine_runtime_surface_functions_mousemove.cpp
    test_prg_engine_runtime_surface_functions_dblclick.cpp
    test_prg_engine_runtime_surface_functions_rightclick.cpp
    test_prg_engine_runtime_surface_functions_right_button_events.cpp
    test_prg_engine_runtime_surface_functions_middleclick.cpp
    test_prg_engine_runtime_surface_functions_middle_button_events.cpp
    test_prg_engine_runtime_surface_functions_interop_03.cpp
    test_prg_engine_runtime_surface_functions_interop_04.cpp
    test_prg_engine_runtime_surface_functions_interop_05.cpp
    test_prg_engine_runtime_surface_functions_interop_06.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_01.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_02.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_03.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_04.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_05.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_06.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_07.cpp
    test_prg_engine_runtime_surface_functions_lifecycle_08.cpp
    test_prg_engine_runtime_surface_functions_report_listener.cpp
    test_prg_engine_runtime_surface_functions_report_listener_security.cpp
    test_prg_engine_runtime_surface_functions_wvisible.cpp
    test_prg_engine_runtime_surface_functions_list_controls_01.cpp
    test_prg_engine_runtime_surface_functions_list_controls_02.cpp
    test_prg_engine_runtime_surface_functions_list_controls_03.cpp
    test_prg_engine_runtime_surface_functions_list_controls_04.cpp
    test_prg_engine_runtime_surface_functions_list_controls_05.cpp
    test_prg_engine_runtime_surface_functions_list_controls_06.cpp
    test_prg_engine_runtime_surface_functions_list_controls_07.cpp
    test_prg_engine_runtime_surface_functions_list_controls_08.cpp
    test_prg_engine_runtime_surface_functions_list_controls_09.cpp
    test_prg_engine_runtime_surface_functions_list_controls_10.cpp
    test_prg_engine_runtime_surface_functions_list_controls_11.cpp
    test_prg_engine_runtime_surface_functions_list_controls_12.cpp
    test_prg_engine_runtime_surface_functions_list_controls_13.cpp
    test_prg_engine_runtime_surface_functions_list_controls_14.cpp
    test_prg_engine_runtime_surface_functions_list_controls_15.cpp
    test_prg_engine_runtime_surface_functions_list_controls_16.cpp
    test_prg_engine_runtime_surface_functions_list_controls_17.cpp
    test_prg_engine_runtime_surface_functions_list_controls_18.cpp
    test_prg_engine_runtime_surface_functions_list_controls_19.cpp
    test_prg_engine_runtime_surface_functions_list_controls_20.cpp
    test_prg_engine_runtime_surface_functions_main.cpp
    test_prg_engine_runtime_surface_functions_object_graph_01.cpp
    test_prg_engine_runtime_surface_functions_object_graph_02.cpp
    test_prg_engine_runtime_surface_functions_object_graph_03.cpp
    test_prg_engine_runtime_surface_functions_object_graph_04.cpp
    test_prg_engine_runtime_surface_functions_object_graph_05.cpp
    test_prg_engine_runtime_surface_functions_object_graph_06.cpp
    test_prg_engine_runtime_surface_functions_object_graph_07.cpp
    test_prg_engine_runtime_surface_functions_object_graph_08.cpp
    test_prg_engine_runtime_surface_functions_object_graph_09.cpp
    test_prg_engine_runtime_surface_functions_object_graph_10.cpp
    test_prg_engine_runtime_surface_functions_object_graph_11.cpp
    test_prg_engine_runtime_surface_functions_object_graph_12.cpp
    test_prg_engine_runtime_surface_functions_properties_01.cpp
    test_prg_engine_runtime_surface_functions_properties_02.cpp
    test_prg_engine_runtime_surface_functions_properties_03.cpp
    test_prg_engine_runtime_surface_functions_properties_04.cpp
    test_prg_engine_runtime_surface_functions_properties_05.cpp
    test_prg_engine_runtime_surface_functions_properties_06.cpp
    test_prg_engine_runtime_surface_functions_properties_07.cpp
    test_prg_engine_runtime_surface_functions_properties_08.cpp
    test_prg_engine_runtime_surface_functions_properties_09.cpp
    test_prg_engine_runtime_surface_functions_properties_10.cpp
    test_prg_engine_runtime_surface_functions_properties_11.cpp
    test_prg_engine_runtime_surface_functions_properties_12.cpp
    test_prg_engine_runtime_surface_functions_picture.cpp
    test_prg_engine_runtime_surface_functions_drag.cpp
    test_prg_engine_runtime_surface_functions_button_pictures.cpp
    test_prg_engine_runtime_surface_functions_command_button_properties.cpp
    test_prg_engine_runtime_surface_functions_autosize.cpp
    test_prg_engine_runtime_surface_functions_properties_13.cpp
    test_prg_engine_runtime_surface_functions_properties_14.cpp
    test_prg_engine_runtime_surface_functions_properties_15.cpp
    test_prg_engine_runtime_surface_functions_properties_16.cpp
    test_prg_engine_runtime_surface_functions_properties_17.cpp
    test_prg_engine_runtime_surface_functions_properties_18.cpp
    test_prg_engine_runtime_surface_functions_properties_19.cpp
    test_prg_engine_runtime_surface_functions_properties_20.cpp
    test_prg_engine_runtime_surface_functions_properties_21.cpp
    test_prg_engine_runtime_surface_functions_properties_22.cpp
    test_prg_engine_runtime_surface_functions_properties_23.cpp
    test_prg_engine_runtime_surface_functions_properties_24.cpp
    test_prg_engine_runtime_surface_functions_properties_25.cpp
    test_prg_engine_runtime_surface_functions_properties_26.cpp
    test_prg_engine_runtime_surface_functions_properties_27.cpp
    test_prg_engine_runtime_surface_functions_properties_28.cpp
    test_prg_engine_runtime_surface_functions_properties_29.cpp
    test_prg_engine_runtime_surface_functions_properties_30.cpp
    test_prg_engine_runtime_surface_functions_properties_31.cpp
    test_prg_engine_runtime_surface_functions_properties_32.cpp
    test_prg_engine_runtime_surface_functions_properties_33.cpp
    test_prg_engine_runtime_surface_functions_properties_34.cpp
    test_prg_engine_runtime_surface_functions_properties_35.cpp
    test_prg_engine_runtime_surface_functions_properties_36.cpp
    test_prg_engine_runtime_surface_functions_properties_37.cpp
    test_prg_engine_runtime_surface_functions_properties_38.cpp
    test_prg_engine_runtime_surface_functions_properties_39.cpp
    test_prg_engine_runtime_surface_functions_properties_40.cpp
    test_prg_engine_runtime_surface_functions_properties_41.cpp
    test_prg_engine_runtime_surface_functions_properties_42.cpp
    test_prg_engine_runtime_surface_functions_properties_43.cpp
    test_prg_engine_runtime_surface_functions_properties_44.cpp
    test_prg_engine_runtime_surface_functions_properties_45.cpp
    test_prg_engine_runtime_surface_functions_properties_46.cpp
    test_prg_engine_runtime_surface_functions_properties_47.cpp
    test_prg_engine_runtime_surface_functions_properties_48.cpp
    test_prg_engine_runtime_surface_functions_properties_49.cpp
    test_prg_engine_runtime_surface_functions_properties_50.cpp
    test_prg_engine_runtime_surface_functions_properties_51.cpp
    test_prg_engine_runtime_surface_functions_properties_52.cpp
    test_prg_engine_runtime_surface_functions_properties_53.cpp
    test_prg_engine_runtime_surface_functions_properties_54.cpp
    test_prg_engine_runtime_surface_functions_properties_55.cpp
    test_prg_engine_runtime_surface_functions_properties_56.cpp
    test_prg_engine_runtime_surface_functions_properties_57.cpp
    test_prg_engine_runtime_surface_functions_properties_58.cpp
    test_prg_engine_runtime_surface_functions_properties_59.cpp
    test_prg_engine_runtime_surface_functions_properties_60.cpp
    test_prg_engine_runtime_surface_functions_properties_61.cpp
    test_prg_engine_runtime_surface_functions_properties_62.cpp
    test_prg_engine_runtime_surface_functions_properties_63.cpp
    test_prg_engine_runtime_surface_functions_properties_64.cpp
    test_prg_engine_runtime_surface_functions_properties_65.cpp
    test_prg_engine_runtime_surface_functions_properties_66.cpp
    test_prg_engine_runtime_surface_functions_properties_67.cpp
    test_prg_engine_runtime_surface_functions_properties_68.cpp
    test_prg_engine_runtime_surface_functions_properties_69.cpp
    test_prg_engine_runtime_surface_functions_properties_70.cpp
    test_prg_engine_runtime_surface_functions_form_scalemode.cpp
    test_prg_engine_runtime_surface_functions_drawmode.cpp
    test_prg_engine_runtime_surface_functions_backstyle.cpp
    test_prg_engine_runtime_surface_functions_specialeffect.cpp
    test_prg_engine_runtime_surface_functions_borderstyle.cpp
    test_prg_engine_runtime_surface_functions_drawstyle.cpp
    test_prg_engine_runtime_surface_functions_fillstyle.cpp
    test_prg_engine_runtime_surface_functions_fillcolor.cpp
    test_prg_engine_runtime_surface_functions_borderwidth.cpp
    test_prg_engine_runtime_surface_functions_bordercolor.cpp
    test_prg_engine_runtime_surface_functions_drawwidth.cpp
    test_prg_engine_runtime_surface_functions_grid_rowheight.cpp
    test_prg_engine_runtime_surface_functions_grid_headerheight.cpp
    test_prg_engine_runtime_surface_functions_grid_scrollbars.cpp
    test_prg_engine_runtime_surface_functions_grid_allowheadersizing.cpp
    test_prg_engine_runtime_surface_functions_grid_allowrowsizing.cpp
    test_prg_engine_runtime_surface_functions_grid_allowautocolumnfit.cpp
    test_prg_engine_runtime_surface_functions_grid_gridline_properties.cpp
    test_prg_engine_runtime_surface_functions_grid_highlight_properties.cpp
    test_prg_engine_runtime_surface_functions_grid_view.cpp
    test_prg_engine_runtime_surface_functions_grid_activecolumn.cpp
    test_prg_engine_runtime_surface_functions_grid_activerow.cpp
    test_prg_engine_runtime_surface_functions_grid_relative_focus.cpp
    test_prg_engine_runtime_surface_functions_column_resizable.cpp
    test_prg_engine_runtime_surface_functions_reflection_01.cpp
    test_prg_engine_runtime_surface_functions_reflection_02.cpp
    test_prg_engine_runtime_surface_functions_reflection_03.cpp
    test_prg_engine_runtime_surface_functions_reflection_04.cpp
    test_prg_engine_runtime_surface_functions_reflection_05.cpp
    test_prg_engine_runtime_surface_functions_reflection_06.cpp
    test_prg_engine_runtime_surface_functions_reflection_07.cpp
    test_prg_engine_runtime_surface_functions_reflection_08.cpp
    test_prg_engine_runtime_surface_functions_reflection_09.cpp
    test_prg_engine_runtime_surface_functions_reflection_10.cpp
    test_prg_engine_runtime_surface_functions_reflection_11.cpp
    test_prg_engine_runtime_surface_functions_reflection_12.cpp
    test_prg_engine_runtime_surface_functions_reflection_13.cpp
    test_prg_engine_runtime_surface_functions_reflection_14.cpp
    test_prg_engine_runtime_surface_functions_reflection_15.cpp
    test_prg_engine_runtime_surface_functions_reflection_16.cpp
    prg_engine_test_support.cpp
)
target_include_directories(test_prg_engine_runtime_surface_functions PRIVATE
    ${CMAKE_SOURCE_DIR}/src/runtime
)

copperfin_add_test(test_prg_engine_runtime_surface_functions_buffering cf_xbase_runtime
    test_prg_engine_runtime_surface_functions_buffering.cpp
    test_prg_engine_runtime_surface_functions_buffering_main.cpp
    prg_engine_test_support.cpp
)
target_include_directories(test_prg_engine_runtime_surface_functions_buffering PRIVATE
    ${CMAKE_SOURCE_DIR}/src/runtime
)

copperfin_add_test(test_prg_engine_runtime_surface_procedure_classes cf_xbase_runtime
    test_prg_engine_runtime_surface_procedure_classes.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_locale_code_page cf_xbase_runtime
    test_prg_engine_locale_code_page.cpp
    prg_engine_test_support.cpp
)
target_include_directories(test_prg_engine_locale_code_page PRIVATE
    ${CMAKE_SOURCE_DIR}/src/runtime
)

copperfin_add_test(test_prg_engine_runtime_surface_itemid_selectors cf_xbase_runtime
    test_prg_engine_runtime_surface_itemid_selectors.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_runtime_surface_requery cf_xbase_runtime
    test_prg_engine_runtime_surface_requery.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_runtime_surface_value cf_xbase_runtime
    test_prg_engine_runtime_surface_value.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_file_io_functions cf_xbase_runtime
    test_prg_engine_file_io_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_native_focus_move_events cf_xbase_runtime
    test_prg_engine_native_focus_move_events.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_native_lifecycle_events cf_xbase_runtime
    test_prg_engine_native_lifecycle_events.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_transform_numeric_pictures cf_xbase_runtime
    test_prg_engine_transform_numeric_pictures.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_aggregate_array_functions cf_xbase_runtime
    test_prg_engine_aggregate_array_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_string_math_functions cf_xbase_runtime
    test_prg_engine_string_math_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_string_minus cf_xbase_runtime
    test_prg_engine_string_minus.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_path_functions cf_xbase_runtime
    test_prg_engine_path_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_date_time_functions cf_xbase_runtime
    test_prg_engine_date_time_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_sql_cursors_metadata cf_xbase_runtime
    test_prg_engine_sql_cursors_metadata.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_sql_cursors_mutation cf_xbase_runtime
    test_prg_engine_sql_cursors_mutation.cpp
    test_prg_engine_sql_cursors_mutation_navigation.cpp
    test_prg_engine_sql_cursors_mutation_lifecycle.cpp
    test_prg_engine_sql_cursors_mutation_import_export.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_sql_cursors_seek_and_order cf_xbase_runtime
    test_prg_engine_sql_cursors_seek_and_order.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_table_mutation cf_xbase_runtime
    test_prg_engine_table_mutation_basic.cpp
    test_prg_engine_table_mutation_maintenance.cpp
    test_prg_engine_table_mutation_locks.cpp
    test_prg_engine_table_mutation_data.cpp
    test_prg_engine_table_mutation_undo.cpp
    test_prg_engine_table_mutation_transactions.cpp
    test_prg_engine_table_mutation_main.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_table_structure cf_xbase_runtime
    test_prg_engine_table_structure.cpp
    prg_engine_test_support.cpp
)
target_include_directories(test_prg_engine_table_structure PRIVATE
    ${CMAKE_SOURCE_DIR}/src/runtime
)

copperfin_add_test(test_xasset_methods cf_xbase_runtime
    test_xasset_methods.cpp
)

# Studio-host tests launch this executable out of process. A TARGET_FILE generator
# expression in add_test() does not make narrow test-target builds rebuild the host.
get_property(copperfin_test_directory_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
set(copperfin_process_capture_test_sources
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_settings_memo_stable.cpp
    test_studio_host_selected_sections_stable.cpp
    test_studio_host_selected_page_header_sections_stable.cpp
    test_studio_host_selected_page_header_objects_orphaned.cpp
    test_studio_host_selected_detail_objects_orphaned.cpp
    test_studio_host_selected_report_sections.cpp
    test_studio_host_selected_label_sections.cpp
    test_studio_host_settings_memo_record.cpp
    test_studio_host_column_setup_record_settings.cpp
    test_studio_host_column_setup_stable_settings.cpp
    test_studio_host_section_delete_record_selection.cpp
    test_studio_host_section_delete_stable_selection.cpp
    test_visual_asset_editor_code_page.cpp
)
foreach(copperfin_test_target IN LISTS copperfin_test_directory_targets)
    get_target_property(copperfin_test_target_type ${copperfin_test_target} TYPE)
    if(NOT copperfin_test_target_type STREQUAL "EXECUTABLE")
        continue()
    endif()
    get_target_property(copperfin_test_target_sources ${copperfin_test_target} SOURCES)
    foreach(copperfin_capture_source IN LISTS copperfin_process_capture_test_sources)
        if(copperfin_capture_source IN_LIST copperfin_test_target_sources)
            target_link_libraries(${copperfin_test_target} PRIVATE cf_test_process_capture_support)
            break()
        endif()
    endforeach()
endforeach()

set(copperfin_self_contained_studio_host_tests
    test_studio_host
    test_studio_host_shell_command
)
foreach(copperfin_test_target IN LISTS copperfin_test_directory_targets)
    if(copperfin_test_target MATCHES "^test_studio_host($|_)" AND
       NOT copperfin_test_target IN_LIST copperfin_self_contained_studio_host_tests)
        get_target_property(copperfin_test_target_type ${copperfin_test_target} TYPE)
        if(copperfin_test_target_type STREQUAL "EXECUTABLE")
            add_dependencies(${copperfin_test_target} copperfin_studio_host)
        endif()
    endif()
endforeach()

copperfin_reuse_test_support_object(
    prg_engine_test_support.cpp
    cf_prg_engine_test_support
)
copperfin_reuse_test_support_object(
    test_studio_host_json_shared_test_helpers.cpp
    cf_studio_host_json_shared_test_helpers
)
copperfin_reuse_test_support_object(
    test_studio_host_json_report_shared_fixtures.cpp
    cf_studio_host_json_report_shared_fixtures
)
copperfin_reuse_test_support_object(
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    cf_studio_host_json_objects_dhf_shared_fixtures
)
copperfin_reuse_test_support_object(
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    cf_studio_host_json_sections_dhf_shared_fixtures
)
copperfin_reuse_test_support_object(
    test_studio_host_json_report_settings_shared_fixtures.cpp
    cf_studio_host_json_report_settings_shared_fixtures
)
copperfin_reuse_test_support_object(
    test_studio_host_json_report_section_shared_fixtures.cpp
    cf_studio_host_json_report_section_shared_fixtures
)
copperfin_reuse_test_support_object(
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    cf_studio_host_json_objects_dhf_expressions_and_exposure
)
copperfin_reuse_test_support_object(
    test_studio_host_json_objects_layout_and_expressions.cpp
    cf_studio_host_json_objects_layout_and_expressions
)
copperfin_reuse_test_support_object(
    test_studio_host_json_report_object_fallbacks.cpp
    cf_studio_host_json_report_object_fallbacks
)

copperfin_reuse_test_support_object_for_targets(
    test_studio_host_json_geometry_preview_helpers.cpp
    cf_studio_host_json_geometry_preview_helpers
    ""
    test_studio_host_label_layout_actions
    test_studio_host_report_font_metadata
    test_studio_host_report_layout_actions
    test_studio_host_report_layout_placement
    test_studio_host_report_live_section_geometry
)
copperfin_reuse_test_support_object_for_targets(
    test_studio_host_json_geometry_preview_bounds.cpp
    cf_studio_host_json_geometry_preview_helpers
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    test_studio_host_deleted_object_height_top_geometry
    test_studio_host_deleted_object_restore_geometry
    test_studio_host_deleted_object_width_left_geometry
    test_studio_host_deleted_report_visual_property_copies
    test_studio_host_deleted_report_visual_property_rejections
    test_studio_host_deleted_section_geometry
    test_studio_host_detail_header_footer_object_geometry
    test_studio_host_edited_object_delete_geometry
    test_studio_host_report_column_setup
    test_studio_host_report_column_width_fields
    test_studio_host_report_direct_setting_fields
    test_studio_host_report_geometry_defaults
    test_studio_host_report_layout_classifications
    test_studio_host_report_page_setup_fields
    test_studio_host_report_settings_diagnostics
    test_studio_host_report_settings_memo_parsing
)
copperfin_reuse_test_support_object_for_targets(
    test_studio_host_json_geometry_preview_bounds.cpp
    cf_studio_host_json_geometry_preview_helpers_with_unresolved
    "COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS;COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS"
    test_studio_host_report_schema_fallbacks
    test_studio_host_report_unresolved_memo_placeholders
)
copperfin_reuse_test_support_object_for_targets(
    test_studio_host_json_geometry_dimensions.cpp
    cf_studio_host_json_geometry_dimensions_with_unresolved
    "COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS;COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS"
    test_studio_host_report_schema_fallbacks
    test_studio_host_report_unresolved_memo_placeholders
)
copperfin_reuse_test_support_object_for_targets(
    test_studio_host_json_sections_diagnostics_and_misc.cpp
    cf_studio_host_json_sections_diagnostics_with_unresolved
    "COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS;COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS"
    test_studio_host_report_schema_fallbacks
    test_studio_host_report_unresolved_memo_placeholders
)
copperfin_reuse_test_support_object_for_targets(
    test_studio_host_json_settings_column_setup.cpp
    cf_studio_host_json_settings_column_setup_helpers
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    test_studio_host_report_column_setup
    test_studio_host_report_direct_setting_fields
)
copperfin_reuse_test_support_object_for_targets(
    test_studio_host_json_settings_page_layout_fields.cpp
    cf_studio_host_json_settings_page_layout_helpers
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    test_studio_host_report_direct_setting_fields
    test_studio_host_report_page_setup_fields
)

include("${CMAKE_CURRENT_SOURCE_DIR}/CopperfinTestIsolation.cmake")
cmake_language(DEFER CALL copperfin_configure_native_test_isolation)

function(copperfin_configure_native_test_resource_environment)
    get_property(copperfin_native_tests DIRECTORY PROPERTY TESTS)
    foreach(copperfin_native_test IN LISTS copperfin_native_tests)
        get_property(
            copperfin_native_test_environment
            TEST ${copperfin_native_test}
            PROPERTY ENVIRONMENT
        )
        set(copperfin_native_test_has_locale_dir FALSE)
        foreach(copperfin_native_test_environment_entry IN LISTS copperfin_native_test_environment)
            if(copperfin_native_test_environment_entry MATCHES "^COPPERFIN_LOCALE_DIR=")
                set(copperfin_native_test_has_locale_dir TRUE)
                break()
            endif()
        endforeach()
        if(copperfin_native_test_has_locale_dir)
            continue()
        endif()
        set_property(
            TEST ${copperfin_native_test}
            APPEND PROPERTY ENVIRONMENT
                "COPPERFIN_LOCALE_DIR=${PROJECT_SOURCE_DIR}/resources/locales"
        )
    endforeach()
endfunction()

cmake_language(DEFER CALL copperfin_configure_native_test_resource_environment)
