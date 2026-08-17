// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// This deliberately avoids C++ streams, filesystem, environment, and argument
// processing. It establishes the restricted-token process-launch baseline before
// the richer self-contained validation fixture is considered.
int main() {
    ::ExitProcess(41U);
}
