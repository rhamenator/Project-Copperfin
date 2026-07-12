// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "dispatch_exception_info.h"
#include "managed_pe_image.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <oleauto.h>

namespace
{
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::ScopedEnvironmentValue;
    using copperfin::test_support::write_text;

    constexpr std::size_t repeated_exception_count = 512U;

    class ScopedCurrentPath final
    {
    public:
        explicit ScopedCurrentPath(const std::filesystem::path &path)
            : original_(std::filesystem::current_path())
        {
            std::filesystem::current_path(path);
        }

        ~ScopedCurrentPath()
        {
            std::error_code ignored;
            std::filesystem::current_path(original_, ignored);
        }

        ScopedCurrentPath(const ScopedCurrentPath &) = delete;
        ScopedCurrentPath &operator=(const ScopedCurrentPath &) = delete;

    private:
        std::filesystem::path original_;
    };

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

    std::vector<char> read_binary(const std::filesystem::path &path)
    {
        std::ifstream input(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

    void write_binary(const std::filesystem::path &path, const std::vector<char> &bytes)
    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }

    bool clear_clr_directory(std::vector<char> &bytes)
    {
        if (bytes.size() < sizeof(IMAGE_DOS_HEADER))
        {
            return false;
        }
        IMAGE_DOS_HEADER dos_header{};
        std::memcpy(&dos_header, bytes.data(), sizeof(dos_header));
        if (dos_header.e_magic != IMAGE_DOS_SIGNATURE || dos_header.e_lfanew < 0)
        {
            return false;
        }
        const std::size_t nt_offset = static_cast<std::size_t>(dos_header.e_lfanew);
        const std::size_t optional_offset =
            nt_offset + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
        if (optional_offset + sizeof(WORD) > bytes.size())
        {
            return false;
        }
        WORD magic = 0U;
        std::memcpy(&magic, bytes.data() + optional_offset, sizeof(magic));
        std::size_t directory_offset = 0U;
        if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            directory_offset = optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory) +
                               IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR * sizeof(IMAGE_DATA_DIRECTORY);
        }
        else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            directory_offset = optional_offset + offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory) +
                               IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR * sizeof(IMAGE_DATA_DIRECTORY);
        }
        else
        {
            return false;
        }
        if (directory_offset + sizeof(IMAGE_DATA_DIRECTORY) > bytes.size())
        {
            return false;
        }
        std::fill_n(bytes.begin() + static_cast<std::ptrdiff_t>(directory_offset),
                    sizeof(IMAGE_DATA_DIRECTORY),
                    '\0');
        return true;
    }

    void test_managed_pe_classification_contract()
    {
        using copperfin::runtime::PortableExecutableKind;
        using copperfin::runtime::inspect_portable_executable;
        namespace fs = std::filesystem;

        const fs::path pe32_path = COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH;
        const fs::path pe64_path = COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH;
        expect(inspect_portable_executable(pe32_path) == PortableExecutableKind::managed,
               "#3947: PE32 CLR fixture should classify as managed on either host architecture");
        expect(inspect_portable_executable(pe64_path) == PortableExecutableKind::managed,
               "#3947: PE32+ CLR fixture should classify as managed on either host architecture");
        expect(inspect_portable_executable(COPPERFIN_DECLARED_DLL_FIXTURE_PATH) ==
                   PortableExecutableKind::native,
               "#3947: repository native DLL fixture should classify as native");
#if defined(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH)
        expect(inspect_portable_executable(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH) ==
                   PortableExecutableKind::managed,
               "#3947: mixed-mode fixture should expose its CLR directory");
#endif

        const fs::path temp_root = fs::temp_directory_path() / "copperfin_managed_pe_inspection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        for (const auto &[source_path, output_name] :
             std::array<std::pair<fs::path, std::string>, 2U>{
                 std::pair<fs::path, std::string>{pe32_path, "synthetic-native-pe32.dll"},
                 std::pair<fs::path, std::string>{pe64_path, "synthetic-native-pe64.dll"},
             })
        {
            std::vector<char> bytes = read_binary(source_path);
            expect(clear_clr_directory(bytes),
                   "#3947: synthetic native fixture should expose a writable CLR directory entry");
            const fs::path native_path = temp_root / output_name;
            write_binary(native_path, bytes);
            expect(inspect_portable_executable(native_path) == PortableExecutableKind::native,
                   "#3947: PE32/PE32+ image with an empty CLR directory should classify as native");
        }

        std::vector<char> truncated = read_binary(pe32_path);
        truncated.resize((std::min)(truncated.size(), sizeof(IMAGE_DOS_HEADER) / 2U));
        const fs::path truncated_path = temp_root / "truncated.dll";
        write_binary(truncated_path, truncated);
        expect(inspect_portable_executable(truncated_path) == PortableExecutableKind::invalid,
               "#3947: truncated PE headers should fail closed");

        std::vector<char> invalid_offset = read_binary(pe64_path);
        const LONG invalid_lfanew = (std::numeric_limits<LONG>::max)();
        std::memcpy(
            invalid_offset.data() + offsetof(IMAGE_DOS_HEADER, e_lfanew),
            &invalid_lfanew,
            sizeof(invalid_lfanew));
        const fs::path invalid_offset_path = temp_root / "invalid-offset.dll";
        write_binary(invalid_offset_path, invalid_offset);
        expect(inspect_portable_executable(invalid_offset_path) == PortableExecutableKind::invalid,
               "#3947: out-of-file e_lfanew should fail closed");

        fs::remove_all(temp_root, ignored);
    }

    void test_mixed_mode_native_export_precedence()
    {
#if defined(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH)
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_mixed_mode_declare";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path program_path = temp_root / "mixed_mode_native.prg";
        write_text(
            program_path,
            "DECLARE INTEGER CopperfinMixedModeNativeValue IN '" +
                fs::path(COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH).generic_string() + "'\n"
            "nResult = CopperfinMixedModeNativeValue()\n"
            "RETURN\n");

        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();

        expect(state.completed, "#3947: mixed-mode native export should remain callable: " + state.message);
        const auto result = state.globals.find("nresult");
        expect(result != state.globals.end() &&
                   copperfin::runtime::format_value(result->second) == "3947",
               "#3947: mixed-mode native export should win before managed reflection fallback");
        expect(stats.invoke_results == 0U,
               "#3947: mixed-mode native export should not enter IDispatch reflection");

        fs::remove_all(temp_root, ignored);
#endif
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
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.Add IN '" +
                fixture_path() + "' AS ManagedAdd INTEGER left, INTEGER right\n"
            "nResult = ManagedSuccess()\n"
            "nSum = ManagedAdd(19, 23)\n"
            "RETURN\n");

        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed, "#3943: successful managed DECLARE should complete: " + state.message);
        const auto result = state.globals.find("nresult");
        expect(result != state.globals.end(), "#3943: successful managed DECLARE should assign its result");
        if (result != state.globals.end())
        {
            expect(copperfin::runtime::format_value(result->second) == "42",
                   "#3943: successful managed dispatch should preserve its return value");
        }
        const auto sum = state.globals.find("nsum");
        expect(sum != state.globals.end(), "#3945: managed DECLARE should assign a typed argument result");
        if (sum != state.globals.end())
        {
            expect(copperfin::runtime::format_value(sum->second) == "42",
                   "#3945: managed dispatch should preserve INTEGER argument and return marshalling");
        }
        expect(
            std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
            {
                return event.category == "runtime.declare_dll" &&
                       event.detail.find("ManagedSuccess") != std::string::npos;
            }),
            "#3943: managed declarations should preserve the invariant runtime.declare_dll event");

        const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();
        expect(stats.invoke_results == 12U,
               "#3943/#3945: two successful managed calls should retain all six IDispatch stages");
        expect(stats.exception_results == 0U,
               "#3943: successful managed dispatch should not report an exception result");

        fs::remove_all(temp_root, ignored);
    }

    void test_managed_declare_explicit_relative_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_relative";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root / "managed");
        const fs::path relative_fixture = temp_root / "managed" / "Copperfin.ManagedDeclareFixture.dll";
        fs::copy_file(fixture_path(), relative_fixture, fs::copy_options::overwrite_existing, ignored);
        expect(!ignored && fs::exists(relative_fixture),
               "#3945: managed fixture should copy beneath the PRG working directory");

        const fs::path program_path = temp_root / "managed_relative.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnFortyTwo IN "
            "'managed/Copperfin.ManagedDeclareFixture.dll' AS ManagedRelative\n"
            "nResult = ManagedRelative()\n"
            "RETURN\n");

        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();

        expect(state.completed, "#3945: explicit-relative managed DECLARE should complete");
        const auto result = state.globals.find("nresult");
        expect(result != state.globals.end() &&
                   copperfin::runtime::format_value(result->second) == "42",
               "#3945: explicit-relative Assembly.LoadFrom should invoke the requested method");
        expect(stats.invoke_results == 6U && stats.exception_results == 0U,
               "#3945: explicit-relative managed dispatch should complete all reflection stages");
        expect(
            std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
            {
                return event.category == "runtime.declare_dll" &&
                       event.detail.find("managed/Copperfin.ManagedDeclareFixture.dll") != std::string::npos;
            }),
            "#3945: explicit-relative managed declaration should preserve its source-facing event path");

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
        expect(stats.invoke_results == repeated_exception_count * 6U,
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

    void test_managed_declare_parentless_search_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_parentless";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        fs::create_directories(temp_root / "moved-cwd");
        const std::string parentless_name = "Copperfin.ManagedDeclareFixture.parentless.dll";
        fs::copy_file(fixture_path(), temp_root / parentless_name, fs::copy_options::overwrite_existing, ignored);
        expect(!ignored, "#3947: parentless managed fixture should copy into the process search directory");

        const fs::path program_path = temp_root / "managed_parentless.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnFortyTwo IN '" +
                parentless_name + "' AS ManagedParentless\n"
            "nResult = ManagedParentless()\n"
            "RETURN\n");

        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        {
            const ScopedCurrentPath current_path(temp_root);
            auto session = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
            session.add_breakpoint({.file_path = program_path.string(), .line = 2U});
            const auto declared_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(declared_state.paused && declared_state.location.line == 2U,
                   "#3947: parentless declaration should complete before invocation breakpoint");

            fs::current_path(temp_root / "moved-cwd");
            const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();

            expect(state.completed,
                   "#3947: parentless managed DECLARE should retain its loader-resolved path after CWD changes: " +
                       state.message);
            const auto result = state.globals.find("nresult");
            expect(result != state.globals.end() &&
                       copperfin::runtime::format_value(result->second) == "42",
                   "#3947: parentless managed DECLARE should retain invocation behavior");
            expect(stats.invoke_results == 6U && stats.exception_results == 0U,
                   "#3947: parentless managed dispatch should complete all reflection stages");
            expect(
                std::any_of(state.events.begin(), state.events.end(), [&](const auto &event)
                {
                    return event.category == "runtime.declare_dll" &&
                           event.detail.find(parentless_name) != std::string::npos;
                }),
                "#3947: parentless managed declaration should preserve its source designator event");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_managed_load_failure_localization()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_dotnet_declare_bad_image";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root / "managed");

        std::ifstream source(fixture_path(), std::ios::binary);
        std::vector<char> truncated_bytes(512U, '\0');
        source.read(truncated_bytes.data(), static_cast<std::streamsize>(truncated_bytes.size()));
        expect(source.gcount() == static_cast<std::streamsize>(truncated_bytes.size()),
               "#3945: managed fixture should contain a complete PE header prefix");
        const fs::path bad_fixture = temp_root / "managed" / "bad-managed.dll";
        std::ofstream output(bad_fixture, std::ios::binary | std::ios::trunc);
        output.write(truncated_bytes.data(), static_cast<std::streamsize>(truncated_bytes.size()));
        output.close();

        const fs::path program_path = temp_root / "managed_bad_image.prg";
        write_text(
            program_path,
            "DECLARE INTEGER Copperfin.ManagedDeclareFixture.Methods.ReturnFortyTwo IN "
            "'managed/bad-managed.dll' AS ManagedBad\n"
            "nIgnored = ManagedBad()\n"
            "cFailureMessage = MESSAGE()\n"
            "RETURN\n");

        const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        copperfin::runtime::reset_dispatch_exception_cleanup_stats();
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto stats = copperfin::runtime::dispatch_exception_cleanup_stats();

        expect(state.completed,
               "#3945: managed bad-image load should retain the existing recoverable expression contract");
        expect(stats.invoke_results == 3U && stats.exception_results == 1U,
               "#3945: bad-image LoadFrom should fail at the path-loader reflection stage");
        const auto failure_message = state.globals.find("cfailuremessage");
        expect(failure_message != state.globals.end(),
               "#3945: bad-image LoadFrom should retain MESSAGE() diagnostic state");
        if (failure_message != state.globals.end())
        {
            const auto catalog = copperfin::localization::load_catalogs(
                copperfin::localization::resolve_catalog_root(),
                "qps-ploc");
            const std::string expected = catalog.translate(
                "Runtime.Prg.Dll.Error.DotNetAssemblyLoadFailed",
                {
                    {"hresult", std::to_string(static_cast<long>(DISP_E_EXCEPTION))},
                    {"path", bad_fixture.string()},
                });
            expect(copperfin::runtime::format_value(failure_message->second) == expected,
                   "#3945: bad-image load should preserve localized path and HRESULT placeholders");
        }

        fs::remove_all(temp_root, ignored);
    }
}

int main()
{
    test_managed_pe_classification_contract();
    test_mixed_mode_native_export_precedence();
    test_deferred_exception_cleanup_contract();
    test_managed_declare_success_contract();
    test_managed_declare_explicit_relative_path();
    test_managed_declare_parentless_search_path();
    test_repeated_managed_exception_cleanup();
    test_managed_load_failure_localization();

    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
