// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"

namespace copperfin::runtime_surface_tests
{
    void test_curval_evaluates_on_disk_record()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "curval.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U},
             {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}},
            {{"Before", "1.00"}});
        expect(create_result.ok, "CURVAL fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'Buffered' IN people\n"
            "REPLACE AMOUNT WITH 2 IN people\n"
            "cDiskName = CURVAL('NAME', 'people')\n"
            "nDiskAmount = CURVAL('AMOUNT', 1)\n"
            "cDiskExpression = CURVAL('people.NAME + '' value''', 'people')\n"
            "=TABLEUPDATE(.T., .T., 'people')\n"
            "cCommittedName = CURVAL('NAME', 'people')\n"
            "RETURN\n");
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("CURVAL regression should complete: ") + state.message);
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("cdiskname") == "Before" && value_for("ndiskamount") == "1" &&
                   value_for("cdiskexpression") == "Before value",
               "CURVAL should evaluate against on-disk rather than buffered values");
        expect(value_for("ccommittedname") == "Buffered",
               "CURVAL should observe the committed on-disk value after TABLEUPDATE");
        fs::remove_all(temp_root, ignored);
    }

    void test_curval_uses_verified_post_commit_session_image()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "curval_verified.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Before"}});
        expect(create_result.ok, "strict CURVAL fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'Committed' IN people\n"
            "=TABLEUPDATE(.T., .T., 'people')\n"
            "cCommittedName = CURVAL('NAME', 'people')\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("strict CURVAL regression should complete: ") + state.message);
        const auto found = state.globals.find("ccommittedname");
        expect(found != state.globals.end() && copperfin::runtime::format_value(found->second) == "Committed",
               "strict CURVAL should expose the session-owned committed value rather than stale admitted bytes");
        fs::remove_all(temp_root, ignored);
    }

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
