void test_sql_result_cursor_right_and_str_temporary_order_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_right_str_orders";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_right_str_orders.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "lSeekRight = SEEK('LIE', 'sqlcust', 'UPPER(RIGHT(NAME, 3))')\n"
        "nRecRight = RECNO()\n"
        "GO TOP IN sqlcust\n"
        "lSeekStr = SEEK(' 30', 'sqlcust', 'UPPER(STR(AMOUNT, 3))')\n"
        "nRecStr = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL RIGHT/STR temporary-order parity script should complete");
    expect(state.sql_connections.empty(), "SQL RIGHT/STR temporary-order parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_right = state.globals.find("lseekright");
    const auto rec_right = state.globals.find("nrecright");
    const auto seek_str = state.globals.find("lseekstr");
    const auto rec_str = state.globals.find("nrecstr");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL RIGHT/STR temporary-order parity");
    expect(seek_right != state.globals.end(), "RIGHT()-derived SQL SEEK() result should be captured");
    expect(rec_right != state.globals.end(), "RIGHT()-derived SQL SEEK() RECNO() should be captured");
    expect(seek_str != state.globals.end(), "STR()-derived SQL SEEK() result should be captured");
    expect(rec_str != state.globals.end(), "STR()-derived SQL SEEK() RECNO() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL RIGHT/STR temporary-order parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before RIGHT/STR SQL seek checks");
    }
    if (seek_right != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_right->second) == "true", "SEEK() should match RIGHT()-derived temporary SQL order keys");
    }
    if (rec_right != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_right->second) == "3", "RIGHT()-derived SQL SEEK() should land on the expected exact match");
    }
    if (seek_str != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_str->second) == "true", "SEEK() should match STR()-derived temporary SQL order keys");
    }
    if (rec_str != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_str->second) == "3", "STR()-derived SQL SEEK() should land on the expected exact match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after RIGHT/STR SQL seek checks");
    }

    fs::remove_all(temp_root, ignored);
}
