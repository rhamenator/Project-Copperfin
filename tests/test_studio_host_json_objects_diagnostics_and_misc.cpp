// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HELPERS)
void write_synthetic_report_table_for_extended_object_kind_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1", "", "", "", "", "", "10", ""},
        {"9", "4", "", "", "0", "", "5000", "", "", ""},
        {"7", "", "", "100", "500", "1000", "400", "", "", "rectangle-guid"},
        {"17", "", "images/logo.bmp", "300", "1200", "900", "800", "", "", "picture-guid"},
        {"18", "", "nTotal", "250", "7000", "800", "200", "", "", "variable-guid"},
        {"17", "", "images/deleted.bmp", "400", "1500", "700", "300", "", "", "deleted-picture-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1762: synthetic report table for extended object-kind JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 5U, true);
    expect(delete_result.ok, "#1762: synthetic report table should mark deleted picture object");
}
#endif

#if !defined(COPPERFIN_REPORT_OBJECT_FALLBACKS_SKIP_HELPERS)
void write_synthetic_report_table_for_missing_object_objcode_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"8", "customer.name", "120", "300", "700", "90", "missing-objcode-live-object-guid"},
        {"5", "\"Deleted no objcode\"", "260", "620", "500", "120",
         "missing-objcode-deleted-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1729: synthetic report table without object OBJCODE schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1729: synthetic report table should mark the no-OBJCODE object deleted");
}

void write_synthetic_report_table_for_unresolved_deleted_object_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "FONTSIZE", .type = 'C', .length = 24U},
        {.name = "MODE", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "<memo block 50>", "1200", "2600", "4000", "450",
         "<memo block 51>", "<memo block 52>", "<memo block 53>"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1738: synthetic report table with unresolved deleted object memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1738: synthetic report table should mark unresolved object memo deleted");
}

void write_synthetic_report_table_for_unresolved_unplaced_object_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "FONTSIZE", .type = 'C', .length = 24U},
        {.name = "MODE", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"8", "0", "<memo block 60>", "1200", "2600", "4000", "450",
         "<memo block 61>", "<memo block 62>", "<memo block 63>"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1739: synthetic report table with unresolved unplaced object memo placeholders should be created");
}

void write_synthetic_report_table_for_missing_object_expr_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "4", "", "200", "", "1000", "missing-expr-detail-section-guid"},
        {"8", "0", "120", "300", "700", "90", "missing-expr-live-object-guid"},
        {"5", "", "260", "620", "500", "120", "missing-expr-deleted-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1725: synthetic report table without object EXPR schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#1725: synthetic report table should mark the no-EXPR object deleted");
}

void write_synthetic_report_table_for_missing_object_title_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "4", "", "100", "", "800"},
        {"8", "0", "140", "220", "420", "80"},
        {"5", "", "360", "500", "380", "110"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1726: synthetic report table without object title schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#1726: synthetic report table should mark the no-title object deleted");
}
#endif

#if !defined(COPPERFIN_REPORT_DELETED_STATES_SKIP_HELPERS)
void write_synthetic_report_table_for_layout_subtree_deleted_state_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ReportSettings", "ReportSettings", "", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "DetailBand", "DetailBand", "", "", "", "2000", "", "5000", ""},
        {"8", "0", "LeftField", "LeftField", "", "left.value", "100", "2600", "50", "200", "left-field-guid"},
        {"8", "0", "MiddleField", "MiddleField", "", "middle.value", "100", "2600", "50", "200", "middle-field-guid"},
        {"8", "0", "RightField", "RightField", "", "right.value", "100", "2600", "50", "200", "right-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1857: synthetic FRX/LBX table for report layout subtree deleted-state should be created");
}
#endif

#if !defined(COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HOST_SMOKE)
void test_studio_host_json_exposes_report_layout_provenance(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_provenance_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host report layout stdout:\n" << process.stdout_text << "\n";
        std::cerr << "studio host report layout stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "#1452: report layout provenance JSON smoke should exit successfully");
    expect_contains(process.stdout_text, "\"reportLayout\": {",
                    "#1452: report documents should expose report layout JSON");
    expect_contains(process.stdout_text, "\"documentTitle\": \"summary.frx\"",
                    "#1452: report layout JSON should preserve document titles");
    expect_contains(process.stdout_text, "\"documentTitleFieldIndex\": 10",
                    "#4248: report layout JSON should expose header NAME field provenance");
    expect_contains(process.stdout_text, "\"documentTitleMemoBlockNumber\": 0",
                    "#4248: report layout JSON should expose header NAME memo provenance");
    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1516: report layout JSON should expose preview bounds availability");
    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1516: report layout JSON should include section-origin preview bounds");
    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1516: report layout JSON should expose top preview bounds across live sections and objects");
    expect_contains(process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1516: report layout JSON should expose right preview bounds across live objects");
    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1516: report layout JSON should expose bottom preview bounds including live unplaced objects");
    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1516: report layout JSON should expose computed preview bounds width");
    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1516: report layout JSON should expose computed preview bounds height");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1524: report layout JSON should expose deleted preview bounds availability");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1524: report layout JSON should expose deleted preview left bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1524: report layout JSON should expose deleted preview top bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1524: report layout JSON should expose deleted preview right bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1524: report layout JSON should expose deleted preview bottom bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1524: report layout JSON should expose deleted preview width");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1524: report layout JSON should expose deleted preview height");
    expect_contains(process.stdout_text, "\"pageSetupAvailable\": true",
                    "#1517: report layout JSON should expose page setup summary availability");
    expect_contains(process.stdout_text, "\"orientationAvailable\": true",
                    "#1517: report layout JSON should expose orientation summary availability");
    expect_contains(process.stdout_text, "\"orientationCode\": 0",
                    "#1517: report layout JSON should expose orientation codes");
    expect_contains(process.stdout_text, "\"paperSizeAvailable\": true",
                    "#1517: report layout JSON should expose paper-size summary availability");
    expect_contains(process.stdout_text, "\"paperSizeCode\": 1",
                    "#1517: report layout JSON should expose paper-size codes");
    expect_contains(process.stdout_text, "\"topMarginAvailable\": true",
                    "#1517: report layout JSON should expose top-margin summary availability");
    expect_contains(process.stdout_text, "\"topMargin\": 10",
                    "#1517: report layout JSON should expose top margins");
    expect_contains(process.stdout_text, "\"bottomMarginAvailable\": true",
                    "#1517: report layout JSON should expose bottom-margin summary availability");
    expect_contains(process.stdout_text, "\"bottomMargin\": 20",
                    "#1517: report layout JSON should expose bottom margins");
    expect_contains(process.stdout_text, "\"gridVerticalAvailable\": true",
                    "#1517: report layout JSON should expose vertical-grid summary availability");
    expect_contains(process.stdout_text, "\"gridVertical\": 4",
                    "#1517: report layout JSON should expose vertical grid spacing");
    expect_contains(process.stdout_text, "\"gridHorizontalAvailable\": true",
                    "#1517: report layout JSON should expose horizontal-grid summary availability");
    expect_contains(process.stdout_text, "\"gridHorizontal\": 8",
                    "#1517: report layout JSON should expose horizontal grid spacing");
    expect_contains(process.stdout_text, "\"liveObjectCount\": 3",
                    "#1516: report layout JSON should summarize live placed and unplaced object counts");
    expect_contains(process.stdout_text, "\"placedObjectCount\": 2",
                    "#1522: report layout JSON should summarize section-contained live object counts");
    expect_contains(process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                    "#1523: report layout JSON should summarize deleted objects still inside section bands");
    expect_contains(process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1523: report layout JSON should not fabricate deleted unplaced object counts");
    const fs::path deleted_unplaced_path = temp_root / "deleted_unplaced_summary.frx";
    write_synthetic_report_table_for_layout_json(deleted_unplaced_path);
    const auto delete_unplaced_result = copperfin::vfp::set_record_deleted_flag(deleted_unplaced_path.string(), 5U, true);
    expect(delete_unplaced_result.ok,
           "#1523: synthetic report layout should mark an unplaced object deleted");
    const auto deleted_unplaced_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_unplaced_path.string(), "--json"},
        temp_root);
    expect(deleted_unplaced_process.exit_code == 0,
           "#1523: deleted-unplaced report layout JSON should exit successfully");
    expect_contains(deleted_unplaced_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                    "#1523: deleted-unplaced report layout JSON should retain deleted placed object counts");
    expect_contains(deleted_unplaced_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                    "#1523: deleted-unplaced report layout JSON should count deleted objects outside section bands");
    expect_contains(process.stdout_text, "\"objectKindCount\": 3",
                    "#1519: report layout JSON should summarize live object-kind count buckets");
    expect_contains(process.stdout_text, "\"objectKindCounts\": [",
                    "#1519: report layout JSON should expose live object-kind count summaries");
    expect_contains(process.stdout_text, "{\"kind\": \"field\", \"count\": 1}",
                    "#1519: report layout JSON should count live field objects");
    expect_contains(process.stdout_text, "{\"kind\": \"label\", \"count\": 1}",
                    "#1519: report layout JSON should count live label objects");
    expect_contains(process.stdout_text, "{\"kind\": \"line\", \"count\": 1}",
                    "#1519: report layout JSON should count live unplaced line objects");
    expect_contains(process.stdout_text, "\"unplacedObjectKindCount\": 1",
                    "#1519: report layout JSON should summarize unplaced object-kind count buckets");
    expect_contains(process.stdout_text, "\"unplacedObjectKindCounts\": [",
                    "#1519: report layout JSON should expose unplaced object-kind count summaries");
    expect_contains(process.stdout_text, "\"unplacedObjectKindCounts\": [\n        {\"kind\": \"line\", \"count\": 1}\n      ]",
                    "#1519: report layout JSON should count unplaced line objects");
    expect_contains(process.stdout_text, "\"deletedObjectKindCount\": 1",
                    "#1519: report layout JSON should summarize deleted object-kind count buckets");
    expect_contains(process.stdout_text, "\"deletedObjectKindCounts\": [",
                    "#1519: report layout JSON should expose deleted object-kind count summaries");
    expect_contains(process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                    "#1519: report layout JSON should count deleted label objects");
    expect_contains(process.stdout_text, "\"sectionKindCount\": 2",
                    "#1520: report layout JSON should summarize live section band-kind buckets");
    expect_contains(process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"detail\", \"count\": 1},\n        {\"kind\": \"page_header\", \"count\": 1}\n      ]",
                    "#1520: report layout JSON should count live detail and page-header sections");
    expect_contains(process.stdout_text, "\"deletedSectionKindCount\": 0",
                    "#1520: report layout JSON should not fabricate deleted section band-kind buckets");
    expect_contains(process.stdout_text, "\"sectionHeightTotal\": 7000",
                    "#1521: report layout JSON should summarize live section heights");
    expect_contains(process.stdout_text, "\"deletedSectionHeightTotal\": 0",
                    "#1521: report layout JSON should not fabricate deleted section heights");
    expect_contains(process.stdout_text, "\"settingCount\": 6",
                    "#1452: report layout JSON should summarize live setting counts");
    expect_contains(process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1452: report layout JSON should summarize deleted report object counts");
    expect_contains(process.stdout_text, "\"deletedObjects\": [",
                    "#1452: report layout JSON should expose deleted report objects separately");
    expect_contains(process.stdout_text, "\"deleted\": true",
                    "#1452: report layout JSON should retain deleted report object state");
    expect_contains(process.stdout_text, "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
                    "#1452: report layout JSON should expose memo-line setting provenance");
    expect_contains(process.stdout_text, "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"1\"",
                    "#1452: report layout JSON should expose later memo-line setting provenance");
    expect_contains(process.stdout_text, "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"10\"",
                    "#1452: report layout JSON should expose direct setting provenance");
    expect_contains(process.stdout_text, "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \"20\"",
                    "#1517: report layout JSON should expose bottom-margin setting provenance");
    expect_contains(process.stdout_text, "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3, \"memoBlockNumber\": 1, \"value\": \"4\"",
                    "#1517: report layout JSON should expose vertical-grid setting provenance");
    expect_contains(process.stdout_text, "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4, \"memoBlockNumber\": 1, \"value\": \"8\"",
                    "#1517: report layout JSON should expose horizontal-grid setting provenance");
    expect_contains(process.stdout_text, "\"sectionCount\": 2",
                    "#1452: report layout JSON should summarize live section counts");
    expect_contains(process.stdout_text, "\"id\": \"page_header_1\"",
                    "#1452: report layout JSON should expose synthesized section ids");
    expect_contains(process.stdout_text, "\"idFieldIndex\": null",
                    "#1452: report layout JSON should expose synthesized section id provenance as null");
    expect_contains(process.stdout_text, "\"objectCode\": 1",
                    "#1452: report layout JSON should expose section raw object codes");
    expect_contains(process.stdout_text, "\"sectionIndex\": 0",
                    "#1460: report layout JSON should expose section order");
    expect_contains(process.stdout_text, "\"sectionCount\": 2",
                    "#1460: report layout JSON should expose live section counts");
    expect_contains(process.stdout_text, "\"objectCodeFieldIndex\": 1",
                    "#1452: report layout JSON should expose section object-code field provenance");
    expect_contains(process.stdout_text, "\"topFieldIndex\": 4",
                    "#1452: report layout JSON should expose section top field provenance");
    expect_contains(process.stdout_text, "\"bottom\": 2000",
                    "#1461: report layout JSON should expose section bottom-edge coordinates");
    expect_contains(process.stdout_text, "\"objectCount\": 1",
                    "#1452: report layout JSON should summarize section object counts");
    expect_contains(process.stdout_text, "\"objectTypeCode\": 8",
                    "#1452: report layout JSON should expose report object raw type codes");
    expect_contains(process.stdout_text, "\"objectKind\": \"field\"",
                    "#1452: report layout JSON should expose report object kinds");
    expect_contains(process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1458: report layout JSON should expose object containing section ids");
    expect_contains(process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1458: report layout JSON should expose object containing section record indexes");
    expect_contains(process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1458: report layout JSON should expose object top coordinates relative to containing sections");
    expect_contains(process.stdout_text, "\"sectionRelativeBottom\": 1050",
                    "#1461: report layout JSON should expose object bottom coordinates relative to containing sections");
    expect_contains(process.stdout_text, "\"sectionObjectIndex\": 0",
                    "#1459: report layout JSON should expose object order inside containing sections");
    expect_contains(process.stdout_text, "\"sectionObjectCount\": 1",
                    "#1459: report layout JSON should expose containing section object counts");
    expect_contains(process.stdout_text, "\"expression\": \"customer.company\"",
                    "#1452: report layout JSON should expose report object expressions");
    expect_contains(process.stdout_text, "\"expressionFieldIndex\": 2",
                    "#1452: report layout JSON should expose expression field provenance");
    expect_contains(process.stdout_text, "\"expressionMemoBlockNumber\": 2",
                    "#1452: report layout JSON should expose expression memo provenance");
    expect_contains(process.stdout_text, "\"leftFieldIndex\": 3",
                    "#1452: report layout JSON should expose object left field provenance");
    expect_contains(process.stdout_text, "\"right\": 5200",
                    "#1462: report layout JSON should expose object right-edge coordinates");
    expect_contains(process.stdout_text, "\"bottom\": 3050",
                    "#1461: report layout JSON should expose object bottom-edge coordinates");
    expect_contains(process.stdout_text, "\"highlightCount\": 1",
                    "#1452: report layout JSON should summarize object highlights");
    expect_contains(process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3, \"fieldIndex\": 7, \"sourceLineIndex\": null, \"memoBlockNumber\": 3, \"value\": \"Segoe UI\"",
                    "#1452: report layout JSON should expose highlight memo provenance");
    expect_contains(process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1452: report layout JSON should summarize unplaced objects");
    expect_contains(process.stdout_text, "\"containingSectionRecordIndex\": null",
                    "#1458: unplaced/deleted report object JSON should expose null containing section record indexes");
    expect_contains(process.stdout_text, "\"sectionObjectIndex\": null",
                    "#1459: unplaced/deleted report object JSON should expose null section object indexes");
    expect_contains(process.stdout_text, "\"title\": \"Record 5\"",
                    "#1452: report layout JSON should preserve synthesized unplaced-object titles");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_extended_report_object_kinds(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_extended_report_object_kind_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_extended_kind_summary = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_extended_object_kind_json(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " extended object-kind summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " extended object-kind summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1762: extended report/label object-kind JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1762: extended report/label object-kind JSON should return report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1762: extended object-kind label layouts should retain label identity");
        }
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2311: extended object-kind JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2311: extended object-kind JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2311: extended object-kind JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 1200",
                        "#2311: extended object-kind JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 7200",
                        "#2311: extended object-kind JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 1200",
                        "#2311: extended object-kind JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 7200",
                        "#2311: extended object-kind JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2311: extended object-kind JSON should preserve deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 400",
                        "#2311: extended object-kind JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 1500",
                        "#2311: extended object-kind JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 1100",
                        "#2311: extended object-kind JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 1800",
                        "#2311: extended object-kind JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 700",
                        "#2311: extended object-kind JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#2311: extended object-kind JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 3",
                        "#1762: extended object-kind JSON should summarize live objects");
        expect_contains(process.stdout_text, "\"placedObjectCount\": 2",
                        "#1762: extended object-kind JSON should summarize placed live objects");
        expect_contains(process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1762: extended object-kind JSON should summarize unplaced live objects");
        expect_contains(process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1762: extended object-kind JSON should summarize deleted objects");
        expect_contains(process.stdout_text, "\"objectKindCount\": 3",
                        "#1762: extended object-kind JSON should expose live kind bucket count");
        expect_contains(process.stdout_text,
                        "\"objectKindCounts\": [\n"
                        "        {\"kind\": \"picture\", \"count\": 1},\n"
                        "        {\"kind\": \"rectangle\", \"count\": 1},\n"
                        "        {\"kind\": \"variable\", \"count\": 1}\n"
                        "      ]",
                        "#1762: extended object-kind JSON should count live picture/rectangle/variable buckets");
        expect_contains(process.stdout_text, "\"unplacedObjectKindCount\": 1",
                        "#1762: extended object-kind JSON should expose unplaced kind bucket count");
        expect_contains(process.stdout_text,
                        "\"unplacedObjectKindCounts\": [\n"
                        "        {\"kind\": \"variable\", \"count\": 1}\n"
                        "      ]",
                        "#1762: extended object-kind JSON should count unplaced variable objects");
        expect_contains(process.stdout_text, "\"deletedObjectKindCount\": 1",
                        "#1762: extended object-kind JSON should expose deleted kind bucket count");
        expect_contains(process.stdout_text,
                        "\"deletedObjectKindCounts\": [\n"
                        "        {\"kind\": \"picture\", \"count\": 1}\n"
                        "      ]",
                        "#1762: extended object-kind JSON should count deleted picture objects");

        const auto expect_selected_kind = [&](const std::string& unique_id,
                                              const std::string& object_type_code,
                                              const std::string& object_kind,
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
                   "#1762: selected extended report/label object-kind JSON should exit successfully");
            expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1762: selected extended object-kind JSON should advertise selected objects");
            expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1762: selected extended object-kind JSON should expose object selection kind");
            expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2311: selected extended object-kind JSON should preserve live preview availability");
            expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1200",
                            "#2311: selected extended object-kind JSON should preserve live preview right bounds");
            expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 7200",
                            "#2311: selected extended object-kind JSON should preserve live preview bottom bounds");
            expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2311: selected extended object-kind JSON should preserve deleted preview availability");
            expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 400",
                            "#2311: selected extended object-kind JSON should preserve deleted preview left bounds");
            expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 1500",
                            "#2311: selected extended object-kind JSON should preserve deleted preview top bounds");
            expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1100",
                            "#2311: selected extended object-kind JSON should preserve deleted preview right bounds");
            expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1800",
                            "#2311: selected extended object-kind JSON should preserve deleted preview bottom bounds");
            expect_contains_in_order(
                object_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"objectTypeCode\": " + object_type_code,
                    "\"objectKind\": \"" + object_kind + "\""
                },
                "#1762: selected " + selection_label + " JSON should expose extended object-kind metadata");
        };

        expect_selected_kind("rectangle-guid", "7", "rectangle", "rectangle");
        expect_selected_kind("picture-guid", "17", "picture", "picture");
        expect_selected_kind("variable-guid", "18", "variable", "variable");
    };

    run_extended_kind_summary(temp_root / "extended_object_kinds.frx",
                              "extended_object_kinds.frx",
                              "report");
    run_extended_kind_summary(temp_root / "extended_object_kinds.lbx",
                              "extended_object_kinds.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif

#if !defined(COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_ONLY)

#include "test_studio_host_json_report_deleted_states.inl"

#if !defined(COPPERFIN_REPORT_DELETED_STATES_ONLY)

#if !defined(COPPERFIN_REPORT_SCHEMA_FALLBACK_SKIP_HOST_SMOKE)
void test_studio_host_json_defaults_missing_report_object_objcode_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_object_objcode_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_object_objcode_layout = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_missing_object_objcode_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing object OBJCODE summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing object OBJCODE summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1729: missing object OBJCODE schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1729: missing object OBJCODE layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1729: missing object OBJCODE label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1729: missing object OBJCODE live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 300",
                        "#1729: missing object OBJCODE live preview top should come from the object");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 390",
                        "#1729: missing object OBJCODE live preview bottom should come from the object");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1729: missing object OBJCODE deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 620",
                        "#1729: missing object OBJCODE deleted preview top should come from the deleted object");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 740",
                        "#1729: missing object OBJCODE deleted preview bottom should come from the deleted object");
        expect_missing_object_objcode_preview_bounds(
            summary_process.stdout_text,
            "#2343: missing object OBJCODE summary JSON");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1729: missing object OBJCODE layouts should not fabricate live section membership");
        expect_contains(summary_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should count sectionless live objects");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 0",
                        "#1729: missing object OBJCODE layouts should not fabricate deleted section membership");
        expect_contains(summary_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                        "#1729: missing object OBJCODE layouts should count sectionless deleted objects");
        expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [\n        {\"kind\": \"field\", \"count\": 1}\n      ]",
                        "#1729: missing live object OBJCODE layouts should preserve OBJTYPE-derived kinds");
        expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                        "#1729: missing deleted object OBJCODE layouts should preserve OBJTYPE-derived kinds");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"unplacedObjects\": [",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"objectTypeCode\": 8",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.name\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"customer.name\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390"
            },
            "#1729: missing live object OBJCODE should serialize default object-code provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"objectTypeCode\": 5",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"label\"",
                "\"title\": \"\\\"Deleted no objcode\\\"\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"\\\"Deleted no objcode\\\"\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740"
            },
            "#1729: missing deleted object OBJCODE should serialize default object-code provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1729: missing object OBJCODE live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1729: missing object OBJCODE live selection should advertise selected objects");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1729: missing object OBJCODE live selection should expose object selection kind");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1729: missing object OBJCODE live selection should not fabricate containing sections");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1729: missing object OBJCODE live selection should serialize null containing sections");
        expect_missing_object_objcode_preview_bounds(
            live_process.stdout_text,
            "#2316: selected missing object OBJCODE live JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionObjectIndex\": null",
                "\"objectTypeCode\": 8",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.name\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"customer.name\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390"
            },
            "#1729: missing live object OBJCODE selection should expose default object-code metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1729: missing object OBJCODE deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1729: missing object OBJCODE deleted selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1729: missing object OBJCODE deleted selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1729: missing object OBJCODE deleted selection should not fabricate containing sections");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1729: missing object OBJCODE deleted selection should serialize null containing sections");
        expect_missing_object_objcode_preview_bounds(
            deleted_process.stdout_text,
            "#2316: selected missing object OBJCODE deleted JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionObjectIndex\": null",
                "\"objectTypeCode\": 5",
                "\"objectTypeFieldIndex\": 0",
                "\"objectCode\": 0",
                "\"objectCodeFieldIndex\": null",
                "\"objectKind\": \"label\"",
                "\"title\": \"\\\"Deleted no objcode\\\"\"",
                "\"titleFieldIndex\": 1",
                "\"expression\": \"\\\"Deleted no objcode\\\"\"",
                "\"expressionFieldIndex\": 1",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740"
            },
            "#1729: missing deleted object OBJCODE selection should expose default object-code metadata");
    };

    run_missing_object_objcode_layout(temp_root / "missing_object_objcode.frx",
                                      "missing_object_objcode.frx",
                                      "report");
    run_missing_object_objcode_layout(temp_root / "missing_object_objcode.lbx",
                                      "missing_object_objcode.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_UNRESOLVED_MEMO_SKIP_HOST_SMOKE)
void test_studio_host_json_suppresses_unresolved_deleted_report_object_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_deleted_report_object_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_unresolved_deleted_object_memo_layout = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_unresolved_deleted_object_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved deleted object memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved deleted object memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1738: unresolved deleted object memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1738: unresolved deleted object memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1738: unresolved deleted object memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1738: unresolved deleted object memo layouts should not fabricate live objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1738: unresolved deleted object memo layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1738: unresolved deleted object memo layouts should preserve deleted placed-object counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1738: unresolved deleted object memo summary should suppress expression/highlight text while preserving object metadata");
        expect_unresolved_deleted_object_memo_preview_bounds(
            summary_process.stdout_text,
            "#2334: unresolved deleted object memo summary JSON");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1738: unresolved deleted object memo placeholders should not leak into summary JSON");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1738: unresolved deleted object memo selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1738: unresolved deleted object memo selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1738: unresolved deleted object memo selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1738: unresolved deleted object memo selection should preserve containing-section metadata");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1738: unresolved deleted object memo selection should suppress expression/highlight text while preserving object metadata");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_0\"",
                "\"title\": \"Detail\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 0",
                "\"deletedObjectCount\": 1"
            },
            "#1738: unresolved deleted object memo selection should expose containing detail-band metadata");
        expect_unresolved_deleted_object_memo_preview_bounds(
            deleted_process.stdout_text,
            "#2334: selected unresolved deleted object memo JSON");
        expect_not_contains(deleted_process.stdout_text, "<memo block",
                            "#1738: unresolved deleted object memo placeholders should not leak into selection JSON");
    };

    run_unresolved_deleted_object_memo_layout(temp_root / "unresolved_deleted_object_memo.frx",
                                              "unresolved_deleted_object_memo.frx",
                                              "report");
    run_unresolved_deleted_object_memo_layout(temp_root / "unresolved_deleted_object_memo.lbx",
                                              "unresolved_deleted_object_memo.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_suppresses_unresolved_unplaced_report_object_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_unplaced_report_object_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_unresolved_unplaced_object_memo_layout = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_synthetic_report_table_for_unresolved_unplaced_object_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved unplaced object memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved unplaced object memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1739: unresolved unplaced object memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1739: unresolved unplaced object memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1739: unresolved unplaced object memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1739: unresolved unplaced object memo layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1739: unresolved unplaced object memo layouts should not fabricate placed objects");
        expect_contains(summary_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1739: unresolved unplaced object memo layouts should preserve unplaced object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1739: unresolved unplaced object memo layouts should not fabricate deleted objects");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"unplacedObjects\": [",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 0\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1739: unresolved unplaced object memo summary should suppress expression/highlight text while preserving object metadata");
        expect_unresolved_unplaced_object_memo_preview_bounds(
            summary_process.stdout_text,
            "#2335: unresolved unplaced object memo summary JSON");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1739: unresolved unplaced object memo placeholders should not leak into summary JSON");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(object_process.exit_code == 0,
               "#1739: unresolved unplaced object memo selection should keep inspection non-failing");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1739: unresolved unplaced object memo selection should advertise selected objects");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1739: unresolved unplaced object memo selection should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1739: unresolved unplaced object memo selection should not fabricate containing sections");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1739: unresolved unplaced object memo selection should serialize null containing sections");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 0\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2",
                "\"left\": 1200",
                "\"top\": 2600",
                "\"width\": 4000",
                "\"height\": 450",
                "\"highlightCount\": 0"
            },
            "#1739: unresolved unplaced object memo selection should suppress expression/highlight text while preserving object metadata");
        expect_unresolved_unplaced_object_memo_preview_bounds(
            object_process.stdout_text,
            "#2335: selected unresolved unplaced object memo JSON");
        expect_not_contains(object_process.stdout_text, "<memo block",
                            "#1739: unresolved unplaced object memo placeholders should not leak into selection JSON");
    };

    run_unresolved_unplaced_object_memo_layout(temp_root / "unresolved_unplaced_object_memo.frx",
                                               "unresolved_unplaced_object_memo.frx",
                                               "report");
    run_unresolved_unplaced_object_memo_layout(temp_root / "unresolved_unplaced_object_memo.lbx",
                                               "unresolved_unplaced_object_memo.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_SCHEMA_FALLBACK_SKIP_HOST_SMOKE)
void test_studio_host_json_preserves_report_objects_without_expr_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_object_expr_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_object_expr_layout = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_missing_object_expr_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing object EXPR summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing object EXPR summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1725: missing object EXPR schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1725: missing object EXPR layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1725: missing object EXPR label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1725: missing object EXPR live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 200",
                        "#1725: missing object EXPR live preview top should include the section band");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 1200",
                        "#1725: missing object EXPR live preview bottom should include the section band");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1725: missing object EXPR deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 260",
                        "#1725: missing object EXPR deleted preview left should come from the deleted object");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 740",
                        "#1725: missing object EXPR deleted preview bottom should come from the deleted object");
        expect_missing_object_expr_preview_bounds(
            summary_process.stdout_text,
            "#2339: missing object EXPR summary JSON");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1725: missing object EXPR layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1725: missing object EXPR layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1725: missing object EXPR layouts should keep live object section membership");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1725: missing object EXPR layouts should preserve deleted placed-object counts");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1725: missing object EXPR layouts should preserve the containing live section");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1725: missing object EXPR layouts should not fabricate deleted sections");
        expect_contains(summary_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                        "#1725: missing object EXPR layouts should not fabricate deleted unplaced objects");
        expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [\n        {\"kind\": \"field\", \"count\": 1}\n      ]",
                        "#1725: missing live object EXPR layouts should preserve object-kind counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                        "#1725: missing deleted object EXPR layouts should preserve object-kind counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"missing-expr-detail-section-guid\"",
                "\"objects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"missing-expr-live-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390",
                "\"highlightCount\": 0"
            },
            "#1725: missing live object EXPR layouts should serialize null expression provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 420",
                "\"sectionRelativeBottom\": 540",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"missing-expr-deleted-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740",
                "\"highlightCount\": 0"
            },
            "#1725: missing deleted object EXPR layouts should serialize null expression provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1725: missing object EXPR live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1725: missing object EXPR live selection should advertise selected objects");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1725: missing object EXPR live selection should expose object selection kind");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1725: missing object EXPR live selection should expose containing-section metadata");
        expect_missing_object_expr_preview_bounds(
            live_process.stdout_text,
            "#2339: selected missing live object EXPR JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 190",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"missing-expr-live-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 120",
                "\"top\": 300",
                "\"width\": 700",
                "\"right\": 820",
                "\"height\": 90",
                "\"bottom\": 390",
                "\"highlightCount\": 0"
            },
            "#1725: missing live object EXPR selection should expose object metadata with null expression provenance");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"missing-expr-detail-section-guid\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 200",
                "\"height\": 1000",
                "\"bottom\": 1200"
            },
            "#1725: missing live object EXPR selection should expose containing section metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1725: missing object EXPR deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1725: missing object EXPR deleted selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1725: missing object EXPR deleted selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1725: missing object EXPR deleted selection should preserve containing sections");
        expect_missing_object_expr_preview_bounds(
            deleted_process.stdout_text,
            "#2339: selected missing deleted object EXPR JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"missing-expr-detail-section-guid\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 420",
                "\"sectionRelativeBottom\": 540",
                "\"sectionObjectIndex\": 0",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"missing-expr-deleted-object-guid\"",
                "\"titleFieldIndex\": 6",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 260",
                "\"top\": 620",
                "\"width\": 500",
                "\"right\": 760",
                "\"height\": 120",
                "\"bottom\": 740",
                "\"highlightCount\": 0"
            },
            "#1725: missing deleted object EXPR selection should expose object metadata with null expression provenance");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"missing-expr-detail-section-guid\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 200",
                "\"height\": 1000",
                "\"bottom\": 1200"
            },
            "#1725: missing deleted object EXPR selection should expose containing section metadata");
    };

    run_missing_object_expr_layout(temp_root / "missing_object_expr.frx",
                                   "missing_object_expr.frx",
                                   "report");
    run_missing_object_expr_layout(temp_root / "missing_object_expr.lbx",
                                   "missing_object_expr.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_synthesizes_report_object_titles_without_title_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_object_title_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_object_title_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_missing_object_title_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing object title summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing object title summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1726: missing object title schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1726: missing object title layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1726: missing object title label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1726: missing object title layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1726: missing object title layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1726: missing object title layouts should keep live object section membership");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1726: missing object title layouts should preserve deleted placed-object counts");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1726: missing object title layouts should preserve the containing live section");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1726: missing object title layouts should not fabricate deleted sections");
        expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [\n        {\"kind\": \"field\", \"count\": 1}\n      ]",
                        "#1726: missing live object title layouts should preserve object-kind counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                        "#1726: missing deleted object title layouts should preserve object-kind counts");
        expect_missing_object_title_preview_bounds(
            summary_process.stdout_text,
            "#2340: missing object title summary JSON");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"detail_0\"",
                "\"objects\": [",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_0\"",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"titleMemoBlockNumber\": 0",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 140",
                "\"top\": 220",
                "\"width\": 420",
                "\"right\": 560",
                "\"height\": 80",
                "\"bottom\": 300"
            },
            "#1726: missing live object title layouts should synthesize titles with null provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 400",
                "\"sectionRelativeBottom\": 510",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 2\"",
                "\"titleFieldIndex\": null",
                "\"titleMemoBlockNumber\": 0",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"left\": 360",
                "\"top\": 500",
                "\"width\": 380",
                "\"right\": 740",
                "\"height\": 110",
                "\"bottom\": 610"
            },
            "#1726: missing deleted object title layouts should synthesize titles with null provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1726: missing object title live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1726: missing object title live selection should advertise selected objects");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1726: missing object title live selection should expose object selection kind");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1726: missing object title live selection should expose containing-section metadata");
        expect_missing_object_title_preview_bounds(
            live_process.stdout_text,
            "#2340: selected missing live object title JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 120",
                "\"sectionRelativeBottom\": 200",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"title\": \"Record 1\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"left\": 140",
                "\"top\": 220",
                "\"width\": 420",
                "\"right\": 560",
                "\"height\": 80",
                "\"bottom\": 300"
            },
            "#1726: missing live object title selection should expose synthesized title metadata");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_0\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 100",
                "\"height\": 800",
                "\"bottom\": 900"
            },
            "#1726: missing live object title selection should expose containing section metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1726: missing object title deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1726: missing object title deleted selection should advertise selected objects");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1726: missing object title deleted selection should expose object selection kind");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1726: missing object title deleted selection should preserve containing sections");
        expect_missing_object_title_preview_bounds(
            deleted_process.stdout_text,
            "#2340: selected missing deleted object title JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_0\"",
                "\"containingSectionRecordIndex\": 0",
                "\"sectionRelativeTop\": 400",
                "\"sectionRelativeBottom\": 510",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 2\"",
                "\"titleFieldIndex\": null",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"left\": 360",
                "\"top\": 500",
                "\"width\": 380",
                "\"right\": 740",
                "\"height\": 110",
                "\"bottom\": 610"
            },
            "#1726: missing deleted object title selection should expose synthesized title metadata");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_0\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 0",
                "\"top\": 100",
                "\"height\": 800",
                "\"bottom\": 900"
            },
            "#1726: missing deleted object title selection should expose containing section metadata");
    };

    run_missing_object_title_layout(temp_root / "missing_object_title.frx",
                                    "missing_object_title.frx",
                                    "report");
    run_missing_object_title_layout(temp_root / "missing_object_title.lbx",
                                    "missing_object_title.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#endif

#endif

}  // namespace cf_test_studio_host_json
