// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include <cstdint>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define COPPERFIN_TEST_EXPORT extern "C" __declspec(dllexport)
#define COPPERFIN_TEST_CALL __stdcall
#else
#define COPPERFIN_TEST_EXPORT extern "C"
#define COPPERFIN_TEST_CALL
#endif

namespace {
#if defined(_WIN32)
int declared_dll_fixture_module_anchor = 0;
#endif
std::int32_t declared_dll_arity_entry_count = 0;
std::int32_t declared_dll_numeric_byref_entry_count = 0;

void record_declared_dll_arity_entry() {
    ++declared_dll_arity_entry_count;
}
}

#if defined(_MSC_VER) && defined(_M_IX86)
#pragma comment(linker, "/EXPORT:CopperfinDeclaredDllNoUnderscore@4=_CopperfinDeclaredDllNoUnderscoreImpl@4")
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

COPPERFIN_TEST_EXPORT std::int64_t COPPERFIN_TEST_CALL CopperfinDeclaredDllInt64BeyondDouble() {
    return 9007199254740993LL;
}

COPPERFIN_TEST_EXPORT std::int64_t COPPERFIN_TEST_CALL CopperfinDeclaredDllInt64Echo(std::int64_t value) {
    return value;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllInt64ByRef(std::int64_t* value) {
    if (value == nullptr) {
        return 0L;
    }
    *value = 9007199254740993LL;
    return 1L;
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

COPPERFIN_TEST_EXPORT float COPPERFIN_TEST_CALL CopperfinDeclaredDllSingleConstant() {
    return 0.625F;
}

COPPERFIN_TEST_EXPORT float COPPERFIN_TEST_CALL CopperfinDeclaredDllSingleMixed(
    std::int32_t first,
    float second,
    double third,
    float fourth,
    float fifth) {
    return static_cast<float>(first) + second + static_cast<float>(third) + fourth + fifth;
}

COPPERFIN_TEST_EXPORT double COPPERFIN_TEST_CALL CopperfinDeclaredDllSingleToDouble(float value, double multiplier) {
    return static_cast<double>(value) * multiplier;
}

COPPERFIN_TEST_EXPORT float COPPERFIN_TEST_CALL CopperfinDeclaredDllSingleSlots(
    float first,
    float second,
    float third,
    float fourth,
    float fifth,
    float sixth,
    float seventh,
    float eighth) {
    return first + second + third + fourth + fifth + sixth + seventh + eighth;
}

COPPERFIN_TEST_EXPORT float COPPERFIN_TEST_CALL CopperfinDeclaredDllSingleSplit(float value, float* whole) {
    const int integral = static_cast<int>(value);
    if (whole != nullptr) {
        *whole = static_cast<float>(integral);
    }
    return value - static_cast<float>(integral);
}

COPPERFIN_TEST_EXPORT double COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Mixed(
    long first,
    double second,
    std::int64_t third,
    std::int32_t fourth) {
    return static_cast<double>(first) + second + static_cast<double>(third) + static_cast<double>(fourth);
}

COPPERFIN_TEST_EXPORT std::int64_t COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Int64() {
    return -4294967297LL;
}

COPPERFIN_TEST_EXPORT std::int64_t COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Int64BeyondDouble() {
    return 9007199254740993LL;
}

COPPERFIN_TEST_EXPORT std::int64_t COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Int64Echo(std::int64_t value) {
    return value;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Int64ByRef(std::int64_t* value) {
    if (value == nullptr) {
        return 0L;
    }
    *value = 9007199254740993LL;
    return 1L;
}

COPPERFIN_TEST_EXPORT double COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Split(double value, double* whole) {
    const int integral = static_cast<int>(value);
    if (whole != nullptr) {
        *whole = static_cast<double>(integral);
    }
    return value - static_cast<double>(integral);
}

COPPERFIN_TEST_EXPORT const char* COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Text() {
    return "copperfin-x86";
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllX86Eight(
    std::int32_t first,
    std::int32_t second,
    std::int32_t third,
    std::int32_t fourth,
    std::int32_t fifth,
    std::int32_t sixth,
    std::int32_t seventh,
    std::int32_t eighth) {
    return first + second + third + fourth + fifth + sixth + seventh + eighth;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllX86NumericByRef(
    long* long_value,
    std::int64_t* integer64_value) {
    if (long_value == nullptr || integer64_value == nullptr) {
        return 0;
    }
    *long_value = -123456789L;
    *integer64_value = -4294967297LL;
    return -7L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllNoUnderscoreImpl(long value) {
    return value + 1L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllAnsiOnlyA(long value) {
    return value + 3942L;
}

COPPERFIN_TEST_EXPORT long CopperfinDeclaredDllAnsiCdeclOnlyA() {
    return 4002L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllArityReset() {
    declared_dll_arity_entry_count = 0;
    return 0L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllArityCount() {
    return static_cast<long>(declared_dll_arity_entry_count);
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllArityZero() {
    record_declared_dll_arity_entry();
    return 3946L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllArityOne(std::int32_t value) {
    record_declared_dll_arity_entry();
    return static_cast<long>(value);
}

COPPERFIN_TEST_EXPORT long CopperfinDeclaredDllArityCdeclOne(std::int32_t value) {
    record_declared_dll_arity_entry();
    return static_cast<long>(value);
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllArityMixed(
    std::int32_t first,
    double second,
    std::int64_t* third,
    float fourth) {
    record_declared_dll_arity_entry();
    return static_cast<long>(first + static_cast<std::int32_t>(second) +
                             (third == nullptr ? 0 : static_cast<std::int32_t>(*third)) +
                             static_cast<std::int32_t>(fourth));
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllArityEight(
    std::int32_t first,
    std::int32_t second,
    std::int32_t third,
    std::int32_t fourth,
    std::int32_t fifth,
    std::int32_t sixth,
    std::int32_t seventh,
    std::int32_t eighth) {
    record_declared_dll_arity_entry();
    return static_cast<long>(first + second + third + fourth + fifth + sixth + seventh + eighth);
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllNumericByRefReset() {
    declared_dll_numeric_byref_entry_count = 0;
    return 0L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllNumericByRefCount() {
    return static_cast<long>(declared_dll_numeric_byref_entry_count);
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllNumericByRefProbe(
    std::int32_t* integer_value,
    std::int32_t* long_value,
    std::int64_t* integer64_value,
    float* single_value,
    double* double_value) {
    ++declared_dll_numeric_byref_entry_count;
    if (integer_value == nullptr || long_value == nullptr || integer64_value == nullptr ||
        single_value == nullptr || double_value == nullptr) {
        return 0L;
    }
    *integer_value = -11;
    *long_value = -22;
    *integer64_value = -4294967297LL;
    *single_value = 3.5F;
    *double_value = 4.25;
    return 3944L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllModulePathA(char* buffer, long capacity) {
#if defined(_WIN32)
    if (buffer == nullptr || capacity <= 0L) {
        return 0L;
    }
    HMODULE module = nullptr;
    if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&declared_dll_fixture_module_anchor),
            &module) == 0) {
        return 0L;
    }
    return static_cast<long>(GetModuleFileNameA(module, buffer, static_cast<DWORD>(capacity)));
#else
    (void)buffer;
    (void)capacity;
    return 0L;
#endif
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllExactPrecedence(long value) {
    return value + 1L;
}

COPPERFIN_TEST_EXPORT long COPPERFIN_TEST_CALL CopperfinDeclaredDllExactPrecedenceA(long value) {
    return value + 1000L;
}

COPPERFIN_TEST_EXPORT short COPPERFIN_TEST_CALL CopperfinDeclaredDllShortNegative() {
    return static_cast<short>(-12345);
}

COPPERFIN_TEST_EXPORT short COPPERFIN_TEST_CALL CopperfinDeclaredDllShortInternetShape(
    std::int32_t* flags,
    std::int32_t reserved) {
    if (flags == nullptr || reserved != 0) {
        return static_cast<short>(-2);
    }
    *flags = 0x12345678;
    return static_cast<short>(-1);
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
