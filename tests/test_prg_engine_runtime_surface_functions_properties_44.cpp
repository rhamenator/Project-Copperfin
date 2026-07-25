#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_column_header_materializes_lazily_and_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_column_header";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_column_header.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 3\n"
            "nColumnsBefore = oGrid.Columns.Count\n"
            "lColumnHasHeader = PEMSTATUS(oGrid.Column1, 'Header', 1)\n"
            "oHeader = oGrid.Column1.Header\n"
            "lSameHeader = COMPOBJ(oHeader, oGrid.Columns(1).Header)\n"
            "lParent = COMPOBJ(oHeader.Parent, oGrid.Column1)\n"
            "cParentName = oHeader.Parent.Name\n"
            "lHasWordWrap = PEMSTATUS(oHeader, 'WordWrap', 1)\n"
            "lDefault = oHeader.WordWrap\n"
            "oGrid.Column1.Header.WordWrap = .T.\n"
            "lDirect = oHeader.WordWrap\n"
            "lSetPem = SETPEM(oGrid.Columns[1].Header, 'WordWrap', .F.)\n"
            "lAfterSetPem = oGrid.Column1.Header.WordWrap\n"
            "lPutPem = PUTPEM(oGrid.Column1.Header, 'WordWrap', .T.)\n"
            "lAfterPutPem = GETPEM(oGrid.Column1.Header, 'WordWrap')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oGrid.Column1, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'HEADER'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "nOrder1Before = oGrid.Column1.ColumnOrder\n"
            "nOrder2Before = oGrid.Column2.ColumnOrder\n"
            "nOrder3Before = oGrid.Column3.ColumnOrder\n"
            "oGrid.Column3.ColumnOrder = 1\n"
            "oGrid.Column1.ColumnOrder = 3\n"
            "nOrder1After = oGrid.Column1.ColumnOrder\n"
            "nOrder2After = oGrid.Column2.ColumnOrder\n"
            "nOrder3After = oGrid.Column3.ColumnOrder\n"
            "cFirstColumn = oGrid.Columns(1).Name\n"
            "lReorderedHeader = PEMSTATUS(oGrid.Columns(1).Header, 'WordWrap', 1)\n"
            "nColumnsAfterReorder = oGrid.Columns.Count\n"
            "oGrid.ColumnCount = 1\n"
            "nColumnsAfterRemove = oGrid.Columns.Count\n"
            "lColumn2AfterRemove = PEMSTATUS(oGrid, 'Column2', 1)\n"
            "oDerivedColumn = CREATEOBJECT('DerivedColumnWithHeader')\n"
            "lDerivedHeader = PEMSTATUS(oDerivedColumn.Header, 'WordWrap', 1)\n"
            "lDerivedDefault = oDerivedColumn.Header.WordWrap\n"
            "RETURN\n"
            "DEFINE CLASS DerivedColumnWithHeader AS Column\n"
            "    ADD OBJECT Header AS DerivedHeader\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedHeader AS Header\n"
            "    WordWrap = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid Column.Header script should complete: ") + state.message +
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

        check("ncolumnsbefore", "3");
        check("lcolumnhasheader", "true");
        check("lsameheader", "true");
        check("lparent", "true");
        check("cparentname", "Column1");
        check("lhaswordwrap", "true");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("laftersetpem", "false");
        check("lputpem", "true");
        check("lafterputpem", "true");
        check("lmember", "true");
        check("norder1before", "1");
        check("norder2before", "2");
        check("norder3before", "3");
        check("norder1after", "3");
        check("norder2after", "2");
        check("norder3after", "1");
        check("cfirstcolumn", "Column3");
        check("lreorderedheader", "true");
        check("ncolumnsafterreorder", "3");
        check("ncolumnsafterremove", "1");
        check("lcolumn2afterremove", "false");
        check("lderivedheader", "true");
        check("lderiveddefault", "true");

        fs::remove_all(temp_root, ignored);
    }
}
