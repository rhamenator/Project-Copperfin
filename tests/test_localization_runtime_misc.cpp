// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_localization_support.h"

void test_runtime_set_filter_dimension_sleep_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions",
        "Runtime.Prg.Dispatch.Error.SetFilterRequiresSelectedWorkArea",
        "Runtime.Prg.Dispatch.Error.SleepInvalidDuration"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SetFilterRequiresSelectedWorkArea") ==
            "SET FILTER requires a selected work area",
        "#2714: SET FILTER selected-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions") ==
            "DIMENSION/DECLARE requires array dimensions",
        "#2714: DIMENSION/DECLARE array-dimensions error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SleepInvalidDuration") ==
            "SLEEP: invalid duration",
        "#2714: SLEEP invalid-duration error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2714: es-419 should define every SET FILTER/DIMENSION/SLEEP runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2714: pt-BR should define every SET FILTER/DIMENSION/SLEEP runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2714: qps-ploc should define every SET FILTER/DIMENSION/SLEEP runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.SetFilterRequiresSelectedWorkArea")
                .find("requires a selected work area") == std::string::npos,
        "#2714: es-419 SET FILTER selected-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.SleepInvalidDuration")
                .find("invalid duration") == std::string::npos,
        "#2714: pt-BR SLEEP invalid-duration error should not fall back to raw English");

    const std::string pseudo_dimension =
        pseudo.translate("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions");
    expect(
        pseudo_dimension.find("[!! ") == 0U &&
            pseudo_dimension.find("requires array dimensions") == std::string::npos,
        "#2714: qps-ploc DIMENSION/DECLARE error should pseudo-localize prose");
}

void test_runtime_declare_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap load_placeholders{
        {"path", "kernel32.dll"},
        {"errorMessage", "Access is denied."}
    };
    const copperfin::localization::PlaceholderMap function_placeholders{
        {"functionName", "MissingSymbol"},
        {"path", "kernel32.dll"}
    };
    const copperfin::localization::PlaceholderMap parameter_type_placeholders{
        {"parameterType", "SHORT"}
    };
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.DeclareCannotLoadDll",
        "Runtime.Prg.Dispatch.Error.DeclareDllOnlySupportedOnWindows",
        "Runtime.Prg.Dispatch.Error.DeclareFunctionNotFoundInDll",
        "Runtime.Prg.Dispatch.Error.DeclareMissingFunctionNameOrDllPath",
        "Runtime.Prg.Dispatch.Error.DeclareUnsupportedParameterType"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareMissingFunctionNameOrDllPath") ==
            "DECLARE: missing function name or DLL path.",
        "#2715: DECLARE missing-name/path error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareCannotLoadDll", load_placeholders) ==
            "DECLARE: cannot load 'kernel32.dll': Access is denied.",
        "#2715: DECLARE load-failure error should preserve path and downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareFunctionNotFoundInDll", function_placeholders) ==
            "DECLARE: function 'MissingSymbol' not found in 'kernel32.dll'.",
        "#2715: DECLARE function-not-found error should preserve function and path placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareDllOnlySupportedOnWindows") ==
            "DECLARE DLL is only supported on Windows.",
        "#2715: DECLARE Windows-only guard should localize through the runtime catalog");
    expect(
        english.translate(
            "Runtime.Prg.Dispatch.Error.DeclareUnsupportedParameterType",
            parameter_type_placeholders) ==
            "DECLARE: parameter type SHORT is not supported.",
        "#3938: help-invalid DECLARE parameter types should preserve the invariant type token");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2715: es-419 should define every DECLARE runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2715: pt-BR should define every DECLARE runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2715: qps-ploc should define every DECLARE runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.DeclareMissingFunctionNameOrDllPath")
                .find("missing function name or DLL path") == std::string::npos,
        "#2715: es-419 DECLARE missing-name/path error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.DeclareDllOnlySupportedOnWindows")
                .find("only supported on Windows") == std::string::npos,
        "#2715: pt-BR DECLARE Windows-only guard should not fall back to raw English");

    const std::string pseudo_load =
        pseudo.translate("Runtime.Prg.Dispatch.Error.DeclareCannotLoadDll", load_placeholders);
    expect(
        pseudo_load.find("[!! ") == 0U &&
            pseudo_load.find("kernel32.dll") != std::string::npos &&
            pseudo_load.find("Access is denied.") != std::string::npos &&
            pseudo_load.find("cannot load") == std::string::npos,
        "#2715: qps-ploc DECLARE load-failure error should pseudo-localize prose while preserving path and downstream error text");
    const std::string pseudo_parameter_type = pseudo.translate(
        "Runtime.Prg.Dispatch.Error.DeclareUnsupportedParameterType",
        parameter_type_placeholders);
    expect(
        pseudo_parameter_type.find("[!! ") == 0U &&
            pseudo_parameter_type.find("SHORT") != std::string::npos &&
            pseudo_parameter_type.find("parameter type") == std::string::npos,
        "#3938: qps-ploc DECLARE parameter rejection should pseudo-localize prose and preserve the type token");
}

void test_runtime_residual_command_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.TextRequiresToVariableInCurrentRuntimeSlice",
        "Runtime.Prg.Dispatch.Error.TryBlockMissingEndtry",
        "Runtime.Prg.Dispatch.Error.ReplaceRequiresFieldWithExpressionAssignment",
        "Runtime.Prg.Dispatch.Error.UpdateRequiresSetFieldExpressionAssignments",
        "Runtime.Prg.Dispatch.Error.InsertIntoRequiresValuesClause",
        "Runtime.Prg.Dispatch.Error.InsertIntoSelectQueryInvalid",
        "Runtime.Prg.Dispatch.Error.UnlockRecordTargetRecordNotFound",
        "Runtime.Prg.Dispatch.Error.SleepCancelled"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.TextRequiresToVariableInCurrentRuntimeSlice") ==
            "TEXT requires TO <variable> in the current runtime slice",
        "#2717: TEXT missing-target error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.TryBlockMissingEndtry") ==
            "TRY block is missing ENDTRY",
        "#2717: TRY missing-ENDTRY error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ReplaceRequiresFieldWithExpressionAssignment") ==
            "REPLACE requires at least one FIELD WITH expression assignment",
        "#2717: REPLACE assignment error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.UpdateRequiresSetFieldExpressionAssignments") ==
            "UPDATE requires SET field = expression assignments",
        "#2717: UPDATE assignment error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.InsertIntoRequiresValuesClause") ==
            "INSERT INTO requires a VALUES clause",
        "#2717: INSERT INTO VALUES-clause error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.InsertIntoSelectQueryInvalid") ==
            "INSERT INTO SELECT query could not be parsed or resolved",
        "#3853: INSERT INTO SELECT query errors should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.UnlockRecordTargetRecordNotFound") ==
            "UNLOCK RECORD target record not found",
        "#2717: UNLOCK RECORD target-record error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SleepCancelled") ==
            "SLEEP cancelled.",
        "#2717: SLEEP cancellation error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2717: es-419 should define every residual command runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2717: pt-BR should define every residual command runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2717: qps-ploc should define every residual command runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.TryBlockMissingEndtry").find("missing ENDTRY") ==
            std::string::npos,
        "#2717: es-419 TRY missing-ENDTRY error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.SleepCancelled").find("cancelled") ==
            std::string::npos,
        "#2717: pt-BR SLEEP cancellation error should not fall back to raw English");

    const std::string pseudo_text =
        pseudo.translate("Runtime.Prg.Dispatch.Error.TextRequiresToVariableInCurrentRuntimeSlice");
    expect(
        pseudo_text.find("[!! ") == 0U &&
            pseudo_text.find("TEXT requires TO <variable> in the current runtime slice") == std::string::npos &&
            pseudo_text.find("<") != std::string::npos,
        "#2717: qps-ploc TEXT missing-target error should pseudo-localize prose while preserving syntax markers");
}

void test_runtime_object_helper_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap ole_placeholders{
        {"targetIdentifier", "missingOle.SomeProperty"}
    };
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed",
        "Runtime.Prg.Dispatch.Error.OleObjectNotFoundForPropertyAssignment",
        "Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed") ==
            "Object target assignment failed",
        "#2718: object-target assignment failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.OleObjectNotFoundForPropertyAssignment", ole_placeholders) ==
            "OLE object not found for property assignment: missingOle.SomeProperty",
        "#2718: OLE property-assignment miss should preserve the failing target identifier");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject") ==
            "SCATTER NAME: unable to create object",
        "#2718: SCATTER NAME object-creation failure should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2718: es-419 should define every object-helper runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2718: pt-BR should define every object-helper runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2718: qps-ploc should define every object-helper runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed")
                .find("Object target assignment failed") == std::string::npos,
        "#2718: es-419 object-target assignment failure should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject")
                .find("unable to create object") == std::string::npos,
        "#2718: pt-BR SCATTER NAME object-creation failure should not fall back to raw English");

    const std::string pseudo_ole =
        pseudo.translate("Runtime.Prg.Dispatch.Error.OleObjectNotFoundForPropertyAssignment", ole_placeholders);
    expect(
        pseudo_ole.find("[!! ") == 0U &&
            pseudo_ole.find("missingOle.SomeProperty") != std::string::npos &&
            pseudo_ole.find("OLE object not found for property assignment") == std::string::npos,
        "#2718: qps-ploc OLE property-assignment miss should pseudo-localize prose while preserving the member path");
}

void test_runtime_ole_invocation_and_property_read_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap missing_method_placeholders{
        {"targetIdentifier", "missingOle.NoSuchMethod"}
    };
    const copperfin::localization::PlaceholderMap missing_member_method_placeholders{
        {"memberIdentifier", "Scripting.Dictionary.NoSuchMethod"}
    };
    const copperfin::localization::PlaceholderMap missing_property_placeholders{
        {"propertyPath", "missingOle.SomeProperty"}
    };
    const copperfin::localization::PlaceholderMap missing_member_property_placeholders{
        {"memberIdentifier", "Scripting.Dictionary.SomeProperty"}
    };
    const std::vector<std::string> keys{
        "Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation",
        "Runtime.Prg.Core.Error.OleMemberNotFoundForMethodInvocation",
        "Runtime.Prg.Core.Error.OleObjectNotFoundForPropertyRead",
        "Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead"
    };

    expect(
        english.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation", missing_method_placeholders) ==
            "OLE object not found for method invocation: missingOle.NoSuchMethod",
        "#2719: OLE object-missing method invocation fault should preserve the missing target identifier");
    expect(
        english.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForMethodInvocation", missing_member_method_placeholders) ==
            "OLE member not found for method invocation: Scripting.Dictionary.NoSuchMethod",
        "#2719: OLE member-missing method invocation fault should preserve the automation member identifier");
    expect(
        english.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForPropertyRead", missing_property_placeholders) ==
            "OLE object not found for property read: missingOle.SomeProperty",
        "#2719: OLE object-missing property-read fault should preserve the missing property path");
    expect(
        english.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead", missing_member_property_placeholders) ==
            "OLE member not found for property read: Scripting.Dictionary.SomeProperty",
        "#2719: OLE member-missing property-read fault should preserve the automation member identifier");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2719: es-419 should define every residual OLE invocation/read runtime key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2719: pt-BR should define every residual OLE invocation/read runtime key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2719: qps-ploc should define every residual OLE invocation/read runtime key");
    }

    expect(
        spanish.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation", missing_method_placeholders)
                .find("OLE object not found for method invocation") == std::string::npos,
        "#2719: es-419 OLE object-missing method invocation fault should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead", missing_member_property_placeholders)
                .find("OLE member not found for property read") == std::string::npos,
        "#2719: pt-BR OLE member-missing property-read fault should not fall back to raw English");

    const std::string pseudo_missing_method =
        pseudo.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation", missing_method_placeholders);
    expect(
        pseudo_missing_method.find("[!! ") == 0U &&
            pseudo_missing_method.find("missingOle.NoSuchMethod") != std::string::npos &&
            pseudo_missing_method.find("OLE object not found for method invocation") == std::string::npos,
        "#2719: qps-ploc OLE object-missing method invocation fault should pseudo-localize prose while preserving the target identifier");

    const std::string pseudo_missing_property =
        pseudo.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead", missing_member_property_placeholders);
    expect(
        pseudo_missing_property.find("[!! ") == 0U &&
            pseudo_missing_property.find("Scripting.Dictionary.SomeProperty") != std::string::npos &&
            pseudo_missing_property.find("OLE member not found for property read") == std::string::npos,
        "#2719: qps-ploc OLE member-missing property-read fault should pseudo-localize prose while preserving the member identifier");
}
