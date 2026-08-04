// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "dispatch_exception_info.h"

#if defined(_WIN32)

#include <oleauto.h>

namespace copperfin::runtime
{
    namespace
    {
#if defined(COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS)
        thread_local DispatchExceptionCleanupStats cleanup_stats;
#endif

        bool release_bstr(BSTR &value) noexcept
        {
            if (value == nullptr)
            {
                return false;
            }

            SysFreeString(value);
            value = nullptr;
            return true;
        }
    }

    DispatchExceptionInfo::~DispatchExceptionInfo() noexcept
    {
        if (result_recorded_ && result_ == DISP_E_EXCEPTION &&
            exception_info_.pfnDeferredFillIn != nullptr)
        {
            const auto deferred_fill = exception_info_.pfnDeferredFillIn;
            exception_info_.pfnDeferredFillIn = nullptr;
            (void)deferred_fill(&exception_info_);
#if defined(COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS)
            ++cleanup_stats.deferred_fill_calls;
#endif
        }

        const bool source_released = release_bstr(exception_info_.bstrSource);
        const bool description_released = release_bstr(exception_info_.bstrDescription);
        const bool help_file_released = release_bstr(exception_info_.bstrHelpFile);
#if defined(COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS)
        cleanup_stats.source_bstrs_released += source_released ? 1U : 0U;
        cleanup_stats.description_bstrs_released += description_released ? 1U : 0U;
        cleanup_stats.help_file_bstrs_released += help_file_released ? 1U : 0U;
        cleanup_stats.bstrs_released +=
            (source_released ? 1U : 0U) +
            (description_released ? 1U : 0U) +
            (help_file_released ? 1U : 0U);
#endif
    }

    EXCEPINFO *DispatchExceptionInfo::output() noexcept
    {
        return &exception_info_;
    }

    void DispatchExceptionInfo::record_result(HRESULT result) noexcept
    {
        result_ = result;
        result_recorded_ = true;
#if defined(COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS)
        ++cleanup_stats.invoke_results;
        if (result == DISP_E_EXCEPTION)
        {
            ++cleanup_stats.exception_results;
        }
#endif
    }

#if defined(COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS)
    void reset_dispatch_exception_cleanup_stats() noexcept
    {
        cleanup_stats = {};
    }

    DispatchExceptionCleanupStats dispatch_exception_cleanup_stats() noexcept
    {
        return cleanup_stats;
    }
#endif
}

#endif
