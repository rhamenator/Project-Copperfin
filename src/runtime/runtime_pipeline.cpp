#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/runtime/xasset_methods.h"
#include "prg_engine_internal.h"
#include "copperfin/security/sha256.h"

#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <unordered_set>

namespace copperfin::runtime {

namespace {

std::string sanitize_file_name(const std::string& value) {
    std::string sanitized;
    sanitized.reserve(value.size());
    for (const char ch : value) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_') {
            sanitized.push_back(ch);
        } else {
            sanitized.push_back('_');
        }
    }
    return sanitized.empty() ? "copperfin_app" : sanitized;
}

std::string trim_copy(std::string value) {
    const auto is_space = [](unsigned char ch) {
        return std::isspace(ch) != 0;
    };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char ch) {
        return !is_space(ch);
    }));
    while (!value.empty() && is_space(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

BuildOutputKind parse_build_output_kind(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "dll") {
        return BuildOutputKind::dll;
    }
    if (normalized == "app") {
        return BuildOutputKind::app;
    }
    if (normalized == "fll") {
        return BuildOutputKind::fll;
    }
    if (normalized == "fxp") {
        return BuildOutputKind::fxp;
    }
    if (normalized == "ocx") {
        return BuildOutputKind::ocx;
    }
    if (normalized == "executable") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
}

std::string quote_manifest_value(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        if (ch == '\\') {
            escaped += "\\\\";
        } else if (ch == '\n') {
            escaped += "\\n";
        } else if (ch == '\r') {
            escaped += "\\r";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
}

std::vector<std::string> unique_non_empty_paths_preserve_order(std::initializer_list<std::string> values) {
    std::vector<std::string> normalized_values;
    normalized_values.reserve(values.size());
    std::unordered_set<std::string> seen;
    for (const std::string& value : values) {
        if (value.empty()) {
            continue;
        }
        const std::string normalized = std::filesystem::path(value).lexically_normal().string();
        if (normalized.empty() || !seen.insert(normalized).second) {
            continue;
        }
        normalized_values.push_back(normalized);
    }
    return normalized_values;
}

bool write_text_file(const std::filesystem::path& path, const std::string& contents, std::string& error) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        error = "Unable to create file: " + path.string();
        return false;
    }

    output << contents;
    if (!output.good()) {
        error = "Unable to write file: " + path.string();
        return false;
    }

    return true;
}

std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
}

std::string read_binary_file(const std::filesystem::path& path, std::string& error) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        error = "Unable to open file: " + path.string();
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string hex_encode_bytes(const std::string& bytes) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (const unsigned char byte : bytes) {
        stream << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return stream.str();
}

bool append_runtime_artifact_digest(
    std::vector<RuntimeArtifactDigest>& digests,
    const std::string& path,
    std::string& error) {
    if (trim_copy(path).empty() || !std::filesystem::exists(path)) {
        return true;
    }

    const auto digest = security::sha256_hex_for_file(path);
    if (!digest.ok) {
        error = digest.error;
        return false;
    }

    const auto existing = std::find_if(digests.begin(), digests.end(), [&](const RuntimeArtifactDigest& entry) {
        return entry.path == path;
    });
    if (existing != digests.end()) {
        existing->sha256 = digest.hex_digest;
        return true;
    }

    digests.push_back({
        .path = path,
        .sha256 = digest.hex_digest
    });
    return true;
}

bool is_library_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::dll ||
        output_kind == BuildOutputKind::fll ||
        output_kind == BuildOutputKind::ocx;
}

bool is_native_host_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::executable ||
        output_kind == BuildOutputKind::unknown;
}

std::string resolve_output_file_name(const studio::StudioProjectWorkspace& workspace, const std::string& project_title) {
    const std::filesystem::path configured_output(workspace.build_plan.output_path);
    const std::string file_name = configured_output.filename().string();
    if (!trim_copy(file_name).empty()) {
        return file_name;
    }
    return sanitize_file_name(project_title) + ".exe";
}

BuildOutputKind infer_build_output_kind_from_output_path(const std::string& output_path) {
    const std::string extension = lowercase_copy(trim_copy(std::filesystem::path(output_path).extension().string()));
    if (extension == ".dll") {
        return BuildOutputKind::dll;
    }
    if (extension == ".app") {
        return BuildOutputKind::app;
    }
    if (extension == ".fll") {
        return BuildOutputKind::fll;
    }
    if (extension == ".fxp") {
        return BuildOutputKind::fxp;
    }
    if (extension == ".ocx") {
        return BuildOutputKind::ocx;
    }
    if (extension == ".exe") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
}

std::string normalize_export_symbol(std::string value) {
    value = trim_copy(std::move(value));
    const std::size_t whitespace = value.find_first_of(" \t(");
    if (whitespace != std::string::npos) {
        value = trim_copy(value.substr(0U, whitespace));
    }
    return value;
}

std::string json_escape(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

std::vector<std::string> collect_library_exported_symbols(const RuntimePackagePlan& plan) {
    std::vector<std::string> exported_symbols;
    std::unordered_set<std::string> seen;

    for (const auto& asset : plan.assets) {
        if (asset.excluded || !asset.exists) {
            continue;
        }

        if (lowercase_copy(std::filesystem::path(asset.source_path).extension().string()) != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }
            exported_symbols.push_back(export_name);
        }
    }

    return exported_symbols;
}

std::string build_module_definition_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string output_stem =
        std::filesystem::path(plan.launcher_output_path).stem().string();
    stream << "LIBRARY " << output_stem << "\n";
    stream << "EXPORTS\n";
    for (const auto& symbol : plan.exported_symbols) {
        stream << "    " << symbol << "\n";
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        stream << "    FoxInfo\n";
    }
    return stream.str();
}

std::string build_native_wrapper_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "// Generated Copperfin native wrapper scaffold\n";
    stream << "// This is an honest bridge scaffold, not a finished FoxPro/VFP-compatible runtime wrapper.\n";
    stream << "#if defined(_WIN32)\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\" __declspec(dllexport)\n";
    stream << "#else\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\"\n";
    stream << "#endif\n\n";

    for (const auto& symbol : plan.exported_symbols) {
        stream << "COPPERFIN_EXPORT int " << symbol << "() {\n";
        stream << "    return -1;\n";
        stream << "}\n\n";
    }

    if (plan.output_kind == BuildOutputKind::fll) {
        stream << "COPPERFIN_EXPORT int FoxInfo() {\n";
        stream << "    return -1;\n";
        stream << "}\n";
    }

    return stream.str();
}

std::string build_native_wrapper_cmake_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string output_stem =
        std::filesystem::path(plan.launcher_output_path).stem().string();
    const std::string output_extension =
        std::filesystem::path(plan.launcher_output_path).extension().string();
    const std::string output_directory = "..";
    const std::string wrapper_file_name =
        std::filesystem::path(plan.native_wrapper_source_path).filename().string();
    const std::string module_definition_file_name =
        std::filesystem::path(plan.module_definition_path).filename().string();

    stream << "cmake_minimum_required(VERSION 3.20)\n";
    stream << "project(" << output_stem << "Wrapper LANGUAGES CXX)\n\n";
    stream << "add_library(" << output_stem << " SHARED " << wrapper_file_name << ")\n";
    stream << "target_compile_features(" << output_stem << " PRIVATE cxx_std_20)\n";
    stream << "set_target_properties(" << output_stem
           << " PROPERTIES OUTPUT_NAME \"" << output_stem
           << "\" PREFIX \"\" SUFFIX \"" << output_extension
           << "\" LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory
           << "\" RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory
           << "\" ARCHIVE_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory << "\")\n";
    stream << "if(MSVC)\n";
    stream << "  target_link_options(" << output_stem
           << " PRIVATE \"/DEF:${CMAKE_CURRENT_SOURCE_DIR}/../" << module_definition_file_name << "\")\n";
    stream << "endif()\n";
    return stream.str();
}

std::string build_native_wrapper_shell_script_source() {
    std::ostringstream stream;
    stream << "#!/usr/bin/env sh\n";
    stream << "set -eu\n";
    stream << "SCRIPT_DIR=\"$(CDPATH= cd -- \"$(dirname -- \"$0\")\" && pwd)\"\n";
    stream << "cmake -S \"$SCRIPT_DIR\" -B \"$SCRIPT_DIR/build\"\n";
    stream << "cmake --build \"$SCRIPT_DIR/build\"\n";
    return stream.str();
}

std::string build_native_wrapper_powershell_script_source() {
    std::ostringstream stream;
    stream << "$ErrorActionPreference = 'Stop'\n";
    stream << "$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path\n";
    stream << "$buildDir = Join-Path $scriptDir 'build'\n";
    stream << "cmake -S $scriptDir -B $buildDir\n";
    stream << "cmake --build $buildDir\n";
    return stream.str();
}

std::string build_fll_api_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "output_kind=fll\n";
    stream << "library_file=" << quote_manifest_value(std::filesystem::path(plan.launcher_output_path).filename().string()) << "\n";
    stream << "registration_command=SET LIBRARY TO\n";
    stream << "release_command=RELEASE LIBRARY\n";
    stream << "additive_supported=true\n";
    stream << "loader_entrypoint=FoxInfo\n";
    for (const auto& symbol : plan.exported_symbols) {
        stream << "function=" << quote_manifest_value(symbol) << "\n";
    }
    return stream.str();
}

void append_fxp_statement_lines(
    std::ostringstream& stream,
    const std::string& scope_name,
    const std::vector<Statement>& statements) {
    for (const auto& statement : statements) {
        stream << "statement="
               << quote_manifest_value(scope_name) << "|"
               << statement.location.line << "|"
               << quote_manifest_value(statement.text) << "\n";
    }
}

std::string build_fxp_token_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "output_kind=fxp\n";
    stream << "token_contract=logical_statements\n";
    stream << "primary_output=" << quote_manifest_value(std::filesystem::path(plan.launcher_output_path).filename().string()) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        stream << "program=" << quote_manifest_value(asset.relative_path) << "\n";
        append_fxp_statement_lines(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            append_fxp_statement_lines(stream, routine_entry.first, routine_entry.second.statements);
        }
    }
    return stream.str();
}

std::string build_app_archive_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "output_kind=app\n";
    stream << "archive_contract=staged_content_manifest\n";
    stream << "primary_output=" << quote_manifest_value(std::filesystem::path(plan.launcher_output_path).filename().string()) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    for (const auto& asset : plan.assets) {
        stream << "asset="
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.required_for_runtime ? "true" : "false") << "|"
               << (asset.copied ? "true" : "false") << "\n";
    }
    return stream.str();
}

bool write_app_archive_primary_output(const RuntimePackagePlan& plan, std::string& error) {
    std::ostringstream stream;
    stream << "copperfin_app_archive_version=1\n";
    stream << "archive_contract=copperfin_content_archive_v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "content_manifest=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";

    for (const auto& asset : plan.assets) {
        if (!asset.copied || trim_copy(asset.staged_path).empty() || !std::filesystem::exists(asset.staged_path)) {
            continue;
        }

        error.clear();
        const std::string bytes = read_binary_file(asset.staged_path, error);
        if (!error.empty()) {
            return false;
        }

        stream << "asset="
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.required_for_runtime ? "true" : "false") << "|"
               << bytes.size() << "|"
               << quote_manifest_value(asset.sha256) << "\n";
        stream << "payload="
               << quote_manifest_value(asset.relative_path) << "|"
               << hex_encode_bytes(bytes) << "\n";
    }

    return write_text_file(plan.launcher_output_path, stream.str(), error);
}

const char* statement_kind_name(const StatementKind kind) {
    switch (kind) {
        case StatementKind::assignment:
            return "assignment";
        case StatementKind::expression:
            return "expression";
        case StatementKind::do_command:
            return "do_command";
        case StatementKind::do_while_statement:
            return "do_while_statement";
        case StatementKind::do_case_statement:
            return "do_case_statement";
        case StatementKind::case_statement:
            return "case_statement";
        case StatementKind::otherwise_statement:
            return "otherwise_statement";
        case StatementKind::calculate_command:
            return "calculate_command";
        case StatementKind::count_command:
            return "count_command";
        case StatementKind::sum_command:
            return "sum_command";
        case StatementKind::average_command:
            return "average_command";
        case StatementKind::text_command:
            return "text_command";
        case StatementKind::total_command:
            return "total_command";
        case StatementKind::do_form:
            return "do_form";
        case StatementKind::report_form:
            return "report_form";
        case StatementKind::label_form:
            return "label_form";
        case StatementKind::activate_surface:
            return "activate_surface";
        case StatementKind::release_surface:
            return "release_surface";
        case StatementKind::return_statement:
            return "return_statement";
        case StatementKind::if_statement:
            return "if_statement";
        case StatementKind::else_statement:
            return "else_statement";
        case StatementKind::endif_statement:
            return "endif_statement";
        case StatementKind::for_statement:
            return "for_statement";
        case StatementKind::endfor_statement:
            return "endfor_statement";
        case StatementKind::loop_statement:
            return "loop_statement";
        case StatementKind::exit_statement:
            return "exit_statement";
        case StatementKind::enddo_statement:
            return "enddo_statement";
        case StatementKind::endcase_statement:
            return "endcase_statement";
        case StatementKind::read_events:
            return "read_events";
        case StatementKind::clear_events:
            return "clear_events";
        case StatementKind::begin_transaction:
            return "begin_transaction";
        case StatementKind::end_transaction:
            return "end_transaction";
        case StatementKind::rollback_transaction:
            return "rollback_transaction";
        case StatementKind::undo_command:
            return "undo_command";
        case StatementKind::seek_command:
            return "seek_command";
        case StatementKind::locate_command:
            return "locate_command";
        case StatementKind::scan_statement:
            return "scan_statement";
        case StatementKind::endscan_statement:
            return "endscan_statement";
        case StatementKind::replace_command:
            return "replace_command";
        case StatementKind::append_blank_command:
            return "append_blank_command";
        case StatementKind::delete_command:
            return "delete_command";
        case StatementKind::recall_command:
            return "recall_command";
        case StatementKind::pack_command:
            return "pack_command";
        case StatementKind::zap_command:
            return "zap_command";
        case StatementKind::unlock_command:
            return "unlock_command";
        case StatementKind::delete_from_command:
            return "delete_from_command";
        case StatementKind::insert_into_command:
            return "insert_into_command";
        case StatementKind::go_command:
            return "go_command";
        case StatementKind::skip_command:
            return "skip_command";
        case StatementKind::browse_command:
            return "browse_command";
        case StatementKind::select_command:
            return "select_command";
        case StatementKind::use_command:
            return "use_command";
        case StatementKind::set_order:
            return "set_order";
        case StatementKind::set_command:
            return "set_command";
        case StatementKind::set_library:
            return "set_library";
        case StatementKind::set_datasession:
            return "set_datasession";
        case StatementKind::set_default:
            return "set_default";
        case StatementKind::set_memowidth:
            return "set_memowidth";
        case StatementKind::on_error:
            return "on_error";
        case StatementKind::on_shutdown:
            return "on_shutdown";
        case StatementKind::with_statement:
            return "with_statement";
        case StatementKind::endwith_statement:
            return "endwith_statement";
        case StatementKind::try_statement:
            return "try_statement";
        case StatementKind::catch_statement:
            return "catch_statement";
        case StatementKind::finally_statement:
            return "finally_statement";
        case StatementKind::endtry_statement:
            return "endtry_statement";
        case StatementKind::public_declaration:
            return "public_declaration";
        case StatementKind::local_declaration:
            return "local_declaration";
        case StatementKind::private_declaration:
            return "private_declaration";
        case StatementKind::parameters_declaration:
            return "parameters_declaration";
        case StatementKind::lparameters_declaration:
            return "lparameters_declaration";
        case StatementKind::dimension_command:
            return "dimension_command";
        case StatementKind::store_command:
            return "store_command";
        case StatementKind::close_command:
            return "close_command";
        case StatementKind::erase_command:
            return "erase_command";
        case StatementKind::copy_file_command:
            return "copy_file_command";
        case StatementKind::rename_file_command:
            return "rename_file_command";
        case StatementKind::print_command:
            return "print_command";
        case StatementKind::create_cursor_command:
            return "create_cursor_command";
        case StatementKind::create_table_command:
            return "create_table_command";
        case StatementKind::alter_table_command:
            return "alter_table_command";
        case StatementKind::copy_to_command:
            return "copy_to_command";
        case StatementKind::append_from_command:
            return "append_from_command";
        case StatementKind::save_memvars_command:
            return "save_memvars_command";
        case StatementKind::restore_memvars_command:
            return "restore_memvars_command";
        case StatementKind::scatter_command:
            return "scatter_command";
        case StatementKind::gather_command:
            return "gather_command";
        case StatementKind::update_command:
            return "update_command";
        case StatementKind::retry_statement:
            return "retry_statement";
        case StatementKind::resume_statement:
            return "resume_statement";
        case StatementKind::declare_dll:
            return "declare_dll";
        case StatementKind::call_command:
            return "call_command";
        case StatementKind::for_each_statement:
            return "for_each_statement";
        case StatementKind::release_command:
            return "release_command";
        case StatementKind::clear_memory_command:
            return "clear_memory_command";
        case StatementKind::cancel_statement:
            return "cancel_statement";
        case StatementKind::quit_statement:
            return "quit_statement";
        case StatementKind::yield_statement:
            return "yield_statement";
        case StatementKind::doevents_command:
            return "doevents_command";
        case StatementKind::enter_critical_command:
            return "enter_critical_command";
        case StatementKind::exit_critical_command:
            return "exit_critical_command";
        case StatementKind::spawn_command:
            return "spawn_command";
        case StatementKind::await_command:
            return "await_command";
        case StatementKind::on_shutdown_statement:
            return "on_shutdown_statement";
        case StatementKind::edit_command:
            return "edit_command";
        case StatementKind::change_command:
            return "change_command";
        case StatementKind::input_command:
            return "input_command";
        case StatementKind::accept_command:
            return "accept_command";
        case StatementKind::getfile_command:
            return "getfile_command";
        case StatementKind::putfile_command:
            return "putfile_command";
        case StatementKind::getdir_command:
            return "getdir_command";
        case StatementKind::inputbox_command:
            return "inputbox_command";
        case StatementKind::wait_command:
            return "wait_command";
        case StatementKind::sleep_command:
            return "sleep_command";
        case StatementKind::keyboard_command:
            return "keyboard_command";
        case StatementKind::push_key_command:
            return "push_key_command";
        case StatementKind::pop_key_command:
            return "pop_key_command";
        case StatementKind::push_menu_command:
            return "push_menu_command";
        case StatementKind::pop_menu_command:
            return "pop_menu_command";
        case StatementKind::push_popup_command:
            return "push_popup_command";
        case StatementKind::pop_popup_command:
            return "pop_popup_command";
        case StatementKind::display_command:
            return "display_command";
        case StatementKind::list_command:
            return "list_command";
        case StatementKind::no_op:
            return "no_op";
    }
    return "no_op";
}

void append_ast_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements) {
    stream << "        {\n";
    stream << "          \"name\": \"" << json_escape(routine_name) << "\",\n";
    stream << "          \"statements\": [\n";
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto& statement = statements[index];
        stream << "            {\"line\": " << statement.location.line
               << ", \"text\": \"" << json_escape(statement.text) << "\""
               << ", \"identifier\": \"" << json_escape(statement.identifier) << "\""
               << ", \"expression\": \"" << json_escape(statement.expression) << "\"}";
        if (index + 1U != statements.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "          ]\n";
    stream << "        }";
}

std::string build_ast_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"schema_version\": 1,\n";
    stream << "  \"project_title\": \"" << json_escape(plan.project_title) << "\",\n";
    stream << "  \"output_kind\": \"" << json_escape(build_output_kind_name(plan.output_kind)) << "\",\n";
    stream << "  \"files\": [\n";

    bool first_file = true;
    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        if (!first_file) {
            stream << ",\n";
        }
        first_file = false;
        stream << "    {\n";
        stream << "      \"relative_path\": \"" << json_escape(asset.relative_path) << "\",\n";
        stream << "      \"routines\": [\n";
        append_ast_routine_json(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            stream << ",\n";
            append_ast_routine_json(stream, routine_entry.first, routine_entry.second.statements);
        }
        stream << "\n";
        stream << "      ]\n";
        stream << "    }";
    }

    stream << "\n";
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

void append_ir_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements) {
    stream << "        {\n";
    stream << "          \"name\": \"" << json_escape(routine_name) << "\",\n";
    stream << "          \"instructions\": [\n";
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto& statement = statements[index];
        stream << "            {\"line\": " << statement.location.line
               << ", \"opcode\": \"" << json_escape(statement_kind_name(statement.kind)) << "\""
               << ", \"text\": \"" << json_escape(statement.text) << "\""
               << ", \"identifier\": \"" << json_escape(statement.identifier) << "\""
               << ", \"expression\": \"" << json_escape(statement.expression) << "\""
               << ", \"secondary_expression\": \"" << json_escape(statement.secondary_expression) << "\""
               << ", \"tertiary_expression\": \"" << json_escape(statement.tertiary_expression) << "\""
               << ", \"quaternary_expression\": \"" << json_escape(statement.quaternary_expression) << "\""
               << ", \"operand_count\": " << statement.names.size() << "}";
        if (index + 1U != statements.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "          ]\n";
    stream << "        }";
}

std::string build_ir_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"schema_version\": 1,\n";
    stream << "  \"project_title\": \"" << json_escape(plan.project_title) << "\",\n";
    stream << "  \"output_kind\": \"" << json_escape(build_output_kind_name(plan.output_kind)) << "\",\n";
    stream << "  \"files\": [\n";

    bool first_file = true;
    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        if (!first_file) {
            stream << ",\n";
        }
        first_file = false;
        stream << "    {\n";
        stream << "      \"relative_path\": \"" << json_escape(asset.relative_path) << "\",\n";
        stream << "      \"routines\": [\n";
        append_ir_routine_json(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            stream << ",\n";
            append_ir_routine_json(stream, routine_entry.first, routine_entry.second.statements);
        }
        stream << "\n";
        stream << "      ]\n";
        stream << "    }";
    }

    stream << "\n";
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

std::string sanitize_csharp_identifier(std::string value, const std::string& fallback) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return fallback;
    }

    std::string sanitized;
    sanitized.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        const unsigned char ch = static_cast<unsigned char>(value[index]);
        const bool valid =
            std::isalnum(ch) != 0 ||
            ch == '_';
        if (!valid) {
            sanitized.push_back('_');
            continue;
        }
        if (sanitized.empty() && std::isdigit(ch) != 0) {
            sanitized.push_back('_');
        }
        sanitized.push_back(static_cast<char>(ch));
    }
    return sanitized.empty() ? fallback : sanitized;
}

std::string sanitize_csharp_routine_identifier(std::string value, const std::string& fallback) {
    std::string sanitized = sanitize_csharp_identifier(std::move(value), fallback);
    for (char& ch : sanitized) {
        if (std::isalpha(static_cast<unsigned char>(ch)) != 0) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            break;
        }
    }
    return sanitized;
}

std::string unquote_literal(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U &&
        ((value.front() == '\'' && value.back() == '\'') ||
         (value.front() == '"' && value.back() == '"'))) {
        value = value.substr(1U, value.size() - 2U);
    }
    return value;
}

std::string transpile_statement_to_csharp(
    const Statement& statement,
    const std::map<std::string, std::string>& routine_name_map) {
    switch (statement.kind) {
        case StatementKind::local_declaration: {
            std::ostringstream stream;
            for (const auto& name : statement.names) {
                stream << "dynamic " << sanitize_csharp_identifier(name, "localValue") << " = null;\n";
            }
            return stream.str();
        }
        case StatementKind::assignment:
            return statement.text + ";\n";
        case StatementKind::do_command: {
            const std::string routine_name = lowercase_copy(trim_copy(statement.identifier));
            const auto found = routine_name_map.find(routine_name);
            if (found != routine_name_map.end()) {
                return found->second + "();\n";
            }
            break;
        }
        case StatementKind::wait_command:
            if (!statement.expression.empty()) {
                return "Console.WriteLine(\"" + json_escape(unquote_literal(statement.expression)) + "\");\n";
            }
            break;
        case StatementKind::return_statement:
            return "return;\n";
        default:
            break;
    }

    return "throw new NotSupportedException(\"Unsupported FoxPro statement: " + json_escape(statement.text) + "\");\n";
}

std::string sanitize_csharp_compound_identifier(std::string value, const std::string& fallback) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return fallback;
    }

    std::string sanitized;
    sanitized.reserve(value.size());
    bool capitalize_next = true;
    for (const char raw_ch : value) {
        const unsigned char ch = static_cast<unsigned char>(raw_ch);
        if (std::isalnum(ch) != 0) {
            char output = static_cast<char>(ch);
            if (capitalize_next && std::isalpha(ch) != 0) {
                output = static_cast<char>(std::toupper(ch));
            }
            if (sanitized.empty() && std::isdigit(ch) != 0) {
                sanitized.push_back('_');
            }
            sanitized.push_back(output);
            capitalize_next = false;
            continue;
        }

        if (!sanitized.empty() && sanitized.back() != '_') {
            sanitized.push_back('_');
        }
        capitalize_next = true;
    }

    while (!sanitized.empty() && sanitized.back() == '_') {
        sanitized.pop_back();
    }
    return sanitized.empty() ? fallback : sanitized;
}

std::string build_xasset_csharp_method_identifier(
    const XAssetExecutableModel& model,
    const XAssetMethod& method) {
    const std::string normalized_root = lowercase_copy(trim_copy(model.root_object_path));
    const std::string normalized_object = lowercase_copy(trim_copy(method.object_path));
    if (!normalized_root.empty() && normalized_object == normalized_root) {
        return sanitize_csharp_compound_identifier(method.method_name, "Method");
    }

    std::string method_prefix = method.object_path;
    if (!model.root_object_path.empty() &&
        normalized_object.size() > normalized_root.size() &&
        normalized_object.rfind(normalized_root + ".", 0U) == 0U) {
        method_prefix = method.object_path.substr(model.root_object_path.size() + 1U);
    }
    if (!method_prefix.empty()) {
        method_prefix += ".";
    }
    method_prefix += method.method_name;
    return sanitize_csharp_compound_identifier(method_prefix, "Method");
}

void append_xasset_csharp_type(
    std::ostringstream& stream,
    const studio::StudioDocumentModel& document) {
    const XAssetExecutableModel model = build_xasset_executable_model(document);
    if (!model.ok || model.root_object_path.empty()) {
        return;
    }

    const std::string type_name = sanitize_csharp_compound_identifier(model.root_object_path, "XAssetObject");
    std::map<std::string, std::string> method_name_map;
    for (const auto& method : model.methods) {
        method_name_map.emplace(method.routine_name, build_xasset_csharp_method_identifier(model, method));
    }

    stream << "    public sealed class " << type_name << "\n";
    stream << "    {\n";

    for (const auto& method : model.methods) {
        const auto mapped_name = method_name_map.find(method.routine_name);
        if (mapped_name == method_name_map.end()) {
            continue;
        }

        const std::string method_identity = method.object_path.empty()
            ? method.method_name
            : method.object_path + "." + method.method_name;
        stream << "        public void " << mapped_name->second << "()\n";
        stream << "        {\n";
        stream << "            throw new NotSupportedException(\"Manual port required for FoxPro xAsset method: "
               << json_escape(method_identity)
               << "\");\n";
        stream << "        }\n\n";
    }

    if (!model.startup_routines.empty()) {
        stream << "        public void RunStartup()\n";
        stream << "        {\n";
        for (const auto& routine_name : model.startup_routines) {
            const auto found = method_name_map.find(routine_name);
            if (found != method_name_map.end()) {
                stream << "            " << found->second << "();\n";
            }
        }
        stream << "        }\n\n";
    }

    if (!model.shutdown_routines.empty()) {
        stream << "        public void RunShutdown()\n";
        stream << "        {\n";
        for (const auto& routine_name : model.shutdown_routines) {
            const auto found = method_name_map.find(routine_name);
            if (found != method_name_map.end()) {
                stream << "            " << found->second << "();\n";
            }
        }
        stream << "        }\n\n";
    }

    stream << "    }\n\n";
}

std::string build_csharp_transpilation_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "using System;\n\n";
    stream << "namespace Copperfin.Generated\n";
    stream << "{\n";
    stream << "    public static class TranspiledProgram\n";
    stream << "    {\n";

    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        std::map<std::string, std::string> routine_name_map;
        for (const auto& routine_entry : program.routines) {
            routine_name_map.emplace(lowercase_copy(routine_entry.first),
                                     sanitize_csharp_routine_identifier(routine_entry.first, "Routine"));
        }

        stream << "        public static void MainRoutine()\n";
        stream << "        {\n";
        for (const auto& statement : program.main.statements) {
            stream << "            " << transpile_statement_to_csharp(statement, routine_name_map);
        }
        stream << "        }\n\n";

        for (const auto& routine_entry : program.routines) {
            stream << "        public static void "
                   << sanitize_csharp_routine_identifier(routine_entry.first, "Routine")
                   << "()\n";
            stream << "        {\n";
            for (const auto& statement : routine_entry.second.statements) {
                stream << "            " << transpile_statement_to_csharp(statement, routine_name_map);
            }
            stream << "        }\n\n";
        }
    }

    stream << "    }\n\n";

    for (const auto& asset : plan.assets) {
        const std::string extension = lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string()));
        if (extension != ".scx" && extension != ".vcx") {
            continue;
        }

        const auto open_result = studio::open_document({
            .path = asset.source_path,
            .load_full_table = true
        });
        if (!open_result.ok) {
            continue;
        }

        append_xasset_csharp_type(stream, open_result.document);
    }

    stream << "}\n";
    return stream.str();
}

std::string build_launcher_program_source(const RuntimePackagePlan&) {
    std::ostringstream stream;
    stream << "using System;\n";
    stream << "using System.Collections.Generic;\n";
    stream << "using System.Diagnostics;\n";
    stream << "using System.IO;\n\n";
    stream << "internal static class Program\n";
    stream << "{\n";
    stream << "    private static int Main(string[] args)\n";
    stream << "    {\n";
    stream << "        var baseDir = AppContext.BaseDirectory;\n";
    stream << "        var runtimeHost = Path.Combine(baseDir, \"copperfin_runtime_host.exe\");\n";
    stream << "        var manifest = Path.Combine(baseDir, \"app.cfmanifest\");\n";
    stream << "        if (!File.Exists(runtimeHost))\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(\"Copperfin runtime host was not found beside the launcher.\");\n";
    stream << "            return 3;\n";
    stream << "        }\n";
    stream << "        if (!File.Exists(manifest))\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(\"Copperfin manifest was not found beside the launcher.\");\n";
    stream << "            return 4;\n";
    stream << "        }\n\n";
    stream << "        var forwarded = new List<string> { \"--manifest\", Quote(manifest) };\n";
        stream << "        foreach (var arg in args)\n";
        stream << "        {\n";
        stream << "            if (string.Equals(arg, \"--debug\", StringComparison.OrdinalIgnoreCase) ||\n";
        stream << "                string.Equals(arg, \"/debug\", StringComparison.OrdinalIgnoreCase))\n";
        stream << "            {\n";
        stream << "                forwarded.Add(\"--debug\");\n";
        stream << "                continue;\n";
        stream << "            }\n";
        stream << "            forwarded.Add(Quote(arg));\n";
        stream << "        }\n\n";
    stream << "        var startInfo = new ProcessStartInfo\n";
    stream << "        {\n";
    stream << "            FileName = runtimeHost,\n";
    stream << "            Arguments = string.Join(\" \", forwarded),\n";
    stream << "            WorkingDirectory = baseDir,\n";
    stream << "            UseShellExecute = false\n";
    stream << "        };\n\n";
    stream << "        using var process = Process.Start(startInfo);\n";
    stream << "        if (process is null)\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(\"Copperfin runtime host could not be started.\");\n";
    stream << "            return 5;\n";
    stream << "        }\n";
    stream << "        process.WaitForExit();\n";
    stream << "        return process.ExitCode;\n";
    stream << "    }\n\n";
    stream << "    private static string Quote(string value)\n";
    stream << "    {\n";
    stream << "        return \"\\\"\" + value.Replace(\"\\\"\", \"\\\"\\\"\") + \"\\\"\";\n";
    stream << "    }\n";
    stream << "}\n";
    return stream.str();
}

std::string build_launcher_project_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "<Project Sdk=\"Microsoft.NET.Sdk\">\n";
    stream << "  <PropertyGroup>\n";
    stream << "    <OutputType>Exe</OutputType>\n";
    stream << "    <TargetFramework>net8.0-windows</TargetFramework>\n";
    stream << "    <ImplicitUsings>enable</ImplicitUsings>\n";
    stream << "    <Nullable>enable</Nullable>\n";
    stream << "    <UseWindowsForms>false</UseWindowsForms>\n";
    stream << "    <AssemblyName>" << sanitize_file_name(plan.project_title) << "</AssemblyName>\n";
    stream << "    <RootNamespace>Copperfin.Generated</RootNamespace>\n";
    stream << "    <PublishSingleFile>false</PublishSingleFile>\n";
    stream << "  </PropertyGroup>\n";
    stream << "</Project>\n";
    return stream.str();
}

bool copy_file_if_exists(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    std::string& error) {
    if (!std::filesystem::exists(source)) {
        error = "Source file does not exist: " + source.string();
        return false;
    }

    std::error_code directory_error;
    std::filesystem::create_directories(destination.parent_path(), directory_error);
    if (directory_error) {
        error = "Unable to create directory: " + destination.parent_path().string();
        return false;
    }

    std::error_code copy_error;
    std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
        error = "Unable to copy file to: " + destination.string();
        return false;
    }

    return true;
}

bool validate_runtime_host_source_path(
    const RuntimePackagePlan& plan,
    const std::string& runtime_host_source_path,
    std::string& error) {
    if (runtime_host_source_path.empty()) {
        error = "Runtime host source path is empty.";
        return false;
    }

    std::filesystem::path source(runtime_host_source_path);
    std::error_code canonical_error;
    source = std::filesystem::weakly_canonical(source, canonical_error);
    if (canonical_error) {
        error = "Unable to resolve runtime host source path.";
        return false;
    }

    if (!std::filesystem::exists(source) || !std::filesystem::is_regular_file(source)) {
        error = "Runtime host source path does not point to a regular file.";
        return false;
    }

    if (plan.security_enabled) {
#ifdef _WIN32
        const std::string expected_file_name = "copperfin_runtime_host.exe";
#else
        const std::string expected_file_name = "copperfin_runtime_host";
#endif
        if (!source.is_absolute()) {
            error = "Security-enabled packaging requires an absolute runtime host source path.";
            return false;
        }

        if (lowercase_copy(source.filename().string()) != expected_file_name) {
            error = "Security-enabled packaging requires canonical runtime host binary name.";
            return false;
        }
    }

    return true;
}

std::string resolve_project_item_source(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectEntry& entry) {
    const std::filesystem::path base_dir = std::filesystem::path(document.path).parent_path();

    if (!entry.relative_path.empty()) {
        const std::filesystem::path from_relative = base_dir / entry.relative_path;
        if (std::filesystem::exists(from_relative)) {
            return from_relative.lexically_normal().string();
        }
    }

    if (entry.name.empty()) {
        return {};
    }

    const std::filesystem::path raw(entry.name);
    if (raw.is_absolute()) {
        if (std::filesystem::exists(raw)) {
            return raw.lexically_normal().string();
        }

        const std::filesystem::path from_filename = base_dir / raw.filename();
        return from_filename.lexically_normal().string();
    }

    return (base_dir / raw).lexically_normal().string();
}

std::string relative_asset_path(const studio::StudioProjectEntry& entry) {
    const std::string path = !entry.relative_path.empty() ? entry.relative_path : entry.name;
    if (!path.empty()) {
        return path;
    }
    return "record_" + std::to_string(entry.record_index) + ".asset";
}

std::string resolve_working_directory(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace) {
    const std::filesystem::path document_dir = std::filesystem::path(document.path).parent_path();
    if (!workspace.home_directory.empty()) {
        const std::filesystem::path home_directory(workspace.home_directory);
        if (std::filesystem::exists(home_directory)) {
            return home_directory.lexically_normal().string();
        }
    }
    return document_dir.lexically_normal().string();
}

std::string resolve_security_role(bool security_enabled) {
    if (!security_enabled) {
        return {};
    }

    std::string role;
#ifdef _WIN32
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, "COPPERFIN_SECURITY_ROLE") == 0 && raw != nullptr) {
        role = raw;
        std::free(raw);
    }
#else
    if (const char* raw = std::getenv("COPPERFIN_SECURITY_ROLE"); raw != nullptr) {
        role = raw;
    }
#endif

    role = trim_copy(role);
    if (!role.empty()) {
        return role;
    }

    return "developer";
}

bool is_extension_payload_path(const std::filesystem::path& path) {
    const std::string extension = lowercase_copy(trim_copy(path.extension().string()));
    return extension == ".dll" || extension == ".exe" || extension == ".vsix";
}

std::string join_strings(const std::vector<std::string>& values) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < values.size(); ++index) {
        stream << values[index];
        if ((index + 1U) != values.size()) {
            stream << ";";
        }
    }
    return stream.str();
}

bool is_prg_path(const std::string& value) {
    return lowercase_copy(trim_copy(std::filesystem::path(value).extension().string())) == ".prg";
}

void append_feature_flag_line(
    std::ostringstream& stream,
    std::string_view name,
    bool enabled,
    std::string_view category) {
    stream << "feature_flag="
           << name << "|"
           << (enabled ? "true" : "false") << "|"
           << category << "\n";
}

bool is_xasset_path(const std::string& value) {
    const std::string extension = trim_copy(std::filesystem::path(value).extension().string());
    return extension == ".scx" ||
        extension == ".vcx" ||
        extension == ".frx" ||
        extension == ".lbx" ||
        extension == ".mnx";
}

bool should_stage_asset(const RuntimePackageAsset& asset) {
    return asset.exists && (!asset.excluded || asset.required_for_runtime);
}

std::vector<std::filesystem::path> infer_companion_source_paths(const std::filesystem::path& source) {
    std::vector<std::filesystem::path> companions;
    const std::string extension = trim_copy(lowercase_copy(source.extension().string()));
    const auto same_stem = [&](const char* companion_extension) {
        auto path = source;
        path.replace_extension(companion_extension);
        return path;
    };

    if (extension == ".pjx") {
        companions.push_back(same_stem(".pjt"));
    } else if (extension == ".scx") {
        companions.push_back(same_stem(".sct"));
    } else if (extension == ".vcx") {
        companions.push_back(same_stem(".vct"));
    } else if (extension == ".frx") {
        companions.push_back(same_stem(".frt"));
    } else if (extension == ".lbx") {
        companions.push_back(same_stem(".lbt"));
    } else if (extension == ".mnx") {
        companions.push_back(same_stem(".mnt"));
    } else if (extension == ".dbf") {
        companions.push_back(same_stem(".fpt"));
        companions.push_back(same_stem(".cdx"));
        companions.push_back(same_stem(".idx"));
        companions.push_back(same_stem(".ndx"));
        companions.push_back(same_stem(".mdx"));
    } else if (extension == ".dbc") {
        companions.push_back(same_stem(".dct"));
        companions.push_back(same_stem(".dcx"));
    }

    return companions;
}

void copy_companion_files_if_present(
    const RuntimePackageAsset& asset,
    std::vector<std::string>& warnings) {
    const std::filesystem::path source(asset.source_path);
    const std::filesystem::path staged(asset.staged_path);
    for (const auto& companion_source : infer_companion_source_paths(source)) {
        if (!std::filesystem::exists(companion_source)) {
            continue;
        }

        auto companion_destination = staged;
        companion_destination.replace_extension(companion_source.extension().string());
        std::string error;
        if (!copy_file_if_exists(companion_source, companion_destination, error)) {
            warnings.push_back(error);
        }
    }
}

}  // namespace

const char* build_configuration_name(BuildConfiguration configuration) {
    switch (configuration) {
        case BuildConfiguration::debug:
            return "debug";
        case BuildConfiguration::release:
            return "release";
    }
    return "debug";
}

BuildConfiguration parse_build_configuration(const std::string& value) {
    return trim_copy(value) == "release"
        ? BuildConfiguration::release
        : BuildConfiguration::debug;
}

const char* build_output_kind_name(BuildOutputKind output_kind) {
    switch (output_kind) {
        case BuildOutputKind::executable:
            return "executable";
        case BuildOutputKind::app:
            return "app";
        case BuildOutputKind::dll:
            return "dll";
        case BuildOutputKind::fll:
            return "fll";
        case BuildOutputKind::fxp:
            return "fxp";
        case BuildOutputKind::ocx:
            return "ocx";
        case BuildOutputKind::unknown:
            return "unknown";
    }
    return "unknown";
}

RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher) {
    RuntimePackagePlan plan;
    plan.project_path = document.path;
    plan.project_title = workspace.project_title.empty()
        ? std::filesystem::path(document.path).stem().string()
        : workspace.project_title;
    plan.configuration = configuration;
    plan.security_enabled = enable_security;
    plan.output_kind = parse_build_output_kind(workspace.build_plan.output_kind);
    if (plan.output_kind == BuildOutputKind::unknown) {
        plan.output_kind = infer_build_output_kind_from_output_path(workspace.build_plan.output_path);
    }
    plan.requested_dotnet_launcher = emit_dotnet_launcher;
    plan.emit_dotnet_launcher =
        is_native_host_output_kind(plan.output_kind) &&
        emit_dotnet_launcher &&
        extensibility_profile.dotnet_output.available;
    if (is_library_output_kind(plan.output_kind)) {
        plan.launcher_mode = "foxpro_library_definition";
        plan.launcher_fallback = "library_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::app) {
        plan.launcher_mode = "foxpro_application_archive_contract";
        plan.launcher_fallback = "foxpro_app_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        plan.launcher_mode = "foxpro_tokenized_contract";
        plan.launcher_fallback = "fxp_binary_generation_pending";
    } else {
        plan.launcher_mode = plan.emit_dotnet_launcher ? "dotnet_launcher" : "native_runtime_host";
        plan.launcher_fallback =
            (plan.requested_dotnet_launcher && !plan.emit_dotnet_launcher)
                ? "dotnet_output_unavailable"
                : "none";
    }

    if (!workspace.available) {
        plan.warnings.push_back("Project workspace is not available.");
        return plan;
    }

    const std::filesystem::path root(output_root);
    const std::filesystem::path package_root = root / sanitize_file_name(plan.project_title);
    const std::filesystem::path content_root = package_root / "content";
    plan.package_root = package_root.string();
    plan.content_root = content_root.string();
    plan.manifest_path = (package_root / "app.cfmanifest").string();
    plan.debug_manifest_path = (package_root / "app.cfdebug").string();
    plan.launcher_project_path = (package_root / "launcher" / "Copperfin.GeneratedLauncher.csproj").string();
    plan.launcher_source_path = (package_root / "launcher" / "Program.cs").string();
    const std::filesystem::path output_file_name(resolve_output_file_name(workspace, plan.project_title));
    plan.ast_manifest_path = (package_root / (output_file_name.string() + ".ast.json")).string();
    plan.ir_manifest_path = (package_root / (output_file_name.string() + ".ir.json")).string();
    plan.transpiled_csharp_path = (package_root / (output_file_name.string() + ".transpiled.cs")).string();
    std::filesystem::path module_definition_file_name = output_file_name;
    module_definition_file_name.replace_extension(".def");
    plan.launcher_output_path = (package_root / output_file_name).string();
    plan.module_definition_path = (package_root / module_definition_file_name).string();
    if (is_library_output_kind(plan.output_kind)) {
        const std::filesystem::path wrapper_root = package_root / "wrapper";
        const std::string output_stem = output_file_name.stem().string();
        plan.native_wrapper_source_path = (wrapper_root / (output_stem + "_wrapper.cpp")).string();
        plan.native_wrapper_cmake_path = (wrapper_root / "CMakeLists.txt").string();
        plan.native_wrapper_build_script_path = (wrapper_root / "build_wrapper.sh").string();
        plan.native_wrapper_build_powershell_path = (wrapper_root / "build_wrapper.ps1").string();
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        std::filesystem::path fll_api_manifest_file_name = output_file_name;
        fll_api_manifest_file_name += ".api";
        plan.fll_api_manifest_path = (package_root / fll_api_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::fxp) {
        std::filesystem::path fxp_token_manifest_file_name = output_file_name;
        fxp_token_manifest_file_name += ".tokens";
        plan.fxp_token_manifest_path = (package_root / fxp_token_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::app) {
        std::filesystem::path app_archive_manifest_file_name = output_file_name;
        app_archive_manifest_file_name += ".contents";
        plan.app_archive_manifest_path = (package_root / app_archive_manifest_file_name).string();
    }
    plan.runtime_host_destination_path = (package_root / "copperfin_runtime_host.exe").string();
    plan.working_directory = content_root.lexically_normal().string();
    plan.startup_item = workspace.build_plan.startup_item;
    plan.security_role = resolve_security_role(enable_security);
    plan.audit_log_path = (package_root / "security_audit.log").string();
    const std::string source_working_directory = resolve_working_directory(document, workspace);

    for (const auto& entry : workspace.entries) {
        RuntimePackageAsset asset;
        asset.record_index = entry.record_index;
        asset.relative_path = relative_asset_path(entry);
        asset.source_path = resolve_project_item_source(document, entry);
        asset.staged_path = (content_root / asset.relative_path).lexically_normal().string();
        asset.type_title = entry.type_title;
        asset.excluded = entry.excluded;
        asset.exists = !asset.source_path.empty() && std::filesystem::exists(asset.source_path);
        if (entry.record_index == workspace.build_plan.startup_record_index) {
            asset.required_for_runtime = true;
            plan.startup_source_path = asset.staged_path;
            plan.debug_plan.startup_source_path = asset.source_path;
        }
        if (!asset.exists && !entry.excluded && entry.group_id != "project") {
            plan.warnings.push_back("Missing project asset: " + asset.source_path);
        }
        plan.assets.push_back(std::move(asset));
    }

    if (plan.startup_source_path.empty()) {
        plan.warnings.push_back("No startup source asset could be resolved.");
    }
    if (plan.debug_plan.startup_source_path.empty()) {
        plan.warnings.push_back("No source-side startup asset could be resolved for debugging.");
    }

    plan.debug_plan.manifest_path = plan.debug_manifest_path;
    plan.debug_plan.startup_item = plan.startup_item;
    plan.debug_plan.working_directory = source_working_directory;
    plan.debug_plan.source_roots = unique_non_empty_paths_preserve_order({
        source_working_directory,
        plan.content_root
    });
    plan.debug_plan.supports_breakpoints =
        is_prg_path(plan.debug_plan.startup_source_path) ||
        is_xasset_path(plan.debug_plan.startup_source_path);
    plan.debug_plan.supports_step_debugging = plan.debug_plan.supports_breakpoints;

    if (enable_security && !security_profile.available) {
        plan.warnings.push_back("Security was requested but no native security profile is available.");
    }
    if (emit_dotnet_launcher && !extensibility_profile.dotnet_output.available) {
        plan.warnings.push_back(".NET launcher generation was requested but no .NET output profile is available.");
    }
    if (is_library_output_kind(plan.output_kind)) {
        plan.exported_symbols = collect_library_exported_symbols(plan);
        if (plan.exported_symbols.empty()) {
            plan.warnings.push_back("No PRG routine exports were discovered for the library output contract.");
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        const bool has_prg_asset = std::any_of(plan.assets.begin(), plan.assets.end(), [](const RuntimePackageAsset& asset) {
            return is_prg_path(asset.source_path);
        });
        if (!has_prg_asset) {
            plan.warnings.push_back("No PRG sources were discovered for the FXP token contract.");
        }
    }

    plan.ok = true;
    return plan;
}

std::string build_runtime_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "project_path=" << quote_manifest_value(plan.project_path) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.working_directory) << "\n";
    stream << "ast_manifest_path=" << quote_manifest_value(plan.ast_manifest_path) << "\n";
    stream << "ir_manifest_path=" << quote_manifest_value(plan.ir_manifest_path) << "\n";
    stream << "transpiled_csharp_path=" << quote_manifest_value(plan.transpiled_csharp_path) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.startup_source_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "primary_output_path=" << quote_manifest_value(plan.launcher_output_path) << "\n";
    stream << "primary_output_materialized=" << (plan.primary_output_materialized ? "true" : "false") << "\n";
    stream << "module_definition_path=" << quote_manifest_value(plan.module_definition_path) << "\n";
    stream << "native_wrapper_source_path=" << quote_manifest_value(plan.native_wrapper_source_path) << "\n";
    stream << "native_wrapper_cmake_path=" << quote_manifest_value(plan.native_wrapper_cmake_path) << "\n";
    stream << "native_wrapper_build_script_path=" << quote_manifest_value(plan.native_wrapper_build_script_path) << "\n";
    stream << "native_wrapper_build_powershell_path=" << quote_manifest_value(plan.native_wrapper_build_powershell_path) << "\n";
    stream << "fll_api_manifest_path=" << quote_manifest_value(plan.fll_api_manifest_path) << "\n";
    stream << "fxp_token_manifest_path=" << quote_manifest_value(plan.fxp_token_manifest_path) << "\n";
    stream << "app_archive_manifest_path=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "security_roles=" << security_profile.roles.size() << "\n";
    stream << "launcher_mode=" << quote_manifest_value(plan.launcher_mode) << "\n";
    stream << "launcher_fallback=" << quote_manifest_value(plan.launcher_fallback) << "\n";
    stream << "dotnet_enabled=" << (extensibility_profile.dotnet_output.available ? "true" : "false") << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "dotnet_policy_allowlist=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    stream << "dotnet_policy_denylist=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    stream << "dotnet_parity_matrix_entries=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";

    const platform::DotNetInteropCallDecision launcher_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    stream << "dotnet_gateway_task_primitives=" << quote_manifest_value(launcher_decision.execution_path + ":" + launcher_decision.reason) << "\n";

    const platform::DotNetInteropCallDecision denied_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 2U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    stream << "dotnet_gateway_unsafe_reflection=" << quote_manifest_value(denied_decision.execution_path + ":" + denied_decision.reason) << "\n";

    stream << "language_integrations=" << extensibility_profile.languages.size() << "\n";
    stream << "ai_features=" << extensibility_profile.ai_features.size() << "\n";
    append_feature_flag_line(stream, "launcher.dotnet.requested", plan.requested_dotnet_launcher, "rollout");
    append_feature_flag_line(stream, "launcher.dotnet.active", plan.emit_dotnet_launcher, "host_compatibility");
    append_feature_flag_line(stream, "runtime.host.native", is_native_host_output_kind(plan.output_kind), "host_compatibility");
    append_feature_flag_line(stream, "build.output.ast_contract", true, "build_output");
    append_feature_flag_line(stream, "build.output.ir_contract", true, "build_output");
    append_feature_flag_line(stream, "build.output.csharp_transpilation", plan.requested_dotnet_launcher, "build_output");
    append_feature_flag_line(stream, "build.output.library_contract", is_library_output_kind(plan.output_kind), "build_output");
    append_feature_flag_line(stream, "build.output.native_library_wrapper", is_library_output_kind(plan.output_kind), "build_output");
    append_feature_flag_line(stream, "build.output.fll_api_contract", plan.output_kind == BuildOutputKind::fll, "build_output");
    append_feature_flag_line(stream, "build.output.fxp_token_contract", plan.output_kind == BuildOutputKind::fxp, "build_output");
    append_feature_flag_line(stream, "build.output.app_archive_contract", plan.output_kind == BuildOutputKind::app, "build_output");
    append_feature_flag_line(stream, "debug.breakpoints", plan.debug_plan.supports_breakpoints, "debug");
    append_feature_flag_line(stream, "debug.step_debugging", plan.debug_plan.supports_step_debugging, "debug");
    append_feature_flag_line(
        stream,
        "security.native",
        plan.security_enabled && security_profile.available,
        "security");

    for (const auto& asset : plan.assets) {
        stream << "asset="
               << asset.record_index << "|"
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.staged_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.excluded ? "true" : "false") << "|"
               << (asset.exists ? "true" : "false") << "|"
               << quote_manifest_value(asset.sha256) << "|"
               << (asset.copied ? "true" : "false") << "\n";
    }

    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }

    for (const auto& digest : plan.compiler_contract_digests) {
        stream << "compiler_contract="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }

    for (const auto& symbol : plan.exported_symbols) {
        stream << "export_symbol=" << quote_manifest_value(symbol) << "\n";
    }

    for (const auto& warning : plan.warnings) {
        stream << "warning=" << quote_manifest_value(warning) << "\n";
    }

    return stream.str();
}

std::string build_debug_manifest_text(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "debug_manifest_version=1\n";
    stream << "startup_item=" << quote_manifest_value(plan.debug_plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.debug_plan.startup_source_path) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.debug_plan.working_directory) << "\n";
    stream << "supports_breakpoints=" << (plan.debug_plan.supports_breakpoints ? "true" : "false") << "\n";
    stream << "supports_step_debugging=" << (plan.debug_plan.supports_step_debugging ? "true" : "false") << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "native_wrapper_source_path=" << quote_manifest_value(plan.native_wrapper_source_path) << "\n";
    stream << "native_wrapper_cmake_path=" << quote_manifest_value(plan.native_wrapper_cmake_path) << "\n";
    stream << "native_wrapper_build_script_path=" << quote_manifest_value(plan.native_wrapper_build_script_path) << "\n";
    stream << "native_wrapper_build_powershell_path=" << quote_manifest_value(plan.native_wrapper_build_powershell_path) << "\n";
    stream << "launcher_mode=" << quote_manifest_value(plan.launcher_mode) << "\n";
    stream << "launcher_fallback=" << quote_manifest_value(plan.launcher_fallback) << "\n";
    stream << "source_roots=" << quote_manifest_value(join_strings(plan.debug_plan.source_roots)) << "\n";
    return stream.str();
}

RuntimeMaterializeResult materialize_runtime_package(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path) {
    if (!plan.ok) {
        return {.ok = false, .error = "Package plan is not valid."};
    }

    std::error_code directory_error;
    std::filesystem::create_directories(plan.package_root, directory_error);
    if (directory_error) {
        return {.ok = false, .error = "Unable to create package root."};
    }
    std::filesystem::create_directories(plan.content_root, directory_error);
    if (directory_error) {
        return {.ok = false, .error = "Unable to create content root."};
    }
    if (plan.emit_dotnet_launcher) {
        std::filesystem::create_directories(std::filesystem::path(plan.launcher_project_path).parent_path(), directory_error);
        if (directory_error) {
            return {.ok = false, .error = "Unable to create launcher directory."};
        }
    }

    RuntimePackagePlan materialized_plan = plan;
    std::string error;
    if (is_native_host_output_kind(plan.output_kind) &&
        !validate_runtime_host_source_path(plan, runtime_host_source_path, error)) {
        return {.ok = false, .error = error};
    }
    for (auto& asset : materialized_plan.assets) {
        if (!should_stage_asset(asset)) {
            continue;
        }

        const std::filesystem::path destination = std::filesystem::path(plan.content_root) / asset.relative_path;
        if (!copy_file_if_exists(asset.source_path, destination, error)) {
            materialized_plan.warnings.push_back(error);
            continue;
        }
        copy_companion_files_if_present(asset, materialized_plan.warnings);
        asset.copied = true;

        const auto digest = security::sha256_hex_for_file(destination.string());
        if (!digest.ok) {
            return {.ok = false, .error = digest.error};
        }
        asset.sha256 = digest.hex_digest;

        if (is_extension_payload_path(destination)) {
            materialized_plan.extension_payload_digests.push_back({
                .path = destination.string(),
                .sha256 = digest.hex_digest
            });
        }
    }

    if (is_library_output_kind(plan.output_kind)) {
        std::filesystem::create_directories(std::filesystem::path(plan.native_wrapper_source_path).parent_path(), directory_error);
        if (directory_error) {
            return {.ok = false, .error = "Unable to create native wrapper directory."};
        }
        if (!write_text_file(plan.module_definition_path, build_module_definition_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.module_definition_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_source_path, build_native_wrapper_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_source_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_cmake_path, build_native_wrapper_cmake_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_cmake_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_build_script_path, build_native_wrapper_shell_script_source(), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_build_script_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_build_powershell_path, build_native_wrapper_powershell_script_source(), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_build_powershell_path, error)) {
            return {.ok = false, .error = error};
        }
        if (plan.output_kind == BuildOutputKind::fll) {
            if (!write_text_file(plan.fll_api_manifest_path, build_fll_api_manifest_source(materialized_plan), error)) {
                return {.ok = false, .error = error};
            }
            if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.fll_api_manifest_path, error)) {
                return {.ok = false, .error = error};
            }
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        if (!write_text_file(plan.fxp_token_manifest_path, build_fxp_token_manifest_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.fxp_token_manifest_path, error)) {
            return {.ok = false, .error = error};
        }
    } else if (plan.output_kind == BuildOutputKind::app) {
        if (!write_text_file(plan.app_archive_manifest_path, build_app_archive_manifest_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.app_archive_manifest_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_app_archive_primary_output(materialized_plan, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.extension_payload_digests, plan.launcher_output_path, error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else {
        if (!copy_file_if_exists(runtime_host_source_path, plan.runtime_host_destination_path, error)) {
            return {.ok = false, .error = error};
        }

        const auto runtime_host_digest = security::sha256_hex_for_file(plan.runtime_host_destination_path);
        if (!runtime_host_digest.ok) {
            return {.ok = false, .error = runtime_host_digest.error};
        }
        materialized_plan.runtime_host_sha256 = runtime_host_digest.hex_digest;
        materialized_plan.extension_payload_digests.push_back({
            .path = plan.runtime_host_destination_path,
            .sha256 = runtime_host_digest.hex_digest
        });

        if (!plan.emit_dotnet_launcher) {
            if (!copy_file_if_exists(plan.runtime_host_destination_path, plan.launcher_output_path, error)) {
                return {.ok = false, .error = error};
            }

            const auto native_entrypoint_digest = security::sha256_hex_for_file(plan.launcher_output_path);
            if (!native_entrypoint_digest.ok) {
                return {.ok = false, .error = native_entrypoint_digest.error};
            }
            materialized_plan.extension_payload_digests.push_back({
                .path = plan.launcher_output_path,
                .sha256 = native_entrypoint_digest.hex_digest
            });
            materialized_plan.primary_output_materialized = true;
        }
    }

    if (plan.emit_dotnet_launcher) {
        if (!write_text_file(plan.launcher_project_path, build_launcher_project_source(plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.launcher_source_path, build_launcher_program_source(plan), error)) {
            return {.ok = false, .error = error};
        }
    }

    if (!write_text_file(plan.ast_manifest_path, build_ast_manifest_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.ast_manifest_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.ir_manifest_path, build_ir_manifest_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.ir_manifest_path, error)) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        !write_text_file(plan.transpiled_csharp_path, build_csharp_transpilation_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        !append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.transpiled_csharp_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.manifest_path, build_runtime_manifest_text(materialized_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.debug_manifest_path, build_debug_manifest_text(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }

    return {.ok = true, .plan = std::move(materialized_plan), .error = {}};
}

RuntimeBuildResult build_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    if (!plan.ok) {
        return {.ok = false, .error = "Package plan is not valid."};
    }
    if (!is_library_output_kind(plan.output_kind)) {
        return {.ok = false, .error = "Primary-output builds are only supported for library-output packages."};
    }
    if (!std::filesystem::exists(plan.native_wrapper_cmake_path)) {
        return {.ok = false, .error = "Native wrapper CMake metadata is missing."};
    }

    RuntimePackagePlan built_plan = plan;
    std::string error;
    const std::filesystem::path source_root = std::filesystem::path(plan.native_wrapper_cmake_path).parent_path();
    const std::filesystem::path build_root = source_root / "cmake_pipeline_build";
    const std::filesystem::path configure_log_path = build_root / "cmake-configure.log";
    const std::filesystem::path build_log_path = build_root / "cmake-build.log";
    std::error_code ignored;
    std::filesystem::remove_all(build_root, ignored);
    std::filesystem::remove(plan.launcher_output_path, ignored);
    std::filesystem::create_directories(build_root, ignored);
    if (ignored) {
        return {.ok = false, .error = "Unable to create native wrapper build directory."};
    }

    const std::string configure_command =
        "cmake -S \"" + source_root.string() + "\" -B \"" + build_root.string() + "\" > \"" +
        configure_log_path.string() + "\" 2>&1";
    if (std::system(configure_command.c_str()) != 0) {
        error = "native wrapper primary-output configure failed";
        if (std::filesystem::exists(configure_log_path)) {
            error += ":\n" + read_text_file(configure_log_path);
        }
        return {.ok = false, .error = error};
    }

    const std::string build_command =
        "cmake --build \"" + build_root.string() + "\" > \"" + build_log_path.string() + "\" 2>&1";
    if (std::system(build_command.c_str()) != 0) {
        error = "native wrapper primary-output build failed";
        if (std::filesystem::exists(build_log_path)) {
            error += ":\n" + read_text_file(build_log_path);
        }
        return {.ok = false, .error = error};
    }

    if (!std::filesystem::exists(plan.launcher_output_path)) {
        return {.ok = false, .error = "native wrapper primary-output build did not materialize the requested output path."};
    }

    built_plan.primary_output_materialized = true;
    if (!append_runtime_artifact_digest(built_plan.extension_payload_digests, plan.launcher_output_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.manifest_path, build_runtime_manifest_text(built_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.debug_manifest_path, build_debug_manifest_text(built_plan), error)) {
        return {.ok = false, .error = error};
    }

    return {.ok = true, .plan = std::move(built_plan), .error = {}};
}

}  // namespace copperfin::runtime
