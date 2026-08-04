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

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
}

std::size_t visual_object_count(const std::filesystem::path& asset_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(asset_path.string());
    return list_result.ok ? list_result.objects.size() : 0U;
}

bool visual_object_deleted(const std::filesystem::path& asset_path, const std::string& unique_id) {
    const auto list_result = copperfin::vfp::list_visual_objects(asset_path.string());
    if (!list_result.ok) {
        return false;
    }
    for (const auto& object : list_result.objects) {
        if (object.unique_id == unique_id) {
            return object.deleted;
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
    expect(create_result.ok, "#2748: mixed deleted-states rollback layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2748: mixed deleted-states rollback layout fixture should seed deleted rows");
}

void write_synthetic_report_table_for_stable_settings_and_section_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto settings_unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(settings_unique_id_result.ok,
           "#2748: mixed deleted-states rollback fixture should seed a settings unique id");
    const auto section_unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "section-guid"
    });
    expect(section_unique_id_result.ok,
           "#2748: mixed deleted-states rollback fixture should seed a section unique id");
    expect(!dbf_record_deleted(report_path, 0U) && !dbf_record_deleted(report_path, 1U),
           "#2748: mixed deleted-states rollback fixture should preserve live settings and section rows");
}

void run_mixed_deleted_states_rollback_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("mixed_deleted_states_rollback" + extension);
    write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);

    const std::size_t before_object_count = visual_object_count(asset_path);

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--deleted-states",
            "--deleted-state-target-unique-id", "settings-guid",
            "--deleted-state", "true",
            "--deleted-state-target-unique-id", "section-guid",
            "--deleted-state", "true",
            "--deleted-state-target-unique-id", "field-guid",
            "--deleted-state", "true",
            "--deleted-state-target-unique-id", "missing-guid",
            "--deleted-state", "true",
            "--json"
        },
        temp_root);

    if (rollback_process.exit_code != 4) {
        std::cerr << "studio host mixed deleted-states rollback " << extension << " stdout:\n"
                  << rollback_process.stdout_text << "\n";
        std::cerr << "studio host mixed deleted-states rollback " << extension << " stderr:\n"
                  << rollback_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(rollback_process.exit_code == 4, issue_prefix + " should fail");
    expect(!dbf_record_deleted(asset_path, 0U) &&
               !dbf_record_deleted(asset_path, 1U) &&
               !visual_object_deleted(asset_path, "field-guid"),
           issue_prefix + " should roll back earlier settings, section, and object deletions");
    expect(dbf_record_deleted(asset_path, 6U),
           issue_prefix + " should preserve unrelated pre-deleted rows");
    expect(visual_object_count(asset_path) == before_object_count,
           issue_prefix + " should preserve the original object count");
    expect_contains(rollback_process.stdout_text, "status: error",
                    issue_prefix + " should report JSON error status");
    expect_contains(rollback_process.stdout_text, "error",
                    issue_prefix + " should report an error message");
}

void test_studio_host_json_preserves_mixed_deleted_states_errors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mixed_deleted_states_error_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    for (const auto& extension : {std::string(".frx"), std::string(".lbx")}) {
        run_mixed_deleted_states_rollback_case(
            studio_host_path,
            temp_root,
            extension,
            "#2748: mixed deleted-states rollback");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_mixed_deleted_states_errors <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_mixed_deleted_states_errors(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
