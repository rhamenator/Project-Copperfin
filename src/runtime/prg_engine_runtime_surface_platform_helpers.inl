// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

std::uint32_t bitwise_value(const PrgValue& value) {
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(std::llround(value_as_number(value))));
}
std::int64_t signed_bitwise_result(std::uint32_t value) {
    return static_cast<std::int64_t>(static_cast<std::int32_t>(value));
}

int bit_position(const PrgValue& value) {
    const int position = static_cast<int>(std::llround(value_as_number(value)));
    if (position < 0 || position > 31) {
        throw std::runtime_error(runtime_text(
            "Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange",
            {
                {"maximum", "31"},
                {"minimum", "0"}
            }));
    }
    return position;
}

std::string host_os_name() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__unix__)
    return "Unix";
#else
    return "Unknown";
#endif
}

std::string make_legal_runtime_temp_file_name() {
    static std::atomic<std::uint64_t> sequence{0U};
    const auto ticks = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
    const std::uint64_t serial = sequence.fetch_add(1U, std::memory_order_relaxed);
    const std::uint64_t value = 10000000U + ((ticks + serial) % 90000000U);

    std::ostringstream result;
    result << std::setw(8) << std::setfill('0') << value;
    return result.str();
}

std::string make_unique_runtime_procedure_name() {
    static std::atomic<std::uint64_t> sequence{0U};
    constexpr std::uint64_t base36_modulus = 101559956668416ULL;
    constexpr char base36_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const auto ticks = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    const std::uint64_t serial = sequence.fetch_add(1U, std::memory_order_relaxed);
    std::uint64_t value = (ticks + serial) % base36_modulus;

    std::string result(10U, '0');
    result.front() = '_';
    for (std::size_t index = result.size(); index-- > 1U;) {
        result[index] = base36_digits[value % 36U];
        value /= 36U;
    }
    return result;
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

std::filesystem::path filesystem_probe_path(const std::string& raw_path, const std::string& default_directory) {
    if (raw_path.empty()) {
        return copperfin::platform::path_from_utf8_string(default_directory).lexically_normal();
    }
    if (is_windows_drive_absolute_path(raw_path) || is_unc_path(raw_path)) {
        return copperfin::platform::path_from_utf8_string(raw_path);
    }

    std::filesystem::path path = copperfin::platform::path_from_utf8_string(
        normalize_relative_path_separators(raw_path));
    if (path.is_relative()) {
        path = copperfin::platform::path_from_utf8_string(default_directory) / path;
    }
    return path.lexically_normal();
}

std::string minimum_runtime_path(
    const std::string& raw_file_path,
    const std::string& raw_base_path,
    const std::string& default_directory) {
    const std::filesystem::path file_path = filesystem_probe_path(raw_file_path, default_directory);
    const std::filesystem::path base_path = raw_base_path.empty()
        ? copperfin::platform::path_from_utf8_string(default_directory).lexically_normal()
        : filesystem_probe_path(raw_base_path, default_directory);
    const std::filesystem::path relative_path = file_path.lexically_relative(base_path);
    if (!relative_path.empty()) {
        return copperfin::platform::path_to_utf8_string(relative_path.lexically_normal());
    }
    return copperfin::platform::path_to_utf8_string(file_path.lexically_normal());
}

std::string strip_surrounding_quotes(std::string text) {
    text = trim_copy(std::move(text));
    if (text.size() >= 2U) {
        const char first = text.front();
        const char last = text.back();
        if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
            return text.substr(1U, text.size() - 2U);
        }
    }
    return text;
}

int current_host_code_page() {
    return detail::default_host_code_page();
}

int current_host_oem_code_page() {
    return detail::default_host_oem_code_page();
}

bool is_supported_vfp_code_page(int code_page) {
    return detail::is_supported_vfp_code_page(code_page);
}

#if defined(_WIN32)
std::optional<std::string> convert_between_host_code_pages(
    int source_code_page,
    int target_code_page,
    const std::string& input) {
    const int wide_count = MultiByteToWideChar(
        static_cast<UINT>(source_code_page),
        0,
        input.data(),
        static_cast<int>(input.size()),
        nullptr,
        0);
    if (wide_count <= 0) {
        return std::nullopt;
    }

    std::wstring wide_text(static_cast<std::size_t>(wide_count), L'\0');
    if (MultiByteToWideChar(
            static_cast<UINT>(source_code_page),
            0,
            input.data(),
            static_cast<int>(input.size()),
            wide_text.data(),
            wide_count) <= 0) {
        return std::nullopt;
    }

    const int byte_count = WideCharToMultiByte(
        static_cast<UINT>(target_code_page),
        0,
        wide_text.data(),
        wide_count,
        nullptr,
        0,
        nullptr,
        nullptr);
    if (byte_count <= 0) {
        return std::nullopt;
    }

    std::string output(static_cast<std::size_t>(byte_count), '\0');
    if (WideCharToMultiByte(
            static_cast<UINT>(target_code_page),
            0,
            wide_text.data(),
            wide_count,
            output.data(),
            byte_count,
            nullptr,
            nullptr) <= 0) {
        return std::nullopt;
    }

    return output;
}
#else
std::optional<std::string> iconv_encoding_name_for_code_page(int code_page) {
    switch (code_page) {
        case 437:
            return "CP437";
        case 620:
            return "CP620";
        case 737:
            return "CP737";
        case 850:
            return "CP850";
        case 852:
            return "CP852";
        case 857:
            return "CP857";
        case 861:
            return "CP861";
        case 865:
            return "CP865";
        case 866:
            return "CP866";
        case 874:
            return "CP874";
        case 895:
            return "CP895";
        case 932:
            return "CP932";
        case 936:
            return "CP936";
        case 949:
            return "CP949";
        case 950:
            return "CP950";
        case 1250:
            return "CP1250";
        case 1251:
            return "CP1251";
        case 1252:
            return "CP1252";
        case 1253:
            return "CP1253";
        case 1254:
            return "CP1254";
        case 1255:
            return "CP1255";
        case 1256:
            return "CP1256";
        case 10000:
            return "MACINTOSH";
        case 10006:
            return "MACGREEK";
        case 10007:
            return "MACCYRILLIC";
        case 10029:
            return "MACCENTRALEUROPE";
        default:
            return std::nullopt;
    }
}

std::optional<std::string> convert_between_host_code_pages(
    int source_code_page,
    int target_code_page,
    const std::string& input) {
    const std::optional<std::string> source_name = iconv_encoding_name_for_code_page(source_code_page);
    const std::optional<std::string> target_name = iconv_encoding_name_for_code_page(target_code_page);
    if (!source_name.has_value() || !target_name.has_value()) {
        return std::nullopt;
    }

    iconv_t converter = iconv_open(target_name->c_str(), source_name->c_str());
    if (converter == reinterpret_cast<iconv_t>(-1)) {
        return std::nullopt;
    }

    std::string output(std::max<std::size_t>(input.size() * 4U, 16U), '\0');
    char* input_buffer = const_cast<char*>(input.data());
    std::size_t input_remaining = input.size();
    char* output_buffer = output.data();
    std::size_t output_remaining = output.size();

    while (true) {
        const std::size_t result = iconv(
            converter,
            &input_buffer,
            &input_remaining,
            &output_buffer,
            &output_remaining);
        if (result != static_cast<std::size_t>(-1)) {
            break;
        }

        if (errno == E2BIG) {
            const std::size_t bytes_written = output.size() - output_remaining;
            output.resize(output.size() * 2U, '\0');
            output_buffer = output.data() + bytes_written;
            output_remaining = output.size() - bytes_written;
            continue;
        }

        iconv_close(converter);
        return std::nullopt;
    }

    iconv_close(converter);
    output.resize(output.size() - output_remaining);
    return output;
}
#endif

std::vector<std::filesystem::path> parse_set_path_entries(const std::string& set_path_value,
                                                          const std::string& default_directory) {
    std::string value = trim_copy(set_path_value);
    if (starts_with_insensitive(value, "TO ")) {
        value = trim_copy(value.substr(3U));
    }
    value = strip_surrounding_quotes(std::move(value));

    std::vector<std::filesystem::path> entries;
    std::size_t token_start = 0U;
    while (token_start <= value.size()) {
        const std::size_t separator = value.find(';', token_start);
        std::string token = separator == std::string::npos
                                ? value.substr(token_start)
                                : value.substr(token_start, separator - token_start);
        token = strip_surrounding_quotes(std::move(token));
        if (!token.empty()) {
            if (!is_windows_drive_absolute_path(token) && !is_unc_path(token)) {
                token = normalize_relative_path_separators(std::move(token));
            }
            std::filesystem::path entry = copperfin::platform::path_from_utf8_string(token);
            if (entry.is_relative()) {
                entry = copperfin::platform::path_from_utf8_string(default_directory) / entry;
            }
            entries.push_back(entry.lexically_normal());
        }

        if (separator == std::string::npos) {
            break;
        }
        token_start = separator + 1U;
    }

    return entries;
}

std::filesystem::path resolve_runtime_file_probe_path(
    const std::string& raw_path,
    const std::string& default_directory,
    const std::function<std::string(const std::string&)>& set_callback,
    bool reject_directories = false) {
    std::error_code ignored;
    const auto candidate_matches = [&](const std::filesystem::path& candidate) {
        ignored.clear();
        const std::filesystem::file_status status = std::filesystem::status(candidate, ignored);
        return !ignored && std::filesystem::exists(status) &&
               (!reject_directories || !std::filesystem::is_directory(status));
    };
    if (raw_path.empty()) {
        return copperfin::platform::path_from_utf8_string(default_directory).lexically_normal();
    }
    if (is_windows_drive_absolute_path(raw_path) || is_unc_path(raw_path)) {
        return copperfin::platform::path_from_utf8_string(raw_path);
    }

    std::filesystem::path path = copperfin::platform::path_from_utf8_string(
        normalize_relative_path_separators(raw_path));
    if (!path.is_relative()) {
        return path.lexically_normal();
    }

    const std::filesystem::path default_candidate =
        (copperfin::platform::path_from_utf8_string(default_directory) / path).lexically_normal();
    if (candidate_matches(default_candidate)) {
        return default_candidate;
    }

    const std::vector<std::filesystem::path> set_path_entries =
        parse_set_path_entries(set_callback("PATH"), default_directory);
    for (const auto& entry : set_path_entries) {
        const std::filesystem::path candidate = (entry / path).lexically_normal();
        if (candidate_matches(candidate)) {
            return candidate;
        }
    }

    return default_candidate;
}

double available_disk_space(const std::string& raw_path, const std::string& default_directory) {
    std::error_code ignored;
    const auto info = std::filesystem::space(filesystem_probe_path(raw_path, default_directory), ignored);
    return ignored ? 0.0 : static_cast<double>(info.available);
}

int drive_type_value(const std::string& raw_path, const std::string& default_directory) {
    std::error_code ignored;
    const std::filesystem::path path = filesystem_probe_path(raw_path, default_directory);
    if (!std::filesystem::exists(path, ignored)) {
        return 0;
    }
    return std::filesystem::is_directory(path, ignored) || std::filesystem::is_regular_file(path, ignored)
               ? 3
               : 1;
}

std::string class_token_from_prog_id(const std::string& prog_id) {
    std::string token = trim_copy(prog_id);
    const std::size_t separator = token.find_last_of('.');
    if (separator != std::string::npos && separator + 1U < token.size()) {
        token = token.substr(separator + 1U);
    }
    token = uppercase_copy(trim_copy(std::move(token)));
    return token.empty() ? "CUSTOM" : token;
}
