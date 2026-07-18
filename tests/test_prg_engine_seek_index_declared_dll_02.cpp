#include "test_prg_engine_seek_index_support.h"

namespace copperfin::seek_index_tests
{
void test_declared_dll_short_return_uses_signed_16_bit_width() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_short_return";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3938: controlled SHORT-return fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_short_return.prg";
    write_text(
        main_path,
        "DECLARE SHORT CopperfinDeclaredDllShortNegative IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE SHORT CopperfinDeclaredDllShortInternetShape IN 'native/" + fixture_name.string() +
            "' INTEGER @ lpdwFlags, INTEGER dwReserved\n"
        "DECLARE SHORT InternetGetConnectedState IN 'wininet.dll' INTEGER @ lpdwFlags, INTEGER dwReserved\n"
        "nFlags = 0\n"
        "nRealFlags = 0\n"
        "nNegative = CopperfinDeclaredDllShortNegative()\n"
        "nConnected = CopperfinDeclaredDllShortInternetShape(@nFlags, 0)\n"
        "nRealConnected = InternetGetConnectedState(@nRealFlags, 0)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3938: SHORT-return fixture should complete: " + state.message);

    const auto negative = state.globals.find("nnegative");
    const auto connected = state.globals.find("nconnected");
    const auto flags = state.globals.find("nflags");
    const auto real_connected = state.globals.find("nrealconnected");
    const auto real_flags = state.globals.find("nrealflags");
    expect(negative != state.globals.end(), "#3938: negative SHORT return should be captured");
    expect(connected != state.globals.end(), "#3938: WinInet-shaped SHORT return should be captured");
    expect(flags != state.globals.end(), "#3938: WinInet-shaped INTEGER @ output should be captured");
    expect(real_connected != state.globals.end(), "#3938: shipped WinInet SHORT return should be captured");
    expect(real_flags != state.globals.end(), "#3938: shipped WinInet INTEGER @ output should be captured");
    if (negative != state.globals.end())
    {
        expect(copperfin::runtime::format_value(negative->second) == "-12345",
               "#3938: SHORT returns should preserve signed 16-bit values");
    }
    if (connected != state.globals.end())
    {
        expect(copperfin::runtime::format_value(connected->second) == "-1",
               "#3938: Microsoft-shipped WinInet declaration shape should preserve a negative SHORT return");
    }
    if (flags != state.globals.end())
    {
        expect(copperfin::runtime::format_value(flags->second) == "305419896",
               "#3938: SHORT return selection should preserve adjacent INTEGER @ writeback");
    }
    if (real_connected != state.globals.end())
    {
        const std::string actual = copperfin::runtime::format_value(real_connected->second);
        expect(actual == "0" || actual == "1",
               "#3938: shipped InternetGetConnectedState SHORT declaration should return a logical status; actual=" + actual);
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllShortInternetShape") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3938: SHORT declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_short_parameter_is_rejected_and_localized() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_short_parameter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "declared_dll_short_parameter.prg";
    write_text(
        main_path,
        "DECLARE SHORT InvalidShortParameter IN 'kernel32.dll' SHORT invalid\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#3938: help-invalid SHORT parameters should reject the declaration");
    expect(state.message.find("[!! ") == 0U &&
               state.message.find("SHORT") != std::string::npos &&
               state.message.find("parameter type") == std::string::npos,
           "#3938: SHORT parameter rejection should pseudo-localize prose and preserve the invariant type token");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32api_search_and_ansi_fallback() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32api";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "declared_dll_win32api.prg";
    write_text(
        main_path,
        "DECLARE INTEGER GetCurrentProcessId IN WIN32API\n"
        "DECLARE INTEGER GetSystemMetrics IN WIN32API INTEGER index\n"
        "DECLARE INTEGER GetUserName IN WIN32API STRING @ buffer, INTEGER @ size\n"
        "cUserBuffer = SPACE(256)\n"
        "nUserSize = 256\n"
        "nProcessId = GetCurrentProcessId()\n"
        "nScreenWidth = GetSystemMetrics(0)\n"
        "nUserResult = GetUserName(@cUserBuffer, @nUserSize)\n"
        "cUserName = cUserBuffer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3939: bare WIN32API declarations should complete: " + state.message);

    const auto process_id = state.globals.find("nprocessid");
    const auto screen_width = state.globals.find("nscreenwidth");
    const auto user_result = state.globals.find("nuserresult");
    const auto user_size = state.globals.find("nusersize");
    const auto user_name = state.globals.find("cusername");
    expect(process_id != state.globals.end(), "#3939: Kernel32 result should be captured");
    expect(screen_width != state.globals.end(), "#3939: User32 result should be captured");
    expect(user_result != state.globals.end(), "#3939: Advapi32 ANSI-fallback result should be captured");
    expect(user_size != state.globals.end(), "#3939: GetUserName size writeback should be captured");
    expect(user_name != state.globals.end(), "#3939: GetUserName buffer writeback should be captured");
    if (process_id != state.globals.end())
    {
        expect(copperfin::runtime::format_value(process_id->second) != "0",
               "#3939: GetCurrentProcessId should return a nonzero process id");
    }
    if (user_result != state.globals.end())
    {
        expect(copperfin::runtime::format_value(user_result->second) == "1",
               "#3939: GetUserName should resolve through GetUserNameA");
    }
    if (user_size != state.globals.end())
    {
        expect(copperfin::runtime::format_value(user_size->second) != "256",
               "#3939: GetUserNameA should update its size argument");
    }
    if (user_name != state.globals.end())
    {
        expect(!copperfin::runtime::format_value(user_name->second).empty(),
               "#3939: GetUserNameA should write back a nonempty user name");
    }

    const auto has_declare_event = [&](const std::string &function_name)
    {
        return std::any_of(state.events.begin(), state.events.end(), [&](const auto &event)
        {
            return event.category == "runtime.declare_dll" &&
                   event.detail == function_name + " IN WIN32API";
        });
    };
    expect(has_declare_event("GetCurrentProcessId"),
           "#3939: Kernel32 declaration event should preserve the WIN32API designator");
    expect(has_declare_event("GetSystemMetrics"),
           "#3939: User32 declaration event should preserve the WIN32API designator");
    expect(has_declare_event("GetUserName"),
           "#3939: ANSI-fallback declaration event should preserve the source export name");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32api_missing_symbol_localizes() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32api_missing";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "declared_dll_win32api_missing.prg";
    write_text(
        main_path,
        "DECLARE INTEGER CopperfinMissingWin32ApiSymbol IN WIN32API\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#3939: a missing WIN32API symbol should fail the declaration");
    expect(state.message.find("[!! ") == 0U &&
               state.message.find("CopperfinMissingWin32ApiSymbol") != std::string::npos &&
               state.message.find("WIN32API") != std::string::npos &&
               state.message.find("function 'CopperfinMissingWin32ApiSymbol' not found") == std::string::npos,
           "#3939: WIN32API failure should pseudo-localize prose and preserve machine identifiers");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_module_ownership_and_failed_redeclare_rollback() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_ownership";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(declared_dll_fixture_source_path(), fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3939: module-ownership fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_ownership.prg";
    write_text(
        main_path,
        "PUBLIC nDeclareErrors\n"
        "nDeclareErrors = 0\n"
        "DECLARE INTEGER CopperfinDeclaredDllFixtureValue IN 'native/" + fixture_name.string() +
            "' AS FirstAlias\n"
        "DECLARE INTEGER CopperfinDeclaredDllFixtureValue IN 'native/" + fixture_name.string() +
            "' AS SecondAlias\n"
        "ON ERROR DO HandleDeclareError\n"
        "DECLARE INTEGER CopperfinMissingDeclaredDllExport IN 'native/" + fixture_name.string() +
            "' AS FirstAlias\n"
        "ON ERROR\n"
        "nFirstValue = FirstAlias()\n"
        "nSecondValue = SecondAlias()\n"
        "RETURN\n"
        "PROCEDURE HandleDeclareError\n"
        "nDeclareErrors = nDeclareErrors + 1\n"
        "RETURN\n"
        "ENDPROC\n");

    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3939: failed redeclaration should roll back to the old binding: " + state.message);

        const auto first_value = state.globals.find("nfirstvalue");
        const auto second_value = state.globals.find("nsecondvalue");
        const auto error_count = state.globals.find("ndeclareerrors");
        expect(first_value != state.globals.end() &&
                   copperfin::runtime::format_value(first_value->second) == "3921",
               "#3939: failed redeclaration should retain the first alias binding");
        expect(second_value != state.globals.end() &&
                   copperfin::runtime::format_value(second_value->second) == "3921",
               "#3939: a second alias should retain its independently owned module reference");
        expect(error_count != state.globals.end() &&
                   copperfin::runtime::format_value(error_count->second) == "1",
               "#3939: failed redeclaration should dispatch exactly one recoverable error");
        expect(GetModuleHandleW(fixture_copy.wstring().c_str()) != nullptr,
               "#3939: the owned fixture module should remain loaded while aliases are callable");
    }

    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3939: session cleanup should release every owned module reference");
    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_explicit_ansi_fallback_and_exact_precedence() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_ansi_fallback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3942: explicit ANSI-fallback fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_ansi_fallback.prg";
    write_text(
        main_path,
        "DECLARE LONG CopperfinDeclaredDllAnsiOnly IN '" + fixture_name.string() +
            "' AS ParentlessAnsiAlias LONG value\n"
        "DECLARE LONG CopperfinDeclaredDllModulePath IN '" + fixture_name.string() +
            "' AS ParentlessModulePathAlias STRING @ buffer, LONG capacity\n"
        "DECLARE LONG CopperfinDeclaredDllAnsiOnly IN 'native/" + fixture_name.string() +
            "' AS RelativeAnsiAlias LONG value\n"
        "DECLARE LONG CopperfinDeclaredDllAnsiCdeclOnly IN 'native/" + fixture_name.string() +
            "' AS RelativeCdeclAnsiAlias\n"
        "DECLARE LONG CopperfinDeclaredDllModulePath IN 'native/" + fixture_name.string() +
            "' AS RelativeModulePathAlias STRING @ buffer, LONG capacity\n"
        "DECLARE LONG CopperfinDeclaredDllExactPrecedence IN 'native/" + fixture_name.string() +
            "' AS ExactPrecedenceAlias LONG value\n"
        "DECLARE INTEGER GetSystemDirectory IN 'kernel32.dll' STRING @ buffer, INTEGER size\n"
        "cParentlessModulePath = SPACE(32768)\n"
        "cRelativeModulePath = SPACE(32768)\n"
        "cSystemDirectory = SPACE(32768)\n"
        "nParentlessAnsi = ParentlessAnsiAlias(3)\n"
        "nParentlessModulePathLength = ParentlessModulePathAlias(@cParentlessModulePath, 32768)\n"
        "nRelativeAnsi = RelativeAnsiAlias(2)\n"
        "nRelativeCdeclAnsi = RelativeCdeclAnsiAlias()\n"
        "nRelativeModulePathLength = RelativeModulePathAlias(@cRelativeModulePath, 32768)\n"
        "nExact = ExactPrecedenceAlias(41)\n"
        "nSystemDirectoryLength = GetSystemDirectory(@cSystemDirectory, 32768)\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3942: explicit ANSI-fallback declarations should complete: " + state.message);

        const auto relative_ansi = state.globals.find("nrelativeansi");
        const auto relative_cdecl_ansi = state.globals.find("nrelativecdeclansi");
        const auto parentless_ansi = state.globals.find("nparentlessansi");
        const auto parentless_module_path = state.globals.find("cparentlessmodulepath");
        const auto relative_module_path = state.globals.find("crelativemodulepath");
        const auto parentless_module_path_length = state.globals.find("nparentlessmodulepathlength");
        const auto relative_module_path_length = state.globals.find("nrelativemodulepathlength");
        const auto exact = state.globals.find("nexact");
        const auto system_directory = state.globals.find("csystemdirectory");
        const auto system_directory_length = state.globals.find("nsystemdirectorylength");
        expect(relative_ansi != state.globals.end() &&
                   copperfin::runtime::format_value(relative_ansi->second) == "3944",
               "#3942: quoted relative lookup should bind the A-suffixed export");
        expect(relative_cdecl_ansi != state.globals.end() &&
                   copperfin::runtime::format_value(relative_cdecl_ansi->second) == "4002",
               "#3942: quoted relative lookup should bind the cdecl A-suffixed export");
        expect(parentless_ansi != state.globals.end() &&
                   copperfin::runtime::format_value(parentless_ansi->second) == "3945",
               "#3942: parentless loader lookup should bind the A-suffixed export");
        expect(parentless_module_path != state.globals.end() &&
                   fs::equivalent(
                       fs::path(copperfin::runtime::format_value(parentless_module_path->second)),
                       fixture_source,
                       ignored) &&
                   !ignored,
               "#3942: parentless lookup should bind the fixture beside the test executable");
        ignored.clear();
        expect(relative_module_path != state.globals.end() &&
                   fs::equivalent(
                       fs::path(copperfin::runtime::format_value(relative_module_path->second)),
                       fixture_copy,
                       ignored) &&
                   !ignored,
               "#3942: explicit relative lookup should bind the copied fixture path");
        ignored.clear();
        expect(parentless_module_path_length != state.globals.end() &&
                   parentless_module_path_length->second.kind == copperfin::runtime::PrgValueKind::number &&
                   parentless_module_path_length->second.number_value > 0.0,
               "#3942: parentless module-path fallback should return a positive path length");
        expect(relative_module_path_length != state.globals.end() &&
                   relative_module_path_length->second.kind == copperfin::runtime::PrgValueKind::number &&
                   relative_module_path_length->second.number_value > 0.0,
               "#3942: relative module-path fallback should return a positive path length");
        expect(exact != state.globals.end() &&
                   copperfin::runtime::format_value(exact->second) == "42",
               "#3942: exact export should take precedence over its A-suffixed sibling");
        expect(system_directory != state.globals.end() &&
                   !copperfin::runtime::format_value(system_directory->second).empty(),
               "#3942: shipped GetSystemDirectory declaration should write back a path");
        expect(system_directory_length != state.globals.end() &&
                   system_directory_length->second.kind == copperfin::runtime::PrgValueKind::number &&
                   system_directory_length->second.number_value > 0.0,
               "#3942: shipped GetSystemDirectory declaration should return a positive path length");

        const auto has_declare_event = [&](const std::string &detail)
        {
            return std::any_of(state.events.begin(), state.events.end(), [&](const auto &event)
            {
                return event.category == "runtime.declare_dll" && event.detail == detail;
            });
        };
        expect(has_declare_event("RelativeAnsiAlias IN native/" + fixture_name.string()),
               "#3942: relative fallback event should preserve source alias and library designator");
        expect(has_declare_event("ParentlessAnsiAlias IN " + fixture_name.string()),
               "#3942: parentless fallback event should preserve source alias and library designator");
        expect(has_declare_event("ExactPrecedenceAlias IN native/" + fixture_name.string()),
               "#3942: exact-precedence event should preserve source export identity");
        expect(has_declare_event("GetSystemDirectory IN kernel32.dll"),
               "#3942: shipped explicit-DLL event should preserve its unsuffixed source function name");
        expect(GetModuleHandleW(fixture_copy.wstring().c_str()) != nullptr,
               "#3942: relative fixture module should remain loaded during the session");
        expect(GetModuleHandleW(fixture_source.wstring().c_str()) != nullptr,
               "#3942: parentless fixture module should remain loaded during the session");
    }

    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3942: explicit fallback module references should unload with the session");
    expect(GetModuleHandleW(fixture_source.wstring().c_str()) == nullptr,
           "#3942: parentless fallback module references should unload with the session");
    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_argument_count_is_validated_before_native_entry() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_arity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(declared_dll_fixture_source_path(), fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3946: controlled arity fixture should copy under the PRG working directory");

    const std::string module = "native/" + fixture_name.string();
    const std::string counter_declarations =
        "DECLARE LONG CopperfinDeclaredDllArityReset IN '" + module + "' AS ArityReset\n"
        "DECLARE LONG CopperfinDeclaredDllArityCount IN '" + module + "' AS ArityCount\n";
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path baseline_path = temp_root / "arity_baseline.prg";
    write_text(
        baseline_path,
        counter_declarations +
        "DECLARE LONG CopperfinDeclaredDllArityOne IN '" + module + "' AS ArityTarget LONG value\n"
        "nReset = ArityReset()\n"
        "nValidResult = ArityTarget(7)\n"
        "nNativeEntries = ArityCount()\n"
        "RETURN\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(baseline_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3946: arity fixture baseline should complete: " + state.message);
        const auto valid_result = state.globals.find("nvalidresult");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(valid_result != state.globals.end() &&
                   copperfin::runtime::format_value(valid_result->second) == "7",
               "#3946: valid one-argument stdcall should enter the controlled fixture");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "1",
               "#3946: controlled fixture counter should observe the valid native entry");
    }

    const fs::path handler_argument_path = temp_root / "arity_handler_argument.prg";
    write_text(
        handler_argument_path,
        counter_declarations +
        "DECLARE LONG CopperfinDeclaredDllArityOne IN '" + module + "' AS ArityTarget LONG value\n"
        "nReset = ArityReset()\n"
        "ON ERROR DO HandleOuterError WITH ArityTarget()\n"
        "DO CopperfinMissingArityHandlerTarget\n"
        "ON ERROR\n"
        "nOuterCode = ERROR()\n"
        "nNativeEntries = ArityCount()\n"
        "RETURN\n"
        "PROCEDURE HandleOuterError\n"
        "RETURN\n"
        "ENDPROC\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(handler_argument_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(!state.completed && state.reason == copperfin::runtime::DebugPauseReason::error,
               "#3946: an arity fault in ON ERROR arguments should pause safely");
        expect(state.message == "Too few arguments.",
               "#3946: the outer run boundary should not wrap an arity compatibility fault");

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "#3946: the runtime should remain resumable after an outer-boundary arity fault: " + state.message);
        const auto outer_code = state.globals.find("noutercode");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(outer_code != state.globals.end() &&
                   copperfin::runtime::format_value(outer_code->second) == "1229",
               "#3946: the outer run boundary should preserve VFP error 1229");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3946: handler-argument arity faults must not enter the native target");
    }

    struct ArityCase {
        std::string label;
        std::string export_name;
        std::string declaration_suffix;
        std::string invocation;
        int expected_error;
        std::string expected_message;
        bool pseudo_locale = false;
    };

    const std::vector<ArityCase> cases{
        {"zero_too_many", "CopperfinDeclaredDllArityZero", "", "ArityTarget(1)", 1230, "Too many arguments."},
        {"stdcall_one_too_few", "CopperfinDeclaredDllArityOne", " LONG value", "ArityTarget()", 1229, "Too few arguments."},
        {"stdcall_one_too_many", "CopperfinDeclaredDllArityOne", " LONG value", "ArityTarget(1, 2)", 1230, "Too many arguments."},
        {"cdecl_one_too_few", "CopperfinDeclaredDllArityCdeclOne", " LONG value", "ArityTarget()", 1229, "Too few arguments.", true},
        {"mixed_byref_too_few", "CopperfinDeclaredDllArityMixed",
         " LONG first, DOUBLE second, INTEGER64 @ third, SINGLE fourth",
         "ArityTarget(1, 2.0, @nWide)", 1229, "Too few arguments."},
        {"eight_too_few", "CopperfinDeclaredDllArityEight",
         " LONG first, LONG second, LONG third, LONG fourth, LONG fifth, LONG sixth, LONG seventh, LONG eighth",
         "ArityTarget(1, 2, 3, 4, 5, 6, 7)", 1229, "Too few arguments."},
        {"eight_too_many", "CopperfinDeclaredDllArityEight",
         " LONG first, LONG second, LONG third, LONG fourth, LONG fifth, LONG sixth, LONG seventh, LONG eighth",
         "ArityTarget(1, 2, 3, 4, 5, 6, 7, 8, 9)", 1230, "Too many arguments."},
        {"native_limit", "CopperfinDeclaredDllArityCdeclOne",
         " LONG first, LONG second, LONG third, LONG fourth, LONG fifth, LONG sixth, LONG seventh, LONG eighth, LONG ninth",
         "ArityTarget(1, 2, 3, 4, 5, 6, 7, 8, 9)", 1230,
         "Native DLL call has 9 arguments; the maximum is 8", true}
    };

    for (const ArityCase &arity_case : cases) {
        set_env_value("COPPERFIN_LOCALE", arity_case.pseudo_locale ? "qps-ploc" : "en-US", true);
        const fs::path script_path = temp_root / (arity_case.label + ".prg");
        write_text(
            script_path,
            "PUBLIC nCapturedCode\n"
            "PUBLIC cCapturedMessage\n"
            "nCapturedCode = 0\n"
            "cCapturedMessage = ''\n" +
            counter_declarations +
            "DECLARE LONG " + arity_case.export_name + " IN '" + module +
                "' AS ArityTarget" + arity_case.declaration_suffix + "\n"
            "nWide = 3\n"
            "nReset = ArityReset()\n"
            "ON ERROR DO HandleArityError\n"
            "nUnexpected = " + arity_case.invocation + "\n"
            "ON ERROR\n"
            "nNativeEntries = ArityCount()\n"
            "RETURN\n"
            "PROCEDURE HandleArityError\n"
            "nCapturedCode = ERROR()\n"
            "cCapturedMessage = MESSAGE()\n"
            "RETURN\n"
            "ENDPROC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3946: recoverable " + arity_case.label + " mismatch should complete: " + state.message);

        const auto captured_code = state.globals.find("ncapturedcode");
        const auto captured_message = state.globals.find("ccapturedmessage");
        const auto native_entries = state.globals.find("nnativeentries");
        const std::string expected_message = arity_case.pseudo_locale
            ? copperfin::localization::pseudo_localize(arity_case.expected_message)
            : arity_case.expected_message;
        expect(captured_code != state.globals.end() &&
                   copperfin::runtime::format_value(captured_code->second) == std::to_string(arity_case.expected_error),
               "#3946: " + arity_case.label + " should preserve the grounded VFP error identity");
        expect(captured_message != state.globals.end() &&
                   copperfin::runtime::format_value(captured_message->second) == expected_message,
               "#3946: " + arity_case.label + " should preserve the localized arity diagnostic");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3946: " + arity_case.label + " must not enter the native target");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.declare_dll" &&
                          event.detail == "ArityTarget IN " + module;
               }),
               "#3946: " + arity_case.label + " should retain the invariant source alias/path DECLARE event");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.error" && event.detail == expected_message;
               }),
               "#3946: " + arity_case.label + " should emit the stable runtime.error category");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "runtime.error_handler";
               }),
               "#3946: " + arity_case.label + " should emit the stable runtime.error_handler category");
    }

    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3946: arity validation sessions should release every controlled module reference");
    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_numeric_byref_requires_callsite_reference() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_numeric_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(declared_dll_fixture_source_path(), fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3944: controlled numeric by-reference fixture should copy under the PRG working directory");

    const std::string module = "native/" + fixture_name.string();
    const std::string declarations =
        "DECLARE LONG CopperfinDeclaredDllNumericByRefReset IN '" + module + "' AS NumericByRefReset\n"
        "DECLARE LONG CopperfinDeclaredDllNumericByRefCount IN '" + module + "' AS NumericByRefCount\n"
        "DECLARE LONG CopperfinDeclaredDllNumericByRefProbe IN '" + module +
            "' AS NumericByRef INTEGER @ integerValue, LONG @ longValue, INTEGER64 @ integer64Value, SINGLE @ singleValue, DOUBLE @ doubleValue\n";
    const std::string initialize_values =
        "nInteger = 1\n"
        "nLong = 2\n"
        "nInteger64 = 3\n"
        "nSingle = 4\n"
        "nDouble = 5\n";

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path valid_path = temp_root / "numeric_byref_valid.prg";
    write_text(
        valid_path,
        declarations + initialize_values +
        "nReset = NumericByRefReset()\n"
        "nResult = NumericByRef(@nInteger, @nLong, @nInteger64, @nSingle, @nDouble)\n"
        "nNativeEntries = NumericByRefCount()\n"
        "RETURN\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(valid_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#3944: valid numeric by-reference call should complete: " + state.message);
        const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &label) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end() &&
                       copperfin::runtime::format_value(found->second) == expected,
                   label);
        };
        expect_value("nresult", "3944", "#3944: valid numeric by-reference fixture should return its sentinel");
        expect_value("ninteger", "-11", "#3944: INTEGER @ should retain valid writeback");
        expect_value("nlong", "-22", "#3944: LONG @ should retain valid writeback");
        expect_value("ninteger64", "-4294967297", "#3944: INTEGER64 @ should retain valid writeback");
        expect_value("nsingle", "3.5", "#3944: SINGLE @ should retain valid writeback");
        expect_value("ndouble", "4.25", "#3944: DOUBLE @ should retain valid writeback");
        expect_value("nnativeentries", "1", "#3944: valid numeric by-reference call should enter native code once");

        const HMODULE loaded_module = GetModuleHandleW(fixture_copy.wstring().c_str());
        expect(loaded_module != nullptr, "#3944: controlled numeric by-reference module should be loaded");
        if (loaded_module != nullptr) {
#if defined(_WIN64)
            expect(GetProcAddress(loaded_module, "CopperfinDeclaredDllNumericByRefProbe") != nullptr,
                   "#3944: x64 should expose the unified exact numeric by-reference export");
#else
            expect(GetProcAddress(loaded_module, "CopperfinDeclaredDllNumericByRefProbe") == nullptr,
                   "#3944: Win32 fixture should not hide stdcall decoration behind an exact alias");
            expect(GetProcAddress(loaded_module, "_CopperfinDeclaredDllNumericByRefProbe@20") != nullptr ||
                       GetProcAddress(loaded_module, "CopperfinDeclaredDllNumericByRefProbe@20") != nullptr,
                   "#3944: Win32 should resolve the five pointer slots through a stdcall @20 export");
#endif
        }
    }

    const fs::path stack_path = temp_root / "numeric_byref_stack_frugal.prg";
    write_text(
        stack_path,
        "PUBLIC nHandled\n"
        "nHandled = 0\n" +
        declarations + initialize_values +
        "nReset = NumericByRefReset()\n"
        "nAttempts = 0\n"
        "ON ERROR DO HandleRepeatedNumericByRefError\n"
        "DO WHILE nAttempts < 512\n"
        "nAttempts = nAttempts + 1\n"
        "nUnexpected = NumericByRef(1, @nLong, @nInteger64, @nSingle, @nDouble)\n"
        "ENDDO\n"
        "ON ERROR\n"
        "nNativeEntries = NumericByRefCount()\n"
        "RETURN\n"
        "PROCEDURE HandleRepeatedNumericByRefError\n"
        "nHandled = nHandled + 1\n"
        "RETURN\n"
        "ENDPROC\n");
    {
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(stack_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "#3944: repeated numeric by-reference rejections should remain stack-frugal: " + state.message);
        const auto attempts = state.globals.find("nattempts");
        const auto handled = state.globals.find("nhandled");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(attempts != state.globals.end() &&
                   copperfin::runtime::format_value(attempts->second) == "512",
               "#3944: repeated rejection loop should complete all iterations");
        expect(handled != state.globals.end() &&
                   copperfin::runtime::format_value(handled->second) == "512",
               "#3944: repeated rejection loop should unwind every recoverable handler frame");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3944: repeated rejection loop must never enter native code");
    }

    struct MissingReferenceCase {
        std::string label;
        std::string invocation_arguments;
        std::size_t position;
        bool pseudo_locale = false;
    };
    const std::vector<MissingReferenceCase> cases{
        {"integer_literal", "1, @nLong, @nInteger64, @nSingle, @nDouble", 1U},
        {"long_variable", "@nInteger, nLong, @nInteger64, @nSingle, @nDouble", 2U},
        {"integer64_variable", "@nInteger, @nLong, nInteger64, @nSingle, @nDouble", 3U},
        {"single_variable", "@nInteger, @nLong, @nInteger64, nSingle, @nDouble", 4U},
        {"double_variable", "@nInteger, @nLong, @nInteger64, @nSingle, nDouble", 5U, true}
    };

    for (const MissingReferenceCase &missing_case : cases) {
        set_env_value("COPPERFIN_LOCALE", missing_case.pseudo_locale ? "qps-ploc" : "en-US", true);
        const fs::path script_path = temp_root / (missing_case.label + ".prg");
        write_text(
            script_path,
            "PUBLIC nCapturedCode\n"
            "PUBLIC cCapturedMessage\n"
            "nCapturedCode = 0\n"
            "cCapturedMessage = ''\n" +
            declarations + initialize_values +
            "nReset = NumericByRefReset()\n"
            "ON ERROR DO HandleNumericByRefError\n"
            "nUnexpected = NumericByRef(" + missing_case.invocation_arguments + ")\n"
            "ON ERROR\n"
            "nNativeEntries = NumericByRefCount()\n"
            "RETURN\n"
            "PROCEDURE HandleNumericByRefError\n"
            "nCapturedCode = ERROR()\n"
            "cCapturedMessage = MESSAGE()\n"
            "RETURN\n"
            "ENDPROC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "#3944: recoverable " + missing_case.label + " rejection should complete: " + state.message);

        const auto captured_code = state.globals.find("ncapturedcode");
        const auto captured_message = state.globals.find("ccapturedmessage");
        const auto native_entries = state.globals.find("nnativeentries");
        expect(captured_code != state.globals.end() &&
                   copperfin::runtime::format_value(captured_code->second) == "11",
               "#3944: " + missing_case.label + " should report VFP-compatible Error 11");
        expect(native_entries != state.globals.end() &&
                   copperfin::runtime::format_value(native_entries->second) == "0",
               "#3944: " + missing_case.label + " must not enter native code");

        const std::string message = captured_message == state.globals.end()
            ? std::string{}
            : copperfin::runtime::format_value(captured_message->second);
        const std::string position = std::to_string(missing_case.position);
        if (missing_case.pseudo_locale) {
            expect(message.find("[!! ") == 0U &&
                       message.find("NumericByRef") != std::string::npos &&
                       message.find(position) != std::string::npos &&
                       message.find("is declared numeric by reference") == std::string::npos,
                   "#3944: qps-ploc should pseudo-localize the rejection while preserving alias and position");
        } else {
            expect(message ==
                       "Native DLL function NumericByRef argument " + position +
                           " is declared numeric by reference and requires a call-site @ variable.",
                   "#3944: " + missing_case.label + " should report the actionable localized rejection");
        }
        expect(state.globals.find("nunexpected") == state.globals.end(),
               "#3944: " + missing_case.label + " should reject before assigning a native result");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.declare_dll" &&
                          event.detail == "NumericByRef IN " + module;
               }),
               "#3944: " + missing_case.label + " should preserve the invariant source DECLARE event");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto &event) {
                   return event.category == "runtime.error" && event.detail == message;
               }),
               "#3944: " + missing_case.label + " should preserve the invariant runtime.error category");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "runtime.error_handler";
               }),
               "#3944: " + missing_case.label + " should dispatch the recoverable error handler");
    }

    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    expect(GetModuleHandleW(fixture_copy.wstring().c_str()) == nullptr,
           "#3944: numeric by-reference sessions should release every controlled module reference");
    fs::remove_all(temp_root, ignored);
#endif
}


}
