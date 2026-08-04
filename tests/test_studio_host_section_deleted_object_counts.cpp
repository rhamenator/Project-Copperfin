// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
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

void write_synthetic_report_table_for_section_deleted_object_count_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "1000", "detail-section-guid"},
        {"9", "8", "", "", "2000", "", "500", "deleted-summary-section-guid"},
        {"8", "0", "detail.value", "100", "200", "400", "100", "detail-field-guid"},
        {"5", "", "\"Deleted detail\"", "150", "300", "200", "100", "deleted-detail-label-guid"},
        {"5", "", "\"Deleted summary\"", "200", "2100", "250", "100", "deleted-summary-label-guid"},
        {"6", "", "", "50", "5000", "100", "100", "deleted-unplaced-line-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#2829: deleted-object-count fixture should be created");

    for (const auto record_index : {2U, 4U, 5U, 6U}) {
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), record_index, true);
        expect(delete_result.ok,
               "#2829: deleted-object-count fixture should mark deleted sections and deleted objects");
    }
}

void run_section_deleted_object_count_json(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_section_deleted_object_count_json(asset_path);

    const auto live_section_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--record", "1", "--json"},
        temp_root);
    if (live_section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " live section deleted-object-count stdout:\n"
                  << live_section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " live section deleted-object-count stderr:\n"
                  << live_section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(live_section_process.exit_code == 0, issue_prefix + " live section query should exit successfully");
    expect_contains(live_section_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " live section query should preserve document titles");
    if (asset_path.extension() == ".lbx") {
        expect_contains(live_section_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " live section query should retain label identity");
    }
    expect_contains_in_order(
        live_section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"detail-section-guid\"",
            "\"deleted\": false",
            "\"objectCount\": 1",
            "\"deletedObjectCount\": 1"
        },
        issue_prefix + " live selected sections should expose deleted placed-object counts");

    const auto deleted_section_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--record", "2", "--json"},
        temp_root);
    if (deleted_section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " deleted section deleted-object-count stdout:\n"
                  << deleted_section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " deleted section deleted-object-count stderr:\n"
                  << deleted_section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(deleted_section_process.exit_code == 0,
           issue_prefix + " deleted section query should exit successfully");
    expect_contains_in_order(
        deleted_section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"deleted-summary-section-guid\"",
            "\"deleted\": true",
            "\"objectCount\": 0",
            "\"deletedObjectCount\": 1"
        },
        issue_prefix + " deleted section arrays should expose deleted placed-object counts");
    expect_contains_in_order(
        deleted_section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"deleted-summary-section-guid\"",
            "\"deleted\": true",
            "\"objectCount\": 0",
            "\"deletedObjectCount\": 1"
        },
        issue_prefix + " deleted selected sections should expose deleted placed-object counts");

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--record", "3", "--json"},
        temp_root);
    if (object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " object-section deleted-object-count stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " object-section deleted-object-count stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(object_process.exit_code == 0,
           issue_prefix + " selected object-section query should exit successfully");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"detail-section-guid\"",
            "\"deleted\": false",
            "\"objectCount\": 1",
            "\"deletedObjectCount\": 1"
        },
        issue_prefix + " selected containing sections should expose deleted placed-object counts");

    const auto deleted_section_object_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--record", "5", "--json"},
        temp_root);
    if (deleted_section_object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " deleted-section object containing-section stdout:\n"
                  << deleted_section_object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " deleted-section object containing-section stderr:\n"
                  << deleted_section_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(deleted_section_object_process.exit_code == 0,
           issue_prefix + " deleted-section object query should exit successfully");
    expect_contains_in_order(
        deleted_section_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 5",
            "\"deleted\": true",
            "\"containingSectionId\": \"deleted-summary-section-guid\"",
            "\"containingSectionRecordIndex\": 2",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 200",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1"
        },
        issue_prefix + " deleted objects inside deleted sections should expose containing-section metadata");
    expect_contains_in_order(
        deleted_section_object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"deleted-summary-section-guid\"",
            "\"deleted\": true",
            "\"objectCount\": 0",
            "\"deletedObjectCount\": 1"
        },
        issue_prefix + " deleted objects inside deleted sections should expose deleted containing-section JSON");

    const auto unplaced_deleted_object_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--record", "6", "--json"},
        temp_root);
    if (unplaced_deleted_object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " deleted unplaced section deleted-object-count stdout:\n"
                  << unplaced_deleted_object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " deleted unplaced section deleted-object-count stderr:\n"
                  << unplaced_deleted_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(unplaced_deleted_object_process.exit_code == 0,
           issue_prefix + " deleted unplaced object query should exit successfully");
    expect_contains(
        unplaced_deleted_object_process.stdout_text,
        "\"selectedReportObjectSectionAvailable\": false",
        issue_prefix + " unplaced deleted object selections should not advertise containing-section availability");
    expect_contains(
        unplaced_deleted_object_process.stdout_text,
        "\"selectedReportObjectSection\": null",
        issue_prefix + " unplaced deleted object selections should keep containing-section JSON null");
}

void test_studio_host_json_preserves_section_deleted_object_counts(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_section_deleted_object_count_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_section_deleted_object_count_json(
        studio_host_path,
        temp_root,
        "section_deleted_object_count.frx",
        "report",
        "#2829: report section deleted-object-count JSON");
    run_section_deleted_object_count_json(
        studio_host_path,
        temp_root,
        "section_deleted_object_count.lbx",
        "label",
        "#2829: label section deleted-object-count JSON");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_section_deleted_object_counts <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_section_deleted_object_counts(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
