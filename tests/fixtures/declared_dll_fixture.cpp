// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include <cstdint>

#if defined(_WIN32)
#define COPPERFIN_TEST_EXPORT extern "C" __declspec(dllexport)
#else
#define COPPERFIN_TEST_EXPORT extern "C"
#endif

COPPERFIN_TEST_EXPORT int CopperfinDeclaredDllFixtureValue() {
    return 3921;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllFraction() {
    return 0.625;
}

COPPERFIN_TEST_EXPORT const char* CopperfinDeclaredDllText() {
    return "copperfin";
}

COPPERFIN_TEST_EXPORT long long CopperfinDeclaredDllInt64() {
    return -4294967297LL;
}

COPPERFIN_TEST_EXPORT std::int32_t CopperfinDeclaredDllLongWidth(
    double multiplier,
    std::int32_t input,
    std::int32_t* output) {
    if (multiplier != 1.5 || input != 42 || output == nullptr) {
        return 0;
    }
    *output = -123456789;
    return -2147483000;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllOneSlot(double first) {
    return first;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllMultiply(double left, double right) {
    return left * right;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllAffine(double left, double right) {
    return (left * 10.0) + right;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllScale(double value, int factor) {
    return value * static_cast<double>(factor);
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllThreeSlots(double first, int second, double third) {
    return first + second + third;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllFourSlots(double first, int second, double third, int fourth) {
    return first + second + third + fourth;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllFiveSlots(
    double first,
    int second,
    double third,
    int fourth,
    double fifth) {
    return first + second + third + fourth + fifth;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllSixSlots(
    double first,
    int second,
    double third,
    int fourth,
    double fifth,
    int sixth) {
    return first + second + third + fourth + fifth + sixth;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllSevenSlots(
    double first,
    int second,
    double third,
    int fourth,
    double fifth,
    int sixth,
    double seventh) {
    return first + second + third + fourth + fifth + sixth + seventh;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllEightSlots(
    double first,
    int second,
    double third,
    int fourth,
    double fifth,
    int sixth,
    double seventh,
    int eighth) {
    return first + second + third + fourth + fifth + sixth + seventh + eighth;
}

COPPERFIN_TEST_EXPORT double CopperfinDeclaredDllSplit(double value, double* whole) {
    const int integral = static_cast<int>(value);
    if (whole != nullptr) {
        *whole = static_cast<double>(integral);
    }
    return value - static_cast<double>(integral);
}

COPPERFIN_TEST_EXPORT int CopperfinDeclaredDllDecrement(int* value) {
    if (value == nullptr) {
        return 0;
    }
    --(*value);
    return *value;
}
