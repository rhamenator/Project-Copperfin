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

struct RealMemoObjectSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::size_t record_index = 0;
    std::string original_title;
    std::string updated_title;
    std::string unique_id;
    std::string original_expr;
    std::string updated_expr;
    std::string preserved_width;
    bool is_label = false;
};

void exercise_real_sample_object_memo_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealMemoObjectSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3528: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3528: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3528: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3528: copied sidecar asset should become writable");

    const auto original_expr = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(original_expr.ok && original_expr.exists && original_expr.value == sample.original_expr,
           "#3528: real sample object should expose the expected original EXPR");

    const auto original_width = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "WIDTH"
    });
    expect(original_width.ok && original_width.exists && original_width.value == sample.preserved_width,
           "#3528: real sample object should expose the expected original WIDTH");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial memo-object read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial memo-object read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3528: real sample memo-object read should succeed");
    expect_contains(initial_process.stdout_text,
                    "\"selectedObjectAvailable\": true",
                    "#3528: real sample memo-object read should expose a selected object");
    expect_contains(initial_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"object\"",
                    "#3528: real sample memo-object read should preserve object selection kind");
    expect_contains(initial_process.stdout_text,
                    "\"title\": \"" + sample.original_title + "\"",
                    "#3528: real sample memo-object read should preserve the original title");
    expect_contains(initial_process.stdout_text,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    "#3528: real sample memo-object read should preserve the selected object unique id");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", std::to_string(sample.record_index),
            "--property-name", "EXPR",
            "--property-value", sample.updated_expr,
            "--json"
        },
        temp_root);
    if (update_process.exit_code != 0) {
        std::cerr << "studio host real sample EXPR update stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host real sample EXPR update stderr:\n"
                  << update_process.stderr_text << "\n";
    }
    expect(update_process.exit_code == 0, "#3528: real sample EXPR update should succeed");
    expect_contains(update_process.stdout_text,
                    "\"selectedObjectAvailable\": true",
                    "#3528: EXPR update should preserve selected-object availability");
    expect_contains(update_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"object\"",
                    "#3528: EXPR update should preserve object selection kind");
    expect_contains(update_process.stdout_text,
                    "\"title\": \"" + sample.updated_title + "\"",
                    "#3528: EXPR update should refresh the selected object title");
    expect_contains(update_process.stdout_text,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    "#3528: EXPR update should preserve the selected object unique id");
    expect_contains(update_process.stdout_text,
                    "\"value\": \"" + sample.updated_expr + "\"",
                    "#3528: EXPR update should report the updated memo value");

    const auto updated_expr = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(updated_expr.ok && updated_expr.exists && updated_expr.value == sample.updated_expr,
           "#3528: real sample EXPR update should persist the memo-backed field");

    const auto preserved_width = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "WIDTH"
    });
    expect(preserved_width.ok && preserved_width.exists && preserved_width.value == sample.preserved_width,
           "#3528: real sample EXPR update should preserve direct WIDTH");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3528: real sample EXPR update should change the primary asset bytes");
    expect(read_binary(copied_sidecar) != original_sidecar_bytes,
           "#3528: real sample EXPR update should change the sidecar bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample memo-object reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample memo-object reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3528: real sample memo-object reopen should succeed");
    expect_contains(reopen_process.stdout_text,
                    "\"selectedObjectAvailable\": true",
                    "#3528: real sample memo-object reopen should preserve selected-object availability");
    expect_contains(reopen_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"object\"",
                    "#3528: real sample memo-object reopen should preserve object selection kind");
    expect_contains(reopen_process.stdout_text,
                    "\"title\": \"" + sample.updated_title + "\"",
                    "#3528: real sample memo-object reopen should preserve the updated title");
    expect_contains(reopen_process.stdout_text,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    "#3528: real sample memo-object reopen should preserve the selected object unique id");
    expect_contains(reopen_process.stdout_text,
                    "\"value\": \"" + sample.updated_expr + "\"",
                    "#3528: real sample memo-object reopen should expose the updated EXPR value");
    expect_contains(reopen_process.stdout_text,
                    "\"value\": \"" + sample.preserved_width + "\"",
                    "#3528: real sample memo-object reopen should preserve WIDTH");
    if (sample.is_label) {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": true",
                        "#3528: label sample memo-object reopen should preserve label identity");
    } else {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": false",
                        "#3528: report sample memo-object reopen should preserve report identity");
    }
}

void test_real_vfp9_report_and_label_sample_object_memo_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3528 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_object_memo_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_object_memo_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .record_index = 9U,
            .original_title = "invoice.order_id_b",
            .updated_title = "invoice.order_id_a",
            .unique_id = "_R8X0QSY9C",
            .original_expr = "invoice.order_id_b",
            .updated_expr = "invoice.order_id_a",
            .preserved_width = "8229.167",
            .is_label = false
        });
    exercise_real_sample_object_memo_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .record_index = 6U,
            .original_title = "customer.company",
            .updated_title = "customer.contact",
            .unique_id = "_RAF0WG5W6",
            .original_expr = "customer.company",
            .updated_expr = "customer.contact",
            .preserved_width = "31562.500",
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_object_memo_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_sample_object_memo_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
