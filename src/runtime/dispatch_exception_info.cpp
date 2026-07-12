// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "dispatch_exception_info.h"

#if defined(_WIN32)

#include <atomic>
#include <oleauto.h>

namespace copperfin::runtime
{
    namespace
    {
        std::atomic<std::uint64_t> invoke_results{0U};
        std::atomic<std::uint64_t> exception_results{0U};
        std::atomic<std::uint64_t> deferred_fill_calls{0U};
        std::atomic<std::uint64_t> bstrs_released{0U};

        void release_bstr(BSTR &value) noexcept
        {
            if (value == nullptr)
            {
                return;
            }

            SysFreeString(value);
            value = nullptr;
            bstrs_released.fetch_add(1U, std::memory_order_relaxed);
        }
    }

    DispatchExceptionInfo::~DispatchExceptionInfo() noexcept
    {
        if (exception_info_.pfnDeferredFillIn != nullptr)
        {
            const auto deferred_fill = exception_info_.pfnDeferredFillIn;
            exception_info_.pfnDeferredFillIn = nullptr;
            (void)deferred_fill(&exception_info_);
            deferred_fill_calls.fetch_add(1U, std::memory_order_relaxed);
        }

        release_bstr(exception_info_.bstrSource);
        release_bstr(exception_info_.bstrDescription);
        release_bstr(exception_info_.bstrHelpFile);
    }

    EXCEPINFO *DispatchExceptionInfo::output() noexcept
    {
        return &exception_info_;
    }

    void DispatchExceptionInfo::record_result(HRESULT result) noexcept
    {
        invoke_results.fetch_add(1U, std::memory_order_relaxed);
        if (result == DISP_E_EXCEPTION)
        {
            exception_results.fetch_add(1U, std::memory_order_relaxed);
        }
    }

    void reset_dispatch_exception_cleanup_stats() noexcept
    {
        invoke_results.store(0U, std::memory_order_relaxed);
        exception_results.store(0U, std::memory_order_relaxed);
        deferred_fill_calls.store(0U, std::memory_order_relaxed);
        bstrs_released.store(0U, std::memory_order_relaxed);
    }

    DispatchExceptionCleanupStats dispatch_exception_cleanup_stats() noexcept
    {
        return {
            .invoke_results = invoke_results.load(std::memory_order_relaxed),
            .exception_results = exception_results.load(std::memory_order_relaxed),
            .deferred_fill_calls = deferred_fill_calls.load(std::memory_order_relaxed),
            .bstrs_released = bstrs_released.load(std::memory_order_relaxed),
        };
    }
}

#endif
