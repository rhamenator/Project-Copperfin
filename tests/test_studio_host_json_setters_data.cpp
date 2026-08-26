// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_button_count(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BUTTONCOUN", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1110: synthetic SCX table for object button-count should be created");
}

void write_synthetic_form_table_for_object_data_session(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DATASESSIO", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1119: synthetic SCX table for object data-session should be created");
}

void write_synthetic_form_table_for_object_lock_columns(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LOCKCOLUMN", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1123: synthetic SCX table for object lock-columns should be created");
}

void write_synthetic_form_table_for_object_lock_columns_left(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LOCKCOLUMN", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1124: synthetic SCX table for object lock-columns-left should be created");
}

void write_synthetic_form_table_for_object_partition(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARTITION", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1128: synthetic SCX table for object partition should be created");
}

void write_synthetic_form_table_for_object_record_source_type(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "RECORDSOUR", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1129: synthetic SCX table for object record-source-type should be created");
}

void write_synthetic_form_table_for_object_column_order(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "COLUMNORDE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1131: synthetic SCX table for object column-order should be created");
}

void write_synthetic_form_table_for_object_child_order(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CHILDORDER", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1133: synthetic SCX table for object child-order should be created");
}

void write_synthetic_form_table_for_object_record_source(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "RECORDSOUR", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "Save"},
        {"cmdCancel", "cmdCancel", "two-guid", "Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cmdOther", "cmdOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1130: synthetic SCX table for object record source should be created");
}

void write_synthetic_form_table_for_object_form_set_class(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FORMSETCLA", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSaveFormSet", "cmdSaveFormSet", "one-guid", "SaveFormSet"},
        {"cmdCancelFormSet", "cmdCancelFormSet", "two-guid", "CancelFormSet"},
        {"lblStatus", "lblStatus", "three-guid", "StatusFormSet"},
        {"cmdOtherFormSet", "cmdOtherFormSet", "other-guid", "OtherFormSet"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1136: synthetic SCX table for object form set class should be created");
}

void write_synthetic_form_table_for_object_default_file_path(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DEFAULTFIL", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdDefaultPath", "cmdDefaultPath", "one-guid", "Data\\Save"},
        {"cmdCancelPath", "cmdCancelPath", "two-guid", "Data\\Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Data\\Status"},
        {"cmdOtherPath", "cmdOtherPath", "other-guid", "Data\\Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1137: synthetic SCX table for object default file path should be created");
}

void write_synthetic_form_table_for_object_initial_selected_alias(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "INITIALSEL", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdInitialAlias", "cmdInitialAlias", "one-guid", "orders"},
        {"cmdCancelAlias", "cmdCancelAlias", "two-guid", "payments"},
        {"lblStatus", "lblStatus", "three-guid", "status_alias"},
        {"cmdOtherAlias", "cmdOtherAlias", "other-guid", "other_alias"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1138: synthetic SCX table for object initial selected alias should be created");
}

void write_synthetic_form_table_for_object_link_master(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LINKMASTER", .type = 'C', .length = 70U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "old_customer_id"},
        {"cmdCancel", "cmdCancel", "two-guid", "old_order_id"},
        {"lblStatus", "lblStatus", "three-guid", "status_id"},
        {"cmdOther", "cmdOther", "other-guid", "other_id"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1165: synthetic SCX table for object link-master should be created");
}

void write_synthetic_form_table_for_object_row_source(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ROWSOURCE", .type = 'C', .length = 80U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "customers.name,customer_id"},
        {"lstOrders", "lstOrders", "two-guid", "orders.order_id,total"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cboOther", "cboOther", "other-guid", "states.name"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1048: synthetic SCX table for object row source should be created");
}

void write_synthetic_form_table_for_object_column_widths(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "COLUMNWIDT", .type = 'C', .length = 80U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "75,125"},
        {"lstOrders", "lstOrders", "two-guid", "60,80,100"},
        {"lblStatus", "lblStatus", "three-guid", "20"},
        {"cboOther", "cboOther", "other-guid", "90,90"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1196: synthetic SCX table for object column widths should be created");
}

void write_synthetic_form_table_for_object_column_lines(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "COLUMNLINE", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", ".F."},
        {"lstOrders", "lstOrders", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".F."},
        {"cboOther", "cboOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1197: synthetic SCX table for object column lines should be created");
}

void write_synthetic_form_table_for_object_integral_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "INTEGRALHE", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", ".F."},
        {"lstOrders", "lstOrders", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".F."},
        {"cboOther", "cboOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1198: synthetic SCX table for object integral height should be created");
}

void write_synthetic_form_table_for_object_incremental_search(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "INCREMENTA", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", ".F."},
        {"lstOrders", "lstOrders", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".F."},
        {"cboOther", "cboOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1199: synthetic SCX table for object incremental search should be created");
}

void write_synthetic_form_table_for_object_multi_select(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MULTISELEC", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", ".F."},
        {"lstOrders", "lstOrders", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".F."},
        {"cboOther", "cboOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1200: synthetic SCX table for object multi select should be created");
}

void write_synthetic_form_table_for_object_row_source_type(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ROWSOURCET", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "2"},
        {"lstOrders", "lstOrders", "two-guid", "3"},
        {"lblStatus", "lblStatus", "three-guid", "0"},
        {"cboOther", "cboOther", "other-guid", "5"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1049: synthetic SCX table for object row source type should be created");
}

void write_synthetic_form_table_for_object_bound_column(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BOUNDCOLUM", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "1"},
        {"lstOrders", "lstOrders", "two-guid", "2"},
        {"lblStatus", "lblStatus", "three-guid", "0"},
        {"cboOther", "cboOther", "other-guid", "3"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1050: synthetic SCX table for object bound column should be created");
}

void write_synthetic_form_table_for_object_column_count(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "COLUMNCOUN", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "2"},
        {"lstOrders", "lstOrders", "two-guid", "3"},
        {"lblStatus", "lblStatus", "three-guid", "0"},
        {"cboOther", "cboOther", "other-guid", "4"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1051: synthetic SCX table for object column count should be created");
}

void write_synthetic_form_table_for_object_list_index(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LISTINDEX", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "0"},
        {"cboOther", "cboOther", "other-guid", "2"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1053: synthetic SCX table for object list index should be created");
}

void write_synthetic_form_table_for_object_left_column(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LEFTCOLUMN", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"grdCustomer", "grdCustomer", "one-guid", "0"},
        {"grdOrders", "grdOrders", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "0"},
        {"grdOther", "grdOther", "other-guid", "2"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1054: synthetic SCX table for object left column should be created");
}

void write_synthetic_form_table_for_object_display_value(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DISPLAYVAL", .type = 'C', .length = 80U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "Alice"},
        {"lstOrders", "lstOrders", "two-guid", "Order 100"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cboOther", "cboOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1055: synthetic SCX table for object display value should be created");
}

#include "test_studio_host_json_setters_data_grid_selectors.inl"
#include "test_studio_host_json_setters_data_stable_selectors.inl"

#include "test_studio_host_json_setters_data_remaining_selectors.inl"

void test_studio_host_json_assigns_column_widths_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_column_widths_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path column_widths_path = temp_root / "column_widths.scx";
    write_synthetic_form_table_for_object_column_widths(column_widths_path);
    const auto column_widths_process = run_process_capture(
        studio_host_path,
        {
            "--path", column_widths_path.string(),
            "--column-widths-object",
            "--column-widths", "40,90,120",
            "--column-widths-target-object-name", "cboCustomer",
            "--column-widths-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(column_widths_process.exit_code == 0,
        "#1196: host object column-widths assignment should exit successfully");
    expect(visual_object_property(column_widths_path, "one-guid", "COLUMNWIDTHS") == "40,90,120" &&
            visual_object_property(column_widths_path, "two-guid", "COLUMNWIDTHS") == "40,90,120" &&
            visual_object_property(column_widths_path, "three-guid", "COLUMNWIDTHS") == "20" &&
            visual_object_property(column_widths_path, "other-guid", "COLUMNWIDTHS") == "90,90",
        "#1196: host object column-widths assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_column_widths(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--column-widths-object",
            "--column-widths", "40,90,120",
            "--column-widths-target-unique-id", "one-guid",
            "--column-widths-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1196: missing-target host object column-widths assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "COLUMNWIDTHS") == "75,125" &&
            visual_object_property(missing_target_path, "two-guid", "COLUMNWIDTHS") == "60,80,100",
        "#1196: missing-target host object column-widths assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_column_widths(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--column-widths-object",
            "--column-widths", "40,90,120",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1196: column-widths-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "COLUMNWIDTHS") == "75,125",
        "#1196: column-widths-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_column_widths(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--column-widths-object",
            "--column-widths-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1196: column-widths-object without column-widths value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "COLUMNWIDTHS") == "75,125",
        "#1196: column-widths-object without column-widths value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_column_widths(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--column-widths-object",
            "--column-widths", "40,90,120",
            "--column-widths-target-unique-id", "one-guid",
            "--column-widths-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1196: duplicate-target host object column-widths assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "COLUMNWIDTHS") == "75,125",
        "#1196: duplicate-target host object column-widths assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_column_widths(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--column-widths-object",
            "--row-source-object",
            "--column-widths", "40,90,120",
            "--column-widths-target-unique-id", "one-guid",
            "--row-source", "products.name",
            "--row-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1196: column-widths-object plus row-source-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "COLUMNWIDTHS") == "75,125",
        "#1196: column-widths-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_column_lines_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_column_lines_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path column_lines_path = temp_root / "column_lines.scx";
    write_synthetic_form_table_for_object_column_lines(column_lines_path);
    const auto column_lines_process = run_process_capture(
        studio_host_path,
        {
            "--path", column_lines_path.string(),
            "--column-lines-object",
            "--column-lines", "true",
            "--column-lines-target-object-name", "cboCustomer",
            "--column-lines-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(column_lines_process.exit_code == 0,
        "#1197: host object column-lines assignment should exit successfully");
    expect(visual_object_property(column_lines_path, "one-guid", "COLUMNLINES") == ".T." &&
            visual_object_property(column_lines_path, "two-guid", "COLUMNLINES") == ".T." &&
            visual_object_property(column_lines_path, "three-guid", "COLUMNLINES") == ".F." &&
            visual_object_property(column_lines_path, "other-guid", "COLUMNLINES") == ".T.",
        "#1197: host object column-lines assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_column_lines(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--column-lines-object",
            "--column-lines", "true",
            "--column-lines-target-unique-id", "one-guid",
            "--column-lines-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1197: missing-target host object column-lines assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "COLUMNLINES") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "COLUMNLINES") == ".F.",
        "#1197: missing-target host object column-lines assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_column_lines(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--column-lines-object",
            "--column-lines", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1197: column-lines-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "COLUMNLINES") == ".F.",
        "#1197: column-lines-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_column_lines(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--column-lines-object",
            "--column-lines-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1197: column-lines-object without column-lines value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "COLUMNLINES") == ".F.",
        "#1197: column-lines-object without column-lines value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_column_lines(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--column-lines-object",
            "--column-lines", "sometimes",
            "--column-lines-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1197: invalid column-lines values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "COLUMNLINES") == ".F.",
        "#1197: invalid column-lines values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_column_lines(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--column-lines-object",
            "--column-lines", "true",
            "--column-lines-target-unique-id", "one-guid",
            "--column-lines-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1197: duplicate-target host object column-lines assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "COLUMNLINES") == ".F.",
        "#1197: duplicate-target host object column-lines assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_column_lines(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--column-lines-object",
            "--row-source-type-object",
            "--column-lines", "true",
            "--column-lines-target-unique-id", "one-guid",
            "--row-source-type", "2",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1197: column-lines-object plus row-source-type-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "COLUMNLINES") == ".F.",
        "#1197: column-lines-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_integral_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_integral_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path integral_height_path = temp_root / "integral_height.scx";
    write_synthetic_form_table_for_object_integral_height(integral_height_path);
    const auto integral_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", integral_height_path.string(),
            "--integral-height-object",
            "--integral-height", "true",
            "--integral-height-target-object-name", "cboCustomer",
            "--integral-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(integral_height_process.exit_code == 0,
        "#1198: host object integral-height assignment should exit successfully");
    expect(visual_object_property(integral_height_path, "one-guid", "INTEGRALHEIGHT") == ".T." &&
            visual_object_property(integral_height_path, "two-guid", "INTEGRALHEIGHT") == ".T." &&
            visual_object_property(integral_height_path, "three-guid", "INTEGRALHEIGHT") == ".F." &&
            visual_object_property(integral_height_path, "other-guid", "INTEGRALHEIGHT") == ".T.",
        "#1198: host object integral-height assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_integral_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--integral-height-object",
            "--integral-height", "true",
            "--integral-height-target-unique-id", "one-guid",
            "--integral-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1198: missing-target host object integral-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "INTEGRALHEIGHT") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "INTEGRALHEIGHT") == ".F.",
        "#1198: missing-target host object integral-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_integral_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--integral-height-object",
            "--integral-height", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1198: integral-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "INTEGRALHEIGHT") == ".F.",
        "#1198: integral-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_integral_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--integral-height-object",
            "--integral-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1198: integral-height-object without integral-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "INTEGRALHEIGHT") == ".F.",
        "#1198: integral-height-object without integral-height value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_integral_height(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--integral-height-object",
            "--integral-height", "sometimes",
            "--integral-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1198: invalid integral-height values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "INTEGRALHEIGHT") == ".F.",
        "#1198: invalid integral-height values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_integral_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--integral-height-object",
            "--integral-height", "true",
            "--integral-height-target-unique-id", "one-guid",
            "--integral-height-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1198: duplicate-target host object integral-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "INTEGRALHEIGHT") == ".F.",
        "#1198: duplicate-target host object integral-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_integral_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--integral-height-object",
            "--row-source-type-object",
            "--integral-height", "true",
            "--integral-height-target-unique-id", "one-guid",
            "--row-source-type", "2",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1198: integral-height-object plus row-source-type-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "INTEGRALHEIGHT") == ".F.",
        "#1198: integral-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_incremental_search_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_incremental_search_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path incremental_search_path = temp_root / "incremental_search.scx";
    write_synthetic_form_table_for_object_incremental_search(incremental_search_path);
    const auto incremental_search_process = run_process_capture(
        studio_host_path,
        {
            "--path", incremental_search_path.string(),
            "--incremental-search-object",
            "--incremental-search", "true",
            "--incremental-search-target-object-name", "cboCustomer",
            "--incremental-search-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(incremental_search_process.exit_code == 0,
        "#1199: host object incremental-search assignment should exit successfully");
    expect(visual_object_property(incremental_search_path, "one-guid", "INCREMENTALSEARCH") == ".T." &&
            visual_object_property(incremental_search_path, "two-guid", "INCREMENTALSEARCH") == ".T." &&
            visual_object_property(incremental_search_path, "three-guid", "INCREMENTALSEARCH") == ".F." &&
            visual_object_property(incremental_search_path, "other-guid", "INCREMENTALSEARCH") == ".T.",
        "#1199: host object incremental-search assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_incremental_search(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--incremental-search-object",
            "--incremental-search", "true",
            "--incremental-search-target-unique-id", "one-guid",
            "--incremental-search-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1199: missing-target host object incremental-search assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "INCREMENTALSEARCH") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "INCREMENTALSEARCH") == ".F.",
        "#1199: missing-target host object incremental-search assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_incremental_search(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--incremental-search-object",
            "--incremental-search", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1199: incremental-search-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "INCREMENTALSEARCH") == ".F.",
        "#1199: incremental-search-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_incremental_search(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--incremental-search-object",
            "--incremental-search-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1199: incremental-search-object without incremental-search value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "INCREMENTALSEARCH") == ".F.",
        "#1199: incremental-search-object without incremental-search value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_incremental_search(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--incremental-search-object",
            "--incremental-search", "sometimes",
            "--incremental-search-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1199: invalid incremental-search values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "INCREMENTALSEARCH") == ".F.",
        "#1199: invalid incremental-search values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_incremental_search(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--incremental-search-object",
            "--incremental-search", "true",
            "--incremental-search-target-unique-id", "one-guid",
            "--incremental-search-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1199: duplicate-target host object incremental-search assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "INCREMENTALSEARCH") == ".F.",
        "#1199: duplicate-target host object incremental-search assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_incremental_search(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--incremental-search-object",
            "--row-source-type-object",
            "--incremental-search", "true",
            "--incremental-search-target-unique-id", "one-guid",
            "--row-source-type", "2",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1199: incremental-search-object plus row-source-type-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "INCREMENTALSEARCH") == ".F.",
        "#1199: incremental-search-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_multi_select_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_multi_select_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path multi_select_path = temp_root / "multi_select.scx";
    write_synthetic_form_table_for_object_multi_select(multi_select_path);
    const auto multi_select_process = run_process_capture(
        studio_host_path,
        {
            "--path", multi_select_path.string(),
            "--multi-select-object",
            "--multi-select", "true",
            "--multi-select-target-object-name", "cboCustomer",
            "--multi-select-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(multi_select_process.exit_code == 0,
        "#1200: host object multi-select assignment should exit successfully");
    expect(visual_object_property(multi_select_path, "one-guid", "MULTISELECT") == ".T." &&
            visual_object_property(multi_select_path, "two-guid", "MULTISELECT") == ".T." &&
            visual_object_property(multi_select_path, "three-guid", "MULTISELECT") == ".F." &&
            visual_object_property(multi_select_path, "other-guid", "MULTISELECT") == ".T.",
        "#1200: host object multi-select assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_multi_select(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--multi-select-object",
            "--multi-select", "true",
            "--multi-select-target-unique-id", "one-guid",
            "--multi-select-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1200: missing-target host object multi-select assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MULTISELECT") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "MULTISELECT") == ".F.",
        "#1200: missing-target host object multi-select assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_multi_select(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--multi-select-object",
            "--multi-select", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1200: multi-select-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MULTISELECT") == ".F.",
        "#1200: multi-select-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_multi_select(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--multi-select-object",
            "--multi-select-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1200: multi-select-object without multi-select value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MULTISELECT") == ".F.",
        "#1200: multi-select-object without multi-select value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_multi_select(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--multi-select-object",
            "--multi-select", "sometimes",
            "--multi-select-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1200: invalid multi-select values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "MULTISELECT") == ".F.",
        "#1200: invalid multi-select values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_multi_select(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--multi-select-object",
            "--multi-select", "true",
            "--multi-select-target-unique-id", "one-guid",
            "--multi-select-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1200: duplicate-target host object multi-select assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MULTISELECT") == ".F.",
        "#1200: duplicate-target host object multi-select assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_multi_select(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--multi-select-object",
            "--row-source-type-object",
            "--multi-select", "true",
            "--multi-select-target-unique-id", "one-guid",
            "--row-source-type", "2",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1200: multi-select-object plus row-source-type-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MULTISELECT") == ".F.",
        "#1200: multi-select-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
