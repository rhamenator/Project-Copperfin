// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"
#include "test_studio_host_real_sample_support.h"

#include <chrono>
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

std::string read_binary(const std::filesystem::path& path) {
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
        result.stdout_text = read_binary(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_binary(stderr_path);
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

bool make_writable(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_write,
        std::filesystem::perm_options::add,
        error);
    return !error;
}

std::string selected_report_section_segment(const std::string& json_text) {
    const std::string start_marker = "\"selectedReportSection\": {";
    const std::string end_marker = "\n    \"selectedReportObjectAvailable\":";
    const std::size_t start = json_text.find(start_marker);
    if (start == std::string::npos) {
        return {};
    }

    const std::size_t end = json_text.find(end_marker, start);
    if (end == std::string::npos) {
        return json_text.substr(start);
    }

    return json_text.substr(start, end - start);
}

std::string extract_json_string_field(const std::string& text, const std::string& field_name) {
    const std::string marker = "\"" + field_name + "\": \"";
    const std::size_t start = text.find(marker);
    if (start == std::string::npos) {
        return {};
    }

    const std::size_t value_start = start + marker.size();
    const std::size_t value_end = text.find('"', value_start);
    if (value_end == std::string::npos) {
        return {};
    }

    return text.substr(value_start, value_end - value_start);
}

std::string extract_json_raw_field(const std::string& text, const std::string& field_name) {
    const std::string marker = "\"" + field_name + "\": ";
    const std::size_t start = text.find(marker);
    if (start == std::string::npos) {
        return {};
    }

    std::size_t value_start = start + marker.size();
    while (value_start < text.size() && (text[value_start] == ' ' || text[value_start] == '\n')) {
        ++value_start;
    }

    std::size_t value_end = value_start;
    while (value_end < text.size()) {
        const char ch = text[value_end];
        if (ch == ',' || ch == '\n' || ch == '}') {
            break;
        }
        ++value_end;
    }

    return text.substr(value_start, value_end - value_start);
}

int extract_json_int_field(const std::string& text, const std::string& field_name) {
    const std::string raw = extract_json_raw_field(text, field_name);
    return raw.empty() ? 0 : std::stoi(raw);
}

bool extract_json_bool_field(const std::string& text, const std::string& field_name) {
    return extract_json_raw_field(text, field_name) == "true";
}

struct RealSampleGroupingCase {
    std::string issue_tag;
    std::string sample_stem;
    std::string primary_filename;
    std::string sidecar_filename;
    std::string original_grouping_expression;
    int header_record_index = 2;
};

struct GroupingSnapshot {
    std::string id;
    std::string title;
    std::string group_role;
    std::string expression;
    std::string grouping_expression;
    int record_index = 0;
    int header_record_index = 0;
    int footer_record_index = 0;
    bool deleted = false;
    bool selected_section_available = false;
    std::string selection_kind;
    bool command_undo_available = false;
    std::string command_undo_label;
    int preview_left = 0;
    int preview_top = 0;
    int preview_right = 0;
    int preview_bottom = 0;
    int preview_width = 0;
    int preview_height = 0;
    bool deleted_preview_available = false;
    int deleted_preview_left = 0;
    int deleted_preview_top = 0;
    int deleted_preview_right = 0;
    int deleted_preview_bottom = 0;
    int deleted_preview_width = 0;
    int deleted_preview_height = 0;
};

GroupingSnapshot capture_grouping_snapshot(const std::string& json_text) {
    const std::string section_segment = selected_report_section_segment(json_text);
    return {
        .id = extract_json_string_field(section_segment, "id"),
        .title = extract_json_string_field(section_segment, "title"),
        .group_role = extract_json_string_field(section_segment, "groupRole"),
        .expression = extract_json_string_field(section_segment, "expression"),
        .grouping_expression = extract_json_string_field(section_segment, "groupingExpression"),
        .record_index = extract_json_int_field(section_segment, "recordIndex"),
        .header_record_index = extract_json_int_field(json_text, "headerRecordIndex"),
        .footer_record_index = extract_json_int_field(json_text, "footerRecordIndex"),
        .deleted = extract_json_bool_field(section_segment, "deleted"),
        .selected_section_available = extract_json_bool_field(json_text, "selectedReportSectionAvailable"),
        .selection_kind = extract_json_string_field(json_text, "selectedReportSelectionKind"),
        .command_undo_available = extract_json_bool_field(json_text, "commandUndoAvailable"),
        .command_undo_label = extract_json_string_field(json_text, "commandUndoLabel"),
        .preview_left = extract_json_int_field(json_text, "previewBoundsLeft"),
        .preview_top = extract_json_int_field(json_text, "previewBoundsTop"),
        .preview_right = extract_json_int_field(json_text, "previewBoundsRight"),
        .preview_bottom = extract_json_int_field(json_text, "previewBoundsBottom"),
        .preview_width = extract_json_int_field(json_text, "previewBoundsWidth"),
        .preview_height = extract_json_int_field(json_text, "previewBoundsHeight"),
        .deleted_preview_available = extract_json_bool_field(json_text, "deletedPreviewBoundsAvailable"),
        .deleted_preview_left = extract_json_int_field(json_text, "deletedPreviewBoundsLeft"),
        .deleted_preview_top = extract_json_int_field(json_text, "deletedPreviewBoundsTop"),
        .deleted_preview_right = extract_json_int_field(json_text, "deletedPreviewBoundsRight"),
        .deleted_preview_bottom = extract_json_int_field(json_text, "deletedPreviewBoundsBottom"),
        .deleted_preview_width = extract_json_int_field(json_text, "deletedPreviewBoundsWidth"),
        .deleted_preview_height = extract_json_int_field(json_text, "deletedPreviewBoundsHeight")
    };
}

void expect_identity_matches(
    const GroupingSnapshot& snapshot,
    const GroupingSnapshot& baseline,
    bool deleted,
    bool undo_available,
    bool deleted_preview_available,
    const std::string& prefix) {
    expect(snapshot.selected_section_available,
           prefix + " should expose selected section availability");
    expect(snapshot.selection_kind == "section",
           prefix + " should preserve section selection kind");
    expect(snapshot.id == baseline.id,
           prefix + " should preserve the selected group-section id");
    expect(snapshot.title == baseline.title,
           prefix + " should preserve the selected group-section title");
    expect(snapshot.group_role == baseline.group_role,
           prefix + " should preserve the selected group-section role");
    expect(snapshot.expression == baseline.expression,
           prefix + " should preserve the selected group-section expression");
    expect(snapshot.grouping_expression == baseline.grouping_expression,
           prefix + " should preserve the grouping summary expression");
    expect(snapshot.record_index == baseline.record_index,
           prefix + " should preserve the selected group-section record identity");
    expect(snapshot.header_record_index == baseline.header_record_index,
           prefix + " should preserve the grouping header record identity");
    expect(snapshot.footer_record_index == baseline.footer_record_index,
           prefix + " should preserve the grouping footer record identity");
    expect(snapshot.deleted == deleted,
           prefix + " should expose the expected deleted state");
    expect(snapshot.command_undo_available == undo_available,
           prefix + " should expose the expected undo availability");
    expect(snapshot.deleted_preview_available == deleted_preview_available,
           prefix + " should expose the expected deleted-preview availability");
}

void expect_live_preview_matches(
    const GroupingSnapshot& snapshot,
    const GroupingSnapshot& baseline,
    const std::string& prefix) {
    expect(snapshot.preview_left == baseline.preview_left,
           prefix + " should preserve live preview left");
    expect(snapshot.preview_top == baseline.preview_top,
           prefix + " should preserve live preview top");
    expect(snapshot.preview_right == baseline.preview_right,
           prefix + " should preserve live preview right");
    expect(snapshot.preview_bottom == baseline.preview_bottom,
           prefix + " should preserve live preview bottom");
    expect(snapshot.preview_width == baseline.preview_width,
           prefix + " should preserve live preview width");
    expect(snapshot.preview_height == baseline.preview_height,
           prefix + " should preserve live preview height");
}

void expect_deleted_preview_matches(
    const GroupingSnapshot& snapshot,
    const GroupingSnapshot& baseline,
    const std::string& prefix) {
    expect(snapshot.deleted_preview_left == baseline.deleted_preview_left,
           prefix + " should preserve deleted preview left");
    expect(snapshot.deleted_preview_top == baseline.deleted_preview_top,
           prefix + " should preserve deleted preview top");
    expect(snapshot.deleted_preview_right == baseline.deleted_preview_right,
           prefix + " should preserve deleted preview right");
    expect(snapshot.deleted_preview_bottom == baseline.deleted_preview_bottom,
           prefix + " should preserve deleted preview bottom");
    expect(snapshot.deleted_preview_width == baseline.deleted_preview_width,
           prefix + " should preserve deleted preview width");
    expect(snapshot.deleted_preview_height == baseline.deleted_preview_height,
           prefix + " should preserve deleted preview height");
}

void expect_deleted_preview_cleared(const GroupingSnapshot& snapshot, const std::string& prefix) {
    expect(snapshot.deleted_preview_left == 0,
           prefix + " should clear deleted preview left");
    expect(snapshot.deleted_preview_top == 0,
           prefix + " should clear deleted preview top");
    expect(snapshot.deleted_preview_right == 0,
           prefix + " should clear deleted preview right");
    expect(snapshot.deleted_preview_bottom == 0,
           prefix + " should clear deleted preview bottom");
    expect(snapshot.deleted_preview_width == 0,
           prefix + " should clear deleted preview width");
    expect(snapshot.deleted_preview_height == 0,
           prefix + " should clear deleted preview height");
}

void run_real_vfp9_report_sample_grouping_deleted_preview_round_trip(
    const std::string& studio_host_path,
    const RealSampleGroupingCase& sample_case,
    const std::filesystem::path& reports_root) {
    namespace fs = std::filesystem;

    if (!fs::exists(reports_root / sample_case.primary_filename) ||
        !fs::exists(reports_root / sample_case.sidecar_filename)) {
        std::cerr << "SKIP: " << sample_case.issue_tag << " real VFP9 report sample "
                  << sample_case.primary_filename << " was not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() /
        ("copperfin_studio_host_real_vfp9_" + sample_case.sample_stem +
         "_grouping_deleted_preview_round_trip_tests_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path copied_primary = temp_root / sample_case.primary_filename;
    const fs::path copied_sidecar = temp_root / sample_case.sidecar_filename;
    std::error_code copy_error;
    fs::copy_file(
        reports_root / sample_case.primary_filename,
        copied_primary,
        fs::copy_options::overwrite_existing,
        copy_error);
    expect(!copy_error, sample_case.issue_tag + ": should copy the real report asset into temp space");
    copy_error.clear();
    fs::copy_file(
        reports_root / sample_case.sidecar_filename,
        copied_sidecar,
        fs::copy_options::overwrite_existing,
        copy_error);
    expect(!copy_error, sample_case.issue_tag + ": should copy the real report sidecar into temp space");
    expect(make_writable(copied_primary), sample_case.issue_tag + ": copied report asset should become writable");
    expect(make_writable(copied_sidecar), sample_case.issue_tag + ": copied report sidecar should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample_case.header_record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host " << sample_case.sample_stem
                  << " initial grouping deleted-preview read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host " << sample_case.sample_stem
                  << " initial grouping deleted-preview read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0,
           sample_case.issue_tag + ": real sample grouping deleted-preview read should succeed");
    const GroupingSnapshot initial_snapshot = capture_grouping_snapshot(initial_process.stdout_text);
    expect_identity_matches(
        initial_snapshot,
        initial_snapshot,
        false,
        false,
        false,
        sample_case.issue_tag + ": initial real sample grouping read");
    expect(initial_snapshot.group_role == "header",
           sample_case.issue_tag + ": initial real sample grouping read should target the group header");
    expect(initial_snapshot.grouping_expression == sample_case.original_grouping_expression,
           sample_case.issue_tag + ": initial real sample grouping read should expose the expected grouping expression");

    const auto delete_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(),
         "--record", std::to_string(sample_case.header_record_index),
         "--delete-object",
         "--json"},
        temp_root);
    if (delete_process.exit_code != 0) {
        std::cerr << "studio host " << sample_case.sample_stem << " grouping delete-preview stdout:\n"
                  << delete_process.stdout_text << "\n";
        std::cerr << "studio host " << sample_case.sample_stem << " grouping delete-preview stderr:\n"
                  << delete_process.stderr_text << "\n";
    }
    expect(delete_process.exit_code == 0,
           sample_case.issue_tag + ": real sample grouping delete should succeed");
    const GroupingSnapshot deleted_snapshot = capture_grouping_snapshot(delete_process.stdout_text);
    expect_identity_matches(
        deleted_snapshot,
        initial_snapshot,
        true,
        true,
        true,
        sample_case.issue_tag + ": deleted real sample grouping read");
    expect(read_binary(copied_primary) != original_primary_bytes,
           sample_case.issue_tag + ": real sample grouping delete should change primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           sample_case.issue_tag + ": real sample grouping delete should preserve sidecar bytes");

    const auto reopen_after_delete = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample_case.header_record_index), "--json"},
        temp_root);
    if (reopen_after_delete.exit_code != 0) {
        std::cerr << "studio host " << sample_case.sample_stem
                  << " reopen-after-delete grouping preview stdout:\n"
                  << reopen_after_delete.stdout_text << "\n";
        std::cerr << "studio host " << sample_case.sample_stem
                  << " reopen-after-delete grouping preview stderr:\n"
                  << reopen_after_delete.stderr_text << "\n";
    }
    expect(reopen_after_delete.exit_code == 0,
           sample_case.issue_tag + ": real sample grouping reopen after delete should succeed");
    const GroupingSnapshot reopen_deleted_snapshot = capture_grouping_snapshot(reopen_after_delete.stdout_text);
    expect_identity_matches(
        reopen_deleted_snapshot,
        initial_snapshot,
        true,
        true,
        true,
        sample_case.issue_tag + ": reopen-after-delete real sample grouping read");
    expect_live_preview_matches(
        reopen_deleted_snapshot,
        deleted_snapshot,
        sample_case.issue_tag + ": reopen-after-delete real sample grouping read");
    expect_deleted_preview_matches(
        reopen_deleted_snapshot,
        deleted_snapshot,
        sample_case.issue_tag + ": reopen-after-delete real sample grouping read");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(),
         "--record", std::to_string(sample_case.header_record_index),
         "--restore-object",
         "--json"},
        temp_root);
    if (restore_process.exit_code != 0) {
        std::cerr << "studio host " << sample_case.sample_stem << " grouping restore-preview stdout:\n"
                  << restore_process.stdout_text << "\n";
        std::cerr << "studio host " << sample_case.sample_stem << " grouping restore-preview stderr:\n"
                  << restore_process.stderr_text << "\n";
    }
    expect(restore_process.exit_code == 0,
           sample_case.issue_tag + ": real sample grouping restore should succeed");
    const GroupingSnapshot restored_snapshot = capture_grouping_snapshot(restore_process.stdout_text);
    expect_identity_matches(
        restored_snapshot,
        initial_snapshot,
        false,
        true,
        false,
        sample_case.issue_tag + ": restored real sample grouping read");
    expect_live_preview_matches(
        restored_snapshot,
        initial_snapshot,
        sample_case.issue_tag + ": restored real sample grouping read");
    expect_deleted_preview_cleared(
        restored_snapshot,
        sample_case.issue_tag + ": restored real sample grouping read");
    expect(read_binary(copied_primary) == original_primary_bytes,
           sample_case.issue_tag + ": real sample grouping restore should rewind primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           sample_case.issue_tag + ": real sample grouping restore should rewind sidecar bytes");

    const auto reopen_after_restore = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample_case.header_record_index), "--json"},
        temp_root);
    if (reopen_after_restore.exit_code != 0) {
        std::cerr << "studio host " << sample_case.sample_stem
                  << " reopen-after-restore grouping preview stdout:\n"
                  << reopen_after_restore.stdout_text << "\n";
        std::cerr << "studio host " << sample_case.sample_stem
                  << " reopen-after-restore grouping preview stderr:\n"
                  << reopen_after_restore.stderr_text << "\n";
    }
    expect(reopen_after_restore.exit_code == 0,
           sample_case.issue_tag + ": real sample grouping reopen after restore should succeed");
    const GroupingSnapshot reopen_restored_snapshot = capture_grouping_snapshot(reopen_after_restore.stdout_text);
    expect_identity_matches(
        reopen_restored_snapshot,
        initial_snapshot,
        false,
        true,
        false,
        sample_case.issue_tag + ": reopen-after-restore real sample grouping read");
    expect_live_preview_matches(
        reopen_restored_snapshot,
        initial_snapshot,
        sample_case.issue_tag + ": reopen-after-restore real sample grouping read");
    expect_deleted_preview_cleared(
        reopen_restored_snapshot,
        sample_case.issue_tag + ": reopen-after-restore real sample grouping read");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_real_vfp9_report_sample_grouping_deleted_preview_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3647/#3649 real VFP9 report samples were not found\n";
        return;
    }

    const std::vector<RealSampleGroupingCase> sample_cases{
        {
            .issue_tag = "#3647",
            .sample_stem = "invoice",
            .primary_filename = "invoice.frx",
            .sidecar_filename = "invoice.frt",
            .original_grouping_expression = "invoice.order_id_a",
            .header_record_index = 2
        },
        {
            .issue_tag = "#3649",
            .sample_stem = "dbctofrx",
            .primary_filename = "dbctofrx.frx",
            .sidecar_filename = "dbctofrx.frt",
            .original_grouping_expression = "dbctofrx.parentid",
            .header_record_index = 2
        }
    };

    for (const auto& sample_case : sample_cases) {
        run_real_vfp9_report_sample_grouping_deleted_preview_round_trip(
            studio_host_path,
            sample_case,
            reports_root);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_grouping_deleted_preview_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_sample_grouping_deleted_preview_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
