// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"

namespace copperfin::runtime_surface_tests
{
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
            "lUnsupported = CURSORSETPROP('Buffering', 2, 'people')\n"
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
        expect(value_for("csecond") == "Other", "moving off a buffered row should select the target row");
        expect(value_for("caftermove") == "MoveCommitted", "moving off a changed row should commit it");

        const auto after_resume = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
        expect(after_resume.ok && after_resume.table.records.size() == 2U &&
                   after_resume.table.records[0].values[0].display_value == "MoveCommitted" &&
                   after_resume.table.records[1].values[0].display_value == "Other",
               "row buffering should persist explicit and navigation commits only");

        fs::remove_all(temp_root, ignored);
    }
}
