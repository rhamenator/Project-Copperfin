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
    expect(create_result.ok, "#2759: live update-batch layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2759: live update-batch fixture should mark deleted layout objects");
}

std::string visual_object_property(
    const std::filesystem::path& asset_path,
    const std::string& unique_id,
    const std::string& property_name) {
    const auto property_result = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = property_name
    });
    if (!property_result.ok || !property_result.exists) {
        return {};
    }
    return property_result.value;
}

void run_update_batch_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("report_object_update_batch" + extension);
    write_synthetic_report_table_for_layout_json(asset_path);

    const auto update_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", asset_path.string(),
            "--selected-unique-id", "field-guid",
            "--property-name", "EXPR",
            "--property-value", "customer.contact",
            "--property-name", "WIDTH",
            "--property-value", "4300",
            "--selected-unique-id", "label-guid",
            "--property-name", "EXPR",
            "--property-value", "\"Updated invoice\"",
            "--property-name", "HPOS",
            "--property-value", "720",
            "--json"
        },
        temp_root);

    if (update_batch_process.exit_code != 0) {
        std::cerr << "studio host " << label << " update-batch stdout:\n"
                  << update_batch_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " update-batch stderr:\n"
                  << update_batch_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_batch_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(update_batch_process.stdout_text, "\"visualObjectUpdateBatch\": {",
                    issue_prefix + " should expose a batch object");
    expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                    issue_prefix + " should expose affected object counts");
    expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
                    issue_prefix + " should expose committed state");
    expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
                    issue_prefix + " should expose mutation state");
    expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
                    issue_prefix + " should expose undo availability");
    expect_contains(update_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                    issue_prefix + " should expose the latest undo label");
    expect(visual_object_property(asset_path, "field-guid", "EXPR") == "customer.contact" &&
               visual_object_property(asset_path, "field-guid", "WIDTH") == "4300" &&
               visual_object_property(asset_path, "label-guid", "EXPR") == "\"Updated invoice\"" &&
               visual_object_property(asset_path, "label-guid", "HPOS") == "720",
           issue_prefix + " should persist direct and memo-backed layout properties");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "field-guid", "--json"},
        temp_root);

    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host " << label << " update-batch reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " update-batch reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(reopen_process.exit_code == 0, issue_prefix + " reopen should exit successfully");
    expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " should leave report-layout JSON readable");
    if (extension == ".lbx") {
        expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 5500",
                    issue_prefix + " should refresh live preview right bounds");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 5500",
                    issue_prefix + " should refresh live preview width");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " should preserve live preview height");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should preserve deleted preview availability");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    issue_prefix + " should preserve deleted preview width");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    issue_prefix + " should preserve deleted preview height");
    expect_contains_in_order(
        reopen_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"width\": 4300",
            "\"right\": 5500",
            "\"objectKind\": \"field\"",
            "\"expression\": \"customer.contact\""
        },
        issue_prefix + " should refresh selected object metadata after reopen");

    const auto undo_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--undo-mode", "command",
            "--json"
        },
        temp_root);

    if (undo_process.exit_code != 0) {
        std::cerr << "studio host " << label << " update-batch undo stdout:\n"
                  << undo_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " update-batch undo stderr:\n"
                  << undo_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(undo_process.exit_code == 0, issue_prefix + " undo should exit successfully");
    expect(visual_object_property(asset_path, "field-guid", "EXPR") == "customer.company" &&
               visual_object_property(asset_path, "field-guid", "WIDTH") == "4000" &&
               visual_object_property(asset_path, "label-guid", "EXPR") == "\"Invoice\"" &&
               visual_object_property(asset_path, "label-guid", "HPOS") == "900",
           issue_prefix + " should restore all batch-updated properties through one command undo");
}

void test_studio_host_json_preserves_update_batch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_update_batch_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_update_batch_case(
        studio_host_path,
        temp_root,
        ".frx",
        "report_object_update_batch.frx",
        "report",
        "#2759: report/label stable visual-object update-batch");
    run_update_batch_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "report_object_update_batch.lbx",
        "label",
        "#2759: report/label stable visual-object update-batch");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_update_batch <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_update_batch(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
