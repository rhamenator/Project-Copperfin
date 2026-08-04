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

struct RealObjectSnapSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::size_t record_index = 0U;
    std::string title;
    std::string unique_id;
    std::string original_hpos;
    std::string original_vpos;
    std::string snapped_hpos;
    std::string snapped_vpos;
    int original_left = 0;
    int original_top = 0;
    int original_right = 0;
    int original_bottom = 0;
    int original_section_relative_top = 0;
    int original_section_relative_bottom = 0;
    int snapped_left = 0;
    int snapped_top = 0;
    int snapped_right = 0;
    int snapped_bottom = 0;
    int snapped_section_relative_top = 0;
    int snapped_section_relative_bottom = 0;
    int preview_right = 0;
    int preview_bottom = 0;
    int preview_width = 0;
    int preview_height = 0;
    bool is_label = false;
};

void expect_common_object_snap_json(
    const std::string& json_text,
    const RealObjectSnapSample& sample,
    const std::string& expected_hpos,
    const std::string& expected_vpos,
    int expected_left,
    int expected_top,
    int expected_right,
    int expected_bottom,
    int expected_section_relative_top,
    int expected_section_relative_bottom,
    bool expect_undo,
    const std::string& context) {
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
        "\"selectedReportSelectionKind\": \"object\"",
        context + " should preserve object selection kind");
    expect_contains(
        json_text,
        "\"title\": \"" + sample.title + "\"",
        context + " should preserve the selected object title");
    expect_contains(
        json_text,
        "\"uniqueId\": \"" + sample.unique_id + "\"",
        context + " should preserve the selected object unique id");
    expect_contains(
        json_text,
        "\"name\": \"HPOS\", \"type\": \"N\", \"isNull\": false, \"value\": \"" + expected_hpos + "\"",
        context + " should expose the expected HPOS value");
    expect_contains(
        json_text,
        "\"name\": \"VPOS\", \"type\": \"N\", \"isNull\": false, \"value\": \"" + expected_vpos + "\"",
        context + " should expose the expected VPOS value");
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
        expect_undo ? "\"commandUndoAvailable\": true" : "\"commandUndoAvailable\": false",
        context + " should expose the expected command-undo availability");
    expect_contains(
        json_text,
        expect_undo ? "\"commandUndoLabel\": \"Property VPOS\"" : "\"commandUndoLabel\": \"\"",
        context + " should expose the expected command-undo label");
    expect_contains(
        json_text,
        "\"previewBoundsAvailable\": true",
        context + " should expose live preview bounds");
    expect_contains(
        json_text,
        "\"previewBoundsRight\": " + std::to_string(sample.preview_right),
        context + " should expose the expected preview right bound");
    expect_contains(
        json_text,
        "\"previewBoundsBottom\": " + std::to_string(sample.preview_bottom),
        context + " should expose the expected preview bottom bound");
    expect_contains(
        json_text,
        "\"previewBoundsWidth\": " + std::to_string(sample.preview_width),
        context + " should expose the expected preview width");
    expect_contains(
        json_text,
        "\"previewBoundsHeight\": " + std::to_string(sample.preview_height),
        context + " should expose the expected preview height");
    expect_contains(
        json_text,
        "\"deletedPreviewBoundsAvailable\": false",
        context + " should keep deleted preview bounds unavailable");

    if (sample.is_label) {
        expect_contains(
            json_text,
            "\"isLabel\": true",
            context + " should preserve label identity");
    } else {
        expect_contains(
            json_text,
            "\"isLabel\": false",
            context + " should preserve report identity");
    }
}

void exercise_real_sample_object_snap_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealObjectSnapSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3657: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3657: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3657: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3657: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial snap read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial snap read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3657: real sample object read should succeed");
    expect_common_object_snap_json(
        initial_process.stdout_text,
        sample,
        sample.original_hpos,
        sample.original_vpos,
        sample.original_left,
        sample.original_top,
        sample.original_right,
        sample.original_bottom,
        sample.original_section_relative_top,
        sample.original_section_relative_bottom,
        false,
        "#3657: initial real sample object read");

    const auto snap_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--record", std::to_string(sample.record_index),
            "--unique-id", sample.unique_id,
            "--snap-object",
            "--snap-mode", "both",
            "--grid-width", "2500",
            "--grid-height", "2500",
            "--snap-target-unique-id", sample.unique_id,
            "--json"
        },
        temp_root);
    if (snap_process.exit_code != 0) {
        std::cerr << "studio host real sample snap stdout:\n"
                  << snap_process.stdout_text << "\n";
        std::cerr << "studio host real sample snap stderr:\n"
                  << snap_process.stderr_text << "\n";
    }
    expect(snap_process.exit_code == 0, "#3657: real sample object snap should succeed");
    expect_common_object_snap_json(
        snap_process.stdout_text,
        sample,
        sample.snapped_hpos,
        sample.snapped_vpos,
        sample.snapped_left,
        sample.snapped_top,
        sample.snapped_right,
        sample.snapped_bottom,
        sample.snapped_section_relative_top,
        sample.snapped_section_relative_bottom,
        true,
        "#3657: snapped real sample object read");

    const auto snapped_hpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(snapped_hpos.ok && snapped_hpos.exists && snapped_hpos.value == sample.snapped_hpos,
           "#3657: real sample object snap should persist the HPOS field");
    const auto snapped_vpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "VPOS"
    });
    expect(snapped_vpos.ok && snapped_vpos.exists && snapped_vpos.value == sample.snapped_vpos,
           "#3657: real sample object snap should persist the VPOS field");
    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3657: real sample object snap should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3657: real sample object snap should preserve sidecar bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample snap reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample snap reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3657: real sample object snap reopen should succeed");
    expect_common_object_snap_json(
        reopen_process.stdout_text,
        sample,
        sample.snapped_hpos,
        sample.snapped_vpos,
        sample.snapped_left,
        sample.snapped_top,
        sample.snapped_right,
        sample.snapped_bottom,
        sample.snapped_section_relative_top,
        sample.snapped_section_relative_bottom,
        true,
        "#3657: reopened snapped real sample object read");

    const auto undo_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--record", std::to_string(sample.record_index),
            "--undo-mode", "command",
            "--json"
        },
        temp_root);
    if (undo_process.exit_code != 0) {
        std::cerr << "studio host real sample snap undo stdout:\n"
                  << undo_process.stdout_text << "\n";
        std::cerr << "studio host real sample snap undo stderr:\n"
                  << undo_process.stderr_text << "\n";
    }
    expect(undo_process.exit_code == 0, "#3657: real sample object snap undo should succeed");
    expect_common_object_snap_json(
        undo_process.stdout_text,
        sample,
        sample.original_hpos,
        sample.original_vpos,
        sample.original_left,
        sample.original_top,
        sample.original_right,
        sample.original_bottom,
        sample.original_section_relative_top,
        sample.original_section_relative_bottom,
        false,
        "#3657: undone real sample object read");

    const auto restored_hpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(restored_hpos.ok && restored_hpos.exists && restored_hpos.value == sample.original_hpos,
           "#3657: real sample object snap undo should restore the HPOS field");
    const auto restored_vpos = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "VPOS"
    });
    expect(restored_vpos.ok && restored_vpos.exists && restored_vpos.value == sample.original_vpos,
           "#3657: real sample object snap undo should restore the VPOS field");
    expect(read_binary(copied_primary) == original_primary_bytes,
           "#3657: real sample object snap undo should restore the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3657: real sample object snap undo should preserve restored sidecar bytes");

    const auto reopen_undo_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_undo_process.exit_code != 0) {
        std::cerr << "studio host real sample snap undo reopen stdout:\n"
                  << reopen_undo_process.stdout_text << "\n";
        std::cerr << "studio host real sample snap undo reopen stderr:\n"
                  << reopen_undo_process.stderr_text << "\n";
    }
    expect(reopen_undo_process.exit_code == 0, "#3657: real sample object snap undo reopen should succeed");
    expect_common_object_snap_json(
        reopen_undo_process.stdout_text,
        sample,
        sample.original_hpos,
        sample.original_vpos,
        sample.original_left,
        sample.original_top,
        sample.original_right,
        sample.original_bottom,
        sample.original_section_relative_top,
        sample.original_section_relative_bottom,
        false,
        "#3657: reopened undone real sample object read");
}

void test_real_vfp9_report_and_label_sample_object_snap_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3657 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_object_snap_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_object_snap_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .record_index = 9U,
            .title = "invoice.order_id_b",
            .unique_id = "_R8X0QSY9C",
            .original_hpos = "67812.500",
            .original_vpos = "15104.167",
            .snapped_hpos = "67500",
            .snapped_vpos = "15000",
            .original_left = 67812,
            .original_top = 15104,
            .original_right = 76041,
            .original_bottom = 17291,
            .original_section_relative_top = 15104,
            .original_section_relative_bottom = 17291,
            .snapped_left = 67500,
            .snapped_top = 15000,
            .snapped_right = 75729,
            .snapped_bottom = 17187,
            .snapped_section_relative_top = 15000,
            .snapped_section_relative_bottom = 17187,
            .preview_right = 76354,
            .preview_bottom = 61770,
            .preview_width = 76354,
            .preview_height = 61770,
            .is_label = false
        });
    exercise_real_sample_object_snap_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .record_index = 6U,
            .title = "customer.company",
            .unique_id = "_RAF0WG5W6",
            .original_hpos = "7812.500",
            .original_vpos = "4791.667",
            .snapped_hpos = "7500",
            .snapped_vpos = "5000",
            .original_left = 7812,
            .original_top = 4791,
            .original_right = 39374,
            .original_bottom = 6770,
            .original_section_relative_top = 4791,
            .original_section_relative_bottom = 6770,
            .snapped_left = 7500,
            .snapped_top = 5000,
            .snapped_right = 39062,
            .snapped_bottom = 6979,
            .snapped_section_relative_top = 5000,
            .snapped_section_relative_bottom = 6979,
            .preview_right = 39374,
            .preview_bottom = 13437,
            .preview_width = 39374,
            .preview_height = 13437,
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_object_snap_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_sample_object_snap_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
