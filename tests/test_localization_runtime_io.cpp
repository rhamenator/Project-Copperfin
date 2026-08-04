// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_localization_support.h"

void test_runtime_save_restore_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.SaveToFilenameRequired",
        "Runtime.Prg.Dispatch.Error.SaveToOpenFailed",
        "Runtime.Prg.Dispatch.Error.SaveToWriteFailed",
        "Runtime.Prg.Dispatch.Error.RestoreFromFilenameRequired",
        "Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SaveToFilenameRequired") == "SAVE TO: filename required",
        "#2705: SAVE TO missing-filename error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SaveToOpenFailed") == "SAVE TO: unable to open output file",
        "#2705: SAVE TO open-failure error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SaveToWriteFailed") == "SAVE TO: unable to write output file",
        "#2705: SAVE TO write-failure error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RestoreFromFilenameRequired") ==
            "RESTORE FROM: filename required",
        "#2705: RESTORE FROM missing-filename error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed") ==
            "RESTORE FROM: unable to open source file",
        "#2705: RESTORE FROM open-failure error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2705: es-419 should define every SAVE/RESTORE runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2705: pt-BR should define every SAVE/RESTORE runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2705: qps-ploc should define every SAVE/RESTORE runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.SaveToFilenameRequired").find("filename required") ==
            std::string::npos,
        "#2705: es-419 SAVE TO missing-filename error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed").find("unable to open source file") ==
            std::string::npos,
        "#2705: pt-BR RESTORE FROM open-failure error should not fall back to raw English");

    expect(
        pseudo.translate("Runtime.Prg.Dispatch.Error.SaveToFilenameRequired") ==
            copperfin::localization::pseudo_localize("SAVE TO: filename required"),
        "#2705: qps-ploc SAVE TO missing-filename error should match the pseudo-localization transform");
    expect(
        pseudo.translate("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed") ==
            copperfin::localization::pseudo_localize("RESTORE FROM: unable to open source file"),
        "#2705: qps-ploc RESTORE FROM open-failure error should match the pseudo-localization transform");
}

void test_runtime_file_operation_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap erase_placeholders{
        {"errorMessage", "directory not empty"},
        {"path", "fixtures/nonempty"}
    };
    const copperfin::localization::PlaceholderMap io_placeholders{{"errorMessage", "No such file or directory"}};
    const copperfin::localization::PlaceholderMap exists_placeholders{{"path", "fixtures/existing.txt"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.EraseFailed",
        "Runtime.Prg.Dispatch.Error.CopyFileFailed",
        "Runtime.Prg.Dispatch.Error.RenameFileFailed",
        "Runtime.Prg.Dispatch.Error.RenameFileTargetExists"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.EraseFailed", erase_placeholders) ==
            "ERASE failed: directory not empty (fixtures/nonempty)",
        "#2706: ERASE failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyFileFailed", io_placeholders) ==
            "COPY FILE failed: No such file or directory",
        "#2706: COPY FILE failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RenameFileFailed", io_placeholders) ==
            "RENAME failed: No such file or directory",
        "#2706: RENAME failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RenameFileTargetExists", exists_placeholders) ==
            "RENAME failed: destination already exists (fixtures/existing.txt)",
        "#3703: existing-destination RENAME failure should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2706: es-419 should define every file-operation runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2706: pt-BR should define every file-operation runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2706: qps-ploc should define every file-operation runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.EraseFailed", erase_placeholders).find("failed:") ==
            std::string::npos,
        "#2706: es-419 ERASE failure should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.CopyFileFailed", io_placeholders).find("failed:") ==
            std::string::npos,
        "#2706: pt-BR COPY FILE failure should not fall back to raw English");

    const std::string pseudo_erase =
        pseudo.translate("Runtime.Prg.Dispatch.Error.EraseFailed", erase_placeholders);
    expect(
        pseudo_erase.find("[!! ") == 0U &&
            pseudo_erase.find("directory not empty") != std::string::npos &&
            pseudo_erase.find("fixtures/nonempty") != std::string::npos &&
            pseudo_erase.find("ERASE failed:") == std::string::npos,
        "#2706: qps-ploc ERASE failure should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_copy =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CopyFileFailed", io_placeholders);
    expect(
        pseudo_copy.find("[!! ") == 0U &&
            pseudo_copy.find("No such file or directory") != std::string::npos &&
            pseudo_copy.find("COPY FILE failed:") == std::string::npos,
        "#2706: qps-ploc COPY FILE failure should pseudo-localize prose while preserving the OS error text");
    const std::string pseudo_rename =
        pseudo.translate("Runtime.Prg.Dispatch.Error.RenameFileFailed", io_placeholders);
    expect(
        pseudo_rename.find("[!! ") == 0U &&
            pseudo_rename.find("No such file or directory") != std::string::npos &&
            pseudo_rename.find("RENAME failed:") == std::string::npos,
        "#2706: qps-ploc RENAME failure should pseudo-localize prose while preserving the OS error text");
    const std::string pseudo_rename_exists =
        pseudo.translate("Runtime.Prg.Dispatch.Error.RenameFileTargetExists", exists_placeholders);
    expect(
        pseudo_rename_exists.find("[!! ") == 0U &&
            pseudo_rename_exists.find("fixtures/existing.txt") != std::string::npos &&
            pseudo_rename_exists.find("destination already exists") == std::string::npos,
        "#3703: qps-ploc existing-destination RENAME failure should pseudo-localize prose while preserving the target path");
}

void test_runtime_copy_to_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap type_placeholders{{"type", "JSON"}};
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "table write failed"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.CopyToArrayNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.CopyToNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.CopyToNoFieldsMatchFieldsClause",
        "Runtime.Prg.Dispatch.Error.CopyToSourceCursorSchemaUnavailable",
        "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
        "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
        "Runtime.Prg.Dispatch.Error.CopyToWriteFailed"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToArrayNoCurrentWorkArea") ==
            "COPY TO ARRAY: no current work area",
        "#2707: COPY TO ARRAY precondition error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToNoCurrentWorkArea") ==
            "COPY TO: no current work area",
        "#2707: COPY TO precondition error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToSourceCursorSchemaUnavailable") ==
            "COPY TO: source cursor schema is unavailable",
        "#2707: COPY TO schema-unavailable error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToNoFieldsMatchFieldsClause") ==
            "COPY TO: no fields match the FIELDS clause",
        "#2707: COPY TO empty-fields error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed", type_placeholders) ==
            "COPY TO TYPE JSON: unable to open output file",
        "#2707: COPY TO TYPE open-failure error should preserve the type placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed", type_placeholders) ==
            "COPY TO TYPE JSON: unable to write output file",
        "#2707: COPY TO TYPE write-failure error should preserve the type placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToWriteFailed", error_placeholders) ==
            "COPY TO: table write failed",
        "#2707: COPY TO wrapper error should preserve downstream error text");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2707: es-419 should define every COPY TO runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2707: pt-BR should define every COPY TO runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2707: qps-ploc should define every COPY TO runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CopyToNoCurrentWorkArea").find("no current work area") ==
            std::string::npos,
        "#2707: es-419 COPY TO precondition error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed", type_placeholders)
                .find("unable to open output file") == std::string::npos,
        "#2707: pt-BR COPY TO TYPE open-failure error should not fall back to raw English");

    const std::string pseudo_type =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed", type_placeholders);
    expect(
        pseudo_type.find("[!! ") == 0U &&
            pseudo_type.find("JSON") != std::string::npos &&
            pseudo_type.find("unable to open output file") == std::string::npos,
        "#2707: qps-ploc COPY TO TYPE open-failure error should pseudo-localize prose while preserving the type");
    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CopyToWriteFailed", error_placeholders);
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("table write failed") != std::string::npos &&
            pseudo_wrapper.find("COPY TO:") == std::string::npos,
        "#2707: qps-ploc COPY TO wrapper error should pseudo-localize prose while preserving downstream error text");
}

void test_runtime_append_from_array_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "table header parse failed"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AppendFromArrayFailed",
        "Runtime.Prg.Dispatch.Error.AppendFromArrayNoFieldsMatchFieldsClause"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayFailed", error_placeholders) ==
            "APPEND FROM ARRAY: table header parse failed",
        "#2708: APPEND FROM ARRAY wrapper error should preserve downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayNoFieldsMatchFieldsClause") ==
            "APPEND FROM ARRAY: no fields match the FIELDS clause",
        "#2708: APPEND FROM ARRAY empty-fields error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2708: es-419 should define every APPEND FROM ARRAY runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2708: pt-BR should define every APPEND FROM ARRAY runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2708: qps-ploc should define every APPEND FROM ARRAY runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayNoFieldsMatchFieldsClause")
                .find("no fields match the FIELDS clause") == std::string::npos,
        "#2708: es-419 APPEND FROM ARRAY empty-fields error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayFailed", error_placeholders) ==
            "APPEND FROM ARRAY: table header parse failed",
        "#2708: pt-BR APPEND FROM ARRAY wrapper error should preserve invariant command text and downstream error text");

    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayFailed", error_placeholders);
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("table header parse failed") != std::string::npos &&
            pseudo_wrapper.find("APPEND FROM ARRAY:") == std::string::npos,
        "#2708: qps-ploc APPEND FROM ARRAY wrapper error should pseudo-localize prose while preserving downstream error text");
}

void test_runtime_append_from_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "open table failed"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AppendFromFailed",
        "Runtime.Prg.Dispatch.Error.AppendFromNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.AppendFromNoFieldsMatchFieldsClause",
        "Runtime.Prg.Dispatch.Error.AppendFromSelectedSqlResultCursorUnsupportedSourceType"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromFailed", error_placeholders) ==
            "APPEND FROM: open table failed",
        "#2709: APPEND FROM wrapper error should preserve downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromNoCurrentWorkArea") ==
            "APPEND FROM: no current work area",
        "#2709: APPEND FROM no-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromNoFieldsMatchFieldsClause") ==
            "APPEND FROM: no fields match the FIELDS clause",
        "#2709: APPEND FROM empty-fields error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromSelectedSqlResultCursorUnsupportedSourceType") ==
            "APPEND FROM: selected SQL/result cursor does not support this source type",
        "#2709: APPEND FROM SQL/result source-type error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2709: es-419 should define every shared APPEND FROM runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2709: pt-BR should define every shared APPEND FROM runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2709: qps-ploc should define every shared APPEND FROM runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.AppendFromNoCurrentWorkArea")
                .find("no current work area") == std::string::npos,
        "#2709: es-419 APPEND FROM no-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromSelectedSqlResultCursorUnsupportedSourceType")
                .find("selected SQL/result cursor does not support this source type") == std::string::npos,
        "#2709: pt-BR APPEND FROM SQL/result source-type error should not fall back to raw English");

    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromFailed", error_placeholders);
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("open table failed") != std::string::npos &&
            pseudo_wrapper.find("APPEND FROM:") == std::string::npos,
        "#2709: qps-ploc APPEND FROM wrapper error should pseudo-localize prose while preserving downstream error text");
}

void test_runtime_append_from_type_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap type_placeholders{{"type", "JSON"}};
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "numeric value too large"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
        "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
        "Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeFailed", {
            {"type", "JSON"},
            {"errorMessage", "numeric value too large"},
        }) == "APPEND FROM TYPE JSON: numeric value too large",
        "#2711: APPEND FROM TYPE wrapper error should preserve type and downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed", type_placeholders) ==
            "APPEND FROM TYPE JSON: unable to open source file",
        "#2710: APPEND FROM TYPE open-source error should preserve the type placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause", type_placeholders) ==
            "APPEND FROM TYPE JSON: no fields match the FIELDS clause",
        "#2710: APPEND FROM TYPE empty-fields error should preserve the type placeholder");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2710: es-419 should define every APPEND FROM TYPE runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2710: pt-BR should define every APPEND FROM TYPE runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2710: qps-ploc should define every APPEND FROM TYPE runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed", type_placeholders)
                .find("unable to open source file") == std::string::npos,
        "#2710: es-419 APPEND FROM TYPE open-source error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause", type_placeholders)
                .find("no fields match the FIELDS clause") == std::string::npos,
        "#2710: pt-BR APPEND FROM TYPE empty-fields error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeFailed", {
            {"type", "JSON"},
            {"errorMessage", "numeric value too large"},
        }) == "APPEND FROM TYPE JSON: numeric value too large",
        "#2711: pt-BR APPEND FROM TYPE wrapper error should preserve invariant type and downstream error text");

    const std::string pseudo_open =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed", type_placeholders);
    expect(
        pseudo_open.find("[!! ") == 0U &&
            pseudo_open.find("JSON") != std::string::npos &&
            pseudo_open.find("unable to open source file") == std::string::npos,
        "#2710: qps-ploc APPEND FROM TYPE open-source error should pseudo-localize prose while preserving the type");
    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeFailed", {
            {"type", "JSON"},
            {"errorMessage", "numeric value too large"},
        });
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("JSON") != std::string::npos &&
            pseudo_wrapper.find("numeric value too large") != std::string::npos &&
            pseudo_wrapper.find("APPEND FROM TYPE JSON:") == std::string::npos,
        "#2711: qps-ploc APPEND FROM TYPE wrapper error should pseudo-localize prose while preserving type and downstream error text");
}

void test_runtime_scatter_gather_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound",
        "Runtime.Prg.Dispatch.Error.GatherNoCurrentRecord",
        "Runtime.Prg.Dispatch.Error.GatherNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.ScatterNoCurrentRecord",
        "Runtime.Prg.Dispatch.Error.ScatterNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.ScatterNoFieldsMatchFieldsClause"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNoCurrentWorkArea") ==
            "SCATTER: no current work area",
        "#2712: SCATTER no-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNoCurrentRecord") ==
            "SCATTER: no current record",
        "#2712: SCATTER no-current-record error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNoFieldsMatchFieldsClause") ==
            "SCATTER: no fields match the FIELDS clause",
        "#2712: SCATTER empty-fields error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.GatherNoCurrentWorkArea") ==
            "GATHER: no current work area",
        "#2712: GATHER no-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.GatherNoCurrentRecord") ==
            "GATHER: no current record",
        "#2712: GATHER no-current-record error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound") ==
            "GATHER NAME: object variable not found",
        "#2712: GATHER NAME missing-object error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2712: es-419 should define every SCATTER/GATHER runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2712: pt-BR should define every SCATTER/GATHER runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2712: qps-ploc should define every SCATTER/GATHER runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.ScatterNoCurrentWorkArea")
                .find("no current work area") == std::string::npos,
        "#2712: es-419 SCATTER no-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound")
                .find("object variable not found") == std::string::npos,
        "#2712: pt-BR GATHER NAME missing-object error should not fall back to raw English");

    const std::string pseudo_scatter =
        pseudo.translate("Runtime.Prg.Dispatch.Error.ScatterNoFieldsMatchFieldsClause");
    expect(
        pseudo_scatter.find("[!! ") == 0U &&
            pseudo_scatter.find("no fields match the FIELDS clause") == std::string::npos,
        "#2712: qps-ploc SCATTER empty-fields error should pseudo-localize prose");
    const std::string pseudo_gather =
        pseudo.translate("Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound");
    expect(
        pseudo_gather.find("[!! ") == 0U &&
            pseudo_gather.find("object variable not found") == std::string::npos,
        "#2712: qps-ploc GATHER NAME missing-object error should pseudo-localize prose");
}

void test_runtime_dispatch_array_and_object_target_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap copy_placeholders{{"command", "COPY TO ARRAY"}};
    const copperfin::localization::PlaceholderMap scatter_placeholders{{"command", "SCATTER NAME"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.ArrayNameRequired",
        "Runtime.Prg.Dispatch.Error.InvalidArrayName",
        "Runtime.Prg.Dispatch.Error.ObjectTargetRequired",
        "Runtime.Prg.Dispatch.Error.InvalidObjectTarget"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ArrayNameRequired", copy_placeholders) ==
            "COPY TO ARRAY: array name required",
        "#2722: array-name-required helper error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.InvalidArrayName", copy_placeholders) ==
            "COPY TO ARRAY: invalid array name",
        "#2722: invalid-array-name helper error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ObjectTargetRequired", scatter_placeholders) ==
            "SCATTER NAME: object target required",
        "#2722: object-target-required helper error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.InvalidObjectTarget", scatter_placeholders) ==
            "SCATTER NAME: invalid object target",
        "#2722: invalid-object-target helper error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2722: es-419 should define every shared array/object-target helper runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2722: pt-BR should define every shared array/object-target helper runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2722: qps-ploc should define every shared array/object-target helper runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.ArrayNameRequired", copy_placeholders)
                .find("array name required") == std::string::npos,
        "#2722: es-419 array-name-required helper error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.InvalidObjectTarget", scatter_placeholders)
                .find("invalid object target") == std::string::npos,
        "#2722: pt-BR invalid-object-target helper error should not fall back to raw English");

    const std::string pseudo_object_target =
        pseudo.translate("Runtime.Prg.Dispatch.Error.ObjectTargetRequired", scatter_placeholders);
    expect(
        pseudo_object_target.find("[!! ") == 0U &&
            pseudo_object_target.find("SCATTER NAME") != std::string::npos &&
            pseudo_object_target.find("object target required") == std::string::npos,
        "#2722: qps-ploc object-target-required helper error should pseudo-localize prose while preserving the command token");
}

void test_runtime_table_structure_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap alter_add_placeholders{{"command", "ALTER TABLE ADD COLUMN"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
        "Runtime.Prg.Dispatch.Error.AlterTableRequiresTargetTableName",
        "Runtime.Prg.Dispatch.Error.AlterTableSupportsAddDropAlterColumnOnly",
        "Runtime.Prg.Dispatch.Error.CreateCursorRequiresNonEmptyAlias",
        "Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration",
        "Runtime.Prg.Dispatch.Error.CreateTableRequiresSupportedFieldDeclaration",
        "Runtime.Prg.Dispatch.Error.CreateTableRequiresTargetTableName"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration") ==
            "CREATE CURSOR requires at least one supported field declaration",
        "#2713: CREATE CURSOR field-declaration error should localize through the runtime catalog");
    expect(
        english.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            alter_add_placeholders) ==
            "ALTER TABLE ADD COLUMN requires a supported field declaration",
        "#2723: ALTER TABLE ADD COLUMN field-declaration error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CreateTableRequiresTargetTableName") ==
            "CREATE TABLE requires a target table name",
        "#2713: CREATE TABLE target-name error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AlterTableSupportsAddDropAlterColumnOnly") ==
            "ALTER TABLE currently supports ADD COLUMN, DROP COLUMN, and ALTER COLUMN only",
        "#2713: ALTER TABLE action-support error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2713: es-419 should define every table-structure runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2713: pt-BR should define every table-structure runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2713: qps-ploc should define every table-structure runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration")
                .find("requires at least one supported field declaration") == std::string::npos,
        "#2713: es-419 CREATE CURSOR field-declaration error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AlterTableSupportsAddDropAlterColumnOnly")
                .find("currently supports ADD COLUMN, DROP COLUMN, and ALTER COLUMN only") == std::string::npos,
        "#2713: pt-BR ALTER TABLE action-support error should not fall back to raw English");
    expect(
        spanish.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            alter_add_placeholders)
                .find("requires a supported field declaration") == std::string::npos,
        "#2723: es-419 ALTER TABLE field-declaration error should not fall back to raw English");

    const std::string pseudo_create_table =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CreateTableRequiresSupportedFieldDeclaration");
    expect(
        pseudo_create_table.find("[!! ") == 0U &&
            pseudo_create_table.find("requires at least one supported field declaration") == std::string::npos,
        "#2713: qps-ploc CREATE TABLE field-declaration error should pseudo-localize prose");
    const std::string pseudo_alter_add =
        pseudo.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            alter_add_placeholders);
    expect(
        pseudo_alter_add.find("[!! ") == 0U &&
            pseudo_alter_add.find("ALTER TABLE ADD COLUMN") != std::string::npos &&
            pseudo_alter_add.find("requires a supported field declaration") == std::string::npos,
        "#2723: qps-ploc ALTER TABLE field-declaration error should pseudo-localize prose while preserving command tokens");
}
