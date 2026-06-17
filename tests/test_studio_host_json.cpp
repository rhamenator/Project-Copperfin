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
    return failures == 0 ? 0 : 1;
}
