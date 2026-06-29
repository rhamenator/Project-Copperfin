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

void test_studio_host_json_exposes_selected_group_header_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_group_header_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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

void test_studio_host_json_exposes_selected_title_report_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_title_report_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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

void test_studio_host_json_exposes_report_title_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_title_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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

void test_studio_host_json_exposes_deleted_report_column_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_footer_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_column_footer_section_json = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_stable_column_section_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 3U),
               "#1681: stable deleted column-footer fixture should mark the column-footer section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-footer-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-footer section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-footer section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1681: stable selected deleted report/label column-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1681: stable selected deleted column-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1681: stable selected deleted column-footer label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1681: stable selected deleted column-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1681: stable selected deleted column-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1681: stable selected deleted column-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1681: stable selected deleted column-footer section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1681: stable selected deleted column-footer section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3050",
                        "#1940: stable selected deleted column-footer section JSON should refresh live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3050",
                        "#1940: stable selected deleted column-footer section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1940: stable selected deleted column-footer section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3050",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 3450",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 400",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1681: stable selected deleted column-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1681: stable selected deleted column-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1681: stable selected deleted column-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1681: stable selected deleted column-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1681: stable selected deleted column-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1681: stable selected deleted column-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"column_footer_3\"",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": true"
            },
            "#1681: stable selected deleted column-footer section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
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
            "#1681: stable selected deleted column-footer sections should expose selected section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1681: stable selected deleted column-footer section JSON should preserve live sibling metadata");
    };

    run_deleted_column_footer_section_json(temp_root / "stable_deleted_column_footer_sections.frx",
                                           "stable_deleted_column_footer_sections.frx",
                                           "report");
    run_deleted_column_footer_section_json(temp_root / "stable_deleted_column_footer_sections.lbx",
                                           "stable_deleted_column_footer_sections.lbx",
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

void test_studio_host_json_exposes_selected_page_header_report_objects_orphaned_by_deleted_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_orphaned_page_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_orphaned_page_header_object_selection = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto delete_section_result =
            copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
        expect(delete_section_result.ok && dbf_record_deleted(asset_path, 1U),
               "#1671: report/label orphaned page-header object fixture should mark the containing section deleted");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected orphaned page-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected orphaned page-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1671: stable selected orphaned page-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1671: stable selected orphaned page-header report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1671: stable selected orphaned page-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1671: stable orphaned page-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1671: stable orphaned page-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1671: stable orphaned page-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1671: stable orphaned page-header object selections should update live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1671: stable orphaned page-header object selections should expose deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1671: stable orphaned page-header object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"placedObjectCount\": 2",
                        "#2690: live objects inside deleted page-header sections should remain placed");
        expect_contains(object_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#2690: live objects inside deleted page-header sections should not be counted as unplaced");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1671: stable orphaned page-header object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1953: stable orphaned page-header object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1953: stable orphaned page-header object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2690: live deleted-section page-header objects should drop out of live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1953: stable orphaned page-header object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1953: stable orphaned page-header object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1953: stable orphaned page-header object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 6100",
                        "#2690: live deleted-section page-header objects should shrink live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1953: stable orphaned page-header object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                        "#2690: live deleted-section page-header objects should expand deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2700",
                        "#2690: live deleted-section page-header objects should expand deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2900",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1671: stable orphaned page-header object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1671: stable orphaned page-header object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1671: stable orphaned page-header object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1671: stable orphaned page-header object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2690: live objects inside deleted page-header sections should advertise containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCount\": 1"
            },
            "#2690: live objects inside deleted page-header sections should expose deleted containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"page_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Invoice\\\"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#2690: live objects inside deleted page-header sections should expose selected object metadata with deleted-section membership");
        expect_contains(object_process.stdout_text, "\"left\": 900",
                        "#1671: stable orphaned page-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1671: stable orphaned page-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1800",
                        "#1671: stable orphaned page-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2700",
                        "#1671: stable orphaned page-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 350",
                        "#1671: stable orphaned page-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 450",
                        "#1671: stable orphaned page-header object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"EXPR\", \"recordIndex\": 4, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 4, \"value\": \"\\\"Invoice\\\"\"",
                        "#1671: stable orphaned page-header object selections should expose selected-object expression provenance");
    };

    run_orphaned_page_header_object_selection(temp_root / "selected_orphaned_page_header_object_stable.frx",
                                              "selected_orphaned_page_header_object_stable.frx",
                                              "report");
    run_orphaned_page_header_object_selection(temp_root / "selected_orphaned_page_header_object_stable.lbx",
                                              "selected_orphaned_page_header_object_stable.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_detail_report_objects_orphaned_by_deleted_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_orphaned_detail_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_orphaned_detail_object_selection = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto delete_section_result =
            copperfin::vfp::set_record_deleted_flag(asset_path.string(), 2U, true);
        expect(delete_section_result.ok && dbf_record_deleted(asset_path, 2U),
               "#1672: report/label orphaned detail object fixture should mark the containing section deleted");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "field-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected orphaned detail object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected orphaned detail object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1672: stable selected orphaned detail report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1672: stable selected orphaned detail report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1672: stable selected orphaned detail label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1672: stable orphaned detail object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1672: stable orphaned detail object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1672: stable orphaned detail object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1672: stable orphaned detail object selections should update live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1672: stable orphaned detail object selections should expose deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1672: stable orphaned detail object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"placedObjectCount\": 2",
                        "#2690: live objects inside deleted detail sections should remain placed");
        expect_contains(object_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#2690: live objects inside deleted detail sections should not be counted as unplaced");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1672: stable orphaned detail object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1954: stable orphaned detail object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1954: stable orphaned detail object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1954: stable orphaned detail object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#2690: live deleted-section detail objects should drop out of live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1954: stable orphaned detail object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1954: stable orphaned detail object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1954: stable orphaned detail object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1954: stable orphaned detail object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                        "#2690: live deleted-section detail objects should expand deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2200",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1672: stable orphaned detail object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1672: stable orphaned detail object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1672: stable orphaned detail object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1672: stable orphaned detail object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2690: live objects inside deleted detail sections should advertise containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"objectCount\": 1"
            },
            "#2690: live objects inside deleted detail sections should expose deleted containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"expressionFieldIndex\": 2",
                "\"highlightCount\": 2"
            },
            "#2690: live objects inside deleted detail sections should expose selected object metadata with deleted-section membership");
        expect_contains(object_process.stdout_text, "\"left\": 1200",
                        "#1672: stable orphaned detail object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 2600",
                        "#1672: stable orphaned detail object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 4000",
                        "#1672: stable orphaned detail object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 5200",
                        "#1672: stable orphaned detail object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 450",
                        "#1672: stable orphaned detail object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3050",
                        "#1672: stable orphaned detail object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"FONTFACE\", \"recordIndex\": 3, \"fieldIndex\": 7, \"sourceLineIndex\": null, \"memoBlockNumber\": 3, \"value\": \"Segoe UI\"",
                        "#1672: stable orphaned detail object selections should expose selected-object field provenance");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1672: stable orphaned detail object selections should preserve sibling page-header metadata");
    };

    run_orphaned_detail_object_selection(temp_root / "selected_orphaned_detail_object_stable.frx",
                                         "selected_orphaned_detail_object_stable.frx",
                                         "report");
    run_orphaned_detail_object_selection(temp_root / "selected_orphaned_detail_object_stable.lbx",
                                         "selected_orphaned_detail_object_stable.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_detail_report_objects_orphaned_by_deleted_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_orphaned_detail_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_orphaned_detail_object_selection = [&](const fs::path& asset_path,
                                                                  const std::string& title,
                                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        const auto delete_section_result =
            copperfin::vfp::set_record_deleted_flag(asset_path.string(), 2U, true);
        expect(delete_section_result.ok && dbf_record_deleted(asset_path, 2U),
               "#1673: report/label deleted orphaned detail object fixture should mark the containing section deleted");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1673: report/label deleted orphaned detail object fixture should preserve deleted object state");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deleted-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted orphaned detail object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted orphaned detail object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1673: stable selected deleted orphaned detail report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1673: stable selected deleted orphaned detail report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1673: stable selected deleted orphaned detail label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1673: stable deleted orphaned detail object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1673: stable deleted orphaned detail object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1673: stable deleted orphaned detail object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1673: stable deleted orphaned detail object selections should update live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1673: stable deleted orphaned detail object selections should expose deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1673: stable deleted orphaned detail object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1673: stable deleted orphaned detail object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1955: stable deleted orphaned detail object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2200",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1673: stable deleted orphaned detail object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1673: stable deleted orphaned detail object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1673: stable deleted orphaned detail object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1673: stable deleted orphaned detail object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2689: deleted objects inside deleted detail sections should advertise containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2689: deleted objects inside deleted detail sections should expose deleted containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Deleted label\\\"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#2689: deleted objects inside deleted detail sections should expose selected object metadata with deleted-section membership");
        expect_contains(object_process.stdout_text, "\"left\": 1000",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 2600",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1200",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2200",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 300",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 2900",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"EXPR\", \"recordIndex\": 6, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 5, \"value\": \"\\\"Deleted label\\\"\"",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object expression provenance");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1673: stable deleted orphaned detail object selections should preserve sibling page-header metadata");
    };

    run_deleted_orphaned_detail_object_selection(
        temp_root / "selected_deleted_orphaned_detail_object_stable.frx",
        "selected_deleted_orphaned_detail_object_stable.frx",
        "report");
    run_deleted_orphaned_detail_object_selection(
        temp_root / "selected_deleted_orphaned_detail_object_stable.lbx",
        "selected_deleted_orphaned_detail_object_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
