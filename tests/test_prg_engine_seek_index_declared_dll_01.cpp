#include "test_prg_engine_seek_index_support.h"

namespace copperfin::seek_index_tests
{
void test_foxtools_registration_and_call_bridge() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools.prg";
    write_text(
        main_path,
        "nLibraryPathCalls = 0\n"
        "SET LIBRARY TO library_name('Foxtools')\n"
        "cFoxTools = FoxToolVer()\n"
        "nMain = MainHwnd()\n"
        "hPid = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "nPid = CallFn(hPid)\n"
        "hLen = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen = CallFn(hLen, 'Copperfin')\n"
        "RETURN\n"
        "FUNCTION library_name\n"
        "LPARAMETERS value\n"
        "nLibraryPathCalls = nLibraryPathCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools bridge script should complete");

    const auto foxtools = state.globals.find("cfoxtools");
    const auto main = state.globals.find("nmain");
    const auto hpid = state.globals.find("hpid");
    const auto pid = state.globals.find("npid");
    const auto hlen = state.globals.find("hlen");
    const auto length = state.globals.find("nlen");
    const auto library_path_calls = state.globals.find("nlibrarypathcalls");

    expect(foxtools != state.globals.end(), "FoxToolVer() should be captured");
    expect(main != state.globals.end(), "MainHwnd() should be captured");
    expect(hpid != state.globals.end(), "RegFn32 handle should be captured");
    expect(pid != state.globals.end(), "CallFn(handle) should be captured");
    expect(hlen != state.globals.end(), "second RegFn32 handle should be captured");
    expect(length != state.globals.end(), "CallFn(string) should be captured");
    expect(library_path_calls != state.globals.end(), "SET LIBRARY should preserve the designator resolver call counter");
    if (library_path_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(library_path_calls->second) == "1",
               "SET LIBRARY should evaluate the designator UDF exactly once");
    }

    if (foxtools != state.globals.end()) {
        expect(!copperfin::runtime::format_value(foxtools->second).empty(), "FoxToolVer() should return a non-empty version string");
    }
    if (main != state.globals.end()) {
        expect(copperfin::runtime::format_value(main->second) == "1001", "MainHwnd() should expose the placeholder host window handle");
    }
    if (hpid != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid->second) == "1", "first RegFn32 call should allocate handle 1");
    }
    if (pid != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid->second) != "0", "CallFn(GetCurrentProcessId) should return a non-zero process id");
    }
    if (hlen != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen->second) == "2", "second RegFn32 call should allocate handle 2");
    }
    if (length != state.globals.end()) {
        expect(copperfin::runtime::format_value(length->second) == "9", "CallFn(lstrlenA, 'Copperfin') should return the string length");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.library"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.regfn"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.callfn"; }),
        "Foxtools bridge should emit library, registration, and call events");
    const auto regfn_getpid_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.regfn" && event.detail.find("GetCurrentProcessId") != std::string::npos &&
               event.detail.find("returns I") != std::string::npos && event.detail.find("args=void") != std::string::npos;
    });
    const auto regfn_strlen_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.regfn" && event.detail.find("lstrlenA") != std::string::npos &&
               event.detail.find("returns I") != std::string::npos && event.detail.find("args=C") != std::string::npos;
    });
    const auto callpid_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.callfn" && event.detail.find("GetCurrentProcessId#1") != std::string::npos &&
               event.detail.find("expects") != std::string::npos;
    });
    const auto callstr_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "interop.callfn" && event.detail.find("lstrlenA#2") != std::string::npos &&
               event.detail.find("expects") != std::string::npos;
    });
    expect(regfn_getpid_event != state.events.end(), "interop.regfn should include GetCurrentProcessId contract detail");
    expect(regfn_strlen_event != state.events.end(), "interop.regfn should include lstrlenA contract detail");
    expect(callpid_event != state.events.end(), "interop.callfn should include handle-scoped function contract detail for GetCurrentProcessId");
    expect(callstr_event != state.events.end(), "interop.callfn should include handle-scoped function contract detail for lstrlenA");

    fs::remove_all(temp_root, ignored);
}

void test_foxtools_registration_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools_datasession.prg";
    write_text(
        main_path,
        "SET LIBRARY TO 'Foxtools'\n"
        "hPid1 = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "SET DATASESSION TO 2\n"
        "SET LIBRARY TO 'Foxtools'\n"
        "nCrossCall = CallFn(hPid1)\n"
        "hLen2 = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen2 = CallFn(hLen2, 'AB')\n"
        "SET DATASESSION TO 1\n"
        "nPid1Back = CallFn(hPid1)\n"
        "hLen1Back = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools data-session script should complete");

    const auto hpid1 = state.globals.find("hpid1");
    const auto cross_call = state.globals.find("ncrosscall");
    const auto hlen2 = state.globals.find("hlen2");
    const auto len2 = state.globals.find("nlen2");
    const auto pid1_back = state.globals.find("npid1back");
    const auto hlen1_back = state.globals.find("hlen1back");

    expect(hpid1 != state.globals.end(), "session-1 RegFn32 handle should be captured");
    expect(cross_call != state.globals.end(), "cross-session CallFn result should be captured");
    expect(hlen2 != state.globals.end(), "session-2 RegFn32 handle should be captured");
    expect(len2 != state.globals.end(), "session-2 CallFn result should be captured");
    expect(pid1_back != state.globals.end(), "restored session-1 CallFn result should be captured");
    expect(hlen1_back != state.globals.end(), "restored session-1 RegFn32 handle should be captured");

    if (hpid1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid1->second) == "1", "the first RegFn32 handle in session 1 should be 1");
    }
    if (cross_call != state.globals.end()) {
        expect(copperfin::runtime::format_value(cross_call->second) == "-1", "CallFn should reject a RegFn32 handle from another data session");
    }
    if (hlen2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen2->second) == "1", "the first RegFn32 handle in a fresh second data session should restart at 1");
    }
    if (len2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(len2->second) == "2", "session-2 CallFn should use its own registered handle");
    }
    if (pid1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid1_back->second) != "0", "restoring session 1 should restore its RegFn32 handle lookup");
    }
    if (hlen1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen1_back->second) == "2", "restoring session 1 should restore its next RegFn32 handle allocation");
    }

    fs::remove_all(temp_root, ignored);
}

void test_declared_dll_string_byref_argument_writeback() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "declared_dll_byref.prg";
    write_text(
        main_path,
        "FUNCTION GetDeclaredDllPath\n"
        "RETURN 'kernel32.dll'\n"
        "ENDFUNC\n"
        "DECLARE INTEGER lstrcpyA(STRING @, STRING) IN GetDeclaredDllPath()\n"
        "cBuffer = SPACE(32)\n"
        "nResult = lstrcpyA(@cBuffer, 'Copperfin')\n"
        "cCopied = cBuffer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "declared DLL by-ref script should complete: " + state.message);

    const auto result = state.globals.find("nresult");
    const auto buffer = state.globals.find("cbuffer");
    const auto copied = state.globals.find("ccopied");

    expect(result != state.globals.end(), "declared DLL call result should be captured");
    expect(buffer != state.globals.end(), "declared DLL by-ref target should be captured");
    expect(copied != state.globals.end(), "copied by-ref value should be captured");

    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) != "0", "lstrcpyA should return a non-null destination pointer");
    }
    if (buffer != state.globals.end()) {
        expect(copperfin::runtime::format_value(buffer->second) == "Copperfin", "STRING @ arguments should write back the mutated buffer");
    }
    if (copied != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied->second) == "Copperfin", "subsequent reads should observe the by-ref writeback");
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("lstrcpyA") != std::string::npos;
    });
    expect(declare_event != state.events.end(), "declared DLL by-ref script should emit the DECLARE event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_explicit_relative_child_path() {
#if defined(_WIN32) && defined(COPPERFIN_DECLARED_DLL_FIXTURE_NAME)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_relative";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    expect(!fixture_source.empty(), "#3921: test executable path should be available");
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3921: explicit-relative DECLARE fixture should copy beside the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_relative.prg";
    write_text(
        main_path,
        "DECLARE INTEGER CopperfinDeclaredDllFixtureValue() IN 'native/" +
            fixture_name.string() + "'\n"
        "nValue = CopperfinDeclaredDllFixtureValue()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3921: explicit-relative DECLARE script should complete: " + state.message);

    const auto value = state.globals.find("nvalue");
    expect(value != state.globals.end(), "#3921: explicit-relative DECLARE result should be captured");
    if (value != state.globals.end()) {
        expect(copperfin::runtime::format_value(value->second) == "3921",
               "#3921: explicit-relative DECLARE should invoke the child DLL");
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllFixtureValue") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3921: explicit-relative DECLARE should emit the stable machine event");

    const std::string missing_library_name = "copperfin_missing_declared_dll_3921.dll";
    const fs::path missing_path = temp_root / "declared_dll_missing.prg";
    write_text(
        missing_path,
        "DECLARE INTEGER MissingExport() IN '" + missing_library_name + "'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession missing_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(missing_path.string(), temp_root.string()));
    const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!missing_state.completed, "#3921: a missing parentless DLL should retain the load failure");
    expect(missing_state.message.find(missing_library_name) != std::string::npos,
           "#3921: a missing parentless DLL diagnostic should retain its invariant designator");
    expect(missing_state.message.find(temp_root.string()) == std::string::npos,
           "#3921: a missing parentless DLL should not be rewritten under the PRG working directory");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_double_arguments_follow_x64_abi() {
#if defined(_WIN64)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_double_abi";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::create_directories(fixture_copy.parent_path());
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3895: typed x64 DECLARE fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_double_abi.prg";
    write_text(
        main_path,
        "DECLARE DOUBLE CopperfinDeclaredDllFraction IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE STRING CopperfinDeclaredDllText IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllInt64 IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllInt64BeyondDouble IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllInt64Echo IN 'native/" + fixture_name.string() + "' INTEGER64 value\n"
        "DECLARE LONG CopperfinDeclaredDllInt64ByRef IN 'native/" + fixture_name.string() + "' INTEGER64 @ value\n"
        "DECLARE DOUBLE CopperfinDeclaredDllOneSlot IN 'native/" + fixture_name.string() + "' DOUBLE first\n"
        "DECLARE DOUBLE CopperfinDeclaredDllMultiply IN 'native/" + fixture_name.string() + "' DOUBLE left, DOUBLE right\n"
        "DECLARE DOUBLE CopperfinDeclaredDllAffine IN 'native/" + fixture_name.string() + "' DOUBLE left, DOUBLE right\n"
        "DECLARE DOUBLE CopperfinDeclaredDllScale IN 'native/" + fixture_name.string() + "' DOUBLE value, INTEGER factor\n"
        "DECLARE DOUBLE CopperfinDeclaredDllThreeSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third\n"
        "DECLARE DOUBLE CopperfinDeclaredDllFourSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllFiveSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSixSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth, INTEGER sixth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSevenSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth, INTEGER sixth, DOUBLE seventh\n"
        "DECLARE DOUBLE CopperfinDeclaredDllEightSlots IN 'native/" + fixture_name.string() + "' DOUBLE first, INTEGER second, DOUBLE third, INTEGER fourth, DOUBLE fifth, INTEGER sixth, DOUBLE seventh, INTEGER eighth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSplit IN 'native/" + fixture_name.string() + "' DOUBLE value, DOUBLE @ whole\n"
        "DECLARE INTEGER CopperfinDeclaredDllDecrement IN 'native/" + fixture_name.string() + "' INTEGER @ value\n"
        "nWhole = 0\n"
        "nCounter = 0\n"
        "nConstant = CopperfinDeclaredDllFraction()\n"
        "cText = CopperfinDeclaredDllText()\n"
        "nInt64 = CopperfinDeclaredDllInt64()\n"
        "nExactInt64 = CopperfinDeclaredDllInt64BeyondDouble()\n"
        "nEchoInt64 = CopperfinDeclaredDllInt64Echo(nExactInt64)\n"
        "nByRefInt64 = 0\n"
        "nByRefResult = CopperfinDeclaredDllInt64ByRef(@nByRefInt64)\n"
        "nOneSlot = CopperfinDeclaredDllOneSlot(1.0)\n"
        "nProduct = CopperfinDeclaredDllMultiply(2.5, 4.0)\n"
        "nAffine = CopperfinDeclaredDllAffine(2.5, 4.0)\n"
        "nScaled = CopperfinDeclaredDllScale(1.5, 8)\n"
        "nThreeSlots = CopperfinDeclaredDllThreeSlots(1.0, 2, 3.0)\n"
        "nFourSlots = CopperfinDeclaredDllFourSlots(1.0, 2, 3.0, 4)\n"
        "nFiveSlots = CopperfinDeclaredDllFiveSlots(1.0, 2, 3.0, 4, 5.0)\n"
        "nSixSlots = CopperfinDeclaredDllSixSlots(1.0, 2, 3.0, 4, 5.0, 6)\n"
        "nSevenSlots = CopperfinDeclaredDllSevenSlots(1.0, 2, 3.0, 4, 5.0, 6, 7.0)\n"
        "nEightSlots = CopperfinDeclaredDllEightSlots(1.0, 2, 3.0, 4, 5.0, 6, 7.0, 8)\n"
        "nFraction = CopperfinDeclaredDllSplit(3.75, @nWhole)\n"
        "nDecremented = CopperfinDeclaredDllDecrement(@nCounter)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "declared DLL double-argument script should complete: " + state.message);

    const auto constant = state.globals.find("nconstant");
    const auto text = state.globals.find("ctext");
    const auto int64_value = state.globals.find("nint64");
    const auto exact_int64 = state.globals.find("nexactint64");
    const auto echo_int64 = state.globals.find("nechoint64");
    const auto byref_int64 = state.globals.find("nbyrefint64");
    const auto byref_result = state.globals.find("nbyrefresult");
    const auto one_slot = state.globals.find("noneslot");
    const auto product = state.globals.find("nproduct");
    const auto affine = state.globals.find("naffine");
    const auto scaled = state.globals.find("nscaled");
    const auto three_slots = state.globals.find("nthreeslots");
    const auto four_slots = state.globals.find("nfourslots");
    const auto five_slots = state.globals.find("nfiveslots");
    const auto six_slots = state.globals.find("nsixslots");
    const auto seven_slots = state.globals.find("nsevenslots");
    const auto eight_slots = state.globals.find("neightslots");
    const auto fraction = state.globals.find("nfraction");
    const auto whole = state.globals.find("nwhole");
    const auto decremented = state.globals.find("ndecremented");
    const auto counter = state.globals.find("ncounter");
    expect(constant != state.globals.end(), "zero-argument DOUBLE fixture result should be captured");
    expect(text != state.globals.end(), "pointer-shaped STRING fixture result should be captured");
    expect(int64_value != state.globals.end(), "signed 64-bit fixture result should be captured");
    expect(exact_int64 != state.globals.end(), "64-bit result beyond binary64 precision should be captured");
    expect(echo_int64 != state.globals.end(), "64-bit argument beyond binary64 precision should be captured");
    expect(byref_int64 != state.globals.end(), "64-bit by-reference output should be captured");
    expect(byref_result != state.globals.end(), "64-bit by-reference return should be captured");
    expect(one_slot != state.globals.end(), "one-slot fixture result should be captured");
    expect(product != state.globals.end(), "two-DOUBLE fixture result should be captured");
    expect(affine != state.globals.end(), "ordered DOUBLE fixture result should be captured");
    expect(scaled != state.globals.end(), "mixed DOUBLE/INTEGER fixture result should be captured");
    expect(three_slots != state.globals.end(), "three-slot fixture result should be captured");
    expect(four_slots != state.globals.end(), "four-slot fixture result should be captured");
    expect(five_slots != state.globals.end(), "five-slot fixture result should be captured");
    expect(six_slots != state.globals.end(), "six-slot fixture result should be captured");
    expect(seven_slots != state.globals.end(), "seven-slot fixture result should be captured");
    expect(eight_slots != state.globals.end(), "eight-slot fixture result should be captured");
    expect(fraction != state.globals.end(), "DOUBLE @ fixture fractional result should be captured");
    expect(whole != state.globals.end(), "DOUBLE @ fixture output should be captured");
    expect(decremented != state.globals.end(), "signed 32-bit return should be captured");
    expect(counter != state.globals.end(), "INTEGER @ output should be captured");
    if (constant != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(constant->second);
        expect(actual == "0.625", "zero-argument DOUBLE return should preserve fractional precision; actual=" + actual);
    }
    if (text != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(text->second);
        expect(actual == "copperfin", "pointer-shaped STRING returns should preserve the native address; actual=" + actual);
    }
    if (int64_value != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(int64_value->second);
        expect(actual == "-4294967297", "signed 64-bit native returns should retain their sign and width; actual=" + actual);
    }
    if (exact_int64 != state.globals.end()) {
        expect(exact_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(exact_int64->second) == "9007199254740993",
               "64-bit native returns should remain exact beyond binary64 precision");
    }
    if (echo_int64 != state.globals.end()) {
        expect(echo_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(echo_int64->second) == "9007199254740993",
               "64-bit native arguments should remain exact beyond binary64 precision");
    }
    if (byref_int64 != state.globals.end()) {
        expect(byref_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
                   copperfin::runtime::format_value(byref_int64->second) == "9007199254740993",
               "64-bit native by-reference writeback should remain exact beyond binary64 precision");
    }
    if (byref_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(byref_result->second) == "1",
               "64-bit native by-reference fixture should return its success sentinel");
    }
    if (one_slot != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(one_slot->second);
        expect(actual == "1", "one-argument typed dispatch should preserve its DOUBLE slot; actual=" + actual);
    }
    if (product != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(product->second);
        expect(actual == "10", "two DOUBLE arguments should reach XMM0 and XMM1; actual=" + actual);
    }
    if (affine != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(affine->second);
        expect(actual == "29", "DOUBLE arguments should retain their declared positions; actual=" + actual);
    }
    if (scaled != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(scaled->second);
        expect(actual == "12", "mixed DOUBLE/INTEGER arguments should preserve x64 register classes; actual=" + actual);
    }
    const auto expect_slot_sum = [&](const auto iterator, const std::string &expected, const std::string &label) {
        if (iterator == state.globals.end()) {
            return;
        }
        const std::string actual = copperfin::runtime::format_value(iterator->second);
        expect(actual == expected, label + "; actual=" + actual);
    };
    expect_slot_sum(three_slots, "6", "three-argument typed dispatch should preserve all slots");
    expect_slot_sum(four_slots, "10", "four-argument typed dispatch should preserve all register slots");
    expect_slot_sum(five_slots, "15", "five-argument typed dispatch should cross into stack slots");
    expect_slot_sum(six_slots, "21", "six-argument typed dispatch should preserve register and stack slots");
    expect_slot_sum(seven_slots, "28", "seven-argument typed dispatch should preserve register and stack slots");
    if (eight_slots != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(eight_slots->second);
        expect(actual == "36", "all eight x64 argument slots should preserve declared register and stack classes; actual=" + actual);
    }
    if (fraction != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(fraction->second);
        expect(actual == "0.75", "DOUBLE return values should preserve fractional precision; actual=" + actual);
    }
    if (whole != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(whole->second);
        expect(actual == "3", "DOUBLE @ arguments should write native changes back to the caller; actual=" + actual);
    }
    if (decremented != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(decremented->second);
        expect(actual == "-1", "signed 32-bit native returns should remain negative; actual=" + actual);
    }
    if (counter != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(counter->second);
        expect(actual == "-1", "INTEGER @ arguments should use signed 32-bit backing storage; actual=" + actual);
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllEightSlots") != std::string::npos;
    });
    expect(declare_event != state.events.end(), "typed native declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_long_uses_vfp_32_bit_width() {
#if defined(_WIN64)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_long_width";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3932: controlled LONG-width fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_long_width.prg";
    write_text(
        main_path,
        "DECLARE LONG CopperfinDeclaredDllLongWidth IN 'native/" + fixture_name.string() +
            "' DOUBLE multiplier, LONG input, LONG @ output\n"
        "nOutput = 42\n"
        "nReturn = CopperfinDeclaredDllLongWidth(1.5, 42, @nOutput)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3932: LONG-width fixture should complete: " + state.message);

    const auto returned = state.globals.find("nreturn");
    const auto output = state.globals.find("noutput");
    expect(returned != state.globals.end(), "#3932: signed LONG return should be captured");
    expect(output != state.globals.end(), "#3932: LONG by-reference output should be captured");
    if (returned != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(returned->second);
        expect(actual == "-2147483000", "#3932: LONG returns should narrow as signed 32-bit values; actual=" + actual);
    }
    if (output != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(output->second);
        expect(actual == "-123456789", "#3932: LONG @ storage should write back as signed 32-bit; actual=" + actual);
    }

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_single_uses_vfp_32_bit_float_width() {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_single_width";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3933: controlled SINGLE fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_single_width.prg";
    write_text(
        main_path,
        "DECLARE SINGLE CopperfinDeclaredDllSingleConstant IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE SINGLE CopperfinDeclaredDllSingleMixed IN 'native/" + fixture_name.string() +
            "' INTEGER first, SINGLE second, DOUBLE third, SINGLE fourth, SINGLE fifth\n"
        "DECLARE DOUBLE CopperfinDeclaredDllSingleToDouble IN 'native/" + fixture_name.string() +
            "' SINGLE value, DOUBLE multiplier\n"
        "DECLARE SINGLE CopperfinDeclaredDllSingleSlots IN 'native/" + fixture_name.string() +
            "' SINGLE first, SINGLE second, SINGLE third, SINGLE fourth, SINGLE fifth, SINGLE sixth, SINGLE seventh, SINGLE eighth\n"
        "DECLARE SINGLE CopperfinDeclaredDllSingleSplit IN 'native/" + fixture_name.string() +
            "' SINGLE value, SINGLE @ whole\n"
        "nWhole = 0\n"
        "nConstant = CopperfinDeclaredDllSingleConstant()\n"
        "nMixed = CopperfinDeclaredDllSingleMixed(1, 2.25, 3.5, 4.25, 5.0)\n"
        "nDouble = CopperfinDeclaredDllSingleToDouble(2.25, 1.5)\n"
        "nSlots = CopperfinDeclaredDllSingleSlots(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)\n"
        "nFraction = CopperfinDeclaredDllSingleSplit(3.75, @nWhole)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3933: SINGLE fixture should complete: " + state.message);

    const auto expect_numeric = [&](const std::string &name,
                                    const std::string &expected,
                                    const std::string &label)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), label + " should be captured");
        if (value != state.globals.end())
        {
            const std::string actual = copperfin::runtime::format_value(value->second);
            expect(actual == expected, label + "; actual=" + actual);
        }
    };
    expect_numeric("nconstant", "0.625", "#3933: zero-argument SINGLE return");
    expect_numeric("nmixed", "16", "#3933: mixed INTEGER/SINGLE/DOUBLE register and stack call");
    expect_numeric("ndouble", "3.375", "#3933: DOUBLE return through a SINGLE signature");
    expect_numeric("nslots", "36", "#3933: eight-position SINGLE call");
    expect_numeric("nfraction", "0.75", "#3933: SINGLE return with by-reference input");
    expect_numeric("nwhole", "3", "#3933: SINGLE @ writeback");

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllSingleSlots") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3933: SINGLE declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32_uses_typed_stdcall_slots() {
#if defined(_WIN32) && !defined(_WIN64)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32_typed";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3940: controlled Win32 typed fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_win32_typed.prg";
    write_text(
        main_path,
        "DECLARE DOUBLE CopperfinDeclaredDllX86Mixed IN 'native/" + fixture_name.string() +
            "' AS X86MixedAlias LONG first, DOUBLE second, INTEGER64 third, INTEGER fourth\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllX86Int64 IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllX86Int64BeyondDouble IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE INTEGER64 CopperfinDeclaredDllX86Int64Echo IN 'native/" + fixture_name.string() + "' INTEGER64 value\n"
        "DECLARE LONG CopperfinDeclaredDllX86Int64ByRef IN 'native/" + fixture_name.string() + "' INTEGER64 @ value\n"
        "DECLARE DOUBLE CopperfinDeclaredDllX86Split IN 'native/" + fixture_name.string() +
            "' DOUBLE value, DOUBLE @ whole\n"
        "DECLARE STRING CopperfinDeclaredDllX86Text IN 'native/" + fixture_name.string() + "'\n"
        "DECLARE LONG CopperfinDeclaredDllX86Eight IN 'native/" + fixture_name.string() +
            "' INTEGER first, INTEGER second, INTEGER third, INTEGER fourth, INTEGER fifth, INTEGER sixth, INTEGER seventh, INTEGER eighth\n"
        "DECLARE LONG CopperfinDeclaredDllX86NumericByRef IN 'native/" + fixture_name.string() +
            "' LONG @ longValue, INTEGER64 @ integer64Value\n"
        "nWhole = 0\n"
        "nLongOut = 0\n"
        "nInteger64Out = 0\n"
        "nMixed = X86MixedAlias(1, 2.5, 4294967297, 4)\n"
        "nInt64 = CopperfinDeclaredDllX86Int64()\n"
        "nExactInt64 = CopperfinDeclaredDllX86Int64BeyondDouble()\n"
        "nEchoInt64 = CopperfinDeclaredDllX86Int64Echo(nExactInt64)\n"
        "nByRefInt64 = 0\n"
        "nByRefResult = CopperfinDeclaredDllX86Int64ByRef(@nByRefInt64)\n"
        "nFraction = CopperfinDeclaredDllX86Split(3.75, @nWhole)\n"
        "cText = CopperfinDeclaredDllX86Text()\n"
        "nEight = CopperfinDeclaredDllX86Eight(1, 2, 3, 4, 5, 6, 7, 8)\n"
        "nNumericByRefResult = CopperfinDeclaredDllX86NumericByRef(@nLongOut, @nInteger64Out)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3940: Win32 typed fixture should complete: " + state.message);

    const auto expect_value = [&](const std::string &name,
                                  const std::string &expected,
                                  const std::string &label)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), label + " should be captured");
        if (value != state.globals.end())
        {
            const std::string actual = copperfin::runtime::format_value(value->second);
            expect(actual == expected, label + "; actual=" + actual);
        }
    };
    const auto mixed = state.globals.find("nmixed");
    expect(mixed != state.globals.end(),
           "#3940: mixed LONG/DOUBLE/INTEGER64/INTEGER call should be captured");
    if (mixed != state.globals.end())
    {
        expect(mixed->second.kind == copperfin::runtime::PrgValueKind::number &&
                   mixed->second.number_value == 4294967304.5,
               "#3940: mixed LONG/DOUBLE/INTEGER64/INTEGER call should preserve its exact numeric payload; actual=" +
                   std::to_string(mixed->second.number_value));
    }
    expect_value("nint64", "-4294967297", "#3940: signed 64-bit return");
    const auto exact_int64 = state.globals.find("nexactint64");
    expect(exact_int64 != state.globals.end() &&
               exact_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
               copperfin::runtime::format_value(exact_int64->second) == "9007199254740993",
           "#3934: Win32 signed 64-bit return should remain exact beyond binary64 precision");
    const auto echo_int64 = state.globals.find("nechoint64");
    expect(echo_int64 != state.globals.end() &&
               echo_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
               copperfin::runtime::format_value(echo_int64->second) == "9007199254740993",
           "#3934: Win32 signed 64-bit argument should remain exact beyond binary64 precision");
    const auto byref_int64 = state.globals.find("nbyrefint64");
    expect(byref_int64 != state.globals.end() &&
               byref_int64->second.kind == copperfin::runtime::PrgValueKind::int64 &&
               copperfin::runtime::format_value(byref_int64->second) == "9007199254740993",
           "#3934: Win32 signed 64-bit by-reference writeback should remain exact beyond binary64 precision");
    expect_value("nbyrefresult", "1", "#3934: Win32 signed 64-bit by-reference return");
    expect_value("nfraction", "0.75", "#3940: DOUBLE return with by-reference input");
    expect_value("nwhole", "3", "#3940: DOUBLE @ writeback");
    expect_value("ctext", "copperfin-x86", "#3940: pointer-shaped STRING return");
    expect_value("neight", "36", "#3940: signed LONG return across eight stdcall stack positions");
    expect_value("nnumericbyrefresult", "-7", "#3940: signed LONG return from numeric by-reference call");
    expect_value("nlongout", "-123456789", "#3940: signed LONG @ writeback");
    expect_value("ninteger64out", "-4294967297", "#3940: signed INTEGER64 @ writeback");

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("X86MixedAlias") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3940: Win32 typed declarations should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}

void test_declared_dll_win32_resolves_no_underscore_stdcall_export() {
#if defined(_MSC_VER) && defined(_M_IX86)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_declared_dll_win32_name_at_n";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "native");

    const fs::path fixture_name = COPPERFIN_DECLARED_DLL_FIXTURE_NAME;
    const fs::path fixture_source = declared_dll_fixture_source_path();
    const fs::path fixture_copy = temp_root / "native" / fixture_name;
    fs::copy_file(fixture_source, fixture_copy, fs::copy_options::overwrite_existing, ignored);
    expect(!ignored && fs::exists(fixture_copy),
           "#3941: no-underscore stdcall fixture should copy under the PRG working directory");

    const fs::path main_path = temp_root / "declared_dll_win32_name_at_n.prg";
    write_text(
        main_path,
        "DECLARE LONG CopperfinDeclaredDllNoUnderscore IN 'native/" + fixture_name.string() + "' LONG value\n"
        "nResult = CopperfinDeclaredDllNoUnderscore(41)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3941: Name@N-only stdcall fixture should complete: " + state.message);

    const auto result = state.globals.find("nresult");
    expect(result != state.globals.end(), "#3941: Name@N-only result should be captured");
    if (result != state.globals.end())
    {
        expect(copperfin::runtime::format_value(result->second) == "42",
               "#3941: Name@N-only stdcall export should resolve through the final decorated probe");
    }

    const auto declare_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.declare_dll" &&
               event.detail.find("CopperfinDeclaredDllNoUnderscore") != std::string::npos;
    });
    expect(declare_event != state.events.end(),
           "#3941: Name@N-only declaration should retain the invariant runtime.declare_dll event");

    fs::remove_all(temp_root, ignored);
#endif
}


}
