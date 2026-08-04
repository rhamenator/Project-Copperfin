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
    expect(create_result.ok, "#2793: section restore layout fixture should be created");
}

void write_synthetic_report_table_for_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#2793: section restore fixture should mark section deleted");
}

void run_section_restore_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_deleted_section_json(asset_path);
    expect(dbf_record_deleted(asset_path, 1U), issue_prefix + " fixture should start with a deleted section");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--restore-object",
            "--record", "1",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host " << label << " section restore stdout:\n" << restore_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " section restore stderr:\n" << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect(!dbf_record_deleted(asset_path, 1U), issue_prefix + " should clear the section record delete flag");
    if (asset_path.extension() == ".lbx") {
        expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(restore_process.stdout_text, "\"sectionCount\": 1",
                    issue_prefix + " should restore live section counts");
    expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                    issue_prefix + " should clear deleted section counts");
    if (asset_path.extension() == ".lbx") {
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        issue_prefix + " should expose live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        issue_prefix + " should preserve live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 2000",
                        issue_prefix + " should preserve live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 150",
                        issue_prefix + " should preserve live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        issue_prefix + " should preserve live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 150",
                        issue_prefix + " should preserve live preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        issue_prefix + " should preserve live preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        issue_prefix + " should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        issue_prefix + " should advertise selected-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        issue_prefix + " should expose section selection kind");
    }
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"sections\": [",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 1",
            "\"bandKind\": \"detail\"",
            "\"objectCount\": 3"
        },
        issue_prefix + " should move the section back into live-section metadata");
    expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                    issue_prefix + " should move formerly unplaced objects back into section membership");
    expect_contains(restore_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    issue_prefix + " should expose containing-section ids again");
}

void test_studio_host_json_preserves_section_restore_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_section_restore_record_selection_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_section_restore_case(
        studio_host_path,
        temp_root,
        "summary.frx",
        "report",
        "#2793: record-selected report section restore");
    run_section_restore_case(
        studio_host_path,
        temp_root,
        "mailing.lbx",
        "label",
        "#2793: record-selected label section restore");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_section_restore_record_selection <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_section_restore_record_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
