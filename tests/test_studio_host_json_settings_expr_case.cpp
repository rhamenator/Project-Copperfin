// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

#include <algorithm>

namespace cf_test_studio_host_json {

void test_studio_host_json_writes_case_insensitive_expr_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_studio_host_expr_case_write_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_write = [&](const fs::path& asset_path, const std::string& expr_field_name,
                               const std::string& asset_kind) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = expr_field_name, .type = 'M', .length = 4U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            asset_path.string(), fields,
            {{"1", "53", "CUSTOMSETTING=preserve\r\nGRIDV=4", "case-host-settings-guid"}});
        expect(create_result.ok, "Studio host EXPR case fixture should be writable");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--set-property", "--unique-id", "case-host-settings-guid",
             "--property-name", "GRIDV", "--property-value", "9", "--json"},
            temp_root);
        if (process.exit_code != 0) {
            std::cerr << "Studio host " << asset_kind << " EXPR case write stdout:\n"
                      << process.stdout_text << "\nStudio host stderr:\n"
                      << process.stderr_text << "\n";
        }
        expect(process.exit_code == 0,
               "Studio host should write settings through a case-insensitive EXPR descriptor");
        expect(process.stdout_text.find("\"gridVertical\": 9") != std::string::npos,
               "Studio host JSON should expose the refreshed grid setting");
        if (asset_path.extension() == ".lbx") {
            expect(process.stdout_text.find("\"isLabel\": true") != std::string::npos,
                   "Studio host JSON should retain label identity after EXPR write");
        }

        const auto query = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(), .record_index = 0U, .object_name = {},
            .unique_id = "case-host-settings-guid", .property_name = "GRIDV"
        });
        expect(query.ok && query.exists && query.value == "9",
               "Studio host EXPR write should survive a reopened query");
        const auto parsed = copperfin::vfp::parse_dbf_table_from_file(asset_path.string(), 1U);
        if (parsed.ok && parsed.table.records.size() == 1U) {
            const auto field = std::find_if(
                parsed.table.records[0].values.begin(), parsed.table.records[0].values.end(),
                [&](const auto& value) { return value.field_name == expr_field_name; });
            expect(field != parsed.table.records[0].values.end(),
                   "Studio host EXPR write should preserve the physical field spelling");
            if (field != parsed.table.records[0].values.end()) {
                expect(field->display_value.find("CUSTOMSETTING=preserve") != std::string::npos &&
                           field->display_value.find("GRIDV=9") != std::string::npos,
                       "Studio host EXPR write should preserve sibling memo content");
            }
        }
    };

    run_write(temp_root / "lower.frx", "expr", "report");
    run_write(temp_root / "mixed.lbx", "eXpR", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
