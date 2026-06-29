#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_group_section_expression_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", ""},
        {"9", "3", "customer.country", "0", "600"},
        {"9", "4", "", "600", "3000"},
        {"9", "5", "customer.country", "3600", "500"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1566: synthetic report table for group section expression JSON should be created");
}

void write_synthetic_report_table_for_stable_group_section_expression_json(
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
        {"9", "3", "customer.country", "0", "600", "group-header-guid"},
        {"9", "4", "", "600", "3000", ""},
        {"9", "5", "customer.country", "3600", "500", "group-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1666: synthetic report table for stable group section expression JSON should be created");
}

void write_synthetic_report_table_for_stable_blank_group_footer_expression_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "3", "customer.country", "", "0", "", "600", "group-header-guid"},
        {"9", "4", "", "", "600", "", "3000", ""},
        {"5", "", "\"Detail label\"", "120", "900", "1400", "240", "detail-object-guid"},
        {"9", "5", "", "", "3600", "", "500", "group-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#2687: synthetic report table for stable blank group-footer expression JSON should be created");
}

void write_synthetic_report_table_for_deleted_blank_group_footer_expression_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_blank_group_footer_expression_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok,
           "#2687: synthetic report table should mark the blank group-footer section deleted");
}

void write_synthetic_report_table_for_stable_nested_group_section_expression_json(
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
        {"9", "3", "customer.region", "0", "400", "region-header-guid"},
        {"9", "3", "customer.country", "400", "300", "country-header-guid"},
        {"9", "4", "", "700", "2200", ""},
        {"9", "5", "customer.country", "2900", "250", "country-footer-guid"},
        {"9", "5", "customer.region", "3150", "350", "region-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#2680: synthetic report table for stable nested group section expression JSON should be created");
}

void write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_nested_group_section_expression_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok,
           "#2681: synthetic report table should mark the nested group footer section deleted");
}

void write_synthetic_report_table_for_deleted_group_section_expression_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_group_section_expression_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1569: synthetic report table should mark group section deleted");
}

void write_synthetic_report_table_for_deleted_group_footer_expression_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_group_section_expression_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1570: synthetic report table should mark group footer section deleted");
}

void test_studio_host_json_exposes_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_section_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_expression_json = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_stable_group_section_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-header-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1666: stable selected report/label group-header section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1666: stable selected group-header section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1666: stable selected label group-header section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1666: stable selected group-header sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1666: stable selected group-header sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1666: stable selected group-header sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1666: stable selected group-header section JSON should preserve sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1666: stable selected group-header section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1929: stable selected group-header section JSON should preserve preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#1929: stable selected group-header section JSON should preserve preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#1929: stable selected group-header section JSON should preserve preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1929: stable selected group-header section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1666: stable selected group-header sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1666: stable selected group-header sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1666: stable selected group-header sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1666: stable selected group-header sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1666: stable selected group-header sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1666: stable selected group-header sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1666: stable selected group-header section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1666: stable selected group-header sections should expose selected expression metadata");
    };

    run_group_expression_json(temp_root / "stable_group_sections.frx",
                              "stable_group_sections.frx",
                              "report");
    run_group_expression_json(temp_root / "stable_group_sections.lbx",
                              "stable_group_sections.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_nested_report_group_section_ordering_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_json = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_stable_nested_group_section_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "country-header-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable nested group section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable nested group section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#2680: stable selected nested report/label group section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2680: stable selected nested group section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2680: stable selected nested label group section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 5",
                        "#2680: stable selected nested group section JSON should preserve both group levels");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2680: stable selected nested group section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2680: stable selected nested group section JSON should preserve preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2680: stable selected nested group section JSON should preserve preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2680: stable selected nested group section JSON should preserve preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2680: stable selected nested group section JSON should preserve preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2680: stable selected nested group section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2680: stable selected nested group section JSON should advertise section selection");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#2680: stable selected nested group section JSON should advertise report selection");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2680: stable selected nested group section JSON should preserve selection classification");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#2680: stable selected nested group section JSON should keep selected object unavailable");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#2680: stable selected nested group section JSON should keep selected settings unavailable");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_1\"",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 1",
                "\"id\": \"group_header_2\"",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"group_footer_4\"",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 4",
                "\"id\": \"group_footer_5\"",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 5"
            },
            "#2680: stable selected nested group section JSON should preserve nested sibling ordering and expressions");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_header_2\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 2",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 5",
                "\"top\": 400",
                "\"height\": 300",
                "\"bottom\": 700"
            },
            "#2680: stable selected nested group section JSON should expose selected inner-group metadata");
    };

    run_nested_group_expression_json(temp_root / "nested_group_sections.frx",
                                     "nested_group_sections.frx",
                                     "report");
    run_nested_group_expression_json(temp_root / "nested_group_sections.lbx",
                                     "nested_group_sections.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_groupings_in_layout_summary(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_groupings_summary_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_groupings_summary_json = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_stable_group_section_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " report grouping summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " report grouping summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#2685: report/label grouping summary JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2685: report/label grouping summary JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2685: label grouping summary JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"groupingCount\": 1",
                        "#2685: report/label grouping summary JSON should expose one grouping");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#2685: report/label grouping summary JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2685: report/label grouping summary JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#2685: report/label grouping summary JSON should preserve no-selection state");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#2685: report/label grouping summary JSON should classify the summary launch as no selection");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"groupings\": [",
                "\"groupingIndex\": 0",
                "\"nestingDepth\": 0",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"headerSectionId\": \"group_header_1\"",
                "\"headerRecordIndex\": 1",
                "\"headerDeleted\": false",
                "\"footerSectionId\": \"group_footer_3\"",
                "\"footerRecordIndex\": 3",
                "\"footerDeleted\": false"
            },
            "#2685: report/label grouping summary JSON should expose explicit group-header/footer pairing metadata");
    };

    run_groupings_summary_json(temp_root / "groupings_summary.frx",
                               "groupings_summary.frx",
                               "report");
    run_groupings_summary_json(temp_root / "groupings_summary.lbx",
                               "groupings_summary.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_nested_mixed_state_groupings_in_layout_summary(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_groupings_summary_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_groupings_summary_json = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " nested grouping summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " nested grouping summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#2685: nested mixed-state report/label grouping summary JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2685: nested mixed-state grouping summary JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2685: nested mixed-state label grouping summary JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"groupingCount\": 2",
                        "#2685: nested mixed-state grouping summary JSON should expose both grouping levels");
        expect_contains(process.stdout_text, "\"sectionCount\": 4",
                        "#2685: nested mixed-state grouping summary JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#2685: nested mixed-state grouping summary JSON should preserve deleted section counts");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"groupings\": [",
                "\"groupingIndex\": 0",
                "\"nestingDepth\": 0",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"headerSectionId\": \"group_header_1\"",
                "\"headerRecordIndex\": 1",
                "\"headerDeleted\": false",
                "\"footerSectionId\": \"group_footer_5\"",
                "\"footerRecordIndex\": 5",
                "\"footerDeleted\": false",
                "\"groupingIndex\": 1",
                "\"nestingDepth\": 1",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"headerSectionId\": \"group_header_2\"",
                "\"headerRecordIndex\": 2",
                "\"headerDeleted\": false",
                "\"footerSectionId\": \"group_footer_4\"",
                "\"footerRecordIndex\": 4",
                "\"footerDeleted\": true"
            },
            "#2685: nested mixed-state grouping summary JSON should pair live headers with deleted nested footers");
    };

    run_nested_groupings_summary_json(temp_root / "nested_groupings_summary.frx",
                                      "nested_groupings_summary.frx",
                                      "report");
    run_nested_groupings_summary_json(temp_root / "nested_groupings_summary.lbx",
                                      "nested_groupings_summary.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_resolved_grouping_expression_for_blank_footer_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_blank_group_footer_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_blank_footer_grouping_expression_json = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label,
                                                               bool deleted_footer) {
        if (deleted_footer) {
            write_synthetic_report_table_for_deleted_blank_group_footer_expression_json(asset_path);
        } else {
            write_synthetic_report_table_for_stable_blank_group_footer_expression_json(asset_path);
        }

        const auto footer_record = deleted_footer ? "4" : "4";
        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", footer_record, "--json"},
            temp_root);

        if (section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank group footer expression stdout:\n"
                      << section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank group footer expression stderr:\n"
                      << section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(section_process.exit_code == 0,
               "#2687: blank group-footer selection JSON should exit successfully");
        expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2687: blank group-footer selection JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(section_process.stdout_text, "\"isLabel\": true",
                            "#2687: blank group-footer label selection JSON should retain label identity");
        }
        expect_contains_in_order(
            section_process.stdout_text,
            {
                deleted_footer ? "\"deletedSections\": [" : "\"sections\": [",
                "\"id\": \"group_footer_4\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"groupingExpression\": \"customer.country\"",
                "\"groupingExpressionFieldIndex\": 2",
                "\"groupingExpressionMemoBlockNumber\": 2"
            },
            "#2687: blank group-footer section arrays should expose resolved grouping-expression metadata without fabricating direct EXPR");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_footer_4\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"groupRole\": \"footer\"",
                "\"groupingExpression\": \"customer.country\"",
                "\"groupingExpressionFieldIndex\": 2",
                "\"groupingExpressionMemoBlockNumber\": 2"
            },
            "#2687: selected blank group footers should expose resolved grouping-expression provenance from the paired header");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank group footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank group footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#2687: blank group-footer selected object JSON should exit successfully");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"groupingContextAvailable\": false",
                "\"groupingExpression\": null",
                "\"groupingExpressionFieldIndex\": null",
                "\"groupingExpressionMemoBlockNumber\": 0"
            },
            "#2687: non-group selected object sections should keep resolved grouping-expression metadata absent");
    };

    run_blank_footer_grouping_expression_json(temp_root / "blank_group_footer.frx",
                                              "blank_group_footer.frx",
                                              "report",
                                              false);
    run_blank_footer_grouping_expression_json(temp_root / "blank_group_footer.lbx",
                                              "blank_group_footer.lbx",
                                              "label",
                                              false);
    run_blank_footer_grouping_expression_json(temp_root / "blank_deleted_group_footer.frx",
                                              "blank_deleted_group_footer.frx",
                                              "deleted report",
                                              true);
    run_blank_footer_grouping_expression_json(temp_root / "blank_deleted_group_footer.lbx",
                                              "blank_deleted_group_footer.lbx",
                                              "deleted label",
                                              true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_nested_report_group_section_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_update = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_stable_nested_group_section_expression_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "2",
                "--property-name", "EXPR",
                "--property-value", "customer.state",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " nested group section expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " nested group section expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2682: nested report/label group section expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 2U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
               "#2682: nested report/label group section expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: nested group section expression update should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2682: nested label group section expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: nested group section expression update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: nested group section expression update should preserve section selection kind");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 5",
                        "#2682: nested group section expression update should preserve live section counts");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2682: nested group section expression update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: nested group section expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: nested group section expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: nested group section expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: nested group section expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2682: nested group section expression update should not fabricate deleted preview bounds");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_1\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"group_header_2\"",
                "\"expression\": \"customer.state\"",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"group_footer_4\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 4",
                "\"id\": \"group_footer_5\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: nested group section expression update should preserve nested sibling ordering and expressions");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_header_2\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.state\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 5",
                "\"top\": 400",
                "\"height\": 300",
                "\"bottom\": 700"
            },
            "#2682: nested group section expression update should refresh selected inner-group metadata");
    };

    run_nested_group_expression_update(temp_root / "nested_expression_update.frx",
                                       "nested_expression_update.frx",
                                       "report");
    run_nested_group_expression_update(temp_root / "nested_expression_update.lbx",
                                       "nested_expression_update.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_nested_report_group_section_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_clear = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_stable_nested_group_section_expression_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "2",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " nested group section expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " nested group section expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2682: nested report/label group section expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: nested group section expression clear should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2682: nested label group section expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: nested group section expression clear should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: nested group section expression clear should preserve section selection kind");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 5",
                        "#2682: nested group section expression clear should preserve live section counts");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2682: nested group section expression clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: nested group section expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: nested group section expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: nested group section expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: nested group section expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2682: nested group section expression clear should not fabricate deleted preview bounds");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_1\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"group_header_2\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"group_footer_4\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 4",
                "\"id\": \"group_footer_5\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: nested group section expression clear should preserve nested sibling ordering and expressions");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_header_2\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 5",
                "\"top\": 400",
                "\"height\": 300",
                "\"bottom\": 700"
            },
            "#2682: nested group section expression clear should refresh selected inner-group metadata");
    };

    run_nested_group_expression_clear(temp_root / "nested_expression_clear.frx",
                                      "nested_expression_clear.frx",
                                      "report");
    run_nested_group_expression_clear(temp_root / "nested_expression_clear.lbx",
                                      "nested_expression_clear.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_nested_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_nested_group_section_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_nested_group_expression_update = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "4",
                "--property-name", "EXPR",
                "--property-value", "customer.state",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted nested group section expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted nested group section expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2682: deleted nested report/label group section expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
               "#2682: deleted nested report/label group section expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: deleted nested group section expression update should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2682: deleted nested label group section expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: deleted nested group section expression update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: deleted nested group section expression update should preserve section selection kind");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 4",
                        "#2682: deleted nested group section expression update should preserve live section counts");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#2682: deleted nested group section expression update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: deleted nested group section expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: deleted nested group section expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: deleted nested group section expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                        "#2682: deleted nested group section expression update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                        "#2682: deleted nested group section expression update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#2682: deleted nested group section expression update should preserve deleted preview heights");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"group_footer_4\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.state\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 4",
                "\"deleted\": true"
            },
            "#2682: deleted nested group section expression update should refresh deleted-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_footer_4\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.state\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2900",
                "\"height\": 250",
                "\"bottom\": 3150"
            },
            "#2682: deleted nested group section expression update should refresh selected deleted-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_1\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"group_header_2\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"group_footer_5\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: deleted nested group section expression update should preserve unaffected live sibling expressions");
    };

    run_deleted_nested_group_expression_update(temp_root / "nested_deleted_expression_update.frx",
                                               "nested_deleted_expression_update.frx",
                                               "report");
    run_deleted_nested_group_expression_update(temp_root / "nested_deleted_expression_update.lbx",
                                               "nested_deleted_expression_update.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_nested_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_nested_group_section_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_nested_group_expression_clear = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "4",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted nested group section expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted nested group section expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2682: deleted nested report/label group section expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: deleted nested group section expression clear should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2682: deleted nested label group section expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: deleted nested group section expression clear should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: deleted nested group section expression clear should preserve section selection kind");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 4",
                        "#2682: deleted nested group section expression clear should preserve live section counts");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#2682: deleted nested group section expression clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: deleted nested group section expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: deleted nested group section expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: deleted nested group section expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                        "#2682: deleted nested group section expression clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                        "#2682: deleted nested group section expression clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#2682: deleted nested group section expression clear should preserve deleted preview heights");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"group_footer_4\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 4",
                "\"deleted\": true"
            },
            "#2682: deleted nested group section expression clear should refresh deleted-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group_footer_4\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2900",
                "\"height\": 250",
                "\"bottom\": 3150"
            },
            "#2682: deleted nested group section expression clear should refresh selected deleted-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"group_header_1\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"group_header_2\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"group_footer_5\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: deleted nested group section expression clear should preserve unaffected live sibling expressions");
    };

    run_deleted_nested_group_expression_clear(temp_root / "nested_deleted_expression_clear.frx",
                                              "nested_deleted_expression_clear.frx",
                                              "report");
    run_deleted_nested_group_expression_clear(temp_root / "nested_deleted_expression_clear.lbx",
                                              "nested_deleted_expression_clear.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_nested_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_stable_nested_group_section_expression_json(asset_path);

            const auto expect_selected_section_expression =
                [&](const ProcessResult& process,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2684: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2684: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 5",
                                    "#2684: " + operation_label + " should preserve live section counts");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                                    "#2684: " + operation_label + " should preserve deleted section counts");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2684: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2684: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                                    "#2684: " + operation_label + " should not fabricate deleted preview bounds");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2684: " + operation_label + " should preserve selected-section availability");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2684: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2684: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2684: " + operation_label + " should not select report settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2684: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2684: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2684: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"group_header_2\"",
                            "\"bandKind\": \"group_header\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": 2",
                            "\"deleted\": false",
                            "\"sectionIndex\": 1",
                            "\"sectionCount\": 5",
                            "\"top\": 400",
                            "\"height\": 300",
                            "\"bottom\": 700"
                        },
                        "#2684: " + operation_label + " should refresh selected nested section metadata");
                };

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "country-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.state",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable nested group section expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable nested group section expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#2684: stable-selected nested report/label group section expression update should exit successfully");
            const auto expr_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "country-header-guid",
                .property_name = "EXPR"
            });
            expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
                   "#2684: stable-selected nested report/label group section expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_process,
                                               "customer.state",
                                               "2",
                                               "6",
                                               "stable-selected nested group section expression update");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_header_2\"",
                    "\"expression\": \"customer.state\"",
                    "\"expressionMemoBlockNumber\": 6",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"group_footer_4\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 4",
                    "\"id\": \"group_footer_5\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected nested group section expression update should preserve unaffected nested sibling expressions");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "country-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable nested group section expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable nested group section expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#2684: stable-selected nested report/label group section expression clear should exit successfully");
            expect_selected_section_expression(clear_process,
                                               "",
                                               "null",
                                               "0",
                                               "stable-selected nested group section expression clear");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_header_2\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"group_footer_4\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 4",
                    "\"id\": \"group_footer_5\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected nested group section expression clear should preserve unaffected nested sibling expressions");
        };

    run_nested_group_expression_stable_edits(temp_root / "nested_stable_expression_update.frx",
                                             "nested_stable_expression_update.frx",
                                             "report");
    run_nested_group_expression_stable_edits(temp_root / "nested_stable_expression_update.lbx",
                                             "nested_stable_expression_update.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_nested_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_nested_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_nested_group_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(asset_path);

            const auto expect_selected_deleted_section_expression =
                [&](const ProcessResult& process,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2684: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2684: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 4",
                                    "#2684: " + operation_label + " should preserve live section counts");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                                    "#2684: " + operation_label + " should preserve deleted section counts");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2684: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2684: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#2684: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                                    "#2684: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                                    "#2684: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                                    "#2684: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2684: " + operation_label + " should preserve selected-section availability");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2684: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2684: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2684: " + operation_label + " should not select report settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2684: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2684: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2684: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"group_footer_4\"",
                            "\"bandKind\": \"group_footer\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": 4",
                            "\"deleted\": true",
                            "\"sectionIndex\": null",
                            "\"sectionCount\": 0",
                            "\"top\": 2900",
                            "\"height\": 250",
                            "\"bottom\": 3150"
                        },
                        "#2684: " + operation_label + " should refresh selected deleted nested section metadata");
                };

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "country-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.state",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted nested group section expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted nested group section expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#2684: stable-selected deleted nested report/label group section expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 4U),
                   "#2684: stable-selected deleted nested report/label group section expression update should preserve deleted state");
            const auto expr_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 4U,
                .object_name = {},
                .unique_id = "country-footer-guid",
                .property_name = "EXPR"
            });
            expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
                   "#2684: stable-selected deleted nested report/label group section expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_process,
                                                       "customer.state",
                                                       "2",
                                                       "6",
                                                       "stable-selected deleted nested group section expression update");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group_footer_4\"",
                    "\"expression\": \"customer.state\"",
                    "\"expressionMemoBlockNumber\": 6",
                    "\"recordIndex\": 4"
                },
                "#2684: stable-selected deleted nested group section expression update should refresh deleted section metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_header_2\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"group_footer_5\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected deleted nested group section expression update should preserve unaffected live sibling expressions");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "country-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted nested group section expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted nested group section expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#2684: stable-selected deleted nested report/label group section expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 4U),
                   "#2684: stable-selected deleted nested report/label group section expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_process,
                                                       "",
                                                       "null",
                                                       "0",
                                                       "stable-selected deleted nested group section expression clear");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group_footer_4\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 4"
                },
                "#2684: stable-selected deleted nested group section expression clear should refresh deleted section metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_header_2\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"group_footer_5\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected deleted nested group section expression clear should preserve unaffected live sibling expressions");
        };

    run_deleted_nested_group_expression_stable_edits(temp_root / "nested_deleted_stable_expression_update.frx",
                                                     "nested_deleted_stable_expression_update.frx",
                                                     "report");
    run_deleted_nested_group_expression_stable_edits(temp_root / "nested_deleted_stable_expression_update.lbx",
                                                     "nested_deleted_stable_expression_update.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_group_footer_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_footer_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_footer_expression_json = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_stable_group_section_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-footer-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable group footer expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable group footer expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1667: stable selected report/label group-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1667: stable selected group-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1667: stable selected label group-footer section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1667: stable selected group-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1667: stable selected group-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1667: stable selected group-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1667: stable selected group-footer section JSON should preserve sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1667: stable selected group-footer section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1929: stable selected group-footer section JSON should preserve preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#1929: stable selected group-footer section JSON should preserve preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#1929: stable selected group-footer section JSON should preserve preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1929: stable selected group-footer section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1667: stable selected group-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1667: stable selected group-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1667: stable selected group-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1667: stable selected group-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1667: stable selected group-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1667: stable selected group-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1667: stable selected group-footer section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1667: stable selected group-footer sections should expose selected expression metadata");
    };

    run_group_footer_expression_json(temp_root / "stable_group_footer.frx",
                                     "stable_group_footer.frx",
                                     "report");
    run_group_footer_expression_json(temp_root / "stable_group_footer.lbx",
                                     "stable_group_footer.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_json = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_stable_group_section_expression_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 1U),
               "#1668: stable deleted group-header fixture should mark the group-header section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-header-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1668: stable selected deleted report/label group-header section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1668: stable selected deleted group-header section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1668: stable selected deleted label group-header section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1668: stable selected deleted group-header sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1668: stable selected deleted group-header sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1668: stable selected deleted group-header sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1668: stable selected deleted group-header section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1668: stable selected deleted group-header section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 600",
                        "#1930: stable selected deleted group-header section JSON should refresh live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#1930: stable selected deleted group-header section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-header section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1668: stable selected deleted group-header sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1668: stable selected deleted group-header sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1668: stable selected deleted group-header sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1668: stable selected deleted group-header sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1668: stable selected deleted group-header sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1668: stable selected deleted group-header sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1668: stable selected deleted group-header section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1668: stable selected deleted group-header sections should expose selected expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"recordIndex\": 3"
            },
            "#1668: stable selected deleted group-header section JSON should preserve live sibling metadata");
    };

    run_deleted_group_expression_json(temp_root / "stable_deleted_group_header.frx",
                                      "stable_deleted_group_header.frx",
                                      "report");
    run_deleted_group_expression_json(temp_root / "stable_deleted_group_header.lbx",
                                      "stable_deleted_group_header.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_group_footer_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_json = [&](const fs::path& asset_path,
                                                              const std::string& title,
                                                              const std::string& label) {
        write_synthetic_report_table_for_stable_group_section_expression_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 3U),
               "#1669: stable deleted group-footer fixture should mark the group-footer section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-footer-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted group footer expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted group footer expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1669: stable selected deleted report/label group-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1669: stable selected deleted group-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1669: stable selected deleted label group-footer section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1669: stable selected deleted group-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1669: stable selected deleted group-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1669: stable selected deleted group-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1669: stable selected deleted group-footer section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1669: stable selected deleted group-footer section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#1930: stable selected deleted group-footer section JSON should refresh live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#1930: stable selected deleted group-footer section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-footer section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1669: stable selected deleted group-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1669: stable selected deleted group-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1669: stable selected deleted group-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1669: stable selected deleted group-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1669: stable selected deleted group-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1669: stable selected deleted group-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1669: stable selected deleted group-footer section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1669: stable selected deleted group-footer sections should expose selected expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1669: stable selected deleted group-footer section JSON should preserve live sibling metadata");
    };

    run_deleted_group_footer_expression_json(temp_root / "stable_deleted_group_footer.frx",
                                             "stable_deleted_group_footer.frx",
                                             "report");
    run_deleted_group_footer_expression_json(temp_root / "stable_deleted_group_footer.lbx",
                                             "stable_deleted_group_footer.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_stable_group_section_expression_json(asset_path);

            const auto expect_selected_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_id,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& section_index,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2683: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2683: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 3",
                                    "#2683: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                                    "#2683: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2683: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                    "#2683: " + operation_label + " should preserve live preview left bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2683: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                    "#2683: " + operation_label + " should preserve live preview right bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                                    "#2683: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                    "#2683: " + operation_label + " should preserve live preview widths");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                                    "#2683: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                                    "#2683: " + operation_label + " should not fabricate deleted preview bounds");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2683: " + operation_label + " should preserve section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2683: " + operation_label + " should preserve selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2683: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2683: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2683: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2683: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2683: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"" + section_id + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": false",
                            "\"sectionIndex\": " + section_index,
                            "\"sectionCount\": 3",
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#2683: " + operation_label + " should refresh selected-section expression metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#2683: stable-selected group-header expression update should exit successfully");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "group-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "customer.region",
                   "#2683: stable-selected group-header expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_header_process,
                                               "group_header_1",
                                               "group_header",
                                               "customer.region",
                                               "2",
                                               "4",
                                               "1",
                                               "0",
                                               "0",
                                               "600",
                                               "600",
                                               "stable-selected group-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-header expression update should preserve sibling footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#2683: stable-selected group-footer expression update should exit successfully");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 3U,
                .object_name = {},
                .unique_id = "group-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "customer.region",
                   "#2683: stable-selected group-footer expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_footer_process,
                                               "group_footer_3",
                                               "group_footer",
                                               "customer.region",
                                               "2",
                                               "5",
                                               "3",
                                               "2",
                                               "3600",
                                               "500",
                                               "4100",
                                               "stable-selected group-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-footer expression update should preserve sibling header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#2683: stable-selected group-header expression clear should exit successfully");
            expect_selected_section_expression(clear_header_process,
                                               "group_header_1",
                                               "group_header",
                                               "",
                                               "null",
                                               "0",
                                               "1",
                                               "0",
                                               "0",
                                               "600",
                                               "600",
                                               "stable-selected group-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-header expression clear should preserve sibling footer metadata");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#2683: stable-selected group-footer expression clear should exit successfully");
            expect_selected_section_expression(clear_footer_process,
                                               "group_footer_3",
                                               "group_footer",
                                               "",
                                               "null",
                                               "0",
                                               "3",
                                               "2",
                                               "3600",
                                               "500",
                                               "4100",
                                               "stable-selected group-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-footer expression clear should preserve sibling header metadata");
        };

    run_group_section_expression_stable_edits(temp_root / "group_section_stable_expression_edits.frx",
                                              "group_section_stable_expression_edits.frx",
                                              "report");
    run_group_section_expression_stable_edits(temp_root / "group_section_stable_expression_edits.lbx",
                                              "group_section_stable_expression_edits.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_stable_group_section_expression_json(asset_path);

            const auto delete_header_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_result.ok && dbf_record_deleted(asset_path, 1U),
                   "#2683: deleted group-header fixture should mark the group-header section deleted");
            const auto delete_footer_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_result.ok && dbf_record_deleted(asset_path, 3U),
                   "#2683: deleted group-footer fixture should mark the group-footer section deleted");

            const auto expect_selected_deleted_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_id,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2683: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2683: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 1",
                                    "#2683: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 2",
                                    "#2683: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2683: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 600",
                                    "#2683: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3600",
                                    "#2683: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3000",
                                    "#2683: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#2683: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                                    "#2683: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                                    "#2683: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 4100",
                                    "#2683: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2683: " + operation_label + " should preserve deleted section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2683: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2683: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2683: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2683: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2683: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2683: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"" + section_id + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"sectionIndex\": null",
                            "\"sectionCount\": 0",
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#2683: " + operation_label + " should refresh selected deleted-section expression metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#2683: stable-selected deleted group-header expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#2683: stable-selected deleted group-header expression update should preserve deleted state");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "group-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "customer.region",
                   "#2683: stable-selected deleted group-header expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_header_process,
                                                       "group_header_1",
                                                       "group_header",
                                                       "customer.region",
                                                       "2",
                                                       "4",
                                                       "1",
                                                       "0",
                                                       "600",
                                                       "600",
                                                       "stable-selected deleted group-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-header update should preserve deleted footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#2683: stable-selected deleted group-footer expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#2683: stable-selected deleted group-footer expression update should preserve deleted state");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 3U,
                .object_name = {},
                .unique_id = "group-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "customer.region",
                   "#2683: stable-selected deleted group-footer expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_footer_process,
                                                       "group_footer_3",
                                                       "group_footer",
                                                       "customer.region",
                                                       "2",
                                                       "5",
                                                       "3",
                                                       "3600",
                                                       "500",
                                                       "4100",
                                                       "stable-selected deleted group-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-footer update should preserve deleted header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#2683: stable-selected deleted group-header expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#2683: stable-selected deleted group-header expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_header_process,
                                                       "group_header_1",
                                                       "group_header",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "1",
                                                       "0",
                                                       "600",
                                                       "600",
                                                       "stable-selected deleted group-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-header clear should preserve deleted footer metadata");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#2683: stable-selected deleted group-footer expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#2683: stable-selected deleted group-footer expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_footer_process,
                                                       "group_footer_3",
                                                       "group_footer",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "3",
                                                       "3600",
                                                       "500",
                                                       "4100",
                                                       "stable-selected deleted group-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group_header_1\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"id\": \"group_footer_3\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-footer clear should preserve deleted header metadata");
        };

    run_deleted_group_section_expression_stable_edits(
        temp_root / "deleted_group_section_stable_expression_edits.frx",
        "deleted_group_section_stable_expression_edits.frx",
        "report");
    run_deleted_group_section_expression_stable_edits(
        temp_root / "deleted_group_section_stable_expression_edits.lbx",
        "deleted_group_section_stable_expression_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_group_section_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_section_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_expression_update = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_group_section_expression_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "1",
                "--property-name", "EXPR",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " group section expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " group section expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1567: report/label group section expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.region",
               "#1567: report/label group section expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1567: report/label group section expression update should return refreshed layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1567: report/label group section expression update should preserve section selection");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1567: report/label group section expression update should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2269: report/label group section expression update should preserve preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2269: report/label group section expression update should preserve preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2269: report/label group section expression update should preserve preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2269: report/label group section expression update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2269: report/label group section expression update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2269: report/label group section expression update should preserve preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2269: report/label group section expression update should preserve preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2269: report/label group section expression update should not fabricate deleted preview bounds");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 1",
                "\"top\": 0",
                "\"height\": 600",
                "\"bottom\": 600"
            },
            "#1567: report/label group section expression update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1567: report/label group section expression update should preserve sibling group expressions");
    };

    run_group_expression_update(temp_root / "expression_update.frx", "expression_update.frx", "report");
    run_group_expression_update(temp_root / "expression_update.lbx", "expression_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_group_section_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_section_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_expression_clear = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_group_section_expression_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "1",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " group section expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " group section expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1567: report/label group section expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1567: report/label group section expression clear should return refreshed layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1567: report/label group section expression clear should preserve section selection");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1567: report/label group section expression clear should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2269: report/label group section expression clear should preserve preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2269: report/label group section expression clear should preserve preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2269: report/label group section expression clear should preserve preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2269: report/label group section expression clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2269: report/label group section expression clear should preserve preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2269: report/label group section expression clear should preserve preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2269: report/label group section expression clear should preserve preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2269: report/label group section expression clear should not fabricate deleted preview bounds");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"top\": 0",
                "\"height\": 600",
                "\"bottom\": 600"
            },
            "#1567: report/label group section expression clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1567: report/label group section expression clear should preserve sibling group expressions");
    };

    run_group_expression_clear(temp_root / "expression_clear.frx", "expression_clear.frx", "report");
    run_group_expression_clear(temp_root / "expression_clear.lbx", "expression_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_group_footer_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_footer_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_footer_expression_update = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_group_section_expression_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " group footer expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " group footer expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1568: report/label group footer expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.region",
               "#1568: report/label group footer expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1568: report/label group footer expression update should return refreshed layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1568: report/label group footer expression update should preserve section selection");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1568: report/label group footer expression update should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2269: report/label group footer expression update should preserve preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2269: report/label group footer expression update should preserve preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2269: report/label group footer expression update should preserve preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2269: report/label group footer expression update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2269: report/label group footer expression update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2269: report/label group footer expression update should preserve preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2269: report/label group footer expression update should preserve preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2269: report/label group footer expression update should not fabricate deleted preview bounds");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 3",
                "\"top\": 3600",
                "\"height\": 500",
                "\"bottom\": 4100"
            },
            "#1568: report/label group footer expression update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1"
            },
            "#1568: report/label group footer expression update should preserve sibling group-header expressions");
    };

    run_group_footer_expression_update(temp_root / "expression_update.frx", "expression_update.frx", "report");
    run_group_footer_expression_update(temp_root / "expression_update.lbx", "expression_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_group_footer_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_footer_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_footer_expression_clear = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_group_section_expression_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " group footer expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " group footer expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1568: report/label group footer expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1568: report/label group footer expression clear should return refreshed layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1568: report/label group footer expression clear should preserve section selection");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1568: report/label group footer expression clear should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2269: report/label group footer expression clear should preserve preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2269: report/label group footer expression clear should preserve preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2269: report/label group footer expression clear should preserve preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2269: report/label group footer expression clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2269: report/label group footer expression clear should preserve preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2269: report/label group footer expression clear should preserve preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2269: report/label group footer expression clear should preserve preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2269: report/label group footer expression clear should not fabricate deleted preview bounds");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 3",
                "\"top\": 3600",
                "\"height\": 500",
                "\"bottom\": 4100"
            },
            "#1568: report/label group footer expression clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1"
            },
            "#1568: report/label group footer expression clear should preserve sibling group-header expressions");
    };

    run_group_footer_expression_clear(temp_root / "expression_clear.frx", "expression_clear.frx", "report");
    run_group_footer_expression_clear(temp_root / "expression_clear.lbx", "expression_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_json = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_deleted_group_section_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1569: deleted report/label group section expression JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1569: deleted report/label group section expression JSON should preserve document titles");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1569: deleted report/label group section expression JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1569: deleted report/label group section expression JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1569: deleted group section selections should expose selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1569: deleted group section selections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2271: deleted group section expression JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2271: deleted group section expression JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 600",
                        "#2271: deleted group section expression JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2271: deleted group section expression JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2271: deleted group section expression JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2271: deleted group section expression JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2271: deleted group section expression JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2271: deleted group section expression JSON should preserve deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#2271: deleted group section expression JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#2271: deleted group section expression JSON should preserve deleted preview heights");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1569: deleted report/label group section JSON should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1569: selected deleted group sections should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1569: deleted group section JSON should preserve live sibling section metadata");
    };

    run_deleted_group_expression_json(temp_root / "deleted_group.frx", "deleted_group.frx", "report");
    run_deleted_group_expression_json(temp_root / "deleted_group.lbx", "deleted_group.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_group_footer_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_json = [&](const fs::path& asset_path,
                                                              const std::string& title,
                                                              const std::string& label) {
        write_synthetic_report_table_for_deleted_group_footer_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group footer expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group footer expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1570: deleted report/label group footer expression JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1570: deleted report/label group footer expression JSON should preserve document titles");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1570: deleted report/label group footer expression JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1570: deleted report/label group footer expression JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1570: deleted group footer selections should expose selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1570: deleted group footer selections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2271: deleted group footer expression JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#2271: deleted group footer expression JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#2271: deleted group footer expression JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2271: deleted group footer expression JSON should preserve deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2271: deleted group footer expression JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#2271: deleted group footer expression JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2271: deleted group footer expression JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#2271: deleted group footer expression JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2271: deleted group footer expression JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#2271: deleted group footer expression JSON should preserve deleted preview heights");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1570: deleted report/label group footer JSON should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1570: selected deleted group footers should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1570: deleted group footer JSON should preserve live sibling section metadata");
    };

    run_deleted_group_footer_expression_json(temp_root / "deleted_group_footer.frx", "deleted_group_footer.frx", "report");
    run_deleted_group_footer_expression_json(temp_root / "deleted_group_footer.lbx", "deleted_group_footer.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_update = [&](const fs::path& asset_path,
                                                         const std::string& title,
                                                         const std::string& label) {
        write_synthetic_report_table_for_deleted_group_section_expression_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "1",
                "--property-name", "EXPR",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group section expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group section expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1571: deleted report/label group section expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.region",
               "#1571: deleted report/label group section expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1571: deleted report/label group section expression update should return refreshed layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1571: deleted group section expression updates should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1571: deleted group section expression updates should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group section expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group section expression update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 600",
                        "#2270: deleted group section expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group section expression update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2270: deleted group section expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group section expression update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2270: deleted group section expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group section expression update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#2270: deleted group section expression update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#2270: deleted group section expression update should preserve deleted preview heights");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1571: deleted report/label group section update should refresh deleted-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1571: deleted report/label group section update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1571: deleted group section expression update should preserve live sibling section metadata");
    };

    run_deleted_group_expression_update(temp_root / "deleted_group_update.frx", "deleted_group_update.frx", "report");
    run_deleted_group_expression_update(temp_root / "deleted_group_update.lbx", "deleted_group_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_clear = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_deleted_group_section_expression_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "1",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group section expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group section expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1572: deleted report/label group section expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1572: deleted report/label group section expression clear should return refreshed layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1572: deleted group section expression clears should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1572: deleted group section expression clears should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group section expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group section expression clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 600",
                        "#2270: deleted group section expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group section expression clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2270: deleted group section expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group section expression clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2270: deleted group section expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group section expression clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#2270: deleted group section expression clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#2270: deleted group section expression clear should preserve deleted preview heights");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1572: deleted report/label group section clear should refresh deleted-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1572: deleted report/label group section clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1572: deleted group section expression clear should preserve live sibling section metadata");
    };

    run_deleted_group_expression_clear(temp_root / "deleted_group_clear.frx", "deleted_group_clear.frx", "report");
    run_deleted_group_expression_clear(temp_root / "deleted_group_clear.lbx", "deleted_group_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_group_footer_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_update = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_synthetic_report_table_for_deleted_group_footer_expression_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group footer expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group footer expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1574: deleted report/label group footer expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.region",
               "#1574: deleted report/label group footer expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1574: deleted report/label group footer expression update should return refreshed layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1574: deleted group footer expression updates should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1574: deleted group footer expression updates should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group footer expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group footer expression update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2270: deleted group footer expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group footer expression update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#2270: deleted group footer expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group footer expression update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#2270: deleted group footer expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group footer expression update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group footer expression update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#2270: deleted group footer expression update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group footer expression update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#2270: deleted group footer expression update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group footer expression update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#2270: deleted group footer expression update should preserve deleted preview heights");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1574: deleted report/label group footer update should refresh deleted-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1574: deleted report/label group footer update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1574: deleted group footer expression update should preserve live sibling section metadata");
    };

    run_deleted_group_footer_expression_update(temp_root / "deleted_group_footer_update.frx",
                                               "deleted_group_footer_update.frx",
                                               "report");
    run_deleted_group_footer_expression_update(temp_root / "deleted_group_footer_update.lbx",
                                               "deleted_group_footer_update.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_group_footer_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_clear = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_deleted_group_footer_expression_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group footer expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group footer expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1573: deleted report/label group footer expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1573: deleted report/label group footer expression clear should return refreshed layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1573: deleted group footer expression clears should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1573: deleted group footer expression clears should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group footer expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#2270: deleted group footer expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#2270: deleted group footer expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group footer expression clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group footer expression clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#2270: deleted group footer expression clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group footer expression clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#2270: deleted group footer expression clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group footer expression clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#2270: deleted group footer expression clear should preserve deleted preview heights");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1573: deleted report/label group footer clear should refresh deleted-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1573: deleted report/label group footer clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1573: deleted group footer expression clear should preserve live sibling section metadata");
    };

    run_deleted_group_footer_expression_clear(temp_root / "deleted_group_footer_clear.frx", "deleted_group_footer_clear.frx", "report");
    run_deleted_group_footer_expression_clear(temp_root / "deleted_group_footer_clear.lbx", "deleted_group_footer_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_group_section_expressions(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_expression_json = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_group_section_expression_json(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1566: report/label group section expression JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1566: report/label group section expression JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2268: record-selected label group section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1566: report/label group section expression JSON should preserve group/detail/footer sections");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2268: record-selected group section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2268: record-selected group section JSON should expose live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2268: record-selected group section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2268: record-selected group section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2268: record-selected group section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2268: record-selected group section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2268: record-selected group section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2268: record-selected group section JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2268: record-selected group section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2268: record-selected group section JSON should expose selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#2268: record-selected group section JSON should expose report-selection availability");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1566: report/label group section expression JSON should expose full section expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1"
            },
            "#1566: selected report/label group sections should expose expression metadata");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1566: selected group sections should preserve section selection classification");
    };

    run_group_expression_json(temp_root / "group_sections.frx", "group_sections.frx", "report");
    run_group_expression_json(temp_root / "group_sections.lbx", "group_sections.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
