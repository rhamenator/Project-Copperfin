#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_missing_section_objcode_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "missing.objcode.live", "150", "450", "missing-objcode-live-section-guid"},
        {"9", "missing.objcode.deleted", "900", "250", "missing-objcode-deleted-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1728: synthetic report table without section OBJCODE schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1728: synthetic report table should mark the no-OBJCODE section deleted");
}

void write_synthetic_report_table_for_unresolved_section_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "3", "<memo block 40>", "100", "500"},
        {"9", "5", "<memo block 41>", "900", "300"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1737: synthetic report table with unresolved section memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1737: synthetic report table should mark unresolved section memo deleted");
}

void write_synthetic_report_table_for_missing_section_expr_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "3", "100", "500", "missing-expr-live-section-guid"},
        {"9", "5", "900", "300", "missing-expr-deleted-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1724: synthetic report table without section EXPR schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1724: synthetic report table should mark the no-EXPR section deleted");
}

void write_synthetic_report_table_for_stable_summary_section_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", ""},
        {"9", "4", "", "0", "3200", ""},
        {"9", "8", "", "3200", "700", "summary-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1694: synthetic report table for stable summary section JSON should be created");
}

void write_synthetic_report_table_for_ambiguous_summary_section_json(
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
        {"1", "53", "ORIENTATION=0", "", "", ""},
        {"9", "4", "", "0", "3200", ""},
        {"9", "8", "summary one", "3200", "700", "duplicate-section-guid"},
        {"9", "8", "summary two", "3900", "500", "DUPLICATE-SECTION-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1701: synthetic report table for ambiguous stable summary section JSON should be created");
}

void write_synthetic_report_table_for_live_deleted_ambiguous_summary_section_json(
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
        {"1", "53", "ORIENTATION=0", "", "", ""},
        {"9", "4", "", "0", "3200", ""},
        {"9", "8", "live summary", "3200", "700", "duplicate-live-deleted-guid"},
        {"9", "8", "deleted summary", "3900", "500", "DUPLICATE-LIVE-DELETED-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1702: synthetic report table for live/deleted ambiguous section JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1702: synthetic report table should mark duplicate section deleted");
}

void write_synthetic_report_table_for_padded_stable_section_json(
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
        {"1", "53", "ORIENTATION=0", "", "", ""},
        {"9", "4", "", "0", "3200", "  padded-section-guid  "}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1703: synthetic report table for padded stable section JSON should be created");
}

void write_synthetic_report_table_for_deep_stable_section_json(
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
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200", ""},
        {"5", "", "\"Preview object 2\"", "100", "200", "1000", "200", ""},
        {"5", "", "\"Preview object 3\"", "100", "500", "1000", "200", ""},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"9", "8", "deep summary", "", "3200", "", "700", "deep-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1707: synthetic report table for deep stable section JSON should be created");
}

void write_synthetic_report_table_for_deep_ambiguous_stable_section_json(
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
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200", "deep-duplicate-section-guid"},
        {"5", "", "\"Preview object 2\"", "100", "200", "1000", "200", ""},
        {"5", "", "\"Preview object 3\"", "100", "500", "1000", "200", ""},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"9", "8", "deep summary", "", "3200", "", "700", "DEEP-DUPLICATE-SECTION-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1708: synthetic report table for deep ambiguous stable section JSON should be created");
}

void write_synthetic_report_table_for_deep_live_deleted_ambiguous_stable_section_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deep_ambiguous_stable_section_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 10U, true);
    expect(delete_result.ok,
           "#1709: synthetic report table should mark the deep duplicate section deleted");
}

void write_synthetic_report_table_for_stable_title_section_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "", ""},
        {"9", "0", "0", "700", "title-section-guid"},
        {"9", "4", "700", "2500", ""},
        {"9", "7", "3200", "500", "page-footer-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1674: synthetic report table for stable title section JSON should be created");
}

void write_synthetic_report_table_for_stable_page_header_section_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "", ""},
        {"9", "1", "0", "700", "page-header-section-guid"},
        {"9", "4", "700", "2500", ""},
        {"9", "7", "3200", "500", "page-footer-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2008: synthetic report table for stable page-header section JSON should be created");
}

void write_synthetic_report_table_for_stable_column_section_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "", ""},
        {"9", "2", "0", "450", "column-header-section-guid"},
        {"9", "4", "450", "2600", ""},
        {"9", "6", "3050", "400", "column-footer-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1676: synthetic report table for stable column section JSON should be created");
}

void write_synthetic_report_table_for_stable_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "section-guid"
    });
    expect(unique_id_result.ok, "#1655: stable section fixture should seed a section unique id");
    expect(!dbf_record_deleted(report_path, 1U),
           "#1655: stable section fixture should preserve the live section state");
}

void write_synthetic_report_table_for_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1474: synthetic FRX table should mark report section deleted");
}

void write_synthetic_report_table_for_section_deleted_object_count_json(
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
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "1000", "detail-section-guid"},
        {"9", "8", "", "", "2000", "", "500", "deleted-summary-section-guid"},
        {"8", "0", "detail.value", "100", "200", "400", "100", "detail-field-guid"},
        {"5", "", "\"Deleted detail\"", "150", "300", "200", "100", "deleted-detail-label-guid"},
        {"5", "", "\"Deleted summary\"", "200", "2100", "250", "100", "deleted-summary-label-guid"},
        {"6", "", "", "50", "5000", "100", "100", "deleted-unplaced-line-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#2688: synthetic report table for section deleted-object counts should be created");

    for (const auto record_index : {2U, 4U, 5U, 6U}) {
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), record_index, true);
        expect(delete_result.ok,
               "#2688: synthetic report table should mark deleted sections and deleted objects");
    }
}

void write_synthetic_report_table_for_stable_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_section_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-section-guid"
    });
    expect(unique_id_result.ok, "#1654: stable deleted section fixture should seed a deleted section unique id");
    expect(dbf_record_deleted(report_path, 1U),
           "#1654: stable deleted section fixture should preserve the deleted section state");
}

void test_studio_host_json_clears_report_section_and_settings_selection_for_ambiguous_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_ambiguous_report_selection_category_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " ambiguous report selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " ambiguous report selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1701: ambiguous stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1701: ambiguous stable selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1701: ambiguous stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1701: ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1701: ambiguous stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1701: ambiguous stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1701: ambiguous stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1701: ambiguous stable selectors should serialize null selected settings");
    };

    const auto run_ambiguous_section_selector = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_ambiguous_summary_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-section-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                        "#1701: ambiguous section selectors should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1701: ambiguous section selectors should preserve deleted section counts");
        expect_contains(section_process.stdout_text, "\"settingCount\": 1",
                        "#1701: ambiguous section selectors should preserve live setting counts");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 3"
            },
            "#1701: ambiguous section selectors should preserve both duplicate section records in layout JSON");
    };

    const auto run_ambiguous_settings_selector = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_ambiguous_settings_json(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-settings-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1701: ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1701: ambiguous settings selectors should preserve both root setting records");
        expect_contains(settings_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1701: ambiguous settings selectors should preserve page setup summaries");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1"
            },
            "#1701: ambiguous settings selectors should preserve duplicate settings records in layout JSON");
    };

    run_ambiguous_section_selector(temp_root / "ambiguous_section_selector.frx",
                                   "ambiguous_section_selector.frx",
                                   "report section");
    run_ambiguous_section_selector(temp_root / "ambiguous_section_selector.lbx",
                                   "ambiguous_section_selector.lbx",
                                   "label section");
    run_ambiguous_settings_selector(temp_root / "ambiguous_settings_selector.frx",
                                    "ambiguous_settings_selector.frx",
                                    "report settings");
    run_ambiguous_settings_selector(temp_root / "ambiguous_settings_selector.lbx",
                                    "ambiguous_settings_selector.lbx",
                                    "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_selects_deep_report_sections_and_settings_by_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_report_section_settings_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto expect_common_selection = [&](const ProcessResult& process,
                                             const std::string& label,
                                             const std::string& title,
                                             const std::string& selection_kind) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1707: deep stable report/label section/settings selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1707: deep stable section/settings selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1707: deep stable label section/settings selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1707: deep stable section/settings selectors should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"" + selection_kind + "\"",
                        "#1707: deep stable section/settings selectors should expose the selected category");
    };

    const auto run_deep_section_selection = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_deep_stable_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-section-guid", "--json"},
            temp_root);

        expect_common_selection(section_process, label, title, "section");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1707: deep stable section selectors should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1707: deep stable section selectors should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#1707: deep stable section selectors should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1707: deep stable section selectors should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1707: deep stable section selectors should serialize null selected settings");
        expect_contains(section_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1707: deep stable section selectors should parse objects beyond the default preview record limit");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 10",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 2"
            },
            "#1707: deep stable section selectors should expose the selected deep report section");
    };

    const auto run_deep_settings_selection = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_deep_stable_settings_json(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-settings-guid", "--json"},
            temp_root);

        expect_common_selection(settings_process, label, title, "settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1707: deep stable settings selectors should advertise selected-settings availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1707: deep stable settings selectors should not advertise selected-section availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                        "#1707: deep stable settings selectors should serialize null selected sections");
        expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1707: deep stable settings selectors should not advertise selected-object availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                        "#1707: deep stable settings selectors should serialize null selected objects");
        expect_contains(settings_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1707: deep stable settings selectors should parse objects beyond the default preview record limit");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1707: deep stable settings selectors should preserve all root settings rows");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 10",
                "\"value\": \"9\""
            },
            "#1707: deep stable settings selectors should expose the selected deep root settings");
    };

    run_deep_section_selection(temp_root / "deep_section_selector.frx",
                               "deep_section_selector.frx",
                               "report section");
    run_deep_section_selection(temp_root / "deep_section_selector.lbx",
                               "deep_section_selector.lbx",
                               "label section");
    run_deep_settings_selection(temp_root / "deep_settings_selector.frx",
                                "deep_settings_selector.frx",
                                "report settings");
    run_deep_settings_selection(temp_root / "deep_settings_selector.lbx",
                                "deep_settings_selector.lbx",
                                "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_deep_ambiguous_section_and_settings_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_ambiguous_report_section_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep ambiguous stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep ambiguous stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1708: deep ambiguous stable section/settings selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1708: deep ambiguous stable section/settings selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1708: deep ambiguous stable label section/settings selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1708: deep ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1708: deep ambiguous stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected settings");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 8",
                        "#1708: deep ambiguous stable selectors should parse objects beyond the default preview record limit");
    };

    const auto run_deep_ambiguous_section = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_deep_ambiguous_stable_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-section-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                        "#1708: deep ambiguous section selectors should preserve both live sections");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1708: deep ambiguous section selectors should preserve deleted section counts");
        expect_not_contains(section_process.stdout_text, "\"selectedReportSection\": {",
                            "#1708: deep ambiguous section selectors should not select the preview-window section");
    };

    const auto run_deep_ambiguous_settings = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_deep_ambiguous_stable_settings_json(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-settings-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1708: deep ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1708: deep ambiguous settings selectors should preserve both root settings rows");
        expect_not_contains(settings_process.stdout_text, "\"selectedReportSettings\": [",
                            "#1708: deep ambiguous settings selectors should not select the preview-window settings row");
    };

    run_deep_ambiguous_section(temp_root / "deep_ambiguous_section.frx",
                               "deep_ambiguous_section.frx",
                               "report section");
    run_deep_ambiguous_section(temp_root / "deep_ambiguous_section.lbx",
                               "deep_ambiguous_section.lbx",
                               "label section");
    run_deep_ambiguous_settings(temp_root / "deep_ambiguous_settings.frx",
                                "deep_ambiguous_settings.frx",
                                "report settings");
    run_deep_ambiguous_settings(temp_root / "deep_ambiguous_settings.lbx",
                                "deep_ambiguous_settings.lbx",
                                "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_defaults_missing_report_section_objcode_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_section_objcode_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_section_objcode_layout = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_missing_section_objcode_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing section OBJCODE summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing section OBJCODE summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1728: missing section OBJCODE schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1728: missing section OBJCODE layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1728: missing section OBJCODE label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1728: missing section OBJCODE layouts should preserve live section counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1728: missing section OBJCODE layouts should preserve deleted section counts");
        expect_contains(summary_process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"title\", \"count\": 1}\n      ]",
                        "#1728: missing live section OBJCODE should summarize through the default title band");
        expect_contains(summary_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"title\", \"count\": 1}\n      ]",
                        "#1728: missing deleted section OBJCODE should summarize through the default title band");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 450",
                        "#1728: missing live section OBJCODE should preserve live section heights");
        expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 250",
                        "#1728: missing deleted section OBJCODE should preserve deleted section heights");
        expect_missing_section_objcode_preview_bounds(
            summary_process.stdout_text,
            "#2342: missing section OBJCODE summary JSON");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"title_0\"",
                "\"title\": \"Title\"",
                "\"titleFieldIndex\": null",
                "\"bandKind\": \"title\"",
                "\"bandKindFieldIndex\": null",
                "\"expression\": \"missing.objcode.live\"",
                "\"expressionFieldIndex\": 1",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"top\": 150",
                "\"height\": 450",
                "\"bottom\": 600"
            },
            "#1728: missing live section OBJCODE should serialize default band metadata with null provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"title_1\"",
                "\"title\": \"Title\"",
                "\"titleFieldIndex\": null",
                "\"bandKind\": \"title\"",
                "\"bandKindFieldIndex\": null",
                "\"expression\": \"missing.objcode.deleted\"",
                "\"expressionFieldIndex\": 1",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"top\": 900",
                "\"height\": 250",
                "\"bottom\": 1150"
            },
            "#1728: missing deleted section OBJCODE should serialize default band metadata with null provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1728: missing section OBJCODE live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1728: missing section OBJCODE live selection should advertise selected sections");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1728: missing section OBJCODE live selection should expose section selection kind");
        expect_missing_section_objcode_preview_bounds(
            live_process.stdout_text,
            "#2316: selected missing section OBJCODE live JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"title_0\"",
                "\"title\": \"Title\"",
                "\"titleFieldIndex\": null",
                "\"bandKind\": \"title\"",
                "\"bandKindFieldIndex\": null",
                "\"expression\": \"missing.objcode.live\"",
                "\"expressionFieldIndex\": 1",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"top\": 150",
                "\"height\": 450",
                "\"bottom\": 600"
            },
            "#1728: missing live section OBJCODE selection should expose default band metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1728: missing section OBJCODE deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1728: missing section OBJCODE deleted selection should advertise selected sections");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1728: missing section OBJCODE deleted selection should expose section selection kind");
        expect_missing_section_objcode_preview_bounds(
            deleted_process.stdout_text,
            "#2316: selected missing section OBJCODE deleted JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"title_1\"",
                "\"title\": \"Title\"",
                "\"titleFieldIndex\": null",
                "\"bandKind\": \"title\"",
                "\"bandKindFieldIndex\": null",
                "\"expression\": \"missing.objcode.deleted\"",
                "\"expressionFieldIndex\": 1",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"top\": 900",
                "\"height\": 250",
                "\"bottom\": 1150"
            },
            "#1728: missing deleted section OBJCODE selection should expose default band metadata");
    };

    run_missing_section_objcode_layout(temp_root / "missing_section_objcode.frx",
                                       "missing_section_objcode.frx",
                                       "report");
    run_missing_section_objcode_layout(temp_root / "missing_section_objcode.lbx",
                                       "missing_section_objcode.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_suppresses_unresolved_report_section_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_report_section_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_unresolved_section_memo_layout = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_unresolved_section_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved section memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved section memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1737: unresolved section memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1737: unresolved section memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1737: unresolved section memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1737: unresolved section memo layouts should preserve live section counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1737: unresolved section memo layouts should preserve deleted section counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 500",
                        "#1737: unresolved section memo layouts should preserve live section heights");
        expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 300",
                        "#1737: unresolved section memo layouts should preserve deleted section heights");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_0\"",
                "\"title\": \"Group Header\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 3",
                "\"top\": 100",
                "\"height\": 500",
                "\"bottom\": 600"
            },
            "#1737: unresolved live section memo layouts should suppress expression text and provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"group_footer_1\"",
                "\"title\": \"Group Footer\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 5",
                "\"top\": 900",
                "\"height\": 300",
                "\"bottom\": 1200"
            },
            "#1737: unresolved deleted section memo layouts should suppress expression text and provenance");
        expect_unresolved_section_memo_preview_bounds(
            summary_process.stdout_text,
            "#2333: unresolved section memo summary JSON");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1737: unresolved section memo placeholders should not leak into summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1737: unresolved live section memo selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1737: unresolved live section memo selection should advertise selected sections");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1737: unresolved live section memo selection should expose section selection kind");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_header_0\"",
                "\"title\": \"Group Header\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 3",
                "\"top\": 100",
                "\"height\": 500",
                "\"bottom\": 600"
            },
            "#1737: unresolved live section memo selection should suppress expression text and provenance");
        expect_unresolved_section_memo_preview_bounds(
            live_process.stdout_text,
            "#2333: selected unresolved live section memo JSON");
        expect_not_contains(live_process.stdout_text, "<memo block",
                            "#1737: unresolved live section memo placeholders should not leak into selection JSON");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1737: unresolved deleted section memo selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1737: unresolved deleted section memo selection should advertise selected sections");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1737: unresolved deleted section memo selection should expose section selection kind");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_footer_1\"",
                "\"title\": \"Group Footer\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 5",
                "\"top\": 900",
                "\"height\": 300",
                "\"bottom\": 1200"
            },
            "#1737: unresolved deleted section memo selection should suppress expression text and provenance");
        expect_unresolved_section_memo_preview_bounds(
            deleted_process.stdout_text,
            "#2333: selected unresolved deleted section memo JSON");
        expect_not_contains(deleted_process.stdout_text, "<memo block",
                            "#1737: unresolved deleted section memo placeholders should not leak into selection JSON");
    };

    run_unresolved_section_memo_layout(temp_root / "unresolved_section_memo.frx",
                                       "unresolved_section_memo.frx",
                                       "report");
    run_unresolved_section_memo_layout(temp_root / "unresolved_section_memo.lbx",
                                       "unresolved_section_memo.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_report_sections_without_expr_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_section_expr_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_section_expr_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_missing_section_expr_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing section EXPR summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing section EXPR summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1724: missing section EXPR schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1724: missing section EXPR layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1724: missing section EXPR label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1724: missing section EXPR layouts should not infer page setup");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1724: missing section EXPR layouts should preserve live section counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1724: missing section EXPR layouts should preserve deleted section counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 500",
                        "#1724: missing section EXPR layouts should preserve live section heights");
        expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 300",
                        "#1724: missing section EXPR layouts should preserve deleted section heights");
        expect_contains(summary_process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"group_header\", \"count\": 1}\n      ]",
                        "#1724: missing live section EXPR layouts should preserve band-kind counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"group_footer\", \"count\": 1}\n      ]",
                        "#1724: missing deleted section EXPR layouts should preserve band-kind counts");
        expect_unresolved_section_memo_preview_bounds(
            summary_process.stdout_text,
            "#2338: missing section EXPR summary JSON");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_0\"",
                "\"title\": \"Group Header\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 3",
                "\"top\": 100",
                "\"height\": 500",
                "\"bottom\": 600"
            },
            "#1724: missing live section EXPR layouts should serialize null expression provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"group_footer_1\"",
                "\"title\": \"Group Footer\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 5",
                "\"top\": 900",
                "\"height\": 300",
                "\"bottom\": 1200"
            },
            "#1724: missing deleted section EXPR layouts should serialize null expression provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1724: missing section EXPR live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1724: missing section EXPR live selection should advertise selected sections");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1724: missing section EXPR live selection should expose section selection kind");
        expect_unresolved_section_memo_preview_bounds(
            live_process.stdout_text,
            "#2338: selected missing live section EXPR JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_header_0\"",
                "\"title\": \"Group Header\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 3",
                "\"top\": 100",
                "\"height\": 500",
                "\"bottom\": 600"
            },
            "#1724: missing live section EXPR selection should expose section metadata with null expression provenance");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1724: missing section EXPR deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1724: missing section EXPR deleted selection should advertise selected sections");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1724: missing section EXPR deleted selection should expose section selection kind");
        expect_unresolved_section_memo_preview_bounds(
            deleted_process.stdout_text,
            "#2338: selected missing deleted section EXPR JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_footer_1\"",
                "\"title\": \"Group Footer\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 5",
                "\"top\": 900",
                "\"height\": 300",
                "\"bottom\": 1200"
            },
            "#1724: missing deleted section EXPR selection should expose section metadata with null expression provenance");
    };

    run_missing_section_expr_layout(temp_root / "missing_section_expr.frx",
                                    "missing_section_expr.frx",
                                    "report");
    run_missing_section_expr_layout(temp_root / "missing_section_expr.lbx",
                                    "missing_section_expr.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
