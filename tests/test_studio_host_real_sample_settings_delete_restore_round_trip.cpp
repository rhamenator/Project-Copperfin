// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

struct RealSettingsLifecycleSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::string document_title;
    std::string unique_id;
    bool is_label = false;
};

void expect_common_settings_json(
    std::string json,
    const RealSettingsLifecycleSample& sample,
    bool deleted,
    bool expect_undo,
    int setting_count,
    int deleted_setting_count,
    const std::string& prefix) {
    json = copperfin::test_support::normalize_json_line_endings(std::move(json));
    expect_contains(json,
                    "\"documentTitle\": \"" + sample.document_title + "\"",
                    prefix + " should preserve the document title");
    expect_contains(json,
                    "\"selectedObjectAvailable\": true",
                    prefix + " should expose the root record");
    expect_contains(json,
                    "\"selectedReportSettingsAvailable\": true",
                    prefix + " should preserve selected settings availability");
    expect_contains(json,
                    "\"selectedReportSelectionKind\": \"settings\"",
                    prefix + " should keep the settings selection kind");
    expect_contains(json,
                    "\"settingCount\": " + std::to_string(setting_count),
                    prefix + " should expose the expected live settings count");
    expect_contains(json,
                    "\"deletedSettingCount\": " + std::to_string(deleted_setting_count),
                    prefix + " should expose the expected deleted settings count");
    expect_contains(json,
                    "\"recordIndex\": 0,\n      \"deleted\": " + std::string(deleted ? "true" : "false"),
                    prefix + " should expose the expected deleted state for the root record");
    expect_contains(json,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    prefix + " should preserve the root record identity");
    expect_contains(json,
                    expect_undo ? "\"commandUndoAvailable\": true" : "\"commandUndoAvailable\": false",
                    prefix + " should expose the expected undo availability");
    expect_contains(json,
                    expect_undo ? "\"commandUndoLabel\": \"Deleted state\"" : "\"commandUndoLabel\": \"\"",
                    prefix + " should expose the expected undo label");
    if (sample.is_label) {
        expect_contains(json, "\"isLabel\": true", prefix + " should preserve label identity");
    } else {
        expect_contains(json, "\"isLabel\": false", prefix + " should preserve report identity");
    }
}

void exercise_real_sample_settings_delete_restore_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealSettingsLifecycleSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3714: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3714: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3714: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3714: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial settings read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial settings read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3714: initial real sample settings read should succeed");
    int initial_setting_count = 0;
    expect(
        copperfin::test_support::extract_json_integer(
            initial_process.stdout_text, "\"settingCount\"", initial_setting_count) &&
            initial_setting_count >= 0,
        "#3714: initial real sample settings read should expose a numeric setting count");
    expect_common_settings_json(
        initial_process.stdout_text,
        sample,
        false,
        false,
        initial_setting_count,
        0,
        "#3714: initial real sample settings read");

    const auto delete_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--delete-object", "--record", "0", "--json"},
        temp_root);
    if (delete_process.exit_code != 0) {
        std::cerr << "studio host real sample settings delete stdout:\n"
                  << delete_process.stdout_text << "\n";
        std::cerr << "studio host real sample settings delete stderr:\n"
                  << delete_process.stderr_text << "\n";
    }
    expect(delete_process.exit_code == 0, "#3714: real sample settings delete should succeed");
    expect_common_settings_json(
        delete_process.stdout_text,
        sample,
        true,
        true,
        0,
        initial_setting_count,
        "#3714: delete real sample settings");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3714: real sample settings delete should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3714: real sample settings delete should preserve sidecar bytes");

    const auto reopen_after_delete = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_delete.exit_code != 0) {
        std::cerr << "studio host real sample settings reopen-after-delete stdout:\n"
                  << reopen_after_delete.stdout_text << "\n";
        std::cerr << "studio host real sample settings reopen-after-delete stderr:\n"
                  << reopen_after_delete.stderr_text << "\n";
    }
    expect(reopen_after_delete.exit_code == 0, "#3714: real sample settings reopen after delete should succeed");
    expect_common_settings_json(
        reopen_after_delete.stdout_text,
        sample,
        true,
        true,
        0,
        initial_setting_count,
        "#3714: reopen after delete");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--restore-object", "--record", "0", "--json"},
        temp_root);
    if (restore_process.exit_code != 0) {
        std::cerr << "studio host real sample settings restore stdout:\n"
                  << restore_process.stdout_text << "\n";
        std::cerr << "studio host real sample settings restore stderr:\n"
                  << restore_process.stderr_text << "\n";
    }
    expect(restore_process.exit_code == 0, "#3714: real sample settings restore should succeed");
    expect_common_settings_json(
        restore_process.stdout_text,
        sample,
        false,
        true,
        initial_setting_count,
        0,
        "#3714: restore real sample settings");

    expect(read_binary(copied_primary) == original_primary_bytes,
           "#3714: real sample settings restore should rewind primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3714: real sample settings restore should preserve sidecar rewind");

    const auto reopen_after_restore = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_restore.exit_code != 0) {
        std::cerr << "studio host real sample settings reopen-after-restore stdout:\n"
                  << reopen_after_restore.stdout_text << "\n";
        std::cerr << "studio host real sample settings reopen-after-restore stderr:\n"
                  << reopen_after_restore.stderr_text << "\n";
    }
    expect(reopen_after_restore.exit_code == 0, "#3714: real sample settings reopen after restore should succeed");
    expect_common_settings_json(
        reopen_after_restore.stdout_text,
        sample,
        false,
        true,
        initial_setting_count,
        0,
        "#3714: reopen after restore");
}

void test_real_vfp9_report_and_label_settings_delete_restore_round_trip(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3714 real VFP9 report samples were not found\n";
        return;
    }

    const auto unique_suffix =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const fs::path temp_root = fs::temp_directory_path() /
                               ("copperfin_studio_host_real_vfp9_settings_delete_restore_round_trip_tests_" +
                                unique_suffix);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_settings_delete_restore_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .document_title = "invoice.frx",
            .unique_id = "_R8X0QSAU8",
            .is_label = false
        });
    exercise_real_sample_settings_delete_restore_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .document_title = "cust.lbx",
            .unique_id = "_RAF0WEKA5",
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_settings_delete_restore_round_trip "
                     "<studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_settings_delete_restore_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
