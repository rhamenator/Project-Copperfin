#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_mousepointer_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_mousepointer";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_mousepointer.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'MousePointer', 1)\n"
            "lColumnHas = PEMSTATUS(oColumn, 'MousePointer', 1)\n"
            "nHeaderDefault = oHeader.MousePointer\n"
            "nColumnDefault = oColumn.MousePointer\n"
            "oHeader.MousePointer = 2\n"
            "nHeaderDirect = oHeader.MousePointer\n"
            "lColumnSetPem = SETPEM(oColumn, 'MousePointer', 4.4)\n"
            "nColumnSetPem = GETPEM(oColumn, 'MousePointer')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'MousePointer', -1)\n"
            "nHeaderPutPem = GETPEM(oHeader, 'MousePointer')\n"
            "oColumn.MousePointer = -3\n"
            "nColumnNormalized = oColumn.MousePointer\n"
            "lHeaderAdd = ADDPROPERTY(oHeader, 'MousePointer', 1)\n"
            "lHeaderRemove = REMOVEPROPERTY(oHeader, 'MousePointer')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'MousePointer', 1)\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'MousePointer')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'MOUSEPOINTER'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.MousePointer = 5\n"
            "nGridHeaderMousePointer = oGrid.Columns(1).Header.MousePointer\n"
            "oDerivedHeader = CREATEOBJECT('DerivedMouseHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedMouseColumn')\n"
            "nDerivedHeader = oDerivedHeader.MousePointer\n"
            "nDerivedColumn = oDerivedColumn.MousePointer\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMouseHeader AS Header\n"
            "    MousePointer = 6\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedMouseColumn AS Column\n"
            "    MousePointer = 7\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column MousePointer script should complete: ") + state.message +
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
        check("nheaderdefault", "0");
        check("ncolumndefault", "0");
        check("nheaderdirect", "2");
        check("lcolumnsetpem", "true");
        check("ncolumnsetpem", "4");
        check("lheaderputpem", "true");
        check("nheaderputpem", "0");
        check("ncolumnnormalized", "0");
        check("lheaderadd", "false");
        check("lheaderremove", "false");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lheadermember", "true");
        check("ngridheadermousepointer", "5");
        check("nderivedheader", "6");
        check("nderivedcolumn", "7");

        fs::remove_all(temp_root, ignored);
    }
}
