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
    expect(create_result.ok, "#2757: subtree deleted-state layout fixture should be created");
}

void run_success_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("object_subtree_deleted_state" + extension);
    write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "true",
            "--unique-id", "middle-field-guid",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host " << label << " subtree delete stdout:\n"
                  << delete_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " subtree delete stderr:\n"
                  << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0, issue_prefix + " delete should succeed");
    expect(visual_object_deleted(asset_path, "middle-field-guid") &&
               !visual_object_deleted(asset_path, "left-field-guid") &&
               !visual_object_deleted(asset_path, "right-field-guid"),
           issue_prefix + " delete should mark only the selected flat layout row");
    expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " delete should return refreshed report-layout JSON");
    if (extension == ".lbx") {
        expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " delete should retain label identity");
    }
    expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " delete should preserve live preview availability");
    expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " delete should preserve live preview left bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 2000",
                    issue_prefix + " delete should preserve live preview top bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " delete should preserve live preview right bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    issue_prefix + " delete should preserve live preview bottom bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 150",
                    issue_prefix + " delete should preserve live preview width");
    expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 5000",
                    issue_prefix + " delete should preserve live preview height");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " delete should expose deleted preview availability");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                    issue_prefix + " delete should expose deleted preview left bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    issue_prefix + " delete should expose deleted preview top bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                    issue_prefix + " delete should expose deleted preview right bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                    issue_prefix + " delete should expose deleted preview bottom bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                    issue_prefix + " delete should expose deleted preview width");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                    issue_prefix + " delete should expose deleted preview height");
    expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 2",
                    issue_prefix + " delete should remove the object from live counts");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " delete should expose deleted object counts");
    expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " delete should leave the report section selection empty");
    expect_contains(delete_process.stdout_text, "\"selectedReportSection\": null",
                    issue_prefix + " delete should serialize a null report section selection");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " delete should preserve selected-object availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    issue_prefix + " delete should preserve containing-section availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    issue_prefix + " delete should preserve report object selection kind");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"containingSectionId\": \"detail_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectKind\": \"field\"",
            "\"expression\": \"middle.value\"",
            "\"uniqueId\": \"middle-field-guid\""
        },
        issue_prefix + " delete should serialize selected deleted-object metadata");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"detail_1\"",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 1",
            "\"objectCount\": 2",
            "\"deletedObjectCount\": 1"
        },
        issue_prefix + " delete should expose containing detail-band metadata");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "false",
            "--unique-id", "middle-field-guid",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host " << label << " subtree restore stdout:\n"
                  << restore_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " subtree restore stderr:\n"
                  << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0, issue_prefix + " restore should succeed");
    expect(!visual_object_deleted(asset_path, "middle-field-guid") &&
               !visual_object_deleted(asset_path, "left-field-guid") &&
               !visual_object_deleted(asset_path, "right-field-guid"),
           issue_prefix + " restore should restore the selected row and preserve siblings");
    expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " restore should return refreshed report-layout JSON");
    if (extension == ".lbx") {
        expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " restore should retain label identity");
    }
    expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " restore should preserve live preview availability");
    expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " restore should preserve live preview left bounds");
    expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 2000",
                    issue_prefix + " restore should preserve live preview top bounds");
    expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " restore should preserve live preview right bounds");
    expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    issue_prefix + " restore should preserve live preview bottom bounds");
    expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 150",
                    issue_prefix + " restore should preserve live preview width");
    expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 5000",
                    issue_prefix + " restore should preserve live preview height");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    issue_prefix + " restore should clear deleted preview availability");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " restore should preserve zero deleted preview left bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    issue_prefix + " restore should preserve zero deleted preview top bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    issue_prefix + " restore should preserve zero deleted preview right bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                    issue_prefix + " restore should preserve zero deleted preview bottom bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    issue_prefix + " restore should preserve zero deleted preview width");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                    issue_prefix + " restore should preserve zero deleted preview height");
    expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                    issue_prefix + " restore should restore live object counts");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                    issue_prefix + " restore should clear deleted object counts");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    issue_prefix + " restore should refresh containing-section availability");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    issue_prefix + " restore should preserve report object selection kind");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionObjectIndex\": 1",
            "\"sectionObjectCount\": 3",
            "\"objectKind\": \"field\"",
            "\"expression\": \"middle.value\"",
            "\"uniqueId\": \"middle-field-guid\""
        },
        issue_prefix + " restore should refresh selected live-object metadata");
}

void test_studio_host_json_preserves_subtree_deleted_state(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_subtree_deleted_state_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_success_case(
        studio_host_path,
        temp_root,
        ".frx",
        "object_subtree_deleted_state.frx",
        "report",
        "#2757: report/label stable object subtree delete/restore");
    run_success_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "object_subtree_deleted_state.lbx",
        "label",
        "#2757: report/label stable object subtree delete/restore");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_subtree_deleted_state <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_subtree_deleted_state(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
