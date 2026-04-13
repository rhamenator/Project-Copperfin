#include "prg_engine_runtime_config.h"

#include "prg_engine_helpers.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace copperfin::runtime {

namespace {

std::string strip_inline_config_comment(const std::string& line) {
    const std::size_t and_comment = line.find("&&");
    return trim_copy(line.substr(0U, and_comment));
}

std::string normalize_config_key(std::string key) {
    key = uppercase_copy(trim_copy(key));
    key.erase(std::remove_if(key.begin(), key.end(), [](unsigned char ch) {
        return std::isspace(ch) || ch == '_';
    }), key.end());
    return key;
}

std::optional<std::size_t> parse_size_option(const std::string& value) {
    const std::string trimmed = trim_copy(value);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    try {
        const std::size_t parsed = static_cast<std::size_t>(std::stoull(trimmed));
        if (parsed == 0U) {
            return std::nullopt;
        }
        return parsed;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<RuntimeConfigFile> try_load_runtime_config_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input.good()) {
        return std::nullopt;
    }

    RuntimeConfigFile config;
    config.source_path = path.lexically_normal().string();
    std::string line;
    while (std::getline(input, line)) {
        std::string trimmed = trim_copy(strip_inline_config_comment(line));
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed[0] == '*' || trimmed[0] == ';' || trimmed[0] == '#') {
            continue;
        }

        std::string raw_key;
        std::string raw_value;
        const std::size_t equals = trimmed.find('=');
        if (equals != std::string::npos) {
            raw_key = trimmed.substr(0U, equals);
            raw_value = trimmed.substr(equals + 1U);
        } else {
            std::istringstream tokens(trimmed);
            tokens >> raw_key;
            std::getline(tokens, raw_value);
        }

        const std::string key = normalize_config_key(raw_key);
        const std::string value = unquote_string(trim_copy(raw_value));

        if (key == "MAXCALLDEPTH") {
            if (const auto parsed = parse_size_option(value)) {
                config.max_call_depth = *parsed;
            }
        } else if (key == "MAXEXECUTEDSTATEMENTS") {
            if (const auto parsed = parse_size_option(value)) {
                config.max_executed_statements = *parsed;
            }
        } else if (key == "MAXLOOPITERATIONS") {
            if (const auto parsed = parse_size_option(value)) {
                config.max_loop_iterations = *parsed;
            }
        } else if (key == "TMPFILES" || key == "TEMPDIRECTORY" || key == "TMPDIR") {
            if (!value.empty()) {
                config.temp_directory = value;
            }
        } else if (key == "SCHEDULERYIELDSTATEMENTINTERVAL") {
            if (const auto parsed = parse_size_option(value)) {
                config.scheduler_yield_statement_interval = *parsed;
            }
        } else if (key == "SCHEDULERYIELDSLEEPMS") {
            if (const auto parsed = parse_size_option(value)) {
                config.scheduler_yield_sleep_ms = *parsed;
            }
        }
    }

    return config;
}

}  // namespace

std::optional<RuntimeConfigFile> load_runtime_config_near(
    const std::filesystem::path& startup_path,
    const std::filesystem::path& working_directory) {
    const std::vector<std::filesystem::path> candidate_roots = {
        working_directory,
        startup_path.parent_path()
    };
    for (const auto& root : candidate_roots) {
        if (root.empty()) {
            continue;
        }
        for (const std::string name : {"config.fpw", "config.fp", "CONFIG.FPW", "CONFIG.FP"}) {
            if (auto loaded = try_load_runtime_config_file(root / name)) {
                return loaded;
            }
        }
    }

    return std::nullopt;
}

void apply_runtime_config_defaults(RuntimeSessionOptions& options, const RuntimeConfigFile& config) {
    if (options.max_call_depth == k_default_max_call_depth && config.max_call_depth.has_value()) {
        options.max_call_depth = *config.max_call_depth;
    }
    if (options.max_executed_statements == k_default_max_executed_statements && config.max_executed_statements.has_value()) {
        options.max_executed_statements = *config.max_executed_statements;
    }
    if (options.max_loop_iterations == k_default_max_loop_iterations && config.max_loop_iterations.has_value()) {
        options.max_loop_iterations = *config.max_loop_iterations;
    }
    if (options.scheduler_yield_statement_interval == k_default_yield_statement_interval &&
        config.scheduler_yield_statement_interval.has_value()) {
        options.scheduler_yield_statement_interval = *config.scheduler_yield_statement_interval;
    }
    if (options.scheduler_yield_sleep_ms == k_default_yield_sleep_ms && config.scheduler_yield_sleep_ms.has_value()) {
        options.scheduler_yield_sleep_ms = *config.scheduler_yield_sleep_ms;
    }
    if (trim_copy(options.temp_directory).empty() && config.temp_directory.has_value()) {
        options.temp_directory = *config.temp_directory;
    }
}

std::filesystem::path choose_runtime_temp_directory(const RuntimeSessionOptions& options) {
    if (!trim_copy(options.temp_directory).empty()) {
        std::error_code ignored;
        std::filesystem::path explicit_path = std::filesystem::path(trim_copy(options.temp_directory));
        if (explicit_path.is_relative()) {
            explicit_path = std::filesystem::path(options.working_directory) / explicit_path;
        }
        std::filesystem::create_directories(explicit_path, ignored);
        if (std::filesystem::exists(explicit_path)) {
            return explicit_path.lexically_normal();
        }
    }

    std::error_code ignored;
    const std::filesystem::path os_temp = std::filesystem::temp_directory_path(ignored);
    if (!os_temp.empty()) {
        return os_temp.lexically_normal();
    }
    if (!options.working_directory.empty()) {
        return std::filesystem::path(options.working_directory).lexically_normal();
    }
    return std::filesystem::current_path(ignored).lexically_normal();
}

}  // namespace copperfin::runtime
