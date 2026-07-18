#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_resettodefault_builtin_fallback_restores_inherited_defaults()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_resettodefault_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_resettodefault_builtin.prg";
        write_text(
            main_path,
            "oListener = CREATEOBJECT('ReportListenerShim')\n"
            "oListener.cTextFile = 'override.txt'\n"
            "oListener.nPageCount = 9\n"
            "lResetText = oListener.ResetToDefault('cTextFile')\n"
            "lResetPage = oListener.ResetToDefault('nPageCount')\n"
            "cTextAfter = oListener.cTextFile\n"
            "nPageAfter = oListener.nPageCount\n"
            "RETURN\n"
            "DEFINE CLASS BaseListener AS Custom\n"
            "    cTextFile = 'seed.txt'\n"
            "    nPageCount = 4\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ReportListenerShim AS BaseListener\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ResetToDefault builtin fallback script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto reset_text = state.globals.find("lresettext");
        expect(reset_text != state.globals.end(),
               "native ResetToDefault builtin fallback script should preserve first reset result");
        if (reset_text != state.globals.end())
        {
            expect(copperfin::runtime::format_value(reset_text->second) == "true",
                   "native ResetToDefault builtin fallback should report success for inherited text defaults");
        }

        const auto reset_page = state.globals.find("lresetpage");
        expect(reset_page != state.globals.end(),
               "native ResetToDefault builtin fallback script should preserve second reset result");
        if (reset_page != state.globals.end())
        {
            expect(copperfin::runtime::format_value(reset_page->second) == "true",
                   "native ResetToDefault builtin fallback should report success for inherited numeric defaults");
        }

        const auto text_after = state.globals.find("ctextafter");
        expect(text_after != state.globals.end(),
               "native ResetToDefault builtin fallback script should preserve restored text value");
        if (text_after != state.globals.end())
        {
            expect(copperfin::runtime::format_value(text_after->second) == "seed.txt",
                   "native ResetToDefault builtin fallback should restore inherited text defaults");
        }

        const auto page_after = state.globals.find("npageafter");
        expect(page_after != state.globals.end(),
               "native ResetToDefault builtin fallback script should preserve restored numeric value");
        if (page_after != state.globals.end())
        {
            expect(copperfin::runtime::format_value(page_after->second) == "4",
                   "native ResetToDefault builtin fallback should restore inherited numeric defaults");
        }

        const std::size_t reset_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.resettodefault";
            }));
        expect(reset_event_count == 2U,
               "native ResetToDefault builtin fallback should emit one event per representative reset call");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_resettodefault_override_wins_over_builtin_default_restore()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_resettodefault_override";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_resettodefault_override.prg";
        write_text(
            main_path,
            "oThing = CREATEOBJECT('ResetBox')\n"
            "oThing.cValue = 'changed'\n"
            "oThing.ResetToDefault('cValue')\n"
            "lResetRan = oThing.lResetRan\n"
            "cValueAfter = oThing.cValue\n"
            "RETURN\n"
            "DEFINE CLASS ResetBox AS Custom\n"
            "    cValue = 'seed'\n"
            "    lResetRan = .F.\n"
            "    PROCEDURE ResetToDefault\n"
            "        LPARAMETERS tcProperty\n"
            "        THIS.lResetRan = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ResetToDefault override script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto reset_ran = state.globals.find("lresetran");
        expect(reset_ran != state.globals.end(),
               "native ResetToDefault override script should preserve override flag");
        if (reset_ran != state.globals.end())
        {
            expect(copperfin::runtime::format_value(reset_ran->second) == "true",
                   "native ResetToDefault override should still invoke the class-defined method");
        }

        const auto value_after = state.globals.find("cvalueafter");
        expect(value_after != state.globals.end(),
               "native ResetToDefault override script should preserve post-call property state");
        if (value_after != state.globals.end())
        {
            expect(copperfin::runtime::format_value(value_after->second) == "changed",
                   "native ResetToDefault override should not fall through to the builtin default restore");
        }

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ResetBox.ResetToDefault";
        });
        expect(has_invoke_event,
               "native ResetToDefault override should emit a prg.object.invoke event");

        const bool has_builtin_reset_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.resettodefault";
        });
        expect(!has_builtin_reset_event,
               "native ResetToDefault override should not emit the builtin prg.object.resettodefault event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_readexpression_returns_live_property_expression_text()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_readexpression";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_readexpression.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('ProbeWidget')\n"
            "cClassTextExpr = oWidget.ReadExpression('cText')\n"
            "cClassCountExpr = oWidget.ReadExpression('nCount')\n"
            "lHasReadExpression = PEMSTATUS(oWidget, 'ReadExpression', 1)\n"
            "lReadExpressionReadOnly = PEMSTATUS(oWidget, 'ReadExpression', 5)\n"
            "lGetReadExpression = GETPEM(oWidget, 'ReadExpression')\n"
            "nMethods = AMEMBERS(aMethods, oWidget, 2)\n"
            "nHasReadExpression = ASCAN(aMethods, 'READEXPRESSION')\n"
            "oWidget.cText = LOWER(\"BETA\")\n"
            "oWidget.nCount = 2 + 5\n"
            "cRuntimeTextExpr = oWidget.ReadExpression('cText')\n"
            "cRuntimeCountExpr = oWidget.ReadExpression('nCount')\n"
            "cMissingExpr = oWidget.ReadExpression('missingProperty')\n"
            "RETURN\n"
            "DEFINE CLASS ProbeWidget AS Custom\n"
            "    cText = UPPER(\"alpha\")\n"
            "    nCount = 1 + 2\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ReadExpression script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("cclasstextexpr", "UPPER(\"alpha\")");
        check("cclasscountexpr", "1 + 2");
        check("lhasreadexpression", "true");
        check("lreadexpressionreadonly", "false");
        check("lgetreadexpression", "true");
        check("cruntimetextexpr", "LOWER(\"BETA\")");
        check("cruntimecountexpr", "2 + 5");
        check("cmissingexpr", "");

        const auto has_readexpression = state.globals.find("nhasreadexpression");
        expect(has_readexpression != state.globals.end(),
               "native ReadExpression reflection script should preserve AMEMBERS() presence");
        if (has_readexpression != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_readexpression->second) != "0",
                   "AMEMBERS(..., 2) should expose the shipped native ReadExpression builtin");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_readmethod_returns_class_method_source_text()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_readmethod";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_readmethod.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('ProbeWidget')\n"
            "cPingMethod = oWidget.ReadMethod('Ping')\n"
            "cDescribeMethod = oWidget.ReadMethod('Describe')\n"
            "lHasReadMethod = PEMSTATUS(oWidget, 'ReadMethod', 1)\n"
            "lReadMethodReadOnly = PEMSTATUS(oWidget, 'ReadMethod', 5)\n"
            "lGetReadMethod = GETPEM(oWidget, 'ReadMethod')\n"
            "nMethods = AMEMBERS(aMethods, oWidget, 2)\n"
            "nHasReadMethod = ASCAN(aMethods, 'READMETHOD')\n"
            "cMissingMethod = oWidget.ReadMethod('MissingMethod')\n"
            "RETURN\n"
            "DEFINE CLASS BaseWidget AS Custom\n"
            "PROCEDURE Describe\n"
            "LPARAMETERS tcPrefix\n"
            "RETURN tcPrefix + '-base'\n"
            "ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ProbeWidget AS BaseWidget\n"
            "PROCEDURE Ping\n"
            "LOCAL lcValue\n"
            "lcValue = 'alpha'\n"
            "RETURN UPPER(lcValue)\n"
            "ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ReadMethod script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("cpingmethod", "LOCAL lcValue\nlcValue = 'alpha'\nRETURN UPPER(lcValue)");
        check("cdescribemethod", "LPARAMETERS tcPrefix\nRETURN tcPrefix + '-base'");
        check("lhasreadmethod", "true");
        check("lreadmethodreadonly", "false");
        check("lgetreadmethod", "true");
        check("cmissingmethod", "");

        const auto has_readmethod = state.globals.find("nhasreadmethod");
        expect(has_readmethod != state.globals.end(),
               "native ReadMethod reflection script should preserve AMEMBERS() presence");
        if (has_readmethod != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_readmethod->second) != "0",
                   "AMEMBERS(..., 2) should expose the shipped native ReadMethod builtin");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_writeexpression_updates_live_property_values_and_preserves_expression_text()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_writeexpression";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_writeexpression.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('ProbeWidget')\n"
            "lHasWriteExpression = PEMSTATUS(oWidget, 'WriteExpression', 1)\n"
            "lWriteExpressionReadOnly = PEMSTATUS(oWidget, 'WriteExpression', 5)\n"
            "lGetWriteExpression = GETPEM(oWidget, 'WriteExpression')\n"
            "nMethods = AMEMBERS(aMethods, oWidget, 2)\n"
            "nHasWriteExpression = ASCAN(aMethods, 'WRITEEXPRESSION')\n"
            "oWidget.WriteExpression('cText', 'LOWER(\"BETA\")')\n"
            "oWidget.WriteExpression('Caption', 'UPPER(\"delta\")')\n"
            "cTextAfter = oWidget.cText\n"
            "cCaptionAfter = oWidget.Caption\n"
            "cTextExpr = oWidget.ReadExpression('cText')\n"
            "cCaptionExpr = oWidget.ReadExpression('Caption')\n"
            "cBackingAfter = oWidget.cBacking\n"
            "cMissingExpr = oWidget.ReadExpression('MissingProperty')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'seed'\n"
            "    Caption = 'parent'\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "    PROCEDURE Caption_Assign\n"
            "        LPARAMETERS tcValue\n"
            "        THIS.cBacking = tcValue + ':S'\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ProbeWidget AS ParentWidget\n"
            "    cText = UPPER(\"alpha\")\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native WriteExpression script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("lhaswriteexpression", "true");
        check("lwriteexpressionreadonly", "false");
        check("lgetwriteexpression", "true");
        check("ctextafter", "beta");
        check("ccaptionafter", "DELTA:S:A");
        check("ctextexpr", "LOWER(\"BETA\")");
        check("ccaptionexpr", "UPPER(\"delta\")");
        check("cbackingafter", "DELTA:S");
        check("cmissingexpr", "");

        const auto has_writeexpression = state.globals.find("nhaswriteexpression");
        expect(has_writeexpression != state.globals.end(),
               "native WriteExpression reflection script should preserve AMEMBERS() presence");
        if (has_writeexpression != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_writeexpression->second) != "0",
                   "AMEMBERS(..., 2) should expose the shipped native WriteExpression builtin");
        }

        const bool has_assign_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentWidget.Caption_Assign";
        });
        expect(has_assign_invoke_event,
               "native WriteExpression should route accessor-backed writes through the existing assigner path");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_writemethod_updates_existing_method_body_and_invocation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_writemethod";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_writemethod.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('ProbeWidget')\n"
            "lHasWriteMethod = PEMSTATUS(oWidget, 'WriteMethod', 1)\n"
            "lWriteMethodReadOnly = PEMSTATUS(oWidget, 'WriteMethod', 5)\n"
            "lGetWriteMethod = GETPEM(oWidget, 'WriteMethod')\n"
            "nMethods = AMEMBERS(aMethods, oWidget, 2)\n"
            "nHasWriteMethod = ASCAN(aMethods, 'WRITEMETHOD')\n"
            "cBeforePing = oWidget.Ping()\n"
            "cBeforeDescribe = oWidget.Describe()\n"
            "oWidget.WriteMethod('Ping', 'RETURN ''updated''')\n"
            "oWidget.WriteMethod('Describe', 'RETURN ''override''')\n"
            "cAfterPing = oWidget.Ping()\n"
            "cAfterDescribe = oWidget.Describe()\n"
            "cPingMethod = oWidget.ReadMethod('Ping')\n"
            "cDescribeMethod = oWidget.ReadMethod('Describe')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    FUNCTION Describe\n"
            "        RETURN 'base'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ProbeWidget AS ParentWidget\n"
            "    FUNCTION Ping\n"
            "        RETURN 'original'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native WriteMethod script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("lhaswritemethod", "true");
        check("lwritemethodreadonly", "false");
        check("lgetwritemethod", "true");
        check("cbeforeping", "original");
        check("cbeforedescribe", "base");
        check("cafterping", "updated");
        check("cafterdescribe", "override");
        check("cpingmethod", "RETURN 'updated'");
        check("cdescribemethod", "RETURN 'override'");

        const auto has_writemethod = state.globals.find("nhaswritemethod");
        expect(has_writemethod != state.globals.end(),
               "native WriteMethod reflection script should preserve AMEMBERS() presence");
        if (has_writemethod != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_writemethod->second) != "0",
                   "AMEMBERS(..., 2) should expose the shipped native WriteMethod builtin");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_writemethod_creates_missing_method_when_create_flag_is_true()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root =
            fs::temp_directory_path() / "copperfin_native_prg_writemethod_create";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_writemethod_create.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('ProbeWidget')\n"
            "cMissingBefore = oWidget.ReadMethod('Ping')\n"
            "cBaseDescribeBefore = oWidget.Describe()\n"
            "oWidget.WriteMethod('Ping', 'LPARAMETERS tcPrefix' + CHR(10) + 'RETURN tcPrefix + ''-created''', .T., 2, 'runtime-only')\n"
            "oWidget.WriteMethod('Describe', 'RETURN ''derived''', .T.)\n"
            "cPingMethod = oWidget.ReadMethod('Ping')\n"
            "cDescribeMethod = oWidget.ReadMethod('Describe')\n"
            "cAfterPing = oWidget.Ping('alpha')\n"
            "cAfterDescribe = oWidget.Describe()\n"
            "nMethods = AMEMBERS(aMethods, oWidget, 2)\n"
            "nHasPing = ASCAN(aMethods, 'PING')\n"
            "nHasDescribe = ASCAN(aMethods, 'DESCRIBE')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    FUNCTION Describe\n"
            "        RETURN 'base'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ProbeWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native WriteMethod create script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("cmissingbefore", "");
        check("cbasedescribebefore", "base");
        check("cpingmethod", "LPARAMETERS tcPrefix\nRETURN tcPrefix + '-created'");
        check("cdescribemethod", "RETURN 'derived'");
        check("cafterping", "alpha-created");
        check("cafterdescribe", "derived");

        const auto has_ping = state.globals.find("nhasping");
        expect(has_ping != state.globals.end(),
               "native WriteMethod create script should preserve AMEMBERS() presence for Ping");
        if (has_ping != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_ping->second) != "0",
                   "AMEMBERS(..., 2) should expose created Ping method");
        }

        const auto has_describe = state.globals.find("nhasdescribe");
        expect(has_describe != state.globals.end(),
               "native WriteMethod create script should preserve AMEMBERS() presence for Describe");
        if (has_describe != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_describe->second) != "0",
                   "AMEMBERS(..., 2) should expose derived Describe override");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_writemethod_missing_method_without_create_flag_remains_noop()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root =
            fs::temp_directory_path() / "copperfin_native_prg_writemethod_no_create";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_writemethod_no_create.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('ProbeWidget')\n"
            "oWidget.WriteMethod('Ping', 'RETURN ''created''')\n"
            "oWidget.WriteMethod('Pong', 'RETURN ''created''', .F.)\n"
            "cPingMethod = oWidget.ReadMethod('Ping')\n"
            "cPongMethod = oWidget.ReadMethod('Pong')\n"
            "nMethods = AMEMBERS(aMethods, oWidget, 2)\n"
            "nHasPing = ASCAN(aMethods, 'PING')\n"
            "nHasPong = ASCAN(aMethods, 'PONG')\n"
            "RETURN\n"
            "DEFINE CLASS ProbeWidget AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native WriteMethod no-create script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("cpingmethod", "");
        check("cpongmethod", "");
        check("nhasping", "0");
        check("nhaspong", "0");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_builtin_methods_reflect_through_pemstatus_getpem_and_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_builtin_method_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_builtin_method_reflection.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lHasRelease = PEMSTATUS(oForm, 'Release', 1)\n"
            "lHasRefresh = PEMSTATUS(oForm, 'Refresh', 1)\n"
            "lHasMove = PEMSTATUS(oForm, 'Move', 1)\n"
            "lHasShow = PEMSTATUS(oForm, 'Show', 1)\n"
            "lHasHide = PEMSTATUS(oForm, 'Hide', 1)\n"
            "lHasReset = PEMSTATUS(oForm, 'ResetToDefault', 1)\n"
            "lHasTextSetFocus = PEMSTATUS(oForm.txtName, 'SetFocus', 1)\n"
            "lReleaseReadOnly = PEMSTATUS(oForm, 'Release', 5)\n"
            "lMoveReadOnly = PEMSTATUS(oForm, 'Move', 5)\n"
            "lShowReadOnly = PEMSTATUS(oForm, 'Show', 5)\n"
            "lResetReadOnly = PEMSTATUS(oForm, 'ResetToDefault', 5)\n"
            "lTextSetFocusReadOnly = PEMSTATUS(oForm.txtName, 'SetFocus', 5)\n"
            "lGetRelease = GETPEM(oForm, 'Release')\n"
            "lGetMove = GETPEM(oForm, 'Move')\n"
            "lGetShow = GETPEM(oForm, 'Show')\n"
            "lGetReset = GETPEM(oForm, 'ResetToDefault')\n"
            "lGetTextSetFocus = GETPEM(oForm.txtName, 'SetFocus')\n"
            "nFormMethods = AMEMBERS(aFormMethods, oForm, 2)\n"
            "nFormUnion = AMEMBERS(aFormUnion, oForm, 3)\n"
            "nTextMethods = AMEMBERS(aTextMethods, oForm.txtName, 2)\n"
            "nFormHasRelease = ASCAN(aFormMethods, 'RELEASE')\n"
            "nFormHasMove = ASCAN(aFormMethods, 'MOVE')\n"
            "nFormHasRefresh = ASCAN(aFormMethods, 'REFRESH')\n"
            "nFormHasShow = ASCAN(aFormMethods, 'SHOW')\n"
            "nFormHasHide = ASCAN(aFormMethods, 'HIDE')\n"
            "nFormUnionHasReset = ASCAN(aFormUnion, 'RESETTODEFAULT')\n"
            "nTextHasSetFocus = ASCAN(aTextMethods, 'SETFOCUS')\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT txtName AS TextBox\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native builtin-method reflection script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("lhasrelease", "true");
        check("lhasrefresh", "true");
        check("lhasmove", "true");
        check("lhasshow", "true");
        check("lhashide", "true");
        check("lhasreset", "true");
        check("lhastextsetfocus", "true");
        check("lreleasereadonly", "false");
        check("lmovereadonly", "false");
        check("lshowreadonly", "false");
        check("lresetreadonly", "false");
        check("ltextsetfocusreadonly", "false");
        check("lgetrelease", "true");
        check("lgetmove", "true");
        check("lgetshow", "true");
        check("lgetreset", "true");
        check("lgettextsetfocus", "true");

        const auto expect_positive = [&](const std::string &name, const std::string &message)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) != "0",
                       message + " expected a positive ASCAN() result");
            }
        };

        expect_positive("nformhasrelease",
                        "AMEMBERS(..., 2) should expose the shipped native Release builtin");
        expect_positive("nformhasmove",
                        "AMEMBERS(..., 2) should expose the shipped native Move builtin");
        expect_positive("nformhasrefresh",
                        "AMEMBERS(..., 2) should expose the shipped native Refresh builtin");
        expect_positive("nformhasshow",
                        "AMEMBERS(..., 2) should expose the shipped native Show builtin");
        expect_positive("nformhashide",
                        "AMEMBERS(..., 2) should expose the shipped native Hide builtin");
        expect_positive("nformunionhasreset",
                        "AMEMBERS(..., 3) should expose the shipped native ResetToDefault builtin");
        expect_positive("ntexthassetfocus",
                        "AMEMBERS(..., 2) should expose the shipped native SetFocus builtin");

        fs::remove_all(temp_root, ignored);
    }

}
