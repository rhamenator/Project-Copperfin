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
    expect(create_result.ok, "#2782: orientation settings fixture should be created");
}

void write_synthetic_report_table_for_deleted_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_orientation_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2782: deleted orientation settings fixture should mark settings deleted");
}

void write_synthetic_report_table_for_memo_backed_orientation_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nTOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3063: memo-backed orientation settings fixture should be created");
}

void run_orientation_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    const std::string& updated_orientation,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_orientation_field_json(asset_path);
    } else {
        write_synthetic_report_table_for_orientation_field_json(asset_path);
    }

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "ORIENTATION",
            "--property-value", updated_orientation,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " orientation field update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " orientation field update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto orientation_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "ORIENTATION"
    });
    expect(orientation_property.ok && orientation_property.exists &&
               orientation_property.value == updated_orientation,
           issue_prefix + " update should persist the ORIENTATION field");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    expect_empty_report_layout_preview_bounds(update_process.stdout_text, issue_prefix + " update");
    if (!deleted) {
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"orientationCode\": " + updated_orientation,
                        issue_prefix + " update should refresh orientation codes");
        expect_contains(update_process.stdout_text, "\"paperSizeCode\": 1",
                        issue_prefix + " update should preserve memo-derived paper-size codes");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        issue_prefix + " update should preserve memo-derived top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        issue_prefix + " update should preserve memo-derived bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        issue_prefix + " update should preserve memo-derived vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        issue_prefix + " update should preserve memo-derived horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 6",
                        issue_prefix + " update should preserve setting counts");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should preserve field setting provenance");
    } else {
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " update should expose effective deleted-root page setup");
        expect_contains(update_process.stdout_text, "\"orientationAvailable\": true",
                        issue_prefix + " update should expose effective orientation availability");
        expect_contains(update_process.stdout_text, "\"orientationCode\": " + updated_orientation,
                        issue_prefix + " update should expose the effective orientation code");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 6",
                        issue_prefix + " update should refresh deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_orientation + "\""
            },
            issue_prefix + " update should refresh deleted setting provenance");
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
            issue_prefix + " update should refresh selected deleted settings");
    }
}

void run_orientation_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_orientation_field_json(asset_path);
    } else {
        write_synthetic_report_table_for_orientation_field_json(asset_path);
    }

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "ORIENTATION",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " orientation field clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " orientation field clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto orientation_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "ORIENTATION"
    });
    expect(orientation_property.ok && orientation_property.exists && orientation_property.value.empty(),
           issue_prefix + " clear should blank the ORIENTATION field");
    expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " clear should return refreshed report-layout JSON");
    expect_empty_report_layout_preview_bounds(clear_process.stdout_text, issue_prefix + " clear");
    if (!deleted) {
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"orientationAvailable\": false",
                        issue_prefix + " clear should clear orientation availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        issue_prefix + " clear should clear orientation codes");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 1",
                        issue_prefix + " clear should preserve memo-derived paper-size codes");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        issue_prefix + " clear should preserve memo-derived top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        issue_prefix + " clear should preserve memo-derived bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        issue_prefix + " clear should preserve memo-derived vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        issue_prefix + " clear should preserve memo-derived horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 5",
                        issue_prefix + " clear should remove the direct setting from counts");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should preserve remaining setting provenance");
    } else {
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " clear should preserve effective deleted-root page setup");
        expect_contains(clear_process.stdout_text, "\"orientationAvailable\": false",
                        issue_prefix + " clear should clear effective orientation availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        issue_prefix + " clear should reset the effective orientation code");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 5",
                        issue_prefix + " clear should refresh deleted setting counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should preserve remaining deleted setting provenance");
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
            issue_prefix + " clear should refresh selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3",
            issue_prefix + " clear should remove direct orientation settings");
    }
}

void run_memo_backed_orientation_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& updated_orientation,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("memo_orientation_update" + extension);
    write_synthetic_report_table_for_memo_backed_orientation_json(asset_path);

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "ORIENTATION",
            "--property-value", updated_orientation,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " memo-backed orientation update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " memo-backed orientation update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto orientation_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "ORIENTATION"
    });
    expect(orientation_property.ok && orientation_property.exists &&
               orientation_property.value == updated_orientation &&
               !orientation_property.direct_field,
           issue_prefix + " update should persist memo-backed ORIENTATION through property query");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(update_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " update should retain label identity");
    }
    expect_empty_report_layout_preview_bounds(update_process.stdout_text, issue_prefix + " update");
    expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " update should preserve page setup availability");
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
    expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " update should preserve selected-settings availability");
    expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " update should preserve settings selection kind");
    expect_contains_in_order(
        update_process.stdout_text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"value\": \"" + updated_orientation + "\"",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
            "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5"
        },
        issue_prefix + " update should preserve memo-backed setting provenance");
    expect_contains_in_order(
        update_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"value\": \"" + updated_orientation + "\"",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
            "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5"
        },
        issue_prefix + " update should refresh selected memo-backed settings");
}

void write_synthetic_report_table_for_memo_backed_driver_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nCOLOR=1", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3064: memo-backed driver settings fixture should be created");
}

std::string normalize_line_endings(std::string text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == '\r') {
            if ((index + 1U) < text.size() && text[index + 1U] == '\n') {
                ++index;
            }
            normalized.push_back('\n');
            continue;
        }

        normalized.push_back(text[index]);
    }

    return normalized;
}

void write_synthetic_report_table_for_unsupported_expr_line_preservation(const std::filesystem::path& report_path) {
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
    expect(create_result.ok, "#3065: unsupported EXPR-line preservation fixture should be created");
}

void write_synthetic_report_table_for_deleted_unsupported_expr_line_preservation(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_unsupported_expr_line_preservation(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#3093: deleted unsupported EXPR-line preservation fixture should mark settings deleted");
}

void run_memo_backed_driver_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& updated_driver,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("memo_driver_update" + extension);
    write_synthetic_report_table_for_memo_backed_driver_json(asset_path);

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "DRIVER",
            "--property-value", updated_driver,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " memo-backed driver update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " memo-backed driver update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto driver_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "DRIVER"
    });
    expect(driver_property.ok && driver_property.exists &&
               driver_property.value == updated_driver &&
               !driver_property.direct_field,
           issue_prefix + " update should persist memo-backed DRIVER through property query");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(update_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " update should retain label identity");
    }
    expect_empty_report_layout_preview_bounds(update_process.stdout_text, issue_prefix + " update");
    expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " update should preserve page setup availability");
    expect_contains(update_process.stdout_text, "\"orientationCode\": 0",
                    issue_prefix + " update should preserve orientation codes");
    expect_contains(update_process.stdout_text, "\"paperSizeCode\": 1",
                    issue_prefix + " update should preserve paper-size codes");
    expect_contains(update_process.stdout_text, "\"settingCount\": 4",
                    issue_prefix + " update should preserve setting counts");
    expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " update should preserve selected-settings availability");
    expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " update should preserve settings selection kind");
    expect_contains_in_order(
        update_process.stdout_text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
            "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"value\": \"" + updated_driver + "\""
        },
        issue_prefix + " update should append memo-backed DRIVER without losing existing settings");
    expect_contains_in_order(
        update_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
            "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"value\": \"" + updated_driver + "\""
        },
        issue_prefix + " update should refresh selected memo-backed DRIVER settings");

    const auto undo_process = run_process_capture(
        studio_host_path,
        {
            "--from-vs",
            "--json",
            "--undo-mode", "command",
            "--record", "0",
            "--path", asset_path.string()
        },
        temp_root);

    if (undo_process.exit_code != 0) {
        std::cerr << "studio host " << label << " memo-backed driver undo " << extension << " stdout:\n"
                  << undo_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " memo-backed driver undo " << extension << " stderr:\n"
                  << undo_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(undo_process.exit_code == 0, issue_prefix + " undo should exit successfully");
    const auto reverted_driver_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "DRIVER"
    });
    expect(reverted_driver_property.ok && !reverted_driver_property.exists &&
               reverted_driver_property.value.empty() &&
               !reverted_driver_property.direct_field,
           issue_prefix + " undo should remove the memo-backed DRIVER addition");
    expect_contains(undo_process.stdout_text, "\"settingCount\": 3",
                    issue_prefix + " undo should restore original setting counts");
    expect_contains_in_order(
        undo_process.stdout_text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
        },
        issue_prefix + " undo should preserve original memo-backed settings");
    expect_not_contains(undo_process.stdout_text,
                        "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                        issue_prefix + " undo should remove memo-backed DRIVER provenance");
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
        temp_root / ((deleted ? "deleted_" : "") + std::string("expr_preservation") + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_unsupported_expr_line_preservation(asset_path);
    } else {
        write_synthetic_report_table_for_unsupported_expr_line_preservation(asset_path);
    }

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "DRIVER",
            "--property-value", updated_driver,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " unsupported EXPR-line preservation update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " unsupported EXPR-line preservation update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
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
                "\"settings\": [",
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

    const auto undo_process = run_process_capture(
        studio_host_path,
        {
            "--from-vs",
            "--json",
            "--undo-mode", "command",
            "--record", "0",
            "--path", asset_path.string()
        },
        temp_root);

    if (undo_process.exit_code != 0) {
        std::cerr << "studio host " << label << " unsupported EXPR-line preservation undo " << extension << " stdout:\n"
                  << undo_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " unsupported EXPR-line preservation undo " << extension << " stderr:\n"
                  << undo_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(undo_process.exit_code == 0, issue_prefix + " undo should exit successfully");
    const auto reverted_expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(reverted_expr_property.ok && reverted_expr_property.exists,
           issue_prefix + " undo should leave the EXPR memo queryable");
    expect(normalize_line_endings(reverted_expr_property.value) ==
               "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1",
           issue_prefix + " undo should restore the raw unsupported EXPR lines");
    if (!deleted) {
        expect_contains_in_order(
            undo_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " undo should restore parsed setting source-line gaps around unsupported EXPR lines");
        expect_not_contains(undo_process.stdout_text,
                            "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                            issue_prefix + " undo should remove the appended DRIVER setting");
        expect_not_contains(undo_process.stdout_text,
                            "\"name\": \"* keep-this-comment\", \"recordIndex\": 0, \"fieldIndex\": 2",
                            issue_prefix + " undo should not fabricate comment lines as parsed settings");
    } else {
        expect_contains(undo_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " undo should preserve effective deleted-root page setup");
        expect_contains(undo_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " undo should keep live settings absent");
        expect_contains(undo_process.stdout_text, "\"deletedSettingCount\": 3",
                        issue_prefix + " undo should restore deleted setting counts");
        expect_contains(undo_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " undo should preserve selected-settings availability");
        expect_contains(undo_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " undo should preserve settings selection kind");
        expect_contains_in_order(
            undo_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " undo should restore deleted parsed setting source-line gaps around unsupported EXPR lines");
        expect_contains_in_order(
            undo_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " undo should refresh selected deleted parsed settings after EXPR preservation");
        expect_not_contains(undo_process.stdout_text,
                            "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                            issue_prefix + " undo should remove the appended deleted DRIVER setting");
        expect_not_contains(undo_process.stdout_text,
                            "\"name\": \"* keep-this-comment\", \"recordIndex\": 0, \"fieldIndex\": 2",
                            issue_prefix + " undo should not fabricate deleted comment lines as parsed settings");
    }
}

void test_studio_host_json_preserves_orientation_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_orientation_record_settings_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_orientation_update_case(
        studio_host_path,
        temp_root,
        "orientation_update",
        ".frx",
        "1",
        false,
        "report",
        "#2782: report/label record-selected orientation settings success");
    run_orientation_update_case(
        studio_host_path,
        temp_root,
        "orientation_update",
        ".lbx",
        "2",
        false,
        "label",
        "#2782: report/label record-selected orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        "orientation_clear",
        ".frx",
        false,
        "report",
        "#2782: report/label record-selected orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        "orientation_clear",
        ".lbx",
        false,
        "label",
        "#2782: report/label record-selected orientation settings success");
    run_orientation_update_case(
        studio_host_path,
        temp_root,
        "deleted_orientation_update",
        ".frx",
        "1",
        true,
        "report",
        "#2782: report/label record-selected deleted orientation settings success");
    run_orientation_update_case(
        studio_host_path,
        temp_root,
        "deleted_orientation_update",
        ".lbx",
        "2",
        true,
        "label",
        "#2782: report/label record-selected deleted orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        "deleted_orientation_clear",
        ".frx",
        true,
        "report",
        "#2782: report/label record-selected deleted orientation settings success");
    run_orientation_clear_case(
        studio_host_path,
        temp_root,
        "deleted_orientation_clear",
        ".lbx",
        true,
        "label",
        "#2782: report/label record-selected deleted orientation settings success");
    run_memo_backed_orientation_update_case(
        studio_host_path,
        temp_root,
        ".frx",
        "1",
        "report",
        "#3063: report/label record-selected memo-backed orientation settings success");
    run_memo_backed_orientation_update_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "2",
        "label",
        "#3063: report/label record-selected memo-backed orientation settings success");
    run_memo_backed_driver_update_case(
        studio_host_path,
        temp_root,
        ".frx",
        "cups",
        "report",
        "#3064: report/label record-selected memo-backed driver settings success");
    run_memo_backed_driver_update_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "cupslbl",
        "label",
        "#3064: report/label record-selected memo-backed driver settings success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".frx",
        "cups",
        false,
        "report",
        "#3065: report/label record-selected EXPR unsupported-line preservation success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "cupslbl",
        false,
        "label",
        "#3065: report/label record-selected EXPR unsupported-line preservation success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".frx",
        "cups",
        true,
        "report",
        "#3093: report/label record-selected deleted EXPR unsupported-line preservation success");
    run_unsupported_expr_line_preservation_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "cupslbl",
        true,
        "label",
        "#3093: report/label record-selected deleted EXPR unsupported-line preservation success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_orientation_record_settings <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_orientation_record_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
