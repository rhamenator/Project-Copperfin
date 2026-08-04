// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

struct RealDuplicateSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::size_t source_record_index = 0U;
    std::size_t duplicate_record_index = 0U;
    std::size_t section_record_index = 0U;
    std::size_t initial_section_object_index = 0U;
    std::size_t initial_section_object_count = 0U;
    std::size_t duplicate_section_object_index = 0U;
    std::size_t duplicate_section_object_count = 0U;
    std::size_t section_count = 0U;
    std::string title;
    std::string expression;
    std::string source_unique_id;
    std::string duplicate_unique_id;
    std::string containing_section_id;
    std::string containing_section_title;
    std::string containing_section_expression;
    std::string grouping_expression_json_value;
    bool is_label = false;
};

void expect_duplicate_selection_json(
    const std::string& json_text,
    const RealDuplicateSample& sample,
    std::size_t expected_record_index,
    const std::string& expected_unique_id,
    std::size_t expected_section_object_index,
    std::size_t expected_section_object_count,
    const std::string& context) {
    expect_contains(
        json_text,
        "\"selectedReportSelectionKind\": \"object\"",
        context + " should preserve object selection kind");
    expect_contains(
        json_text,
        "\"selectedObjectAvailable\": true",
        context + " should expose the selected object");
    expect_contains(
        json_text,
        "\"selectedReportObjectAvailable\": true",
        context + " should expose the selected report object");
    expect_contains(
        json_text,
        "\"recordIndex\": " + std::to_string(expected_record_index),
        context + " should expose the expected selected record index");
    expect_contains(
        json_text,
        "\"containingSectionId\": \"" + sample.containing_section_id + "\"",
        context + " should preserve the containing section id");
    expect_contains(
        json_text,
        "\"sectionObjectIndex\": " + std::to_string(expected_section_object_index),
        context + " should expose the expected containing-section object index");
    expect_contains(
        json_text,
        "\"sectionObjectCount\": " + std::to_string(expected_section_object_count),
        context + " should expose the expected containing-section object count");
    expect_contains(
        json_text,
        "\"title\": \"" + sample.title + "\"",
        context + " should preserve the selected object title");
    expect_contains(
        json_text,
        "\"expression\": \"" + sample.expression + "\"",
        context + " should preserve the selected object expression");
    expect_contains(
        json_text,
        "\"uniqueId\": \"" + expected_unique_id + "\"",
        context + " should expose the expected selected object unique id");
    expect_contains(
        json_text,
        "\"selectedReportObjectSectionAvailable\": true",
        context + " should expose the containing section");
    expect_contains(
        json_text,
        "\"id\": \"" + sample.containing_section_id + "\"",
        context + " should preserve the containing section unique id");
    expect_contains(
        json_text,
        "\"title\": \"" + sample.containing_section_title + "\"",
        context + " should preserve the containing section title");
    expect_contains(
        json_text,
        "\"expression\": \"" + sample.containing_section_expression + "\"",
        context + " should preserve the containing section expression");
    expect_contains(
        json_text,
        "\"recordIndex\": " + std::to_string(sample.section_record_index),
        context + " should expose the expected containing section record index");
    expect_contains(
        json_text,
        "\"sectionCount\": " + std::to_string(sample.section_count),
        context + " should preserve total section count");
    expect_contains(
        json_text,
        "\"groupingExpression\": " + sample.grouping_expression_json_value,
        context + " should preserve grouping expression exposure");
    if (sample.is_label) {
        expect_contains(json_text, "\"assetFamily\": \"label\"", context + " should preserve label identity");
    } else {
        expect_contains(json_text, "\"assetFamily\": \"report\"", context + " should preserve report identity");
    }
}

void exercise_real_sample_object_duplicate_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealDuplicateSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3653: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3653: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3653: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3653: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.source_record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial duplicate read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial duplicate read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3653: real sample object duplicate baseline read should succeed");
    expect_duplicate_selection_json(
        initial_process.stdout_text,
        sample,
        sample.source_record_index,
        sample.source_unique_id,
        sample.initial_section_object_index,
        sample.initial_section_object_count,
        "#3653: initial real sample duplicate baseline");

    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--duplicate-object",
            "--record", std::to_string(sample.source_record_index),
            "--unique-id", sample.source_unique_id,
            "--new-unique-id", sample.duplicate_unique_id,
            "--json"
        },
        temp_root);
    if (duplicate_process.exit_code != 0) {
        std::cerr << "studio host real sample duplicate stdout:\n"
                  << duplicate_process.stdout_text << "\n";
        std::cerr << "studio host real sample duplicate stderr:\n"
                  << duplicate_process.stderr_text << "\n";
    }
    expect(duplicate_process.exit_code == 0, "#3653: real sample object duplicate should succeed");
    expect_duplicate_selection_json(
        duplicate_process.stdout_text,
        sample,
        sample.duplicate_record_index,
        sample.duplicate_unique_id,
        sample.duplicate_section_object_index,
        sample.duplicate_section_object_count,
        "#3653: duplicated real sample object read");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3653: real sample object duplicate should change primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#4057: real sample object duplicate should preserve exact sidecar bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--unique-id", sample.duplicate_unique_id, "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample duplicate reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample duplicate reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3653: real sample object duplicate reopen should succeed");
    expect_duplicate_selection_json(
        reopen_process.stdout_text,
        sample,
        sample.duplicate_record_index,
        sample.duplicate_unique_id,
        sample.duplicate_section_object_index,
        sample.duplicate_section_object_count,
        "#3653: reopened duplicated real sample object read");
}

void test_real_vfp9_report_and_label_sample_object_duplicate_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3653 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_object_duplicate_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_object_duplicate_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .source_record_index = 9U,
            .duplicate_record_index = 58U,
            .section_record_index = 2U,
            .initial_section_object_index = 3U,
            .initial_section_object_count = 25U,
            .duplicate_section_object_index = 4U,
            .duplicate_section_object_count = 26U,
            .section_count = 5U,
            .title = "invoice.order_id_b",
            .expression = "invoice.order_id_b",
            .source_unique_id = "_R8X0QSY9C",
            .duplicate_unique_id = "_TSTINV001",
            .containing_section_id = "_R8X0R1TYA",
            .containing_section_title = "Group Header",
            .containing_section_expression = "invoice.order_id_a",
            .grouping_expression_json_value = "\"invoice.order_id_a\"",
            .is_label = false
        });
    exercise_real_sample_object_duplicate_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .source_record_index = 6U,
            .duplicate_record_index = 16U,
            .section_record_index = 3U,
            .initial_section_object_index = 0U,
            .initial_section_object_count = 4U,
            .duplicate_section_object_index = 1U,
            .duplicate_section_object_count = 5U,
            .section_count = 5U,
            .title = "customer.company",
            .expression = "customer.company",
            .source_unique_id = "_RAF0WG5W6",
            .duplicate_unique_id = "_TSTCUS001",
            .containing_section_id = "_RAF0WF3C5",
            .containing_section_title = "Detail",
            .containing_section_expression = "",
            .grouping_expression_json_value = "null",
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_object_duplicate_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_sample_object_duplicate_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
