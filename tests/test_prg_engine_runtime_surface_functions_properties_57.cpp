#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_column_format_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_format";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_format.prg";
        write_text(
            main_path,
            "oColumn = CREATEOBJECT('Column')\n"
            "oHeader = CREATEOBJECT('Header')\n"
            "lColumnHas = PEMSTATUS(oColumn, 'Format', 1)\n"
            "lColumnReadOnly = PEMSTATUS(oColumn, 'Format', 5)\n"
            "cColumnDefault = oColumn.Format\n"
            "xColumnGetPem = GETPEM(oColumn, 'Format')\n"
            "oColumn.Format = 'F'\n"
            "cColumnDirect = oColumn.Format\n"
            "lColumnSetPem = SETPEM(oColumn, 'Format', '@!')\n"
            "cColumnAfterSetPem = GETPEM(oColumn, 'Format')\n"
            "lColumnPutPem = PUTPEM(oColumn, 'Format', 'R')\n"
            "cColumnAfterPutPem = GETPEM(oColumn, 'Format')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'Format', 'shadow')\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'Format')\n"
            "lColumnMember = .F.\n"
            "nColumnMembers = AMEMBERS(aColumnMembers, oColumn, 1)\n"
            "FOR i = 1 TO nColumnMembers\n"
            "    IF UPPER(aColumnMembers[i]) == 'FORMAT'\n"
            "        lColumnMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.Format = 'grid-format'\n"
            "cGridColumnFormat = oGrid.Column1.Format\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'Format', 1)\n"
            "oDerived = CREATEOBJECT('DerivedFormatColumn')\n"
            "cDerived = oDerived.Format\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFormatColumn AS Column\n"
            "    Format = 'derived-format'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Column Format script should complete: ") + state.message +
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

        check("lcolumnhas", "true");
        check("lcolumnreadonly", "false");
        check("ccolumndefault", "");
        check("xcolumngetpem", "");
        check("ccolumndirect", "F");
        check("lcolumnsetpem", "true");
        check("ccolumnaftersetpem", "@!");
        check("lcolumnputpem", "true");
        check("ccolumnafterputpem", "R");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lcolumnmember", "true");
        check("cgridcolumnformat", "grid-format");
        check("lheaderhas", "false");
        check("cderived", "derived-format");

        fs::remove_all(temp_root, ignored);
    }
}
