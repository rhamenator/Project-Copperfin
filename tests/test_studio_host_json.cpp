#include "copperfin/vfp/dbf_table.h"

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

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_json <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_exposes_designer_contexts(argv[1]);
    return failures == 0 ? 0 : 1;
}
