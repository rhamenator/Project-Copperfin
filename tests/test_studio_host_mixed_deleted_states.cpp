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

void expect_full_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 8100",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 8100",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1000",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 2200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 2900",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 1200",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
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

    const auto create_result =
        copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2770: mixed deleted-states success layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2770: mixed deleted-states success layout fixture should seed deleted rows");
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
           "#2770: mixed deleted-states success fixture should seed a settings unique id");
    const auto section_unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "section-guid"
    });
    expect(section_unique_id_result.ok,
           "#2770: mixed deleted-states success fixture should seed a section unique id");
    expect(!dbf_record_deleted(report_path, 0U) && !dbf_record_deleted(report_path, 1U),
           "#2770: mixed deleted-states success fixture should preserve live settings and section rows");
}

void write_synthetic_report_table_for_stable_deleted_settings_and_section_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_settings_and_section_json(report_path);
    const auto settings_delete_result =
        copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(settings_delete_result.ok,
           "#2770: mixed deleted-states restore fixture should mark settings deleted");
    const auto section_delete_result =
        copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(section_delete_result.ok,
           "#2770: mixed deleted-states restore fixture should mark section deleted");
    expect(dbf_record_deleted(report_path, 0U) && dbf_record_deleted(report_path, 1U),
           "#2770: mixed deleted-states restore fixture should preserve deleted settings and section rows");
}

void run_mixed_deleted_states_delete_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("mixed_deleted_states_delete" + extension);
    write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);

    const auto delete_process = run_process_capture(
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
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host " << label << " mixed deleted-states delete " << extension
                  << " stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " mixed deleted-states delete " << extension
                  << " stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0, issue_prefix + " delete should exit successfully");
    expect(dbf_record_deleted(asset_path, 0U) &&
               dbf_record_deleted(asset_path, 1U) &&
               visual_object_deleted(asset_path, "field-guid"),
           issue_prefix + " delete should mark settings, section, and object rows deleted");
    expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " delete should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " delete should retain label identity");
    }
    expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " delete should preserve live preview availability");
    expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " delete should preserve live preview left bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 100",
                    issue_prefix + " delete should preserve retained live-object top bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                    issue_prefix + " delete should preserve retained live-object right bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " delete should preserve live preview bottom bounds");
    expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                    issue_prefix + " delete should preserve retained live-object preview width");
    expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8000",
                    issue_prefix + " delete should preserve retained live-object preview height");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " delete should expose deleted preview availability");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " delete should refresh deleted preview left bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    issue_prefix + " delete should refresh deleted preview top bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                    issue_prefix + " delete should refresh deleted preview right bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3050",
                    issue_prefix + " delete should refresh deleted preview bottom bounds");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 5200",
                    issue_prefix + " delete should refresh deleted preview width");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 3050",
                    issue_prefix + " delete should refresh deleted preview height");
    expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                    issue_prefix + " delete should remove live settings");
    expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                    issue_prefix + " delete should expose deleted settings");
    expect_contains(delete_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " delete should expose effective deleted-root page setup");
    expect_contains(delete_process.stdout_text, "\"orientationAvailable\": true",
                    issue_prefix + " delete should expose deleted-root orientation availability");
    expect_contains(delete_process.stdout_text, "\"orientationCode\": 0",
                    issue_prefix + " delete should expose deleted-root orientation");
    expect_contains(delete_process.stdout_text, "\"paperSizeAvailable\": true",
                    issue_prefix + " delete should expose deleted-root paper-size availability");
    expect_contains(delete_process.stdout_text, "\"paperSizeCode\": 1",
                    issue_prefix + " delete should expose deleted-root paper size");
    expect_contains(delete_process.stdout_text, "\"topMarginAvailable\": true",
                    issue_prefix + " delete should expose deleted-root top-margin availability");
    expect_contains(delete_process.stdout_text, "\"topMargin\": 10",
                    issue_prefix + " delete should expose deleted-root top margin");
    expect_contains(delete_process.stdout_text, "\"bottomMarginAvailable\": true",
                    issue_prefix + " delete should expose deleted-root bottom-margin availability");
    expect_contains(delete_process.stdout_text, "\"bottomMargin\": 20",
                    issue_prefix + " delete should expose deleted-root bottom margin");
    expect_contains(delete_process.stdout_text, "\"gridVerticalAvailable\": true",
                    issue_prefix + " delete should expose deleted-root vertical-grid availability");
    expect_contains(delete_process.stdout_text, "\"gridVertical\": 4",
                    issue_prefix + " delete should expose deleted-root vertical grid");
    expect_contains(delete_process.stdout_text, "\"gridHorizontalAvailable\": true",
                    issue_prefix + " delete should expose deleted-root horizontal-grid availability");
    expect_contains(delete_process.stdout_text, "\"gridHorizontal\": 8",
                    issue_prefix + " delete should expose deleted-root horizontal grid");
    expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                    issue_prefix + " delete should remove the selected section from live counts");
    expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                    issue_prefix + " delete should expose deleted section counts");
    expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 2",
                    issue_prefix + " delete should remove the selected object from live counts");
    expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 1",
                    issue_prefix + " delete should preserve unrelated placed live objects");
    expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                    issue_prefix + " delete should preserve unrelated unplaced objects");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                    issue_prefix + " delete should expose deleted object counts");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedSettings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0"
        },
        issue_prefix + " delete should move settings into deleted metadata");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        issue_prefix + " delete should move the section into deleted metadata");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedObjects\": [",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"objectKind\": \"field\""
        },
        issue_prefix + " delete should move the object into deleted metadata");
    expect_contains(delete_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " delete should not fabricate selected settings");
    expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " delete should not fabricate selected sections");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " delete should not fabricate selected objects");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                    issue_prefix + " delete should not fabricate a report selection");
}

void run_mixed_deleted_states_restore_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& extension,
    const std::string& title,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / ("mixed_deleted_states_restore" + extension);
    write_synthetic_report_table_for_stable_deleted_settings_and_section_json(asset_path);

    const auto field_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "field-guid",
        .deleted = true
    });
    expect(field_delete_result.ok &&
               dbf_record_deleted(asset_path, 0U) &&
               dbf_record_deleted(asset_path, 1U) &&
               visual_object_deleted(asset_path, "field-guid"),
           issue_prefix + " restore fixture should start deleted");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--deleted-states",
            "--deleted-state-target-unique-id", "settings-guid",
            "--deleted-state", "false",
            "--deleted-state-target-unique-id", "section-guid",
            "--deleted-state", "false",
            "--deleted-state-target-unique-id", "field-guid",
            "--deleted-state", "false",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host " << label << " mixed deleted-states restore " << extension
                  << " stdout:\n" << restore_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " mixed deleted-states restore " << extension
                  << " stderr:\n" << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0, issue_prefix + " restore should exit successfully");
    expect(!dbf_record_deleted(asset_path, 0U) &&
               !dbf_record_deleted(asset_path, 1U) &&
               !visual_object_deleted(asset_path, "field-guid"),
           issue_prefix + " restore should restore settings, section, and object rows");
    expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " restore should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " restore should retain label identity");
    }
    expect_full_report_layout_preview_bounds(
        restore_process.stdout_text,
        issue_prefix + " restore");
    expect_contains(restore_process.stdout_text, "\"settingCount\": 6",
                    issue_prefix + " restore should restore live settings");
    expect_contains(restore_process.stdout_text, "\"deletedSettingCount\": 0",
                    issue_prefix + " restore should clear deleted settings");
    expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                    issue_prefix + " restore should restore live sections");
    expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                    issue_prefix + " restore should clear deleted sections");
    expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                    issue_prefix + " restore should restore live object counts");
    expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                    issue_prefix + " restore should restore placed object counts");
    expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 1",
                    issue_prefix + " restore should preserve unrelated unplaced objects");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " restore should clear restored deleted objects");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0"
        },
        issue_prefix + " restore should move settings into live metadata");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false"
        },
        issue_prefix + " restore should move the section into live metadata");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"title\": \"Detail\"",
            "\"recordIndex\": 2",
            "\"objectCount\": 1",
            "\"objects\": [",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"objectKind\": \"field\""
        },
        issue_prefix + " restore should move the object into live section metadata");
    expect_contains(restore_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " restore should not fabricate selected settings");
    expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " restore should not fabricate selected sections");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " restore should not fabricate selected objects");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                    issue_prefix + " restore should not fabricate a report selection");
}

void test_studio_host_json_preserves_mixed_deleted_states(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mixed_deleted_states_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_mixed_deleted_states_delete_case(
        studio_host_path,
        temp_root,
        ".frx",
        "mixed_deleted_states_delete.frx",
        "report",
        "#2770: report/label mixed deleted-states success");
    run_mixed_deleted_states_delete_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "mixed_deleted_states_delete.lbx",
        "label",
        "#2770: report/label mixed deleted-states success");
    run_mixed_deleted_states_restore_case(
        studio_host_path,
        temp_root,
        ".frx",
        "mixed_deleted_states_restore.frx",
        "report",
        "#2770: report/label mixed deleted-states success");
    run_mixed_deleted_states_restore_case(
        studio_host_path,
        temp_root,
        ".lbx",
        "mixed_deleted_states_restore.lbx",
        "label",
        "#2770: report/label mixed deleted-states success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_mixed_deleted_states <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_mixed_deleted_states(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
