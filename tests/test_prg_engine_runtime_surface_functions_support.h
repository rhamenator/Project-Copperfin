#pragma once
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_locale_code_page.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <iconv.h>
#include <langinfo.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


namespace copperfin::runtime_surface_tests
{
    using namespace copperfin::test_support;

    inline int expected_host_code_page()
    {
#if defined(_WIN32)
        const UINT active_code_page = GetACP();
        return active_code_page == 0U ? 1252 : static_cast<int>(active_code_page);
#else
        std::optional<std::string> system_codeset;
        if (const char *codeset = nl_langinfo(CODESET); codeset != nullptr)
        {
            system_codeset = codeset;
        }

        const std::array<std::optional<std::string>, 3U> locale_candidates = {
            copperfin::test_support::getenv_optional("LC_ALL"),
            copperfin::test_support::getenv_optional("LC_CTYPE"),
            copperfin::test_support::getenv_optional("LANG"),
        };
        return copperfin::runtime::detail::resolve_posix_host_code_page(system_codeset, locale_candidates);
#endif
    }

    inline int expected_host_oem_code_page()
    {
#if defined(_WIN32)
        const UINT oem_code_page = GetOEMCP();
        return oem_code_page == 0U ? expected_host_code_page() : static_cast<int>(oem_code_page);
#else
        return expected_host_code_page();
#endif
    }

    inline void set_dbf_code_page_mark(const std::filesystem::path &path, std::uint8_t code_page_mark)
    {
        std::fstream stream(path, std::ios::binary | std::ios::in | std::ios::out);
        expect(stream.good(), "DBF fixture should open for code-page patching");
        if (!stream.good())
        {
            return;
        }

        stream.seekp(29, std::ios::beg);
        const char byte = static_cast<char>(code_page_mark);
        stream.write(&byte, 1);
        stream.flush();
        expect(stream.good(), "DBF fixture code-page patch should succeed");
    }

    inline bool is_supported_vfp_code_page(int code_page)
    {
        switch (code_page)
        {
            case 437:
            case 620:
            case 737:
            case 850:
            case 852:
            case 857:
            case 861:
            case 865:
            case 866:
            case 874:
            case 895:
            case 932:
            case 936:
            case 949:
            case 950:
            case 1250:
            case 1251:
            case 1252:
            case 1253:
            case 1254:
            case 1255:
            case 1256:
            case 10000:
            case 10006:
            case 10007:
            case 10029:
                return true;
            default:
                return false;
        }
    }

#if defined(_WIN32)
    inline std::optional<std::string> expected_host_code_page_conversion(
        int source_code_page,
        int target_code_page,
        const std::string &input)
    {
        const int wide_count = MultiByteToWideChar(
            static_cast<UINT>(source_code_page),
            0,
            input.data(),
            static_cast<int>(input.size()),
            nullptr,
            0);
        if (wide_count <= 0)
        {
            return std::nullopt;
        }

        std::wstring wide_text(static_cast<std::size_t>(wide_count), L'\0');
        if (MultiByteToWideChar(
                static_cast<UINT>(source_code_page),
                0,
                input.data(),
                static_cast<int>(input.size()),
                wide_text.data(),
                wide_count) <= 0)
        {
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
        if (byte_count <= 0)
        {
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
                nullptr) <= 0)
        {
            return std::nullopt;
        }

        return output;
    }
#else
    inline std::optional<std::string> iconv_encoding_name_for_code_page(int code_page)
    {
        switch (code_page)
        {
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

    inline std::optional<std::string> expected_host_code_page_conversion(
        int source_code_page,
        int target_code_page,
        const std::string &input)
    {
        const std::optional<std::string> source_name = iconv_encoding_name_for_code_page(source_code_page);
        const std::optional<std::string> target_name = iconv_encoding_name_for_code_page(target_code_page);
        if (!source_name.has_value() || !target_name.has_value())
        {
            return std::nullopt;
        }

        iconv_t converter = iconv_open(target_name->c_str(), source_name->c_str());
        if (converter == reinterpret_cast<iconv_t>(-1))
        {
            return std::nullopt;
        }

        std::string output(std::max<std::size_t>(input.size() * 4U, 16U), '\0');
        char *input_buffer = const_cast<char *>(input.data());
        std::size_t input_remaining = input.size();
        char *output_buffer = output.data();
        std::size_t output_remaining = output.size();

        while (true)
        {
            const std::size_t result = iconv(
                converter,
                &input_buffer,
                &input_remaining,
                &output_buffer,
                &output_remaining);
            if (result != static_cast<std::size_t>(-1))
            {
                break;
            }

            if (errno == E2BIG)
            {
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
}
