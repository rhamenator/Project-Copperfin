#define _CRT_SECURE_NO_WARNINGS
#include "prg_engine_file_io_functions.h"

#include "prg_engine_helpers.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace copperfin::runtime {

namespace {

struct OpenFileHandle {
    std::FILE* file = nullptr;
    std::filesystem::path path;
};

std::unordered_map<int, OpenFileHandle>& open_file_handles() {
    static std::unordered_map<int, OpenFileHandle> handles;
    return handles;
}

int& next_file_handle_id() {
    static int next_handle = 1;
    return next_handle;
}

std::filesystem::path resolve_file_path(const std::string& raw_path, const std::string& default_directory) {
    std::filesystem::path path(unquote_string(raw_path));
    if (path.empty()) {
        path = std::filesystem::path(default_directory);
    }
    if (path.is_relative()) {
        path = std::filesystem::path(default_directory) / path;
    }
    return path.lexically_normal();
}

OpenFileHandle* resolve_open_handle(int handle) {
    auto& handles = open_file_handles();
    const auto found = handles.find(handle);
    return found == handles.end() ? nullptr : &found->second;
}

std::string fopen_mode_from_value(const PrgValue& mode_value) {
    if (mode_value.kind == PrgValueKind::string) {
        const std::string raw_mode = trim_copy(value_as_string(mode_value));
        if (!raw_mode.empty()) {
            return raw_mode;
        }
    }

    const int mode = static_cast<int>(std::llround(value_as_number(mode_value)));
    if (mode == 1) {
        return "wb";
    }
    if (mode == 2) {
        return "rb+";
    }
    return "rb";
}

std::string trim_newline(std::string value) {
    while (!value.empty() && (value.back() == '\r' || value.back() == '\n')) {
        value.pop_back();
    }
    return value;
}

}  // namespace

std::optional<PrgValue> evaluate_file_io_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::string& default_directory) {
    if (function == "fopen" && !arguments.empty()) {
        const std::filesystem::path path = resolve_file_path(value_as_string(arguments[0]), default_directory);
        const std::string mode = arguments.size() >= 2U ? fopen_mode_from_value(arguments[1]) : std::string{"rb"};

        std::FILE* opened = std::fopen(path.string().c_str(), mode.c_str());
        if (opened == nullptr) {
            return make_number_value(-1.0);
        }

        const int handle = next_file_handle_id()++;
        open_file_handles()[handle] = OpenFileHandle{.file = opened, .path = path};
        return make_number_value(static_cast<double>(handle));
    }

    if (function == "fclose" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_number_value(-1.0);
        }

        const int result = std::fclose(opened->file);
        open_file_handles().erase(handle);
        return make_number_value(result == 0 ? 0.0 : -1.0);
    }

    if (function == "fread" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_string_value(std::string{});
        }

        const std::size_t requested = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])));
        std::string buffer(requested, '\0');
        if (requested == 0U) {
            return make_string_value(std::string{});
        }

        const std::size_t read = std::fread(buffer.data(), 1U, requested, opened->file);
        buffer.resize(read);
        return make_string_value(std::move(buffer));
    }

    if (function == "fwrite" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_number_value(-1.0);
        }

        std::string text = value_as_string(arguments[1]);
        if (arguments.size() >= 3U) {
            const std::size_t requested = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])));
            if (requested < text.size()) {
                text.resize(requested);
            }
        }

        const std::size_t written = std::fwrite(text.data(), 1U, text.size(), opened->file);
        return make_number_value(static_cast<double>(written));
    }

    if (function == "fgets" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_string_value(std::string{});
        }

        const std::size_t max_length = arguments.size() >= 2U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[1])))
                                           : 4096U;
        std::string buffer(max_length + 1U, '\0');
        if (std::fgets(buffer.data(), static_cast<int>(buffer.size()), opened->file) == nullptr) {
            return make_string_value(std::string{});
        }

        buffer.resize(std::strlen(buffer.c_str()));
        return make_string_value(trim_newline(std::move(buffer)));
    }

    if (function == "fputs" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_number_value(-1.0);
        }

        std::string text = value_as_string(arguments[1]);
        if (arguments.size() >= 3U) {
            const std::size_t max_length = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])));
            if (max_length < text.size()) {
                text.resize(max_length);
            }
        }
        text.push_back('\n');

        const std::size_t written = std::fwrite(text.data(), 1U, text.size(), opened->file);
        return make_number_value(static_cast<double>(written));
    }

    if (function == "fseek" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_number_value(-1.0);
        }

        const long offset = static_cast<long>(std::llround(value_as_number(arguments[1])));
        const int origin_mode = arguments.size() >= 3U ? static_cast<int>(std::llround(value_as_number(arguments[2]))) : 0;
        int origin = SEEK_SET;
        if (origin_mode == 1) {
            origin = SEEK_CUR;
        } else if (origin_mode == 2) {
            origin = SEEK_END;
        }

        if (std::fseek(opened->file, offset, origin) != 0) {
            return make_number_value(-1.0);
        }
        const long position = std::ftell(opened->file);
        return make_number_value(position < 0 ? -1.0 : static_cast<double>(position));
    }

    if (function == "ftell" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_number_value(-1.0);
        }

        const long position = std::ftell(opened->file);
        return make_number_value(position < 0 ? -1.0 : static_cast<double>(position));
    }

    if (function == "feof" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_boolean_value(true);
        }

        return make_boolean_value(std::feof(opened->file) != 0);
    }

    if (function == "fflush" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_number_value(-1.0);
        }

        return make_number_value(std::fflush(opened->file) == 0 ? 0.0 : -1.0);
    }

    if (function == "fchsize" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            return make_number_value(-1.0);
        }

        const long long requested_size = static_cast<long long>(std::max(0.0, value_as_number(arguments[1])));
        std::fflush(opened->file);
#if defined(_WIN32)
        const int fd = _fileno(opened->file);
        const int result = fd >= 0 ? _chsize_s(fd, static_cast<std::size_t>(requested_size)) : -1;
#else
        const int fd = fileno(opened->file);
        const int result = fd >= 0 ? ftruncate(fd, static_cast<off_t>(requested_size)) : -1;
#endif
        return make_number_value(result == 0 ? 0.0 : -1.0);
    }

    if (function == "filetostr" && !arguments.empty()) {
        std::ifstream input(resolve_file_path(value_as_string(arguments[0]), default_directory), std::ios::binary);
        if (!input.good()) {
            return make_string_value(std::string{});
        }

        std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        return make_string_value(std::move(content));
    }

    if (function == "strtofile" && arguments.size() >= 2U) {
        const std::filesystem::path path = resolve_file_path(value_as_string(arguments[1]), default_directory);
        std::error_code ignored;
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path(), ignored);
        }

        const bool additive = arguments.size() >= 3U && value_as_bool(arguments[2]);
        std::ofstream output(
            path,
            std::ios::binary | (additive ? std::ios::app : std::ios::trunc));
        if (!output.good()) {
            return make_number_value(-1.0);
        }

        const std::string text = value_as_string(arguments[0]);
        output.write(text.data(), static_cast<std::streamsize>(text.size()));
        if (!output.good()) {
            return make_number_value(-1.0);
        }

        return make_number_value(static_cast<double>(text.size()));
    }

    return std::nullopt;
}

void close_all_file_io_handles() {
    auto& handles = open_file_handles();
    for (auto& [_, opened] : handles) {
        if (opened.file != nullptr) {
            std::fclose(opened.file);
            opened.file = nullptr;
        }
    }
    handles.clear();
}

}  // namespace copperfin::runtime
