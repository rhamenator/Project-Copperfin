// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"
#include "test_process_capture_support.h"

namespace cf_test_studio_host_json {
int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

void expect_not_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) == std::string::npos, message);
}

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }

    return missing;
}

void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message) {
    std::size_t offset = 0U;
    for (const auto& needle : needles) {
        const std::size_t position = text.find(needle, offset);
        if (position == std::string::npos) {
            expect(false, message);
            return;
        }
        offset = position + needle.size();
    }
}

std::string expected_json_shell_quote(const std::string& value) {
#if defined(_WIN32)
    std::string quoted = "\\\"";
    quoted.reserve(value.size() + 4U);
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"\\\"";
        } else if (ch == '%') {
            quoted += "%%";
        } else if (ch == '\\') {
            quoted += "\\\\";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted += "\\\"";
    return quoted;
#else
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('\'');
    return quoted;
#endif
}

std::string expected_json_shell_command(
    const std::string& launch_command,
    std::initializer_list<std::string> arguments) {
    std::string command = expected_json_shell_quote(launch_command);
    for (const auto& argument : arguments) {
        command.push_back(' ');
        command += expected_json_shell_quote(argument);
    }
    return command;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string normalize_captured_line_endings(const std::string_view text) {
    return copperfin::test_support::normalize_captured_line_endings(text);
}

void test_captured_process_output_line_endings_normalize(const std::string&) {
    expect(normalize_captured_line_endings("") == "",
           "#4080: empty captured output should remain empty");
    expect(normalize_captured_line_endings("alpha\nbeta\n") == "alpha\nbeta\n",
           "#4080: LF captured output should remain unchanged");
    expect(normalize_captured_line_endings("alpha\r\nbeta\r\n") == "alpha\nbeta\n",
           "#4080: CRLF captured output should normalize to LF");
    expect(normalize_captured_line_endings("alpha\rbeta\r") == "alpha\nbeta\n",
           "#4080: lone CR captured output should normalize to LF");

    std::string mixed_input = "alpha\r\nbeta\rgamma\n";
    mixed_input.push_back('\0');
    mixed_input += "delta";
    std::string mixed_expected = "alpha\nbeta\ngamma\n";
    mixed_expected.push_back('\0');
    mixed_expected += "delta";
    expect(normalize_captured_line_endings(mixed_input) == mixed_expected,
           "#4080: mixed captured output should preserve non-line-ending bytes");
    expect(normalize_captured_line_endings("unterminated\r\nline") == "unterminated\nline",
           "#4080: normalization should not add a final line ending");
}

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    // Source-tree Studio tests own checkout catalogs; installed-layout probes use separate launchers.
    copperfin::test_support::ScopedTestLocaleCatalogDirectory locale_catalog_directory;
    const auto captured = copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            copperfin::test_support::path_from_utf8_string(executable_path),
            arguments,
            working_directory));
    return {
        .exit_code = captured.exit_code,
        .stdout_text = captured.stdout_text,
        .stderr_text = captured.stderr_text
    };
}

std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x01U;
    bytes[29] = 0x03U;
    return bytes;
}

void write_synthetic_form_asset(const std::filesystem::path& form_path) {
    const auto bytes = make_vfp_header();
    std::ofstream output(form_path, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_synthetic_form_table_with_objects(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "PLATFORM", .type = 'C', .length = 12U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"2", "0", "WINDOWS", "Dataenvironment", "de-1", "", "", "dataenvironment"},
        {"1", "0", "WINDOWS", "frmCustomer", "form-1", "", "customerform", "form"},
        {"4", "2", "WINDOWS", "cmdSave", "button-1", "frmCustomer", "commandbutton", "commandbutton"},
        {"4", "1", "WINDOWS", "txtName", "textbox-1", "frmCustomer", "textbox", "textbox"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#967: synthetic SCX table with selectable objects should be created");
}

void write_synthetic_form_table_with_invalid_raw_codes(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'C', .length = 48U},
        {.name = "OBJCODE", .type = 'C', .length = 48U},
        {.name = "PLATFORM", .type = 'C', .length = 12U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"type?", "code?", "WINDOWS", "cmdMalformed", "malformed-guid", "", "commandbutton", "commandbutton"},
        {
            "999999999999999999999999999999",
            "-999999999999999999999999999999",
            "WINDOWS",
            "cmdOversized",
            "oversized-guid",
            "",
            "commandbutton",
            "commandbutton"
        },
        {".5", ".7", "WINDOWS", "cmdDotLeading", "dot-leading-guid", "", "commandbutton", "commandbutton"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1749: synthetic SCX table with invalid raw codes should be created");
}

void write_synthetic_form_table_for_toolbox_creation(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "CAPTION", .type = 'C', .length = 32U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", "Customer", ""},
        {"txt1", "txt1", "existing-textbox-guid", "frmCustomer", "TextBox", "TextBox", "Existing", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1018: synthetic SCX table for toolbox creation should be created");
}

void write_synthetic_form_table_for_visual_object_list(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "Page1",
            "pageOne",
            "page-guid",
            "",
            "pageframe",
            "Page",
            "Caption = \"Page\"\r\nWidth = 200\r\n",
            "PROCEDURE Activate\r\nRETURN\r\n"
        },
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "Page1",
            "cmdButton",
            "CommandButton",
            "Caption = \"Save\"\r\n",
            "PROCEDURE Click\r\nRETURN\r\nFUNCTION CanSave\r\nRETURN .T.\r\n"
        },
        {
            "",
            "fallbackButton",
            "fallback-guid",
            "Page1",
            "commandButton",
            "CommandButton",
            "Caption = \"Fallback\"\r\n",
            ""
        },
        {
            "lblNested",
            "nestedLabel",
            "nested-guid",
            "cmdSave",
            "label",
            "Label",
            "Caption = \"Nested\"\r\n",
            ""
        }
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1418: synthetic SCX table for visual-object list should be created");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "fallback-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#1418: synthetic SCX table should allow marking fallback object deleted");
}

std::size_t visual_object_count(const std::filesystem::path& form_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(form_path.string());
    return list_result.ok ? list_result.objects.size() : 0U;
}

bool visual_object_deleted(const std::filesystem::path& form_path, const std::string& unique_id) {
    const auto list_result = copperfin::vfp::list_visual_objects(form_path.string());
    if (!list_result.ok) {
        return false;
    }
    for (const auto& object : list_result.objects) {
        if (object.unique_id == unique_id) {
            return object.deleted;
        }
    }
    return false;
}

bool visual_object_exists(const std::filesystem::path& form_path, const std::string& unique_id) {
    const auto list_result = copperfin::vfp::list_visual_objects(form_path.string());
    if (!list_result.ok) {
        return false;
    }
    for (const auto& object : list_result.objects) {
        if (object.unique_id == unique_id) {
            return true;
        }
    }
    return false;
}

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
}

bool visual_object_is_deleted(const std::filesystem::path& form_path, const std::string& unique_id) {
    const auto list_result = copperfin::vfp::list_visual_objects(form_path.string());
    if (!list_result.ok) {
        return false;
    }
    for (const auto& object : list_result.objects) {
        if (object.unique_id == unique_id) {
            return object.deleted;
        }
    }
    return false;
}

std::string visual_object_parent(const std::filesystem::path& form_path, const std::string& unique_id) {
    const auto list_result = copperfin::vfp::list_visual_objects(form_path.string());
    if (!list_result.ok) {
        return {};
    }
    for (const auto& object : list_result.objects) {
        if (object.unique_id == unique_id) {
            return object.parent_name;
        }
    }
    return {};
}

std::string visual_object_property(
    const std::filesystem::path& form_path,
    const std::string& unique_id,
    const std::string& property_name) {
    const auto result = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = property_name
    });
    if (!result.ok || !result.exists) {
        return {};
    }
    return result.value;
}

std::string visual_object_property_order(
    const std::filesystem::path& form_path,
    const std::string& unique_id) {
    const auto result = copperfin::vfp::list_visual_object_properties({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id
    });
    if (!result.ok) {
        return {};
    }

    std::string order;
    for (const auto& property : result.properties) {
        if (property.direct_field) {
            continue;
        }
        if (!order.empty()) {
            order += ",";
        }
        order += property.property_name;
    }
    return order;
}

std::string visual_object_order(const std::filesystem::path& form_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(form_path.string());
    if (!list_result.ok) {
        return {};
    }
    std::string value;
    for (const auto& object : list_result.objects) {
        if (!value.empty()) {
            value += ",";
        }
        value += object.unique_id;
    }
    return value;
}

void delete_existing_textbox(const std::filesystem::path& form_path, const std::string& evidence) {
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .deleted = true
    });
    expect(delete_result.ok, evidence);
}

void write_synthetic_form_table_with_container_object(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "PLATFORM", .type = 'C', .length = 12U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "0", "WINDOWS", "frmCustomer", "form-1", "", "customerform", "form"},
        {"4", "0", "WINDOWS", "pgfMain", "pageframe-1", "frmCustomer", "pageframe", "pageframe"},
        {"4", "0", "WINDOWS", "grdOrders", "grid-1", "frmCustomer", "grid", "grid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1015: synthetic SCX table with selectable container objects should be created");
}

void write_synthetic_table_with_data_environment(const std::filesystem::path& asset_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"Dataenvironment", "dataenvironment", ""},
        {"DetailExpression", "field", "field"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
    expect(create_result.ok, "#1016: synthetic table with DataEnvironment record should be created");
}

}  // namespace cf_test_studio_host_json
