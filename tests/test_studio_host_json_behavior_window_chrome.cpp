#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_caption(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CAPTION", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "Save"},
        {"cmdCancel", "cmdCancel", "two-guid", "Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cmdOther", "cmdOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1042: synthetic SCX table for object caption should be created");
}

void write_synthetic_form_table_for_object_whats_this_help_id(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "WHATSTHISHELPID", .type = 'N', .length = 6U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1142: synthetic SCX table for object whats-this-help-id should be created");
}

void write_synthetic_form_table_for_object_whats_this_help(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "WHATSTHISHELP", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", ".F."},
        {"cmdCancel", "cmdCancel", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".T."},
        {"cmdOther", "cmdOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1143: synthetic SCX table for object whats-this-help should be created");
}

void write_synthetic_form_table_for_object_whats_this_button(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "WHATSTHISBUTTON", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", ".F."},
        {"cmdCancel", "cmdCancel", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".T."},
        {"cmdOther", "cmdOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1144: synthetic SCX table for object whats-this-button should be created");
}

void write_synthetic_form_table_for_object_status_bar_text(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "STATUSBARTEXT", .type = 'C', .length = 70U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "Save"},
        {"cmdCancel", "cmdCancel", "two-guid", "Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cmdOther", "cmdOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1044: synthetic SCX table for object status-bar text should be created");
}

void write_synthetic_form_table_for_object_closable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CLOSABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1073: synthetic SCX table for object closable should be created");
}

void write_synthetic_form_table_for_object_control_box(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CONTROLBOX", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1074: synthetic SCX table for object control box should be created");
}

void write_synthetic_form_table_for_object_auto_center(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "AUTOCENTER", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1078: synthetic SCX table for object auto center should be created");
}

void write_synthetic_form_table_for_object_desktop(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DESKTOP", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1147: synthetic SCX table for object desktop should be created");
}

void write_synthetic_form_table_for_object_key_preview(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "KEYPREVIEW", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1148: synthetic SCX table for object key preview should be created");
}

void write_synthetic_form_table_for_object_mac_desktop(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MACDESKTOP", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1149: synthetic SCX table for object mac desktop should be created");
}

void write_synthetic_form_table_for_object_max_button(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXBUTTON", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1150: synthetic SCX table for object max button should be created");
}

void write_synthetic_form_table_for_object_min_button(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MINBUTTON", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1155: synthetic SCX table for object min button should be created");
}

void write_synthetic_form_table_for_object_min_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MINHEIGHT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1156: synthetic SCX table for object min height should be created");
}

void write_synthetic_form_table_for_object_min_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MINWIDTH", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1157: synthetic SCX table for object min width should be created");
}

void write_synthetic_form_table_for_object_max_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXHEIGHT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1151: synthetic SCX table for object max height should be created");
}

void write_synthetic_form_table_for_object_movable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MOVABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1158: synthetic SCX table for object movable should be created");
}

void write_synthetic_form_table_for_object_half_height_caption(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HALFHEIGHTCAPTION", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1159: synthetic SCX table for object half-height-caption should be created");
}

void write_synthetic_form_table_for_object_mdi_form(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MDIFORM", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1160: synthetic SCX table for object MDI form should be created");
}

void write_synthetic_form_table_for_object_max_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXWIDTH", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1152: synthetic SCX table for object max width should be created");
}

void write_synthetic_form_table_for_object_max_left(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXLEFT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1153: synthetic SCX table for object max left should be created");
}

void write_synthetic_form_table_for_object_max_top(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXTOP", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1154: synthetic SCX table for object max top should be created");
}

void write_synthetic_form_table_for_object_dockable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DOCKABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1082: synthetic SCX table for object dockable should be created");
}

void write_synthetic_form_table_for_object_lock_screen(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LOCKSCREEN", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1085: synthetic SCX table for object lock screen should be created");
}

void write_synthetic_form_table_for_object_split_bar(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SPLITBAR", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1089: synthetic SCX table for object split bar should be created");
}

void write_synthetic_form_table_for_object_panel_link(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PANELLINK", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1091: synthetic SCX table for object panel link should be created");
}

void write_synthetic_form_table_for_object_resizable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "RESIZABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1094: synthetic SCX table for object resizable should be created");
}

void write_synthetic_form_table_for_object_always_on_top(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALWAYSONTOP", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1096: synthetic SCX table for object always on top should be created");
}

void write_synthetic_form_table_for_object_always_on_bottom(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALWAYSONBOTTOM", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1097: synthetic SCX table for object always on bottom should be created");
}

void test_studio_host_json_assigns_caption_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_caption_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path caption_path = temp_root / "caption.scx";
    write_synthetic_form_table_for_object_caption(caption_path);
    const auto caption_process = run_process_capture(
        studio_host_path,
        {
            "--path", caption_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--caption-target-object-name", "cmdSave",
            "--caption-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(caption_process.exit_code == 0,
        "#1042: host object caption assignment should exit successfully");
    expect(visual_object_property(caption_path, "one-guid", "CAPTION") == "Save Customer" &&
            visual_object_property(caption_path, "two-guid", "CAPTION") == "Save Customer" &&
            visual_object_property(caption_path, "three-guid", "CAPTION") == "Ready" &&
            visual_object_property(caption_path, "other-guid", "CAPTION") == "Other",
        "#1042: host object caption assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_caption(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--caption-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1042: missing-target host object caption assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CAPTION") == "Save" &&
            visual_object_property(missing_target_path, "two-guid", "CAPTION") == "Cancel",
        "#1042: missing-target host object caption assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_caption(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1042: caption-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CAPTION") == "Save",
        "#1042: caption-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_caption(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--caption-object",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1042: caption-object without caption value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CAPTION") == "Save",
        "#1042: caption-object without caption value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_caption(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--caption-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1042: duplicate-target host object caption assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CAPTION") == "Save",
        "#1042: duplicate-target host object caption assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_caption(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--caption-object",
            "--locked-object",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1042: caption-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CAPTION") == "Save",
        "#1042: caption-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_whats_this_help_id_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_whats_this_help_id_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path whats_this_help_id_path = temp_root / "whats_this_help_id.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(whats_this_help_id_path);
    const auto whats_this_help_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", whats_this_help_id_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "900",
            "--whats-this-help-id-target-object-name", "cmdSave",
            "--whats-this-help-id-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(whats_this_help_id_process.exit_code == 0,
        "#1142: host object whats-this-help-id assignment should exit successfully");
    expect(visual_object_property(whats_this_help_id_path, "one-guid", "WHATSTHISHELPID") == "900" &&
            visual_object_property(whats_this_help_id_path, "two-guid", "WHATSTHISHELPID") == "900" &&
            visual_object_property(whats_this_help_id_path, "three-guid", "WHATSTHISHELPID") == "2" &&
            visual_object_property(whats_this_help_id_path, "other-guid", "WHATSTHISHELPID") == "0",
        "#1142: host object whats-this-help-id assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "2",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--whats-this-help-id-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1142: missing-target host object whats-this-help-id assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "WHATSTHISHELPID") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "WHATSTHISHELPID") == "1",
        "#1142: missing-target host object whats-this-help-id assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1142: whats-this-help-id-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: whats-this-help-id-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1142: whats-this-help-id-object without whats-this-help-id value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: whats-this-help-id-object without whats-this-help-id value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "-1",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1142: negative whats-this-help-id values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: negative whats-this-help-id values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "2",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--whats-this-help-id-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1142: duplicate-target host object whats-this-help-id assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: duplicate-target host object whats-this-help-id assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--whats-this-help-id-object",
            "--locked-object",
            "--whats-this-help-id", "2",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1142: whats-this-help-id-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: whats-this-help-id-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_whats_this_help_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_whats_this_help_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path whats_this_help_path = temp_root / "whats_this_help.scx";
    write_synthetic_form_table_for_object_whats_this_help(whats_this_help_path);
    const auto whats_this_help_process = run_process_capture(
        studio_host_path,
        {
            "--path", whats_this_help_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-object-name", "cmdSave",
            "--whats-this-help-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(whats_this_help_process.exit_code == 0,
        "#1143: host object whats-this-help assignment should exit successfully");
    expect(visual_object_property(whats_this_help_path, "one-guid", "WHATSTHISHELP") == ".T." &&
            visual_object_property(whats_this_help_path, "two-guid", "WHATSTHISHELP") == ".T." &&
            visual_object_property(whats_this_help_path, "three-guid", "WHATSTHISHELP") == ".T." &&
            visual_object_property(whats_this_help_path, "other-guid", "WHATSTHISHELP") == ".F.",
        "#1143: host object whats-this-help assignment should assign selected logical values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_whats_this_help(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-unique-id", "one-guid",
            "--whats-this-help-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1143: missing-target host object whats-this-help assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "WHATSTHISHELP") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "WHATSTHISHELP") == ".F.",
        "#1143: missing-target host object whats-this-help assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_whats_this_help(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1143: whats-this-help-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: whats-this-help-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_whats_this_help(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--whats-this-help-object",
            "--whats-this-help-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1143: whats-this-help-object without whats-this-help value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: whats-this-help-object without whats-this-help value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_whats_this_help(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-unique-id", "one-guid",
            "--whats-this-help-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1143: duplicate-target host object whats-this-help assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: duplicate-target host object whats-this-help assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_whats_this_help(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--whats-this-help-object",
            "--locked-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1143: whats-this-help-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: whats-this-help-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_whats_this_button_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_whats_this_button_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path whats_this_button_path = temp_root / "whats_this_button.scx";
    write_synthetic_form_table_for_object_whats_this_button(whats_this_button_path);
    const auto whats_this_button_process = run_process_capture(
        studio_host_path,
        {
            "--path", whats_this_button_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-object-name", "cmdSave",
            "--whats-this-button-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(whats_this_button_process.exit_code == 0,
        "#1144: host object whats-this-button assignment should exit successfully");
    expect(visual_object_property(whats_this_button_path, "one-guid", "WHATSTHISBUTTON") == ".T." &&
            visual_object_property(whats_this_button_path, "two-guid", "WHATSTHISBUTTON") == ".T." &&
            visual_object_property(whats_this_button_path, "three-guid", "WHATSTHISBUTTON") == ".T." &&
            visual_object_property(whats_this_button_path, "other-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: host object whats-this-button assignment should assign selected logical values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_whats_this_button(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-unique-id", "one-guid",
            "--whats-this-button-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1144: missing-target host object whats-this-button assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "WHATSTHISBUTTON") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: missing-target host object whats-this-button assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_whats_this_button(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1144: whats-this-button-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: whats-this-button-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_whats_this_button(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--whats-this-button-object",
            "--whats-this-button-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1144: whats-this-button-object without whats-this-button value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: whats-this-button-object without whats-this-button value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_whats_this_button(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-unique-id", "one-guid",
            "--whats-this-button-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1144: duplicate-target host object whats-this-button assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: duplicate-target host object whats-this-button assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_whats_this_button(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--whats-this-button-object",
            "--locked-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1144: whats-this-button-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: whats-this-button-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_status_bar_text_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_status_bar_text_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path status_bar_text_path = temp_root / "status_bar_text.scx";
    write_synthetic_form_table_for_object_status_bar_text(status_bar_text_path);
    const auto status_bar_text_process = run_process_capture(
        studio_host_path,
        {
            "--path", status_bar_text_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-object-name", "cmdSave",
            "--status-bar-text-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(status_bar_text_process.exit_code == 0,
        "#1044: host object status-bar text assignment should exit successfully");
    expect(visual_object_property(status_bar_text_path, "one-guid", "STATUSBARTEXT") == "Ready to save" &&
            visual_object_property(status_bar_text_path, "two-guid", "STATUSBARTEXT") == "Ready to save" &&
            visual_object_property(status_bar_text_path, "three-guid", "STATUSBARTEXT") == "Ready" &&
            visual_object_property(status_bar_text_path, "other-guid", "STATUSBARTEXT") == "Other",
        "#1044: host object status-bar text assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_status_bar_text(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-unique-id", "one-guid",
            "--status-bar-text-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1044: missing-target host object status-bar text assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "STATUSBARTEXT") == "Save" &&
            visual_object_property(missing_target_path, "two-guid", "STATUSBARTEXT") == "Cancel",
        "#1044: missing-target host object status-bar text assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_status_bar_text(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1044: status-bar-text-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: status-bar-text-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_status_bar_text(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--status-bar-text-object",
            "--status-bar-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1044: status-bar-text-object without status-bar text value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: status-bar-text-object without status-bar text value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_status_bar_text(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-unique-id", "one-guid",
            "--status-bar-text-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1044: duplicate-target host object status-bar text assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: duplicate-target host object status-bar text assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_status_bar_text(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--status-bar-text-object",
            "--tooltip-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-unique-id", "one-guid",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1044: status-bar-text-object plus tooltip-text-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: status-bar-text-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_closable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_closable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path closable_path = temp_root / "closable.scx";
    write_synthetic_form_table_for_object_closable(closable_path);
    const auto closable_process = run_process_capture(
        studio_host_path,
        {
            "--path", closable_path.string(),
            "--closable-object",
            "--closable", "false",
            "--closable-target-object-name", "frmCustomer",
            "--closable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(closable_process.exit_code == 0,
        "#1073: host object closable assignment should exit successfully");
    expect(visual_object_property(closable_path, "one-guid", "CLOSABLE") == "false" &&
            visual_object_property(closable_path, "two-guid", "CLOSABLE") == "false" &&
            visual_object_property(closable_path, "three-guid", "CLOSABLE") == "false" &&
            visual_object_property(closable_path, "other-guid", "CLOSABLE") == "true",
        "#1073: host object closable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_closable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--closable-object",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--closable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1073: missing-target host object closable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CLOSABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "CLOSABLE") == "true",
        "#1073: missing-target host object closable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_closable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--closable-object",
            "--closable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1073: closable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CLOSABLE") == "true",
        "#1073: closable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_closable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--closable-object",
            "--closable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1073: closable-object without closable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CLOSABLE") == "true",
        "#1073: closable-object without closable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_closable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--closable-object",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--closable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1073: duplicate-target host object closable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CLOSABLE") == "true",
        "#1073: duplicate-target host object closable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_closable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--closable-object",
            "--dynamic-fore-color-object",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--dynamic-fore-color", "RGB(9,9,9)",
            "--dynamic-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1073: closable-object plus dynamic-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CLOSABLE") == "true",
        "#1073: closable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_control_box_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_control_box_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path control_box_path = temp_root / "control_box.scx";
    write_synthetic_form_table_for_object_control_box(control_box_path);
    const auto control_box_process = run_process_capture(
        studio_host_path,
        {
            "--path", control_box_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--control-box-target-object-name", "frmCustomer",
            "--control-box-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(control_box_process.exit_code == 0,
        "#1074: host object control-box assignment should exit successfully");
    expect(visual_object_property(control_box_path, "one-guid", "CONTROLBOX") == "false" &&
            visual_object_property(control_box_path, "two-guid", "CONTROLBOX") == "false" &&
            visual_object_property(control_box_path, "three-guid", "CONTROLBOX") == "false" &&
            visual_object_property(control_box_path, "other-guid", "CONTROLBOX") == "true",
        "#1074: host object control-box assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_control_box(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--control-box-target-unique-id", "one-guid",
            "--control-box-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1074: missing-target host object control-box assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CONTROLBOX") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "CONTROLBOX") == "true",
        "#1074: missing-target host object control-box assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_control_box(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1074: control-box-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: control-box-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_control_box(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--control-box-object",
            "--control-box-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1074: control-box-object without control-box value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: control-box-object without control-box value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_control_box(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--control-box-target-unique-id", "one-guid",
            "--control-box-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1074: duplicate-target host object control-box assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: duplicate-target host object control-box assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_control_box(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--control-box-object",
            "--closable-object",
            "--control-box", "false",
            "--control-box-target-unique-id", "one-guid",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1074: control-box-object plus closable-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: control-box-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_desktop_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_desktop_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path desktop_path = temp_root / "desktop.scx";
    write_synthetic_form_table_for_object_desktop(desktop_path);
    const auto desktop_process = run_process_capture(
        studio_host_path,
        {
            "--path", desktop_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--desktop-target-object-name", "frmCustomer",
            "--desktop-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(desktop_process.exit_code == 0,
        "#1147: host object desktop assignment should exit successfully");
    expect(visual_object_property(desktop_path, "one-guid", "DESKTOP") == "false" &&
            visual_object_property(desktop_path, "two-guid", "DESKTOP") == "false" &&
            visual_object_property(desktop_path, "three-guid", "DESKTOP") == "false" &&
            visual_object_property(desktop_path, "other-guid", "DESKTOP") == "true",
        "#1147: host object desktop assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_desktop(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--desktop-target-unique-id", "one-guid",
            "--desktop-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1147: missing-target host object desktop assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DESKTOP") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "DESKTOP") == "true",
        "#1147: missing-target host object desktop assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_desktop(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1147: desktop-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DESKTOP") == "true",
        "#1147: desktop-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_desktop(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--desktop-object",
            "--desktop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1147: desktop-object without desktop value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DESKTOP") == "true",
        "#1147: desktop-object without desktop value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_desktop(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--desktop-target-unique-id", "one-guid",
            "--desktop-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1147: duplicate-target host object desktop assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DESKTOP") == "true",
        "#1147: duplicate-target host object desktop assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_desktop(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--desktop-object",
            "--allow-output-object",
            "--desktop", "false",
            "--desktop-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1147: desktop-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DESKTOP") == "true",
        "#1147: desktop-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_key_preview_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_key_preview_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path key_preview_path = temp_root / "key_preview.scx";
    write_synthetic_form_table_for_object_key_preview(key_preview_path);
    const auto key_preview_process = run_process_capture(
        studio_host_path,
        {
            "--path", key_preview_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--key-preview-target-object-name", "frmCustomer",
            "--key-preview-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(key_preview_process.exit_code == 0,
        "#1148: host object key-preview assignment should exit successfully");
    expect(visual_object_property(key_preview_path, "one-guid", "KEYPREVIEW") == "false" &&
            visual_object_property(key_preview_path, "two-guid", "KEYPREVIEW") == "false" &&
            visual_object_property(key_preview_path, "three-guid", "KEYPREVIEW") == "false" &&
            visual_object_property(key_preview_path, "other-guid", "KEYPREVIEW") == "true",
        "#1148: host object key-preview assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_key_preview(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--key-preview-target-unique-id", "one-guid",
            "--key-preview-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1148: missing-target host object key-preview assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "KEYPREVIEW") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "KEYPREVIEW") == "true",
        "#1148: missing-target host object key-preview assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_key_preview(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1148: key-preview-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: key-preview-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_key_preview(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--key-preview-object",
            "--key-preview-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1148: key-preview-object without key-preview value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: key-preview-object without key-preview value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_key_preview(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--key-preview-target-unique-id", "one-guid",
            "--key-preview-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1148: duplicate-target host object key-preview assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: duplicate-target host object key-preview assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_key_preview(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--key-preview-object",
            "--allow-output-object",
            "--key-preview", "false",
            "--key-preview-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1148: key-preview-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: key-preview-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_mac_desktop_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mac_desktop_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mac_desktop_path = temp_root / "mac_desktop.scx";
    write_synthetic_form_table_for_object_mac_desktop(mac_desktop_path);
    const auto mac_desktop_process = run_process_capture(
        studio_host_path,
        {
            "--path", mac_desktop_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-object-name", "frmCustomer",
            "--mac-desktop-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(mac_desktop_process.exit_code == 0,
        "#1149: host object mac-desktop assignment should exit successfully");
    expect(visual_object_property(mac_desktop_path, "one-guid", "MACDESKTOP") == "false" &&
            visual_object_property(mac_desktop_path, "two-guid", "MACDESKTOP") == "false" &&
            visual_object_property(mac_desktop_path, "three-guid", "MACDESKTOP") == "false" &&
            visual_object_property(mac_desktop_path, "other-guid", "MACDESKTOP") == "true",
        "#1149: host object mac-desktop assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_mac_desktop(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-unique-id", "one-guid",
            "--mac-desktop-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1149: missing-target host object mac-desktop assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MACDESKTOP") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MACDESKTOP") == "true",
        "#1149: missing-target host object mac-desktop assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_mac_desktop(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1149: mac-desktop-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: mac-desktop-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_mac_desktop(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--mac-desktop-object",
            "--mac-desktop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1149: mac-desktop-object without mac-desktop value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: mac-desktop-object without mac-desktop value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_mac_desktop(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-unique-id", "one-guid",
            "--mac-desktop-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1149: duplicate-target host object mac-desktop assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: duplicate-target host object mac-desktop assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_mac_desktop(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--mac-desktop-object",
            "--allow-output-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1149: mac-desktop-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: mac-desktop-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_max_button_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_button_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_button_path = temp_root / "max_button.scx";
    write_synthetic_form_table_for_object_max_button(max_button_path);
    const auto max_button_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_button_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--max-button-target-object-name", "frmCustomer",
            "--max-button-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_button_process.exit_code == 0,
        "#1150: host object max-button assignment should exit successfully");
    expect(visual_object_property(max_button_path, "one-guid", "MAXBUTTON") == "false" &&
            visual_object_property(max_button_path, "two-guid", "MAXBUTTON") == "false" &&
            visual_object_property(max_button_path, "three-guid", "MAXBUTTON") == "false" &&
            visual_object_property(max_button_path, "other-guid", "MAXBUTTON") == "true",
        "#1150: host object max-button assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_button(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--max-button-target-unique-id", "one-guid",
            "--max-button-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1150: missing-target host object max-button assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXBUTTON") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MAXBUTTON") == "true",
        "#1150: missing-target host object max-button assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_button(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1150: max-button-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: max-button-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_button(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-button-object",
            "--max-button-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1150: max-button-object without max-button value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: max-button-object without max-button value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_button(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--max-button-target-unique-id", "one-guid",
            "--max-button-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1150: duplicate-target host object max-button assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: duplicate-target host object max-button assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_button(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-button-object",
            "--allow-output-object",
            "--max-button", "false",
            "--max-button-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1150: max-button-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: max-button-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_min_button_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_min_button_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path min_button_path = temp_root / "min_button.scx";
    write_synthetic_form_table_for_object_min_button(min_button_path);
    const auto min_button_process = run_process_capture(
        studio_host_path,
        {
            "--path", min_button_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--min-button-target-object-name", "frmCustomer",
            "--min-button-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(min_button_process.exit_code == 0,
        "#1155: host object min-button assignment should exit successfully");
    expect(visual_object_property(min_button_path, "one-guid", "MINBUTTON") == "false" &&
            visual_object_property(min_button_path, "two-guid", "MINBUTTON") == "false" &&
            visual_object_property(min_button_path, "three-guid", "MINBUTTON") == "false" &&
            visual_object_property(min_button_path, "other-guid", "MINBUTTON") == "true",
        "#1155: host object min-button assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_min_button(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--min-button-target-unique-id", "one-guid",
            "--min-button-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1155: missing-target host object min-button assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MINBUTTON") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MINBUTTON") == "true",
        "#1155: missing-target host object min-button assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_min_button(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1155: min-button-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MINBUTTON") == "true",
        "#1155: min-button-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_min_button(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--min-button-object",
            "--min-button-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1155: min-button-object without min-button value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MINBUTTON") == "true",
        "#1155: min-button-object without min-button value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_min_button(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--min-button-target-unique-id", "one-guid",
            "--min-button-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1155: duplicate-target host object min-button assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MINBUTTON") == "true",
        "#1155: duplicate-target host object min-button assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_min_button(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--min-button-object",
            "--allow-output-object",
            "--min-button", "false",
            "--min-button-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1155: min-button-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MINBUTTON") == "true",
        "#1155: min-button-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_min_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_min_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path min_height_path = temp_root / "min_height.scx";
    write_synthetic_form_table_for_object_min_height(min_height_path);
    const auto min_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", min_height_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--min-height-target-object-name", "frmCustomer",
            "--min-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(min_height_process.exit_code == 0,
        "#1156: host object min-height assignment should exit successfully");
    expect(visual_object_property(min_height_path, "one-guid", "MINHEIGHT") == "640" &&
            visual_object_property(min_height_path, "two-guid", "MINHEIGHT") == "640" &&
            visual_object_property(min_height_path, "three-guid", "MINHEIGHT") == "300" &&
            visual_object_property(min_height_path, "other-guid", "MINHEIGHT") == "400",
        "#1156: host object min-height assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_min_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--min-height-target-unique-id", "one-guid",
            "--min-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1156: missing-target host object min-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MINHEIGHT") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MINHEIGHT") == "200",
        "#1156: missing-target host object min-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_min_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1156: min-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_min_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--min-height-object",
            "--min-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1156: min-height-object without min-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object without min-height value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_min_height(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--min-height-object",
            "--min-height", "-1",
            "--min-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1156: min-height-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_min_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--min-height-target-unique-id", "one-guid",
            "--min-height-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1156: duplicate-target host object min-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: duplicate-target host object min-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_min_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--min-height-object",
            "--allow-output-object",
            "--min-height", "640",
            "--min-height-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1156: min-height-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_min_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_min_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path min_width_path = temp_root / "min_width.scx";
    write_synthetic_form_table_for_object_min_width(min_width_path);
    const auto min_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", min_width_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--min-width-target-object-name", "frmCustomer",
            "--min-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(min_width_process.exit_code == 0,
        "#1157: host object min-width assignment should exit successfully");
    expect(visual_object_property(min_width_path, "one-guid", "MINWIDTH") == "640" &&
            visual_object_property(min_width_path, "two-guid", "MINWIDTH") == "640" &&
            visual_object_property(min_width_path, "three-guid", "MINWIDTH") == "300" &&
            visual_object_property(min_width_path, "other-guid", "MINWIDTH") == "400",
        "#1157: host object min-width assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_min_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--min-width-target-unique-id", "one-guid",
            "--min-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1157: missing-target host object min-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MINWIDTH") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MINWIDTH") == "200",
        "#1157: missing-target host object min-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_min_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1157: min-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_min_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--min-width-object",
            "--min-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1157: min-width-object without min-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object without min-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_min_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--min-width-object",
            "--min-width", "-1",
            "--min-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1157: min-width-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_min_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--min-width-target-unique-id", "one-guid",
            "--min-width-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1157: duplicate-target host object min-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MINWIDTH") == "100",
        "#1157: duplicate-target host object min-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_min_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--min-width-object",
            "--allow-output-object",
            "--min-width", "640",
            "--min-width-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1157: min-width-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_max_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_height_path = temp_root / "max_height.scx";
    write_synthetic_form_table_for_object_max_height(max_height_path);
    const auto max_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_height_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--max-height-target-object-name", "frmCustomer",
            "--max-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_height_process.exit_code == 0,
        "#1151: host object max-height assignment should exit successfully");
    expect(visual_object_property(max_height_path, "one-guid", "MAXHEIGHT") == "640" &&
            visual_object_property(max_height_path, "two-guid", "MAXHEIGHT") == "640" &&
            visual_object_property(max_height_path, "three-guid", "MAXHEIGHT") == "300" &&
            visual_object_property(max_height_path, "other-guid", "MAXHEIGHT") == "400",
        "#1151: host object max-height assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--max-height-target-unique-id", "one-guid",
            "--max-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1151: missing-target host object max-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXHEIGHT") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXHEIGHT") == "200",
        "#1151: missing-target host object max-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1151: max-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-height-object",
            "--max-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1151: max-height-object without max-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object without max-height value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_height(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-height-object",
            "--max-height", "-1",
            "--max-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1151: max-height-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--max-height-target-unique-id", "one-guid",
            "--max-height-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1151: duplicate-target host object max-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: duplicate-target host object max-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-height-object",
            "--allow-output-object",
            "--max-height", "640",
            "--max-height-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1151: max-height-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_movable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_movable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path movable_path = temp_root / "movable.scx";
    write_synthetic_form_table_for_object_movable(movable_path);
    const auto movable_process = run_process_capture(
        studio_host_path,
        {
            "--path", movable_path.string(),
            "--movable-object",
            "--movable", "false",
            "--movable-target-object-name", "frmCustomer",
            "--movable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(movable_process.exit_code == 0,
        "#1158: host object movable assignment should exit successfully");
    expect(visual_object_property(movable_path, "one-guid", "MOVABLE") == "false" &&
            visual_object_property(movable_path, "two-guid", "MOVABLE") == "false" &&
            visual_object_property(movable_path, "three-guid", "MOVABLE") == "false" &&
            visual_object_property(movable_path, "other-guid", "MOVABLE") == "true",
        "#1158: host object movable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_movable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--movable-object",
            "--movable", "false",
            "--movable-target-unique-id", "one-guid",
            "--movable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1158: missing-target host object movable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MOVABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MOVABLE") == "true",
        "#1158: missing-target host object movable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_movable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--movable-object",
            "--movable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1158: movable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MOVABLE") == "true",
        "#1158: movable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_movable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--movable-object",
            "--movable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1158: movable-object without movable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MOVABLE") == "true",
        "#1158: movable-object without movable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_movable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--movable-object",
            "--movable", "false",
            "--movable-target-unique-id", "one-guid",
            "--movable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1158: duplicate-target host object movable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MOVABLE") == "true",
        "#1158: duplicate-target host object movable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_movable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--movable-object",
            "--allow-output-object",
            "--movable", "false",
            "--movable-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1158: movable-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MOVABLE") == "true",
        "#1158: movable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_half_height_caption_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_half_height_caption_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path half_height_caption_path = temp_root / "half_height_caption.scx";
    write_synthetic_form_table_for_object_half_height_caption(half_height_caption_path);
    const auto half_height_caption_process = run_process_capture(
        studio_host_path,
        {
            "--path", half_height_caption_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-object-name", "frmCustomer",
            "--half-height-caption-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(half_height_caption_process.exit_code == 0,
        "#1159: host object half-height-caption assignment should exit successfully");
    expect(visual_object_property(half_height_caption_path, "one-guid", "HALFHEIGHTCAPTION") == "false" &&
            visual_object_property(half_height_caption_path, "two-guid", "HALFHEIGHTCAPTION") == "false" &&
            visual_object_property(half_height_caption_path, "three-guid", "HALFHEIGHTCAPTION") == "false" &&
            visual_object_property(half_height_caption_path, "other-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: host object half-height-caption assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_half_height_caption(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-unique-id", "one-guid",
            "--half-height-caption-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1159: missing-target host object half-height-caption assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HALFHEIGHTCAPTION") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: missing-target host object half-height-caption assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_half_height_caption(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1159: half-height-caption-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: half-height-caption-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_half_height_caption(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--half-height-caption-object",
            "--half-height-caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1159: half-height-caption-object without half-height-caption value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: half-height-caption-object without half-height-caption value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_half_height_caption(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-unique-id", "one-guid",
            "--half-height-caption-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1159: duplicate-target host object half-height-caption assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: duplicate-target host object half-height-caption assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_half_height_caption(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--half-height-caption-object",
            "--allow-output-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1159: half-height-caption-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: half-height-caption-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_mdi_form_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mdi_form_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mdi_form_path = temp_root / "mdi_form.scx";
    write_synthetic_form_table_for_object_mdi_form(mdi_form_path);
    const auto mdi_form_process = run_process_capture(
        studio_host_path,
        {
            "--path", mdi_form_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--mdi-form-target-object-name", "frmCustomer",
            "--mdi-form-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(mdi_form_process.exit_code == 0,
        "#1160: host object MDI-form assignment should exit successfully");
    expect(visual_object_property(mdi_form_path, "one-guid", "MDIFORM") == "false" &&
            visual_object_property(mdi_form_path, "two-guid", "MDIFORM") == "false" &&
            visual_object_property(mdi_form_path, "three-guid", "MDIFORM") == "false" &&
            visual_object_property(mdi_form_path, "other-guid", "MDIFORM") == "true",
        "#1160: host object MDI-form assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_mdi_form(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--mdi-form-target-unique-id", "one-guid",
            "--mdi-form-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1160: missing-target host object MDI-form assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MDIFORM") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MDIFORM") == "true",
        "#1160: missing-target host object MDI-form assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_mdi_form(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1160: mdi-form-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MDIFORM") == "true",
        "#1160: mdi-form-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_mdi_form(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--mdi-form-object",
            "--mdi-form-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1160: mdi-form-object without MDI-form value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MDIFORM") == "true",
        "#1160: mdi-form-object without MDI-form value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_mdi_form(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--mdi-form-target-unique-id", "one-guid",
            "--mdi-form-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1160: duplicate-target host object MDI-form assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MDIFORM") == "true",
        "#1160: duplicate-target host object MDI-form assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_mdi_form(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--mdi-form-object",
            "--allow-output-object",
            "--mdi-form", "false",
            "--mdi-form-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1160: mdi-form-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MDIFORM") == "true",
        "#1160: mdi-form-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_max_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_width_path = temp_root / "max_width.scx";
    write_synthetic_form_table_for_object_max_width(max_width_path);
    const auto max_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_width_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--max-width-target-object-name", "frmCustomer",
            "--max-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_width_process.exit_code == 0,
        "#1152: host object max-width assignment should exit successfully");
    expect(visual_object_property(max_width_path, "one-guid", "MAXWIDTH") == "640" &&
            visual_object_property(max_width_path, "two-guid", "MAXWIDTH") == "640" &&
            visual_object_property(max_width_path, "three-guid", "MAXWIDTH") == "300" &&
            visual_object_property(max_width_path, "other-guid", "MAXWIDTH") == "400",
        "#1152: host object max-width assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--max-width-target-unique-id", "one-guid",
            "--max-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1152: missing-target host object max-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXWIDTH") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXWIDTH") == "200",
        "#1152: missing-target host object max-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1152: max-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-width-object",
            "--max-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1152: max-width-object without max-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object without max-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-width-object",
            "--max-width", "-1",
            "--max-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1152: max-width-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--max-width-target-unique-id", "one-guid",
            "--max-width-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1152: duplicate-target host object max-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: duplicate-target host object max-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-width-object",
            "--allow-output-object",
            "--max-width", "640",
            "--max-width-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1152: max-width-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_max_left_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_left_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_left_path = temp_root / "max_left.scx";
    write_synthetic_form_table_for_object_max_left(max_left_path);
    const auto max_left_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_left_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--max-left-target-object-name", "frmCustomer",
            "--max-left-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_left_process.exit_code == 0,
        "#1153: host object max-left assignment should exit successfully");
    expect(visual_object_property(max_left_path, "one-guid", "MAXLEFT") == "640" &&
            visual_object_property(max_left_path, "two-guid", "MAXLEFT") == "640" &&
            visual_object_property(max_left_path, "three-guid", "MAXLEFT") == "300" &&
            visual_object_property(max_left_path, "other-guid", "MAXLEFT") == "400",
        "#1153: host object max-left assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_left(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--max-left-target-unique-id", "one-guid",
            "--max-left-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1153: missing-target host object max-left assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXLEFT") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXLEFT") == "200",
        "#1153: missing-target host object max-left assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_left(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1153: max-left-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_left(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-left-object",
            "--max-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1153: max-left-object without max-left value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object without max-left value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_left(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-left-object",
            "--max-left", "-1",
            "--max-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1153: max-left-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_left(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--max-left-target-unique-id", "one-guid",
            "--max-left-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1153: duplicate-target host object max-left assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXLEFT") == "100",
        "#1153: duplicate-target host object max-left assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_left(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-left-object",
            "--allow-output-object",
            "--max-left", "640",
            "--max-left-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1153: max-left-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_max_top_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_top_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_top_path = temp_root / "max_top.scx";
    write_synthetic_form_table_for_object_max_top(max_top_path);
    const auto max_top_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_top_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--max-top-target-object-name", "frmCustomer",
            "--max-top-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_top_process.exit_code == 0,
        "#1154: host object max-top assignment should exit successfully");
    expect(visual_object_property(max_top_path, "one-guid", "MAXTOP") == "640" &&
            visual_object_property(max_top_path, "two-guid", "MAXTOP") == "640" &&
            visual_object_property(max_top_path, "three-guid", "MAXTOP") == "300" &&
            visual_object_property(max_top_path, "other-guid", "MAXTOP") == "400",
        "#1154: host object max-top assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_top(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--max-top-target-unique-id", "one-guid",
            "--max-top-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1154: missing-target host object max-top assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXTOP") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXTOP") == "200",
        "#1154: missing-target host object max-top assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_top(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1154: max-top-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_top(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-top-object",
            "--max-top-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1154: max-top-object without max-top value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object without max-top value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_top(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-top-object",
            "--max-top", "-1",
            "--max-top-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1154: max-top-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_top(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--max-top-target-unique-id", "one-guid",
            "--max-top-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1154: duplicate-target host object max-top assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXTOP") == "100",
        "#1154: duplicate-target host object max-top assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_top(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-top-object",
            "--allow-output-object",
            "--max-top", "640",
            "--max-top-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1154: max-top-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_auto_center_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_auto_center_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path auto_center_path = temp_root / "auto_center.scx";
    write_synthetic_form_table_for_object_auto_center(auto_center_path);
    const auto auto_center_process = run_process_capture(
        studio_host_path,
        {
            "--path", auto_center_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--auto-center-target-object-name", "frmCustomer",
            "--auto-center-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(auto_center_process.exit_code == 0,
        "#1078: host object auto-center assignment should exit successfully");
    expect(visual_object_property(auto_center_path, "one-guid", "AUTOCENTER") == "false" &&
            visual_object_property(auto_center_path, "two-guid", "AUTOCENTER") == "false" &&
            visual_object_property(auto_center_path, "three-guid", "AUTOCENTER") == "false" &&
            visual_object_property(auto_center_path, "other-guid", "AUTOCENTER") == "true",
        "#1078: host object auto-center assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_auto_center(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--auto-center-target-unique-id", "one-guid",
            "--auto-center-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1078: missing-target host object auto-center assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "AUTOCENTER") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "AUTOCENTER") == "true",
        "#1078: missing-target host object auto-center assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_auto_center(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1078: auto-center-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: auto-center-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_auto_center(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--auto-center-object",
            "--auto-center-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1078: auto-center-object without auto-center value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: auto-center-object without auto-center value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_auto_center(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--auto-center-target-unique-id", "one-guid",
            "--auto-center-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1078: duplicate-target host object auto-center assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: duplicate-target host object auto-center assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_auto_center(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--auto-center-object",
            "--allow-output-object",
            "--auto-center", "false",
            "--auto-center-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1078: auto-center-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: auto-center-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dockable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dockable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path dockable_path = temp_root / "dockable.scx";
    write_synthetic_form_table_for_object_dockable(dockable_path);
    const auto dockable_process = run_process_capture(
        studio_host_path,
        {
            "--path", dockable_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--dockable-target-object-name", "frmCustomer",
            "--dockable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dockable_process.exit_code == 0,
        "#1082: host object dockable assignment should exit successfully");
    expect(visual_object_property(dockable_path, "one-guid", "DOCKABLE") == "false" &&
            visual_object_property(dockable_path, "two-guid", "DOCKABLE") == "false" &&
            visual_object_property(dockable_path, "three-guid", "DOCKABLE") == "false" &&
            visual_object_property(dockable_path, "other-guid", "DOCKABLE") == "true",
        "#1082: host object dockable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dockable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--dockable-target-unique-id", "one-guid",
            "--dockable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1082: missing-target host object dockable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DOCKABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "DOCKABLE") == "true",
        "#1082: missing-target host object dockable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dockable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1082: dockable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DOCKABLE") == "true",
        "#1082: dockable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dockable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dockable-object",
            "--dockable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1082: dockable-object without dockable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DOCKABLE") == "true",
        "#1082: dockable-object without dockable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dockable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--dockable-target-unique-id", "one-guid",
            "--dockable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1082: duplicate-target host object dockable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DOCKABLE") == "true",
        "#1082: duplicate-target host object dockable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dockable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dockable-object",
            "--auto-size-object",
            "--dockable", "false",
            "--dockable-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1082: dockable-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DOCKABLE") == "true",
        "#1082: dockable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_lock_screen_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_lock_screen_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path lock_screen_path = temp_root / "lock_screen.scx";
    write_synthetic_form_table_for_object_lock_screen(lock_screen_path);
    const auto lock_screen_process = run_process_capture(
        studio_host_path,
        {
            "--path", lock_screen_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--lock-screen-target-object-name", "frmCustomer",
            "--lock-screen-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(lock_screen_process.exit_code == 0,
        "#1085: host object lock-screen assignment should exit successfully");
    expect(visual_object_property(lock_screen_path, "one-guid", "LOCKSCREEN") == "false" &&
            visual_object_property(lock_screen_path, "two-guid", "LOCKSCREEN") == "false" &&
            visual_object_property(lock_screen_path, "three-guid", "LOCKSCREEN") == "false" &&
            visual_object_property(lock_screen_path, "other-guid", "LOCKSCREEN") == "true",
        "#1085: host object lock-screen assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_lock_screen(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--lock-screen-target-unique-id", "one-guid",
            "--lock-screen-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1085: missing-target host object lock-screen assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LOCKSCREEN") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "LOCKSCREEN") == "true",
        "#1085: missing-target host object lock-screen assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_lock_screen(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1085: lock-screen-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: lock-screen-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_lock_screen(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--lock-screen-object",
            "--lock-screen-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1085: lock-screen-object without lock-screen value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: lock-screen-object without lock-screen value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_lock_screen(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--lock-screen-target-unique-id", "one-guid",
            "--lock-screen-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1085: duplicate-target host object lock-screen assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: duplicate-target host object lock-screen assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_lock_screen(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--lock-screen-object",
            "--auto-size-object",
            "--lock-screen", "false",
            "--lock-screen-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1085: lock-screen-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: lock-screen-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_split_bar_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_split_bar_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path split_bar_path = temp_root / "split_bar.scx";
    write_synthetic_form_table_for_object_split_bar(split_bar_path);
    const auto split_bar_process = run_process_capture(
        studio_host_path,
        {
            "--path", split_bar_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--split-bar-target-object-name", "frmCustomer",
            "--split-bar-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(split_bar_process.exit_code == 0,
        "#1089: host object split-bar assignment should exit successfully");
    expect(visual_object_property(split_bar_path, "one-guid", "SPLITBAR") == "false" &&
            visual_object_property(split_bar_path, "two-guid", "SPLITBAR") == "false" &&
            visual_object_property(split_bar_path, "three-guid", "SPLITBAR") == "false" &&
            visual_object_property(split_bar_path, "other-guid", "SPLITBAR") == "true",
        "#1089: host object split-bar assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_split_bar(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--split-bar-target-unique-id", "one-guid",
            "--split-bar-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1089: missing-target host object split-bar assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SPLITBAR") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "SPLITBAR") == "true",
        "#1089: missing-target host object split-bar assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_split_bar(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1089: split-bar-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SPLITBAR") == "true",
        "#1089: split-bar-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_split_bar(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--split-bar-object",
            "--split-bar-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1089: split-bar-object without split-bar value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SPLITBAR") == "true",
        "#1089: split-bar-object without split-bar value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_split_bar(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--split-bar-target-unique-id", "one-guid",
            "--split-bar-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1089: duplicate-target host object split-bar assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SPLITBAR") == "true",
        "#1089: duplicate-target host object split-bar assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_split_bar(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--split-bar-object",
            "--auto-size-object",
            "--split-bar", "false",
            "--split-bar-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1089: split-bar-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SPLITBAR") == "true",
        "#1089: split-bar-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_panel_link_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_panel_link_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path panel_link_path = temp_root / "panel_link.scx";
    write_synthetic_form_table_for_object_panel_link(panel_link_path);
    const auto panel_link_process = run_process_capture(
        studio_host_path,
        {
            "--path", panel_link_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--panel-link-target-object-name", "frmCustomer",
            "--panel-link-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(panel_link_process.exit_code == 0,
        "#1091: host object panel-link assignment should exit successfully");
    expect(visual_object_property(panel_link_path, "one-guid", "PANELLINK") == "false" &&
            visual_object_property(panel_link_path, "two-guid", "PANELLINK") == "false" &&
            visual_object_property(panel_link_path, "three-guid", "PANELLINK") == "false" &&
            visual_object_property(panel_link_path, "other-guid", "PANELLINK") == "true",
        "#1091: host object panel-link assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_panel_link(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--panel-link-target-unique-id", "one-guid",
            "--panel-link-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1091: missing-target host object panel-link assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PANELLINK") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "PANELLINK") == "true",
        "#1091: missing-target host object panel-link assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_panel_link(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1091: panel-link-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PANELLINK") == "true",
        "#1091: panel-link-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_panel_link(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--panel-link-object",
            "--panel-link-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1091: panel-link-object without panel-link value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PANELLINK") == "true",
        "#1091: panel-link-object without panel-link value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_panel_link(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--panel-link-target-unique-id", "one-guid",
            "--panel-link-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1091: duplicate-target host object panel-link assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PANELLINK") == "true",
        "#1091: duplicate-target host object panel-link assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_panel_link(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--panel-link-object",
            "--auto-size-object",
            "--panel-link", "false",
            "--panel-link-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1091: panel-link-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PANELLINK") == "true",
        "#1091: panel-link-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_resizable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_resizable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path resizable_path = temp_root / "resizable.scx";
    write_synthetic_form_table_for_object_resizable(resizable_path);
    const auto resizable_process = run_process_capture(
        studio_host_path,
        {
            "--path", resizable_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--resizable-target-object-name", "frmCustomer",
            "--resizable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(resizable_process.exit_code == 0,
        "#1094: host object resizable assignment should exit successfully");
    expect(visual_object_property(resizable_path, "one-guid", "RESIZABLE") == "false" &&
            visual_object_property(resizable_path, "two-guid", "RESIZABLE") == "false" &&
            visual_object_property(resizable_path, "three-guid", "RESIZABLE") == "false" &&
            visual_object_property(resizable_path, "other-guid", "RESIZABLE") == "true",
        "#1094: host object resizable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_resizable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--resizable-target-unique-id", "one-guid",
            "--resizable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1094: missing-target host object resizable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "RESIZABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "RESIZABLE") == "true",
        "#1094: missing-target host object resizable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_resizable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1094: resizable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "RESIZABLE") == "true",
        "#1094: resizable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_resizable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--resizable-object",
            "--resizable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1094: resizable-object without resizable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "RESIZABLE") == "true",
        "#1094: resizable-object without resizable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_resizable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--resizable-target-unique-id", "one-guid",
            "--resizable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1094: duplicate-target host object resizable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "RESIZABLE") == "true",
        "#1094: duplicate-target host object resizable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_resizable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--resizable-object",
            "--auto-size-object",
            "--resizable", "false",
            "--resizable-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1094: resizable-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "RESIZABLE") == "true",
        "#1094: resizable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_always_on_top_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_always_on_top_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path always_on_top_path = temp_root / "always_on_top.scx";
    write_synthetic_form_table_for_object_always_on_top(always_on_top_path);
    const auto always_on_top_process = run_process_capture(
        studio_host_path,
        {
            "--path", always_on_top_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--always-on-top-target-object-name", "frmCustomer",
            "--always-on-top-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(always_on_top_process.exit_code == 0,
        "#1096: host object always-on-top assignment should exit successfully");
    expect(visual_object_property(always_on_top_path, "one-guid", "ALWAYSONTOP") == "false" &&
            visual_object_property(always_on_top_path, "two-guid", "ALWAYSONTOP") == "false" &&
            visual_object_property(always_on_top_path, "three-guid", "ALWAYSONTOP") == "false" &&
            visual_object_property(always_on_top_path, "other-guid", "ALWAYSONTOP") == "true",
        "#1096: host object always-on-top assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_always_on_top(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--always-on-top-target-unique-id", "one-guid",
            "--always-on-top-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1096: missing-target host object always-on-top assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALWAYSONTOP") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALWAYSONTOP") == "true",
        "#1096: missing-target host object always-on-top assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_always_on_top(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1096: always-on-top-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: always-on-top-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_always_on_top(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--always-on-top-object",
            "--always-on-top-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1096: always-on-top-object without always-on-top value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: always-on-top-object without always-on-top value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_always_on_top(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--always-on-top-target-unique-id", "one-guid",
            "--always-on-top-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1096: duplicate-target host object always-on-top assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: duplicate-target host object always-on-top assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_always_on_top(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--always-on-top-object",
            "--auto-size-object",
            "--always-on-top", "false",
            "--always-on-top-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1096: always-on-top-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: always-on-top-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_always_on_bottom_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_always_on_bottom_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path always_on_bottom_path = temp_root / "always_on_bottom.scx";
    write_synthetic_form_table_for_object_always_on_bottom(always_on_bottom_path);
    const auto always_on_bottom_process = run_process_capture(
        studio_host_path,
        {
            "--path", always_on_bottom_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-object-name", "frmCustomer",
            "--always-on-bottom-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(always_on_bottom_process.exit_code == 0,
        "#1097: host object always-on-bottom assignment should exit successfully");
    expect(visual_object_property(always_on_bottom_path, "one-guid", "ALWAYSONBOTTOM") == "false" &&
            visual_object_property(always_on_bottom_path, "two-guid", "ALWAYSONBOTTOM") == "false" &&
            visual_object_property(always_on_bottom_path, "three-guid", "ALWAYSONBOTTOM") == "false" &&
            visual_object_property(always_on_bottom_path, "other-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: host object always-on-bottom assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_always_on_bottom(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--always-on-bottom-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1097: missing-target host object always-on-bottom assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALWAYSONBOTTOM") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: missing-target host object always-on-bottom assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_always_on_bottom(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1097: always-on-bottom-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: always-on-bottom-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_always_on_bottom(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1097: always-on-bottom-object without always-on-bottom value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: always-on-bottom-object without always-on-bottom value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_always_on_bottom(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--always-on-bottom-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1097: duplicate-target host object always-on-bottom assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: duplicate-target host object always-on-bottom assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_always_on_bottom(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--always-on-bottom-object",
            "--auto-size-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1097: always-on-bottom-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: always-on-bottom-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
