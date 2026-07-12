// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#if defined(_WIN32)

#include <cstdint>
#include <oaidl.h>

namespace copperfin::runtime
{
    struct DispatchExceptionCleanupStats
    {
        std::uint64_t invoke_results = 0U;
        std::uint64_t exception_results = 0U;
        std::uint64_t deferred_fill_calls = 0U;
        std::uint64_t bstrs_released = 0U;
    };

    class DispatchExceptionInfo final
    {
    public:
        DispatchExceptionInfo() = default;
        ~DispatchExceptionInfo() noexcept;

        DispatchExceptionInfo(const DispatchExceptionInfo &) = delete;
        DispatchExceptionInfo &operator=(const DispatchExceptionInfo &) = delete;
        DispatchExceptionInfo(DispatchExceptionInfo &&) = delete;
        DispatchExceptionInfo &operator=(DispatchExceptionInfo &&) = delete;

        [[nodiscard]] EXCEPINFO *output() noexcept;
        void record_result(HRESULT result) noexcept;

    private:
        EXCEPINFO exception_info_{};
    };

    void reset_dispatch_exception_cleanup_stats() noexcept;
    [[nodiscard]] DispatchExceptionCleanupStats dispatch_exception_cleanup_stats() noexcept;
}

#endif
