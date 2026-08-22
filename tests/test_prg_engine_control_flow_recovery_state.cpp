// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_on_error_resume_restores_fault_session_and_cursor_state() {
    // #150: RESUME should restore the captured fault-side data session/work area
    // even when the handler changes its own session and cursor selection.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_resume_fault_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path items_path = temp_root / "items.dbf";
    const fs::path alt_path = temp_root / "alt.dbf";
    write_people_dbf(items_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_people_dbf(alt_path, {{"DELTA", 40}, {"ECHO", 50}});

    const fs::path main_path = temp_root / "resume_fault_state.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "USE '" + items_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "rec_before = RECNO()\n"
        "fault = LOG(-1)\n"
        "alias_after = ALIAS()\n"
        "rec_after = RECNO()\n"
        "name_after = Items.NAME\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "SET DATASESSION TO 2\n"
        "USE '" + alt_path.string() + "' ALIAS Alt IN 0\n"
        "SELECT Alt\n"
        "RESUME\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#150: ON ERROR RESUME script should complete");

    const auto rec_before = state.globals.find("rec_before");
    const auto alias_after = state.globals.find("alias_after");
    const auto rec_after = state.globals.find("rec_after");
    const auto name_after = state.globals.find("name_after");

    expect(rec_before != state.globals.end(), "#150: rec_before should be captured before the fault");
    expect(alias_after != state.globals.end(), "#150: post-fault ALIAS() should be captured");
    expect(rec_after != state.globals.end(), "#150: post-fault RECNO() should be captured");
    expect(name_after != state.globals.end(), "#150: post-fault field read should succeed");

    if (rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_before->second) == "2",
               "#150: fault-side cursor should be positioned on record 2 before fault");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Items",
               "#150: RESUME should restore the fault-side selected alias");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "2",
               "#150: RESUME should preserve fault-side cursor record position");
    }
    if (name_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_after->second) == "BRAVO",
               "#150: post-fault field reads should remain on the original cursor row");
    }

    fs::remove_all(temp_root, ignored);
}

void test_fault_continue_cycle_preserves_open_cursor_and_record_position() {
    // #151: after each debug-continue across a runtime fault, cursor state and
    // record position must remain stable so the developer can keep inspecting data.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fault_cursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "items.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "fault_cursor.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "recno_before = RECNO()\n"
        "x = LOG(-1)\n"
        "recno_after_first = RECNO()\n"
        "y = ACOS(2)\n"
        "recno_after_second = RECNO()\n"
        "cur_name = Items.NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    // First fault: LOG(-1)
    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: first fault should pause with an error reason");
    expect(state.location.line == 5U,
           "#151: first fault should highlight line 5 (LOG(-1))");

    // Second fault: ACOS(2)
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: second fault should pause with an error reason");
    expect(state.location.line == 7U,
           "#151: second fault should highlight line 7 (ACOS(2))");

    // Final continue — should complete
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#151: session should complete after continuing past both faults");

    // Cursor should have been at record 2 (after SKIP from record 1)
    const auto recno_before = state.globals.find("recno_before");
    const auto recno_after_first = state.globals.find("recno_after_first");
    const auto recno_after_second = state.globals.find("recno_after_second");
    const auto cur_name = state.globals.find("cur_name");

    expect(recno_before != state.globals.end(), "#151: recno_before should be set");
    expect(recno_after_first != state.globals.end(), "#151: recno_after_first should be set after first fault continue");
    expect(recno_after_second != state.globals.end(), "#151: recno_after_second should be set after second fault continue");
    expect(cur_name != state.globals.end(), "#151: cursor field read should succeed after fault cycle");

    if (recno_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_before->second) == "2",
               "#151: SKIP from record 1 should position at record 2");
    }
    if (recno_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_first->second) == "2",
               "#151: cursor record position should survive the first fault continue");
    }
    if (recno_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_second->second) == "2",
               "#151: cursor record position should survive the second fault continue");
    }
    if (cur_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(cur_name->second) == "BRAVO",
               "#151: cursor field access after fault cycle should return the expected record value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_fault_continue_cycle_preserves_selected_alias_across_data_session_scope() {
    // #151: repeated CONTINUE over multiple faults should preserve selected-alias
    // stability even when faults occur inside a non-default data session.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fault_cursor_ds";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "items_ds.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "fault_cursor_ds.prg";
    write_text(
        main_path,
        "SET DATASESSION TO 2\n"
        "USE '" + table_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "alias_before = ALIAS()\n"
        "recno_before = RECNO()\n"
        "x = LOG(-1)\n"
        "alias_after_first = ALIAS()\n"
        "recno_after_first = RECNO()\n"
        "y = ACOS(2)\n"
        "alias_after_second = ALIAS()\n"
        "recno_after_second = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: first non-default-session fault should pause with error");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: second non-default-session fault should pause with error");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#151: session should complete after continuing past both non-default-session faults");

    const auto alias_before = state.globals.find("alias_before");
    const auto alias_after_first = state.globals.find("alias_after_first");
    const auto alias_after_second = state.globals.find("alias_after_second");
    const auto recno_before = state.globals.find("recno_before");
    const auto recno_after_first = state.globals.find("recno_after_first");
    const auto recno_after_second = state.globals.find("recno_after_second");

    expect(alias_before != state.globals.end(), "#151: alias_before should be captured");
    expect(alias_after_first != state.globals.end(), "#151: alias_after_first should be captured");
    expect(alias_after_second != state.globals.end(), "#151: alias_after_second should be captured");
    expect(recno_before != state.globals.end(), "#151: recno_before should be captured");
    expect(recno_after_first != state.globals.end(), "#151: recno_after_first should be captured");
    expect(recno_after_second != state.globals.end(), "#151: recno_after_second should be captured");

    if (alias_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_before->second) == "Items",
               "#151: selected alias should be Items before first fault");
    }
    if (alias_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_first->second) == "Items",
               "#151: selected alias should survive first fault continue");
    }
    if (alias_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_second->second) == "Items",
               "#151: selected alias should survive second fault continue");
    }
    if (recno_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_before->second) == "2",
               "#151: pre-fault record position should be 2");
    }
    if (recno_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_first->second) == "2",
               "#151: record position should survive first fault continue");
    }
    if (recno_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_second->second) == "2",
               "#151: record position should survive second fault continue");
    }

    fs::remove_all(temp_root, ignored);
}

void test_pause_stack_frame_contains_accurate_intermediate_frame_lines() {
    // #152: all frames in the call stack at a fault pause should report
    // the line at which each caller invoked the next routine — not zero or stale.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_frame_lines";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "frame_lines.prg";
    write_text(
        main_path,
        "before_call = 1\n"
        "DO outerproc\n"
        "after_call = 1\n"
        "RETURN\n"
        "PROCEDURE outerproc\n"
        "outer_start = 1\n"
        "DO innerproc\n"
        "outer_end = 1\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE innerproc\n"
        "inner_start = 1\n"
        "fault_val = LOG(-1)\n"
        "inner_end = 1\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: nested fault should pause with error reason");
    expect(state.location.line == 13U,
           "#152: fault location should point at the LOG(-1) line");
    expect(state.call_stack.size() >= 3U,
           "#152: call stack should expose all three frames at fault time");

    if (state.call_stack.size() >= 3U) {
        // Top frame: innerproc, faulting line
        expect(state.call_stack[0].routine_name == "innerproc",
               "#152: top frame routine name should be innerproc");
        expect(state.call_stack[0].line == 13U,
               "#152: top frame line should be the fault line inside innerproc");

        // Middle frame: outerproc, line where DO innerproc was invoked
        expect(state.call_stack[1].routine_name == "outerproc",
               "#152: middle frame routine name should be outerproc");
         // The runtime records the resume PC (statement after the DO), so line 8 = outer_end = 1
         expect(state.call_stack[1].line == 8U,
             "#152: middle frame line should be the resume line after DO innerproc (outer_end = 1)");

        // Bottom frame: main, line where DO outerproc was invoked
        expect(state.call_stack[2].routine_name == "main",
               "#152: bottom frame routine name should be main");
         // The runtime records the resume PC (statement after the DO), so line 3 = after_call = 1
         expect(state.call_stack[2].line == 3U,
             "#152: bottom frame line should be the resume line after DO outerproc (after_call = 1)");
    }

    // Continue past the fault; the session should remain alive
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#152: session should complete after continuing past the nested fault");
    const auto after_call = state.globals.find("after_call");
    expect(after_call != state.globals.end(), "#152: post-call statement should execute after fault continue");

    fs::remove_all(temp_root, ignored);
}

void test_repeated_fault_pauses_refresh_intermediate_stack_frame_lines() {
    // #152: intermediate caller-frame line metadata should refresh across
    // repeated nested fault pauses instead of leaking stale frame line values.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_frame_lines_repeat";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "frame_lines_repeat.prg";
    write_text(
        main_path,
        "DO outerfirst\n"
        "DO outersecond\n"
        "RETURN\n"
        "PROCEDURE outerfirst\n"
        "DO innerfirst\n"
        "RETURN\n"
        "PROCEDURE outersecond\n"
        "DO innersecond\n"
        "RETURN\n"
        "PROCEDURE innerfirst\n"
        "x = LOG(-1)\n"
        "RETURN\n"
        "PROCEDURE innersecond\n"
        "y = ACOS(2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: first nested fault should pause with error");
    expect(state.location.line == 11U,
           "#152: first nested fault should highlight the first inner routine line");
    expect(state.call_stack.size() >= 3U,
           "#152: first nested fault should expose full call stack");
    if (state.call_stack.size() >= 2U) {
        expect(state.call_stack[1].routine_name == "outerfirst",
               "#152: first pause should report outerfirst as intermediate frame");
        expect(state.call_stack[1].line == 6U,
               "#152: first pause intermediate frame line should match the caller resume PC");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: second nested fault should pause with error");
    expect(state.location.line == 14U,
           "#152: second nested fault should highlight the second inner routine line");
    expect(state.call_stack.size() >= 3U,
           "#152: second nested fault should expose full call stack");
    if (state.call_stack.size() >= 2U) {
        expect(state.call_stack[1].routine_name == "outersecond",
               "#152: second pause should refresh intermediate frame routine");
        expect(state.call_stack[1].line == 9U,
               "#152: second pause intermediate frame line should refresh to the second caller resume PC");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#152: session should complete after continuing past both nested faults");

    fs::remove_all(temp_root, ignored);
}
} // namespace cf_test_prg_engine_control_flow

