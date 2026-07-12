// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "dispatch_exception_info.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <oleauto.h>

namespace
{
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::ScopedEnvironmentValue;
    using copperfin::test_support::write_text;

    constexpr std::size_t repeated_exception_count = 512U;

    HRESULT __stdcall fill_deferred_exception(EXCEPINFO *exception_info)
    {
        exception_info->bstrSource = SysAllocString(L"Copperfin deferred source");
        exception_info->bstrDescription = SysAllocString(L"Copperfin deferred description");
        exception_info->bstrHelpFile = SysAllocString(L"Copperfin deferred help");
        return S_OK;
    }

    std::string fixture_path()
    {
        return std::filesystem::path(COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH).generic_string();
    }

    void test_deferred_exception_cleanup_contract()
    {
        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        {
            copperfin::runtime::DispatchExceptionInfo exception_info;
            exception_info.output()->pfnDeferredFillIn = &fill_deferred_exception;
            exception_info.record_result(DISP_E_EXCEPTION);
        }

        const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();
        expect(stats.invoke_results == 1U,
               "#3943: controlled EXCEPINFO cleanup should retain the Invoke result count");
        expect(stats.exception_results == 1U,
               "#3943: controlled EXCEPINFO cleanup should retain DISP_E_EXCEPTION identity");
        expect(stats.deferred_fill_calls == 1U,
               "#3943: cleanup should invoke deferred fill-in exactly once");
        expect(stats.bstrs_released == 3U,
               "#3943: cleanup should release source, description, and help BSTRs");
        expect(stats.source_bstrs_released == 1U &&
                   stats.description_bstrs_released == 1U &&
                   stats.help_file_bstrs_released == 1U,
               "#3943: cleanup should release each populated EXCEPINFO BSTR field exactly once");

        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        {
            copperfin::runtime::DispatchExceptionInfo exception_info;
            exception_info.output()->pfnDeferredFillIn = &fill_deferred_exception;
            exception_info.record_result(S_OK);
        }
        const auto success_stats = copperfin::runtime::dispatch_exception_cleanup_stats();
        expect(success_stats.invoke_results == 1U && success_stats.exception_results == 0U,
               "#3943: controlled success should retain a non-exception Invoke result");
        expect(success_stats.deferred_fill_calls == 0U && success_stats.bstrs_released == 0U,
               "#3943: non-exception results must not execute a deferred exception callback");
    }

    void test_managed_declare_success_contract()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_success";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path program_path = temp_root / "managed_success.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnFortyTwo IN '" +
                fixture_path() + "' AS ManagedSuccess\n"
            "nResult = ManagedSuccess()\n"
            "RETURN\n");

        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed, "#3943: successful managed DECLARE should complete");
        const auto result = state.globals.find("nresult");
        expect(result != state.globals.end(), "#3943: successful managed DECLARE should assign its result");
        if (result != state.globals.end())
        {
            expect(copperfin::runtime::format_value(result->second) == "42",
                   "#3943: successful managed dispatch should preserve its return value");
        }
        expect(
            std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
            {
                return event.category == "runtime.declare_dll" &&
                       event.detail.find("ManagedSuccess") != std::string::npos;
            }),
            "#3943: managed declarations should preserve the invariant runtime.declare_dll event");

        const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();
        expect(stats.invoke_results == 4U,
               "#3943: successful managed dispatch should retain all four IDispatch stages");
        expect(stats.exception_results == 0U,
               "#3943: successful managed dispatch should not report an exception result");

        fs::remove_all(temp_root, ignored);
    }

    void test_repeated_managed_exception_cleanup()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_throw";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path program_path = temp_root / "managed_throw.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ThrowAlways IN '" +
                fixture_path() + "' AS ManagedThrow\n"
            "FOR nCall = 1 TO " + std::to_string(repeated_exception_count) + "\n"
            "  nIgnored = ManagedThrow()\n"
            "ENDFOR\n"
            "cFailureMessage = MESSAGE()\n"
            "RETURN\n");

        const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();

        expect(state.completed,
               "#3943: repeatedly-throwing managed DECLARE fixture should retain the existing recoverable contract");
        expect(stats.invoke_results == repeated_exception_count * 4U,
               "#3943: repeated managed calls should execute all IDispatch stages without stale state");
        expect(stats.exception_results == repeated_exception_count,
               "#3943: every controlled managed throw should retain DISP_E_EXCEPTION identity");
        expect(stats.bstrs_released >= repeated_exception_count,
               "#3943: every managed exception should release at least one COM-owned BSTR");
        expect(stats.bstrs_released ==
                   stats.source_bstrs_released +
                       stats.description_bstrs_released +
                       stats.help_file_bstrs_released,
               "#3943: managed cleanup should account for every populated EXCEPINFO BSTR field");

        const auto failure_message = state.globals.find("cfailuremessage");
        expect(failure_message != state.globals.end(),
               "#3943: managed failure should retain the localized MESSAGE() contract");
        if (failure_message != state.globals.end())
        {
            const std::string text = copperfin::runtime::format_value(failure_message->second);
            const auto catalog = copperfin::localization::load_catalogs(
                copperfin::localization::resolve_catalog_root(),
                "qps-ploc");
            const std::string expected = catalog.translate(
                "Runtime.Prg.Dll.Error.DotNetMethodInvokeFailed",
                {{"hresult", std::to_string(static_cast<long>(DISP_E_EXCEPTION))}});
            expect(text == expected,
                   "#3943: managed failure should preserve the localized diagnostic and HRESULT value");
        }

        fs::remove_all(temp_root, ignored);
    }
}

int main()
{
    test_deferred_exception_cleanup_contract();
    test_managed_declare_success_contract();
    test_repeated_managed_exception_cleanup();

    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
