#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_control_array_range_properties_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_array_range";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_array_range.prg";
        write_text(
            main_path,
            "DIMENSION gaValues[5]\n"
            "gaValues[1] = 'Alpha'\n"
            "gaValues[2] = 'Bravo'\n"
            "gaValues[3] = 'Charlie'\n"
            "gaValues[4] = 'Delta'\n"
            "gaValues[5] = 'Echo'\n"
            "DIMENSION gaMatrix[3,2]\n"
            "gaMatrix[1,1] = 'M1'\n"
            "gaMatrix[1,2] = 'X1'\n"
            "gaMatrix[2,1] = 'M2'\n"
            "gaMatrix[2,2] = 'X2'\n"
            "gaMatrix[3,1] = 'M3'\n"
            "gaMatrix[3,2] = 'X3'\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "nDefaultFirst = oList.FirstElement\n"
            "nDefaultCount = oList.NumberOfElements\n"
            "lFirstHas = PEMSTATUS(oList, 'FirstElement', 1)\n"
            "lCountHas = PEMSTATUS(oList, 'NumberOfElements', 1)\n"
            "lFirstReadOnly = PEMSTATUS(oList, 'FirstElement', 5)\n"
            "oList.RowSourceType = 5\n"
            "oList.RowSource = 'gaValues'\n"
            "oList.FirstElement = 2\n"
            "oList.NumberOfElements = 2\n"
            "oList.Requery()\n"
            "nRangeListCount = oList.ListCount\n"
            "cRangeFirst = oList.List(1)\n"
            "cRangeLast = oList.List(2)\n"
            "lSetFirst = SETPEM(oList, 'FirstElement', 4)\n"
            "lPutCount = PUTPEM(oList, 'NumberOfElements', 1)\n"
            "oList.Requery()\n"
            "cSetRangeFirst = oList.List(1)\n"
            "nSetRangeCount = oList.ListCount\n"
            "oList.FirstElement = 0\n"
            "oList.NumberOfElements = -1\n"
            "oList.Requery()\n"
            "nNormalizedFirst = oList.FirstElement\n"
            "nNormalizedCount = oList.NumberOfElements\n"
            "nNormalizedListCount = oList.ListCount\n"
            "oCombo.RowSourceType = 5\n"
            "oCombo.RowSource = 'gaMatrix'\n"
            "oCombo.FirstElement = 2\n"
            "oCombo.NumberOfElements = 1\n"
            "oCombo.Requery()\n"
            "nMatrixListCount = oCombo.ListCount\n"
            "cMatrixSecond = oCombo.List(2, 1)\n"
            "lAddFirst = ADDPROPERTY(oList, 'FirstElement', 2)\n"
            "lRemoveCount = REMOVEPROPERTY(oList, 'NumberOfElements')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lFirstProp = .F.\n"
            "lCountProp = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'FIRSTELEMENT'\n"
            "        lFirstProp = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'NUMBEROFELEMENTS'\n"
            "        lCountProp = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedArrayList')\n"
            "nDerivedFirst = oDerived.FirstElement\n"
            "nDerivedCount = oDerived.NumberOfElements\n"
            "nDerivedListCount = oDerived.ListCount\n"
            "cDerivedFirst = oDerived.List(1)\n"
            "RETURN\n"
            "DEFINE CLASS DerivedArrayList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.RowSourceType = 5\n"
            "        THIS.RowSource = 'gaValues'\n"
            "        THIS.FirstElement = 3\n"
            "        THIS.NumberOfElements = 1\n"
            "        THIS.Requery()\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native array range script should complete: ") + state.message +
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

        check("ndefaultfirst", "1");
        check("ndefaultcount", "0");
        check("lfirsthas", "true");
        check("lcounthas", "true");
        check("lfirstreadonly", "false");
        check("nrangelistcount", "2");
        check("crangefirst", "Bravo");
        check("crangelast", "Charlie");
        check("lsetfirst", "true");
        check("lputcount", "true");
        check("csetrangefirst", "Delta");
        check("nsetrangecount", "1");
        check("nnormalizedfirst", "1");
        check("nnormalizedcount", "0");
        check("nnormalizedlistcount", "5");
        check("nmatrixlistcount", "3");
        check("cmatrixsecond", "M2");
        check("laddfirst", "false");
        check("lremovecount", "false");
        check("lfirstprop", "true");
        check("lcountprop", "true");
        check("nderivedfirst", "3");
        check("nderivedcount", "1");
        check("nderivedlistcount", "1");
        check("cderivedfirst", "Charlie");
        expect(state.ole_objects.size() == 3U,
               "native array range coverage should register ListBox, ComboBox, and derived ListBox");

        fs::remove_all(temp_root, ignored);
    }
}
