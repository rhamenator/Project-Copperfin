#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

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

std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    quoted.reserve(value.size() + 2U);
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('"');
    return quoted;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    namespace fs = std::filesystem;

    const fs::path resolved_executable_path = fs::absolute(executable_path);
    const fs::path stdout_path = working_directory / "studio_host_stdout.log";
    const fs::path stderr_path = working_directory / "studio_host_stderr.log";

    std::string command = quote_command_argument(resolved_executable_path.string());
    for (const auto& argument : arguments) {
        command += " ";
        command += quote_command_argument(argument);
    }
    command += " > ";
    command += quote_command_argument(stdout_path.string());
    command += " 2> ";
    command += quote_command_argument(stderr_path.string());

    const fs::path original_directory = fs::current_path();
    fs::current_path(working_directory);
    const int raw_exit_code = std::system(command.c_str());
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
    }

#if defined(_WIN32)
    result.exit_code = raw_exit_code;
#else
    if (raw_exit_code != -1 && WIFEXITED(raw_exit_code)) {
        result.exit_code = WEXITSTATUS(raw_exit_code);
    } else {
        result.exit_code = raw_exit_code;
    }
#endif
    return result;
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

void write_synthetic_form_table_for_toolbox_creation(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
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

std::filesystem::path write_synthetic_form_table_for_property_rename(
    const std::filesystem::path& temp_root,
    const std::string& file_name) {
    const std::filesystem::path form_path = temp_root / file_name;
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"txt1", "txt1", "existing-textbox-guid", "frmCustomer", "TextBox", "TextBox",
            "ControlSource = \"customer.name\"\r\nLeft = 12\r\n"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1022: synthetic SCX table for property rename should be created");
    return form_path;
}

void write_synthetic_form_table_for_object_reparent(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"cntPanel", "cntPanel", "panel-guid", "frmCustomer", "Container", "Container", ""},
        {"txt1", "txt1", "existing-textbox-guid", "frmCustomer", "TextBox", "TextBox", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1027: synthetic SCX table for object reparent should be created");
}

void write_synthetic_form_table_for_object_reorder(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdA", "cmdA", "a-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"cmdB", "cmdB", "b-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"cmdC", "cmdC", "c-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"cmdD", "cmdD", "d-guid", "frmCustomer", "CommandButton", "CommandButton", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1028: synthetic SCX table for object reorder should be created");
}

void write_synthetic_form_table_for_object_group(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"cmdSave", "cmdSave", "save-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"txtName", "txtName", "name-guid", "frmCustomer", "TextBox", "TextBox", ""},
        {"lblStatus", "lblStatus", "status-guid", "frmCustomer", "Label", "Label", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1030: synthetic SCX table for object group should be created");
}

void write_synthetic_form_table_for_object_align(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "WIDTH", .type = 'C', .length = 10U},
        {.name = "HEIGHT", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdAnchor", "cmdAnchor", "anchor-guid", "10", "20", "100", "50"},
        {"txtName", "txtName", "name-guid", "1", "2", "30", "10"},
        {"lblStatus", "lblStatus", "status-guid", "5", "6", "20", "25"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1031: synthetic SCX table for object alignment should be created");
}

void write_synthetic_form_table_for_object_distribute(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "WIDTH", .type = 'C', .length = 10U},
        {.name = "HEIGHT", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdLeft", "cmdLeft", "left-guid", "10", "10", "20", "10"},
        {"cmdMiddle", "cmdMiddle", "middle-guid", "90", "50", "20", "10"},
        {"cmdRight", "cmdRight", "right-guid", "110", "90", "20", "10"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1033: synthetic SCX table for object distribution should be created");
}

void write_synthetic_form_table_for_object_snap(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", "13.2", "24.9"},
        {"cmdTwo", "cmdTwo", "two-guid", "36", "51"},
        {"cmdOther", "cmdOther", "other-guid", "77", "88"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1034: synthetic SCX table for object snap should be created");
}

void write_synthetic_form_table_for_object_nudge(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", "10", "20"},
        {"cmdTwo", "cmdTwo", "two-guid", "33.5", "44.5"},
        {"cmdOther", "cmdOther", "other-guid", "77", "88"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1035: synthetic SCX table for object nudge should be created");
}

void write_synthetic_form_table_for_object_tab_order(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TABINDEX", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", "10"},
        {"cmdTwo", "cmdTwo", "two-guid", "20"},
        {"cmdThree", "cmdThree", "three-guid", "30"},
        {"cmdOther", "cmdOther", "other-guid", "99"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1036: synthetic SCX table for object tab order should be created");
}

void write_synthetic_form_table_for_object_tab_stop(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TABSTOP", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", ".T."},
        {"cmdTwo", "cmdTwo", "two-guid", ".T."},
        {"cmdThree", "cmdThree", "three-guid", ".F."},
        {"cmdOther", "cmdOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1037: synthetic SCX table for object tab stop should be created");
}

void write_synthetic_form_table_for_object_visibility(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "VISIBLE", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", ".T."},
        {"cmdTwo", "cmdTwo", "two-guid", ".T."},
        {"cmdThree", "cmdThree", "three-guid", ".F."},
        {"cmdOther", "cmdOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1038: synthetic SCX table for object visibility should be created");
}

void write_synthetic_form_table_for_object_enabled(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ENABLED", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", ".T."},
        {"cmdTwo", "cmdTwo", "two-guid", ".T."},
        {"cmdThree", "cmdThree", "three-guid", ".F."},
        {"cmdOther", "cmdOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1039: synthetic SCX table for object enabled state should be created");
}

void write_synthetic_form_table_for_object_read_only(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "READONLY", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "txtOne", "one-guid", ".F."},
        {"txtTwo", "txtTwo", "two-guid", ".F."},
        {"txtThree", "txtThree", "three-guid", ".T."},
        {"txtOther", "txtOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1040: synthetic SCX table for object read-only state should be created");
}

void write_synthetic_form_table_for_object_locked(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LOCKED", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "txtOne", "one-guid", ".F."},
        {"txtTwo", "txtTwo", "two-guid", ".F."},
        {"txtThree", "txtThree", "three-guid", ".T."},
        {"txtOther", "txtOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1041: synthetic SCX table for object locked state should be created");
}

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

void write_synthetic_form_table_for_object_tooltip_text(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TOOLTIPTEXT", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "Save"},
        {"cmdCancel", "cmdCancel", "two-guid", "Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cmdOther", "cmdOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1043: synthetic SCX table for object tooltip text should be created");
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

void write_synthetic_form_table_for_object_control_source(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CONTROLSOURCE", .type = 'C', .length = 70U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", "customers.name"},
        {"txtCity", "txtCity", "two-guid", "customers.city"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"txtOther", "txtOther", "other-guid", "customers.state"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1045: synthetic SCX table for object control source should be created");
}

void write_synthetic_form_table_for_object_input_mask(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "INPUTMASK", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtPhone", "txtPhone", "one-guid", "(999) 999-9999"},
        {"txtZip", "txtZip", "two-guid", "99999"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"txtOther", "txtOther", "other-guid", "XXXXXXXX"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1046: synthetic SCX table for object input mask should be created");
}

void write_synthetic_form_table_for_object_format(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FORMAT", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtAmount", "txtAmount", "one-guid", "999,999.99"},
        {"txtPercent", "txtPercent", "two-guid", "99.99%"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"txtOther", "txtOther", "other-guid", "!"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1047: synthetic SCX table for object format should be created");
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

void write_synthetic_form_table_for_object_row_source_type(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ROWSOURCETYPE", .type = 'C', .length = 10U}
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
        {.name = "BOUNDCOLUMN", .type = 'C', .length = 10U}
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
        {.name = "COLUMNCOUNT", .type = 'C', .length = 10U}
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

void write_synthetic_form_table_for_object_style(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "STYLE", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "0"},
        {"cboOther", "cboOther", "other-guid", "2"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1052: synthetic SCX table for object style should be created");
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
        {.name = "DISPLAYVALUE", .type = 'C', .length = 80U}
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

void write_synthetic_form_table_for_object_ungroup(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"cntGroup", "cntGroup", "group-guid", "frmCustomer", "Container", "Container", ""},
        {"txtName", "txtName", "name-guid", "cntGroup", "TextBox", "TextBox", ""},
        {"cmdSave", "cmdSave", "save-guid", "cntGroup", "CommandButton", "CommandButton", ""},
        {"cntRoot", "cntRoot", "root-group-guid", "", "Container", "Container", ""},
        {"txtRoot", "txtRoot", "root-child-guid", "cntRoot", "TextBox", "TextBox", ""},
        {"cntEmpty", "cntEmpty", "empty-guid", "frmCustomer", "Container", "Container", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1029: synthetic SCX table for object ungroup should be created");
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

void test_studio_host_json_exposes_designer_contexts(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_asset(form_path);

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host stdout:\n" << process.stdout_text << "\n";
        std::cerr << "studio host stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "#961: Studio host JSON smoke should exit successfully");
    expect_contains(process.stdout_text, "\"designerContexts\": [",
                    "#961: document JSON should expose designer context array");
    expect_contains(process.stdout_text, "\"selectionContext\": \"visual_object\"",
                    "#961: form JSON should expose the visual-object context token");
    expect_contains(process.stdout_text, "\"editorActionCount\": 5",
                    "#1009: form JSON should expose designer context editor-action count");
    expect_contains(process.stdout_text, "\"builderCount\": 3",
                    "#1009: form JSON should expose designer context builder count");
    expect_contains(process.stdout_text, "\"toolboxItemCount\": ",
                    "#1009: form JSON should expose designer context toolbox-item count");
    expect_contains(process.stdout_text, "\"editorActions\": [",
                    "#961: designer context JSON should expose editor actions");
    expect_contains(process.stdout_text, "\"id\": \"show-property-grid\"",
                    "#961: designer context JSON should expose property-grid action ids");
    expect_contains(process.stdout_text, "\"builders\": [",
                    "#961: designer context JSON should expose builders");
    expect_contains(process.stdout_text, "\"id\": \"form-builder\"",
                    "#1010: designer context JSON should expose form builder ids");
    expect_contains(process.stdout_text, "\"id\": \"control-builder\"",
                    "#961: designer context JSON should expose control builder ids");
    expect_contains(process.stdout_text, "\"toolboxItems\": [",
                    "#961: designer context JSON should expose toolbox items");
    expect_contains(process.stdout_text, "\"id\": \"textbox\"",
                    "#961: designer context JSON should expose TextBox toolbox ids");

    const auto override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "visual_method", "--json"},
        temp_root);

    if (override_process.exit_code != 0) {
        std::cerr << "studio host override stdout:\n" << override_process.stdout_text << "\n";
        std::cerr << "studio host override stderr:\n" << override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(override_process.exit_code == 0, "#962: Studio host explicit context JSON smoke should exit successfully");
    expect_contains(override_process.stdout_text, "\"selectionContext\": \"visual_method\"",
                    "#962: explicit visual_method selection contexts should serialize through host JSON");
    expect_contains(override_process.stdout_text, "\"id\": \"edit-visual-method\"",
                    "#962: explicit visual_method contexts should expose method-editor actions");
    expect_not_contains(override_process.stdout_text, "\"selectionContext\": \"visual_object\"",
                        "#962: explicit selection contexts should override the form default selection context");

    const auto container_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "container_object", "--json"},
        temp_root);

    if (container_override_process.exit_code != 0) {
        std::cerr << "studio host container override stdout:\n" << container_override_process.stdout_text << "\n";
        std::cerr << "studio host container override stderr:\n" << container_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(container_override_process.exit_code == 0,
           "#1014: Studio host explicit container context JSON smoke should exit successfully");
    expect_contains(container_override_process.stdout_text, "\"selectionContext\": \"container_object\"",
                    "#1014: explicit container_object selection contexts should serialize through host JSON");
    expect_contains(container_override_process.stdout_text, "\"id\": \"control-builder\"",
                    "#1014: explicit container_object contexts should expose control builder metadata");
    expect_contains(container_override_process.stdout_text, "\"id\": \"grid-builder\"",
                    "#1014: explicit container_object contexts should expose grid builder metadata");
    expect_contains(container_override_process.stdout_text, "\"id\": \"checkbox\"",
                    "#1014: explicit container_object contexts should expose container-safe toolbox metadata");
    expect_not_contains(container_override_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1014: explicit container_object contexts should not expose form builders");
    expect_not_contains(container_override_process.stdout_text, "\"id\": \"class-builder\"",
                        "#1014: explicit container_object contexts should not expose class builders");

    const auto label_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "label_expression", "--json"},
        temp_root);

    if (label_override_process.exit_code != 0) {
        std::cerr << "studio host label override stdout:\n" << label_override_process.stdout_text << "\n";
        std::cerr << "studio host label override stderr:\n" << label_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_override_process.exit_code == 0,
           "#1011: Studio host explicit label context JSON smoke should exit successfully");
    expect_contains(label_override_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                    "#1011: explicit label_expression selection contexts should serialize through host JSON");
    expect_contains(label_override_process.stdout_text, "\"id\": \"label-wizard\"",
                    "#1011: explicit label_expression contexts should expose label wizard builders");
    expect_not_contains(label_override_process.stdout_text, "\"id\": \"report-builder\"",
                        "#1011: explicit label_expression contexts should not reuse report builders");

    const auto class_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "class_designer", "--json"},
        temp_root);

    if (class_override_process.exit_code != 0) {
        std::cerr << "studio host class override stdout:\n" << class_override_process.stdout_text << "\n";
        std::cerr << "studio host class override stderr:\n" << class_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(class_override_process.exit_code == 0,
           "#1012: Studio host explicit class context JSON smoke should exit successfully");
    expect_contains(class_override_process.stdout_text, "\"selectionContext\": \"class_designer\"",
                    "#1012: explicit class_designer selection contexts should serialize through host JSON");
    expect_contains(class_override_process.stdout_text, "\"id\": \"class-builder\"",
                    "#1012: explicit class_designer contexts should expose class builder metadata");
    expect_not_contains(class_override_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1012: explicit class_designer contexts should not expose form builders");
    expect_not_contains(class_override_process.stdout_text, "\"id\": \"control-builder\"",
                        "#1012: explicit class_designer contexts should not expose control builders");

    const auto menu_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "menu_item", "--json"},
        temp_root);

    if (menu_override_process.exit_code != 0) {
        std::cerr << "studio host menu override stdout:\n" << menu_override_process.stdout_text << "\n";
        std::cerr << "studio host menu override stderr:\n" << menu_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(menu_override_process.exit_code == 0,
           "#1013: Studio host explicit menu context JSON smoke should exit successfully");
    expect_contains(menu_override_process.stdout_text, "\"selectionContext\": \"menu_item\"",
                    "#1013: explicit menu_item selection contexts should serialize through host JSON");
    expect_contains(menu_override_process.stdout_text, "\"id\": \"menu-designer\"",
                    "#1013: explicit menu_item contexts should expose menu designer metadata");
    expect_contains(menu_override_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1013: explicit menu_item contexts should expose zero toolbox-item count");
    expect_not_contains(menu_override_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1013: explicit menu_item contexts should not expose form builders");

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_form_asset(label_path);
    const auto label_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--json"},
        temp_root);

    if (label_process.exit_code != 0) {
        std::cerr << "studio host label stdout:\n" << label_process.stdout_text << "\n";
        std::cerr << "studio host label stderr:\n" << label_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_process.exit_code == 0, "#1011: Studio host label JSON smoke should exit successfully");
    expect_contains(label_process.stdout_text, "\"kind\": \"label\"",
                    "#1011: label JSON should preserve label document kind");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                    "#1011: label documents should default to label-expression JSON contexts");
    expect_contains(label_process.stdout_text, "\"builderCount\": 1",
                    "#1011: label JSON should expose label builder count");
    expect_contains(label_process.stdout_text, "\"id\": \"label-wizard\"",
                    "#1011: label JSON should expose label wizard builder ids");
    expect_not_contains(label_process.stdout_text, "\"id\": \"report-builder\"",
                        "#1011: label JSON should not expose report builder ids");

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_form_asset(report_path);
    const auto report_data_environment_symbol_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--symbol", "Dataenvironment.OpenTables", "--json"},
        temp_root);

    if (report_data_environment_symbol_process.exit_code != 0) {
        std::cerr << "studio host report data-environment symbol stdout:\n"
                  << report_data_environment_symbol_process.stdout_text << "\n";
        std::cerr << "studio host report data-environment symbol stderr:\n"
                  << report_data_environment_symbol_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(report_data_environment_symbol_process.exit_code == 0,
           "#1016: Studio host report DataEnvironment symbol JSON smoke should exit successfully");
    expect_contains(report_data_environment_symbol_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                    "#1016: report DataEnvironment symbols should infer data-environment JSON contexts");
    expect_contains(report_data_environment_symbol_process.stdout_text, "\"id\": \"data-environment-builder\"",
                    "#1016: report DataEnvironment symbols should expose data-environment builders");
    expect_not_contains(report_data_environment_symbol_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                        "#1016: DataEnvironment symbols should override report/label expression defaults");

    const fs::path class_path = temp_root / "customer.vcx";
    write_synthetic_form_asset(class_path);
    const auto class_process = run_process_capture(
        studio_host_path,
        {"--path", class_path.string(), "--json"},
        temp_root);

    if (class_process.exit_code != 0) {
        std::cerr << "studio host class stdout:\n" << class_process.stdout_text << "\n";
        std::cerr << "studio host class stderr:\n" << class_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(class_process.exit_code == 0, "#1012: Studio host class-library JSON smoke should exit successfully");
    expect_contains(class_process.stdout_text, "\"kind\": \"class_library\"",
                    "#1012: class-library JSON should preserve class-library document kind");
    expect_contains(class_process.stdout_text, "\"selectionContext\": \"class_designer\"",
                    "#1012: class-library documents should default to class-designer JSON contexts");
    expect_contains(class_process.stdout_text, "\"builderCount\": 1",
                    "#1012: class-library JSON should expose class builder count");
    expect_contains(class_process.stdout_text, "\"id\": \"class-builder\"",
                    "#1012: class-library JSON should expose class builder ids");
    expect_not_contains(class_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1012: class-library JSON should not expose form builder ids");
    expect_not_contains(class_process.stdout_text, "\"id\": \"control-builder\"",
                        "#1012: class-library JSON should not expose control builder ids");

    const fs::path menu_path = temp_root / "mainmenu.mnx";
    write_synthetic_form_asset(menu_path);
    const auto menu_process = run_process_capture(
        studio_host_path,
        {"--path", menu_path.string(), "--json"},
        temp_root);

    if (menu_process.exit_code != 0) {
        std::cerr << "studio host menu stdout:\n" << menu_process.stdout_text << "\n";
        std::cerr << "studio host menu stderr:\n" << menu_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(menu_process.exit_code == 0, "#1013: Studio host menu JSON smoke should exit successfully");
    expect_contains(menu_process.stdout_text, "\"kind\": \"menu\"",
                    "#1013: menu JSON should preserve menu document kind");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
                    "#1013: menu documents should default to menu-item JSON contexts");
    expect_contains(menu_process.stdout_text, "\"builderCount\": 1",
                    "#1013: menu JSON should expose menu builder count");
    expect_contains(menu_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1013: menu JSON should expose zero toolbox-item count");
    expect_contains(menu_process.stdout_text, "\"id\": \"menu-designer\"",
                    "#1013: menu JSON should expose menu designer builder ids");
    expect_not_contains(menu_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1013: menu JSON should not expose form builder ids");

    const fs::path label_data_environment_path = temp_root / "mailing_data_environment.lbx";
    write_synthetic_table_with_data_environment(label_data_environment_path);
    const auto label_data_environment_process = run_process_capture(
        studio_host_path,
        {"--path", label_data_environment_path.string(), "--record", "0", "--json"},
        temp_root);

    if (label_data_environment_process.exit_code != 0) {
        std::cerr << "studio host label data-environment stdout:\n"
                  << label_data_environment_process.stdout_text << "\n";
        std::cerr << "studio host label data-environment stderr:\n"
                  << label_data_environment_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_data_environment_process.exit_code == 0,
           "#1016: Studio host selected label DataEnvironment JSON smoke should exit successfully");
    expect_contains(label_data_environment_process.stdout_text, "\"kind\": \"label\"",
                    "#1016: selected DataEnvironment label JSON should preserve label document kind");
    expect_contains(label_data_environment_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                    "#1016: selected label DataEnvironment records should infer data-environment JSON contexts");
    expect_contains(label_data_environment_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1016: selected label DataEnvironment records should expose zero toolbox-item count");
    expect_contains(label_data_environment_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#1016: selected label DataEnvironment records should expose data-environment editor actions");
    expect_contains(label_data_environment_process.stdout_text, "\"id\": \"data-environment-builder\"",
                    "#1016: selected label DataEnvironment records should expose data-environment builders");
    expect_not_contains(label_data_environment_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                        "#1016: selected label DataEnvironment records should not keep label-expression defaults");

    const auto symbol_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--record", "5",
            "--json"
        },
        temp_root);

    if (symbol_process.exit_code != 0) {
        std::cerr << "studio host symbol stdout:\n" << symbol_process.stdout_text << "\n";
        std::cerr << "studio host symbol stderr:\n" << symbol_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(symbol_process.exit_code == 0, "#963: Studio host symbol-inferred context JSON smoke should exit successfully");
    expect_contains(symbol_process.stdout_text, "\"launchSelection\": {",
                    "#964: Studio host JSON should expose launch selection metadata");
    expect_contains(symbol_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
                    "#964: Studio host JSON should expose launch selection symbols");
    expect_contains(symbol_process.stdout_text, "\"line\": 42",
                    "#964: Studio host JSON should expose launch selection lines");
    expect_contains(symbol_process.stdout_text, "\"column\": 7",
                    "#964: Studio host JSON should expose launch selection columns");
    expect_contains(symbol_process.stdout_text, "\"recordAvailable\": true",
                    "#967: Studio host JSON should expose explicit launch record availability");
    expect_contains(symbol_process.stdout_text, "\"recordIndex\": 5",
                    "#964: Studio host JSON should expose launch selection record indexes");
    expect_contains(symbol_process.stdout_text, "\"selectedObjectAvailable\": false",
                    "#979: unmatched explicit selected records should report no selected object availability");
    expect_contains(symbol_process.stdout_text, "\"selectedObject\": null",
                    "#967: Studio host JSON should report null selectedObject when no parsed object matches");
    expect_contains(symbol_process.stdout_text, "\"selectionContext\": \"visual_method\"",
                    "#963: method-like launch symbols should infer visual-method JSON contexts");
    expect_contains(symbol_process.stdout_text, "\"id\": \"edit-visual-method\"",
                    "#963: symbol-inferred visual-method contexts should expose method-editor actions");
    expect_not_contains(symbol_process.stdout_text, "\"selectionContext\": \"visual_object\"",
                        "#963: symbol-inferred contexts should replace the form default selection context");

    const auto data_environment_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--symbol", "Dataenvironment.OpenTables", "--json"},
        temp_root);

    if (data_environment_process.exit_code != 0) {
        std::cerr << "studio host data-environment stdout:\n" << data_environment_process.stdout_text << "\n";
        std::cerr << "studio host data-environment stderr:\n" << data_environment_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(data_environment_process.exit_code == 0,
           "#965: Studio host data-environment symbol context JSON smoke should exit successfully");
    expect_contains(data_environment_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                    "#965: DataEnvironment symbols should infer data-environment JSON contexts");
    expect_contains(data_environment_process.stdout_text, "\"builderCount\": 1",
                    "#1009: data-environment JSON should expose designer context builder count");
    expect_contains(data_environment_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1009: data-environment JSON should expose zero toolbox-item count");
    expect_contains(data_environment_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#965: inferred data-environment contexts should expose data-environment editor actions");
    expect_contains(data_environment_process.stdout_text, "\"id\": \"data-environment-builder\"",
                    "#965: inferred data-environment contexts should expose data-environment builders");
    expect_not_contains(data_environment_process.stdout_text, "\"selectionContext\": \"visual_method\"",
                        "#965: DataEnvironment symbols should not fall through to visual-method contexts");

    const auto explicit_precedence_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--symbol", "Dataenvironment.OpenTables",
            "--selection-context", "report_expression",
            "--json"
        },
        temp_root);

    if (explicit_precedence_process.exit_code != 0) {
        std::cerr << "studio host explicit precedence stdout:\n" << explicit_precedence_process.stdout_text << "\n";
        std::cerr << "studio host explicit precedence stderr:\n" << explicit_precedence_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(explicit_precedence_process.exit_code == 0,
           "#965: Studio host explicit-over-DataEnvironment context JSON smoke should exit successfully");
    expect_contains(explicit_precedence_process.stdout_text, "\"selectionContext\": \"report_expression\"",
                    "#965: explicit selection contexts should serialize when a DataEnvironment symbol is also present");
    expect_not_contains(explicit_precedence_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                        "#965: explicit selection contexts should override symbol-inferred data-environment contexts");

    const fs::path selected_container_path = temp_root / "selected_container.scx";
    write_synthetic_form_table_with_container_object(selected_container_path);
    const auto selected_container_process = run_process_capture(
        studio_host_path,
        {"--path", selected_container_path.string(), "--record", "1", "--json"},
        temp_root);

    if (selected_container_process.exit_code != 0) {
        std::cerr << "studio host selected container stdout:\n" << selected_container_process.stdout_text << "\n";
        std::cerr << "studio host selected container stderr:\n" << selected_container_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(selected_container_process.exit_code == 0,
           "#1015: Studio host selected container JSON smoke should exit successfully");
    expect_contains(selected_container_process.stdout_text, "\"recordIndex\": 1",
                    "#1015: selected container JSON should preserve selected record index");
    expect_contains(selected_container_process.stdout_text, "\"baseclassName\": \"pageframe\"",
                    "#1015: selected container JSON should expose selected baseclass metadata");
    expect_contains(selected_container_process.stdout_text, "\"selectionContext\": \"container_object\"",
                    "#1015: selected container records should infer container-object JSON contexts");
    expect_contains(selected_container_process.stdout_text, "\"id\": \"control-builder\"",
                    "#1015: inferred container contexts should expose control builders");
    expect_contains(selected_container_process.stdout_text, "\"id\": \"grid-builder\"",
                    "#1015: inferred container contexts should expose grid builders");
    expect_contains(selected_container_process.stdout_text, "\"id\": \"checkbox\"",
                    "#1015: inferred container contexts should expose container-safe toolbox items");
    expect_not_contains(selected_container_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1015: inferred container contexts should not expose form builders");

    const auto selected_grid_process = run_process_capture(
        studio_host_path,
        {"--path", selected_container_path.string(), "--record", "2", "--json"},
        temp_root);

    if (selected_grid_process.exit_code != 0) {
        std::cerr << "studio host selected grid stdout:\n" << selected_grid_process.stdout_text << "\n";
        std::cerr << "studio host selected grid stderr:\n" << selected_grid_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(selected_grid_process.exit_code == 0,
           "#1015: Studio host selected grid JSON smoke should exit successfully");
    expect_contains(selected_grid_process.stdout_text, "\"baseclassName\": \"grid\"",
                    "#1015: selected grid JSON should expose selected baseclass metadata");
    expect_contains(selected_grid_process.stdout_text, "\"selectionContext\": \"container_object\"",
                    "#1015: selected grid records should infer container-object JSON contexts");

    const fs::path selected_form_path = temp_root / "selected.scx";
    write_synthetic_form_table_with_objects(selected_form_path);
    const auto selected_object_process = run_process_capture(
        studio_host_path,
        {"--path", selected_form_path.string(), "--record", "1", "--json"},
        temp_root);

    if (selected_object_process.exit_code != 0) {
        std::cerr << "studio host selected object stdout:\n" << selected_object_process.stdout_text << "\n";
        std::cerr << "studio host selected object stderr:\n" << selected_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(selected_object_process.exit_code == 0,
           "#967: Studio host selected-object JSON smoke should exit successfully");
    expect_contains(selected_object_process.stdout_text, "\"objectCount\": 4",
                    "#977: Studio host JSON should expose document-level object counts");
    expect_contains(selected_object_process.stdout_text, "\"deletedObjectCount\": 0",
                    "#977: Studio host JSON should expose document-level deleted object counts");
    expect_contains(selected_object_process.stdout_text, "\"rootObjectCount\": 2",
                    "#978: Studio host JSON should expose document-level root object counts");
    expect_contains(selected_object_process.stdout_text, "\"rootRecordIndexes\": [0, 1]",
                    "#981: Studio host JSON should expose document-level root record links");
    expect_contains(selected_object_process.stdout_text, "\"leafObjectCount\": 3",
                    "#982: Studio host JSON should expose document-level leaf object counts");
    expect_contains(selected_object_process.stdout_text, "\"leafRecordIndexes\": [0, 2, 3]",
                    "#982: Studio host JSON should expose document-level leaf record links");
    expect_contains(selected_object_process.stdout_text, "\"maxObjectDepth\": 1",
                    "#983: Studio host JSON should expose document-level maximum object tree depth");
    expect_contains(selected_object_process.stdout_text, "\"selectedObjectAvailable\": true",
                    "#979: matched explicit selected records should report selected object availability");
    expect_contains(selected_object_process.stdout_text, "\"selectedObject\": {",
                    "#967: Studio host JSON should expose selected object summaries for matching records");
    expect_contains(selected_object_process.stdout_text, "\"recordIndex\": 1",
                    "#967: selected object summaries should expose selected record indexes");
    expect_contains(selected_object_process.stdout_text, "\"objectName\": \"frmCustomer\"",
                    "#967: selected object summaries should expose object names");
    expect_contains(selected_object_process.stdout_text, "\"uniqueId\": \"form-1\"",
                    "#967: selected object summaries should expose unique ids");
    expect_contains(selected_object_process.stdout_text, "\"className\": \"customerform\"",
                    "#967: selected object summaries should expose class names");
    expect_contains(selected_object_process.stdout_text, "\"baseclassName\": \"form\"",
                    "#967: selected object summaries should expose baseclass names");
    const auto selected_object_begin = selected_object_process.stdout_text.find("\"selectedObject\": {");
    const auto selected_object_end =
        selected_object_begin == std::string::npos
            ? std::string::npos
            : selected_object_process.stdout_text.find("\"hasSidecar\"", selected_object_begin);
    expect(selected_object_begin != std::string::npos &&
               selected_object_end != std::string::npos &&
               selected_object_end > selected_object_begin,
           "#968: Studio host JSON should delimit a selected-object section before document metadata resumes");
    if (selected_object_begin != std::string::npos &&
        selected_object_end != std::string::npos &&
        selected_object_end > selected_object_begin) {
        const auto selected_object_json =
            selected_object_process.stdout_text.substr(selected_object_begin, selected_object_end - selected_object_begin);
        expect_contains(selected_object_json, "\"deleted\": false",
                        "#974: selected object summaries should expose parsed deletion state");
        expect_contains(selected_object_json, "\"properties\": [",
                        "#968: selected object summaries should expose direct property snapshots");
        expect_contains(selected_object_json, "\"name\": \"OBJNAME\"",
                        "#968: selected object properties should include DBF field names");
        expect_contains(selected_object_json, "\"type\": \"C\"",
                        "#968: selected object properties should preserve DBF field types");
        expect_contains(selected_object_json, "\"isNull\": false",
                        "#968: selected object properties should preserve DBF null flags");
        expect_contains(selected_object_json, "\"value\": \"frmCustomer\"",
                        "#968: selected object properties should include selected object values");
        const auto objname_property_begin = selected_object_json.find("\"name\": \"OBJNAME\"");
        expect(objname_property_begin != std::string::npos,
               "#975: selected object properties should include an OBJNAME property object");
        if (objname_property_begin != std::string::npos) {
            const auto objname_property_end = selected_object_json.find("}", objname_property_begin);
            const auto objname_property_json =
                objname_property_end == std::string::npos
                    ? selected_object_json.substr(objname_property_begin)
                    : selected_object_json.substr(objname_property_begin, objname_property_end - objname_property_begin);
            expect_contains(objname_property_json, "\"fieldIndex\": 3",
                            "#975: selected direct-field properties should expose DBF field indexes");
            expect_contains(objname_property_json, "\"memoBlockNumber\": 0",
                            "#975: selected direct-field properties should expose memo block provenance");
            expect_contains(objname_property_json, "\"derivedFromPropertyBlob\": false",
                            "#975: selected direct-field properties should not be marked as property-blob derived");
            expect_contains(objname_property_json, "\"sourceLineIndex\": null",
                            "#975: selected direct-field properties should expose null source line provenance");
        }
        expect_contains(selected_object_json, "\"name\": \"BASECLASS\"",
                        "#968: selected object properties should include later direct DBF fields");
        expect_contains(selected_object_json, "\"value\": \"form\"",
                        "#968: selected object properties should include selected baseclass values");
        expect_contains(selected_object_json, "\"childCount\": 2",
                        "#970: selected parent object summaries should expose direct child counts");
        expect_contains(selected_object_json, "\"childRecordIndexes\": [2, 3]",
                        "#980: selected parent object summaries should expose direct child record links");
        expect_contains(selected_object_json, "\"parentRecordIndex\": null",
                        "#971: root selected object summaries should expose null parent record links");
        expect_contains(selected_object_json, "\"ancestorRecordIndexes\": []",
                        "#985: root selected object summaries should expose empty ancestor record links");
        expect_contains(selected_object_json, "\"objectPath\": \"frmCustomer\"",
                        "#972: root selected object summaries should expose direct object paths");
        expect_contains(selected_object_json, "\"objectDepth\": 0",
                        "#983: root selected object summaries should expose zero object tree depth");
        expect_contains(selected_object_json, "\"siblingIndex\": 1",
                        "#984: selected root object summaries should expose document-root sibling order");
        expect_contains(selected_object_json, "\"siblingCount\": 2",
                        "#984: selected root object summaries should expose document-root sibling count");
        expect_contains(selected_object_json, "\"objectTypeCode\": 1",
                        "#973: selected object summaries should expose raw object type codes");
        expect_contains(selected_object_json, "\"objectCode\": 0",
                        "#973: selected object summaries should expose raw object codes");
        expect_contains(selected_object_json, "\"platform\": \"WINDOWS\"",
                        "#973: selected object summaries should expose parsed platform metadata");
        expect_contains(selected_object_json, "\"propertyCount\": 8",
                        "#976: selected object summaries should expose direct property counts");
    }
    const auto objects_begin = selected_object_process.stdout_text.find("\"objects\": [");
    expect(objects_begin != std::string::npos,
           "#969: Studio host JSON should expose a full object array section");
    if (objects_begin != std::string::npos) {
        const auto objects_json = selected_object_process.stdout_text.substr(objects_begin);
        expect_contains(objects_json, "\"objectName\": \"frmCustomer\"",
                        "#969: full object entries should expose object names directly");
        expect_contains(objects_json, "\"uniqueId\": \"form-1\"",
                        "#969: full object entries should expose unique ids directly");
        expect_contains(objects_json, "\"parentName\": \"\"",
                        "#969: full object entries should expose parent names directly");
        expect_contains(objects_json, "\"className\": \"customerform\"",
                        "#969: full object entries should expose class names directly");
        expect_contains(objects_json, "\"baseclassName\": \"form\"",
                        "#969: full object entries should expose baseclass names directly");
        const auto child_object_begin = objects_json.find("\"objectName\": \"cmdSave\"");
        expect(child_object_begin != std::string::npos,
               "#970: synthetic SCX object array should include the child control object");
        if (child_object_begin != std::string::npos) {
            const auto child_entry_begin = objects_json.rfind("{", child_object_begin);
            const auto child_properties_begin = objects_json.find("\"properties\": [", child_object_begin);
            const auto child_object_json =
                child_entry_begin == std::string::npos
                    ? objects_json.substr(child_object_begin)
                    : child_properties_begin == std::string::npos
                        ? objects_json.substr(child_entry_begin)
                        : objects_json.substr(child_entry_begin, child_properties_begin - child_entry_begin);
            expect_contains(child_object_json, "\"parentName\": \"frmCustomer\"",
                            "#970: child object entries should expose their parent object name");
            expect_contains(child_object_json, "\"parentRecordIndex\": 1",
                            "#971: child object entries should expose resolved parent record links");
            expect_contains(child_object_json, "\"ancestorRecordIndexes\": [1]",
                            "#985: child object entries should expose root-to-parent ancestor record links");
            expect_contains(child_object_json, "\"childCount\": 0",
                            "#970: leaf child object entries should expose zero child count");
            expect_contains(child_object_json, "\"childRecordIndexes\": []",
                            "#980: leaf child object entries should expose empty child record links");
            expect_contains(child_object_json, "\"objectPath\": \"frmCustomer.cmdSave\"",
                            "#972: child object entries should expose parent-prefixed object paths");
            expect_contains(child_object_json, "\"objectDepth\": 1",
                            "#983: child object entries should expose nested object tree depth");
            expect_contains(child_object_json, "\"siblingIndex\": 0",
                            "#984: first child object entries should expose sibling order");
            expect_contains(child_object_json, "\"siblingCount\": 2",
                            "#984: child object entries should expose sibling count");
            expect_contains(child_object_json, "\"objectTypeCode\": 4",
                            "#973: child object entries should expose raw object type codes");
            expect_contains(child_object_json, "\"objectCode\": 2",
                            "#973: child object entries should expose raw object codes");
            expect_contains(child_object_json, "\"platform\": \"WINDOWS\"",
                            "#973: child object entries should expose parsed platform metadata");
            expect_contains(child_object_json, "\"propertyCount\": 8",
                            "#976: child object entries should expose direct property counts");
        }
        const auto sibling_object_begin = objects_json.find("\"objectName\": \"txtName\"");
        expect(sibling_object_begin != std::string::npos,
               "#984: synthetic SCX object array should include the second sibling control object");
        if (sibling_object_begin != std::string::npos) {
            const auto sibling_entry_begin = objects_json.rfind("{", sibling_object_begin);
            const auto sibling_properties_begin = objects_json.find("\"properties\": [", sibling_object_begin);
            const auto sibling_object_json =
                sibling_entry_begin == std::string::npos
                    ? objects_json.substr(sibling_object_begin)
                    : sibling_properties_begin == std::string::npos
                        ? objects_json.substr(sibling_entry_begin)
                        : objects_json.substr(sibling_entry_begin, sibling_properties_begin - sibling_entry_begin);
            expect_contains(sibling_object_json, "\"parentRecordIndex\": 1",
                            "#984: second child object entries should preserve resolved parent links");
            expect_contains(sibling_object_json, "\"ancestorRecordIndexes\": [1]",
                            "#985: second child object entries should expose root-to-parent ancestor record links");
            expect_contains(sibling_object_json, "\"objectDepth\": 1",
                            "#984: second child object entries should expose nested object tree depth");
            expect_contains(sibling_object_json, "\"siblingIndex\": 1",
                            "#984: second child object entries should expose sibling order");
            expect_contains(sibling_object_json, "\"siblingCount\": 2",
                            "#984: second child object entries should expose sibling count");
        }
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_creates_toolbox_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "textbox",
            "--unique-id", "created-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Customer",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    if (create_process.exit_code != 0) {
        std::cerr << "studio host toolbox-create stdout:\n" << create_process.stdout_text << "\n";
        std::cerr << "studio host toolbox-create stderr:\n" << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(create_process.exit_code == 0, "#1018: toolbox-create JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
                    "#1018: successful toolbox-create JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"toolboxCreate\": {",
                    "#1018: toolbox-create JSON should use a stable result object");
    expect_contains(create_process.stdout_text, "\"ok\": true",
                    "#1018: toolbox-create JSON should expose result success");
    expect_contains(create_process.stdout_text, "\"recordIndex\": 2",
                    "#1018: toolbox-create JSON should expose appended record index");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
                    "#1018: toolbox-create JSON should expose generated object name");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"created-textbox-guid\"",
                    "#1018: toolbox-create JSON should expose created unique id");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
                    "#1018: toolbox-create JSON should expose created parent name");

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(caption.ok && caption.exists && caption.value == "Customer",
        "#1018: toolbox-create host command should propagate extra direct fields");

    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1018: toolbox-create host command should propagate extra memo fields");

    const std::size_t object_count_before_failure = visual_object_count(form_path);
    const auto failure_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "missing-toolbox-item",
            "--unique-id", "should-not-exist",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);

    expect(failure_process.exit_code == 4, "#1018: unknown toolbox ids should return a command failure exit code");
    expect_contains(failure_process.stdout_text, "\"status\": \"error\"",
                    "#1018: failed toolbox-create JSON should report error status");
    expect_contains(failure_process.stdout_text, "\"ok\": false",
                    "#1018: failed toolbox-create JSON should expose result failure");
    expect_contains(failure_process.stdout_text, "\"error\": \"The requested toolbox item was not found.\"",
                    "#1018: failed toolbox-create JSON should expose clean error text");
    expect_contains(failure_process.stdout_text, "\"objectName\": \"\"",
                    "#1018: failed toolbox-create JSON should not report stale object names");
    expect(visual_object_count(form_path) == object_count_before_failure,
        "#1018: failed toolbox-create host commands should not mutate the asset");

    const auto report_label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "label",
            "--toolbox-context", "report",
            "--unique-id", "report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Total",
            "--json"
        },
        temp_root);

    expect(report_label_process.exit_code == 0,
        "#1019: report-compatible toolbox items should create through host JSON when report context is requested");
    expect_contains(report_label_process.stdout_text, "\"objectName\": \"lbl1\"",
                    "#1019: report-compatible toolbox creates should expose generated label names");
    expect_contains(report_label_process.stdout_text, "\"uniqueId\": \"report-label-guid\"",
                    "#1019: report-compatible toolbox creates should expose created unique ids");

    const std::size_t object_count_before_context_failure = visual_object_count(form_path);
    const auto report_textbox_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "textbox",
            "--toolbox-context", "report",
            "--unique-id", "report-textbox-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);

    expect(report_textbox_process.exit_code == 4,
        "#1019: report-incompatible toolbox items should fail through host JSON when report context is requested");
    expect_contains(report_textbox_process.stdout_text, "\"status\": \"error\"",
                    "#1019: context-filtered toolbox failures should report JSON error status");
    expect_contains(
        report_textbox_process.stdout_text,
        "\"error\": \"The requested toolbox item is not available in the requested designer context.\"",
        "#1019: context-filtered toolbox failures should expose clean error text");
    expect(visual_object_count(form_path) == object_count_before_context_failure,
        "#1019: context-filtered toolbox failures should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_sets_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    auto caption_value = [&]() {
        return copperfin::vfp::query_visual_object_property({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "existing-textbox-guid",
            .property_name = "CAPTION"
        });
    };

    const auto record_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--record", "1",
            "--property-name", "CAPTION",
            "--property-value", "RecordTarget",
            "--json"
        },
        temp_root);
    expect(record_process.exit_code == 0,
        "#1020: record-index host property edits should remain compatible");
    auto caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "RecordTarget",
        "#1020: record-index host property edits should update the selected record");

    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--object-name", "txt1",
            "--property-name", "CAPTION",
            "--property-value", "NameTarget",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1020: object-name host property edits should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "NameTarget",
        "#1020: object-name host property edits should update the named object");

    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "UniqueTarget",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1020: unique-id host property edits should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "UniqueTarget",
        "#1020: unique-id host property edits should update the stable selected object");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--property-value", "ShouldNotWrite",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1020: missing object-name host property edits should return command failure");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "UniqueTarget",
        "#1020: missing object-name host property edits should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    auto caption_value = [&]() {
        return copperfin::vfp::query_visual_object_property({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "existing-textbox-guid",
            .property_name = "CAPTION"
        });
    };

    const auto object_name_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--object-name", "txt1",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(object_name_clear_process.exit_code == 0,
        "#1021: object-name host property clears should exit successfully");
    auto caption = caption_value();
    expect(caption.ok && caption.exists && caption.value.empty(),
        "#1021: object-name host property clears should empty direct-field properties");

    const auto set_before_unique_clear = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BeforeUniqueClear",
            "--json"
        },
        temp_root);
    expect(set_before_unique_clear.exit_code == 0,
        "#1021: clear-property setup should be able to restore a direct-field value");

    const auto unique_id_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(unique_id_clear_process.exit_code == 0,
        "#1021: unique-id host property clears should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value.empty(),
        "#1021: unique-id host property clears should empty direct-field properties");

    const auto set_before_missing_clear = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BeforeMissingClear",
            "--json"
        },
        temp_root);
    expect(set_before_missing_clear.exit_code == 0,
        "#1021: missing-clear setup should be able to restore a direct-field value");

    const auto missing_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(missing_clear_process.exit_code == 4,
        "#1021: missing object-name host property clears should return command failure");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "BeforeMissingClear",
        "#1021: missing object-name host property clears should not mutate the asset");

    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--clear-property",
            "--property-name", "CAPTION",
            "--property-value", "Ambiguous",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1021: ambiguous set/clear property requests should fail during launch parsing");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "BeforeMissingClear",
        "#1021: ambiguous set/clear property requests should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path object_name_path = write_synthetic_form_table_for_property_rename(temp_root, "object_name.scx");
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--rename-property",
            "--object-name", "txt1",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1022: object-name host property renames should exit successfully");
    auto renamed_property = copperfin::vfp::query_visual_object_property({
        .path = object_name_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "InputSource"
    });
    expect(renamed_property.ok && renamed_property.exists && renamed_property.value == "\"customer.name\"",
        "#1022: object-name host property renames should create the target memo-backed property");
    auto source_property = copperfin::vfp::query_visual_object_property({
        .path = object_name_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && !source_property.exists,
        "#1022: object-name host property renames should remove the source memo-backed property");

    const fs::path unique_id_path = write_synthetic_form_table_for_property_rename(temp_root, "unique_id.scx");
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--rename-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1022: unique-id host property renames should exit successfully");
    renamed_property = copperfin::vfp::query_visual_object_property({
        .path = unique_id_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "InputSource"
    });
    expect(renamed_property.ok && renamed_property.exists && renamed_property.value == "\"customer.name\"",
        "#1022: unique-id host property renames should create the target memo-backed property");

    const fs::path missing_path = write_synthetic_form_table_for_property_rename(temp_root, "missing.scx");
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--rename-property",
            "--object-name", "missingObject",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1022: missing object-name host property renames should return command failure");
    source_property = copperfin::vfp::query_visual_object_property({
        .path = missing_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && source_property.exists && source_property.value == "\"customer.name\"",
        "#1022: missing object-name host property renames should not mutate the asset");

    const fs::path ambiguous_path = write_synthetic_form_table_for_property_rename(temp_root, "ambiguous.scx");
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--set-property",
            "--rename-property",
            "--property-name", "ControlSource",
            "--property-value", "Ambiguous",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1022: ambiguous set/rename property requests should fail during launch parsing");
    source_property = copperfin::vfp::query_visual_object_property({
        .path = ambiguous_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && source_property.exists && source_property.value == "\"customer.name\"",
        "#1022: ambiguous set/rename property requests should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_delete_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--delete-object",
            "--object-name", "txt1",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1023: object-name host object deletes should exit successfully");
    expect(visual_object_deleted(object_name_path, "existing-textbox-guid"),
        "#1023: object-name host object deletes should mark the targeted object deleted");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--delete-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1023: unique-id host object deletes should exit successfully");
    expect(visual_object_deleted(unique_id_path, "existing-textbox-guid"),
        "#1023: unique-id host object deletes should mark the targeted object deleted");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--delete-object",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1023: missing object-name host object deletes should return command failure");
    expect(!visual_object_deleted(missing_path, "existing-textbox-guid"),
        "#1023: missing object-name host object deletes should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--delete-object",
            "--clear-property",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1023: delete-object plus property command requests should fail during launch parsing");
    expect(!visual_object_deleted(ambiguous_path, "existing-textbox-guid"),
        "#1023: delete-object/property ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_restore_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    delete_existing_textbox(object_name_path,
        "#1024: restore-object object-name fixture should start with a deleted target");
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--restore-object",
            "--object-name", "txt1",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1024: object-name host object restores should exit successfully");
    expect(!visual_object_deleted(object_name_path, "existing-textbox-guid"),
        "#1024: object-name host object restores should clear the targeted object's deleted state");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    delete_existing_textbox(unique_id_path,
        "#1024: restore-object unique-id fixture should start with a deleted target");
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--restore-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1024: unique-id host object restores should exit successfully");
    expect(!visual_object_deleted(unique_id_path, "existing-textbox-guid"),
        "#1024: unique-id host object restores should clear the targeted object's deleted state");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    delete_existing_textbox(missing_path,
        "#1024: restore-object missing-object fixture should start with a deleted target");
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--restore-object",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1024: missing object-name host object restores should return command failure");
    expect(visual_object_deleted(missing_path, "existing-textbox-guid"),
        "#1024: missing object-name host object restores should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    delete_existing_textbox(ambiguous_path,
        "#1024: restore-object ambiguity fixture should start with a deleted target");
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--delete-object",
            "--restore-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1024: delete-object plus restore-object requests should fail during launch parsing");
    expect(visual_object_deleted(ambiguous_path, "existing-textbox-guid"),
        "#1024: delete-object/restore-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_duplicate_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--duplicate-object",
            "--object-name", "txt1",
            "--new-object-name", "txtCopy",
            "--new-name", "txtCopy",
            "--new-unique-id", "copied-textbox-guid",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1025: object-name host object duplicates should exit successfully");
    expect(visual_object_count(object_name_path) == 3U,
        "#1025: object-name host object duplicates should append one visual object");
    expect(visual_object_exists(object_name_path, "copied-textbox-guid"),
        "#1025: object-name host object duplicates should use replacement unique ids");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--duplicate-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "txtCopyById",
            "--new-name", "txtCopyById",
            "--new-unique-id", "copied-by-id-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1025: unique-id host object duplicates should exit successfully");
    expect(visual_object_count(unique_id_path) == 3U,
        "#1025: unique-id host object duplicates should append one visual object");
    expect(visual_object_exists(unique_id_path, "copied-by-id-guid"),
        "#1025: unique-id host object duplicates should use replacement unique ids");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--duplicate-object",
            "--object-name", "missingObject",
            "--new-object-name", "missingCopy",
            "--new-name", "missingCopy",
            "--new-unique-id", "missing-copy-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1025: missing object-name host object duplicates should return command failure");
    expect(visual_object_count(missing_path) == 2U,
        "#1025: missing object-name host object duplicates should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--duplicate-object",
            "--delete-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "ambiguousCopy",
            "--new-name", "ambiguousCopy",
            "--new-unique-id", "ambiguous-copy-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1025: duplicate-object plus delete-object requests should fail during launch parsing");
    expect(visual_object_count(ambiguous_path) == 2U,
        "#1025: duplicate-object/delete-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_rename_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--rename-object",
            "--object-name", "txt1",
            "--new-object-name", "txtCustomer",
            "--new-name", "txtCustomer",
            "--new-unique-id", "customer-textbox-guid",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1026: object-name host object renames should exit successfully");
    expect(visual_object_count(object_name_path) == 2U,
        "#1026: object-name host object renames should not append visual objects");
    expect(visual_object_exists(object_name_path, "customer-textbox-guid") &&
            !visual_object_exists(object_name_path, "existing-textbox-guid"),
        "#1026: object-name host object renames should replace the target identity");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--rename-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "txtCustomerById",
            "--new-name", "txtCustomerById",
            "--new-unique-id", "customer-by-id-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1026: unique-id host object renames should exit successfully");
    expect(visual_object_count(unique_id_path) == 2U,
        "#1026: unique-id host object renames should not append visual objects");
    expect(visual_object_exists(unique_id_path, "customer-by-id-guid") &&
            !visual_object_exists(unique_id_path, "existing-textbox-guid"),
        "#1026: unique-id host object renames should replace the target identity");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--rename-object",
            "--object-name", "missingObject",
            "--new-object-name", "missingRename",
            "--new-name", "missingRename",
            "--new-unique-id", "missing-rename-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1026: missing object-name host object renames should return command failure");
    expect(visual_object_count(missing_path) == 2U &&
            visual_object_exists(missing_path, "existing-textbox-guid"),
        "#1026: missing object-name host object renames should not mutate the asset");

    const fs::path empty_identity_path = temp_root / "empty_identity.scx";
    write_synthetic_form_table_for_toolbox_creation(empty_identity_path);
    const auto empty_identity_process = run_process_capture(
        studio_host_path,
        {
            "--path", empty_identity_path.string(),
            "--rename-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(empty_identity_process.exit_code == 2,
        "#1026: rename-object without replacement identity fields should fail during launch parsing");
    expect(visual_object_count(empty_identity_path) == 2U &&
            visual_object_exists(empty_identity_path, "existing-textbox-guid"),
        "#1026: rename-object empty-identity failures should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--rename-object",
            "--duplicate-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "ambiguousRename",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1026: rename-object plus duplicate-object requests should fail during launch parsing");
    expect(visual_object_count(ambiguous_path) == 2U &&
            visual_object_exists(ambiguous_path, "existing-textbox-guid"),
        "#1026: rename-object/duplicate-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reparents_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_reparent_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_object_reparent(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--reparent-object",
            "--object-name", "txt1",
            "--parent-name", "cntPanel",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1027: object-name host object reparent should exit successfully");
    expect(visual_object_parent(object_name_path, "existing-textbox-guid") == "cntPanel",
        "#1027: object-name host object reparent should update the target parent");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_object_reparent(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--reparent-object",
            "--unique-id", "existing-textbox-guid",
            "--parent-unique-id", "panel-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1027: unique-id host object reparent should exit successfully");
    expect(visual_object_parent(unique_id_path, "existing-textbox-guid") == "cntPanel",
        "#1027: unique-id host object reparent should update the target parent");

    const fs::path clear_parent_path = temp_root / "clear_parent.scx";
    write_synthetic_form_table_for_object_reparent(clear_parent_path);
    const auto clear_parent_process = run_process_capture(
        studio_host_path,
        {
            "--path", clear_parent_path.string(),
            "--reparent-object",
            "--unique-id", "existing-textbox-guid",
            "--clear-parent",
            "--json"
        },
        temp_root);
    expect(clear_parent_process.exit_code == 0,
        "#1027: clear-parent host object reparent should exit successfully");
    expect(visual_object_parent(clear_parent_path, "existing-textbox-guid").empty(),
        "#1027: clear-parent host object reparent should clear the target parent");

    const fs::path missing_source_path = temp_root / "missing_source.scx";
    write_synthetic_form_table_for_object_reparent(missing_source_path);
    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_source_path.string(),
            "--reparent-object",
            "--object-name", "missingObject",
            "--parent-name", "cntPanel",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 4,
        "#1027: missing source host object reparent should return command failure");
    expect(visual_object_parent(missing_source_path, "existing-textbox-guid") == "frmCustomer",
        "#1027: missing source host object reparent should not mutate the asset");

    const fs::path missing_parent_path = temp_root / "missing_parent.scx";
    write_synthetic_form_table_for_object_reparent(missing_parent_path);
    const auto missing_parent_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_parent_path.string(),
            "--reparent-object",
            "--unique-id", "existing-textbox-guid",
            "--parent-name", "missingParent",
            "--json"
        },
        temp_root);
    expect(missing_parent_process.exit_code == 4,
        "#1027: missing parent host object reparent should return command failure");
    expect(visual_object_parent(missing_parent_path, "existing-textbox-guid") == "frmCustomer",
        "#1027: missing parent host object reparent should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_reparent(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--reparent-object",
            "--rename-object",
            "--unique-id", "existing-textbox-guid",
            "--parent-name", "cntPanel",
            "--new-object-name", "txtCustomer",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1027: reparent-object plus rename-object requests should fail during launch parsing");
    expect(visual_object_parent(ambiguous_path, "existing-textbox-guid") == "frmCustomer",
        "#1027: reparent-object/rename-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_reorder_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path reorder_path = temp_root / "reorder.scx";
    write_synthetic_form_table_for_object_reorder(reorder_path);

    const auto front_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "c-guid",
            "--placement", "front",
            "--json"
        },
        temp_root);
    expect(front_process.exit_code == 0,
        "#1028: front host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,b-guid,d-guid",
        "#1028: front host object reorder should move the selected object to the front");

    const auto back_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--object-name", "cmdA",
            "--placement", "back",
            "--json"
        },
        temp_root);
    expect(back_process.exit_code == 0,
        "#1028: back host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,b-guid,d-guid,a-guid",
        "#1028: back host object reorder should move the selected object to the back");

    const auto before_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "a-guid",
            "--placement", "before",
            "--target-object-name", "cmdB",
            "--json"
        },
        temp_root);
    expect(before_process.exit_code == 0,
        "#1028: before host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,b-guid,d-guid",
        "#1028: before host object reorder should move the selected object before the target object name");

    const auto after_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "b-guid",
            "--placement", "after",
            "--target-unique-id", "d-guid",
            "--json"
        },
        temp_root);
    expect(after_process.exit_code == 0,
        "#1028: after host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: after host object reorder should move the selected object after the target unique id");

    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "b-guid",
            "--placement", "before",
            "--target-object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1028: missing target host object reorder should return command failure");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: missing target host object reorder should not mutate order");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "b-guid",
            "--placement", "sideways",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1028: unsupported placement host object reorder should return command failure");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: unsupported placement host object reorder should not mutate order");

    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--reparent-object",
            "--unique-id", "b-guid",
            "--placement", "front",
            "--parent-name", "cmdA",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1028: reorder-object plus reparent-object requests should fail during launch parsing");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: reorder-object/reparent-object ambiguity should not mutate order");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_groups_objects_by_stable_child_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_group_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path group_path = temp_root / "group.scx";
    write_synthetic_form_table_for_object_group(group_path);
    const auto group_process = run_process_capture(
        studio_host_path,
        {
            "--path", group_path.string(),
            "--group-object",
            "--field-value", "OBJNAME=cntGroup",
            "--field-value", "NAME=cntGroup",
            "--field-value", "UNIQUEID=group-guid",
            "--field-value", "PARENT=frmCustomer",
            "--field-value", "CLASS=Container",
            "--field-value", "BASECLASS=Container",
            "--field-value", "PROPERTIES=Caption = \"Group\"",
            "--group-child-object-name", "cmdSave",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(group_process.exit_code == 0,
        "#1030: host object group should exit successfully");
    expect(visual_object_count(group_path) == 5U &&
            visual_object_parent(group_path, "save-guid") == "cntGroup" &&
            visual_object_parent(group_path, "name-guid") == "cntGroup" &&
            visual_object_parent(group_path, "status-guid") == "frmCustomer",
        "#1030: host object group should append a container and reparent only selected children");

    const fs::path missing_child_path = temp_root / "missing_child.scx";
    write_synthetic_form_table_for_object_group(missing_child_path);
    const auto missing_child_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_child_path.string(),
            "--group-object",
            "--field-value", "OBJNAME=cntGroup",
            "--field-value", "UNIQUEID=group-guid",
            "--group-child-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_child_process.exit_code == 4,
        "#1030: missing child host object group should return command failure");
    expect(visual_object_count(missing_child_path) == 4U &&
            visual_object_parent(missing_child_path, "save-guid") == "frmCustomer" &&
            visual_object_parent(missing_child_path, "name-guid") == "frmCustomer",
        "#1030: missing child host object group should not mutate the asset");

    const fs::path missing_field_values_path = temp_root / "missing_field_values.scx";
    write_synthetic_form_table_for_object_group(missing_field_values_path);
    const auto missing_field_values_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_field_values_path.string(),
            "--group-object",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_field_values_process.exit_code == 2,
        "#1030: group-object without container field values should fail during launch parsing");
    expect(visual_object_count(missing_field_values_path) == 4U &&
            visual_object_parent(missing_field_values_path, "name-guid") == "frmCustomer",
        "#1030: group-object without container field values should not mutate the asset");

    const fs::path missing_children_path = temp_root / "missing_children.scx";
    write_synthetic_form_table_for_object_group(missing_children_path);
    const auto missing_children_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_children_path.string(),
            "--group-object",
            "--field-value", "OBJNAME=cntGroup",
            "--json"
        },
        temp_root);
    expect(missing_children_process.exit_code == 2,
        "#1030: group-object without child selectors should fail during launch parsing");
    expect(visual_object_count(missing_children_path) == 4U &&
            visual_object_parent(missing_children_path, "save-guid") == "frmCustomer",
        "#1030: group-object without child selectors should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_group(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--group-object",
            "--ungroup-object",
            "--field-value", "OBJNAME=cntGroup",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1030: group-object plus ungroup-object requests should fail during launch parsing");
    expect(visual_object_count(ambiguous_path) == 4U &&
            visual_object_parent(ambiguous_path, "name-guid") == "frmCustomer",
        "#1030: group-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_aligns_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_align_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path align_path = temp_root / "align.scx";
    write_synthetic_form_table_for_object_align(align_path);
    const auto align_process = run_process_capture(
        studio_host_path,
        {
            "--path", align_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-object-name", "txtName",
            "--align-target-unique-id", "status-guid",
            "--json"
        },
        temp_root);
    expect(align_process.exit_code == 0,
        "#1031: host object alignment should exit successfully");
    expect(visual_object_property(align_path, "name-guid", "HPOS") == "10" &&
            visual_object_property(align_path, "status-guid", "HPOS") == "10" &&
            visual_object_property(align_path, "name-guid", "VPOS") == "2",
        "#1031: host object alignment should align selected objects and preserve unrelated geometry");

    const fs::path missing_anchor_path = temp_root / "missing_anchor.scx";
    write_synthetic_form_table_for_object_align(missing_anchor_path);
    const auto missing_anchor_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_anchor_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "missing-anchor",
            "--align-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_anchor_process.exit_code == 4,
        "#1031: missing-anchor host object alignment should return command failure");
    expect(visual_object_property(missing_anchor_path, "name-guid", "HPOS") == "1" &&
            visual_object_property(missing_anchor_path, "name-guid", "VPOS") == "2",
        "#1031: missing-anchor host object alignment should not mutate the asset");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_align(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-unique-id", "missing-target",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1031: missing-target host object alignment should return command failure");
    expect(visual_object_property(missing_target_path, "name-guid", "HPOS") == "1" &&
            visual_object_property(missing_target_path, "status-guid", "HPOS") == "5",
        "#1031: missing-target host object alignment should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_align(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--align-object",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1031: align-object without alignment mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "name-guid", "HPOS") == "1",
        "#1031: align-object without alignment mode should not mutate the asset");

    const fs::path missing_targets_path = temp_root / "missing_targets.scx";
    write_synthetic_form_table_for_object_align(missing_targets_path);
    const auto missing_targets_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_targets_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-object-name", "cmdAnchor",
            "--json"
        },
        temp_root);
    expect(missing_targets_process.exit_code == 2,
        "#1031: align-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_targets_path, "name-guid", "HPOS") == "1",
        "#1031: align-object without target selectors should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_align(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--align-object",
            "--group-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-unique-id", "name-guid",
            "--field-value", "OBJNAME=cntGroup",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1031: align-object plus group-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "name-guid", "HPOS") == "1",
        "#1031: align-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_resize_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path resize_path = temp_root / "resize.scx";
    write_synthetic_form_table_for_object_align(resize_path);
    const auto resize_process = run_process_capture(
        studio_host_path,
        {
            "--path", resize_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-object-name", "txtName",
            "--resize-target-unique-id", "status-guid",
            "--json"
        },
        temp_root);
    expect(resize_process.exit_code == 0,
        "#1032: host object resize should exit successfully");
    expect(visual_object_property(resize_path, "name-guid", "WIDTH") == "100" &&
            visual_object_property(resize_path, "status-guid", "WIDTH") == "100" &&
            visual_object_property(resize_path, "name-guid", "HEIGHT") == "10",
        "#1032: host object resize should resize selected objects and preserve unrelated geometry");

    const fs::path missing_anchor_path = temp_root / "missing_anchor.scx";
    write_synthetic_form_table_for_object_align(missing_anchor_path);
    const auto missing_anchor_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_anchor_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-unique-id", "missing-anchor",
            "--resize-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_anchor_process.exit_code == 4,
        "#1032: missing-anchor host object resize should return command failure");
    expect(visual_object_property(missing_anchor_path, "name-guid", "WIDTH") == "30" &&
            visual_object_property(missing_anchor_path, "name-guid", "HEIGHT") == "10",
        "#1032: missing-anchor host object resize should not mutate the asset");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_align(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-unique-id", "missing-target",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1032: missing-target host object resize should return command failure");
    expect(visual_object_property(missing_target_path, "name-guid", "WIDTH") == "30" &&
            visual_object_property(missing_target_path, "status-guid", "WIDTH") == "20",
        "#1032: missing-target host object resize should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_align(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--resize-object",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1032: resize-object without resize mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "name-guid", "WIDTH") == "30",
        "#1032: resize-object without resize mode should not mutate the asset");

    const fs::path missing_targets_path = temp_root / "missing_targets.scx";
    write_synthetic_form_table_for_object_align(missing_targets_path);
    const auto missing_targets_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_targets_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-object-name", "cmdAnchor",
            "--json"
        },
        temp_root);
    expect(missing_targets_process.exit_code == 2,
        "#1032: resize-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_targets_path, "name-guid", "WIDTH") == "30",
        "#1032: resize-object without target selectors should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_align(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--resize-object",
            "--align-object",
            "--resize-mode", "width",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-unique-id", "name-guid",
            "--align-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1032: resize-object plus align-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "name-guid", "WIDTH") == "30",
        "#1032: resize-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_distribute_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path distribute_path = temp_root / "distribute.scx";
    write_synthetic_form_table_for_object_distribute(distribute_path);
    const auto distribute_process = run_process_capture(
        studio_host_path,
        {
            "--path", distribute_path.string(),
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-object-name", "cmdLeft",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-object-name", "cmdRight",
            "--json"
        },
        temp_root);
    expect(distribute_process.exit_code == 0,
        "#1033: host object distribution should exit successfully");
    expect(visual_object_property(distribute_path, "left-guid", "HPOS") == "10" &&
            visual_object_property(distribute_path, "middle-guid", "HPOS") == "60" &&
            visual_object_property(distribute_path, "right-guid", "HPOS") == "110",
        "#1033: host object distribution should evenly position the middle object");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_distribute(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "missing-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1033: missing-target host object distribution should return command failure");
    expect(visual_object_property(missing_target_path, "middle-guid", "HPOS") == "90",
        "#1033: missing-target host object distribution should not mutate the asset");

    const fs::path too_few_path = temp_root / "too_few.scx";
    write_synthetic_form_table_for_object_distribute(too_few_path);
    const auto too_few_process = run_process_capture(
        studio_host_path,
        {
            "--path", too_few_path.string(),
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(too_few_process.exit_code == 4,
        "#1033: too-few-target host object distribution should return command failure");
    expect(visual_object_property(too_few_path, "middle-guid", "HPOS") == "90",
        "#1033: too-few-target host object distribution should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_distribute(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--distribute-object",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1033: distribute-object without distribution mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "middle-guid", "HPOS") == "90",
        "#1033: distribute-object without distribution mode should not mutate the asset");

    const fs::path unsupported_mode_path = temp_root / "unsupported_mode.scx";
    write_synthetic_form_table_for_object_distribute(unsupported_mode_path);
    const auto unsupported_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", unsupported_mode_path.string(),
            "--distribute-object",
            "--distribution-mode", "diagonal",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_mode_process.exit_code == 4,
        "#1033: unsupported-mode host object distribution should return command failure");
    expect(visual_object_property(unsupported_mode_path, "middle-guid", "HPOS") == "90",
        "#1033: unsupported-mode host object distribution should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_distribute(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--distribute-object",
            "--resize-object",
            "--distribution-mode", "horizontal",
            "--resize-mode", "width",
            "--anchor-unique-id", "left-guid",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-unique-id", "right-guid",
            "--resize-target-unique-id", "middle-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1033: distribute-object plus resize-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "middle-guid", "HPOS") == "90",
        "#1033: distribute-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_snap_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path snap_path = temp_root / "snap.scx";
    write_synthetic_form_table_for_object_snap(snap_path);
    const auto snap_process = run_process_capture(
        studio_host_path,
        {
            "--path", snap_path.string(),
            "--snap-object",
            "--snap-mode", "both",
            "--grid-width", "10",
            "--grid-height", "25",
            "--snap-target-object-name", "cmdOne",
            "--snap-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(snap_process.exit_code == 0,
        "#1034: host object snap should exit successfully");
    expect(visual_object_property(snap_path, "one-guid", "HPOS") == "10" &&
            visual_object_property(snap_path, "one-guid", "VPOS") == "25" &&
            visual_object_property(snap_path, "two-guid", "HPOS") == "40" &&
            visual_object_property(snap_path, "two-guid", "VPOS") == "50" &&
            visual_object_property(snap_path, "other-guid", "HPOS") == "77",
        "#1034: host object snap should round selected coordinates and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_snap(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--snap-object",
            "--snap-mode", "horizontal",
            "--grid-width", "10",
            "--snap-target-unique-id", "one-guid",
            "--snap-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1034: missing-target host object snap should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HPOS") == "13.2" &&
            visual_object_property(missing_target_path, "two-guid", "HPOS") == "36",
        "#1034: missing-target host object snap should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_snap(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--snap-object",
            "--grid-width", "10",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1034: snap-object without snap mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "one-guid", "HPOS") == "13.2",
        "#1034: snap-object without snap mode should not mutate the asset");

    const fs::path invalid_grid_path = temp_root / "invalid_grid.scx";
    write_synthetic_form_table_for_object_snap(invalid_grid_path);
    const auto invalid_grid_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_grid_path.string(),
            "--snap-object",
            "--snap-mode", "horizontal",
            "--grid-width", "0",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_grid_process.exit_code == 4,
        "#1034: invalid-grid host object snap should return command failure");
    expect(visual_object_property(invalid_grid_path, "one-guid", "HPOS") == "13.2",
        "#1034: invalid-grid host object snap should not mutate the asset");

    const fs::path unsupported_mode_path = temp_root / "unsupported_mode.scx";
    write_synthetic_form_table_for_object_snap(unsupported_mode_path);
    const auto unsupported_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", unsupported_mode_path.string(),
            "--snap-object",
            "--snap-mode", "diagonal",
            "--grid-width", "10",
            "--grid-height", "25",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_mode_process.exit_code == 4,
        "#1034: unsupported-mode host object snap should return command failure");
    expect(visual_object_property(unsupported_mode_path, "one-guid", "HPOS") == "13.2",
        "#1034: unsupported-mode host object snap should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_snap(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--snap-object",
            "--distribute-object",
            "--snap-mode", "horizontal",
            "--grid-width", "10",
            "--distribution-mode", "horizontal",
            "--snap-target-unique-id", "one-guid",
            "--distribute-target-unique-id", "one-guid",
            "--distribute-target-unique-id", "two-guid",
            "--distribute-target-unique-id", "other-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1034: snap-object plus distribute-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HPOS") == "13.2",
        "#1034: snap-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_nudges_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_nudge_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path nudge_path = temp_root / "nudge.scx";
    write_synthetic_form_table_for_object_nudge(nudge_path);
    const auto nudge_process = run_process_capture(
        studio_host_path,
        {
            "--path", nudge_path.string(),
            "--nudge-object",
            "--nudge-mode", "both",
            "--delta-hpos", "5",
            "--delta-vpos", "-2.5",
            "--nudge-target-object-name", "cmdOne",
            "--nudge-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(nudge_process.exit_code == 0,
        "#1035: host object nudge should exit successfully");
    expect(visual_object_property(nudge_path, "one-guid", "HPOS") == "15" &&
            visual_object_property(nudge_path, "one-guid", "VPOS") == "17.5" &&
            visual_object_property(nudge_path, "two-guid", "HPOS") == "38.5" &&
            visual_object_property(nudge_path, "two-guid", "VPOS") == "42" &&
            visual_object_property(nudge_path, "other-guid", "HPOS") == "77",
        "#1035: host object nudge should move selected coordinates and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_nudge(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--nudge-object",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--nudge-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1035: missing-target host object nudge should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HPOS") == "10" &&
            visual_object_property(missing_target_path, "two-guid", "HPOS") == "33.5",
        "#1035: missing-target host object nudge should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_nudge(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--nudge-object",
            "--delta-hpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1035: nudge-object without nudge mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "one-guid", "HPOS") == "10",
        "#1035: nudge-object without nudge mode should not mutate the asset");

    const fs::path zero_delta_path = temp_root / "zero_delta.scx";
    write_synthetic_form_table_for_object_nudge(zero_delta_path);
    const auto zero_delta_process = run_process_capture(
        studio_host_path,
        {
            "--path", zero_delta_path.string(),
            "--nudge-object",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "0",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(zero_delta_process.exit_code == 4,
        "#1035: zero-delta host object nudge should return command failure");
    expect(visual_object_property(zero_delta_path, "one-guid", "HPOS") == "10",
        "#1035: zero-delta host object nudge should not mutate the asset");

    const fs::path unsupported_mode_path = temp_root / "unsupported_mode.scx";
    write_synthetic_form_table_for_object_nudge(unsupported_mode_path);
    const auto unsupported_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", unsupported_mode_path.string(),
            "--nudge-object",
            "--nudge-mode", "diagonal",
            "--delta-hpos", "1",
            "--delta-vpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_mode_process.exit_code == 4,
        "#1035: unsupported-mode host object nudge should return command failure");
    expect(visual_object_property(unsupported_mode_path, "one-guid", "HPOS") == "10",
        "#1035: unsupported-mode host object nudge should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_nudge(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--nudge-object",
            "--snap-object",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "1",
            "--snap-mode", "horizontal",
            "--grid-width", "10",
            "--nudge-target-unique-id", "one-guid",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1035: nudge-object plus snap-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HPOS") == "10",
        "#1035: nudge-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_tab_order_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_tab_order_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path tab_order_path = temp_root / "tab_order.scx";
    write_synthetic_form_table_for_object_tab_order(tab_order_path);
    const auto tab_order_process = run_process_capture(
        studio_host_path,
        {
            "--path", tab_order_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "5",
            "--tab-order-target-unique-id", "two-guid",
            "--tab-order-target-object-name", "cmdOne",
            "--tab-order-target-unique-id", "three-guid",
            "--json"
        },
        temp_root);
    expect(tab_order_process.exit_code == 0,
        "#1036: host object tab-order assignment should exit successfully");
    expect(visual_object_property(tab_order_path, "two-guid", "TABINDEX") == "5" &&
            visual_object_property(tab_order_path, "one-guid", "TABINDEX") == "6" &&
            visual_object_property(tab_order_path, "three-guid", "TABINDEX") == "7" &&
            visual_object_property(tab_order_path, "other-guid", "TABINDEX") == "99",
        "#1036: host object tab-order assignment should assign sequential indexes and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_tab_order(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--tab-order-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1036: missing-target host object tab-order assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TABINDEX") == "10" &&
            visual_object_property(missing_target_path, "two-guid", "TABINDEX") == "20",
        "#1036: missing-target host object tab-order assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_tab_order(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "0",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1036: tab-order-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TABINDEX") == "10",
        "#1036: tab-order-object without target selectors should not mutate the asset");

    const fs::path negative_start_path = temp_root / "negative_start.scx";
    write_synthetic_form_table_for_object_tab_order(negative_start_path);
    const auto negative_start_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_start_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "-1",
            "--tab-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_start_process.exit_code == 2,
        "#1036: negative-start host object tab-order assignment should fail during launch parsing");
    expect(visual_object_property(negative_start_path, "one-guid", "TABINDEX") == "10",
        "#1036: negative-start host object tab-order assignment should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_tab_order(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--tab-order-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1036: duplicate-target host object tab-order assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TABINDEX") == "10",
        "#1036: duplicate-target host object tab-order assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_tab_order(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--tab-order-object",
            "--nudge-object",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1036: tab-order-object plus nudge-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TABINDEX") == "10",
        "#1036: tab-order-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_tab_stop_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_tab_stop_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path tab_stop_path = temp_root / "tab_stop.scx";
    write_synthetic_form_table_for_object_tab_stop(tab_stop_path);
    const auto tab_stop_process = run_process_capture(
        studio_host_path,
        {
            "--path", tab_stop_path.string(),
            "--tab-stop-object",
            "--tab-stop", "false",
            "--tab-stop-target-object-name", "cmdOne",
            "--tab-stop-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(tab_stop_process.exit_code == 0,
        "#1037: host object tab-stop assignment should exit successfully");
    expect(visual_object_property(tab_stop_path, "one-guid", "TABSTOP") == ".F." &&
            visual_object_property(tab_stop_path, "two-guid", "TABSTOP") == ".F." &&
            visual_object_property(tab_stop_path, "three-guid", "TABSTOP") == ".F." &&
            visual_object_property(tab_stop_path, "other-guid", "TABSTOP") == ".T.",
        "#1037: host object tab-stop assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_tab_stop(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--tab-stop-object",
            "--tab-stop", "false",
            "--tab-stop-target-unique-id", "one-guid",
            "--tab-stop-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1037: missing-target host object tab-stop assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TABSTOP") == ".T." &&
            visual_object_property(missing_target_path, "two-guid", "TABSTOP") == ".T.",
        "#1037: missing-target host object tab-stop assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_tab_stop(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--tab-stop-object",
            "--tab-stop", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1037: tab-stop-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: tab-stop-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_tab_stop(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--tab-stop-object",
            "--tab-stop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1037: tab-stop-object without tab-stop value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: tab-stop-object without tab-stop value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_tab_stop(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--tab-stop-object",
            "--tab-stop", "false",
            "--tab-stop-target-unique-id", "one-guid",
            "--tab-stop-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1037: duplicate-target host object tab-stop assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: duplicate-target host object tab-stop assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_tab_stop(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--tab-stop-object",
            "--tab-order-object",
            "--tab-stop", "false",
            "--tab-stop-target-unique-id", "one-guid",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1037: tab-stop-object plus tab-order-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: tab-stop-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_visibility_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visibility_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path visibility_path = temp_root / "visibility.scx";
    write_synthetic_form_table_for_object_visibility(visibility_path);
    const auto visibility_process = run_process_capture(
        studio_host_path,
        {
            "--path", visibility_path.string(),
            "--visibility-object",
            "--visible", "false",
            "--visibility-target-object-name", "cmdOne",
            "--visibility-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(visibility_process.exit_code == 0,
        "#1038: host object visibility assignment should exit successfully");
    expect(visual_object_property(visibility_path, "one-guid", "VISIBLE") == ".F." &&
            visual_object_property(visibility_path, "two-guid", "VISIBLE") == ".F." &&
            visual_object_property(visibility_path, "three-guid", "VISIBLE") == ".F." &&
            visual_object_property(visibility_path, "other-guid", "VISIBLE") == ".T.",
        "#1038: host object visibility assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_visibility(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--visibility-object",
            "--visible", "false",
            "--visibility-target-unique-id", "one-guid",
            "--visibility-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1038: missing-target host object visibility assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "VISIBLE") == ".T." &&
            visual_object_property(missing_target_path, "two-guid", "VISIBLE") == ".T.",
        "#1038: missing-target host object visibility assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_visibility(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--visibility-object",
            "--visible", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1038: visibility-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: visibility-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_visibility(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--visibility-object",
            "--visibility-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1038: visibility-object without visible value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: visibility-object without visible value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_visibility(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--visibility-object",
            "--visible", "false",
            "--visibility-target-unique-id", "one-guid",
            "--visibility-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1038: duplicate-target host object visibility assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: duplicate-target host object visibility assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_visibility(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--visibility-object",
            "--tab-stop-object",
            "--visible", "false",
            "--visibility-target-unique-id", "one-guid",
            "--tab-stop", "true",
            "--tab-stop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1038: visibility-object plus tab-stop-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: visibility-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_enabled_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_enabled_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path enabled_path = temp_root / "enabled.scx";
    write_synthetic_form_table_for_object_enabled(enabled_path);
    const auto enabled_process = run_process_capture(
        studio_host_path,
        {
            "--path", enabled_path.string(),
            "--enabled-object",
            "--enabled", "false",
            "--enabled-target-object-name", "cmdOne",
            "--enabled-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(enabled_process.exit_code == 0,
        "#1039: host object enabled assignment should exit successfully");
    expect(visual_object_property(enabled_path, "one-guid", "ENABLED") == ".F." &&
            visual_object_property(enabled_path, "two-guid", "ENABLED") == ".F." &&
            visual_object_property(enabled_path, "three-guid", "ENABLED") == ".F." &&
            visual_object_property(enabled_path, "other-guid", "ENABLED") == ".T.",
        "#1039: host object enabled assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_enabled(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--enabled-object",
            "--enabled", "false",
            "--enabled-target-unique-id", "one-guid",
            "--enabled-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1039: missing-target host object enabled assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ENABLED") == ".T." &&
            visual_object_property(missing_target_path, "two-guid", "ENABLED") == ".T.",
        "#1039: missing-target host object enabled assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_enabled(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--enabled-object",
            "--enabled", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1039: enabled-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ENABLED") == ".T.",
        "#1039: enabled-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_enabled(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--enabled-object",
            "--enabled-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1039: enabled-object without enabled value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ENABLED") == ".T.",
        "#1039: enabled-object without enabled value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_enabled(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--enabled-object",
            "--enabled", "false",
            "--enabled-target-unique-id", "one-guid",
            "--enabled-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1039: duplicate-target host object enabled assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ENABLED") == ".T.",
        "#1039: duplicate-target host object enabled assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_enabled(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--enabled-object",
            "--visibility-object",
            "--enabled", "false",
            "--enabled-target-unique-id", "one-guid",
            "--visible", "true",
            "--visibility-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1039: enabled-object plus visibility-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ENABLED") == ".T.",
        "#1039: enabled-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_read_only_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_read_only_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path read_only_path = temp_root / "read_only.scx";
    write_synthetic_form_table_for_object_read_only(read_only_path);
    const auto read_only_process = run_process_capture(
        studio_host_path,
        {
            "--path", read_only_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--read-only-target-object-name", "txtOne",
            "--read-only-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(read_only_process.exit_code == 0,
        "#1040: host object read-only assignment should exit successfully");
    expect(visual_object_property(read_only_path, "one-guid", "READONLY") == ".T." &&
            visual_object_property(read_only_path, "two-guid", "READONLY") == ".T." &&
            visual_object_property(read_only_path, "three-guid", "READONLY") == ".T." &&
            visual_object_property(read_only_path, "other-guid", "READONLY") == ".F.",
        "#1040: host object read-only assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_read_only(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--read-only-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1040: missing-target host object read-only assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "READONLY") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "READONLY") == ".F.",
        "#1040: missing-target host object read-only assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_read_only(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1040: read-only-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "READONLY") == ".F.",
        "#1040: read-only-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_read_only(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--read-only-object",
            "--read-only-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1040: read-only-object without read-only value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "READONLY") == ".F.",
        "#1040: read-only-object without read-only value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_read_only(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--read-only-target-object-name", "txtOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1040: duplicate-target host object read-only assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "READONLY") == ".F.",
        "#1040: duplicate-target host object read-only assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_read_only(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--read-only-object",
            "--enabled-object",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--enabled", "true",
            "--enabled-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1040: read-only-object plus enabled-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "READONLY") == ".F.",
        "#1040: read-only-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_locked_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_locked_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path locked_path = temp_root / "locked.scx";
    write_synthetic_form_table_for_object_locked(locked_path);
    const auto locked_process = run_process_capture(
        studio_host_path,
        {
            "--path", locked_path.string(),
            "--locked-object",
            "--locked", "true",
            "--locked-target-object-name", "txtOne",
            "--locked-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(locked_process.exit_code == 0,
        "#1041: host object locked assignment should exit successfully");
    expect(visual_object_property(locked_path, "one-guid", "LOCKED") == ".T." &&
            visual_object_property(locked_path, "two-guid", "LOCKED") == ".T." &&
            visual_object_property(locked_path, "three-guid", "LOCKED") == ".T." &&
            visual_object_property(locked_path, "other-guid", "LOCKED") == ".F.",
        "#1041: host object locked assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_locked(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--locked-object",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--locked-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1041: missing-target host object locked assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LOCKED") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "LOCKED") == ".F.",
        "#1041: missing-target host object locked assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_locked(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--locked-object",
            "--locked", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1041: locked-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LOCKED") == ".F.",
        "#1041: locked-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_locked(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--locked-object",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1041: locked-object without locked value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LOCKED") == ".F.",
        "#1041: locked-object without locked value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_locked(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--locked-object",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--locked-target-object-name", "txtOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1041: duplicate-target host object locked assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LOCKED") == ".F.",
        "#1041: duplicate-target host object locked assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_locked(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--locked-object",
            "--read-only-object",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1041: locked-object plus read-only-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LOCKED") == ".F.",
        "#1041: locked-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
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

void test_studio_host_json_assigns_tooltip_text_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_tooltip_text_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path tooltip_text_path = temp_root / "tooltip_text.scx";
    write_synthetic_form_table_for_object_tooltip_text(tooltip_text_path);
    const auto tooltip_text_process = run_process_capture(
        studio_host_path,
        {
            "--path", tooltip_text_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-object-name", "cmdSave",
            "--tooltip-text-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(tooltip_text_process.exit_code == 0,
        "#1043: host object tooltip text assignment should exit successfully");
    expect(visual_object_property(tooltip_text_path, "one-guid", "TOOLTIPTEXT") == "Save this customer" &&
            visual_object_property(tooltip_text_path, "two-guid", "TOOLTIPTEXT") == "Save this customer" &&
            visual_object_property(tooltip_text_path, "three-guid", "TOOLTIPTEXT") == "Ready" &&
            visual_object_property(tooltip_text_path, "other-guid", "TOOLTIPTEXT") == "Other",
        "#1043: host object tooltip text assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_tooltip_text(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--tooltip-text-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1043: missing-target host object tooltip text assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TOOLTIPTEXT") == "Save" &&
            visual_object_property(missing_target_path, "two-guid", "TOOLTIPTEXT") == "Cancel",
        "#1043: missing-target host object tooltip text assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_tooltip_text(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1043: tooltip-text-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: tooltip-text-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_tooltip_text(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--tooltip-text-object",
            "--tooltip-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1043: tooltip-text-object without tooltip text value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: tooltip-text-object without tooltip text value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_tooltip_text(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--tooltip-text-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1043: duplicate-target host object tooltip text assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: duplicate-target host object tooltip text assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_tooltip_text(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--tooltip-text-object",
            "--caption-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1043: tooltip-text-object plus caption-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: tooltip-text-object ambiguity should not mutate the asset");

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

void test_studio_host_json_assigns_control_source_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_control_source_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path control_source_path = temp_root / "control_source.scx";
    write_synthetic_form_table_for_object_control_source(control_source_path);
    const auto control_source_process = run_process_capture(
        studio_host_path,
        {
            "--path", control_source_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-object-name", "txtName",
            "--control-source-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(control_source_process.exit_code == 0,
        "#1045: host object control-source assignment should exit successfully");
    expect(visual_object_property(control_source_path, "one-guid", "CONTROLSOURCE") == "ThisForm.Current Customer" &&
            visual_object_property(control_source_path, "two-guid", "CONTROLSOURCE") == "ThisForm.Current Customer" &&
            visual_object_property(control_source_path, "three-guid", "CONTROLSOURCE") == "Ready" &&
            visual_object_property(control_source_path, "other-guid", "CONTROLSOURCE") == "customers.state",
        "#1045: host object control-source assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_control_source(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-unique-id", "one-guid",
            "--control-source-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1045: missing-target host object control-source assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CONTROLSOURCE") == "customers.name" &&
            visual_object_property(missing_target_path, "two-guid", "CONTROLSOURCE") == "customers.city",
        "#1045: missing-target host object control-source assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_control_source(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1045: control-source-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: control-source-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_control_source(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--control-source-object",
            "--control-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1045: control-source-object without control-source value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: control-source-object without control-source value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_control_source(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-unique-id", "one-guid",
            "--control-source-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1045: duplicate-target host object control-source assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: duplicate-target host object control-source assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_control_source(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--control-source-object",
            "--status-bar-text-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-unique-id", "one-guid",
            "--status-bar-text", "Ready",
            "--status-bar-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1045: control-source-object plus status-bar-text-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: control-source-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_input_mask_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_input_mask_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path input_mask_path = temp_root / "input_mask.scx";
    write_synthetic_form_table_for_object_input_mask(input_mask_path);
    const auto input_mask_process = run_process_capture(
        studio_host_path,
        {
            "--path", input_mask_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-object-name", "txtPhone",
            "--input-mask-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(input_mask_process.exit_code == 0,
        "#1046: host object input-mask assignment should exit successfully");
    expect(visual_object_property(input_mask_path, "one-guid", "INPUTMASK") == "AA 9999" &&
            visual_object_property(input_mask_path, "two-guid", "INPUTMASK") == "AA 9999" &&
            visual_object_property(input_mask_path, "three-guid", "INPUTMASK") == "Ready" &&
            visual_object_property(input_mask_path, "other-guid", "INPUTMASK") == "XXXXXXXX",
        "#1046: host object input-mask assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_input_mask(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-unique-id", "one-guid",
            "--input-mask-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1046: missing-target host object input-mask assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "INPUTMASK") == "(999) 999-9999" &&
            visual_object_property(missing_target_path, "two-guid", "INPUTMASK") == "99999",
        "#1046: missing-target host object input-mask assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_input_mask(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1046: input-mask-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: input-mask-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_input_mask(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--input-mask-object",
            "--input-mask-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1046: input-mask-object without input-mask value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: input-mask-object without input-mask value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_input_mask(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-unique-id", "one-guid",
            "--input-mask-target-object-name", "txtPhone",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1046: duplicate-target host object input-mask assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: duplicate-target host object input-mask assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_input_mask(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--input-mask-object",
            "--control-source-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-unique-id", "one-guid",
            "--control-source", "customers.name",
            "--control-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1046: input-mask-object plus control-source-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: input-mask-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_format_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_format_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path format_path = temp_root / "format.scx";
    write_synthetic_form_table_for_object_format(format_path);
    const auto format_process = run_process_capture(
        studio_host_path,
        {
            "--path", format_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--format-target-object-name", "txtAmount",
            "--format-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(format_process.exit_code == 0,
        "#1047: host object format assignment should exit successfully");
    expect(visual_object_property(format_path, "one-guid", "FORMAT") == "@R 999,999.99" &&
            visual_object_property(format_path, "two-guid", "FORMAT") == "@R 999,999.99" &&
            visual_object_property(format_path, "three-guid", "FORMAT") == "Ready" &&
            visual_object_property(format_path, "other-guid", "FORMAT") == "!",
        "#1047: host object format assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_format(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--format-target-unique-id", "one-guid",
            "--format-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1047: missing-target host object format assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FORMAT") == "999,999.99" &&
            visual_object_property(missing_target_path, "two-guid", "FORMAT") == "99.99%",
        "#1047: missing-target host object format assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_format(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1047: format-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: format-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_format(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--format-object",
            "--format-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1047: format-object without format value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: format-object without format value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_format(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--format-target-unique-id", "one-guid",
            "--format-target-object-name", "txtAmount",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1047: duplicate-target host object format assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: duplicate-target host object format assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_format(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--format-object",
            "--input-mask-object",
            "--format", "@R 999,999.99",
            "--format-target-unique-id", "one-guid",
            "--input-mask", "99999",
            "--input-mask-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1047: format-object plus input-mask-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: format-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_row_source_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_row_source_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path row_source_path = temp_root / "row_source.scx";
    write_synthetic_form_table_for_object_row_source(row_source_path);
    const auto row_source_process = run_process_capture(
        studio_host_path,
        {
            "--path", row_source_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-object-name", "cboCustomer",
            "--row-source-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(row_source_process.exit_code == 0,
        "#1048: host object row-source assignment should exit successfully");
    expect(visual_object_property(row_source_path, "one-guid", "ROWSOURCE") == "products.name,product_id" &&
            visual_object_property(row_source_path, "two-guid", "ROWSOURCE") == "products.name,product_id" &&
            visual_object_property(row_source_path, "three-guid", "ROWSOURCE") == "Ready" &&
            visual_object_property(row_source_path, "other-guid", "ROWSOURCE") == "states.name",
        "#1048: host object row-source assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_row_source(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-unique-id", "one-guid",
            "--row-source-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1048: missing-target host object row-source assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id" &&
            visual_object_property(missing_target_path, "two-guid", "ROWSOURCE") == "orders.order_id,total",
        "#1048: missing-target host object row-source assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_row_source(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1048: row-source-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: row-source-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_row_source(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--row-source-object",
            "--row-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1048: row-source-object without row-source value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: row-source-object without row-source value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_row_source(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-unique-id", "one-guid",
            "--row-source-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1048: duplicate-target host object row-source assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: duplicate-target host object row-source assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_row_source(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--row-source-object",
            "--format-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-unique-id", "one-guid",
            "--format", "!",
            "--format-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1048: row-source-object plus format-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: row-source-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_row_source_type_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_row_source_type_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path row_source_type_path = temp_root / "row_source_type.scx";
    write_synthetic_form_table_for_object_row_source_type(row_source_type_path);
    const auto row_source_type_process = run_process_capture(
        studio_host_path,
        {
            "--path", row_source_type_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--row-source-type-target-object-name", "cboCustomer",
            "--row-source-type-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(row_source_type_process.exit_code == 0,
        "#1049: host object row-source-type assignment should exit successfully");
    expect(visual_object_property(row_source_type_path, "one-guid", "ROWSOURCETYPE") == "6" &&
            visual_object_property(row_source_type_path, "two-guid", "ROWSOURCETYPE") == "6" &&
            visual_object_property(row_source_type_path, "three-guid", "ROWSOURCETYPE") == "0" &&
            visual_object_property(row_source_type_path, "other-guid", "ROWSOURCETYPE") == "5",
        "#1049: host object row-source-type assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_row_source_type(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--row-source-type-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1049: missing-target host object row-source-type assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ROWSOURCETYPE") == "2" &&
            visual_object_property(missing_target_path, "two-guid", "ROWSOURCETYPE") == "3",
        "#1049: missing-target host object row-source-type assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_row_source_type(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1049: row-source-type-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: row-source-type-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_row_source_type(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--row-source-type-object",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1049: row-source-type-object without row-source-type value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: row-source-type-object without row-source-type value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_row_source_type(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--row-source-type-object",
            "--row-source-type", "-1",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1049: negative row-source-type values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: negative row-source-type values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_row_source_type(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--row-source-type-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1049: duplicate-target host object row-source-type assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: duplicate-target host object row-source-type assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_row_source_type(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--row-source-type-object",
            "--row-source-object",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--row-source", "products.name",
            "--row-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1049: row-source-type-object plus row-source-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: row-source-type-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_bound_column_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_bound_column_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path bound_column_path = temp_root / "bound_column.scx";
    write_synthetic_form_table_for_object_bound_column(bound_column_path);
    const auto bound_column_process = run_process_capture(
        studio_host_path,
        {
            "--path", bound_column_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--bound-column-target-object-name", "cboCustomer",
            "--bound-column-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(bound_column_process.exit_code == 0,
        "#1050: host object bound-column assignment should exit successfully");
    expect(visual_object_property(bound_column_path, "one-guid", "BOUNDCOLUMN") == "4" &&
            visual_object_property(bound_column_path, "two-guid", "BOUNDCOLUMN") == "4" &&
            visual_object_property(bound_column_path, "three-guid", "BOUNDCOLUMN") == "0" &&
            visual_object_property(bound_column_path, "other-guid", "BOUNDCOLUMN") == "3",
        "#1050: host object bound-column assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_bound_column(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--bound-column-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1050: missing-target host object bound-column assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BOUNDCOLUMN") == "1" &&
            visual_object_property(missing_target_path, "two-guid", "BOUNDCOLUMN") == "2",
        "#1050: missing-target host object bound-column assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_bound_column(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1050: bound-column-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: bound-column-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_bound_column(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--bound-column-object",
            "--bound-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1050: bound-column-object without bound-column value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: bound-column-object without bound-column value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_bound_column(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--bound-column-object",
            "--bound-column", "-1",
            "--bound-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1050: negative bound-column values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: negative bound-column values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_bound_column(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--bound-column-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1050: duplicate-target host object bound-column assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: duplicate-target host object bound-column assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_bound_column(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--bound-column-object",
            "--row-source-type-object",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1050: bound-column-object plus row-source-type-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: bound-column-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_column_count_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_column_count_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path column_count_path = temp_root / "column_count.scx";
    write_synthetic_form_table_for_object_column_count(column_count_path);
    const auto column_count_process = run_process_capture(
        studio_host_path,
        {
            "--path", column_count_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--column-count-target-object-name", "cboCustomer",
            "--column-count-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(column_count_process.exit_code == 0,
        "#1051: host object column-count assignment should exit successfully");
    expect(visual_object_property(column_count_path, "one-guid", "COLUMNCOUNT") == "5" &&
            visual_object_property(column_count_path, "two-guid", "COLUMNCOUNT") == "5" &&
            visual_object_property(column_count_path, "three-guid", "COLUMNCOUNT") == "0" &&
            visual_object_property(column_count_path, "other-guid", "COLUMNCOUNT") == "4",
        "#1051: host object column-count assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_column_count(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--column-count-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1051: missing-target host object column-count assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "COLUMNCOUNT") == "2" &&
            visual_object_property(missing_target_path, "two-guid", "COLUMNCOUNT") == "3",
        "#1051: missing-target host object column-count assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_column_count(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1051: column-count-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: column-count-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_column_count(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--column-count-object",
            "--column-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1051: column-count-object without column-count value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: column-count-object without column-count value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_column_count(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--column-count-object",
            "--column-count", "-1",
            "--column-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1051: negative column-count values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: negative column-count values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_column_count(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--column-count-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1051: duplicate-target host object column-count assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: duplicate-target host object column-count assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_column_count(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--column-count-object",
            "--bound-column-object",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1051: column-count-object plus bound-column-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: column-count-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_style_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_style_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path style_path = temp_root / "style.scx";
    write_synthetic_form_table_for_object_style(style_path);
    const auto style_process = run_process_capture(
        studio_host_path,
        {
            "--path", style_path.string(),
            "--style-object",
            "--style", "2",
            "--style-target-object-name", "cboCustomer",
            "--style-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(style_process.exit_code == 0,
        "#1052: host object style assignment should exit successfully");
    expect(visual_object_property(style_path, "one-guid", "STYLE") == "2" &&
            visual_object_property(style_path, "two-guid", "STYLE") == "2" &&
            visual_object_property(style_path, "three-guid", "STYLE") == "0" &&
            visual_object_property(style_path, "other-guid", "STYLE") == "2",
        "#1052: host object style assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_style(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--style-object",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--style-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1052: missing-target host object style assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "STYLE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "STYLE") == "1",
        "#1052: missing-target host object style assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_style(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--style-object",
            "--style", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1052: style-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "STYLE") == "0",
        "#1052: style-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_style(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--style-object",
            "--style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1052: style-object without style value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "STYLE") == "0",
        "#1052: style-object without style value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_style(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--style-object",
            "--style", "-1",
            "--style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1052: negative style values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "STYLE") == "0",
        "#1052: negative style values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_style(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--style-object",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--style-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1052: duplicate-target host object style assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "STYLE") == "0",
        "#1052: duplicate-target host object style assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_style(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--style-object",
            "--column-count-object",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1052: style-object plus column-count-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "STYLE") == "0",
        "#1052: style-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_list_index_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_list_index_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path list_index_path = temp_root / "list_index.scx";
    write_synthetic_form_table_for_object_list_index(list_index_path);
    const auto list_index_process = run_process_capture(
        studio_host_path,
        {
            "--path", list_index_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--list-index-target-object-name", "cboCustomer",
            "--list-index-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(list_index_process.exit_code == 0,
        "#1053: host object list-index assignment should exit successfully");
    expect(visual_object_property(list_index_path, "one-guid", "LISTINDEX") == "3" &&
            visual_object_property(list_index_path, "two-guid", "LISTINDEX") == "3" &&
            visual_object_property(list_index_path, "three-guid", "LISTINDEX") == "0" &&
            visual_object_property(list_index_path, "other-guid", "LISTINDEX") == "2",
        "#1053: host object list-index assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_list_index(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--list-index-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1053: missing-target host object list-index assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LISTINDEX") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "LISTINDEX") == "1",
        "#1053: missing-target host object list-index assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_list_index(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1053: list-index-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LISTINDEX") == "0",
        "#1053: list-index-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_list_index(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--list-index-object",
            "--list-index-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1053: list-index-object without list-index value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LISTINDEX") == "0",
        "#1053: list-index-object without list-index value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_list_index(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--list-index-object",
            "--list-index", "-1",
            "--list-index-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1053: negative list-index values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "LISTINDEX") == "0",
        "#1053: negative list-index values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_list_index(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--list-index-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1053: duplicate-target host object list-index assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LISTINDEX") == "0",
        "#1053: duplicate-target host object list-index assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_list_index(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--list-index-object",
            "--style-object",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1053: list-index-object plus style-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LISTINDEX") == "0",
        "#1053: list-index-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_left_column_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_left_column_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path left_column_path = temp_root / "left_column.scx";
    write_synthetic_form_table_for_object_left_column(left_column_path);
    const auto left_column_process = run_process_capture(
        studio_host_path,
        {
            "--path", left_column_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--left-column-target-object-name", "grdCustomer",
            "--left-column-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(left_column_process.exit_code == 0,
        "#1054: host object left-column assignment should exit successfully");
    expect(visual_object_property(left_column_path, "one-guid", "LEFTCOLUMN") == "7" &&
            visual_object_property(left_column_path, "two-guid", "LEFTCOLUMN") == "7" &&
            visual_object_property(left_column_path, "three-guid", "LEFTCOLUMN") == "0" &&
            visual_object_property(left_column_path, "other-guid", "LEFTCOLUMN") == "2",
        "#1054: host object left-column assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_left_column(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--left-column-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1054: missing-target host object left-column assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LEFTCOLUMN") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "LEFTCOLUMN") == "1",
        "#1054: missing-target host object left-column assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_left_column(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1054: left-column-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: left-column-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_left_column(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--left-column-object",
            "--left-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1054: left-column-object without left-column value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: left-column-object without left-column value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_left_column(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--left-column-object",
            "--left-column", "-1",
            "--left-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1054: negative left-column values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: negative left-column values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_left_column(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--left-column-target-object-name", "grdCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1054: duplicate-target host object left-column assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: duplicate-target host object left-column assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_left_column(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--left-column-object",
            "--list-index-object",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1054: left-column-object plus list-index-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: left-column-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_display_value_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_display_value_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path display_value_path = temp_root / "display_value.scx";
    write_synthetic_form_table_for_object_display_value(display_value_path);
    const auto display_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", display_value_path.string(),
            "--display-value-object",
            "--display-value", "Bob \"B\"",
            "--display-value-target-object-name", "cboCustomer",
            "--display-value-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(display_value_process.exit_code == 0,
        "#1055: host object display-value assignment should exit successfully");
    expect(visual_object_property(display_value_path, "one-guid", "DISPLAYVALUE") == "Bob \"B\"" &&
            visual_object_property(display_value_path, "two-guid", "DISPLAYVALUE") == "Bob \"B\"" &&
            visual_object_property(display_value_path, "three-guid", "DISPLAYVALUE") == "Ready" &&
            visual_object_property(display_value_path, "other-guid", "DISPLAYVALUE") == "Other",
        "#1055: host object display-value assignment should assign selected text values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_display_value(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--display-value-object",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--display-value-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1055: missing-target host object display-value assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISPLAYVALUE") == "Alice" &&
            visual_object_property(missing_target_path, "two-guid", "DISPLAYVALUE") == "Order 100",
        "#1055: missing-target host object display-value assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_display_value(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--display-value-object",
            "--display-value", "Bob",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1055: display-value-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: display-value-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_display_value(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--display-value-object",
            "--display-value-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1055: display-value-object without display-value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: display-value-object without display-value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_display_value(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--display-value-object",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--display-value-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1055: duplicate-target host object display-value assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: duplicate-target host object display-value assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_display_value(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--display-value-object",
            "--left-column-object",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1055: display-value-object plus left-column-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: display-value-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ungroups_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_ungroup_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_object_ungroup(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--ungroup-object",
            "--object-name", "cntGroup",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1029: object-name host object ungroup should exit successfully");
    expect(visual_object_is_deleted(object_name_path, "group-guid") &&
            visual_object_parent(object_name_path, "name-guid") == "frmCustomer" &&
            visual_object_parent(object_name_path, "save-guid") == "frmCustomer",
        "#1029: object-name host object ungroup should move children to the container parent and delete the container");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_object_ungroup(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--ungroup-object",
            "--unique-id", "group-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1029: unique-id host object ungroup should exit successfully");
    expect(visual_object_is_deleted(unique_id_path, "group-guid") &&
            visual_object_parent(unique_id_path, "name-guid") == "frmCustomer" &&
            visual_object_parent(unique_id_path, "save-guid") == "frmCustomer",
        "#1029: unique-id host object ungroup should move children to the container parent and delete the container");

    const fs::path root_path = temp_root / "root.scx";
    write_synthetic_form_table_for_object_ungroup(root_path);
    const auto root_process = run_process_capture(
        studio_host_path,
        {
            "--path", root_path.string(),
            "--ungroup-object",
            "--unique-id", "root-group-guid",
            "--json"
        },
        temp_root);
    expect(root_process.exit_code == 0,
        "#1029: root-level host object ungroup should exit successfully");
    expect(visual_object_is_deleted(root_path, "root-group-guid") &&
            visual_object_parent(root_path, "root-child-guid").empty(),
        "#1029: root-level host object ungroup should clear child parents and delete the container");

    const fs::path empty_path = temp_root / "empty.scx";
    write_synthetic_form_table_for_object_ungroup(empty_path);
    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--path", empty_path.string(),
            "--ungroup-object",
            "--unique-id", "empty-guid",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 4,
        "#1029: empty-container host object ungroup should return command failure");
    expect(!visual_object_is_deleted(empty_path, "empty-guid") &&
            visual_object_parent(empty_path, "name-guid") == "cntGroup",
        "#1029: empty-container host object ungroup should not mutate the asset");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_object_ungroup(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--ungroup-object",
            "--unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1029: missing-container host object ungroup should return command failure");
    expect(!visual_object_is_deleted(missing_path, "group-guid") &&
            visual_object_parent(missing_path, "name-guid") == "cntGroup",
        "#1029: missing-container host object ungroup should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ungroup(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ungroup-object",
            "--reorder-object",
            "--unique-id", "group-guid",
            "--placement", "front",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1029: ungroup-object plus reorder-object requests should fail during launch parsing");
    expect(!visual_object_is_deleted(ambiguous_path, "group-guid") &&
            visual_object_parent(ambiguous_path, "name-guid") == "cntGroup",
        "#1029: ungroup-object/reorder-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_json <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_exposes_designer_contexts(argv[1]);
    test_studio_host_json_creates_toolbox_objects(argv[1]);
    test_studio_host_json_sets_properties_by_stable_selectors(argv[1]);
    test_studio_host_json_clears_properties_by_stable_selectors(argv[1]);
    test_studio_host_json_renames_properties_by_stable_selectors(argv[1]);
    test_studio_host_json_deletes_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_restores_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_duplicates_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_renames_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_reparents_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_reorders_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_groups_objects_by_stable_child_selectors(argv[1]);
    test_studio_host_json_aligns_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_resizes_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_distributes_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_snaps_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_nudges_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_tab_order_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_tab_stop_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_visibility_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_enabled_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_read_only_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_locked_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_caption_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_tooltip_text_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_status_bar_text_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_control_source_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_input_mask_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_format_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_row_source_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_row_source_type_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_bound_column_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_column_count_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_style_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_list_index_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_left_column_by_stable_selectors(argv[1]);
    test_studio_host_json_assigns_display_value_by_stable_selectors(argv[1]);
    test_studio_host_json_ungroups_objects_by_stable_selectors(argv[1]);
    return failures == 0 ? 0 : 1;
}
