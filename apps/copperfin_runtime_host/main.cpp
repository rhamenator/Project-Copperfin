#include "copperfin/runtime/prg_engine.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/platform/federation_execution.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/security_model.h"
#include "copperfin/security/sha256.h"
#include "copperfin/studio/document_model.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace {

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
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

bool starts_with_insensitive(const std::string& value, const std::string& prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (std::tolower(static_cast<unsigned char>(value[index])) !=
            std::tolower(static_cast<unsigned char>(prefix[index]))) {
            return false;
        }
    }
    return true;
}

const copperfin::runtime::XAssetActionBinding* find_pause_xasset_action(
    const copperfin::runtime::RuntimePauseState& state,
    const copperfin::runtime::XAssetExecutableModel& model) {
    if (model.actions.empty()) {
        return nullptr;
    }

    for (const auto& frame : state.call_stack) {
        const std::string normalized_routine_name = lowercase_copy(trim_copy(frame.routine_name));
        if (normalized_routine_name.empty()) {
            continue;
        }

        const auto found = std::find_if(model.actions.begin(), model.actions.end(), [&](const copperfin::runtime::XAssetActionBinding& action) {
            return lowercase_copy(action.routine_name) == normalized_routine_name;
        });
        if (found != model.actions.end()) {
            return &(*found);
        }
    }

    return nullptr;
}

bool parse_bool(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    return normalized == "1" || normalized == "true" || normalized == "yes";
}

std::string unescape_manifest_value(std::string value) {
    std::string result;
    result.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '\\' && (index + 1U) < value.size()) {
            const char next = value[index + 1U];
            if (next == 'n') {
                result.push_back('\n');
                ++index;
                continue;
            }
            if (next == 'r') {
                result.push_back('\r');
                ++index;
                continue;
            }
            if (next == '\\') {
                result.push_back('\\');
                ++index;
                continue;
            }
        }
        result.push_back(value[index]);
    }
    return result;
}

using ManifestMap = std::multimap<std::string, std::string>;

ManifestMap load_manifest(const std::string& path) {
    ManifestMap values;
    std::ifstream input(path, std::ios::binary);
    std::string line;
    while (std::getline(input, line)) {
        const auto delimiter = line.find('=');
        if (delimiter == std::string::npos) {
            continue;
        }

        const std::string key = trim_copy(line.substr(0U, delimiter));
        const std::string value = trim_copy(unescape_manifest_value(line.substr(delimiter + 1U)));
        values.emplace(key, value);
    }
    return values;
}

std::string first_value(const ManifestMap& values, const std::string& key) {
    const auto found = values.find(key);
    return found == values.end() ? std::string{} : found->second;
}

std::vector<std::string> all_values(const ManifestMap& values, const std::string& key) {
    std::vector<std::string> result;
    const auto [begin, end] = values.equal_range(key);
    for (auto iterator = begin; iterator != end; ++iterator) {
        result.push_back(iterator->second);
    }
    return result;
}

std::vector<std::string> split_pipe(const std::string& value) {
    std::vector<std::string> result;
    std::string current;
    for (const char ch : value) {
        if (ch == '|') {
            result.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(ch);
    }
    result.push_back(current);
    return result;
}

bool relative_path_escapes_root(const std::filesystem::path& relative_path) {
    for (const auto& part : relative_path) {
        if (part == "..") {
            return true;
        }
    }
    return false;
}

std::optional<std::filesystem::path> bind_packaged_path(
    const std::string& manifest_value,
    const std::string& recorded_package_root,
    const std::filesystem::path& manifest_directory) {
    if (trim_copy(manifest_value).empty()) {
        return std::nullopt;
    }

    const std::filesystem::path recorded_path(manifest_value);
    if (std::filesystem::exists(recorded_path)) {
        return recorded_path.lexically_normal();
    }

    if (recorded_path.is_relative()) {
        const std::filesystem::path relative_candidate =
            (manifest_directory / recorded_path).lexically_normal();
        if (std::filesystem::exists(relative_candidate)) {
            return relative_candidate;
        }
    }

    if (!trim_copy(recorded_package_root).empty()) {
        const std::filesystem::path package_root(recorded_package_root);
        const std::filesystem::path relative =
            recorded_path.lexically_relative(package_root);
        if (!relative.empty() &&
            relative != recorded_path &&
            !relative_path_escapes_root(relative)) {
            const std::filesystem::path rebound =
                (manifest_directory / relative).lexically_normal();
            if (std::filesystem::exists(rebound)) {
                return rebound;
            }
        }
    }

    const std::filesystem::path filename_candidate =
        (manifest_directory / recorded_path.filename()).lexically_normal();
    if (std::filesystem::exists(filename_candidate)) {
        return filename_candidate;
    }

    return std::nullopt;
}

std::string resolve_manifest_bound_directory(
    const ManifestMap& manifest,
    const std::string& key,
    const std::filesystem::path& manifest_directory,
    const std::filesystem::path& fallback_relative_path) {
    const std::string recorded_package_root = first_value(manifest, "package_root");
    if (const auto bound = bind_packaged_path(first_value(manifest, key), recorded_package_root, manifest_directory)) {
        return bound->string();
    }

    const std::filesystem::path fallback_path =
        (manifest_directory / fallback_relative_path).lexically_normal();
    return fallback_path.string();
}

bool verify_manifest_hashes(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory,
    std::string& error) {
    const std::string recorded_package_root = first_value(manifest, "package_root");
    const std::string expected_runtime_host_hash = first_value(manifest, "runtime_host_sha256");
    if (expected_runtime_host_hash.empty()) {
        error = "security-enabled manifest is missing runtime_host_sha256.";
        return false;
    }

    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(
        (manifest_directory / "copperfin_runtime_host.exe").string());
    if (!runtime_host_hash.ok) {
        error = runtime_host_hash.error;
        return false;
    }
    if (lowercase_copy(runtime_host_hash.hex_digest) != lowercase_copy(expected_runtime_host_hash)) {
        error = "runtime host hash does not match manifest digest.";
        return false;
    }

    const auto payload_values = all_values(manifest, "extension_payload");
    for (const auto& payload : payload_values) {
        const auto parts = split_pipe(payload);
        if (parts.size() != 2U) {
            error = "extension_payload entry is malformed in manifest.";
            return false;
        }

        const auto bound_payload_path = bind_packaged_path(parts[0], recorded_package_root, manifest_directory);
        if (!bound_payload_path.has_value()) {
            error = "extension payload is missing from the package: " + std::filesystem::path(parts[0]).filename().string();
            return false;
        }

        const auto digest = copperfin::security::sha256_hex_for_file(bound_payload_path->string());
        if (!digest.ok) {
            error = digest.error;
            return false;
        }
        if (lowercase_copy(digest.hex_digest) != lowercase_copy(parts[1])) {
            error = "extension payload hash mismatch: " + bound_payload_path->filename().string();
            return false;
        }
    }

    return true;
}

void print_usage() {
    std::cout << "Usage: copperfin_runtime_host --manifest <path> [--debug] [--breakpoint <file:line>] [--debug-command <continue|step|next|out|watch:<expr>|select:<action-id>|invoke:<action-id>|break:add:<file:line>|break:remove:<file:line>|break:add-action:<action-id>|break:remove-action:<action-id>|break:clear|break:list>]\n";
    std::cout << "   or: copperfin_runtime_host --federation-backend <sqlite|postgresql|sqlserver|oracle> --federation-query <fox-sql> [--federation-target <name>]\n";
}

std::optional<copperfin::runtime::RuntimeBreakpoint> parse_breakpoint(const std::string& value, const std::string& startup_source) {
    const auto separator = value.rfind(':');
    if (separator == std::string::npos) {
        return copperfin::runtime::RuntimeBreakpoint{
            .file_path = startup_source,
            .line = static_cast<std::size_t>(std::stoull(value))
        };
    }

    return copperfin::runtime::RuntimeBreakpoint{
        .file_path = value.substr(0U, separator),
        .line = static_cast<std::size_t>(std::stoull(value.substr(separator + 1U)))
    };
}

copperfin::runtime::DebugResumeAction parse_resume_action(const std::string& value) {
    const std::string normalized = lowercase_copy(value);
    if (normalized == "step") {
        return copperfin::runtime::DebugResumeAction::step_into;
    }
    if (normalized == "next") {
        return copperfin::runtime::DebugResumeAction::step_over;
    }
    if (normalized == "out") {
        return copperfin::runtime::DebugResumeAction::step_out;
    }
    return copperfin::runtime::DebugResumeAction::continue_run;
}

const copperfin::runtime::XAssetActionBinding* find_breakpoint_xasset_action(
    const copperfin::runtime::RuntimeBreakpoint& breakpoint,
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& bootstrap_path,
    const std::string& bootstrap_source);

void print_pause_state(
    const copperfin::runtime::RuntimePauseState& state,
    const copperfin::runtime::XAssetExecutableModel* xasset_model = nullptr,
    const std::vector<copperfin::runtime::RuntimeBreakpoint>* breakpoints = nullptr,
    const std::string& xasset_bootstrap_path = {},
    const std::string& xasset_bootstrap_source = {}) {
    std::cout << "debug.reason: " << copperfin::runtime::debug_pause_reason_name(state.reason) << "\n";
    std::cout << "debug.location: " << state.location.file_path << ":" << state.location.line << "\n";
    std::cout << "debug.statement: " << state.statement_text << "\n";
    std::cout << "debug.message: " << state.message << "\n";
    std::cout << "debug.stack.depth: " << state.call_stack.size() << "\n";
    std::cout << "debug.executed.statements: " << state.executed_statement_count << "\n";
    if (breakpoints != nullptr) {
        std::cout << "debug.breakpoint.count: " << breakpoints->size() << "\n";
        for (std::size_t index = 0; index < breakpoints->size(); ++index) {
            const auto& breakpoint = (*breakpoints)[index];
            std::cout << "debug.breakpoint[" << index << "]: " << breakpoint.file_path << ":" << breakpoint.line << "\n";
            if (xasset_model != nullptr) {
                if (const auto* action = find_breakpoint_xasset_action(
                        breakpoint,
                        *xasset_model,
                        xasset_bootstrap_path,
                        xasset_bootstrap_source)) {
                    std::cout << "debug.breakpoint[" << index << "].xasset.action_id: " << action->action_id << "\n";
                    std::cout << "debug.breakpoint[" << index << "].xasset.title: " << action->title << "\n";
                }
            }
        }
    }
    if (xasset_model != nullptr) {
        if (const auto* xasset_action = find_pause_xasset_action(state, *xasset_model)) {
            std::cout << "debug.xasset.action_id: " << xasset_action->action_id << "\n";
            std::cout << "debug.xasset.record_index: " << xasset_action->record_index << "\n";
            std::cout << "debug.xasset.kind: " << xasset_action->kind << "\n";
            std::cout << "debug.xasset.title: " << xasset_action->title << "\n";
        }
    }
    std::cout << "debug.workarea.selected: " << state.work_area.selected << "\n";
    std::cout << "debug.datasession.current: " << state.work_area.data_session << "\n";
    for (const auto& [area, alias] : state.work_area.aliases) {
        std::cout << "debug.workarea[" << area << "].alias: " << alias << "\n";
    }
    for (const auto& cursor : state.cursors) {
        std::cout << "debug.cursor[" << cursor.work_area << "].alias: " << cursor.alias << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].source: " << cursor.source_path << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].kind: " << cursor.source_kind << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].remote: " << (cursor.remote ? "true" : "false") << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].reccount: " << cursor.record_count << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].recno: " << cursor.recno << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].bof: " << (cursor.bof ? "true" : "false") << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].eof: " << (cursor.eof ? "true" : "false") << "\n";
    }
    for (const auto& connection : state.sql_connections) {
        std::cout << "debug.sql[" << connection.handle << "].target: " << connection.target << "\n";
        std::cout << "debug.sql[" << connection.handle << "].provider: " << connection.provider << "\n";
        std::cout << "debug.sql[" << connection.handle << "].cursor: " << connection.last_cursor_alias << "\n";
        std::cout << "debug.sql[" << connection.handle << "].rows: " << connection.last_result_count << "\n";
    }
    for (const auto& object : state.ole_objects) {
        std::cout << "debug.ole[" << object.handle << "].progid: " << object.prog_id << "\n";
        std::cout << "debug.ole[" << object.handle << "].source: " << object.source << "\n";
        std::cout << "debug.ole[" << object.handle << "].lastaction: " << object.last_action << "\n";
        std::cout << "debug.ole[" << object.handle << "].actioncount: " << object.action_count << "\n";
    }
    for (std::size_t index = 0; index < state.call_stack.size(); ++index) {
        const auto& frame = state.call_stack[index];
        std::cout << "debug.frame[" << index << "]: " << frame.routine_name << "@" << frame.file_path << ":" << frame.line << "\n";
        for (const auto& [name, value] : frame.locals) {
            std::cout << "debug.frame[" << index << "].local." << name << ": " << copperfin::runtime::format_value(value) << "\n";
        }
    }
    for (const auto& [name, value] : state.globals) {
        std::cout << "debug.global." << name << ": " << copperfin::runtime::format_value(value) << "\n";
    }
    for (std::size_t index = 0; index < state.events.size(); ++index) {
        const auto& event = state.events[index];
        std::cout << "debug.event[" << index << "].category: " << event.category << "\n";
        std::cout << "debug.event[" << index << "].detail: " << event.detail << "\n";
        std::cout << "debug.event[" << index << "].location: " << event.location.file_path << ":" << event.location.line << "\n";
    }
}

void print_breakpoint_inventory(
    const copperfin::runtime::PrgRuntimeSession& session,
    const copperfin::runtime::XAssetExecutableModel* xasset_model = nullptr,
    const std::string& xasset_bootstrap_path = {},
    const std::string& xasset_bootstrap_source = {}) {
    const auto breakpoints = session.list_breakpoints();
    std::cout << "debug.breakpoint.count: " << breakpoints.size() << "\n";
    for (std::size_t index = 0; index < breakpoints.size(); ++index) {
        const auto& breakpoint = breakpoints[index];
        std::cout << "debug.breakpoint[" << index << "]: " << breakpoint.file_path << ":" << breakpoint.line << "\n";
        if (xasset_model != nullptr) {
            if (const auto* action = find_breakpoint_xasset_action(
                    breakpoint,
                    *xasset_model,
                    xasset_bootstrap_path,
                    xasset_bootstrap_source)) {
                std::cout << "debug.breakpoint[" << index << "].xasset.action_id: " << action->action_id << "\n";
                std::cout << "debug.breakpoint[" << index << "].xasset.title: " << action->title << "\n";
            }
        }
    }
}

struct XAssetBootstrapResult {
    std::optional<std::string> bootstrap_path;
    std::string bootstrap_source;
    copperfin::runtime::XAssetExecutableModel model;
    std::string error;
};

XAssetBootstrapResult materialize_xasset_bootstrap(
    const std::string& startup_source,
    bool include_read_events) {
    XAssetBootstrapResult result;
    copperfin::studio::StudioOpenRequest request{};
    request.path = startup_source;
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    if (!open_result.ok) {
        result.error = open_result.error;
        return result;
    }

    result.model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    if (!result.model.ok || !result.model.runnable_startup) {
        result.error = result.model.error.empty()
            ? "No runnable startup methods were found in asset."
            : result.model.error;
        return result;
    }

    const std::filesystem::path startup_path(startup_source);
    const std::filesystem::path bootstrap_path =
        std::filesystem::temp_directory_path() /
        (startup_path.stem().string() + "_copperfin_host_bootstrap.prg");
    result.bootstrap_source =
        copperfin::runtime::build_xasset_bootstrap_source(result.model, include_read_events);

    std::ofstream output(bootstrap_path, std::ios::binary);
    output << result.bootstrap_source;
    output.close();
    if (!output.good()) {
        result.error = "Unable to materialize xAsset bootstrap.";
        return result;
    }

    result.bootstrap_path = bootstrap_path.string();
    return result;
}

std::optional<std::string> resolve_action_routine_name(
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& command) {
    std::size_t prefix_length = 0;
    if (starts_with_insensitive(command, "select:")) {
        prefix_length = 7U;
    } else if (starts_with_insensitive(command, "invoke:")) {
        prefix_length = 7U;
    } else {
        return std::nullopt;
    }

    const std::string action_id = lowercase_copy(trim_copy(command.substr(prefix_length)));
    const auto found = std::find_if(model.actions.begin(), model.actions.end(), [&](const copperfin::runtime::XAssetActionBinding& action) {
        return lowercase_copy(action.action_id) == action_id;
    });
    if (found == model.actions.end()) {
        return std::nullopt;
    }
    return found->routine_name;
}

std::optional<copperfin::runtime::RuntimeBreakpoint> resolve_action_breakpoint(
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& bootstrap_path,
    const std::string& bootstrap_source,
    const std::string& action_id) {
    const std::string normalized_action_id = lowercase_copy(trim_copy(action_id));
    const auto found = std::find_if(model.actions.begin(), model.actions.end(), [&](const copperfin::runtime::XAssetActionBinding& action) {
        return lowercase_copy(action.action_id) == normalized_action_id;
    });
    if (found == model.actions.end()) {
        return std::nullopt;
    }

    std::size_t current_line = 0;
    bool in_target_routine = false;
    std::size_t line_start = 0;
    while (line_start <= bootstrap_source.size()) {
        const std::size_t line_end = bootstrap_source.find('\n', line_start);
        std::string line = bootstrap_source.substr(
            line_start,
            line_end == std::string::npos ? std::string::npos : line_end - line_start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        ++current_line;
        if (line == "PROCEDURE " + found->routine_name) {
            in_target_routine = true;
        } else if (in_target_routine) {
            const std::string trimmed = trim_copy(line);
            if (starts_with_insensitive(trimmed, "ENDPROC")) {
                return std::nullopt;
            }
            if (!trimmed.empty() && trimmed[0] != '*') {
                return copperfin::runtime::RuntimeBreakpoint{
                    .file_path = bootstrap_path,
                    .line = current_line
                };
            }
        }

        if (line_end == std::string::npos) {
            break;
        }
        line_start = line_end + 1U;
    }

    return std::nullopt;
}

const copperfin::runtime::XAssetActionBinding* find_breakpoint_xasset_action(
    const copperfin::runtime::RuntimeBreakpoint& breakpoint,
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& bootstrap_path,
    const std::string& bootstrap_source) {
    for (const auto& action : model.actions) {
        const auto resolved = resolve_action_breakpoint(
            model,
            bootstrap_path,
            bootstrap_source,
            action.action_id);
        if (!resolved.has_value()) {
            continue;
        }
        if (resolved->file_path == breakpoint.file_path && resolved->line == breakpoint.line) {
            return &action;
        }
    }

    return nullptr;
}

std::string resolve_effective_working_directory(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory) {
    return resolve_manifest_bound_directory(
        manifest,
        "working_directory",
        manifest_directory,
        "content");
}

std::string resolve_effective_audit_log_path(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory) {
    return resolve_manifest_bound_directory(
        manifest,
        "audit_log_path",
        manifest_directory,
        "security_audit.log");
}

std::string resolve_startup_source(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory) {
    const std::string recorded_package_root = first_value(manifest, "package_root");
    const std::string startup_source = first_value(manifest, "startup_source");
    if (const auto bound_startup = bind_packaged_path(startup_source, recorded_package_root, manifest_directory)) {
        return bound_startup->string();
    }

    const std::string startup_item = first_value(manifest, "startup_item");
    const std::string content_root = resolve_manifest_bound_directory(
        manifest,
        "content_root",
        manifest_directory,
        "content");
    if (!startup_item.empty() && !content_root.empty()) {
        const std::filesystem::path candidate =
            (std::filesystem::path(content_root) / startup_item).lexically_normal();
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    const std::string working_directory =
        resolve_effective_working_directory(manifest, manifest_directory);
    if (!startup_item.empty() && !working_directory.empty()) {
        const std::filesystem::path candidate =
            (std::filesystem::path(working_directory) / startup_item).lexically_normal();
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    return startup_source;
}

std::string resolve_implicit_manifest_path(const char* argv0) {
    if (argv0 == nullptr || *argv0 == '\0') {
        return {};
    }

    std::error_code path_error;
    std::filesystem::path executable_path(argv0);
    if (executable_path.is_relative()) {
        executable_path = std::filesystem::absolute(executable_path, path_error);
        if (path_error) {
            return {};
        }
    }

    const std::filesystem::path manifest_path =
        executable_path.parent_path() / "app.cfmanifest";
    if (!std::filesystem::exists(manifest_path)) {
        return {};
    }
    return manifest_path.lexically_normal().string();
}

}  // namespace

int main(int argc, char** argv) {
    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied) {
        std::cerr << "warning: " << hardening.message << "\n";
    }

    std::string manifest_path;
    std::string federation_backend;
    std::string federation_query;
    std::string federation_target;
    bool debug_mode = false;
    std::vector<std::string> breakpoint_args;
    std::vector<std::string> debug_commands;

    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--manifest" && (index + 1) < argc) {
            manifest_path = argv[++index];
        } else if (arg == "--federation-backend" && (index + 1) < argc) {
            federation_backend = argv[++index];
        } else if (arg == "--federation-query" && (index + 1) < argc) {
            federation_query = argv[++index];
        } else if (arg == "--federation-target" && (index + 1) < argc) {
            federation_target = argv[++index];
        } else if (arg == "--debug") {
            debug_mode = true;
        } else if (arg == "--breakpoint" && (index + 1) < argc) {
            breakpoint_args.emplace_back(argv[++index]);
        } else if (arg == "--debug-command" && (index + 1) < argc) {
            debug_commands.emplace_back(argv[++index]);
        } else {
            std::cout << "status: error\n";
            std::cout << "error: Unknown argument: " << arg << "\n";
            print_usage();
            return 2;
        }
    }

    const bool federation_mode_requested =
        !trim_copy(federation_backend).empty() || !trim_copy(federation_query).empty();
    if (federation_mode_requested) {
        if (trim_copy(federation_backend).empty() || trim_copy(federation_query).empty()) {
            std::cout << "status: error\n";
            std::cout << "error: --federation-backend and --federation-query are both required in federation mode.\n";
            return 2;
        }

        const auto backend = copperfin::platform::federation_backend_from_string(federation_backend);
        if (!backend.has_value()) {
            std::cout << "status: error\n";
            std::cout << "error: Unknown federation backend: " << federation_backend << "\n";
            return 2;
        }

        const auto plan = copperfin::platform::build_federation_execution_plan({
            .backend = *backend,
            .fox_sql = federation_query,
            .target = federation_target
        });
        if (!plan.ok) {
            std::cout << "status: error\n";
            std::cout << "runtime.mode: federation-query-plan\n";
            std::cout << "error: " << plan.error << "\n";
            return 6;
        }

        std::cout << "status: ok\n";
        std::cout << "runtime.mode: federation-query-plan\n";
        std::cout << "federation.backend: " << copperfin::platform::federation_backend_name(plan.backend) << "\n";
        std::cout << "federation.connector: " << plan.connector << "\n";
        std::cout << "federation.target: " << plan.target << "\n";
        std::cout << "federation.translated_sql: " << plan.translated_sql << "\n";
        std::cout << "federation.command: " << plan.execution_command << "\n";
        return 0;
    }

    if (manifest_path.empty()) {
        manifest_path = resolve_implicit_manifest_path(argc > 0 ? argv[0] : nullptr);
        if (manifest_path.empty()) {
            print_usage();
            return 2;
        }
    }

    if (!std::filesystem::exists(manifest_path)) {
        std::cout << "status: error\n";
        std::cout << "error: Manifest file not found.\n";
        return 3;
    }

    const auto manifest = load_manifest(manifest_path);
    if (manifest.empty()) {
        std::cout << "status: error\n";
        std::cout << "error: Manifest is empty or invalid.\n";
        return 4;
    }

    const std::filesystem::path manifest_directory =
        std::filesystem::path(manifest_path).parent_path().lexically_normal();
    const auto assets = all_values(manifest, "asset");
    const auto warnings = all_values(manifest, "warning");
    const bool security_enabled = parse_bool(first_value(manifest, "security_enabled"));
    const std::string security_role = first_value(manifest, "security_role");
    const std::string audit_log_path =
        resolve_effective_audit_log_path(manifest, manifest_directory);
    const auto security_profile = copperfin::security::default_native_security_profile();

    if (security_enabled) {
        if (!copperfin::security::role_has_permission(security_profile, security_role, "project.open")) {
            if (!audit_log_path.empty()) {
                (void)copperfin::security::append_immutable_audit_event(
                    audit_log_path,
                    "policy.denied",
                    "role missing permission: project.open");
            }
            std::cout << "status: error\n";
            std::cout << "error: Security policy denied project.open for role '" << security_role << "'.\n";
            return 7;
        }

        std::string verification_error;
        if (!verify_manifest_hashes(manifest, manifest_directory, verification_error)) {
            if (!audit_log_path.empty()) {
                (void)copperfin::security::append_immutable_audit_event(
                    audit_log_path,
                    "policy.denied",
                    "hash verification failed: " + verification_error);
            }
            std::cout << "status: error\n";
            std::cout << "error: " << verification_error << "\n";
            return 8;
        }

        if (!audit_log_path.empty()) {
            (void)copperfin::security::append_immutable_audit_event(
                audit_log_path,
                "runtime.start",
                "role=" + security_role + ",manifest=" + manifest_path);
        }
    }
    const std::string startup_source =
        resolve_startup_source(manifest, manifest_directory);
    const std::string working_directory =
        resolve_effective_working_directory(manifest, manifest_directory);
    const std::string startup_extension = lowercase_copy(std::filesystem::path(startup_source).extension().string());
    const bool prg_startup = startup_extension == ".prg";
    copperfin::runtime::XAssetExecutableModel xasset_model;

    std::cout << "status: ok\n";
    std::cout << "project.title: " << first_value(manifest, "project_title") << "\n";
    std::cout << "startup.item: " << first_value(manifest, "startup_item") << "\n";
    std::cout << "startup.source: " << startup_source << "\n";
    std::cout << "working.directory: " << working_directory << "\n";
    std::cout << "security.enabled: " << first_value(manifest, "security_enabled") << "\n";
    std::cout << "security.role: " << security_role << "\n";
    std::cout << "security.mode: " << first_value(manifest, "security_mode") << "\n";
    std::cout << "dotnet.story: " << first_value(manifest, "dotnet_story") << "\n";
    std::cout << "asset.count: " << assets.size() << "\n";
    std::cout << "warning.count: " << warnings.size() << "\n";

    std::string effective_startup_source = startup_source;
    std::string runtime_mode = "prg-engine";
    std::string xasset_bootstrap_source;
    if (!prg_startup) {
        const auto bootstrap = materialize_xasset_bootstrap(startup_source, true);
        xasset_model = bootstrap.model;
        if (!bootstrap.bootstrap_path.has_value()) {
            std::cout << "runtime.mode: compatibility-launcher\n";
            std::cout << "launch.note: Startup asset is not a PRG file. PRG execution is real; xBase code embedded in SCX/VCX/FRX/MNX/LBX assets is a later runtime slice.\n";
            std::cout << "launch.note: " << bootstrap.error << "\n";
            std::cout << "debug.breakpoint_support: false\n";
            std::cout << "debug.step_support: false\n";
            return 0;
        }
        effective_startup_source = *bootstrap.bootstrap_path;
        xasset_bootstrap_source = bootstrap.bootstrap_source;
        runtime_mode = "xasset-bootstrap";
    }

    copperfin::runtime::RuntimeSessionOptions session_options{};
    session_options.startup_path = effective_startup_source;
    session_options.working_directory = working_directory;
    session_options.stop_on_entry = false;
    session_options.quit_confirm_callback = []() -> bool {
            std::cerr << "\nDo you want to quit this application? [y/N]: ";
            std::cerr.flush();
            std::string answer;
            if (!std::getline(std::cin, answer)) {
                return true;  // EOF or non-interactive stdin — allow quit
            }
            return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
        };
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);
    for (const auto& breakpoint_arg : breakpoint_args) {
        if (const auto breakpoint = parse_breakpoint(breakpoint_arg, effective_startup_source)) {
            session.add_breakpoint(*breakpoint);
        }
    }

    std::cout << "runtime.mode: " << runtime_mode << "\n";
    std::cout << "debug.breakpoint_support: true\n";
    std::cout << "debug.step_support: true\n";

    copperfin::runtime::RuntimePauseState state;
    if (!debug_mode) {
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    } else if (debug_commands.empty()) {
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto breakpoints = session.list_breakpoints();
        print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
    } else {
        for (std::size_t index = 0; index < debug_commands.size(); ++index) {
            const std::string& command = debug_commands[index];
            if (security_enabled && !copperfin::security::role_has_permission(security_profile, security_role, "runtime.admin")) {
                if (!audit_log_path.empty()) {
                    (void)copperfin::security::append_immutable_audit_event(
                        audit_log_path,
                        "policy.denied",
                        "role missing permission: runtime.admin");
                }
                std::cout << "status: error\n";
                std::cout << "error: Security policy denied runtime.admin for role '" << security_role << "'.\n";
                return 9;
            }

            if (starts_with_insensitive(command, "select:") || starts_with_insensitive(command, "invoke:")) {
                const auto action_routine = resolve_action_routine_name(xasset_model, command);
                if (!action_routine.has_value()) {
                    std::cout << "status: error\n";
                    std::cout << "error: Unknown xAsset action: " << command << "\n";
                    return 5;
                }
                if (!session.dispatch_event_handler(*action_routine)) {
                    std::cout << "status: error\n";
                    std::cout << "error: Unable to dispatch xAsset action: " << command << "\n";
                    return 5;
                }
                state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            } else if (starts_with_insensitive(command, "watch:")) {
                if (!state.paused || state.completed) {
                    std::cout << "status: error\n";
                    std::cout << "error: Watch evaluation requires an active paused state.\n";
                    return 5;
                }
                const auto watch = session.evaluate_watch_expression(command.substr(6U));
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                std::cout << "debug.watch.expression: " << watch.expression << "\n";
                std::cout << "debug.watch.ok: " << (watch.ok ? "true" : "false") << "\n";
                if (watch.ok) {
                    std::cout << "debug.watch.value: " << copperfin::runtime::format_value(watch.value) << "\n";
                } else {
                    std::cout << "debug.watch.error: " << watch.message << "\n";
                }
                const auto breakpoints = session.list_breakpoints();
                print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:add:")) {
                const auto breakpoint = parse_breakpoint(command.substr(10U), effective_startup_source);
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    std::cout << "error: Invalid breakpoint command: " << command << "\n";
                    return 5;
                }
                session.add_breakpoint(*breakpoint);
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:remove:")) {
                const auto breakpoint = parse_breakpoint(command.substr(13U), effective_startup_source);
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    std::cout << "error: Invalid breakpoint command: " << command << "\n";
                    return 5;
                }
                if (!session.remove_breakpoint(*breakpoint)) {
                    std::cout << "status: error\n";
                    std::cout << "error: Unknown breakpoint: " << breakpoint->file_path << ":" << breakpoint->line << "\n";
                    return 5;
                }
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:add-action:")) {
                if (runtime_mode != "xasset-bootstrap") {
                    std::cout << "status: error\n";
                    std::cout << "error: xAsset action breakpoints require xasset-bootstrap mode.\n";
                    return 5;
                }
                const auto breakpoint = resolve_action_breakpoint(
                    xasset_model,
                    effective_startup_source,
                    xasset_bootstrap_source,
                    command.substr(17U));
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    std::cout << "error: Unknown or non-breakpointable xAsset action: " << trim_copy(command.substr(17U)) << "\n";
                    return 5;
                }
                session.add_breakpoint(*breakpoint);
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:remove-action:")) {
                if (runtime_mode != "xasset-bootstrap") {
                    std::cout << "status: error\n";
                    std::cout << "error: xAsset action breakpoints require xasset-bootstrap mode.\n";
                    return 5;
                }
                const auto breakpoint = resolve_action_breakpoint(
                    xasset_model,
                    effective_startup_source,
                    xasset_bootstrap_source,
                    command.substr(20U));
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    std::cout << "error: Unknown or non-breakpointable xAsset action: " << trim_copy(command.substr(20U)) << "\n";
                    return 5;
                }
                if (!session.remove_breakpoint(*breakpoint)) {
                    std::cout << "status: error\n";
                    std::cout << "error: Unknown breakpoint for xAsset action: " << trim_copy(command.substr(20U)) << "\n";
                    return 5;
                }
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (lowercase_copy(trim_copy(command)) == "break:clear") {
                session.clear_breakpoints();
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (lowercase_copy(trim_copy(command)) == "break:list") {
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else {
                state = session.run(parse_resume_action(command));
            }
            std::cout << "debug.command[" << index << "]: " << command << "\n";
            const auto breakpoints = session.list_breakpoints();
            print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
            if (state.completed || state.reason == copperfin::runtime::DebugPauseReason::error) {
                break;
            }
        }
    }

    if (!debug_mode) {
        std::cout << "runtime.completed: " << (state.completed ? "true" : "false") << "\n";
        std::cout << "runtime.waiting_for_events: " << (state.waiting_for_events ? "true" : "false") << "\n";
        std::cout << "runtime.reason: " << copperfin::runtime::debug_pause_reason_name(state.reason) << "\n";
        std::cout << "runtime.executed.statements: " << state.executed_statement_count << "\n";
    }

    if (security_enabled && !audit_log_path.empty()) {
        (void)copperfin::security::append_immutable_audit_event(
            audit_log_path,
            "runtime.complete",
            std::string("completed=") + (state.completed ? "true" : "false") + ",reason=" + copperfin::runtime::debug_pause_reason_name(state.reason));
    }

    return state.reason == copperfin::runtime::DebugPauseReason::error ? 5 : 0;
}
