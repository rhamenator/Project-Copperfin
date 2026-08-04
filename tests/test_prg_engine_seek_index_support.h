#pragma once

// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#define _getpid getpid
#endif
#include <sstream>
#include <system_error>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


namespace copperfin::seek_index_tests
{
using namespace copperfin::test_support;

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
inline std::filesystem::path declared_dll_fixture_source_path() {
    std::wstring executable_path(32768U, L'\0');
    const DWORD executable_length = GetModuleFileNameW(
        nullptr,
        executable_path.data(),
        static_cast<DWORD>(executable_path.size()));
    if (executable_length == 0U || executable_length >= executable_path.size()) {
        return {};
    }
    executable_path.resize(executable_length);
    return std::filesystem::path(executable_path).parent_path() /
           std::filesystem::path(COPPERFIN_DECLARED_DLL_FIXTURE_NAME);
}
#endif

}
