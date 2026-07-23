#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_value_assignment_routes_selection()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_value";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_value.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ListBox')\n"
            "oPlain.AddItem('Alpha')\n"
            "oPlain.AddItem('Beta')\n"
            "oPlain.AddItem('Gamma')\n"
            "lPlainBefore = oPlain.Value\n"
            "oPlain.Value = 'Beta'\n"
            "lPlainDirect = oPlain.Value\n"
            "nPlainDirectIndex = oPlain.ListIndex\n"
            "nPlainDirectItemID = oPlain.ListItemID\n"
            "cPlainDirectDisplay = oPlain.DisplayValue\n"
            "lPlainSetPem = SETPEM(oPlain, 'Value', 'Alpha')\n"
            "lPlainSetPemValue = oPlain.Value\n"
            "nPlainSetPemIndex = oPlain.ListIndex\n"
            "lPlainMissingSetPem = SETPEM(oPlain, 'Value', 'Missing')\n"
            "lPlainAfterMissing = oPlain.Value\n"
            "nPlainAfterMissingIndex = oPlain.ListIndex\n"
            "oMulti = CREATEOBJECT('ListBox')\n"
            "oMulti.ColumnCount = 2\n"
            "oMulti.BoundColumn = 2\n"
            "oMulti.AddListItem('North', 100)\n"
            "oMulti.AddListItem('100', 100, 2)\n"
            "oMulti.AddListItem('South', 200)\n"
            "oMulti.AddListItem('200', 200, 2)\n"
            "nMultiBeforeCount = oMulti.ListCount\n"
            "cMultiBeforeCol2 = oMulti.List(2, 2)\n"
            "oMulti.Value = '200'\n"
            "lMultiValue = oMulti.Value\n"
            "nMultiIndex = oMulti.ListIndex\n"
            "cMultiDisplay = oMulti.DisplayValue\n"
            "oDerived = CREATEOBJECT('DerivedValueList')\n"
            "lDerivedValue = oDerived.Value\n"
            "nDerivedIndex = oDerived.ListIndex\n"
            "RETURN\n"
            "DEFINE CLASS DerivedValueList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.AddItem('South')\n"
            "        THIS.Value = 'South'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native list-control Value script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lplainbefore", "");
        check("lplaindirect", "Beta");
        check("nplaindirectindex", "2");
        check("nplaindirectitemid", "2");
        check("cplaindirectdisplay", "Beta");
        check("lplainsetpem", "true");
        check("lplainsetpemvalue", "Alpha");
        check("nplainsetpemindex", "1");
        check("lplainmissingsetpem", "false");
        check("lplainaftermissing", "Alpha");
        check("nplainaftermissingindex", "1");
        check("nmultibeforecount", "2");
        check("cmultibeforecol2", "200");
        check("lmultivalue", "200");
        check("nmultiindex", "2");
        check("cmultidisplay", "South");
        check("lderivedvalue", "South");
        check("nderivedindex", "2");

        expect(state.ole_objects.size() == 3U,
               "native Value coverage should register plain, multicolumn, and derived list controls");
        fs::remove_all(temp_root, ignored);
    }
}
