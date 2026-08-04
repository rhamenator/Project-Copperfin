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
    expect(create_result.ok, "#2730: clear layout fixture should be created");
}

void mark_deleted(const std::filesystem::path& asset_path, const std::string& unique_id) {
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .deleted = true
    });
    expect(delete_result.ok, "#2730: clear fixture should mark target rows deleted");
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

void run_deleted_property_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension) {
    const std::filesystem::path asset_path = temp_root / ("deleted_property_clear" + extension);
    write_synthetic_report_table_for_layout_reorder_json(asset_path);
    mark_deleted(asset_path, "middle-field-guid");
    mark_deleted(asset_path, "right-field-guid");

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--path", asset_path.string(),
            "--property-name", "EXPR",
            "--unique-id", "middle-field-guid",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host deleted property clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host deleted property clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, "#2730: deleted property clear execution should exit successfully");
    expect(visual_object_deleted(asset_path, "middle-field-guid") && visual_object_deleted(asset_path, "right-field-guid"),
           "#2730: deleted property clear should preserve the deleted state");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
        temp_root);

    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host deleted property clear reopen " << extension << " stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host deleted property clear reopen " << extension << " stderr:\n"
                  << reopen_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(reopen_process.exit_code == 0, "#2730: deleted property clear reopen should exit successfully");
    if (extension == ".lbx") {
        expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                        "#2730: deleted property clear reopen should preserve label identity");
    }
    expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#2730: deleted property clear reopen should keep the deleted row selected");
    expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#2730: deleted property clear reopen should preserve containing-section availability");
    expect_contains_in_order(
        reopen_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"containingSectionId\": \"detail_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionRelativeTop\": 600",
            "\"sectionRelativeBottom\": 800",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 2",
            "\"objectKind\": \"field\"",
            "\"expression\": \"\"",
            "\"uniqueId\": \"middle-field-guid\""
        },
        "#2730: deleted property clear reopen should preserve selected deleted-row containing-section metadata");
    expect_contains_in_order(
        reopen_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"detail_1\"",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 1",
            "\"objectCount\": 1",
            "\"deletedObjectCount\": 2"
        },
        "#2730: deleted property clear reopen should expose containing detail-band metadata");
}

void test_studio_host_json_preserves_deleted_property_clear_sections(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_deleted_property_clear_section_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_deleted_property_clear_case(studio_host_path, temp_root, ".frx");
    run_deleted_property_clear_case(studio_host_path, temp_root, ".lbx");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_deleted_property_clear_sections <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_deleted_property_clear_sections(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
