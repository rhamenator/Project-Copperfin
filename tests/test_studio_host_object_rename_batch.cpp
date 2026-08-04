// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

std::string visual_object_order(const std::filesystem::path& asset_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(asset_path.string());
    if (!list_result.ok) {
        return {};
    }
    std::string value;
    for (const auto& object : list_result.objects) {
        if (!value.empty()) {
            value += ",";
        }
        value += object.unique_id;
    }
    return value;
}

std::size_t visual_object_count(const std::filesystem::path& asset_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(asset_path.string());
    if (!list_result.ok) {
        return 0U;
    }
    return list_result.objects.size();
}

bool visual_object_exists(const std::filesystem::path& asset_path, const std::string& unique_id) {
    const auto list_result = copperfin::vfp::list_visual_objects(asset_path.string());
    if (!list_result.ok) {
        return false;
    }
    for (const auto& object : list_result.objects) {
        if (object.unique_id == unique_id) {
            return true;
        }
    }
    return false;
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

void write_synthetic_report_table_for_layout_reorder_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", ""},
        {"8", "0", "left.value", "100", "2600", "50", "200", "left-field-guid"},
        {"8", "0", "middle.value", "100", "2600", "50", "200", "middle-field-guid"},
        {"8", "0", "right.value", "100", "2600", "50", "200", "right-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2762: rename-batch success layout fixture should be created");
}

void run_rename_batch_success_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("object_rename_batch" + extension);
    write_synthetic_report_table_for_layout_reorder_json(asset_path);

    const std::size_t before_count = visual_object_count(asset_path);

    const auto rename_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", asset_path.string(),
            "--selected-unique-id", "left-field-guid",
            "--new-unique-id", "left-renamed-guid",
            "--selected-unique-id", "right-field-guid",
            "--new-unique-id", "right-renamed-guid",
            "--json"
        },
        temp_root);

    if (rename_batch_process.exit_code != 0) {
        std::cerr << "studio host " << label << " rename-batch stdout:\n"
                  << rename_batch_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " rename-batch stderr:\n"
                  << rename_batch_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(rename_batch_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(rename_batch_process.stdout_text, "\"visualObjectRenameBatch\": {",
                    issue_prefix + " should expose a batch object");
    expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                    issue_prefix + " should expose affected object counts");
    expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
                    issue_prefix + " should expose committed state");
    expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
                    issue_prefix + " should expose mutation state");
    expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
                    issue_prefix + " should expose undo availability");
    expect_contains(rename_batch_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                    issue_prefix + " should expose renamed-identity undo labels");
    expect(visual_object_count(asset_path) == before_count &&
               !visual_object_exists(asset_path, "left-field-guid") &&
               !visual_object_exists(asset_path, "right-field-guid") &&
               visual_object_exists(asset_path, "left-renamed-guid") &&
               visual_object_exists(asset_path, "right-renamed-guid") &&
               visual_object_order(asset_path) == "left-renamed-guid,middle-field-guid,right-renamed-guid",
           issue_prefix + " should replace identities without changing object order");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "left-renamed-guid", "--json"},
        temp_root);

    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host " << label << " rename-batch reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " rename-batch reopen stderr:\n"
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
    expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                    issue_prefix + " should preserve live preview width");
    expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                    issue_prefix + " should preserve live preview height");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    issue_prefix + " should not fabricate deleted preview availability");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " should preserve zero deleted preview left bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    issue_prefix + " should preserve zero deleted preview top bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    issue_prefix + " should preserve zero deleted preview right bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                    issue_prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    issue_prefix + " should preserve zero deleted preview width");
    expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                    issue_prefix + " should preserve zero deleted preview height");
    expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"left-renamed-guid\"",
                    issue_prefix + " should preserve selected object identity after reopen");
    expect_contains_in_order(
        reopen_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 2",
            "\"sectionObjectIndex\": 0",
            "\"objectKind\": \"field\"",
            "\"expression\": \"left.value\""
        },
        issue_prefix + " should refresh selected renamed object metadata after reopen");
}

void test_studio_host_json_preserves_object_rename_batch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_object_rename_batch_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_rename_batch_success_case(
        studio_host_path,
        temp_root,
        ".frx",
        "object_rename_batch.frx",
        "report",
        "#2762: report/label rename-batch success");
    run_rename_batch_success_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "object_rename_batch.lbx",
        "label",
        "#2762: report/label rename-batch success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_object_rename_batch <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_object_rename_batch(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
