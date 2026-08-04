// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

void write_synthetic_report_table_for_stable_page_footer_object_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 28U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "", "", "", "", ""},
        {"9", "0", "", "", "0", "", "700", "title-section-guid"},
        {"9", "4", "", "", "700", "", "2500", ""},
        {"9", "7", "", "", "3200", "", "500", "page-footer-section-guid"},
        {"5", "", "\"Page footer label\"", "150", "3300", "1600", "300", "page-footer-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2821: stable page-footer object fixture should be created");
}

void write_synthetic_report_table_for_deleted_page_footer_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_page_footer_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#2821: deleted page-footer object fixture should mark page-footer object deleted");
}

void run_page_footer_object_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_page_footer_object_json(asset_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "page-footer-label-guid", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected page-footer object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected page-footer object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should preserve document titles");
    if (asset_path.extension() == ".lbx") {
        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    issue_prefix + " should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    issue_prefix + " should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                    issue_prefix + " should preserve live section counts");
    expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                    issue_prefix + " should preserve deleted section counts");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                    issue_prefix + " should preserve live object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                    issue_prefix + " should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1750",
                    issue_prefix + " should include selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1750",
                    issue_prefix + " should include selected-object preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    issue_prefix + " should not fabricate deleted preview availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    issue_prefix + " should advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    issue_prefix + " should expose containing-section JSON");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": false",
            "\"containingSectionId\": \"page-footer-section-guid\"",
            "\"containingSectionRecordIndex\": 3",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 400",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Page footer label\\\"\""
        },
        issue_prefix + " should expose selected object metadata");
    expect_contains(object_process.stdout_text, "\"left\": 150",
                    issue_prefix + " should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 3300",
                    issue_prefix + " should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"width\": 1600",
                    issue_prefix + " should expose selected-object widths");
    expect_contains(object_process.stdout_text, "\"right\": 1750",
                    issue_prefix + " should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"height\": 300",
                    issue_prefix + " should expose selected-object heights");
    expect_contains(object_process.stdout_text, "\"bottom\": 3600",
                    issue_prefix + " should expose selected-object bottom bounds");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page-footer-section-guid\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3200",
            "\"height\": 500",
            "\"bottom\": 3700",
            "\"objectCount\": 1"
        },
        issue_prefix + " should expose the containing page-footer metadata");
}

void run_deleted_page_footer_object_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_deleted_page_footer_object_json(asset_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "page-footer-label-guid", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected deleted page-footer object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected deleted page-footer object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should preserve document titles");
    if (asset_path.extension() == ".lbx") {
        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    issue_prefix + " should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    issue_prefix + " should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                    issue_prefix + " should preserve live section counts");
    expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                    issue_prefix + " should preserve deleted section counts");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                    issue_prefix + " should clear live object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 150",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 3300",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1750",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3600",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1600",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    issue_prefix + " should advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    issue_prefix + " should expose containing-section JSON");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": true",
            "\"containingSectionId\": \"page-footer-section-guid\"",
            "\"containingSectionRecordIndex\": 3",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 400",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Page footer label\\\"\""
        },
        issue_prefix + " should expose selected deleted-object metadata");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page-footer-section-guid\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"objectCount\": 0",
            "\"deletedObjectCount\": 1"
        },
        issue_prefix + " should expose containing page-footer metadata");
    expect_contains(object_process.stdout_text, "\"left\": 150",
                    issue_prefix + " should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 3300",
                    issue_prefix + " should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"width\": 1600",
                    issue_prefix + " should expose selected-object widths");
    expect_contains(object_process.stdout_text, "\"right\": 1750",
                    issue_prefix + " should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"height\": 300",
                    issue_prefix + " should expose selected-object heights");
    expect_contains(object_process.stdout_text, "\"bottom\": 3600",
                    issue_prefix + " should expose selected-object bottom bounds");
}

void test_studio_host_json_preserves_selected_page_footer_objects_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_footer_objects_stable_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_page_footer_object_selection(
        studio_host_path,
        temp_root,
        "selected_page_footer_object_stable.frx",
        "report",
        "#2821: stable live report page-footer object selection");
    run_page_footer_object_selection(
        studio_host_path,
        temp_root,
        "selected_page_footer_object_stable.lbx",
        "label",
        "#2821: stable live label page-footer object selection");
    run_deleted_page_footer_object_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_page_footer_object_stable.frx",
        "report",
        "#2821: stable deleted report page-footer object selection");
    run_deleted_page_footer_object_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_page_footer_object_stable.lbx",
        "label",
        "#2821: stable deleted label page-footer object selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_page_footer_objects_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_page_footer_objects_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
