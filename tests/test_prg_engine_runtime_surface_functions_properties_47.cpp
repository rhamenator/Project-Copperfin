#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_colors_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_colors";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_colors.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderBack = PEMSTATUS(oHeader, 'BackColor', 1)\n"
            "lHeaderFore = PEMSTATUS(oHeader, 'ForeColor', 1)\n"
            "lColumnBack = PEMSTATUS(oColumn, 'BackColor', 1)\n"
            "lColumnFore = PEMSTATUS(oColumn, 'ForeColor', 1)\n"
            "nHeaderBackDefault = oHeader.BackColor\n"
            "nHeaderForeDefault = oHeader.ForeColor\n"
            "nColumnBackDefault = oColumn.BackColor\n"
            "nColumnForeDefault = oColumn.ForeColor\n"
            "oHeader.BackColor = 255\n"
            "oColumn.ForeColor = 65280\n"
            "nHeaderBackDirect = oHeader.BackColor\n"
            "nColumnForeDirect = oColumn.ForeColor\n"
            "lHeaderSetPem = SETPEM(oHeader, 'ForeColor', 16711680)\n"
            "nHeaderForeSetPem = GETPEM(oHeader, 'ForeColor')\n"
            "lColumnSetPem = SETPEM(oColumn, 'BackColor', 65535)\n"
            "nColumnBackSetPem = GETPEM(oColumn, 'BackColor')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'BackColor', 16776960)\n"
            "nHeaderBackPutPem = GETPEM(oHeader, 'BackColor')\n"
            "lColumnPutPem = PUTPEM(oColumn, 'ForeColor', 16711935)\n"
            "nColumnForePutPem = GETPEM(oColumn, 'ForeColor')\n"
            "lHeaderAddBack = ADDPROPERTY(oHeader, 'BackColor', 0)\n"
            "lHeaderRemoveFore = REMOVEPROPERTY(oHeader, 'ForeColor')\n"
            "lColumnAddBack = ADDPROPERTY(oColumn, 'BackColor', 0)\n"
            "lColumnRemoveFore = REMOVEPROPERTY(oColumn, 'ForeColor')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'BACKCOLOR'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.BackColor = 16777215\n"
            "oGrid.Column1.Header.ForeColor = 0\n"
            "nGridHeaderBack = oGrid.Columns(1).Header.BackColor\n"
            "nGridHeaderFore = oGrid.Columns(1).Header.ForeColor\n"
            "oDerivedHeader = CREATEOBJECT('DerivedColorHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedColorColumn')\n"
            "nDerivedHeaderBack = oDerivedHeader.BackColor\n"
            "nDerivedColumnFore = oDerivedColumn.ForeColor\n"
            "RETURN\n"
            "DEFINE CLASS DerivedColorHeader AS Header\n"
            "    BackColor = 8421504\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedColorColumn AS Column\n"
            "    ForeColor = 8388608\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column color script should complete: ") + state.message +
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

        check("lheaderback", "true");
        check("lheaderfore", "true");
        check("lcolumnback", "true");
        check("lcolumnfore", "true");
        check("nheaderbackdefault", "16777215");
        check("nheaderforedefault", "0");
        check("ncolumnbackdefault", "16777215");
        check("ncolumnforedefault", "0");
        check("nheaderbackdirect", "255");
        check("ncolumnforedirect", "65280");
        check("lheadersetpem", "true");
        check("nheaderforesetpem", "16711680");
        check("lcolumnsetpem", "true");
        check("ncolumnbacksetpem", "65535");
        check("lheaderputpem", "true");
        check("nheaderbackputpem", "16776960");
        check("lcolumnputpem", "true");
        check("ncolumnforeputpem", "16711935");
        check("lheaderaddback", "false");
        check("lheaderremovefore", "false");
        check("lcolumnaddback", "false");
        check("lcolumnremovefore", "false");
        check("lheadermember", "true");
        check("ngridheaderback", "16777215");
        check("ngridheaderfore", "0");
        check("nderivedheaderback", "8421504");
        check("nderivedcolumnfore", "8388608");

        fs::remove_all(temp_root, ignored);
    }
}
