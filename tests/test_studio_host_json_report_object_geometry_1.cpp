#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void expect_full_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 8100",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 8100",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1000",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 2200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 2900",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 1200",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
}

void expect_empty_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": false",
                    prefix + " should not fabricate live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve zero live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve zero live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve zero live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve zero live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve zero live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve zero live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

void expect_fractional_geometry_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 10",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 425",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 1010",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 425",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 1000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 425",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 700",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 575",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 740",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 150",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 40",
                    prefix + " should preserve deleted preview heights");
}

void expect_negative_dimension_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 300",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 300",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 700",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 50",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 700",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 50",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve deleted preview heights");
}

void expect_zero_available_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve zero live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve zero live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve zero live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve zero live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve zero live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve zero live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

void expect_unresolved_memo_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 2000",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 7000",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 5000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

void expect_unresolved_section_memo_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 100",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 600",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 500",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 900",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 1200",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
}

void expect_unresolved_deleted_object_memo_preview_bounds(
    const std::string& text,
    const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 2000",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 7000",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 5000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1200",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 5200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 3050",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 4000",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 450",
                    prefix + " should preserve deleted preview heights");
}

void expect_unresolved_unplaced_object_memo_preview_bounds(
    const std::string& text,
    const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 1200",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 2600",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 3050",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 4000",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 450",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

void expect_missing_section_objcode_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 150",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 600",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 450",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 900",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 1150",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 250",
                    prefix + " should preserve deleted preview heights");
}

void expect_missing_object_objcode_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 120",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 300",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 820",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 390",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 700",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 90",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 260",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 620",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 760",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 740",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 500",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 120",
                    prefix + " should preserve deleted preview heights");
}

void expect_missing_object_expr_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 200",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 820",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 1200",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 820",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 1000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 260",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 620",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 760",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 740",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 500",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 120",
                    prefix + " should preserve deleted preview heights");
}

void expect_missing_object_title_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 100",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 560",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 900",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 560",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 800",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 360",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 500",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 740",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 610",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 380",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 110",
                    prefix + " should preserve deleted preview heights");
}

void write_synthetic_report_table_for_layout_font_options_json(const std::filesystem::path& report_path) {
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
        {.name = "MODE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "10", "3", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "Tahoma", "11", "5", "", "deleted-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2691: synthetic FRX table for report layout font option JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2691: synthetic FRX table should mark deleted layout objects");
}

void write_synthetic_report_table_for_negative_dimension_layout_json(
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
        {"9", "4", "", "", "0", "", "-500", ""},
        {"5", "", "\"Negative live\"", "300", "0", "-100", "-200", "negative-live-guid"},
        {"5", "", "\"Negative deleted\"", "700", "50", "-400", "-300", "negative-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1715: synthetic report table for negative layout dimensions should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1715: synthetic report table should mark the negative-dimension object deleted");
}

void write_synthetic_report_table_for_missing_geometry_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", ""},
        {"9", "4", "", ""},
        {"5", "", "\"Missing geometry live\"", "missing-geometry-live-guid"},
        {"5", "", "\"Missing geometry deleted\"", "missing-geometry-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1720: synthetic report table without geometry fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1720: synthetic report table should mark the missing-geometry object deleted");
}

void write_synthetic_report_table_for_unresolved_geometry_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'C', .length = 24U},
        {.name = "VPOS", .type = 'C', .length = 24U},
        {.name = "WIDTH", .type = 'C', .length = 24U},
        {.name = "HEIGHT", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "4", "detail.placeholder.geometry", "", "<memo block 70>", "", "<memo block 71>"},
        {"8", "0", "customer.unresolved.geometry", "<memo block 72>", "<memo block 73>",
         "<memo block 74>", "<memo block 75>"},
        {"5", "", "\"Deleted unresolved geometry\"", "<memo block 76>", "<memo block 77>",
         "<memo block 78>", "<memo block 79>"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1740: synthetic report table with unresolved geometry memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#1740: synthetic report table should mark unresolved geometry object deleted");
}

void write_synthetic_report_table_for_missing_section_geometry_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "1", "page.header.missing.geometry", "missing-geometry-live-section-guid"},
        {"9", "8", "summary.missing.geometry", "missing-geometry-deleted-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1727: synthetic report table without section geometry schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1727: synthetic report table should mark the no-geometry section deleted");
}

void write_synthetic_report_table_for_column_width_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "COLWIDTH", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "COLS=2\nCOLSPACING=120", "3600", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1539: synthetic report table for column width field JSON should be created");
}

void write_synthetic_report_table_for_deleted_column_width_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_width_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1595: synthetic report table should mark column-width settings deleted");
}

void write_synthetic_report_table_for_stable_column_width_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_width_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1836: stable column-width fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1836: stable column-width fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_column_width_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_column_width_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1836: stable deleted column-width fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1836: stable deleted column-width fixture should preserve the deleted settings state");
}

void test_studio_host_json_refreshes_detail_header_footer_section_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_detail_header_footer_section_preview_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto update_footer_height_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "900",
                    "--json"
                },
                temp_root);

            if (update_footer_height_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview height stdout:\n"
                          << update_footer_height_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview height stderr:\n"
                          << update_footer_height_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_height_process.exit_code == 0,
                   "#1819: detail-footer section preview height update by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "900",
                   "#1819: detail-footer section preview height update should persist the HEIGHT field");
            expect_contains(update_footer_height_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1819: detail-footer section preview height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_height_process.stdout_text, "\"isLabel\": true",
                                "#1819: detail-footer label section preview height update should retain label identity");
            }
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1819: detail-footer section preview height update should preserve preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1819: detail-footer section preview height update should preserve preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsBottom\": 1200",
                            "#1819: detail-footer section preview height update should refresh preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsHeight\": 1200",
                            "#1819: detail-footer section preview height update should refresh preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1819: detail-footer section preview height update should preserve deleted preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1819: detail-footer section preview height update should preserve deleted preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1819: detail-footer section preview height update should preserve deleted preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"sectionHeightTotal\": 1200",
                            "#1819: detail-footer section preview height update should refresh live section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1819: detail-footer section preview height update should preserve selected section availability");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1819: detail-footer section preview height update should preserve selection kind");
            expect_contains_in_order(
                update_footer_height_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"top\": 300",
                    "\"height\": 900",
                    "\"bottom\": 1200"
                },
                "#1819: detail-footer section preview height update should refresh selected-section geometry");

            const auto update_header_top_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "-150",
                    "--json"
                },
                temp_root);

            if (update_header_top_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview top stdout:\n"
                          << update_header_top_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview top stderr:\n"
                          << update_header_top_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_top_process.exit_code == 0,
                   "#1819: detail-header section preview top update by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "-150",
                   "#1819: detail-header section preview top update should persist the VPOS field");
            expect_contains(update_header_top_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1819: detail-header section preview top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_top_process.stdout_text, "\"isLabel\": true",
                                "#1819: detail-header label section preview top update should retain label identity");
            }
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1819: detail-header section preview top update should preserve preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsTop\": -150",
                            "#1819: detail-header section preview top update should refresh preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsBottom\": 1200",
                            "#1819: detail-header section preview top update should preserve expanded preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsHeight\": 1350",
                            "#1819: detail-header section preview top update should refresh preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1819: detail-header section preview top update should preserve deleted preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1819: detail-header section preview top update should preserve deleted preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1819: detail-header section preview top update should preserve deleted preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"sectionHeightTotal\": 1200",
                            "#1819: detail-header section preview top update should preserve live section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1819: detail-header section preview top update should preserve selected section availability");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1819: detail-header section preview top update should preserve selection kind");
            expect_contains_in_order(
                update_header_top_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"top\": -150",
                    "\"height\": 300",
                    "\"bottom\": 150"
                },
                "#1819: detail-header section preview top update should refresh selected-section geometry");
        };

    run_detail_header_footer_section_preview_bounds(
        temp_root / "detail_header_footer_section_preview_bounds_stable.frx",
        "detail_header_footer_section_preview_bounds_stable.frx",
        "report");
    run_detail_header_footer_section_preview_bounds(
        temp_root / "detail_header_footer_section_preview_bounds_stable.lbx",
        "detail_header_footer_section_preview_bounds_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_nudges_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_nudge_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_nudge = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " nudge live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " nudge live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1628: live report/label layout object nudge geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1628: live report/label layout object nudge geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto nudge_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--nudge-object",
                "--nudge-mode", "both",
                "--delta-hpos", "25",
                "--delta-vpos", "-100",
                "--nudge-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (nudge_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout nudge stdout:\n"
                      << nudge_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout nudge stderr:\n"
                      << nudge_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(nudge_process.exit_code == 0,
               "#1628: live edited report/label layout object nudge should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "325" &&
                   visual_object_property(asset_path, "field-guid", "VPOS") == "2500" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "500",
               "#1628: live edited report/label layout object nudge should apply delta position and preserve edited size fields");
        expect_contains(nudge_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1628: live edited report/label layout object nudge should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(nudge_process.stdout_text, "\"isLabel\": true",
                            "#1628: live edited label layout object nudge should retain label identity");
        }
        expect_contains(nudge_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1883: live edited report/label layout object nudge should preserve preview availability");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1883: live edited report/label layout object nudge should preserve preview left bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1883: live edited report/label layout object nudge should preserve preview top bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1883: live edited report/label layout object nudge should preserve preview right bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1883: live edited report/label layout object nudge should preserve preview bottom bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1883: live edited report/label layout object nudge should preserve preview widths");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1883: live edited report/label layout object nudge should preserve preview heights");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview availability");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview left bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview top bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview right bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview bottom bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview widths");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview heights");
        expect_contains(nudge_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1628: live edited report/label layout object nudge should preserve selected-object availability");
        expect_contains(nudge_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1628: live edited report/label layout object nudge should preserve containing-section availability");
        expect_contains(nudge_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1628: live edited report/label layout object nudge should serialize containing-section metadata");
        expect_contains_in_order(
            nudge_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 500",
                "\"sectionRelativeBottom\": 1000",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 325",
                "\"top\": 2500",
                "\"width\": 250",
                "\"right\": 575",
                "\"height\": 500",
                "\"bottom\": 3000"
            },
            "#1628: live edited report/label layout object nudge should refresh selected nudged geometry and section metadata");
    };

    run_live_edited_nudge(temp_root / "nudge_live_edited.frx",
                          "nudge_live_edited.frx",
                          "report");
    run_live_edited_nudge(temp_root / "nudge_live_edited.lbx",
                          "nudge_live_edited.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_aligns_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_align_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_align = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " align live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " align live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1625: live report/label layout object align geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1625: live report/label layout object align geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto align_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--align-object",
                "--alignment-mode", "left",
                "--anchor-unique-id", "label-guid",
                "--align-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (align_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout align stdout:\n"
                      << align_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout align stderr:\n"
                      << align_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(align_process.exit_code == 0,
               "#1625: live edited report/label layout object align should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "900" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "500",
               "#1625: live edited report/label layout object align should apply alignment and preserve edited size fields");
        expect(visual_object_property(asset_path, "label-guid", "HPOS") == "900",
               "#1625: live edited report/label layout object align should preserve anchor geometry");
        expect_contains(align_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1625: live edited report/label layout object align should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(align_process.stdout_text, "\"isLabel\": true",
                            "#1625: live edited label layout object align should retain label identity");
        }
        expect_contains(align_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1884: live edited report/label layout object align should preserve preview availability");
        expect_contains(align_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1884: live edited report/label layout object align should preserve preview left bounds");
        expect_contains(align_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1884: live edited report/label layout object align should preserve preview top bounds");
        expect_contains(align_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1884: live edited report/label layout object align should preserve preview right bounds");
        expect_contains(align_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1884: live edited report/label layout object align should preserve preview bottom bounds");
        expect_contains(align_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1884: live edited report/label layout object align should preserve preview widths");
        expect_contains(align_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1884: live edited report/label layout object align should preserve preview heights");
        expect_contains(align_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1884: live edited report/label layout object align should preserve deleted preview availability");
        expect_contains(align_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1884: live edited report/label layout object align should preserve deleted preview left bounds");
        expect_contains(align_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1884: live edited report/label layout object align should preserve deleted preview top bounds");
        expect_contains(align_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1884: live edited report/label layout object align should preserve deleted preview right bounds");
        expect_contains(align_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1884: live edited report/label layout object align should preserve deleted preview bottom bounds");
        expect_contains(align_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1884: live edited report/label layout object align should preserve deleted preview widths");
        expect_contains(align_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1884: live edited report/label layout object align should preserve deleted preview heights");
        expect_contains(align_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1625: live edited report/label layout object align should preserve selected-object availability");
        expect_contains(align_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1625: live edited report/label layout object align should preserve containing-section availability");
        expect_contains(align_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1625: live edited report/label layout object align should serialize containing-section metadata");
        expect_contains_in_order(
            align_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 900",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 1150",
                "\"height\": 500",
                "\"bottom\": 3100"
            },
            "#1625: live edited report/label layout object align should refresh selected aligned geometry and section metadata");
    };

    run_live_edited_align(temp_root / "align_live_edited.frx",
                          "align_live_edited.frx",
                          "report");
    run_live_edited_align(temp_root / "align_live_edited.lbx",
                          "align_live_edited.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_resize_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_resize = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " resize live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " resize live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1626: live report/label layout object resize geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1626: live report/label layout object resize geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto resize_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--resize-object",
                "--resize-mode", "size",
                "--anchor-unique-id", "label-guid",
                "--resize-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (resize_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout resize stdout:\n"
                      << resize_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout resize stderr:\n"
                      << resize_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(resize_process.exit_code == 0,
               "#1626: live edited report/label layout object resize should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "1800" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "350",
               "#1626: live edited report/label layout object resize should apply anchor size and preserve edited left position");
        expect(visual_object_property(asset_path, "label-guid", "WIDTH") == "1800" &&
                   visual_object_property(asset_path, "label-guid", "HEIGHT") == "350",
               "#1626: live edited report/label layout object resize should preserve anchor size");
        expect_contains(resize_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1626: live edited report/label layout object resize should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(resize_process.stdout_text, "\"isLabel\": true",
                            "#1626: live edited label layout object resize should retain label identity");
        }
        expect_contains(resize_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1885: live edited report/label layout object resize should preserve preview availability");
        expect_contains(resize_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1885: live edited report/label layout object resize should preserve preview left bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1885: live edited report/label layout object resize should preserve preview top bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1885: live edited report/label layout object resize should preserve preview right bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1885: live edited report/label layout object resize should preserve preview bottom bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1885: live edited report/label layout object resize should preserve preview widths");
        expect_contains(resize_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1885: live edited report/label layout object resize should preserve preview heights");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1885: live edited report/label layout object resize should preserve deleted preview availability");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1885: live edited report/label layout object resize should preserve deleted preview left bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1885: live edited report/label layout object resize should preserve deleted preview top bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1885: live edited report/label layout object resize should preserve deleted preview right bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1885: live edited report/label layout object resize should preserve deleted preview bottom bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1885: live edited report/label layout object resize should preserve deleted preview widths");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1885: live edited report/label layout object resize should preserve deleted preview heights");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1626: live edited report/label layout object resize should preserve selected-object availability");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1626: live edited report/label layout object resize should preserve containing-section availability");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1626: live edited report/label layout object resize should serialize containing-section metadata");
        expect_contains_in_order(
            resize_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 950",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 1800",
                "\"right\": 2100",
                "\"height\": 350",
                "\"bottom\": 2950"
            },
            "#1626: live edited report/label layout object resize should refresh selected resized geometry and section metadata");
    };

    run_live_edited_resize(temp_root / "resize_live_edited.frx",
                           "resize_live_edited.frx",
                           "report");
    run_live_edited_resize(temp_root / "resize_live_edited.lbx",
                           "resize_live_edited.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_snap_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_snap = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " snap live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " snap live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1627: live report/label layout object snap geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1627: live report/label layout object snap geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "375");
        set_live_geometry("VPOS", "2550");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto snap_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--snap-object",
                "--snap-mode", "both",
                "--grid-width", "700",
                "--grid-height", "750",
                "--snap-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (snap_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout snap stdout:\n"
                      << snap_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout snap stderr:\n"
                      << snap_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(snap_process.exit_code == 0,
               "#1627: live edited report/label layout object snap should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "700" &&
                   visual_object_property(asset_path, "field-guid", "VPOS") == "2250" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "500",
               "#1627: live edited report/label layout object snap should apply grid position and preserve edited size fields");
        expect_contains(snap_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1627: live edited report/label layout object snap should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(snap_process.stdout_text, "\"isLabel\": true",
                            "#1627: live edited label layout object snap should retain label identity");
        }
        expect_contains(snap_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1886: live edited report/label layout object snap should preserve preview availability");
        expect_contains(snap_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1886: live edited report/label layout object snap should preserve preview left bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1886: live edited report/label layout object snap should preserve preview top bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1886: live edited report/label layout object snap should preserve preview right bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1886: live edited report/label layout object snap should preserve preview bottom bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1886: live edited report/label layout object snap should preserve preview widths");
        expect_contains(snap_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1886: live edited report/label layout object snap should preserve preview heights");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1886: live edited report/label layout object snap should preserve deleted preview availability");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1886: live edited report/label layout object snap should preserve deleted preview left bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1886: live edited report/label layout object snap should preserve deleted preview top bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1886: live edited report/label layout object snap should preserve deleted preview right bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1886: live edited report/label layout object snap should preserve deleted preview bottom bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1886: live edited report/label layout object snap should preserve deleted preview widths");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1886: live edited report/label layout object snap should preserve deleted preview heights");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1627: live edited report/label layout object snap should preserve selected-object availability");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1627: live edited report/label layout object snap should preserve containing-section availability");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1627: live edited report/label layout object snap should serialize containing-section metadata");
        expect_contains_in_order(
            snap_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 250",
                "\"sectionRelativeBottom\": 750",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 700",
                "\"top\": 2250",
                "\"width\": 250",
                "\"right\": 950",
                "\"height\": 500",
                "\"bottom\": 2750"
            },
            "#1627: live edited report/label layout object snap should refresh selected snapped geometry and section metadata");
    };

    run_live_edited_snap(temp_root / "snap_live_edited.frx",
                         "snap_live_edited.frx",
                         "report");
    run_live_edited_snap(temp_root / "snap_live_edited.lbx",
                         "snap_live_edited.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_edited_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_edited_geometry_delete = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1615: edited report/label layout object delete fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live " << property_name
                          << " pre-delete update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live " << property_name
                          << " pre-delete update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1615: report/label layout object geometry pre-delete update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1615: report/label layout object geometry pre-delete update should preserve live state");
        };

        set_live_geometry("HPOS", "1400");
        set_live_geometry("WIDTH", "2400");
        set_live_geometry("HEIGHT", "900");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1615: edited report/label layout object delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1615: edited report/label layout object delete should mark the DBF record deleted");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400" &&
                   width_property.ok && width_property.exists && width_property.value == "2400" &&
                   height_property.ok && height_property.exists && height_property.value == "900",
               "#1615: edited report/label layout object delete should preserve edited geometry fields");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1615: edited report/label layout object delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1887: edited label layout object delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1887: edited report/label layout object delete should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1887: edited report/label layout object delete should refresh live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1887: edited report/label layout object delete should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1887: edited report/label layout object delete should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1887: edited report/label layout object delete should refresh live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1887: edited report/label layout object delete should refresh live preview widths");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1887: edited report/label layout object delete should refresh live preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1887: edited report/label layout object delete should preserve deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1887: edited report/label layout object delete should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1887: edited report/label layout object delete should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 3800",
                        "#1887: edited report/label layout object delete should refresh deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3500",
                        "#1887: edited report/label layout object delete should refresh deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2800",
                        "#1887: edited report/label layout object delete should refresh deleted preview widths");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 900",
                        "#1887: edited report/label layout object delete should refresh deleted preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1615: edited report/label layout object delete should add the edited object to deleted-object counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1615: edited report/label layout object delete should preserve selected deleted-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1615: edited report/label layout object delete should preserve object selection kind");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1615: edited report/label layout object delete should not fabricate containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1615: edited report/label layout object delete should serialize null containing-section metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1615: edited report/label layout object delete should preserve edited deleted-object geometry metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1615: edited report/label layout object delete should preserve selected deleted-object geometry metadata");
    };

    run_edited_geometry_delete(temp_root / "delete_edited_geometry.frx",
                               "delete_edited_geometry.frx",
                               "report");
    run_edited_geometry_delete(temp_root / "delete_edited_geometry.lbx",
                               "delete_edited_geometry.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_edited_unplaced_delete = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1616: edited unplaced report/label layout object delete fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live unplaced " << property_name
                          << " pre-delete update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live unplaced " << property_name
                          << " pre-delete update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1616: report/label layout object unplaced pre-delete update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1616: report/label layout object unplaced pre-delete update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1616: edited unplaced report/label layout object delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1616: edited unplaced report/label layout object delete should mark the DBF record deleted");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-300" &&
                   top_property.ok && top_property.exists && top_property.value == "9000" &&
                   height_property.ok && height_property.exists && height_property.value == "700",
               "#1616: edited unplaced report/label layout object delete should preserve edited geometry fields");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1616: edited unplaced report/label layout object delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1889: edited unplaced label layout object delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1889: edited unplaced report/label layout object delete should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview widths");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1616: edited unplaced report/label layout object delete should keep deleted preview bounds available");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": -300",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1889: edited unplaced report/label layout object delete should preserve deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 3700",
                        "#1889: edited unplaced report/label layout object delete should expand deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 9700",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4000",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview widths");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 7100",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1616: edited unplaced report/label layout object delete should add the edited object to deleted-object counts");
        expect_contains(delete_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should preserve deleted placed-object counts");
        expect_contains(delete_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should count the edited deleted object as unplaced");
        expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should remove the edited object from live unplaced counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1616: edited unplaced report/label layout object delete should preserve selected deleted-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1616: edited unplaced report/label layout object delete should preserve object selection kind");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1616: edited unplaced report/label layout object delete should not fabricate containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1616: edited unplaced report/label layout object delete should serialize null containing-section metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1616: edited unplaced report/label layout object delete should preserve edited deleted-object geometry metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1616: edited unplaced report/label layout object delete should preserve selected deleted-object geometry metadata");
    };

    run_edited_unplaced_delete(temp_root / "delete_edited_unplaced.frx",
                               "delete_edited_unplaced.frx",
                               "report");
    run_edited_unplaced_delete(temp_root / "delete_edited_unplaced.lbx",
                               "delete_edited_unplaced.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_live_edited_then_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_restore_live_edited_deleted_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_delete_restore = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1617: live edited then deleted report/label layout object restore fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live round-trip " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live round-trip " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1617: live report/label layout object round-trip geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1617: live report/label layout object round-trip geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "1400");
        set_live_geometry("WIDTH", "2400");
        set_live_geometry("HEIGHT", "900");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited delete before restore stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited delete before restore stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1617: live edited report/label layout object delete before restore should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1617: live edited report/label layout object delete before restore should mark the DBF record deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--restore-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited delete restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited delete restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1617: live edited then deleted report/label layout object restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1617: live edited then deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400" &&
                   width_property.ok && width_property.exists && width_property.value == "2400" &&
                   height_property.ok && height_property.exists && height_property.value == "900",
               "#1617: live edited then deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1617: live edited then deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1888: edited label layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1888: edited report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1888: edited report/label layout object restore should refresh live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1888: edited report/label layout object restore should refresh live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 3800",
                        "#1888: edited report/label layout object restore should refresh live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1888: edited report/label layout object restore should refresh live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 3800",
                        "#1888: edited report/label layout object restore should refresh live preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1888: edited report/label layout object restore should refresh live preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1888: edited report/label layout object restore should preserve deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1888: edited report/label layout object restore should preserve deleted preview left bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1888: edited report/label layout object restore should preserve deleted preview top bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1888: edited report/label layout object restore should preserve deleted preview right bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1888: edited report/label layout object restore should preserve deleted preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1888: edited report/label layout object restore should preserve deleted preview widths");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1888: edited report/label layout object restore should preserve deleted preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1617: live edited then deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1617: live edited then deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1617: live edited then deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1617: live edited then deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1617: live edited then deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1617: live edited then deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_live_edited_delete_restore(temp_root / "restore_live_edited_deleted.frx",
                                   "restore_live_edited_deleted.frx",
                                   "report");
    run_live_edited_delete_restore(temp_root / "restore_live_edited_deleted.lbx",
                                   "restore_live_edited_deleted.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_live_edited_unplaced_then_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_restore_live_edited_unplaced_deleted_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_delete_restore = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced then deleted report/label layout object restore fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live unplaced round-trip "
                          << property_name << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live unplaced round-trip "
                          << property_name << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1618: live unplaced report/label layout object round-trip geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1618: live unplaced report/label layout object round-trip geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited unplaced delete before restore stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited unplaced delete before restore stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1618: live edited unplaced report/label layout object delete before restore should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced report/label layout object delete before restore should mark the DBF record deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--restore-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited unplaced delete restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited unplaced delete restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1618: live edited unplaced then deleted report/label layout object restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced then deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-300" &&
                   top_property.ok && top_property.exists && top_property.value == "9000" &&
                   height_property.ok && height_property.exists && height_property.value == "700",
               "#1618: live edited unplaced then deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1618: live edited unplaced then deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1890: edited unplaced label layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1890: edited unplaced report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1890: edited unplaced report/label layout object restore should preserve live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 3700",
                        "#1890: edited unplaced report/label layout object restore should expand preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 4000",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 9700",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview left bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview top bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview right bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview widths");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1618: live edited unplaced then deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1618: live edited unplaced then deleted report/label layout object restore should not count out-of-band restored objects as placed");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1618: live edited unplaced then deleted report/label layout object restore should add restored objects to unplaced counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1618: live edited unplaced then deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1618: live edited unplaced then deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1618: live edited unplaced then deleted report/label layout object restore should not fabricate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1618: live edited unplaced then deleted report/label layout object restore should serialize null containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1618: live edited unplaced then deleted report/label layout object restore should refresh selected unplaced geometry without section metadata");
    };

    run_live_edited_unplaced_delete_restore(temp_root / "restore_live_edited_unplaced_deleted.frx",
                                            "restore_live_edited_unplaced_deleted.frx",
                                            "report");
    run_live_edited_unplaced_delete_restore(temp_root / "restore_live_edited_unplaced_deleted.lbx",
                                            "restore_live_edited_unplaced_deleted.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_distribute_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_distribution = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_distribution_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " distribute live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " distribute live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1629: live report/label layout object distribution geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1629: live report/label layout object distribution geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto distribute_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--distribute-object",
                "--distribution-mode", "horizontal",
                "--distribute-target-unique-id", "left-field-guid",
                "--distribute-target-unique-id", "middle-field-guid",
                "--distribute-target-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (distribute_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout distribution stdout:\n"
                      << distribute_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout distribution stderr:\n"
                      << distribute_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(distribute_process.exit_code == 0,
               "#1629: live edited report/label layout object distribution should exit successfully");
        expect(visual_object_property(asset_path, "left-field-guid", "HPOS") == "100" &&
                   visual_object_property(asset_path, "middle-field-guid", "HPOS") == "400" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "700",
               "#1629: live edited report/label layout object distribution should distribute the edited object between endpoints");
        expect(visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-field-guid", "HEIGHT") == "500",
               "#1629: live edited report/label layout object distribution should preserve edited size fields");
        expect_contains(distribute_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1629: live edited report/label layout object distribution should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(distribute_process.stdout_text, "\"isLabel\": true",
                            "#1629: live edited label layout object distribution should retain label identity");
        }
        expect_contains(distribute_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1629: live edited report/label layout object distribution should preserve selected-object availability");
        expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1629: live edited report/label layout object distribution should preserve containing-section availability");
        expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1629: live edited report/label layout object distribution should serialize containing-section metadata");
        expect_contains_in_order(
            distribute_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": 400",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 650",
                "\"height\": 500",
                "\"bottom\": 3100"
            },
            "#1629: live edited report/label layout object distribution should refresh selected distributed geometry and section metadata");
    };

    run_live_edited_distribution(temp_root / "distribute_live_edited.frx",
                                 "distribute_live_edited.frx",
                                 "report");
    run_live_edited_distribution(temp_root / "distribute_live_edited.lbx",
                                 "distribute_live_edited.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_live_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_reorder_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_reorder = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " reorder live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " reorder live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1623: live report/label layout object reorder geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1623: live report/label layout object reorder geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto reorder_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--reorder-object",
                "--unique-id", "middle-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout reorder stdout:\n"
                      << reorder_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout reorder stderr:\n"
                      << reorder_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_process.exit_code == 0,
               "#1623: live edited report/label layout object reorder should exit successfully");
        expect(visual_object_order(asset_path) == "middle-field-guid,left-field-guid,right-field-guid",
               "#1623: live edited report/label layout object reorder should move the edited object before the target");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-field-guid", "HEIGHT") == "500",
               "#1623: live edited report/label layout object reorder should preserve edited geometry fields");
        expect_contains(reorder_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1623: live edited report/label layout object reorder should return refreshed report-layout JSON");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1623: live edited report/label layout object reorder should preserve selected-object availability");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1623: live edited report/label layout object reorder should preserve containing-section availability");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1623: live edited report/label layout object reorder should serialize containing-section metadata");
        expect_contains_in_order(
            reorder_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 2",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 550",
                "\"height\": 500",
                "\"bottom\": 3100"
            },
            "#1623: live edited report/label layout object reorder should refresh selected reordered geometry and section metadata");
    };

    run_live_edited_reorder(temp_root / "reorder_live_edited.frx",
                            "reorder_live_edited.frx",
                            "report");
    run_live_edited_reorder(temp_root / "reorder_live_edited.lbx",
                            "reorder_live_edited.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_reorder_live_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_reorder = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " reorder unplaced live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " reorder unplaced live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1624: live unplaced report/label layout object reorder geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1624: live unplaced report/label layout object reorder geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto reorder_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--reorder-object",
                "--unique-id", "middle-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout reorder stdout:\n"
                      << reorder_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout reorder stderr:\n"
                      << reorder_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_process.exit_code == 0,
               "#1624: live edited unplaced report/label layout object reorder should exit successfully");
        expect(visual_object_order(asset_path) == "middle-field-guid,left-field-guid,right-field-guid",
               "#1624: live edited unplaced report/label layout object reorder should move the edited object before the target");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "-300" &&
                   visual_object_property(asset_path, "middle-field-guid", "VPOS") == "9000" &&
                   visual_object_property(asset_path, "middle-field-guid", "HEIGHT") == "700",
               "#1624: live edited unplaced report/label layout object reorder should preserve edited geometry fields");
        expect_contains(reorder_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1624: live edited unplaced report/label layout object reorder should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                            "#1880: live edited unplaced label layout object reorder should retain label identity");
        }
        expect_contains(reorder_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1880: live edited unplaced report/label layout object reorder should keep preview bounds available");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1880: live edited unplaced report/label layout object reorder should expand preview left bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1880: live edited unplaced report/label layout object reorder should preserve section-origin preview top bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1880: live edited unplaced report/label layout object reorder should preserve preview right bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1880: live edited unplaced report/label layout object reorder should expand preview bottom bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsWidth\": 450",
                        "#1880: live edited unplaced report/label layout object reorder should refresh preview widths");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsHeight\": 7700",
                        "#1880: live edited unplaced report/label layout object reorder should refresh preview heights");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1624: live edited unplaced report/label layout object reorder should preserve selected-object availability");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1624: live edited unplaced report/label layout object reorder should keep containing-section unavailable");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1624: live edited unplaced report/label layout object reorder should serialize null containing section");
        expect_contains_in_order(
            reorder_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 50",
                "\"right\": -250",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1624: live edited unplaced report/label layout object reorder should refresh selected reordered geometry without fabricated section metadata");
    };

    run_live_edited_unplaced_reorder(temp_root / "reorder_live_edited_unplaced.frx",
                                     "reorder_live_edited_unplaced.frx",
                                     "report");
    run_live_edited_unplaced_reorder(temp_root / "reorder_live_edited_unplaced.lbx",
                                     "reorder_live_edited_unplaced.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_live_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_duplicate_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_duplicate = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " duplicate live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " duplicate live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1619: live report/label layout object duplicate geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1619: live report/label layout object duplicate geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--duplicate-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-edited-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1619: live edited report/label layout object duplicate should exit successfully");
        expect(visual_object_count(asset_path) == before_count + 1U,
               "#1619: live edited report/label layout object duplicate should append one object record");
        expect(visual_object_exists(asset_path, "middle-edited-copy-guid"),
               "#1619: live edited report/label layout object duplicate should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-edited-copy-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "middle-edited-copy-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-edited-copy-guid", "HEIGHT") == "500",
               "#1619: live edited report/label layout object duplicate should preserve edited geometry fields");
        expect_contains(duplicate_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1619: live edited report/label layout object duplicate should return refreshed report-layout JSON");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1619: live edited report/label layout object duplicate should preserve selected-object availability");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1619: live edited report/label layout object duplicate should preserve containing-section availability");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1619: live edited report/label layout object duplicate should serialize containing-section metadata");
        expect_contains_in_order(
            duplicate_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 3",
                "\"sectionObjectCount\": 4",
                "\"objectKind\": \"field\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 550",
                "\"height\": 500",
                "\"bottom\": 3100",
                "\"expression\": \"middle.value\""
            },
            "#1619: live edited report/label layout object duplicate should refresh selected duplicate geometry and section metadata");
    };

    run_live_edited_duplicate(temp_root / "duplicate_live_edited.frx",
                              "duplicate_live_edited.frx",
                              "report");
    run_live_edited_duplicate(temp_root / "duplicate_live_edited.lbx",
                              "duplicate_live_edited.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_duplicate_live_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_duplicate = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " duplicate unplaced live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " duplicate unplaced live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1620: live unplaced report/label layout object duplicate geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1620: live unplaced report/label layout object duplicate geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--duplicate-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-offband-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1620: live edited unplaced report/label layout object duplicate should exit successfully");
        expect(visual_object_count(asset_path) == before_count + 1U,
               "#1620: live edited unplaced report/label layout object duplicate should append one object record");
        expect(visual_object_exists(asset_path, "middle-offband-guid"),
               "#1620: live edited unplaced report/label layout object duplicate should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-offband-guid", "HPOS") == "-300" &&
                   visual_object_property(asset_path, "middle-offband-guid", "VPOS") == "9000" &&
                   visual_object_property(asset_path, "middle-offband-guid", "HEIGHT") == "700",
               "#1620: live edited unplaced report/label layout object duplicate should preserve edited geometry fields");
        expect_contains(duplicate_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1620: live edited unplaced report/label layout object duplicate should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(duplicate_process.stdout_text, "\"isLabel\": true",
                            "#1881: live edited unplaced label layout object duplicate should retain label identity");
        }
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1881: live edited unplaced report/label layout object duplicate should keep preview bounds available");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1881: live edited unplaced report/label layout object duplicate should expand preview left bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1881: live edited unplaced report/label layout object duplicate should preserve section-origin preview top bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1881: live edited unplaced report/label layout object duplicate should preserve preview right bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1881: live edited unplaced report/label layout object duplicate should expand preview bottom bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsWidth\": 450",
                        "#1881: live edited unplaced report/label layout object duplicate should refresh preview widths");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsHeight\": 7700",
                        "#1881: live edited unplaced report/label layout object duplicate should refresh preview heights");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1620: live edited unplaced report/label layout object duplicate should preserve selected-object availability");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1620: live edited unplaced report/label layout object duplicate should keep containing-section unavailable");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1620: live edited unplaced report/label layout object duplicate should serialize null containing section");
        expect_contains_in_order(
            duplicate_process.stdout_text,
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
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 50",
                "\"right\": -250",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1620: live edited unplaced report/label layout object duplicate should refresh selected duplicate geometry without fabricated section metadata");
    };

    run_live_edited_unplaced_duplicate(temp_root / "duplicate_live_edited_unplaced.frx",
                                       "duplicate_live_edited_unplaced.frx",
                                       "report");
    run_live_edited_unplaced_duplicate(temp_root / "duplicate_live_edited_unplaced.lbx",
                                       "duplicate_live_edited_unplaced.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_live_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_rename_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_rename = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " rename live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " rename live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1621: live report/label layout object rename geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1621: live report/label layout object rename geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto rename_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--rename-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-renamed-guid",
                "--json"
            },
            temp_root);

        if (rename_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout rename stdout:\n"
                      << rename_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout rename stderr:\n"
                      << rename_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_process.exit_code == 0,
               "#1621: live edited report/label layout object rename should exit successfully");
        expect(visual_object_count(asset_path) == before_count,
               "#1621: live edited report/label layout object rename should preserve object count");
        expect(!visual_object_exists(asset_path, "middle-field-guid"),
               "#1621: live edited report/label layout object rename should remove the old unique id");
        expect(visual_object_exists(asset_path, "middle-renamed-guid"),
               "#1621: live edited report/label layout object rename should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-renamed-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "middle-renamed-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-renamed-guid", "HEIGHT") == "500",
               "#1621: live edited report/label layout object rename should preserve edited geometry fields");
        expect_contains(rename_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1621: live edited report/label layout object rename should return refreshed report-layout JSON");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1621: live edited report/label layout object rename should preserve selected-object availability");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1621: live edited report/label layout object rename should preserve containing-section availability");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1621: live edited report/label layout object rename should serialize containing-section metadata");
        expect_contains_in_order(
            rename_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 2",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 550",
                "\"height\": 500",
                "\"bottom\": 3100"
            },
            "#1621: live edited report/label layout object rename should refresh selected renamed geometry and section metadata");
    };

    run_live_edited_rename(temp_root / "rename_live_edited.frx",
                           "rename_live_edited.frx",
                           "report");
    run_live_edited_rename(temp_root / "rename_live_edited.lbx",
                           "rename_live_edited.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_rename_live_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_rename = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " rename unplaced live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " rename unplaced live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1622: live unplaced report/label layout object rename geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1622: live unplaced report/label layout object rename geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto rename_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--rename-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-offband-guid",
                "--json"
            },
            temp_root);

        if (rename_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout rename stdout:\n"
                      << rename_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout rename stderr:\n"
                      << rename_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_process.exit_code == 0,
               "#1622: live edited unplaced report/label layout object rename should exit successfully");
        expect(visual_object_count(asset_path) == before_count,
               "#1622: live edited unplaced report/label layout object rename should preserve object count");
        expect(!visual_object_exists(asset_path, "middle-field-guid"),
               "#1622: live edited unplaced report/label layout object rename should remove the old unique id");
        expect(visual_object_exists(asset_path, "middle-offband-guid"),
               "#1622: live edited unplaced report/label layout object rename should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-offband-guid", "HPOS") == "-300" &&
                   visual_object_property(asset_path, "middle-offband-guid", "VPOS") == "9000" &&
                   visual_object_property(asset_path, "middle-offband-guid", "HEIGHT") == "700",
               "#1622: live edited unplaced report/label layout object rename should preserve edited geometry fields");
        expect_contains(rename_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1622: live edited unplaced report/label layout object rename should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                            "#1882: live edited unplaced label layout object rename should retain label identity");
        }
        expect_contains(rename_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1882: live edited unplaced report/label layout object rename should keep preview bounds available");
        expect_contains(rename_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1882: live edited unplaced report/label layout object rename should expand preview left bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1882: live edited unplaced report/label layout object rename should preserve section-origin preview top bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1882: live edited unplaced report/label layout object rename should preserve preview right bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1882: live edited unplaced report/label layout object rename should expand preview bottom bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsWidth\": 450",
                        "#1882: live edited unplaced report/label layout object rename should refresh preview widths");
        expect_contains(rename_process.stdout_text, "\"previewBoundsHeight\": 7700",
                        "#1882: live edited unplaced report/label layout object rename should refresh preview heights");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1622: live edited unplaced report/label layout object rename should preserve selected-object availability");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1622: live edited unplaced report/label layout object rename should keep containing-section unavailable");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1622: live edited unplaced report/label layout object rename should serialize null containing section");
        expect_contains_in_order(
            rename_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 50",
                "\"right\": -250",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1622: live edited unplaced report/label layout object rename should refresh selected renamed geometry without fabricated section metadata");
    };

    run_live_edited_unplaced_rename(temp_root / "rename_live_edited_unplaced.frx",
                                    "rename_live_edited_unplaced.frx",
                                    "report");
    run_live_edited_unplaced_rename(temp_root / "rename_live_edited_unplaced.lbx",
                                    "rename_live_edited_unplaced.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "FONTFACE",
                "--property-value", "Consolas",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout font update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout font update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1530: report/label layout object font update should exit successfully");
        const auto font_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTFACE"
        });
        expect(font_property.ok && font_property.exists && font_property.value == "Consolas",
               "#1530: report/label layout object font update should persist the FONTFACE memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1530: report/label layout object font update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1530: report/label layout object font update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1530: report/label layout object font update should preserve object selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"value\": \"Consolas\""
            },
            "#1530: report/label layout object font update should refresh selected object highlight metadata");
    };

    run_font_update(temp_root / "font_update.frx", "font_update.frx", "report");
    run_font_update(temp_root / "font_update.lbx", "font_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "FONTFACE",
                "--property-value", "Consolas",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout font update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout font update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1632: report/label layout object stable font update should exit successfully");
        const auto font_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "FONTFACE"
        });
        expect(font_property.ok && font_property.exists && font_property.value == "Consolas",
               "#1632: report/label layout object stable font update should persist the FONTFACE memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1632: report/label layout object stable font update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1632: label layout object stable font update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1632: report/label layout object stable font update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1632: report/label layout object stable font update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1632: report/label layout object stable font update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"value\": \"Consolas\""
            },
            "#1632: report/label layout object stable font update should refresh selected object highlight metadata");
    };

    run_font_update(temp_root / "font_update_stable.frx",
                    "font_update_stable.frx",
                    "report");
    run_font_update(temp_root / "font_update_stable.lbx",
                    "font_update_stable.lbx",
                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_font_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_font_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1603: deleted report/label layout object font update fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "FONTFACE",
                "--property-value", "Consolas",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout font update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout font update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1603: deleted report/label layout object font update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1603: deleted report/label layout object font update should preserve deleted state");
        const auto font_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTFACE"
        });
        expect(font_property.ok && font_property.exists && font_property.value == "Consolas",
               "#1603: deleted report/label layout object font update should persist the FONTFACE memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1603: deleted report/label layout object font update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1603: deleted report/label layout object font update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1603: deleted report/label layout object font update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1603: deleted report/label layout object font update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1603: deleted report/label layout object font update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"value\": \"Consolas\""
            },
            "#1603: deleted report/label layout object font update should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
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
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"value\": \"Consolas\""
            },
            "#1603: deleted report/label layout object font update should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#1603: deleted report/label layout object font update should expose live containing-section JSON");
    };

    run_deleted_font_update(temp_root / "deleted_font_update.frx",
                            "deleted_font_update.frx",
                            "report");
    run_deleted_font_update(temp_root / "deleted_font_update.lbx",
                            "deleted_font_update.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "FONTFACE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout font clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout font clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1559: report/label layout object font clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1559: report/label layout object font clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1559: report/label layout object font clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1559: report/label layout object font clear should preserve object selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"value\": \"customer.company\""
            },
            "#1559: report/label layout object font clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"Segoe UI\"",
                            "#1559: report/label layout object font clear should not leak stale font values");
    };

    run_font_clear(temp_root / "font_clear.frx", "font_clear.frx", "report");
    run_font_clear(temp_root / "font_clear.lbx", "font_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "FONTFACE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout font clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout font clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1633: report/label layout object stable font clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1633: report/label layout object stable font clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1633: label layout object stable font clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1633: report/label layout object stable font clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1633: report/label layout object stable font clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1633: report/label layout object stable font clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"value\": \"customer.company\""
            },
            "#1633: report/label layout object stable font clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                            "#1633: report/label layout object stable font clear should remove font highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"Segoe UI\"",
                            "#1633: report/label layout object stable font clear should not leak stale font values");
    };

    run_font_clear(temp_root / "font_clear_stable.frx",
                   "font_clear_stable.frx",
                   "report");
    run_font_clear(temp_root / "font_clear_stable.lbx",
                   "font_clear_stable.lbx",
                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_font_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_font_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1604: deleted report/label layout object font clear fixture should start deleted");
        const auto seed_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTFACE",
            .property_value = "Consolas"
        });
        expect(seed_result.ok,
               "#1604: deleted report/label layout object font clear fixture should seed FONTFACE");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1604: deleted report/label layout object font clear seed should preserve deleted state");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "FONTFACE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout font clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout font clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1604: deleted report/label layout object font clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1604: deleted report/label layout object font clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1604: deleted report/label layout object font clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1604: deleted report/label layout object font clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1604: deleted report/label layout object font clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1604: deleted report/label layout object font clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1604: deleted report/label layout object font clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"value\": \"\\\"Deleted label\\\"\""
            },
            "#1604: deleted report/label layout object font clear should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
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
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"value\": \"\\\"Deleted label\\\"\""
            },
            "#1604: deleted report/label layout object font clear should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#1604: deleted report/label layout object font clear should expose live containing-section JSON");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                            "#1604: deleted report/label layout object font clear should remove deleted font highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"Consolas\"",
                            "#1604: deleted report/label layout object font clear should not leak stale font values");
    };

    run_deleted_font_clear(temp_root / "deleted_font_clear.frx",
                           "deleted_font_clear.frx",
                           "report");
    run_deleted_font_clear(temp_root / "deleted_font_clear.lbx",
                           "deleted_font_clear.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_font_options_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_options_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_option_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_font_options_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "FONTSIZE",
                "--property-value", "14",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout fontsize update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout fontsize update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2691: report/label layout object fontsize update should exit successfully");
        const auto font_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTSIZE"
        });
        expect(font_size_property.ok && font_size_property.exists && font_size_property.value == "14",
               "#2691: report/label layout object fontsize update should persist the FONTSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object fontsize update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object fontsize update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object fontsize update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object fontsize update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object fontsize update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\"",
                "\"name\": \"MODE\", \"recordIndex\": 3, \"fieldIndex\": 9",
                "\"value\": \"3\""
            },
            "#2691: report/label layout object fontsize update should refresh selected object highlight metadata");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "MODE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout mode clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout mode clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2691: report/label layout object mode clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object mode clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                        "#2691: report/label layout object mode clear should blank the MODE field");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object mode clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object mode clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object mode clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object mode clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\""
            },
            "#2691: report/label layout object mode clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                            "#2691: report/label layout object mode clear should remove mode highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"3\"",
                            "#2691: report/label layout object mode clear should not leak stale mode values");
    };

    run_font_option_update(temp_root / "font_options.frx", "font_options.frx", "report");
    run_font_option_update(temp_root / "font_options.lbx", "font_options.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_font_options_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_options_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_option_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_font_options_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "FONTSIZE",
                "--property-value", "14",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout fontsize update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout fontsize update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2691: report/label layout object stable fontsize update should exit successfully");
        const auto font_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "FONTSIZE"
        });
        expect(font_size_property.ok && font_size_property.exists && font_size_property.value == "14",
               "#2691: report/label layout object stable fontsize update should persist the FONTSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object stable fontsize update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object stable fontsize update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object stable fontsize update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object stable fontsize update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object stable fontsize update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\"",
                "\"name\": \"MODE\", \"recordIndex\": 3, \"fieldIndex\": 9",
                "\"value\": \"3\""
            },
            "#2691: report/label layout object stable fontsize update should refresh selected object highlight metadata");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "MODE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout mode clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout mode clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2691: report/label layout object stable mode clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object stable mode clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                        "#2691: report/label layout object stable mode clear should blank the MODE field");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object stable mode clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object stable mode clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object stable mode clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object stable mode clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\""
            },
            "#2691: report/label layout object stable mode clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                            "#2691: report/label layout object stable mode clear should remove mode highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"3\"",
                            "#2691: report/label layout object stable mode clear should not leak stale mode values");
    };

    run_font_option_update(temp_root / "font_options_stable.frx",
                           "font_options_stable.frx",
                           "report");
    run_font_option_update(temp_root / "font_options_stable.lbx",
                           "font_options_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_font_options_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_font_options_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_font_option_update = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_layout_font_options_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#2691: deleted report/label layout object font option fixture should start deleted");

        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "FONTSIZE",
                "--property-value", "12",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout fontsize update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout fontsize update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2691: deleted report/label layout object fontsize update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#2691: deleted report/label layout object fontsize update should preserve deleted state");
        const auto font_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTSIZE"
        });
        expect(font_size_property.ok && font_size_property.exists && font_size_property.value == "12",
               "#2691: deleted report/label layout object fontsize update should persist the FONTSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: deleted report/label layout object fontsize update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2691: deleted label layout object fontsize update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#2691: deleted report/label layout object fontsize update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: deleted report/label layout object fontsize update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: deleted report/label layout object fontsize update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: deleted report/label layout object fontsize update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\"",
                "\"name\": \"MODE\", \"recordIndex\": 6, \"fieldIndex\": 9",
                "\"value\": \"5\""
            },
            "#2691: deleted report/label layout object fontsize update should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
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
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\"",
                "\"name\": \"MODE\", \"recordIndex\": 6, \"fieldIndex\": 9",
                "\"value\": \"5\""
            },
            "#2691: deleted report/label layout object fontsize update should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2691: deleted report/label layout object fontsize update should expose live containing-section JSON");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "MODE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout mode clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout mode clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2691: deleted report/label layout object mode clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#2691: deleted report/label layout object mode clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: deleted report/label layout object mode clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                        "#2691: deleted report/label layout object mode clear should blank the MODE field");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2691: deleted label layout object mode clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#2691: deleted report/label layout object mode clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: deleted report/label layout object mode clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: deleted report/label layout object mode clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: deleted report/label layout object mode clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\""
            },
            "#2691: deleted report/label layout object mode clear should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
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
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\""
            },
            "#2691: deleted report/label layout object mode clear should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2691: deleted report/label layout object mode clear should expose live containing-section JSON");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 6",
                            "#2691: deleted report/label layout object mode clear should remove deleted mode highlights");
    };

    run_deleted_font_option_update(temp_root / "deleted_font_options.frx",
                                   "deleted_font_options.frx",
                                   "report");
    run_deleted_font_option_update(temp_root / "deleted_font_options.lbx",
                                   "deleted_font_options.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_width_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1605: deleted report/label layout object width fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "WIDTH",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1605: deleted report/label layout object width update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1605: deleted report/label layout object width update should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.value == "2400",
               "#1605: deleted report/label layout object width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1605: deleted report/label layout object width update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1605: deleted report/label layout object width update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1605: deleted report/label layout object width update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1605: deleted report/label layout object width update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1605: deleted report/label layout object width update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1605: deleted report/label layout object width update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1605: deleted report/label layout object width update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1605: deleted report/label layout object width update should refresh selected deleted-object geometry metadata");
        expect_not_contains(update_process.stdout_text, "\"width\": 1200",
                            "#1605: deleted report/label layout object width update should not leak stale deleted-object widths");
        expect_not_contains(update_process.stdout_text, "\"right\": 2200",
                            "#1605: deleted report/label layout object width update should not leak stale deleted-object right bounds");
    };

    run_deleted_width_update(temp_root / "deleted_width_update.frx",
                             "deleted_width_update.frx",
                             "report");
    run_deleted_width_update(temp_root / "deleted_width_update.lbx",
                             "deleted_width_update.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_width_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1606: deleted report/label layout object width clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1606: deleted report/label layout object width clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1606: deleted report/label layout object width clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1606: deleted report/label layout object width clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1606: deleted report/label layout object width clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1606: deleted report/label layout object width clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1606: deleted report/label layout object width clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1606: deleted report/label layout object width clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1606: deleted report/label layout object width clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1606: deleted report/label layout object width clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1606: deleted report/label layout object width clear should refresh selected deleted-object geometry metadata");
        expect_not_contains(clear_process.stdout_text, "\"width\": 1200",
                            "#1606: deleted report/label layout object width clear should not leak stale deleted-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2200",
                            "#1606: deleted report/label layout object width clear should not leak stale deleted-object right bounds");
    };

    run_deleted_width_clear(temp_root / "deleted_width_clear.frx",
                            "deleted_width_clear.frx",
                            "report");
    run_deleted_width_clear(temp_root / "deleted_width_clear.lbx",
                            "deleted_width_clear.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_width_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1644: deleted report/label layout object stable width fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "WIDTH",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1644: deleted report/label layout object stable width update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1644: deleted report/label layout object stable width update should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.record_deleted &&
                   width_property.value == "2400",
               "#1644: deleted report/label layout object stable width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1644: deleted report/label layout object stable width update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1644: label deleted layout object stable width update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1644: deleted report/label layout object stable width update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1644: deleted report/label layout object stable width update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1644: deleted report/label layout object stable width update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1644: deleted report/label layout object stable width update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1644: deleted report/label layout object stable width update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1644: deleted report/label layout object stable width update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1644: deleted report/label layout object stable width update should refresh selected deleted-object geometry metadata");
        expect_not_contains(update_process.stdout_text, "\"width\": 1200",
                            "#1644: deleted report/label layout object stable width update should not leak stale deleted-object widths");
        expect_not_contains(update_process.stdout_text, "\"right\": 2200",
                            "#1644: deleted report/label layout object stable width update should not leak stale deleted-object right bounds");
    };

    run_deleted_width_update(temp_root / "deleted_width_update_stable.frx",
                             "deleted_width_update_stable.frx",
                             "report");
    run_deleted_width_update(temp_root / "deleted_width_update_stable.lbx",
                             "deleted_width_update_stable.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_width_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1645: deleted report/label layout object stable width clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1645: deleted report/label layout object stable width clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1645: deleted report/label layout object stable width clear should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.record_deleted &&
                   width_property.direct_field && width_property.value.empty(),
               "#1645: deleted report/label layout object stable width clear should blank the WIDTH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1645: deleted report/label layout object stable width clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1645: label deleted layout object stable width clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1645: deleted report/label layout object stable width clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1645: deleted report/label layout object stable width clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1645: deleted report/label layout object stable width clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1645: deleted report/label layout object stable width clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1645: deleted report/label layout object stable width clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1645: deleted report/label layout object stable width clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1645: deleted report/label layout object stable width clear should refresh selected deleted-object geometry metadata");
        expect_not_contains(clear_process.stdout_text, "\"width\": 1200",
                            "#1645: deleted report/label layout object stable width clear should not leak stale deleted-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2200",
                            "#1645: deleted report/label layout object stable width clear should not leak stale deleted-object right bounds");
    };

    run_deleted_width_clear(temp_root / "deleted_width_clear_stable.frx",
                            "deleted_width_clear_stable.frx",
                            "report");
    run_deleted_width_clear(temp_root / "deleted_width_clear_stable.lbx",
                            "deleted_width_clear_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
