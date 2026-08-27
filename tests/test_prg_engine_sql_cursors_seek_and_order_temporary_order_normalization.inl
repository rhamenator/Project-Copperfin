void test_sql_result_cursor_temporary_order_normalization_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_normalization_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_normalization_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "lSeekFn = SEEK('bravo', 'sqlcust', 'UPPER(NAME)')\n"
        "nSeekFnRec = RECNO()\n"
        "GO TOP\n"
        "SET ORDER TO UPPER(NAME)\n"
        "SEEK 'charlie'\n"
        "lFoundCmd = FOUND()\n"
        "nSeekCmdRec = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL temporary-order normalization parity script should complete");
    expect(state.sql_connections.empty(), "SQL temporary-order normalization parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");
    const auto found_cmd = state.globals.find("lfoundcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL temporary-order normalization parity");
    expect(seek_fn != state.globals.end(), "SEEK() with UPPER(NAME) should be captured for a SQL cursor");
    expect(seek_fn_rec != state.globals.end(), "RECNO() after SQL SEEK() with UPPER(NAME) should be captured");
    expect(found_cmd != state.globals.end(), "FOUND() after command SEEK with SQL UPPER(NAME) order should be captured");
    expect(seek_cmd_rec != state.globals.end(), "RECNO() after command SEEK with SQL UPPER(NAME) order should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL temporary-order normalization parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL temporary-order normalization checks");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should case-fold search keys for SQL UPPER(NAME) temporary orders");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should position the SQL cursor on the normalized match");
    }
    if (found_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_cmd->second) == "true", "command SEEK should case-fold search keys for SQL UPPER(NAME) orders");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should position the SQL cursor on the normalized match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL temporary-order normalization checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]: charlie -> found") != std::string::npos;
        }),
        "SQL temporary-order normalization should emit runtime.order and runtime.seek metadata");

    fs::remove_all(temp_root, ignored);
}
