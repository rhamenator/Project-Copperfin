// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"
#include "test_process_capture_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message) {
    std::size_t offset = 0U;
    for (const auto& needle : needles) {
        const std::size_t position = text.find(needle, offset);
        if (position == std::string::npos) {
            expect(false, message);
            return;
        }
        offset = position + needle.size();
    }
}

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
}

using ProcessResult = copperfin::test_support::CapturedProcessResult;

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    return copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            copperfin::test_support::path_from_utf8_string(executable_path),
            arguments,
            working_directory));
}

void write_synthetic_report_table_for_layout_reorder_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "PAGEBREAK", .type = 'L', .length = 1U},
        {.name = "COLBREAK", .type = 'L', .length = 1U},
        {.name = "RESETPAGE", .type = 'L', .length = 1U},
        {.name = "EJECTBEFOR", .type = 'L', .length = 1U},
        {.name = "EJECTAFTER", .type = 'L', .length = 1U},
        {.name = "PLAIN", .type = 'L', .length = 1U},
        {.name = "TAG", .type = 'M', .length = 4U},
        {.name = "TAG2", .type = 'M', .length = 4U},
        {.name = "PENRED", .type = 'N', .length = 8U},
        {.name = "PENGREEN", .type = 'N', .length = 8U},
        {.name = "PENBLUE", .type = 'N', .length = 8U},
        {.name = "FILLRED", .type = 'N', .length = 8U},
        {.name = "FILLGREEN", .type = 'N', .length = 8U},
        {.name = "FILLBLUE", .type = 'N', .length = 8U}
    };
    std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", ".T.", ".F.", ".T.", ".T.", ".F.", ".T.", "DO ENTRY", "DO EXIT", "", "", "", "", ""},
        {"8", "0", "left.value", "100", "2600", "50", "200", "left-field-guid", "", "", "", "", "", "", "", "10", "20", "30", "40", "50", "60"},
        {"8", "0", "middle.value", "100", "2600", "50", "200", "middle-field-guid", "", "", "", "", "", "", "", "", "", "", "", "", ""},
        {"8", "0", "right.value", "100", "2600", "50", "200", "right-field-guid", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}
    };

    for (auto& record : records) {
        while (record.size() < fields.size()) {
            record.emplace_back();
        }
    }

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2800: selected-sections layout fixture should be created");
}

void write_synthetic_report_table_for_stable_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "section-guid"
    });
    expect(unique_id_result.ok, "#2800: stable section fixture should seed a section unique id");
    expect(!dbf_record_deleted(report_path, 1U),
           "#2800: stable section fixture should preserve the live section state");
}

void write_synthetic_report_table_for_line_shape_style_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "PENSIZE", .type = 'N', .length = 8U},
        {.name = "PENPAT", .type = 'N', .length = 8U},
        {.name = "FILLPAT", .type = 'N', .length = 8U}
    };
    std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3000", "", "", "", ""},
        {"6", "0", "", "100", "200", "800", "400", "line-style-guid", "2", "3", ""},
        {"7", "4", "", "1000", "200", "800", "400", "shape-style-guid", "4", "8", "2"}
    };
    for (auto& record : records) {
        while (record.size() < fields.size()) {
            record.emplace_back();
        }
    }
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#4535: line/shape host fixture should be created");
}

void write_synthetic_report_table_for_font_charset_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "DOUBLE", .type = 'L', .length = 1U},
        {.name = "RESOID", .type = 'N', .length = 8U}
    };
    std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", "settings-font-guid", ".T.", "0"},
        {"9", "4", "", "", "0", "", "3000", "", "", ""},
        {"5", "0", "label.value", "100", "200", "800", "400", "label-font-guid", ".T.", "1"},
        {"8", "0", "expression.value", "1000", "200", "800", "400", "expression-font-guid", ".T.", "2"},
        {"17", "0", "", "1900", "200", "800", "400", "picture-font-guid", ".T.", "3"},
        {"6", "0", "", "2800", "200", "800", "400", "line-font-guid", ".T.", "4"}
    };
    for (auto& record : records) {
        while (record.size() < fields.size()) {
            record.emplace_back();
        }
    }
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#4536: font-charset host fixture should be created");
}

void write_synthetic_report_table_for_header_view_settings_json(
    const std::filesystem::path& report_path,
    bool deleted) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "GRID", .type = 'L', .length = 1U},
        {.name = "RULER", .type = 'N', .length = 8U},
        {.name = "RULERLINES", .type = 'N', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "header-view-guid", ".T.", "4", "1"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#4537: header-view settings host fixture should be created");
    if (deleted) {
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
        expect(delete_result.ok, "#4537: header-view settings host fixture should preserve deleted state");
    }
}

void write_synthetic_report_table_for_header_picture_overrides_json(
    const std::filesystem::path& report_path,
    bool deleted) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "PICTURE", .type = 'M', .length = 4U},
        {.name = "COLOR", .type = 'N', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "header-picture-guid", "COLOR=2\nUNSUPPORTED=expr", "COLOR=1\nCOPIES=3\nUNSUPPORTED=picture", "9"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#4539: header PICTURE fixture should be created");
    if (deleted) {
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
        expect(delete_result.ok, "#4539: header PICTURE fixture should preserve deleted state");
    }
}

void write_synthetic_report_table_for_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#2800: deleted section fixture should mark section deleted");
}

void write_synthetic_report_table_for_stable_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_section_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-section-guid"
    });
    expect(unique_id_result.ok, "#2800: stable deleted section fixture should seed a deleted section unique id");
    expect(dbf_record_deleted(report_path, 1U),
           "#2800: stable deleted section fixture should preserve the deleted section state");
}

void run_live_section_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_section_json(asset_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "section-guid", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(section_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    issue_prefix + " should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    issue_prefix + " should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 2000",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " should include selected section object right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 150",
                    issue_prefix + " should include selected section object widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 5000",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    issue_prefix + " should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"pageBreak\": \"true\"",
                    issue_prefix + " should expose the invariant PAGEBREAK section value");
    expect_contains(section_process.stdout_text, "\"pageBreakFieldIndex\": 8",
                    issue_prefix + " should expose PAGEBREAK field provenance");
    expect_contains(section_process.stdout_text, "\"columnBreak\": \"false\"",
                    issue_prefix + " should expose the invariant COLBREAK section value");
    expect_contains(section_process.stdout_text, "\"resetPage\": \"true\"",
                    issue_prefix + " should expose the invariant RESETPAGE section value");
    expect_contains(section_process.stdout_text, "\"onEntryExpression\": \"DO ENTRY\"",
                    issue_prefix + " should expose the invariant TAG entry expression");
    expect_contains(section_process.stdout_text, "\"onEntryExpressionFieldIndex\": 14",
                    issue_prefix + " should expose TAG entry-expression field provenance");
    expect_contains(section_process.stdout_text, "\"onExitExpression\": \"DO EXIT\"",
                    issue_prefix + " should expose the invariant TAG2 exit expression");
    expect_contains(
        section_process.stdout_text,
        "\"sectionCount\": 1,\n      \"deletedSectionCount\": 0",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains(section_process.stdout_text, "\"unplacedObjectCount\": 0",
                    issue_prefix + " should preserve section object membership");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 1",
            "\"objectCount\": 3"
        },
        issue_prefix + " should expose selected live-section metadata");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "section-guid",
            "--property-name", "PAGEBREAK", "--property-value", "false",
            "--property-name", "COLBREAK", "--property-value", "true",
            "--property-name", "RESETPAGE", "--property-value", "false",
            "--property-name", "EJECTBEFOR", "--property-value", "false",
            "--property-name", "EJECTAFTER", "--property-value", "true",
            "--property-name", "PLAIN", "--property-value", "false",
            "--property-name", "TAG", "--property-value", "UPDATED ENTRY",
            "--property-name", "TAG2", "--property-value", "UPDATED EXIT",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip section pagination flag edits through the host update path");
    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "section-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen a section after pagination flag edits");
    expect_contains(reopened_process.stdout_text, "\"pageBreak\": \"false\"",
                    issue_prefix + " should persist PAGEBREAK edits");
    expect_contains(reopened_process.stdout_text, "\"columnBreak\": \"true\"",
                    issue_prefix + " should persist COLBREAK edits");
    expect_contains(reopened_process.stdout_text, "\"resetPage\": \"false\"",
                    issue_prefix + " should persist RESETPAGE edits");
    expect_contains(reopened_process.stdout_text, "\"ejectBefore\": \"false\"",
                    issue_prefix + " should persist EJECTBEFOR edits");
    expect_contains(reopened_process.stdout_text, "\"ejectAfter\": \"true\"",
                    issue_prefix + " should persist EJECTAFTER edits");
    expect_contains(reopened_process.stdout_text, "\"plain\": \"false\"",
                    issue_prefix + " should persist PLAIN edits");
    expect_contains(reopened_process.stdout_text, "\"onEntryExpression\": \"UPDATED ENTRY\"",
                    issue_prefix + " should persist TAG entry-expression edits");
    expect_contains(reopened_process.stdout_text, "\"onExitExpression\": \"UPDATED EXIT\"",
                    issue_prefix + " should persist TAG2 exit-expression edits");
}

void run_live_object_color_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_section_json(asset_path);

    const auto selected_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "left-field-guid", "--json"},
        temp_root);
    expect(selected_process.exit_code == 0, issue_prefix + " should select the color-bearing report object");
    expect_contains(selected_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " should advertise selected report-object availability");
    expect_contains(selected_process.stdout_text, "\"name\": \"PENRED\"",
                    issue_prefix + " should expose PENRED in report-object highlights");
    expect_contains(selected_process.stdout_text, "\"fieldIndex\": 16",
                    issue_prefix + " should preserve PENRED field provenance");
    expect_contains(selected_process.stdout_text, "\"value\": \"10\"",
                    issue_prefix + " should expose the live PENRED value");
    expect_contains(selected_process.stdout_text, "\"name\": \"FILLBLUE\"",
                    issue_prefix + " should expose FILLBLUE in report-object highlights");
    expect_contains(selected_process.stdout_text, "\"value\": \"60\"",
                    issue_prefix + " should expose the live FILLBLUE value");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "left-field-guid",
            "--property-name", "PENRED", "--property-value", "11",
            "--property-name", "PENGREEN", "--property-value", "21",
            "--property-name", "PENBLUE", "--property-value", "31",
            "--property-name", "FILLRED", "--property-value", "41",
            "--property-name", "FILLGREEN", "--property-value", "51",
            "--property-name", "FILLBLUE", "--property-value", "61",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip RGB channel edits through the host update path");
    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "left-field-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen a report object after RGB channel edits");
    expect_contains(reopened_process.stdout_text, "\"name\": \"PENRED\"",
                    issue_prefix + " should preserve PENRED after reopen");
    expect_contains(reopened_process.stdout_text, "\"value\": \"11\"",
                    issue_prefix + " should persist PENRED edits");
    expect_contains(reopened_process.stdout_text, "\"name\": \"FILLBLUE\"",
                    issue_prefix + " should preserve FILLBLUE after reopen");
    expect_contains(reopened_process.stdout_text, "\"value\": \"61\"",
                    issue_prefix + " should persist FILLBLUE edits");
}

void run_line_shape_style_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_line_shape_style_json(asset_path);

    const auto shape_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "shape-style-guid", "--json"},
        temp_root);
    expect(shape_process.exit_code == 0, issue_prefix + " should select the shape object");
    expect_contains(shape_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " should advertise selected shape availability");
    expect_contains(shape_process.stdout_text, "\"name\": \"PENSIZE\"",
                    issue_prefix + " should expose shape PENSIZE");
    expect_contains(shape_process.stdout_text, "\"name\": \"PENPAT\"",
                    issue_prefix + " should expose shape PENPAT");
    expect_contains(shape_process.stdout_text, "\"name\": \"FILLPAT\"",
                    issue_prefix + " should expose shape FILLPAT");
    expect_contains(shape_process.stdout_text, "\"value\": \"2\"",
                    issue_prefix + " should expose the shape FILLPAT value");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "shape-style-guid",
            "--property-name", "PENSIZE", "--property-value", "6",
            "--property-name", "PENPAT", "--property-value", "5",
            "--property-name", "FILLPAT", "--property-value", "7",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip shape styling edits through the host update path");
    const auto reopened_shape_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "shape-style-guid", "--json"},
        temp_root);
    expect_contains(reopened_shape_process.stdout_text, "\"value\": \"7\"",
                    issue_prefix + " should persist shape FILLPAT edits");

    const auto line_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "line-style-guid", "--json"},
        temp_root);
    expect(line_process.exit_code == 0, issue_prefix + " should select the line object");
    expect_contains(line_process.stdout_text, "\"name\": \"PENSIZE\"",
                    issue_prefix + " should expose line PENSIZE");
    expect_contains(line_process.stdout_text, "\"name\": \"PENPAT\"",
                    issue_prefix + " should expose line PENPAT");
    const auto selected_line_start = line_process.stdout_text.find("\"selectedReportObject\": {");
    const auto selected_line_end = line_process.stdout_text.find("\"selectedReportObjectSectionAvailable\"", selected_line_start);
    const std::string selected_line_json = selected_line_start == std::string::npos
        ? std::string{}
        : line_process.stdout_text.substr(selected_line_start, selected_line_end - selected_line_start);
    expect(selected_line_json.find("\"name\": \"FILLPAT\"") == std::string::npos,
           issue_prefix + " should not expose shape-only FILLPAT on lines");
}

void run_font_charset_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_font_charset_json(asset_path);

    const auto settings_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "settings-font-guid", "--json"},
        temp_root);
    expect(settings_process.exit_code == 0, issue_prefix + " should select the header settings record");
    expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected header settings");
    expect_contains(settings_process.stdout_text, "\"name\": \"DOUBLE\"",
                    issue_prefix + " should expose header DOUBLE");
    expect_contains(settings_process.stdout_text, "\"name\": \"RESOID\"",
                    issue_prefix + " should expose header RESOID");
    expect_contains(settings_process.stdout_text, "\"fieldIndex\": 8",
                    issue_prefix + " should preserve header DOUBLE field provenance");

    const auto settings_update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "settings-font-guid",
            "--property-name", "DOUBLE", "--property-value", "false",
            "--property-name", "RESOID", "--property-value", "5",
            "--json"
        },
        temp_root);
    expect(settings_update_process.exit_code == 0,
           issue_prefix + " should round-trip header font charset edits");
    const auto reopened_settings_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "settings-font-guid", "--json"},
        temp_root);
    expect_contains(reopened_settings_process.stdout_text, "\"value\": \"false\"",
                    issue_prefix + " should persist header DOUBLE edits");
    expect_contains(reopened_settings_process.stdout_text, "\"value\": \"5\"",
                    issue_prefix + " should persist header RESOID edits");

    const auto label_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "label-font-guid", "--json"},
        temp_root);
    expect(label_process.exit_code == 0, issue_prefix + " should select the label object");
    expect_contains(label_process.stdout_text, "\"name\": \"DOUBLE\"",
                    issue_prefix + " should expose label DOUBLE");
    expect_contains(label_process.stdout_text, "\"name\": \"RESOID\"",
                    issue_prefix + " should expose label RESOID");
    expect_contains(label_process.stdout_text, "\"value\": \"1\"",
                    issue_prefix + " should expose the label font charset value");
    expect_contains(label_process.stdout_text, "\"fieldIndex\": 9",
                    issue_prefix + " should preserve label font charset field provenance");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "label-font-guid",
            "--property-name", "DOUBLE", "--property-value", "false",
            "--property-name", "RESOID", "--property-value", "4",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip font charset edits through the host update path");
    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "label-font-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen a label after font charset edits");
    expect_contains(reopened_process.stdout_text, "\"value\": \"false\"",
                    issue_prefix + " should persist DOUBLE edits");
    expect_contains(reopened_process.stdout_text, "\"value\": \"4\"",
                    issue_prefix + " should persist RESOID edits");

    const auto picture_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "picture-font-guid", "--json"},
        temp_root);
    expect(picture_process.exit_code == 0, issue_prefix + " should select the picture object");
    const auto selected_picture_start = picture_process.stdout_text.find("\"selectedReportObject\": {");
    const auto selected_picture_end = picture_process.stdout_text.find("\"selectedReportObjectSectionAvailable\"", selected_picture_start);
    const std::string selected_picture_json = selected_picture_start == std::string::npos
        ? std::string{}
        : picture_process.stdout_text.substr(selected_picture_start, selected_picture_end - selected_picture_start);
    expect(selected_picture_json.find("\"name\": \"DOUBLE\"") != std::string::npos,
           issue_prefix + " should expose picture DOUBLE");
    expect(selected_picture_json.find("\"name\": \"RESOID\"") == std::string::npos,
           issue_prefix + " should not expose picture RESOID");

    const auto line_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "line-font-guid", "--json"},
        temp_root);
    expect(line_process.exit_code == 0, issue_prefix + " should select the line object");
    const auto selected_line_start = line_process.stdout_text.find("\"selectedReportObject\": {");
    const auto selected_line_end = line_process.stdout_text.find("\"selectedReportObjectSectionAvailable\"", selected_line_start);
    const std::string selected_line_json = selected_line_start == std::string::npos
        ? std::string{}
        : line_process.stdout_text.substr(selected_line_start, selected_line_end - selected_line_start);
    expect(selected_line_json.find("\"name\": \"DOUBLE\"") == std::string::npos &&
           selected_line_json.find("\"name\": \"RESOID\"") == std::string::npos,
           issue_prefix + " should not expose font charset fields on lines");
}

void run_header_view_settings_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    bool deleted,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_header_view_settings_json(asset_path, deleted);

    const auto selected_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-view-guid", "--json"},
        temp_root);
    expect(selected_process.exit_code == 0, issue_prefix + " should select the header settings record");
    expect_contains(selected_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected header settings");
    expect_contains(selected_process.stdout_text, "\"name\": \"GRID\"",
                    issue_prefix + " should expose header GRID");
    expect_contains(selected_process.stdout_text, "\"name\": \"RULER\"",
                    issue_prefix + " should expose header RULER");
    expect_contains(selected_process.stdout_text, "\"name\": \"RULERLINES\"",
                    issue_prefix + " should expose header RULERLINES");
    expect_contains(selected_process.stdout_text, "\"fieldIndex\": 3",
                    issue_prefix + " should preserve header GRID field provenance");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-view-guid",
            "--property-name", "GRID", "--property-value", "false",
            "--property-name", "RULER", "--property-value", "2",
            "--property-name", "RULERLINES", "--property-value", "0",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip header view setting edits");

    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-view-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen header settings after edits");
    expect_contains(reopened_process.stdout_text, "\"value\": \"false\"",
                    issue_prefix + " should persist GRID edits");
    expect_contains(reopened_process.stdout_text, "\"value\": \"2\"",
                    issue_prefix + " should persist RULER edits");
    expect_contains(reopened_process.stdout_text, "\"value\": \"0\"",
                    issue_prefix + " should persist RULERLINES edits");

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--clear-property",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-view-guid",
            "--property-name", "GRID",
            "--json"
        },
        temp_root);
    expect(clear_process.exit_code == 0,
           issue_prefix + " should clear header GRID through the generic property path");
    const auto reopened_clear_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-view-guid", "--json"},
        temp_root);
    expect(reopened_clear_process.exit_code == 0,
           issue_prefix + " should reopen header settings after clearing GRID");
    const auto selected_settings_start = reopened_clear_process.stdout_text.find("\"selectedReportSettings\": [");
    const auto selected_settings_end = reopened_clear_process.stdout_text.find("]", selected_settings_start);
    const std::string selected_settings_json = selected_settings_start == std::string::npos
        ? std::string{}
        : reopened_clear_process.stdout_text.substr(selected_settings_start, selected_settings_end - selected_settings_start);
    expect(selected_settings_json.find("\"name\": \"GRID\"") == std::string::npos,
           issue_prefix + " should remove cleared GRID from selected settings");
    expect_contains(selected_settings_json, "\"name\": \"RULER\"",
                    issue_prefix + " should preserve unaffected RULER after clearing GRID");

    const auto clear_ruler_lines_process = run_process_capture(
        studio_host_path,
        {
            "--clear-property",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-view-guid",
            "--property-name", "RULERLINES",
            "--json"
        },
        temp_root);
    expect(clear_ruler_lines_process.exit_code == 0,
           issue_prefix + " should clear header RULERLINES through the generic property path");
    const auto reopened_ruler_lines_clear_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-view-guid", "--json"},
        temp_root);
    expect(reopened_ruler_lines_clear_process.exit_code == 0,
           issue_prefix + " should reopen header settings after clearing RULERLINES");
    const auto ruler_lines_clear_start = reopened_ruler_lines_clear_process.stdout_text.find("\"selectedReportSettings\": [");
    const auto ruler_lines_clear_end = reopened_ruler_lines_clear_process.stdout_text.find("]", ruler_lines_clear_start);
    const std::string ruler_lines_clear_settings = ruler_lines_clear_start == std::string::npos
        ? std::string{}
        : reopened_ruler_lines_clear_process.stdout_text.substr(
              ruler_lines_clear_start,
              ruler_lines_clear_end - ruler_lines_clear_start);
    expect(ruler_lines_clear_settings.find("\"name\": \"RULERLINES\"") == std::string::npos,
           issue_prefix + " should remove cleared header RULERLINES from selected settings");
}

void run_header_picture_overrides_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    bool deleted,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_header_picture_overrides_json(asset_path, deleted);

    const auto selected_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-picture-guid", "--json"},
        temp_root);
    expect(selected_process.exit_code == 0, issue_prefix + " should select header PICTURE settings");
    expect_contains(selected_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected header PICTURE settings");
    expect_contains(selected_process.stdout_text, "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 4",
                    issue_prefix + " should expose effective PICTURE COLOR provenance");
    expect_contains(selected_process.stdout_text, "\"value\": \"1\"",
                    issue_prefix + " should prefer the PICTURE COLOR value over EXPR");
    expect_contains(selected_process.stdout_text, "\"name\": \"COPIES\"",
                    issue_prefix + " should expose unique PICTURE printer settings");
    expect_contains(selected_process.stdout_text, "\"value\": \"picture\"",
                    issue_prefix + " should preserve unsupported PICTURE lines");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-picture-guid",
            "--property-name", "COLOR", "--property-value", "0",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should update the effective PICTURE printer setting");

    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-picture-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen header PICTURE settings after editing");
    expect_contains(reopened_process.stdout_text, "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 4",
                    issue_prefix + " should retain PICTURE provenance after editing");
    expect_contains(reopened_process.stdout_text, "\"value\": \"0\"",
                    issue_prefix + " should persist the PICTURE override edit");

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--clear-property",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-picture-guid",
            "--property-name", "COPIES",
            "--json"
        },
        temp_root);
    expect(clear_process.exit_code == 0,
           issue_prefix + " should clear a header PICTURE printer setting");
    const auto reopened_clear_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-picture-guid", "--json"},
        temp_root);
    expect(reopened_clear_process.exit_code == 0,
           issue_prefix + " should reopen after clearing a header PICTURE setting");
    const auto settings_start = reopened_clear_process.stdout_text.find("\"selectedReportSettings\": [");
    const auto settings_end = reopened_clear_process.stdout_text.find("]", settings_start);
    const std::string selected_settings_json = settings_start == std::string::npos
        ? std::string{}
        : reopened_clear_process.stdout_text.substr(settings_start, settings_end - settings_start);
    expect(selected_settings_json.find("\"name\": \"COPIES\"") == std::string::npos,
           issue_prefix + " should remove cleared PICTURE COPIES");
    expect_contains(selected_settings_json, "\"value\": \"picture\"",
                    issue_prefix + " should preserve unsupported PICTURE content after clearing");
}

void write_synthetic_report_table_for_header_add_alias_json(
    const std::filesystem::path& report_path,
    bool deleted) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "RULERLINES", .type = 'N', .length = 8U},
        {.name = "ADDALIAS", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "header-add-alias-guid", "1", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#4540: header ADDALIAS fixture should be created");
    if (deleted) {
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
        expect(delete_result.ok, "#4540: header ADDALIAS fixture should preserve deleted state");
    }
}

void run_header_add_alias_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    bool deleted,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_header_add_alias_json(asset_path, deleted);

    const auto selected_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-add-alias-guid", "--json"},
        temp_root);
    expect(selected_process.exit_code == 0, issue_prefix + " should select header ADDALIAS settings");
    expect_contains(selected_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected header ADDALIAS settings");
    expect_contains(selected_process.stdout_text, "\"name\": \"ADDALIAS\"",
                    issue_prefix + " should expose header ADDALIAS");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-add-alias-guid",
            "--property-name", "ADDALIAS", "--property-value", "false",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip header ADDALIAS edits");

    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-add-alias-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen header ADDALIAS after editing");
    expect_contains(reopened_process.stdout_text, "\"name\": \"ADDALIAS\"",
                    issue_prefix + " should retain ADDALIAS after editing");
    expect_contains(reopened_process.stdout_text, "\"value\": \"false\"",
                    issue_prefix + " should persist the ADDALIAS edit");

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--clear-property",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-add-alias-guid",
            "--property-name", "ADDALIAS",
            "--json"
        },
        temp_root);
    expect(clear_process.exit_code == 0,
           issue_prefix + " should clear header ADDALIAS");
    const auto reopened_clear_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-add-alias-guid", "--json"},
        temp_root);
    expect(reopened_clear_process.exit_code == 0,
           issue_prefix + " should reopen after clearing ADDALIAS");
    const auto settings_start = reopened_clear_process.stdout_text.find("\"selectedReportSettings\": [");
    const auto settings_end = reopened_clear_process.stdout_text.find("]", settings_start);
    const std::string selected_settings_json = settings_start == std::string::npos
        ? std::string{}
        : reopened_clear_process.stdout_text.substr(settings_start, settings_end - settings_start);
    expect(selected_settings_json.find("\"name\": \"ADDALIAS\"") == std::string::npos,
           issue_prefix + " should remove cleared ADDALIAS");
    expect_contains(selected_settings_json, "\"name\": \"RULERLINES\"",
                    issue_prefix + " should preserve unrelated header settings");
}

void write_synthetic_report_table_for_header_curpos_json(
    const std::filesystem::path& report_path,
    bool deleted) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "RULERLINES", .type = 'N', .length = 8U},
        {.name = "ADDALIAS", .type = 'L', .length = 1U},
        {.name = "CURPOS", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "header-curpos-guid", "1", ".T.", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#4541: header CURPOS fixture should be created");
    if (deleted) {
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
        expect(delete_result.ok, "#4541: header CURPOS fixture should preserve deleted state");
    }
}

void run_header_curpos_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    bool deleted,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_header_curpos_json(asset_path, deleted);

    const auto selected_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-curpos-guid", "--json"},
        temp_root);
    expect(selected_process.exit_code == 0, issue_prefix + " should select header CURPOS settings");
    expect_contains(selected_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected header CURPOS settings");
    expect_contains(selected_process.stdout_text, "\"name\": \"CURPOS\"",
                    issue_prefix + " should expose header CURPOS");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-curpos-guid",
            "--property-name", "CURPOS", "--property-value", "false",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip header CURPOS edits");

    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-curpos-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen header CURPOS after editing");
    expect_contains(reopened_process.stdout_text, "\"name\": \"CURPOS\"",
                    issue_prefix + " should retain CURPOS after editing");
    expect_contains(reopened_process.stdout_text, "\"value\": \"false\"",
                    issue_prefix + " should persist the CURPOS edit");

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--clear-property",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-curpos-guid",
            "--property-name", "CURPOS",
            "--json"
        },
        temp_root);
    expect(clear_process.exit_code == 0,
           issue_prefix + " should clear header CURPOS");
    const auto reopened_clear_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-curpos-guid", "--json"},
        temp_root);
    expect(reopened_clear_process.exit_code == 0,
           issue_prefix + " should reopen after clearing CURPOS");
    const auto settings_start = reopened_clear_process.stdout_text.find("\"selectedReportSettings\": [");
    const auto settings_end = reopened_clear_process.stdout_text.find("]", settings_start);
    const std::string selected_settings_json = settings_start == std::string::npos
        ? std::string{}
        : reopened_clear_process.stdout_text.substr(settings_start, settings_end - settings_start);
    expect(selected_settings_json.find("\"name\": \"CURPOS\"") == std::string::npos,
           issue_prefix + " should remove cleared CURPOS");
    expect_contains(selected_settings_json, "\"name\": \"ADDALIAS\"",
                    issue_prefix + " should preserve unrelated header settings");
}

void write_synthetic_report_table_for_header_unique_json(
    const std::filesystem::path& report_path,
    bool deleted) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "RULERLINES", .type = 'N', .length = 8U},
        {.name = "ADDALIAS", .type = 'L', .length = 1U},
        {.name = "CURPOS", .type = 'L', .length = 1U},
        {.name = "UNIQUE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "header-unique-guid", "1", ".T.", ".T.", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#4542: header UNIQUE fixture should be created");
    if (deleted) {
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
        expect(delete_result.ok, "#4542: header UNIQUE fixture should preserve deleted state");
    }
}

void run_header_unique_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    bool deleted,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_header_unique_json(asset_path, deleted);

    const auto selected_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-unique-guid", "--json"},
        temp_root);
    expect(selected_process.exit_code == 0, issue_prefix + " should select header UNIQUE settings");
    expect_contains(selected_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected header UNIQUE settings");
    expect_contains(selected_process.stdout_text, "\"name\": \"UNIQUE\"",
                    issue_prefix + " should expose header UNIQUE");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-unique-guid",
            "--property-name", "UNIQUE", "--property-value", "false",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip header UNIQUE edits");

    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-unique-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen header UNIQUE after editing");
    expect_contains(reopened_process.stdout_text, "\"name\": \"UNIQUE\"",
                    issue_prefix + " should retain UNIQUE after editing");
    expect_contains(reopened_process.stdout_text, "\"value\": \"false\"",
                    issue_prefix + " should persist the UNIQUE edit");

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--clear-property",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "header-unique-guid",
            "--property-name", "UNIQUE",
            "--json"
        },
        temp_root);
    expect(clear_process.exit_code == 0,
           issue_prefix + " should clear header UNIQUE");
    const auto reopened_clear_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "header-unique-guid", "--json"},
        temp_root);
    expect(reopened_clear_process.exit_code == 0,
           issue_prefix + " should reopen after clearing UNIQUE");
    const auto settings_start = reopened_clear_process.stdout_text.find("\"selectedReportSettings\": [");
    const auto settings_end = reopened_clear_process.stdout_text.find("]", settings_start);
    const std::string selected_settings_json = settings_start == std::string::npos
        ? std::string{}
        : reopened_clear_process.stdout_text.substr(settings_start, settings_end - settings_start);
    expect(selected_settings_json.find("\"name\": \"UNIQUE\"") == std::string::npos,
           issue_prefix + " should remove cleared UNIQUE");
    expect_contains(selected_settings_json, "\"name\": \"CURPOS\"",
                    issue_prefix + " should preserve unrelated header settings");
}

void run_deleted_section_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_deleted_section_json(asset_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "deleted-section-guid", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected deleted section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected deleted section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(section_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    issue_prefix + " should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    issue_prefix + " should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve retained live-object preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 100",
                    issue_prefix + " should preserve retained live-object left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 2600",
                    issue_prefix + " should preserve retained live-object top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " should preserve retained live-object right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 2800",
                    issue_prefix + " should preserve retained live-object bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 50",
                    issue_prefix + " should preserve retained live-object preview width");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 200",
                    issue_prefix + " should preserve retained live-object preview height");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " should preserve deleted section left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                    issue_prefix + " should preserve deleted section top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                    issue_prefix + " should preserve deleted section right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                    issue_prefix + " should preserve deleted section bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                    issue_prefix + " should preserve deleted section widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                    issue_prefix + " should preserve deleted section heights");
    expect_contains(
        section_process.stdout_text,
        "\"sectionCount\": 0,\n      \"deletedSectionCount\": 1",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains(section_process.stdout_text, "\"unplacedObjectCount\": 0",
                    issue_prefix + " should preserve deleted-section object membership");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0"
        },
        issue_prefix + " should expose selected deleted-section metadata");
    expect_contains(section_process.stdout_text, "\"pageBreak\": \"true\"",
                    issue_prefix + " should expose deleted-section PAGEBREAK values");
    expect_contains(section_process.stdout_text, "\"columnBreak\": \"false\"",
                    issue_prefix + " should expose deleted-section COLBREAK values");
    expect_contains(section_process.stdout_text, "\"resetPage\": \"true\"",
                    issue_prefix + " should expose deleted-section RESETPAGE values");
    expect_contains(section_process.stdout_text, "\"ejectBefore\": \"true\"",
                    issue_prefix + " should expose deleted-section EJECTBEFOR values");
    expect_contains(section_process.stdout_text, "\"ejectAfter\": \"false\"",
                    issue_prefix + " should expose deleted-section EJECTAFTER values");
    expect_contains(section_process.stdout_text, "\"plain\": \"true\"",
                    issue_prefix + " should expose deleted-section PLAIN values");
    expect_contains(section_process.stdout_text, "\"onEntryExpression\": \"DO ENTRY\"",
                    issue_prefix + " should expose deleted-section TAG entry expressions");
    expect_contains(section_process.stdout_text, "\"onExitExpression\": \"DO EXIT\"",
                    issue_prefix + " should expose deleted-section TAG2 exit expressions");
}

void test_studio_host_json_preserves_selected_sections_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_sections_stable_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_live_section_selection(
        studio_host_path,
        temp_root,
        "selected_section_stable.frx",
        "report",
        "#2800: stable live report section selection");
    run_live_section_selection(
        studio_host_path,
        temp_root,
        "selected_section_stable.lbx",
        "label",
        "#2800: stable live label section selection");
    run_live_object_color_selection(
        studio_host_path,
        temp_root,
        "selected_object_color_stable.frx",
        "#4534: stable live report-object color selection");
    run_live_object_color_selection(
        studio_host_path,
        temp_root,
        "selected_object_color_stable.lbx",
        "#4534: stable live label-object color selection");
    run_line_shape_style_selection(
        studio_host_path,
        temp_root,
        "selected_line_shape_style_stable.frx",
        "#4535: stable live report line/shape styling selection");
    run_line_shape_style_selection(
        studio_host_path,
        temp_root,
        "selected_line_shape_style_stable.lbx",
        "#4535: stable live label line/shape styling selection");
    run_font_charset_selection(
        studio_host_path,
        temp_root,
        "selected_font_charset_stable.frx",
        "#4536: stable live report font charset selection");
    run_font_charset_selection(
        studio_host_path,
        temp_root,
        "selected_font_charset_stable.lbx",
        "#4536: stable live label font charset selection");
    run_header_view_settings_selection(
        studio_host_path,
        temp_root,
        "selected_header_view_settings_stable.frx",
        false,
        "#4537: stable live report header-view settings selection");
    run_header_view_settings_selection(
        studio_host_path,
        temp_root,
        "selected_header_view_settings_stable.lbx",
        false,
        "#4537: stable live label header-view settings selection");
    run_header_view_settings_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_view_settings_stable.frx",
        true,
        "#4537: stable deleted report header-view settings selection");
    run_header_view_settings_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_view_settings_stable.lbx",
        true,
        "#4537: stable deleted label header-view settings selection");
    run_header_picture_overrides_selection(
        studio_host_path,
        temp_root,
        "selected_header_picture_overrides_stable.frx",
        false,
        "#4539: stable live report header PICTURE settings selection");
    run_header_picture_overrides_selection(
        studio_host_path,
        temp_root,
        "selected_header_picture_overrides_stable.lbx",
        false,
        "#4539: stable live label header PICTURE settings selection");
    run_header_picture_overrides_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_picture_overrides_stable.frx",
        true,
        "#4539: stable deleted report header PICTURE settings selection");
    run_header_picture_overrides_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_picture_overrides_stable.lbx",
        true,
        "#4539: stable deleted label header PICTURE settings selection");
    run_header_add_alias_selection(
        studio_host_path,
        temp_root,
        "selected_header_add_alias_stable.frx",
        false,
        "#4540: stable live report header ADDALIAS selection");
    run_header_add_alias_selection(
        studio_host_path,
        temp_root,
        "selected_header_add_alias_stable.lbx",
        false,
        "#4540: stable live label header ADDALIAS selection");
    run_header_add_alias_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_add_alias_stable.frx",
        true,
        "#4540: stable deleted report header ADDALIAS selection");
    run_header_add_alias_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_add_alias_stable.lbx",
        true,
        "#4540: stable deleted label header ADDALIAS selection");
    run_header_curpos_selection(
        studio_host_path,
        temp_root,
        "selected_header_curpos_stable.frx",
        false,
        "#4541: stable live report header CURPOS selection");
    run_header_curpos_selection(
        studio_host_path,
        temp_root,
        "selected_header_curpos_stable.lbx",
        false,
        "#4541: stable live label header CURPOS selection");
    run_header_curpos_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_curpos_stable.frx",
        true,
        "#4541: stable deleted report header CURPOS selection");
    run_header_curpos_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_curpos_stable.lbx",
        true,
        "#4541: stable deleted label header CURPOS selection");
    run_header_unique_selection(
        studio_host_path,
        temp_root,
        "selected_header_unique_stable.frx",
        false,
        "#4542: stable live report header UNIQUE selection");
    run_header_unique_selection(
        studio_host_path,
        temp_root,
        "selected_header_unique_stable.lbx",
        false,
        "#4542: stable live label header UNIQUE selection");
    run_header_unique_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_unique_stable.frx",
        true,
        "#4542: stable deleted report header UNIQUE selection");
    run_header_unique_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_header_unique_stable.lbx",
        true,
        "#4542: stable deleted label header UNIQUE selection");
    run_deleted_section_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_section_stable.frx",
        "report",
        "#2800: stable deleted report section selection");
    run_deleted_section_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_section_stable.lbx",
        "label",
        "#2800: stable deleted label section selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_sections_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_sections_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
