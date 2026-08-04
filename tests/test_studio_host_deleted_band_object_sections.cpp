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

void write_deleted_band_object_fixture(
    const std::filesystem::path& report_path,
    const std::vector<std::vector<std::string>>& records,
    const std::string& message_prefix) {
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

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, message_prefix + " should create the synthetic report table");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, message_prefix + " should mark the placed object deleted");
}

void write_deleted_group_header_object_fixture(const std::filesystem::path& report_path) {
    write_deleted_band_object_fixture(
        report_path,
        {
            {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
            {"9", "3", "customer.country", "", "0", "", "600", "group-header-guid"},
            {"9", "4", "", "", "600", "", "3000", ""},
            {"9", "5", "customer.country", "", "3600", "", "500", "group-footer-guid"},
            {"5", "", "\"Group header label\"", "300", "100", "1400", "250", "group-header-label-guid"}
        },
        "#2728: deleted group-header fixture");
}

void write_deleted_group_footer_object_fixture(const std::filesystem::path& report_path) {
    write_deleted_band_object_fixture(
        report_path,
        {
            {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
            {"9", "3", "customer.country", "", "0", "", "600", "group-header-guid"},
            {"9", "4", "", "", "600", "", "3000", ""},
            {"9", "5", "customer.country", "", "3600", "", "500", "group-footer-guid"},
            {"5", "", "\"Group footer label\"", "350", "3700", "1450", "250", "group-footer-label-guid"}
        },
        "#2728: deleted group-footer fixture");
}

void write_deleted_title_object_fixture(const std::filesystem::path& report_path) {
    write_deleted_band_object_fixture(
        report_path,
        {
            {"1", "53", "", "", "", "", "", ""},
            {"9", "0", "", "", "0", "", "700", "title-section-guid"},
            {"9", "4", "", "", "700", "", "2500", ""},
            {"9", "7", "", "", "3200", "", "500", ""},
            {"5", "", "\"Title label\"", "100", "120", "1400", "300", "title-label-guid"}
        },
        "#2728: deleted title fixture");
}

void write_deleted_page_footer_object_fixture(const std::filesystem::path& report_path) {
    write_deleted_band_object_fixture(
        report_path,
        {
            {"1", "53", "", "", "", "", "", ""},
            {"9", "0", "", "", "0", "", "700", "title-section-guid"},
            {"9", "4", "", "", "700", "", "2500", ""},
            {"9", "7", "", "", "3200", "", "500", "page-footer-section-guid"},
            {"5", "", "\"Page footer label\"", "150", "3300", "1600", "300", "page-footer-label-guid"}
        },
        "#2728: deleted page-footer fixture");
}

void write_deleted_column_header_object_fixture(const std::filesystem::path& report_path) {
    write_deleted_band_object_fixture(
        report_path,
        {
            {"1", "53", "", "", "", "", "", ""},
            {"9", "2", "", "", "0", "", "450", "column-header-section-guid"},
            {"9", "4", "", "", "450", "", "2600", ""},
            {"9", "6", "", "", "3050", "", "400", "column-footer-section-guid"},
            {"5", "", "\"Column header label\"", "200", "100", "1700", "250", "column-header-label-guid"}
        },
        "#2728: deleted column-header fixture");
}

void write_deleted_column_footer_object_fixture(const std::filesystem::path& report_path) {
    write_deleted_band_object_fixture(
        report_path,
        {
            {"1", "53", "", "", "", "", "", ""},
            {"9", "2", "", "", "0", "", "450", "column-header-section-guid"},
            {"9", "4", "", "", "450", "", "2600", ""},
            {"9", "6", "", "", "3050", "", "400", "column-footer-section-guid"},
            {"5", "", "\"Column footer label\"", "250", "3150", "1750", "250", "column-footer-label-guid"}
        },
        "#2728: deleted column-footer fixture");
}

struct DeletedBandObjectCase {
    std::string fixture_name;
    std::string label;
    std::string object_unique_id;
    std::string section_id;
    std::string band_kind;
    std::string section_record_index;
    std::string section_index;
    std::string relative_top;
    std::string relative_bottom;
    void (*write_fixture)(const std::filesystem::path&);
};

void run_deleted_band_object_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const DeletedBandObjectCase& test_case,
    const std::string& extension) {
    const std::filesystem::path asset_path = temp_root / (test_case.fixture_name + extension);
    test_case.write_fixture(asset_path);

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", test_case.object_unique_id, "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host " << test_case.label << " " << extension << " stdout:\n"
                  << process.stdout_text << "\n";
        std::cerr << "studio host " << test_case.label << " " << extension << " stderr:\n"
                  << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "#2728: " + test_case.label + " should exit successfully");
    if (extension == ".lbx") {
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"reportLayout\": {",
                "\"isLabel\": true"
            },
            "#2728: " + test_case.label + " should preserve label identity");
    }
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"deleted\": true",
            "\"containingSectionId\": \"" + test_case.section_id + "\"",
            "\"containingSectionRecordIndex\": " + test_case.section_record_index,
            "\"sectionRelativeTop\": " + test_case.relative_top,
            "\"sectionRelativeBottom\": " + test_case.relative_bottom,
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1"
        },
        "#2728: " + test_case.label + " should preserve deleted placed-object containing-section metadata");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"selectedReportObjectSectionAvailable\": true",
            "\"selectedReportObjectSection\": {",
            "\"id\": \"" + test_case.section_id + "\"",
            "\"bandKind\": \"" + test_case.band_kind + "\"",
            "\"recordIndex\": " + test_case.section_record_index,
            "\"deleted\": false",
            "\"sectionIndex\": " + test_case.section_index,
            "\"sectionCount\": 3",
            "\"objectCount\": 0",
            "\"deletedObjectCount\": 1"
        },
        "#2728: " + test_case.label + " should expose containing live-section metadata");
}

void test_studio_host_json_preserves_containing_sections_for_deleted_band_objects(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_deleted_band_object_section_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const std::vector<DeletedBandObjectCase> cases{
        {
            "deleted_group_header_object",
            "deleted group-header object selection",
            "group-header-label-guid",
            "group-header-guid",
            "group_header",
            "1",
            "0",
            "100",
            "350",
            write_deleted_group_header_object_fixture
        },
        {
            "deleted_group_footer_object",
            "deleted group-footer object selection",
            "group-footer-label-guid",
            "group-footer-guid",
            "group_footer",
            "3",
            "2",
            "100",
            "350",
            write_deleted_group_footer_object_fixture
        },
        {
            "deleted_title_object",
            "deleted title-band object selection",
            "title-label-guid",
            "title-section-guid",
            "title",
            "1",
            "0",
            "120",
            "420",
            write_deleted_title_object_fixture
        },
        {
            "deleted_page_footer_object",
            "deleted page-footer object selection",
            "page-footer-label-guid",
            "page-footer-section-guid",
            "page_footer",
            "3",
            "2",
            "100",
            "400",
            write_deleted_page_footer_object_fixture
        },
        {
            "deleted_column_header_object",
            "deleted column-header object selection",
            "column-header-label-guid",
            "column-header-section-guid",
            "column_header",
            "1",
            "0",
            "100",
            "350",
            write_deleted_column_header_object_fixture
        },
        {
            "deleted_column_footer_object",
            "deleted column-footer object selection",
            "column-footer-label-guid",
            "column-footer-section-guid",
            "column_footer",
            "3",
            "2",
            "100",
            "350",
            write_deleted_column_footer_object_fixture
        }
    };

    for (const auto& test_case : cases) {
        run_deleted_band_object_case(studio_host_path, temp_root, test_case, ".frx");
        run_deleted_band_object_case(studio_host_path, temp_root, test_case, ".lbx");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_deleted_band_object_sections <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_containing_sections_for_deleted_band_objects(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
