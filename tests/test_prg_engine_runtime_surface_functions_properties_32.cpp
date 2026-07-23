#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_selection_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_selection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_selection.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oText.Value = 'Copperfin'\n"
            "lDefaultStart = oText.SelStart\n"
            "lDefaultLength = oText.SelLength\n"
            "lDefaultText = oText.SelText\n"
            "lTextHas = PEMSTATUS(oText, 'SelText', 1)\n"
            "lEditHas = PEMSTATUS(oEdit, 'SelStart', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'SelLength', 5)\n"
            "oText.SelStart = 2\n"
            "lStart = oText.SelStart\n"
            "lResetLength = oText.SelLength\n"
            "oText.SelLength = 4\n"
            "lLength = oText.SelLength\n"
            "lSelected = oText.SelText\n"
            "oText.SelText = 'X'\n"
            "lReplacedValue = oText.Value\n"
            "lAfterReplaceStart = oText.SelStart\n"
            "lAfterReplaceLength = oText.SelLength\n"
            "oText.Value = 'abcdef'\n"
            "lSetStart = SETPEM(oText, 'SelStart', 1)\n"
            "lPutLength = PUTPEM(oText, 'SelLength', 2)\n"
            "lGetSelected = GETPEM(oText, 'SelText')\n"
            "lNegative = SETPEM(oText, 'SelLength', -1)\n"
            "lAfterNegative = oText.SelLength\n"
            "lPutText = PUTPEM(oText, 'SelText', 'Q')\n"
            "lPutValue = oText.Value\n"
            "lAddStart = ADDPROPERTY(oText, 'SelStart', 1)\n"
            "lRemoveLength = REMOVEPROPERTY(oText, 'SelLength')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHasStart = .F.\n"
            "lPropHasLength = .F.\n"
            "lPropHasText = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SELSTART'\n"
            "        lPropHasStart = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'SELLENGTH'\n"
            "        lPropHasLength = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'SELTEXT'\n"
            "        lPropHasText = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedSelection')\n"
            "lDerivedStart = oDerived.SelStart\n"
            "lDerivedLength = oDerived.SelLength\n"
            "lDerivedText = oDerived.SelText\n"
            "RETURN\n"
            "DEFINE CLASS DerivedSelection AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Value = 'xyz'\n"
            "        THIS.SelStart = 1\n"
            "        THIS.SelLength = 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox selection script should complete: ") + state.message +
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

        check("ldefaultstart", "0");
        check("ldefaultlength", "0");
        check("ldefaulttext", "");
        check("ltexthas", "true");
        check("ledithas", "true");
        check("lreadonly", "false");
        check("lstart", "2");
        check("lresetlength", "0");
        check("llength", "4");
        check("lselected", "pper");
        check("lreplacedvalue", "CoXfin");
        check("lafterreplacestart", "3");
        check("lafterreplacelength", "0");
        check("lsetstart", "true");
        check("lputlength", "true");
        check("lgetselected", "bc");
        check("lnegative", "false");
        check("lafternegative", "2");
        check("lputtext", "true");
        check("lputvalue", "aQdef");
        check("laddstart", "false");
        check("lremovelength", "false");
        check("lprophasstart", "true");
        check("lprophaslength", "true");
        check("lprophastext", "true");
        check("lderivedstart", "1");
        check("lderivedlength", "1");
        check("lderivedtext", "y");
        expect(state.ole_objects.size() == 3U,
               "native selection coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
