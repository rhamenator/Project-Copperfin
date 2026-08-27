void test_sql_result_cursor_plain_string_collate_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_collate_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_collate_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET ORDER TO NAME\n"
        "lMachineMiss = SEEK('bravo')\n"
        "nMachineRec = RECNO()\n"
        "SET COLLATE TO GENERAL\n"
        "GO TOP\n"
        "lGeneralHit = SEEK('bravo')\n"
        "nGeneralRec = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL plain-string collate parity script should complete");
    expect(state.sql_connections.empty(), "SQL plain-string collate parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto machine_miss = state.globals.find("lmachinemiss");
    const auto machine_rec = state.globals.find("nmachinerec");
    const auto general_hit = state.globals.find("lgeneralhit");
    const auto general_rec = state.globals.find("ngeneralrec");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL plain-string collate parity");
    expect(machine_miss != state.globals.end(), "default-collate SQL SEEK result should be captured");
    expect(machine_rec != state.globals.end(), "default-collate SQL SEEK RECNO() should be captured");
    expect(general_hit != state.globals.end(), "SET COLLATE-guided SQL SEEK result should be captured");
    expect(general_rec != state.globals.end(), "SET COLLATE-guided SQL SEEK RECNO() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL plain-string collate parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL plain-string collate checks");
    }
    if (machine_miss != state.globals.end()) {
        expect(copperfin::runtime::format_value(machine_miss->second) == "false", "default MACHINE collation should keep plain SQL NAME seeks case-sensitive");
    }
    if (machine_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(machine_rec->second) == "4", "default MACHINE collation failed SQL seek should still land at EOF");
    }
    if (general_hit != state.globals.end()) {
        expect(copperfin::runtime::format_value(general_hit->second) == "true", "SET COLLATE TO GENERAL should case-fold plain string SQL seeks");
    }
    if (general_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(general_rec->second) == "2", "SET COLLATE TO GENERAL should land on the case-folded plain SQL-string match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL plain-string collate checks");
    }

    fs::remove_all(temp_root, ignored);
}
