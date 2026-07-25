#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_header_column_alignment_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_header_column_alignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_column_alignment.prg";
        write_text(
            main_path,
            "oHeader = CREATEOBJECT('Header')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'Alignment', 1)\n"
            "lColumnHas = PEMSTATUS(oColumn, 'Alignment', 1)\n"
            "lHeaderReadOnly = PEMSTATUS(oHeader, 'Alignment', 5)\n"
            "lColumnReadOnly = PEMSTATUS(oColumn, 'Alignment', 5)\n"
            "nHeaderDefault = oHeader.Alignment\n"
            "nColumnDefault = oColumn.Alignment\n"
            "oHeader.Alignment = 9\n"
            "oColumn.Alignment = 4\n"
            "nHeaderDirect = oHeader.Alignment\n"
            "nColumnDirect = oColumn.Alignment\n"
            "lHeaderSetPem = SETPEM(oHeader, 'Alignment', -1)\n"
            "nHeaderSetPem = GETPEM(oHeader, 'Alignment')\n"
            "lColumnSetPem = SETPEM(oColumn, 'Alignment', 12)\n"
            "nColumnSetPem = GETPEM(oColumn, 'Alignment')\n"
            "lHeaderPutPem = PUTPEM(oHeader, 'Alignment', '7.4')\n"
            "nHeaderPutPem = GETPEM(oHeader, 'Alignment')\n"
            "lColumnPutPem = PUTPEM(oColumn, 'Alignment', '1.8')\n"
            "nColumnPutPem = GETPEM(oColumn, 'Alignment')\n"
            "lHeaderAddProperty = ADDPROPERTY(oHeader, 'Alignment', 0)\n"
            "lHeaderRemoveProperty = REMOVEPROPERTY(oHeader, 'Alignment')\n"
            "lColumnAddProperty = ADDPROPERTY(oColumn, 'Alignment', 0)\n"
            "lColumnRemoveProperty = REMOVEPROPERTY(oColumn, 'Alignment')\n"
            "lHeaderMember = .F.\n"
            "nHeaderMembers = AMEMBERS(aHeaderMembers, oHeader, 1)\n"
            "FOR i = 1 TO nHeaderMembers\n"
            "    IF UPPER(aHeaderMembers[i]) == 'ALIGNMENT'\n"
            "        lHeaderMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Header.Alignment = 6\n"
            "nGridHeaderAlignment = oGrid.Columns(1).Header.Alignment\n"
            "oDerivedHeader = CREATEOBJECT('DerivedAlignmentHeader')\n"
            "oDerivedColumn = CREATEOBJECT('DerivedAlignmentColumn')\n"
            "nDerivedHeaderAlignment = oDerivedHeader.Alignment\n"
            "nDerivedColumnAlignment = oDerivedColumn.Alignment\n"
            "RETURN\n"
            "DEFINE CLASS DerivedAlignmentHeader AS Header\n"
            "    Alignment = 8\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedAlignmentColumn AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.Alignment = 5\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header/Column Alignment script should complete: ") + state.message +
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
        check("lheaderreadonly", "false");
        check("lcolumnreadonly", "false");
        check("nheaderdefault", "3");
        check("ncolumndefault", "3");
        check("nheaderdirect", "9");
        check("ncolumndirect", "4");
        check("lheadersetpem", "true");
        check("nheadersetpem", "0");
        check("lcolumnsetpem", "true");
        check("ncolumnsetpem", "9");
        check("lheaderputpem", "true");
        check("nheaderputpem", "7");
        check("lcolumnputpem", "true");
        check("ncolumnputpem", "2");
        check("lheaderaddproperty", "false");
        check("lheaderremoveproperty", "false");
        check("lcolumnaddproperty", "false");
        check("lcolumnremoveproperty", "false");
        check("lheadermember", "true");
        check("ngridheaderalignment", "6");
        check("nderivedheaderalignment", "8");
        check("nderivedcolumnalignment", "5");

        fs::remove_all(temp_root, ignored);
    }
}
