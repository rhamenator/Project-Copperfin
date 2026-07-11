// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#ifndef COPPERFIN_TEST_PROCESS_EXIT_CODE
#error COPPERFIN_TEST_PROCESS_EXIT_CODE must be defined by the build.
#endif

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    return COPPERFIN_TEST_PROCESS_EXIT_CODE;
}
