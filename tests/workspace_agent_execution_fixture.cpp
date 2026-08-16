// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main(int argc, char** argv) {
    std::cerr << "workspace-agent-child-entry-v1\n";
    if (argc == 2 && argv[1] != nullptr &&
        std::string_view(argv[1]) == "--workspace-agent-child-wait-v1") {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 29;
    }
    if (argc == 3 && argv[0] != nullptr && argv[1] != nullptr &&
        argv[2] != nullptr &&
        std::string_view(argv[1]) == "--workspace-agent-child-v1") {
        ::SetLastError(ERROR_SUCCESS);
        const DWORD ambient_length =
            ::GetEnvironmentVariableW(L"GITHUB_TOKEN", nullptr, 0U);
        const bool ambient_unset = ambient_length == 0U &&
            ::GetLastError() == ERROR_ENVVAR_NOT_FOUND;
        std::cout << "workspace-agent-child-v1\n"
                  << "argv0=" << argv[0] << '\n'
                  << "payload=" << argv[2] << '\n'
                  << "cwd=" << std::filesystem::current_path().string() << '\n'
                  << "ambient=" << (ambient_unset ? "<unset>" : "<set>")
                  << '\n';
        return 23;
    }
    std::cerr << "workspace-agent-child-arguments-unrecognized-v1\n";
    return 31;
}
