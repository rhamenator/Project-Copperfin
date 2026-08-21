// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/visual_asset_editor.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"
#include "test_studio_host_real_sample_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;
using copperfin::test_support::getenv_value;

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

std::filesystem::path try_resolve_asset_under_root(
    const std::filesystem::path& root,
    const std::string& archive_relative_path) {
    namespace fs = std::filesystem;

    if (root.empty()) {
        return {};
    }

    const fs::path normalized_relative = fs::path(archive_relative_path);
    const fs::path rooted_candidate = root / normalized_relative;
    if (fs::exists(rooted_candidate)) {
        return rooted_candidate;
    }

    constexpr std::string_view prefix = "VFPSource/";
    if (archive_relative_path.rfind(prefix, 0U) == 0U) {
        const fs::path stripped_candidate = root / archive_relative_path.substr(prefix.size());
        if (fs::exists(stripped_candidate)) {
            return stripped_candidate;
        }
    }

    return {};
}

std::filesystem::path find_vfp_source_style3v_path() {
    namespace fs = std::filesystem;

    const std::string archive_relative_path = "VFPSource/Wizards/wzreport/STYLES/STYLE3V.FRX";

    std::vector<fs::path> candidate_roots;
    if (const std::string configured_root = getenv_value("COPPERFIN_VFPSOURCE_ROOT");
        !configured_root.empty()) {
        candidate_roots.emplace_back(configured_root);
    }

    candidate_roots.emplace_back("/tmp/vfpsource_3217");
    candidate_roots.emplace_back("/tmp/vfpsource_3217/VFPSource");

    if (const std::string home = getenv_value("HOME"); !home.empty()) {
        candidate_roots.emplace_back(fs::path(home) / "Downloads" / "VFPSource");
    }

    for (const auto& root : candidate_roots) {
        const fs::path resolved = try_resolve_asset_under_root(root, archive_relative_path);
        if (!resolved.empty()) {
            return resolved;
        }
    }

    return {};
}

struct RealObjectDistributeSample {
    std::string distribution_mode;
    std::string changed_property_name;
    std::string original_hpos;
    std::string original_vpos;
    std::string updated_hpos;
    std::string updated_vpos;
    int original_left = 0;
    int original_top = 0;
    int original_right = 0;
    int original_bottom = 0;
    int original_section_relative_top = 0;
    int original_section_relative_bottom = 0;
    int updated_left = 0;
    int updated_top = 0;
    int updated_right = 0;
    int updated_bottom = 0;
    int updated_section_relative_top = 0;
    int updated_section_relative_bottom = 0;
};

void expect_common_object_distribute_json(
    const std::string& json_text,
    int expected_left,
    int expected_top,
    int expected_right,
    int expected_bottom,
    int expected_section_relative_top,
    int expected_section_relative_bottom,
    const std::string& context) {
    expect_contains(
        json_text,
        "\"status\": \"ok\"",
        context + " should return ok status");
    expect_contains(
        json_text,
        "\"selectedObjectAvailable\": true",
        context + " should expose a selected object");
    expect_contains(
        json_text,
        "\"selectedReportObjectAvailable\": true",
        context + " should expose the selected report object");
    expect_contains(
        json_text,
        "\"selectedReportObjectSectionAvailable\": true",
        context + " should expose the selected report section");
    expect_contains(
        json_text,
        "\"selectedReportSelectionAvailable\": true",
        context + " should expose report selection state");
    expect_contains(
        json_text,
        "\"selectedReportSelectionKind\": \"object\"",
        context + " should preserve object selection kind");
    expect_contains(
        json_text,
        "\"assetFamily\": \"report\"",
        context + " should preserve report asset identity");
    expect_contains(
        json_text,
        "\"isLabel\": false",
        context + " should preserve report layout identity");
    expect_contains(
        json_text,
        "\"recordIndex\": 13",
        context + " should preserve the focused record");
    expect_contains(
        json_text,
        "\"title\": \"\\\"TITLE\\\"\"",
        context + " should preserve the focused object title");
    expect_contains(
        json_text,
        "\"uniqueId\": \"_QVL0O0NVK\"",
        context + " should preserve the focused object unique id");
    expect_contains(
        json_text,
        "\"containingSectionId\": \"_QEE1DQLGZ\"",
        context + " should preserve the containing section id");
    expect_contains(
        json_text,
        "\"containingSectionRecordIndex\": 2",
        context + " should preserve the containing section record");
    expect_contains(
        json_text,
        "\"title\": \"Page Header\"",
        context + " should preserve the section title");
    expect_contains(
        json_text,
        "\"sectionCount\": 4",
        context + " should preserve the report section count");
    expect_contains(
        json_text,
        "\"left\": " + std::to_string(expected_left),
        context + " should expose the expected left coordinate");
    expect_contains(
        json_text,
        "\"top\": " + std::to_string(expected_top),
        context + " should expose the expected top coordinate");
    expect_contains(
        json_text,
        "\"right\": " + std::to_string(expected_right),
        context + " should expose the expected right coordinate");
    expect_contains(
        json_text,
        "\"bottom\": " + std::to_string(expected_bottom),
        context + " should expose the expected bottom coordinate");
    expect_contains(
        json_text,
        "\"sectionRelativeTop\": " + std::to_string(expected_section_relative_top),
        context + " should expose the expected section-relative top coordinate");
    expect_contains(
        json_text,
        "\"sectionRelativeBottom\": " + std::to_string(expected_section_relative_bottom),
        context + " should expose the expected section-relative bottom coordinate");
    expect_contains(
        json_text,
        "\"previewBoundsAvailable\": true",
        context + " should expose preview bounds");
    expect_contains(
        json_text,
        "\"previewBoundsRight\": 75000",
        context + " should preserve the preview right bound");
    expect_contains(
        json_text,
        "\"previewBoundsBottom\": 21666",
        context + " should preserve the preview bottom bound");
    expect_contains(
        json_text,
        "\"previewBoundsWidth\": 75000",
        context + " should preserve the preview width");
    expect_contains(
        json_text,
        "\"previewBoundsHeight\": 21666",
        context + " should preserve the preview height");
    expect_contains(
        json_text,
        "\"deletedPreviewBoundsAvailable\": false",
        context + " should keep deleted preview bounds unavailable");
}

void exercise_real_sample_object_distribute_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::filesystem::path& source_primary,
    const RealObjectDistributeSample& sample) {
    namespace fs = std::filesystem;

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path copied_primary = temp_root / source_primary.filename();
    const fs::path source_sidecar = fs::path(source_primary).replace_extension(".FRT");
    const fs::path copied_sidecar = temp_root / source_sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(source_primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3833: should copy the real report asset into temp space");
    copy_error.clear();
    fs::copy_file(source_sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3833: should copy the real report sidecar into temp space");
    expect(make_writable(copied_primary), "#3833: copied report asset should become writable");
    expect(make_writable(copied_sidecar), "#3833: copied report sidecar should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "13", "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample distribute initial read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample distribute initial read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3833: real sample distribute initial read should succeed");
    expect_common_object_distribute_json(
        initial_process.stdout_text,
        sample.original_left,
        sample.original_top,
        sample.original_right,
        sample.original_bottom,
        sample.original_section_relative_top,
        sample.original_section_relative_bottom,
        "#3833: initial real sample distribute read");

    const auto distribute_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--record", "13",
            "--distribute-object",
            "--distribution-mode", sample.distribution_mode,
            "--distribute-target-unique-id", "_QVL0O0NVJ",
            "--distribute-target-unique-id", "_QVL0O0NVK",
            "--distribute-target-unique-id", "_QVL0O0NV9",
            "--json"
        },
        temp_root);
    if (distribute_process.exit_code != 0) {
        std::cerr << "studio host real sample distribute stdout:\n"
                  << distribute_process.stdout_text << "\n";
        std::cerr << "studio host real sample distribute stderr:\n"
                  << distribute_process.stderr_text << "\n";
    }
    expect(distribute_process.exit_code == 0, "#3833: real sample distribute should succeed");
    expect_common_object_distribute_json(
        distribute_process.stdout_text,
        sample.updated_left,
        sample.updated_top,
        sample.updated_right,
        sample.updated_bottom,
        sample.updated_section_relative_top,
        sample.updated_section_relative_bottom,
        "#3833: updated real sample distribute read");

    const auto updated_hpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 13U,
        .object_name = {},
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(updated_hpos.ok && updated_hpos.exists && updated_hpos.value == sample.updated_hpos,
           "#3833: real sample distribute should persist the expected HPOS field");
    const auto updated_vpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 13U,
        .object_name = {},
        .unique_id = {},
        .property_name = "VPOS"
    });
    expect(updated_vpos.ok && updated_vpos.exists && updated_vpos.value == sample.updated_vpos,
           "#3833: real sample distribute should persist the expected VPOS field");
    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3833: real sample distribute should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3833: real sample distribute should preserve sidecar bytes");

    const auto undo_status = copperfin::vfp::query_visual_object_undo(copied_primary.string());
    expect(undo_status.available, "#3833: real sample distribute should leave undo history available");
    expect(undo_status.label == "Property " + sample.changed_property_name,
           "#3833: real sample distribute should preserve the undo label");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "13", "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample distribute reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample distribute reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3833: real sample distribute reopen should succeed");
    expect_common_object_distribute_json(
        reopen_process.stdout_text,
        sample.updated_left,
        sample.updated_top,
        sample.updated_right,
        sample.updated_bottom,
        sample.updated_section_relative_top,
        sample.updated_section_relative_bottom,
        "#3833: reopened updated real sample distribute read");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(copied_primary.string());
    expect(undo_result.ok, "#3833: real sample distribute undo should succeed");

    const auto restored_hpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 13U,
        .object_name = {},
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(restored_hpos.ok && restored_hpos.exists && restored_hpos.value == sample.original_hpos,
           "#3833: real sample distribute undo should restore the HPOS field");
    const auto restored_vpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 13U,
        .object_name = {},
        .unique_id = {},
        .property_name = "VPOS"
    });
    expect(restored_vpos.ok && restored_vpos.exists && restored_vpos.value == sample.original_vpos,
           "#3833: real sample distribute undo should restore the VPOS field");
    const std::string restored_primary_bytes = read_binary(copied_primary);
    expect(copperfin::test_support::dbf_bytes_match_except_last_update_date(
               original_primary_bytes, restored_primary_bytes),
           "#3833: real sample distribute undo should restore every primary asset byte except the DBF update date");
    expect(copperfin::test_support::dbf_last_update_date_matches_local_calendar(restored_primary_bytes),
           "#3833: real sample distribute undo should stamp the DBF update date");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3833: real sample distribute undo should preserve restored sidecar bytes");

    const auto cleared_undo_status = copperfin::vfp::query_visual_object_undo(copied_primary.string());
    expect(!cleared_undo_status.available, "#3833: real sample distribute undo should clear undo history");

    const auto reopen_undo_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "13", "--json"},
        temp_root);
    if (reopen_undo_process.exit_code != 0) {
        std::cerr << "studio host real sample distribute undo reopen stdout:\n"
                  << reopen_undo_process.stdout_text << "\n";
        std::cerr << "studio host real sample distribute undo reopen stderr:\n"
                  << reopen_undo_process.stderr_text << "\n";
    }
    expect(reopen_undo_process.exit_code == 0, "#3833: real sample distribute undo reopen should succeed");
    expect_common_object_distribute_json(
        reopen_undo_process.stdout_text,
        sample.original_left,
        sample.original_top,
        sample.original_right,
        sample.original_bottom,
        sample.original_section_relative_top,
        sample.original_section_relative_bottom,
        "#3833: reopened undone real sample distribute read");
}

void test_real_vfp_source_style3v_object_distribute_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path source_primary = find_vfp_source_style3v_path();
    if (source_primary.empty()) {
        std::cerr << "SKIP: #3833 real VFPSource STYLE3V sample was not found\n";
        return;
    }

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto temp_suffix =
        std::to_string(static_cast<long long>(std::chrono::steady_clock::now().time_since_epoch().count()));
    const fs::path temp_root = fs::temp_directory_path() /
                               ("copperfin_studio_host_real_vfpsource_style3v_object_distribute_round_trip_tests_" +
                                temp_suffix);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    exercise_real_sample_object_distribute_round_trip(
        studio_host_path,
        temp_root / "horizontal",
        source_primary,
        {
            .distribution_mode = "horizontal",
            .changed_property_name = "HPOS",
            .original_hpos = "416.667",
            .original_vpos = "3541.667",
            .updated_hpos = "364.583",
            .updated_vpos = "3541.667",
            .original_left = 416,
            .original_top = 3541,
            .original_right = 6249,
            .original_bottom = 5936,
            .original_section_relative_top = 3541,
            .original_section_relative_bottom = 5936,
            .updated_left = 364,
            .updated_top = 3541,
            .updated_right = 6197,
            .updated_bottom = 5936,
            .updated_section_relative_top = 3541,
            .updated_section_relative_bottom = 5936
        });

    exercise_real_sample_object_distribute_round_trip(
        studio_host_path,
        temp_root / "vertical",
        source_primary,
        {
            .distribution_mode = "vertical",
            .changed_property_name = "VPOS",
            .original_hpos = "416.667",
            .original_vpos = "3541.667",
            .updated_hpos = "416.667",
            .updated_vpos = "4427.083",
            .original_left = 416,
            .original_top = 3541,
            .original_right = 6249,
            .original_bottom = 5936,
            .original_section_relative_top = 3541,
            .original_section_relative_bottom = 5936,
            .updated_left = 416,
            .updated_top = 4427,
            .updated_right = 6249,
            .updated_bottom = 6822,
            .updated_section_relative_top = 4427,
            .updated_section_relative_bottom = 6822
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_real_sample_object_distribute_round_trip <copperfin_studio_host>\n";
        return 2;
    }

    test_real_vfp_source_style3v_object_distribute_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
