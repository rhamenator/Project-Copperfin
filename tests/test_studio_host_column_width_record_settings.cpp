// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

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

void write_synthetic_report_table_for_column_width_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "COLWIDTH", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "COLS=2\nCOLSPACING=120", "3600", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2786: column-width settings fixture should be created");
}

void write_synthetic_report_table_for_deleted_column_width_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_width_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2786: deleted column-width settings fixture should mark settings deleted");
}

void run_column_width_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    const std::string& updated_width,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_column_width_field_json(asset_path);
    } else {
        write_synthetic_report_table_for_column_width_field_json(asset_path);
    }

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "COLWIDTH",
            "--property-value", updated_width,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " column width field update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " column width field update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto column_width_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "COLWIDTH"
    });
    expect(column_width_property.ok && column_width_property.exists &&
               column_width_property.value == updated_width,
           issue_prefix + " update should persist the COLWIDTH field");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(update_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " update should retain label identity");
    }
    if (!deleted) {
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2032: record-selected report/label column-width update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " update should not fabricate page setup availability");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        issue_prefix + " update should preserve column setup availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": 2",
                        issue_prefix + " update should preserve memo-derived column counts");
        expect_contains(update_process.stdout_text, "\"columnWidth\": " + updated_width,
                        issue_prefix + " update should refresh column widths");
        expect_contains(update_process.stdout_text, "\"columnSpacing\": 120",
                        issue_prefix + " update should preserve memo-derived column spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 3",
                        issue_prefix + " update should preserve setting counts");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should preserve field setting provenance");
    } else {
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2032: record-selected deleted report/label column-width update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        issue_prefix + " update should expose effective deleted-root column setup");
        expect_contains(update_process.stdout_text, "\"columnWidthAvailable\": true",
                        issue_prefix + " update should expose effective column-width availability");
        expect_contains(update_process.stdout_text, "\"columnWidth\": " + updated_width,
                        issue_prefix + " update should expose the effective column width");
        expect_contains(update_process.stdout_text, "\"columnCountAvailable\": true",
                        issue_prefix + " update should preserve effective column-count availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": 2",
                        issue_prefix + " update should preserve the effective column count");
        expect_contains(update_process.stdout_text, "\"columnSpacingAvailable\": true",
                        issue_prefix + " update should preserve effective column-spacing availability");
        expect_contains(update_process.stdout_text, "\"columnSpacing\": 120",
                        issue_prefix + " update should preserve the effective column spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 3",
                        issue_prefix + " update should refresh deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            issue_prefix + " update should refresh deleted setting provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            issue_prefix + " update should refresh selected deleted settings");
    }
}

void run_column_width_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_column_width_field_json(asset_path);
    } else {
        write_synthetic_report_table_for_column_width_field_json(asset_path);
    }

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "COLWIDTH",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " column width field clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " column width field clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto column_width_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "COLWIDTH"
    });
    expect(column_width_property.ok && column_width_property.exists &&
               column_width_property.value.empty(),
           issue_prefix + " clear should blank the COLWIDTH field");
    expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " clear should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " clear should retain label identity");
    }
    if (!deleted) {
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2032: record-selected report/label column-width clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " clear should not fabricate page setup availability");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        issue_prefix + " clear should preserve column setup availability");
        expect_contains(clear_process.stdout_text, "\"columnCount\": 2",
                        issue_prefix + " clear should preserve memo-derived column counts");
        expect_contains(clear_process.stdout_text, "\"columnWidthAvailable\": false",
                        issue_prefix + " clear should clear column-width availability");
        expect_contains(clear_process.stdout_text, "\"columnWidth\": 0",
                        issue_prefix + " clear should clear column widths");
        expect_contains(clear_process.stdout_text, "\"columnSpacing\": 120",
                        issue_prefix + " clear should preserve memo-derived column spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 2",
                        issue_prefix + " clear should remove the direct setting from counts");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            issue_prefix + " clear should preserve remaining setting provenance");
    } else {
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2032: record-selected deleted report/label column-width clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        issue_prefix + " clear should preserve effective deleted-root column setup");
        expect_contains(clear_process.stdout_text, "\"columnWidthAvailable\": false",
                        issue_prefix + " clear should clear effective column-width availability");
        expect_contains(clear_process.stdout_text, "\"columnWidth\": 0",
                        issue_prefix + " clear should reset the effective column width");
        expect_contains(clear_process.stdout_text, "\"columnCountAvailable\": true",
                        issue_prefix + " clear should preserve effective column-count availability");
        expect_contains(clear_process.stdout_text, "\"columnCount\": 2",
                        issue_prefix + " clear should preserve the effective column count");
        expect_contains(clear_process.stdout_text, "\"columnSpacingAvailable\": true",
                        issue_prefix + " clear should preserve effective column-spacing availability");
        expect_contains(clear_process.stdout_text, "\"columnSpacing\": 120",
                        issue_prefix + " clear should preserve the effective column spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 2",
                        issue_prefix + " clear should refresh deleted setting counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            issue_prefix + " clear should preserve remaining deleted setting provenance");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            issue_prefix + " clear should refresh selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            issue_prefix + " clear should remove direct column-width settings");
    }
}

void test_studio_host_json_preserves_column_width_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_column_width_record_settings_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_column_width_update_case(
        studio_host_path,
        temp_root,
        "column_width",
        ".frx",
        "2800",
        false,
        "report",
        "#2786: report/label record-selected column-width settings success");
    run_column_width_update_case(
        studio_host_path,
        temp_root,
        "column_width",
        ".lbx",
        "3000",
        false,
        "label",
        "#2786: report/label record-selected column-width settings success");
    run_column_width_clear_case(
        studio_host_path,
        temp_root,
        "column_width_clear",
        ".frx",
        false,
        "report",
        "#2786: report/label record-selected column-width settings success");
    run_column_width_clear_case(
        studio_host_path,
        temp_root,
        "column_width_clear",
        ".lbx",
        false,
        "label",
        "#2786: report/label record-selected column-width settings success");
    run_column_width_update_case(
        studio_host_path,
        temp_root,
        "deleted_column_width",
        ".frx",
        "4800",
        true,
        "report",
        "#2786: report/label record-selected deleted column-width settings success");
    run_column_width_update_case(
        studio_host_path,
        temp_root,
        "deleted_column_width",
        ".lbx",
        "2400",
        true,
        "label",
        "#2786: report/label record-selected deleted column-width settings success");
    run_column_width_clear_case(
        studio_host_path,
        temp_root,
        "deleted_column_width_clear",
        ".frx",
        true,
        "report",
        "#2786: report/label record-selected deleted column-width settings success");
    run_column_width_clear_case(
        studio_host_path,
        temp_root,
        "deleted_column_width_clear",
        ".lbx",
        true,
        "label",
        "#2786: report/label record-selected deleted column-width settings success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_column_width_record_settings <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_column_width_record_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
