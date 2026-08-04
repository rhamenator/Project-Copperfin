// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
#include "test_locale_catalog_environment_support.h"

#include <cstdint>
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

void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message) {
    std::size_t offset = 0U;
    for (const auto& needle : needles) {
        const std::size_t position = text.find(needle, offset);
        if (position == std::string::npos) {
            expect(false, message);
            return;
        }
        offset = position + needle.size();
    }
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

std::string read_text(const std::filesystem::path& path) {
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
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
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

void write_synthetic_report_table_for_detail_header_footer_section_kind_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "0", "300", "detail-header-guid"},
        {"9", "10", "detail footer expression", "300", "250", "detail-footer-guid"},
        {"9", "10", "deleted detail footer expression", "550", "200", "deleted-detail-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2727: synthetic report table for section-id host JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#2727: synthetic report table should mark deleted detail footer sections");
}

void write_synthetic_report_table_for_detail_header_footer_object_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "300", "detail-header-guid"},
        {"5", "", "\"Header label\"", "100", "50", "700", "120", "detail-header-label-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "detail-footer-guid"},
        {"8", "", "footer.total", "140", "360", "900", "100", "detail-footer-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2727: synthetic report table for containing-section host JSON should be created");
}

void test_studio_host_json_preserves_band_unique_ids_for_selected_sections(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_report_section_id_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_selection_checks = [&](const fs::path& asset_path, const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "detail-header-guid", "--json"},
            temp_root);
        if (live_process.exit_code != 0) {
            std::cerr << "studio host " << label << " selected live section stdout:\n"
                      << live_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " selected live section stderr:\n"
                      << live_process.stderr_text << "\n";
        }
        expect(live_process.exit_code == 0, "#2727: selected live detail-header sections should exit successfully");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"detail-header-guid\"",
                "\"recordIndex\": 0",
                "\"deleted\": false"
            },
            "#2727: selected live detail-header sections should preserve UNIQUEID-backed ids in host JSON");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deleted-detail-footer-guid", "--json"},
            temp_root);
        if (deleted_process.exit_code != 0) {
            std::cerr << "studio host " << label << " selected deleted section stdout:\n"
                      << deleted_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " selected deleted section stderr:\n"
                      << deleted_process.stderr_text << "\n";
        }
        expect(deleted_process.exit_code == 0,
               "#2727: selected deleted detail-footer sections should exit successfully");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"deleted-detail-footer-guid\"",
                "\"recordIndex\": 2",
                "\"deleted\": true"
            },
            "#2727: selected deleted detail-footer sections should preserve UNIQUEID-backed ids in host JSON");
    };

    run_selection_checks(temp_root / "detail_header_footer_section_ids.frx", "report");
    run_selection_checks(temp_root / "detail_header_footer_section_ids.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_band_unique_ids_for_object_containing_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_report_object_section_id_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_object_checks = [&](const fs::path& asset_path, const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

        const auto header_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "detail-header-label-guid", "--json"},
            temp_root);
        if (header_process.exit_code != 0) {
            std::cerr << "studio host " << label << " selected detail-header object stdout:\n"
                      << header_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " selected detail-header object stderr:\n"
                      << header_process.stderr_text << "\n";
        }
        expect(header_process.exit_code == 0,
               "#2727: selected detail-header report objects should exit successfully");
        expect_contains_in_order(
            header_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"containingSectionId\": \"detail-header-guid\"",
                "\"containingSectionRecordIndex\": 0"
            },
            "#2727: selected detail-header report objects should expose UNIQUEID-backed containing sections");
        expect_contains_in_order(
            header_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail-header-guid\"",
                "\"recordIndex\": 0"
            },
            "#2727: selected detail-header report objects should expose UNIQUEID-backed containing-section metadata");

        const auto footer_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "detail-footer-field-guid", "--json"},
            temp_root);
        if (footer_process.exit_code != 0) {
            std::cerr << "studio host " << label << " selected detail-footer object stdout:\n"
                      << footer_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " selected detail-footer object stderr:\n"
                      << footer_process.stderr_text << "\n";
        }
        expect(footer_process.exit_code == 0,
               "#2727: selected detail-footer report objects should exit successfully");
        expect_contains_in_order(
            footer_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail-footer-guid\"",
                "\"containingSectionRecordIndex\": 2"
            },
            "#2727: selected detail-footer report objects should expose UNIQUEID-backed containing sections");
        expect_contains_in_order(
            footer_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail-footer-guid\"",
                "\"recordIndex\": 2"
            },
            "#2727: selected detail-footer report objects should expose UNIQUEID-backed containing-section metadata");
    };

    run_object_checks(temp_root / "detail_header_footer_object_ids.frx", "report");
    run_object_checks(temp_root / "detail_header_footer_object_ids.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_report_section_ids <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_band_unique_ids_for_selected_sections(argv[1]);
    test_studio_host_json_preserves_band_unique_ids_for_object_containing_sections(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
