// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#if defined(_WIN32)
#define COPPERFIN_TEST_EXPORT extern "C" __declspec(dllexport)
#else
#define COPPERFIN_TEST_EXPORT extern "C"
#endif

COPPERFIN_TEST_EXPORT int CopperfinDeclaredDllFixtureValue() {
    return 3921;
}
