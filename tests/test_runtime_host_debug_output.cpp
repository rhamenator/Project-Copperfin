#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/vfp/dbf_table.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
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

void write_text(const std::filesystem::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
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

    const fs::path stdout_path = working_directory / "runtime_host_stdout.log";
    const fs::path stderr_path = working_directory / "runtime_host_stderr.log";

    std::string command = quote_command_argument(executable_path);
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

std::optional<copperfin::runtime::XAssetActionBinding> find_action(
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& action_id) {
    for (const auto& action : model.actions) {
        if (action.action_id == action_id) {
            return action;
        }
    }
    return std::nullopt;
}

std::size_t find_breakpoint_line_for_routine_statement(
    const std::string& source,
    const std::string& routine_name,
    const std::string& statement_text) {
    std::size_t current_line = 0;
    bool in_target_routine = false;
    std::string line;
    std::size_t line_start = 0;

    while (line_start <= source.size()) {
        const std::size_t line_end = source.find('\n', line_start);
        line = source.substr(
            line_start,
            line_end == std::string::npos ? std::string::npos : line_end - line_start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        ++current_line;
        if (line == "PROCEDURE " + routine_name) {
            in_target_routine = true;
        } else if (in_target_routine && line == statement_text) {
            return current_line;
        } else if (in_target_routine && line == "ENDPROC") {
            break;
        }

        if (line_end == std::string::npos) {
            break;
        }
        line_start = line_end + 1U;
    }

    return 0U;
}

void write_synthetic_form_asset(const std::filesystem::path& table_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "PLATFORM", .type = 'C', .length = 16U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "WINDOWS",
            "Dataenvironment",
            "",
            "dataenvironment",
            "PROCEDURE BeforeOpenTables\r\nSET DELETED ON\r\nENDPROC\r\n"
            "PROCEDURE OpenTables\r\nx = 1\r\nENDPROC\r\n"
            "PROCEDURE CloseTables\r\nCLEAR EVENTS\r\nENDPROC\r\n"
        },
        {
            "WINDOWS",
            "frmDemo",
            "",
            "form",
            "PROCEDURE Load\r\nx = 2\r\nENDPROC\r\n"
            "PROCEDURE Init\r\nx = 3\r\nENDPROC\r\n"
            "PROCEDURE Activate\r\nx = 4\r\nENDPROC\r\n"
            "PROCEDURE Destroy\r\nx = 5\r\nENDPROC\r\n"
        },
        {
            "WINDOWS",
            "pgfMain",
            "frmDemo",
            "pageframe",
            "PROCEDURE Page2.Activate\r\nTHISFORM.Refresh\r\nENDPROC\r\n"
        }
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "synthetic SCX/SCT debugger fixture should be created");
}

void test_runtime_host_reports_xasset_pause_identity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_debug_output_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "runtime-host debugger fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "runtime-host debugger fixture should yield an xAsset executable model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    expect(page_activate.has_value(), "synthetic form fixture should expose the nested page action");
    if (!model.ok || !page_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
    const std::size_t breakpoint_line = find_breakpoint_line_for_routine_statement(
        bootstrap,
        page_activate->routine_name,
        "THISFORM.Refresh");
    expect(breakpoint_line != 0U, "synthetic xAsset bootstrap should contain a breakpointable nested page statement");
    if (breakpoint_line == 0U) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoForm\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--breakpoint", std::to_string(breakpoint_line),
            "--debug-command", "continue",
            "--debug-command", "select:frmdemo.pgfmain.page2.activate"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "runtime host stdout:\n" << process.stdout_text << "\n";
        std::cerr << "runtime host stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host xAsset debugger smoke should exit successfully");
    expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
           "runtime host should report xasset-bootstrap mode");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should pause on the nested xAsset breakpoint");
    expect(process.stdout_text.find("debug.command[1]: select:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report the dispatched xAsset debug command");
    expect(process.stdout_text.find("debug.xasset.action_id: " + page_activate->action_id) != std::string::npos,
           "runtime host pause output should report the originating xAsset action id");
    expect(process.stdout_text.find("debug.xasset.record_index: " + std::to_string(page_activate->record_index)) != std::string::npos,
           "runtime host pause output should report the originating xAsset record index");
    expect(process.stdout_text.find("debug.xasset.kind: " + page_activate->kind) != std::string::npos,
           "runtime host pause output should report the xAsset action kind");
    expect(process.stdout_text.find("debug.xasset.title: " + page_activate->title) != std::string::npos,
           "runtime host pause output should report the xAsset action title");
    expect(process.stdout_text.find("debug.frame[0]: " + page_activate->routine_name + "@") != std::string::npos,
           "runtime host pause stack should still identify the generated action routine");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "FAIL: runtime host executable path argument is required\n";
        return 1;
    }

    test_runtime_host_reports_xasset_pause_identity(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All runtime host debug-output tests passed\n";
    return 0;
}
