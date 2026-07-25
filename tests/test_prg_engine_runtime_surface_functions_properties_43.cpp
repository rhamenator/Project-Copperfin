#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_wordwrap_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_wordwrap";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_wordwrap.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "lHas = PEMSTATUS(oHeader, 'WordWrap', 1)\n"
            "lDefault = oHeader.WordWrap\n"
            "lGetPem = GETPEM(oHeader, 'WordWrap')\n"
            "lReadOnly = PEMSTATUS(oHeader, 'WordWrap', 5)\n"
            "oHeader.WordWrap = .T.\n"
            "lDirect = oHeader.WordWrap\n"
            "lSetPem = SETPEM(oHeader, 'WordWrap', .F.)\n"
            "lSetPemValue = oHeader.WordWrap\n"
            "lPutPem = PUTPEM(oHeader, 'WordWrap', .T.)\n"
            "lPutPemValue = oHeader.WordWrap\n"
            "lAddProperty = ADDPROPERTY(oHeader, 'WordWrap', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oHeader, 'WordWrap')\n"
            "oDerived = CREATEOBJECT('DerivedHeaderWordWrap')\n"
            "lDerived = oDerived.WordWrap\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oDerived, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'WORDWRAP'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DerivedHeaderWordWrap AS Header\n"
            "    WordWrap = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header WordWrap script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("lhas", "true");
        check("ldefault", "false");
        check("lgetpem", "false");
        check("lreadonly", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lderived", "true");
        check("lmember", "true");
        expect(state.ole_objects.size() == 2U,
               "native Header WordWrap coverage should register base and derived Header objects");

        fs::remove_all(temp_root, ignored);
    }
}
