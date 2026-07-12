// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#if !defined(_WIN32) || !defined(_MSC_VER)
#error This fixture requires the MSVC C++/CLI toolchain.
#endif

public ref class CopperfinMixedModeMarker abstract sealed
{
public:
    static int Value()
    {
        return 1;
    }
};

extern "C" __declspec(dllexport) int __stdcall CopperfinMixedModeNativeValue()
{
    return 3947;
}
