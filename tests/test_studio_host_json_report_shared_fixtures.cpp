// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_layout_json(const std::filesystem::path& report_path) {
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
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", "", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid", ""},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid", ""},
        {"6", "", "", "50", "8000", "100", "100", "", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1452: synthetic FRX table for report layout JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#1452: synthetic FRX table should mark deleted layout objects");
}

void write_synthetic_report_table_for_zero_top_section_reflow_json(const std::filesystem::path& report_path) {
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
        {"9", "0", "", "", "", "", "11459", "", "", "title-section-guid"},
        {"9", "1", "", "", "", "", "4480", "", "", "page-header-guid"},
        {"9", "4", "", "", "", "", "1875", "", "", "detail-guid"},
        {"9", "7", "", "", "", "", "6875", "", "", "page-footer-guid"},
        {"5", "", "\"Deep title\"", "8645.833", "6666.667", "19687.500", "3333.333", "Times New Roman", "", "title-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3057: synthetic FRX/LBX table for zero-top section reflow should be created");
}

void write_synthetic_report_table_for_stable_deleted_layout_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 6U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-label-guid"
    });
    expect(unique_id_result.ok, "#1644: stable deleted layout fixture should seed a deleted object unique id");
    expect(dbf_record_deleted(report_path, 6U),
           "#1644: stable deleted layout fixture should preserve the deleted object state");
}

void write_synthetic_report_table_for_malformed_numeric_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'C', .length = 16U},
        {.name = "VPOS", .type = 'C', .length = 16U},
        {.name = "WIDTH", .type = 'C', .length = 16U},
        {.name = "HEIGHT", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "not-top", "", "not-height", ""},
        {"5", "", "\"Malformed live\"", "left?", "top?", "wide?", "tall?", "malformed-live-guid"},
        {"5", "", "\"Malformed deleted\"", "left!", "top!", "wide!", "tall!", "malformed-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1716: synthetic report table for malformed layout numerics should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1716: synthetic report table should mark the malformed-numeric object deleted");
}

void write_synthetic_report_table_for_oversized_numeric_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'C', .length = 48U},
        {.name = "VPOS", .type = 'C', .length = 48U},
        {.name = "WIDTH", .type = 'C', .length = 48U},
        {.name = "HEIGHT", .type = 'C', .length = 48U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::string huge_positive = "999999999999999999999999999999";
    const std::string huge_negative = "-999999999999999999999999999999";
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", huge_positive, "", huge_negative, ""},
        {"5", "", "\"Oversized live\"", huge_positive, huge_positive, huge_positive, huge_positive,
         "oversized-live-guid"},
        {"5", "", "\"Oversized deleted\"", huge_negative, huge_negative, huge_negative, huge_negative,
         "oversized-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1717: synthetic report table for oversized layout numerics should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1717: synthetic report table should mark the oversized-numeric object deleted");
}

void write_synthetic_report_table_for_fractional_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 12U, .decimal_count = 2U},
        {.name = "VPOS", .type = 'N', .length = 12U, .decimal_count = 2U},
        {.name = "WIDTH", .type = 'N', .length = 12U, .decimal_count = 2U},
        {.name = "HEIGHT", .type = 'N', .length = 12U, .decimal_count = 2U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "10.75", "", "1000.90", ""},
        {"5", "", "\"Fractional live\"", "125.90", "200.50", "300.75", "80.25",
         "fractional-live-guid"},
        {"5", "", "\"Fractional deleted\"", "425.80", "700.60", "150.95", "40.70",
         "fractional-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1719: synthetic report table for fractional layout numerics should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1719: synthetic report table should mark the fractional object deleted");
}

void write_synthetic_report_table_for_missing_root_objcode_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "ORIENTATIO", .type = 'N', .length = 8U},
        {.name = "PAPERSIZE", .type = 'N', .length = 8U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "GRIDV=1\nGRIDH=0", "1", "9", "120", "missing-objcode-live-settings-guid"},
        {"1", "COLS=3\nCOLWIDTH=5000", "0", "1", "240", "missing-objcode-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1730: synthetic report table without root OBJCODE schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1730: synthetic report table should mark the no-OBJCODE settings row deleted");
}

void write_synthetic_report_table_for_printer_identity_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "DRIVER=cups\nDEVICE=HP LaserJet\nOUTPUT=LPT1", "live-printer-identity-guid"},
        {"1", "53", "DRIVER=deleted-cups\nDEVICE=Deleted Printer\nOUTPUT=FILE:", "deleted-printer-identity-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3796: synthetic report table for printer-identity summary should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#3796: synthetic report table should mark deleted printer-identity settings");
}

void write_synthetic_report_table_for_invalid_first_duplicate_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=sideways\n"
         "ORIENTATION=1\n"
         "COLS=many\n"
         "COLS=3",
         "9", "8", "invalid-first-duplicate-live-settings-guid"},
        {"1", "53",
         "ORIENTATION=deleted-sideways\n"
         "ORIENTATION=5\n"
         "COLS=deleted-many\n"
         "COLS=7",
         "10", "11", "invalid-first-duplicate-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1755: synthetic report table with invalid-first duplicate settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1755: synthetic report table should mark invalid-first duplicate settings deleted");
}

void write_synthetic_report_table_for_missing_root_expr_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'N', .length = 8U},
        {.name = "PAPERSIZE", .type = 'N', .length = 8U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "1", "9", "120", "missing-expr-live-settings-guid"},
        {"1", "53", "0", "1", "240", "missing-expr-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1723: synthetic report table without root EXPR schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1723: synthetic report table should mark the no-EXPR settings row deleted");
}

void write_synthetic_report_table_without_unique_id_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200"},
        {"5", "", "\"No unique id label\"", "400", "1200", "1500", "250"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1711: synthetic report table without UNIQUEID field should be created");
}

void write_synthetic_report_table_for_layout_distribution_json(const std::filesystem::path& report_path) {
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
        {"9", "4", "", "", "2000", "", "5000", ""},
        {"8", "0", "left.value", "100", "2600", "50", "200", "left-field-guid"},
        {"8", "0", "middle.value", "175", "2600", "50", "200", "middle-field-guid"},
        {"8", "0", "right.value", "700", "2600", "50", "200", "right-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1469: synthetic FRX table for report layout distribution should be created");
}

void write_synthetic_report_table_for_layout_reorder_json(const std::filesystem::path& report_path) {
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
        {"9", "4", "", "", "2000", "", "5000", ""},
        {"8", "0", "left.value", "100", "2600", "50", "200", "left-field-guid"},
        {"8", "0", "middle.value", "100", "2600", "50", "200", "middle-field-guid"},
        {"8", "0", "right.value", "100", "2600", "50", "200", "right-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1470: synthetic FRX table for report layout reorder should be created");
}

void test_studio_host_json_defaults_malformed_report_layout_numerics(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_malformed_report_layout_numeric_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_malformed_numeric_layout = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_malformed_numeric_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " malformed numeric layout summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " malformed numeric layout summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1716: malformed report/label layout numerics should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1716: malformed layout numerics should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1716: malformed numeric label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1716: malformed live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1716: malformed live layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1716: malformed live layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1716: malformed live layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1716: malformed live layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1716: malformed live layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1716: malformed live layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1716: malformed deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1716: malformed deleted layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1716: malformed deleted layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1716: malformed deleted layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#1716: malformed deleted layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1716: malformed deleted layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1716: malformed deleted layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1716: malformed layout numerics should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1716: malformed layout numerics should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1716: malformed section heights should default to zero in summaries");
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2354: malformed numeric summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1716: malformed live object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2354: selected malformed numeric live object JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 0",
                "\"top\": 0",
                "\"width\": 0",
                "\"right\": 0",
                "\"height\": 0",
                "\"bottom\": 0"
            },
            "#1716: malformed live object numerics should default selected geometry to zero");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1716: malformed deleted object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2354: selected malformed numeric deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 0",
                "\"top\": 0",
                "\"width\": 0",
                "\"right\": 0",
                "\"height\": 0",
                "\"bottom\": 0"
            },
            "#1716: malformed deleted object numerics should default selected geometry to zero");
    };

    run_malformed_numeric_layout(temp_root / "malformed_numerics.frx",
                                 "malformed_numerics.frx",
                                 "report");
    run_malformed_numeric_layout(temp_root / "malformed_numerics.lbx",
                                 "malformed_numerics.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_defaults_oversized_report_layout_numerics(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_oversized_report_layout_numeric_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_oversized_numeric_layout = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_oversized_numeric_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " oversized numeric layout summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " oversized numeric layout summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1717: oversized report/label layout numerics should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1717: oversized layout numerics should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1717: oversized numeric label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1717: oversized live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1717: oversized live layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1717: oversized live layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1717: oversized live layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1717: oversized live layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1717: oversized live layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1717: oversized live layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1717: oversized deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1717: oversized deleted layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1717: oversized deleted layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1717: oversized deleted layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#1717: oversized deleted layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1717: oversized deleted layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1717: oversized deleted layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1717: oversized layout numerics should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1717: oversized layout numerics should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1717: oversized section heights should default to zero in summaries");
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2355: oversized numeric summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1717: oversized live object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2355: selected oversized numeric live object JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 0",
                "\"top\": 0",
                "\"width\": 0",
                "\"right\": 0",
                "\"height\": 0",
                "\"bottom\": 0"
            },
            "#1717: oversized live object numerics should default selected geometry to zero");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1717: oversized deleted object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2355: selected oversized numeric deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 0",
                "\"top\": 0",
                "\"width\": 0",
                "\"right\": 0",
                "\"height\": 0",
                "\"bottom\": 0"
            },
            "#1717: oversized deleted object numerics should default selected geometry to zero");
    };

    run_oversized_numeric_layout(temp_root / "oversized_numerics.frx",
                                 "oversized_numerics.frx",
                                 "report");
    run_oversized_numeric_layout(temp_root / "oversized_numerics.lbx",
                                 "oversized_numerics.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
