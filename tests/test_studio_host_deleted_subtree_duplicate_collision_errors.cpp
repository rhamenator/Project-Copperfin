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

std::size_t visual_object_count(const std::filesystem::path& asset_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(asset_path.string());
    return list_result.ok ? list_result.objects.size() : 0U;
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

void write_synthetic_report_table_for_layout_subtree_deleted_state_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ReportSettings", "ReportSettings", "", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "DetailBand", "DetailBand", "", "", "", "2000", "", "5000", ""},
        {"8", "0", "LeftField", "LeftField", "", "left.value", "100", "2600", "50", "200", "left-field-guid"},
        {"8", "0", "MiddleField", "MiddleField", "", "middle.value", "100", "2600", "50", "200", "middle-field-guid"},
        {"8", "0", "RightField", "RightField", "", "right.value", "100", "2600", "50", "200", "right-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2754: deleted subtree duplicate collision layout fixture should be created");
}

void run_collision_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("deleted_object_subtree_duplicate_collision" + extension);
    write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "middle-field-guid",
        .deleted = true
    });
    expect(delete_result.ok && visual_object_deleted(asset_path, "middle-field-guid"),
           issue_prefix + " fixture should start with a deleted root row");

    const std::size_t before_count = visual_object_count(asset_path);

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", asset_path.string(),
            "--unique-id", "middle-field-guid",
            "--replacement-source-unique-id", "middle-field-guid",
            "--new-object-name", "MiddleDeletedCopy",
            "--new-name", "MiddleDeletedCopy",
            "--new-unique-id", "right-field-guid",
            "--json"
        },
        temp_root);

    if (collision_process.exit_code != 4) {
        std::cerr << "studio host deleted subtree duplicate collision " << extension << " stdout:\n"
                  << collision_process.stdout_text << "\n";
        std::cerr << "studio host deleted subtree duplicate collision " << extension << " stderr:\n"
                  << collision_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(collision_process.exit_code == 4, issue_prefix + " should fail");
    expect_contains(collision_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
                    issue_prefix + " should not expose stale result objects");
    expect_contains(collision_process.stdout_text,
                    "The requested replacement identity already exists in the asset.",
                    issue_prefix + " should report collision errors");
    expect(visual_object_count(asset_path) == before_count &&
               visual_object_deleted(asset_path, "middle-field-guid") &&
               !visual_object_exists(asset_path, "middle-deleted-copy-guid") &&
               !visual_object_deleted(asset_path, "left-field-guid") &&
               !visual_object_deleted(asset_path, "right-field-guid") &&
               visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
           issue_prefix + " should not mutate layout rows");
}

void test_studio_host_json_preserves_deleted_subtree_duplicate_collision_errors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_subtree_duplicate_collision_error_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    for (const auto& extension : {std::string(".frx"), std::string(".lbx")}) {
        run_collision_case(
            studio_host_path,
            temp_root,
            extension,
            "#2754: deleted subtree duplicate collision");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_deleted_subtree_duplicate_collision_errors <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_deleted_subtree_duplicate_collision_errors(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
