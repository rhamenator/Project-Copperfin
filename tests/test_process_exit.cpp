// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COPPERFIN_TEST_PROCESS_EXIT_CODE
#error COPPERFIN_TEST_PROCESS_EXIT_CODE must be defined by the build.
#endif

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    return COPPERFIN_TEST_PROCESS_EXIT_CODE;
}
