// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_for_each_iterates_array_elements() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_array";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "DIMENSION fruits(3)\n"
        "fruits(1) = 'apple'\n"
        "fruits(2) = 'banana'\n"
        "fruits(3) = 'cherry'\n"
        "result = ''\n"
        "FOR EACH elem IN fruits\n"
        "    result = result + elem + ','\n"
        "ENDFOR\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH over array should complete");
    const auto it = state.globals.find("result");
    expect(it != state.globals.end(), "result should be set after FOR EACH");
    expect(it->second.string_value == "apple,banana,cherry,", "FOR EACH should iterate all array elements");
    fs::remove_all(tmp, ign);
}

void test_for_each_single_element_expression() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_scalar";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "result = ''\n"
        "FOR EACH item IN 'hello'\n"
        "    result = item\n"
        "ENDFOR\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH over scalar should complete");
    const auto it = state.globals.find("result");
    expect(it != state.globals.end(), "result should be set");
    expect(it->second.string_value == "hello", "FOR EACH scalar treats expression as single element");
    fs::remove_all(tmp, ign);
}

void test_for_each_iterates_native_collection_direct_and_member_path() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_native_collection";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "oHost = CREATEOBJECT('HostBox')\n"
        "oItems = oHost.oItems\n"
        "cDirect = ''\n"
        "cMember = ''\n"
        "cEmpty = 'start'\n"
        "FOR EACH oItem IN oItems\n"
        "    cDirect = cDirect + oItem + ','\n"
        "ENDFOR\n"
        "FOR EACH oItem IN oHost.oItems\n"
        "    cMember = cMember + oItem + ','\n"
        "ENDFOR\n"
        "FOR EACH oItem IN oHost.oEmpty\n"
        "    cEmpty = cEmpty + '!'\n"
        "ENDFOR\n"
        "RETURN\n"
        "DEFINE CLASS WorkerCollection AS Collection\n"
        "ENDDEFINE\n"
        "DEFINE CLASS HostBox AS Custom\n"
        "    oItems = .NULL.\n"
        "    oEmpty = .NULL.\n"
        "    PROCEDURE Init\n"
        "        THIS.oItems = CREATEOBJECT('WorkerCollection')\n"
        "        THIS.oItems.Add('alpha')\n"
        "        THIS.oItems.Add('beta', 'second')\n"
        "        THIS.oEmpty = CREATEOBJECT('WorkerCollection')\n"
        "        RETURN\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH over native Collection should complete");

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be set");
        if (it != state.globals.end())
        {
            expect(it->second.string_value == expected,
                   name + " expected '" + expected + "' got '" + it->second.string_value + "'");
        }
    };

    check("cdirect", "alpha,beta,");
    check("cmember", "alpha,beta,");
    check("cempty", "start");
    fs::remove_all(tmp, ign);
}

void test_for_each_foxobject_qualifier_tolerates_direct_and_member_path_collections() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_foxobject_collection";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "oHost = CREATEOBJECT('HostBox')\n"
        "oItems = oHost.oItems\n"
        "cDirect = ''\n"
        "cMember = ''\n"
        "FOR EACH oItem IN oItems FOXOBJECT\n"
        "    cDirect = cDirect + oItem.cTag + ','\n"
        "ENDFOR\n"
        "FOR EACH oItem IN oHost.oItems FOXOBJECT\n"
        "    cMember = cMember + oItem.cTag + ','\n"
        "ENDFOR\n"
        "RETURN\n"
        "DEFINE CLASS WorkerCollection AS Collection\n"
        "ENDDEFINE\n"
        "DEFINE CLASS TagChild AS Custom\n"
        "    cTag = ''\n"
        "ENDDEFINE\n"
        "DEFINE CLASS HostBox AS Custom\n"
        "    oItems = .NULL.\n"
        "    PROCEDURE Init\n"
        "        LOCAL oFirst, oSecond\n"
        "        THIS.oItems = CREATEOBJECT('WorkerCollection')\n"
        "        oFirst = CREATEOBJECT('TagChild')\n"
        "        oFirst.cTag = 'alpha'\n"
        "        oSecond = CREATEOBJECT('TagChild')\n"
        "        oSecond.cTag = 'beta'\n"
        "        THIS.oItems.Add(oFirst)\n"
        "        THIS.oItems.Add(oSecond)\n"
        "        RETURN\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH ... FOXOBJECT over native Collection should complete");

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be set");
        if (it != state.globals.end())
        {
            expect(it->second.string_value == expected,
                   name + " expected '" + expected + "' got '" + it->second.string_value + "'");
        }
    };

    check("cdirect", "alpha,beta,");
    check("cmember", "alpha,beta,");
    fs::remove_all(tmp, ign);
}

}  // namespace cf_test_prg_engine_control_flow
