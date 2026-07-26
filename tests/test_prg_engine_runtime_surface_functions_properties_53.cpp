#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_statusbartext_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_statusbartext";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_statusbartext.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'StatusBarText', 1)\n"
            "lColumnHas = PEMSTATUS(oColumn, 'StatusBarText', 1)\n"
            "cHeaderDefault = oHeader.StatusBarText\n"
            "cColumnDefault = oColumn.StatusBarText\n"
            "oHeader.StatusBarText = 'header-direct'\n"
            "cHeaderDirect = oHeader.StatusBarText\n"
            "lColumnSetPem = SETPEM(oColumn, 'StatusBarText', 'column-set')\n"
            "cColumnSetPem = GETPEM(oColumn, 'StatusBarText')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'StatusBarText', 'header-put')\n"
            "cHeaderPutPem = GETPEM(oHeader, 'StatusBarText')\n"
            "lHeaderAdd = ADDPROPERTY(oHeader, 'StatusBarText', 'shadow')\n"
            "lHeaderRemove = REMOVEPROPERTY(oHeader, 'StatusBarText')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'StatusBarText', 'shadow')\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'StatusBarText')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'STATUSBARTEXT'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.StatusBarText = 'grid-header'\n"
            "cGridHeaderStatus = oGrid.Columns(1).Header.StatusBarText\n"
            "oDerivedHeader = CREATEOBJECT('DerivedStatusHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedStatusColumn')\n"
            "cDerivedHeaderStatus = oDerivedHeader.StatusBarText\n"
            "cDerivedColumnStatus = oDerivedColumn.StatusBarText\n"
            "RETURN\n"
            "DEFINE CLASS DerivedStatusHeader AS Header\n"
            "    StatusBarText = 'derived-header'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedStatusColumn AS Column\n"
            "    StatusBarText = 'derived-column'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column StatusBarText script should complete: ") + state.message +
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
        check("cheaderdirect", "header-direct");
        check("lcolumnsetpem", "true");
        check("ccolumnsetpem", "column-set");
        check("lheaderputpem", "true");
        check("cheaderputpem", "header-put");
        check("lheaderadd", "false");
        check("lheaderremove", "false");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lheadermember", "true");
        check("cgridheaderstatus", "grid-header");
        check("cderivedheaderstatus", "derived-header");
        check("cderivedcolumnstatus", "derived-column");

        fs::remove_all(temp_root, ignored);
    }
}
