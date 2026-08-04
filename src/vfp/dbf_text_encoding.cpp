// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/vfp/dbf_text_encoding.h"

#include "copperfin/vfp/dbf_header.h"

#include <cerrno>
#include <cstddef>
#include <optional>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <iconv.h>
#endif

namespace copperfin::vfp {

namespace {

std::optional<int> resolved_code_page(std::uint8_t code_page_mark) {
    if (code_page_mark == 0U) {
        return 65001;
    }
    return dbf_code_page_from_mark(code_page_mark);
}

#if defined(_WIN32)

DbfTextConversionResult decode_windows(int source_code_page, std::string_view input) {
    if (input.empty()) {
        return {.ok = true, .text = {}, .error = DbfTextEncodingError::none};
    }

    const DWORD source_flags = source_code_page == 65001 ? MB_ERR_INVALID_CHARS : 0U;
    const int wide_count = MultiByteToWideChar(
        static_cast<UINT>(source_code_page),
        source_flags,
        input.data(),
        static_cast<int>(input.size()),
        nullptr,
        0);
    if (wide_count <= 0) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::invalid_input};
    }

    std::wstring wide_text(static_cast<std::size_t>(wide_count), L'\0');
    if (MultiByteToWideChar(
            static_cast<UINT>(source_code_page),
            source_flags,
            input.data(),
            static_cast<int>(input.size()),
            wide_text.data(),
            wide_count) <= 0) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::invalid_input};
    }

    const int output_size = WideCharToMultiByte(
        CP_UTF8,
        0U,
        wide_text.data(),
        wide_count,
        nullptr,
        0,
        nullptr,
        nullptr);
    if (output_size <= 0) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::invalid_input};
    }

    std::string output(static_cast<std::size_t>(output_size), '\0');
    if (WideCharToMultiByte(
            CP_UTF8,
            0U,
            wide_text.data(),
            wide_count,
            output.data(),
            output_size,
            nullptr,
            nullptr) <= 0) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::invalid_input};
    }
    return {.ok = true, .text = std::move(output), .error = DbfTextEncodingError::none};
}

DbfTextConversionResult encode_windows(int target_code_page, std::string_view input) {
    if (input.empty()) {
        return {.ok = true, .text = {}, .error = DbfTextEncodingError::none};
    }

    const int wide_count = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        input.data(),
        static_cast<int>(input.size()),
        nullptr,
        0);
    if (wide_count <= 0) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::invalid_input};
    }

    std::wstring wide_text(static_cast<std::size_t>(wide_count), L'\0');
    if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            input.data(),
            static_cast<int>(input.size()),
            wide_text.data(),
            wide_count) <= 0) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::invalid_input};
    }

    BOOL used_default_character = FALSE;
    const DWORD target_flags = target_code_page == 65001 ? 0U : WC_NO_BEST_FIT_CHARS;
    BOOL* const used_default_character_pointer =
        target_code_page == 65001 ? nullptr : &used_default_character;
    const int output_size = WideCharToMultiByte(
        static_cast<UINT>(target_code_page),
        target_flags,
        wide_text.data(),
        wide_count,
        nullptr,
        0,
        nullptr,
        used_default_character_pointer);
    if (output_size <= 0 || used_default_character != FALSE) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::unrepresentable_character};
    }

    std::string output(static_cast<std::size_t>(output_size), '\0');
    used_default_character = FALSE;
    if (WideCharToMultiByte(
            static_cast<UINT>(target_code_page),
            target_flags,
            wide_text.data(),
            wide_count,
            output.data(),
            output_size,
            nullptr,
            used_default_character_pointer) <= 0 ||
        used_default_character != FALSE) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::unrepresentable_character};
    }
    return {.ok = true, .text = std::move(output), .error = DbfTextEncodingError::none};
}

#else

std::optional<std::string> iconv_encoding_name(int code_page) {
    switch (code_page) {
        case 437: return "CP437";
        case 620: return "CP620";
        case 737: return "CP737";
        case 850: return "CP850";
        case 852: return "CP852";
        case 857: return "CP857";
        case 861: return "CP861";
        case 865: return "CP865";
        case 866: return "CP866";
        case 874: return "CP874";
        case 895: return "CP895";
        case 932: return "CP932";
        case 936: return "CP936";
        case 949: return "CP949";
        case 950: return "CP950";
        case 1250: return "CP1250";
        case 1251: return "CP1251";
        case 1252: return "CP1252";
        case 1253: return "CP1253";
        case 1254: return "CP1254";
        case 1255: return "CP1255";
        case 1256: return "CP1256";
        case 10000: return "MACINTOSH";
        case 10006: return "MACGREEK";
        case 10007: return "MACCYRILLIC";
        case 10029: return "MACCENTRALEUROPE";
        case 65001: return "UTF-8";
        default: return std::nullopt;
    }
}

DbfTextConversionResult convert_iconv(
    const std::string& source_encoding,
    const std::string& target_encoding,
    std::string_view input,
    DbfTextEncodingError failure_error) {
    if (input.empty()) {
        return {.ok = true, .text = {}, .error = DbfTextEncodingError::none};
    }

    iconv_t converter = iconv_open(target_encoding.c_str(), source_encoding.c_str());
    if (converter == reinterpret_cast<iconv_t>(-1)) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::unsupported_code_page};
    }

    std::string output(input.size() * 4U + 16U, '\0');
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
        if (errno != E2BIG) {
            iconv_close(converter);
            return {.ok = false, .text = {}, .error = failure_error};
        }
        const std::size_t bytes_written = output.size() - output_remaining;
        output.resize(output.size() * 2U, '\0');
        output_buffer = output.data() + static_cast<std::ptrdiff_t>(bytes_written);
        output_remaining = output.size() - bytes_written;
    }
    iconv_close(converter);
    output.resize(output.size() - output_remaining);
    return {.ok = true, .text = std::move(output), .error = DbfTextEncodingError::none};
}

#endif

}  // namespace

DbfTextConversionResult decode_dbf_text(std::uint8_t code_page_mark, std::string_view bytes) {
    const std::optional<int> code_page = resolved_code_page(code_page_mark);
    if (!code_page.has_value()) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::unsupported_code_page};
    }
#if defined(_WIN32)
    return decode_windows(*code_page, bytes);
#else
    const auto source_encoding = iconv_encoding_name(*code_page);
    if (!source_encoding.has_value()) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::unsupported_code_page};
    }
    return convert_iconv(*source_encoding, "UTF-8", bytes, DbfTextEncodingError::invalid_input);
#endif
}

DbfTextConversionResult encode_dbf_text(std::uint8_t code_page_mark, std::string_view utf8_text) {
    const std::optional<int> code_page = resolved_code_page(code_page_mark);
    if (!code_page.has_value()) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::unsupported_code_page};
    }
#if defined(_WIN32)
    return encode_windows(*code_page, utf8_text);
#else
    const auto target_encoding = iconv_encoding_name(*code_page);
    if (!target_encoding.has_value()) {
        return {.ok = false, .text = {}, .error = DbfTextEncodingError::unsupported_code_page};
    }
    return convert_iconv("UTF-8", *target_encoding, utf8_text, DbfTextEncodingError::unrepresentable_character);
#endif
}

}  // namespace copperfin::vfp
