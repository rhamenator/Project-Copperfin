void test_sql_result_cursor_derived_string_temporary_order_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_derived_string_orders";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_derived_string_orders.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "lSeekLeft = SEEK('CHA', 'sqlcust', 'UPPER(LEFT(NAME, 3))')\n"
        "nRecLeft = RECNO()\n"
        "GO TOP IN sqlcust\n"
        "lSeekSub = SEEK('RAV', 'sqlcust', 'UPPER(SUBSTR(NAME, 2, 3))')\n"
        "nRecSub = RECNO()\n"
        "GO TOP IN sqlcust\n"
        "lSeekPad = SEEK('000BRAVO', 'sqlcust', \"UPPER(PADL(NAME, 8, '0'))\")\n"
        "nRecPad = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL derived-string temporary-order parity script should complete");
    expect(state.sql_connections.empty(), "SQL derived-string temporary-order parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_left = state.globals.find("lseekleft");
    const auto rec_left = state.globals.find("nrecleft");
    const auto seek_sub = state.globals.find("lseeksub");
    const auto rec_sub = state.globals.find("nrecsub");
    const auto seek_pad = state.globals.find("lseekpad");
    const auto rec_pad = state.globals.find("nrecpad");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL derived-string temporary-order parity");
    expect(seek_left != state.globals.end(), "LEFT()-derived SQL SEEK() result should be captured");
    expect(rec_left != state.globals.end(), "LEFT()-derived SQL SEEK() RECNO() should be captured");
    expect(seek_sub != state.globals.end(), "SUBSTR()-derived SQL SEEK() result should be captured");
    expect(rec_sub != state.globals.end(), "SUBSTR()-derived SQL SEEK() RECNO() should be captured");
    expect(seek_pad != state.globals.end(), "PADL()-derived SQL SEEK() result should be captured");
    expect(rec_pad != state.globals.end(), "PADL()-derived SQL SEEK() RECNO() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL derived-string temporary-order parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before derived-string SQL seek checks");
    }
    if (seek_left != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_left->second) == "true", "SEEK() should match LEFT()-derived temporary SQL order keys");
    }
    if (rec_left != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_left->second) == "3", "LEFT()-derived SQL SEEK() should land on the expected exact match");
    }
    if (seek_sub != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_sub->second) == "true", "SEEK() should match SUBSTR()-derived temporary SQL order keys");
    }
    if (rec_sub != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_sub->second) == "2", "SUBSTR()-derived SQL SEEK() should land on the expected exact match");
    }
    if (seek_pad != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_pad->second) == "true", "SEEK() should match PADL()-derived temporary SQL order keys");
    }
    if (rec_pad != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_pad->second) == "2", "PADL()-derived SQL SEEK() should land on the expected exact match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after derived-string SQL seek checks");
    }

    fs::remove_all(temp_root, ignored);
}
