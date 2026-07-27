if(NOT DEFINED DOTNET_EXECUTABLE OR DOTNET_EXECUTABLE STREQUAL "")
    message(FATAL_ERROR "DOTNET_EXECUTABLE is required for the managed compile check.")
endif()

if(NOT DEFINED SOURCE_DIR OR SOURCE_DIR STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required for the managed compile check.")
endif()

set(test_configuration "${TEST_CONFIGURATION}")
if(test_configuration STREQUAL "")
    set(test_configuration "Debug")
endif()

function(run_managed_build project_relative_path)
    set(project_path "${SOURCE_DIR}/${project_relative_path}")
    execute_process(
        COMMAND "${DOTNET_EXECUTABLE}" build "${project_path}" --configuration "${test_configuration}" --nologo -v minimal
        WORKING_DIRECTORY "${SOURCE_DIR}"
        COMMAND_ECHO STDOUT
        RESULT_VARIABLE build_result
    )

    if(NOT build_result EQUAL 0)
        message(FATAL_ERROR "Managed compile check failed for ${project_relative_path}")
    endif()
endfunction()

function(run_managed_test project_relative_path)
    set(project_path "${SOURCE_DIR}/${project_relative_path}")
    execute_process(
        COMMAND "${DOTNET_EXECUTABLE}" run
            --project "${project_path}"
            --configuration "${test_configuration}"
            --no-build
        WORKING_DIRECTORY "${SOURCE_DIR}"
        COMMAND_ECHO STDOUT
        RESULT_VARIABLE test_result
    )

    if(NOT test_result EQUAL 0)
        message(FATAL_ERROR "Managed test failed for ${project_relative_path}")
    endif()
endfunction()

file(READ "${SOURCE_DIR}/vsix/Copperfin.Studio/Copperfin.Studio.csproj" standalone_studio_project)
if(NOT standalone_studio_project MATCHES "CopperfinAssetEditorControl[.]ProjectExplorer[.]cs")
    message(FATAL_ERROR
        "Standalone Studio project must compile the shared Project Explorer partial source")
endif()

run_managed_build("vsix/Copperfin.LanguageServiceTests/Copperfin.StudioTargetSelectionTests.csproj")
run_managed_test("vsix/Copperfin.LanguageServiceTests/Copperfin.StudioTargetSelectionTests.csproj")
run_managed_build("tools/Copperfin.LocalizationCatalogGenerator/Copperfin.LocalizationCatalogGenerator.csproj")
execute_process(
    COMMAND "${DOTNET_EXECUTABLE}" run
        --project "${SOURCE_DIR}/tools/Copperfin.LocalizationCatalogGenerator/Copperfin.LocalizationCatalogGenerator.csproj"
        --configuration "${test_configuration}"
        --no-build -- "${SOURCE_DIR}" --check
    WORKING_DIRECTORY "${SOURCE_DIR}"
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE catalog_result
)
if(NOT catalog_result EQUAL 0)
    message(FATAL_ERROR "Managed localization catalog parity check failed")
endif()
run_managed_build("vsix/Copperfin.LanguageServiceTests/Copperfin.LanguageServiceTests.csproj")
run_managed_test("vsix/Copperfin.LanguageServiceTests/Copperfin.LanguageServiceTests.csproj")
run_managed_build("vsix/Copperfin.ProcessRunnerNetFrameworkTests/Copperfin.ProcessRunnerNetFrameworkTests.csproj")
run_managed_build("vsix/Copperfin.Studio/Copperfin.Studio.csproj")
run_managed_build("vsix/Copperfin.DesignerSmokeTests/Copperfin.DesignerSmokeTests.csproj")
