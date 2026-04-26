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

bool verify_manifest_hashes(const ManifestMap& manifest, std::string& error) {
    const std::string expected_runtime_host_hash = first_value(manifest, "runtime_host_sha256");
    if (expected_runtime_host_hash.empty()) {
        error = "security-enabled manifest is missing runtime_host_sha256.";
        return false;
    }

    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file("copperfin_runtime_host.exe");
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

        const std::filesystem::path payload_path(parts[0]);
        const std::filesystem::path file_name = payload_path.filename();
        const auto digest = copperfin::security::sha256_hex_for_file(file_name.string());
        if (!digest.ok) {
            error = digest.error;
            return false;
        }
        if (lowercase_copy(digest.hex_digest) != lowercase_copy(parts[1])) {
            error = "extension payload hash mismatch: " + file_name.string();
            return false;
        }
    }

    return true;
}

void print_usage() {
    std::cout << "Usage: copperfin_runtime_host --manifest <path> [--debug] [--breakpoint <file:line>] [--debug-command <continue|step|next|out|select:<action-id>|invoke:<action-id>>]\n";
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

void print_pause_state(const copperfin::runtime::RuntimePauseState& state) {
    std::cout << "debug.reason: " << copperfin::runtime::debug_pause_reason_name(state.reason) << "\n";
    std::cout << "debug.location: " << state.location.file_path << ":" << state.location.line << "\n";
    std::cout << "debug.statement: " << state.statement_text << "\n";
    std::cout << "debug.message: " << state.message << "\n";
    std::cout << "debug.stack.depth: " << state.call_stack.size() << "\n";
    std::cout << "debug.executed.statements: " << state.executed_statement_count << "\n";
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

struct XAssetBootstrapResult {
    std::optional<std::string> bootstrap_path;
    copperfin::runtime::XAssetExecutableModel model;
    std::string error;
};

XAssetBootstrapResult materialize_xasset_bootstrap(
    const std::string& startup_source,
    bool include_read_events) {
    XAssetBootstrapResult result;
    const auto open_result = copperfin::studio::open_document({
        .path = startup_source,
        .read_only = true,
        .load_full_table = true
    });
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

    std::ofstream output(bootstrap_path, std::ios::binary);
    output << copperfin::runtime::build_xasset_bootstrap_source(result.model, include_read_events);
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

std::string resolve_startup_source(const ManifestMap& manifest) {
    const std::string startup_source = first_value(manifest, "startup_source");
    if (!startup_source.empty() && std::filesystem::exists(startup_source)) {
        return std::filesystem::path(startup_source).lexically_normal().string();
    }

    const std::string startup_item = first_value(manifest, "startup_item");
    const std::string working_directory = first_value(manifest, "working_directory");
    if (!startup_item.empty() && !working_directory.empty()) {
        const std::filesystem::path candidate =
            (std::filesystem::path(working_directory) / startup_item).lexically_normal();
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    const std::string content_root = first_value(manifest, "content_root");
    if (!startup_item.empty() && !content_root.empty()) {
        const std::filesystem::path candidate =
            (std::filesystem::path(content_root) / startup_item).lexically_normal();
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    return startup_source;
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
        print_usage();
        return 2;
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

    const auto assets = all_values(manifest, "asset");
    const auto warnings = all_values(manifest, "warning");
    const bool security_enabled = parse_bool(first_value(manifest, "security_enabled"));
    const std::string security_role = first_value(manifest, "security_role");
    const std::string audit_log_path = first_value(manifest, "audit_log_path");
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
        if (!verify_manifest_hashes(manifest, verification_error)) {
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
    const std::string startup_source = resolve_startup_source(manifest);
    const std::string working_directory = first_value(manifest, "working_directory");
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
        runtime_mode = "xasset-bootstrap";
    }

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = effective_startup_source,
        .working_directory = working_directory,
        .stop_on_entry = false,
        .quit_confirm_callback = []() -> bool {
            std::cerr << "\nDo you want to quit this application? [y/N]: ";
            std::cerr.flush();
            std::string answer;
            if (!std::getline(std::cin, answer)) {
                return true;  // EOF or non-interactive stdin — allow quit
            }
            return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
        }
    });
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
        print_pause_state(state);
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
            } else {
                state = session.run(parse_resume_action(command));
            }
            std::cout << "debug.command[" << index << "]: " << command << "\n";
            print_pause_state(state);
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
