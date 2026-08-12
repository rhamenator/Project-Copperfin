// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::runtime
{
    enum class NativeDeclaredArgumentKind
    {
        signed_integer32,
        signed_integer64,
        floating_point32,
        floating_point64,
        string
    };

    struct NativeDeclaredArgument
    {
        NativeDeclaredArgumentKind kind = NativeDeclaredArgumentKind::signed_integer32;
        bool by_reference = false;
        std::int64_t signed_integer_value = 0;
        double floating_point_value = 0.0;
        std::string string_value;
    };

    enum class NativeDeclaredReturnKind
    {
        signed_integer16,
        signed_integer32,
        signed_integer64,
        floating_point32,
        floating_point64,
        string
    };

    struct NativeDeclaredCallRequest
    {
        std::uintptr_t function_address = 0U;
        bool use_cdecl = false;
        NativeDeclaredReturnKind return_kind = NativeDeclaredReturnKind::signed_integer32;
        std::vector<NativeDeclaredArgument> arguments;
    };

    struct NativeDeclaredCallResult
    {
        bool succeeded = false;
        std::int32_t compatible_error_code = 0;
        std::int64_t signed_integer_value = 0;
        double floating_point_value = 0.0;
        std::string string_value;
        std::vector<NativeDeclaredArgument> arguments;
    };

    [[nodiscard]] NativeDeclaredCallResult invoke_native_declared_function(
        const NativeDeclaredCallRequest &request);
}
