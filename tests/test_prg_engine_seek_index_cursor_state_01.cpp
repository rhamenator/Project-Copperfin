#include "test_prg_engine_seek_index_support.h"

namespace copperfin::seek_index_tests
{
void test_set_exact_affects_comparisons_and_seek() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_exact";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_exact.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lEqOff = 'CHARLIE' = 'CHAR'\n"
        "lEqOffReverse = 'CHAR' = 'CHARLIE'\n"
        "lSeekOff = SEEK('BR')\n"
        "nRecOff = RECNO()\n"
        "SET EXACT ON\n"
        "lEqOn = 'CHARLIE' = 'CHAR'\n"
        "lExactLeadingDiff = ' CHARLIE' = 'CHARLIE'\n"
        "lExactLeadingSame = ' CHARLIE' = ' CHARLIE'\n"
        "lExactTrailingSpaces = 'CHARLIE  ' = 'CHARLIE'\n"
        "lExactAllSpaces = '   ' = ''\n"
        "lExactDifferentLength = 'CHARLIE' = 'CHAR'\n"
        "lExactTrailingTab = ('CHARLIE' + CHR(9)) = 'CHARLIE'\n"
        "lExactTrailingNul = ('CHARLIE' + CHR(0)) = 'CHARLIE'\n"
        "lSeekOn = SEEK('BR')\n"
        "lEofOn = EOF()\n"
        "SET DATASESSION TO 2\n"
        "lEqSession2 = 'CHARLIE' = 'CHAR'\n"
        "SET DATASESSION TO 1\n"
        "lEqBack = 'CHARLIE' = 'CHAR'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET EXACT script should complete");

    const auto eq_off = state.globals.find("leqoff");
    const auto eq_off_reverse = state.globals.find("leqoffreverse");
    const auto seek_off = state.globals.find("lseekoff");
    const auto rec_off = state.globals.find("nrecoff");
    const auto eq_on = state.globals.find("leqon");
    const auto seek_on = state.globals.find("lseekon");
    const auto eof_on = state.globals.find("leofon");
    const auto eq_session2 = state.globals.find("leqsession2");
    const auto eq_back = state.globals.find("leqback");

    expect(eq_off != state.globals.end(), "SET EXACT OFF comparison result should be captured");
    expect(eq_off_reverse != state.globals.end(), "SET EXACT OFF reverse comparison result should be captured");
    expect(seek_off != state.globals.end(), "SET EXACT OFF seek result should be captured");
    expect(rec_off != state.globals.end(), "SET EXACT OFF RECNO() should be captured");
    expect(eq_on != state.globals.end(), "SET EXACT ON comparison result should be captured");
    expect(seek_on != state.globals.end(), "SET EXACT ON seek result should be captured");
    expect(eof_on != state.globals.end(), "SET EXACT ON EOF() should be captured");
    expect(eq_session2 != state.globals.end(), "session 2 comparison result should be captured");
    expect(eq_back != state.globals.end(), "session 1 restored comparison result should be captured");

    if (eq_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_off->second) == "true", "SET EXACT OFF should allow right-side prefix string comparison");
    }
    if (eq_off_reverse != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_off_reverse->second) == "false",
               "SET EXACT OFF should preserve documented right-operand-length comparison");
    }
    if (seek_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_off->second) == "true", "SET EXACT OFF should allow prefix seeks");
    }
    if (rec_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_off->second) == "2", "SET EXACT OFF seek should land on the matching prefix row");
    }
    if (eq_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_on->second) == "false", "SET EXACT ON should require full string equality");
    }
    if (seek_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_on->second) == "false", "SET EXACT ON should reject prefix seeks");
    }
    if (eof_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_on->second) == "true", "SET EXACT ON failed seek should leave the cursor at EOF");
    }
    if (eq_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_session2->second) == "true", "SET EXACT should be scoped to the current data session");
    }
    if (eq_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_back->second) == "false", "restoring the original data session should restore its SET EXACT state");
    }

    const auto check_exact = [&](const std::string& name, const std::string& expected, const std::string& message) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), name + " SET EXACT ON result should be captured");
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected, message);
        }
    };
    check_exact("lexactleadingdiff", "false", "#3963: SET EXACT ON must preserve significant leading spaces");
    check_exact("lexactleadingsame", "true", "#3963: equal leading spaces should still compare equal");
    check_exact("lexacttrailingspaces", "true", "#3963: SET EXACT ON should disregard trailing spaces");
    check_exact("lexactallspaces", "true", "#3963: all-space and empty strings should compare equal after right padding");
    check_exact("lexactdifferentlength", "false", "#3963: non-space length differences should remain significant");
    check_exact("lexacttrailingtab", "false", "#3963: SET EXACT ON should not discard trailing tabs");
    check_exact("lexacttrailingnul", "false", "#3963: SET EXACT ON should not discard trailing NUL bytes");

    fs::remove_all(temp_root, ignored);
}

void test_use_again_and_alias_collision_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_again";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_again.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS PeopleAgain AGAIN IN 0\n"
        "nAreaAgain = SELECT()\n"
        "cAliasAgain = ALIAS()\n"
        "USE '" + table_path.string() + "' ALIAS PeopleThird IN 0\n"
        "xAfterError = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "duplicate table opens without AGAIN should pause with an error");
    expect(state.location.line == 5U, "duplicate table opens without AGAIN should highlight the failing USE statement");
    expect(
        state.message.find("USE AGAIN is required") != std::string::npos,
        "duplicate table opens without AGAIN should report a USE AGAIN message");

    const auto area_again = state.globals.find("nareaagain");
    const auto alias_again = state.globals.find("caliasagain");
    expect(area_again != state.globals.end(), "USE AGAIN should let execution reach the second-area checks");
    expect(alias_again != state.globals.end(), "USE AGAIN should let execution expose the second alias");
    if (area_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_again->second) == "2", "USE AGAIN IN 0 should allocate a second work area");
    }
    if (alias_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_again->second) == "PeopleAgain", "USE AGAIN should keep the requested second alias");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE AGAIN error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after continuing");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "7", "post-error statements should still update globals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_again_without_in_allocates_new_area_and_preserves_alias_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_again_without_in";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_again_without_in.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS P1 IN 0\n"
        "nAreaP1 = SELECT()\n"
        "USE '" + table_path.string() + "' AGAIN ALIAS P2\n"
        "nAreaP2 = SELECT()\n"
        "nP1Area = SELECT('P1')\n"
        "nP2Area = SELECT('P2')\n"
        "SELECT P1\n"
        "nSelectedAfterSelectP1 = SELECT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "USE AGAIN without IN should complete and preserve both aliases");

    const auto area_p1 = state.globals.find("nareap1");
    const auto area_p2 = state.globals.find("nareap2");
    const auto p1_area = state.globals.find("np1area");
    const auto p2_area = state.globals.find("np2area");
    const auto selected_after_select_p1 = state.globals.find("nselectedafterselectp1");

    expect(area_p1 != state.globals.end(), "initial work area should be captured");
    expect(area_p2 != state.globals.end(), "work area after USE AGAIN should be captured");
    expect(p1_area != state.globals.end(), "SELECT('P1') lookup should be captured");
    expect(p2_area != state.globals.end(), "SELECT('P2') lookup should be captured");
    expect(selected_after_select_p1 != state.globals.end(), "SELECT P1 result should be captured");

    if (area_p1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_p1->second) == "1", "initial USE IN 0 should allocate area 1");
    }
    if (area_p2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_p2->second) == "2", "USE AGAIN without IN should allocate a new work area");
    }
    if (p1_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(p1_area->second) == "1", "P1 alias should remain bound to area 1");
    }
    if (p2_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(p2_area->second) == "2", "P2 alias should bind to area 2");
    }
    if (selected_after_select_p1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_select_p1->second) == "1", "SELECT P1 should successfully target the original alias");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_selected_alias_replacement_clears_old_alias_and_order_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_selected_alias_replacement";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path people_idx_path = temp_root / "people.idx";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "ROME"});
    write_synthetic_idx(people_idx_path, "UPPER(NAME)");

    const fs::path main_path = temp_root / "selected_alias_replacement.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "GO BOTTOM\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN People\n"
        "nOldAliasArea = SELECT('People')\n"
        "nNewAliasArea = SELECT('Cities')\n"
        "cAliasAfter = ALIAS()\n"
        "cOrderAfter = ORDER()\n"
        "nRecAfter = RECNO()\n"
        "cTopAfter = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "selected alias replacement script should complete");

    const auto old_alias_area = state.globals.find("noldaliasarea");
    const auto new_alias_area = state.globals.find("nnewaliasarea");
    const auto alias_after = state.globals.find("caliasafter");
    const auto order_after = state.globals.find("corderafter");
    const auto rec_after = state.globals.find("nrecafter");
    const auto top_after = state.globals.find("ctopafter");

    expect(old_alias_area != state.globals.end(), "SELECT('People') after selected alias replacement should be captured");
    expect(new_alias_area != state.globals.end(), "SELECT('Cities') after selected alias replacement should be captured");
    expect(alias_after != state.globals.end(), "ALIAS() after selected alias replacement should be captured");
    expect(order_after != state.globals.end(), "ORDER() after selected alias replacement should be captured");
    expect(rec_after != state.globals.end(), "RECNO() after selected alias replacement should be captured");
    expect(top_after != state.globals.end(), "field access after selected alias replacement should be captured");

    if (old_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(old_alias_area->second) == "0", "selected alias replacement should clear the old alias lookup");
    }
    if (new_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(new_alias_area->second) == "1", "selected alias replacement should reuse the selected work area in place");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Cities", "selected alias replacement should expose the new alias immediately");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "selected alias replacement should clear the old active order state when the replacement has no orders");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "1", "selected alias replacement should reset the cursor position for the new table");
    }
    if (top_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after->second) == "OSLO", "selected alias replacement should expose the new table's first record");
    }

    fs::remove_all(temp_root, ignored);
}


void test_select_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_select_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "select_missing.prg";
    write_text(
        main_path,
        "SELECT MissingAlias\n"
        "xAfterError = 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "SELECT MissingAlias should pause with an error");
    expect(state.location.line == 1U, "SELECT MissingAlias should highlight the failing line");
    expect(
        state.message.find("SELECT target work area not found") != std::string::npos,
        "SELECT MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a SELECT error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after SELECT errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "9", "post-error statements should still update globals after SELECT errors");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_in_missing_alias.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN MissingAlias\n"
        "xAfterError = 11\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "USE ... IN MissingAlias should pause with an error");
    expect(state.location.line == 1U, "USE ... IN MissingAlias should highlight the failing line");
    expect(
        state.message.find("USE target work area not found") != std::string::npos,
        "USE ... IN MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE ... IN alias error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after USE ... IN alias errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "11", "post-error statements should still update globals after USE ... IN alias errors");
    }
    expect(state.work_area.aliases.empty(), "USE ... IN MissingAlias should not open a fallback work area");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_fault_containment() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "faults.prg";
    write_text(
        main_path,
        "x = 'abc' - 1\n"
        "y = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "mixed-type code should pause with an error instead of killing the host");
    expect(state.location.line == 1U, "runtime faults should highlight the faulting line");
    expect(state.message.find("Operator/operand type mismatch.") != std::string::npos,
        "mixed string-minus operands should report VFP Error 107 prose");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.error"; }),
        "runtime faults should emit a runtime.error event");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a trapped runtime error should keep the session alive");
    const auto y = state.globals.find("y");
    expect(y != state.globals.end(), "post-fault statements should still be able to run");
    if (y != state.globals.end()) {
        expect(copperfin::runtime::format_value(y->second) == "7", "post-fault statements should update globals");
    }

    fs::remove_all(temp_root, ignored);
}


}
