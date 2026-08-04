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

void expect_full_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 8100",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 8100",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1000",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 2200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 2900",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 1200",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
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

void write_synthetic_report_table_for_layout_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2778: synthetic report/label layout settings fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2778: synthetic report/label layout settings fixture should mark deleted objects");
}

void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2778: deleted settings fixture should mark report settings deleted");
}

void run_page_margin_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& updated_margin,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path =
        temp_root / ((deleted ? "deleted_" : "") + std::string("page_margin_update") + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_settings_json(asset_path);
    } else {
        write_synthetic_report_table_for_layout_json(asset_path);
    }

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "TOPMARGIN",
            "--property-value", updated_margin,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " record page margin field update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " record page margin field update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto margin_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "TOPMARGIN"
    });
    expect(margin_property.ok && margin_property.exists && margin_property.value == updated_margin,
           issue_prefix + " update should persist the TOPMARGIN field");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    expect_full_report_layout_preview_bounds(update_process.stdout_text, issue_prefix + " update");
    if (!deleted) {
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": " + updated_margin,
                        issue_prefix + " update should refresh top margins");
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
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should preserve field setting provenance");
    } else {
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " update should expose effective deleted-root page setup");
        expect_contains(update_process.stdout_text, "\"topMarginAvailable\": true",
                        issue_prefix + " update should expose effective top-margin availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": " + updated_margin,
                        issue_prefix + " update should expose the effective top margin");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 6",
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
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            issue_prefix + " update should refresh deleted direct-field provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            issue_prefix + " update should refresh selected deleted settings");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 2",
                        issue_prefix + " update should preserve live section metadata");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        issue_prefix + " update should preserve deleted object metadata");
    }
}

void run_page_margin_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path =
        temp_root / ((deleted ? "deleted_" : "") + std::string("page_margin_clear") + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_settings_json(asset_path);
    } else {
        write_synthetic_report_table_for_layout_json(asset_path);
    }

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "TOPMARGIN",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " record page margin field clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " record page margin field clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto margin_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "TOPMARGIN"
    });
    expect(margin_property.ok && margin_property.exists && margin_property.value.empty(),
           issue_prefix + " clear should blank the TOPMARGIN field");
    expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " clear should return refreshed report-layout JSON");
    expect_full_report_layout_preview_bounds(clear_process.stdout_text, issue_prefix + " clear");
    if (!deleted) {
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        issue_prefix + " clear should preserve memo-derived orientation codes");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 1",
                        issue_prefix + " clear should preserve memo-derived paper-size codes");
        expect_contains(clear_process.stdout_text, "\"topMarginAvailable\": false",
                        issue_prefix + " clear should clear top-margin availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 0",
                        issue_prefix + " clear should clear top margins");
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
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should preserve remaining setting provenance");
    } else {
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        issue_prefix + " clear should preserve effective deleted-root page setup");
        expect_contains(clear_process.stdout_text, "\"topMarginAvailable\": false",
                        issue_prefix + " clear should clear effective top-margin availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 0",
                        issue_prefix + " clear should reset the effective top margin");
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
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should preserve remaining deleted setting provenance");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            issue_prefix + " clear should refresh selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8",
                            issue_prefix + " clear should remove direct top-margin settings");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 2",
                        issue_prefix + " clear should preserve live section metadata");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        issue_prefix + " clear should preserve deleted object metadata");
    }
}

void test_studio_host_json_preserves_page_margin_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_page_margin_record_settings_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_page_margin_update_case(
        studio_host_path,
        temp_root,
        ".frx",
        "page_margin_update.frx",
        "24",
        false,
        "report",
        "#2778: report/label record-selected page-margin settings success");
    run_page_margin_update_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "page_margin_update.lbx",
        "26",
        false,
        "label",
        "#2778: report/label record-selected page-margin settings success");
    run_page_margin_clear_case(
        studio_host_path,
        temp_root,
        ".frx",
        "page_margin_clear.frx",
        false,
        "report",
        "#2778: report/label record-selected page-margin settings success");
    run_page_margin_clear_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "page_margin_clear.lbx",
        false,
        "label",
        "#2778: report/label record-selected page-margin settings success");
    run_page_margin_update_case(
        studio_host_path,
        temp_root,
        ".frx",
        "deleted_page_margin_update.frx",
        "24",
        true,
        "report",
        "#2778: report/label record-selected deleted page-margin settings success");
    run_page_margin_update_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "deleted_page_margin_update.lbx",
        "26",
        true,
        "label",
        "#2778: report/label record-selected deleted page-margin settings success");
    run_page_margin_clear_case(
        studio_host_path,
        temp_root,
        ".frx",
        "deleted_page_margin_clear.frx",
        true,
        "report",
        "#2778: report/label record-selected deleted page-margin settings success");
    run_page_margin_clear_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "deleted_page_margin_clear.lbx",
        true,
        "label",
        "#2778: report/label record-selected deleted page-margin settings success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_page_margin_record_settings <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_page_margin_record_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
