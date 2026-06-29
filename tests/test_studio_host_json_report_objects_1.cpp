#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
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

void write_synthetic_report_table_for_detail_header_footer_object_json(
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
        {"9", "9", "detail header expression", "", "0", "", "300", "detail-header-guid"},
        {"5", "", "\"Header label\"", "100", "50", "700", "120", "detail-header-label-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "detail-footer-guid"},
        {"8", "", "footer.total", "140", "360", "900", "100", "detail-footer-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1764: synthetic report table for detail header/footer object JSON should be created");
}

void write_synthetic_report_table_for_detail_header_footer_object_distribution_json(
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
        {"9", "9", "detail header expression", "", "0", "", "300", "detail-header-guid"},
        {"5", "", "\"Header left\"", "100", "50", "50", "100", "detail-header-left-guid"},
        {"5", "", "\"Header middle\"", "175", "60", "50", "100", "detail-header-middle-guid"},
        {"5", "", "\"Header right\"", "700", "70", "50", "100", "detail-header-right-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "detail-footer-guid"},
        {"8", "", "footer.left", "140", "360", "50", "100", "detail-footer-left-guid"},
        {"8", "", "footer.middle", "200", "370", "50", "100", "detail-footer-middle-guid"},
        {"8", "", "footer.right", "740", "380", "50", "100", "detail-footer-right-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1797: synthetic report table for detail header/footer object distribution JSON should be created");
}

void write_synthetic_report_table_for_detail_header_footer_object_vertical_distribution_json(
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
        {"9", "9", "detail header expression", "", "0", "", "300", "detail-header-guid"},
        {"5", "", "\"Header left\"", "100", "40", "50", "100", "detail-header-left-guid"},
        {"5", "", "\"Header middle\"", "175", "42", "50", "100", "detail-header-middle-guid"},
        {"5", "", "\"Header right\"", "700", "90", "50", "100", "detail-header-right-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "detail-footer-guid"},
        {"8", "", "footer.left", "140", "330", "50", "100", "detail-footer-left-guid"},
        {"8", "", "footer.middle", "200", "335", "50", "100", "detail-footer-middle-guid"},
        {"8", "", "footer.right", "740", "390", "50", "100", "detail-footer-right-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1799: synthetic report table for detail header/footer object vertical distribution JSON should be created");
}

void write_synthetic_report_table_for_detail_header_footer_object_font_json(
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
        {.name = "FONTSIZE", .type = 'C', .length = 16U},
        {.name = "MODE", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "300", "", "", "", "detail-header-guid"},
        {"5", "", "\"Header label\"", "100", "50", "700", "120", "Courier New", "12", "1",
         "detail-header-label-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "", "", "", "detail-footer-guid"},
        {"8", "", "footer.total", "140", "360", "900", "100", "Segoe UI", "10", "2",
         "detail-footer-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1773: synthetic report table for detail header/footer object font JSON should be created");
}

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

void write_synthetic_report_table_for_stable_summary_object_json(
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
        {"9", "8", "", "", "3200", "", "700", "summary-section-guid"},
        {"5", "", "\"Summary label\"", "400", "3300", "1500", "250", "summary-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1696: synthetic report table for stable summary object JSON should be created");
}

void write_synthetic_report_table_for_deleted_summary_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_summary_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1697: synthetic report table should mark summary object deleted");
}

void write_synthetic_report_table_for_ambiguous_summary_object_json(
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
        {"9", "8", "", "", "3200", "", "700", "summary-section-guid"},
        {"5", "", "\"Summary label one\"", "400", "3300", "1500", "250", "duplicate-summary-guid"},
        {"5", "", "\"Summary label two\"", "2100", "3350", "1500", "250", "DUPLICATE-SUMMARY-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1699: synthetic report table for ambiguous stable summary object JSON should be created");
}

void write_synthetic_report_table_for_live_deleted_ambiguous_summary_object_json(
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
        {"9", "8", "", "", "3200", "", "700", "summary-section-guid"},
        {"5", "", "\"Live summary label\"", "400", "3300", "1500", "250", "duplicate-live-deleted-guid"},
        {"5", "", "\"Deleted summary label\"", "2100", "3350", "1500", "250", "DUPLICATE-LIVE-DELETED-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1702: synthetic report table for live/deleted ambiguous object JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1702: synthetic report table should mark duplicate object deleted");
}

void write_synthetic_report_table_for_padded_stable_object_json(
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
        {"5", "", "\"Padded label\"", "400", "1200", "1500", "250", "  padded-object-guid  "}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1703: synthetic report table for padded stable object JSON should be created");
}

void write_synthetic_report_table_for_deep_stable_object_json(
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
        {"5", "", "\"Deep label\"", "400", "2600", "1500", "250", "deep-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1705: synthetic report table for deep stable object JSON should be created");
}

void write_synthetic_report_table_for_deep_ambiguous_stable_object_json(
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
        {"5", "", "\"Preview duplicate\"", "100", "500", "1000", "200", "deep-duplicate-guid"},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"5", "", "\"Deep duplicate\"", "400", "2600", "1500", "250", "DEEP-DUPLICATE-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1706: synthetic report table for deep ambiguous stable object JSON should be created");
}

void write_synthetic_report_table_for_deep_live_deleted_ambiguous_stable_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deep_ambiguous_stable_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 10U, true);
    expect(delete_result.ok,
           "#1709: synthetic report table should mark the deep duplicate object deleted");
}

void write_synthetic_report_table_for_stable_group_header_object_json(
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
        {"9", "3", "customer.country", "", "0", "", "600", "group-header-guid"},
        {"9", "4", "", "", "600", "", "3000", ""},
        {"9", "5", "customer.country", "", "3600", "", "500", "group-footer-guid"},
        {"5", "", "\"Group header label\"", "300", "100", "1400", "250", "group-header-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1690: synthetic report table for stable group-header object JSON should be created");
}

void write_synthetic_report_table_for_deleted_group_header_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_group_header_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1692: synthetic report table should mark group-header object deleted");
}

void write_synthetic_report_table_for_stable_group_footer_object_json(
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
        {"9", "3", "customer.country", "", "0", "", "600", "group-header-guid"},
        {"9", "4", "", "", "600", "", "3000", ""},
        {"9", "5", "customer.country", "", "3600", "", "500", "group-footer-guid"},
        {"5", "", "\"Group footer label\"", "350", "3700", "1450", "250", "group-footer-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1691: synthetic report table for stable group-footer object JSON should be created");
}

void write_synthetic_report_table_for_deleted_group_footer_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_group_footer_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1693: synthetic report table should mark group-footer object deleted");
}

void write_synthetic_report_table_for_stable_title_object_json(
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
        {"1", "53", "", "", "", "", "", ""},
        {"9", "0", "", "", "0", "", "700", "title-section-guid"},
        {"9", "4", "", "", "700", "", "2500", ""},
        {"9", "7", "", "", "3200", "", "500", ""},
        {"5", "", "\"Title label\"", "100", "120", "1400", "300", "title-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1682: synthetic report table for stable title object JSON should be created");
}

void write_synthetic_report_table_for_deleted_title_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_title_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1686: synthetic report table should mark title object deleted");
}

void write_synthetic_report_table_for_stable_page_footer_object_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 28U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "", "", "", "", ""},
        {"9", "0", "", "", "0", "", "700", "title-section-guid"},
        {"9", "4", "", "", "700", "", "2500", ""},
        {"9", "7", "", "", "3200", "", "500", "page-footer-section-guid"},
        {"5", "", "\"Page footer label\"", "150", "3300", "1600", "300", "page-footer-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1683: synthetic report table for stable page-footer object JSON should be created");
}

void write_synthetic_report_table_for_deleted_page_footer_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_page_footer_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1687: synthetic report table should mark page-footer object deleted");
}

void write_synthetic_report_table_for_stable_column_header_object_json(
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
        {"1", "53", "", "", "", "", "", ""},
        {"9", "2", "", "", "0", "", "450", "column-header-section-guid"},
        {"9", "4", "", "", "450", "", "2600", ""},
        {"9", "6", "", "", "3050", "", "400", "column-footer-section-guid"},
        {"5", "", "\"Column header label\"", "200", "100", "1700", "250", "column-header-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1684: synthetic report table for stable column-header object JSON should be created");
}

void write_synthetic_report_table_for_deleted_column_header_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_column_header_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1688: synthetic report table should mark column-header object deleted");
}

void write_synthetic_report_table_for_stable_column_footer_object_json(
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
        {"1", "53", "", "", "", "", "", ""},
        {"9", "2", "", "", "0", "", "450", "column-header-section-guid"},
        {"9", "4", "", "", "450", "", "2600", ""},
        {"9", "6", "", "", "3050", "", "400", "column-footer-section-guid"},
        {"5", "", "\"Column footer label\"", "250", "3150", "1750", "250", "column-footer-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1685: synthetic report table for stable column-footer object JSON should be created");
}

void write_synthetic_report_table_for_deleted_column_footer_object_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_column_footer_object_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1689: synthetic report table should mark column-footer object deleted");
}

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
    expect_contains(process.stdout_text, "\"documentTitleFieldIndex\": null",
                    "#1452: report layout JSON should expose missing document-title field provenance as null");
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

void test_studio_host_json_exposes_selected_report_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_report_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

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

void test_studio_host_json_nudges_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_nudge_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto nudge_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--nudge-object",
            "--nudge-mode", "both",
            "--delta-hpos", "25",
            "--delta-vpos", "-100",
            "--nudge-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (nudge_process.exit_code != 0) {
        std::cerr << "studio host report object nudge stdout:\n" << nudge_process.stdout_text << "\n";
        std::cerr << "studio host report object nudge stderr:\n" << nudge_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(nudge_process.exit_code == 0,
           "#1463: report layout object nudge should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "1225" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2500",
           "#1463: report layout object nudge should mutate FRX HPOS and VPOS fields");
    expect_contains(nudge_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1463: nudged report object JSON should retain selected-object availability");
    expect_contains(nudge_process.stdout_text, "\"left\": 1225",
                    "#1463: nudged report object JSON should expose updated left coordinates");
    expect_contains(nudge_process.stdout_text, "\"top\": 2500",
                    "#1463: nudged report object JSON should expose updated top coordinates");
    expect_contains(nudge_process.stdout_text, "\"right\": 5225",
                    "#1463: nudged report object JSON should recompute right-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"bottom\": 2950",
                    "#1463: nudged report object JSON should recompute bottom-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeTop\": 500",
                    "#1463: nudged report object JSON should recompute section-relative top coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeBottom\": 950",
                    "#1463: nudged report object JSON should recompute section-relative bottom coordinates");
    expect_contains(nudge_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1463: nudged report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_aligns_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_align_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto align_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "label-guid",
            "--align-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (align_process.exit_code != 0) {
        std::cerr << "studio host report object align stdout:\n" << align_process.stdout_text << "\n";
        std::cerr << "studio host report object align stderr:\n" << align_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(align_process.exit_code == 0,
           "#1464: report layout object alignment should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "900" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2600",
           "#1464: report layout object left alignment should mutate FRX HPOS and preserve VPOS");
    expect(visual_object_property(report_path, "label-guid", "HPOS") == "900",
           "#1464: report layout object alignment should preserve anchor geometry");
    expect_contains(align_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1464: aligned report object JSON should retain selected-object availability");
    expect_contains(align_process.stdout_text, "\"left\": 900",
                    "#1464: aligned report object JSON should expose updated left coordinates");
    expect_contains(align_process.stdout_text, "\"top\": 2600",
                    "#1464: aligned report object JSON should preserve top coordinates");
    expect_contains(align_process.stdout_text, "\"right\": 4900",
                    "#1464: aligned report object JSON should recompute right-edge coordinates");
    expect_contains(align_process.stdout_text, "\"bottom\": 3050",
                    "#1464: aligned report object JSON should preserve bottom-edge coordinates");
    expect_contains(align_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1464: aligned report object JSON should preserve section-relative top coordinates");
    expect_contains(align_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1464: aligned report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_resize_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto resize_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--resize-object",
            "--resize-mode", "size",
            "--anchor-unique-id", "label-guid",
            "--resize-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (resize_process.exit_code != 0) {
        std::cerr << "studio host report object resize stdout:\n" << resize_process.stdout_text << "\n";
        std::cerr << "studio host report object resize stderr:\n" << resize_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(resize_process.exit_code == 0,
           "#1465: report layout object resize should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "WIDTH") == "1800" &&
               visual_object_property(report_path, "field-guid", "HEIGHT") == "350",
           "#1465: report layout object size resize should mutate FRX WIDTH and HEIGHT fields");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "1200" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2600",
           "#1465: report layout object size resize should preserve FRX HPOS and VPOS fields");
    expect_contains(resize_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1465: resized report object JSON should retain selected-object availability");
    expect_contains(resize_process.stdout_text, "\"left\": 1200",
                    "#1465: resized report object JSON should preserve left coordinates");
    expect_contains(resize_process.stdout_text, "\"top\": 2600",
                    "#1465: resized report object JSON should preserve top coordinates");
    expect_contains(resize_process.stdout_text, "\"width\": 1800",
                    "#1465: resized report object JSON should expose updated width");
    expect_contains(resize_process.stdout_text, "\"height\": 350",
                    "#1465: resized report object JSON should expose updated height");
    expect_contains(resize_process.stdout_text, "\"right\": 3000",
                    "#1465: resized report object JSON should recompute right-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"bottom\": 2950",
                    "#1465: resized report object JSON should recompute bottom-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1465: resized report object JSON should preserve section-relative top coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeBottom\": 950",
                    "#1465: resized report object JSON should recompute section-relative bottom coordinates");
    expect_contains(resize_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1465: resized report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_snap_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto snap_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--snap-object",
            "--snap-mode", "both",
            "--grid-width", "700",
            "--grid-height", "750",
            "--snap-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (snap_process.exit_code != 0) {
        std::cerr << "studio host report object snap stdout:\n" << snap_process.stdout_text << "\n";
        std::cerr << "studio host report object snap stderr:\n" << snap_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(snap_process.exit_code == 0,
           "#1466: report layout object snap should exit successfully");
    expect(visual_object_property(report_path, "field-guid", "HPOS") == "1400" &&
               visual_object_property(report_path, "field-guid", "VPOS") == "2250",
           "#1466: report layout object snap should mutate FRX HPOS and VPOS fields");
    expect_contains(snap_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1466: snapped report object JSON should retain selected-object availability");
    expect_contains(snap_process.stdout_text, "\"left\": 1400",
                    "#1466: snapped report object JSON should expose updated left coordinates");
    expect_contains(snap_process.stdout_text, "\"top\": 2250",
                    "#1466: snapped report object JSON should expose updated top coordinates");
    expect_contains(snap_process.stdout_text, "\"right\": 5400",
                    "#1466: snapped report object JSON should recompute right-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"bottom\": 2700",
                    "#1466: snapped report object JSON should recompute bottom-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeTop\": 250",
                    "#1466: snapped report object JSON should recompute section-relative top coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeBottom\": 700",
                    "#1466: snapped report object JSON should recompute section-relative bottom coordinates");
    expect_contains(snap_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1466: snapped report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--delete-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host report object delete stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host report object delete stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0,
           "#1467: report layout object delete should exit successfully");
    expect(visual_object_deleted(report_path, "field-guid"),
           "#1467: report layout object delete should mark the FRX object record deleted");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1467: deleted report object JSON should move the object into deleted report objects");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1467: deleted selected report object JSON should remain available");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1467: deleted report objects should not advertise containing-section availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1467: deleted report objects should serialize null containing-section JSON");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1467: deleted report object selections should still classify as report objects");
    expect_contains(delete_process.stdout_text, "\"recordIndex\": 3",
                    "#1467: deleted selected report object JSON should preserve selected record indexes");
    expect_contains(delete_process.stdout_text, "\"deleted\": true",
                    "#1467: deleted selected report object JSON should expose deleted state");
    expect_contains(delete_process.stdout_text, "\"containingSectionRecordIndex\": null",
                    "#1467: deleted report object JSON should not fabricate containing section record indexes");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);
    const auto seed_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "field-guid",
        .deleted = true
    });
    expect(seed_delete_result.ok && visual_object_deleted(report_path, "field-guid"),
           "#1468: report layout restore fixture should start with a deleted report object");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--restore-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host report object restore stdout:\n" << restore_process.stdout_text << "\n";
        std::cerr << "studio host report object restore stderr:\n" << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0,
           "#1468: report layout object restore should exit successfully");
    expect(!visual_object_deleted(report_path, "field-guid"),
           "#1468: report layout object restore should clear the FRX object deleted flag");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1468: restored report object JSON should move the object out of deleted report objects");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1468: restored selected report object JSON should remain available");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1468: restored report objects should advertise containing-section availability");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1468: restored report objects should serialize containing-section JSON");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1468: restored report object selections should classify as report objects");
    expect_contains(restore_process.stdout_text, "\"recordIndex\": 3",
                    "#1468: restored selected report object JSON should preserve selected record indexes");
    expect_contains(restore_process.stdout_text, "\"deleted\": false",
                    "#1468: restored selected report object JSON should expose live state");
    expect_contains(restore_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1468: restored report object JSON should expose containing section ids again");
    expect_contains(restore_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1468: restored report object JSON should expose containing section record indexes again");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_distribution_json(report_path);

    const auto distribute_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--record", "3",
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-unique-id", "left-field-guid",
            "--distribute-target-unique-id", "middle-field-guid",
            "--distribute-target-unique-id", "right-field-guid",
            "--json"
        },
        temp_root);

    if (distribute_process.exit_code != 0) {
        std::cerr << "studio host report object distribute stdout:\n" << distribute_process.stdout_text << "\n";
        std::cerr << "studio host report object distribute stderr:\n" << distribute_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(distribute_process.exit_code == 0,
           "#1469: report layout object distribution should exit successfully");
    expect(visual_object_property(report_path, "left-field-guid", "HPOS") == "100" &&
               visual_object_property(report_path, "middle-field-guid", "HPOS") == "400" &&
               visual_object_property(report_path, "right-field-guid", "HPOS") == "700",
           "#1469: report layout object distribution should evenly position the middle FRX object");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1469: distributed report object JSON should retain selected-object availability");
    expect_contains(distribute_process.stdout_text, "\"left\": 400",
                    "#1469: distributed report object JSON should expose updated left coordinates");
    expect_contains(distribute_process.stdout_text, "\"right\": 450",
                    "#1469: distributed report object JSON should recompute right-edge coordinates");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectIndex\": 1",
                    "#1469: distributed report object JSON should preserve sorted section object order");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1469: distributed report object JSON should expose containing section object counts");
    expect_contains(distribute_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1469: distributed report object JSON should preserve containing section metadata");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1469: distributed report object JSON should keep selected containing-section availability");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_reorder_json(report_path);

    const auto reorder_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--reorder-object",
            "--unique-id", "right-field-guid",
            "--placement", "before",
            "--target-unique-id", "left-field-guid",
            "--json"
        },
        temp_root);

    if (reorder_process.exit_code != 0) {
        std::cerr << "studio host report object reorder stdout:\n" << reorder_process.stdout_text << "\n";
        std::cerr << "studio host report object reorder stderr:\n" << reorder_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(reorder_process.exit_code == 0,
           "#1470: report layout object reorder should exit successfully");
    expect(visual_object_order(report_path) == "right-field-guid,left-field-guid,middle-field-guid",
           "#1470: report layout object reorder should update physical FRX record order");
    expect_contains_in_order(
        reorder_process.stdout_text,
        {
            "\"sectionObjectIndex\": 0",
            "\"expression\": \"right.value\"",
            "\"sectionObjectIndex\": 1",
            "\"expression\": \"left.value\"",
            "\"sectionObjectIndex\": 2",
            "\"expression\": \"middle.value\""
        },
        "#1470: report layout JSON should serialize tied-geometry section objects in reordered record order");
    expect_contains(reorder_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1470: reordered report object JSON should expose containing section object counts");
    expect_contains(reorder_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1470: reordered report object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_report_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const std::size_t before_count = visual_object_count(report_path);

    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--duplicate-object",
            "--unique-id", "middle-field-guid",
            "--new-unique-id", "middle-copy-guid",
            "--json"
        },
        temp_root);

    if (duplicate_process.exit_code != 0) {
        std::cerr << "studio host report object duplicate stdout:\n" << duplicate_process.stdout_text << "\n";
        std::cerr << "studio host report object duplicate stderr:\n" << duplicate_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(duplicate_process.exit_code == 0,
           "#1471: report layout object duplicate should exit successfully");
    expect(visual_object_count(report_path) == before_count + 1U,
           "#1471: report layout object duplicate should append one FRX object record");
    expect(visual_object_exists(report_path, "middle-copy-guid"),
           "#1471: report layout object duplicate should persist replacement unique ids");
    expect(visual_object_order(report_path) == "left-field-guid,middle-field-guid,right-field-guid,middle-copy-guid",
           "#1471: report layout object duplicate should append the copied FRX object after existing layout objects");
    expect_contains_in_order(
        duplicate_process.stdout_text,
        {
            "\"recordIndex\": 5",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 3",
            "\"sectionObjectCount\": 4",
            "\"expression\": \"middle.value\""
        },
        "#1471: report layout JSON should expose the duplicated object in refreshed section membership");
    expect_contains(duplicate_process.stdout_text, "\"sectionObjectCount\": 4",
                    "#1471: duplicated report object JSON should refresh containing section object counts");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_report_layout_object_identity_by_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const std::size_t before_count = visual_object_count(report_path);

    const auto rename_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--rename-object",
            "--unique-id", "right-field-guid",
            "--new-unique-id", "renamed-right-guid",
            "--json"
        },
        temp_root);

    if (rename_process.exit_code != 0) {
        std::cerr << "studio host report object rename stdout:\n" << rename_process.stdout_text << "\n";
        std::cerr << "studio host report object rename stderr:\n" << rename_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(rename_process.exit_code == 0,
           "#1472: report layout object rename should exit successfully");
    expect(visual_object_count(report_path) == before_count,
           "#1472: report layout object rename should preserve FRX object count");
    expect(!visual_object_exists(report_path, "right-field-guid"),
           "#1472: report layout object rename should remove the old unique id");
    expect(visual_object_exists(report_path, "renamed-right-guid"),
           "#1472: report layout object rename should persist replacement unique ids");
    expect(visual_object_order(report_path) == "left-field-guid,middle-field-guid,renamed-right-guid",
           "#1472: report layout object rename should preserve physical FRX object order");
    expect_contains_in_order(
        rename_process.stdout_text,
        {
            "\"recordIndex\": 4",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 2",
            "\"sectionObjectCount\": 3",
            "\"expression\": \"right.value\""
        },
        "#1472: report layout JSON should keep the renamed object in refreshed section membership");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_expression_update = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--property-value", "customer.contact",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1529: report/label layout object expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.contact",
               "#1529: report/label layout object expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1529: report/label layout object expression update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1529: report/label layout object expression update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1529: report/label layout object expression update should preserve object selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.contact\"",
                "\"expression\": \"customer.contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1529: report/label layout object expression update should refresh selected object expression metadata");
    };

    run_expression_update(temp_root / "expression_update.frx", "expression_update.frx", "report");
    run_expression_update(temp_root / "expression_update.lbx", "expression_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_expression_update = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "EXPR",
                "--property-value", "customer.contact",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1630: report/label layout object stable expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.contact",
               "#1630: report/label layout object stable expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1630: report/label layout object stable expression update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1630: label layout object stable expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1630: report/label layout object stable expression update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1630: report/label layout object stable expression update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1630: report/label layout object stable expression update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.contact\"",
                "\"expression\": \"customer.contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1630: report/label layout object stable expression update should refresh selected object expression metadata");
    };

    run_expression_update(temp_root / "expression_update_stable.frx",
                          "expression_update_stable.frx",
                          "report");
    run_expression_update(temp_root / "expression_update_stable.lbx",
                          "expression_update_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_expression_update = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1601: deleted report/label layout object expression fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "EXPR",
                "--property-value", "customer.deleted_contact",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1601: deleted report/label layout object expression update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1601: deleted report/label layout object expression update should preserve deleted state");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.deleted_contact",
               "#1601: deleted report/label layout object expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1601: deleted report/label layout object expression update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1601: deleted report/label layout object expression update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1601: deleted report/label layout object expression update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1601: deleted report/label layout object expression update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1601: deleted report/label layout object expression update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1601: deleted report/label layout object expression update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"objectKind\": \"label\"",
                "\"title\": \"customer.deleted_contact\"",
                "\"expression\": \"customer.deleted_contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1601: deleted report/label layout object expression update should refresh deleted-object expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"objectKind\": \"label\"",
                "\"title\": \"customer.deleted_contact\"",
                "\"expression\": \"customer.deleted_contact\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1601: deleted report/label layout object expression update should refresh selected deleted-object metadata");
        expect_not_contains(update_process.stdout_text, "\"expression\": \"\\\"Deleted label\\\"\"",
                            "#1601: deleted report/label layout object expression update should not leak stale expression values");
    };

    run_deleted_expression_update(temp_root / "deleted_expression_update.frx",
                                  "deleted_expression_update.frx",
                                  "report");
    run_deleted_expression_update(temp_root / "deleted_expression_update.lbx",
                                  "deleted_expression_update.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_expression_clear = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
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
            std::cerr << "studio host " << label << " layout expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1558: report/label layout object expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1558: report/label layout object expression clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1558: report/label layout object expression clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1558: report/label layout object expression clear should preserve object selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"field-guid\"",
                "\"titleFieldIndex\": 9",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1558: report/label layout object expression clear should refresh selected object expression metadata");
        expect_not_contains(clear_process.stdout_text, "\"expression\": \"customer.company\"",
                            "#1558: report/label layout object expression clear should not leak stale expression values");
    };

    run_expression_clear(temp_root / "expression_clear.frx", "expression_clear.frx", "report");
    run_expression_clear(temp_root / "expression_clear.lbx", "expression_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_expression_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_expression_clear = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1631: report/label layout object stable expression clear should exit successfully");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"EXPR\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 2",
                        "#1631: report/label layout object stable expression clear should blank the EXPR memo field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1631: report/label layout object stable expression clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1631: label layout object stable expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1631: report/label layout object stable expression clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1631: report/label layout object stable expression clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1631: report/label layout object stable expression clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"field-guid\"",
                "\"titleFieldIndex\": 9",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1631: report/label layout object stable expression clear should refresh selected object expression metadata");
        expect_not_contains(clear_process.stdout_text, "\"expression\": \"customer.company\"",
                            "#1631: report/label layout object stable expression clear should not leak stale expression values");
    };

    run_expression_clear(temp_root / "expression_clear_stable.frx",
                         "expression_clear_stable.frx",
                         "report");
    run_expression_clear(temp_root / "expression_clear_stable.lbx",
                         "expression_clear_stable.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_expression_clear = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1602: deleted report/label layout object expression clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1602: deleted report/label layout object expression clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1602: deleted report/label layout object expression clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1602: deleted report/label layout object expression clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1602: deleted report/label layout object expression clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1602: deleted report/label layout object expression clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1602: deleted report/label layout object expression clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1602: deleted report/label layout object expression clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1602: deleted report/label layout object expression clear should serialize null containing-section metadata");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"EXPR\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 2",
                        "#1602: deleted report/label layout object expression clear should blank the EXPR memo property");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 6\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1602: deleted report/label layout object expression clear should refresh deleted-object expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"objectKind\": \"label\"",
                "\"title\": \"Record 6\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#1602: deleted report/label layout object expression clear should refresh selected deleted-object metadata");
        expect_not_contains(clear_process.stdout_text, "\"expression\": \"\\\"Deleted label\\\"\"",
                            "#1602: deleted report/label layout object expression clear should not leak stale expression values");
    };

    run_deleted_expression_clear(temp_root / "deleted_expression_clear.frx",
                                 "deleted_expression_clear.frx",
                                 "report");
    run_deleted_expression_clear(temp_root / "deleted_expression_clear.lbx",
                                 "deleted_expression_clear.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_unplaced_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1614: restore edited deleted layout object as unplaced fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "6",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted unplaced " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted unplaced " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1614: deleted report/label layout object unplaced pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1614: deleted report/label layout object unplaced pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "-300");
        set_deleted_geometry("VPOS", "9000");
        set_deleted_geometry("HEIGHT", "700");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "6",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited deleted layout unplaced restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited deleted layout unplaced restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1614: edited deleted report/label layout object unplaced restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1614: edited deleted report/label layout object unplaced restore should clear deleted state");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1614: edited deleted report/label layout object unplaced restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1892: label edited deleted layout object unplaced restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1614: edited deleted report/label layout object unplaced restore should keep preview bounds available");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1892: edited deleted report/label layout object unplaced restore should preserve preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1614: edited deleted report/label layout object unplaced restore should preserve preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5500",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 9700",
                        "#1614: edited deleted report/label layout object unplaced restore should expand preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1892: edited deleted report/label layout object unplaced restore should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1614: edited deleted report/label layout object unplaced restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1614: edited deleted report/label layout object unplaced restore should not count out-of-band restored objects as placed");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1614: edited deleted report/label layout object unplaced restore should add restored objects to unplaced counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1614: edited deleted report/label layout object unplaced restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1614: edited deleted report/label layout object unplaced restore should not fabricate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1614: edited deleted report/label layout object unplaced restore should serialize null containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 1200",
                "\"right\": 900",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1614: edited deleted report/label layout object unplaced restore should refresh selected unplaced geometry without section metadata");
    };

    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced.frx",
                                 "deleted_restore_unplaced.frx",
                                 "report");
    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced.lbx",
                                 "deleted_restore_unplaced.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_unplaced_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_unplaced_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1653: stable restore edited deleted layout object as unplaced fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-label-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted unplaced " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted unplaced " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1653: stable deleted report/label layout object unplaced pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1653: stable deleted report/label layout object unplaced pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "-300");
        set_deleted_geometry("VPOS", "9000");
        set_deleted_geometry("HEIGHT", "700");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "deleted-label-guid",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable edited deleted layout unplaced restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable edited deleted layout unplaced restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1653: stable edited deleted report/label layout object unplaced restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1653: stable edited deleted report/label layout object unplaced restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && !left_property.record_deleted &&
                   left_property.value == "-300" &&
                   top_property.ok && top_property.exists && !top_property.record_deleted &&
                   top_property.value == "9000" &&
                   height_property.ok && height_property.exists && !height_property.record_deleted &&
                   height_property.value == "700",
               "#1653: stable edited deleted report/label layout object unplaced restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1653: stable edited deleted report/label layout object unplaced restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1653: label stable edited deleted layout object unplaced restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1653: stable edited deleted report/label layout object unplaced restore should keep preview bounds available");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1893: stable edited deleted report/label layout object unplaced restore should preserve preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1653: stable edited deleted report/label layout object unplaced restore should preserve preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5500",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 9700",
                        "#1653: stable edited deleted report/label layout object unplaced restore should expand preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1893: stable edited deleted report/label layout object unplaced restore should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1653: stable edited deleted report/label layout object unplaced restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1653: stable edited deleted report/label layout object unplaced restore should not count out-of-band restored objects as placed");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1653: stable edited deleted report/label layout object unplaced restore should add restored objects to unplaced counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1653: stable edited deleted report/label layout object unplaced restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1653: stable edited deleted report/label layout object unplaced restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1653: stable edited deleted report/label layout object unplaced restore should not fabricate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1653: stable edited deleted report/label layout object unplaced restore should serialize null containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 1200",
                "\"right\": 900",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1653: stable edited deleted report/label layout object unplaced restore should refresh selected unplaced geometry without section metadata");
    };

    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced_stable.frx",
                                 "deleted_restore_unplaced_stable.frx",
                                 "report");
    run_deleted_unplaced_restore(temp_root / "deleted_restore_unplaced_stable.lbx",
                                 "deleted_restore_unplaced_stable.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_applies_report_deleted_states_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_deleted_states_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_batch_delete = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted-states batch delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted-states batch delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1839: report/label stable deleted-states batch delete should exit successfully");
        expect(dbf_record_deleted(asset_path, 0U) && dbf_record_deleted(asset_path, 1U),
               "#1839: report/label stable deleted-states batch delete should mark settings and section rows deleted");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1839: report/label stable deleted-states batch delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1839: label stable deleted-states batch delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should expose live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 100",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8000",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh live preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2200",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2900",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview height");
        expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                        "#1839: report/label stable deleted-states batch delete should remove live settings");
        expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1839: report/label stable deleted-states batch delete should expose deleted settings");
        expect_contains(delete_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1839: report/label stable deleted-states batch delete should clear live page setup");
        expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                        "#1839: report/label stable deleted-states batch delete should remove the section from live counts");
        expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1839: report/label stable deleted-states batch delete should expose deleted section counts");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0"
            },
            "#1839: report/label stable deleted-states batch delete should move settings into deleted metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true"
            },
            "#1839: report/label stable deleted-states batch delete should move the section into deleted metadata");
        expect_contains(delete_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1839: report/label stable deleted-states batch delete should not fabricate selected settings");
        expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1839: report/label stable deleted-states batch delete should not fabricate selected sections");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1839: report/label stable deleted-states batch delete should not fabricate a report selection");
    };

    const auto run_batch_restore = [&](const fs::path& asset_path,
                                       const std::string& title,
                                       const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_settings_and_section_json(asset_path);
        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "false",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted-states batch restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted-states batch restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1839: report/label stable deleted-states batch restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 0U) && !dbf_record_deleted(asset_path, 1U),
               "#1839: report/label stable deleted-states batch restore should restore settings and section rows");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1839: report/label stable deleted-states batch restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1839: label stable deleted-states batch restore should retain label identity");
        }
        expect_full_report_layout_preview_bounds(
            restore_process.stdout_text,
            "#2042: stable report/label settings+section deleted-states batch restore JSON");
        expect_contains(restore_process.stdout_text, "\"settingCount\": 6",
                        "#1839: report/label stable deleted-states batch restore should restore live settings");
        expect_contains(restore_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1839: report/label stable deleted-states batch restore should clear deleted settings");
        expect_contains(restore_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1839: report/label stable deleted-states batch restore should restore live page setup");
        expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                        "#1839: report/label stable deleted-states batch restore should restore live section counts");
        expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1839: report/label stable deleted-states batch restore should clear deleted section counts");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0"
            },
            "#1839: report/label stable deleted-states batch restore should move settings into live metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1839: report/label stable deleted-states batch restore should move the section into live metadata");
        expect_contains(restore_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1839: report/label stable deleted-states batch restore should not fabricate selected settings");
        expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1839: report/label stable deleted-states batch restore should not fabricate selected sections");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1839: report/label stable deleted-states batch restore should not fabricate a report selection");
    };

    const auto run_batch_rollback = [&](const fs::path& asset_path,
                                        const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "missing-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1839: report/label stable deleted-states missing-target batch should fail");
        expect(!dbf_record_deleted(asset_path, 0U) && !dbf_record_deleted(asset_path, 1U),
               "#1839: failed report/label stable deleted-states batch should roll back earlier mutations");
        expect_contains(rollback_process.stdout_text, "status: error",
                        "#1839: failed report/label stable deleted-states batch should report JSON error status");
        expect_contains(rollback_process.stdout_text, "error",
                        "#1839: failed report/label stable deleted-states batch should report an error message");
        if (asset_path.extension() == ".lbx") {
            expect(!dbf_record_deleted(asset_path, 0U) && !dbf_record_deleted(asset_path, 1U),
                   "#1839: failed label stable deleted-states batch should preserve label settings and section rows");
        }
        (void)label;
    };

    run_batch_delete(temp_root / "deleted_states_delete_stable.frx",
                     "deleted_states_delete_stable.frx",
                     "report");
    run_batch_delete(temp_root / "deleted_states_delete_stable.lbx",
                     "deleted_states_delete_stable.lbx",
                     "label");
    run_batch_restore(temp_root / "deleted_states_restore_stable.frx",
                      "deleted_states_restore_stable.frx",
                      "report");
    run_batch_restore(temp_root / "deleted_states_restore_stable.lbx",
                      "deleted_states_restore_stable.lbx",
                      "label");
    run_batch_rollback(temp_root / "deleted_states_rollback_stable.frx",
                       "report");
    run_batch_rollback(temp_root / "deleted_states_rollback_stable.lbx",
                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_applies_report_object_deleted_states_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_object_deleted_states_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_object_batch_delete = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "label-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable object deleted-states batch delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable object deleted-states batch delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1840: report/label stable object deleted-states batch delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid") &&
                   visual_object_deleted(asset_path, "label-guid"),
               "#1840: report/label stable object deleted-states batch delete should mark both object rows deleted");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1840: report/label stable object deleted-states batch delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1840: label stable object deleted-states batch delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh live preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2043: stable report/label object deleted-states batch delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3050",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4300",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2950",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview height");
        expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch delete should remove objects from live counts");
        expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1840: report/label stable object deleted-states batch delete should remove placed live objects");
        expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch delete should preserve unrelated unplaced objects");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 3",
                        "#1840: report/label stable object deleted-states batch delete should expose deleted object counts");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"objectKind\": \"field\"",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"objectKind\": \"label\""
            },
            "#1840: report/label stable object deleted-states batch delete should move both objects into deleted metadata");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1840: report/label stable object deleted-states batch delete should not fabricate selected objects");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1840: report/label stable object deleted-states batch delete should not fabricate containing sections");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1840: report/label stable object deleted-states batch delete should not fabricate a report selection");
    };

    const auto run_object_batch_restore = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto field_delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "field-guid",
            .deleted = true
        });
        const auto label_delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "label-guid",
            .deleted = true
        });
        expect(field_delete_result.ok && label_delete_result.ok &&
                   visual_object_deleted(asset_path, "field-guid") &&
                   visual_object_deleted(asset_path, "label-guid"),
               "#1840: report/label stable object deleted-states restore fixture should start deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "label-guid",
                "--deleted-state", "false",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable object deleted-states batch restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable object deleted-states batch restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1840: report/label stable object deleted-states batch restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "field-guid") &&
                   !visual_object_deleted(asset_path, "label-guid"),
               "#1840: report/label stable object deleted-states batch restore should restore both object rows");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1840: report/label stable object deleted-states batch restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1840: label stable object deleted-states batch restore should retain label identity");
        }
        expect_full_report_layout_preview_bounds(
            restore_process.stdout_text,
            "#2043: stable report/label object deleted-states batch restore JSON");
        expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1840: report/label stable object deleted-states batch restore should restore live object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1840: report/label stable object deleted-states batch restore should restore placed object counts");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch restore should preserve unrelated unplaced objects");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch restore should clear restored deleted objects");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"recordIndex\": 1",
                "\"objectCount\": 1",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"objectKind\": \"label\"",
                "\"recordIndex\": 2",
                "\"objectCount\": 1",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"objectKind\": \"field\""
            },
            "#1840: report/label stable object deleted-states batch restore should move both objects into live section metadata");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1840: report/label stable object deleted-states batch restore should not fabricate selected objects");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1840: report/label stable object deleted-states batch restore should not fabricate containing sections");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1840: report/label stable object deleted-states batch restore should not fabricate a report selection");
    };

    const auto run_object_batch_rollback = [&](const fs::path& asset_path,
                                               const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "missing-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1840: report/label stable object deleted-states missing-target batch should fail");
        expect(!visual_object_deleted(asset_path, "field-guid") &&
                   !visual_object_deleted(asset_path, "label-guid"),
               "#1840: failed report/label stable object deleted-states batch should roll back earlier mutations");
        expect_contains(rollback_process.stdout_text, "status: error",
                        "#1840: failed report/label stable object deleted-states batch should report JSON error status");
        expect_contains(rollback_process.stdout_text, "error",
                        "#1840: failed report/label stable object deleted-states batch should report an error message");
        (void)label;
    };

    run_object_batch_delete(temp_root / "object_deleted_states_delete.frx",
                            "object_deleted_states_delete.frx",
                            "report");
    run_object_batch_delete(temp_root / "object_deleted_states_delete.lbx",
                            "object_deleted_states_delete.lbx",
                            "label");
    run_object_batch_restore(temp_root / "object_deleted_states_restore.frx",
                             "object_deleted_states_restore.frx",
                             "report");
    run_object_batch_restore(temp_root / "object_deleted_states_restore.lbx",
                             "object_deleted_states_restore.lbx",
                             "label");
    run_object_batch_rollback(temp_root / "object_deleted_states_rollback.frx",
                              "report");
    run_object_batch_rollback(temp_root / "object_deleted_states_rollback.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_applies_report_object_subtree_deleted_state_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_object_subtree_deleted_state_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_subtree_delete_restore = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--subtree-deleted-state",
                "--subtree-deleted", "true",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1857: report/label stable object subtree delete should exit successfully");
        expect(visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid"),
               "#1857: report/label stable object subtree delete should mark only the selected flat layout row");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1857: report/label stable object subtree delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1857: label stable object subtree delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview height");
        expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1857: report/label stable object subtree delete should remove the object from live counts");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1857: report/label stable object subtree delete should expose deleted object counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1857: report/label stable object subtree delete should leave the report section selection empty");
        expect_contains(delete_process.stdout_text, "\"selectedReportSection\": null",
                        "#1857: report/label stable object subtree delete should serialize a null report section selection");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1857: report/label stable object subtree delete should preserve selected-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1857: report/label stable object subtree delete should preserve containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1857: report/label stable object subtree delete should preserve report object selection kind");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1857: report/label stable object subtree delete should serialize selected deleted-object metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 2",
                "\"deletedObjectCount\": 1"
            },
            "#1857: report/label stable object subtree delete should expose containing detail-band metadata");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--subtree-deleted-state",
                "--subtree-deleted", "false",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1857: report/label stable object subtree restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid"),
               "#1857: report/label stable object subtree restore should restore the selected row and preserve siblings");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1857: report/label stable object subtree restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1857: label stable object subtree restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview width");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview height");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2045: stable report/label object subtree restore JSON should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview left bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview top bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview right bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview width");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview height");
        expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1857: report/label stable object subtree restore should restore live object counts");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1857: report/label stable object subtree restore should clear deleted object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1857: report/label stable object subtree restore should refresh containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1857: report/label stable object subtree restore should preserve report object selection kind");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1857: report/label stable object subtree restore should refresh selected live-object metadata");
    };

    const auto run_missing_selector = [&](const fs::path& asset_path,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const auto missing_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--subtree-deleted-state",
                "--subtree-deleted", "true",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(missing_process.exit_code == 4,
               "#1857: report/label stable object subtree delete should reject missing stable selectors");
        expect_contains(missing_process.stdout_text, "status: error",
                        "#1857: missing-selector report/label subtree delete should report error status");
        expect_contains(missing_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1857: missing-selector report/label subtree delete should report selector errors");
        expect(!visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid"),
               "#1857: failed report/label stable object subtree delete should not mutate layout rows");
        (void)label;
    };

    run_subtree_delete_restore(temp_root / "object_subtree_deleted_state.frx",
                               "object_subtree_deleted_state.frx",
                               "report");
    run_subtree_delete_restore(temp_root / "object_subtree_deleted_state.lbx",
                               "object_subtree_deleted_state.lbx",
                               "label");
    run_missing_selector(temp_root / "object_subtree_deleted_state_missing.frx",
                         "report");
    run_missing_selector(temp_root / "object_subtree_deleted_state_missing.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_report_visual_object_subtrees_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_subtree_duplicate_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_subtree_duplicate = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleFieldCopy",
                "--new-name", "MiddleFieldCopy",
                "--new-unique-id", "middle-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1858: report/label stable visual-object duplicate-subtree JSON should exit successfully");
        expect_contains(duplicate_process.stdout_text, "\"visualObjectDuplicateSubtree\": {",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose a result object");
        expect_contains(duplicate_process.stdout_text, "\"rootRecordIndex\": 5",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose appended root indexes");
        expect_contains(duplicate_process.stdout_text, "\"copiedCount\": 1",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied counts");
        expect_contains(duplicate_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose affected object counts");
        expect_contains(duplicate_process.stdout_text, "\"rootObjectName\": \"MiddleFieldCopy\"",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied root object names");
        expect_contains(duplicate_process.stdout_text, "\"rootUniqueId\": \"middle-copy-guid\"",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied root unique ids");
        expect_contains(duplicate_process.stdout_text, "\"rootParentName\": \"\"",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied root parent names");
        expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose committed state");
        expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose mutation state");
        expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose undo availability");
        expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2173: report/label stable visual-object duplicate-subtree JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 1U &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "middle-copy-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,middle-copy-guid",
               "#1858: report/label stable visual-object duplicate-subtree should append one copied flat layout row");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-copy-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree duplicate reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree duplicate reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1858: report/label stable visual-object duplicate-subtree reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1858: report/label stable visual-object duplicate-subtree should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1858: label stable visual-object duplicate-subtree should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2046: stable report/label object subtree duplicate JSON should keep deleted preview unavailable");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 4",
                        "#1858: report/label stable visual-object duplicate-subtree should refresh live object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1858: report/label stable visual-object duplicate-subtree should expose copied-object section metadata");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1858: report/label stable visual-object duplicate-subtree should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": false",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 3",
                "\"sectionObjectCount\": 4",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-copy-guid\""
            },
            "#1858: report/label stable visual-object duplicate-subtree should refresh selected copied-object metadata");
    };

    const auto run_subtree_duplicate_collision = [&](const fs::path& asset_path,
                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto collision_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "RightField",
                "--new-name", "MiddleFieldCopy",
                "--new-unique-id", "middle-copy-guid",
                "--json"
            },
            temp_root);

        expect(collision_process.exit_code == 4,
               "#1858: report/label stable visual-object duplicate-subtree should reject replacement collisions");
        expect_contains(collision_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
                        "#1858: failed report/label stable visual-object duplicate-subtree JSON should not expose stale result objects");
        expect_not_contains(collision_process.stdout_text, "\"dryRun\": false",
                            "#2222: failed report/label stable visual-object duplicate-subtree JSON should not expose stale committed state");
        expect_not_contains(collision_process.stdout_text, "\"mutatesAsset\": true",
                            "#2222: failed report/label stable visual-object duplicate-subtree JSON should not expose stale mutation state");
        expect_not_contains(collision_process.stdout_text, "\"undoAvailable\": true",
                            "#2176: failed report/label stable visual-object duplicate-subtree JSON should not advertise undo availability");
        expect_not_contains(collision_process.stdout_text, "\"undoLabel\":",
                            "#2200: failed report/label stable visual-object duplicate-subtree JSON should not expose stale undo labels");
        expect_contains(collision_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1858: failed report/label stable visual-object duplicate-subtree JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "middle-copy-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1858: failed report/label stable visual-object duplicate-subtree should not mutate layout rows");
        (void)label;
    };

    const auto run_subtree_duplicate_missing_selector = [&](const fs::path& asset_path,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const auto missing_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "missing-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleFieldCopy",
                "--new-name", "MiddleFieldCopy",
                "--new-unique-id", "middle-copy-guid",
                "--json"
            },
            temp_root);

        expect(missing_process.exit_code == 4,
               "#1858: report/label stable visual-object duplicate-subtree should reject missing stable selectors");
        expect_contains(missing_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
                        "#1858: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale result objects");
        expect_not_contains(missing_process.stdout_text, "\"dryRun\": false",
                            "#2222: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale committed state");
        expect_not_contains(missing_process.stdout_text, "\"mutatesAsset\": true",
                            "#2222: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale mutation state");
        expect_not_contains(missing_process.stdout_text, "\"undoAvailable\": true",
                            "#2176: missing-selector report/label stable visual-object duplicate-subtree JSON should not advertise undo availability");
        expect_not_contains(missing_process.stdout_text, "\"undoLabel\":",
                            "#2200: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale undo labels");
        expect_contains(missing_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1858: missing-selector report/label stable visual-object duplicate-subtree JSON should report selector errors");
        expect(visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid" &&
                   !visual_object_exists(asset_path, "middle-copy-guid"),
               "#1858: missing-selector report/label stable visual-object duplicate-subtree should not mutate layout rows");
        (void)label;
    };

    run_subtree_duplicate(temp_root / "object_subtree_duplicate.frx",
                          "object_subtree_duplicate.frx",
                          "report");
    run_subtree_duplicate(temp_root / "object_subtree_duplicate.lbx",
                          "object_subtree_duplicate.lbx",
                          "label");
    run_subtree_duplicate_collision(temp_root / "object_subtree_duplicate_collision.frx",
                                    "report");
    run_subtree_duplicate_collision(temp_root / "object_subtree_duplicate_collision.lbx",
                                    "label");
    run_subtree_duplicate_missing_selector(temp_root / "object_subtree_duplicate_missing.frx",
                                           "report");
    run_subtree_duplicate_missing_selector(temp_root / "object_subtree_duplicate_missing.lbx",
                                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_deleted_report_visual_object_subtrees_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_subtree_duplicate_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_middle_deleted = [](const fs::path& asset_path) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "middle-field-guid",
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, "middle-field-guid"),
               "#1859: deleted report/label duplicate-subtree fixture should start with a deleted root row");
    };

    const auto run_deleted_subtree_duplicate = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        mark_middle_deleted(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleDeletedCopy",
                "--new-name", "MiddleDeletedCopy",
                "--new-unique-id", "middle-deleted-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1859: deleted report/label stable visual-object duplicate-subtree JSON should exit successfully");
        expect_contains(duplicate_process.stdout_text, "\"visualObjectDuplicateSubtree\": {",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose a result object");
        expect_contains(duplicate_process.stdout_text, "\"rootRecordIndex\": 5",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose appended root indexes");
        expect_contains(duplicate_process.stdout_text, "\"copiedCount\": 1",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose copied counts");
        expect_contains(duplicate_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose affected object counts");
        expect_contains(duplicate_process.stdout_text, "\"rootObjectName\": \"MiddleDeletedCopy\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose copied root object names");
        expect_contains(duplicate_process.stdout_text, "\"rootUniqueId\": \"middle-deleted-copy-guid\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose copied root unique ids");
        expect_contains(duplicate_process.stdout_text, "\"rootParentName\": \"\"",
                        "#2175: deleted report/label stable visual-object duplicate-subtree JSON should expose copied root parent names");
        expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose committed state");
        expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose mutation state");
        expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose undo availability");
        expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 1U &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "middle-deleted-copy-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,middle-deleted-copy-guid",
               "#1859: deleted report/label stable visual-object duplicate-subtree should append a deleted copied row");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-deleted-copy-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1859: deleted report/label stable visual-object duplicate-subtree reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1859: deleted label stable visual-object duplicate-subtree should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should expose original and copied deleted rows");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should leave the report section selection empty");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSection\": null",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should serialize a null report section selection");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should select the copied deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should preserve containing-section availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"title\": \"MiddleDeletedCopy\"",
                "\"expression\": \"middle.value\"",
                "\"highlightCount\": 1"
            },
            "#1859: deleted report/label stable visual-object duplicate-subtree should preserve copied deleted-object metadata");
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
                "\"objectCount\": 2",
                "\"deletedObjectCount\": 2"
            },
            "#1859: deleted report/label stable visual-object duplicate-subtree should expose containing detail-band metadata");
    };

    const auto run_deleted_subtree_duplicate_collision = [&](const fs::path& asset_path,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        mark_middle_deleted(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto collision_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleDeletedCopy",
                "--new-name", "MiddleDeletedCopy",
                "--new-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        expect(collision_process.exit_code == 4,
               "#1859: deleted report/label stable visual-object duplicate-subtree should reject replacement collisions");
        expect_contains(collision_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
                        "#1859: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale result objects");
        expect_not_contains(collision_process.stdout_text, "\"dryRun\": false",
                            "#2223: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale committed state");
        expect_not_contains(collision_process.stdout_text, "\"mutatesAsset\": true",
                            "#2223: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale mutation state");
        expect_not_contains(collision_process.stdout_text, "\"undoAvailable\": true",
                            "#2177: failed deleted report/label stable visual-object duplicate-subtree JSON should not advertise undo availability");
        expect_not_contains(collision_process.stdout_text, "\"undoLabel\":",
                            "#2200: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale undo labels");
        expect_contains(collision_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1859: failed deleted report/label stable visual-object duplicate-subtree JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_exists(asset_path, "middle-deleted-copy-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1859: failed deleted report/label stable visual-object duplicate-subtree should not mutate layout rows");
        (void)label;
    };

    run_deleted_subtree_duplicate(temp_root / "deleted_object_subtree_duplicate.frx",
                                  "deleted_object_subtree_duplicate.frx",
                                  "report");
    run_deleted_subtree_duplicate(temp_root / "deleted_object_subtree_duplicate.lbx",
                                  "deleted_object_subtree_duplicate.lbx",
                                  "label");
    run_deleted_subtree_duplicate_collision(temp_root / "deleted_object_subtree_duplicate_collision.frx",
                                            "report");
    run_deleted_subtree_duplicate_collision(temp_root / "deleted_object_subtree_duplicate_collision.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_applies_mixed_report_deleted_states_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_mixed_report_deleted_states_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_mixed_batch_delete = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable mixed deleted-states batch delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable mixed deleted-states batch delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1841: report/label stable mixed deleted-states batch delete should exit successfully");
        expect(dbf_record_deleted(asset_path, 0U) &&
                   dbf_record_deleted(asset_path, 1U) &&
                   visual_object_deleted(asset_path, "field-guid"),
               "#1841: report/label stable mixed deleted-states batch delete should mark settings, section, and object rows deleted");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1841: report/label stable mixed deleted-states batch delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1841: label stable mixed deleted-states batch delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 100",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh live preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8000",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh live preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3050",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 5200",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 3050",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview height");
        expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                        "#1841: report/label stable mixed deleted-states batch delete should remove live settings");
        expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted settings");
        expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch delete should remove the selected section from live counts");
        expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted section counts");
        expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch delete should remove the selected object from live counts");
        expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1841: report/label stable mixed deleted-states batch delete should leave no placed live objects");
        expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch delete should move section members into unplaced metadata");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted object counts");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0"
            },
            "#1841: report/label stable mixed deleted-states batch delete should move settings into deleted metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true"
            },
            "#1841: report/label stable mixed deleted-states batch delete should move the section into deleted metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"objectKind\": \"field\""
            },
            "#1841: report/label stable mixed deleted-states batch delete should move the object into deleted metadata");
        expect_contains(delete_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate selected settings");
        expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate selected sections");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate selected objects");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate a report selection");
    };

    const auto run_mixed_batch_restore = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_settings_and_section_json(asset_path);
        const auto field_delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "field-guid",
            .deleted = true
        });
        expect(field_delete_result.ok &&
                   dbf_record_deleted(asset_path, 0U) &&
                   dbf_record_deleted(asset_path, 1U) &&
                   visual_object_deleted(asset_path, "field-guid"),
               "#1841: report/label stable mixed deleted-states restore fixture should start deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "false",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable mixed deleted-states batch restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable mixed deleted-states batch restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1841: report/label stable mixed deleted-states batch restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 0U) &&
                   !dbf_record_deleted(asset_path, 1U) &&
                   !visual_object_deleted(asset_path, "field-guid"),
               "#1841: report/label stable mixed deleted-states batch restore should restore settings, section, and object rows");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1841: report/label stable mixed deleted-states batch restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1841: label stable mixed deleted-states batch restore should retain label identity");
        }
        expect_full_report_layout_preview_bounds(
            restore_process.stdout_text,
            "#2044: stable report/label mixed deleted-states batch restore JSON");
        expect_contains(restore_process.stdout_text, "\"settingCount\": 6",
                        "#1841: report/label stable mixed deleted-states batch restore should restore live settings");
        expect_contains(restore_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1841: report/label stable mixed deleted-states batch restore should clear deleted settings");
        expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch restore should restore live sections");
        expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1841: report/label stable mixed deleted-states batch restore should clear deleted sections");
        expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1841: report/label stable mixed deleted-states batch restore should restore live object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch restore should restore placed object counts");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch restore should preserve unrelated unplaced objects");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch restore should clear restored deleted objects");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0"
            },
            "#1841: report/label stable mixed deleted-states batch restore should move settings into live metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1841: report/label stable mixed deleted-states batch restore should move the section into live metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"recordIndex\": 2",
                "\"objectCount\": 1",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"objectKind\": \"field\""
            },
            "#1841: report/label stable mixed deleted-states batch restore should move the object into live section metadata");
        expect_contains(restore_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate selected settings");
        expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate selected sections");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate selected objects");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate a report selection");
    };

    const auto run_mixed_batch_rollback = [&](const fs::path& asset_path,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "missing-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1841: report/label stable mixed deleted-states missing-target batch should fail");
        expect(!dbf_record_deleted(asset_path, 0U) &&
                   !dbf_record_deleted(asset_path, 1U) &&
                   !visual_object_deleted(asset_path, "field-guid"),
               "#1841: failed report/label stable mixed deleted-states batch should roll back earlier mutations");
        expect_contains(rollback_process.stdout_text, "status: error",
                        "#1841: failed report/label stable mixed deleted-states batch should report JSON error status");
        expect_contains(rollback_process.stdout_text, "error",
                        "#1841: failed report/label stable mixed deleted-states batch should report an error message");
        (void)label;
    };

    run_mixed_batch_delete(temp_root / "mixed_deleted_states_delete.frx",
                           "mixed_deleted_states_delete.frx",
                           "report");
    run_mixed_batch_delete(temp_root / "mixed_deleted_states_delete.lbx",
                           "mixed_deleted_states_delete.lbx",
                           "label");
    run_mixed_batch_restore(temp_root / "mixed_deleted_states_restore.frx",
                            "mixed_deleted_states_restore.frx",
                            "report");
    run_mixed_batch_restore(temp_root / "mixed_deleted_states_restore.lbx",
                            "mixed_deleted_states_restore.lbx",
                            "label");
    run_mixed_batch_rollback(temp_root / "mixed_deleted_states_rollback.frx",
                             "report");
    run_mixed_batch_rollback(temp_root / "mixed_deleted_states_rollback.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_summary_report_objects_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_summary_report_objects_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "selected_summary_object_record.frx";
    write_synthetic_report_table_for_stable_summary_object_json(report_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host record-selected summary report object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host record-selected summary report object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1976: record-selected summary report object JSON should exit successfully");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"selected_summary_object_record.frx\"",
                    "#1976: record-selected summary report object JSON should preserve document titles");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1976: record-selected summary report object selections should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1976: record-selected summary report object selections should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1976: record-selected summary report object selections should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1976: record-selected summary report object JSON should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1976: record-selected summary report object JSON should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1976: record-selected summary report object JSON should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1900",
                    "#1976: record-selected summary report object JSON should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3900",
                    "#1976: record-selected summary report object JSON should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1900",
                    "#1976: record-selected summary report object JSON should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3900",
                    "#1976: record-selected summary report object JSON should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1976: record-selected summary report object JSON should not fabricate deleted preview availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1976: record-selected summary report objects should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1976: record-selected summary report objects should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1976: record-selected summary report objects should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1976: record-selected summary report objects should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                    "#1976: record-selected summary report object JSON should preserve live section counts");
    expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1976: record-selected summary report object JSON should preserve deleted section counts");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                    "#1976: record-selected summary report object JSON should preserve live object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                    "#1976: record-selected summary report object JSON should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1976: record-selected summary report objects should advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1976: record-selected summary report objects should expose containing-section JSON");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"containingSectionId\": \"summary_2\"",
            "\"containingSectionRecordIndex\": 2",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 350",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Summary label\\\"\""
        },
        "#1976: record-selected summary report object selections should expose selected-object metadata");
    expect_contains(object_process.stdout_text, "\"left\": 400",
                    "#1976: record-selected summary report object selections should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 3300",
                    "#1976: record-selected summary report object selections should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"right\": 1900",
                    "#1976: record-selected summary report object selections should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"bottom\": 3550",
                    "#1976: record-selected summary report object selections should expose selected-object bottom bounds");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": false",
            "\"sectionIndex\": 1",
            "\"sectionCount\": 2",
            "\"top\": 3200",
            "\"height\": 700",
            "\"bottom\": 3900",
            "\"objectCount\": 1"
        },
        "#1976: record-selected summary report object selections should expose containing summary metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
