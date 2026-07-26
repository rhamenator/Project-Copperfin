#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_font_styles_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_font_styles";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_font_styles.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderBold = PEMSTATUS(oHeader, 'FontBold', 1)\n"
            "lHeaderItalic = PEMSTATUS(oHeader, 'FontItalic', 1)\n"
            "lHeaderUnderline = PEMSTATUS(oHeader, 'FontUnderline', 1)\n"
            "lHeaderStrike = PEMSTATUS(oHeader, 'FontStrikeThru', 1)\n"
            "lHeaderOutline = PEMSTATUS(oHeader, 'FontOutline', 1)\n"
            "lHeaderShadow = PEMSTATUS(oHeader, 'FontShadow', 1)\n"
            "lColumnBold = PEMSTATUS(oColumn, 'FontBold', 1)\n"
            "lColumnItalic = PEMSTATUS(oColumn, 'FontItalic', 1)\n"
            "lColumnUnderline = PEMSTATUS(oColumn, 'FontUnderline', 1)\n"
            "lColumnStrike = PEMSTATUS(oColumn, 'FontStrikeThru', 1)\n"
            "lColumnOutline = PEMSTATUS(oColumn, 'FontOutline', 1)\n"
            "lColumnShadow = PEMSTATUS(oColumn, 'FontShadow', 1)\n"
            "lHeaderDefault = !oHeader.FontBold AND !oHeader.FontItalic AND !oHeader.FontUnderline AND !oHeader.FontStrikeThru AND !oHeader.FontOutline AND !oHeader.FontShadow\n"
            "lColumnDefault = !oColumn.FontBold AND !oColumn.FontItalic AND !oColumn.FontUnderline AND !oColumn.FontStrikeThru AND !oColumn.FontOutline AND !oColumn.FontShadow\n"
            "oHeader.FontBold = .T.\n"
            "oColumn.FontShadow = .T.\n"
            "lHeaderDirect = oHeader.FontBold\n"
            "lColumnDirect = oColumn.FontShadow\n"
            "lHeaderSetPem = SETPEM(oHeader, 'FontItalic', .T.)\n"
            "lHeaderItalicSetPem = GETPEM(oHeader, 'FontItalic')\n"
            "lColumnSetPem = SETPEM(oColumn, 'FontUnderline', .T.)\n"
            "lColumnUnderlineSetPem = GETPEM(oColumn, 'FontUnderline')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'FontStrikeThru', .T.)\n"
            "lHeaderStrikePutPem = GETPEM(oHeader, 'FontStrikeThru')\n"
            "lColumnPutPem = PUTPEM(oColumn, 'FontOutline', .T.)\n"
            "lColumnOutlinePutPem = GETPEM(oColumn, 'FontOutline')\n"
            "lHeaderAdd = ADDPROPERTY(oHeader, 'FontBold', .F.)\n"
            "lHeaderRemove = REMOVEPROPERTY(oHeader, 'FontItalic')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'FontShadow', .F.)\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'FontOutline')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'FONTBOLD'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.FontBold = .T.\n"
            "oGrid.Column1.Header.FontShadow = .T.\n"
            "lGridHeaderBold = oGrid.Columns(1).Header.FontBold\n"
            "lGridHeaderShadow = oGrid.Columns(1).Header.FontShadow\n"
            "oDerivedHeader = CREATEOBJECT('DerivedStyleHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedStyleColumn')\n"
            "lDerivedHeader = oDerivedHeader.FontItalic\n"
            "lDerivedColumn = oDerivedColumn.FontOutline\n"
            "RETURN\n"
            "DEFINE CLASS DerivedStyleHeader AS Header\n"
            "    FontItalic = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedStyleColumn AS Column\n"
            "    FontOutline = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column font-style script should complete: ") + state.message +
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

        check("lheaderbold", "true");
        check("lheaderitalic", "true");
        check("lheaderunderline", "true");
        check("lheaderstrike", "true");
        check("lheaderoutline", "true");
        check("lheadershadow", "true");
        check("lcolumnbold", "true");
        check("lcolumnitalic", "true");
        check("lcolumnunderline", "true");
        check("lcolumnstrike", "true");
        check("lcolumnoutline", "true");
        check("lcolumnshadow", "true");
        check("lheaderdefault", "true");
        check("lcolumndefault", "true");
        check("lheaderdirect", "true");
        check("lcolumndirect", "true");
        check("lheadersetpem", "true");
        check("lheaderitalicsetpem", "true");
        check("lcolumnsetpem", "true");
        check("lcolumnunderlinesetpem", "true");
        check("lheaderputpem", "true");
        check("lheaderstrikeputpem", "true");
        check("lcolumnputpem", "true");
        check("lcolumnoutlineputpem", "true");
        check("lheaderadd", "false");
        check("lheaderremove", "false");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lheadermember", "true");
        check("lgridheaderbold", "true");
        check("lgridheadershadow", "true");
        check("lderivedheader", "true");
        check("lderivedcolumn", "true");

        fs::remove_all(temp_root, ignored);
    }
}
