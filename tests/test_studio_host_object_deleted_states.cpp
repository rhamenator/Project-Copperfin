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
    expect(create_result.ok, "#2768: object deleted-states success layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2768: object deleted-states success layout fixture should seed deleted rows");
}

void run_object_batch_delete(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("object_deleted_states_delete" + extension);
    write_synthetic_report_table_for_layout_json(asset_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--deleted-states",
            "--deleted-state-target-unique-id", "field-guid",
            "--deleted-state", "true",
            "--deleted-state-target-unique-id", "label-guid",
            "--deleted-state", "true",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host " << label << " object deleted-states delete stdout:\n"
                  << delete_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " object deleted-states delete stderr:\n"
                  << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0, issue_prefix + " delete should exit successfully");
    expect(visual_object_deleted(asset_path, "field-guid") &&
               visual_object_deleted(asset_path, "label-guid"),
           issue_prefix + " delete should mark both object rows deleted");
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
    expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " delete should preserve live preview top bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " delete should refresh live preview right bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " delete should preserve live preview bottom bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 150",
                    issue_prefix + " delete should refresh live preview width");
    expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " delete should preserve live preview height");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " delete should expose deleted preview availability");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                    issue_prefix + " delete should refresh deleted preview left bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                    issue_prefix + " delete should refresh deleted preview top bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                    issue_prefix + " delete should refresh deleted preview right bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3050",
                    issue_prefix + " delete should refresh deleted preview bottom bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4300",
                    issue_prefix + " delete should refresh deleted preview width");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2950",
                    issue_prefix + " delete should refresh deleted preview height");
    expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 1",
                    issue_prefix + " delete should remove objects from live counts");
    expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 0",
                    issue_prefix + " delete should remove placed live objects");
    expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                    issue_prefix + " delete should preserve unrelated unplaced objects");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 3",
                    issue_prefix + " delete should expose deleted object counts");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedObjects\": [",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"objectKind\": \"field\"",
            "\"recordIndex\": 4",
            "\"deleted\": true",
            "\"objectKind\": \"label\""
        },
        issue_prefix + " delete should move both objects into deleted metadata");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " delete should not fabricate selected objects");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " delete should not fabricate containing sections");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                    issue_prefix + " delete should not fabricate a report selection");
}

void run_object_batch_restore(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("object_deleted_states_restore" + extension);
    write_synthetic_report_table_for_layout_json(asset_path);

    const auto field_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "field-guid",
        .deleted = true
    });
    const auto label_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "label-guid",
        .deleted = true
    });
    expect(field_delete_result.ok && label_delete_result.ok &&
               visual_object_deleted(asset_path, "field-guid") &&
               visual_object_deleted(asset_path, "label-guid"),
           issue_prefix + " restore fixture should start deleted");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--deleted-states",
            "--deleted-state-target-unique-id", "field-guid",
            "--deleted-state", "false",
            "--deleted-state-target-unique-id", "label-guid",
            "--deleted-state", "false",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host " << label << " object deleted-states restore stdout:\n"
                  << restore_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " object deleted-states restore stderr:\n"
                  << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0, issue_prefix + " restore should exit successfully");
    expect(!visual_object_deleted(asset_path, "field-guid") &&
               !visual_object_deleted(asset_path, "label-guid"),
           issue_prefix + " restore should restore both object rows");
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
    expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " restore should preserve live preview top bounds");
    expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " restore should preserve live preview right bounds");
    expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " restore should preserve live preview bottom bounds");
    expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " restore should preserve live preview width");
    expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " restore should preserve live preview height");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " restore should preserve deleted preview availability");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    issue_prefix + " restore should preserve deleted preview left bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    issue_prefix + " restore should preserve deleted preview top bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    issue_prefix + " restore should preserve deleted preview right bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " restore should preserve deleted preview bottom bounds");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    issue_prefix + " restore should preserve deleted preview width");
    expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    issue_prefix + " restore should preserve deleted preview height");
    expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                    issue_prefix + " restore should restore live object counts");
    expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                    issue_prefix + " restore should restore placed live objects");
    expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 1",
                    issue_prefix + " restore should preserve unrelated unplaced objects");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " restore should clear the deleted object count for restored rows");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"title\": \"Page Header\"",
            "\"objects\": [",
            "\"recordIndex\": 4",
            "\"deleted\": false",
            "\"objectKind\": \"label\""
        },
        issue_prefix + " restore should expose the restored label in its live section metadata");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"title\": \"Detail\"",
            "\"objects\": [",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"objectKind\": \"field\""
        },
        issue_prefix + " restore should expose the restored field in its live section metadata");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"deletedObjects\": [",
            "\"recordIndex\": 6",
            "\"deleted\": true",
            "\"objectKind\": \"label\""
        },
        issue_prefix + " restore should preserve unrelated deleted object metadata");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " restore should not fabricate selected objects");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " restore should not fabricate containing sections");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                    issue_prefix + " restore should not fabricate a report selection");
}

void test_studio_host_json_preserves_object_deleted_states(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_object_deleted_states_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_object_batch_delete(
        studio_host_path,
        temp_root,
        ".frx",
        "object_deleted_states_delete.frx",
        "report",
        "#2768: report/label object deleted-states success");
    run_object_batch_delete(
        studio_host_path,
        temp_root,
        ".lbx",
        "object_deleted_states_delete.lbx",
        "label",
        "#2768: report/label object deleted-states success");
    run_object_batch_restore(
        studio_host_path,
        temp_root,
        ".frx",
        "object_deleted_states_restore.frx",
        "report",
        "#2768: report/label object deleted-states success");
    run_object_batch_restore(
        studio_host_path,
        temp_root,
        ".lbx",
        "object_deleted_states_restore.lbx",
        "label",
        "#2768: report/label object deleted-states success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_object_deleted_states <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_object_deleted_states(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
