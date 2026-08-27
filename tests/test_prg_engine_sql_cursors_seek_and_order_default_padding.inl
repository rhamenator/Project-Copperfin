void test_sql_result_cursor_default_padding_and_str_variant_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_padding_str_variants";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_padding_str_variants.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "lSeekPadDefault = SEEK('  BRAVO', 'sqlcust', 'UPPER(PADL(NAME, 7))')\n"
        "nRecPadDefault = RECNO()\n"
        "GO TOP IN sqlcust\n"
        "lSeekStrDefault = SEEK('        30', 'sqlcust', 'UPPER(STR(AMOUNT))')\n"
        "nRecStrDefault = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL default-padding and STR-variant parity script should complete");
    expect(state.sql_connections.empty(), "SQL default-padding and STR-variant parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_pad_default = state.globals.find("lseekpaddefault");
    const auto rec_pad_default = state.globals.find("nrecpaddefault");
    const auto seek_str_default = state.globals.find("lseekstrdefault");
    const auto rec_str_default = state.globals.find("nrecstrdefault");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL default-padding/STR variant parity");
    expect(seek_pad_default != state.globals.end(), "default PADL()-derived SQL SEEK() result should be captured");
    expect(rec_pad_default != state.globals.end(), "default PADL()-derived SQL SEEK() RECNO() should be captured");
    expect(seek_str_default != state.globals.end(), "default STR()-derived SQL SEEK() result should be captured");
    expect(rec_str_default != state.globals.end(), "default STR()-derived SQL SEEK() RECNO() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL default-padding/STR variant parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before default-padding/STR variant SQL seek checks");
    }
    if (seek_pad_default != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_pad_default->second) == "true", "SEEK() should match default PADL()-derived temporary SQL order keys");
    }
    if (rec_pad_default != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_pad_default->second) == "2", "default PADL()-derived SQL SEEK() should land on the expected exact match");
    }
    if (seek_str_default != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_str_default->second) == "true", "SEEK() should match default STR()-derived temporary SQL order keys");
    }
    if (rec_str_default != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_str_default->second) == "3", "default STR()-derived SQL SEEK() should land on the expected exact match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after default-padding/STR variant SQL seek checks");
    }

    fs::remove_all(temp_root, ignored);
}
