// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#define _CRT_SECURE_NO_WARNINGS
#include "prg_engine_file_io_functions.h"

#include "copperfin/platform/path.h"
#include "prg_engine_helpers.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
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
    std::string verified_bytes;
    std::size_t verified_position = 0U;
    bool verified_read = false;
    bool verified_eof = false;
};

std::unordered_map<int, OpenFileHandle>& open_file_handles() {
    static std::unordered_map<int, OpenFileHandle> handles;
    return handles;
}

int& next_file_handle_id() {
    static int next_handle = 1;
    return next_handle;
}

bool is_windows_drive_absolute_path(const std::string& value) {
    return value.size() >= 3U &&
        std::isalpha(static_cast<unsigned char>(value[0])) != 0 &&
        value[1] == ':' &&
        (value[2] == '\\' || value[2] == '/');
}

bool is_unc_path(const std::string& value) {
    return value.size() >= 2U &&
        ((value[0] == '\\' && value[1] == '\\') || (value[0] == '/' && value[1] == '/'));
}

std::string normalize_relative_path_separators(std::string value) {
    std::replace(
        value.begin(),
        value.end(),
        '\\',
        static_cast<char>(std::filesystem::path::preferred_separator));
    return value;
}

std::filesystem::path resolve_file_path(const std::string& raw_path, const std::string& default_directory) {
    const std::string unquoted = unquote_string(raw_path);
    if (unquoted.empty()) {
        return copperfin::platform::path_from_utf8_string(default_directory).lexically_normal();
    }

    if (is_windows_drive_absolute_path(unquoted) || is_unc_path(unquoted)) {
        return copperfin::platform::path_from_utf8_string(unquoted);
    }

    std::filesystem::path path = copperfin::platform::path_from_utf8_string(
        normalize_relative_path_separators(unquoted));
    if (path.is_relative()) {
        path = copperfin::platform::path_from_utf8_string(default_directory) / path;
    }
    return path.lexically_normal();
}

std::FILE* open_file_utf8(const std::filesystem::path& path, const std::string& mode) {
#if defined(_WIN32)
    std::wstring wide_mode;
    wide_mode.reserve(mode.size());
    for (const unsigned char ch : mode) {
        wide_mode.push_back(static_cast<wchar_t>(ch));
    }
    return ::_wfopen(path.c_str(), wide_mode.c_str());
#else
    return std::fopen(path.c_str(), mode.c_str());
#endif
}

OpenFileHandle* resolve_open_handle(int handle) {
    auto& handles = open_file_handles();
    const auto found = handles.find(handle);
    return found == handles.end() ? nullptr : &found->second;
}

int& last_file_error_code() {
    static int error_code = 0;
    return error_code;
}

void clear_file_error() {
    last_file_error_code() = 0;
}

void set_file_error_from_errno(int fallback_code = 31) {
    switch (errno) {
    case ENOENT:
        last_file_error_code() = 2;
        break;
    case EMFILE:
    case ENFILE:
        last_file_error_code() = 4;
        break;
    case EACCES:
    case EPERM:
        last_file_error_code() = 5;
        break;
    case EBADF:
        last_file_error_code() = 6;
        break;
    case ENOMEM:
        last_file_error_code() = 8;
        break;
    case ENOSPC:
        last_file_error_code() = 29;
        break;
    default:
        last_file_error_code() = fallback_code;
        break;
    }
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
    if (mode == 10) {
        return "rb";
    }
    if (mode == 11) {
        return "wb";
    }
    if (mode == 12) {
        return "rb+";
    }
    return "rb";
}

bool fopen_numeric_read_write_mode(const PrgValue& mode_value) {
    if (mode_value.kind == PrgValueKind::string) {
        return false;
    }
    const int mode = static_cast<int>(std::llround(value_as_number(mode_value)));
    return mode == 2 || mode == 12;
}

bool fopen_read_only_mode(const std::string& mode) {
    return mode.find('r') != std::string::npos &&
        mode.find('+') == std::string::npos &&
        mode.find('w') == std::string::npos &&
        mode.find('a') == std::string::npos;
}

bool is_open_handle(const OpenFileHandle* handle) {
    return handle != nullptr && (handle->file != nullptr || handle->verified_read);
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
    const std::string& default_directory,
    bool require_verified_file_byte_overrides,
    const std::function<std::optional<std::string>(const std::filesystem::path&)>& read_verified_file_callback,
    const std::function<void(const std::filesystem::path&)>& verified_file_unavailable_callback) {
    if (function == "ferror" && arguments.empty()) {
        return make_number_value(static_cast<double>(last_file_error_code()));
    }

    if (function == "fopen" && !arguments.empty()) {
        const std::filesystem::path path = resolve_file_path(value_as_string(arguments[0]), default_directory);
        const std::string mode = arguments.size() >= 2U ? fopen_mode_from_value(arguments[1]) : std::string{"rb"};

        if (require_verified_file_byte_overrides && fopen_read_only_mode(mode)) {
            const auto verified = read_verified_file_callback ? read_verified_file_callback(path) : std::nullopt;
            if (!verified.has_value()) {
                last_file_error_code() = 5;
                return make_number_value(-1.0);
            }

            const int handle = next_file_handle_id()++;
            open_file_handles()[handle] = OpenFileHandle{
                .path = path,
                .verified_bytes = *verified,
                .verified_read = true,
                .verified_eof = false};
            clear_file_error();
            return make_number_value(static_cast<double>(handle));
        }

        std::FILE* opened = open_file_utf8(path, mode);
        if (opened == nullptr &&
            fopen_numeric_read_write_mode(arguments.size() >= 2U ? arguments[1] : make_number_value(0.0)) &&
            errno == ENOENT) {
            // rb+ preserves existing contents; only create a missing file after
            // that first open proves the path does not exist.
            opened = open_file_utf8(path, "wb+");
        }
        if (opened == nullptr) {
            set_file_error_from_errno();
            return make_number_value(-1.0);
        }

        const int handle = next_file_handle_id()++;
        open_file_handles()[handle] = OpenFileHandle{
            .file = opened,
            .path = path,
            .verified_bytes = {},
            .verified_position = 0U,
            .verified_read = false,
            .verified_eof = false};
        clear_file_error();
        return make_number_value(static_cast<double>(handle));
    }

    if (function == "fclose" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened)) {
            last_file_error_code() = 6;
            return make_number_value(-1.0);
        }

        if (opened->verified_read) {
            open_file_handles().erase(handle);
            clear_file_error();
            return make_number_value(0.0);
        }

        const int result = std::fclose(opened->file);
        open_file_handles().erase(handle);
        if (result == 0) {
            clear_file_error();
        } else {
            set_file_error_from_errno();
        }
        return make_number_value(result == 0 ? 0.0 : -1.0);
    }

    if (function == "fread" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened)) {
            last_file_error_code() = 6;
            return make_string_value(std::string{});
        }

        const std::size_t requested = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])));
        if (opened->verified_read) {
            const std::size_t available = opened->verified_position < opened->verified_bytes.size()
                ? opened->verified_bytes.size() - opened->verified_position
                : 0U;
            const std::size_t read = std::min(requested, available);
            std::string buffer = opened->verified_bytes.substr(opened->verified_position, read);
            opened->verified_position += read;
            if (requested > 0U && read == 0U) {
                opened->verified_eof = true;
            }
            clear_file_error();
            return make_string_value(std::move(buffer));
        }

        std::string buffer(requested, '\0');
        if (requested == 0U) {
            clear_file_error();
            return make_string_value(std::string{});
        }

        const std::size_t read = std::fread(buffer.data(), 1U, requested, opened->file);
        buffer.resize(read);
        if (std::ferror(opened->file) != 0) {
            set_file_error_from_errno();
        } else {
            clear_file_error();
        }
        return make_string_value(std::move(buffer));
    }

    if (function == "fwrite" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened) || opened->verified_read) {
            last_file_error_code() = 6;
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
        if (written == text.size()) {
            clear_file_error();
        } else {
            set_file_error_from_errno();
        }
        return make_number_value(static_cast<double>(written));
    }

    if (function == "fgets" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened)) {
            last_file_error_code() = 6;
            return make_string_value(std::string{});
        }

        const std::size_t max_length = arguments.size() >= 2U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[1])))
                                           : 4096U;
        if (opened->verified_read) {
            if (opened->verified_position >= opened->verified_bytes.size()) {
                opened->verified_eof = true;
                clear_file_error();
                return make_string_value(std::string{});
            }
            const std::size_t remaining = opened->verified_bytes.size() - opened->verified_position;
            const std::size_t limit = std::min(max_length, remaining);
            const std::size_t newline = opened->verified_bytes.find_first_of(
                "\r\n",
                opened->verified_position);
            std::size_t count = limit;
            if (newline != std::string::npos && newline - opened->verified_position < count) {
                count = newline - opened->verified_position + 1U;
                if (opened->verified_bytes[newline] == '\r' &&
                    newline + 1U < opened->verified_bytes.size() &&
                    opened->verified_bytes[newline + 1U] == '\n' &&
                    count < limit) {
                    ++count;
                }
            }
            std::string buffer = opened->verified_bytes.substr(opened->verified_position, count);
            opened->verified_position += count;
            clear_file_error();
            return make_string_value(trim_newline(std::move(buffer)));
        }

        std::string buffer(max_length + 1U, '\0');
        if (std::fgets(buffer.data(), static_cast<int>(buffer.size()), opened->file) == nullptr) {
            if (std::ferror(opened->file) != 0) {
                set_file_error_from_errno();
            } else {
                clear_file_error();
            }
            return make_string_value(std::string{});
        }

        buffer.resize(std::strlen(buffer.c_str()));
        clear_file_error();
        return make_string_value(trim_newline(std::move(buffer)));
    }

    if (function == "fputs" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened) || opened->verified_read) {
            last_file_error_code() = 6;
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
        if (written == text.size()) {
            clear_file_error();
        } else {
            set_file_error_from_errno();
        }
        return make_number_value(static_cast<double>(written));
    }

    if (function == "fseek" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened)) {
            last_file_error_code() = 6;
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

        if (opened->verified_read) {
            const long long base = origin == SEEK_SET
                ? 0LL
                : origin == SEEK_CUR
                    ? static_cast<long long>(opened->verified_position)
                    : static_cast<long long>(opened->verified_bytes.size());
            const long long delta = static_cast<long long>(offset);
            if ((delta > 0 && base > std::numeric_limits<long long>::max() - delta) ||
                (delta < 0 && base < std::numeric_limits<long long>::min() - delta)) {
                last_file_error_code() = 25;
                return make_number_value(-1.0);
            }
            const long long target = base + delta;
            if (target < 0 || static_cast<unsigned long long>(target) >
                    static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
                last_file_error_code() = 25;
                return make_number_value(-1.0);
            }
            opened->verified_position = static_cast<std::size_t>(target);
            opened->verified_eof = false;
            clear_file_error();
            return make_number_value(static_cast<double>(opened->verified_position));
        }

        if (std::fseek(opened->file, offset, origin) != 0) {
            set_file_error_from_errno(25);
            return make_number_value(-1.0);
        }
        const long position = std::ftell(opened->file);
        if (position < 0) {
            set_file_error_from_errno();
        } else {
            clear_file_error();
        }
        return make_number_value(position < 0 ? -1.0 : static_cast<double>(position));
    }

    if (function == "ftell" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened)) {
            last_file_error_code() = 6;
            return make_number_value(-1.0);
        }

        if (opened->verified_read) {
            clear_file_error();
            return make_number_value(static_cast<double>(opened->verified_position));
        }

        const long position = std::ftell(opened->file);
        if (position < 0) {
            set_file_error_from_errno();
        } else {
            clear_file_error();
        }
        return make_number_value(position < 0 ? -1.0 : static_cast<double>(position));
    }

    if (function == "feof" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened)) {
            last_file_error_code() = 6;
            return make_boolean_value(true);
        }

        if (opened->verified_read) {
            clear_file_error();
            return make_boolean_value(opened->verified_eof);
        }

        clear_file_error();
        return make_boolean_value(std::feof(opened->file) != 0);
    }

    if (function == "fflush" && !arguments.empty()) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (!is_open_handle(opened)) {
            last_file_error_code() = 6;
            return make_number_value(-1.0);
        }

        if (opened->verified_read) {
            clear_file_error();
            return make_number_value(0.0);
        }

        const int result = std::fflush(opened->file);
        if (result == 0) {
            clear_file_error();
        } else {
            set_file_error_from_errno();
        }
        return make_number_value(result == 0 ? 0.0 : -1.0);
    }

    if (function == "fchsize" && arguments.size() >= 2U) {
        const int handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
        auto* opened = resolve_open_handle(handle);
        if (opened == nullptr || opened->file == nullptr) {
            last_file_error_code() = 6;
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
        if (result == 0) {
            clear_file_error();
        } else {
            set_file_error_from_errno();
        }
        return make_number_value(result == 0 ? 0.0 : -1.0);
    }

    if (function == "filetostr" && !arguments.empty()) {
        const std::filesystem::path path = resolve_file_path(value_as_string(arguments[0]), default_directory);
        if (require_verified_file_byte_overrides) {
            const auto verified = read_verified_file_callback ? read_verified_file_callback(path) : std::nullopt;
            if (!verified.has_value()) {
                if (verified_file_unavailable_callback) {
                    verified_file_unavailable_callback(path);
                }
                last_file_error_code() = 5;
                return make_string_value(std::string{});
            }
            clear_file_error();
            return make_string_value(*verified);
        }

        std::ifstream input(path, std::ios::binary);
        if (!input.good()) {
            set_file_error_from_errno();
            return make_string_value(std::string{});
        }

        std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        clear_file_error();
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
            set_file_error_from_errno();
            return make_number_value(-1.0);
        }

        const std::string text = value_as_string(arguments[0]);
        output.write(text.data(), static_cast<std::streamsize>(text.size()));
        if (!output.good()) {
            set_file_error_from_errno();
            return make_number_value(-1.0);
        }

        clear_file_error();
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
    clear_file_error();
}

}  // namespace copperfin::runtime
