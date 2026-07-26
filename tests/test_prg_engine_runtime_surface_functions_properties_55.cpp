#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_column_enabled_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_enabled";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_enabled.prg";
        write_text(
            main_path,
            "oColumn = CREATEOBJECT('Column')\n"
            "lColumnHas = PEMSTATUS(oColumn, 'Enabled', 1)\n"
            "lColumnReadOnly = PEMSTATUS(oColumn, 'Enabled', 5)\n"
            "lColumnDefault = oColumn.Enabled\n"
            "xColumnGetPem = GETPEM(oColumn, 'Enabled')\n"
            "oColumn.Enabled = .F.\n"
            "lColumnDirect = oColumn.Enabled\n"
            "lColumnSetPem = SETPEM(oColumn, 'Enabled', .T.)\n"
            "lColumnAfterSetPem = oColumn.Enabled\n"
            "lColumnPutPem = PUTPEM(oColumn, 'Enabled', .F.)\n"
            "lColumnAfterPutPem = GETPEM(oColumn, 'Enabled')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'Enabled', .T.)\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'Enabled')\n"
            "lColumnMember = .F.\n"
            "nColumnMembers = AMEMBERS(aColumnMembers, oColumn, 1)\n"
            "FOR i = 1 TO nColumnMembers\n"
            "    IF UPPER(aColumnMembers[i]) == 'ENABLED'\n"
            "        lColumnMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "lGridColumnDefault = oGrid.Column1.Enabled\n"
            "oGrid.Column1.Enabled = .F.\n"
            "lGridColumnDirect = oGrid.Column1.Enabled\n"
            "oDerived = CREATEOBJECT('DerivedEnabledColumn')\n"
            "lDerived = oDerived.Enabled\n"
            "RETURN\n"
            "DEFINE CLASS DerivedEnabledColumn AS Column\n"
            "    Enabled = .F.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Column Enabled script should complete: ") + state.message +
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
        check("lcolumndefault", "true");
        check("xcolumngetpem", "true");
        check("lcolumndirect", "false");
        check("lcolumnsetpem", "true");
        check("lcolumnaftersetpem", "true");
        check("lcolumnputpem", "true");
        check("lcolumnafterputpem", "false");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lcolumnmember", "true");
        check("lgridcolumndefault", "true");
        check("lgridcolumndirect", "false");
        check("lderived", "false");

        fs::remove_all(temp_root, ignored);
    }
}
