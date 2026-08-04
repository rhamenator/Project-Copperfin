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

struct RealObjectResizeSample {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::size_t record_index = 0U;
    std::string title;
    std::string unique_id;
    std::string anchor_unique_id;
    bool is_label = false;
};

void expect_common_object_resize_json(
    const std::string& json_text,
    const RealObjectResizeSample& sample,
    const std::string& expected_width,
    const std::string& expected_height,
    bool expect_undo,
    const std::string& context) {
    expect_contains(
        json_text,
        "\"selectedObjectAvailable\": true",
        context + " should expose a selected object");
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
        "\"name\": \"WIDTH\", \"type\": \"N\", \"isNull\": false, \"value\": \"" + expected_width + "\"",
        context + " should expose the expected WIDTH value");
    expect_contains(
        json_text,
        "\"name\": \"HEIGHT\", \"type\": \"N\", \"isNull\": false, \"value\": \"" + expected_height + "\"",
        context + " should expose the expected HEIGHT value");
    expect_contains(
        json_text,
        expect_undo ? "\"commandUndoAvailable\": true" : "\"commandUndoAvailable\": false",
        context + " should expose the expected command-undo availability");
    expect_contains(
        json_text,
        "\"previewBoundsAvailable\": true",
        context + " should expose live preview bounds");
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

void exercise_real_sample_object_resize_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealObjectResizeSample& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3831: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3831: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3831: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3831: copied sidecar asset should become writable");

    const auto query_property = [&](const std::string& unique_id,
                                    std::size_t record_index,
                                    const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = copied_primary.string(),
            .record_index = record_index,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    const auto original_width = query_property({}, sample.record_index, "WIDTH");
    const auto original_height = query_property({}, sample.record_index, "HEIGHT");
    const auto anchor_width = query_property(sample.anchor_unique_id, 0U, "WIDTH");
    const auto anchor_height = query_property(sample.anchor_unique_id, 0U, "HEIGHT");
    expect(original_width.ok && original_width.exists,
           "#3831: real sample object should expose its original WIDTH");
    expect(original_height.ok && original_height.exists,
           "#3831: real sample object should expose its original HEIGHT");
    expect(anchor_width.ok && anchor_width.exists,
           "#3831: real sample anchor object should expose WIDTH");
    expect(anchor_height.ok && anchor_height.exists,
           "#3831: real sample anchor object should expose HEIGHT");
    if (!(original_width.ok && original_width.exists &&
          original_height.ok && original_height.exists &&
          anchor_width.ok && anchor_width.exists &&
          anchor_height.ok && anchor_height.exists)) {
        return;
    }

    std::string resize_mode;
    std::string expected_width = original_width.value;
    std::string expected_height = original_height.value;
    if (original_width.value != anchor_width.value && original_height.value != anchor_height.value) {
        resize_mode = "size";
        expected_width = anchor_width.value;
        expected_height = anchor_height.value;
    } else if (original_width.value != anchor_width.value) {
        resize_mode = "width";
        expected_width = anchor_width.value;
    } else if (original_height.value != anchor_height.value) {
        resize_mode = "height";
        expected_height = anchor_height.value;
    } else {
        expect(false, "#3831: chosen real sample anchor should differ in WIDTH or HEIGHT");
        return;
    }

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host real sample initial resize read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host real sample initial resize read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0, "#3831: real sample object read should succeed");
    expect_common_object_resize_json(
        initial_process.stdout_text,
        sample,
        original_width.value,
        original_height.value,
        false,
        "#3831: initial real sample object read");

    const auto resize_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--record", std::to_string(sample.record_index),
            "--resize-object",
            "--resize-mode", resize_mode,
            "--anchor-unique-id", sample.anchor_unique_id,
            "--resize-target-unique-id", sample.unique_id,
            "--json"
        },
        temp_root);
    if (resize_process.exit_code != 0) {
        std::cerr << "studio host real sample resize stdout:\n"
                  << resize_process.stdout_text << "\n";
        std::cerr << "studio host real sample resize stderr:\n"
                  << resize_process.stderr_text << "\n";
    }
    expect(resize_process.exit_code == 0, "#3831: real sample object resize should succeed");
    expect_common_object_resize_json(
        resize_process.stdout_text,
        sample,
        expected_width,
        expected_height,
        true,
        "#3831: resized real sample object read");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3831: real sample object resize should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3831: real sample object resize should preserve sidecar bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host real sample resize reopen stdout:\n"
                  << reopen_process.stdout_text << "\n";
        std::cerr << "studio host real sample resize reopen stderr:\n"
                  << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3831: real sample object resize reopen should succeed");
    expect_common_object_resize_json(
        reopen_process.stdout_text,
        sample,
        expected_width,
        expected_height,
        true,
        "#3831: reopened resized real sample object read");

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
        std::cerr << "studio host real sample resize undo stdout:\n"
                  << undo_process.stdout_text << "\n";
        std::cerr << "studio host real sample resize undo stderr:\n"
                  << undo_process.stderr_text << "\n";
    }
    expect(undo_process.exit_code == 0, "#3831: real sample object resize undo should succeed");
    expect_common_object_resize_json(
        undo_process.stdout_text,
        sample,
        original_width.value,
        original_height.value,
        false,
        "#3831: undone real sample object read");

    expect(read_binary(copied_primary) == original_primary_bytes,
           "#3831: real sample object resize undo should restore the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3831: real sample object resize undo should preserve restored sidecar bytes");

    const auto reopen_undo_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", std::to_string(sample.record_index), "--json"},
        temp_root);
    if (reopen_undo_process.exit_code != 0) {
        std::cerr << "studio host real sample resize undo reopen stdout:\n"
                  << reopen_undo_process.stdout_text << "\n";
        std::cerr << "studio host real sample resize undo reopen stderr:\n"
                  << reopen_undo_process.stderr_text << "\n";
    }
    expect(reopen_undo_process.exit_code == 0, "#3831: real sample object resize undo reopen should succeed");
    expect_common_object_resize_json(
        reopen_undo_process.stdout_text,
        sample,
        original_width.value,
        original_height.value,
        false,
        "#3831: reopened undone real sample object read");
}

void test_real_vfp9_report_and_label_sample_object_resize_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3831 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_object_resize_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_object_resize_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .record_index = 9U,
            .title = "invoice.order_id_b",
            .unique_id = "_R8X0QSY9C",
            .anchor_unique_id = "_R8X0QSYAN",
            .is_label = false
        });
    exercise_real_sample_object_resize_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .record_index = 6U,
            .title = "customer.company",
            .unique_id = "_RAF0WG5W6",
            .anchor_unique_id = "_RAF0WU5W6",
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_object_resize_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_sample_object_resize_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
