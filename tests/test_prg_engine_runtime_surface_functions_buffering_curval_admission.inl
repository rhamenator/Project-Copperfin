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

    void test_lupdate_reads_table_header_date_and_designator_variants()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_lupdate";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path blank_table_path = temp_root / "fresh.dbf";
        const fs::path program_path = temp_root / "lupdate.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Alpha"}});
        expect(create_result.ok, "LUPDATE dated fixture should be writable");
        // Force a specific, deterministic date rather than relying on
        // create_dbf_table_file()'s now-stamped creation-time date (today).
        write_dbf_last_update_bytes(table_path, 124U, 3U, 15U); // 1900 + 124 = 2024-03-15

        const auto create_blank_result = copperfin::vfp::create_dbf_table_file(
            blank_table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Beta"}});
        expect(create_blank_result.ok, "LUPDATE blank fixture should be writable");
        // create_dbf_table_file() now stamps today's date on creation
        // (RQ-CF-PRG-012); force a genuinely undated header to keep
        // exercising LUPDATE()'s blank-date fallback path.
        write_dbf_last_update_bytes(blank_table_path, 0U, 0U, 0U);

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people IN 1\n"
            "USE '" + blank_table_path.string() + "' ALIAS fresh IN 2\n"
            "SELECT people\n"
            "dSelected = LUPDATE()\n"
            "dByAlias = LUPDATE('fresh')\n"
            "dByWorkArea = LUPDATE(1)\n"
            "dEmptyWorkArea = LUPDATE(9)\n"
            "PUBLIC nErrorCount, nErrorCode1, nErrorCode2\n"
            "nErrorCount = 0\n"
            "nErrorCode1 = 0\n"
            "nErrorCode2 = 0\n"
            "ON ERROR DO HandleLupdateError\n"
            "dBadAlias = LUPDATE('NoSuchAlias')\n"
            "dQuotedNumericAlias = LUPDATE('1')\n"
            "ON ERROR\n"
            "RETURN\n"
            "PROCEDURE HandleLupdateError\n"
            "nErrorCount = nErrorCount + 1\n"
            "IF nErrorCount = 1\n"
            "    nErrorCode1 = ERROR()\n"
            "ELSE\n"
            "    nErrorCode2 = ERROR()\n"
            "ENDIF\n"
            "RETURN\n"
            "ENDPROC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("LUPDATE regression should complete: ") + state.message);

        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };

        expect(value_for("dselected") == "03/15/2024",
               "LUPDATE() with no arguments should return the selected work area's table header date");
        expect(value_for("dbyalias").empty(),
               "LUPDATE('fresh') should return a blank date for a table whose header was never dated");
        expect(value_for("dbyworkarea") == "03/15/2024",
               "LUPDATE(nWorkArea) should return the given work area's table header date");
        expect(value_for("demptyworkarea").empty(),
               "LUPDATE(nWorkArea) should return a blank date when no table is open in that valid work area");
        expect(value_for("nerrorcount") == "2",
               "LUPDATE(cTableAlias) with an alias that isn't open should raise a runtime error rather than returning blank");
        expect(value_for("nerrorcode1") == "13",
               "LUPDATE() with an unresolvable alias should raise VFP error 13 (alias not found)");
        expect(value_for("nerrorcode2") == "13",
               "LUPDATE('1'), a quoted character alias that looks numeric, should raise error 13 like any other "
               "unresolved alias rather than being misread as work area 1");

        fs::remove_all(temp_root, ignored);
    }

    void test_lupdate_respects_set_date_format()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_lupdate_set_date";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "lupdate_set_date.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Alpha"}});
        expect(create_result.ok, "LUPDATE SET DATE fixture should be writable");
        write_dbf_last_update_bytes(table_path, 124U, 3U, 15U); // 1900 + 124 = 2024-03-15

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "dDefaultOrder = LUPDATE()\n"
            "SET DATE TO DMY\n"
            "dDmyOrder = LUPDATE()\n"
            "SET CENTURY OFF\n"
            "dDmyNoCentury = LUPDATE()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("LUPDATE SET DATE regression should complete: ") + state.message);

        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };

        expect(value_for("ddefaultorder") == "03/15/2024",
               "LUPDATE() should render MDY by default, matching the other date-producing functions");
        expect(value_for("ddmyorder") == "15/03/2024",
               "LUPDATE() should honor SET DATE TO DMY rather than always rendering MDY");
        expect(value_for("ddmynocentury") == "15/03/24",
               "LUPDATE() should honor SET CENTURY OFF in combination with the active date order");

        fs::remove_all(temp_root, ignored);
    }

    void test_lupdate_designator_edge_cases()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_lupdate_designator_edge_cases";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "lupdate_designator_edge_cases.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U},
             {.name = "AREACUR", .type = 'Y', .length = 8U}},
            {{"Alpha", "1.0000"}});
        expect(create_result.ok, "LUPDATE designator-edge-case fixture should be writable");
        write_dbf_last_update_bytes(table_path, 124U, 3U, 15U); // 1900 + 124 = 2024-03-15

        // AREACUR holds the Currency value 1: value_as_string() would render it as
        // "1.0000", which a naive integer-text parse rejects even though it names the
        // open work area 1. cSourcePathAlias uses the table's own on-disk path, which
        // is not an open alias and must be rejected by the strict cTableAlias contract
        // rather than falling back to path-based cursor resolution.
        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people IN 1\n"
            "dByCurrencyWorkArea = LUPDATE(people.AREACUR)\n"
            "PUBLIC nErrorCount, nErrorCode\n"
            "nErrorCount = 0\n"
            "nErrorCode = 0\n"
            "ON ERROR DO HandleLupdateDesignatorError\n"
            "dBySourcePath = LUPDATE('" + table_path.string() + "')\n"
            "ON ERROR\n"
            "RETURN\n"
            "PROCEDURE HandleLupdateDesignatorError\n"
            "nErrorCount = nErrorCount + 1\n"
            "nErrorCode = ERROR()\n"
            "RETURN\n"
            "ENDPROC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("LUPDATE designator-edge-case regression should complete: ") + state.message);

        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };

        expect(value_for("dbycurrencyworkarea") == "03/15/2024",
               "LUPDATE(nWorkArea) should resolve a Currency-typed work-area value by its numeric value, "
               "not by round-tripping through its rendered text");
        expect(value_for("nerrorcount") == "1",
               "LUPDATE() given the table's own on-disk source path should raise an error rather than "
               "silently resolving it as if it were an open alias");
        expect(value_for("nerrorcode") == "13",
               "LUPDATE(sourcePath) should raise VFP error 13 (alias not found), matching the strict "
               "cTableAlias contract");

        fs::remove_all(temp_root, ignored);
    }

    void test_curval_verified_commit_overlay_survives_cursor_reopen()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_reopen";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "curval_verified_reopen.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U}},
            {{"Before"}});
        expect(create_result.ok, "strict CURVAL reopen fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'Committed' IN people\n"
            "=TABLEUPDATE(.T., .T., 'people')\n"
            "cFirstRead = CURVAL('NAME', 'people')\n"
            "USE IN people\n"
            "USE '" + table_path.string() + "' ALIAS people\n"
            "cReopenedRead = CURVAL('NAME', 'people')\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, std::string("strict CURVAL reopen regression should complete: ") + state.message);
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("cfirstread") == "Committed",
               "strict CURVAL should observe its own cursor's committed value before reopen");
        expect(value_for("creopenedread") == "Committed",
               "RQ-CF-PRG-011: strict CURVAL should preserve the committed admitted value after reopen");
        fs::remove_all(temp_root, ignored);
    }

    void test_curval_verified_admission_patch_tracks_second_cursor_memo_and_rollback()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_admission";
        const fs::path table_path = temp_root / "people.dbf";
        fs::path memo_path = table_path;
        memo_path.replace_extension(".fpt");
        const fs::path program_path = temp_root / "curval_verified_admission.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            {{.name = "NAME", .type = 'C', .length = 24U},
             {.name = "NOTES", .type = 'M', .length = 4U}},
            {{"Before", "Initial memo"}});
        expect(create_result.ok && fs::exists(memo_path),
               "strict CURVAL admission fixture should create DBF and memo sidecar");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS writer\n"
            "USE '" + table_path.string() + "' AGAIN ALIAS reader\n"
            "=CURSORSETPROP('Buffering', 5, 'writer')\n"
            "REPLACE NAME WITH 'Committed' IN writer\n"
            "REPLACE NOTES WITH 'Committed memo' IN writer\n"
            "=TABLEUPDATE(.T., .T., 'writer')\n"
            "cSecondName = CURVAL('NAME', 'reader')\n"
            "cSecondMemo = CURVAL('NOTES', 'reader')\n"
            "=CURSORSETPROP('Buffering', 5, 'reader')\n"
            "SET DATASESSION TO 2\n"
            "USE '" + table_path.string() + "' ALIAS reader\n"
            "=CURSORSETPROP('Buffering', 5, 'reader')\n"
            "REPLACE NAME WITH 'Later' IN reader\n"
            "=TABLEUPDATE(.T., .T., 'reader')\n"
            "SET DATASESSION TO 1\n"
            "cWriterAfterReader = CURVAL('NAME', 'writer')\n"
            "BEGIN TRANSACTION\n"
            "REPLACE NAME WITH 'RolledBack' IN writer\n"
            "REPLACE NOTES WITH 'Rolled back memo' IN writer\n"
            "=TABLEUPDATE(.T., .T., 'writer')\n"
            "ROLLBACK\n"
            "cRollbackWriterName = CURVAL('NAME', 'writer')\n"
            "cRollbackWriterMemo = CURVAL('NOTES', 'writer')\n"
            "USE IN writer\n"
            "USE '" + table_path.string() + "' ALIAS reopened\n"
            "cRollbackName = CURVAL('NAME', 'reopened')\n"
            "cRollbackMemo = CURVAL('NOTES', 'reopened')\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        options.verified_file_byte_overrides.emplace(memo_path.string(), read_text(memo_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("strict CURVAL admission patch regression should complete: ") + state.message);
        const auto value_for = [&](const std::string &name) -> std::string
        {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string{} : copperfin::runtime::format_value(found->second);
        };
        expect(value_for("csecondname") == "Committed" &&
                   value_for("csecondmemo") == "Committed memo",
               "strict CURVAL on a second cursor should use the committed DBF and memo admission patch");
        expect(value_for("cwriterafterreader") == "Later",
               "a later verified commit from a second alias should retire the first alias's stale overlay");
        expect(value_for("crollbackwritername") == "Later" &&
                   value_for("crollbackwritermemo") == "Committed memo",
               "ROLLBACK should clear the originating cursor overlay before it reads restored admitted bytes");
        expect(value_for("crollbackname") == "Later" &&
                   value_for("crollbackmemo") == "Committed memo",
               "ROLLBACK should restore both DBF and memo admitted bytes before a verified cursor reopens");
        fs::remove_all(temp_root, ignored);
    }

    void test_curval_verified_command_undo_restores_admission_bytes()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_undo";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "curval_verified_undo.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        expect(copperfin::vfp::create_dbf_table_file(
                   table_path.string(), {{.name = "NAME", .type = 'C', .length = 24U}}, {{"Before"}}).ok,
               "strict CURVAL command-undo fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'Committed' IN people\n"
            "=TABLEUPDATE(.T., .T., 'people')\n"
            "UNDO\n"
            "cAfterUndo = CURVAL('NAME', 'people')\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("strict CURVAL command-undo regression should complete: ") + state.message);
        const auto found = state.globals.find("cafterundo");
        expect(found != state.globals.end() && copperfin::runtime::format_value(found->second) == "Before",
               "UNDO must restore the pre-commit verified admission before CURVAL reads it");
        fs::remove_all(temp_root, ignored);
    }

    void test_curval_verified_failed_command_undo_keeps_admission_bytes()
    {
#if defined(_WIN32)
        // This regression deliberately removes POSIX write permission between
        // the commit and UNDO.  Windows ACL mutation is not portable enough
        // for this focused test; protected Windows CI covers the production
        // undo path separately.
        return;
#else
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_undo_failure";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "curval_verified_undo_failure.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        expect(copperfin::vfp::create_dbf_table_file(
                   table_path.string(), {{.name = "NAME", .type = 'C', .length = 24U}}, {{"Before"}}).ok,
               "strict CURVAL failed-command-undo fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'Committed' IN people\n"
            "=TABLEUPDATE(.T., .T., 'people')\n"
            "PUBLIC nUndoErrors\n"
            "nUndoErrors = 0\n"
            "ON ERROR DO HandleUndoError\n"
            "UNDO\n"
            "ON ERROR\n"
            "cAfterFailedUndo = CURVAL('NAME', 'people')\n"
            "RETURN\n"
            "PROCEDURE HandleUndoError\n"
            "nUndoErrors = nUndoErrors + 1\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        session.add_breakpoint({.file_path = program_path.string(), .line = 8U});
        const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(paused.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "failed-command-undo regression should pause before UNDO");

        fs::permissions(table_path, fs::perms::owner_write, fs::perm_options::remove, ignored);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        fs::permissions(table_path, fs::perms::owner_write, fs::perm_options::add, ignored);
        expect(state.completed,
               std::string("strict CURVAL failed-command-undo regression should complete: ") + state.message);
        const auto after = state.globals.find("cafterfailedundo");
        const auto errors = state.globals.find("nundoerrors");
        expect(errors != state.globals.end() && copperfin::runtime::format_value(errors->second) == "1",
               "the permission-denied replay fixture must drive the UNDO error handler exactly once");
        expect(after != state.globals.end() && copperfin::runtime::format_value(after->second) == "Committed",
               "a failed UNDO must retain the current verified admission instead of publishing pre-command bytes");
        fs::remove_all(temp_root, ignored);
#endif
    }

    void test_curval_verified_missing_command_undo_backup_keeps_admission_bytes()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_undo_missing_backup";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "curval_verified_undo_missing_backup.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        expect(copperfin::vfp::create_dbf_table_file(
                   table_path.string(), {{.name = "NAME", .type = 'C', .length = 24U}}, {{"Before"}}).ok,
               "strict CURVAL missing-undo-backup fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS people\n"
            "=CURSORSETPROP('Buffering', 5, 'people')\n"
            "REPLACE NAME WITH 'Committed' IN people\n"
            "=TABLEUPDATE(.T., .T., 'people')\n"
            "PUBLIC nUndoErrors\n"
            "nUndoErrors = 0\n"
            "ON ERROR DO HandleUndoError\n"
            "UNDO\n"
            "ON ERROR\n"
            "cAfterMissingBackupUndo = CURVAL('NAME', 'people')\n"
            "RETURN\n"
            "PROCEDURE HandleUndoError\n"
            "nUndoErrors = nUndoErrors + 1\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        session.add_breakpoint({.file_path = program_path.string(), .line = 8U});
        const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(paused.reason == copperfin::runtime::DebugPauseReason::breakpoint,
               "missing-command-undo-backup regression should pause before UNDO");

        fs::path backup_path;
        // make_runtime_session_options() sets options.temp_directory to
        // working_directory / "runtime-temp"; command-undo backups land
        // under that runtime temp root, not directly under temp_root.
        for (fs::recursive_directory_iterator iterator(temp_root / "runtime-temp" / "command_undo", ignored), end;
             !ignored && iterator != end;
             iterator.increment(ignored))
        {
            if (iterator->is_regular_file(ignored) && iterator->path().filename().string().starts_with("backup_"))
            {
                backup_path = iterator->path();
                break;
            }
        }
        expect(!backup_path.empty(), "missing-command-undo-backup regression should locate the staged backup");
        if (!backup_path.empty())
        {
            fs::remove(backup_path, ignored);
        }

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("strict CURVAL missing-command-undo-backup regression should complete: ") + state.message);
        const auto after = state.globals.find("caftermissingbackupundo");
        const auto errors = state.globals.find("nundoerrors");
        expect(errors != state.globals.end() && copperfin::runtime::format_value(errors->second) == "1",
               "a missing undo backup must drive the UNDO error handler exactly once");
        expect(after != state.globals.end() && copperfin::runtime::format_value(after->second) == "Committed",
               "a missing undo backup must retain the current verified admission instead of publishing pre-command bytes");
        fs::remove_all(temp_root, ignored);
    }

    void test_curval_verified_force_commit_merges_other_alias_admission()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_force_merge";
        const fs::path table_path = temp_root / "people.dbf";
        const fs::path program_path = temp_root / "curval_verified_force_merge.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        expect(copperfin::vfp::create_dbf_table_file(
                   table_path.string(),
                   {{.name = "NAME", .type = 'C', .length = 24U},
                    {.name = "AMOUNT", .type = 'N', .length = 12U, .decimal_count = 2U}},
                   {{"Before", "1.00"}}).ok,
               "strict CURVAL force-merge fixture should be writable");

        write_text(
            program_path,
            "USE '" + table_path.string() + "' ALIAS first\n"
            "USE '" + table_path.string() + "' AGAIN ALIAS second\n"
            "=CURSORSETPROP('Buffering', 5, 'first')\n"
            "=CURSORSETPROP('Buffering', 5, 'second')\n"
            "REPLACE NAME WITH 'First committed' IN first\n"
            "REPLACE AMOUNT WITH 2 IN second\n"
            "=TABLEUPDATE(.T., .T., 'second')\n"
            "=TABLEUPDATE(.T., .T., 'first')\n"
            "cName = CURVAL('NAME', 'first')\n"
            "nAmount = CURVAL('AMOUNT', 'first')\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("strict CURVAL force-merge regression should complete: ") + state.message);
        const auto name = state.globals.find("cname");
        const auto amount = state.globals.find("namount");
        expect(name != state.globals.end() && amount != state.globals.end() &&
                   copperfin::runtime::format_value(name->second) == "First committed" &&
                   copperfin::runtime::format_value(amount->second) == "2",
               "force-committing one buffered alias must preserve another alias's admitted field for CURVAL");
        fs::remove_all(temp_root, ignored);
    }

    void test_curval_verified_rollback_deduplicates_windows_case_aliases()
    {
#if !defined(_WIN32)
        // Case-distinct paths identify distinct files on POSIX.  The
        // case-insensitive identity contract is Windows-specific and is
        // exercised by the protected Windows matrix.
        return;
#else
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_case_aliases";
        const fs::path table_path = temp_root / "people.dbf";
        fs::path upper_case_path = table_path;
        upper_case_path.replace_filename("PEOPLE.DBF");
        const fs::path program_path = temp_root / "curval_verified_case_aliases.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        expect(copperfin::vfp::create_dbf_table_file(
                   table_path.string(), {{.name = "NAME", .type = 'C', .length = 24U}}, {{"Before"}}).ok,
               "Windows case-alias strict CURVAL fixture should be writable");

        write_text(
            program_path,
            "USE '" + upper_case_path.string() + "' ALIAS first\n"
            "USE '" + table_path.string() + "' AGAIN ALIAS second\n"
            "=CURSORSETPROP('Buffering', 5, 'first')\n"
            "=CURSORSETPROP('Buffering', 5, 'second')\n"
            "BEGIN TRANSACTION\n"
            "REPLACE NAME WITH 'First' IN first\n"
            "=TABLEUPDATE(.T., .T., 'first')\n"
            "REPLACE NAME WITH 'Second' IN second\n"
            "=TABLEUPDATE(.T., .T., 'second')\n"
            "ROLLBACK\n"
            "cAfterRollback = CURVAL('NAME', 'second')\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(table_path.string(), read_text(table_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("Windows case-alias strict CURVAL rollback regression should complete: ") + state.message);
        const auto found = state.globals.find("cafterrollback");
        expect(found != state.globals.end() && copperfin::runtime::format_value(found->second) == "Before",
               "ROLLBACK must restore the original admission once when Windows aliases differ only by path casing");
        fs::remove_all(temp_root, ignored);
#endif
    }

    void test_curval_verified_rollback_preserves_case_distinct_posix_admissions()
    {
#if defined(_WIN32)
        // Windows file names are case-insensitive, so this POSIX-only
        // admission-identity boundary cannot be represented there.
        return;
#else
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_surface_curval_verified_case";
        const fs::path upper_path = temp_root / "UPPER.dbf";
        const fs::path lower_path = temp_root / "upper.dbf";
        const fs::path program_path = temp_root / "curval_verified_case.prg";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const auto upper_create_result = copperfin::vfp::create_dbf_table_file(
            upper_path.string(), {{.name = "NAME", .type = 'C', .length = 24U}}, {{"Upper"}});
        const auto lower_create_result = copperfin::vfp::create_dbf_table_file(
            lower_path.string(), {{.name = "NAME", .type = 'C', .length = 24U}}, {{"Lower"}});
        expect(upper_create_result.ok && lower_create_result.ok,
               "case-distinct strict CURVAL fixtures should be writable on POSIX");

        write_text(
            program_path,
            "USE '" + upper_path.string() + "' ALIAS upper\n"
            "=CURSORSETPROP('Buffering', 5, 'upper')\n"
            "BEGIN TRANSACTION\n"
            "REPLACE NAME WITH 'Transient' IN upper\n"
            "=TABLEUPDATE(.T., .T., 'upper')\n"
            "ROLLBACK\n"
            "USE '" + lower_path.string() + "' ALIAS lower\n"
            "cLower = CURVAL('NAME', 'lower')\n"
            "RETURN\n");
        auto options = make_runtime_session_options(program_path.string(), temp_root.string());
        options.require_verified_file_byte_overrides = true;
        options.verified_file_byte_overrides.emplace(upper_path.string(), read_text(upper_path));
        options.verified_file_byte_overrides.emplace(lower_path.string(), read_text(lower_path));
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            std::move(options));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("case-distinct strict CURVAL rollback regression should complete: ") + state.message);
        const auto found = state.globals.find("clower");
        expect(found != state.globals.end() && copperfin::runtime::format_value(found->second) == "Lower",
               "rollback for UPPER.dbf must not remove the distinct upper.dbf admission entry on POSIX");
        fs::remove_all(temp_root, ignored);
#endif
    }

