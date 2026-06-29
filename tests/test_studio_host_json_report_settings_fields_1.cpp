#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_invalid_direct_column_setup_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "many", "wide?", "spaced?", "invalid-direct-live-column-settings-guid"},
        {"1", "53", "deleted-many", "deleted-wide?", "deleted-spaced?",
         "invalid-direct-deleted-column-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1734: synthetic report table with invalid direct column setup fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1734: synthetic report table should mark invalid direct column settings deleted");
}

void write_synthetic_report_table_for_invalid_direct_margin_grid_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "bottom?", "vertical?", "horizontal?", "invalid-direct-live-grid-settings-guid"},
        {"1", "53", "deleted-bottom?", "deleted-vertical?", "deleted-horizontal?",
         "invalid-direct-deleted-grid-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1735: synthetic report table with invalid direct margin/grid fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1735: synthetic report table should mark invalid direct margin/grid settings deleted");
}

void write_synthetic_report_table_for_unresolved_direct_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "<memo block 80>", "<memo block 81>", "<memo block 82>", "<memo block 83>",
         "<memo block 84>", "<memo block 85>", "<memo block 86>", "<memo block 87>",
         "<memo block 88>", "unresolved-direct-live-settings-guid"},
        {"1", "53", "<memo block 89>", "<memo block 90>", "<memo block 91>", "<memo block 92>",
         "<memo block 93>", "<memo block 94>", "<memo block 95>", "<memo block 96>",
         "<memo block 97>", "unresolved-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1741: synthetic report table with unresolved direct-setting memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok,
           "#1741: synthetic report table should mark unresolved direct-setting memo settings deleted");
}

void write_synthetic_report_table_for_mixed_direct_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "1", "<memo block 100>", "120", "<memo block 101>", "1",
         "<memo block 102>", "3", "<memo block 103>", "42", "mixed-direct-live-settings-guid"},
        {"1", "53", "<memo block 104>", "9", "<memo block 105>", "240", "<memo block 106>",
         "0", "<memo block 107>", "5000", "<memo block 108>", "mixed-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1742: synthetic report table with mixed direct-setting memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok,
           "#1742: synthetic report table should mark mixed direct-setting memo settings deleted");
}

void write_synthetic_report_table_for_blank_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "   ", "\t", " \t ", "", "  ", "\t ", "", "   ",
         "blank-direct-live-settings-guid"},
        {"1", "53", " ", "", "  ", "\t\t", " \t", "", " ", "\t", "",
         "blank-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1743: synthetic report table with blank direct-setting fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1743: synthetic report table should mark blank direct settings deleted");
}

void write_synthetic_report_table_for_mixed_invalid_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "1", "paper?", "120", "bottom?", "1", "wide-grid?", "3", "wide?", "42",
         "mixed-invalid-live-settings-guid"},
        {"1", "53", "deleted-sideways", "9", "deleted-top?", "240", "deleted-grid?", "0",
         "deleted-many?", "5000", "deleted-spacing?", "mixed-invalid-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1744: synthetic report table with mixed invalid direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1744: synthetic report table should mark mixed invalid direct settings deleted");
}

void write_synthetic_report_table_for_trimmed_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", " 1 ", "\t9", " 120\t", "\t240 ", " 1 ", "\t0\t", " 3 ", " 5000 ",
         "\t42", "trimmed-direct-live-settings-guid"},
        {"1", "53", "\t2", " 10 ", "\t360", "480\t", " 0", " 1 ", "\t4\t", " 6000",
         "84 ", "trimmed-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1745: synthetic report table with trimmed direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1745: synthetic report table should mark trimmed direct settings deleted");
}

void write_synthetic_report_table_for_fractional_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "1.9", "9.8", "120.75", "240.25", "1.1", "0.9", "3.5", "5000.99",
         "42.42", "fractional-direct-live-settings-guid"},
        {"1", "53", "2.1", "10.9", "360.5", "480.5", "0.1", "1.1", "4.9", "6000.5",
         "84.9", "fractional-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1746: synthetic report table with fractional direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1746: synthetic report table should mark fractional direct settings deleted");
}

void write_synthetic_report_table_for_oversized_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 32U},
        {.name = "PAPERSIZE", .type = 'C', .length = 32U},
        {.name = "TOPMARGIN", .type = 'C', .length = 32U},
        {.name = "BOTMARGIN", .type = 'C', .length = 32U},
        {.name = "GRIDV", .type = 'C', .length = 32U},
        {.name = "GRIDH", .type = 'C', .length = 32U},
        {.name = "COLS", .type = 'C', .length = 32U},
        {.name = "COLWIDTH", .type = 'C', .length = 32U},
        {.name = "COLSPACING", .type = 'C', .length = 32U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "999999999999", "999999999998", "999999999997", "999999999996",
         "999999999995", "999999999994", "999999999993", "999999999992", "999999999991",
         "oversized-direct-live-settings-guid"},
        {"1", "53", "888888888888", "888888888887", "888888888886", "888888888885",
         "888888888884", "888888888883", "888888888882", "888888888881", "888888888880",
         "oversized-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1747: synthetic report table with oversized direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1747: synthetic report table should mark oversized direct settings deleted");
}

void write_synthetic_report_table_for_dot_leading_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", ".1", ".9", ".120", ".240", ".1", ".0", ".3", ".5000", ".42",
         "dot-leading-direct-live-settings-guid"},
        {"1", "53", ".2", ".10", ".360", ".480", ".0", ".1", ".4", ".6000", ".84",
         "dot-leading-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1748: synthetic report table with dot-leading direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1748: synthetic report table should mark dot-leading direct settings deleted");
}

void write_synthetic_report_table_for_invalid_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=sideways\n"
         "PAPERSIZE=999999999999\n"
         "TOPMARGIN=.120\n"
         "BOTMARGIN=bottom?\n"
         "GRIDV=999999999995\n"
         "GRIDH=.0\n"
         "COLS=many\n"
         "COLWIDTH=999999999992\n"
         "COLSPACING=.42",
         "invalid-memo-live-settings-guid"},
        {"1", "53",
         "ORIENTATION=deleted-sideways\n"
         "PAPERSIZE=888888888887\n"
         "TOPMARGIN=.360\n"
         "BOTMARGIN=deleted-bottom?\n"
         "GRIDV=888888888884\n"
         "GRIDH=.1\n"
         "COLS=deleted-many\n"
         "COLWIDTH=888888888881\n"
         "COLSPACING=.84",
         "invalid-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1750: synthetic report table with invalid settings memo values should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1750: synthetic report table should mark invalid memo settings deleted");
}

void write_synthetic_report_table_for_fractional_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         " ORIENTATION = 1.9 \n"
         "PAPERSIZE=\t9.8\n"
         " TOPMARGIN = 120.75\n"
         "BOTMARGIN=240.25\n"
         "GRIDV=1.1\n"
         "GRIDH=0.9\n"
         "COLS=3.5\n"
         "COLWIDTH=5000.99\n"
         "COLSPACING=42.42",
         "fractional-memo-live-settings-guid"},
        {"1", "53",
         " ORIENTATION = 2.1\n"
         "PAPERSIZE= 10.9\n"
         "TOPMARGIN=360.5\n"
         "BOTMARGIN=480.5\n"
         "GRIDV=0.1\n"
         "GRIDH=1.1\n"
         "COLS=4.9\n"
         "COLWIDTH=6000.5\n"
         "COLSPACING=84.9",
         "fractional-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1751: synthetic report table with fractional settings memo values should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1751: synthetic report table should mark fractional memo settings deleted");
}

void write_synthetic_report_table_for_blank_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=\n"
         "PAPERSIZE=   \n"
         "TOPMARGIN=\t\n"
         "BOTMARGIN= \t \n"
         "GRIDV=\n"
         "GRIDH=  \n"
         "COLS=\t \n"
         "COLWIDTH=\n"
         "COLSPACING=   ",
         "blank-memo-live-settings-guid"},
        {"1", "53",
         "ORIENTATION= \n"
         "PAPERSIZE=\n"
         "TOPMARGIN=  \n"
         "BOTMARGIN=\t\t\n"
         "GRIDV= \t\n"
         "GRIDH=\n"
         "COLS= \n"
         "COLWIDTH=\t\n"
         "COLSPACING=",
         "blank-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1752: synthetic report table with blank settings memo values should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1752: synthetic report table should mark blank memo settings deleted");
}

void write_synthetic_report_table_for_malformed_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "orphan live memo text\n"
         " = ignored-live-name\n"
         "ORIENTATION=1\r\n"
         "live no equals\r\n"
         "PAPERSIZE=9\n"
         " = \n"
         "COLS=3",
         "malformed-memo-live-settings-guid"},
        {"1", "53",
         "deleted orphan text\n"
         " = ignored-deleted-name\r\n"
         "TOPMARGIN=120\n"
         "deleted no equals\n"
         "BOTMARGIN=240\r\n"
         " =\n"
         "COLWIDTH=5000\r\n"
         "COLSPACING=42",
         "malformed-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1753: synthetic report table with malformed settings memo lines should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1753: synthetic report table should mark malformed memo settings deleted");
}

void write_synthetic_report_table_for_duplicate_setting_precedence_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "ORIENTATION", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=1\n"
         "ORIENTATION=2\n"
         "COLS=3\n"
         "COLS=4",
         "9", "8", "duplicate-precedence-live-settings-guid"},
        {"1", "53",
         "ORIENTATION=5\n"
         "ORIENTATION=6\n"
         "COLS=7\n"
         "COLS=8",
         "10", "11", "duplicate-precedence-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1754: synthetic report table with duplicate settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1754: synthetic report table should mark duplicate settings deleted");
}

void write_synthetic_report_table_for_cr_only_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=1\r"
         "PAPERSIZE=9\r"
         "COLS=3\r"
         "COLWIDTH=5000",
         "cr-only-memo-live-settings-guid"},
        {"1", "53",
         "TOPMARGIN=120\r"
         "BOTMARGIN=240\r"
         "COLSPACING=42",
         "cr-only-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1756: synthetic report table with CR-only settings memo lines should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1756: synthetic report table should mark CR-only memo settings deleted");
}

void write_synthetic_report_table_for_mixed_case_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "orientation=1\n"
         "PaperSize=9\n"
         "TopMargin=120\n"
         "cols=3\n"
         "ColWidth=5000",
         "mixed-case-memo-live-settings-guid"},
        {"1", "53",
         "bottommargin=240\n"
         "GridV=1\n"
         "gridh=0\n"
         "ColSpacing=42",
         "mixed-case-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1757: synthetic report table with mixed-case settings memo names should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1757: synthetic report table should mark mixed-case memo settings deleted");
}

void write_synthetic_report_table_for_ambiguous_settings_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "duplicate-settings-guid"},
        {"1", "53", "PAPERSIZE=1", "", "", "DUPLICATE-SETTINGS-GUID"},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1701: synthetic report table for ambiguous stable settings JSON should be created");
}

void write_synthetic_report_table_for_live_deleted_ambiguous_settings_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "duplicate-live-deleted-guid"},
        {"1", "53", "PAPERSIZE=1", "", "", "DUPLICATE-LIVE-DELETED-GUID"},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1702: synthetic report table for live/deleted ambiguous settings JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1702: synthetic report table should mark duplicate settings deleted");
}

void write_synthetic_report_table_for_padded_stable_settings_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "  padded-settings-guid  "},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1703: synthetic report table for padded stable settings JSON should be created");
}

void write_synthetic_report_table_for_deep_stable_settings_json(
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
        {"1", "53", "PAPERSIZE=9", "", "", "", "", "deep-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1707: synthetic report table for deep stable settings JSON should be created");
}

void write_synthetic_report_table_for_deep_ambiguous_stable_settings_json(
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
        {"1", "53", "ORIENTATION=0", "", "", "", "", "deep-duplicate-settings-guid"},
        {"9", "4", "", "", "0", "", "3200", ""},
        {"5", "", "\"Preview object 2\"", "100", "200", "1000", "200", ""},
        {"5", "", "\"Preview object 3\"", "100", "500", "1000", "200", ""},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"1", "53", "PAPERSIZE=9", "", "", "", "", "DEEP-DUPLICATE-SETTINGS-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1708: synthetic report table for deep ambiguous stable settings JSON should be created");
}

void write_synthetic_report_table_for_deep_live_deleted_ambiguous_stable_settings_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deep_ambiguous_stable_settings_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 10U, true);
    expect(delete_result.ok,
           "#1709: synthetic report table should mark the deep duplicate settings row deleted");
}

void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1476: synthetic FRX table should mark report settings deleted");
}

void write_synthetic_report_table_for_stable_settings_and_section_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto settings_unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(settings_unique_id_result.ok,
           "#1839: stable deleted-state batch fixture should seed a settings unique id");
    const auto section_unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "section-guid"
    });
    expect(section_unique_id_result.ok,
           "#1839: stable deleted-state batch fixture should seed a section unique id");
    expect(!dbf_record_deleted(report_path, 0U) && !dbf_record_deleted(report_path, 1U),
           "#1839: stable deleted-state batch fixture should preserve live settings and section rows");
}

void write_synthetic_report_table_for_stable_deleted_settings_and_section_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_settings_and_section_json(report_path);
    const auto settings_delete_result =
        copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(settings_delete_result.ok,
           "#1839: stable deleted-state restore fixture should mark settings deleted");
    const auto section_delete_result =
        copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(section_delete_result.ok,
           "#1839: stable deleted-state restore fixture should mark section deleted");
    expect(dbf_record_deleted(report_path, 0U) && dbf_record_deleted(report_path, 1U),
           "#1839: stable deleted-state restore fixture should preserve deleted settings and section rows");
}

void write_synthetic_report_table_for_column_setup_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "COLS=2\nCOLWIDTH=3600\nCOLSPACING=120", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1518: synthetic report table for column setup JSON should be created");
}

void write_synthetic_report_table_for_column_setup_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "COLS", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "COLWIDTH=3600\nCOLSPACING=120", "2", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1538: synthetic report table for column setup field JSON should be created");
}

void write_synthetic_report_table_for_deleted_column_setup_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_setup_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1593: synthetic report table should mark column-count settings deleted");
}

void write_synthetic_report_table_for_stable_column_setup_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_setup_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1835: stable column-count fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1835: stable column-count fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_column_setup_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_column_setup_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1835: stable deleted column-count fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1835: stable deleted column-count fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_bottom_margin_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "BOTMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nGRIDV=4\nGRIDH=8", "20", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1541: synthetic report table for bottom margin field JSON should be created");
}

void write_synthetic_report_table_for_deleted_bottom_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_bottom_margin_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1583: synthetic report table should mark bottom-margin settings deleted");
}

void write_synthetic_report_table_for_stable_bottom_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_bottom_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1830: stable bottom-margin fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1830: stable bottom-margin fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_bottom_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_bottom_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1830: stable deleted bottom-margin fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1830: stable deleted bottom-margin fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_grid_vertical_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "GRIDV", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nBOTMARGIN=20\nGRIDH=8", "4", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1542: synthetic report table for vertical grid field JSON should be created");
}

void write_synthetic_report_table_for_deleted_grid_vertical_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_vertical_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1585: synthetic report table should mark vertical-grid settings deleted");
}

void write_synthetic_report_table_for_stable_grid_vertical_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_vertical_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1831: stable vertical-grid fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1831: stable vertical-grid fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_grid_vertical_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_grid_vertical_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1831: stable deleted vertical-grid fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1831: stable deleted vertical-grid fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_grid_horizontal_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "GRIDH", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4", "8", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1543: synthetic report table for horizontal grid field JSON should be created");
}

void write_synthetic_report_table_for_deleted_grid_horizontal_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_horizontal_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1587: synthetic report table should mark horizontal-grid settings deleted");
}

void write_synthetic_report_table_for_stable_grid_horizontal_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_horizontal_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1832: stable horizontal-grid fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1832: stable horizontal-grid fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_grid_horizontal_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_grid_horizontal_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1832: stable deleted horizontal-grid fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1832: stable deleted horizontal-grid fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_orientation_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "ORIENTATION", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "PAPERSIZE=1\nTOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "0", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1544: synthetic report table for orientation field JSON should be created");
}

void write_synthetic_report_table_for_deleted_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_orientation_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1589: synthetic report table should mark orientation settings deleted");
}

void write_synthetic_report_table_for_stable_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_orientation_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1833: stable orientation fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1833: stable orientation fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_orientation_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1833: stable deleted orientation fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1833: stable deleted orientation fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_paper_size_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "PAPERSIZE", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nTOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "1", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1545: synthetic report table for paper-size field JSON should be created");
}

void write_synthetic_report_table_for_deleted_paper_size_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_paper_size_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1591: synthetic report table should mark paper-size settings deleted");
}

void write_synthetic_report_table_for_stable_paper_size_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_paper_size_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1834: stable paper-size fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1834: stable paper-size fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_paper_size_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_paper_size_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1834: stable deleted paper-size fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1834: stable deleted paper-size fixture should preserve the deleted settings state");
}

void test_studio_host_json_exposes_report_layout_column_setup(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_column_setup_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "columns.frx";
    write_synthetic_report_table_for_column_setup_json(report_path);
    const auto report_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--json"},
        temp_root);

    if (report_process.exit_code != 0) {
        std::cerr << "studio host report column setup stdout:\n" << report_process.stdout_text << "\n";
        std::cerr << "studio host report column setup stderr:\n" << report_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(report_process.exit_code == 0,
           "#1518: report column setup JSON should exit successfully");
    expect_contains(report_process.stdout_text, "\"reportLayout\": {",
                    "#1518: report column setup JSON should expose report-layout JSON");
    expect_contains(report_process.stdout_text, "\"pageSetupAvailable\": false",
                    "#1518: report column setup JSON should not fabricate page setup availability");
    expect_contains(report_process.stdout_text, "\"columnSetupAvailable\": true",
                    "#1518: report column setup JSON should expose column setup availability");
    expect_contains(report_process.stdout_text, "\"columnCountAvailable\": true",
                    "#1518: report column setup JSON should expose column count availability");
    expect_contains(report_process.stdout_text, "\"columnCount\": 2",
                    "#1518: report column setup JSON should expose column counts");
    expect_contains(report_process.stdout_text, "\"columnWidthAvailable\": true",
                    "#1518: report column setup JSON should expose column width availability");
    expect_contains(report_process.stdout_text, "\"columnWidth\": 3600",
                    "#1518: report column setup JSON should expose column widths");
    expect_contains(report_process.stdout_text, "\"columnSpacingAvailable\": true",
                    "#1518: report column setup JSON should expose column spacing availability");
    expect_contains(report_process.stdout_text, "\"columnSpacing\": 120",
                    "#1518: report column setup JSON should expose column spacing");
    expect_contains(report_process.stdout_text, "\"previewBoundsAvailable\": false",
                    "#2017: report column setup JSON should not fabricate live preview availability");
    expect_contains(report_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2017: report column setup JSON should preserve zero live preview left bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2017: report column setup JSON should preserve zero live preview top bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2017: report column setup JSON should preserve zero live preview right bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsBottom\": 0",
                    "#2017: report column setup JSON should preserve zero live preview bottom bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2017: report column setup JSON should preserve zero live preview widths");
    expect_contains(report_process.stdout_text, "\"previewBoundsHeight\": 0",
                    "#2017: report column setup JSON should preserve zero live preview heights");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2017: report column setup JSON should not fabricate deleted preview availability");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview left bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview top bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview right bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview bottom bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview widths");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview heights");
    expect_contains(report_process.stdout_text, "\"settingCount\": 3",
                    "#1518: report column setup JSON should preserve compact setting counts");
    expect_contains(report_process.stdout_text, "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"2\"",
                    "#1518: report column setup JSON should preserve column-count setting provenance");
    expect_contains(report_process.stdout_text, "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"3600\"",
                    "#1518: report column setup JSON should preserve column-width setting provenance");
    expect_contains(report_process.stdout_text, "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \"120\"",
                    "#1518: report column setup JSON should preserve column-spacing setting provenance");

    const fs::path label_path = temp_root / "columns.lbx";
    write_synthetic_report_table_for_column_setup_json(label_path);
    const auto label_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--json"},
        temp_root);

    if (label_process.exit_code != 0) {
        std::cerr << "studio host label column setup stdout:\n" << label_process.stdout_text << "\n";
        std::cerr << "studio host label column setup stderr:\n" << label_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_process.exit_code == 0,
           "#1518: label column setup JSON should exit successfully");
    expect_contains(label_process.stdout_text, "\"isLabel\": true",
                    "#1518: label column setup JSON should preserve label identity");
    expect_contains(label_process.stdout_text, "\"pageSetupAvailable\": false",
                    "#1518: label column setup JSON should not fabricate page setup availability");
    expect_contains(label_process.stdout_text, "\"columnSetupAvailable\": true",
                    "#1518: label column setup JSON should expose column setup availability");
    expect_contains(label_process.stdout_text, "\"columnCount\": 2",
                    "#1518: label column setup JSON should expose column counts");
    expect_contains(label_process.stdout_text, "\"columnWidth\": 3600",
                    "#1518: label column setup JSON should expose column widths");
    expect_contains(label_process.stdout_text, "\"columnSpacing\": 120",
                    "#1518: label column setup JSON should expose column spacing");
    expect_contains(label_process.stdout_text, "\"previewBoundsAvailable\": false",
                    "#2017: label column setup JSON should not fabricate live preview availability");
    expect_contains(label_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2017: label column setup JSON should preserve zero live preview left bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2017: label column setup JSON should preserve zero live preview top bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2017: label column setup JSON should preserve zero live preview right bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsBottom\": 0",
                    "#2017: label column setup JSON should preserve zero live preview bottom bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2017: label column setup JSON should preserve zero live preview widths");
    expect_contains(label_process.stdout_text, "\"previewBoundsHeight\": 0",
                    "#2017: label column setup JSON should preserve zero live preview heights");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2017: label column setup JSON should not fabricate deleted preview availability");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview left bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview top bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview right bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview bottom bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview widths");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview heights");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_bottom_margin_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_bottom_margin_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& updated_margin,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_bottom_margin_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "BOTMARGIN",
                "--property-value", updated_margin,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable bottom margin field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable bottom margin field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1830: report/label stable bottom-margin field update should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value == updated_margin,
               "#1830: report/label stable bottom-margin field update should persist the BOTMARGIN field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1830: report/label stable bottom-margin field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1830: label stable bottom-margin field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2029: stable-selected report/label bottom-margin update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1830: report/label stable bottom-margin field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1830: report/label stable bottom-margin field update should preserve memo-derived top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": " + updated_margin,
                        "#1830: report/label stable bottom-margin field update should refresh bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#1830: report/label stable bottom-margin field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1830: report/label stable bottom-margin field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 4",
                        "#1830: report/label stable bottom-margin field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1830: report/label stable bottom-margin field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1830: report/label stable bottom-margin field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            "#1830: report/label stable bottom-margin field update should refresh selected direct-field provenance");
    };

    run_bottom_margin_update(temp_root / "bottom_margin_stable.frx",
                             "bottom_margin_stable.frx",
                             "34",
                             "report");
    run_bottom_margin_update(temp_root / "bottom_margin_stable.lbx",
                             "bottom_margin_stable.lbx",
                             "36",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_bottom_margin_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_bottom_margin_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_bottom_margin_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "BOTMARGIN",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable bottom margin field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable bottom margin field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1830: report/label stable bottom-margin field clear should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value.empty(),
               "#1830: report/label stable bottom-margin field clear should blank the BOTMARGIN field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1830: report/label stable bottom-margin field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1830: label stable bottom-margin field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2029: stable-selected report/label bottom-margin clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1830: report/label stable bottom-margin field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1830: report/label stable bottom-margin field clear should preserve memo-derived top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1830: report/label stable bottom-margin field clear should clear bottom-margin availability");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 0",
                        "#1830: report/label stable bottom-margin field clear should clear bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#1830: report/label stable bottom-margin field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1830: report/label stable bottom-margin field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 3",
                        "#1830: report/label stable bottom-margin field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1830: report/label stable bottom-margin field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1830: report/label stable bottom-margin field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1830: report/label stable bottom-margin field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1830: report/label stable bottom-margin field clear should remove direct BOTMARGIN provenance");
    };

    run_bottom_margin_clear(temp_root / "bottom_margin_clear_stable.frx",
                            "bottom_margin_clear_stable.frx",
                            "report");
    run_bottom_margin_clear(temp_root / "bottom_margin_clear_stable.lbx",
                            "bottom_margin_clear_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_bottom_margin_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_bottom_margin_update = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& updated_margin,
                                                      const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_bottom_margin_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "BOTMARGIN",
                "--property-value", updated_margin,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted bottom margin field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted bottom margin field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1830: report/label stable deleted bottom-margin field update should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value == updated_margin,
               "#1830: report/label stable deleted bottom-margin field update should persist the BOTMARGIN field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1830: report/label stable deleted bottom-margin field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1830: label stable deleted bottom-margin field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2029: stable-selected deleted report/label bottom-margin update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1830: report/label stable deleted bottom-margin field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1830: report/label stable deleted bottom-margin field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1830: report/label stable deleted bottom-margin field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1830: report/label stable deleted bottom-margin field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1830: report/label stable deleted bottom-margin field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            "#1830: report/label stable deleted bottom-margin field update should refresh selected deleted settings");
    };

    run_deleted_bottom_margin_update(temp_root / "deleted_bottom_margin_stable.frx",
                                     "deleted_bottom_margin_stable.frx",
                                     "34",
                                     "report");
    run_deleted_bottom_margin_update(temp_root / "deleted_bottom_margin_stable.lbx",
                                     "deleted_bottom_margin_stable.lbx",
                                     "36",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_bottom_margin_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_bottom_margin_clear = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_bottom_margin_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "BOTMARGIN",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted bottom margin field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted bottom margin field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1830: report/label stable deleted bottom-margin field clear should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value.empty(),
               "#1830: report/label stable deleted bottom-margin field clear should blank the BOTMARGIN field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1830: report/label stable deleted bottom-margin field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1830: label stable deleted bottom-margin field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2029: stable-selected deleted report/label bottom-margin clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1830: report/label stable deleted bottom-margin field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1830: report/label stable deleted bottom-margin field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1830: report/label stable deleted bottom-margin field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1830: report/label stable deleted bottom-margin field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1830: report/label stable deleted bottom-margin field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1830: report/label stable deleted bottom-margin field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1830: report/label stable deleted bottom-margin field clear should remove direct BOTMARGIN provenance");
    };

    run_deleted_bottom_margin_clear(temp_root / "deleted_bottom_margin_clear_stable.frx",
                                    "deleted_bottom_margin_clear_stable.frx",
                                    "report");
    run_deleted_bottom_margin_clear(temp_root / "deleted_bottom_margin_clear_stable.lbx",
                                    "deleted_bottom_margin_clear_stable.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_grid_vertical_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_grid_vertical_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& updated_grid,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_grid_vertical_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "GRIDV",
                "--property-value", updated_grid,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable vertical-grid field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable vertical-grid field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1831: report/label stable vertical-grid field update should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value == updated_grid,
               "#1831: report/label stable vertical-grid field update should persist the GRIDV field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable vertical-grid field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable vertical-grid field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2025: stable-selected report/label vertical-grid update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1831: report/label stable vertical-grid field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1831: report/label stable vertical-grid field update should preserve memo-derived top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#1831: report/label stable vertical-grid field update should preserve memo-derived bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": " + updated_grid,
                        "#1831: report/label stable vertical-grid field update should refresh vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1831: report/label stable vertical-grid field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 4",
                        "#1831: report/label stable vertical-grid field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable vertical-grid field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable vertical-grid field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_grid + "\""
            },
            "#1831: report/label stable vertical-grid field update should refresh selected direct-field provenance");
    };

    run_grid_vertical_update(temp_root / "grid_vertical_stable.frx",
                             "grid_vertical_stable.frx",
                             "12",
                             "report");
    run_grid_vertical_update(temp_root / "grid_vertical_stable.lbx",
                             "grid_vertical_stable.lbx",
                             "14",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_grid_vertical_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_grid_vertical_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_grid_vertical_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "GRIDV",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable vertical-grid field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable vertical-grid field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1831: report/label stable vertical-grid field clear should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value.empty(),
               "#1831: report/label stable vertical-grid field clear should blank the GRIDV field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable vertical-grid field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable vertical-grid field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2025: stable-selected report/label vertical-grid clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1831: report/label stable vertical-grid field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1831: report/label stable vertical-grid field clear should preserve memo-derived top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#1831: report/label stable vertical-grid field clear should preserve memo-derived bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1831: report/label stable vertical-grid field clear should clear vertical-grid availability");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 0",
                        "#1831: report/label stable vertical-grid field clear should clear vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1831: report/label stable vertical-grid field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 3",
                        "#1831: report/label stable vertical-grid field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable vertical-grid field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable vertical-grid field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1831: report/label stable vertical-grid field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1831: report/label stable vertical-grid field clear should remove direct GRIDV provenance");
    };

    run_grid_vertical_clear(temp_root / "grid_vertical_clear_stable.frx",
                            "grid_vertical_clear_stable.frx",
                            "report");
    run_grid_vertical_clear(temp_root / "grid_vertical_clear_stable.lbx",
                            "grid_vertical_clear_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_grid_vertical_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_grid_vertical_update = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& updated_grid,
                                                      const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_grid_vertical_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "GRIDV",
                "--property-value", updated_grid,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted vertical-grid field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted vertical-grid field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1831: report/label stable deleted vertical-grid field update should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value == updated_grid,
               "#1831: report/label stable deleted vertical-grid field update should persist the GRIDV field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable deleted vertical-grid field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable deleted vertical-grid field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2025: stable-selected deleted report/label vertical-grid update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1831: report/label stable deleted vertical-grid field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1831: report/label stable deleted vertical-grid field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1831: report/label stable deleted vertical-grid field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable deleted vertical-grid field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable deleted vertical-grid field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_grid + "\""
            },
            "#1831: report/label stable deleted vertical-grid field update should refresh selected deleted settings");
    };

    run_deleted_grid_vertical_update(temp_root / "deleted_grid_vertical_stable.frx",
                                     "deleted_grid_vertical_stable.frx",
                                     "12",
                                     "report");
    run_deleted_grid_vertical_update(temp_root / "deleted_grid_vertical_stable.lbx",
                                     "deleted_grid_vertical_stable.lbx",
                                     "14",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_grid_vertical_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_grid_vertical_clear = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_grid_vertical_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "GRIDV",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted vertical-grid field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted vertical-grid field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1831: report/label stable deleted vertical-grid field clear should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value.empty(),
               "#1831: report/label stable deleted vertical-grid field clear should blank the GRIDV field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable deleted vertical-grid field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable deleted vertical-grid field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2025: stable-selected deleted report/label vertical-grid clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1831: report/label stable deleted vertical-grid field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1831: report/label stable deleted vertical-grid field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1831: report/label stable deleted vertical-grid field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable deleted vertical-grid field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable deleted vertical-grid field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1831: report/label stable deleted vertical-grid field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1831: report/label stable deleted vertical-grid field clear should remove direct GRIDV provenance");
    };

    run_deleted_grid_vertical_clear(temp_root / "deleted_grid_vertical_clear_stable.frx",
                                    "deleted_grid_vertical_clear_stable.frx",
                                    "report");
    run_deleted_grid_vertical_clear(temp_root / "deleted_grid_vertical_clear_stable.lbx",
                                    "deleted_grid_vertical_clear_stable.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_grid_horizontal_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_grid_horizontal_update = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& updated_grid,
                                                const std::string& label) {
        write_synthetic_report_table_for_stable_grid_horizontal_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "GRIDH",
                "--property-value", updated_grid,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable horizontal-grid field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable horizontal-grid field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1832: report/label stable horizontal-grid field update should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "GRIDH"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value == updated_grid,
               "#1832: report/label stable horizontal-grid field update should persist the GRIDH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1832: report/label stable horizontal-grid field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1832: label stable horizontal-grid field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2027: stable-selected report/label horizontal-grid update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1832: report/label stable horizontal-grid field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1832: report/label stable horizontal-grid field update should preserve memo-derived top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#1832: report/label stable horizontal-grid field update should preserve memo-derived bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#1832: report/label stable horizontal-grid field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": " + updated_grid,
                        "#1832: report/label stable horizontal-grid field update should refresh horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 4",
                        "#1832: report/label stable horizontal-grid field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1832: report/label stable horizontal-grid field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1832: report/label stable horizontal-grid field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_grid + "\""
            },
            "#1832: report/label stable horizontal-grid field update should refresh selected direct-field provenance");
    };

    run_grid_horizontal_update(temp_root / "grid_horizontal_stable.frx",
                               "grid_horizontal_stable.frx",
                               "16",
                               "report");
    run_grid_horizontal_update(temp_root / "grid_horizontal_stable.lbx",
                               "grid_horizontal_stable.lbx",
                               "18",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_grid_horizontal_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_grid_horizontal_clear = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_stable_grid_horizontal_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "GRIDH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable horizontal-grid field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable horizontal-grid field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1832: report/label stable horizontal-grid field clear should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "GRIDH"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value.empty(),
               "#1832: report/label stable horizontal-grid field clear should blank the GRIDH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1832: report/label stable horizontal-grid field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1832: label stable horizontal-grid field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2027: stable-selected report/label horizontal-grid clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1832: report/label stable horizontal-grid field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1832: report/label stable horizontal-grid field clear should preserve memo-derived top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#1832: report/label stable horizontal-grid field clear should preserve memo-derived bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#1832: report/label stable horizontal-grid field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1832: report/label stable horizontal-grid field clear should clear horizontal-grid availability");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1832: report/label stable horizontal-grid field clear should clear horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 3",
                        "#1832: report/label stable horizontal-grid field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1832: report/label stable horizontal-grid field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1832: report/label stable horizontal-grid field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1832: report/label stable horizontal-grid field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1832: report/label stable horizontal-grid field clear should remove direct GRIDH provenance");
    };

    run_grid_horizontal_clear(temp_root / "grid_horizontal_clear_stable.frx",
                              "grid_horizontal_clear_stable.frx",
                              "report");
    run_grid_horizontal_clear(temp_root / "grid_horizontal_clear_stable.lbx",
                              "grid_horizontal_clear_stable.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_grid_horizontal_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_grid_horizontal_update = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& updated_grid,
                                                        const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_grid_horizontal_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "GRIDH",
                "--property-value", updated_grid,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted horizontal-grid field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted horizontal-grid field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1832: report/label stable deleted horizontal-grid field update should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "GRIDH"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value == updated_grid,
               "#1832: report/label stable deleted horizontal-grid field update should persist the GRIDH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1832: report/label stable deleted horizontal-grid field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1832: label stable deleted horizontal-grid field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2027: stable-selected deleted report/label horizontal-grid update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1832: report/label stable deleted horizontal-grid field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1832: report/label stable deleted horizontal-grid field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1832: report/label stable deleted horizontal-grid field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1832: report/label stable deleted horizontal-grid field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1832: report/label stable deleted horizontal-grid field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_grid + "\""
            },
            "#1832: report/label stable deleted horizontal-grid field update should refresh selected deleted settings");
    };

    run_deleted_grid_horizontal_update(temp_root / "deleted_grid_horizontal_stable.frx",
                                       "deleted_grid_horizontal_stable.frx",
                                       "16",
                                       "report");
    run_deleted_grid_horizontal_update(temp_root / "deleted_grid_horizontal_stable.lbx",
                                       "deleted_grid_horizontal_stable.lbx",
                                       "18",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_grid_horizontal_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_grid_horizontal_clear = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_grid_horizontal_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "GRIDH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted horizontal-grid field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted horizontal-grid field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1832: report/label stable deleted horizontal-grid field clear should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "GRIDH"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value.empty(),
               "#1832: report/label stable deleted horizontal-grid field clear should blank the GRIDH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1832: report/label stable deleted horizontal-grid field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1832: label stable deleted horizontal-grid field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2027: stable-selected deleted report/label horizontal-grid clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1832: report/label stable deleted horizontal-grid field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1832: report/label stable deleted horizontal-grid field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1832: report/label stable deleted horizontal-grid field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1832: report/label stable deleted horizontal-grid field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1832: report/label stable deleted horizontal-grid field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1832: report/label stable deleted horizontal-grid field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1832: report/label stable deleted horizontal-grid field clear should remove direct GRIDH provenance");
    };

    run_deleted_grid_horizontal_clear(temp_root / "deleted_grid_horizontal_clear_stable.frx",
                                      "deleted_grid_horizontal_clear_stable.frx",
                                      "report");
    run_deleted_grid_horizontal_clear(temp_root / "deleted_grid_horizontal_clear_stable.lbx",
                                      "deleted_grid_horizontal_clear_stable.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_orientation_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_orientation_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& updated_orientation,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_orientation_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "ORIENTATION",
                "--property-value", updated_orientation,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable orientation field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable orientation field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1833: report/label stable orientation field update should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists &&
                   orientation_property.value == updated_orientation,
               "#1833: report/label stable orientation field update should persist the ORIENTATION field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable orientation field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable orientation field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2023: stable-selected report/label orientation update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1833: report/label stable orientation field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"orientationCode\": " + updated_orientation,
                        "#1833: report/label stable orientation field update should refresh orientation codes");
        expect_contains(update_process.stdout_text, "\"paperSizeCode\": 1",
                        "#1833: report/label stable orientation field update should preserve paper-size codes");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1833: report/label stable orientation field update should preserve top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#1833: report/label stable orientation field update should preserve bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#1833: report/label stable orientation field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1833: report/label stable orientation field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 6",
                        "#1833: report/label stable orientation field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable orientation field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable orientation field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_orientation + "\""
            },
            "#1833: report/label stable orientation field update should refresh selected direct-field provenance");
    };

    run_orientation_update(temp_root / "orientation_stable.frx",
                           "orientation_stable.frx",
                           "1",
                           "report");
    run_orientation_update(temp_root / "orientation_stable.lbx",
                           "orientation_stable.lbx",
                           "2",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_orientation_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_orientation_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_orientation_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "ORIENTATION",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable orientation field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable orientation field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1833: report/label stable orientation field clear should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists && orientation_property.value.empty(),
               "#1833: report/label stable orientation field clear should blank the ORIENTATION field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable orientation field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable orientation field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2023: stable-selected report/label orientation clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1833: report/label stable orientation field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"orientationAvailable\": false",
                        "#1833: report/label stable orientation field clear should clear orientation availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        "#1833: report/label stable orientation field clear should clear orientation codes");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 1",
                        "#1833: report/label stable orientation field clear should preserve paper-size codes");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1833: report/label stable orientation field clear should preserve top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#1833: report/label stable orientation field clear should preserve bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#1833: report/label stable orientation field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1833: report/label stable orientation field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 5",
                        "#1833: report/label stable orientation field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable orientation field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable orientation field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1833: report/label stable orientation field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1833: report/label stable orientation field clear should remove direct ORIENTATION provenance");
    };

    run_orientation_clear(temp_root / "orientation_clear_stable.frx",
                          "orientation_clear_stable.frx",
                          "report");
    run_orientation_clear(temp_root / "orientation_clear_stable.lbx",
                          "orientation_clear_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_orientation_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_orientation_update = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& updated_orientation,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_orientation_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "ORIENTATION",
                "--property-value", updated_orientation,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted orientation field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted orientation field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1833: report/label stable deleted orientation field update should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists &&
                   orientation_property.value == updated_orientation,
               "#1833: report/label stable deleted orientation field update should persist the ORIENTATION field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable deleted orientation field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable deleted orientation field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2023: stable-selected deleted report/label orientation update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1833: report/label stable deleted orientation field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1833: report/label stable deleted orientation field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1833: report/label stable deleted orientation field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable deleted orientation field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable deleted orientation field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_orientation + "\""
            },
            "#1833: report/label stable deleted orientation field update should refresh selected deleted settings");
    };

    run_deleted_orientation_update(temp_root / "deleted_orientation_stable.frx",
                                   "deleted_orientation_stable.frx",
                                   "1",
                                   "report");
    run_deleted_orientation_update(temp_root / "deleted_orientation_stable.lbx",
                                   "deleted_orientation_stable.lbx",
                                   "2",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_orientation_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_orientation_clear = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_orientation_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "ORIENTATION",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted orientation field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted orientation field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1833: report/label stable deleted orientation field clear should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists && orientation_property.value.empty(),
               "#1833: report/label stable deleted orientation field clear should blank the ORIENTATION field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable deleted orientation field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable deleted orientation field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2023: stable-selected deleted report/label orientation clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1833: report/label stable deleted orientation field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1833: report/label stable deleted orientation field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#1833: report/label stable deleted orientation field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable deleted orientation field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable deleted orientation field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1833: report/label stable deleted orientation field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1833: report/label stable deleted orientation field clear should remove direct ORIENTATION provenance");
    };

    run_deleted_orientation_clear(temp_root / "deleted_orientation_clear_stable.frx",
                                  "deleted_orientation_clear_stable.frx",
                                  "report");
    run_deleted_orientation_clear(temp_root / "deleted_orientation_clear_stable.lbx",
                                  "deleted_orientation_clear_stable.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_paper_size_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_paper_size_update = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& updated_paper_size,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_paper_size_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "PAPERSIZE",
                "--property-value", updated_paper_size,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable paper-size field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable paper-size field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1834: report/label stable paper-size field update should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists &&
                   paper_size_property.value == updated_paper_size,
               "#1834: report/label stable paper-size field update should persist the PAPERSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable paper-size field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable paper-size field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2021: stable-selected report/label paper-size update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1834: report/label stable paper-size field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"orientationCode\": 0",
                        "#1834: report/label stable paper-size field update should preserve orientation codes");
        expect_contains(update_process.stdout_text, "\"paperSizeCode\": " + updated_paper_size,
                        "#1834: report/label stable paper-size field update should refresh paper-size codes");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1834: report/label stable paper-size field update should preserve top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#1834: report/label stable paper-size field update should preserve bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#1834: report/label stable paper-size field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1834: report/label stable paper-size field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 6",
                        "#1834: report/label stable paper-size field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable paper-size field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable paper-size field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_paper_size + "\""
            },
            "#1834: report/label stable paper-size field update should refresh selected direct-field provenance");
    };

    run_paper_size_update(temp_root / "paper_size_stable.frx",
                          "paper_size_stable.frx",
                          "9",
                          "report");
    run_paper_size_update(temp_root / "paper_size_stable.lbx",
                          "paper_size_stable.lbx",
                          "5",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_paper_size_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_paper_size_clear = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_stable_paper_size_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "PAPERSIZE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable paper-size field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable paper-size field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1834: report/label stable paper-size field clear should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists && paper_size_property.value.empty(),
               "#1834: report/label stable paper-size field clear should blank the PAPERSIZE field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable paper-size field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable paper-size field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2021: stable-selected report/label paper-size clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1834: report/label stable paper-size field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        "#1834: report/label stable paper-size field clear should preserve orientation codes");
        expect_contains(clear_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1834: report/label stable paper-size field clear should clear paper-size availability");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 0",
                        "#1834: report/label stable paper-size field clear should clear paper-size codes");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1834: report/label stable paper-size field clear should preserve top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#1834: report/label stable paper-size field clear should preserve bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#1834: report/label stable paper-size field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1834: report/label stable paper-size field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 5",
                        "#1834: report/label stable paper-size field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable paper-size field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable paper-size field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1834: report/label stable paper-size field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1834: report/label stable paper-size field clear should remove direct PAPERSIZE provenance");
    };

    run_paper_size_clear(temp_root / "paper_size_clear_stable.frx",
                         "paper_size_clear_stable.frx",
                         "report");
    run_paper_size_clear(temp_root / "paper_size_clear_stable.lbx",
                         "paper_size_clear_stable.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_paper_size_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_paper_size_update = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& updated_paper_size,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_paper_size_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "PAPERSIZE",
                "--property-value", updated_paper_size,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted paper-size field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted paper-size field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1834: report/label stable deleted paper-size field update should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists &&
                   paper_size_property.value == updated_paper_size,
               "#1834: report/label stable deleted paper-size field update should persist the PAPERSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable deleted paper-size field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable deleted paper-size field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2021: stable-selected deleted report/label paper-size update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1834: report/label stable deleted paper-size field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1834: report/label stable deleted paper-size field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1834: report/label stable deleted paper-size field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable deleted paper-size field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable deleted paper-size field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_paper_size + "\""
            },
            "#1834: report/label stable deleted paper-size field update should refresh selected deleted settings");
    };

    run_deleted_paper_size_update(temp_root / "deleted_paper_size_stable.frx",
                                  "deleted_paper_size_stable.frx",
                                  "9",
                                  "report");
    run_deleted_paper_size_update(temp_root / "deleted_paper_size_stable.lbx",
                                  "deleted_paper_size_stable.lbx",
                                  "5",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_paper_size_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_paper_size_clear = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_paper_size_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "PAPERSIZE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted paper-size field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted paper-size field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1834: report/label stable deleted paper-size field clear should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists && paper_size_property.value.empty(),
               "#1834: report/label stable deleted paper-size field clear should blank the PAPERSIZE field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable deleted paper-size field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable deleted paper-size field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2021: stable-selected deleted report/label paper-size clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1834: report/label stable deleted paper-size field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1834: report/label stable deleted paper-size field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#1834: report/label stable deleted paper-size field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable deleted paper-size field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable deleted paper-size field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1834: report/label stable deleted paper-size field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1834: report/label stable deleted paper-size field clear should remove direct PAPERSIZE provenance");
    };

    run_deleted_paper_size_clear(temp_root / "deleted_paper_size_clear_stable.frx",
                                 "deleted_paper_size_clear_stable.frx",
                                 "report");
    run_deleted_paper_size_clear(temp_root / "deleted_paper_size_clear_stable.lbx",
                                 "deleted_paper_size_clear_stable.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_count_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_count_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& updated_count,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_column_setup_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLS",
                "--property-value", updated_count,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-count field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-count field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1835: report/label stable column-count field update should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value == updated_count,
               "#1835: report/label stable column-count field update should persist the COLS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable column-count field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable column-count field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2031: stable-selected report/label column-count update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable column-count field update should not fabricate page setup availability");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1835: report/label stable column-count field update should preserve column setup availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": " + updated_count,
                        "#1835: report/label stable column-count field update should refresh column counts");
        expect_contains(update_process.stdout_text, "\"columnWidth\": 3600",
                        "#1835: report/label stable column-count field update should preserve memo-derived column widths");
        expect_contains(update_process.stdout_text, "\"columnSpacing\": 120",
                        "#1835: report/label stable column-count field update should preserve memo-derived column spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 3",
                        "#1835: report/label stable column-count field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable column-count field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable column-count field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_count + "\""
            },
            "#1835: report/label stable column-count field update should refresh selected direct-field provenance");
    };

    run_column_count_update(temp_root / "column_count_stable.frx",
                            "column_count_stable.frx",
                            "4",
                            "report");
    run_column_count_update(temp_root / "column_count_stable.lbx",
                            "column_count_stable.lbx",
                            "5",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_count_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_count_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_column_setup_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-count field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-count field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1835: report/label stable column-count field clear should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value.empty(),
               "#1835: report/label stable column-count field clear should blank the COLS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable column-count field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable column-count field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2031: stable-selected report/label column-count clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable column-count field clear should not fabricate page setup availability");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1835: report/label stable column-count field clear should preserve column setup availability");
        expect_contains(clear_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1835: report/label stable column-count field clear should clear column-count availability");
        expect_contains(clear_process.stdout_text, "\"columnCount\": 0",
                        "#1835: report/label stable column-count field clear should clear column counts");
        expect_contains(clear_process.stdout_text, "\"columnWidth\": 3600",
                        "#1835: report/label stable column-count field clear should preserve memo-derived column widths");
        expect_contains(clear_process.stdout_text, "\"columnSpacing\": 120",
                        "#1835: report/label stable column-count field clear should preserve memo-derived column spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 2",
                        "#1835: report/label stable column-count field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable column-count field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable column-count field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1835: report/label stable column-count field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1835: report/label stable column-count field clear should remove direct COLS provenance");
    };

    run_column_count_clear(temp_root / "column_count_clear_stable.frx",
                           "column_count_clear_stable.frx",
                           "report");
    run_column_count_clear(temp_root / "column_count_clear_stable.lbx",
                           "column_count_clear_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_count_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_count_update = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& updated_count,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_setup_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLS",
                "--property-value", updated_count,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-count field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-count field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1835: report/label stable deleted column-count field update should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value == updated_count,
               "#1835: report/label stable deleted column-count field update should persist the COLS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable deleted column-count field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable deleted column-count field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2031: stable-selected deleted report/label column-count update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable deleted column-count field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1835: report/label stable deleted column-count field update should not fabricate live column setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1835: report/label stable deleted column-count field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1835: report/label stable deleted column-count field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable deleted column-count field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable deleted column-count field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_count + "\""
            },
            "#1835: report/label stable deleted column-count field update should refresh deleted setting provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_count + "\""
            },
            "#1835: report/label stable deleted column-count field update should refresh selected deleted settings");
    };

    run_deleted_column_count_update(temp_root / "deleted_column_count_stable.frx",
                                    "deleted_column_count_stable.frx",
                                    "4",
                                    "report");
    run_deleted_column_count_update(temp_root / "deleted_column_count_stable.lbx",
                                    "deleted_column_count_stable.lbx",
                                    "5",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_count_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_count_clear = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_setup_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-count field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-count field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1835: report/label stable deleted column-count field clear should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value.empty(),
               "#1835: report/label stable deleted column-count field clear should blank the COLS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable deleted column-count field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable deleted column-count field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2031: stable-selected deleted report/label column-count clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable deleted column-count field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1835: report/label stable deleted column-count field clear should not fabricate live column setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1835: report/label stable deleted column-count field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 2",
                        "#1835: report/label stable deleted column-count field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable deleted column-count field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable deleted column-count field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1835: report/label stable deleted column-count field clear should preserve remaining deleted setting provenance");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1835: report/label stable deleted column-count field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1835: report/label stable deleted column-count field clear should remove direct COLS provenance");
    };

    run_deleted_column_count_clear(temp_root / "deleted_column_count_clear_stable.frx",
                                   "deleted_column_count_clear_stable.frx",
                                   "report");
    run_deleted_column_count_clear(temp_root / "deleted_column_count_clear_stable.lbx",
                                   "deleted_column_count_clear_stable.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_report_settings_by_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_settings_delete_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--delete-object",
            "--record", "0",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host report settings delete stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host report settings delete stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0,
           "#1475: report settings delete should exit successfully");
    expect(dbf_record_deleted(report_path, 0U),
           "#1475: report settings delete should mark the FRX settings record deleted");
    expect_full_report_layout_preview_bounds(
        delete_process.stdout_text,
        "#2040: record-selected report settings delete JSON");
    expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                    "#1475: deleted report settings JSON should remove settings from live counts");
    expect_contains(delete_process.stdout_text, "\"pageSetupAvailable\": false",
                    "#1517: deleted report settings JSON should clear live page setup summaries");
    expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                    "#1475: deleted report settings JSON should expose deleted setting counts");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedSettings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERSIZE\"",
            "\"recordIndex\": 0",
            "\"name\": \"BOTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDV\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDH\"",
            "\"recordIndex\": 0",
            "\"name\": \"TOPMARGIN\"",
            "\"recordIndex\": 0"
        },
        "#1475: report layout JSON should move root settings into deleted-setting metadata");
    expect_contains(delete_process.stdout_text, "\"sectionCount\": 2",
                    "#1475: deleting report settings should preserve live section metadata");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1475: deleting report settings should preserve deleted object metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_report_settings(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_report_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto settings_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "0", "--json"},
        temp_root);

    if (settings_process.exit_code != 0) {
        std::cerr << "studio host selected report settings stdout:\n" << settings_process.stdout_text << "\n";
        std::cerr << "studio host selected report settings stderr:\n" << settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(settings_process.exit_code == 0,
           "#1456: selected report settings JSON smoke should exit successfully");
    expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    "#1456: report root selections should advertise selected-settings availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1457: report settings selections should advertise report-selection availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    "#1457: report settings selections should expose settings selection kind");
    expect_contains(settings_process.stdout_text, "\"selectedReportSettings\": [",
                    "#1456: report root selections should expose selected-settings JSON");
    expect_contains(settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1956: selected report settings JSON should expose live preview availability");
    expect_contains(settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1956: selected report settings JSON should preserve live preview left bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1956: selected report settings JSON should preserve live preview top bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1956: selected report settings JSON should preserve live preview right bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1956: selected report settings JSON should preserve live preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1956: selected report settings JSON should preserve live preview widths");
    expect_contains(settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1956: selected report settings JSON should preserve live preview heights");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1956: selected report settings JSON should expose deleted preview availability");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1956: selected report settings JSON should preserve deleted preview left bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1956: selected report settings JSON should preserve deleted preview top bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1956: selected report settings JSON should preserve deleted preview right bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1956: selected report settings JSON should preserve deleted preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1956: selected report settings JSON should preserve deleted preview widths");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1956: selected report settings JSON should preserve deleted preview heights");
    expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1514: selected report settings should not advertise selected-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                    "#1514: selected report settings should serialize null selected sections");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1514: selected report settings should not advertise selected-object availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                    "#1514: selected report settings should serialize null selected objects");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1514: selected report settings should not advertise containing-object-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1514: selected report settings should serialize null containing-object sections");
    expect_contains(settings_process.stdout_text, "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
                    "#1456: selected report settings should expose memo-line setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"1\"",
                    "#1456: selected report settings should expose later memo-line setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"10\"",
                    "#1456: selected report settings should expose direct setting provenance");

    const fs::path deleted_settings_path = temp_root / "deleted_settings.frx";
    write_synthetic_report_table_for_deleted_settings_json(deleted_settings_path);
    const auto deleted_settings_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_settings_path.string(), "--record", "0", "--json"},
        temp_root);

    if (deleted_settings_process.exit_code != 0) {
        std::cerr << "studio host selected deleted report settings stdout:\n"
                  << deleted_settings_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted report settings stderr:\n"
                  << deleted_settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_settings_process.exit_code == 0,
           "#1477: selected deleted report settings JSON should exit successfully");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    "#1477: deleted report settings selections should advertise selected-settings availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1477: deleted report settings selections should advertise report-selection availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    "#1477: deleted report settings selections should expose settings selection kind");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1957: selected deleted report settings JSON should expose live preview availability");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1957: selected deleted report settings JSON should preserve live preview left bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1957: selected deleted report settings JSON should preserve live preview top bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1957: selected deleted report settings JSON should preserve live preview right bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1957: selected deleted report settings JSON should preserve live preview bottom bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1957: selected deleted report settings JSON should preserve live preview widths");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1957: selected deleted report settings JSON should preserve live preview heights");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1957: selected deleted report settings JSON should expose deleted preview availability");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1957: selected deleted report settings JSON should preserve deleted preview left bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1957: selected deleted report settings JSON should preserve deleted preview top bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1957: selected deleted report settings JSON should preserve deleted preview right bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1957: selected deleted report settings JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1957: selected deleted report settings JSON should preserve deleted preview widths");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1957: selected deleted report settings JSON should preserve deleted preview heights");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1515: selected deleted report settings should not advertise selected-section availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSection\": null",
                    "#1515: selected deleted report settings should serialize null selected sections");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1515: selected deleted report settings should not advertise selected-object availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObject\": null",
                    "#1515: selected deleted report settings should serialize null selected objects");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1515: selected deleted report settings should not advertise containing-object-section availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1515: selected deleted report settings should serialize null containing-object sections");
    expect_contains(deleted_settings_process.stdout_text, "\"settingCount\": 0",
                    "#1477: deleted selected report settings JSON should not expose live settings");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedSettingCount\": 6",
                    "#1477: deleted selected report settings JSON should expose deleted setting counts");
    expect_contains_in_order(
        deleted_settings_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERSIZE\"",
            "\"recordIndex\": 0",
            "\"name\": \"BOTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDV\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDH\"",
            "\"recordIndex\": 0",
            "\"name\": \"TOPMARGIN\"",
            "\"recordIndex\": 0"
        },
        "#1477: deleted report settings selections should expose selected deleted-setting provenance");

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
           "#1456: selected report object JSON smoke should exit successfully");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1456: non-settings report selections should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1456: non-settings report selections should serialize null selected settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_report_settings_without_root_objcode_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_root_objcode_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_missing_root_objcode_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_missing_root_objcode_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing root OBJCODE summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing root OBJCODE summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1730: missing root OBJCODE schema should keep report/label settings inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1730: missing root OBJCODE layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1730: missing root OBJCODE label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live page setup availability");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live orientation availability");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1730: missing root OBJCODE should preserve live orientation codes");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live paper-size availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1730: missing root OBJCODE should preserve live paper-size codes");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live top-margin availability");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1730: missing root OBJCODE should preserve live top margins");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1730: missing root OBJCODE should preserve EXPR-derived vertical grid availability");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1730: missing root OBJCODE should preserve EXPR-derived vertical grid values");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": true",
                        "#1730: missing root OBJCODE should preserve EXPR-derived horizontal grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1730: missing root OBJCODE should preserve EXPR-derived horizontal grid values");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 5",
                        "#1730: missing root OBJCODE layouts should preserve live setting counts");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#1730: missing root OBJCODE layouts should preserve deleted setting counts");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 1, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"1\"",
                        "#1730: live EXPR-derived setting provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 1, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"0\"",
                        "#1730: live second-line EXPR setting provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"1\"",
                        "#1730: live direct orientation provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLS\", \"recordIndex\": 1, \"fieldIndex\": 1, \"sourceLineIndex\": 0, \"memoBlockNumber\": 2, \"value\": \"3\"",
                        "#1730: deleted EXPR-derived setting provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"TOPMARGIN\", \"recordIndex\": 1, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"240\"",
                        "#1730: deleted direct top-margin provenance should remain available without OBJCODE");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2317: missing root OBJCODE settings summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1730: missing root OBJCODE live settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1730: missing root OBJCODE live settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1730: missing root OBJCODE live settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2317: selected missing root OBJCODE live settings JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 0",
                "\"value\": \"1\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 1",
                "\"value\": \"0\"",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"120\""
            },
            "#1730: missing root OBJCODE live selection should expose selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1730: missing root OBJCODE deleted settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1730: missing root OBJCODE deleted settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1730: missing root OBJCODE deleted settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2317: selected missing root OBJCODE deleted settings JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 0",
                "\"value\": \"3\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 1",
                "\"value\": \"5000\"",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"0\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"1\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"240\""
            },
            "#1730: missing root OBJCODE deleted selection should expose selected-settings metadata");
    };

    run_missing_root_objcode_layout(temp_root / "missing_root_objcode.frx",
                                    "missing_root_objcode.frx",
                                    "report");
    run_missing_root_objcode_layout(temp_root / "missing_root_objcode.lbx",
                                    "missing_root_objcode.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_invalid_direct_report_column_setup_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_direct_column_setup_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_invalid_direct_column_setup_layout = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_invalid_direct_column_setup_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid direct column setup summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid direct column setup summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1734: invalid direct column setup fields should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1734: invalid direct column setup layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1734: invalid direct column setup label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1734: invalid direct column setup fields should not fabricate column setup availability");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1734: invalid direct column count should not advertise column-count availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1734: invalid direct column width should not advertise column-width availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1734: invalid direct column spacing should not advertise column-spacing availability");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 0",
                        "#1734: invalid direct column count should keep the default column-count value inert");
        expect_contains(summary_process.stdout_text, "\"columnWidth\": 0",
                        "#1734: invalid direct column width should keep the default column-width value inert");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 0",
                        "#1734: invalid direct column spacing should keep the default column-spacing value inert");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#1734: invalid direct column settings should still be counted as live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1734: invalid direct column settings should still be counted as deleted raw settings");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"many\"",
                        "#1734: invalid direct column-count provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"wide?\"",
                        "#1734: invalid direct column-width provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"spaced?\"",
                        "#1734: invalid direct column-spacing provenance should remain inspectable");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1734: invalid direct live column settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1734: invalid direct live column settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1734: invalid direct live column settings should expose settings selection kind");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"many\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"wide?\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"spaced?\""
            },
            "#1734: invalid direct live column selection should expose raw selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1734: invalid direct deleted column settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1734: invalid direct deleted column settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1734: invalid direct deleted column settings should expose settings selection kind");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"deleted-many\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"deleted-wide?\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"deleted-spaced?\""
            },
            "#1734: invalid direct deleted column selection should expose raw selected-settings metadata");
    };

    run_invalid_direct_column_setup_layout(temp_root / "invalid_direct_column_setup.frx",
                                           "invalid_direct_column_setup.frx",
                                           "report");
    run_invalid_direct_column_setup_layout(temp_root / "invalid_direct_column_setup.lbx",
                                           "invalid_direct_column_setup.lbx",
                                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_invalid_direct_report_margin_grid_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_direct_margin_grid_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_invalid_direct_margin_grid_layout = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_invalid_direct_margin_grid_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid direct margin/grid summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid direct margin/grid summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1735: invalid direct margin/grid fields should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1735: invalid direct margin/grid layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1735: invalid direct margin/grid label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1735: invalid direct margin/grid fields should not fabricate page setup availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1735: invalid direct bottom margin should not advertise bottom-margin availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1735: invalid direct vertical grid should not advertise vertical-grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1735: invalid direct horizontal grid should not advertise horizontal-grid availability");
        expect_contains(summary_process.stdout_text, "\"bottomMargin\": 0",
                        "#1735: invalid direct bottom margin should keep the default bottom-margin value inert");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 0",
                        "#1735: invalid direct vertical grid should keep the default vertical-grid value inert");
        expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1735: invalid direct horizontal grid should keep the default horizontal-grid value inert");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#1735: invalid direct margin/grid settings should still be counted as live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1735: invalid direct margin/grid settings should still be counted as deleted raw settings");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"bottom?\"",
                        "#1735: invalid direct bottom-margin provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"vertical?\"",
                        "#1735: invalid direct vertical-grid provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"horizontal?\"",
                        "#1735: invalid direct horizontal-grid provenance should remain inspectable");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1735: invalid direct live margin/grid settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1735: invalid direct live margin/grid settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1735: invalid direct live margin/grid settings should expose settings selection kind");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"bottom?\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"vertical?\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"horizontal?\""
            },
            "#1735: invalid direct live margin/grid selection should expose raw selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1735: invalid direct deleted margin/grid settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1735: invalid direct deleted margin/grid settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1735: invalid direct deleted margin/grid settings should expose settings selection kind");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"deleted-bottom?\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"deleted-vertical?\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"deleted-horizontal?\""
            },
            "#1735: invalid direct deleted margin/grid selection should expose raw selected-settings metadata");
    };

    run_invalid_direct_margin_grid_layout(temp_root / "invalid_direct_margin_grid.frx",
                                          "invalid_direct_margin_grid.frx",
                                          "report");
    run_invalid_direct_margin_grid_layout(temp_root / "invalid_direct_margin_grid.lbx",
                                          "invalid_direct_margin_grid.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_suppresses_unresolved_report_direct_setting_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_direct_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unresolved_direct_setting_memo_layout = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_unresolved_direct_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved direct setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved direct setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1741: unresolved direct-setting memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1741: unresolved direct-setting memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1741: unresolved direct-setting memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1741: unresolved direct-setting memo placeholders should not become live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1741: unresolved direct-setting memo placeholders should not become deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1741: unresolved direct-setting memo placeholders should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1741: unresolved direct-setting memo placeholders should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1741: unresolved orientation placeholders should not advertise orientation availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1741: unresolved paper-size placeholders should not advertise paper-size availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1741: unresolved top-margin placeholders should not advertise top-margin availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1741: unresolved bottom-margin placeholders should not advertise bottom-margin availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1741: unresolved vertical-grid placeholders should not advertise vertical-grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1741: unresolved horizontal-grid placeholders should not advertise horizontal-grid availability");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1741: unresolved column-count placeholders should not advertise column-count availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1741: unresolved column-width placeholders should not advertise column-width availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1741: unresolved column-spacing placeholders should not advertise column-spacing availability");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1741: unresolved direct-setting memo placeholders should not leak into summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1741: unresolved live direct-setting memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1741: unresolved live direct-setting memo placeholders should not expose selected settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1741: unresolved live direct-setting memo selected settings should be null");
        expect_not_contains(live_settings_process.stdout_text, "<memo block",
                            "#1741: unresolved live direct-setting memo placeholders should not leak into selection JSON");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1741: unresolved deleted direct-setting memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1741: unresolved deleted direct-setting memo placeholders should not expose selected settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1741: unresolved deleted direct-setting memo selected settings should be null");
        expect_not_contains(deleted_settings_process.stdout_text, "<memo block",
                            "#1741: unresolved deleted direct-setting memo placeholders should not leak into selection JSON");
    };

    run_unresolved_direct_setting_memo_layout(temp_root / "unresolved_direct_setting_memo.frx",
                                              "unresolved_direct_setting_memo.frx",
                                              "report");
    run_unresolved_direct_setting_memo_layout(temp_root / "unresolved_direct_setting_memo.lbx",
                                              "unresolved_direct_setting_memo.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_mixed_report_direct_setting_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_mixed_direct_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_mixed_direct_setting_memo_layout = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_mixed_direct_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " mixed direct setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " mixed direct setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1742: mixed direct-setting memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1742: mixed direct-setting memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1742: mixed direct-setting memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 5",
                        "#1742: mixed direct-setting memo placeholders should preserve live valid settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1742: mixed direct-setting memo placeholders should preserve deleted valid settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep live page setup available");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid orientation available");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1742: mixed direct-setting memo layouts should keep valid orientation code");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder paper size");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid top margin available");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1742: mixed direct-setting memo layouts should keep valid top margin");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder bottom margin");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid vertical grid available");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1742: mixed direct-setting memo layouts should keep valid vertical grid");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder horizontal grid");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep live column setup available");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid column count available");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1742: mixed direct-setting memo layouts should keep valid column count");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder column width");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid column spacing available");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 42",
                        "#1742: mixed direct-setting memo layouts should keep valid column spacing");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1742: mixed direct-setting memo placeholders should not leak into summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1742: mixed live direct-setting memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1742: mixed live direct-setting memo selection should expose valid settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1742: mixed live direct-setting memo selection should expose settings kind");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"1\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"120\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 6",
                "\"value\": \"1\"",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 8",
                "\"value\": \"3\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 10",
                "\"value\": \"42\""
            },
            "#1742: mixed live direct-setting memo selection should expose only valid settings");
        expect_not_contains(live_settings_process.stdout_text, "<memo block",
                            "#1742: mixed live direct-setting memo placeholders should not leak into selection JSON");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1742: mixed deleted direct-setting memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1742: mixed deleted direct-setting memo selection should expose valid settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1742: mixed deleted direct-setting memo selection should expose settings kind");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"9\"",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 5",
                "\"value\": \"240\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 7",
                "\"value\": \"0\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 9",
                "\"value\": \"5000\""
            },
            "#1742: mixed deleted direct-setting memo selection should expose only valid settings");
        expect_not_contains(deleted_settings_process.stdout_text, "<memo block",
                            "#1742: mixed deleted direct-setting memo placeholders should not leak into selection JSON");
    };

    run_mixed_direct_setting_memo_layout(temp_root / "mixed_direct_setting_memo.frx",
                                         "mixed_direct_setting_memo.frx",
                                         "report");
    run_mixed_direct_setting_memo_layout(temp_root / "mixed_direct_setting_memo.lbx",
                                         "mixed_direct_setting_memo.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_skips_blank_report_direct_setting_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_blank_direct_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_blank_direct_setting_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_blank_direct_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank direct setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank direct setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1743: blank direct-setting fields should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1743: blank direct-setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1743: blank direct-setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1743: blank direct-setting fields should not become live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1743: blank direct-setting fields should not become deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1743: blank direct-setting fields should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1743: blank direct-setting fields should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1743: blank orientation fields should not advertise orientation availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1743: blank paper-size fields should not advertise paper-size availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1743: blank top-margin fields should not advertise top-margin availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1743: blank bottom-margin fields should not advertise bottom-margin availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1743: blank vertical-grid fields should not advertise vertical-grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1743: blank horizontal-grid fields should not advertise horizontal-grid availability");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1743: blank column-count fields should not advertise column-count availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1743: blank column-width fields should not advertise column-width availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1743: blank column-spacing fields should not advertise column-spacing availability");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1743: blank live direct-setting selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1743: blank live direct-setting fields should not expose selected settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1743: blank live direct-setting selected settings should be null");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1743: blank deleted direct-setting selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1743: blank deleted direct-setting fields should not expose selected settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1743: blank deleted direct-setting selected settings should be null");
    };

    run_blank_direct_setting_layout(temp_root / "blank_direct_setting.frx",
                                    "blank_direct_setting.frx",
                                    "report");
    run_blank_direct_setting_layout(temp_root / "blank_direct_setting.lbx",
                                    "blank_direct_setting.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
