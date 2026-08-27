void test_sql_result_cursor_padr_default_and_str_decimal_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_padr_str_decimal";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_padr_str_decimal.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "lSeekPadRight = SEEK('BRAVO  ', 'sqlcust', 'UPPER(PADR(NAME, 7))')\n"
        "nRecPadRight = RECNO()\n"
        "GO TOP IN sqlcust\n"
        "lSeekStrDec = SEEK(' 30.0', 'sqlcust', 'UPPER(STR(AMOUNT, 5, 1))')\n"
        "nRecStrDec = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL PADR default and STR decimal parity script should complete");
    expect(state.sql_connections.empty(), "SQL PADR default and STR decimal parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_pad_right = state.globals.find("lseekpadright");
    const auto rec_pad_right = state.globals.find("nrecpadright");
    const auto seek_str_dec = state.globals.find("lseekstrdec");
    const auto rec_str_dec = state.globals.find("nrecstrdec");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL PADR-default/STR-decimal parity");
    expect(seek_pad_right != state.globals.end(), "default PADR()-derived SQL SEEK() result should be captured");
    expect(rec_pad_right != state.globals.end(), "default PADR()-derived SQL SEEK() RECNO() should be captured");
    expect(seek_str_dec != state.globals.end(), "decimal STR()-derived SQL SEEK() result should be captured");
    expect(rec_str_dec != state.globals.end(), "decimal STR()-derived SQL SEEK() RECNO() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL PADR-default/STR-decimal parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before PADR-default/STR-decimal SQL seek checks");
    }
    if (seek_pad_right != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_pad_right->second) == "true", "SEEK() should match default PADR()-derived temporary SQL order keys");
    }
    if (rec_pad_right != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_pad_right->second) == "2", "default PADR()-derived SQL SEEK() should land on the expected exact match");
    }
    if (seek_str_dec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_str_dec->second) == "true", "SEEK() should match decimal STR()-derived temporary SQL order keys");
    }
    if (rec_str_dec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_str_dec->second) == "3", "decimal STR()-derived SQL SEEK() should land on the expected exact match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after PADR-default/STR-decimal SQL seek checks");
    }

    fs::remove_all(temp_root, ignored);
}
