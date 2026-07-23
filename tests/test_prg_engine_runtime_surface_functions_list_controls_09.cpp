#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_control_nulldisplay_stays_builtin_and_visible()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_nulldisplay";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_nulldisplay.prg";
        write_text(
            main_path,
            "DIMENSION gaValues[2]\n"
            "gaValues[1] = .NULL.\n"
            "gaValues[2] = 'Ready'\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "cDefault = oList.NullDisplay\n"
            "lListHas = PEMSTATUS(oList, 'NullDisplay', 1)\n"
            "lComboHas = PEMSTATUS(oCombo, 'NullDisplay', 1)\n"
            "lReadOnly = PEMSTATUS(oList, 'NullDisplay', 5)\n"
            "oList.RowSourceType = 5\n"
            "oList.RowSource = 'gaValues'\n"
            "oList.Requery()\n"
            "oList.ListIndex = 1\n"
            "cInitialDisplay = oList.DisplayValue\n"
            "lInitialValueNull = ISNULL(oList.Value)\n"
            "lSetPem = SETPEM(oList, 'NullDisplay', '[null]')\n"
            "oList.ListIndex = 1\n"
            "cSetPemDisplay = oList.DisplayValue\n"
            "lPutPem = PUTPEM(oList, 'NullDisplay', '<none>')\n"
            "oList.ListIndex = 1\n"
            "cPutPemDisplay = oList.DisplayValue\n"
            "oList.NullDisplay = 42\n"
            "oList.ListIndex = 1\n"
            "cNormalizedDisplay = oList.DisplayValue\n"
            "lAfterValueNull = ISNULL(oList.Value)\n"
            "lAddProperty = ADDPROPERTY(oList, 'NullDisplay', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'NullDisplay')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'NULLDISPLAY'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oList.ListIndex = 2\n"
            "cNonNullDisplay = oList.DisplayValue\n"
            "oDerived = CREATEOBJECT('DerivedNullList')\n"
            "nDerived = oDerived.NullDisplay\n"
            "cDerivedDisplay = oDerived.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS DerivedNullList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.NullDisplay = 'missing'\n"
            "        THIS.RowSourceType = 5\n"
            "        THIS.RowSource = 'gaValues'\n"
            "        THIS.Requery()\n"
            "        THIS.ListIndex = 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native NullDisplay script should complete: ") + state.message +
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

        check("cdefault", ".NULL.");
        check("llisthas", "true");
        check("lcombohas", "true");
        check("lreadonly", "false");
        check("cinitialdisplay", ".NULL.");
        check("linitialvaluenull", "true");
        check("lsetpem", "true");
        check("csetpemdisplay", "[null]");
        check("lputpem", "true");
        check("cputpemdisplay", "<none>");
        check("cnormalizeddisplay", "42");
        check("laftervaluenull", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("cnonnulldisplay", "Ready");
        check("nderived", "missing");
        check("cderiveddisplay", "missing");
        expect(state.ole_objects.size() == 3U,
               "native NullDisplay coverage should register ListBox, ComboBox, and derived ListBox");

        fs::remove_all(temp_root, ignored);
    }
}
