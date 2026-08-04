// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#if defined(_WIN32)

#include <cstdint>
#include <oaidl.h>

namespace copperfin::runtime
{
#if defined(COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS)
    struct DispatchExceptionCleanupStats
    {
        std::uint64_t invoke_results = 0U;
        std::uint64_t exception_results = 0U;
        std::uint64_t deferred_fill_calls = 0U;
        std::uint64_t bstrs_released = 0U;
        std::uint64_t source_bstrs_released = 0U;
        std::uint64_t description_bstrs_released = 0U;
        std::uint64_t help_file_bstrs_released = 0U;
    };
#endif

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
        HRESULT result_ = S_OK;
        bool result_recorded_ = false;
    };

#if defined(COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS)
    void reset_dispatch_exception_cleanup_stats() noexcept;
    [[nodiscard]] DispatchExceptionCleanupStats dispatch_exception_cleanup_stats() noexcept;
#endif
}

#endif
