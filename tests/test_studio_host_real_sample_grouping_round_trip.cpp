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

void test_real_vfp9_report_sample_grouping_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3530 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_grouping_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path copied_primary = temp_root / "invoice.frx";
    const fs::path copied_sidecar = temp_root / "invoice.frt";
    std::error_code copy_error;
    fs::copy_file(reports_root / "invoice.frx", copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3530: should copy the real report asset into temp space");
    copy_error.clear();
    fs::copy_file(reports_root / "invoice.frt", copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3530: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3530: copied report asset should become writable");
    expect(make_writable(copied_sidecar), "#3530: copied sidecar asset should become writable");

    constexpr std::size_t header_record_index = 2U;
    constexpr std::size_t footer_record_index = 4U;
    const std::string original_grouping_expression = "invoice.order_id_a";
    const std::string updated_grouping_expression = "invoice.order_date";

    const auto original_expr = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = header_record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(original_expr.ok && original_expr.exists && original_expr.value == original_grouping_expression,
           "#3530: real report grouping header should expose the expected original EXPR");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(header_record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial grouping read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial grouping read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3530: real sample grouping read should succeed");
    expect_contains(initial_process.stdout_text,
                    "\"groupingCount\": 1",
                    "#3530: real sample grouping read should expose a single grouping");
    expect_contains(initial_process.stdout_text,
                    "\"headerRecordIndex\": " + std::to_string(header_record_index),
                    "#3530: real sample grouping read should preserve the header record identity");
    expect_contains(initial_process.stdout_text,
                    "\"footerRecordIndex\": " + std::to_string(footer_record_index),
                    "#3530: real sample grouping read should preserve the footer record identity");
    expect_contains(initial_process.stdout_text,
                    "\"groupingExpression\": \"" + original_grouping_expression + "\"",
                    "#3530: real sample grouping read should expose the original grouping expression");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--record", std::to_string(header_record_index),
            "--set-property",
            "--property-name", "EXPR",
            "--property-value", updated_grouping_expression,
            "--json"
        },
        temp_root);
    if (update_process.exit_code != 0) {
        std::cerr << "studio host real sample grouping EXPR update stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host real sample grouping EXPR update stderr:\n"
                  << update_process.stderr_text << "\n";
    }
    expect(update_process.exit_code == 0, "#3530: real sample grouping EXPR update should succeed");
    expect_contains(update_process.stdout_text,
                    "\"groupingCount\": 1",
                    "#3530: grouping EXPR update should preserve grouping count");
    expect_contains(update_process.stdout_text,
                    "\"headerRecordIndex\": " + std::to_string(header_record_index),
                    "#3530: grouping EXPR update should preserve the header record identity");
    expect_contains(update_process.stdout_text,
                    "\"footerRecordIndex\": " + std::to_string(footer_record_index),
                    "#3530: grouping EXPR update should preserve the footer record identity");
    expect_contains(update_process.stdout_text,
                    "\"groupingExpression\": \"" + updated_grouping_expression + "\"",
                    "#3530: grouping EXPR update should expose the updated grouping expression");

    const auto updated_expr = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = header_record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(updated_expr.ok && updated_expr.exists && updated_expr.value == updated_grouping_expression,
           "#3530: real sample grouping EXPR update should persist the direct field");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3530: real sample grouping EXPR update should change the primary asset bytes");
    expect(read_binary(copied_sidecar) != original_sidecar_bytes,
           "#3530: real sample grouping EXPR update should change the sidecar asset bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(header_record_index), "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample grouping reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample grouping reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3530: real sample grouping reopen should succeed");
    expect_contains(reopen_process.stdout_text,
                    "\"groupingCount\": 1",
                    "#3530: real sample grouping reopen should preserve grouping count");
    expect_contains(reopen_process.stdout_text,
                    "\"headerRecordIndex\": " + std::to_string(header_record_index),
                    "#3530: real sample grouping reopen should preserve the header record identity");
    expect_contains(reopen_process.stdout_text,
                    "\"footerRecordIndex\": " + std::to_string(footer_record_index),
                    "#3530: real sample grouping reopen should preserve the footer record identity");
    expect_contains(reopen_process.stdout_text,
                    "\"groupingExpression\": \"" + updated_grouping_expression + "\"",
                    "#3530: real sample grouping reopen should expose the updated grouping expression");
    expect_contains(reopen_process.stdout_text,
                    "\"recordIndex\": 2",
                    "#3530: real sample grouping reopen should expose the selected group header record");
    expect_contains(reopen_process.stdout_text,
                    "\"groupRole\": \"header\"",
                    "#3530: real sample grouping reopen should preserve group-header context");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_grouping_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_sample_grouping_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
