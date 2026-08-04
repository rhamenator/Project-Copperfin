// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

struct RealDeletedPreviewSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::size_t record_index = 0U;
    std::string title;
    std::string unique_id;
    int live_preview_right = 0;
    int live_preview_bottom = 0;
    int live_preview_width = 0;
    int live_preview_height = 0;
    int deleted_preview_right = 0;
    int deleted_preview_bottom = 0;
    int deleted_preview_width = 0;
    int deleted_preview_height = 0;
    bool is_label = false;
};

void expect_common_selected_object_json(
    const std::string& json_text,
    const RealDeletedPreviewSample& sample,
    bool deleted,
    bool expect_undo,
    const std::string& prefix) {
    expect_contains(json_text,
                    "\"selectedObjectAvailable\": true",
                    prefix + " should expose a selected object");
    expect_contains(json_text,
                    "\"selectedReportSelectionKind\": \"object\"",
                    prefix + " should preserve object selection kind");
    expect_contains(json_text,
                    "\"title\": \"" + sample.title + "\"",
                    prefix + " should preserve the selected object title");
    expect_contains(json_text,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    prefix + " should preserve the selected object unique id");
    expect_contains(json_text,
                    deleted ? "\"deleted\": true" : "\"deleted\": false",
                    prefix + " should expose the expected deleted-state flag");
    expect_contains(json_text,
                    expect_undo ? "\"commandUndoAvailable\": true" : "\"commandUndoAvailable\": false",
                    prefix + " should expose the expected undo availability");
    expect_contains(json_text,
                    expect_undo ? "\"commandUndoLabel\": \"Deleted state\"" : "\"commandUndoLabel\": \"\"",
                    prefix + " should preserve the expected undo label");
    if (sample.is_label) {
        expect_contains(json_text, "\"isLabel\": true", prefix + " should preserve label identity");
    } else {
        expect_contains(json_text, "\"isLabel\": false", prefix + " should preserve report identity");
    }
}

void expect_preview_json(
    const std::string& json_text,
    const RealDeletedPreviewSample& sample,
    bool deleted_preview_available,
    const std::string& prefix) {
    expect_contains(json_text,
                    "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(json_text,
                    "\"previewBoundsRight\": " + std::to_string(sample.live_preview_right),
                    prefix + " should preserve live preview right");
    expect_contains(json_text,
                    "\"previewBoundsBottom\": " + std::to_string(sample.live_preview_bottom),
                    prefix + " should preserve live preview bottom");
    expect_contains(json_text,
                    "\"previewBoundsWidth\": " + std::to_string(sample.live_preview_width),
                    prefix + " should preserve live preview width");
    expect_contains(json_text,
                    "\"previewBoundsHeight\": " + std::to_string(sample.live_preview_height),
                    prefix + " should preserve live preview height");
    expect_contains(json_text,
                    deleted_preview_available ? "\"deletedPreviewBoundsAvailable\": true"
                                              : "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should expose the expected deleted-preview availability");
    if (deleted_preview_available) {
        expect_contains(json_text,
                        "\"deletedPreviewBoundsRight\": " + std::to_string(sample.deleted_preview_right),
                        prefix + " should expose deleted preview right");
        expect_contains(json_text,
                        "\"deletedPreviewBoundsBottom\": " + std::to_string(sample.deleted_preview_bottom),
                        prefix + " should expose deleted preview bottom");
        expect_contains(json_text,
                        "\"deletedPreviewBoundsWidth\": " + std::to_string(sample.deleted_preview_width),
                        prefix + " should expose deleted preview width");
        expect_contains(json_text,
                        "\"deletedPreviewBoundsHeight\": " + std::to_string(sample.deleted_preview_height),
                        prefix + " should expose deleted preview height");
    } else {
        expect_contains(json_text,
                        "\"deletedPreviewBoundsRight\": 0",
                        prefix + " should clear deleted preview right");
        expect_contains(json_text,
                        "\"deletedPreviewBoundsBottom\": 0",
                        prefix + " should clear deleted preview bottom");
        expect_contains(json_text,
                        "\"deletedPreviewBoundsWidth\": 0",
                        prefix + " should clear deleted preview width");
        expect_contains(json_text,
                        "\"deletedPreviewBoundsHeight\": 0",
                        prefix + " should clear deleted preview height");
    }
}

void exercise_real_sample_object_deleted_preview_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealDeletedPreviewSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3641: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3641: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3641: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3641: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial deleted-preview read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial deleted-preview read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3641: real sample object deleted-preview read should succeed");
    expect_common_selected_object_json(
        initial_process.stdout_text,
        sample,
        false,
        false,
        "#3641: initial real sample object read");
    expect_preview_json(
        initial_process.stdout_text,
        sample,
        false,
        "#3641: initial real sample object read");

    const auto delete_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--delete-object", "--json"},
        temp_root);
    if (delete_process.exit_code != 0) {
        std::cerr << "studio host real sample delete-preview stdout:\n"
                  << delete_process.stdout_text << "\n";
        std::cerr << "studio host real sample delete-preview stderr:\n"
                  << delete_process.stderr_text << "\n";
    }
    expect(delete_process.exit_code == 0, "#3641: real sample object delete should succeed");
    expect_common_selected_object_json(
        delete_process.stdout_text,
        sample,
        true,
        true,
        "#3641: deleted real sample object read");
    expect_preview_json(
        delete_process.stdout_text,
        sample,
        true,
        "#3641: deleted real sample object read");
    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3641: real sample object delete should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3641: real sample object delete should preserve sidecar bytes");

    const auto reopen_after_delete = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_after_delete.exit_code != 0) {
        std::cerr << "studio host real sample reopen-after-delete preview stdout:\n"
                  << reopen_after_delete.stdout_text << "\n";
        std::cerr << "studio host real sample reopen-after-delete preview stderr:\n"
                  << reopen_after_delete.stderr_text << "\n";
    }
    expect(reopen_after_delete.exit_code == 0, "#3641: real sample reopen after delete should succeed");
    expect_common_selected_object_json(
        reopen_after_delete.stdout_text,
        sample,
        true,
        true,
        "#3641: reopen-after-delete real sample object read");
    expect_preview_json(
        reopen_after_delete.stdout_text,
        sample,
        true,
        "#3641: reopen-after-delete real sample object read");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--restore-object", "--json"},
        temp_root);
    if (restore_process.exit_code != 0) {
        std::cerr << "studio host real sample restore-preview stdout:\n"
                  << restore_process.stdout_text << "\n";
        std::cerr << "studio host real sample restore-preview stderr:\n"
                  << restore_process.stderr_text << "\n";
    }
    expect(restore_process.exit_code == 0, "#3641: real sample object restore should succeed");
    expect_common_selected_object_json(
        restore_process.stdout_text,
        sample,
        false,
        true,
        "#3641: restored real sample object read");
    expect_preview_json(
        restore_process.stdout_text,
        sample,
        false,
        "#3641: restored real sample object read");
    expect(read_binary(copied_primary) == original_primary_bytes,
           "#3641: real sample object restore should rewind primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3641: real sample object restore should rewind sidecar bytes");

    const auto reopen_after_restore = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_after_restore.exit_code != 0) {
        std::cerr << "studio host real sample reopen-after-restore preview stdout:\n"
                  << reopen_after_restore.stdout_text << "\n";
        std::cerr << "studio host real sample reopen-after-restore preview stderr:\n"
                  << reopen_after_restore.stderr_text << "\n";
    }
    expect(reopen_after_restore.exit_code == 0, "#3641: real sample reopen after restore should succeed");
    expect_common_selected_object_json(
        reopen_after_restore.stdout_text,
        sample,
        false,
        true,
        "#3641: reopen-after-restore real sample object read");
    expect_preview_json(
        reopen_after_restore.stdout_text,
        sample,
        false,
        "#3641: reopen-after-restore real sample object read");
}

void test_real_vfp9_report_and_label_sample_object_deleted_preview_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3641 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() /
        ("copperfin_studio_host_real_vfp9_sample_object_deleted_preview_round_trip_tests_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_object_deleted_preview_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .record_index = 9U,
            .title = "invoice.order_id_b",
            .unique_id = "_R8X0QSY9C",
            .live_preview_right = 76354,
            .live_preview_bottom = 61770,
            .live_preview_width = 76354,
            .live_preview_height = 61770,
            .deleted_preview_right = 76041,
            .deleted_preview_bottom = 17291,
            .deleted_preview_width = 8229,
            .deleted_preview_height = 2187,
            .is_label = false
        });
    exercise_real_sample_object_deleted_preview_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .record_index = 6U,
            .title = "customer.company",
            .unique_id = "_RAF0WG5W6",
            .live_preview_right = 39374,
            .live_preview_bottom = 13437,
            .live_preview_width = 39374,
            .live_preview_height = 13437,
            .deleted_preview_right = 39374,
            .deleted_preview_bottom = 6770,
            .deleted_preview_width = 31562,
            .deleted_preview_height = 1979,
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_object_deleted_preview_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_sample_object_deleted_preview_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
