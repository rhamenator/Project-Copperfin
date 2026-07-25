#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_caption_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_caption";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_caption.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "lHas = PEMSTATUS(oHeader, 'Caption', 1)\n"
            "lReadOnly = PEMSTATUS(oHeader, 'Caption', 5)\n"
            "cDefault = oHeader.Caption\n"
            "cGetPem = GETPEM(oHeader, 'Caption')\n"
            "oHeader.Caption = 'Explicit Header'\n"
            "cDirect = oHeader.Caption\n"
            "lSetPem = SETPEM(oHeader, 'Caption', 'Set Header')\n"
            "cAfterSetPem = oHeader.Caption\n"
            "lPutPem = PUTPEM(oHeader, 'Caption', 'Put Header')\n"
            "cAfterPutPem = GETPEM(oHeader, 'Caption')\n"
            "lAddProperty = ADDPROPERTY(oHeader, 'Caption', 'Shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oHeader, 'Caption')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oHeader, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'CAPTION'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.Caption = 'Grid Header'\n"
            "cGridCaption = oGrid.Columns(1).Header.Caption\n"
            "oDerived = CREATEOBJECT('DerivedHeaderCaption')\n"
            "cDerivedCaption = oDerived.Caption\n"
            "RETURN\n"
            "DEFINE CLASS DerivedHeaderCaption AS Header\n"
            "    Caption = 'Derived Header'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header Caption script should complete: ") + state.message +
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
        check("lreadonly", "false");
        check("cdefault", "");
        check("cgetpem", "");
        check("cdirect", "Explicit Header");
        check("lsetpem", "true");
        check("caftersetpem", "Set Header");
        check("lputpem", "true");
        check("cafterputpem", "Put Header");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cgridcaption", "Grid Header");
        check("cderivedcaption", "Derived Header");

        fs::remove_all(temp_root, ignored);
    }
}
