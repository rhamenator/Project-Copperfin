#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
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
    expect(create_result.ok, "#1763: synthetic report table for detail header/footer section JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#1763: synthetic report table should mark deleted detail footer section");
}

void write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(
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
        {"9", "4", "live detail expression", "0", "500", "live-detail-guid"},
        {"9", "9", "deleted detail header expression", "500", "300", "deleted-detail-header-guid"},
        {"9", "10", "deleted detail footer expression", "800", "250", "deleted-detail-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1766: synthetic report table for deleted detail header/footer expression JSON should be created");

    const auto delete_header_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_header_result.ok, "#1766: synthetic report table should mark deleted detail header section");
    const auto delete_footer_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_footer_result.ok, "#1766: synthetic report table should mark deleted detail footer section");
}

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

void test_studio_host_json_exposes_detail_header_footer_section_kinds(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_kind_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_sections = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " detail header/footer section summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail header/footer section summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1763: detail header/footer report/label section-kind JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1763: detail header/footer section-kind JSON should return report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1763: detail header/footer label layouts should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1763: detail header/footer section-kind JSON should summarize live sections");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1763: detail header/footer section-kind JSON should summarize deleted sections");
        expect_contains(process.stdout_text, "\"sectionKindCount\": 2",
                        "#1763: detail header/footer section-kind JSON should expose live kind bucket count");
        expect_contains(process.stdout_text,
                        "\"sectionKindCounts\": [\n"
                        "        {\"kind\": \"detail_footer\", \"count\": 1},\n"
                        "        {\"kind\": \"detail_header\", \"count\": 1}\n"
                        "      ]",
                        "#1763: detail header/footer section-kind JSON should count live detail header/footer buckets");
        expect_contains(process.stdout_text, "\"deletedSectionKindCount\": 1",
                        "#1763: detail header/footer section-kind JSON should expose deleted kind bucket count");
        expect_contains(process.stdout_text,
                        "\"deletedSectionKindCounts\": [\n"
                        "        {\"kind\": \"detail_footer\", \"count\": 1}\n"
                        "      ]",
                        "#1763: detail header/footer section-kind JSON should count deleted detail footer buckets");
        expect_contains(process.stdout_text, "\"sectionHeightTotal\": 550",
                        "#1763: detail header/footer section-kind JSON should sum live section heights");
        expect_contains(process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                        "#1763: detail header/footer section-kind JSON should sum deleted section heights");

        const auto expect_selected_section = [&](const std::string& unique_id,
                                                 const std::string& record_index,
                                                 const std::string& object_code,
                                                 const std::string& band_title,
                                                 const std::string& band_kind,
                                                 const std::string& selection_label) {
            const auto section_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                temp_root);

            if (section_process.exit_code != 0) {
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stdout:\n" << section_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stderr:\n" << section_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(section_process.exit_code == 0,
                   "#1763: selected detail header/footer section JSON should exit successfully");
            expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1763: selected detail header/footer JSON should advertise selected sections");
            expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1763: selected detail header/footer JSON should expose section selection kind");
            expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2275: selected detail header/footer section JSON should preserve live preview availability");
            expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview left bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview top bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview right bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2275: selected detail header/footer section JSON should preserve live preview bottom bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview widths");
            expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2275: selected detail header/footer section JSON should preserve live preview heights");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview availability");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview left bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview top bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview right bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview bottom bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview widths");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview heights");
            expect_contains_in_order(
                section_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"" + band_title + "\"",
                    "\"bandKind\": \"" + band_kind + "\"",
                    "\"recordIndex\": " + record_index,
                    "\"objectCode\": " + object_code
                },
                "#1763: selected " + selection_label + " JSON should expose detail header/footer band metadata");
        };

        expect_selected_section("detail-header-guid", "0", "9", "Detail Header", "detail_header", "detail header");
        expect_selected_section("detail-footer-guid", "1", "10", "Detail Footer", "detail_footer", "detail footer");
        expect_selected_section("deleted-detail-footer-guid",
                                "2",
                                "10",
                                "Detail Footer",
                                "detail_footer",
                                "deleted detail footer");
    };

    run_detail_header_footer_sections(temp_root / "detail_header_footer_sections.frx",
                                      "detail_header_footer_sections.frx",
                                      "report");
    run_detail_header_footer_sections(temp_root / "detail_header_footer_sections.lbx",
                                      "detail_header_footer_sections.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_height_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_height_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "420",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section height update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section height update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1802: detail-header section height update by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.value == "420",
                   "#1802: detail-header section height update should persist the HEIGHT field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1802: detail-header section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1802: detail-header label section height update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 670",
                            "#1802: detail-header section height update should refresh live section height totals");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1802: detail-header section height update should preserve deleted section height totals");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2276: detail-header section height update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2276: detail-header section height update should preserve live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2276: detail-header section height update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2276: detail-header section height update should preserve live preview heights");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2276: detail-header section height update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2276: detail-header section height update should preserve deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2276: detail-header section height update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1802: detail-header section height update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1802: detail-header section height update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2232: detail-header section height update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2232: detail-header section height update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2232: detail-header section height update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2232: detail-header section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 0",
                    "\"height\": 420",
                    "\"bottom\": 420"
                },
                "#1802: detail-header section height update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "280",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section height update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section height update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1802: detail-footer section height update by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "280",
                   "#1802: detail-footer section height update should persist the HEIGHT field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1802: detail-footer section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1802: detail-footer label section height update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 700",
                            "#1802: detail-footer section height update should refresh live section height totals");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1802: detail-footer section height update should preserve deleted section height totals");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2276: detail-footer section height update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2276: detail-footer section height update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 580",
                            "#2276: detail-footer section height update should refresh live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsHeight\": 580",
                            "#2276: detail-footer section height update should refresh live preview heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2276: detail-footer section height update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2276: detail-footer section height update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2276: detail-footer section height update should preserve deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1802: detail-footer section height update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1802: detail-footer section height update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2232: detail-footer section height update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2232: detail-footer section height update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2232: detail-footer section height update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2232: detail-footer section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 300",
                    "\"height\": 280",
                    "\"bottom\": 580"
                },
                "#1802: detail-footer section height update should refresh selected-section geometry");
        };

    run_detail_header_footer_section_height_update(
        temp_root / "detail_header_footer_section_height_stable.frx",
        "detail_header_footer_section_height_stable.frx",
        "report");
    run_detail_header_footer_section_height_update(
        temp_root / "detail_header_footer_section_height_stable.lbx",
        "detail_header_footer_section_height_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_height_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_height_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section height clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section height clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1803: detail-header section height clear by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.direct_field && header_height_property.value.empty(),
                   "#1803: detail-header section height clear should blank the HEIGHT field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1803: detail-header section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1803: detail-header label section height clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 250",
                            "#1803: detail-header section height clear should refresh live section height totals");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1803: detail-header section height clear should preserve deleted section height totals");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: detail-header section height clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: detail-header section height clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#1822: detail-header section height clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#1822: detail-header section height clear should preserve live preview heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: detail-header section height clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1822: detail-header section height clear should preserve deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1822: detail-header section height clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1803: detail-header section height clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1803: detail-header section height clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2234: detail-header section height clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2234: detail-header section height clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2234: detail-header section height clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2234: detail-header section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 0",
                    "\"height\": 0",
                    "\"bottom\": 0"
                },
                "#1803: detail-header section height clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section height clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section height clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1803: detail-footer section height clear by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.direct_field && footer_height_property.value.empty(),
                   "#1803: detail-footer section height clear should blank the HEIGHT field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1803: detail-footer section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1803: detail-footer label section height clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 0",
                            "#1803: detail-footer section height clear should refresh live section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1803: detail-footer section height clear should preserve deleted section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: detail-footer section height clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: detail-footer section height clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 300",
                            "#1822: detail-footer section height clear should shrink live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsHeight\": 300",
                            "#1822: detail-footer section height clear should shrink live preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: detail-footer section height clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1822: detail-footer section height clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1822: detail-footer section height clear should preserve deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1803: detail-footer section height clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1803: detail-footer section height clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2234: detail-footer section height clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2234: detail-footer section height clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2234: detail-footer section height clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2234: detail-footer section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 300",
                    "\"height\": 0",
                    "\"bottom\": 300"
                },
                "#1803: detail-footer section height clear should refresh selected-section geometry");
        };

    run_detail_header_footer_section_height_clear(
        temp_root / "detail_header_footer_section_height_clear_stable.frx",
        "detail_header_footer_section_height_clear_stable.frx",
        "report");
    run_detail_header_footer_section_height_clear(
        temp_root / "detail_header_footer_section_height_clear_stable.lbx",
        "detail_header_footer_section_height_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_height_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_height_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "360",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section height update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section height update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1804: deleted detail-header section height update by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.value == "360",
                   "#1804: deleted detail-header section height update should persist the HEIGHT field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1804: deleted detail-header section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1804: deleted detail-header label section height update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1804: deleted detail-header section height update should preserve live section count");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1804: deleted detail-header section height update should preserve deleted section count");
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1804: deleted detail-header section height update should preserve live section heights");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 610",
                            "#1804: deleted detail-header section height update should refresh deleted section heights");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2236: deleted detail-header section height update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2236: deleted detail-header section height update should preserve live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2236: deleted detail-header section height update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2236: deleted detail-header section height update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#2236: deleted detail-header section height update should preserve deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#2236: deleted detail-header section height update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                            "#2236: deleted detail-header section height update should preserve deleted preview heights");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1804: deleted detail-header section height update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1804: deleted detail-header section height update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2236: deleted detail-header section height update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2236: deleted detail-header section height update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2236: deleted detail-header section height update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2236: deleted detail-header section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 500",
                    "\"height\": 360",
                    "\"bottom\": 860"
                },
                "#1804: deleted detail-header section height update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "290",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section height update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section height update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1804: deleted detail-footer section height update by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "290",
                   "#1804: deleted detail-footer section height update should persist the HEIGHT field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1804: deleted detail-footer section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1804: deleted detail-footer label section height update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1804: deleted detail-footer section height update should preserve live section count");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1804: deleted detail-footer section height update should preserve deleted section count");
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1804: deleted detail-footer section height update should preserve live section heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 650",
                            "#1804: deleted detail-footer section height update should refresh deleted section heights");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2236: deleted detail-footer section height update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2236: deleted detail-footer section height update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2236: deleted detail-footer section height update should preserve live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2236: deleted detail-footer section height update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#2236: deleted detail-footer section height update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1090",
                            "#2236: deleted detail-footer section height update should refresh deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 590",
                            "#2236: deleted detail-footer section height update should refresh deleted preview heights");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1804: deleted detail-footer section height update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1804: deleted detail-footer section height update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2236: deleted detail-footer section height update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2236: deleted detail-footer section height update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2236: deleted detail-footer section height update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2236: deleted detail-footer section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"top\": 800",
                    "\"height\": 290",
                    "\"bottom\": 1090"
                },
                "#1804: deleted detail-footer section height update should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_height_update(
        temp_root / "deleted_detail_header_footer_section_height_stable.frx",
        "deleted_detail_header_footer_section_height_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_height_update(
        temp_root / "deleted_detail_header_footer_section_height_stable.lbx",
        "deleted_detail_header_footer_section_height_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_object_counts_per_section(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_section_deleted_object_count_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_section_deleted_object_count_json = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_section_deleted_object_count_json(asset_path);

        const auto live_section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);
        if (live_section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live section deleted-object-count stdout:\n"
                      << live_section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live section deleted-object-count stderr:\n"
                      << live_section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(live_section_process.exit_code == 0,
               "#2688: live section deleted-object-count JSON should exit successfully");
        expect_contains(live_section_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2688: live section deleted-object-count JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(live_section_process.stdout_text, "\"isLabel\": true",
                            "#2688: live label section deleted-object-count JSON should retain label identity");
        }
        expect_contains_in_order(
            live_section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"detail_1\"",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2688: live selected sections should expose deleted placed-object counts");

        const auto deleted_section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);
        if (deleted_section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted section deleted-object-count stdout:\n"
                      << deleted_section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted section deleted-object-count stderr:\n"
                      << deleted_section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(deleted_section_process.exit_code == 0,
               "#2688: deleted section deleted-object-count JSON should exit successfully");
        expect_contains_in_order(
            deleted_section_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"summary_2\"",
                "\"deleted\": true",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#2688: deleted section arrays should expose deleted placed-object counts");
        expect_contains_in_order(
            deleted_section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"summary_2\"",
                "\"deleted\": true",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#2688: deleted selected sections should expose deleted placed-object counts");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);
        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " object-section deleted-object-count stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " object-section deleted-object-count stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(object_process.exit_code == 0,
               "#2688: selected object-section deleted-object-count JSON should exit successfully");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2688: selected containing sections should expose deleted placed-object counts");

        const auto deleted_section_object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "5", "--json"},
            temp_root);
        if (deleted_section_object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted-section object containing-section stdout:\n"
                      << deleted_section_object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted-section object containing-section stderr:\n"
                      << deleted_section_object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(deleted_section_object_process.exit_code == 0,
               "#2689: deleted-section object containing-section JSON should exit successfully");
        expect_contains_in_order(
            deleted_section_object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": true",
                "\"containingSectionId\": \"summary_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 200",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1"
            },
            "#2689: deleted objects inside deleted sections should expose containing-section metadata");
        expect_contains_in_order(
            deleted_section_object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"summary_2\"",
                "\"deleted\": true",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#2689: deleted objects inside deleted sections should expose deleted containing-section JSON");

        const auto unplaced_deleted_object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "6", "--json"},
            temp_root);
        if (unplaced_deleted_object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted unplaced section deleted-object-count stdout:\n"
                      << unplaced_deleted_object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted unplaced section deleted-object-count stderr:\n"
                      << unplaced_deleted_object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(unplaced_deleted_object_process.exit_code == 0,
               "#2688: deleted unplaced object deleted-object-count JSON should exit successfully");
        expect_contains(unplaced_deleted_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#2688: unplaced deleted object selections should not advertise containing-section availability");
        expect_contains(unplaced_deleted_object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#2688: unplaced deleted object selections should keep containing-section JSON null");
    };

    run_section_deleted_object_count_json(temp_root / "section_deleted_object_count.frx",
                                          "section_deleted_object_count.frx",
                                          "report");
    run_section_deleted_object_count_json(temp_root / "section_deleted_object_count.lbx",
                                          "section_deleted_object_count.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_record_selected_nested_group_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_nested_group_section_json = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_nested_group_section_expression_json(asset_path);
        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        if (section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " record-selected nested group section stdout:\n"
                      << section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " record-selected nested group section stderr:\n"
                      << section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(section_process.exit_code == 0,
               "#2681: record-selected nested report/label group section JSON should exit successfully");
        expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2681: record-selected nested group section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(section_process.stdout_text, "\"isLabel\": true",
                            "#2681: record-selected nested label group section JSON should retain label identity");
        }
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2681: record-selected nested group sections should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#2681: record-selected nested group sections should advertise report-selection availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2681: record-selected nested group sections should preserve section selection kind");
        expect_contains(section_process.stdout_text, "\"sectionCount\": 5",
                        "#2681: record-selected nested group section JSON should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2681: record-selected nested group section JSON should preserve deleted section counts");
        expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2681: record-selected nested group section JSON should preserve live preview availability");
        expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2681: record-selected nested group section JSON should preserve live preview top bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2681: record-selected nested group section JSON should preserve live preview bottom bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2681: record-selected nested group section JSON should preserve live preview heights");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2681: record-selected nested group section JSON should not fabricate deleted preview bounds");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#2681: record-selected nested group sections should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#2681: record-selected nested group sections should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#2681: record-selected nested group sections should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#2681: record-selected nested group sections should serialize null selected settings");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_1\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 0",
                "\"groupingNestingDepth\": 0",
                "\"groupRole\": \"header\"",
                "\"groupPartnerSectionId\": \"group_footer_5\"",
                "\"groupPartnerRecordIndex\": 5",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 1",
                "\"id\": \"group_header_2\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 1",
                "\"groupingNestingDepth\": 1",
                "\"groupRole\": \"header\"",
                "\"groupPartnerSectionId\": \"group_footer_4\"",
                "\"groupPartnerRecordIndex\": 4",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"groupingContextAvailable\": false",
                "\"groupingIndex\": null",
                "\"groupingNestingDepth\": null",
                "\"groupRole\": null",
                "\"groupPartnerSectionId\": null",
                "\"groupPartnerRecordIndex\": null",
                "\"groupPartnerDeleted\": false",
                "\"recordIndex\": 3",
                "\"id\": \"group_footer_4\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 1",
                "\"groupingNestingDepth\": 1",
                "\"groupRole\": \"footer\"",
                "\"groupPartnerSectionId\": \"group_header_2\"",
                "\"groupPartnerRecordIndex\": 2",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 4",
                "\"id\": \"group_footer_5\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 0",
                "\"groupingNestingDepth\": 0",
                "\"groupRole\": \"footer\"",
                "\"groupPartnerSectionId\": \"group_header_1\"",
                "\"groupPartnerRecordIndex\": 1",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 5"
            },
            "#2681: record-selected nested group section JSON should preserve nested sibling ordering and expressions");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_header_2\"",
                "\"bandKind\": \"group_header\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 1",
                "\"groupingNestingDepth\": 1",
                "\"groupRole\": \"header\"",
                "\"groupPartnerSectionId\": \"group_footer_4\"",
                "\"groupPartnerRecordIndex\": 4",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 5",
                "\"top\": 400",
                "\"height\": 300",
                "\"bottom\": 700"
            },
            "#2681: record-selected nested group sections should expose selected inner-group metadata");
    };

    run_nested_group_section_json(temp_root / "nested_group_sections_record.frx",
                                  "nested_group_sections_record.frx",
                                  "report");
    run_nested_group_section_json(temp_root / "nested_group_sections_record.lbx",
                                  "nested_group_sections_record.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_record_selected_deleted_nested_group_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_deleted_group_section_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_nested_group_section_json = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(asset_path);
        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "4", "--json"},
            temp_root);

        if (section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " record-selected deleted nested group section stdout:\n"
                      << section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " record-selected deleted nested group section stderr:\n"
                      << section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(section_process.exit_code == 0,
               "#2681: record-selected deleted nested report/label group section JSON should exit successfully");
        expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2681: record-selected deleted nested group section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(section_process.stdout_text, "\"isLabel\": true",
                            "#2681: record-selected deleted nested label group section JSON should retain label identity");
        }
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2681: record-selected deleted nested group sections should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#2681: record-selected deleted nested group sections should advertise report-selection availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2681: record-selected deleted nested group sections should preserve selection kind");
        expect_contains(section_process.stdout_text, "\"sectionCount\": 4",
                        "#2681: record-selected deleted nested group section JSON should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#2681: record-selected deleted nested group section JSON should expose deleted section counts");
        expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2681: record-selected deleted nested group section JSON should preserve live preview availability");
        expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2681: record-selected deleted nested group section JSON should preserve live preview top bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2681: record-selected deleted nested group section JSON should preserve live preview bottom bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2681: record-selected deleted nested group section JSON should preserve live preview heights");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2681: record-selected deleted nested group section JSON should expose deleted preview availability");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                        "#2681: record-selected deleted nested group section JSON should preserve deleted preview top bounds");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                        "#2681: record-selected deleted nested group section JSON should preserve deleted preview bottom bounds");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#2681: record-selected deleted nested group section JSON should preserve deleted preview heights");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#2681: record-selected deleted nested group sections should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#2681: record-selected deleted nested group sections should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#2681: record-selected deleted nested group sections should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#2681: record-selected deleted nested group sections should serialize null selected settings");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"group_footer_4\"",
                "\"bandKind\": \"group_footer\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 1",
                "\"groupingNestingDepth\": 1",
                "\"groupRole\": \"footer\"",
                "\"groupPartnerSectionId\": \"group_header_2\"",
                "\"groupPartnerRecordIndex\": 2",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 4",
                "\"deleted\": true"
            },
            "#2681: record-selected deleted nested group section JSON should expose deleted nested section metadata");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_footer_4\"",
                "\"bandKind\": \"group_footer\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 1",
                "\"groupingNestingDepth\": 1",
                "\"groupRole\": \"footer\"",
                "\"groupPartnerSectionId\": \"group_header_2\"",
                "\"groupPartnerRecordIndex\": 2",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2900",
                "\"height\": 250",
                "\"bottom\": 3150"
            },
            "#2681: record-selected deleted nested group sections should expose selected deleted-section metadata");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_1\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 0",
                "\"groupingNestingDepth\": 0",
                "\"groupRole\": \"header\"",
                "\"groupPartnerSectionId\": \"group_footer_5\"",
                "\"groupPartnerRecordIndex\": 5",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 1",
                "\"id\": \"group_header_2\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 1",
                "\"groupingNestingDepth\": 1",
                "\"groupRole\": \"header\"",
                "\"groupPartnerSectionId\": \"group_footer_4\"",
                "\"groupPartnerRecordIndex\": 4",
                "\"groupPartnerDeleted\": true",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"groupingContextAvailable\": false",
                "\"groupingIndex\": null",
                "\"groupingNestingDepth\": null",
                "\"groupRole\": null",
                "\"groupPartnerSectionId\": null",
                "\"groupPartnerRecordIndex\": null",
                "\"groupPartnerDeleted\": false",
                "\"recordIndex\": 3",
                "\"id\": \"group_footer_5\"",
                "\"groupingContextAvailable\": true",
                "\"groupingIndex\": 0",
                "\"groupingNestingDepth\": 0",
                "\"groupRole\": \"footer\"",
                "\"groupPartnerSectionId\": \"group_header_1\"",
                "\"groupPartnerRecordIndex\": 1",
                "\"groupPartnerDeleted\": false",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 5"
            },
            "#2681: record-selected deleted nested group section JSON should preserve unaffected live sibling expressions");
    };

    run_deleted_nested_group_section_json(temp_root / "nested_deleted_group_sections_record.frx",
                                          "nested_deleted_group_sections_record.frx",
                                          "report");
    run_deleted_nested_group_section_json(temp_root / "nested_deleted_group_sections_record.lbx",
                                          "nested_deleted_group_sections_record.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_group_header_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_group_header_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_group_header_section_record.frx";
    write_synthetic_report_table_for_stable_group_section_expression_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected group-header report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected group-header report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1980: record-selected group-header report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_group_header_section_record.frx\"",
                    "#1980: record-selected group-header report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1980: record-selected group-header report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1980: record-selected group-header report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1980: record-selected group-header report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1980: record-selected group-header report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1980: record-selected group-header report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1980: record-selected group-header report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1980: record-selected group-header report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 4100",
                    "#1980: record-selected group-header report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1980: record-selected group-header report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 4100",
                    "#1980: record-selected group-header report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1980: record-selected group-header report section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1980: record-selected group-header report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1980: record-selected group-header report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1980: record-selected group-header report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1980: record-selected group-header report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1980: record-selected group-header report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1980: record-selected group-header report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1980: record-selected group-header report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1980: record-selected group-header report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"recordIndex\": 3"
        },
        "#1980: record-selected group-header report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_header_1\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 600",
            "\"bottom\": 600"
        },
        "#1980: record-selected group-header report sections should expose selected expression metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_group_footer_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_group_footer_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_group_footer_section_record.frx";
    write_synthetic_report_table_for_stable_group_section_expression_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected group-footer report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected group-footer report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1982: record-selected group-footer report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_group_footer_section_record.frx\"",
                    "#1982: record-selected group-footer report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1982: record-selected group-footer report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1982: record-selected group-footer report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1982: record-selected group-footer report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1982: record-selected group-footer report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1982: record-selected group-footer report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1982: record-selected group-footer report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1982: record-selected group-footer report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 4100",
                    "#1982: record-selected group-footer report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1982: record-selected group-footer report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 4100",
                    "#1982: record-selected group-footer report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1982: record-selected group-footer report section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1982: record-selected group-footer report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1982: record-selected group-footer report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1982: record-selected group-footer report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1982: record-selected group-footer report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1982: record-selected group-footer report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1982: record-selected group-footer report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1982: record-selected group-footer report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1982: record-selected group-footer report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3"
        },
        "#1982: record-selected group-footer report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_footer_3\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3600",
            "\"height\": 500",
            "\"bottom\": 4100"
        },
        "#1982: record-selected group-footer report sections should expose selected expression metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_group_header_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_group_header_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_group_header_section_record.frx";
    write_synthetic_report_table_for_stable_group_section_expression_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 1U),
           "#1984: record-selected deleted group-header report fixture should mark the group-header section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted group-header report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted group-header report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1984: record-selected deleted group-header report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_group_header_section_record.frx\"",
                    "#1984: record-selected deleted group-header report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1984: record-selected deleted group-header report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1984: record-selected deleted group-header report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1984: record-selected deleted group-header report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1984: record-selected deleted group-header report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1984: record-selected deleted group-header report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 600",
                    "#1984: record-selected deleted group-header report section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1984: record-selected deleted group-header report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 4100",
                    "#1984: record-selected deleted group-header report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1984: record-selected deleted group-header report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3500",
                    "#1984: record-selected deleted group-header report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1984: record-selected deleted group-header report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1984: record-selected deleted group-header report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#1984: record-selected deleted group-header report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1984: record-selected deleted group-header report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                    "#1984: record-selected deleted group-header report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1984: record-selected deleted group-header report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                    "#1984: record-selected deleted group-header report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1984: record-selected deleted group-header report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1984: record-selected deleted group-header report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1984: record-selected deleted group-header report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1984: record-selected deleted group-header report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1984: record-selected deleted group-header report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1984: record-selected deleted group-header report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1984: record-selected deleted group-header report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1984: record-selected deleted group-header report section JSON should expose deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"group_header_1\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#1984: record-selected deleted group-header report section JSON should expose deleted section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_header_1\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 600",
            "\"bottom\": 600"
        },
        "#1984: record-selected deleted group-header report sections should expose selected expression metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"group_footer\"",
            "\"recordIndex\": 3"
        },
        "#1984: record-selected deleted group-header report section JSON should preserve live sibling metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_group_footer_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_group_footer_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_group_footer_section_record.frx";
    write_synthetic_report_table_for_stable_group_section_expression_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 3U),
           "#1986: record-selected deleted group-footer report fixture should mark the group-footer section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted group-footer report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted group-footer report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1986: record-selected deleted group-footer report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_group_footer_section_record.frx\"",
                    "#1986: record-selected deleted group-footer report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1986: record-selected deleted group-footer report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1986: record-selected deleted group-footer report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1986: record-selected deleted group-footer report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1986: record-selected deleted group-footer report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1986: record-selected deleted group-footer report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1986: record-selected deleted group-footer report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1986: record-selected deleted group-footer report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3600",
                    "#1986: record-selected deleted group-footer report section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1986: record-selected deleted group-footer report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3600",
                    "#1986: record-selected deleted group-footer report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1986: record-selected deleted group-footer report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1986: record-selected deleted group-footer report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                    "#1986: record-selected deleted group-footer report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1986: record-selected deleted group-footer report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                    "#1986: record-selected deleted group-footer report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1986: record-selected deleted group-footer report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                    "#1986: record-selected deleted group-footer report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1986: record-selected deleted group-footer report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1986: record-selected deleted group-footer report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1986: record-selected deleted group-footer report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1986: record-selected deleted group-footer report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1986: record-selected deleted group-footer report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1986: record-selected deleted group-footer report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1986: record-selected deleted group-footer report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1986: record-selected deleted group-footer report section JSON should expose deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"group_footer_3\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3",
            "\"deleted\": true"
        },
        "#1986: record-selected deleted group-footer report section JSON should expose deleted section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_footer_3\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3600",
            "\"height\": 500",
            "\"bottom\": 4100"
        },
        "#1986: record-selected deleted group-footer report sections should expose selected expression metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"group_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2"
        },
        "#1986: record-selected deleted group-footer report section JSON should preserve live sibling metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_summary_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_summary_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_summary_section_record.frx";
    write_synthetic_report_table_for_stable_summary_section_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "2", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected summary report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected summary report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1988: record-selected summary report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_summary_section_record.frx\"",
                    "#1988: record-selected summary report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1988: record-selected summary report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1988: record-selected summary report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1988: record-selected summary report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1988: record-selected summary report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1988: record-selected summary report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1988: record-selected summary report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1988: record-selected summary report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3900",
                    "#1988: record-selected summary report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1988: record-selected summary report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3900",
                    "#1988: record-selected summary report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1988: record-selected summary report section JSON should not fabricate deleted preview bounds");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1988: record-selected summary report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1988: record-selected summary report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1988: record-selected summary report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1988: record-selected summary report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1988: record-selected summary report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1988: record-selected summary report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1988: record-selected summary report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1988: record-selected summary report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2"
        },
        "#1988: record-selected summary report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": false",
            "\"sectionIndex\": 1",
            "\"sectionCount\": 2",
            "\"top\": 3200",
            "\"height\": 700",
            "\"bottom\": 3900",
            "\"objectCount\": 0"
        },
        "#1988: record-selected summary report sections should expose selected summary metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_summary_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_summary_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_summary_section_record.frx";
    write_synthetic_report_table_for_stable_summary_section_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 2U),
           "#1990: record-selected deleted summary report fixture should mark the summary section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "2", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted summary report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted summary report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1990: record-selected deleted summary report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_summary_section_record.frx\"",
                    "#1990: record-selected deleted summary report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1990: record-selected deleted summary report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1990: record-selected deleted summary report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1990: record-selected deleted summary report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1990: record-selected deleted summary report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1990: record-selected deleted summary report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1990: record-selected deleted summary report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1990: record-selected deleted summary report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3200",
                    "#1990: record-selected deleted summary report section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1990: record-selected deleted summary report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3200",
                    "#1990: record-selected deleted summary report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1990: record-selected deleted summary report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1990: record-selected deleted summary report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3200",
                    "#1990: record-selected deleted summary report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1990: record-selected deleted summary report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3900",
                    "#1990: record-selected deleted summary report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1990: record-selected deleted summary report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                    "#1990: record-selected deleted summary report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1990: record-selected deleted summary report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1990: record-selected deleted summary report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1990: record-selected deleted summary report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1990: record-selected deleted summary report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1990: record-selected deleted summary report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1990: record-selected deleted summary report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 1",
                    "#1990: record-selected deleted summary report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1990: record-selected deleted summary report section JSON should expose deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": true"
        },
        "#1990: record-selected deleted summary report section JSON should expose deleted summary metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3200",
            "\"height\": 700",
            "\"bottom\": 3900"
        },
        "#1990: record-selected deleted summary report sections should expose selected deleted summary metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1"
        },
        "#1990: record-selected deleted summary report section JSON should preserve live detail metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_title_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_title_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_title_section_record.frx";
    write_synthetic_report_table_for_stable_title_section_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected title report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected title report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1992: record-selected title report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_title_section_record.frx\"",
                    "#1992: record-selected title report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1992: record-selected title report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1992: record-selected title report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1992: record-selected title report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1992: record-selected title report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1992: record-selected title report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1992: record-selected title report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1992: record-selected title report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#1992: record-selected title report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1992: record-selected title report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    "#1992: record-selected title report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1992: record-selected title report section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1992: record-selected title report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1992: record-selected title report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1992: record-selected title report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1992: record-selected title report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1992: record-selected title report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1992: record-selected title report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1992: record-selected title report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1992: record-selected title report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#1992: record-selected title report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"title_1\"",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#1992: record-selected title report sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_title_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_title_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_title_section_record.frx";
    write_synthetic_report_table_for_stable_title_section_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 1U),
           "#1994: record-selected deleted title report fixture should mark the title section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted title report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted title report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1994: record-selected deleted title report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_title_section_record.frx\"",
                    "#1994: record-selected deleted title report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1994: record-selected deleted title report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1994: record-selected deleted title report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1994: record-selected deleted title report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1994: record-selected deleted title report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1994: record-selected deleted title report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 700",
                    "#1994: record-selected deleted title report section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1994: record-selected deleted title report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#1994: record-selected deleted title report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1994: record-selected deleted title report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3000",
                    "#1994: record-selected deleted title report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1994: record-selected deleted title report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1994: record-selected deleted title report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#1994: record-selected deleted title report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1994: record-selected deleted title report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 700",
                    "#1994: record-selected deleted title report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1994: record-selected deleted title report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                    "#1994: record-selected deleted title report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1994: record-selected deleted title report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1994: record-selected deleted title report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1994: record-selected deleted title report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1994: record-selected deleted title report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1994: record-selected deleted title report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1994: record-selected deleted title report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1994: record-selected deleted title report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1994: record-selected deleted title report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"title_1\"",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#1994: record-selected deleted title report section JSON should expose deleted title metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#1994: record-selected deleted title report section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"title_1\"",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#1994: record-selected deleted title report sections should expose selected deleted title metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_page_footer_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_footer_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_page_footer_section_record.frx";
    write_synthetic_report_table_for_stable_title_section_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected page-footer report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected page-footer report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1996: record-selected page-footer report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_page_footer_section_record.frx\"",
                    "#1996: record-selected page-footer report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1996: record-selected page-footer report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1996: record-selected page-footer report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1996: record-selected page-footer report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1996: record-selected page-footer report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1996: record-selected page-footer report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1996: record-selected page-footer report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1996: record-selected page-footer report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#1996: record-selected page-footer report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1996: record-selected page-footer report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    "#1996: record-selected page-footer report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1996: record-selected page-footer report section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1996: record-selected page-footer report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1996: record-selected page-footer report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1996: record-selected page-footer report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1996: record-selected page-footer report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1996: record-selected page-footer report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1996: record-selected page-footer report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1996: record-selected page-footer report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1996: record-selected page-footer report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#1996: record-selected page-footer report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_footer_3\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3200",
            "\"height\": 500",
            "\"bottom\": 3700"
        },
        "#1996: record-selected page-footer report sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_page_footer_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_page_footer_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_page_footer_section_record.frx";
    write_synthetic_report_table_for_stable_title_section_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 3U),
           "#1998: record-selected deleted page-footer report fixture should mark the page-footer section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted page-footer report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted page-footer report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1998: record-selected deleted page-footer report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_page_footer_section_record.frx\"",
                    "#1998: record-selected deleted page-footer report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1998: record-selected deleted page-footer report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1998: record-selected deleted page-footer report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1998: record-selected deleted page-footer report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1998: record-selected deleted page-footer report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1998: record-selected deleted page-footer report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1998: record-selected deleted page-footer report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1998: record-selected deleted page-footer report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3200",
                    "#1998: record-selected deleted page-footer report section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1998: record-selected deleted page-footer report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3200",
                    "#1998: record-selected deleted page-footer report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1998: record-selected deleted page-footer report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1998: record-selected deleted page-footer report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3200",
                    "#1998: record-selected deleted page-footer report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1998: record-selected deleted page-footer report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3700",
                    "#1998: record-selected deleted page-footer report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1998: record-selected deleted page-footer report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                    "#1998: record-selected deleted page-footer report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1998: record-selected deleted page-footer report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1998: record-selected deleted page-footer report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1998: record-selected deleted page-footer report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1998: record-selected deleted page-footer report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1998: record-selected deleted page-footer report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1998: record-selected deleted page-footer report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1998: record-selected deleted page-footer report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1998: record-selected deleted page-footer report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"page_footer_3\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true"
        },
        "#1998: record-selected deleted page-footer report section JSON should expose deleted page-footer metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2"
        },
        "#1998: record-selected deleted page-footer report section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_footer_3\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3200",
            "\"height\": 500",
            "\"bottom\": 3700"
        },
        "#1998: record-selected deleted page-footer report sections should expose selected deleted page-footer metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_column_header_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_column_header_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_column_header_section_record.frx";
    write_synthetic_report_table_for_stable_column_section_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected column-header report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected column-header report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2000: record-selected column-header report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_column_header_section_record.frx\"",
                    "#2000: record-selected column-header report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2000: record-selected column-header report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2000: record-selected column-header report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2000: record-selected column-header report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2000: record-selected column-header report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2000: record-selected column-header report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2000: record-selected column-header report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2000: record-selected column-header report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3450",
                    "#2000: record-selected column-header report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2000: record-selected column-header report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3450",
                    "#2000: record-selected column-header report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2000: record-selected column-header report section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2000: record-selected column-header report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2000: record-selected column-header report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2000: record-selected column-header report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2000: record-selected column-header report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2000: record-selected column-header report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2000: record-selected column-header report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#2000: record-selected column-header report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#2000: record-selected column-header report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3"
        },
        "#2000: record-selected column-header report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_header_1\"",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 450",
            "\"bottom\": 450"
        },
        "#2000: record-selected column-header report sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_column_header_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_column_header_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_column_header_section_record.frx";
    write_synthetic_report_table_for_stable_column_section_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 1U),
           "#2002: record-selected deleted column-header report fixture should mark the column-header section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted column-header report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted column-header report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2002: record-selected deleted column-header report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_column_header_section_record.frx\"",
                    "#2002: record-selected deleted column-header report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2002: record-selected deleted column-header report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2002: record-selected deleted column-header report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2002: record-selected deleted column-header report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2002: record-selected deleted column-header report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2002: record-selected deleted column-header report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 450",
                    "#2002: record-selected deleted column-header report section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2002: record-selected deleted column-header report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3450",
                    "#2002: record-selected deleted column-header report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2002: record-selected deleted column-header report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3000",
                    "#2002: record-selected deleted column-header report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2002: record-selected deleted column-header report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2002: record-selected deleted column-header report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2002: record-selected deleted column-header report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2002: record-selected deleted column-header report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 450",
                    "#2002: record-selected deleted column-header report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2002: record-selected deleted column-header report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 450",
                    "#2002: record-selected deleted column-header report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2002: record-selected deleted column-header report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2002: record-selected deleted column-header report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2002: record-selected deleted column-header report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2002: record-selected deleted column-header report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2002: record-selected deleted column-header report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2002: record-selected deleted column-header report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#2002: record-selected deleted column-header report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#2002: record-selected deleted column-header report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"column_header_1\"",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#2002: record-selected deleted column-header report section JSON should expose deleted column-header metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3"
        },
        "#2002: record-selected deleted column-header report section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_header_1\"",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 450",
            "\"bottom\": 450"
        },
        "#2002: record-selected deleted column-header report sections should expose selected deleted column-header metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_column_footer_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_column_footer_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_column_footer_section_record.frx";
    write_synthetic_report_table_for_stable_column_section_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected column-footer report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected column-footer report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2004: record-selected column-footer report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_column_footer_section_record.frx\"",
                    "#2004: record-selected column-footer report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2004: record-selected column-footer report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2004: record-selected column-footer report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2004: record-selected column-footer report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2004: record-selected column-footer report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2004: record-selected column-footer report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2004: record-selected column-footer report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2004: record-selected column-footer report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3450",
                    "#2004: record-selected column-footer report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2004: record-selected column-footer report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3450",
                    "#2004: record-selected column-footer report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2004: record-selected column-footer report section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2004: record-selected column-footer report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2004: record-selected column-footer report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2004: record-selected column-footer report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2004: record-selected column-footer report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2004: record-selected column-footer report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2004: record-selected column-footer report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#2004: record-selected column-footer report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#2004: record-selected column-footer report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3"
        },
        "#2004: record-selected column-footer report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_footer_3\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3050",
            "\"height\": 400",
            "\"bottom\": 3450"
        },
        "#2004: record-selected column-footer report sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_column_footer_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_column_footer_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_column_footer_section_record.frx";
    write_synthetic_report_table_for_stable_column_section_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 3U),
           "#2006: record-selected deleted column-footer report fixture should mark the column-footer section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted column-footer report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted column-footer report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2006: record-selected deleted column-footer report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_column_footer_section_record.frx\"",
                    "#2006: record-selected deleted column-footer report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2006: record-selected deleted column-footer report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2006: record-selected deleted column-footer report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2006: record-selected deleted column-footer report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2006: record-selected deleted column-footer report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2006: record-selected deleted column-footer report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2006: record-selected deleted column-footer report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2006: record-selected deleted column-footer report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3050",
                    "#2006: record-selected deleted column-footer report section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2006: record-selected deleted column-footer report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3050",
                    "#2006: record-selected deleted column-footer report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2006: record-selected deleted column-footer report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2006: record-selected deleted column-footer report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3050",
                    "#2006: record-selected deleted column-footer report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2006: record-selected deleted column-footer report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3450",
                    "#2006: record-selected deleted column-footer report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2006: record-selected deleted column-footer report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 400",
                    "#2006: record-selected deleted column-footer report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2006: record-selected deleted column-footer report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2006: record-selected deleted column-footer report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2006: record-selected deleted column-footer report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2006: record-selected deleted column-footer report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2006: record-selected deleted column-footer report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2006: record-selected deleted column-footer report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#2006: record-selected deleted column-footer report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#2006: record-selected deleted column-footer report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"column_footer_3\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true"
        },
        "#2006: record-selected deleted column-footer report section JSON should expose deleted column-footer metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2"
        },
        "#2006: record-selected deleted column-footer report section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_footer_3\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3050",
            "\"height\": 400",
            "\"bottom\": 3450"
        },
        "#2006: record-selected deleted column-footer report sections should expose selected deleted column-footer metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_page_header_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_header_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_page_header_section_record.frx";
    write_synthetic_report_table_for_stable_page_header_section_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected page-header report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected page-header report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2008: record-selected page-header report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_page_header_section_record.frx\"",
                    "#2008: record-selected page-header report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2008: record-selected page-header report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2008: record-selected page-header report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2008: record-selected page-header report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2008: record-selected page-header report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2008: record-selected page-header report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2008: record-selected page-header report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2008: record-selected page-header report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#2008: record-selected page-header report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2008: record-selected page-header report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    "#2008: record-selected page-header report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2008: record-selected page-header report section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2008: record-selected page-header report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2008: record-selected page-header report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2008: record-selected page-header report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2008: record-selected page-header report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2008: record-selected page-header report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2008: record-selected page-header report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#2008: record-selected page-header report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#2008: record-selected page-header report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#2008: record-selected page-header report section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#2008: record-selected page-header report sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_page_header_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_page_header_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_page_header_section_record.frx";
    write_synthetic_report_table_for_stable_page_header_section_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(report_path, 1U),
           "#2010: record-selected deleted page-header report fixture should mark the page-header section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted page-header report section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted page-header report section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2010: record-selected deleted page-header report section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_page_header_section_record.frx\"",
                    "#2010: record-selected deleted page-header report section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2010: record-selected deleted page-header report sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2010: record-selected deleted page-header report sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2010: record-selected deleted page-header report sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2010: record-selected deleted page-header report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2010: record-selected deleted page-header report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 700",
                    "#2010: record-selected deleted page-header report section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2010: record-selected deleted page-header report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#2010: record-selected deleted page-header report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2010: record-selected deleted page-header report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3000",
                    "#2010: record-selected deleted page-header report section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2010: record-selected deleted page-header report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2010: record-selected deleted page-header report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2010: record-selected deleted page-header report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2010: record-selected deleted page-header report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 700",
                    "#2010: record-selected deleted page-header report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2010: record-selected deleted page-header report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                    "#2010: record-selected deleted page-header report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2010: record-selected deleted page-header report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2010: record-selected deleted page-header report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2010: record-selected deleted page-header report sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2010: record-selected deleted page-header report sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2010: record-selected deleted page-header report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2010: record-selected deleted page-header report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#2010: record-selected deleted page-header report section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#2010: record-selected deleted page-header report section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#2010: record-selected deleted page-header report section JSON should expose deleted page-header metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#2010: record-selected deleted page-header report section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#2010: record-selected deleted page-header report sections should expose selected deleted page-header metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_summary_report_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_summary_report_sections_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_summary_section_selection = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_summary_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "summary-section-guid", "--json"},
            temp_root);

        if (section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected summary section stdout:\n"
                      << section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected summary section stderr:\n"
                      << section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(section_process.exit_code == 0,
               "#1694: stable selected summary report/label section JSON should exit successfully");
        expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1694: stable selected summary section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(section_process.stdout_text, "\"isLabel\": true",
                            "#1694: stable selected summary label section JSON should retain label identity");
        }
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1694: stable selected summary sections should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1694: stable selected summary sections should advertise report-selection availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1694: stable selected summary sections should expose section selection kind");
        expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                        "#1694: stable selected summary sections should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1694: stable selected summary sections should preserve deleted section counts");
        expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1931: stable selected summary section JSON should preserve preview availability");
        expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1931: stable selected summary section JSON should preserve preview left bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1931: stable selected summary section JSON should preserve preview top bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1931: stable selected summary section JSON should preserve preview right bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3900",
                        "#1931: stable selected summary section JSON should preserve preview bottom bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1931: stable selected summary section JSON should preserve preview widths");
        expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3900",
                        "#1931: stable selected summary section JSON should preserve preview heights");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1931: stable selected summary section JSON should not fabricate deleted preview bounds");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1694: stable selected summary sections should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#1694: stable selected summary sections should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1694: stable selected summary sections should not advertise selected object-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1694: stable selected summary sections should serialize null selected object sections");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1694: stable selected summary sections should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1694: stable selected summary sections should serialize null selected settings");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 2"
            },
            "#1694: stable selected summary section JSON should expose sibling section metadata");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"summary_2\"",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 2",
                "\"top\": 3200",
                "\"height\": 700",
                "\"bottom\": 3900",
                "\"objectCount\": 0"
            },
            "#1694: stable selected summary sections should expose selected summary metadata");
    };

    run_summary_section_selection(temp_root / "selected_summary_section_stable.frx",
                                  "selected_summary_section_stable.frx",
                                  "report");
    run_summary_section_selection(temp_root / "selected_summary_section_stable.lbx",
                                  "selected_summary_section_stable.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_summary_report_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_summary_report_sections_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_summary_section_selection = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_stable_summary_section_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 2U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 2U),
               "#1695: stable deleted summary fixture should mark the summary section deleted");

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "summary-section-guid", "--json"},
            temp_root);

        if (section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted summary section stdout:\n"
                      << section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted summary section stderr:\n"
                      << section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(section_process.exit_code == 0,
               "#1695: stable selected deleted summary report/label section JSON should exit successfully");
        expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1695: stable selected deleted summary section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(section_process.stdout_text, "\"isLabel\": true",
                            "#1695: stable selected deleted summary label section JSON should retain label identity");
        }
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1695: stable selected deleted summary sections should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1695: stable selected deleted summary sections should advertise report-selection availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1695: stable selected deleted summary sections should expose section selection kind");
        expect_contains(section_process.stdout_text, "\"sectionCount\": 1",
                        "#1695: stable selected deleted summary sections should preserve live sibling counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1695: stable selected deleted summary sections should expose deleted section counts");
        expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1932: stable selected deleted summary section JSON should preserve live preview availability");
        expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1932: stable selected deleted summary section JSON should preserve live preview left bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1932: stable selected deleted summary section JSON should preserve live preview top bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1932: stable selected deleted summary section JSON should preserve live preview right bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3200",
                        "#1932: stable selected deleted summary section JSON should refresh live preview bottom bounds");
        expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1932: stable selected deleted summary section JSON should preserve live preview widths");
        expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3200",
                        "#1932: stable selected deleted summary section JSON should refresh live preview heights");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1932: stable selected deleted summary section JSON should expose deleted preview availability");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1932: stable selected deleted summary section JSON should preserve deleted preview left bounds");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3200",
                        "#1932: stable selected deleted summary section JSON should preserve deleted preview top bounds");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1932: stable selected deleted summary section JSON should preserve deleted preview right bounds");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3900",
                        "#1932: stable selected deleted summary section JSON should preserve deleted preview bottom bounds");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1932: stable selected deleted summary section JSON should preserve deleted preview widths");
        expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                        "#1932: stable selected deleted summary section JSON should preserve deleted preview heights");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1695: stable selected deleted summary sections should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#1695: stable selected deleted summary sections should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1695: stable selected deleted summary sections should not advertise selected object-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1695: stable selected deleted summary sections should serialize null selected object sections");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1695: stable selected deleted summary sections should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1695: stable selected deleted summary sections should serialize null selected settings");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"summary_2\"",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 2",
                "\"deleted\": true"
            },
            "#1695: stable selected deleted summary section JSON should expose deleted summary metadata");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"summary_2\"",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 3200",
                "\"height\": 700",
                "\"bottom\": 3900"
            },
            "#1695: stable selected deleted summary sections should expose selected deleted summary metadata");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1"
            },
            "#1695: stable selected deleted summary section JSON should preserve live detail metadata");
    };

    run_deleted_summary_section_selection(temp_root / "selected_deleted_summary_section_stable.frx",
                                          "selected_deleted_summary_section_stable.frx",
                                          "report");
    run_deleted_summary_section_selection(temp_root / "selected_deleted_summary_section_stable.lbx",
                                          "selected_deleted_summary_section_stable.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_title_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_title_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_title_section_json = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_title_section_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "title-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable title section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable title section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1674: stable selected report/label title section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1674: stable selected title section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1674: stable selected title label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1674: stable selected title sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1674: stable selected title sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1674: stable selected title sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1674: stable selected title section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1674: stable selected title section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1933: stable selected title section JSON should expose live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1933: stable selected title section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1933: stable selected title section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1933: stable selected title section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3700",
                        "#1933: stable selected title section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1933: stable selected title section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3700",
                        "#1933: stable selected title section JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1933: stable selected title section JSON should not fabricate deleted preview availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1674: stable selected title sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1674: stable selected title sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1674: stable selected title sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1674: stable selected title sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1674: stable selected title sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1674: stable selected title sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3"
            },
            "#1674: stable selected title section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"title_1\"",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"top\": 0",
                "\"height\": 700",
                "\"bottom\": 700"
            },
            "#1674: stable selected title sections should expose selected section metadata");
    };

    run_title_section_json(temp_root / "stable_title_sections.frx",
                           "stable_title_sections.frx",
                           "report");
    run_title_section_json(temp_root / "stable_title_sections.lbx",
                           "stable_title_sections.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_title_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_title_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_title_section_json = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_title_section_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 1U),
               "#1678: stable deleted title fixture should mark the title section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "title-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted title section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted title section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1678: stable selected deleted report/label title section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1678: stable selected deleted title section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1678: stable selected deleted title label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1678: stable selected deleted title sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1678: stable selected deleted title sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1678: stable selected deleted title sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1678: stable selected deleted title section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1678: stable selected deleted title section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1934: stable selected deleted title section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1934: stable selected deleted title section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 700",
                        "#1934: stable selected deleted title section JSON should refresh live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1934: stable selected deleted title section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3700",
                        "#1934: stable selected deleted title section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1934: stable selected deleted title section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3000",
                        "#1934: stable selected deleted title section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1934: stable selected deleted title section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1934: stable selected deleted title section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1934: stable selected deleted title section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1934: stable selected deleted title section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 700",
                        "#1934: stable selected deleted title section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1934: stable selected deleted title section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                        "#1934: stable selected deleted title section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1678: stable selected deleted title sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1678: stable selected deleted title sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1678: stable selected deleted title sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1678: stable selected deleted title sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1678: stable selected deleted title sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1678: stable selected deleted title sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"title_1\"",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"deleted\": true"
            },
            "#1678: stable selected deleted title section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"title_1\"",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 0",
                "\"height\": 700",
                "\"bottom\": 700"
            },
            "#1678: stable selected deleted title sections should expose selected section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3"
            },
            "#1678: stable selected deleted title section JSON should preserve live sibling metadata");
    };

    run_deleted_title_section_json(temp_root / "stable_deleted_title_sections.frx",
                                   "stable_deleted_title_sections.frx",
                                   "report");
    run_deleted_title_section_json(temp_root / "stable_deleted_title_sections.lbx",
                                   "stable_deleted_title_sections.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_page_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_page_footer_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_page_footer_section_json = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_stable_title_section_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 3U),
               "#1679: stable deleted page-footer fixture should mark the page-footer section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "page-footer-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted page-footer section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted page-footer section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1679: stable selected deleted report/label page-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1679: stable selected deleted page-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1679: stable selected deleted page-footer label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1679: stable selected deleted page-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1679: stable selected deleted page-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1679: stable selected deleted page-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1679: stable selected deleted page-footer section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1679: stable selected deleted page-footer section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1936: stable selected deleted page-footer section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1936: stable selected deleted page-footer section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1936: stable selected deleted page-footer section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1936: stable selected deleted page-footer section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3200",
                        "#1936: stable selected deleted page-footer section JSON should refresh live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1936: stable selected deleted page-footer section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3200",
                        "#1936: stable selected deleted page-footer section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1936: stable selected deleted page-footer section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1936: stable selected deleted page-footer section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3200",
                        "#1936: stable selected deleted page-footer section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1936: stable selected deleted page-footer section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 3700",
                        "#1936: stable selected deleted page-footer section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1936: stable selected deleted page-footer section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#1936: stable selected deleted page-footer section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1679: stable selected deleted page-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1679: stable selected deleted page-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1679: stable selected deleted page-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1679: stable selected deleted page-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1679: stable selected deleted page-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1679: stable selected deleted page-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"page_footer_3\"",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": true"
            },
            "#1679: stable selected deleted page-footer section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"page_footer_3\"",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 3200",
                "\"height\": 500",
                "\"bottom\": 3700"
            },
            "#1679: stable selected deleted page-footer sections should expose selected section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1679: stable selected deleted page-footer section JSON should preserve live sibling metadata");
    };

    run_deleted_page_footer_section_json(temp_root / "stable_deleted_page_footer_sections.frx",
                                         "stable_deleted_page_footer_sections.frx",
                                         "report");
    run_deleted_page_footer_section_json(temp_root / "stable_deleted_page_footer_sections.lbx",
                                         "stable_deleted_page_footer_sections.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_page_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_page_footer_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_page_footer_section_json = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_title_section_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "page-footer-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable page-footer section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable page-footer section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1675: stable selected report/label page-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1675: stable selected page-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1675: stable selected page-footer label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1675: stable selected page-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1675: stable selected page-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1675: stable selected page-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1675: stable selected page-footer section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1675: stable selected page-footer section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1935: stable selected page-footer section JSON should expose live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1935: stable selected page-footer section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1935: stable selected page-footer section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1935: stable selected page-footer section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3700",
                        "#1935: stable selected page-footer section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1935: stable selected page-footer section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3700",
                        "#1935: stable selected page-footer section JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1935: stable selected page-footer section JSON should not fabricate deleted preview availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1675: stable selected page-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1675: stable selected page-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1675: stable selected page-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1675: stable selected page-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1675: stable selected page-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1675: stable selected page-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3"
            },
            "#1675: stable selected page-footer section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"page_footer_3\"",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"top\": 3200",
                "\"height\": 500",
                "\"bottom\": 3700"
            },
            "#1675: stable selected page-footer sections should expose selected section metadata");
    };

    run_page_footer_section_json(temp_root / "stable_page_footer_sections.frx",
                                 "stable_page_footer_sections.frx",
                                 "report");
    run_page_footer_section_json(temp_root / "stable_page_footer_sections.lbx",
                                 "stable_page_footer_sections.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_column_header_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_header_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_header_section_json = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_column_section_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-header-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-header section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-header section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1676: stable selected report/label column-header section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1676: stable selected column-header section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1676: stable selected column-header label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1676: stable selected column-header sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1676: stable selected column-header sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1676: stable selected column-header sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1676: stable selected column-header section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1676: stable selected column-header section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1937: stable selected column-header section JSON should expose live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1937: stable selected column-header section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1937: stable selected column-header section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1937: stable selected column-header section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3450",
                        "#1937: stable selected column-header section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1937: stable selected column-header section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3450",
                        "#1937: stable selected column-header section JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1937: stable selected column-header section JSON should not fabricate deleted preview availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1676: stable selected column-header sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1676: stable selected column-header sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1676: stable selected column-header sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1676: stable selected column-header sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1676: stable selected column-header sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1676: stable selected column-header sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3"
            },
            "#1676: stable selected column-header section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"column_header_1\"",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"top\": 0",
                "\"height\": 450",
                "\"bottom\": 450"
            },
            "#1676: stable selected column-header sections should expose selected section metadata");
    };

    run_column_header_section_json(temp_root / "stable_column_header_sections.frx",
                                   "stable_column_header_sections.frx",
                                   "report");
    run_column_header_section_json(temp_root / "stable_column_header_sections.lbx",
                                   "stable_column_header_sections.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_column_header_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_header_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_header_section_json = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_stable_column_section_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 1U),
               "#1680: stable deleted column-header fixture should mark the column-header section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-header-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-header section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-header section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1680: stable selected deleted report/label column-header section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1680: stable selected deleted column-header section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1680: stable selected deleted column-header label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1680: stable selected deleted column-header sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1680: stable selected deleted column-header sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1680: stable selected deleted column-header sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1680: stable selected deleted column-header section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1680: stable selected deleted column-header section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1938: stable selected deleted column-header section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1938: stable selected deleted column-header section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 450",
                        "#1938: stable selected deleted column-header section JSON should refresh live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1938: stable selected deleted column-header section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3450",
                        "#1938: stable selected deleted column-header section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1938: stable selected deleted column-header section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3000",
                        "#1938: stable selected deleted column-header section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1938: stable selected deleted column-header section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1938: stable selected deleted column-header section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1938: stable selected deleted column-header section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1938: stable selected deleted column-header section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 450",
                        "#1938: stable selected deleted column-header section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1938: stable selected deleted column-header section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 450",
                        "#1938: stable selected deleted column-header section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1680: stable selected deleted column-header sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1680: stable selected deleted column-header sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1680: stable selected deleted column-header sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1680: stable selected deleted column-header sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1680: stable selected deleted column-header sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1680: stable selected deleted column-header sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"column_header_1\"",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true"
            },
            "#1680: stable selected deleted column-header section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"column_header_1\"",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 0",
                "\"height\": 450",
                "\"bottom\": 450"
            },
            "#1680: stable selected deleted column-header sections should expose selected section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3"
            },
            "#1680: stable selected deleted column-header section JSON should preserve live sibling metadata");
    };

    run_deleted_column_header_section_json(temp_root / "stable_deleted_column_header_sections.frx",
                                           "stable_deleted_column_header_sections.frx",
                                           "report");
    run_deleted_column_header_section_json(temp_root / "stable_deleted_column_header_sections.lbx",
                                           "stable_deleted_column_header_sections.lbx",
                                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_column_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_footer_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_footer_section_json = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_column_section_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-footer-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-footer section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-footer section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1677: stable selected report/label column-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1677: stable selected column-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1677: stable selected column-footer label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1677: stable selected column-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1677: stable selected column-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1677: stable selected column-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1677: stable selected column-footer section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1677: stable selected column-footer section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1939: stable selected column-footer section JSON should expose live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1939: stable selected column-footer section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1939: stable selected column-footer section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1939: stable selected column-footer section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3450",
                        "#1939: stable selected column-footer section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1939: stable selected column-footer section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3450",
                        "#1939: stable selected column-footer section JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1939: stable selected column-footer section JSON should not fabricate deleted preview availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1677: stable selected column-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1677: stable selected column-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1677: stable selected column-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1677: stable selected column-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1677: stable selected column-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1677: stable selected column-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3"
            },
            "#1677: stable selected column-footer section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"column_footer_3\"",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"top\": 3050",
                "\"height\": 400",
                "\"bottom\": 3450"
            },
            "#1677: stable selected column-footer sections should expose selected section metadata");
    };

    run_column_footer_section_json(temp_root / "stable_column_footer_sections.frx",
                                   "stable_column_footer_sections.frx",
                                   "report");
    run_column_footer_section_json(temp_root / "stable_column_footer_sections.lbx",
                                   "stable_column_footer_sections.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
