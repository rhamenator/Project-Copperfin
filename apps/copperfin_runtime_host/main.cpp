#include "copperfin/runtime/prg_engine.h"

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

void print_usage() {
    std::cout << "Usage: copperfin_runtime_host --manifest <path> [--debug] [--breakpoint <file:line>] [--debug-command <continue|step|next|out>]\n";
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
    for (std::size_t index = 0; index < state.call_stack.size(); ++index) {
        const auto& frame = state.call_stack[index];
        std::cout << "debug.frame[" << index << "]: " << frame.routine_name << "@" << frame.file_path << ":" << frame.line << "\n";
    }
    for (const auto& [name, value] : state.globals) {
        std::cout << "debug.global." << name << ": " << copperfin::runtime::format_value(value) << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::string manifest_path;
    bool debug_mode = false;
    std::vector<std::string> breakpoint_args;
    std::vector<std::string> debug_commands;

    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--manifest" && (index + 1) < argc) {
            manifest_path = argv[++index];
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
    const std::string startup_source = first_value(manifest, "startup_source");
    const std::string working_directory = first_value(manifest, "working_directory");
    const std::string startup_extension = lowercase_copy(std::filesystem::path(startup_source).extension().string());
    const bool prg_startup = startup_extension == ".prg";

    std::cout << "status: ok\n";
    std::cout << "project.title: " << first_value(manifest, "project_title") << "\n";
    std::cout << "startup.item: " << first_value(manifest, "startup_item") << "\n";
    std::cout << "startup.source: " << startup_source << "\n";
    std::cout << "working.directory: " << working_directory << "\n";
    std::cout << "security.enabled: " << first_value(manifest, "security_enabled") << "\n";
    std::cout << "security.mode: " << first_value(manifest, "security_mode") << "\n";
    std::cout << "dotnet.story: " << first_value(manifest, "dotnet_story") << "\n";
    std::cout << "asset.count: " << assets.size() << "\n";
    std::cout << "warning.count: " << warnings.size() << "\n";

    if (!prg_startup) {
        std::cout << "runtime.mode: compatibility-launcher\n";
        std::cout << "launch.note: Startup asset is not a PRG file. PRG execution is real; xBase code embedded in SCX/VCX/FRX/MNX/LBX assets is a later runtime slice.\n";
        std::cout << "debug.breakpoint_support: false\n";
        std::cout << "debug.step_support: false\n";
        return 0;
    }

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = startup_source,
        .working_directory = working_directory,
        .stop_on_entry = false
    });
    for (const auto& breakpoint_arg : breakpoint_args) {
        if (const auto breakpoint = parse_breakpoint(breakpoint_arg, startup_source)) {
            session.add_breakpoint(*breakpoint);
        }
    }

    std::cout << "runtime.mode: prg-engine\n";
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
            state = session.run(parse_resume_action(debug_commands[index]));
            std::cout << "debug.command[" << index << "]: " << debug_commands[index] << "\n";
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

    return state.reason == copperfin::runtime::DebugPauseReason::error ? 5 : 0;
}
