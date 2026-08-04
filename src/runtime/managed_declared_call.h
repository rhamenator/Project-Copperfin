// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#if defined(_WIN32)

#include <string>
#include <vector>

#include <windows.h>
#include <oleauto.h>

namespace copperfin::runtime
{
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
        HRESULT hresult = S_OK;
        ManagedInvocationStage stage = ManagedInvocationStage::none;
    };

    [[nodiscard]] ManagedInvocationResult invoke_managed_declared_method(
        const std::string &assembly_path_utf8,
        const std::string &type_name_utf8,
        const std::string &method_name_utf8,
        const std::vector<VARIANT> &arguments,
        VARIANT *return_value);
}

#endif
