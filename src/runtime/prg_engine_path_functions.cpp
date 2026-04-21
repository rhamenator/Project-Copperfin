#include "prg_engine_path_functions.h"

#include "prg_engine_helpers.h"

#include <filesystem>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_path_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::string& default_directory) {
    if (function == "fullpath" && !arguments.empty()) {
        std::filesystem::path path(value_as_string(arguments[0]));
        if (path.is_relative()) {
            path = std::filesystem::path(default_directory) / path;
        }
        return make_string_value(std::filesystem::absolute(path).string());
    }
    if (function == "justfname" && !arguments.empty()) {
        return make_string_value(portable_path_filename(value_as_string(arguments[0])));
    }
    if (function == "justpath" && !arguments.empty()) {
        return make_string_value(portable_path_parent(value_as_string(arguments[0])));
    }
    if (function == "juststem" && !arguments.empty()) {
        return make_string_value(portable_path_stem(value_as_string(arguments[0])));
    }
    if (function == "justext" && !arguments.empty()) {
        return make_string_value(portable_path_extension(value_as_string(arguments[0])));
    }
    if (function == "justdrive" && !arguments.empty()) {
        return make_string_value(portable_path_drive(value_as_string(arguments[0])));
    }
    if (function == "forceext" && arguments.size() >= 2U) {
        return make_string_value(portable_force_extension(value_as_string(arguments[0]), value_as_string(arguments[1])));
    }
    if (function == "forcepath" && arguments.size() >= 2U) {
        return make_string_value(portable_force_path(value_as_string(arguments[0]), value_as_string(arguments[1])));
    }
    if (function == "addbs" && !arguments.empty()) {
        std::string path = value_as_string(arguments[0]);
        if (!path.empty() && path.back() != '\\' && path.back() != '/') {
            path += '\\';
        }
        return make_string_value(std::move(path));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
