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

struct RealSectionSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::size_t record_index = 0;
    std::string title;
    std::string original_height;
    std::string updated_height;
    std::string updated_json_height;
    bool is_label = false;
};

void exercise_real_sample_section_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealSectionSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3529: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3529: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3529: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3529: copied sidecar asset should become writable");

    const auto original_height = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "HEIGHT"
    });
    expect(original_height.ok && original_height.exists && original_height.value == sample.original_height,
           "#3529: real sample section should expose the expected original HEIGHT");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial section read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial section read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3529: real sample section read should succeed");
    expect_contains(initial_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"section\"",
                    "#3529: real sample section read should preserve section selection kind");
    expect_contains(initial_process.stdout_text,
                    "\"title\": \"" + sample.title + "\"",
                    "#3529: real sample section read should preserve the section title");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--record", std::to_string(sample.record_index),
            "--set-property",
            "--property-name", "HEIGHT",
            "--property-value", sample.updated_height,
            "--json"
        },
        temp_root);
    if (update_process.exit_code != 0) {
        std::cerr << "studio host real sample section HEIGHT update stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host real sample section HEIGHT update stderr:\n"
                  << update_process.stderr_text << "\n";
    }
    expect(update_process.exit_code == 0, "#3529: real sample section HEIGHT update should succeed");
    expect_contains(update_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"section\"",
                    "#3529: section HEIGHT update should preserve section selection kind");
    expect_contains(update_process.stdout_text,
                    "\"title\": \"" + sample.title + "\"",
                    "#3529: section HEIGHT update should preserve the section title");
    expect_contains(update_process.stdout_text,
                    "\"recordIndex\": " + std::to_string(sample.record_index),
                    "#3529: section HEIGHT update should preserve the section record identity");
    expect_contains(update_process.stdout_text,
                    "\"height\": " + sample.updated_json_height,
                    "#3529: section HEIGHT update should report the updated section height");

    const auto updated_height = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "HEIGHT"
    });
    expect(updated_height.ok && updated_height.exists && updated_height.value == sample.updated_height,
           "#3529: real sample section HEIGHT update should persist the direct field");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3529: real sample section HEIGHT update should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3529: real sample section HEIGHT update should preserve sidecar bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample section reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample section reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3529: real sample section reopen should succeed");
    expect_contains(reopen_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"section\"",
                    "#3529: real sample section reopen should preserve section selection kind");
    expect_contains(reopen_process.stdout_text,
                    "\"title\": \"" + sample.title + "\"",
                    "#3529: real sample section reopen should preserve the section title");
    expect_contains(reopen_process.stdout_text,
                    "\"recordIndex\": " + std::to_string(sample.record_index),
                    "#3529: real sample section reopen should preserve the section record identity");
    expect_contains(reopen_process.stdout_text,
                    "\"height\": " + sample.updated_json_height,
                    "#3529: real sample section reopen should expose the updated section height");
    if (sample.is_label) {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": true",
                        "#3529: label sample section reopen should preserve label identity");
    } else {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": false",
                        "#3529: report sample section reopen should preserve report identity");
    }
}

void test_real_vfp9_report_and_label_sample_section_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3529 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_section_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_section_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .record_index = 3U,
            .title = "Detail",
            .original_height = "2188.000",
            .updated_height = "2500.000",
            .updated_json_height = "2500",
            .is_label = false
        });
    exercise_real_sample_section_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .record_index = 3U,
            .title = "Detail",
            .original_height = "10000.000",
            .updated_height = "12000.000",
            .updated_json_height = "12000",
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_section_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_sample_section_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
