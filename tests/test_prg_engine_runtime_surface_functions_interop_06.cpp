#include "test_prg_engine_runtime_surface_functions_support.h"
#include "copperfin/platform/invariant_numeric.h"

namespace copperfin::runtime_surface_tests
{
    void test_lookup_expression_function_supports_sql_cursors()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_sql";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}});

        const fs::path main_path = temp_root / "lookup_sql_test.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "nConn = SQLCONNECT('dsn=Northwind')\n"
            "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
            "SELECT people\n"
            "nSelectedBefore = SELECT()\n"
            "cAliasBefore = ALIAS()\n"
            "nLookupAmount = LOOKUP(sqlcust.AMOUNT, 'BRAVO', 'sqlcust', 'NAME')\n"
            "lLookupMissing = LOOKUP(sqlcust.NAME, 'ZZZZ', 'sqlcust', 'NAME')\n"
            "nSqlRec = RECNO('sqlcust')\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "lDisc = SQLDISCONNECT(nConn)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SQL LOOKUP test script should complete");
        expect(state.sql_connections.empty(), "SQL LOOKUP test should disconnect its SQL handle");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("nselectedbefore", "1");
        check("caliasbefore", "people");
        check("nlookupamount", "20");
        check("llookupmissing", "false");
        check("nselectedafter", "1");
        check("caliasafter", "people");
        check("ldisc", "1");
        {
            const auto it = state.globals.find("nsqlrec");
            expect(it != state.globals.end(), "nsqlrec from SQL LOOKUP should be set");
            if (it != state.globals.end())
            {
                const double recno = copperfin::platform::try_parse_invariant_double(
                                          copperfin::runtime::format_value(it->second))
                                          .value_or(0.0);
                expect(recno > 0.0,
                       "SQL LOOKUP should leave the targeted SQL cursor on a found record");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_supports_macro_alias_and_tag_arguments()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_target_context";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        const fs::path people_cdx  = temp_root / "people.cdx";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}, {"CAROL", 35}});
        write_synthetic_cdx(people_cdx, "NAME", "UPPER(NAME)");

        const fs::path main_path = temp_root / "lookup_macro_arguments.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "USE '" + people_path.string() + "' ALIAS other AGAIN IN 0\n"
            "SELECT other\n"
            "SET ORDER TO TAG NAME IN people\n"
            "cAlias = 'people'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "cFound = LOOKUP(people.NAME, 'BOB', cAlias, cTag)\n"
            "cFoundNested = LOOKUP(people.NAME, 'BOB', &cAliasHolder, &cTagHolder)\n"
            "cFoundSecondHop = LOOKUP(people.NAME, 'BOB', &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "nPeopleRec = RECNO('people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "LOOKUP macro-argument test should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("cfound", "BOB");
        check("cfoundnested", "BOB");
        check("cfoundsecondhop", "BOB");
        check("nselectedafter", "2");
        check("caliasafter", "other");
        check("npeoplerec", "2");

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_supports_macro_return_and_search_expressions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_macro_exprs";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        const fs::path people_cdx  = temp_root / "people.cdx";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}, {"CAROL", 35}});
        write_synthetic_cdx(people_cdx, "NAME", "UPPER(NAME)");

        const fs::path main_path = temp_root / "lookup_macro_exprs.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "USE '" + people_path.string() + "' ALIAS other AGAIN IN 0\n"
            "SELECT other\n"
            "SET ORDER TO TAG NAME IN people\n"
            "cAlias = 'people'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "cReturnExpr = 'people.NAME'\n"
            "cReturnExprHolder = 'cReturnExpr'\n"
            "cReturnExprDeepHolder = 'cReturnExprHolder'\n"
            "cAgeExpr = 'people.AGE'\n"
            "cAgeExprHolder = 'cAgeExpr'\n"
            "cAgeExprDeepHolder = 'cAgeExprHolder'\n"
            "cSearchExpr = 'BOB'\n"
            "cSearchExprHolder = 'cSearchExpr'\n"
            "cSearchExprDeepHolder = 'cSearchExprHolder'\n"
            "cFound = LOOKUP(&cReturnExpr, &cSearchExpr, cAlias, cTag)\n"
            "cFoundNested = LOOKUP(&cReturnExprHolder, &cSearchExprHolder, cAlias, cTag)\n"
            "cFoundSecondHop = LOOKUP(&cReturnExprDeepHolder, &cSearchExprDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nFoundAge = LOOKUP(&cAgeExprDeepHolder, &cSearchExprDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "nPeopleRec = RECNO('people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "LOOKUP macro return/search-expression test should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("cfound", "BOB");
        check("cfoundnested", "BOB");
        check("cfoundsecondhop", "BOB");
        check("nfoundage", "25");
        check("nselectedafter", "2");
        check("caliasafter", "other");
        check("npeoplerec", "2");

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_supports_macro_arguments_on_sql_cursors()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_sql_macro_exprs";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}});

        const fs::path main_path = temp_root / "lookup_sql_macro_exprs.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "nConn = SQLCONNECT('dsn=Northwind')\n"
            "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
            "SELECT people\n"
            "cAlias = 'sqlcust'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "cReturnExpr = 'sqlcust.AMOUNT'\n"
            "cReturnExprHolder = 'cReturnExpr'\n"
            "cReturnExprDeepHolder = 'cReturnExprHolder'\n"
            "cSearchExpr = 'BRAVO'\n"
            "cSearchExprHolder = 'cSearchExpr'\n"
            "cSearchExprDeepHolder = 'cSearchExprHolder'\n"
            "cMissingSearch = 'ZZZZ'\n"
            "cMissingSearchHolder = 'cMissingSearch'\n"
            "cMissingSearchDeepHolder = 'cMissingSearchHolder'\n"
            "nLookupAmount = LOOKUP(&cReturnExpr, &cSearchExpr, &cAlias, &cTag)\n"
            "nLookupAmountNested = LOOKUP(&cReturnExpr, &cSearchExpr, &cAliasHolder, &cTagHolder)\n"
            "nLookupAmountSecondHop = LOOKUP(&cReturnExpr, &cSearchExpr, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nLookupAmountExprSecondHop = LOOKUP(&cReturnExprDeepHolder, &cSearchExprDeepHolder, &cAlias, &cTag)\n"
            "nLookupAmountAllSecondHop = LOOKUP(&cReturnExprDeepHolder, &cSearchExprDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nLookupAmountMissAllSecondHop = LOOKUP(&cReturnExprDeepHolder, &cMissingSearchDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nSqlRec = RECNO('sqlcust')\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "lDisc = SQLDISCONNECT(nConn)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SQL LOOKUP macro-argument test should complete");
        expect(state.sql_connections.empty(), "SQL LOOKUP macro-argument test should disconnect its SQL handle");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("nlookupamount", "20");
        check("nlookupamountnested", "20");
        check("nlookupamountsecondhop", "20");
        check("nlookupamountexprsecondhop", "20");
        check("nlookupamountallsecondhop", "20");
        check("nlookupamountmissallsecondhop", "0");
        check("nselectedafter", "1");
        check("caliasafter", "people");
        check("ldisc", "1");
        {
            const auto it = state.globals.find("nsqlrec");
            expect(it != state.globals.end(), "nsqlrec from SQL LOOKUP macro-argument test should be set");
            if (it != state.globals.end())
            {
                const double recno = copperfin::platform::try_parse_invariant_double(
                                          copperfin::runtime::format_value(it->second))
                                          .value_or(0.0);
                expect(recno > 0.0,
                       "SQL LOOKUP with macro-expanded arguments should leave the targeted SQL cursor on a found record");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_macro_return_expressions_preserve_typed_defaults_on_miss()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_macro_miss_defaults";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        const fs::path people_cdx  = temp_root / "people.cdx";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}, {"CAROL", 35}});
        write_synthetic_cdx(people_cdx, "NAME", "UPPER(NAME)");

        const fs::path main_path = temp_root / "lookup_macro_miss_defaults.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "SET ORDER TO TAG NAME IN people\n"
            "cNumberExpr = 'AGE + 0'\n"
            "cNumberExprHolder = 'cNumberExpr'\n"
            "cNumberExprDeepHolder = 'cNumberExprHolder'\n"
            "cTextExpr = 'LEFT(NAME, 4)'\n"
            "cTextExprHolder = 'cTextExpr'\n"
            "cTextExprDeepHolder = 'cTextExprHolder'\n"
            "cMissingSearch = 'ZZZZ'\n"
            "cMissingSearchHolder = 'cMissingSearch'\n"
            "cMissingSearchDeepHolder = 'cMissingSearchHolder'\n"
            "cAlias = 'people'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "nMissing = LOOKUP(&cNumberExpr, &cMissingSearch, 'people', 'NAME')\n"
            "lMissing = LOOKUP(&cTextExpr, &cMissingSearch, 'people', 'NAME')\n"
            "nMissingSecondHop = LOOKUP(&cNumberExprDeepHolder, &cMissingSearchDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "lMissingSecondHop = LOOKUP(&cTextExprDeepHolder, &cMissingSearchDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "LOOKUP macro miss-default test should complete: " + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("nmissing", "0");
        check("lmissing", "false");
        check("nmissingsecondhop", "0");
        check("lmissingsecondhop", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_property_assignment_round_trips()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_ole_property_roundtrip";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "ole_property_roundtrip.prg";
        write_text(
            main_path,
            "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
            "oDict.CompareMode = 1\n"
            "nCompareMode = oDict.CompareMode\n"
            "oDict.Caption = 'Copperfin'\n"
            "cCaption = oDict.Caption\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "OLE property round-trip script should complete");

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

        check("ncomparemode", "1");
        check("ccaption", "Copperfin");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.set"; }),
               "OLE property round-trip should emit ole.set events");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.get"; }),
               "OLE property round-trip should emit ole.get events");

        fs::remove_all(temp_root, ignored);
    }

    void test_scripting_dictionary_collection_methods()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_scripting_dictionary_methods";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "scripting_dictionary_methods.prg";
        write_text(
            main_path,
            "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
            "oDict.Add('Alpha', 41)\n"
            "lExistsBefore = oDict.Exists('Alpha')\n"
            "nItem = oDict.Item('Alpha')\n"
            "nCountAfterAdd = oDict.Count\n"
            "oDict.Remove('Alpha')\n"
            "lExistsAfterRemove = oDict.Exists('Alpha')\n"
            "oDict.Add('Beta', 5)\n"
            "oDict.Add('Gamma', 7)\n"
            "nCountBeforeClear = oDict.Count\n"
            "oDict.RemoveAll()\n"
            "nCountAfterClear = oDict.Count\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "Scripting.Dictionary method script should complete");

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

        check("lexistsbefore", "true");
        check("nitem", "41");
        check("ncountafteradd", "1");
        check("lexistsafterremove", "false");
        check("ncountbeforeclear", "2");
        check("ncountafterclear", "0");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.invoke"; }),
               "Scripting.Dictionary methods should emit ole.invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_preserves_library_and_server_targeting()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_server_targeting";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "newobject_server_targeting.prg";
        write_text(
            main_path,
            "oRemote = NEWOBJECT('Session', 'app.vcx', '', '', .F., 'AppServer01')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "NEWOBJECT server-targeting script should complete");
        expect(state.ole_objects.size() == 1U, "NEWOBJECT server-targeting script should register one object");
        if (!state.ole_objects.empty())
        {
            expect(state.ole_objects.front().prog_id == "Session",
                   "NEWOBJECT should preserve the requested class name as the OLE prog_id");
            expect(state.ole_objects.front().source == "app.vcx@AppServer01",
                   "NEWOBJECT should preserve library/server targeting metadata in object source");
        }
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
                   return event.category == "ole.newobject" &&
                          event.detail == "Session:app.vcx@AppServer01";
               }),
               "NEWOBJECT should emit library/server detail in ole.newobject events");

        fs::remove_all(temp_root, ignored);
    }

    void test_getobject_reuses_existing_class_and_source_targets()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_getobject_reuse";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "getobject_reuse.prg";
        write_text(
            main_path,
            "oWord1 = CREATEOBJECT('Word.Application')\n"
            "oWord2 = GETOBJECT('', 'Word.Application')\n"
            "lSameRunning = COMPOBJ(oWord1, oWord2)\n"
            "oDoc1 = GETOBJECT('sample.doc', 'Word.Application')\n"
            "oDoc2 = GETOBJECT('sample.doc', 'Word.Application')\n"
            "lSameDocument = COMPOBJ(oDoc1, oDoc2)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "GETOBJECT reuse script should complete");

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

        check("lsamerunning", "true");
        check("lsamedocument", "true");
        expect(state.ole_objects.size() == 2U,
               "GETOBJECT reuse should attach to existing targets instead of registering duplicate objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "Word.Application",
                   "GETOBJECT class-target reuse should preserve the requested class");
            expect(state.ole_objects[1].prog_id == "Word.Application",
                   "GETOBJECT file/class activation should preserve the requested class");
            expect(state.ole_objects[1].source == "sample.doc",
                   "GETOBJECT file/class activation should preserve the requested source file");
        }
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
                   return event.category == "ole.getobject" &&
                          event.detail == "sample.doc -> Word.Application";
               }),
               "GETOBJECT should emit resolved source/class detail in ole.getobject events");

        fs::remove_all(temp_root, ignored);
    }

}
