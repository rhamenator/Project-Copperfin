void test_set_filter_defers_sql_cursor_evaluation_until_navigation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_filter_deferred";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_filter_deferred.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecPeople = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlcust\n"
        "LOCATE FOR NAME = 'CHARLIE'\n"
        "SET FILTER TO ID <= 2\n"
        "nBeforeOnlyRec = RECNO()\n"
        "cBeforeOnlyName = NAME\n"
        "lBeforeOnlyFound = FOUND()\n"
        "lBeforeOnlyBof = BOF()\n"
        "lBeforeOnlyEof = EOF()\n"
        "GO TOP\n"
        "nBeforeOnlyTop = RECNO()\n"
        "SET FILTER TO\n"
        "LOCATE FOR NAME = 'MISSING'\n"
        "GO 1\n"
        "SET FILTER TO ID >= 2\n"
        "nAfterOnlyRec = RECNO()\n"
        "cAfterOnlyName = NAME\n"
        "lAfterOnlyFound = FOUND()\n"
        "lAfterOnlyBof = BOF()\n"
        "lAfterOnlyEof = EOF()\n"
        "SKIP\n"
        "nAfterOnlySkip = RECNO()\n"
        "cAfterOnlySkipName = NAME\n"
        "SET FILTER TO\n"
        "GO 2\n"
        "SET FILTER TO .F.\n"
        "nEmptyRec = RECNO()\n"
        "cEmptyName = NAME\n"
        "lEmptyBof = BOF()\n"
        "lEmptyEof = EOF()\n"
        "lEmptyFound = FOUND()\n"
        "GO TOP\n"
        "nEmptyTopRec = RECNO()\n"
        "lEmptyTopBof = BOF()\n"
        "lEmptyTopEof = EOF()\n"
        "lEmptyTopFound = FOUND()\n"
        "cEmptyTopName = NAME\n"
        "nEmptyTopId = ID\n"
        "GO BOTTOM\n"
        "nEmptyBottomRec = RECNO()\n"
        "lEmptyBottomBof = BOF()\n"
        "lEmptyBottomEof = EOF()\n"
        "lEmptyBottomFound = FOUND()\n"
        "cEmptyBottomName = NAME\n"
        "nEmptyBottomId = ID\n"
        "SET FILTER TO\n"
        "GO 2\n"
        "SET FILTER TO ID >= 2\n"
        "nVisibleRec = RECNO()\n"
        "cVisibleName = NAME\n"
        "lVisibleBof = BOF()\n"
        "lVisibleEof = EOF()\n"
        "SET FILTER TO\n"
        "GO 3 IN sqlcust\n"
        "SELECT sqlother\n"
        "GO 2\n"
        "SET FILTER TO ID <= 2 IN sqlcust\n"
        "cSelectedAfterTarget = ALIAS()\n"
        "nSelectedRecAfterTarget = RECNO()\n"
        "nTargetRecAfterFilter = RECNO('sqlcust')\n"
        "cTargetNameAfterFilter = sqlcust.NAME\n"
        "GO TOP IN sqlcust\n"
        "nTargetTopAfterMove = RECNO('sqlcust')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "deferred SQL SET FILTER evaluation script should complete");
    expect(state.sql_connections.empty(), "deferred SQL SET FILTER evaluation script should disconnect its SQL handle");

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_value("nexecpeople", "1", "first SQLEXEC should succeed before deferred filter checks");
    expect_value("nexecother", "1", "second SQLEXEC should succeed before targeted deferred filter checks");
    expect_value("nbeforeonlyrec", "3", "SQL SET FILTER with visible rows only before the pointer should preserve RECNO()");
    expect_value("cbeforeonlyname", "CHARLIE", "SQL field access before navigation should retain the excluded physical row");
    expect_value("lbeforeonlyfound", "true", "SQL SET FILTER installation should preserve FOUND()");
    expect_value("lbeforeonlybof", "false", "SQL SET FILTER installation should preserve BOF()");
    expect_value("lbeforeonlyeof", "false", "SQL SET FILTER installation should preserve EOF()");
    expect_value("nbeforeonlytop", "1", "SQL GO TOP should evaluate the filter and find an earlier visible row");
    expect_value("nafteronlyrec", "1", "SQL SET FILTER with visible rows only after the pointer should preserve RECNO()");
    expect_value("cafteronlyname", "ALPHA", "deferred SQL filtering should preserve immediate field access");
    expect_value("lafteronlyfound", "false", "deferred SQL SET FILTER should preserve a false FOUND() state");
    expect_value("lafteronlybof", "false", "deferred SQL SET FILTER should not synthesize BOF()");
    expect_value("lafteronlyeof", "false", "deferred SQL SET FILTER should not synthesize EOF()");
    expect_value("nafteronlyskip", "2", "SQL SKIP should evaluate the installed filter from the preserved row");
    expect_value("cafteronlyskipname", "BRAVO", "SQL SKIP should expose the next filter-visible row");
    expect_value("nemptyrec", "2", "an empty SQL filter result should not move the pointer before navigation");
    expect_value("cemptyname", "BRAVO", "an empty SQL filter result should retain immediate field access");
    expect_value("lemptybof", "false", "an empty SQL filter result should preserve BOF() before navigation");
    expect_value("lemptyeof", "false", "an empty SQL filter result should preserve EOF() before navigation");
    expect_value("lemptyfound", "false", "an empty SQL filter result should preserve FOUND() before navigation");
    expect_value("nemptytoprec", "4", "SQL GO TOP with no filter-visible rows should move past the physical row set");
    expect_value("lemptytopbof", "false", "SQL GO TOP with no filter-visible rows in a nonempty cursor should leave BOF() false");
    expect_value("lemptytopeof", "true", "SQL GO TOP should report EOF() when no rows satisfy the filter");
    expect_value("lemptytopfound", "false", "SQL GO TOP should preserve FOUND() when no rows satisfy the filter");
    expect_value("cemptytopname", "", "SQL field access at filtered EOF should return the field's typed blank value");
    expect_value("nemptytopid", "0", "numeric SQL field access at filtered EOF should return zero");
    expect_value("nemptybottomrec", "4", "SQL GO BOTTOM with no filter-visible rows should retain the physical EOF record number");
    expect_value("lemptybottombof", "true", "SQL GO BOTTOM with no filter-visible rows should set BOF()");
    expect_value("lemptybottomeof", "true", "SQL GO BOTTOM with no filter-visible rows should retain EOF()");
    expect_value("lemptybottomfound", "false", "SQL GO BOTTOM should preserve FOUND() when no rows satisfy the filter");
    expect_value("cemptybottomname", "", "character SQL field access after empty-result GO BOTTOM should stay blank");
    expect_value("nemptybottomid", "0", "numeric SQL field access after empty-result GO BOTTOM should stay zero");
    expect_value("nvisiblerec", "2", "a current filter-visible SQL row should remain selected");
    expect_value("cvisiblename", "BRAVO", "a current filter-visible SQL row should retain field access");
    expect_value("lvisiblebof", "false", "a current filter-visible SQL row should retain BOF()");
    expect_value("lvisibleeof", "false", "a current filter-visible SQL row should retain EOF()");
    expect_value("cselectedaftertarget", "sqlother", "targeted SQL SET FILTER should preserve the selected work area");
    expect_value("nselectedrecaftertarget", "2", "targeted SQL SET FILTER should preserve the selected cursor pointer");
    expect_value("ntargetrecafterfilter", "3", "targeted SQL SET FILTER should preserve the non-selected target pointer");
    expect_value("ctargetnameafterfilter", "CHARLIE", "targeted SQL SET FILTER should preserve immediate target field access");
    expect_value("ntargettopaftermove", "1", "targeted SQL navigation should later evaluate the installed filter");
    expect_value("ldisc", "1", "SQLDISCONNECT should succeed after deferred filter checks");

    std::vector<std::string> filter_event_details;
    for (const auto &event : state.events) {
        if (event.category == "runtime.filter") {
            filter_event_details.push_back(event.detail);
        }
    }
    expect(
        filter_event_details == std::vector<std::string>{
            "ID <= 2", "OFF", "ID >= 2", "OFF", ".F.", "OFF", "ID >= 2", "OFF", "ID <= 2"},
        "deferred and targeted SQL SET FILTER changes should emit one invariant event with exact detail per statement");

    fs::remove_all(temp_root, ignored);
}
