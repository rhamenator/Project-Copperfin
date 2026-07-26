#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_base_font_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_base_fonts";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_base_fonts.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderFontName = PEMSTATUS(oHeader, 'FontName', 1)\n"
            "lHeaderFontCharSet = PEMSTATUS(oHeader, 'FontCharSet', 1)\n"
            "lHeaderFontSize = PEMSTATUS(oHeader, 'FontSize', 1)\n"
            "lColumnFontName = PEMSTATUS(oColumn, 'FontName', 1)\n"
            "lColumnFontCharSet = PEMSTATUS(oColumn, 'FontCharSet', 1)\n"
            "lColumnFontSize = PEMSTATUS(oColumn, 'FontSize', 1)\n"
            "cHeaderFontNameDefault = oHeader.FontName\n"
            "nHeaderFontCharSetDefault = oHeader.FontCharSet\n"
            "nHeaderFontSizeDefault = oHeader.FontSize\n"
            "cColumnFontNameDefault = oColumn.FontName\n"
            "nColumnFontCharSetDefault = oColumn.FontCharSet\n"
            "nColumnFontSizeDefault = oColumn.FontSize\n"
            "oHeader.FontName = 'Consolas'\n"
            "oColumn.FontSize = 12.5\n"
            "cHeaderFontNameDirect = oHeader.FontName\n"
            "nColumnFontSizeDirect = oColumn.FontSize\n"
            "lHeaderSetPem = SETPEM(oHeader, 'FontCharSet', -3.9)\n"
            "nHeaderFontCharSetSetPem = GETPEM(oHeader, 'FontCharSet')\n"
            "lColumnSetPem = SETPEM(oColumn, 'FontSize', -2.25)\n"
            "nColumnFontSizeSetPem = GETPEM(oColumn, 'FontSize')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'FontSize', 14.75)\n"
            "nHeaderFontSizePutPem = GETPEM(oHeader, 'FontSize')\n"
            "lColumnPutPem = PUTPEM(oColumn, 'FontCharSet', 3.8)\n"
            "nColumnFontCharSetPutPem = GETPEM(oColumn, 'FontCharSet')\n"
            "lHeaderAddFontName = ADDPROPERTY(oHeader, 'FontName', 'Shadow')\n"
            "lHeaderRemoveFontSize = REMOVEPROPERTY(oHeader, 'FontSize')\n"
            "lColumnAddFontCharSet = ADDPROPERTY(oColumn, 'FontCharSet', 0)\n"
            "lColumnRemoveFontName = REMOVEPROPERTY(oColumn, 'FontName')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'FONTNAME'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.FontName = 'Courier New'\n"
            "oGrid.Column1.Header.FontCharSet = 4\n"
            "oGrid.Column1.Header.FontSize = 11.25\n"
            "cGridHeaderFontName = oGrid.Columns(1).Header.FontName\n"
            "nGridHeaderFontCharSet = oGrid.Columns(1).Header.FontCharSet\n"
            "nGridHeaderFontSize = oGrid.Columns(1).Header.FontSize\n"
            "oDerivedHeader = CREATEOBJECT('DerivedFontHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedFontColumn')\n"
            "cDerivedHeaderFontName = oDerivedHeader.FontName\n"
            "nDerivedColumnFontSize = oDerivedColumn.FontSize\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontHeader AS Header\n"
            "    FontName = 'Segoe UI'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedFontColumn AS Column\n"
            "    FontSize = 9.5\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column base-font script should complete: ") + state.message +
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

        check("lheaderfontname", "true");
        check("lheaderfontcharset", "true");
        check("lheaderfontsize", "true");
        check("lcolumnfontname", "true");
        check("lcolumnfontcharset", "true");
        check("lcolumnfontsize", "true");
        check("cheaderfontnamedefault", "Arial");
        check("nheaderfontcharsetdefault", "1");
        check("nheaderfontsizedefault", "10");
        check("ccolumnfontnamedefault", "Arial");
        check("ncolumnfontcharsetdefault", "1");
        check("ncolumnfontsizedefault", "10");
        check("cheaderfontnamedirect", "Consolas");
        check("ncolumnfontsizedirect", "12.5");
        check("lheadersetpem", "true");
        check("nheaderfontcharsetsetpem", "0");
        check("lcolumnsetpem", "true");
        check("ncolumnfontsizesetpem", "0");
        check("lheaderputpem", "true");
        check("nheaderfontsizeputpem", "14.75");
        check("lcolumnputpem", "true");
        check("ncolumnfontcharsetputpem", "4");
        check("lheaderaddfontname", "false");
        check("lheaderremovefontsize", "false");
        check("lcolumnaddfontcharset", "false");
        check("lcolumnremovefontname", "false");
        check("lheadermember", "true");
        check("cgridheaderfontname", "Courier New");
        check("ngridheaderfontcharset", "4");
        check("ngridheaderfontsize", "11.25");
        check("cderivedheaderfontname", "Segoe UI");
        check("nderivedcolumnfontsize", "9.5");

        fs::remove_all(temp_root, ignored);
    }
}
