// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/xasset_methods.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
}

void expect_substring_order(
    const std::string& source,
    const std::vector<std::string>& markers,
    const std::string& message_prefix) {
    std::size_t cursor = 0;
    for (const auto& marker : markers) {
        const auto found = source.find(marker, cursor);
        if (found == std::string::npos) {
            expect(false, message_prefix + ": missing marker '" + marker + "'");
            return;
        }
        cursor = found + marker.size();
    }
}

bool has_action_id(
    const std::vector<copperfin::runtime::XAssetActionBinding>& actions,
    const std::string& expected_action_id) {
    return std::any_of(actions.begin(), actions.end(), [&](const auto& action) {
        return action.action_id == expected_action_id;
    });
}

void test_xasset_executable_model_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys{
        "Runtime.XAsset.Error.TablePreviewMissing",
        "Runtime.XAsset.Error.UnsupportedExecutableFamily"};

    expect(
        english_catalog.translate("Runtime.XAsset.Error.TablePreviewMissing") ==
            "Asset does not have a table preview.",
        "#2392: missing table-preview xAsset diagnostic should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Runtime.XAsset.Error.UnsupportedExecutableFamily") ==
            "Asset family is not a supported executable xAsset.",
        "#2392: unsupported executable xAsset diagnostic should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Runtime.XAsset.Error.TablePreviewMissing") ==
            "El asset no tiene una vista previa de tabla.",
        "#2604: xAsset missing-preview diagnostics should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Runtime.XAsset.Error.UnsupportedExecutableFamily") ==
            "A familia do asset nao e um xAsset executavel suportado.",
        "#2604: xAsset unsupported-family diagnostics should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Runtime.XAsset.Error.TablePreviewMissing") !=
            english_catalog.translate("Runtime.XAsset.Error.TablePreviewMissing"),
        "#2392: missing table-preview xAsset diagnostic should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Runtime.XAsset.Error.TablePreviewMissing") ==
            copperfin::localization::pseudo_localize("Asset does not have a table preview."),
        "#2604: xAsset qps-ploc diagnostics should route through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2604: es-419 should define every remaining Runtime.XAsset localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2604: pt-BR should define every remaining Runtime.XAsset localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2604: qps-ploc should define every remaining Runtime.XAsset localization key");

    copperfin::studio::StudioDocumentModel missing_preview;
    missing_preview.kind = copperfin::studio::StudioAssetKind::form;
    missing_preview.table_preview_available = false;
    const auto missing_preview_model = copperfin::runtime::build_xasset_executable_model(missing_preview);
    expect(!missing_preview_model.ok, "#2392: xAsset model without table preview should fail");
    expect(
        missing_preview_model.error == "Asset does not have a table preview.",
        "#2392: xAsset model should preserve default localized missing table-preview diagnostic");

    copperfin::studio::StudioDocumentModel unsupported_family;
    unsupported_family.kind = copperfin::studio::StudioAssetKind::project;
    unsupported_family.table_preview_available = true;
    const auto unsupported_family_model = copperfin::runtime::build_xasset_executable_model(unsupported_family);
    expect(!unsupported_family_model.ok, "#2392: unsupported executable xAsset family should fail");
    expect(
        unsupported_family_model.error == "Asset family is not a supported executable xAsset.",
        "#2392: xAsset model should preserve default localized unsupported-family diagnostic");

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_missing_preview_model =
        copperfin::runtime::build_xasset_executable_model(missing_preview);
    expect(
        !spanish_missing_preview_model.ok &&
            spanish_missing_preview_model.error == "El asset no tiene una vista previa de tabla.",
        "#2604: xAsset runtime diagnostics should refresh to es-419 when the runtime locale changes in-process");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_unsupported_family_model =
        copperfin::runtime::build_xasset_executable_model(unsupported_family);
    expect(
        !portuguese_unsupported_family_model.ok &&
            portuguese_unsupported_family_model.error ==
                "A familia do asset nao e um xAsset executavel suportado.",
        "#2604: xAsset runtime diagnostics should refresh to pt-BR when the runtime locale changes in-process");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_missing_preview_model =
        copperfin::runtime::build_xasset_executable_model(missing_preview);
    expect(
        !pseudo_missing_preview_model.ok &&
            pseudo_missing_preview_model.error ==
                copperfin::localization::pseudo_localize("Asset does not have a table preview."),
        "#2604: xAsset runtime diagnostics should refresh to qps-ploc when the runtime locale changes in-process");
}

const copperfin::runtime::XAssetMethod* find_method(
    const std::vector<copperfin::runtime::XAssetMethod>& methods,
    const std::string& routine_name) {
    const auto found = std::find_if(methods.begin(), methods.end(), [&](const auto& method) {
        return method.routine_name == routine_name;
    });
    return found == methods.end() ? nullptr : &*found;
}

copperfin::vfp::DbfRecord make_record(
    std::size_t record_index,
    std::initializer_list<copperfin::vfp::DbfRecordValue> values,
    bool deleted = false) {
    copperfin::vfp::DbfRecord record;
    record.record_index = record_index;
    record.deleted = deleted;
    record.values.assign(values.begin(), values.end());
    return record;
}

void test_build_xasset_executable_model() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\demo.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "Dataenvironment"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "dataenvironment"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE BeforeOpenTables\r\nSET DELETED ON\r\nENDPROC\r\nPROCEDURE OpenTables\r\nx = 1\r\nENDPROC\r\nPROCEDURE CloseTables\r\nCLEAR EVENTS\r\nENDPROC", .memo_block_number = 30U}
        }),
        make_record(1, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "frmDemo"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "form"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Load\r\nx = 2\r\nENDPROC\r\nPROCEDURE Init\r\nSET DELETED OFF\r\nENDPROC\r\nPROCEDURE Activate\r\nx = 3\r\nENDPROC\r\nPROCEDURE Destroy\r\nx = 4\r\nENDPROC", .memo_block_number = 40U}
        }),
        make_record(2, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "pgfMain"},
            {.field_name = "PARENT", .field_type = 'M', .display_value = "frmDemo"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "pageframe"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Page2.Activate\r\nTHISFORM.Refresh\r\nENDPROC", .memo_block_number = 50U}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "xAsset executable model should be created for forms");
    expect(model.runnable_startup, "form model should be runnable when startup methods exist");
    expect(model.root_object_path == "frmDemo", "root object path should identify the root form");
    expect(model.methods.size() == 8U, "all form/data-environment methods should be extracted");
    expect(model.actions.size() == 8U, "form model should expose all extracted methods as runtime actions");
    if (model.methods.size() == 8U) {
        expect(model.methods[0].source_field_index == 3U,
               "#706: data-environment methods should preserve the source METHODS field ordinal");
        expect(model.methods[0].source_memo_block_number == 30U,
               "#719: data-environment methods should preserve the source METHODS memo block");
        expect(model.methods[0].source_line_index == 0U,
               "#707: first data-environment method should preserve its declaration line index");
        expect(model.methods[1].source_line_index == 3U,
               "#707: later data-environment methods should preserve declaration line indexes inside METHODS");
        expect(model.methods[3].source_field_index == 3U,
               "#706: root form methods should preserve the source METHODS field ordinal");
        expect(model.methods[3].source_memo_block_number == 40U,
               "#719: root form methods should preserve the source METHODS memo block");
        expect(model.methods[3].source_line_index == 0U,
               "#707: root form method line provenance should start at its declaration line");
        expect(model.methods[4].source_line_index == 3U,
               "#707: later root form methods should preserve declaration line indexes inside METHODS");
        expect(model.methods[7].source_field_index == 4U,
               "#706: nested object methods should preserve the source METHODS field ordinal");
        expect(model.methods[7].source_memo_block_number == 50U,
               "#719: nested object methods should preserve the source METHODS memo block");
        expect(model.methods[7].source_line_index == 0U,
               "#707: nested object methods should preserve their METHODS declaration line index");
    }
    if (model.actions.size() >= 8U) {
        expect(model.actions[0].action_id == "dataenvironment.beforeopentables", "data environment startup should be dispatchable");
        expect(model.actions[0].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#708: synthesized form action titles should use the missing-title-field sentinel");
        expect(model.actions[0].routine_source_field_index == 3U,
               "#708: form action routines should inherit method source field provenance");
        expect(model.actions[0].routine_source_line_index == 0U,
               "#708: form action routines should inherit method source line provenance");
        expect(model.actions[0].title_memo_block_number == 0U,
               "#720: synthesized form action titles should expose memo block zero");
        expect(model.actions[0].routine_source_memo_block_number == 30U,
               "#720: form action routines should inherit method memo block provenance");
        expect(model.actions[4].action_id == "frmdemo.init", "root form init should be dispatchable");
        expect(model.actions[4].routine_source_field_index == 3U,
               "#708: root form action routines should inherit method source field provenance");
        expect(model.actions[4].routine_source_line_index == 3U,
               "#708: root form action routines should inherit method declaration line provenance");
        expect(model.actions[4].routine_source_memo_block_number == 40U,
               "#720: root form action routines should inherit method memo block provenance");
        expect(model.actions[7].action_id == "frmdemo.pgfmain.page2.activate", "nested page methods should be dispatchable");
    }
    expect(model.startup_routines.size() == 5U, "startup should include data environment, load, init, and activate methods");
    expect(model.shutdown_routines.size() == 2U, "shutdown should include form and data-environment cleanup methods");
    expect(model.startup_lines.size() == 5U, "form startup lines should mirror startup routine count");
    expect(model.shutdown_lines.size() == 2U, "form shutdown lines should mirror shutdown routine count");
    expect(model.startup_steps.size() == 5U, "#709: form startup steps should mirror startup routine count");
    expect(model.shutdown_steps.size() == 2U, "#709: form shutdown steps should mirror shutdown routine count");
    if (model.startup_routines.size() == 5U) {
        expect(model.startup_routines[0] == "__cf_Dataenvironment_BeforeOpenTables",
               "form startup should begin with data-environment BeforeOpenTables");
        expect(model.startup_routines[1] == "__cf_Dataenvironment_OpenTables",
               "form startup should run data-environment OpenTables before form methods");
        expect(model.startup_routines[2] == "__cf_frmDemo_Load",
               "form startup should run Load before Init");
        expect(model.startup_routines[3] == "__cf_frmDemo_Init",
               "form startup should run Init before Activate");
        expect(model.startup_routines[4] == "__cf_frmDemo_Activate",
               "form startup should end with Activate");
    }
    if (model.startup_lines.size() == 5U) {
        expect(model.startup_lines[0] == "DO __cf_Dataenvironment_BeforeOpenTables",
               "form startup lines should match runtime-startup order");
        expect(model.startup_lines[1] == "DO __cf_Dataenvironment_OpenTables",
               "form startup lines should match OpenTables ordering");
        expect(model.startup_lines[2] == "DO __cf_frmDemo_Load",
               "form startup lines should match Load ordering");
        expect(model.startup_lines[3] == "DO __cf_frmDemo_Init",
               "form startup lines should match Init ordering");
        expect(model.startup_lines[4] == "DO __cf_frmDemo_Activate",
               "form startup lines should match Activate ordering");
    }
    if (model.startup_steps.size() == 5U) {
        expect(model.startup_steps[0].kind == "method", "#709: method-backed startup steps should be tagged as methods");
        expect(model.startup_steps[0].command_text == "DO __cf_Dataenvironment_BeforeOpenTables",
               "#709: startup step command text should mirror the legacy startup line");
        expect(model.startup_steps[0].routine_name == "__cf_Dataenvironment_BeforeOpenTables",
               "#709: startup steps should carry routine identity");
        expect(model.startup_steps[0].record_index == 0U, "#709: startup steps should retain source record provenance");
        expect(model.startup_steps[0].source_field_index == 3U, "#709: startup steps should inherit method source field provenance");
        expect(model.startup_steps[0].source_line_index == 0U, "#709: startup steps should inherit method source line provenance");
        expect(model.startup_steps[0].source_memo_block_number == 30U,
               "#721: startup steps should inherit method memo block provenance");
        expect(model.startup_steps[3].routine_name == "__cf_frmDemo_Init",
               "#709: later startup steps should preserve lifecycle ordering");
        expect(model.startup_steps[3].record_index == 1U, "#709: later startup steps should retain source record provenance");
        expect(model.startup_steps[3].source_field_index == 3U, "#709: later startup steps should inherit source field provenance");
        expect(model.startup_steps[3].source_line_index == 3U, "#709: later startup steps should inherit source line provenance");
        expect(model.startup_steps[3].source_memo_block_number == 40U,
               "#721: later startup steps should inherit method memo block provenance");
    }
    if (model.shutdown_routines.size() == 2U) {
        expect(model.shutdown_routines[0] == "__cf_frmDemo_Destroy",
               "form shutdown should call Destroy before data-environment cleanup");
        expect(model.shutdown_routines[1] == "__cf_Dataenvironment_CloseTables",
               "form shutdown should end with data-environment CloseTables");
    }
    if (model.shutdown_lines.size() == 2U) {
        expect(model.shutdown_lines[0] == "DO __cf_frmDemo_Destroy",
               "form shutdown lines should match runtime-shutdown order");
        expect(model.shutdown_lines[1] == "DO __cf_Dataenvironment_CloseTables",
               "form shutdown lines should match close-tables ordering");
    }
    if (model.shutdown_steps.size() == 2U) {
        expect(model.shutdown_steps[0].command_text == "DO __cf_frmDemo_Destroy",
               "#709: shutdown step command text should mirror the legacy shutdown line");
        expect(model.shutdown_steps[0].record_index == 1U, "#709: shutdown steps should retain source record provenance");
        expect(model.shutdown_steps[0].source_field_index == 3U, "#709: shutdown steps should inherit method source field provenance");
        expect(model.shutdown_steps[0].source_line_index == 9U, "#709: shutdown steps should inherit method source line provenance");
        expect(model.shutdown_steps[0].source_memo_block_number == 40U,
               "#721: shutdown steps should inherit method memo block provenance");
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect_substring_order(
        bootstrap,
        {
            "DO __cf_Dataenvironment_BeforeOpenTables",
            "DO __cf_Dataenvironment_OpenTables",
            "DO __cf_frmDemo_Load",
            "DO __cf_frmDemo_Init",
            "DO __cf_frmDemo_Activate"
        },
        "form bootstrap should execute startup lifecycle methods in sequence");
    expect(bootstrap.find("PROCEDURE __cf_frmDemo_pgfMain_Page2_Activate") != std::string::npos, "bootstrap should materialize nested object methods");
    expect_substring_order(
        bootstrap,
        {
            "DO __cf_frmDemo_Activate",
            "READ EVENTS",
            "DO __cf_frmDemo_Destroy",
            "DO __cf_Dataenvironment_CloseTables"
        },
        "form bootstrap should sequence active-loop and shutdown");
    expect(bootstrap.find("READ EVENTS") != std::string::npos, "bootstrap should optionally include READ EVENTS");
    expect(bootstrap.find("DO __cf_frmDemo_Destroy") != std::string::npos, "form bootstrap should call the form destroy method after event-loop exit");
    expect(bootstrap.find("DO __cf_Dataenvironment_CloseTables") != std::string::npos, "form bootstrap should call the data-environment close method after event-loop exit");
}

void test_xasset_routine_names_are_collision_resistant() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\collision.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "foo-bar"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "custom"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Init\r\nRETURN 1\r\nENDPROC"}
        }),
        make_record(1, {
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "foo_bar"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "custom"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Init\r\nRETURN 2\r\nENDPROC"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "colliding xAsset method fixture should build successfully");
    expect(model.methods.size() == 2U, "colliding xAsset method fixture should preserve both methods");
    if (model.methods.size() == 2U) {
        expect(model.methods[0].object_path == "foo-bar" && model.methods[1].object_path == "foo_bar",
               "collision-resistant naming should preserve original object identities");
        expect(model.methods[0].method_name == "Init" && model.methods[1].method_name == "Init",
               "collision-resistant naming should preserve original method identities");
        expect(model.methods[0].routine_name != model.methods[1].routine_name,
               "sanitized xAsset routine names should be unique");
        expect(model.methods[0].routine_name == "__cf_foo_bar_Init",
               "the first sanitized xAsset routine should preserve its established name");
        expect(model.methods[1].routine_name == "__cf_foo_bar_Init_2",
               "a colliding sanitized xAsset routine should receive a deterministic suffix");

        const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, false);
        expect(bootstrap.find("PROCEDURE " + model.methods[0].routine_name) != std::string::npos,
               "bootstrap should emit the first collision-resistant procedure");
        expect(bootstrap.find("PROCEDURE " + model.methods[1].routine_name) != std::string::npos,
               "bootstrap should emit the second collision-resistant procedure");
    }
    expect(model.actions.size() == 2U, "colliding xAsset methods should remain independently dispatchable");
    for (const auto& action : model.actions) {
        if (action.action_id == "foo-bar.init") {
            expect(action.routine_name == "__cf_foo_bar_Init",
                   "the first xAsset action should retain the established routine name");
        } else if (action.action_id == "foo_bar.init") {
            expect(action.routine_name == "__cf_foo_bar_Init_2",
                   "the colliding xAsset action should follow its suffixed routine");
        } else {
            expect(false, "collision fixture should expose the original object/method action identity");
        }
    }
}

void test_build_class_library_xasset_executable_model() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\demo.vcx)";
    document.kind = copperfin::studio::StudioAssetKind::class_library;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "custWidget"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "custom"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Load\r\nx = 1\r\nENDPROC\r\nPROCEDURE Init\r\nx = 2\r\nENDPROC\r\nPROCEDURE Destroy\r\nx = 3\r\nENDPROC"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "xAsset executable model should be created for class libraries");
    expect(model.runnable_startup, "class-library model should be runnable when startup methods exist");
    expect(model.root_object_path == "custWidget", "class-library root object path should identify the root class");
    expect(model.startup_routines.size() == 2U, "class-library startup should include load and init");
    expect(model.shutdown_routines.size() == 1U, "class-library shutdown should include destroy");
    expect(model.startup_lines.size() == 2U, "class-library startup lines should match startup routine count");
    expect(model.shutdown_lines.size() == 1U, "class-library shutdown lines should match shutdown routine count");
    if (model.startup_routines.size() == 2U) {
        expect(model.startup_routines[0] == "__cf_custWidget_Load",
               "class-library startup should run Load before Init");
        expect(model.startup_routines[1] == "__cf_custWidget_Init",
               "class-library startup should end with Init");
    }
    if (model.startup_lines.size() == 2U) {
        expect(model.startup_lines[0] == "DO __cf_custWidget_Load",
               "class-library startup lines should mirror startup routine order");
        expect(model.startup_lines[1] == "DO __cf_custWidget_Init",
               "class-library startup lines should end with Init");
    }
    if (model.shutdown_routines.size() == 1U) {
        expect(model.shutdown_routines[0] == "__cf_custWidget_Destroy",
               "class-library shutdown should dispatch Destroy");
    }
    if (model.shutdown_lines.size() == 1U) {
        expect(model.shutdown_lines[0] == "DO __cf_custWidget_Destroy",
               "class-library shutdown lines should dispatch Destroy");
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect_substring_order(
        bootstrap,
        {
            "DO __cf_custWidget_Load",
            "DO __cf_custWidget_Init",
            "DO __cf_custWidget_Destroy"
        },
        "class-library bootstrap should execute lifecycle calls in order");
    expect(bootstrap.find("DO __cf_custWidget_Load") != std::string::npos, "class-library bootstrap should call the load method");
    expect(bootstrap.find("DO __cf_custWidget_Init") != std::string::npos, "class-library bootstrap should call the init method");
    expect(bootstrap.find("DO __cf_custWidget_Destroy") != std::string::npos, "class-library bootstrap should call the destroy method after event-loop exit");
    expect(bootstrap.find("READ EVENTS") != std::string::npos, "class-library bootstrap should enter read events");
    expect(bootstrap.find("READ EVENTS") > bootstrap.find("DO __cf_custWidget_Init"), "class-library bootstrap should enter event loop after startup");
    expect(bootstrap.find("DO __cf_custWidget_Destroy") > bootstrap.find("READ EVENTS"), "class-library destroy should run after event loop");
}

void test_form_root_object_path_ignores_comments_and_data_environment() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\root_select.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "COMMENT"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "CommentRow"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "custom"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Init\r\nx = 0\r\nENDPROC"}
        }),
        make_record(1, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "Dataenvironment"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "dataenvironment"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE OpenTables\r\nx = 1\r\nENDPROC"}
        }),
        make_record(2, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "frmPrimary"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "form"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Load\r\nx = 2\r\nENDPROC\r\nPROCEDURE Init\r\nx = 3\r\nENDPROC"}
        }),
        make_record(3, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "txtName"},
            {.field_name = "PARENT", .field_type = 'M', .display_value = "frmPrimary"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "textbox"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Valid\r\nx = 4\r\nENDPROC"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "form executable model should still build when comment/dataenvironment records appear first");
    expect(model.root_object_path == "frmPrimary",
           "root form selection should ignore comment/data-environment records and choose top-level form object");
    expect(model.actions.size() >= 4U,
           "form executable model should include actions for data-environment, root form, and nested child object methods");
    expect(model.startup_routines.size() == 3U,
           "data-environment and root form methods should both participate in form startup");
    expect(model.startup_lines.size() == 3U, "startup lines should expose the startup event-order seam");
    if (model.startup_lines.size() == 3U) {
        expect(model.startup_lines[0] == "DO __cf_Dataenvironment_OpenTables",
               "data-environment OpenTables should start form startup");
        expect(model.startup_lines[1] == "DO __cf_frmPrimary_Load",
               "root form Load should follow data-environment startup");
        expect(model.startup_lines[2] == "DO __cf_frmPrimary_Init",
               "root form Init should follow Load");
    }
    expect(model.shutdown_lines.empty(), "no form/class shutdown methods should be generated for this asset");

    expect(has_action_id(model.actions, "dataenvironment.opentables"), "data-environment methods should remain dispatchable");
    expect(has_action_id(model.actions, "frmprimary.load"), "root form load should remain dispatchable");
    expect(has_action_id(model.actions, "frmprimary.init"), "root form init should remain dispatchable");
    expect(has_action_id(model.actions, "frmprimary.txtname.valid"), "nested object methods should retain full object-graph action ids");

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect(bootstrap.find("DO __cf_Dataenvironment_OpenTables") != std::string::npos, "bootstrap should dispatch data-environment OpenTables");
    expect(bootstrap.find("DO __cf_frmPrimary_Load") != std::string::npos, "bootstrap should dispatch root form Load");
    expect(bootstrap.find("DO __cf_frmPrimary_Init") != std::string::npos, "bootstrap should dispatch root form Init");
    expect_substring_order(
        bootstrap,
        {
            "DO __cf_Dataenvironment_OpenTables",
            "DO __cf_frmPrimary_Load",
            "DO __cf_frmPrimary_Init",
            "READ EVENTS"
        },
        "bootstrap should sequence data-environment startup into form startup");
}

void test_xasset_executable_model_suppresses_unresolved_memo_placeholders() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\placeholder.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        make_record(0, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "<memo block 50>"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "form"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "<memo block 51>"}
        }),
        make_record(1, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "frmLive"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "form"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Load\r\nx = 1\r\nENDPROC"}
        })
    };

    const auto form_model = copperfin::runtime::build_xasset_executable_model(form_document);
    expect(form_model.ok, "#697: form xAsset model should still build with unresolved memo placeholders");
    expect(form_model.root_object_path == "frmLive", "#697: unresolved memo object names should not become root object paths");
    expect(form_model.methods.size() == 1U, "#697: unresolved METHODS memo placeholders should not materialize methods");
    if (!form_model.methods.empty()) {
        expect(form_model.methods[0].object_path == "frmLive", "#697: live method owner should remain intact");
        expect(form_model.methods[0].source_text.find("<memo block") == std::string::npos,
               "#697: unresolved memo placeholders should not become method source text");
    }

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\placeholder.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "4"},
            {.field_name = "SETUP", .field_type = 'M', .display_value = "<memo block 52>"},
            {.field_name = "CLEANUP", .field_type = 'M', .display_value = "<memo block 53>"}
        }),
        make_record(1, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "2"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "Shortcut"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Shortcut"}
        }),
        make_record(2, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "3"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Shortcut"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  1"},
            {.field_name = "PROMPT", .field_type = 'M', .display_value = "<memo block 54>"},
            {.field_name = "COMMAND", .field_type = 'M', .display_value = "<memo block 55>"}
        }),
        make_record(3, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "2"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "<memo block 56>"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "SubTarget"}
        })
    };

    const auto menu_model = copperfin::runtime::build_xasset_executable_model(menu_document);
    expect(menu_model.ok, "#697: menu xAsset model should still build with unresolved memo placeholders");
    expect(menu_model.methods.size() == 1U, "#697: unresolved setup/cleanup/command placeholders should not materialize wrapped methods");
    expect(menu_model.actions.size() == 1U, "#697: submenu fallback should still create one action");
    if (!menu_model.actions.empty()) {
        expect(menu_model.actions[0].action_id == "shortcut.item1", "#697: action id should use deterministic owner fallback");
        expect(menu_model.actions[0].title == "Shortcut.item1", "#697: unresolved PROMPT should fall back to the owner path");
        expect(menu_model.actions[0].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#708: unresolved prompt fallback titles should use the missing-title-field sentinel");
        expect(menu_model.actions[0].routine_source_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#708: synthetic submenu action routines should use missing routine field provenance");
        expect(menu_model.actions[0].routine_source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
               "#708: synthetic submenu action routines should use missing routine line provenance");
        expect(menu_model.actions[0].kind == "submenu", "#697: unresolved command should allow submenu fallback");
    }
    if (!menu_model.methods.empty()) {
        expect(menu_model.methods[0].source_text == "ACTIVATE POPUP SubTarget",
               "#697: submenu activation should use the following usable submenu level name");
        expect(menu_model.methods[0].source_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#706: synthetic submenu activation methods should use the missing-field sentinel");
        expect(menu_model.methods[0].source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
               "#707: synthetic submenu activation methods should use the missing-line sentinel");
    }
    const std::string menu_bootstrap = copperfin::runtime::build_xasset_bootstrap_source(menu_model, true);
    expect(menu_bootstrap.find("<memo block") == std::string::npos,
           "#697: unresolved memo placeholders should not appear in generated xAsset bootstrap source");
}

void test_xasset_executable_model_skips_deleted_records() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\deleted.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        make_record(0, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "oldForm"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "form"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Init\r\nx = 0\r\nENDPROC"}
        }, true),
        make_record(1, {
            {.field_name = "PLATFORM", .field_type = 'C', .display_value = "WINDOWS"},
            {.field_name = "OBJNAME", .field_type = 'M', .display_value = "frmLive"},
            {.field_name = "BASECLASS", .field_type = 'M', .display_value = "form"},
            {.field_name = "METHODS", .field_type = 'M', .display_value = "PROCEDURE Load\r\nx = 1\r\nENDPROC"}
        })
    };

    const auto form_model = copperfin::runtime::build_xasset_executable_model(form_document);
    expect(form_model.ok, "#699: form xAsset model should still build with deleted records present");
    expect(form_model.root_object_path == "frmLive", "#699: deleted form records should not become root object paths");
    expect(form_model.methods.size() == 1U, "#699: deleted form records should not materialize methods");
    expect(!has_action_id(form_model.actions, "oldform.init"), "#699: deleted form methods should not become actions");
    expect(has_action_id(form_model.actions, "frmlive.load"), "#699: live form methods should remain actions");

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\deletedmenu.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "4"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "DeletedShortcut"}
        }, true),
        make_record(1, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "3"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Deleted"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  1"},
            {.field_name = "COMMAND", .field_type = 'M', .display_value = "CLEAR EVENTS"}
        }, true),
        make_record(2, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "1"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "LiveMenu"}
        })
    };

    const auto menu_model = copperfin::runtime::build_xasset_executable_model(menu_document);
    expect(menu_model.ok, "#699: menu xAsset model should still build with deleted records present");
    expect(menu_model.activation_kind == "menu", "#699: deleted shortcut records should not force popup activation");
    expect(menu_model.activation_target == "deletedmenu", "#699: live non-shortcut menu activation should use the path stem");
    expect(menu_model.actions.empty(), "#699: deleted menu item records should not become actions");
    expect(menu_model.methods.empty(), "#699: deleted menu command records should not materialize methods");
}

void test_build_menu_xasset_executable_model() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\shortcut.mnx)";
    document.kind = copperfin::studio::StudioAssetKind::menu;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "4"},
            {.field_name = "SETUP", .field_type = 'M', .display_value = "PARAMETERS Param1\nParam1 = 'ready'", .memo_block_number = 61U},
            {.field_name = "CLEANUP", .field_type = 'M', .display_value = "CLEAR EVENTS", .memo_block_number = 62U},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  0"}
        }),
        make_record(1, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "2"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "Shortcut"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Shortcut"},
            {.field_name = "NUMITEMS", .field_type = 'N', .display_value = "2"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  0"}
        }),
        make_record(2, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "3"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Shortcut"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  1"},
            {.field_name = "COMMAND", .field_type = 'M', .display_value = "CLEAR EVENTS", .memo_block_number = 63U}
        }),
        make_record(3, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "3"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Shortcut"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  2"},
            {.field_name = "PROCEDURE", .field_type = 'M', .display_value = "* metadata header\r\nPROCEDURE ItemAction\r\nCLEAR EVENTS\r\nENDPROC", .memo_block_number = 64U}
        }),
        make_record(4, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "3"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Shortcut"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  3"},
            {.field_name = "PROMPT", .field_type = 'M', .display_value = "More", .memo_block_number = 65U}
        }),
        make_record(5, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "2"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "Thisitemha"},
            {.field_name = "LEVELNAME", .field_type = 'C', .display_value = "Thisitemha"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "  0"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "xAsset executable model should be created for menus");
    expect(model.runnable_startup, "menu model should be runnable when activation can be derived");
    expect(model.activation_kind == "popup", "shortcut menus should activate as popups");
    expect(model.activation_target == "Shortcut", "shortcut menus should target the first popup/submenu name");
    expect(model.startup_enters_event_loop, "menu startup should enter the runtime event loop");
    expect(model.startup_lines.size() >= 2U, "menu startup should include setup and activation lines");
    expect(model.methods.size() >= 5U, "menu methods should include wrapped setup/command/procedure/cleanup code");
    expect(model.actions.size() >= 3U, "menu model should expose runnable menu actions");
    expect(model.shutdown_routines.size() == 1U, "menu model should expose cleanup routines for post-event-loop shutdown");
    expect(model.shutdown_lines.size() == 1U, "menu model should emit exactly one cleanup shutdown line");
    expect(model.startup_steps.size() == 2U, "#709: menu startup steps should include setup plus synthetic activation");
    expect(model.shutdown_steps.size() == 1U, "#709: menu shutdown steps should include cleanup provenance");
    const auto* setup_method = find_method(model.methods, "__cf_shortcut_setup");
    const auto* cleanup_method = find_method(model.methods, "__cf_shortcut_cleanup");
    const auto* command_method = find_method(model.methods, "__cf_Shortcut_item1_command");
    const auto* procedure_method = find_method(model.methods, "__cf_Shortcut_item2_ItemAction");
    const auto* submenu_method = find_method(model.methods, "__cf_Shortcut_item3_activate_popup");
    expect(setup_method != nullptr, "#706: menu setup method should be discoverable for provenance assertions");
    if (setup_method != nullptr) {
        expect(setup_method->source_field_index == 1U, "#706: wrapped SETUP methods should retain SETUP field provenance");
        expect(setup_method->source_line_index == 0U, "#707: wrapped SETUP methods should start at source line zero");
        expect(setup_method->source_memo_block_number == 61U, "#719: wrapped SETUP methods should retain SETUP memo block provenance");
    }
    expect(cleanup_method != nullptr, "#706: menu cleanup method should be discoverable for provenance assertions");
    if (cleanup_method != nullptr) {
        expect(cleanup_method->source_field_index == 2U, "#706: wrapped CLEANUP methods should retain CLEANUP field provenance");
        expect(cleanup_method->source_line_index == 0U, "#707: wrapped CLEANUP methods should start at source line zero");
        expect(cleanup_method->source_memo_block_number == 62U, "#719: wrapped CLEANUP methods should retain CLEANUP memo block provenance");
    }
    expect(command_method != nullptr, "#706: menu command method should be discoverable for provenance assertions");
    if (command_method != nullptr) {
        expect(command_method->source_field_index == 3U, "#706: wrapped COMMAND methods should retain COMMAND field provenance");
        expect(command_method->source_line_index == 0U, "#707: wrapped COMMAND methods should start at source line zero");
        expect(command_method->source_memo_block_number == 63U, "#719: wrapped COMMAND methods should retain COMMAND memo block provenance");
    }
    expect(procedure_method != nullptr, "#706: menu procedure method should be discoverable for provenance assertions");
    if (procedure_method != nullptr) {
        expect(procedure_method->source_field_index == 3U, "#706: embedded PROCEDURE methods should retain PROCEDURE field provenance");
        expect(procedure_method->source_line_index == 1U,
               "#707: embedded PROCEDURE methods should retain declaration line indexes inside the memo field");
        expect(procedure_method->source_memo_block_number == 64U, "#719: embedded PROCEDURE methods should retain PROCEDURE memo block provenance");
    }
    expect(submenu_method != nullptr, "#706: synthetic submenu method should be discoverable for provenance assertions");
    if (submenu_method != nullptr) {
        expect(submenu_method->source_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#706: synthetic submenu methods should use the missing-field sentinel");
        expect(submenu_method->source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
               "#707: synthetic submenu methods should use the missing-line sentinel");
        expect(submenu_method->source_memo_block_number == 0U, "#719: synthetic submenu methods should expose memo block zero");
    }
    if (model.actions.size() >= 3U) {
        expect(model.actions[0].action_id == "shortcut.item1", "first action should expose the item1 action id");
        expect(model.actions[0].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#708: command action title fallbacks should use the missing-title-field sentinel");
        expect(model.actions[0].routine_source_field_index == 3U,
               "#708: command actions should inherit COMMAND field provenance");
        expect(model.actions[0].routine_source_line_index == 0U,
               "#708: command actions should inherit wrapped COMMAND line provenance");
        expect(model.actions[0].title_memo_block_number == 0U,
               "#720: fallback command action titles should expose memo block zero");
        expect(model.actions[0].routine_source_memo_block_number == 63U,
               "#720: command actions should inherit COMMAND memo block provenance");
        expect(model.actions[1].action_id == "shortcut.item2", "second action should expose the item2 action id");
        expect(model.actions[1].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#708: procedure action title fallbacks should use the missing-title-field sentinel");
        expect(model.actions[1].routine_source_field_index == 3U,
               "#708: procedure actions should inherit PROCEDURE field provenance");
        expect(model.actions[1].routine_source_line_index == 1U,
               "#708: procedure actions should inherit embedded declaration line provenance");
        expect(model.actions[1].routine_source_memo_block_number == 64U,
               "#720: procedure actions should inherit PROCEDURE memo block provenance");
        expect(model.actions[2].action_id == "shortcut.item3", "third action should expose the submenu action id");
        expect(model.actions[2].title_field_index == 3U,
               "#708: submenu action titles sourced from PROMPT should retain PROMPT field provenance");
        expect(model.actions[2].title_memo_block_number == 65U,
               "#720: submenu action titles should retain PROMPT memo block provenance");
        expect(model.actions[2].routine_source_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#708: synthetic submenu action routines should use missing routine field provenance");
        expect(model.actions[2].routine_source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
               "#708: synthetic submenu action routines should use missing routine line provenance");
        expect(model.actions[2].routine_source_memo_block_number == 0U,
               "#720: synthetic submenu action routines should expose memo block zero");
        expect(model.actions[2].kind == "submenu", "submenu item should be tagged as a submenu action");
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect(bootstrap.find("DO __cf_shortcut_setup") != std::string::npos, "menu bootstrap should call setup code");
    expect(bootstrap.find("ACTIVATE POPUP Shortcut") != std::string::npos, "menu bootstrap should activate the popup");
    expect(bootstrap.find("PROCEDURE __cf_Shortcut_item1_command") != std::string::npos, "menu bootstrap should materialize command routines");
    expect(bootstrap.find("PROCEDURE __cf_Shortcut_item2_ItemAction") != std::string::npos, "menu bootstrap should materialize embedded snippet procedures");
    expect(bootstrap.find("PROCEDURE __cf_Shortcut_item3_activate_popup") != std::string::npos, "menu bootstrap should materialize submenu activation routines");
    expect(bootstrap.find("DO __cf_shortcut_cleanup") != std::string::npos, "menu bootstrap should execute cleanup after event-loop exit");
    expect(bootstrap.find("READ EVENTS") == std::string::npos, "menu bootstrap should not append READ EVENTS when activation already enters the event loop");
    expect(model.startup_lines.size() == 2U, "menu startup should emit setup then activation lines");
    if (model.startup_lines.size() == 2U) {
        expect(model.startup_lines[0] == "DO __cf_shortcut_setup",
               "menu startup should run setup before activation");
        expect(model.startup_lines[1] == "ACTIVATE POPUP Shortcut",
               "menu startup should activate the popup after setup");
    }
    if (model.startup_steps.size() == 2U) {
        expect(model.startup_steps[0].kind == "method", "#709: setup startup step should be method-backed");
        expect(model.startup_steps[0].command_text == "DO __cf_shortcut_setup",
               "#709: setup startup step should mirror the legacy startup line");
        expect(model.startup_steps[0].source_field_index == 1U,
               "#709: setup startup step should retain SETUP field provenance");
        expect(model.startup_steps[0].source_line_index == 0U,
               "#709: setup startup step should retain wrapped source line provenance");
        expect(model.startup_steps[0].source_memo_block_number == 61U,
               "#721: setup startup step should inherit SETUP memo block provenance");
        expect(model.startup_steps[1].kind == "activation", "#709: menu activation should be tagged as synthetic activation");
        expect(model.startup_steps[1].command_text == "ACTIVATE POPUP Shortcut",
               "#709: activation step command text should mirror the legacy startup line");
        expect(model.startup_steps[1].source_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#709: synthetic activation should use the missing source-field sentinel");
        expect(model.startup_steps[1].source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
               "#709: synthetic activation should use the missing source-line sentinel");
        expect(model.startup_steps[1].source_memo_block_number == 0U,
               "#721: synthetic activation should expose memo block zero");
        expect(model.startup_steps[1].routine_name.empty(), "#709: synthetic activation should not masquerade as a routine");
    }
    if (model.actions.size() >= 2U) {
        expect(model.actions[0].routine_name == "__cf_Shortcut_item1_command",
               "menu action binding should dispatch item1 command routine");
        expect(model.actions[1].routine_name == "__cf_Shortcut_item2_ItemAction",
               "menu action binding should dispatch item2 procedure routine");
    }
    if (model.shutdown_routines.size() == 1U) {
        expect(model.shutdown_routines[0] == "__cf_shortcut_cleanup",
               "menu shutdown routine should dispatch wrapped cleanup routine");
    }
    if (model.shutdown_lines.size() == 1U) {
        expect(model.shutdown_lines[0] == "DO __cf_shortcut_cleanup",
               "menu shutdown should execute cleanup after event-loop exit");
    }
    if (model.shutdown_steps.size() == 1U) {
        expect(model.shutdown_steps[0].command_text == "DO __cf_shortcut_cleanup",
               "#709: cleanup shutdown step should mirror the legacy shutdown line");
        expect(model.shutdown_steps[0].source_field_index == 2U,
               "#709: cleanup shutdown step should retain CLEANUP field provenance");
        expect(model.shutdown_steps[0].source_line_index == 0U,
               "#709: cleanup shutdown step should retain wrapped source line provenance");
        expect(model.shutdown_steps[0].source_memo_block_number == 62U,
               "#721: cleanup shutdown step should inherit CLEANUP memo block provenance");
    }
}

void test_build_menu_xasset_activation_uses_vfp_path_stem() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\mainmenu.mnx)";
    document.kind = copperfin::studio::StudioAssetKind::menu;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "1"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "MainMenu"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "#698: non-shortcut menu xAsset model should build from Windows-style paths");
    expect(model.activation_kind == "menu", "#698: non-shortcut menus should activate as menus");
    expect(model.activation_target == "mainmenu", "#698: activation target should use the VFP path filename stem");
    expect(model.startup_lines.size() == 1U, "#698: non-shortcut menu startup should contain one activation line");
    if (!model.startup_lines.empty()) {
        expect(model.startup_lines[0] == "ACTIVATE MENU mainmenu",
               "#698: generated activation line should not include Windows directory text");
    }
    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect(bootstrap.find("DEFINE MENU mainmenu") != std::string::npos,
           "#4753: generated non-shortcut menu bootstrap should define its menu before activation");
}

void test_build_menu_xasset_rejects_partial_numeric_object_types() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\strict_numeric_types.mnx)";
    document.kind = copperfin::studio::StudioAssetKind::menu;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "4abc"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "TrailingShortcut"}
        }),
        make_record(1, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "4.000"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "GroupedShortcut"}
        }),
        make_record(2, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "3abc"},
            {.field_name = "LEVELNAME", .field_type = 'M', .display_value = "TrailingShortcut"},
            {.field_name = "ITEMNUM", .field_type = 'C', .display_value = "1"},
            {.field_name = "PROMPT", .field_type = 'M', .display_value = "Invalid item"},
            {.field_name = "COMMAND", .field_type = 'M', .display_value = "x = 1"}
        }),
        make_record(3, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "999999999999999999999999"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "OverflowType"}
        }),
        make_record(4, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "1"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "ValidMenu"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "#4858: menu model should tolerate invalid numeric object-type metadata");
    expect(model.activation_kind == "menu",
           "#4858: trailing/grouped shortcut types must not alias valid OBJTYPE 4");
    expect(model.activation_target == "strict_numeric_types",
           "#4858: invalid shortcut types should retain the ordinary menu path-stem target");
    expect(model.actions.empty(),
           "#4858: trailing menu-item types must not alias valid OBJTYPE 3 actions");
}

void test_build_report_xasset_executable_model() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\invoice.frx)";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "1"},
            {.field_name = "EXPR", .field_type = 'M', .display_value = "ENVIRONMENT = 1"}
        }),
        make_record(1, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "9"},
            {.field_name = "OBJCODE", .field_type = 'N', .display_value = "4"},
            {.field_name = "TOP", .field_type = 'N', .display_value = "0"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "xAsset executable model should be created for reports");
    expect(model.runnable_startup, "report model should be runnable without embedded methods");
    expect(model.startup_enters_event_loop, "report preview startup should enter the event loop");
    expect(model.methods.empty(), "report executable model should not synthesize object methods for preview-only startup");
    expect(model.actions.empty(), "report executable model should not emit method actions when no methods exist");
    expect(model.shutdown_lines.empty(), "report executable model should not emit shutdown lines for preview-only startup");
    expect(model.startup_lines.size() == 1U, "report startup should be a direct preview command");
    expect(model.startup_lines[0].find("REPORT FORM") != std::string::npos, "report startup should preview the report");
    expect(model.startup_steps.size() == 1U, "#710: report startup should expose one structured preview lifecycle step");
    if (!model.startup_steps.empty()) {
        expect(model.startup_steps[0].kind == "preview", "#710: report lifecycle step should be tagged as preview");
        expect(model.startup_steps[0].command_text == model.startup_lines[0],
               "#710: report lifecycle step command should mirror the legacy startup line");
        expect(model.startup_steps[0].routine_name.empty(), "#710: report preview lifecycle step should not masquerade as a routine");
        expect(model.startup_steps[0].source_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#710: report preview lifecycle step should use the missing source-field sentinel");
        expect(model.startup_steps[0].source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
               "#710: report preview lifecycle step should use the missing source-line sentinel");
        expect(model.startup_steps[0].source_memo_block_number == 0U,
               "#721: report preview lifecycle should expose memo block zero");
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect(bootstrap.find("REPORT FORM 'E:\\Project-Copperfin\\samples\\invoice.frx' PREVIEW") != std::string::npos, "bootstrap should preview the report asset directly");
    expect(bootstrap.find("READ EVENTS") == std::string::npos, "report preview bootstrap should not append a second event loop");
    const std::string snapshot_bootstrap = copperfin::runtime::build_xasset_bootstrap_source(
        model,
        true,
        "/private/snapshot/invoice.frx");
    expect(snapshot_bootstrap.find("REPORT FORM '/private/snapshot/invoice.frx' PREVIEW") != std::string::npos,
           "verified report bootstraps should execute the immutable snapshot path");
    expect(snapshot_bootstrap.find("E:\\Project-Copperfin\\samples\\invoice.frx") == std::string::npos,
           "verified report bootstraps should not reopen the logical package path");
    expect(model.asset_path == document.path && model.startup_lines[0].find(document.path) != std::string::npos,
           "snapshot execution overrides should preserve the report model's logical source identity");
}

void test_build_label_xasset_executable_model() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\mailing.lbx)";
    document.kind = copperfin::studio::StudioAssetKind::label;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "OBJTYPE", .field_type = 'N', .display_value = "1"},
            {.field_name = "EXPR", .field_type = 'M', .display_value = "ENVIRONMENT = 1"}
        })
    };

    const auto model = copperfin::runtime::build_xasset_executable_model(document);
    expect(model.ok, "xAsset executable model should be created for labels");
    expect(model.runnable_startup, "label model should be runnable without embedded methods");
    expect(model.startup_enters_event_loop, "label preview startup should enter the event loop");
    expect(model.methods.empty(), "label executable model should not synthesize object methods for preview-only startup");
    expect(model.actions.empty(), "label executable model should not emit method actions when no methods exist");
    expect(model.shutdown_lines.empty(), "label executable model should not emit shutdown lines for preview-only startup");
    expect(model.startup_lines.size() == 1U, "label startup should be a direct preview command");
    expect(model.startup_lines[0].find("LABEL FORM") != std::string::npos, "label startup should preview the label");
    expect(model.startup_steps.size() == 1U, "#710: label startup should expose one structured preview lifecycle step");
    if (!model.startup_steps.empty()) {
        expect(model.startup_steps[0].kind == "preview", "#710: label lifecycle step should be tagged as preview");
        expect(model.startup_steps[0].command_text == model.startup_lines[0],
               "#710: label lifecycle step command should mirror the legacy startup line");
        expect(model.startup_steps[0].routine_name.empty(), "#710: label preview lifecycle step should not masquerade as a routine");
        expect(model.startup_steps[0].source_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
               "#710: label preview lifecycle step should use the missing source-field sentinel");
        expect(model.startup_steps[0].source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
               "#710: label preview lifecycle step should use the missing source-line sentinel");
        expect(model.startup_steps[0].source_memo_block_number == 0U,
               "#721: label preview lifecycle should expose memo block zero");
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    expect(bootstrap.find("LABEL FORM 'E:\\Project-Copperfin\\samples\\mailing.lbx' PREVIEW") != std::string::npos, "bootstrap should preview the label asset directly");
    expect(bootstrap.find("READ EVENTS") == std::string::npos, "label preview bootstrap should not append a second event loop");
}

void test_build_real_menu_xasset_executable_model() {
    namespace fs = std::filesystem;
    const fs::path menu_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Menus\frmshort.mnx)";
    if (!fs::exists(menu_path)) {
        return;
    }

    const auto open_result = copperfin::studio::open_document({
        .path = menu_path.string(),
        .read_only = true,
        .load_full_table = true
    });
    expect(open_result.ok, "real sample MNX should open successfully");
    if (!open_result.ok) {
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "xAsset executable model should be created for real sample menus");
    expect(model.runnable_startup, "real sample menu should expose runnable startup");
    expect(model.startup_enters_event_loop, "real sample menu startup should enter the event loop");
    expect(!model.activation_kind.empty(), "real sample menu should derive an activation kind");
    expect(!model.activation_target.empty(), "real sample menu should derive an activation target");
    expect(!model.actions.empty(), "real sample menu should expose actionable menu items");
}

}  // namespace

int main() {
    test_xasset_executable_model_errors_resolve_through_localization_catalog();
    test_build_xasset_executable_model();
    test_xasset_routine_names_are_collision_resistant();
    test_build_class_library_xasset_executable_model();
    test_form_root_object_path_ignores_comments_and_data_environment();
    test_xasset_executable_model_suppresses_unresolved_memo_placeholders();
    test_xasset_executable_model_skips_deleted_records();
    test_build_menu_xasset_executable_model();
    test_build_menu_xasset_activation_uses_vfp_path_stem();
    test_build_menu_xasset_rejects_partial_numeric_object_types();
    test_build_report_xasset_executable_model();
    test_build_label_xasset_executable_model();
    test_build_real_menu_xasset_executable_model();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
