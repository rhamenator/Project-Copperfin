#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_duplicates_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_duplicate =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto expect_document_identity = [&](const ProcessResult& process,
                                                      const fs::path& asset_path,
                                                      const std::string& operation_label) {
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1813: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1813: " + operation_label + " should retain label identity");
                }
            };

            const auto duplicate_section = [&](const fs::path& asset_path,
                                               const std::string& unique_id,
                                               const std::string& new_unique_id,
                                               const std::string& section_title,
                                               const std::string& band_kind,
                                               const std::string& expression,
                                               const std::string& object_code,
                                               const std::string& top,
                                               const std::string& height,
                                               const std::string& bottom,
                                               const std::string& detail_header_count,
                                               const std::string& detail_footer_count,
                                               const std::string& section_height_total,
                                               const std::string& selected_section_index,
                                               const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto duplicate_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--duplicate-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (duplicate_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << duplicate_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << duplicate_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(duplicate_process.exit_code == 0,
                       "#1813: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count + 1U,
                       "#1813: " + operation_label + " should append one section record");
                expect(visual_object_exists(asset_path, new_unique_id),
                       "#1813: " + operation_label + " should persist the replacement unique id");
                expect(!dbf_record_deleted(asset_path, before_count),
                       "#1813: " + operation_label + " should append a live duplicate section");
                const auto duplicated_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = before_count,
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(duplicated_expression.ok && duplicated_expression.exists &&
                           duplicated_expression.value == expression,
                       "#1813: " + operation_label + " should preserve section expression data");

                expect_document_identity(duplicate_process, asset_path, operation_label);
                expect_contains(duplicate_process.stdout_text, "\"sectionCount\": 3",
                                "#1813: " + operation_label + " should refresh live section counts");
                expect_contains(duplicate_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1813: " + operation_label + " should preserve deleted section counts");
                expect_contains(duplicate_process.stdout_text,
                                "\"sectionKindCounts\": [\n"
                                "        {\"kind\": \"detail_footer\", \"count\": " + detail_footer_count + "},\n"
                                "        {\"kind\": \"detail_header\", \"count\": " + detail_header_count + "}\n"
                                "      ]",
                                "#1813: " + operation_label + " should refresh detail section kind counts");
                expect_contains(duplicate_process.stdout_text, "\"sectionHeightTotal\": " + section_height_total,
                                "#1813: " + operation_label + " should refresh live section height totals");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1877: " + operation_label + " should preserve live preview availability");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1877: " + operation_label + " should preserve live preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#1877: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#1877: " + operation_label + " should preserve live preview height");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1877: " + operation_label + " should preserve deleted preview availability");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#1877: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#1877: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#1877: " + operation_label + " should preserve deleted preview height");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1813: " + operation_label + " should advertise selected sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1813: " + operation_label + " should expose section selection kind");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1813: " + operation_label + " should not select report objects");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1813: " + operation_label + " should not fabricate object sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1813: " + operation_label + " should not select settings");
                expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                                "#2242: " + operation_label + " JSON should expose committed state");
                expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                                "#2242: " + operation_label + " JSON should expose mutation state");
                expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                                "#2242: " + operation_label + " JSON should expose undo availability");
                expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2242: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + std::to_string(before_count),
                        "\"deleted\": false",
                        "\"sectionIndex\": " + selected_section_index,
                        "\"sectionCount\": 3",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1813: " + operation_label + " should select the duplicated section metadata");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"Detail Footer\"",
                        "\"bandKind\": \"detail_footer\"",
                        "\"recordIndex\": 2",
                        "\"deleted\": true"
                    },
                    "#1813: " + operation_label + " should preserve existing deleted-section metadata");
            };

            duplicate_section(header_asset_path,
                              "detail-header-guid",
                              "detail-header-copy-guid",
                              "Detail Header",
                              "detail_header",
                              "detail header expression",
                              "9",
                              "0",
                              "300",
                              "300",
                              "2",
                              "1",
                              "850",
                              "1",
                              "stable detail-header section duplicate");
            duplicate_section(footer_asset_path,
                              "detail-footer-guid",
                              "detail-footer-copy-guid",
                              "Detail Footer",
                              "detail_footer",
                              "detail footer expression",
                              "10",
                              "300",
                              "250",
                              "550",
                              "1",
                              "2",
                              "800",
                              "2",
                              "stable detail-footer section duplicate");
        };

    run_detail_header_footer_section_duplicate(
        temp_root / "detail_header_section_duplicate.frx",
        temp_root / "detail_footer_section_duplicate.frx",
        "report");
    run_detail_header_footer_section_duplicate(
        temp_root / "detail_header_section_duplicate.lbx",
        temp_root / "detail_footer_section_duplicate.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_section_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_section_duplicate =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto duplicate_deleted_section = [&](const fs::path& asset_path,
                                                       const std::string& unique_id,
                                                       const std::string& new_unique_id,
                                                       const std::string& section_title,
                                                       const std::string& band_kind,
                                                       const std::string& expression,
                                                       const std::string& object_code,
                                                       const std::string& top,
                                                       const std::string& height,
                                                       const std::string& bottom,
                                                       const std::string& deleted_height_total,
                                                       const std::string& operation_label) {
                write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto duplicate_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--duplicate-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (duplicate_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << duplicate_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << duplicate_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(duplicate_process.exit_code == 0,
                       "#1816: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count + 1U,
                       "#1816: " + operation_label + " should append one section record");
                expect(visual_object_exists(asset_path, new_unique_id),
                       "#1816: " + operation_label + " should persist the replacement unique id");
                expect(dbf_record_deleted(asset_path, before_count),
                       "#1816: " + operation_label + " should append a deleted section duplicate");
                const auto duplicated_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = before_count,
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(duplicated_expression.ok && duplicated_expression.exists &&
                           duplicated_expression.value == expression,
                       "#1816: " + operation_label + " should preserve section expression data");

                expect_contains(duplicate_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1816: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(duplicate_process.stdout_text, "\"isLabel\": true",
                                    "#1816: " + operation_label + " should retain label identity");
                }
                expect_contains(duplicate_process.stdout_text, "\"sectionCount\": 1",
                                "#1816: " + operation_label + " should preserve live section counts");
                expect_contains(duplicate_process.stdout_text, "\"deletedSectionCount\": 3",
                                "#1816: " + operation_label + " should refresh deleted section counts");
                expect_contains(duplicate_process.stdout_text,
                                "\"deletedSectionHeightTotal\": " + deleted_height_total,
                                "#1816: " + operation_label + " should refresh deleted section height totals");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1874: " + operation_label + " should preserve live preview availability");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1874: " + operation_label + " should preserve live preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#1874: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#1874: " + operation_label + " should preserve live preview height");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1874: " + operation_label + " should preserve deleted preview availability");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#1874: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#1874: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#1874: " + operation_label + " should preserve deleted preview height");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1816: " + operation_label + " should advertise selected sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1816: " + operation_label + " should expose section selection kind");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1816: " + operation_label + " should not select report objects");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1816: " + operation_label + " should not fabricate object sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1816: " + operation_label + " should not select settings");
                expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                                "#2243: " + operation_label + " JSON should expose committed state");
                expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                                "#2243: " + operation_label + " JSON should expose mutation state");
                expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                                "#2243: " + operation_label + " JSON should expose undo availability");
                expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2243: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + std::to_string(before_count),
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1816: " + operation_label + " should select the duplicated deleted-section metadata");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"recordIndex\": 0",
                        "\"deleted\": false"
                    },
                    "#1816: " + operation_label + " should preserve live Detail section metadata");
            };

            duplicate_deleted_section(header_asset_path,
                                      "deleted-detail-header-guid",
                                      "deleted-detail-header-copy-guid",
                                      "Detail Header",
                                      "detail_header",
                                      "deleted detail header expression",
                                      "9",
                                      "500",
                                      "300",
                                      "800",
                                      "850",
                                      "stable deleted detail-header section duplicate");
            duplicate_deleted_section(footer_asset_path,
                                      "deleted-detail-footer-guid",
                                      "deleted-detail-footer-copy-guid",
                                      "Detail Footer",
                                      "detail_footer",
                                      "deleted detail footer expression",
                                      "10",
                                      "800",
                                      "250",
                                      "1050",
                                      "800",
                                      "stable deleted detail-footer section duplicate");
        };

    run_deleted_detail_header_footer_section_duplicate(
        temp_root / "deleted_detail_header_section_duplicate.frx",
        temp_root / "deleted_detail_footer_section_duplicate.frx",
        "report");
    run_deleted_detail_header_footer_section_duplicate(
        temp_root / "deleted_detail_header_section_duplicate.lbx",
        temp_root / "deleted_detail_footer_section_duplicate.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_rename =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto rename_section = [&](const fs::path& asset_path,
                                            const std::string& unique_id,
                                            const std::string& new_unique_id,
                                            const std::string& section_title,
                                            const std::string& band_kind,
                                            const std::string& expression,
                                            const std::string& record_index,
                                            const std::string& section_index,
                                            const std::string& object_code,
                                            const std::string& top,
                                            const std::string& height,
                                            const std::string& bottom,
                                            const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto rename_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--rename-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (rename_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << rename_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << rename_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(rename_process.exit_code == 0,
                       "#1814: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1814: " + operation_label + " should not append section records");
                expect(!visual_object_exists(asset_path, unique_id) && visual_object_exists(asset_path, new_unique_id),
                       "#1814: " + operation_label + " should replace the section unique id");
                const auto renamed_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = static_cast<std::size_t>(std::stoul(record_index)),
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(renamed_expression.ok && renamed_expression.exists &&
                           renamed_expression.value == expression,
                       "#1814: " + operation_label + " should preserve section expression data");

                expect_contains(rename_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1814: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                                    "#1814: " + operation_label + " should retain label identity");
                }
                expect_contains(rename_process.stdout_text, "\"sectionCount\": 2",
                                "#1814: " + operation_label + " should preserve live section counts");
                expect_contains(rename_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1814: " + operation_label + " should preserve deleted section counts");
                expect_contains(rename_process.stdout_text, "\"sectionHeightTotal\": 550",
                                "#1814: " + operation_label + " should preserve live section height totals");
                expect_contains(rename_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1878: " + operation_label + " should preserve live preview availability");
                expect_contains(rename_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1878: " + operation_label + " should preserve live preview top bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#1878: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#1878: " + operation_label + " should preserve live preview height");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1878: " + operation_label + " should preserve deleted preview availability");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#1878: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#1878: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#1878: " + operation_label + " should preserve deleted preview height");
                expect_contains(rename_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1814: " + operation_label + " should advertise selected sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1814: " + operation_label + " should expose section selection kind");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1814: " + operation_label + " should not select report objects");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1814: " + operation_label + " should not fabricate object sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1814: " + operation_label + " should not select settings");
                expect_contains(rename_process.stdout_text, "\"dryRun\": false",
                                "#2244: " + operation_label + " JSON should expose committed state");
                expect_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
                                "#2244: " + operation_label + " JSON should expose mutation state");
                expect_contains(rename_process.stdout_text, "\"undoAvailable\": true",
                                "#2244: " + operation_label + " JSON should expose undo availability");
                expect_contains(rename_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                                "#2244: " + operation_label + " JSON should expose renamed-identity undo labels");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1814: " + operation_label + " should select the renamed section metadata");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"Detail Footer\"",
                        "\"bandKind\": \"detail_footer\"",
                        "\"recordIndex\": 2",
                        "\"deleted\": true"
                    },
                    "#1814: " + operation_label + " should preserve existing deleted-section metadata");
            };

            rename_section(header_asset_path,
                           "detail-header-guid",
                           "detail-header-renamed-guid",
                           "Detail Header",
                           "detail_header",
                           "detail header expression",
                           "0",
                           "0",
                           "9",
                           "0",
                           "300",
                           "300",
                           "stable detail-header section rename");
            rename_section(footer_asset_path,
                           "detail-footer-guid",
                           "detail-footer-renamed-guid",
                           "Detail Footer",
                           "detail_footer",
                           "detail footer expression",
                           "1",
                           "1",
                           "10",
                           "300",
                           "250",
                           "550",
                           "stable detail-footer section rename");
        };

    run_detail_header_footer_section_rename(
        temp_root / "detail_header_section_rename.frx",
        temp_root / "detail_footer_section_rename.frx",
        "report");
    run_detail_header_footer_section_rename(
        temp_root / "detail_header_section_rename.lbx",
        temp_root / "detail_footer_section_rename.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_section_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_section_rename =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto rename_deleted_section = [&](const fs::path& asset_path,
                                                    const std::string& unique_id,
                                                    const std::string& new_unique_id,
                                                    const std::string& section_title,
                                                    const std::string& band_kind,
                                                    const std::string& expression,
                                                    const std::string& record_index,
                                                    const std::string& object_code,
                                                    const std::string& top,
                                                    const std::string& height,
                                                    const std::string& bottom,
                                                    const std::string& operation_label) {
                write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);
                const std::size_t section_record_index = static_cast<std::size_t>(std::stoul(record_index));

                const auto rename_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--rename-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (rename_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << rename_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << rename_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(rename_process.exit_code == 0,
                       "#1817: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1817: " + operation_label + " should not append section records");
                expect(!visual_object_exists(asset_path, unique_id) && visual_object_exists(asset_path, new_unique_id),
                       "#1817: " + operation_label + " should replace the section unique id");
                expect(dbf_record_deleted(asset_path, section_record_index),
                       "#1817: " + operation_label + " should preserve deleted state");
                const auto renamed_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = section_record_index,
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(renamed_expression.ok && renamed_expression.exists &&
                           renamed_expression.value == expression,
                       "#1817: " + operation_label + " should preserve section expression data");

                expect_contains(rename_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1817: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                                    "#1817: " + operation_label + " should retain label identity");
                }
                expect_contains(rename_process.stdout_text, "\"sectionCount\": 1",
                                "#1817: " + operation_label + " should preserve live section counts");
                expect_contains(rename_process.stdout_text, "\"deletedSectionCount\": 2",
                                "#1817: " + operation_label + " should preserve deleted section counts");
                expect_contains(rename_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                                "#1817: " + operation_label + " should preserve deleted section height totals");
                expect_contains(rename_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1875: " + operation_label + " should preserve live preview availability");
                expect_contains(rename_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1875: " + operation_label + " should preserve live preview top bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#1875: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#1875: " + operation_label + " should preserve live preview height");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1875: " + operation_label + " should preserve deleted preview availability");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#1875: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#1875: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#1875: " + operation_label + " should preserve deleted preview height");
                expect_contains(rename_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1817: " + operation_label + " should advertise selected sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1817: " + operation_label + " should expose section selection kind");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1817: " + operation_label + " should not select report objects");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1817: " + operation_label + " should not fabricate object sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1817: " + operation_label + " should not select settings");
                expect_contains(rename_process.stdout_text, "\"dryRun\": false",
                                "#2245: " + operation_label + " JSON should expose committed state");
                expect_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
                                "#2245: " + operation_label + " JSON should expose mutation state");
                expect_contains(rename_process.stdout_text, "\"undoAvailable\": true",
                                "#2245: " + operation_label + " JSON should expose undo availability");
                expect_contains(rename_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                                "#2245: " + operation_label + " JSON should expose renamed-identity undo labels");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1817: " + operation_label + " should select the renamed deleted-section metadata");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"recordIndex\": 0",
                        "\"deleted\": false"
                    },
                    "#1817: " + operation_label + " should preserve live Detail section metadata");
            };

            rename_deleted_section(header_asset_path,
                                   "deleted-detail-header-guid",
                                   "deleted-header-renamed-guid",
                                   "Detail Header",
                                   "detail_header",
                                   "deleted detail header expression",
                                   "1",
                                   "9",
                                   "500",
                                   "300",
                                   "800",
                                   "stable deleted detail-header section rename");
            rename_deleted_section(footer_asset_path,
                                   "deleted-detail-footer-guid",
                                   "deleted-footer-renamed-guid",
                                   "Detail Footer",
                                   "detail_footer",
                                   "deleted detail footer expression",
                                   "2",
                                   "10",
                                   "800",
                                   "250",
                                   "1050",
                                   "stable deleted detail-footer section rename");
        };

    run_deleted_detail_header_footer_section_rename(
        temp_root / "deleted_detail_header_section_rename.frx",
        temp_root / "deleted_detail_footer_section_rename.frx",
        "report");
    run_deleted_detail_header_footer_section_rename(
        temp_root / "deleted_detail_header_section_rename.lbx",
        temp_root / "deleted_detail_footer_section_rename.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_reorder =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto reorder_section = [&](const fs::path& asset_path,
                                             const std::string& unique_id,
                                             const std::string& target_unique_id,
                                             const std::string& placement,
                                             const std::string& section_title,
                                             const std::string& band_kind,
                                             const std::string& expression,
                                             const std::string& record_index,
                                             const std::string& section_index,
                                             const std::string& object_code,
                                             const std::string& top,
                                             const std::string& height,
                                             const std::string& bottom,
                                             const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto reorder_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--reorder-object",
                        "--unique-id", unique_id,
                        "--placement", placement,
                        "--target-unique-id", target_unique_id,
                        "--json"
                    },
                    temp_root);

                if (reorder_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << reorder_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << reorder_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(reorder_process.exit_code == 0,
                       "#1815: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1815: " + operation_label + " should preserve section record count");
                expect(visual_object_exists(asset_path, unique_id),
                       "#1815: " + operation_label + " should preserve the moved section unique id");
                expect(dbf_record_deleted(asset_path, 2U),
                       "#1815: " + operation_label + " should preserve deleted-section state");
                const auto moved_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = static_cast<std::size_t>(std::stoul(record_index)),
                    .object_name = {},
                    .unique_id = unique_id,
                    .property_name = "EXPR"
                });
                expect(moved_expression.ok && moved_expression.exists &&
                           moved_expression.value == expression,
                       "#1815: " + operation_label + " should preserve section expression data");

                expect_contains(reorder_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1815: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                                    "#1815: " + operation_label + " should retain label identity");
                }
                expect_contains(reorder_process.stdout_text, "\"sectionCount\": 2",
                                "#1815: " + operation_label + " should preserve live section counts");
                expect_contains(reorder_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1815: " + operation_label + " should preserve deleted section counts");
                expect_contains(reorder_process.stdout_text, "\"sectionHeightTotal\": 550",
                                "#1815: " + operation_label + " should preserve live section height totals");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1879: " + operation_label + " should preserve live preview availability");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1879: " + operation_label + " should preserve live preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#1879: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#1879: " + operation_label + " should preserve live preview height");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1879: " + operation_label + " should preserve deleted preview availability");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#1879: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#1879: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#1879: " + operation_label + " should preserve deleted preview height");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1815: " + operation_label + " should advertise selected sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1815: " + operation_label + " should expose section selection kind");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1815: " + operation_label + " should not select report objects");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1815: " + operation_label + " should not fabricate object sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1815: " + operation_label + " should not select settings");
                expect_contains(reorder_process.stdout_text, "\"dryRun\": false",
                                "#2246: " + operation_label + " JSON should expose committed state");
                expect_contains(reorder_process.stdout_text, "\"mutatesAsset\": true",
                                "#2246: " + operation_label + " JSON should expose mutation state");
                expect_contains(reorder_process.stdout_text, "\"undoAvailable\": false",
                                "#2246: " + operation_label + " JSON should expose undo availability");
                expect_contains(reorder_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2246: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1815: " + operation_label + " should select the moved section metadata");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"Detail Footer\"",
                        "\"bandKind\": \"detail_footer\"",
                        "\"recordIndex\": 2",
                        "\"deleted\": true"
                    },
                    "#1815: " + operation_label + " should preserve existing deleted-section metadata");
            };

            reorder_section(header_asset_path,
                            "detail-header-guid",
                            "detail-footer-guid",
                            "after",
                            "Detail Header",
                            "detail_header",
                            "detail header expression",
                            "1",
                            "0",
                            "9",
                            "0",
                            "300",
                            "300",
                            "stable detail-header section reorder");
            reorder_section(footer_asset_path,
                            "detail-footer-guid",
                            "detail-header-guid",
                            "before",
                            "Detail Footer",
                            "detail_footer",
                            "detail footer expression",
                            "0",
                            "1",
                            "10",
                            "300",
                            "250",
                            "550",
                            "stable detail-footer section reorder");
        };

    run_detail_header_footer_section_reorder(
        temp_root / "detail_header_section_reorder.frx",
        temp_root / "detail_footer_section_reorder.frx",
        "report");
    run_detail_header_footer_section_reorder(
        temp_root / "detail_header_section_reorder.lbx",
        temp_root / "detail_footer_section_reorder.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_section_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_section_reorder =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto reorder_deleted_section = [&](const fs::path& asset_path,
                                                     const std::string& unique_id,
                                                     const std::string& target_unique_id,
                                                     const std::string& placement,
                                                     const std::string& section_title,
                                                     const std::string& band_kind,
                                                     const std::string& expression,
                                                     const std::string& record_index,
                                                     const std::string& object_code,
                                                     const std::string& top,
                                                     const std::string& height,
                                                     const std::string& bottom,
                                                     const std::string& operation_label) {
                write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);
                const std::size_t moved_record_index = static_cast<std::size_t>(std::stoul(record_index));

                const auto reorder_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--reorder-object",
                        "--unique-id", unique_id,
                        "--placement", placement,
                        "--target-unique-id", target_unique_id,
                        "--json"
                    },
                    temp_root);

                if (reorder_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << reorder_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << reorder_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(reorder_process.exit_code == 0,
                       "#1818: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1818: " + operation_label + " should preserve section record count");
                expect(visual_object_exists(asset_path, unique_id),
                       "#1818: " + operation_label + " should preserve the moved section unique id");
                expect(dbf_record_deleted(asset_path, moved_record_index),
                       "#1818: " + operation_label + " should preserve deleted state");
                const auto moved_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = moved_record_index,
                    .object_name = {},
                    .unique_id = unique_id,
                    .property_name = "EXPR"
                });
                expect(moved_expression.ok && moved_expression.exists &&
                           moved_expression.value == expression,
                       "#1818: " + operation_label + " should preserve section expression data");

                expect_contains(reorder_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1818: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                                    "#1818: " + operation_label + " should retain label identity");
                }
                expect_contains(reorder_process.stdout_text, "\"sectionCount\": 1",
                                "#1818: " + operation_label + " should preserve live section counts");
                expect_contains(reorder_process.stdout_text, "\"deletedSectionCount\": 2",
                                "#1818: " + operation_label + " should preserve deleted section counts");
                expect_contains(reorder_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                                "#1818: " + operation_label + " should preserve deleted section height totals");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1876: " + operation_label + " should preserve live preview availability");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1876: " + operation_label + " should preserve live preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#1876: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#1876: " + operation_label + " should preserve live preview height");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1876: " + operation_label + " should preserve deleted preview availability");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#1876: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#1876: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#1876: " + operation_label + " should preserve deleted preview height");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1818: " + operation_label + " should advertise selected sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1818: " + operation_label + " should expose section selection kind");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1818: " + operation_label + " should not select report objects");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1818: " + operation_label + " should not fabricate object sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1818: " + operation_label + " should not select settings");
                expect_contains(reorder_process.stdout_text, "\"dryRun\": false",
                                "#2247: " + operation_label + " JSON should expose committed state");
                expect_contains(reorder_process.stdout_text, "\"mutatesAsset\": true",
                                "#2247: " + operation_label + " JSON should expose mutation state");
                expect_contains(reorder_process.stdout_text, "\"undoAvailable\": false",
                                "#2247: " + operation_label + " JSON should expose undo availability");
                expect_contains(reorder_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2247: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1818: " + operation_label + " should select the moved deleted-section metadata");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"recordIndex\": 0",
                        "\"deleted\": false"
                    },
                    "#1818: " + operation_label + " should preserve live Detail section metadata");
            };

            reorder_deleted_section(header_asset_path,
                                    "deleted-detail-header-guid",
                                    "deleted-detail-footer-guid",
                                    "after",
                                    "Detail Header",
                                    "detail_header",
                                    "deleted detail header expression",
                                    "2",
                                    "9",
                                    "500",
                                    "300",
                                    "800",
                                    "stable deleted detail-header section reorder");
            reorder_deleted_section(footer_asset_path,
                                    "deleted-detail-footer-guid",
                                    "deleted-detail-header-guid",
                                    "before",
                                    "Detail Footer",
                                    "detail_footer",
                                    "deleted detail footer expression",
                                    "1",
                                    "10",
                                    "800",
                                    "250",
                                    "1050",
                                    "stable deleted detail-footer section reorder");
        };

    run_deleted_detail_header_footer_section_reorder(
        temp_root / "deleted_detail_header_section_reorder.frx",
        temp_root / "deleted_detail_footer_section_reorder.frx",
        "report");
    run_deleted_detail_header_footer_section_reorder(
        temp_root / "deleted_detail_header_section_reorder.lbx",
        temp_root / "deleted_detail_footer_section_reorder.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_expression_edits = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "0",
                "--property-name", "EXPR",
                "--property-value", "detail header updated",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " detail-header expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail-header expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1765: detail-header report/label expression update should exit successfully");
        const auto updated_expr = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(updated_expr.ok && updated_expr.exists && updated_expr.value == "detail header updated",
               "#1765: detail-header expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1765: detail-header expression update should return refreshed layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1765: detail-header expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1765: detail-header expression update should preserve section selection");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1765: detail-header expression update should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2262: detail-header expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2262: detail-header expression update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2262: detail-header expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2262: detail-header expression update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                        "#2262: detail-header expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2262: detail-header expression update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                        "#2262: detail-header expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"dryRun\": false",
                        "#2262: detail-header expression update JSON should expose committed execution");
        expect_contains(update_process.stdout_text, "\"mutatesAsset\": true",
                        "#2262: detail-header expression update JSON should expose mutation state");
        expect_contains(update_process.stdout_text, "\"undoAvailable\": true",
                        "#2262: detail-header expression update JSON should expose undo availability");
        expect_contains(update_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2262: detail-header expression update JSON should expose expression undo labels");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"title\": \"Detail Header\"",
                "\"bandKind\": \"detail_header\"",
                "\"expression\": \"detail header updated\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 0",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1765: detail-header expression update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"title\": \"Detail Footer\"",
                "\"bandKind\": \"detail_footer\"",
                "\"expression\": \"detail footer expression\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1"
            },
            "#1765: detail-header expression update should preserve sibling detail-footer expression metadata");

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
            std::cerr << "studio host " << label << " detail-footer expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail-footer expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1765: detail-footer report/label expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1765: detail-footer expression clear should return refreshed layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1765: detail-footer expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1765: detail-footer expression clear should preserve section selection");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1765: detail-footer expression clear should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2262: detail-footer expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2262: detail-footer expression clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2262: detail-footer expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2262: detail-footer expression clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                        "#2262: detail-footer expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2262: detail-footer expression clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                        "#2262: detail-footer expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"dryRun\": false",
                        "#2262: detail-footer expression clear JSON should expose committed execution");
        expect_contains(clear_process.stdout_text, "\"mutatesAsset\": true",
                        "#2262: detail-footer expression clear JSON should expose mutation state");
        expect_contains(clear_process.stdout_text, "\"undoAvailable\": true",
                        "#2262: detail-footer expression clear JSON should expose undo availability");
        expect_contains(clear_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2262: detail-footer expression clear JSON should expose expression undo labels");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"title\": \"Detail Footer\"",
                "\"bandKind\": \"detail_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"top\": 300",
                "\"height\": 250",
                "\"bottom\": 550"
            },
            "#1765: detail-footer expression clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"title\": \"Detail Header\"",
                "\"bandKind\": \"detail_header\"",
                "\"expression\": \"detail header updated\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 0"
            },
            "#1765: detail-footer expression clear should preserve sibling detail-header expression metadata");
    };

    run_detail_header_footer_expression_edits(temp_root / "detail_header_footer_expression.frx",
                                              "detail_header_footer_expression.frx",
                                              "report");
    run_detail_header_footer_expression_edits(temp_root / "detail_header_footer_expression.lbx",
                                              "detail_header_footer_expression.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_stable_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_stable_expression_selection =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto expect_live_section = [&](const std::string& unique_id,
                                                 const std::string& section_title,
                                                 const std::string& band_kind,
                                                 const std::string& expression,
                                                 const std::string& memo_block,
                                                 const std::string& record_index,
                                                 const std::string& section_index,
                                                 const std::string& object_code,
                                                 const std::string& top,
                                                 const std::string& height,
                                                 const std::string& bottom,
                                                 const std::string& selection_label) {
                const auto process = run_process_capture(
                    studio_host_path,
                    {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                    temp_root);

                if (process.exit_code != 0) {
                    std::cerr << "studio host " << label << " stable live " << selection_label
                              << " stdout:\n" << process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " stable live " << selection_label
                              << " stderr:\n" << process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(process.exit_code == 0,
                       "#1768: stable detail header/footer section expression selection should exit successfully");
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                "#1768: stable detail header/footer expression selection should preserve document titles");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1768: stable detail header/footer expression label selection should retain identity");
                }
                expect_contains(process.stdout_text, "\"sectionCount\": 2",
                                "#1768: stable detail header/footer expression selection should preserve live sections");
                expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1768: stable detail header/footer expression selection should preserve deleted sections");
                expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#2267: stable detail header/footer expression selection should preserve live preview availability");
                expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview left bounds");
                expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview top bounds");
                expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview right bounds");
                expect_contains(process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#2267: stable detail header/footer expression selection should preserve live preview bottom bounds");
                expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview widths");
                expect_contains(process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#2267: stable detail header/footer expression selection should preserve live preview heights");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview availability");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview left bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview top bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview right bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview bottom bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview widths");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview heights");
                expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1768: stable detail header/footer expression selection should expose selected sections");
                expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1768: stable detail header/footer expression selection should expose section selections");
                expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1768: stable detail header/footer expression selection should not select report objects");
                expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1768: stable detail header/footer expression selection should not fabricate object sections");
                expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1768: stable detail header/footer expression selection should not select settings");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1768: stable live " + selection_label + " should expose section expression metadata");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1768: stable live " + selection_label + " should expose selected-section expression metadata");
            };

            expect_live_section("detail-header-guid",
                                "Detail Header",
                                "detail_header",
                                "detail header expression",
                                "1",
                                "0",
                                "0",
                                "9",
                                "0",
                                "300",
                                "300",
                                "detail header");
            expect_live_section("detail-footer-guid",
                                "Detail Footer",
                                "detail_footer",
                                "detail footer expression",
                                "2",
                                "1",
                                "1",
                                "10",
                                "300",
                                "250",
                                "550",
                                "detail footer");
        };

    run_detail_header_footer_stable_expression_selection(
        temp_root / "detail_header_footer_stable_expression.frx",
        "detail_header_footer_stable_expression.frx",
        "report");
    run_detail_header_footer_stable_expression_selection(
        temp_root / "detail_header_footer_stable_expression.lbx",
        "detail_header_footer_stable_expression.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_detail_header_footer_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto expect_selected_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_title,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& section_index,
                    const std::string& object_code,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1810: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#1810: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 2",
                                    "#1810: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                                    "#1810: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#1927: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                    "#1927: " + operation_label + " should preserve live preview left bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#1927: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                    "#1927: " + operation_label + " should preserve live preview right bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 550",
                                    "#1927: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                    "#1927: " + operation_label + " should preserve live preview widths");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 550",
                                    "#1927: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#1927: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                    "#1927: " + operation_label + " should preserve deleted preview left bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                    "#1927: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                    "#1927: " + operation_label + " should preserve deleted preview right bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                    "#1927: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                    "#1927: " + operation_label + " should preserve deleted preview widths");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                    "#1927: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#1810: " + operation_label + " should preserve section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#1810: " + operation_label + " should preserve selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#1810: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1810: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2263: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2263: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2263: " + operation_label + " JSON should expose undo availability");
                    expect_contains(process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                                    "#2263: " + operation_label + " JSON should expose expression undo labels");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"title\": \"" + section_title + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": false",
                            "\"sectionIndex\": " + section_index,
                            "\"sectionCount\": 2",
                            "\"objectCode\": " + object_code,
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#1810: " + operation_label + " should refresh selected-section expression metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "detail header stable updated",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1810: stable-selected detail-header expression update should exit successfully");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "detail header stable updated",
                   "#1810: stable-selected detail-header expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_header_process,
                                               "Detail Header",
                                               "detail_header",
                                               "detail header stable updated",
                                               "2",
                                               "4",
                                               "0",
                                               "0",
                                               "9",
                                               "0",
                                               "300",
                                               "300",
                                               "stable-selected detail-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"detail footer expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1"
                },
                "#1810: stable-selected detail-header expression update should preserve sibling footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "detail footer stable updated",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1810: stable-selected detail-footer expression update should exit successfully");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "detail footer stable updated",
                   "#1810: stable-selected detail-footer expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_footer_process,
                                               "Detail Footer",
                                               "detail_footer",
                                               "detail footer stable updated",
                                               "2",
                                               "5",
                                               "1",
                                               "1",
                                               "10",
                                               "300",
                                               "250",
                                               "550",
                                               "stable-selected detail-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"detail header stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 0"
                },
                "#1810: stable-selected detail-footer expression update should preserve sibling header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1810: stable-selected detail-header expression clear should exit successfully");
            expect_selected_section_expression(clear_header_process,
                                               "Detail Header",
                                               "detail_header",
                                               "",
                                               "null",
                                               "0",
                                               "0",
                                               "0",
                                               "9",
                                               "0",
                                               "300",
                                               "300",
                                               "stable-selected detail-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"detail footer stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 1"
                },
                "#1810: stable-selected detail-header expression clear should preserve sibling footer metadata");
            expect_not_contains(clear_header_process.stdout_text,
                                "\"expression\": \"detail header stable updated\"",
                                "#1810: stable-selected detail-header expression clear should not leak stale selected expressions");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1810: stable-selected detail-footer expression clear should exit successfully");
            expect_selected_section_expression(clear_footer_process,
                                               "Detail Footer",
                                               "detail_footer",
                                               "",
                                               "null",
                                               "0",
                                               "1",
                                               "1",
                                               "10",
                                               "300",
                                               "250",
                                               "550",
                                               "stable-selected detail-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 0"
                },
                "#1810: stable-selected detail-footer expression clear should preserve sibling header metadata");
            expect_not_contains(clear_footer_process.stdout_text,
                                "\"expression\": \"detail footer stable updated\"",
                                "#1810: stable-selected detail-footer expression clear should not leak stale selected expressions");
        };

    run_detail_header_footer_section_expression_stable_edits(
        temp_root / "detail_header_footer_section_stable_expression_edits.frx",
        "detail_header_footer_section_stable_expression_edits.frx",
        "report");
    run_detail_header_footer_section_expression_stable_edits(
        temp_root / "detail_header_footer_section_stable_expression_edits.lbx",
        "detail_header_footer_section_stable_expression_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_expression_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "1",
                    "--property-name", "EXPR",
                    "--property-value", "deleted detail header updated",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1766: deleted detail-header report/label expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#2264: deleted detail-header expression update should preserve deleted state");
            const auto updated_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = {},
                .property_name = "EXPR"
            });
            expect(updated_expr.ok && updated_expr.exists &&
                       updated_expr.value == "deleted detail header updated",
                   "#1766: deleted detail-header expression update should persist the EXPR memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1766: deleted detail-header expression update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1766: deleted detail-header expression update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"sectionCount\": 1",
                            "#1766: deleted detail-header expression update should preserve live section count");
            expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1766: deleted detail-header expression update should preserve deleted section count");
            expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1766: deleted detail-header expression update should preserve section selection");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1766: deleted detail-header expression update should preserve selection kind");
            expect_contains(update_process.stdout_text, "\"dryRun\": false",
                            "#2264: deleted detail-header expression update JSON should expose committed execution");
            expect_contains(update_process.stdout_text, "\"mutatesAsset\": true",
                            "#2264: deleted detail-header expression update JSON should expose mutation state");
            expect_contains(update_process.stdout_text, "\"undoAvailable\": true",
                            "#2264: deleted detail-header expression update JSON should expose undo availability");
            expect_contains(update_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2264: deleted detail-header expression update JSON should expose expression undo labels");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"deleted detail header updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"deleted detail footer expression\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2"
                },
                "#1766: deleted detail-header update should refresh deleted-section expression metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"deleted detail header updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"objectCodeFieldIndex\": 1",
                    "\"objectCodeMemoBlockNumber\": 0",
                    "\"top\": 500",
                    "\"height\": 300",
                    "\"bottom\": 800"
                },
                "#1766: deleted detail-header update should refresh selected-section expression metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"title\": \"Detail\"",
                    "\"bandKind\": \"detail\"",
                    "\"expression\": \"live detail expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 1",
                    "\"recordIndex\": 0"
                },
                "#1766: deleted detail-header expression update should preserve live detail metadata");

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
                std::cerr << "studio host " << label << " deleted detail-footer expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1766: deleted detail-footer report/label expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#2264: deleted detail-footer expression clear should preserve deleted state");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1766: deleted detail-footer expression clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1766: deleted detail-footer expression clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"sectionCount\": 1",
                            "#2264: deleted detail-footer expression clear should preserve live section count");
            expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#2264: deleted detail-footer expression clear should preserve deleted section count");
            expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1766: deleted detail-footer expression clear should preserve section selection");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1766: deleted detail-footer expression clear should preserve selection kind");
            expect_contains(clear_process.stdout_text, "\"dryRun\": false",
                            "#2264: deleted detail-footer expression clear JSON should expose committed execution");
            expect_contains(clear_process.stdout_text, "\"mutatesAsset\": true",
                            "#2264: deleted detail-footer expression clear JSON should expose mutation state");
            expect_contains(clear_process.stdout_text, "\"undoAvailable\": true",
                            "#2264: deleted detail-footer expression clear JSON should expose undo availability");
            expect_contains(clear_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2264: deleted detail-footer expression clear JSON should expose expression undo labels");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"deleted detail header updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2"
                },
                "#1766: deleted detail-footer clear should refresh deleted-section expression metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"objectCodeFieldIndex\": 1",
                    "\"objectCodeMemoBlockNumber\": 0",
                    "\"top\": 800",
                    "\"height\": 250",
                    "\"bottom\": 1050"
                },
                "#1766: deleted detail-footer clear should refresh selected-section expression metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"title\": \"Detail\"",
                    "\"bandKind\": \"detail\"",
                    "\"expression\": \"live detail expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 1",
                    "\"recordIndex\": 0"
                },
                "#1766: deleted detail-footer expression clear should preserve live detail metadata");
        };

    run_deleted_detail_header_footer_expression_edits(
        temp_root / "deleted_detail_header_footer_expression.frx",
        "deleted_detail_header_footer_expression.frx",
        "report");
    run_deleted_detail_header_footer_expression_edits(
        temp_root / "deleted_detail_header_footer_expression.lbx",
        "deleted_detail_header_footer_expression.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto expect_selected_deleted_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_title,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& object_code,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1811: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#1811: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 1",
                                    "#1811: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 2",
                                    "#1811: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#1928: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                    "#1928: " + operation_label + " should preserve live preview left bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#1928: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                    "#1928: " + operation_label + " should preserve live preview right bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 500",
                                    "#1928: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                    "#1928: " + operation_label + " should preserve live preview widths");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 500",
                                    "#1928: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#1928: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                    "#1928: " + operation_label + " should preserve deleted preview left bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                    "#1928: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                    "#1928: " + operation_label + " should preserve deleted preview right bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                    "#1928: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                    "#1928: " + operation_label + " should preserve deleted preview widths");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                    "#1928: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#1811: " + operation_label + " should preserve deleted section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#1811: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#1811: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1811: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2265: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2265: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2265: " + operation_label + " JSON should expose undo availability");
                    expect_contains(process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                                    "#2265: " + operation_label + " JSON should expose expression undo labels");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"title\": \"" + section_title + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"sectionIndex\": null",
                            "\"sectionCount\": 0",
                            "\"objectCode\": " + object_code,
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#1811: " + operation_label + " should refresh selected deleted-section expression metadata");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"sections\": [",
                            "\"title\": \"Detail\"",
                            "\"bandKind\": \"detail\"",
                            "\"expression\": \"live detail expression\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": 1",
                            "\"recordIndex\": 0"
                        },
                        "#1811: " + operation_label + " should preserve live detail metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "deleted detail header stable updated",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-header expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1811: stable-selected deleted detail-header expression update should preserve deleted state");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "deleted detail header stable updated",
                   "#1811: stable-selected deleted detail-header expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_header_process,
                                                       "Detail Header",
                                                       "detail_header",
                                                       "deleted detail header stable updated",
                                                       "2",
                                                       "4",
                                                       "1",
                                                       "9",
                                                       "500",
                                                       "300",
                                                       "800",
                                                       "stable-selected deleted detail-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"deleted detail header stable updated\"",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"deleted detail footer expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-header update should preserve deleted footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "deleted detail footer stable updated",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-footer expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1811: stable-selected deleted detail-footer expression update should preserve deleted state");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "deleted detail footer stable updated",
                   "#1811: stable-selected deleted detail-footer expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_footer_process,
                                                       "Detail Footer",
                                                       "detail_footer",
                                                       "deleted detail footer stable updated",
                                                       "2",
                                                       "5",
                                                       "2",
                                                       "10",
                                                       "800",
                                                       "250",
                                                       "1050",
                                                       "stable-selected deleted detail-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"deleted detail header stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"deleted detail footer stable updated\"",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-footer update should preserve deleted header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-header expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1811: stable-selected deleted detail-header expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_header_process,
                                                       "Detail Header",
                                                       "detail_header",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "1",
                                                       "9",
                                                       "500",
                                                       "300",
                                                       "800",
                                                       "stable-selected deleted detail-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"deleted detail footer stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-header clear should preserve deleted footer metadata");
            expect_not_contains(clear_header_process.stdout_text,
                                "\"expression\": \"deleted detail header stable updated\"",
                                "#1811: stable-selected deleted detail-header clear should not leak stale selected expressions");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-footer expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1811: stable-selected deleted detail-footer expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_footer_process,
                                                       "Detail Footer",
                                                       "detail_footer",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "2",
                                                       "10",
                                                       "800",
                                                       "250",
                                                       "1050",
                                                       "stable-selected deleted detail-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-footer clear should preserve deleted header metadata");
            expect_not_contains(clear_footer_process.stdout_text,
                                "\"expression\": \"deleted detail footer stable updated\"",
                                "#1811: stable-selected deleted detail-footer clear should not leak stale selected expressions");
        };

    run_deleted_detail_header_footer_section_expression_stable_edits(
        temp_root / "deleted_detail_header_footer_section_stable_expression_edits.frx",
        "deleted_detail_header_footer_section_stable_expression_edits.frx",
        "report");
    run_deleted_detail_header_footer_section_expression_stable_edits(
        temp_root / "deleted_detail_header_footer_section_stable_expression_edits.lbx",
        "deleted_detail_header_footer_section_stable_expression_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_stable_section_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_stable_selection =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto expect_deleted_section = [&](const std::string& unique_id,
                                                    const std::string& section_title,
                                                    const std::string& band_kind,
                                                    const std::string& expression,
                                                    const std::string& memo_block,
                                                    const std::string& record_index,
                                                    const std::string& object_code,
                                                    const std::string& top,
                                                    const std::string& height,
                                                    const std::string& bottom,
                                                    const std::string& selection_label) {
                const auto process = run_process_capture(
                    studio_host_path,
                    {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                    temp_root);

                if (process.exit_code != 0) {
                    std::cerr << "studio host " << label << " stable deleted " << selection_label
                              << " stdout:\n" << process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " stable deleted " << selection_label
                              << " stderr:\n" << process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(process.exit_code == 0,
                       "#1767: stable deleted detail header/footer section selection should exit successfully");
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                "#1767: stable deleted detail header/footer selection should preserve document titles");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1767: stable deleted detail header/footer label selection should retain label identity");
                }
                expect_contains(process.stdout_text, "\"sectionCount\": 1",
                                "#1767: stable deleted detail header/footer selection should preserve live sections");
                expect_contains(process.stdout_text, "\"deletedSectionCount\": 2",
                                "#1767: stable deleted detail header/footer selection should expose deleted sections");
                expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#2266: stable deleted detail header/footer selection should preserve live preview availability");
                expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview left bounds");
                expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview top bounds");
                expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview right bounds");
                expect_contains(process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#2266: stable deleted detail header/footer selection should preserve live preview bottom bounds");
                expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview widths");
                expect_contains(process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#2266: stable deleted detail header/footer selection should preserve live preview heights");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview availability");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview left bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview top bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview right bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview bottom bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview widths");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview heights");
                expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1767: stable deleted detail header/footer selection should expose selected sections");
                expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1767: stable deleted detail header/footer selection should expose section selections");
                expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1767: stable deleted detail header/footer selection should not select report objects");
                expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1767: stable deleted detail header/footer selection should not fabricate object sections");
                expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1767: stable deleted detail header/footer selection should not select settings");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1767: stable deleted " + selection_label + " should expose deleted-section metadata");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1767: stable deleted " + selection_label + " should expose selected-section metadata");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"expression\": \"live detail expression\"",
                        "\"recordIndex\": 0"
                    },
                    "#1767: stable deleted " + selection_label + " should preserve live detail metadata");
            };

            expect_deleted_section("deleted-detail-header-guid",
                                   "Detail Header",
                                   "detail_header",
                                   "deleted detail header expression",
                                   "2",
                                   "1",
                                   "9",
                                   "500",
                                   "300",
                                   "800",
                                   "detail header");
            expect_deleted_section("deleted-detail-footer-guid",
                                   "Detail Footer",
                                   "detail_footer",
                                   "deleted detail footer expression",
                                   "3",
                                   "2",
                                   "10",
                                   "800",
                                   "250",
                                   "1050",
                                   "detail footer");
        };

    run_deleted_detail_header_footer_stable_selection(
        temp_root / "deleted_detail_header_footer_stable_sections.frx",
        "deleted_detail_header_footer_stable_sections.frx",
        "report");
    run_deleted_detail_header_footer_stable_selection(
        temp_root / "deleted_detail_header_footer_stable_sections.lbx",
        "deleted_detail_header_footer_stable_sections.lbx",
        "label");

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
