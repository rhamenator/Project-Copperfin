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

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllMultiply(double left, double right) {
    return left * right;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllScale(double value, int factor) {
    return value * static_cast<double>(factor);
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllSplit(double value, double* whole) {
    const int integral = static_cast<int>(value);
    if (whole != nullptr) {
        *whole = static_cast<double>(integral);
    }
    return value - static_cast<double>(integral);
}
