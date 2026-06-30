#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_deleted_object_counts_per_section(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_section_deleted_object_count_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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

void test_studio_host_json_exposes_selected_report_sections(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_report_section_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host selected report section stdout:\n" << section_process.stdout_text << "\n";
        std::cerr << "studio host selected report section stderr:\n" << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1453: selected report section JSON smoke should exit successfully");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1453: report section selections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1457: report section selections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1457: report section selections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSection\": {",
                    "#1453: report section selections should expose selected-section JSON");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1960: selected report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1960: selected report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1960: selected report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1960: selected report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1960: selected report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1960: selected report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1960: selected report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1960: selected report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1960: selected report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1960: selected report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1960: selected report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1960: selected report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1960: selected report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1960: selected report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1512: selected report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1512: selected report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1512: selected report sections should not advertise containing-object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1512: selected report sections should serialize null containing-object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1512: selected report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1512: selected report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"id\": \"page_header_1\"",
                    "#1453: selected report section JSON should expose the selected section id");
    expect_contains(section_process.stdout_text, "\"bandKind\": \"page_header\"",
                    "#1453: selected report section JSON should expose the selected band kind");
    expect_contains(section_process.stdout_text, "\"recordIndex\": 1",
                    "#1453: selected report section JSON should expose the selected section record index");
    expect_contains(section_process.stdout_text, "\"sectionIndex\": 0",
                    "#1460: selected report section JSON should expose section order");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1460: selected report section JSON should expose live section counts");
    expect_contains(section_process.stdout_text, "\"objectCode\": 1",
                    "#1453: selected report section JSON should preserve raw section object codes");
    expect_contains(section_process.stdout_text, "\"bottom\": 2000",
                    "#1461: selected report section JSON should expose section bottom-edge coordinates");
    expect_contains(section_process.stdout_text, "\"objectCount\": 1",
                    "#1453: selected report section JSON should preserve selected section object counts");
    expect_contains(section_process.stdout_text, "\"objectKind\": \"label\"",
                    "#1453: selected report section JSON should include selected section objects");

    const fs::path deleted_section_path = temp_root / "deleted_section.frx";
    write_synthetic_report_table_for_deleted_section_json(deleted_section_path);
    const auto deleted_section_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_section_path.string(), "--record", "1", "--json"},
        temp_root);

    if (deleted_section_process.exit_code != 0) {
        std::cerr << "studio host selected deleted report section stdout:\n"
                  << deleted_section_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted report section stderr:\n"
                  << deleted_section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_section_process.exit_code == 0,
           "#1478: selected deleted report section JSON should exit successfully");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1478: deleted report section selections should advertise selected-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1478: deleted report section selections should advertise report-selection availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1478: deleted report section selections should expose section selection kind");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1513: selected deleted report sections should not advertise selected-object availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1513: selected deleted report sections should serialize null selected objects");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1513: selected deleted report sections should not advertise containing-object-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1513: selected deleted report sections should serialize null containing-object sections");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1513: selected deleted report sections should not advertise selected-settings availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1513: selected deleted report sections should serialize null selected settings");
    expect_contains(deleted_section_process.stdout_text, "\"sectionCount\": 0",
                    "#1478: deleted selected report section JSON should not expose live sections");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1478: deleted selected report section JSON should expose deleted section counts");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1961: deleted selected report section JSON should preserve live preview availability");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsLeft\": 100",
                    "#1961: deleted selected report section JSON should preserve remaining live preview left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsTop\": 2600",
                    "#1961: deleted selected report section JSON should preserve remaining live preview top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsRight\": 150",
                    "#1961: deleted selected report section JSON should preserve remaining live preview right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsBottom\": 2800",
                    "#1961: deleted selected report section JSON should preserve remaining live preview bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsWidth\": 50",
                    "#1961: deleted selected report section JSON should preserve remaining live preview widths");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsHeight\": 200",
                    "#1961: deleted selected report section JSON should preserve remaining live preview heights");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1524: deleted selected report section JSON should expose deleted preview bounds availability");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1524: deleted selected report section JSON should expose deleted section preview left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                    "#1524: deleted selected report section JSON should expose deleted section preview top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1524: deleted selected report section JSON should expose deleted section preview right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                    "#1524: deleted selected report section JSON should expose deleted section preview bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1524: deleted selected report section JSON should expose deleted section preview width");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                    "#1524: deleted selected report section JSON should expose deleted section preview height");
    expect_contains(deleted_section_process.stdout_text, "\"placedObjectCount\": 0",
                    "#1522: deleted selected report section JSON should not fabricate placed object counts");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPlacedObjectCount\": 0",
                    "#1523: deleted selected report section JSON should not fabricate deleted placed object counts");
    expect_contains(deleted_section_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1523: deleted selected report section JSON should not fabricate deleted unplaced object counts");
    expect_contains(deleted_section_process.stdout_text, "\"sectionKindCount\": 0",
                    "#1520: deleted selected report section JSON should not fabricate live section band-kind buckets");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionKindCount\": 1",
                    "#1520: deleted selected report section JSON should summarize deleted section band-kind buckets");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"detail\", \"count\": 1}\n      ]",
                    "#1520: deleted selected report section JSON should count deleted detail sections");
    expect_contains(deleted_section_process.stdout_text, "\"sectionHeightTotal\": 0",
                    "#1521: deleted selected report section JSON should not fabricate live section heights");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionHeightTotal\": 5000",
                    "#1521: deleted selected report section JSON should summarize deleted section heights");
    expect_contains_in_order(
        deleted_section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0"
        },
        "#1478: deleted report section selections should expose deleted selected-section metadata");

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host selected report object stdout:\n" << object_process.stdout_text << "\n";
        std::cerr << "studio host selected report object stderr:\n" << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1453: selected report object JSON smoke should exit successfully");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1453: non-section report selections should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1453: non-section report selections should serialize null selected sections");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
}  // namespace cf_test_studio_host_json
