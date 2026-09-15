// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"

namespace copperfin::runtime_surface_tests
{
#include "test_prg_engine_runtime_surface_functions_buffering_curval_admission.inl"

    void test_oldval_evaluates_buffered_original_record()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_oldval";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "oldval.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U},
             {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}},
            {{"Before", "1.00"}});
        expect(create_result.ok, "OLDVAL fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'After' IN people\n"
            "REPLACE AMOUNT WITH 2 IN people\n"
            "cOriginalName = OLDVAL('NAME', 'people')\n"
            "nOriginalAmount = OLDVAL('AMOUNT', 1)\n"
            "cOriginalExpression = OLDVAL('people.NAME + '' value''', 'people')\n"
            "REPLACE NAME WITH 'Nested reverted' IN people\n"
            "lNestedRevertOriginal = OLDVAL(\"TABLEREVERT(.T., 'people') AND people.NAME = 'Before'\", 'people')\n"
            "REPLACE NAME WITH 'Nested committed' IN people\n"
            "lNestedUpdateOriginal = OLDVAL(\"TABLEUPDATE(.T., .T., 'people') AND people.NAME = 'Before'\", 'people')\n"
            "=TABLEREVERT(.T., 'people')\n"
            "cAfterRevert = OLDVAL('NAME', 'people')\n"
            "REPLACE NAME WITH 'Committed' IN people\n"
            "=TABLEUPDATE(.T., .T., 'people')\n"
            "cAfterCommit = OLDVAL('NAME', 'people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("OLDVAL regression should complete: ") + state.message);
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("coriginalname") == "Before",
               "OLDVAL should expose the original buffered character field value");
        expect(value_for("noriginalamount") == "1",
               "OLDVAL should preserve the original field data type through an explicit work area");
        expect(value_for("coriginalexpression") == "Before value",
               "OLDVAL should evaluate a documented field expression against the original record");
        expect(value_for("lnestedrevertoriginal") == "true",
               "OLDVAL should retain its original-record override through a nested TABLEREVERT");
        expect(value_for("lnestedupdateoriginal") == "true",
               "OLDVAL should retain its original-record override through a nested TABLEUPDATE");
        expect(value_for("cafterrevert").empty(),
               "OLDVAL should no longer expose an original record after TABLEREVERT");
        expect(value_for("caftercommit").empty(),
               "OLDVAL should no longer expose an original record after TABLEUPDATE");

        fs::remove_all(temp_root, ignored);
    }

    void test_curval_oldval_reject_reentrant_cursor_replacement()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_curval_oldval_reentrant_cursor";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path other_table_path = temp_root / "other.dbf";
        const fs::path other_cdx_path = temp_root / "other.cdx";
        const fs::path program_path = temp_root / "curval_oldval_reentrant_cursor.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "ID", .type = 'I', .length = 4U},
             {.name = "NAME", .type = 'C', .length = 10U}},
            {{"1", "ALPHA"}});
        expect(create_result.ok, "RQ-CF-PRG-034/#6270: reentrant CURVAL/OLDVAL fixture should be writable");
        const auto other_create_result = copperfin::vfp::create_dbf_table_file(
            other_table_path.string(),
            {{.name = "OTHER", .type = 'C', .length = 4U},
             {.name = "NAME", .type = 'C', .length = 6U}},
            {{"BETA", "BRAVO"}});
        expect(other_create_result.ok, "RQ-CF-PRG-034/#6270: alternate data-session fixture should be writable");
        write_synthetic_cdx(other_cdx_path, "OTHER", "OTHER");

        write_text(
            program_path,
            "SET MULTILOCKS ON\n"
            "SET DATASESSION TO 2\n"
            "USE '" + other_table_path.string() + "' ALIAS cursession\n"
            "SET ORDER TO TAG OTHER\n"
            "USE '" + other_table_path.string() + "' ALIAS q AGAIN IN 0\n"
            "SET DATASESSION TO 1\n"
            "USE '" + table_path.string() + "' ALIAS cursession\n"
            "=CURSORSETPROP('Buffering', 5, 'cursession')\n"
            "nCursessionArea = SELECT('cursession')\n"
            "SELECT CURVAL(\"IIF(SwitchSession(), q.NAME, q.NAME)\", 'q') AS NAME FROM cursession q INTO ARRAY aQuerySafe\n"
            "SET DATASESSION TO 1\n"
            "cQueryAliasSwitch = aQuerySafe[1]\n"
            "REPLACE NAME WITH 'CHANGED' IN cursession\n"
            "cSessionSwitch = CURVAL(\"IIF(SwitchSession(), cursession.NAME + '|' + FIELD(1, 'cursession') + '|' + TRANSFORM(FSIZE('NAME', 'cursession')) + '|' + ORDER('cursession') + '|' + TAG(1, '', 'cursession') + '|' + TRANSFORM(GETFLDSTATE('NAME', 'cursession')) + '|' + CURVAL('NAME', 'cursession'), '')\", 'cursession')\n"
            "nSessionAfterCallback = VAL(SET('DATASESSION'))\n"
            "SET DATASESSION TO 1\n"
            "lSessionUpdate = CURVAL(\"IIF(SwitchSession(), TABLEUPDATE(.T., .F., 'cursession'), .F.)\", 'cursession')\n"
            "SET DATASESSION TO 1\n"
            "cSessionCommitted = CURVAL('NAME', 'cursession')\n"
            "REPLACE NAME WITH 'ALPHA' IN cursession\n"
            "=TABLEUPDATE(.T., .F., 'cursession')\n"
            "TRY\n"
            "  dQuotedNumericAlias = CURVAL(\"LUPDATE('1')\", 'cursession')\n"
            "CATCH TO oQuotedNumericAlias\n"
            "  nQuotedNumericAliasError = oQuotedNumericAlias.ErrorNo\n"
            "ENDTRY\n"
            "cOrdinarySwitch = IIF(SwitchSession(), cursession.NAME + '|' + FIELD(1, 'cursession') + '|' + TRANSFORM(FSIZE('NAME', 'cursession')) + '|' + ORDER('cursession') + '|' + TAG(1, '', 'cursession'), '')\n"
            "SET DATASESSION TO 1\n"
            "USE IN cursession\n"
            "USE '" + table_path.string() + "' ALIAS curclose\n"
            "=CURSORSETPROP('Buffering', 5, 'curclose')\n"
            "TRY\n"
            "  cCurClose = CURVAL(\"IIF(DropCurClose(), NAME + '!', NAME + '!')\")\n"
            "CATCH TO oCurClose\n"
            "  nCurCloseError = oCurClose.ErrorNo\n"
            "  cCurCloseMessage = oCurClose.Message\n"
            "ENDTRY\n"
            "lCurClosed = NOT USED('curclose')\n"
            "USE '" + table_path.string() + "' ALIAS oldclose\n"
            "=CURSORSETPROP('Buffering', 3, 'oldclose')\n"
            "REPLACE NAME WITH 'CHANGED' IN oldclose\n"
            "DELETE IN oldclose\n"
            "TRY\n"
            "  cOldClose = OLDVAL(\"IIF(DropOldClose(), NAME + '!', NAME + '!')\", SELECT('oldclose'))\n"
            "CATCH TO oOldClose\n"
            "  nOldCloseError = oOldClose.ErrorNo\n"
            "  cOldCloseMessage = oOldClose.Message\n"
            "ENDTRY\n"
            "lOldClosed = NOT USED('oldclose')\n"
            "USE '" + table_path.string() + "' ALIAS curreuse\n"
            "=CURSORSETPROP('Buffering', 3, 'curreuse')\n"
            "TRY\n"
            "  cCurReuse = CURVAL(\"IIF(ReplaceCurReuse(), curreuse.NAME, curreuse.NAME)\", 'curreuse')\n"
            "CATCH TO oCurReuse\n"
            "  nCurReuseError = oCurReuse.ErrorNo\n"
            "  cCurReuseMessage = oCurReuse.Message\n"
            "ENDTRY\n"
            "cCurReuseName = curreuse.NAME\n"
            "USE IN curreuse\n"
            "USE '" + table_path.string() + "' ALIAS oldreuse\n"
            "=CURSORSETPROP('Buffering', 5, 'oldreuse')\n"
            "REPLACE NAME WITH 'CHANGED' IN oldreuse\n"
            "TRY\n"
            "  cOldReuse = OLDVAL(\"IIF(ReplaceOldReuse(), oldreuse.NAME, oldreuse.NAME)\", 'oldreuse')\n"
            "CATCH TO oOldReuse\n"
            "  nOldReuseError = oOldReuse.ErrorNo\n"
            "  cOldReuseMessage = oOldReuse.Message\n"
            "ENDTRY\n"
            "cOldReuseName = oldreuse.NAME\n"
            "USE IN oldreuse\n"
            "USE '" + table_path.string() + "' ALIAS nestedreuse\n"
            "=CURSORSETPROP('Buffering', 5, 'nestedreuse')\n"
            "TRY\n"
            "  cNestedReuse = CURVAL(\"IIF(ReplaceNestedReuse(), CURVAL('NAME', SELECT('nestedreuse')), '')\", 'nestedreuse')\n"
            "CATCH TO oNestedReuse\n"
            "  nNestedReuseError = oNestedReuse.ErrorNo\n"
            "ENDTRY\n"
            "cNestedReuseName = nestedreuse.NAME\n"
            "USE IN nestedreuse\n"
            "USE '" + table_path.string() + "' ALIAS currelated IN 0\n"
            "=CURSORSETPROP('Buffering', 5, 'currelated')\n"
            "USE '" + other_table_path.string() + "' ALIAS otheropen AGAIN IN 0\n"
            "cCurUnrelated = CURVAL(\"IIF(DropCurRelated(), otheropen.NAME, otheropen.NAME)\", 'currelated')\n"
            "USE '" + table_path.string() + "' ALIAS oldrelated IN 0\n"
            "=CURSORSETPROP('Buffering', 3, 'oldrelated')\n"
            "REPLACE NAME WITH 'CHANGED' IN oldrelated\n"
            "cOldUnrelated = OLDVAL(\"IIF(DropOldRelated(), otheropen.NAME, otheropen.NAME)\", 'oldrelated')\n"
            "USE '" + table_path.string() + "' ALIAS nested\n"
            "=CURSORSETPROP('Buffering', 5, 'nested')\n"
            "REPLACE NAME WITH 'CHANGED' IN nested\n"
            "cNestedValues = CURVAL(\"OLDVAL('NAME','nested')\", 'nested')\n"
            "RETURN\n"
            "FUNCTION SwitchSession\n"
            "SET DATASESSION TO 2\n"
            "RETURN .T.\n"
            "ENDFUNC\n"
            "FUNCTION DropCurClose\n"
            "USE IN curclose\n"
            "RETURN .T.\n"
            "ENDFUNC\n"
            "FUNCTION DropOldClose\n"
            "USE IN oldclose\n"
            "RETURN .T.\n"
            "ENDFUNC\n"
            "FUNCTION DropCurRelated\n"
            "USE IN currelated\n"
            "RETURN .T.\n"
            "ENDFUNC\n"
            "FUNCTION DropOldRelated\n"
            "USE IN oldrelated\n"
            "RETURN .T.\n"
            "ENDFUNC\n"
            "FUNCTION ReplaceCurReuse\n"
            "USE IN curreuse\n"
            "USE '" + table_path.string() + "' ALIAS curreuse\n"
            "RETURN .T.\n"
            "ENDFUNC\n"
            "FUNCTION ReplaceOldReuse\n"
            "USE IN oldreuse\n"
            "USE '" + table_path.string() + "' ALIAS oldreuse\n"
            "RETURN .T.\n"
            "ENDFUNC\n"
            "FUNCTION ReplaceNestedReuse\n"
            "USE IN nestedreuse\n"
            "USE '" + table_path.string() + "' ALIAS nestedreuse\n"
            "RETURN .T.\n"
            "ENDFUNC\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(program_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "RQ-CF-PRG-034/#6270: reentrant CURVAL/OLDVAL errors should be catchable without invalid memory access: " +
                   state.message);
        const auto expect_global = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), "RQ-CF-PRG-034/#6270: " + name + " should be captured");
            if (found != state.globals.end())
            {
                const std::string actual = copperfin::runtime::format_value(found->second);
                expect(actual == expected,
                       "RQ-CF-PRG-034/#6270: " + name + " expected " + expected + ", got " + actual);
            }
        };
        expect_global("ncurcloseerror", "12");
        expect_global("noldcloseerror", "12");
        expect_global("ncurreuseerror", "12");
        expect_global("noldreuseerror", "12");
        expect_global("lcurclosed", "true");
        expect_global("loldclosed", "true");
        expect_global("ccurunrelated", "BRAVO");
        expect_global("coldunrelated", "BRAVO");
        expect_global("ccurreusename", "ALPHA");
        expect_global("coldreusename", "ALPHA");
        expect_global("nnestedreuseerror", "13");
        expect_global("cnestedreusename", "ALPHA");
        expect_global("cnestedvalues", "ALPHA");
        expect_global("csessionswitch", "ALPHA|ID|10|||2|ALPHA");
        expect_global("ncursessionarea", "1");
        expect_global("lsessionupdate", "true");
        expect_global("csessioncommitted", "CHANGED");
        expect_global("nquotednumericaliaserror", "13");
        expect_global("cqueryaliasswitch", "ALPHA");
        expect_global("cordinaryswitch", "BRAVO|OTHER|6|OTHER|OTHER");
        expect_global("nsessionaftercallback", "2");
        expect_global("ccurclosemessage", "Variable 'NAME' is not found.");
        expect_global("coldclosemessage", "Variable 'NAME' is not found.");
        expect_global("ccurreusemessage", "Variable 'NAME' is not found.");
        expect_global("coldreusemessage", "Variable 'NAME' is not found.");

        fs::remove_all(temp_root, ignored);
    }

    void test_navigation_commands_reject_reentrant_cursor_replacement()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_navigation_reentrant_cursor";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path other_path = temp_root / "other.dbf";
        const fs::path cdx_path = temp_root / "people.cdx";
        const fs::path program_path = temp_root / "navigation_reentrant_cursor.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 10U}},
            {{"ALPHA"}, {"BRAVO"}});
        const auto other_result = copperfin::vfp::create_dbf_table_file(
            other_path.string(),
            {{.name = "NAME", .type = 'C', .length = 10U}},
            {{"OTHER"}, {"SECOND"}});
        expect(create_result.ok && other_result.ok,
               "RQ-CF-PRG-035/#6319: navigation cursor fixtures should be writable");
        write_synthetic_cdx(cdx_path, "NAME", "NAME");

        write_text(
            program_path,
            "SET MULTILOCKS ON\n"
            "SET DATASESSION TO 2\n"
            "USE '" + other_path.string() + "' ALIAS switchpeople\n"
            "SET DATASESSION TO 1\n"
            "nGoCalls = 0\n"
            "nSkipCalls = 0\n"
            "nSeekCalls = 0\n"
            "nUnlockCalls = 0\n"
            "nSwitchCalls = 0\n"
            "USE '" + table_path.string() + "' ALIAS gopeople\n"
            "TRY\n"
            "  GO ReplaceGo() IN gopeople\n"
            "CATCH TO oGo\n"
            "  nGoError = oGo.ErrorNo\n"
            "ENDTRY\n"
            "cGoReplacement = gopeople.NAME\n"
            "USE '" + table_path.string() + "' ALIAS skippeople\n"
            "TRY\n"
            "  SKIP EVALUATE(\"ReplaceSkip()\") IN skippeople\n"
            "CATCH TO oSkip\n"
            "  nSkipError = oSkip.ErrorNo\n"
            "ENDTRY\n"
            "cSkipReplacement = skippeople.NAME\n"
            "USE '" + table_path.string() + "' ALIAS seekpeople\n"
            "SET ORDER TO TAG NAME IN seekpeople\n"
            "TRY\n"
            "  SEEK ReplaceSeek() IN seekpeople\n"
            "CATCH TO oSeek\n"
            "  nSeekError = oSeek.ErrorNo\n"
            "ENDTRY\n"
            "cSeekReplacement = seekpeople.NAME\n"
            "USE '" + table_path.string() + "' ALIAS unlockpeople\n"
            "=RLOCK('unlockpeople')\n"
            "TRY\n"
            "  UNLOCK RECORD EVALUATE(\"ReplaceUnlock()\") IN unlockpeople\n"
            "CATCH TO oUnlock\n"
            "  nUnlockError = oUnlock.ErrorNo\n"
            "ENDTRY\n"
            "cUnlockReplacement = unlockpeople.NAME\n"
            "USE '" + table_path.string() + "' ALIAS switchpeople\n"
            "GO 1 IN switchpeople\n"
            "SKIP EVALUATE(\"SwitchSession()\") IN switchpeople\n"
            "SET DATASESSION TO 1\n"
            "nSwitchRecno = RECNO('switchpeople')\n"
            "RETURN\n"
            "FUNCTION ReplaceGo\n"
            "nGoCalls = nGoCalls + 1\n"
            "USE IN gopeople\n"
            "USE '" + table_path.string() + "' ALIAS gopeople\n"
            "RETURN 2\n"
            "ENDFUNC\n"
            "FUNCTION ReplaceSkip\n"
            "nSkipCalls = nSkipCalls + 1\n"
            "USE IN skippeople\n"
            "USE '" + table_path.string() + "' ALIAS skippeople\n"
            "RETURN 1\n"
            "ENDFUNC\n"
            "FUNCTION ReplaceSeek\n"
            "nSeekCalls = nSeekCalls + 1\n"
            "USE IN seekpeople\n"
            "USE '" + table_path.string() + "' ALIAS seekpeople\n"
            "RETURN 'BRAVO'\n"
            "ENDFUNC\n"
            "FUNCTION ReplaceUnlock\n"
            "nUnlockCalls = nUnlockCalls + 1\n"
            "USE IN unlockpeople\n"
            "USE '" + table_path.string() + "' ALIAS unlockpeople\n"
            "RETURN 1\n"
            "ENDFUNC\n"
            "FUNCTION SwitchSession\n"
            "nSwitchCalls = nSwitchCalls + 1\n"
            "SET DATASESSION TO 2\n"
            "RETURN 1\n"
            "ENDFUNC\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(program_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "RQ-CF-PRG-035/#6319: navigation closure errors should remain catchable: " + state.message);
        const auto expect_global = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), "RQ-CF-PRG-035/#6319: " + name + " should be captured");
            if (found != state.globals.end())
            {
                const std::string actual = copperfin::runtime::format_value(found->second);
                expect(actual == expected,
                       "RQ-CF-PRG-035/#6319: " + name + " expected " + expected + ", got " + actual);
            }
        };
        expect_global("ngoerror", "1002");
        expect_global("nskiperror", "1002");
        expect_global("nseekerror", "1002");
        expect_global("nunlockerror", "1002");
        expect_global("cgoreplacement", "ALPHA");
        expect_global("cskipreplacement", "ALPHA");
        expect_global("cseekreplacement", "ALPHA");
        expect_global("cunlockreplacement", "ALPHA");
        expect_global("nswitchrecno", "2");
        expect_global("ngocalls", "1");
        expect_global("nskipcalls", "1");
        expect_global("nseekcalls", "1");
        expect_global("nunlockcalls", "1");
        expect_global("nswitchcalls", "1");

        const auto event_count = [&](const std::string &category)
        {
            return std::count_if(
                state.events.begin(),
                state.events.end(),
                [&](const auto &event) { return event.category == category; });
        };
        expect(event_count("runtime.go") == 1U,
               "RQ-CF-PRG-035/#6319: only the valid session-switch GO should emit success");
        expect(event_count("runtime.skip") == 1U,
               "RQ-CF-PRG-035/#6319: only the valid session-switch SKIP should emit success");
        expect(event_count("runtime.seek") == 0U,
               "RQ-CF-PRG-035/#6319: rejected SEEK should not emit false success");
        expect(event_count("runtime.unlock") == 0U,
               "RQ-CF-PRG-035/#6319: rejected UNLOCK should not emit false success");

        fs::remove_all(temp_root, ignored);
    }

    void test_setfldstate_assigns_buffered_mutation_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_setfldstate";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "setfldstate.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U},
             {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}},
            {{"Before", "1.00"}});
        expect(create_result.ok, "SETFLDSTATE fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "lStateOnly = SETFLDSTATE('NAME', 2, 'people')\n"
            "nStateOnlyNext = GETNEXTMODIFIED(0, 'people')\n"
            "lStateOnlyCommitted = TABLEUPDATE(.T., .T., 'people')\n"
            "nStateOnlyAfterCommit = GETNEXTMODIFIED(0, 'people')\n"
            "REPLACE NAME WITH 'Suppressed' IN people\n"
            "REPLACE AMOUNT WITH 2 IN people\n"
            "lByName = SETFLDSTATE('NAME', 1, 'people')\n"
            "lByNumber = SETFLDSTATE(2, 2, 'people')\n"
            "lDeletion = SETFLDSTATE(0, 1, 'people')\n"
            "cAssigned = GETFLDSTATE(-1, 'people')\n"
            "lCommitted = TABLEUPDATE(.T., .T., 'people')\n"
            "cPersistedName = people.NAME\n"
            "nPersistedAmount = people.AMOUNT\n"
            "lWorkArea = SETFLDSTATE(1, 2, 1)\n"
            "nWorkArea = GETFLDSTATE(1, 1)\n"
            "DELETE IN people\n"
            "lDeletionSuppressed = SETFLDSTATE(0, 1, 'people')\n"
            "lDeletionCommitted = TABLEUPDATE(.T., .T., 'people')\n"
            "lStillActive = NOT DELETED('people')\n"
            "=CURSORSETPROP('Buffering', 3, 'people')\n"
            "REPLACE AMOUNT WITH 3 IN people\n"
            "lRowSuppressed = SETFLDSTATE('AMOUNT', 1, 'people')\n"
            "lRowCommitted = TABLEUPDATE(.T., .T., 'people')\n"
            "nRowPersistedAmount = people.AMOUNT\n"
            "lInvalidState = SETFLDSTATE('NAME', 5, 'people')\n"
            "lMissingField = SETFLDSTATE('MISSING', 2, 'people')\n"
            "lInvalidNumber = SETFLDSTATE(3, 2, 'people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("SETFLDSTATE regression should complete: ") + state.message);
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lbyname") == "true", "SETFLDSTATE should accept a named field");
        expect(value_for("lstateonly") == "true" && value_for("nstateonlynext") == "1" &&
                   value_for("lstateonlycommitted") == "true" &&
                   value_for("nstateonlyaftercommit") == "0",
               "a first modified SETFLDSTATE assignment should materialize and clear its buffered record");
        expect(value_for("lbynumber") == "true", "SETFLDSTATE should accept a one-based field number");
        expect(value_for("ldeletion") == "true", "SETFLDSTATE field zero should assign deletion state");
        expect(value_for("cassigned") == "112",
               "SETFLDSTATE assignments should be visible through GETFLDSTATE aggregate state");
        expect(value_for("lcommitted") == "true", "TABLEUPDATE should accept SETFLDSTATE-controlled records");
        expect(value_for("cpersistedname") == "Before",
               "state 1 should suppress an existing buffered field write (actual=" +
                   value_for("cpersistedname") + ")");
        expect(value_for("npersistedamount") == "2", "state 2 should retain an existing buffered field write");
        expect(value_for("lworkarea") == "true" && value_for("nworkarea") == "2",
               "SETFLDSTATE should resolve an explicit numeric work area");
        expect(value_for("ldeletionsuppressed") == "true" && value_for("ldeletioncommitted") == "true" &&
                   value_for("lstillactive") == "true",
               "state 1 should suppress an existing buffered deletion write (actual=" +
                   value_for("ldeletionsuppressed") + "," + value_for("ldeletioncommitted") + "," +
                   value_for("lstillactive") + ")");
        expect(value_for("lrowsuppressed") == "true" && value_for("lrowcommitted") == "true" &&
                   value_for("nrowpersistedamount") == "2",
               "state 1 should suppress a row-buffered field write");
        expect(value_for("linvalidstate") == "false", "SETFLDSTATE should reject state values outside 1 through 4");
        expect(value_for("lmissingfield") == "false", "SETFLDSTATE should reject unknown field names");
        expect(value_for("linvalidnumber") == "false", "SETFLDSTATE should reject out-of-range field numbers");

        fs::remove_all(temp_root, ignored);
    }

    void test_getfldstate_tracks_buffered_mutation_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_getfldstate";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "getfldstate.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {
                {.name = "NAME", .type = 'C', .length = 24U},
                {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}
            },
            {{"Before", "1.00"}});
        expect(create_result.ok, "GETFLDSTATE fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "cInitial = GETFLDSTATE(-1, 'people')\n"
            "nWorkAreaName = GETFLDSTATE(1, 1)\n"
            "REPLACE NAME WITH 'Buffered' IN people\n"
            "nNameByNumber = GETFLDSTATE(1, 'people')\n"
            "nAmountByName = GETFLDSTATE('AMOUNT', 'people')\n"
            "cChanged = GETFLDSTATE(-1, 'people')\n"
            "REPLACE NAME WITH 'Before' IN people\n"
            "nRestoredName = GETFLDSTATE('NAME', 'people')\n"
            "DELETE IN people\n"
            "RECALL IN people\n"
            "nRecalledDeletion = GETFLDSTATE(0, 'people')\n"
            "cRecalled = GETFLDSTATE(-1, 'people')\n"
            "=TABLEREVERT(.T., 'people')\n"
            "nRevertedName = GETFLDSTATE('NAME', 'people')\n"
            "APPEND BLANK IN people\n"
            "cAppended = GETFLDSTATE(-1, 'people')\n"
            "REPLACE NAME WITH 'Appended' IN people\n"
            "cAppendedChanged = GETFLDSTATE(-1, 'people')\n"
            "GO 99 IN people\n"
            "lEofNull = ISNULL(GETFLDSTATE(1, 'people'))\n"
            "=TABLEREVERT(.T., 'people')\n"
            "=CURSORSETPROP('Buffering', 3, 'people')\n"
            "GO 1 IN people\n"
            "REPLACE AMOUNT WITH 2 IN people\n"
            "nRowBufferedAmount = GETFLDSTATE('AMOUNT', 'people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("GETFLDSTATE regression should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("cinitial") == "111", "unchanged buffered record should expose deletion plus field state 1");
        expect(value_for("nworkareaname") == "1", "numeric work-area target should resolve the local cursor");
        expect(value_for("nnamebynumber") == "2", "field-number lookup should report the changed field");
        expect(value_for("namountbyname") == "1", "field-name lookup should preserve unchanged-field state");
        expect(value_for("cchanged") == "121", "aggregate state should preserve field ordering");
        expect(value_for("nrestoredname") == "2", "restoring a value must not clear buffered mutation state");
        expect(value_for("nrecalleddeletion") == "2", "recalling a deleted row must retain deletion mutation state");
        expect(value_for("crecalled") == "221", "aggregate state should retain recall mutation state");
        expect(value_for("nrevertedname") == "1", "TABLEREVERT should clear buffered field mutation state");
        expect(value_for("cappended") == "333", "appended rows should expose unmodified appended state");
        expect(value_for("cappendedchanged") == "343", "appended changed fields should expose state 4");
        expect(value_for("leofnull") == "true", "GETFLDSTATE should return .NULL. at EOF");
        expect(value_for("nrowbufferedamount") == "2", "row buffering should expose changed field state");

        fs::remove_all(temp_root, ignored);
    }

    void test_local_optimistic_table_buffering()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_table_buffering";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "buffering.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {
                {.name = "NAME", .type = 'C', .length = 24U},
                {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}
            },
            {{"Before", "1.00"}, {"Other", "2.00"}});
        expect(create_result.ok, "buffering fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "lSet = CURSORSETPROP('Buffering', 5, 'people')\n"
            "nMode = CURSORGETPROP('Buffering', 'people')\n"
            "REPLACE NAME WITH 'Buffered' IN people\n"
            "cBuffered = people.NAME\n"
            "TABLEUPDATE(.T., .T., 'people')\n"
            "cCommitted = people.NAME\n"
            "REPLACE NAME WITH 'Discarded' IN people\n"
            "TABLEREVERT(.T., 'people')\n"
            "cReverted = people.NAME\n"
            "lUnsupported = CURSORSETPROP('Buffering', 6, 'people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 5U});
        const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(paused.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "buffering regression should pause after the buffered replace");

        const auto before_commit = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(before_commit.ok && before_commit.table.records.size() == 2U &&
                   before_commit.table.records[0].values[0].display_value == "Before",
               "optimistic table buffering should leave disk unchanged before TABLEUPDATE");

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "buffering regression should complete after resume");

        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lset") == "true", "CURSORSETPROP Buffering 5 should succeed");
        expect(value_for("nmode") == "5", "CURSORGETPROP Buffering should report mode 5");
        expect(value_for("cbuffered") == "Buffered", "buffered field reads should see pending values");
        expect(value_for("ccommitted") == "Buffered", "TABLEUPDATE should retain committed values in the cursor");
        expect(value_for("creverted") == "Buffered", "TABLEREVERT should restore the last committed value");
        expect(value_for("lunsupported") == "false", "unsupported buffering modes should fail explicitly");

        const auto after_resume = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
        expect(after_resume.ok && after_resume.table.records.size() == 2U &&
                   after_resume.table.records[0].values[0].display_value == "Buffered",
               "TABLEUPDATE should persist the buffered value to disk");

        fs::remove_all(temp_root, ignored);
    }

    void test_local_optimistic_table_buffering_append_lifecycle()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_table_buffering_append";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "buffering_append.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {
                {.name = "NAME", .type = 'C', .length = 24U},
                {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}
            },
            {{"Before", "1.00"}, {"Existing", "1.50"}, {"Another", "2.00"}});
        expect(create_result.ok, "append buffering fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "lSet = CURSORSETPROP('Buffering', 5, 'people')\n"
            "APPEND BLANK\n"
            "REPLACE NAME WITH 'Appended', AMOUNT WITH 2 IN people\n"
            "cPending = people.NAME\n"
            "nPendingCount = RECCOUNT('people')\n"
            "lUpdate1 = TABLEUPDATE(.T., .T., 'people')\n"
            "nCommittedCount = RECCOUNT('people')\n"
            "APPEND BLANK\n"
            "REPLACE NAME WITH 'Discarded' IN people\n"
            "nRevertCount = RECCOUNT('people')\n"
            "TABLEREVERT(.T., 'people')\n"
            "nAfterRevertCount = RECCOUNT('people')\n"
            "INSERT INTO people (NAME, AMOUNT) VALUES ('Inserted', 3)\n"
            "nInsertPendingCount = RECCOUNT('people')\n"
            "cInsertPending = people.NAME\n"
            "lUpdate2 = TABLEUPDATE(.T., .T., 'people')\n"
            "nFinalCount = RECCOUNT('people')\n"
            "GO 5 IN people\n"
            "cFinalName = people.NAME\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 5U});
        const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(paused.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "append buffering regression should pause before the first commit");

        const auto before_commit = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(before_commit.ok && before_commit.table.records.size() == 3U,
               "pending APPEND BLANK should leave the persisted row count unchanged");

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "append buffering regression should complete after resume");
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lset") == "true", "append buffering should enable mode 5");
        expect(value_for("cpending") == "Appended", "pending APPEND BLANK fields should be readable");
        expect(value_for("npendingcount") == "4", "pending APPEND BLANK should affect the cursor count");
        expect(value_for("ncommittedcount") == "4", "TABLEUPDATE should commit the appended row");
        expect(value_for("lupdate1") == "true", "TABLEUPDATE should report success for APPEND BLANK");
        expect(value_for("nrevertcount") == "5", "pending second append should affect the cursor count");
        expect(value_for("nafterrevertcount") == "4", "TABLEREVERT should discard the pending append");
        expect(value_for("ninsertpendingcount") == "5", "INSERT should create a pending buffered row");
        expect(value_for("cinsertpending") == "Inserted", "pending INSERT fields should be readable");
        expect(value_for("nfinalcount") == "5", "TABLEUPDATE should commit the INSERT row");
        expect(value_for("lupdate2") == "true", "TABLEUPDATE should report success for INSERT");
        expect(value_for("cfinalname") == "Inserted", "the committed INSERT row should retain its values");

        const auto after_commit = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        std::string persisted_names;
        if (after_commit.ok)
        {
            for (const auto &record : after_commit.table.records)
            {
                if (!persisted_names.empty())
                {
                    persisted_names += ",";
                }
                persisted_names += record.values.empty() ? "<missing>" : record.values[0].display_value;
            }
        }
        expect(after_commit.ok && after_commit.table.records.size() == 5U &&
                   after_commit.table.records[3].values[0].display_value == "Appended" &&
                   after_commit.table.records[4].values[0].display_value == "Inserted",
               "TABLEUPDATE should persist committed append and INSERT rows only (got " + persisted_names + ")");

        fs::remove_all(temp_root, ignored);
    }

    void test_table_buffer_appends_use_negative_recno_identity()
    {
        namespace fs = std::filesystem;
        const auto exercise_mode = [](int buffering_mode, const std::string &mode_name)
        {
            const fs::path temp_root = fs::temp_directory_path() /
                ("copperfin_runtime_surface_negative_table_buffer_recno_" + mode_name);
            const fs::path table_path = temp_root / "people.dbf";
            const fs::path program_path = temp_root / "negative_recno.prg";
            std::error_code ignored;
            fs::remove_all(temp_root, ignored);
            fs::create_directories(temp_root);

            const auto create_result = copperfin::vfp::create_dbf_table_file(
                table_path.string(),
                {{.name = "NAME", .type = 'C', .length = 24U}},
                {{"PersistedOne"}, {"PersistedTwo"}});
            expect(create_result.ok, mode_name + " negative-RECNO fixture should be writable");

            write_text(
                program_path,
                "USE '" + table_path.string() + "' ALIAS people\n"
                "=CURSORSETPROP('Buffering', " + std::to_string(buffering_mode) + ", 'people')\n"
                "APPEND BLANK\n"
                "REPLACE NAME WITH 'PendingOne' IN people\n"
                "nFirstPending = RECNO('people')\n"
                "APPEND BLANK\n"
                "REPLACE NAME WITH 'PendingTwo' IN people\n"
                "nSecondPending = RECNO('people')\n"
                "GO -1 IN people\n"
                "cFirstPending = people.NAME\n"
                "nFirstAfterGo = RECNO('people')\n"
                "GO -2 IN people\n"
                "cSecondPending = people.NAME\n"
                "nSecondAfterGo = RECNO('people')\n"
                "nPendingCount = RECCOUNT('people')\n"
                "lUpdate = TABLEUPDATE(.T., .T., 'people')\n"
                "nCommittedRec = RECNO('people')\n"
                "nCommittedCount = RECCOUNT('people')\n"
                "APPEND BLANK\n"
                "nRevertPending = RECNO('people')\n"
                "=TABLEREVERT(.T., 'people')\n"
                "nAfterRevertCount = RECCOUNT('people')\n"
                "RETURN\n");

            copperfin::runtime::PrgRuntimeSession session =
                copperfin::runtime::PrgRuntimeSession::create(
                    make_runtime_session_options(program_path.string(), temp_root.string()));
            const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.completed, mode_name + " negative-RECNO regression should complete: " + state.message);
            const auto value_for = [&](const std::string &name) -> std::string
            {
                const auto found = state.globals.find(name);
                return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
            };
            expect(value_for("nfirstpending") == "-1", mode_name + " first pending append should expose RECNO() -1");
            expect(value_for("nsecondpending") == "-2", mode_name + " second pending append should expose RECNO() -2");
            expect(value_for("cfirstpending") == "PendingOne" && value_for("nfirstaftergo") == "-1",
                   mode_name + " GO -1 should select the first pending append");
            expect(value_for("csecondpending") == "PendingTwo" && value_for("nsecondaftergo") == "-2",
                   mode_name + " GO -2 should select the second pending append");
            expect(value_for("npendingcount") == "4", mode_name + " pending appends should remain visible to RECCOUNT()");
            expect(value_for("lupdate") == "true" && value_for("ncommittedrec") == "4" &&
                       value_for("ncommittedcount") == "4",
                   mode_name + " TABLEUPDATE should materialize positive physical record numbers");
            expect(value_for("nrevertpending") == "-1" && value_for("nafterrevertcount") == "4",
                   mode_name + " TABLEREVERT(.T.) should remove a pending negative-RECNO append");

            const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 4U);
            expect(persisted.ok && persisted.table.records.size() == 4U &&
                       persisted.table.records[2].values[0].display_value == "PendingOne" &&
                       persisted.table.records[3].values[0].display_value == "PendingTwo",
                   mode_name + " update should persist both selected pending rows without the reverted append");
            fs::remove_all(temp_root, ignored);
        };

        exercise_mode(4, "pessimistic");
        exercise_mode(5, "optimistic");
    }

    void test_table_buffer_append_identity_survives_partial_update_failure()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
            "copperfin_runtime_surface_negative_table_buffer_recno_partial_update";
        const fs::path table_path = temp_root / "partial_update.dbf";
        const fs::path program_path = temp_root / "partial_update.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Persisted"}});
        expect(create_result.ok, "partial-update negative-RECNO fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "APPEND BLANK\n"
            "REPLACE NAME WITH 'PendingOne' IN people\n"
            "APPEND BLANK\n"
            "REPLACE NAME WITH 'PendingTwo' IN people\n"
            "lUpdate = TABLEUPDATE(.T., .T., 'people')\n"
            "GO -1 IN people\n"
            "cFirstPending = people.NAME\n"
            "nFirstPending = RECNO('people')\n"
            "GO -2 IN people\n"
            "cSecondPending = people.NAME\n"
            "nSecondPending = RECNO('people')\n"
            "RETURN\n");

        // The first append writes its blank row and value (matches 1 and 2).
        // Failing the second append's blank-row promotion (match 3) leaves a
        // deliberately partially materialized table-buffer batch.
        const ScopedEnvironmentValue fail_path(
            "COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS", table_path.filename().string());
        const ScopedEnvironmentValue fail_stage("COPPERFIN_TEST_FAIL_WRITE_STAGE", "before-promote");
        const ScopedEnvironmentValue fail_match("COPPERFIN_TEST_FAIL_WRITE_MATCH_NUMBER", "3");
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "partial-update negative-RECNO regression should complete: " + state.message);
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lupdate") == "false", "injected second append write failure should fail TABLEUPDATE");
        expect(value_for("cfirstpending") == "PendingOne" && value_for("nfirstpending") == "-1",
               "the first pending append must retain its -1 identity after partial materialization");
        expect(value_for("csecondpending") == "PendingTwo" && value_for("nsecondpending") == "-2",
               "GO -2 must still select the unmaterialized second pending append after partial failure");

        const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 3U);
        expect(persisted.ok && persisted.table.records.size() == 2U &&
                   persisted.table.records[1].values[0].display_value == "PendingOne",
               "only the first append should be physically materialized before the injected failure");
        fs::remove_all(temp_root, ignored);
    }

    void test_local_optimistic_table_buffering_delete_recall()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_table_buffering_delete";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "buffering_delete.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {
                {.name = "NAME", .type = 'C', .length = 24U},
                {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}
            },
            {{"Alpha", "1.00"}, {"Bravo", "2.00"}, {"Charlie", "3.00"}});
        expect(create_result.ok, "delete buffering fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "lSet = CURSORSETPROP('Buffering', 5, 'people')\n"
            "SET DELETED ON\n"
            "GO 1 IN people\n"
            "DELETE IN people\n"
            "lCurrentDeleted = DELETED()\n"
            "RECALL IN people\n"
            "lRecalled = DELETED()\n"
            "DELETE FOR NAME = 'Alpha' IN people\n"
            "DELETE FOR NAME = 'Bravo' IN people\n"
            "GO TOP IN people\n"
            "nTopAfterDeletes = RECNO('people')\n"
            "lUpdate = TABLEUPDATE(.T., .T., 'people')\n"
            "SET DELETED OFF\n"
            "GO 1 IN people\n"
            "lCommittedAlpha = DELETED()\n"
            "GO 2 IN people\n"
            "lCommittedBravo = DELETED()\n"
            "SET DELETED ON\n"
            "RECALL FOR NAME = 'Alpha' IN people\n"
            "GO 1 IN people\n"
            "lPendingRecall = DELETED()\n"
            "DELETE FOR NAME = 'Charlie' IN people\n"
            "TABLEREVERT(.T., 'people')\n"
            "SET DELETED OFF\n"
            "GO 1 IN people\n"
            "lRevertedAlpha = DELETED()\n"
            "GO 3 IN people\n"
            "lRevertedCharlie = DELETED()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 12U});
        const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(paused.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "delete buffering regression should pause before TABLEUPDATE");

        const auto before_commit = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(before_commit.ok && before_commit.table.records.size() == 3U &&
                   !before_commit.table.records[0].deleted && !before_commit.table.records[1].deleted,
               "pending DELETE should leave the persisted tombstones unchanged");

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "delete buffering regression should complete after resume");
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lset") == "true", "delete buffering should enable mode 5");
        expect(value_for("lcurrentdeleted") == "true", "current DELETE should be visible through DELETED()");
        expect(value_for("lrecalled") == "false", "current RECALL should clear pending deletion");
        expect(value_for("ntopafterdeletes") == "3", "SET DELETED should skip pending tombstones");
        expect(value_for("lupdate") == "true", "TABLEUPDATE should commit pending tombstones");
        expect(value_for("lcommittedalpha") == "true", "TABLEUPDATE should persist Alpha deletion");
        expect(value_for("lcommittedbravo") == "true", "TABLEUPDATE should persist Bravo deletion");
        expect(value_for("lpendingrecall") == "false", "RECALL should clear a pending committed deletion");
        expect(value_for("lrevertedalpha") == "true", "TABLEREVERT should restore committed Alpha state");
        expect(value_for("lrevertedcharlie") == "false", "TABLEREVERT should discard pending Charlie deletion");

        const auto after_revert = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(after_revert.ok && after_revert.table.records.size() == 3U &&
                   after_revert.table.records[0].deleted && after_revert.table.records[1].deleted &&
                   !after_revert.table.records[2].deleted,
               "TABLEUPDATE should persist committed tombstones while TABLEREVERT discards later changes");

        fs::remove_all(temp_root, ignored);
    }

    void test_local_pessimistic_table_buffering()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_pessimistic_table_buffering";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "pessimistic_table_buffering.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Alpha"}, {"Bravo"}, {"Charlie"}});
        expect(create_result.ok, "pessimistic table buffering fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "SET REPROCESS TO 0\n"
            "lSet = CURSORSETPROP('Buffering', 4, 'people')\n"
            "nMode = CURSORGETPROP('Buffering', 'people')\n"
            "REPLACE NAME WITH 'PendingOne' IN people\n"
            "GO 2 IN people\n"
            "REPLACE NAME WITH 'PendingTwo' IN people\n"
            "cSecondPending = people.NAME\n"
            "GO 1 IN people\n"
            "cFirstPending = people.NAME\n"
            "lFirstLocked = ISRLOCKED('people')\n"
            "GO 2 IN people\n"
            "lSecondLocked = ISRLOCKED('people')\n"
            "GO 1 IN people\n"
            "cStillPending = people.NAME\n"
            "TABLEUPDATE(.T., .T., 'people')\n"
            "lUnlockedUpdate = !ISRLOCKED('people')\n"
            "cFirstCommitted = people.NAME\n"
            "GO 2 IN people\n"
            "cSecondCommitted = people.NAME\n"
            "GO 3 IN people\n"
            "DELETE IN people\n"
            "REPLACE NAME WITH 'PendingThree' IN people\n"
            "lThirdLocked = ISRLOCKED('people')\n"
            "GO 1 IN people\n"
            "cAfterNavigation = people.NAME\n"
            "TABLEREVERT(.T., 'people')\n"
            "lUnlockedRevert = !ISRLOCKED('people')\n"
            "GO 3 IN people\n"
            "lThirdReverted = DELETED()\n"
            "APPEND BLANK\n"
            "REPLACE NAME WITH 'Appended' IN people\n"
            "INSERT INTO people (NAME) VALUES ('Inserted')\n"
            "nPendingCount = RECCOUNT('people')\n"
            "GO 1 IN people\n"
            "cBeforeAppendUpdate = people.NAME\n"
            "TABLEUPDATE(.T., .T., 'people')\n"
            "nAfterUpdate = RECCOUNT('people')\n"
            "GO 4 IN people\n"
            "cAppended = people.NAME\n"
            "GO 5 IN people\n"
            "cInserted = people.NAME\n"
            "GO 1 IN people\n"
            "REPLACE NAME WITH 'Held' IN people\n"
            "lHeld = ISRLOCKED('people')\n"
            "SET DATASESSION TO 2\n"
            "USE '" + table_path.string() + "' ALIAS other\n"
            "SET REPROCESS TO 0\n"
            "= CURSORSETPROP('Buffering', 4, 'other')\n"
            "GO 1 IN other\n"
            "lCompeting = .T.\n"
            "TRY\n"
            "    REPLACE NAME WITH 'Blocked' IN other\n"
            "CATCH TO loError\n"
            "    lCompeting = .F.\n"
            "ENDTRY\n"
            "SET DATASESSION TO 1\n"
            "TABLEUPDATE(.T., .T., 'people')\n"
            "SET DATASESSION TO 2\n"
            "REPLACE NAME WITH 'SecondCanEdit' IN other\n"
            "lSecondUpdate = TABLEUPDATE(.T., .T., 'other')\n"
            "SET DATASESSION TO 1\n"
            "cFinal = people.NAME\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 15U});
        session.add_breakpoint({.file_path = program_path.string(), .line = 37U});

        const auto first_pause = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(first_pause.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "pessimistic table buffering should pause before TABLEUPDATE");
        const auto before_update = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(before_update.ok && before_update.table.records.size() == 3U &&
                   before_update.table.records[0].values[0].display_value == "Alpha" &&
                   before_update.table.records[1].values[0].display_value == "Bravo",
               "mode 4 should keep multiple pending edits off disk before TABLEUPDATE");

        const auto second_pause = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(second_pause.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "pessimistic table buffering should pause before append TABLEUPDATE");
        const auto before_append_update = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(before_append_update.ok && before_append_update.table.records.size() == 3U,
               "mode 4 APPEND and INSERT should remain pending until TABLEUPDATE");

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "pessimistic table buffering regression should complete after resume");
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lset") == "true", "CURSORSETPROP Buffering 4 should succeed");
        expect(value_for("nmode") == "4", "CURSORGETPROP Buffering should report mode 4");
        expect(value_for("csecondpending") == "PendingTwo", "mode 4 should expose a pending second-row value");
        expect(value_for("cfirstpending") == "PendingOne", "navigation should preserve the first pending value");
        expect(value_for("lfirstlocked") == "true", "mode 4 should lock the first changed row");
        expect(value_for("lsecondlocked") == "true", "mode 4 should lock each changed row");
        expect(value_for("cstillpending") == "PendingOne", "navigation should not commit mode-4 changes");
        expect(value_for("lunlockedupdate") == "true", "mode 4 TABLEUPDATE should release all row locks");
        expect(value_for("cfirstcommitted") == "PendingOne", "mode 4 should commit the first row");
        expect(value_for("csecondcommitted") == "PendingTwo", "mode 4 should commit the second row");
        expect(value_for("lthirdlocked") == "true", "mode 4 DELETE/REPLACE should retain a row lock");
        expect(value_for("cafternavigation") == "PendingOne", "mode 4 navigation should retain deleted-row edits");
        expect(value_for("lunlockedrevert") == "true", "mode 4 TABLEREVERT should release all row locks");
        expect(value_for("lthirdreverted") == "false", "mode 4 TABLEREVERT should discard DELETE/REPLACE");
        expect(value_for("npendingcount") == "5", "mode 4 APPEND and INSERT should affect cursor count");
        expect(value_for("cbeforeappendupdate") == "PendingOne", "mode 4 should keep prior values while appends are pending");
        expect(value_for("nafterupdate") == "5", "mode 4 TABLEUPDATE should commit appended rows");
        expect(value_for("cappended") == "Appended", "mode 4 should commit APPEND BLANK values");
        expect(value_for("cinserted") == "Inserted", "mode 4 should commit INSERT values");
        expect(value_for("lheld") == "true", "mode 4 should retain a lock for a later pending edit");
        expect(value_for("lcompeting") == "false", "a competing data session should not edit a locked mode-4 row");
        expect(value_for("lsecondupdate") == "true", "a competing session should edit after the owner commits");
        expect(value_for("cfinal") == "SecondCanEdit", "the competing mode-4 commit should persist");

        const auto after_resume = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(after_resume.ok && after_resume.table.records.size() == 5U &&
                   after_resume.table.records[0].values[0].display_value == "SecondCanEdit" &&
                   after_resume.table.records[1].values[0].display_value == "PendingTwo" &&
                   after_resume.table.records[2].values[0].display_value == "Charlie" &&
                   after_resume.table.records[3].values[0].display_value == "Appended" &&
                   after_resume.table.records[4].values[0].display_value == "Inserted",
               "mode 4 should persist only the committed table changes");

        fs::remove_all(temp_root, ignored);
    }

    void test_local_optimistic_buffer_conflicts()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_optimistic_buffer_conflicts";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "optimistic_buffer_conflicts.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Alpha"}, {"Bravo"}});
        expect(create_result.ok, "optimistic conflict fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "SET REPROCESS TO 0\n"
            "lSet5 = CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'Mode5One' IN people\n"
            "GO 2 IN people\n"
            "REPLACE NAME WITH 'Mode5Two' IN people\n"
            "SET DATASESSION TO 2\n"
            "USE '" + table_path.string() + "' ALIAS other\n"
            "GO 1 IN other\n"
            "REPLACE NAME WITH 'ExternalFive' IN other\n"
            "SET DATASESSION TO 1\n"
            "lConflict5 = TABLEUPDATE(.T., .F., 'people')\n"
            "GO 1 IN people\n"
            "cPending5One = people.NAME\n"
            "GO 2 IN people\n"
            "cPending5Two = people.NAME\n"
            "lForce5 = TABLEUPDATE(.T., .T., 'people')\n"
            "GO 1 IN people\n"
            "cForced5One = people.NAME\n"
            "GO 2 IN people\n"
            "cForced5Two = people.NAME\n"
            "lSet3 = CURSORSETPROP('Buffering', 3, 'people')\n"
            "GO 1 IN people\n"
            "REPLACE NAME WITH 'Mode3Pending' IN people\n"
            "SET DATASESSION TO 2\n"
            "REPLACE NAME WITH 'ExternalThree' IN other\n"
            "SET DATASESSION TO 1\n"
            "lConflict3 = TABLEUPDATE(.T., .F., 'people')\n"
            "cPending3 = people.NAME\n"
            "lRevert3 = TABLEREVERT(.T., 'people')\n"
            "cAfterRevert3 = people.NAME\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 11U});
        session.add_breakpoint({.file_path = program_path.string(), .line = 27U});

        const auto first_pause = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(first_pause.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "optimistic mode-5 conflict regression should pause before TABLEUPDATE");
        const auto before_force = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(before_force.ok && before_force.table.records.size() == 2U &&
                   before_force.table.records[0].values[0].display_value == "ExternalFive" &&
                   before_force.table.records[1].values[0].display_value == "Bravo",
               "mode 5 conflict setup should change only the competing disk row");

        const auto second_pause = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(second_pause.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "optimistic mode-3 conflict regression should pause before TABLEUPDATE");
        const auto before_revert = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(before_revert.ok && before_revert.table.records.size() == 2U &&
                   before_revert.table.records[0].values[0].display_value == "ExternalThree" &&
                   before_revert.table.records[1].values[0].display_value == "Mode5Two",
               "forced mode-5 update should commit the full pending batch before mode-3 conflict");

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "optimistic conflict regression should complete after resume");
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lset5") == "true", "mode 5 should remain enabled for conflict testing");
        expect(value_for("lconflict5") == "false", "mode 5 TABLEUPDATE should reject an external row change");
        expect(value_for("cpending5one") == "Mode5One", "mode 5 should retain pending values after conflict");
        expect(value_for("cpending5two") == "Mode5Two", "mode 5 should retain all pending rows after conflict");
        expect(value_for("lforce5") == "true", "forced mode 5 TABLEUPDATE should bypass the conflict");
        expect(value_for("cforced5one") == "Mode5One", "forced mode 5 update should write the conflicting row");
        expect(value_for("cforced5two") == "Mode5Two", "forced mode 5 update should write the unaffected row");
        expect(value_for("lset3") == "true", "mode 3 should remain enabled for conflict testing");
        expect(value_for("lconflict3") == "false", "mode 3 TABLEUPDATE should reject an external row change");
        expect(value_for("cpending3") == "Mode3Pending", "mode 3 should retain pending values after conflict");
        expect(value_for("lrevert3") == "true", "TABLEREVERT should discard a conflicted mode-3 row");
        expect(value_for("cafterrevert3") == "ExternalThree", "TABLEREVERT should expose the competing disk value");

        const auto after_resume = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(after_resume.ok && after_resume.table.records.size() == 2U &&
                   after_resume.table.records[0].values[0].display_value == "ExternalThree" &&
                   after_resume.table.records[1].values[0].display_value == "Mode5Two",
               "conflict rejection and revert should leave only committed disk values");

        fs::remove_all(temp_root, ignored);
    }

    void test_local_optimistic_row_buffering()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_row_buffering";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "row_buffering.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Before"}, {"Other"}});
        expect(create_result.ok, "row buffering fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "lSet = CURSORSETPROP('Buffering', 3, 'people')\n"
            "nMode = CURSORGETPROP('Buffering', 'people')\n"
            "REPLACE NAME WITH 'RowPending' IN people\n"
            "cPending = people.NAME\n"
            "TABLEUPDATE(.T., .T., 'people')\n"
            "cCommitted = people.NAME\n"
            "REPLACE NAME WITH 'RowReverted' IN people\n"
            "cBeforeRevert = people.NAME\n"
            "TABLEREVERT(.T., 'people')\n"
            "cReverted = people.NAME\n"
            "DELETE IN people\n"
            "lDeleted = DELETED()\n"
            "TABLEREVERT(.T., 'people')\n"
            "lDeleteReverted = DELETED()\n"
            "REPLACE NAME WITH 'MoveCommitted' IN people\n"
            "GO 2 IN people\n"
            "cSecond = people.NAME\n"
            "GO 1 IN people\n"
            "cAfterMove = people.NAME\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 5U});
        const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(paused.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "row buffering regression should pause with the first row change pending");

        const auto before_update = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
        expect(before_update.ok && before_update.table.records.size() == 2U &&
                   before_update.table.records[0].values[0].display_value == "Before",
               "row buffering should leave disk unchanged before TABLEUPDATE");

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "row buffering regression should complete after resume");
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lset") == "true", "CURSORSETPROP Buffering 3 should succeed");
        expect(value_for("nmode") == "3", "CURSORGETPROP Buffering should report mode 3");
        expect(value_for("cpending") == "RowPending", "row buffering should expose pending field values");
        expect(value_for("ccommitted") == "RowPending", "TABLEUPDATE should commit the current row");
        expect(value_for("cbeforerevert") == "RowReverted", "row buffering should expose the second pending value");
        expect(value_for("creverted") == "RowPending", "TABLEREVERT should discard the current row change");
        expect(value_for("ldeleted") == "true", "row buffering should expose a pending deletion");
        expect(value_for("ldeletereverted") == "false", "TABLEREVERT should discard a pending deletion");
        expect(value_for("csecond") == "Other", "moving off a buffered row should select the target row");
        expect(value_for("caftermove") == "MoveCommitted", "moving off a changed row should commit it");

        const auto after_resume = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
        expect(after_resume.ok && after_resume.table.records.size() == 2U &&
                   after_resume.table.records[0].values[0].display_value == "MoveCommitted" &&
                   after_resume.table.records[1].values[0].display_value == "Other",
               "row buffering should persist explicit and navigation commits only");

        fs::remove_all(temp_root, ignored);
    }

    void test_local_pessimistic_row_buffering()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_pessimistic_row_buffering";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "pessimistic_row_buffering.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Before"}, {"Other"}});
        expect(create_result.ok, "pessimistic row buffering fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "SET REPROCESS TO 0\n"
            "lSet = CURSORSETPROP('Buffering', 2, 'people')\n"
            "nMode = CURSORGETPROP('Buffering', 'people')\n"
            "REPLACE NAME WITH 'Pending' IN people\n"
            "cPending = people.NAME\n"
            "lLockedPending = ISRLOCKED('people')\n"
            "TABLEUPDATE(.T., .T., 'people')\n"
            "lUnlockedUpdate = !ISRLOCKED('people')\n"
            "REPLACE NAME WITH 'Reverted' IN people\n"
            "lLockedRevert = ISRLOCKED('people')\n"
            "TABLEREVERT(.T., 'people')\n"
            "lUnlockedRevert = !ISRLOCKED('people')\n"
            "REPLACE NAME WITH 'MovedCommit' IN people\n"
            "GO 2 IN people\n"
            "cSecond = people.NAME\n"
            "lUnlockedMove = !ISRLOCKED('people')\n"
            "GO 1 IN people\n"
            "REPLACE NAME WITH 'Held' IN people\n"
            "lHeld = ISRLOCKED('people')\n"
            "SET DATASESSION TO 2\n"
            "USE '" + table_path.string() + "' ALIAS other\n"
            "SET REPROCESS TO 0\n"
            "= CURSORSETPROP('Buffering', 2, 'other')\n"
            "GO 1 IN other\n"
            "lCompeting = .T.\n"
            "TRY\n"
            "    REPLACE NAME WITH 'Blocked' IN other\n"
            "CATCH TO loError\n"
            "    lCompeting = .F.\n"
            "ENDTRY\n"
            "SET DATASESSION TO 1\n"
            "lStillHeld = ISRLOCKED('people')\n"
            "TABLEUPDATE(.T., .T., 'people')\n"
            "SET DATASESSION TO 2\n"
            "REPLACE NAME WITH 'SecondCanEdit' IN other\n"
            "lSecondUpdate = TABLEUPDATE(.T., .T., 'other')\n"
            "SET DATASESSION TO 1\n"
            "cFinal = people.NAME\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(program_path.string(), temp_root.string()));
        session.add_breakpoint({.file_path = program_path.string(), .line = 7U});
        const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(paused.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "pessimistic row buffering regression should pause before the first update");

        const auto before_update = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
        expect(before_update.ok && before_update.table.records.size() == 2U &&
                   before_update.table.records[0].values[0].display_value == "Before",
               "pessimistic row buffering should leave disk unchanged before TABLEUPDATE");

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "pessimistic row buffering regression should complete after resume");
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("lset") == "true", "CURSORSETPROP Buffering 2 should succeed");
        expect(value_for("nmode") == "2", "CURSORGETPROP Buffering should report mode 2");
        expect(value_for("cpending") == "Pending", "pessimistic row buffering should expose pending field values");
        expect(value_for("llockedpending") == "true", "mode 2 should retain a record lock while editing");
        expect(value_for("lunlockedupdate") == "true", "TABLEUPDATE should release the mode-2 record lock");
        expect(value_for("llockedrevert") == "true", "mode 2 should lock a row before a revert");
        expect(value_for("lunlockedrevert") == "true", "TABLEREVERT should release the mode-2 record lock");
        expect(value_for("csecond") == "Other", "navigation should select the target row after a mode-2 commit");
        expect(value_for("lunlockedmove") == "true", "navigation commit should release the mode-2 record lock");
        expect(value_for("lheld") == "true", "mode 2 should retain the lock for a pending row");
        expect(value_for("lcompeting") == "false", "a competing data session should not edit a locked row");
        expect(value_for("lstillheld") == "true", "a failed competing edit should not release the owner lock");
        expect(value_for("lsecondupdate") == "true", "the competing session should edit after the owner commits");
        expect(value_for("cfinal") == "SecondCanEdit", "the competing session commit should persist after release");

        const auto after_resume = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
        expect(after_resume.ok && after_resume.table.records.size() == 2U &&
                   after_resume.table.records[0].values[0].display_value == "SecondCanEdit" &&
                   after_resume.table.records[1].values[0].display_value == "Other",
               "pessimistic row buffering should persist only committed values");

        fs::remove_all(temp_root, ignored);
    }
}
