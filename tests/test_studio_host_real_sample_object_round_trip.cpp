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

struct RealObjectSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::size_t record_index = 0;
    std::string title;
    std::string unique_id;
    std::string original_width;
    std::string updated_width;
    std::string preserved_expr;
    bool is_label = false;
    bool exercise_raw_picture_round_trip = false;
};

void exercise_real_sample_object_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealObjectSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3527: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3527: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3527: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3527: copied sidecar asset should become writable");

    const auto original_width = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "WIDTH"
    });
    expect(original_width.ok && original_width.exists && original_width.value == sample.original_width,
           "#3527: real sample object should expose the expected original WIDTH");

    const auto original_expr = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(original_expr.ok && original_expr.exists && original_expr.value == sample.preserved_expr,
           "#3527: real sample object should expose the expected original EXPR");

    const auto preserved_picture_object = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 10U,
        .object_name = {},
        .unique_id = {},
        .property_name = "PICTURE"
    });
    if (sample.exercise_raw_picture_round_trip) {
        expect(preserved_picture_object.ok && preserved_picture_object.exists &&
                   preserved_picture_object.value == "\"logo.bmp\"",
               "#4291: real LBX picture-object PICTURE should expose its original memo text");
    }

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial object read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial object read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3527: real sample object read should succeed");
    expect_contains(initial_process.stdout_text,
                    "\"selectedObjectAvailable\": true",
                    "#3527: real sample object read should expose a selected object");
    expect_contains(initial_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"object\"",
                    "#3527: real sample object read should preserve object selection kind");
    expect_contains(initial_process.stdout_text,
                    "\"title\": \"" + sample.title + "\"",
                    "#3527: real sample object read should preserve the selected object title");
    expect_contains(initial_process.stdout_text,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    "#3527: real sample object read should preserve the selected object unique id");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", std::to_string(sample.record_index),
            "--property-name", "WIDTH",
            "--property-value", sample.updated_width,
            "--json"
        },
        temp_root);
    if (update_process.exit_code != 0) {
        std::cerr << "studio host real sample WIDTH update stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host real sample WIDTH update stderr:\n"
                  << update_process.stderr_text << "\n";
    }
    expect(update_process.exit_code == 0, "#3527: real sample WIDTH update should succeed");
    expect_contains(update_process.stdout_text,
                    "\"selectedObjectAvailable\": true",
                    "#3527: WIDTH update should preserve selected-object availability");
    expect_contains(update_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"object\"",
                    "#3527: WIDTH update should preserve object selection kind");
    expect_contains(update_process.stdout_text,
                    "\"title\": \"" + sample.title + "\"",
                    "#3527: WIDTH update should preserve the selected object title");
    expect_contains(update_process.stdout_text,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    "#3527: WIDTH update should preserve the selected object unique id");
    expect_contains(update_process.stdout_text,
                    "\"value\": \"" + sample.updated_width + "\"",
                    "#3527: WIDTH update should report the updated direct field value");

    const auto updated_width = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "WIDTH"
    });
    expect(updated_width.ok && updated_width.exists && updated_width.value == sample.updated_width,
           "#3527: real sample WIDTH update should persist the direct field");

    const auto preserved_expr = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = sample.record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(preserved_expr.ok && preserved_expr.exists && preserved_expr.value == sample.preserved_expr,
           "#3527: real sample WIDTH update should preserve memo-backed EXPR text");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3527: real sample WIDTH update should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3527: real sample WIDTH update should preserve sidecar bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample object reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample object reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3527: real sample object reopen should succeed");
    expect_contains(reopen_process.stdout_text,
                    "\"selectedObjectAvailable\": true",
                    "#3527: real sample object reopen should preserve selected-object availability");
    expect_contains(reopen_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"object\"",
                    "#3527: real sample object reopen should preserve object selection kind");
    expect_contains(reopen_process.stdout_text,
                    "\"title\": \"" + sample.title + "\"",
                    "#3527: real sample object reopen should preserve the selected object title");
    expect_contains(reopen_process.stdout_text,
                    "\"uniqueId\": \"" + sample.unique_id + "\"",
                    "#3527: real sample object reopen should preserve the selected object unique id");
    expect_contains(reopen_process.stdout_text,
                    "\"value\": \"" + sample.updated_width + "\"",
                    "#3527: real sample object reopen should expose the updated WIDTH value");
    expect_contains(reopen_process.stdout_text,
                    "\"value\": \"" + sample.preserved_expr + "\"",
                    "#3527: real sample object reopen should preserve the EXPR memo text");
    if (sample.is_label) {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": true",
                        "#3527: label sample object reopen should preserve label identity");
    } else {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": false",
                        "#3527: report sample object reopen should preserve report identity");
    }

    if (sample.exercise_raw_picture_round_trip) {
        const auto picture_set_process = run_process_capture(
            studio_host_path,
            {
                "--path", copied_primary.string(),
                "--set-property",
                "--record", std::to_string(sample.record_index),
                "--property-name", "PICTURE",
                "--property-value", "@I",
                "--json"
            },
            temp_root);
        expect(picture_set_process.exit_code == 0,
               "#4291: real LBX PICTURE set should succeed for the selected field object");
        const std::string picture_set_json =
            copperfin::test_support::normalize_json_line_endings(picture_set_process.stdout_text);
        expect_contains(
            picture_set_process.stdout_text,
            "\"name\": \"PICTURE\", \"type\": \"M\", \"isNull\": false, \"value\": \"@I\", \"fieldIndex\": 12, \"memoBlockNumber\": ",
            "#4291: real LBX PICTURE set should expose field and memo provenance");
        expect_contains(
            picture_set_process.stdout_text,
            "\"objectKind\": \"field\"",
            "#4291: real LBX PICTURE set should preserve the field object kind");
        expect_contains(
            picture_set_json,
            "\"picture\": \"\",\n      \"pictureFieldIndex\": null,\n      \"pictureMemoBlockNumber\": 0",
            "#4291: field-object PICTURE should not be reinterpreted as label alignment metadata");

        const auto updated_picture = copperfin::vfp::query_visual_object_property({
            .path = copied_primary.string(),
            .record_index = sample.record_index,
            .object_name = {},
            .unique_id = {},
            .property_name = "PICTURE"
        });
        expect(updated_picture.ok && updated_picture.exists && updated_picture.value == "@I",
               "#4291: real LBX PICTURE set should persist the raw memo value");
        expect(read_binary(copied_sidecar) != original_sidecar_bytes,
               "#4291: real LBX PICTURE set should update the memo sidecar");
        expect(copperfin::vfp::query_visual_object_property({
                   .path = copied_primary.string(),
                   .record_index = 10U,
                   .object_name = {},
                   .unique_id = {},
                   .property_name = "PICTURE"
               }).value == preserved_picture_object.value,
               "#4291: real LBX PICTURE set should preserve the unrelated image-object PICTURE");

        const auto picture_clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", copied_primary.string(),
                "--clear-property",
                "--record", std::to_string(sample.record_index),
                "--property-name", "PICTURE",
                "--json"
            },
            temp_root);
        expect(picture_clear_process.exit_code == 0,
               "#4291: real LBX PICTURE clear should succeed for the selected field object");
        const std::string picture_clear_json =
            copperfin::test_support::normalize_json_line_endings(picture_clear_process.stdout_text);
        expect_contains(
            picture_clear_process.stdout_text,
            "\"name\": \"PICTURE\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 12, \"memoBlockNumber\": ",
            "#4291: real LBX PICTURE clear should retain field and memo provenance");
        expect_contains(
            picture_clear_json,
            "\"picture\": \"\",\n      \"pictureFieldIndex\": null,\n      \"pictureMemoBlockNumber\": 0",
            "#4291: cleared field-object PICTURE should remain outside label alignment metadata");

        const auto cleared_picture = copperfin::vfp::query_visual_object_property({
            .path = copied_primary.string(),
            .record_index = sample.record_index,
            .object_name = {},
            .unique_id = {},
            .property_name = "PICTURE"
        });
        expect(cleared_picture.ok && cleared_picture.exists && cleared_picture.value.empty(),
               "#4291: real LBX PICTURE clear should persist an empty raw memo value");
        expect(copperfin::vfp::query_visual_object_property({
                   .path = copied_primary.string(),
                   .record_index = sample.record_index,
                   .object_name = {},
                   .unique_id = {},
                   .property_name = "EXPR"
               }).value == sample.preserved_expr,
               "#4291: real LBX PICTURE round trip should preserve the selected expression");
        expect(copperfin::vfp::query_visual_object_property({
                   .path = copied_primary.string(),
                   .record_index = 10U,
                   .object_name = {},
                   .unique_id = {},
                   .property_name = "PICTURE"
               }).value == preserved_picture_object.value,
               "#4291: real LBX PICTURE clear should preserve the unrelated image-object PICTURE");

        const auto picture_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
            temp_root);
        expect(picture_reopen_process.exit_code == 0,
               "#4291: real LBX PICTURE clear reopen should succeed");
        expect_contains(
            picture_reopen_process.stdout_text,
            "\"name\": \"PICTURE\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 12, \"memoBlockNumber\": ",
            "#4291: reopened real LBX PICTURE clear should preserve raw-field provenance");
    }
}

void test_real_vfp9_report_and_label_sample_object_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3527 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_object_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_object_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .record_index = 9U,
            .title = "invoice.order_id_b",
            .unique_id = "_R8X0QSY9C",
            .original_width = "8229.167",
            .updated_width = "9000.000",
            .preserved_expr = "invoice.order_id_b",
            .is_label = false
        });
    exercise_real_sample_object_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .record_index = 6U,
            .title = "customer.company",
            .unique_id = "_RAF0WG5W6",
            .original_width = "31562.500",
            .updated_width = "32000.000",
            .preserved_expr = "customer.company",
            .is_label = true,
            .exercise_raw_picture_round_trip = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_object_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_sample_object_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
