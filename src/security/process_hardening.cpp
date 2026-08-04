// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/security/process_hardening.h"

#include "localized_text.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace copperfin::security {

ProcessHardeningStatus apply_default_process_hardening() {
#ifdef _WIN32
    ProcessHardeningStatus status{.applied = true};

    if (!SetDllDirectoryW(L"")) {
        status.applied = false;
        status.message = security_text("Security.ProcessHardening.Error.SetDllDirectoryFailed");
    }

    using SetDefaultDllDirectoriesFn = BOOL(WINAPI*)(DWORD);
    auto kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (kernel32 != nullptr) {
        const auto set_default_dll_directories = reinterpret_cast<SetDefaultDllDirectoriesFn>(
            GetProcAddress(kernel32, "SetDefaultDllDirectories"));
        if (set_default_dll_directories != nullptr) {
            if (!set_default_dll_directories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)) {
                status.applied = false;
                if (status.message.empty()) {
                    status.message = security_text("Security.ProcessHardening.Error.SetDefaultDllDirectoriesFailed");
                } else {
                    status.message += " " + security_text("Security.ProcessHardening.Error.SetDefaultDllDirectoriesFailed");
                }
            }
        }
    }

    if (status.applied && status.message.empty()) {
        status.message = security_text("Security.ProcessHardening.Status.AppliedWindowsDllSearchPathHardening");
    }

    return status;
#else
    return ProcessHardeningStatus{
        .applied = true,
        .message = security_text("Security.ProcessHardening.Status.NoopOutsideWindows")
    };
#endif
}

}  // namespace copperfin::security
