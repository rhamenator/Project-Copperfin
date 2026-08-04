// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#if defined(_WIN64)

#include <cstdint>
#include <span>

namespace copperfin::runtime::detail
{
    struct Win64NativeCallArgument
    {
        std::uint64_t integer_value = 0U;
        double double_value = 0.0;
        bool is_double = false;
    };

    enum class Win64NativeReturnKind
    {
        integer32,
        integer64,
        string_pointer,
        floating64
    };

    struct Win64NativeCallResult
    {
        std::uint64_t integer_value = 0U;
        const char *string_pointer = nullptr;
        double double_value = 0.0;
    };

    [[nodiscard]] Win64NativeCallResult invoke_win64_native_function(
        std::uintptr_t function_address,
        std::span<const Win64NativeCallArgument> arguments,
        Win64NativeReturnKind return_kind);
}

#endif
