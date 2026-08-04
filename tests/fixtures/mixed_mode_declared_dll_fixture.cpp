// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
