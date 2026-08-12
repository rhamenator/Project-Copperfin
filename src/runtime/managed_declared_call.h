// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::runtime
{
    enum class ManagedDeclaredValueKind
    {
        empty,
        string,
        signed_integer64,
        unsigned_integer64,
        floating_point,
        boolean,
    };

    struct ManagedDeclaredValue
    {
        ManagedDeclaredValueKind kind = ManagedDeclaredValueKind::empty;
        std::string string_value;
        std::int64_t signed_integer_value = 0;
        std::uint64_t unsigned_integer_value = 0U;
        double floating_point_value = 0.0;
        bool boolean_value = false;
    };

    enum class ManagedDeclaredArgumentKind
    {
        string,
        signed_integer32,
        signed_integer64,
        floating_point32,
        floating_point64,
    };

    struct ManagedDeclaredArgument
    {
        ManagedDeclaredArgumentKind kind = ManagedDeclaredArgumentKind::signed_integer32;
        std::string string_value;
        std::int64_t signed_integer_value = 0;
        double floating_point_value = 0.0;
    };

    enum class ManagedInvocationStage
    {
        none,
        create_runtime,
        locate_runtime,
        acquire_runtime_host,
        start_runtime,
        acquire_app_domain,
        load_assembly,
        find_type,
        find_method,
        invoke_method,
    };

    struct ManagedInvocationResult
    {
        bool succeeded = false;
        std::int32_t compatible_error_code = 0;
        ManagedInvocationStage stage = ManagedInvocationStage::none;
        ManagedDeclaredValue value;
    };

    [[nodiscard]] ManagedInvocationResult invoke_managed_declared_method(
        const std::string &assembly_path_utf8,
        const std::string &type_name_utf8,
        const std::string &method_name_utf8,
        const std::vector<ManagedDeclaredArgument> &arguments);
}
