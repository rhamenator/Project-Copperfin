// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_runtime_host_debug_output_support.h"

void test_runtime_host_rejects_audit_paths_outside_the_direct_package(
    const std::string& runtime_host_path);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "FAIL: runtime host executable path argument is required\n";
        return 1;
    }

    test_runtime_host_rejects_audit_paths_outside_the_direct_package(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All runtime host audit-containment tests passed\n";
    return 0;
}
