#include "copperfin/studio/document_model.h"
#include "copperfin/studio/vs_launch_contract.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

template <typename Descriptor>
bool has_descriptor_id(const std::vector<Descriptor>& descriptors, std::string_view id) {
    for (const auto& descriptor : descriptors) {
        if (descriptor.id == id) {
            return true;
        }
    }
    return false;
}

std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x01U;
    bytes[29] = 0x03U;
    return bytes;
}

void write_synthetic_form_table_with_data_environment(const std::filesystem::path& path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"Dataenvironment", "dataenvironment", ""},
        {"frmCustomer", "form", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(path.string(), fields, records);
    expect(create_result.ok, "#966: synthetic SCX table with DataEnvironment record should be created");
}

void test_parse_launch_arguments() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--from-vs",
        "--read-only",
        "--json",
        "--set-property",
        "--record", "3",
        "--property-name", "Left",
        "--property-value", "25",
        "--line", "25",
        "--column", "7",
        "--symbol", "cmdSave.Click",
        "--selection-context", "visual_method",
        "--selection-context", "report_expression",
        "--undo-mode", "command",
        "--undo-label", "Bulk Undo"
    });

    expect(result.ok, "launch contract should parse a complete Visual Studio launch request");
    expect(result.request.path == "E:\\Forms\\customer.scx", "launch path should be captured");
    expect(result.request.launched_from_visual_studio, "launch contract should detect --from-vs");
    expect(result.request.read_only, "launch contract should detect --read-only");
    expect(result.output_json, "launch contract should detect --json");
    expect(result.request.apply_property_update, "launch contract should detect --set-property");
    expect(result.request.record_index == 3U, "launch contract should parse the record index");
    expect(result.request.property_name == "Left", "launch contract should capture the property name");
    expect(result.request.property_value == "25", "launch contract should capture the property value");
    expect(result.request.line == 25U, "launch contract should parse the line value");
    expect(result.request.column == 7U, "launch contract should parse the column value");
    expect(result.request.symbol == "cmdSave.Click", "launch contract should parse the symbol");
    expect(result.request.designer_selection_contexts.size() == 2U,
           "#962: launch contract should collect explicit selection-context tokens");
    if (result.request.designer_selection_contexts.size() == 2U) {
        expect(result.request.designer_selection_contexts[0] == copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#962: launch contract should parse visual_method selection-context tokens");
        expect(result.request.designer_selection_contexts[1] == copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#962: launch contract should parse report_expression selection-context tokens");
    }
    expect(result.request.undo_mode == copperfin::studio::StudioUndoMode::command, "launch contract should parse the undo mode");
    expect(result.request.undo_label == "Bulk Undo", "launch contract should parse the undo label");
}

void test_parse_launch_arguments_rejects_unknown_switch() {
    const auto result = copperfin::studio::parse_launch_arguments({"--mystery"});
    expect(!result.ok, "launch contract should reject unknown switches");
}

void test_parse_launch_arguments_rejects_unknown_undo_mode() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--undo-mode", "mystery"
    });
    expect(!result.ok, "launch contract should reject unknown undo modes");
}

void test_parse_launch_arguments_rejects_unknown_selection_context() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selection-context", "mystery"
    });
    expect(!result.ok, "#962: launch contract should reject unknown selection-context tokens");
}

void test_parse_launch_arguments_rejects_missing_selection_context() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selection-context"
    });
    expect(!result.ok, "#962: launch contract should reject missing selection-context values");
}

void test_open_document_infers_form_sidecar() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_tests";
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const fs::path sidecar_path = temp_dir / "customer.sct";

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .symbol = "form1",
        .line = 10U,
        .column = 2U,
        .launched_from_visual_studio = true,
        .read_only = false
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a valid synthetic SCX file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::form, "SCX should map to a form document");
    expect(result.document.display_name == "customer.scx", "document display name should use the file name");
    expect(result.document.has_sidecar, "open_document should detect the SCT sidecar");
    expect(result.document.sidecar_path == sidecar_path.string(), "open_document should infer the SCT sidecar path");
    expect(result.document.launched_from_visual_studio, "launch metadata should flow into the Studio document");
    expect(result.document.selection_symbol == "form1", "#964: launch selection symbol should flow into the Studio document");
    expect(result.document.selection_line == 10U, "#964: launch selection line should flow into the Studio document");
    expect(result.document.selection_column == 2U, "#964: launch selection column should flow into the Studio document");
    expect(result.document.selection_record_index == 0U,
           "#964: launch selection record index should keep the default when none is supplied");
    expect(result.document.inspection.header_available, "inspection metadata should be attached to the document");

    const auto objects = copperfin::studio::build_object_snapshot(result.document);
    expect(objects.empty(), "header-only synthetic SCX should not produce object snapshots without parsed records");

    std::error_code ignored;
    fs::remove(form_path, ignored);
    fs::remove(sidecar_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_open_document_uses_vfp_filename_for_display_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_vfp_filename_tests";
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / R"(E:\Forms\customer.scx)";
    const fs::path sidecar_path = temp_dir / R"(E:\Forms\customer.sct)";

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const auto result = copperfin::studio::open_document({
        .path = form_path.string(),
        .launched_from_visual_studio = true
    });

    expect(result.ok, "#702: open_document should accept synthetic Windows-style VFP path names on the host filesystem");
    expect(result.document.display_name == "customer.scx",
           "#702: Studio display names should use VFP-aware filename parsing for backslash paths");
    expect(result.document.has_sidecar, "#702: sidecar inference should remain compatible with VFP-style path text");
    expect(result.document.sidecar_path == sidecar_path.string(),
           "#702: inferred sidecar path should still replace the extension in the host path");

    std::error_code ignored;
    fs::remove(form_path, ignored);
    fs::remove(sidecar_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_open_document_attaches_default_designer_contexts() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_designer_context_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto write_synthetic_asset = [&](const std::string& filename) {
        const fs::path path = temp_dir / filename;
        const auto bytes = make_vfp_header();
        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        return path;
    };

    const auto form_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("customer.scx").string()
    });
    expect(form_result.ok, "#960: synthetic form should open for designer-context checks");
    expect(form_result.document.designer_contexts.size() == 1U,
           "#960: form documents should expose one default designer context");
    if (!form_result.document.designer_contexts.empty()) {
        const auto& context = form_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object,
               "#960: form documents should expose the visual-object designer context");
        expect(has_descriptor_id(context.editor_actions, "show-property-grid"),
               "#960: form designer context should include property-grid actions");
        expect(has_descriptor_id(context.builders, "control-builder"),
               "#960: form designer context should include control builders");
        expect(has_descriptor_id(context.toolbox_items, "textbox"),
               "#960: form designer context should include form toolbox items");
    }

    const auto method_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .symbol = "cmdSave.Click"
    });
    expect(method_symbol_result.ok, "#963: synthetic form should open for method-symbol context checks");
    expect(method_symbol_result.document.designer_contexts.size() == 1U,
           "#963: method-symbol form documents should expose one inferred designer context");
    if (!method_symbol_result.document.designer_contexts.empty()) {
        const auto& context = method_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#963: method-like symbols should infer the visual-method designer context for forms");
        expect(has_descriptor_id(context.editor_actions, "edit-visual-method"),
               "#963: inferred visual-method contexts should include method-editor actions");
    }

    const auto data_environment_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .symbol = "Dataenvironment.OpenTables"
    });
    expect(data_environment_symbol_result.ok, "#965: synthetic form should open for data-environment symbol checks");
    expect(data_environment_symbol_result.document.designer_contexts.size() == 1U,
           "#965: data-environment symbols should expose one inferred designer context");
    if (!data_environment_symbol_result.document.designer_contexts.empty()) {
        const auto& context = data_environment_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#965: DataEnvironment method symbols should infer the data-environment designer context for forms");
        expect(has_descriptor_id(context.editor_actions, "edit-data-environment"),
               "#965: inferred data-environment contexts should include data-environment editor actions");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#965: inferred data-environment contexts should include data-environment builders");
    }

    const fs::path selected_record_path = temp_dir / "selected_record.scx";
    write_synthetic_form_table_with_data_environment(selected_record_path);
    const auto data_environment_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 0U
    });
    expect(data_environment_record_result.ok, "#966: synthetic form should open for selected-record context checks");
    expect(data_environment_record_result.document.designer_contexts.size() == 1U,
           "#966: selected DataEnvironment records should expose one inferred designer context");
    if (!data_environment_record_result.document.designer_contexts.empty()) {
        const auto& context = data_environment_record_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#966: selected DataEnvironment records should infer the data-environment designer context");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#966: selected DataEnvironment records should include data-environment builders");
    }

    const auto visual_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 1U
    });
    expect(visual_record_result.ok, "#966: synthetic form should open for visual selected-record context checks");
    expect(visual_record_result.document.designer_contexts.size() == 1U,
           "#966: visual selected records should preserve the generic form default context count");
    if (!visual_record_result.document.designer_contexts.empty()) {
        expect(visual_record_result.document.designer_contexts.front().selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::visual_object,
               "#966: non-DataEnvironment selected records should preserve visual-object defaults");
    }

    const auto multi_override_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::visual_method,
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(multi_override_result.ok, "#962: synthetic form should open for explicit designer-context checks");
    expect(multi_override_result.document.designer_contexts.size() == 2U,
           "#962: explicit selection contexts should override the form default context list");
    if (multi_override_result.document.designer_contexts.size() == 2U) {
        expect(multi_override_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#962: explicit visual_method contexts should be preserved in request order");
        expect(has_descriptor_id(multi_override_result.document.designer_contexts[0].editor_actions, "edit-visual-method"),
               "#962: explicit visual_method contexts should include method-editor actions");
        expect(multi_override_result.document.designer_contexts[1].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#962: explicit report_expression contexts should be preserved in request order");
        expect(has_descriptor_id(multi_override_result.document.designer_contexts[1].editor_actions, "edit-report-expression"),
               "#962: explicit report_expression contexts should include expression-editor actions");
    }

    const auto override_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 0U,
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(override_result.ok, "#962: synthetic form should open for explicit designer-context checks");
    expect(override_result.document.designer_contexts.size() == 1U,
           "#966: explicit selection contexts should override selected-record context defaults");
    if (override_result.document.designer_contexts.size() == 1U) {
        expect(override_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#966: explicit report_expression contexts should win over selected-record data-environment contexts");
        expect(has_descriptor_id(override_result.document.designer_contexts[0].editor_actions, "edit-report-expression"),
               "#962: explicit report_expression contexts should include expression-editor actions");
    }

    const auto report_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("summary.frx").string()
    });
    expect(report_result.ok, "#960: synthetic report should open for designer-context checks");
    expect(report_result.document.designer_contexts.size() == 1U,
           "#960: report documents should expose one default designer context");
    if (!report_result.document.designer_contexts.empty()) {
        const auto& context = report_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#960: report documents should expose the report-expression designer context");
        expect(has_descriptor_id(context.editor_actions, "edit-report-expression"),
               "#960: report designer context should include expression editor actions");
        expect(has_descriptor_id(context.builders, "report-builder"),
               "#960: report designer context should include report builders");
        expect(has_descriptor_id(context.toolbox_items, "label"),
               "#960: report designer context should include report-safe toolbox items");
        expect(!has_descriptor_id(context.toolbox_items, "textbox"),
               "#960: report designer context should exclude form-only toolbox items");
    }

    const auto project_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("demo.pjx").string()
    });
    expect(project_result.ok, "#960: synthetic project should open for designer-context checks");
    expect(project_result.document.designer_contexts.size() == 1U,
           "#960: project documents should expose one default designer context");
    if (!project_result.document.designer_contexts.empty()) {
        const auto& context = project_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::project_item,
               "#960: project documents should expose the project-item designer context");
        expect(has_descriptor_id(context.editor_actions, "navigate-project-item"),
               "#960: project designer context should include project navigation actions");
        expect(has_descriptor_id(context.builders, "application-wizard"),
               "#960: project designer context should include application wizard builders");
        expect(context.toolbox_items.empty(), "#960: project designer context should not expose toolbox items");
    }

    const auto database_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("data.dbc").string()
    });
    expect(database_result.ok, "#960: synthetic database container should open for designer-context checks");
    expect(database_result.document.designer_contexts.size() == 1U,
           "#960: database documents should expose one default designer context");
    if (!database_result.document.designer_contexts.empty()) {
        const auto& context = database_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#960: database documents should expose the data-environment designer context");
        expect(has_descriptor_id(context.editor_actions, "edit-data-environment"),
               "#960: data designer context should include data-environment actions");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#960: data designer context should include data-environment builders");
        expect(context.toolbox_items.empty(), "#960: data designer context should not expose toolbox items");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_launch_selection_record_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_selection_metadata_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const auto bytes = make_vfp_header();
    {
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::studio::open_document({
        .path = form_path.string(),
        .symbol = "cmdSave.Click",
        .line = 42U,
        .column = 7U,
        .record_index = 5U
    });

    expect(result.ok, "#964: synthetic form should open for launch selection metadata checks");
    expect(result.document.selection_symbol == "cmdSave.Click",
           "#964: open_document should preserve launch selection symbols");
    expect(result.document.selection_line == 42U,
           "#964: open_document should preserve launch selection lines");
    expect(result.document.selection_column == 7U,
           "#964: open_document should preserve launch selection columns");
    expect(result.document.selection_record_index == 5U,
           "#964: open_document should preserve launch selection record indexes");

    fs::remove_all(temp_dir, ignored);
}

void test_object_snapshot_preserves_empty_and_null_design_fields() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\customer.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 7U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'M', .is_null = false, .display_value = "cmdSave", .memo_block_number = 101U},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "8.000", .memo_block_number = 102U},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "1.000", .memo_block_number = 103U},
                {.field_name = "PLATFORM", .field_type = 'C', .is_null = false, .display_value = "WINDOWS", .memo_block_number = 104U},
                {.field_name = "PARENT", .field_type = 'M', .is_null = false, .display_value = "frmCustomer", .memo_block_number = 105U},
                {.field_name = "HELP", .field_type = 'M', .is_null = true, .display_value = "", .memo_block_number = 0U},
                {.field_name = "TAG", .field_type = 'M', .is_null = false, .display_value = ""},
                {.field_name = "PROPERTIES", .field_type = 'M', .is_null = false, .display_value = "Caption = Save\r\nEnabled = .T.", .memo_block_number = 7U},
                {.field_name = "UNIQUEID", .field_type = 'M', .is_null = false, .display_value = "cmd-save-1", .memo_block_number = 108U},
                {.field_name = "CLASS", .field_type = 'M', .is_null = false, .display_value = "commandbutton", .memo_block_number = 109U},
                {.field_name = "BASECLASS", .field_type = 'M', .is_null = false, .display_value = "commandbutton", .memo_block_number = 110U}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 1U, "#658: form design snapshot should include the parsed record");
    if (!objects.empty()) {
        expect(objects[0].objtype_code == 8, "#667: object snapshots should expose raw OBJTYPE metadata");
        expect(objects[0].objtype_field_index == 1U, "#671: raw OBJTYPE metadata should retain DBF field provenance");
        expect(objects[0].objtype_memo_block_number == 102U, "#724: raw OBJTYPE metadata should retain memo block provenance");
        expect(objects[0].objcode_code == 1, "#667: object snapshots should expose raw OBJCODE metadata");
        expect(objects[0].objcode_field_index == 2U, "#671: raw OBJCODE metadata should retain DBF field provenance");
        expect(objects[0].objcode_memo_block_number == 103U, "#724: raw OBJCODE metadata should retain memo block provenance");
        expect(objects[0].platform == "WINDOWS", "#667: object snapshots should expose raw PLATFORM metadata");
        expect(objects[0].platform_field_index == 3U, "#671: raw PLATFORM metadata should retain DBF field provenance");
        expect(objects[0].platform_memo_block_number == 104U, "#724: raw PLATFORM metadata should retain memo block provenance");
        expect(objects[0].object_name == "cmdSave", "#660: object snapshots should expose the design object name");
        expect(objects[0].object_name_field_index == 0U, "#672: object name metadata should retain DBF field provenance");
        expect(objects[0].object_name_memo_block_number == 101U, "#717: object names should retain selected memo block provenance");
        expect(objects[0].unique_id == "cmd-save-1", "#660: object snapshots should expose stable UNIQUEID metadata");
        expect(objects[0].unique_id_field_index == 8U, "#672: UNIQUEID metadata should retain DBF field provenance");
        expect(objects[0].unique_id_memo_block_number == 108U, "#717: unique IDs should retain selected memo block provenance");
        expect(objects[0].parent_name == "frmCustomer", "#660: object snapshots should expose parent hierarchy metadata");
        expect(objects[0].parent_name_field_index == 4U, "#672: parent hierarchy metadata should retain DBF field provenance");
        expect(objects[0].parent_name_memo_block_number == 105U, "#717: parent names should retain selected memo block provenance");
        expect(objects[0].class_name == "commandbutton", "#660: object snapshots should expose CLASS metadata");
        expect(objects[0].class_name_field_index == 9U, "#672: CLASS metadata should retain DBF field provenance");
        expect(objects[0].class_name_memo_block_number == 109U, "#717: class names should retain selected memo block provenance");
        expect(objects[0].baseclass_name == "commandbutton", "#660: object snapshots should expose BASECLASS metadata");
        expect(objects[0].baseclass_name_field_index == 10U, "#672: BASECLASS metadata should retain DBF field provenance");
        expect(objects[0].baseclass_name_memo_block_number == 110U, "#717: baseclass names should retain selected memo block provenance");
        expect(objects[0].title == "cmdSave", "#673: friendly titles should keep existing form selection priority");
        expect(objects[0].title_field_index == 0U, "#673: friendly title metadata should retain selected DBF field provenance");
        expect(objects[0].title_memo_block_number == 101U, "#717: friendly titles should inherit selected field memo block provenance");
        expect(objects[0].subtitle == "commandbutton", "#673: friendly subtitles should keep existing form selection priority");
        expect(objects[0].subtitle_field_index == 10U, "#673: friendly subtitle metadata should retain selected DBF field provenance");
        expect(objects[0].subtitle_memo_block_number == 110U, "#717: friendly subtitles should inherit selected field memo block provenance");
        const auto parent = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "PARENT";
        });
        const auto tag = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "TAG";
        });
        const auto help = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "HELP";
        });
        const auto caption = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "Caption";
        });
        const auto enabled = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "Enabled";
        });

        expect(parent != objects[0].properties.end(), "#660: parent design field should stay in object snapshots");
        if (parent != objects[0].properties.end()) {
            expect(parent->value == "frmCustomer", "#660: parent field should remain available as direct property metadata");
            expect(parent->field_index == 4U, "#659: direct design fields should preserve their DBF field ordinal");
            expect(!parent->derived_from_property_blob, "#659: direct DBF fields should not be marked blob-derived");
            expect(parent->source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
                "#684: direct DBF fields should not masquerade as property-blob line metadata");
            expect(parent->memo_block_number == 105U, "#717: memo-backed direct identity properties should retain memo block provenance");
        }
        expect(help != objects[0].properties.end(), "#658: null design fields should stay in object snapshots");
        if (help != objects[0].properties.end()) {
            expect(help->is_null, "#658: null design field metadata should stay attached");
            expect(help->field_index == 5U, "#659: null direct fields should preserve their DBF field ordinal");
            expect(help->memo_block_number == 0U, "#712: null block-zero memo properties should expose memo block zero");
        }
        expect(tag != objects[0].properties.end(), "#658: empty memo-backed design fields should stay in object snapshots");
        if (tag != objects[0].properties.end()) {
            expect(tag->value.empty(), "#658: empty design fields should preserve their empty value");
            expect(tag->field_index == 6U, "#659: empty direct fields should preserve their DBF field ordinal");
            expect(tag->memo_block_number == 0U, "#712: empty direct memo properties should expose memo block zero");
        }
        expect(caption != objects[0].properties.end(), "#658: visual property blob expansion should still work");
        if (caption != objects[0].properties.end()) {
            expect(caption->field_index == 7U, "#659: blob-derived properties should retain the source PROPERTIES field ordinal");
            expect(caption->derived_from_property_blob, "#659: blob-derived properties should expose their provenance");
            expect(caption->source_line_index == 0U, "#684: first blob-derived property should retain its source memo line");
            expect(caption->memo_block_number == 7U, "#712: blob-derived properties should inherit the source PROPERTIES memo block");
        }
        expect(enabled != objects[0].properties.end(), "#684: second visual property blob line should expand into snapshots");
        if (enabled != objects[0].properties.end()) {
            expect(enabled->field_index == 7U, "#684: later blob-derived properties should retain the source PROPERTIES field ordinal");
            expect(enabled->source_line_index == 1U, "#684: later blob-derived properties should retain their source memo line");
            expect(enabled->memo_block_number == 7U, "#712: later blob-derived properties should inherit the source PROPERTIES memo block");
        }
    }
}

void test_object_snapshot_suppresses_unresolved_memo_placeholders() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\placeholder.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        {
            .record_index = 10U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'M', .is_null = false, .display_value = "<memo block 40>", .memo_block_number = 40U},
                {.field_name = "NAME", .field_type = 'M', .is_null = false, .display_value = "<memo block 41>"},
                {.field_name = "TITLE", .field_type = 'M', .is_null = false, .display_value = "<memo block 42>"},
                {.field_name = "CLASS", .field_type = 'M', .is_null = false, .display_value = "<memo block 43>"},
                {.field_name = "BASECLASS", .field_type = 'M', .is_null = false, .display_value = "<memo block 44>"},
                {.field_name = "PROPERTIES", .field_type = 'M', .is_null = false, .display_value = "<memo block 45>", .memo_block_number = 45U}
            }
        }
    };

    const auto form_objects = copperfin::studio::build_object_snapshot(form_document);
    expect(form_objects.size() == 1U, "#696: unresolved memo placeholders should not prevent object capture");
    if (!form_objects.empty()) {
        expect(form_objects[0].object_name.empty(), "#696: unresolved memo object names should not become active names");
        expect(form_objects[0].object_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#696: active object-name provenance should be missing when usable text is absent");
        expect(form_objects[0].object_name_memo_block_number == 0U,
            "#717: suppressed object-name metadata should expose memo block zero");
        expect(form_objects[0].title == "Record 10", "#696: unresolved memo title sources should use synthetic fallback");
        expect(form_objects[0].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#696: synthetic titles should not masquerade as unresolved memo provenance");
        expect(form_objects[0].title_memo_block_number == 0U,
            "#717: synthetic object titles should expose memo block zero");
        expect(form_objects[0].subtitle.empty(), "#696: unresolved memo subtitle sources should remain absent");
        expect(form_objects[0].subtitle_memo_block_number == 0U,
            "#717: suppressed subtitles should expose memo block zero");
        const auto objname = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "OBJNAME";
        });
        expect(objname != form_objects[0].properties.end(), "#696: direct unresolved memo fields should remain visible as properties");
        if (objname != form_objects[0].properties.end()) {
            expect(objname->field_index == 0U, "#696: direct unresolved memo fields should retain field provenance");
            expect(objname->value.empty(), "#696: direct unresolved memo placeholders should not become property values");
            expect(objname->memo_block_number == 40U, "#712: unresolved direct memo fields should retain memo block provenance");
        }
        const auto caption = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "Caption";
        });
        expect(caption == form_objects[0].properties.end(), "#696: unresolved PROPERTIES memo placeholders should not expand blob properties");
    }

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\placeholder.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        {
            .record_index = 11U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "<memo block 46>", .memo_block_number = 46U},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "<memo block 47>", .memo_block_number = 47U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "<memo block 48>", .memo_block_number = 48U},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "fallback_menu"}
            }
        }
    };

    const auto menu_objects = copperfin::studio::build_object_snapshot(menu_document);
    expect(menu_objects.size() == 1U, "#696: unresolved menu memo placeholders should not prevent object capture");
    if (!menu_objects.empty()) {
        expect(menu_objects[0].menu_prompt.empty(), "#696: unresolved menu PROMPT placeholders should not become prompt text");
        expect(menu_objects[0].menu_prompt_field_index == 0U, "#696: menu PROMPT provenance should remain available");
        expect(menu_objects[0].menu_prompt_memo_block_number == 46U,
            "#718: unresolved menu PROMPT metadata should retain memo block provenance");
        expect(menu_objects[0].menu_command.empty(), "#696: unresolved menu COMMAND placeholders should not become command text");
        expect(menu_objects[0].menu_command_field_index == 1U, "#696: menu COMMAND provenance should remain available");
        expect(menu_objects[0].menu_command_memo_block_number == 47U,
            "#718: unresolved menu COMMAND metadata should retain memo block provenance");
        expect(menu_objects[0].menu_message.empty(), "#696: unresolved menu MESSAGE placeholders should not become message text");
        expect(menu_objects[0].menu_message_field_index == 2U, "#696: menu MESSAGE provenance should remain available");
        expect(menu_objects[0].menu_message_memo_block_number == 48U,
            "#718: unresolved menu MESSAGE metadata should retain memo block provenance");
        expect(menu_objects[0].title == "fallback_menu", "#696: menu titles should skip unresolved PROMPT and use the next usable source");
        expect(menu_objects[0].title_field_index == 3U, "#696: menu title provenance should point at the selected usable source");
    }
}

void test_object_snapshot_trims_normalized_display_metadata() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\trimmed.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        {
            .record_index = 12U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'C', .is_null = false, .display_value = "   "},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "  cmdSave  "},
                {.field_name = "BASECLASS", .field_type = 'C', .is_null = false, .display_value = "  commandbutton  "},
                {.field_name = "PLATFORM", .field_type = 'C', .is_null = false, .display_value = "  WINDOWS  ", .memo_block_number = 134U},
                {.field_name = "CLASS", .field_type = 'C', .is_null = false, .display_value = "  commandbutton  "}
            }
        }
    };

    const auto form_objects = copperfin::studio::build_object_snapshot(form_document);
    expect(form_objects.size() == 1U, "#705: trimmed form metadata should still produce an object snapshot");
    if (!form_objects.empty()) {
        expect(form_objects[0].object_name == "cmdSave",
            "#705: whitespace-only OBJNAME should be ignored and fallback NAME should be trimmed");
        expect(form_objects[0].object_name_field_index == 1U,
            "#705: object-name provenance should point at the selected fallback field");
        expect(form_objects[0].title == "cmdSave", "#705: form titles should use trimmed display metadata");
        expect(form_objects[0].title_field_index == 1U, "#705: form title provenance should point at trimmed NAME");
        expect(form_objects[0].subtitle == "commandbutton", "#705: form subtitles should use trimmed display metadata");
        expect(form_objects[0].subtitle_field_index == 2U, "#705: form subtitle provenance should point at BASECLASS");
        expect(form_objects[0].platform == "WINDOWS", "#705: platform metadata should be trimmed");
        expect(form_objects[0].platform_field_index == 3U, "#705: platform provenance should still point at PLATFORM");
        expect(form_objects[0].platform_memo_block_number == 134U, "#724: trimmed platform metadata should retain source memo block provenance");
        const auto name = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "NAME";
        });
        expect(name != form_objects[0].properties.end(), "#705: direct NAME property should remain visible");
        if (name != form_objects[0].properties.end()) {
            expect(name->value == "  cmdSave  ", "#705: direct DBF property values should remain source-faithful");
            expect(name->field_index == 1U, "#705: direct DBF property provenance should remain unchanged");
        }
    }

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\trimmed.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        {
            .record_index = 13U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "  Customer  ", .memo_block_number = 130U},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "  MAIN  "},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "  DO FORM customer  ", .memo_block_number = 132U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "  Open customer  ", .memo_block_number = 133U},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "  customer_menu  "}
            }
        }
    };

    const auto menu_objects = copperfin::studio::build_object_snapshot(menu_document);
    expect(menu_objects.size() == 1U, "#705: trimmed menu metadata should still produce an object snapshot");
    if (!menu_objects.empty()) {
        expect(menu_objects[0].menu_prompt == "Customer", "#705: menu PROMPT metadata should be trimmed");
        expect(menu_objects[0].menu_prompt_memo_block_number == 130U, "#718: decoded menu PROMPT metadata should retain memo block provenance");
        expect(menu_objects[0].menu_level_name == "MAIN", "#705: menu LEVELNAME metadata should be trimmed");
        expect(menu_objects[0].menu_level_name_memo_block_number == 0U, "#718: non-memo menu LEVELNAME metadata should expose memo block zero");
        expect(menu_objects[0].menu_command == "DO FORM customer", "#705: menu COMMAND metadata should be trimmed");
        expect(menu_objects[0].menu_command_memo_block_number == 132U, "#718: decoded menu COMMAND metadata should retain memo block provenance");
        expect(menu_objects[0].menu_message == "Open customer", "#705: menu MESSAGE metadata should be trimmed");
        expect(menu_objects[0].menu_message_memo_block_number == 133U, "#718: decoded menu MESSAGE metadata should retain memo block provenance");
        expect(menu_objects[0].object_name == "customer_menu", "#705: menu object-name fallback should be trimmed");
        expect(menu_objects[0].object_name_field_index == 4U, "#705: menu object-name provenance should stay on NAME");
        expect(menu_objects[0].title == "Customer", "#705: menu title metadata should be trimmed");
        expect(menu_objects[0].title_field_index == 0U, "#705: menu title provenance should stay on PROMPT");
        expect(menu_objects[0].subtitle == "MAIN", "#705: menu subtitle metadata should be trimmed");
        expect(menu_objects[0].subtitle_field_index == 1U, "#705: menu subtitle provenance should stay on LEVELNAME");
        const auto prompt = std::find_if(menu_objects[0].properties.begin(), menu_objects[0].properties.end(), [](const auto& property) {
            return property.name == "PROMPT";
        });
        expect(prompt != menu_objects[0].properties.end(), "#705: direct PROMPT property should remain visible");
        if (prompt != menu_objects[0].properties.end()) {
            expect(prompt->value == "  Customer  ", "#705: direct menu property values should remain source-faithful");
            expect(prompt->field_index == 0U, "#705: direct menu property provenance should remain unchanged");
        }
    }
}

void test_menu_object_snapshot_preserves_normalized_menu_metadata() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\mainmenu.mnx)";
    document.kind = copperfin::studio::StudioAssetKind::menu;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Customer", .memo_block_number = 201U},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "MAIN"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO FORM customer", .memo_block_number = 203U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "Open customer maintenance", .memo_block_number = 204U},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "3.000", .memo_block_number = 205U},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "7.000", .memo_block_number = 206U}
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "TOOLS"},
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Tools"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO tools"},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "tools_menu"},
                {.field_name = "PARENTID", .field_type = 'C', .is_null = false, .display_value = "main_menu"}
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                {.field_name = "COMMENT", .field_type = 'M', .is_null = false, .display_value = "No display fields"}
            }
        },
        {
            .record_index = 5U,
            .deleted = true,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Obsolete"},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "OLD"}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 4U, "#668: menu snapshot should include parsed menu records");
    if (objects.size() >= 1U) {
        expect(objects[0].menu_prompt == "Customer", "#668: menu snapshots should expose PROMPT metadata");
        expect(objects[0].menu_prompt_field_index == 0U, "#669: menu PROMPT metadata should retain DBF field provenance");
        expect(objects[0].menu_prompt_memo_block_number == 201U, "#718: menu PROMPT metadata should retain source memo block provenance");
        expect(objects[0].menu_level_name == "MAIN", "#668: menu snapshots should expose LEVELNAME metadata");
        expect(objects[0].menu_level_name_field_index == 1U, "#669: menu LEVELNAME metadata should retain DBF field provenance");
        expect(objects[0].menu_level_name_memo_block_number == 0U, "#718: non-memo menu LEVELNAME metadata should expose memo block zero");
        expect(objects[0].menu_command == "DO FORM customer", "#668: menu snapshots should expose COMMAND metadata");
        expect(objects[0].menu_command_field_index == 2U, "#669: menu COMMAND metadata should retain DBF field provenance");
        expect(objects[0].menu_command_memo_block_number == 203U, "#718: menu COMMAND metadata should retain source memo block provenance");
        expect(objects[0].menu_message == "Open customer maintenance", "#668: menu snapshots should expose MESSAGE metadata");
        expect(objects[0].menu_message_field_index == 3U, "#669: menu MESSAGE metadata should retain DBF field provenance");
        expect(objects[0].menu_message_memo_block_number == 204U, "#718: menu MESSAGE metadata should retain source memo block provenance");
        expect(objects[0].title == "Customer", "#668: menu prompt should continue to drive friendly title fallback");
        expect(objects[0].title_field_index == 0U, "#673: menu title metadata should retain selected PROMPT provenance");
        expect(objects[0].subtitle == "MAIN", "#668: menu level name should continue to drive friendly subtitle fallback");
        expect(objects[0].subtitle_field_index == 1U, "#673: menu subtitle metadata should retain selected LEVELNAME provenance");
        expect(objects[0].objtype_code == 3, "#668: menu snapshots should retain raw OBJTYPE metadata");
        expect(objects[0].objtype_memo_block_number == 205U, "#724: menu OBJTYPE metadata should retain source memo block provenance");
        expect(objects[0].objcode_code == 7, "#668: menu snapshots should retain raw OBJCODE metadata");
        expect(objects[0].objcode_memo_block_number == 206U, "#724: menu OBJCODE metadata should retain source memo block provenance");
    }
    if (objects.size() >= 2U) {
        expect(objects[1].menu_prompt == "Tools", "#668: menu snapshots should expose PROMPT metadata when it is not field zero");
        expect(objects[1].menu_prompt_field_index == 1U, "#670: present menu fields should keep their actual DBF ordinal");
        expect(objects[1].object_name == "tools_menu", "#672: object name metadata should fall back to NAME");
        expect(objects[1].object_name_field_index == 3U, "#672: object name fallback should retain selected NAME field provenance");
        expect(objects[1].parent_name == "main_menu", "#672: parent metadata should fall back to PARENTID");
        expect(objects[1].parent_name_field_index == 4U, "#672: parent fallback should retain selected PARENTID field provenance");
        expect(objects[1].title == "Tools", "#673: menu title metadata should preserve existing PROMPT priority");
        expect(objects[1].title_field_index == 1U, "#673: menu title fallback should retain selected PROMPT provenance");
        expect(objects[1].subtitle == "TOOLS", "#673: menu subtitle metadata should preserve existing LEVELNAME priority");
        expect(objects[1].subtitle_field_index == 0U, "#673: menu subtitle fallback should retain selected LEVELNAME provenance");
        expect(objects[1].unique_id_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing UNIQUEID provenance should use the object missing-field sentinel");
        expect(objects[1].class_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing CLASS provenance should use the object missing-field sentinel");
        expect(objects[1].baseclass_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing BASECLASS provenance should use the object missing-field sentinel");
        expect(objects[1].menu_message.empty(), "#670: missing menu MESSAGE values should remain empty");
        expect(objects[1].menu_message_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#670: missing menu MESSAGE provenance should not masquerade as field zero");
        expect(objects[1].menu_message_memo_block_number == 0U,
            "#718: missing menu MESSAGE metadata should expose memo block zero");
        expect(objects[1].objtype_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJTYPE provenance should use the object missing-field sentinel");
        expect(objects[1].objtype_memo_block_number == 0U,
            "#724: missing OBJTYPE metadata should expose memo block zero");
        expect(objects[1].objcode_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJCODE provenance should use the object missing-field sentinel");
        expect(objects[1].objcode_memo_block_number == 0U,
            "#724: missing OBJCODE metadata should expose memo block zero");
        expect(objects[1].platform_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing PLATFORM provenance should use the object missing-field sentinel");
        expect(objects[1].platform_memo_block_number == 0U,
            "#724: missing PLATFORM metadata should expose memo block zero");
    }
    if (objects.size() >= 3U) {
        expect(objects[2].title == "Record 4", "#673: snapshots without display fields should keep synthetic title fallback");
        expect(objects[2].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#673: synthesized titles should use the object missing-field sentinel");
        expect(objects[2].subtitle.empty(), "#673: snapshots without subtitle fields should keep empty subtitle fallback");
        expect(objects[2].subtitle_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#673: missing subtitles should use the object missing-field sentinel");
    }
    if (objects.size() >= 4U) {
        expect(objects[3].deleted, "#688: deleted menu records should stay visible on object snapshots");
        expect(objects[3].title == "Obsolete", "#688: deleted menu records should keep normalized title metadata");
        expect(objects[3].title_field_index == 0U, "#688: deleted menu records should keep title provenance");
    }
}

void test_open_document_preserves_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "missing_sidecar.scx";
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .launched_from_visual_studio = false,
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for readable assets that carry validation findings");
    expect(
        result.document.inspection.has_validation_issues(),
        "Studio documents should retain validation findings from asset inspection");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.sidecar_missing";
            }),
        "Studio documents should expose the missing-sidecar validation finding");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_memo_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_memo_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "payload_truncated.scx";
    const fs::path sidecar_path = temp_dir / "payload_truncated.sct";

    {
        std::vector<std::uint8_t> table_bytes(115U, 0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 11U;
        table_bytes[4] = 0x01U;
        table_bytes[8] = 97U;
        table_bytes[10] = 18U;
        table_bytes[11] = 0U;
        table_bytes[28] = 0x00U;
        table_bytes[29] = 0x03U;
        table_bytes[32] = 'O';
        table_bytes[33] = 'B';
        table_bytes[34] = 'J';
        table_bytes[35] = 'N';
        table_bytes[36] = 'A';
        table_bytes[37] = 'M';
        table_bytes[38] = 'E';
        table_bytes[43] = 'C';
        table_bytes[44] = 1U;
        table_bytes[48] = 12U;
        table_bytes[64] = 'P';
        table_bytes[65] = 'R';
        table_bytes[66] = 'O';
        table_bytes[67] = 'P';
        table_bytes[68] = 'E';
        table_bytes[69] = 'R';
        table_bytes[70] = 'T';
        table_bytes[71] = 'I';
        table_bytes[72] = 'E';
        table_bytes[73] = 'S';
        table_bytes[75] = 'M';
        table_bytes[76] = 13U;
        table_bytes[80] = 4U;
        table_bytes[96] = 0x0DU;
        table_bytes[97] = 0x20U;
        table_bytes[98] = 't';
        table_bytes[99] = 'x';
        table_bytes[100] = 't';
        table_bytes[101] = 'T';
        table_bytes[102] = 'i';
        table_bytes[103] = 't';
        table_bytes[104] = 'l';
        table_bytes[105] = 'e';
        table_bytes[110] = 0x01U;
        table_bytes[114] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        memo_bytes[3] = 2U;
        memo_bytes[6] = 0x02U;
        memo_bytes[7] = 0x00U;
        memo_bytes[512U + 3U] = 1U;
        memo_bytes[512U + 4U] = 0x00U;
        memo_bytes[512U + 5U] = 0x00U;
        memo_bytes[512U + 6U] = 0x03U;
        memo_bytes[512U + 7U] = 0x84U;
        std::ofstream output(sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for forms with truncated memo payloads");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.payload_truncated";
            }),
        "Studio documents should preserve memo payload validation findings");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_dbf_descriptor_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_descriptor_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "bad_fields.scx";
    {
        std::vector<std::uint8_t> bytes(129U, 0U);
        bytes[0] = 0x30U;
        bytes[1] = 126U;
        bytes[2] = 4U;
        bytes[3] = 11U;
        bytes[4] = 0x01U;
        bytes[8] = 97U;
        bytes[10] = 17U;
        bytes[11] = 0U;
        bytes[28] = 0x00U;
        bytes[29] = 0x03U;
        bytes[32] = '1';
        bytes[33] = '2';
        bytes[34] = '3';
        bytes[35] = 'B';
        bytes[36] = 'A';
        bytes[37] = 'D';
        bytes[38] = 'N';
        bytes[39] = 'A';
        bytes[40] = 'M';
        bytes[41] = 'E';
        bytes[43] = 'C';
        bytes[44] = 1U;
        bytes[48] = 8U;
        bytes[64] = '1';
        bytes[65] = '2';
        bytes[66] = '3';
        bytes[67] = 'B';
        bytes[68] = 'A';
        bytes[69] = 'D';
        bytes[70] = 'N';
        bytes[71] = 'A';
        bytes[72] = 'M';
        bytes[73] = 'E';
        bytes[75] = 'C';
        bytes[76] = 9U;
        bytes[80] = 8U;
        bytes[96] = 0x0DU;
        bytes[97] = 0x20U;
        bytes[128] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for assets with DBF descriptor validation findings");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "dbf.field_name_duplicate";
            }),
        "Studio documents should preserve DBF descriptor validation findings");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_includes_prg_static_diagnostics() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_prg_diagnostics";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path program_path = temp_dir / "flagged.prg";
    {
        std::ofstream output(program_path, std::ios::binary);
        output << "DO WHILE .T.\n";
        output << "x = 1\n";
        output << "ENDDO\n";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = program_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a PRG file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::program, "PRG should map to a program document");
    expect(!result.document.static_diagnostics.empty(), "Studio documents should include PRG static diagnostics");
    expect(
        std::any_of(
            result.document.static_diagnostics.begin(),
            result.document.static_diagnostics.end(),
            [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
                return diagnostic.code == "PRG1001";
            }),
        "Studio documents should surface analyzer diagnostics for PRG files");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_parse_launch_arguments();
    test_parse_launch_arguments_rejects_unknown_switch();
    test_parse_launch_arguments_rejects_unknown_undo_mode();
    test_parse_launch_arguments_rejects_unknown_selection_context();
    test_parse_launch_arguments_rejects_missing_selection_context();
    test_open_document_infers_form_sidecar();
    test_open_document_uses_vfp_filename_for_display_name();
    test_open_document_attaches_default_designer_contexts();
    test_open_document_preserves_launch_selection_record_metadata();
    test_object_snapshot_preserves_empty_and_null_design_fields();
    test_object_snapshot_suppresses_unresolved_memo_placeholders();
    test_object_snapshot_trims_normalized_display_metadata();
    test_menu_object_snapshot_preserves_normalized_menu_metadata();
    test_open_document_preserves_validation_findings();
    test_open_document_preserves_memo_validation_findings();
    test_open_document_preserves_dbf_descriptor_validation_findings();
    test_open_document_includes_prg_static_diagnostics();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
