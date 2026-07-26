#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_mouseicon_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_mouseicon";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_mouseicon.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'MouseIcon', 1)\n"
            "lColumnHas = PEMSTATUS(oColumn, 'MouseIcon', 1)\n"
            "cHeaderDefault = oHeader.MouseIcon\n"
            "cColumnDefault = oColumn.MouseIcon\n"
            "oHeader.MouseIcon = 'header.ico'\n"
            "cHeaderDirect = oHeader.MouseIcon\n"
            "lColumnSetPem = SETPEM(oColumn, 'MouseIcon', 'column.cur')\n"
            "cColumnSetPem = GETPEM(oColumn, 'MouseIcon')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'MouseIcon', 'header.ani')\n"
            "cHeaderPutPem = GETPEM(oHeader, 'MouseIcon')\n"
            "lHeaderAdd = ADDPROPERTY(oHeader, 'MouseIcon', 'shadow')\n"
            "lHeaderRemove = REMOVEPROPERTY(oHeader, 'MouseIcon')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'MouseIcon', 'shadow')\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'MouseIcon')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'MOUSEICON'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.MouseIcon = 'grid.cur'\n"
            "cGridHeaderIcon = oGrid.Columns(1).Header.MouseIcon\n"
            "oDerivedHeader = CREATEOBJECT('DerivedIconHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedIconColumn')\n"
            "cDerivedHeaderIcon = oDerivedHeader.MouseIcon\n"
            "cDerivedColumnIcon = oDerivedColumn.MouseIcon\n"
            "RETURN\n"
            "DEFINE CLASS DerivedIconHeader AS Header\n"
            "    MouseIcon = 'derived-header.ico'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedIconColumn AS Column\n"
            "    MouseIcon = 'derived-column.cur'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column MouseIcon script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end()) {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("lheaderhas", "true");
        check("lcolumnhas", "true");
        check("cheaderdefault", "");
        check("ccolumndefault", "");
        check("cheaderdirect", "header.ico");
        check("lcolumnsetpem", "true");
        check("ccolumnsetpem", "column.cur");
        check("lheaderputpem", "true");
        check("cheaderputpem", "header.ani");
        check("lheaderadd", "false");
        check("lheaderremove", "false");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lheadermember", "true");
        check("cgridheadericon", "grid.cur");
        check("cderivedheadericon", "derived-header.ico");
        check("cderivedcolumnicon", "derived-column.cur");

        fs::remove_all(temp_root, ignored);
    }
}
