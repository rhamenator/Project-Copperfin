#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_programmaticchange_dispatches_after_effective_selection_changes()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_programmaticchange";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_programmaticchange.prg";
        write_text(
            main_path,
            "DIMENSION gaValues[2]\n"
            "gaValues[1] = 'Alpha'\n"
            "gaValues[2] = 'Beta'\n"
            "oCombo = CREATEOBJECT('ProgrammaticCombo')\n"
            "oCombo.AddItem('Alpha')\n"
            "oCombo.AddItem('Beta')\n"
            "oCombo.ListIndex = 1\n"
            "nComboAfterFirstSelection = oCombo.nChanges\n"
            "oCombo.ListIndex = 1\n"
            "nComboAfterNoOpSelection = oCombo.nChanges\n"
            "lComboSetPem = SETPEM(oCombo, 'ListItemID', 2)\n"
            "nComboAfterSetPem = oCombo.nChanges\n"
            "oCombo.Value = 'Beta'\n"
            "nComboAfterNoOpValue = oCombo.nChanges\n"
            "oCombo.Value = 'Alpha'\n"
            "nComboAfterValueChange = oCombo.nChanges\n"
            "oCombo.RowSourceType = 5\n"
            "oCombo.RowSource = 'gaValues'\n"
            "oCombo.Requery()\n"
            "oCombo.ListIndex = 1\n"
            "nComboBeforeRequeryChange = oCombo.nChanges\n"
            "gaValues[1] = 'Changed'\n"
            "oCombo.Requery()\n"
            "nComboAfterRequeryChange = oCombo.nChanges\n"
            "oList = CREATEOBJECT('ProgrammaticList')\n"
            "oList.MultiSelect = .T.\n"
            "oList.AddListItem('North', 11)\n"
            "oList.AddListItem('South', 22)\n"
            "oList.SelectedID(11) = .T.\n"
            "nListAfterSelectedId = oList.nChanges\n"
            "oList.SelectedID(11) = .T.\n"
            "nListAfterNoOpSelectedId = oList.nChanges\n"
            "oList.Selected(2) = .T.\n"
            "nListAfterSelectedSlot = oList.nChanges\n"
            "RETURN\n"
            "DEFINE CLASS ProgrammaticCombo AS ComboBox\n"
            "    nChanges = 0\n"
            "    PROCEDURE ProgrammaticChange\n"
            "        THIS.nChanges = THIS.nChanges + 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ProgrammaticList AS ListBox\n"
            "    nChanges = 0\n"
            "    PROCEDURE ProgrammaticChange\n"
            "        THIS.nChanges = THIS.nChanges + 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ProgrammaticChange script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end()) {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ncomboafterfirstselection", "1");
        check("ncomboafternoopselection", "1");
        check("lcombosetpem", "true");
        check("ncomboaftersetpem", "2");
        check("ncomboafternoopvalue", "2");
        check("ncomboaftervaluechange", "3");
        check("ncombobeforerequerychange", "4");
        check("ncomboafterrequerychange", "5");
        check("nlistafterselectedid", "1");
        check("nlistafternoopselectedid", "1");
        check("nlistafterselectedslot", "2");

        fs::remove_all(temp_root, ignored);
    }
}
