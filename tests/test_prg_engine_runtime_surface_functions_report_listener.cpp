#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_reportlistener_getconfigtable_resolves_existing_casefolded_table_only_for_report_listeners()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
            "copperfin_reportlistener_getconfigtable";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path config_path = temp_root / "_reportoutputconfig.dbf";
        write_text(config_path, "fixture");
        const fs::path main_path = temp_root / "reportlistener_config.prg";
        write_text(
            main_path,
            "oViaDo = NULL\n"
            "DO GetConfigObject WITH m.oViaDo\n"
            "cViaDoType = VARTYPE(oViaDo)\n"
            "oLocalListener = makeobject()\n"
            "cLocalDoType = VARTYPE(oLocalListener)\n"
            "cLocalDoConfig = oLocalListener.GetConfigTable()\n"
            "oListener = CREATEOBJECT('ReportListenerShim')\n"
            "cConfig = oListener.GetConfigTable()\n"
            "cProperty = oListener.ConfigurationTable\n"
            "oPlain = CREATEOBJECT('PlainShim')\n"
            "cPlainConfig = oPlain.GetConfigTable()\n"
            "cPlainProperty = oPlain.ConfigurationTable\n"
            "RETURN\n"
            "DEFINE CLASS ReportListenerShim AS ReportListener\n"
            "    PROCEDURE Init\n"
            "        IF DODEFAULT()\n"
            "            RETURN NOT THIS.HadError\n"
            "        ENDIF\n"
            "        RETURN .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PlainShim AS Custom\n"
            "ENDDEFINE\n"
            "PROCEDURE GetConfigObject(m.toCfg, m.tXML)\n"
            "    LOCAL m.lcModule\n"
            "    m.lcModule = ''\n"
            "    IF m.tXML\n"
            "        m.toCfg = CREATEOBJECT('PlainShim')\n"
            "    ELSE\n"
            "        m.toCfg = NEWOBJECT('ReportListenerShim', 'reportlistener_config.prg', m.lcModule)\n"
            "    ENDIF\n"
            "    IF VARTYPE(toCfg) = 'O'\n"
            "        m.toCfg.Name = 'configured'\n"
            "    ENDIF\n"
            "RETURN\n"
            "FUNCTION makeobject\n"
            "    LOCAL m.oLocal\n"
            "    m.oLocal = NULL\n"
            "    DO GetConfigObject WITH m.oLocal\n"
            "    RETURN m.oLocal\n"
            "ENDFUNC\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        std::string event_summary;
        for (const auto &event : state.events)
        {
            if (!event_summary.empty())
            {
                event_summary += "; ";
            }
            event_summary += event.category + "=" + event.detail;
        }
        expect(state.completed,
               std::string("ReportListener GetConfigTable script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line) + " events=" + event_summary);

        const auto config = state.globals.find("cconfig");
        const auto via_do_type = state.globals.find("cviadotype");
        const auto local_do_type = state.globals.find("clocaldotype");
        const auto local_do_config = state.globals.find("clocaldoconfig");
        const auto property = state.globals.find("cproperty");
        const auto plain_config = state.globals.find("cplainconfig");
        const auto plain_property = state.globals.find("cplainproperty");
        expect(config != state.globals.end(), "ReportListener GetConfigTable result should be present");
        std::string global_names;
        for (const auto &[name, value] : state.globals)
        {
            if (!global_names.empty())
            {
                global_names += ",";
            }
            global_names += name + "=" + copperfin::runtime::format_value(value);
        }
        expect(via_do_type != state.globals.end(),
               "DO object assignment type result should be present; globals=" + global_names);
        if (via_do_type != state.globals.end())
        {
            expect(copperfin::runtime::format_value(via_do_type->second) == "O",
                   "DO WITH object assignment should write back an object reference");
        }
        expect(local_do_type != state.globals.end(), "local DO object assignment type result should be present");
        if (local_do_type != state.globals.end())
        {
            expect(copperfin::runtime::format_value(local_do_type->second) == "O",
                   "DO WITH object assignment should write back into a caller local");
        }
        expect(local_do_config != state.globals.end(), "local DO object configuration result should be present");
        if (local_do_config != state.globals.end())
        {
            expect(fs::weakly_canonical(fs::path(copperfin::runtime::format_value(local_do_config->second))) ==
                       fs::weakly_canonical(config_path),
                   "local DO object assignment should preserve the ReportListener reference");
        }
        expect(property != state.globals.end(), "ReportListener ConfigurationTable should be present");
        expect(plain_config != state.globals.end(), "non-ReportListener method result should be present");
        expect(plain_property != state.globals.end(), "non-ReportListener ConfigurationTable result should be present");
        if (config != state.globals.end() && property != state.globals.end())
        {
            const fs::path expected = fs::weakly_canonical(config_path);
            expect(fs::weakly_canonical(fs::path(copperfin::runtime::format_value(config->second))) == expected,
                   "ReportListener GetConfigTable should resolve the case-folded existing table");
            expect(fs::weakly_canonical(fs::path(copperfin::runtime::format_value(property->second))) == expected,
                   "ReportListener GetConfigTable should update ConfigurationTable");
        }
        if (plain_config != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_config->second) !=
                       config_path.string(),
                   "non-ReportListener objects should not resolve the report configuration table");
        }
        if (plain_property != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_property->second) !=
                       config_path.string(),
                   "non-ReportListener objects should not receive ConfigurationTable state; got '" +
                       copperfin::runtime::format_value(plain_property->second) + "'");
        }

        fs::remove_all(temp_root, ignored);
    }
}
