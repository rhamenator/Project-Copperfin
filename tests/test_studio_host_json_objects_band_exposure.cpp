#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_selected_report_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_report_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

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
           "#1454: selected report object JSON smoke should exit successfully");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1454: report object selections should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1457: report object selections should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1457: report object selections should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"selectedReportObject\": {",
                    "#1454: report object selections should expose selected-object JSON");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1964: selected report object JSON should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1964: selected report object JSON should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1964: selected report object JSON should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1964: selected report object JSON should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1964: selected report object JSON should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1964: selected report object JSON should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1964: selected report object JSON should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1964: selected report object JSON should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1964: selected report object JSON should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1964: selected report object JSON should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1964: selected report object JSON should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1964: selected report object JSON should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1964: selected report object JSON should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1964: selected report object JSON should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1509: selected report objects should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1509: selected report objects should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1509: selected report objects should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1509: selected report objects should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"recordIndex\": 3",
                    "#1454: selected report object JSON should expose the selected object record index");
    expect_contains(object_process.stdout_text, "\"objectTypeCode\": 8",
                    "#1454: selected report object JSON should expose raw report object type codes");
    expect_contains(object_process.stdout_text, "\"objectKind\": \"field\"",
                    "#1454: selected report object JSON should expose report object kind");
    expect_contains(object_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1458: selected report object JSON should expose containing section ids");
    expect_contains(object_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1458: selected report object JSON should expose containing section record indexes");
    expect_contains(object_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1458: selected report object JSON should expose top coordinates relative to containing sections");
    expect_contains(object_process.stdout_text, "\"sectionRelativeBottom\": 1050",
                    "#1461: selected report object JSON should expose bottom coordinates relative to containing sections");
    expect_contains(object_process.stdout_text, "\"sectionObjectIndex\": 0",
                    "#1459: selected report object JSON should expose object order inside containing sections");
    expect_contains(object_process.stdout_text, "\"sectionObjectCount\": 1",
                    "#1459: selected report object JSON should expose containing section object counts");
    expect_contains(object_process.stdout_text, "\"expression\": \"customer.company\"",
                    "#1454: selected report object JSON should expose report object expressions");
    expect_contains(object_process.stdout_text, "\"expressionFieldIndex\": 2",
                    "#1454: selected report object JSON should expose expression field provenance");
    expect_contains(object_process.stdout_text, "\"right\": 5200",
                    "#1462: selected report object JSON should expose object right-edge coordinates");
    expect_contains(object_process.stdout_text, "\"bottom\": 3050",
                    "#1461: selected report object JSON should expose object bottom-edge coordinates");
    expect_contains(object_process.stdout_text, "\"highlightCount\": 1",
                    "#1454: selected report object JSON should expose highlight counts");
    expect_contains(object_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3, \"fieldIndex\": 7, \"sourceLineIndex\": null, \"memoBlockNumber\": 3, \"value\": \"Segoe UI\"",
                    "#1454: selected report object JSON should expose highlight provenance");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1455: section-contained report objects should advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1455: section-contained report objects should expose containing-section JSON");
    expect_contains(object_process.stdout_text, "\"id\": \"detail_2\"",
                    "#1455: containing-section JSON should expose selected object section ids");
    expect_contains(object_process.stdout_text, "\"bandKind\": \"detail\"",
                    "#1455: containing-section JSON should expose selected object band kinds");

    const auto page_header_object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "4", "--json"},
        temp_root);

    if (page_header_object_process.exit_code != 0) {
        std::cerr << "studio host selected page-header report object stdout:\n"
                  << page_header_object_process.stdout_text << "\n";
        std::cerr << "studio host selected page-header report object stderr:\n"
                  << page_header_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(page_header_object_process.exit_code == 0,
           "#1972: selected page-header report object JSON should exit successfully");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1972: page-header report object selections should advertise selected-object availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1972: page-header report object selections should advertise report-selection availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1972: page-header report object selections should expose object selection kind");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1972: selected page-header report object JSON should expose live preview availability");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1972: selected page-header report object JSON should preserve live preview left bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1972: selected page-header report object JSON should preserve live preview top bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1972: selected page-header report object JSON should preserve live preview right bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1972: selected page-header report object JSON should preserve live preview bottom bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1972: selected page-header report object JSON should preserve live preview widths");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1972: selected page-header report object JSON should preserve live preview heights");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1972: selected page-header report object JSON should expose deleted preview availability");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1972: selected page-header report object JSON should preserve deleted preview left bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1972: selected page-header report object JSON should preserve deleted preview top bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1972: selected page-header report object JSON should preserve deleted preview right bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1972: selected page-header report object JSON should preserve deleted preview bottom bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1972: selected page-header report object JSON should preserve deleted preview widths");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1972: selected page-header report object JSON should preserve deleted preview heights");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1972: selected page-header report objects should not advertise selected-section availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1972: selected page-header report objects should serialize null selected sections");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1972: selected page-header report objects should not advertise selected-settings availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1972: selected page-header report objects should serialize null selected settings");
    expect_contains_in_order(
        page_header_object_process.stdout_text,
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
            "\"expression\": \"\\\"Invoice\\\"\""
        },
        "#1972: page-header report object selections should expose selected-object metadata");
    expect_contains(page_header_object_process.stdout_text, "\"right\": 2700",
                    "#1972: selected page-header report object JSON should expose object right-edge coordinates");
    expect_contains(page_header_object_process.stdout_text, "\"bottom\": 450",
                    "#1972: selected page-header report object JSON should expose object bottom-edge coordinates");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1972: page-header report objects should advertise containing-section availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1972: page-header report objects should expose containing-section JSON");
    expect_contains(page_header_object_process.stdout_text, "\"id\": \"page_header_1\"",
                    "#1972: containing-section JSON should expose selected page-header object section ids");
    expect_contains(page_header_object_process.stdout_text, "\"bandKind\": \"page_header\"",
                    "#1972: containing-section JSON should expose selected page-header object band kinds");

    const fs::path deleted_page_header_object_path = temp_root / "deleted_page_header_object.frx";
    write_synthetic_report_table_for_layout_json(deleted_page_header_object_path);
    const auto delete_page_header_object_result =
        copperfin::vfp::set_record_deleted_flag(deleted_page_header_object_path.string(), 4U, true);
    expect(delete_page_header_object_result.ok,
           "#1974: selected deleted page-header report object fixture should mark the page-header object deleted");

    const auto deleted_page_header_object_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_page_header_object_path.string(), "--record", "4", "--json"},
        temp_root);

    if (deleted_page_header_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted page-header report object stdout:\n"
                  << deleted_page_header_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted page-header report object stderr:\n"
                  << deleted_page_header_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_page_header_object_process.exit_code == 0,
           "#1974: selected deleted page-header report object JSON should exit successfully");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1974: deleted page-header report object selections should advertise selected-object availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1974: deleted page-header report object selections should advertise report-selection availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1974: deleted page-header report object selections should expose object selection kind");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1974: selected deleted page-header report object JSON should preserve live preview availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1974: selected deleted page-header report object JSON should preserve live preview left bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1974: selected deleted page-header report object JSON should preserve live preview top bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1974: selected deleted page-header report object JSON should preserve live preview right bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1974: selected deleted page-header report object JSON should preserve live preview bottom bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1974: selected deleted page-header report object JSON should preserve live preview widths");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1974: selected deleted page-header report object JSON should preserve live preview heights");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1974: selected deleted page-header report object JSON should expose deleted preview availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview left bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview top bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview right bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1974: selected deleted page-header report object JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1800",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview widths");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2800",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview heights");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1974: selected deleted page-header report objects should not advertise selected-section availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1974: selected deleted page-header report objects should serialize null selected sections");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1974: selected deleted page-header report objects should not advertise selected-settings availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1974: selected deleted page-header report objects should serialize null selected settings");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"liveObjectCount\": 2",
                    "#1974: selected deleted page-header report object JSON should summarize remaining live objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1974: selected deleted page-header report object JSON should summarize deleted objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPlacedObjectCount\": 2",
                    "#1974: selected deleted page-header report object JSON should count deleted placed objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1974: selected deleted page-header report object JSON should not fabricate deleted unplaced objects");
    expect_contains_in_order(
        deleted_page_header_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": true",
            "\"containingSectionId\": \"page_header_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 450",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Invoice\\\"\""
        },
        "#1974: deleted page-header report object selections should expose selected-object metadata with containing-section membership");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"right\": 2700",
                    "#1974: selected deleted page-header report object JSON should expose object right-edge coordinates");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"bottom\": 450",
                    "#1974: selected deleted page-header report object JSON should expose object bottom-edge coordinates");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1974: deleted page-header report objects should advertise selected containing-section availability");
    expect_contains_in_order(
        deleted_page_header_object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false"
        },
        "#1974: deleted page-header report objects should expose live containing-section JSON");

    const auto deleted_object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "6", "--json"},
        temp_root);

    if (deleted_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted report object stdout:\n"
                  << deleted_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted report object stderr:\n"
                  << deleted_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_object_process.exit_code == 0,
           "#1479: selected deleted report object JSON should exit successfully");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1479: deleted report object selections should advertise selected-object availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1479: deleted report object selections should advertise report-selection availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1479: deleted report object selections should expose object selection kind");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1965: selected deleted report object JSON should preserve live preview availability");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1965: selected deleted report object JSON should preserve live preview left bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1965: selected deleted report object JSON should preserve live preview top bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1965: selected deleted report object JSON should preserve live preview right bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1965: selected deleted report object JSON should preserve live preview bottom bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1965: selected deleted report object JSON should preserve live preview widths");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1965: selected deleted report object JSON should preserve live preview heights");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1965: selected deleted report object JSON should expose deleted preview availability");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1965: selected deleted report object JSON should preserve deleted preview left bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1965: selected deleted report object JSON should preserve deleted preview top bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1965: selected deleted report object JSON should preserve deleted preview right bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1965: selected deleted report object JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1965: selected deleted report object JSON should preserve deleted preview widths");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1965: selected deleted report object JSON should preserve deleted preview heights");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1510: selected deleted report objects should not advertise selected-section availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1510: selected deleted report objects should serialize null selected sections");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1510: selected deleted report objects should not advertise selected-settings availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1510: selected deleted report objects should serialize null selected settings");
    expect_contains(deleted_object_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1479: deleted selected report object JSON should expose deleted object counts");
    expect_contains_in_order(
        deleted_object_process.stdout_text,
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
            "\"expression\": \"\\\"Deleted label\\\"\""
        },
        "#1479: deleted report object selections should expose deleted selected-object metadata");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1479: deleted report objects should advertise selected containing-section availability");
    expect_contains_in_order(
        deleted_object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"detail_2\"",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"deleted\": false"
        },
        "#1479: deleted report objects should expose live containing-section JSON");

    const auto unplaced_object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "5", "--json"},
        temp_root);

    if (unplaced_object_process.exit_code != 0) {
        std::cerr << "studio host selected unplaced report object stdout:\n"
                  << unplaced_object_process.stdout_text << "\n";
        std::cerr << "studio host selected unplaced report object stderr:\n"
                  << unplaced_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(unplaced_object_process.exit_code == 0,
           "#1480: selected unplaced report object JSON should exit successfully");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1480: unplaced report object selections should advertise selected-object availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1480: unplaced report object selections should advertise report-selection availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1480: unplaced report object selections should expose object selection kind");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1968: selected unplaced report object JSON should expose live preview availability");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1968: selected unplaced report object JSON should preserve live preview left bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1968: selected unplaced report object JSON should preserve live preview top bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1968: selected unplaced report object JSON should preserve live preview right bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1968: selected unplaced report object JSON should preserve live preview bottom bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1968: selected unplaced report object JSON should preserve live preview widths");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1968: selected unplaced report object JSON should preserve live preview heights");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1968: selected unplaced report object JSON should expose deleted preview availability");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1968: selected unplaced report object JSON should preserve deleted preview left bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1968: selected unplaced report object JSON should preserve deleted preview top bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1968: selected unplaced report object JSON should preserve deleted preview right bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1968: selected unplaced report object JSON should preserve deleted preview bottom bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1968: selected unplaced report object JSON should preserve deleted preview widths");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1968: selected unplaced report object JSON should preserve deleted preview heights");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1511: selected unplaced report objects should not advertise selected-section availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1511: selected unplaced report objects should serialize null selected sections");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1511: selected unplaced report objects should not advertise selected-settings availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1511: selected unplaced report objects should serialize null selected settings");
    expect_contains(unplaced_object_process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1480: unplaced selected report object JSON should expose unplaced object counts");
    expect_contains_in_order(
        unplaced_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 5",
            "\"deleted\": false",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionObjectIndex\": null",
            "\"sectionObjectCount\": 0",
            "\"objectKind\": \"line\""
        },
        "#1480: unplaced report object selections should expose selected-object metadata without section membership");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1480: unplaced report objects should not advertise selected containing-section availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1480: unplaced report objects should serialize null selected containing-section JSON");

    const fs::path deleted_unplaced_object_path = temp_root / "deleted_unplaced_object.frx";
    write_synthetic_report_table_for_layout_json(deleted_unplaced_object_path);
    const auto delete_unplaced_object_result =
        copperfin::vfp::set_record_deleted_flag(deleted_unplaced_object_path.string(), 5U, true);
    expect(delete_unplaced_object_result.ok,
           "#1970: selected deleted unplaced report object fixture should mark the unplaced object deleted");

    const auto deleted_unplaced_object_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_unplaced_object_path.string(), "--record", "5", "--json"},
        temp_root);

    if (deleted_unplaced_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted unplaced report object stdout:\n"
                  << deleted_unplaced_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted unplaced report object stderr:\n"
                  << deleted_unplaced_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_unplaced_object_process.exit_code == 0,
           "#1970: selected deleted unplaced report object JSON should exit successfully");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1970: deleted unplaced report object selections should advertise selected-object availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1970: deleted unplaced report object selections should advertise report-selection availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1970: deleted unplaced report object selections should expose object selection kind");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    "#1970: selected deleted unplaced report object JSON should preserve remaining live preview bottom bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview widths");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsHeight\": 7000",
                    "#1970: selected deleted unplaced report object JSON should preserve remaining live preview heights");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1970: selected deleted unplaced report object JSON should expose deleted preview availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 50",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1970: selected deleted unplaced report object JSON should preserve deleted preview top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1970: selected deleted unplaced report object JSON should preserve deleted preview right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 8100",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview bottom bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2150",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview widths");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5500",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview heights");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1970: selected deleted unplaced report objects should not advertise selected-section availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1970: selected deleted unplaced report objects should serialize null selected sections");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1970: selected deleted unplaced report objects should not advertise selected-settings availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1970: selected deleted unplaced report objects should serialize null selected settings");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"liveObjectCount\": 2",
                    "#1970: selected deleted unplaced report object JSON should summarize remaining live objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1970: selected deleted unplaced report object JSON should summarize deleted objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                    "#1970: selected deleted unplaced report object JSON should retain deleted placed object counts");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                    "#1970: selected deleted unplaced report object JSON should count deleted unplaced objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"left\": 50",
                    "#1970: deleted unplaced report object selections should expose selected-object left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"top\": 8000",
                    "#1970: deleted unplaced report object selections should expose selected-object top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"right\": 150",
                    "#1970: deleted unplaced report object selections should expose selected-object right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"bottom\": 8100",
                    "#1970: deleted unplaced report object selections should expose selected-object bottom bounds");
    expect_contains_in_order(
        deleted_unplaced_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 5",
            "\"deleted\": true",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionObjectIndex\": null",
            "\"sectionObjectCount\": 0",
            "\"objectKind\": \"line\""
        },
        "#1970: deleted unplaced report object selections should expose selected-object metadata without section membership");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1970: deleted unplaced report objects should not advertise selected containing-section availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1970: deleted unplaced report objects should serialize null selected containing-section JSON");

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
           "#1454: selected report section JSON smoke should exit successfully");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1454: non-object report selections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1454: non-object report selections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1455: non-object report selections should not advertise containing-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1455: non-object report selections should serialize null containing sections");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}




void test_studio_host_json_exposes_selected_page_header_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_page_header_object_selection = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected page-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected page-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1663: stable selected page-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1663: stable selected page-header report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1663: stable selected page-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1663: stable page-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1663: stable page-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1663: stable page-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1663: stable page-header object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1663: stable page-header object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1951: stable selected page-header object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1951: stable selected page-header object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1951: stable selected page-header object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1951: stable selected page-header object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1951: stable selected page-header object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1951: stable selected page-header object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1951: stable selected page-header object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1951: stable selected page-header object JSON should preserve deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1951: stable selected page-header object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1951: stable selected page-header object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1951: stable selected page-header object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1951: stable selected page-header object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1951: stable selected page-header object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1951: stable selected page-header object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1663: stable page-header object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1663: stable page-header object selections should expose containing-section JSON");
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
                "\"expressionFieldIndex\": 2",
                "\"highlightCount\": 1"
            },
            "#1663: stable page-header object selections should expose selected object metadata and provenance");
        expect_contains(object_process.stdout_text, "\"left\": 900",
                        "#1663: stable page-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1663: stable page-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1800",
                        "#1663: stable page-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2700",
                        "#1663: stable page-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 350",
                        "#1663: stable page-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 450",
                        "#1663: stable page-header object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"EXPR\", \"recordIndex\": 4, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 4, \"value\": \"\\\"Invoice\\\"\"",
                        "#1663: stable page-header object selections should expose selected-object expression provenance");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1663: stable page-header object selections should expose containing page-header metadata");
    };

    run_page_header_object_selection(temp_root / "selected_page_header_object_stable.frx",
                                     "selected_page_header_object_stable.frx",
                                     "report");
    run_page_header_object_selection(temp_root / "selected_page_header_object_stable.lbx",
                                     "selected_page_header_object_stable.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_page_header_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_page_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_page_header_object_selection = [&](const fs::path& asset_path,
                                                              const std::string& title,
                                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 4U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 4U),
               "#1670: report/label deleted page-header object stable selection fixture should mark the object deleted");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted page-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted page-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1670: stable selected deleted page-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1670: stable selected deleted page-header report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1670: stable selected deleted page-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1670: stable deleted page-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1670: stable deleted page-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1670: stable deleted page-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1670: stable deleted page-header object selections should update live object counts");
        expect_contains(object_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1670: stable deleted page-header object selections should update live placed object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1670: stable deleted page-header object selections should update deleted object counts");
        expect_contains(object_process.stdout_text, "\"deletedPlacedObjectCount\": 2",
                        "#1670: stable deleted page-header object selections should expose deleted placed object counts");
        expect_contains(object_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                        "#1670: stable deleted page-header object selections should preserve deleted unplaced object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1952: stable selected deleted page-header object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1952: stable selected deleted page-header object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1952: stable selected deleted page-header object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1952: stable selected deleted page-header object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1952: stable selected deleted page-header object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1952: stable selected deleted page-header object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1952: stable selected deleted page-header object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1952: stable selected deleted page-header object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                        "#1952: stable selected deleted page-header object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                        "#1952: stable selected deleted page-header object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                        "#1952: stable selected deleted page-header object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1952: stable selected deleted page-header object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1800",
                        "#1952: stable selected deleted page-header object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2800",
                        "#1952: stable selected deleted page-header object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1670: stable deleted page-header object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1670: stable deleted page-header object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1670: stable deleted page-header object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1670: stable deleted page-header object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1670: stable deleted page-header object selections should advertise containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1670: stable deleted page-header object selections should expose live containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
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
            "#1670: stable deleted page-header object selections should expose selected object metadata with containing-section membership");
        expect_contains(object_process.stdout_text, "\"left\": 900",
                        "#1670: stable deleted page-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1670: stable deleted page-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1800",
                        "#1670: stable deleted page-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2700",
                        "#1670: stable deleted page-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 350",
                        "#1670: stable deleted page-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 450",
                        "#1670: stable deleted page-header object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"EXPR\", \"recordIndex\": 4, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 4, \"value\": \"\\\"Invoice\\\"\"",
                        "#1670: stable deleted page-header object selections should expose selected-object expression provenance");
    };

    run_deleted_page_header_object_selection(temp_root / "selected_deleted_page_header_object_stable.frx",
                                             "selected_deleted_page_header_object_stable.frx",
                                             "report");
    run_deleted_page_header_object_selection(temp_root / "selected_deleted_page_header_object_stable.lbx",
                                             "selected_deleted_page_header_object_stable.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
