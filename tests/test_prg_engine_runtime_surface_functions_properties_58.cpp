#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_column_dynamic_color_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_dynamic_color";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_dynamic_color.prg";
        write_text(
            main_path,
            "oColumn = CREATEOBJECT('Column')\n"
            "oHeader = CREATEOBJECT('Header')\n"
            "lBackHas = PEMSTATUS(oColumn, 'DynamicBackColor', 1)\n"
            "lForeHas = PEMSTATUS(oColumn, 'DynamicForeColor', 1)\n"
            "lBackReadOnly = PEMSTATUS(oColumn, 'DynamicBackColor', 5)\n"
            "lForeReadOnly = PEMSTATUS(oColumn, 'DynamicForeColor', 5)\n"
            "cBackDefault = oColumn.DynamicBackColor\n"
            "cForeDefault = GETPEM(oColumn, 'DynamicForeColor')\n"
            "oColumn.DynamicBackColor = 'IIF(MOD(RECNO(),2)=0, RGB(1,2,3), RGB(4,5,6))'\n"
            "cBackDirect = oColumn.DynamicBackColor\n"
            "lForeSetPem = SETPEM(oColumn, 'DynamicForeColor', 'RGB(7,8,9)')\n"
            "cForeAfterSetPem = GETPEM(oColumn, 'DynamicForeColor')\n"
            "lBackPutPem = PUTPEM(oColumn, 'DynamicBackColor', 'RGB(10,11,12)')\n"
            "cBackAfterPutPem = GETPEM(oColumn, 'DynamicBackColor')\n"
            "lBackAdd = ADDPROPERTY(oColumn, 'DynamicBackColor', 'shadow')\n"
            "lBackRemove = REMOVEPROPERTY(oColumn, 'DynamicBackColor')\n"
            "lForeAdd = ADDPROPERTY(oColumn, 'DynamicForeColor', 'shadow')\n"
            "lForeRemove = REMOVEPROPERTY(oColumn, 'DynamicForeColor')\n"
            "lBackMember = .F.\n"
            "lForeMember = .F.\n"
            "nColumnMembers = AMEMBERS(aColumnMembers, oColumn, 1)\n"
            "FOR i = 1 TO nColumnMembers\n"
            "    IF UPPER(aColumnMembers[i]) == 'DYNAMICBACKCOLOR'\n"
            "        lBackMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aColumnMembers[i]) == 'DYNAMICFORECOLOR'\n"
            "        lForeMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.DynamicBackColor = 'grid-back'\n"
            "oGrid.Column1.DynamicForeColor = 'grid-fore'\n"
            "cGridBack = oGrid.Column1.DynamicBackColor\n"
            "cGridFore = oGrid.Column1.DynamicForeColor\n"
            "lHeaderBackHas = PEMSTATUS(oHeader, 'DynamicBackColor', 1)\n"
            "lHeaderForeHas = PEMSTATUS(oHeader, 'DynamicForeColor', 1)\n"
            "oDerived = CREATEOBJECT('DerivedDynamicColorColumn')\n"
            "cDerivedBack = oDerived.DynamicBackColor\n"
            "cDerivedFore = oDerived.DynamicForeColor\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicColorColumn AS Column\n"
            "    DynamicBackColor = 'derived-back'\n"
            "    DynamicForeColor = 'derived-fore'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Column dynamic color script should complete: ") + state.message +
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

        check("lbackhas", "true");
        check("lforehas", "true");
        check("lbackreadonly", "false");
        check("lforereadonly", "false");
        check("cbackdefault", "");
        check("cforedefault", "");
        check("cbackdirect", "IIF(MOD(RECNO(),2)=0, RGB(1,2,3), RGB(4,5,6))");
        check("lforesetpem", "true");
        check("cforeaftersetpem", "RGB(7,8,9)");
        check("lbackputpem", "true");
        check("cbackafterputpem", "RGB(10,11,12)");
        check("lbackadd", "false");
        check("lbackremove", "false");
        check("lforeadd", "false");
        check("lforeremove", "false");
        check("lbackmember", "true");
        check("lforemember", "true");
        check("cgridback", "grid-back");
        check("cgridfore", "grid-fore");
        check("lheaderbackhas", "false");
        check("lheaderforehas", "false");
        check("cderivedback", "derived-back");
        check("cderivedfore", "derived-fore");

        fs::remove_all(temp_root, ignored);
    }
}
