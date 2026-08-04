// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

int failures = 0;
using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

void expect_not_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) == std::string::npos, message);
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

void expect_empty_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": false",
                    prefix + " should not fabricate live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve zero live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve zero live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve zero live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve zero live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve zero live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve zero live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    quoted.reserve(value.size() + 2U);
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('"');
    return quoted;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string normalize_line_endings(std::string text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == '\r') {
            if (index + 1U < text.size() && text[index + 1U] == '\n') {
                continue;
            }
            normalized.push_back('\n');
        } else {
            normalized.push_back(text[index]);
        }
    }
    return normalized;
}

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    namespace fs = std::filesystem;

    const fs::path resolved_executable_path = fs::absolute(executable_path);
    const fs::path stdout_path = working_directory / "studio_host_stdout.log";
    const fs::path stderr_path = working_directory / "studio_host_stderr.log";

    std::string command = quote_command_argument(resolved_executable_path.string());
    for (const auto& argument : arguments) {
        command += " ";
        command += quote_command_argument(argument);
    }
    command += " > ";
    command += quote_command_argument(stdout_path.string());
    command += " 2> ";
    command += quote_command_argument(stderr_path.string());

    const fs::path original_directory = fs::current_path();
    fs::current_path(working_directory);
    const int raw_exit_code = copperfin::test_support::run_shell_command(command);
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
    }

#if defined(_WIN32)
    result.exit_code = raw_exit_code;
#else
    if (raw_exit_code != -1 && WIFEXITED(raw_exit_code)) {
        result.exit_code = WEXITSTATUS(raw_exit_code);
    } else {
        result.exit_code = raw_exit_code;
    }
#endif
    return result;
}

void write_synthetic_report_table_for_orientation_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "ORIENTATIO", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "PAPERSIZE=1\nTOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "0", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2774: orientation settings fixture should be created");
}

void write_synthetic_report_table_for_deleted_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_orientation_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2774: deleted orientation settings fixture should mark settings deleted");
}

void write_synthetic_report_table_for_stable_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_orientation_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#2774: stable orientation fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#2774: stable orientation fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_orientation_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok,
           "#2774: stable deleted orientation fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#2774: stable deleted orientation fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_unsupported_expr_line_preservation(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3094: unsupported EXPR-line preservation fixture should be created");
}

void write_synthetic_report_table_for_deleted_unsupported_expr_line_preservation(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_unsupported_expr_line_preservation(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#3094: deleted unsupported EXPR-line preservation fixture should mark settings deleted");
}

void write_synthetic_report_table_for_stable_unsupported_expr_line_preservation(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_unsupported_expr_line_preservation(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#3094: stable unsupported EXPR fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#3094: stable unsupported EXPR fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_unsupported_expr_line_preservation(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_unsupported_expr_line_preservation(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok,
           "#3094: stable deleted unsupported EXPR fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#3094: stable deleted unsupported EXPR fixture should preserve the deleted settings state");
}

void run_orientation_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& updated_orientation,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path =
        temp_root / ((deleted ? "deleted_" : "") + std::string("orientation_update") + extension);
    if (deleted) {
        write_synthetic_report_table_for_stable_deleted_orientation_field_json(asset_path);
    } else {
        write_synthetic_report_table_for_stable_orientation_field_json(asset_path);
    }

    const std::string unique_id = deleted ? "deleted-settings-guid" : "settings-guid";
    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--unique-id", unique_id,
            "--property-name", "ORIENTATION",
            "--property-value", updated_orientation,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable orientation field update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable orientation field update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto orientation_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = "ORIENTATION"
    });
    expect(orientation_property.ok && orientation_property.exists &&
               orientation_property.value == updated_orientation,
           issue_prefix + " update should persist the ORIENTATION field");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(update_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " update should retain label identity");
    }
    expect_empty_report_layout_preview_bounds(
        update_process.stdout_text,
        issue_prefix + " update");
    expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " update should preserve page setup availability");
    if (!deleted) {
        expect_contains(update_process.stdout_text, "\"orientationCode\": " + updated_orientation,
                        issue_prefix + " update should refresh orientation codes");
        expect_contains(update_process.stdout_text, "\"paperSizeCode\": 1",
                        issue_prefix + " update should preserve paper-size codes");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        issue_prefix + " update should preserve top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        issue_prefix + " update should preserve bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        issue_prefix + " update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        issue_prefix + " update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 6",
                        issue_prefix + " update should preserve setting counts");
    } else {
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 6",
                        issue_prefix + " update should preserve deleted setting counts");
    }
    expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " update should preserve selected-settings availability");
    expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " update should preserve settings selection kind");
    expect_contains_in_order(
        update_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + updated_orientation + "\""
        },
        issue_prefix + " update should refresh selected settings provenance");
}

void run_orientation_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path =
        temp_root / ((deleted ? "deleted_" : "") + std::string("orientation_clear") + extension);
    if (deleted) {
        write_synthetic_report_table_for_stable_deleted_orientation_field_json(asset_path);
    } else {
        write_synthetic_report_table_for_stable_orientation_field_json(asset_path);
    }

    const std::string unique_id = deleted ? "deleted-settings-guid" : "settings-guid";
    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--clear-property",
            "--unique-id", unique_id,
            "--property-name", "ORIENTATION",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable orientation field clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable orientation field clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto orientation_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = "ORIENTATION"
    });
    expect(orientation_property.ok && orientation_property.exists && orientation_property.value.empty(),
           issue_prefix + " clear should blank the ORIENTATION field");
    expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " clear should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " clear should retain label identity");
    }
    expect_empty_report_layout_preview_bounds(
        clear_process.stdout_text,
        issue_prefix + " clear");
    expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " clear should preserve page setup availability");
    if (!deleted) {
        expect_contains(clear_process.stdout_text, "\"orientationAvailable\": false",
                        issue_prefix + " clear should clear orientation availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        issue_prefix + " clear should clear orientation codes");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 1",
                        issue_prefix + " clear should preserve paper-size codes");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        issue_prefix + " clear should preserve top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        issue_prefix + " clear should preserve bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        issue_prefix + " clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        issue_prefix + " clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 5",
                        issue_prefix + " clear should remove the direct setting from counts");
    } else {
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 5",
                        issue_prefix + " clear should remove the deleted direct setting from counts");
    }
    expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " clear should preserve selected-settings availability");
    expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " clear should preserve settings selection kind");
    expect_contains_in_order(
        clear_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
        },
        issue_prefix + " clear should preserve remaining selected setting provenance");
    expect_not_contains(clear_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3",
                        issue_prefix + " clear should remove direct ORIENTATION provenance");
}

void run_unsupported_expr_line_preservation_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& updated_driver,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path =
        temp_root / ((deleted ? "deleted_" : "") + std::string("stable_expr_preservation") + extension);
    if (deleted) {
        write_synthetic_report_table_for_stable_deleted_unsupported_expr_line_preservation(asset_path);
    } else {
        write_synthetic_report_table_for_stable_unsupported_expr_line_preservation(asset_path);
    }

    const std::string unique_id = deleted ? "deleted-settings-guid" : "settings-guid";
    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--unique-id", unique_id,
            "--property-name", "DRIVER",
            "--property-value", updated_driver,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable unsupported EXPR-line preservation update " << extension
                  << " stdout:\n" << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable unsupported EXPR-line preservation update " << extension
                  << " stderr:\n" << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists,
           issue_prefix + " update should leave the EXPR memo queryable");
    expect(normalize_line_endings(expr_property.value) ==
               "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1\nDRIVER=" + updated_driver,
           issue_prefix + " update should preserve raw unsupported EXPR lines while appending DRIVER");
    if (!deleted) {
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"value\": \"" + updated_driver + "\""
            },
            issue_prefix + " update should preserve parsed setting source-line gaps around unsupported EXPR lines");
        expect_not_contains(update_process.stdout_text,
                            "\"name\": \"* keep-this-comment\", \"recordIndex\": 0, \"fieldIndex\": 2",
                            issue_prefix + " update should not fabricate comment lines as parsed settings");
    } else {
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " update should expose effective deleted-root page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 4",
                        issue_prefix + " update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"value\": \"" + updated_driver + "\""
            },
            issue_prefix + " update should preserve deleted parsed setting source-line gaps around unsupported EXPR lines");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"value\": \"" + updated_driver + "\""
            },
            issue_prefix + " update should refresh selected deleted parsed settings after EXPR preservation");
        expect_not_contains(update_process.stdout_text,
                            "\"name\": \"* keep-this-comment\", \"recordIndex\": 0, \"fieldIndex\": 2",
                            issue_prefix + " update should not fabricate deleted comment lines as parsed settings");
    }

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--clear-property",
            "--unique-id", unique_id,
            "--property-name", "DRIVER",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable unsupported EXPR-line preservation clear " << extension
                  << " stdout:\n" << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable unsupported EXPR-line preservation clear " << extension
                  << " stderr:\n" << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto reverted_expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = "EXPR"
    });
    expect(reverted_expr_property.ok && reverted_expr_property.exists,
           issue_prefix + " clear should leave the EXPR memo queryable");
    expect(normalize_line_endings(reverted_expr_property.value) ==
               "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1",
           issue_prefix + " clear should restore the raw unsupported EXPR lines");
    if (!deleted) {
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should restore parsed setting source-line gaps around unsupported EXPR lines");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                            issue_prefix + " clear should remove the appended DRIVER setting");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"* keep-this-comment\", \"recordIndex\": 0, \"fieldIndex\": 2",
                            issue_prefix + " clear should not fabricate comment lines as parsed settings");
    } else {
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " clear should preserve effective deleted-root page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should keep live settings absent");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 3",
                        issue_prefix + " clear should restore deleted setting counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should restore deleted parsed setting source-line gaps around unsupported EXPR lines");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should refresh selected deleted parsed settings after EXPR preservation");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                            issue_prefix + " clear should remove the appended deleted DRIVER setting");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"* keep-this-comment\", \"recordIndex\": 0, \"fieldIndex\": 2",
                            issue_prefix + " clear should not fabricate deleted comment lines as parsed settings");
    }
}

void test_studio_host_json_preserves_orientation_stable_settings(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_orientation_stable_settings_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_orientation_update_case(
        studio_host_path,
        temp_root,
        ".frx",
        "orientation_update.frx",
        "1",
        false,
        "report",
        "#2774: report/label stable orientation settings success");
    run_orientation_update_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "orientation_update.lbx",
        "2",
        false,
        "label",
        "#2774: report/label stable orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        ".frx",
        "orientation_clear.frx",
        false,
        "report",
        "#2774: report/label stable orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "orientation_clear.lbx",
        false,
        "label",
        "#2774: report/label stable orientation settings success");
    run_orientation_update_case(
        studio_host_path,
        temp_root,
        ".frx",
        "deleted_orientation_update.frx",
        "1",
        true,
        "report",
        "#2774: report/label stable deleted orientation settings success");
    run_orientation_update_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "deleted_orientation_update.lbx",
        "2",
        true,
        "label",
        "#2774: report/label stable deleted orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        ".frx",
        "deleted_orientation_clear.frx",
        true,
        "report",
        "#2774: report/label stable deleted orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "deleted_orientation_clear.lbx",
        true,
        "label",
        "#2774: report/label stable deleted orientation settings success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".frx",
        "cups",
        false,
        "report",
        "#3094: report/label stable EXPR unsupported-line preservation success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "cupslbl",
        false,
        "label",
        "#3094: report/label stable EXPR unsupported-line preservation success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".frx",
        "cups",
        true,
        "report",
        "#3094: report/label stable deleted EXPR unsupported-line preservation success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "cupslbl",
        true,
        "label",
        "#3094: report/label stable deleted EXPR unsupported-line preservation success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_orientation_stable_settings <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_orientation_stable_settings(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
