#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_column_inputmask_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_inputmask";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_inputmask.prg";
        write_text(
            main_path,
            "oColumn = CREATEOBJECT('Column')\n"
            "oHeader = CREATEOBJECT('Header')\n"
            "lColumnHas = PEMSTATUS(oColumn, 'InputMask', 1)\n"
            "lColumnReadOnly = PEMSTATUS(oColumn, 'InputMask', 5)\n"
            "cColumnDefault = oColumn.InputMask\n"
            "xColumnGetPem = GETPEM(oColumn, 'InputMask')\n"
            "oColumn.InputMask = '@R ###'\n"
            "cColumnDirect = oColumn.InputMask\n"
            "lColumnSetPem = SETPEM(oColumn, 'InputMask', '@!')\n"
            "cColumnAfterSetPem = GETPEM(oColumn, 'InputMask')\n"
            "lColumnPutPem = PUTPEM(oColumn, 'InputMask', '9999')\n"
            "cColumnAfterPutPem = GETPEM(oColumn, 'InputMask')\n"
            "lColumnAdd = ADDPROPERTY(oColumn, 'InputMask', 'shadow')\n"
            "lColumnRemove = REMOVEPROPERTY(oColumn, 'InputMask')\n"
            "lColumnMember = .F.\n"
            "nColumnMembers = AMEMBERS(aColumnMembers, oColumn, 1)\n"
            "FOR i = 1 TO nColumnMembers\n"
            "    IF UPPER(aColumnMembers[i]) == 'INPUTMASK'\n"
            "        lColumnMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.InputMask = 'grid-mask'\n"
            "cGridColumnMask = oGrid.Column1.InputMask\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'InputMask', 1)\n"
            "oDerived = CREATEOBJECT('DerivedInputMaskColumn')\n"
            "cDerived = oDerived.InputMask\n"
            "RETURN\n"
            "DEFINE CLASS DerivedInputMaskColumn AS Column\n"
            "    InputMask = 'derived-mask'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Column InputMask script should complete: ") + state.message +
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
        check("ccolumndirect", "@R ###");
        check("lcolumnsetpem", "true");
        check("ccolumnaftersetpem", "@!");
        check("lcolumnputpem", "true");
        check("ccolumnafterputpem", "9999");
        check("lcolumnadd", "false");
        check("lcolumnremove", "false");
        check("lcolumnmember", "true");
        check("cgridcolumnmask", "grid-mask");
        check("lheaderhas", "false");
        check("cderived", "derived-mask");

        fs::remove_all(temp_root, ignored);
    }
}
