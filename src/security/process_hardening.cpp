#include "copperfin/security/process_hardening.h"

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
        status.message = "SetDllDirectoryW(\"\") failed; current-directory DLL lookup may remain enabled.";
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
                    status.message = "SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS) failed.";
                } else {
                    status.message += " SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS) failed.";
                }
            }
        }
    }

    if (status.applied && status.message.empty()) {
        status.message = "Applied Windows DLL search-path hardening.";
    }

    return status;
#else
    return ProcessHardeningStatus{
        .applied = true,
        .message = "Process hardening is currently a no-op outside Windows."
    };
#endif
}

}  // namespace copperfin::security
