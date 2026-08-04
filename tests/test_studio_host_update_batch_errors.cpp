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

void expect_not_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) == std::string::npos, message);
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
    expect(create_result.ok, "#2760: update-batch rollback layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2760: update-batch rollback fixture should mark deleted layout objects");
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

void run_update_batch_rollback_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("report_object_update_batch_rollback" + extension);
    write_synthetic_report_table_for_layout_json(asset_path);

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", asset_path.string(),
            "--selected-unique-id", "field-guid",
            "--property-name", "EXPR",
            "--property-value", "should.rollback",
            "--property-name", "WIDTH",
            "--property-value", "4444",
            "--selected-unique-id", "label-guid",
            "--property-name", "HPOS",
            "--property-value", "111",
            "--selected-unique-id", "missing-guid",
            "--property-name", "EXPR",
            "--property-value", "missing",
            "--json"
        },
        temp_root);

    if (rollback_process.exit_code != 4) {
        std::cerr << "studio host update-batch rollback " << extension << " stdout:\n"
                  << rollback_process.stdout_text << "\n";
        std::cerr << "studio host update-batch rollback " << extension << " stderr:\n"
                  << rollback_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(rollback_process.exit_code == 4, issue_prefix + " should fail");
    expect_contains(rollback_process.stdout_text, "\"visualObjectUpdateBatch\": null",
                    issue_prefix + " should not expose stale batch objects");
    expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                        issue_prefix + " should not expose stale committed state");
    expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                        issue_prefix + " should not expose stale mutation state");
    expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                        issue_prefix + " should not advertise undo availability");
    expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                        issue_prefix + " should not expose stale undo labels");
    expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                    issue_prefix + " should report missing selector errors");
    expect(visual_object_property(asset_path, "field-guid", "EXPR") == "customer.company" &&
               visual_object_property(asset_path, "field-guid", "WIDTH") == "4000" &&
               visual_object_property(asset_path, "label-guid", "HPOS") == "900",
           issue_prefix + " should roll back earlier layout property mutations");
}

void test_studio_host_json_preserves_update_batch_errors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_update_batch_error_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    for (const auto& extension : {std::string(".frx"), std::string(".lbx")}) {
        run_update_batch_rollback_case(
            studio_host_path,
            temp_root,
            extension,
            "#2760: report/label update-batch rollback");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_update_batch_errors <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_update_batch_errors(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
