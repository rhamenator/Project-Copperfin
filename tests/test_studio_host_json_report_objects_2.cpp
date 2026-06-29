#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_selected_deleted_summary_report_objects_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_summary_report_objects_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_deleted_summary_object_record.frx";
    write_synthetic_report_table_for_deleted_summary_object_json(report_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted summary report object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted summary report object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1978: record-selected deleted summary report object JSON should exit successfully");
    expect_contains(object_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_summary_object_record.frx\"",
                    "#1978: record-selected deleted summary report object JSON should preserve document titles");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1978: record-selected deleted summary report object selections should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1978: record-selected deleted summary report object selections should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1978: record-selected deleted summary report object selections should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1978: record-selected deleted summary report object JSON should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1978: record-selected deleted summary report object JSON should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1978: record-selected deleted summary report object JSON should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1978: record-selected deleted summary report object JSON should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3900",
                    "#1978: record-selected deleted summary report object JSON should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1978: record-selected deleted summary report object JSON should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3900",
                    "#1978: record-selected deleted summary report object JSON should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1978: record-selected deleted summary report object JSON should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 400",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 3300",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1900",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3550",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1500",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1978: record-selected deleted summary report objects should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1978: record-selected deleted summary report objects should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1978: record-selected deleted summary report objects should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1978: record-selected deleted summary report objects should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                    "#1978: record-selected deleted summary report object JSON should preserve live section counts");
    expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted section counts");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                    "#1978: record-selected deleted summary report object JSON should clear live object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1978: record-selected deleted summary report object JSON should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1978: record-selected deleted summary report objects should not advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1978: record-selected deleted summary report objects should serialize null containing sections");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionRelativeTop\": 0",
            "\"sectionRelativeBottom\": 0",
            "\"sectionObjectIndex\": null",
            "\"sectionObjectCount\": 0",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Summary label\\\"\""
        },
        "#1978: record-selected deleted summary report object selections should expose selected deleted-object metadata");
    expect_contains(object_process.stdout_text, "\"left\": 400",
                    "#1978: record-selected deleted summary report object selections should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 3300",
                    "#1978: record-selected deleted summary report object selections should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"right\": 1900",
                    "#1978: record-selected deleted summary report object selections should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"bottom\": 3550",
                    "#1978: record-selected deleted summary report object selections should expose selected-object bottom bounds");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_group_header_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_group_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_group_header_object_selection = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_stable_group_header_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-header-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected group-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected group-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1690: stable selected group-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1690: stable selected group-header object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1690: stable selected group-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2068: stable selected group-header report/label object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1700",
                        "#2068: stable selected group-header report/label object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2068: stable selected group-header report/label object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1700",
                        "#2068: stable selected group-header report/label object JSON should preserve live preview width");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2068: stable selected group-header report/label object JSON should preserve live preview height");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2068: stable selected group-header report/label object JSON should not fabricate deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve zero deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve zero deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve zero deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve zero deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve zero deleted preview width");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2068: stable selected group-header report/label object JSON should preserve zero deleted preview height");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1690: stable group-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1690: stable group-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1690: stable group-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1690: stable group-header object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1690: stable group-header object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1690: stable group-header object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1690: stable group-header object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1690: stable group-header object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1690: stable group-header object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1690: stable group-header object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1690: stable group-header object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1690: stable group-header object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1690: stable group-header object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"group_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Group header label\\\"\""
            },
            "#1690: stable group-header object selections should expose selected object metadata");
        expect_contains(object_process.stdout_text, "\"left\": 300",
                        "#1690: stable group-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1690: stable group-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1400",
                        "#1690: stable group-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1700",
                        "#1690: stable group-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1690: stable group-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 350",
                        "#1690: stable group-header object selections should expose selected-object bottom bounds");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"group_header_1\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"top\": 0",
                "\"height\": 600",
                "\"bottom\": 600",
                "\"objectCount\": 1"
            },
            "#1690: stable group-header object selections should expose the containing group-header metadata");
    };

    run_group_header_object_selection(temp_root / "selected_group_header_object_stable.frx",
                                      "selected_group_header_object_stable.frx",
                                      "report");
    run_group_header_object_selection(temp_root / "selected_group_header_object_stable.lbx",
                                      "selected_group_header_object_stable.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_group_header_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_group_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_group_header_object_selection = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_deleted_group_header_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-header-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted group-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted group-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1692: stable selected deleted group-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1692: stable selected deleted group-header object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1692: stable selected deleted group-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2069: stable selected deleted group-header report/label object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve live preview width");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve live preview height");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2069: stable selected deleted group-header report/label object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 300",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1700",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 350",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1400",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve deleted preview width");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#2069: stable selected deleted group-header report/label object JSON should preserve deleted preview height");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1692: stable deleted group-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1692: stable deleted group-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1692: stable deleted group-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1692: stable deleted group-header object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1692: stable deleted group-header object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1692: stable deleted group-header object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1692: stable deleted group-header object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1692: stable deleted group-header object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1692: stable deleted group-header object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1692: stable deleted group-header object selections should clear live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1692: stable deleted group-header object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1692: stable deleted group-header object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1692: stable deleted group-header object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"group-header-guid\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Group header label\\\"\""
            },
            "#1692: stable deleted group-header object selections should expose selected deleted-object metadata");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"group-header-guid\"",
                "\"bandKind\": \"group_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1692: stable deleted group-header object selections should expose containing group-header metadata");
        expect_contains(object_process.stdout_text, "\"left\": 300",
                        "#1692: stable deleted group-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1692: stable deleted group-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1400",
                        "#1692: stable deleted group-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1700",
                        "#1692: stable deleted group-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1692: stable deleted group-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 350",
                        "#1692: stable deleted group-header object selections should expose selected-object bottom bounds");
    };

    run_deleted_group_header_object_selection(temp_root / "selected_deleted_group_header_object_stable.frx",
                                              "selected_deleted_group_header_object_stable.frx",
                                              "report");
    run_deleted_group_header_object_selection(temp_root / "selected_deleted_group_header_object_stable.lbx",
                                              "selected_deleted_group_header_object_stable.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_group_footer_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_group_footer_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_group_footer_object_selection = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_stable_group_footer_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-footer-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected group-footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected group-footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1691: stable selected group-footer report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1691: stable selected group-footer object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1691: stable selected group-footer label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2070: stable selected group-footer report/label object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1800",
                        "#2070: stable selected group-footer report/label object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2070: stable selected group-footer report/label object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1800",
                        "#2070: stable selected group-footer report/label object JSON should preserve live preview width");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2070: stable selected group-footer report/label object JSON should preserve live preview height");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2070: stable selected group-footer report/label object JSON should not fabricate deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve zero deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve zero deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve zero deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve zero deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve zero deleted preview width");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2070: stable selected group-footer report/label object JSON should preserve zero deleted preview height");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1691: stable group-footer object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1691: stable group-footer object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1691: stable group-footer object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1691: stable group-footer object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1691: stable group-footer object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1691: stable group-footer object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1691: stable group-footer object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1691: stable group-footer object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1691: stable group-footer object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1691: stable group-footer object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1691: stable group-footer object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1691: stable group-footer object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1691: stable group-footer object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"group_footer_3\"",
                "\"containingSectionRecordIndex\": 3",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Group footer label\\\"\""
            },
            "#1691: stable group-footer object selections should expose selected object metadata");
        expect_contains(object_process.stdout_text, "\"left\": 350",
                        "#1691: stable group-footer object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 3700",
                        "#1691: stable group-footer object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1450",
                        "#1691: stable group-footer object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1800",
                        "#1691: stable group-footer object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1691: stable group-footer object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3950",
                        "#1691: stable group-footer object selections should expose selected-object bottom bounds");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"group_footer_3\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"top\": 3600",
                "\"height\": 500",
                "\"bottom\": 4100",
                "\"objectCount\": 1"
            },
            "#1691: stable group-footer object selections should expose the containing group-footer metadata");
    };

    run_group_footer_object_selection(temp_root / "selected_group_footer_object_stable.frx",
                                      "selected_group_footer_object_stable.frx",
                                      "report");
    run_group_footer_object_selection(temp_root / "selected_group_footer_object_stable.lbx",
                                      "selected_group_footer_object_stable.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_group_footer_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_group_footer_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_group_footer_object_selection = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_deleted_group_footer_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-footer-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted group-footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted group-footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1693: stable selected deleted group-footer report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1693: stable selected deleted group-footer object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1693: stable selected deleted group-footer label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2071: stable selected deleted group-footer report/label object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve live preview width");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve live preview height");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2071: stable selected deleted group-footer report/label object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 350",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 3700",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1800",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3950",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1450",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve deleted preview width");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#2071: stable selected deleted group-footer report/label object JSON should preserve deleted preview height");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1693: stable deleted group-footer object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1693: stable deleted group-footer object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1693: stable deleted group-footer object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1693: stable deleted group-footer object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1693: stable deleted group-footer object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1693: stable deleted group-footer object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1693: stable deleted group-footer object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1693: stable deleted group-footer object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1693: stable deleted group-footer object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1693: stable deleted group-footer object selections should clear live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1693: stable deleted group-footer object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1693: stable deleted group-footer object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1693: stable deleted group-footer object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"group-footer-guid\"",
                "\"containingSectionRecordIndex\": 3",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Group footer label\\\"\""
            },
            "#1693: stable deleted group-footer object selections should expose selected deleted-object metadata");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"group-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1693: stable deleted group-footer object selections should expose containing group-footer metadata");
        expect_contains(object_process.stdout_text, "\"left\": 350",
                        "#1693: stable deleted group-footer object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 3700",
                        "#1693: stable deleted group-footer object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1450",
                        "#1693: stable deleted group-footer object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1800",
                        "#1693: stable deleted group-footer object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1693: stable deleted group-footer object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3950",
                        "#1693: stable deleted group-footer object selections should expose selected-object bottom bounds");
    };

    run_deleted_group_footer_object_selection(temp_root / "selected_deleted_group_footer_object_stable.frx",
                                              "selected_deleted_group_footer_object_stable.frx",
                                              "report");
    run_deleted_group_footer_object_selection(temp_root / "selected_deleted_group_footer_object_stable.lbx",
                                              "selected_deleted_group_footer_object_stable.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_title_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_title_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_title_object_selection = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_stable_title_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "title-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected title object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected title object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1682: stable selected title-band report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1682: stable selected title-band object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1682: stable selected title-band label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1682: stable title-band object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1682: stable title-band object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1682: stable title-band object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1682: stable title-band object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1682: stable title-band object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1682: stable title-band object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1682: stable title-band object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1682: stable title-band object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1682: stable title-band object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1682: stable title-band object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1682: stable title-band object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1941: stable selected title-band object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1941: stable selected title-band object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1941: stable selected title-band object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1500",
                        "#1941: stable selected title-band object JSON should include selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3700",
                        "#1941: stable selected title-band object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1500",
                        "#1941: stable selected title-band object JSON should include selected-object preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3700",
                        "#1941: stable selected title-band object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1941: stable selected title-band object JSON should not fabricate deleted preview availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1682: stable title-band object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1682: stable title-band object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"title_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 120",
                "\"sectionRelativeBottom\": 420",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Title label\\\"\""
            },
            "#1682: stable title-band object selections should expose selected object metadata");
        expect_contains(object_process.stdout_text, "\"left\": 100",
                        "#1682: stable title-band object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 120",
                        "#1682: stable title-band object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1400",
                        "#1682: stable title-band object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1500",
                        "#1682: stable title-band object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 300",
                        "#1682: stable title-band object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 420",
                        "#1682: stable title-band object selections should expose selected-object bottom bounds");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"title_1\"",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"top\": 0",
                "\"height\": 700",
                "\"bottom\": 700",
                "\"objectCount\": 1"
            },
            "#1682: stable title-band object selections should expose the containing title-section metadata");
    };

    run_title_object_selection(temp_root / "selected_title_object_stable.frx",
                               "selected_title_object_stable.frx",
                               "report");
    run_title_object_selection(temp_root / "selected_title_object_stable.lbx",
                               "selected_title_object_stable.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_title_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_title_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_title_object_selection = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_deleted_title_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "title-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted title object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted title object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1686: stable selected deleted title-band report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1686: stable selected deleted title-band object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1686: stable selected deleted title-band label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1686: stable deleted title-band object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1686: stable deleted title-band object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1686: stable deleted title-band object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1686: stable deleted title-band object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1686: stable deleted title-band object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1686: stable deleted title-band object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1686: stable deleted title-band object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1686: stable deleted title-band object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1686: stable deleted title-band object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1686: stable deleted title-band object selections should clear live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1686: stable deleted title-band object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1942: stable selected deleted title-band object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1942: stable selected deleted title-band object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1942: stable selected deleted title-band object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1942: stable selected deleted title-band object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3700",
                        "#1942: stable selected deleted title-band object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1942: stable selected deleted title-band object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3700",
                        "#1942: stable selected deleted title-band object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1942: stable selected deleted title-band object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#1942: stable selected deleted title-band object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 120",
                        "#1942: stable selected deleted title-band object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1500",
                        "#1942: stable selected deleted title-band object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 420",
                        "#1942: stable selected deleted title-band object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1400",
                        "#1942: stable selected deleted title-band object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1942: stable selected deleted title-band object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1686: stable deleted title-band object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1686: stable deleted title-band object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"title-section-guid\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 120",
                "\"sectionRelativeBottom\": 420",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Title label\\\"\""
            },
            "#1686: stable deleted title-band object selections should expose selected deleted-object metadata");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"title-section-guid\"",
                "\"bandKind\": \"title\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1686: stable deleted title-band object selections should expose containing title-band metadata");
        expect_contains(object_process.stdout_text, "\"left\": 100",
                        "#1686: stable deleted title-band object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 120",
                        "#1686: stable deleted title-band object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1400",
                        "#1686: stable deleted title-band object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1500",
                        "#1686: stable deleted title-band object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 300",
                        "#1686: stable deleted title-band object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 420",
                        "#1686: stable deleted title-band object selections should expose selected-object bottom bounds");
    };

    run_deleted_title_object_selection(temp_root / "selected_deleted_title_object_stable.frx",
                                       "selected_deleted_title_object_stable.frx",
                                       "report");
    run_deleted_title_object_selection(temp_root / "selected_deleted_title_object_stable.lbx",
                                       "selected_deleted_title_object_stable.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_page_footer_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_footer_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_page_footer_object_selection = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_stable_page_footer_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "page-footer-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected page-footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected page-footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1683: stable selected page-footer report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1683: stable selected page-footer object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1683: stable selected page-footer label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1683: stable page-footer object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1683: stable page-footer object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1683: stable page-footer object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1683: stable page-footer object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1683: stable page-footer object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1683: stable page-footer object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1683: stable page-footer object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1683: stable page-footer object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1683: stable page-footer object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1683: stable page-footer object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1683: stable page-footer object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1943: stable selected page-footer object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1943: stable selected page-footer object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1943: stable selected page-footer object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1750",
                        "#1943: stable selected page-footer object JSON should include selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3700",
                        "#1943: stable selected page-footer object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1750",
                        "#1943: stable selected page-footer object JSON should include selected-object preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3700",
                        "#1943: stable selected page-footer object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1943: stable selected page-footer object JSON should not fabricate deleted preview availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1683: stable page-footer object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1683: stable page-footer object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"page_footer_3\"",
                "\"containingSectionRecordIndex\": 3",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 400",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Page footer label\\\"\""
            },
            "#1683: stable page-footer object selections should expose selected object metadata");
        expect_contains(object_process.stdout_text, "\"left\": 150",
                        "#1683: stable page-footer object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 3300",
                        "#1683: stable page-footer object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1600",
                        "#1683: stable page-footer object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1750",
                        "#1683: stable page-footer object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 300",
                        "#1683: stable page-footer object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3600",
                        "#1683: stable page-footer object selections should expose selected-object bottom bounds");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"page_footer_3\"",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"top\": 3200",
                "\"height\": 500",
                "\"bottom\": 3700",
                "\"objectCount\": 1"
            },
            "#1683: stable page-footer object selections should expose the containing page-footer metadata");
    };

    run_page_footer_object_selection(temp_root / "selected_page_footer_object_stable.frx",
                                     "selected_page_footer_object_stable.frx",
                                     "report");
    run_page_footer_object_selection(temp_root / "selected_page_footer_object_stable.lbx",
                                     "selected_page_footer_object_stable.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_page_footer_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_page_footer_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_page_footer_object_selection = [&](const fs::path& asset_path,
                                                              const std::string& title,
                                                              const std::string& label) {
        write_synthetic_report_table_for_deleted_page_footer_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "page-footer-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted page-footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted page-footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1687: stable selected deleted page-footer report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1687: stable selected deleted page-footer object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1687: stable selected deleted page-footer label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1687: stable deleted page-footer object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1687: stable deleted page-footer object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1687: stable deleted page-footer object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1687: stable deleted page-footer object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1687: stable deleted page-footer object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1687: stable deleted page-footer object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1687: stable deleted page-footer object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1687: stable deleted page-footer object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1687: stable deleted page-footer object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1687: stable deleted page-footer object selections should clear live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1687: stable deleted page-footer object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1944: stable selected deleted page-footer object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1944: stable selected deleted page-footer object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1944: stable selected deleted page-footer object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1944: stable selected deleted page-footer object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3700",
                        "#1944: stable selected deleted page-footer object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1944: stable selected deleted page-footer object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3700",
                        "#1944: stable selected deleted page-footer object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1944: stable selected deleted page-footer object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 150",
                        "#1944: stable selected deleted page-footer object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 3300",
                        "#1944: stable selected deleted page-footer object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1750",
                        "#1944: stable selected deleted page-footer object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3600",
                        "#1944: stable selected deleted page-footer object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1600",
                        "#1944: stable selected deleted page-footer object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1944: stable selected deleted page-footer object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1687: stable deleted page-footer object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1687: stable deleted page-footer object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"page-footer-section-guid\"",
                "\"containingSectionRecordIndex\": 3",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 400",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Page footer label\\\"\""
            },
            "#1687: stable deleted page-footer object selections should expose selected deleted-object metadata");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"page-footer-section-guid\"",
                "\"bandKind\": \"page_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1687: stable deleted page-footer object selections should expose containing page-footer metadata");
        expect_contains(object_process.stdout_text, "\"left\": 150",
                        "#1687: stable deleted page-footer object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 3300",
                        "#1687: stable deleted page-footer object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1600",
                        "#1687: stable deleted page-footer object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1750",
                        "#1687: stable deleted page-footer object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 300",
                        "#1687: stable deleted page-footer object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3600",
                        "#1687: stable deleted page-footer object selections should expose selected-object bottom bounds");
    };

    run_deleted_page_footer_object_selection(temp_root / "selected_deleted_page_footer_object_stable.frx",
                                             "selected_deleted_page_footer_object_stable.frx",
                                             "report");
    run_deleted_page_footer_object_selection(temp_root / "selected_deleted_page_footer_object_stable.lbx",
                                             "selected_deleted_page_footer_object_stable.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_column_header_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_column_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_header_object_selection = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_stable_column_header_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-header-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected column-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected column-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1684: stable selected column-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1684: stable selected column-header object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1684: stable selected column-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1684: stable column-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1684: stable column-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1684: stable column-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1684: stable column-header object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1684: stable column-header object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1684: stable column-header object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1684: stable column-header object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1684: stable column-header object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1684: stable column-header object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1684: stable column-header object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1684: stable column-header object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1945: stable selected column-header object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1945: stable selected column-header object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1945: stable selected column-header object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1900",
                        "#1945: stable selected column-header object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3450",
                        "#1945: stable selected column-header object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1900",
                        "#1945: stable selected column-header object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3450",
                        "#1945: stable selected column-header object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1945: stable selected column-header object JSON should not fabricate deleted preview bounds");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1684: stable column-header object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1684: stable column-header object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"column_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Column header label\\\"\""
            },
            "#1684: stable column-header object selections should expose selected object metadata");
        expect_contains(object_process.stdout_text, "\"left\": 200",
                        "#1684: stable column-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1684: stable column-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1700",
                        "#1684: stable column-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1900",
                        "#1684: stable column-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1684: stable column-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 350",
                        "#1684: stable column-header object selections should expose selected-object bottom bounds");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"column_header_1\"",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"top\": 0",
                "\"height\": 450",
                "\"bottom\": 450",
                "\"objectCount\": 1"
            },
            "#1684: stable column-header object selections should expose the containing column-header metadata");
    };

    run_column_header_object_selection(temp_root / "selected_column_header_object_stable.frx",
                                       "selected_column_header_object_stable.frx",
                                       "report");
    run_column_header_object_selection(temp_root / "selected_column_header_object_stable.lbx",
                                       "selected_column_header_object_stable.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_column_header_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_column_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_header_object_selection = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_synthetic_report_table_for_deleted_column_header_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-header-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted column-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted column-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1688: stable selected deleted column-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1688: stable selected deleted column-header object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1688: stable selected deleted column-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1688: stable deleted column-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1688: stable deleted column-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1688: stable deleted column-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1688: stable deleted column-header object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1688: stable deleted column-header object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1688: stable deleted column-header object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1688: stable deleted column-header object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1688: stable deleted column-header object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1688: stable deleted column-header object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1688: stable deleted column-header object selections should clear live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1688: stable deleted column-header object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1946: stable selected deleted column-header object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1946: stable selected deleted column-header object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1946: stable selected deleted column-header object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1946: stable selected deleted column-header object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3450",
                        "#1946: stable selected deleted column-header object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1946: stable selected deleted column-header object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3450",
                        "#1946: stable selected deleted column-header object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1946: stable selected deleted column-header object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 200",
                        "#1946: stable selected deleted column-header object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                        "#1946: stable selected deleted column-header object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1900",
                        "#1946: stable selected deleted column-header object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 350",
                        "#1946: stable selected deleted column-header object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1700",
                        "#1946: stable selected deleted column-header object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#1946: stable selected deleted column-header object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1688: stable deleted column-header object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1688: stable deleted column-header object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"column-header-section-guid\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Column header label\\\"\""
            },
            "#1688: stable deleted column-header object selections should expose selected deleted-object metadata");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"column-header-section-guid\"",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1688: stable deleted column-header object selections should expose containing column-header metadata");
        expect_contains(object_process.stdout_text, "\"left\": 200",
                        "#1688: stable deleted column-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1688: stable deleted column-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1700",
                        "#1688: stable deleted column-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 1900",
                        "#1688: stable deleted column-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1688: stable deleted column-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 350",
                        "#1688: stable deleted column-header object selections should expose selected-object bottom bounds");
    };

    run_deleted_column_header_object_selection(temp_root / "selected_deleted_column_header_object_stable.frx",
                                               "selected_deleted_column_header_object_stable.frx",
                                               "report");
    run_deleted_column_header_object_selection(temp_root / "selected_deleted_column_header_object_stable.lbx",
                                               "selected_deleted_column_header_object_stable.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_column_footer_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_column_footer_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_footer_object_selection = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_stable_column_footer_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-footer-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected column-footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected column-footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1685: stable selected column-footer report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1685: stable selected column-footer object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1685: stable selected column-footer label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1685: stable column-footer object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1685: stable column-footer object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1685: stable column-footer object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1685: stable column-footer object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1685: stable column-footer object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1685: stable column-footer object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1685: stable column-footer object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1685: stable column-footer object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1685: stable column-footer object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1685: stable column-footer object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1685: stable column-footer object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1947: stable selected column-footer object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1947: stable selected column-footer object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1947: stable selected column-footer object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 2000",
                        "#1947: stable selected column-footer object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3450",
                        "#1947: stable selected column-footer object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 2000",
                        "#1947: stable selected column-footer object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3450",
                        "#1947: stable selected column-footer object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1947: stable selected column-footer object JSON should not fabricate deleted preview bounds");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1685: stable column-footer object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1685: stable column-footer object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"column_footer_3\"",
                "\"containingSectionRecordIndex\": 3",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Column footer label\\\"\""
            },
            "#1685: stable column-footer object selections should expose selected object metadata");
        expect_contains(object_process.stdout_text, "\"left\": 250",
                        "#1685: stable column-footer object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 3150",
                        "#1685: stable column-footer object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1750",
                        "#1685: stable column-footer object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2000",
                        "#1685: stable column-footer object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1685: stable column-footer object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3400",
                        "#1685: stable column-footer object selections should expose selected-object bottom bounds");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"column_footer_3\"",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"top\": 3050",
                "\"height\": 400",
                "\"bottom\": 3450",
                "\"objectCount\": 1"
            },
            "#1685: stable column-footer object selections should expose the containing column-footer metadata");
    };

    run_column_footer_object_selection(temp_root / "selected_column_footer_object_stable.frx",
                                       "selected_column_footer_object_stable.frx",
                                       "report");
    run_column_footer_object_selection(temp_root / "selected_column_footer_object_stable.lbx",
                                       "selected_column_footer_object_stable.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_column_footer_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_column_footer_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_footer_object_selection = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_synthetic_report_table_for_deleted_column_footer_object_json(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-footer-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted column-footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted column-footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1689: stable selected deleted column-footer report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1689: stable selected deleted column-footer object JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1689: stable selected deleted column-footer label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1689: stable deleted column-footer object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1689: stable deleted column-footer object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1689: stable deleted column-footer object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1689: stable deleted column-footer object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1689: stable deleted column-footer object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1689: stable deleted column-footer object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1689: stable deleted column-footer object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 3",
                        "#1689: stable deleted column-footer object selections should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1689: stable deleted column-footer object selections should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1689: stable deleted column-footer object selections should clear live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1689: stable deleted column-footer object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1948: stable selected deleted column-footer object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1948: stable selected deleted column-footer object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1948: stable selected deleted column-footer object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1948: stable selected deleted column-footer object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3450",
                        "#1948: stable selected deleted column-footer object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1948: stable selected deleted column-footer object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3450",
                        "#1948: stable selected deleted column-footer object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1948: stable selected deleted column-footer object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 250",
                        "#1948: stable selected deleted column-footer object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 3150",
                        "#1948: stable selected deleted column-footer object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2000",
                        "#1948: stable selected deleted column-footer object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3400",
                        "#1948: stable selected deleted column-footer object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1750",
                        "#1948: stable selected deleted column-footer object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#1948: stable selected deleted column-footer object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1689: stable deleted column-footer object selections should advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1689: stable deleted column-footer object selections should expose containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"column-footer-section-guid\"",
                "\"containingSectionRecordIndex\": 3",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 350",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Column footer label\\\"\""
            },
            "#1689: stable deleted column-footer object selections should expose selected deleted-object metadata");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"column-footer-section-guid\"",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1689: stable deleted column-footer object selections should expose containing column-footer metadata");
        expect_contains(object_process.stdout_text, "\"left\": 250",
                        "#1689: stable deleted column-footer object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 3150",
                        "#1689: stable deleted column-footer object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1750",
                        "#1689: stable deleted column-footer object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2000",
                        "#1689: stable deleted column-footer object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 250",
                        "#1689: stable deleted column-footer object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3400",
                        "#1689: stable deleted column-footer object selections should expose selected-object bottom bounds");
    };

    run_deleted_column_footer_object_selection(temp_root / "selected_deleted_column_footer_object_stable.frx",
                                               "selected_deleted_column_footer_object_stable.frx",
                                               "report");
    run_deleted_column_footer_object_selection(temp_root / "selected_deleted_column_footer_object_stable.lbx",
                                               "selected_deleted_column_footer_object_stable.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_unplaced_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_unplaced_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unplaced_object_selection = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto seed_identity = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 5U,
            .object_name = {},
            .unique_id = {},
            .property_name = "UNIQUEID",
            .property_value = "unplaced-line-guid"
        });
        expect(seed_identity.ok,
               "#1661: report/label unplaced object stable selection fixture should seed a stable id");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "unplaced-line-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected unplaced object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected unplaced object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1661: stable selected unplaced report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1661: stable selected unplaced report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1661: stable selected unplaced label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1661: stable unplaced object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1661: stable unplaced object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1661: stable unplaced object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1661: stable unplaced object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1661: stable unplaced object selections should preserve unplaced object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1661: stable unplaced object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1949: stable selected unplaced object JSON should expose live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1949: stable selected unplaced object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1949: stable selected unplaced object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1949: stable selected unplaced object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1949: stable selected unplaced object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1949: stable selected unplaced object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1949: stable selected unplaced object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1949: stable selected unplaced object JSON should preserve deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1949: stable selected unplaced object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1949: stable selected unplaced object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1949: stable selected unplaced object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1949: stable selected unplaced object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1949: stable selected unplaced object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1949: stable selected unplaced object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1661: stable unplaced object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1661: stable unplaced object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1661: stable unplaced object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1661: stable unplaced object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1661: stable unplaced object selections should not advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1661: stable unplaced object selections should serialize null containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectTypeCode\": 6",
                "\"objectKind\": \"line\"",
                "\"title\": \"unplaced-line-guid\"",
                "\"expression\": \"\"",
                "\"highlightCount\": 0"
            },
            "#1661: stable unplaced object selections should expose selected object metadata without section membership");
        expect_contains(object_process.stdout_text, "\"left\": 50",
                        "#1661: stable unplaced object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 8000",
                        "#1661: stable unplaced object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 100",
                        "#1661: stable unplaced object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 150",
                        "#1661: stable unplaced object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 100",
                        "#1661: stable unplaced object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 8100",
                        "#1661: stable unplaced object selections should expose selected-object bottom bounds");
    };

    run_unplaced_object_selection(temp_root / "selected_unplaced_object_stable.frx",
                                  "selected_unplaced_object_stable.frx",
                                  "report");
    run_unplaced_object_selection(temp_root / "selected_unplaced_object_stable.lbx",
                                  "selected_unplaced_object_stable.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_unplaced_report_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_unplaced_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_unplaced_object_selection = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        const auto top_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS",
            .property_value = "9000"
        });
        expect(top_result.ok,
               "#1662: report/label deleted unplaced object stable selection fixture should move the deleted object out of sections");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1662: report/label deleted unplaced object stable selection fixture should preserve deleted state");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deleted-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted unplaced object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted unplaced object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1662: stable selected deleted unplaced report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1662: stable selected deleted unplaced report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1662: stable selected deleted unplaced label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1662: stable deleted unplaced object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1662: stable deleted unplaced object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1662: stable deleted unplaced object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1662: stable deleted unplaced object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1662: stable deleted unplaced object selections should preserve live unplaced object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1662: stable deleted unplaced object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"deletedPlacedObjectCount\": 0",
                        "#1662: stable deleted unplaced object selections should clear deleted placed object counts");
        expect_contains(object_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                        "#1662: stable deleted unplaced object selections should expose deleted unplaced object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1950: stable selected deleted unplaced object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1950: stable selected deleted unplaced object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1950: stable selected deleted unplaced object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1950: stable selected deleted unplaced object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1950: stable selected deleted unplaced object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1950: stable selected deleted unplaced object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1950: stable selected deleted unplaced object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1950: stable selected deleted unplaced object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1950: stable selected deleted unplaced object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 9000",
                        "#1950: stable selected deleted unplaced object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1950: stable selected deleted unplaced object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 9300",
                        "#1950: stable selected deleted unplaced object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1950: stable selected deleted unplaced object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1950: stable selected deleted unplaced object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1662: stable deleted unplaced object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1662: stable deleted unplaced object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1662: stable deleted unplaced object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1662: stable deleted unplaced object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1662: stable deleted unplaced object selections should not advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1662: stable deleted unplaced object selections should serialize null containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Deleted label\\\"\""
            },
            "#1662: stable deleted unplaced object selections should expose selected object metadata without section membership");
        expect_contains(object_process.stdout_text, "\"left\": 1000",
                        "#1662: stable deleted unplaced object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 9000",
                        "#1662: stable deleted unplaced object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1200",
                        "#1662: stable deleted unplaced object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2200",
                        "#1662: stable deleted unplaced object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 300",
                        "#1662: stable deleted unplaced object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 9300",
                        "#1662: stable deleted unplaced object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"EXPR\", \"recordIndex\": 6, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 5, \"value\": \"\\\"Deleted label\\\"\"",
                        "#1662: stable deleted unplaced object selections should expose selected-object expression provenance");
    };

    run_deleted_unplaced_object_selection(temp_root / "selected_deleted_unplaced_object_stable.frx",
                                          "selected_deleted_unplaced_object_stable.frx",
                                          "report");
    run_deleted_unplaced_object_selection(temp_root / "selected_deleted_unplaced_object_stable.lbx",
                                          "selected_deleted_unplaced_object_stable.lbx",
                                          "label");

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

void test_studio_host_json_updates_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_update_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_report_object_update_batch = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "field-guid",
                "--property-name", "EXPR",
                "--property-value", "customer.contact",
                "--property-name", "WIDTH",
                "--property-value", "4300",
                "--selected-unique-id", "label-guid",
                "--property-name", "EXPR",
                "--property-value", "\"Updated invoice\"",
                "--property-name", "HPOS",
                "--property-value", "720",
                "--json"
            },
            temp_root);

        if (update_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object update-batch stdout:\n"
                      << update_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object update-batch stderr:\n"
                      << update_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_batch_process.exit_code == 0,
               "#1842: report/label stable visual-object update-batch JSON should exit successfully");
        expect_contains(update_batch_process.stdout_text, "\"visualObjectUpdateBatch\": {",
                        "#1842: report/label stable visual-object update-batch JSON should expose a batch object");
        expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1842: report/label stable visual-object update-batch JSON should expose affected object counts");
        expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
                        "#1842: report/label stable visual-object update-batch JSON should expose committed state");
        expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1842: report/label stable visual-object update-batch JSON should expose mutation state");
        expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1842: report/label stable visual-object update-batch JSON should expose undo availability");
        expect_contains(update_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2156: report/label stable visual-object update-batch JSON should expose the latest undo label");
        expect(visual_object_property(asset_path, "field-guid", "EXPR") == "customer.contact" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "4300" &&
                   visual_object_property(asset_path, "label-guid", "EXPR") == "\"Updated invoice\"" &&
                   visual_object_property(asset_path, "label-guid", "HPOS") == "720",
               "#1842: report/label stable visual-object update-batch should persist direct and memo-backed layout properties");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "field-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1842: report/label stable visual-object update-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1842: report/label stable visual-object update-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1842: label stable visual-object update-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 5500",
                        "#2048: stable report/label live visual-object update-batch JSON should refresh live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 5500",
                        "#2048: stable report/label live visual-object update-batch JSON should refresh live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview height");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"width\": 4300",
                "\"right\": 5500",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.contact\""
            },
            "#1842: report/label stable visual-object update-batch should refresh selected object metadata after reopen");
    };

    const auto run_report_object_update_batch_rollback = [&](const fs::path& asset_path,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "field-guid",
                "--property-name", "EXPR",
                "--property-value", "should.rollback",
                "--property-name", "WIDTH",
                "--property-value", "4444",
                "--selected-unique-id", "label-guid",
                "--property-name", "HPOS",
                "--property-value", "111",
                "--selected-unique-id", "missing-guid",
                "--property-name", "EXPR",
                "--property-value", "missing",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1842: report/label stable visual-object update-batch missing selector should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectUpdateBatch\": null",
                        "#1842: failed report/label stable visual-object update-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2228: failed report/label stable visual-object update-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2228: failed report/label stable visual-object update-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2184: failed report/label stable visual-object update-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2201: failed report/label stable visual-object update-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1842: failed report/label stable visual-object update-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "field-guid", "EXPR") == "customer.company" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "4000" &&
                   visual_object_property(asset_path, "label-guid", "HPOS") == "900",
               "#1842: failed report/label stable visual-object update-batch should roll back earlier layout property mutations");
        (void)label;
    };

    run_report_object_update_batch(temp_root / "report_object_update_batch.frx",
                                   "report_object_update_batch.frx",
                                   "report");
    run_report_object_update_batch(temp_root / "report_object_update_batch.lbx",
                                   "report_object_update_batch.lbx",
                                   "label");
    run_report_object_update_batch_rollback(temp_root / "report_object_update_batch_rollback.frx",
                                            "report");
    run_report_object_update_batch_rollback(temp_root / "report_object_update_batch_rollback.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_update_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1863: deleted report/label update-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_update_batch = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto update_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--property-name", "EXPR",
                "--property-value", "middle.updated",
                "--property-name", "WIDTH",
                "--property-value", "4300",
                "--selected-unique-id", "right-field-guid",
                "--property-name", "EXPR",
                "--property-value", "right.updated",
                "--property-name", "HPOS",
                "--property-value", "720",
                "--json"
            },
            temp_root);

        if (update_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object update-batch stdout:\n"
                      << update_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object update-batch stderr:\n"
                      << update_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_batch_process.exit_code == 0,
               "#1863: deleted report/label stable visual-object update-batch JSON should exit successfully");
        expect_contains(update_batch_process.stdout_text, "\"visualObjectUpdateBatch\": {",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose a batch object");
        expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose affected object counts");
        expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose committed state");
        expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose mutation state");
        expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose undo availability");
        expect_contains(update_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2156: deleted report/label stable visual-object update-batch JSON should expose the latest undo label");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.updated" &&
                   visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "4300" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.updated" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "720" &&
                   visual_object_property(asset_path, "left-field-guid", "EXPR") == "left.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1863: deleted report/label stable visual-object update-batch should persist properties without changing deleted state");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object update-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object update-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1863: deleted report/label stable visual-object update-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1863: deleted report/label stable visual-object update-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1863: deleted label stable visual-object update-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2049: stable deleted report/label visual-object update-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 4400",
                        "#2049: stable deleted report/label visual-object update-batch JSON should refresh deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4300",
                        "#2049: stable deleted report/label visual-object update-batch JSON should refresh deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1863: deleted report/label stable visual-object update-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1863: deleted report/label stable visual-object update-batch should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1863: deleted report/label stable visual-object update-batch should select the updated deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1863: deleted report/label stable visual-object update-batch should preserve containing-section availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1863: deleted report/label stable visual-object update-batch should serialize containing-section metadata");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1863: deleted report/label stable visual-object update-batch should preserve report object selection kind");
        expect_contains(reopen_process.stdout_text, "\"recordIndex\": 3",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated record indexes");
        expect_contains(reopen_process.stdout_text, "\"deleted\": true",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated deleted state");
        expect_contains(reopen_process.stdout_text, "\"width\": 4300",
                        "#1863: deleted report/label stable visual-object update-batch should refresh updated width metadata");
        expect_contains(reopen_process.stdout_text, "\"right\": 4400",
                        "#1863: deleted report/label stable visual-object update-batch should refresh updated bounds metadata");
        expect_contains(reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated object kind");
        expect_contains(reopen_process.stdout_text, "\"expression\": \"middle.updated\"",
                        "#1863: deleted report/label stable visual-object update-batch should refresh updated expressions");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"middle-field-guid\"",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated stable identities");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.updated\"",
                "\"width\": 4300",
                "\"right\": 4400",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1863: deleted report/label stable visual-object update-batch should refresh selected deleted-row section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1863: deleted report/label stable visual-object update-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_update_batch_rollback = [&](const fs::path& asset_path,
                                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--property-name", "EXPR",
                "--property-value", "should.rollback",
                "--property-name", "WIDTH",
                "--property-value", "4444",
                "--selected-unique-id", "right-field-guid",
                "--property-name", "HPOS",
                "--property-value", "111",
                "--selected-unique-id", "missing-guid",
                "--property-name", "EXPR",
                "--property-value", "missing",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1863: deleted report/label stable visual-object update-batch missing selector should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectUpdateBatch\": null",
                        "#1863: failed deleted report/label stable visual-object update-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2229: failed deleted report/label stable visual-object update-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2229: failed deleted report/label stable visual-object update-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2185: failed deleted report/label stable visual-object update-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2201: failed deleted report/label stable visual-object update-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1863: failed deleted report/label stable visual-object update-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "50" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "100" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1863: failed deleted report/label stable visual-object update-batch should roll back earlier property mutations");
        (void)label;
    };

    run_deleted_report_object_update_batch(temp_root / "deleted_report_object_update_batch.frx",
                                           "deleted_report_object_update_batch.frx",
                                           "report");
    run_deleted_report_object_update_batch(temp_root / "deleted_report_object_update_batch.lbx",
                                           "deleted_report_object_update_batch.lbx",
                                           "label");
    run_deleted_report_object_update_batch_rollback(temp_root / "deleted_report_object_update_batch_rollback.frx",
                                                    "report");
    run_deleted_report_object_update_batch_rollback(temp_root / "deleted_report_object_update_batch_rollback.lbx",
                                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_rename_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_report_object_rename_batch = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto rename_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-renamed-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-renamed-guid",
                "--json"
            },
            temp_root);

        if (rename_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object rename-batch stdout:\n"
                      << rename_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object rename-batch stderr:\n"
                      << rename_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_batch_process.exit_code == 0,
               "#1843: report/label stable visual-object rename-batch JSON should exit successfully");
        expect_contains(rename_batch_process.stdout_text, "\"visualObjectRenameBatch\": {",
                        "#1843: report/label stable visual-object rename-batch JSON should expose a batch object");
        expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1843: report/label stable visual-object rename-batch JSON should expose affected object counts");
        expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
                        "#1843: report/label stable visual-object rename-batch JSON should expose committed state");
        expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1843: report/label stable visual-object rename-batch JSON should expose mutation state");
        expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1843: report/label stable visual-object rename-batch JSON should expose undo availability");
        expect_contains(rename_batch_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                        "#2168: report/label stable visual-object rename-batch JSON should expose renamed-identity undo labels");
        expect(visual_object_count(asset_path) == before_count &&
                   !visual_object_exists(asset_path, "left-field-guid") &&
                   !visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "left-renamed-guid") &&
                   visual_object_exists(asset_path, "right-renamed-guid") &&
                   visual_object_order(asset_path) == "left-renamed-guid,middle-field-guid,right-renamed-guid",
               "#1843: report/label stable visual-object rename-batch should replace identities without changing object order");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "left-renamed-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1843: report/label stable visual-object rename-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1843: report/label stable visual-object rename-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1843: label stable visual-object rename-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2050: stable report/label live visual-object rename-batch JSON should not fabricate deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"left-renamed-guid\"",
                        "#1843: report/label stable visual-object rename-batch should preserve selected object identity after reopen");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"expression\": \"left.value\""
            },
            "#1843: report/label stable visual-object rename-batch should refresh selected renamed object metadata after reopen");
    };

    const auto run_report_object_rename_batch_rollback = [&](const fs::path& asset_path,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-rollback-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1843: report/label stable visual-object rename-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectRenameBatch\": null",
                        "#1843: failed report/label stable visual-object rename-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2230: failed report/label stable visual-object rename-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2230: failed report/label stable visual-object rename-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2178: failed report/label stable visual-object rename-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed report/label stable visual-object rename-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested identity value already exists in the asset.",
                        "#1843: failed report/label stable visual-object rename-batch JSON should report collision errors");
        expect(visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "left-rollback-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1843: failed report/label stable visual-object rename-batch should roll back earlier identity mutations");
        (void)label;
    };

    run_report_object_rename_batch(temp_root / "report_object_rename_batch.frx",
                                   "report_object_rename_batch.frx",
                                   "report");
    run_report_object_rename_batch(temp_root / "report_object_rename_batch.lbx",
                                   "report_object_rename_batch.lbx",
                                   "label");
    run_report_object_rename_batch_rollback(temp_root / "report_object_rename_batch_rollback.frx",
                                            "report");
    run_report_object_rename_batch_rollback(temp_root / "report_object_rename_batch_rollback.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_rename_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1862: deleted report/label rename-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_rename_batch = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");
        const std::size_t before_count = visual_object_count(asset_path);

        const auto rename_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "middle-renamed-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-renamed-guid",
                "--json"
            },
            temp_root);

        if (rename_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object rename-batch stdout:\n"
                      << rename_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object rename-batch stderr:\n"
                      << rename_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_batch_process.exit_code == 0,
               "#1862: deleted report/label stable visual-object rename-batch JSON should exit successfully");
        expect_contains(rename_batch_process.stdout_text, "\"visualObjectRenameBatch\": {",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose a batch object");
        expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose affected object counts");
        expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose committed state");
        expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose mutation state");
        expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose undo availability");
        expect_contains(rename_batch_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                        "#2167: deleted report/label stable visual-object rename-batch JSON should expose renamed-identity undo labels");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   !visual_object_exists(asset_path, "middle-field-guid") &&
                   !visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "middle-renamed-guid") &&
                   visual_object_exists(asset_path, "right-renamed-guid") &&
                   visual_object_deleted(asset_path, "middle-renamed-guid") &&
                   visual_object_deleted(asset_path, "right-renamed-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-renamed-guid,right-renamed-guid",
               "#1862: deleted report/label stable visual-object rename-batch should replace deleted-row identities without changing order");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-renamed-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object rename-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object rename-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1862: deleted report/label stable visual-object rename-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1862: deleted report/label stable visual-object rename-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1862: deleted label stable visual-object rename-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1862: deleted report/label stable visual-object rename-batch should select the renamed deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-renamed-guid\""
            },
            "#1862: deleted report/label stable visual-object rename-batch should preserve selected deleted-row containing-section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1862: deleted report/label stable visual-object rename-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_rename_batch_rollback = [&](const fs::path& asset_path,
                                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "mid-rb-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1862: deleted report/label stable visual-object rename-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectRenameBatch\": null",
                        "#1862: failed deleted report/label stable visual-object rename-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2231: failed deleted report/label stable visual-object rename-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2231: failed deleted report/label stable visual-object rename-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2179: failed deleted report/label stable visual-object rename-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed deleted report/label stable visual-object rename-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested identity value already exists in the asset.",
                        "#1862: failed deleted report/label stable visual-object rename-batch JSON should report collision errors");
        expect(visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "mid-rb-guid") &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1862: failed deleted report/label stable visual-object rename-batch should roll back earlier identity mutations");
        (void)label;
    };

    run_deleted_report_object_rename_batch(temp_root / "deleted_report_object_rename_batch.frx",
                                           "deleted_report_object_rename_batch.frx",
                                           "report");
    run_deleted_report_object_rename_batch(temp_root / "deleted_report_object_rename_batch.lbx",
                                           "deleted_report_object_rename_batch.lbx",
                                           "label");
    run_deleted_report_object_rename_batch_rollback(temp_root / "deleted_report_object_rename_batch_rollback.frx",
                                                    "report");
    run_deleted_report_object_rename_batch_rollback(temp_root / "deleted_report_object_rename_batch_rollback.lbx",
                                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_duplicate_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_report_object_duplicate_batch = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto duplicate_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object duplicate-batch stdout:\n"
                      << duplicate_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object duplicate-batch stderr:\n"
                      << duplicate_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_batch_process.exit_code == 0,
               "#1844: report/label stable visual-object duplicate-batch JSON should exit successfully");
        expect_contains(duplicate_batch_process.stdout_text, "\"visualObjectDuplicateBatch\": {",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose a batch object");
        expect_contains(duplicate_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose affected object counts");
        expect_contains(duplicate_batch_process.stdout_text, "\"dryRun\": false",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose committed state");
        expect_contains(duplicate_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose mutation state");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose undo availability");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2169: report/label stable visual-object duplicate-batch JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 2U &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "left-copy-guid") &&
                   visual_object_exists(asset_path, "right-copy-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,left-copy-guid,right-copy-guid",
               "#1844: report/label stable visual-object duplicate-batch should append duplicates after original objects");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "left-copy-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1844: report/label stable visual-object duplicate-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1844: report/label stable visual-object duplicate-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1844: label stable visual-object duplicate-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should not fabricate deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"left-copy-guid\"",
                        "#1844: report/label stable visual-object duplicate-batch should preserve selected duplicate identity after reopen");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"sectionObjectIndex\": 3",
                "\"sectionObjectCount\": 5",
                "\"objectKind\": \"field\"",
                "\"expression\": \"left.value\""
            },
            "#1844: report/label stable visual-object duplicate-batch should refresh selected duplicate metadata after reopen");
    };

    const auto run_report_object_duplicate_batch_rollback = [&](const fs::path& asset_path,
                                                                const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-rollback-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "left-rollback-copy-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1844: report/label stable visual-object duplicate-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectDuplicateBatch\": null",
                        "#1844: failed report/label stable visual-object duplicate-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2224: failed report/label stable visual-object duplicate-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2224: failed report/label stable visual-object duplicate-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2180: failed report/label stable visual-object duplicate-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed report/label stable visual-object duplicate-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1844: failed report/label stable visual-object duplicate-batch JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "left-rollback-copy-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1844: failed report/label stable visual-object duplicate-batch should roll back earlier duplicates");
        (void)label;
    };

    run_report_object_duplicate_batch(temp_root / "report_object_duplicate_batch.frx",
                                      "report_object_duplicate_batch.frx",
                                      "report");
    run_report_object_duplicate_batch(temp_root / "report_object_duplicate_batch.lbx",
                                      "report_object_duplicate_batch.lbx",
                                      "label");
    run_report_object_duplicate_batch_rollback(temp_root / "report_object_duplicate_batch_rollback.frx",
                                               "report");
    run_report_object_duplicate_batch_rollback(temp_root / "report_object_duplicate_batch_rollback.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_duplicate_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1860: deleted report/label duplicate-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_duplicate_batch = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");
        const std::size_t before_count = visual_object_count(asset_path);

        const auto duplicate_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "middle-deleted-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-deleted-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch stdout:\n"
                      << duplicate_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch stderr:\n"
                      << duplicate_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_batch_process.exit_code == 0,
               "#1860: deleted report/label stable visual-object duplicate-batch JSON should exit successfully");
        expect_contains(duplicate_batch_process.stdout_text, "\"visualObjectDuplicateBatch\": {",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose a batch object");
        expect_contains(duplicate_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose affected object counts");
        expect_contains(duplicate_batch_process.stdout_text, "\"dryRun\": false",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose committed state");
        expect_contains(duplicate_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose mutation state");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose undo availability");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2170: deleted report/label stable visual-object duplicate-batch JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 2U &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "middle-deleted-copy-guid") &&
                   visual_object_exists(asset_path, "right-deleted-copy-guid") &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_deleted(asset_path, "middle-deleted-copy-guid") &&
                   visual_object_deleted(asset_path, "right-deleted-copy-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,middle-deleted-copy-guid,right-deleted-copy-guid",
               "#1860: deleted report/label stable visual-object duplicate-batch should append deleted duplicate rows");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-deleted-copy-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1860: deleted report/label stable visual-object duplicate-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1860: deleted report/label stable visual-object duplicate-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1860: deleted label stable visual-object duplicate-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1860: deleted report/label stable visual-object duplicate-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 4",
                        "#1860: deleted report/label stable visual-object duplicate-batch should expose original and copied deleted rows");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1860: deleted report/label stable visual-object duplicate-batch should select the copied deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1860: deleted report/label stable visual-object duplicate-batch should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1860: deleted report/label stable visual-object duplicate-batch should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-deleted-copy-guid\""
            },
            "#1860: deleted report/label stable visual-object duplicate-batch should preserve selected copied-row containing-section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 4"
            },
            "#1860: deleted report/label stable visual-object duplicate-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_duplicate_batch_rollback = [&](const fs::path& asset_path,
                                                                        const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");
        const std::size_t before_count = visual_object_count(asset_path);

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "mid-rb-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "mid-rb-copy-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1860: deleted report/label stable visual-object duplicate-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectDuplicateBatch\": null",
                        "#1860: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2225: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2225: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2181: failed deleted report/label stable visual-object duplicate-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1860: failed deleted report/label stable visual-object duplicate-batch JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_exists(asset_path, "mid-rb-copy-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1860: failed deleted report/label stable visual-object duplicate-batch should roll back earlier duplicates");
        (void)label;
    };

    run_deleted_report_object_duplicate_batch(temp_root / "deleted_report_object_duplicate_batch.frx",
                                              "deleted_report_object_duplicate_batch.frx",
                                              "report");
    run_deleted_report_object_duplicate_batch(temp_root / "deleted_report_object_duplicate_batch.lbx",
                                              "deleted_report_object_duplicate_batch.lbx",
                                              "label");
    run_deleted_report_object_duplicate_batch_rollback(temp_root / "deleted_report_object_duplicate_batch_rollback.frx",
                                                       "report");
    run_deleted_report_object_duplicate_batch_rollback(temp_root / "deleted_report_object_duplicate_batch_rollback.lbx",
                                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_reorder_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_report_object_reorder_batch = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const auto reorder_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object reorder-batch stdout:\n"
                      << reorder_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object reorder-batch stderr:\n"
                      << reorder_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_batch_process.exit_code == 0,
               "#1845: report/label stable visual-object reorder-batch JSON should exit successfully");
        expect_contains(reorder_batch_process.stdout_text, "\"visualObjectReorderBatch\": {",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose a batch object");
        expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose affected object counts");
        expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose committed state");
        expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose mutation state");
        expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose undo availability");
        expect_contains(reorder_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2171: report/label stable visual-object reorder-batch JSON should expose empty undo labels");
        expect(visual_object_order(asset_path) == "right-field-guid,middle-field-guid,left-field-guid",
               "#1845: report/label stable visual-object reorder-batch should apply ordered stable-selector moves");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1845: report/label stable visual-object reorder-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1845: report/label stable visual-object reorder-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1845: label stable visual-object reorder-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2054: stable report/label live visual-object reorder-batch JSON should not fabricate deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#1845: report/label stable visual-object reorder-batch should preserve selected object identity after reopen");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"right.value\""
            },
            "#1845: report/label stable visual-object reorder-batch should refresh selected reordered object metadata after reopen");
    };

    const auto run_report_object_reorder_batch_rollback = [&](const fs::path& asset_path,
                                                              const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1845: report/label stable visual-object reorder-batch missing target should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectReorderBatch\": null",
                        "#1845: failed report/label stable visual-object reorder-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2226: failed report/label stable visual-object reorder-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2226: failed report/label stable visual-object reorder-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2182: failed report/label stable visual-object reorder-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed report/label stable visual-object reorder-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1845: failed report/label stable visual-object reorder-batch JSON should report missing-target errors");
        expect(visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1845: failed report/label stable visual-object reorder-batch should roll back earlier reorder mutations");
        (void)label;
    };

    run_report_object_reorder_batch(temp_root / "report_object_reorder_batch.frx",
                                    "report_object_reorder_batch.frx",
                                    "report");
    run_report_object_reorder_batch(temp_root / "report_object_reorder_batch.lbx",
                                    "report_object_reorder_batch.lbx",
                                    "label");
    run_report_object_reorder_batch_rollback(temp_root / "report_object_reorder_batch_rollback.frx",
                                             "report");
    run_report_object_reorder_batch_rollback(temp_root / "report_object_reorder_batch_rollback.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_reorder_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1861: deleted report/label reorder-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_reorder_batch = [&](const fs::path& asset_path,
                                                             const std::string& title,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto reorder_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch stdout:\n"
                      << reorder_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch stderr:\n"
                      << reorder_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_batch_process.exit_code == 0,
               "#1861: deleted report/label stable visual-object reorder-batch JSON should exit successfully");
        expect_contains(reorder_batch_process.stdout_text, "\"visualObjectReorderBatch\": {",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose a batch object");
        expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose affected object counts");
        expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose committed state");
        expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose mutation state");
        expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose undo availability");
        expect_contains(reorder_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2172: deleted report/label stable visual-object reorder-batch JSON should expose empty undo labels");
        expect(visual_object_order(asset_path) == "right-field-guid,middle-field-guid,left-field-guid" &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1861: deleted report/label stable visual-object reorder-batch should move deleted rows without changing deleted state");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1861: deleted report/label stable visual-object reorder-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1861: deleted report/label stable visual-object reorder-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1861: deleted label stable visual-object reorder-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1861: deleted report/label stable visual-object reorder-batch should select the moved deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"right.value\"",
                "\"uniqueId\": \"right-field-guid\""
            },
            "#1861: deleted report/label stable visual-object reorder-batch should preserve selected moved-row containing-section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1861: deleted report/label stable visual-object reorder-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_reorder_batch_rollback = [&](const fs::path& asset_path,
                                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1861: deleted report/label stable visual-object reorder-batch missing target should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectReorderBatch\": null",
                        "#1861: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2227: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2227: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2183: failed deleted report/label stable visual-object reorder-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1861: failed deleted report/label stable visual-object reorder-batch JSON should report missing-target errors");
        expect(visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1861: failed deleted report/label stable visual-object reorder-batch should roll back earlier reorder mutations");
        (void)label;
    };

    run_deleted_report_object_reorder_batch(temp_root / "deleted_report_object_reorder_batch.frx",
                                            "deleted_report_object_reorder_batch.frx",
                                            "report");
    run_deleted_report_object_reorder_batch(temp_root / "deleted_report_object_reorder_batch.lbx",
                                            "deleted_report_object_reorder_batch.lbx",
                                            "label");
    run_deleted_report_object_reorder_batch_rollback(temp_root / "deleted_report_object_reorder_batch_rollback.frx",
                                                     "report");
    run_deleted_report_object_reorder_batch_rollback(temp_root / "deleted_report_object_reorder_batch_rollback.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_detail_header_footer_object_containment(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_objects = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " detail header/footer object summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail header/footer object summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1764: detail header/footer object containment JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1764: detail header/footer object containment JSON should return report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1764: detail header/footer object label layouts should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1764: detail header/footer object JSON should summarize live sections");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 2",
                        "#1764: detail header/footer object JSON should summarize live objects");
        expect_contains(process.stdout_text, "\"placedObjectCount\": 2",
                        "#1764: detail header/footer object JSON should count both objects as placed");
        expect_contains(process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1764: detail header/footer object JSON should not fabricate unplaced objects");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2310: detail header/footer object JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2310: detail header/footer object JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 550",
                        "#2310: detail header/footer object JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 550",
                        "#2310: detail header/footer object JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2310: detail header/footer object JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text,
                        "\"sectionKindCounts\": [\n"
                        "        {\"kind\": \"detail_footer\", \"count\": 1},\n"
                        "        {\"kind\": \"detail_header\", \"count\": 1}\n"
                        "      ]",
                        "#1764: detail header/footer object JSON should preserve section-kind buckets");

        const auto expect_selected_object = [&](const std::string& unique_id,
                                                const std::string& record_index,
                                                const std::string& object_kind,
                                                const std::string& containing_section_id,
                                                const std::string& containing_section_record_index,
                                                const std::string& relative_top,
                                                const std::string& relative_bottom,
                                                const std::string& selection_label) {
            const auto object_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                temp_root);

            if (object_process.exit_code != 0) {
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stdout:\n" << object_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stderr:\n" << object_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(object_process.exit_code == 0,
                   "#1764: selected detail header/footer object JSON should exit successfully");
            expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1764: selected detail header/footer object JSON should advertise selected objects");
            expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1764: selected detail header/footer object JSON should expose object selection kind");
            expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1764: selected detail header/footer object JSON should expose containing sections");
            expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2310: selected detail header/footer object JSON should preserve live preview availability");
            expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2310: selected detail header/footer object JSON should preserve live preview top bounds");
            expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2310: selected detail header/footer object JSON should preserve live preview bottom bounds");
            expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2310: selected detail header/footer object JSON should preserve live preview heights");
            expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2310: selected detail header/footer object JSON should not fabricate deleted preview bounds");
            expect_contains_in_order(
                object_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": " + record_index,
                    "\"deleted\": false",
                    "\"containingSectionId\": \"" + containing_section_id + "\"",
                    "\"containingSectionRecordIndex\": " + containing_section_record_index,
                    "\"sectionRelativeTop\": " + relative_top,
                    "\"sectionRelativeBottom\": " + relative_bottom,
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"" + object_kind + "\""
                },
                "#1764: selected " + selection_label + " JSON should expose containing-section metadata");
            expect_contains_in_order(
                object_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"" + containing_section_id + "\"",
                    "\"recordIndex\": " + containing_section_record_index,
                    "\"sectionIndex\": ",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1764: selected " + selection_label + " JSON should expose containing-section object counts");
        };

        expect_selected_object("detail-header-label-guid",
                               "1",
                               "label",
                               "detail-header-guid",
                               "0",
                               "50",
                               "170",
                               "detail-header label");
        expect_selected_object("detail-footer-field-guid",
                               "3",
                               "field",
                               "detail-footer-guid",
                               "2",
                               "60",
                               "160",
                               "detail-footer field");
    };

    run_detail_header_footer_objects(temp_root / "detail_header_footer_objects.frx",
                                     "detail_header_footer_objects.frx",
                                     "report");
    run_detail_header_footer_objects(temp_root / "detail_header_footer_objects.lbx",
                                     "detail_header_footer_objects.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_expressions =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto expect_selected_object_expression =
                [&](const std::string& unique_id,
                    const std::string& record_index,
                    const std::string& object_kind,
                    const std::string& containing_section_id,
                    const std::string& containing_section_record_index,
                    const std::string& relative_top,
                    const std::string& relative_bottom,
                    const std::string& expression,
                    const std::string& memo_block,
                    const std::string& selection_label) {
                    const auto object_process = run_process_capture(
                        studio_host_path,
                        {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                        temp_root);

                    if (object_process.exit_code != 0) {
                        std::cerr << "studio host " << label << " stable selected " << selection_label
                                  << " expression stdout:\n" << object_process.stdout_text << "\n";
                        std::cerr << "studio host " << label << " stable selected " << selection_label
                                  << " expression stderr:\n" << object_process.stderr_text << "\n";
                        std::cerr << "fixture root: " << temp_root << "\n";
                    }

                    expect(object_process.exit_code == 0,
                           "#1769: selected detail header/footer object expression JSON should exit successfully");
                    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1769: selected detail header/footer object expression JSON should preserve titles");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                                        "#1769: selected detail header/footer label object expression JSON should retain identity");
                    }
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                                    "#1769: selected detail header/footer object expressions should select report objects");
                    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                                    "#1769: selected detail header/footer object expressions should expose object selection kind");
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                                    "#1769: selected detail header/footer object expressions should expose containing sections");
                    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                                    "#1769: selected detail header/footer object expressions should not select sections");
                    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1769: selected detail header/footer object expressions should not select settings");
                    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2279: selected detail header/footer object expressions should preserve live preview availability");
                    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2279: selected detail header/footer object expressions should preserve live preview top bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 550",
                                    "#2279: selected detail header/footer object expressions should preserve live preview bottom bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 550",
                                    "#2279: selected detail header/footer object expressions should preserve live preview heights");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                                    "#2279: selected detail header/footer object expressions should not fabricate deleted preview availability");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObject\": {",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": false",
                            "\"containingSectionId\": \"" + containing_section_id + "\"",
                            "\"containingSectionRecordIndex\": " + containing_section_record_index,
                            "\"sectionRelativeTop\": " + relative_top,
                            "\"sectionRelativeBottom\": " + relative_bottom,
                            "\"sectionObjectIndex\": 0",
                            "\"sectionObjectCount\": 1",
                            "\"objectKind\": \"" + object_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": " + memo_block
                        },
                        "#1769: stable selected " + selection_label + " should expose selected-object expression metadata");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObjectSection\": {",
                            "\"id\": \"" + containing_section_id + "\"",
                            "\"recordIndex\": " + containing_section_record_index,
                            "\"sectionCount\": 2",
                            "\"objectCount\": 1"
                        },
                        "#1769: stable selected " + selection_label + " should expose containing-section metadata");
                };

            expect_selected_object_expression("detail-header-label-guid",
                                              "1",
                                              "label",
                                              "detail-header-guid",
                                              "0",
                                              "50",
                                              "170",
                                              "\\\"Header label\\\"",
                                              "2",
                                              "detail-header label");
            expect_selected_object_expression("detail-footer-field-guid",
                                              "3",
                                              "field",
                                              "detail-footer-guid",
                                              "2",
                                              "60",
                                              "160",
                                              "footer.total",
                                              "4",
                                              "detail-footer field");
        };

    run_detail_header_footer_object_expressions(temp_root / "detail_header_footer_object_expressions.frx",
                                                "detail_header_footer_object_expressions.frx",
                                                "report");
    run_detail_header_footer_object_expressions(temp_root / "detail_header_footer_object_expressions.lbx",
                                                "detail_header_footer_object_expressions.lbx",
                                                "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_and_restores_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_delete_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_delete_restore =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#1782: detail-header object delete should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-header-label-guid"),
                   "#1782: detail-header object delete should mark the selected object deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1782: detail-header object delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#1782: detail-header label object delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 1",
                            "#1782: detail-header object delete should reduce live object counts");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 1",
                            "#1782: detail-header object delete should reduce placed object counts");
            expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                            "#1782: detail-header object delete should add deleted object counts");
            expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1782: detail-header object delete should preserve selected object availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1782: detail-header object delete should preserve object selection kind");
            expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1782: detail-header object delete should preserve containing-section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1782: detail-header object delete should serialize containing-section JSON");
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2288: detail-header object delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2288: detail-header object delete should preserve live preview top bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2288: detail-header object delete should preserve live preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2288: detail-header object delete should preserve live preview heights");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2288: detail-header object delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2288: detail-header object delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 170",
                            "#2288: detail-header object delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 120",
                            "#2288: detail-header object delete should expose deleted preview heights");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1782: detail-header object delete should move the object into deleted-object JSON");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1782: detail-header object delete should refresh selected deleted-object JSON");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1782: detail-header object delete should preserve deleted containing-section metadata");

            const auto seed_footer_delete =
                copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(seed_footer_delete.ok && visual_object_deleted(asset_path, "detail-footer-field-guid"),
                   "#1782: detail-footer object restore fixture should mark the footer object deleted");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#1782: detail-footer object restore should exit successfully");
            expect(!visual_object_deleted(asset_path, "detail-footer-field-guid"),
                   "#1782: detail-footer object restore should clear the selected object's deleted state");
            expect(visual_object_deleted(asset_path, "detail-header-label-guid"),
                   "#1782: detail-footer object restore should preserve unrelated deleted header state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1782: detail-footer object restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#1782: detail-footer label object restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 1",
                            "#1782: detail-footer object restore should restore one live object while header remains deleted");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 1",
                            "#1782: detail-footer object restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                            "#1782: detail-footer object restore should leave the header object deleted");
            expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1782: detail-footer object restore should preserve selected object availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1782: detail-footer object restore should preserve object selection kind");
            expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1782: detail-footer object restore should rehydrate containing-section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1782: detail-footer object restore should serialize containing-section JSON");
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2288: detail-footer object restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2288: detail-footer object restore should preserve live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2288: detail-footer object restore should preserve live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2288: detail-footer object restore should preserve live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2288: detail-footer object restore should preserve deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2288: detail-footer object restore should preserve deleted preview top bounds");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 170",
                            "#2288: detail-footer object restore should preserve deleted preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 120",
                            "#2288: detail-footer object restore should preserve deleted preview heights");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460"
                },
                "#1782: detail-footer object restore should refresh selected live-object JSON");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1782: detail-footer object restore should preserve containing-section object metadata");
        };

    run_detail_header_footer_object_delete_restore(temp_root / "detail_header_footer_object_delete_restore.frx",
                                                   "detail_header_footer_object_delete_restore.frx",
                                                   "report");
    run_detail_header_footer_object_delete_restore(temp_root / "detail_header_footer_object_delete_restore.lbx",
                                                   "detail_header_footer_object_delete_restore.lbx",
                                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
