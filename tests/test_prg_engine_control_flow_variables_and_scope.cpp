// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_private_declaration_masks_caller_variable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_private_mask";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "private_mask.prg";
    write_text(
        main_path,
        "PUBLIC sub_x\n"
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "sub_x = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRIVATE mask script should complete");

    const auto sub_x = state.globals.find("sub_x");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x != state.globals.end(), "sub_x should be in globals");
    expect(caller_x != state.globals.end(), "caller_x should be in globals");

    if (sub_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x->second) == "99", "sub should see its own PRIVATE x = 99");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42", "caller x should be restored to 42 after sub returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_private_all_hides_matching_caller_variables_and_arrays() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_private_all";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "private_all.prg";
    write_text(
        main_path,
        "cLikeOne = 'outer-like-one'\n"
        "cLikeTwo = 'outer-like-two'\n"
        "cExceptKeep = 'outer-keep'\n"
        "cExceptHide = 'outer-hide'\n"
        "DIMENSION aAll[1]\n"
        "aAll[1] = 'outer-array'\n"
        "DO private_all_scope\n"
        "after_all_one = cLikeOne\n"
        "after_all_two = cLikeTwo\n"
        "after_all_keep = cExceptKeep\n"
        "after_all_hide = cExceptHide\n"
        "after_all_array = aAll[1]\n"
        "DO private_like_scope\n"
        "after_like_one = cLikeOne\n"
        "after_like_keep = cExceptKeep\n"
        "DO private_except_scope\n"
        "after_except_keep = cExceptKeep\n"
        "after_except_hide = cExceptHide\n"
        "RETURN\n"
        "PROCEDURE private_all_scope\n"
        "PRIVATE ALL\n"
        "cLikeOne = 'inner-one'\n"
        "cLikeTwo = 'inner-two'\n"
        "cExceptKeep = 'inner-keep'\n"
        "cExceptHide = 'inner-hide'\n"
        "aAll[1] = 'inner-array'\n"
        "RETURN\n"
        "PROCEDURE private_like_scope\n"
        "PRIVATE ALL LIKE cLike*\n"
        "cLikeOne = 'inner-like'\n"
        "cExceptKeep = 'changed-through'\n"
        "RETURN\n"
        "PROCEDURE private_except_scope\n"
        "PRIVATE ALL EXCEPT cExceptKeep\n"
        "cExceptKeep = 'changed-through-except'\n"
        "cExceptHide = 'inner-except-hide'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRIVATE ALL script should complete");

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), name + " should be captured");
        if (value != state.globals.end())
        {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(value->second) + "'");
        }
    };

    check("after_all_one", "outer-like-one");
    check("after_all_two", "outer-like-two");
    check("after_all_keep", "outer-keep");
    check("after_all_hide", "outer-hide");
    check("after_all_array", "outer-array");
    check("after_like_one", "outer-like-one");
    check("after_like_keep", "changed-through");
    check("after_except_keep", "changed-through-except");
    check("after_except_hide", "outer-hide");

    fs::remove_all(temp_root, ignored);
}

void test_scoped_array_declarations_follow_vfp_lifetime_rules() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scoped_array_declarations";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "scoped_arrays.prg";
    write_text(
        main_path,
        "PUBLIC observer_public, observer_private, observer_local_type, local_initial, local_release\n"
        "PUBLIC ARRAY aPublic[1]\n"
        "aPublic[1] = 'public'\n"
        "DIMENSION aPrivate[1]\n"
        "aPrivate[1] = 'outer-private'\n"
        "DO scoped_arrays\n"
        "after_public = aPublic[1]\n"
        "after_private = aPrivate[1]\n"
        "after_local_type = TYPE('aLocal')\n"
        "RETURN\n"
        "PROCEDURE scoped_arrays\n"
        "LOCAL ARRAY aLocal[1]\n"
        "local_initial = aLocal[1]\n"
        "aLocal[1] = 'local'\n"
        "PRIVATE ARRAY aPrivate[1]\n"
        "aPrivate[1] = 'private'\n"
        "DO scoped_array_observer\n"
        "local_release = TYPE('aReleased')\n"
        "LOCAL ARRAY aReleased[1]\n"
        "RELEASE aReleased\n"
        "local_release = TYPE('aReleased')\n"
        "RETURN\n"
        "PROCEDURE scoped_array_observer\n"
        "observer_public = aPublic[1]\n"
        "observer_private = aPrivate[1]\n"
        "observer_local_type = TYPE('aLocal')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "scoped-array declaration script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), name + " should be captured");
        if (value != state.globals.end())
        {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   name + " should be " + expected);
        }
    };
    check("after_public", "public");
    check("observer_public", "public");
    check("observer_private", "private");
    check("after_private", "outer-private");
    check("local_initial", "false");
    check("observer_local_type", "U");
    check("after_local_type", "U");
    check("local_release", "U");

    fs::remove_all(temp_root, ignored);
}

void test_double_parentheses_force_array_value_copy() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_value_argument";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "array_value_argument.prg";
    write_text(
        main_path,
        "DIMENSION aData[3]\n"
        "aData[1] = 'source-one'\n"
        "aData[2] = 'source-two'\n"
        "aData[3] = 'source-three'\n"
        "forced_result = read_and_change((aData))\n"
        "source_after = aData[1]\n"
        "RETURN\n"
        "FUNCTION read_and_change\n"
        "LPARAMETERS aItems\n"
        "aItems[1] = 'copy-changed'\n"
        "RETURN aItems[1] + '|' + aItems[2] + '|' + TRANSFORM(ALEN(aItems))\n"
        "ENDFUNC\n");

    const auto state = copperfin::runtime::PrgRuntimeSession::create(
                           make_runtime_session_options(main_path.string(), temp_root.string(), false))
                           .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "double-parentheses array argument script should complete: " + state.message);

    const auto forced_result = state.globals.find("forced_result");
    const auto source_after = state.globals.find("source_after");
    expect(forced_result != state.globals.end(), "forced array-copy result should be captured");
    expect(source_after != state.globals.end(), "source array value after forced copy should be captured");
    if (forced_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(forced_result->second) == "copy-changed|source-two|3",
               "double parentheses should pass an independent full array copy");
    }
    if (source_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(source_after->second) == "source-one",
               "mutating a forced array copy should not mutate the caller array");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_and_private_array_by_reference_bindings() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scoped_array_by_reference";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "scoped_array_by_reference.prg";
    write_text(
        main_path,
        "PUBLIC local_after_one, local_after_two, private_after_one, private_after_two\n"
        "DO use_local_array\n"
        "DO use_private_array\n"
        "RETURN\n"
        "PROCEDURE use_local_array\n"
        "LOCAL ARRAY aLocal[2]\n"
        "aLocal[1] = 'local-one'\n"
        "aLocal[2] = 'local-two'\n"
        "DO mutate_array WITH aLocal\n"
        "local_after_one = aLocal[1]\n"
        "local_after_two = aLocal[2]\n"
        "RETURN\n"
        "PROCEDURE use_private_array\n"
        "PRIVATE ARRAY aPrivate[2]\n"
        "aPrivate[1] = 'private-one'\n"
        "aPrivate[2] = 'private-two'\n"
        "DO mutate_array WITH aPrivate\n"
        "private_after_one = aPrivate[1]\n"
        "private_after_two = aPrivate[2]\n"
        "RETURN\n"
        "PROCEDURE mutate_array\n"
        "LPARAMETERS aItems\n"
        "aItems[1] = 'mutated-one'\n"
        "DO forward_array WITH aItems\n"
        "RETURN\n"
        "PROCEDURE forward_array\n"
        "LPARAMETERS aForward\n"
        "aForward[2] = 'mutated-two'\n"
        "RETURN\n");

    const auto state = copperfin::runtime::PrgRuntimeSession::create(
                           make_runtime_session_options(main_path.string(), temp_root.string(), false))
                           .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "scoped array by-reference script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), name + " should be captured");
        if (value != state.globals.end())
        {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   name + " should reflect nested by-reference array mutation");
        }
    };

    check("local_after_one", "mutated-one");
    check("local_after_two", "mutated-two");
    check("private_after_one", "mutated-one");
    check("private_after_two", "mutated-two");

    fs::remove_all(temp_root, ignored);
}

void test_private_variable_visible_to_called_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_private_visible";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "private_visible.prg";
    write_text(
        main_path,
        "PUBLIC inner_saw\n"
        "DO caller\n"
        "RETURN\n"
        "PROCEDURE caller\n"
        "PRIVATE shared_val\n"
        "shared_val = 77\n"
        "DO inner\n"
        "RETURN\n"
        "PROCEDURE inner\n"
        "inner_saw = shared_val\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRIVATE visibility script should complete");

    const auto inner_saw = state.globals.find("inner_saw");
    expect(inner_saw != state.globals.end(), "inner_saw should be in globals");
    if (inner_saw != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_saw->second) == "77", "PRIVATE variable should be visible to called routines");
    }

    fs::remove_all(temp_root, ignored);
}

void test_release_private_restores_saved_binding_immediately() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_release_private_restore";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "release_private_restore.prg";
    write_text(
        main_path,
        "PUBLIC sub_x_after_release\n"
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "RELEASE x\n"
        "sub_x_after_release = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE of PRIVATE binding script should complete");

    const auto sub_x_after_release = state.globals.find("sub_x_after_release");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x_after_release != state.globals.end(), "sub_x_after_release should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");

    if (sub_x_after_release != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release->second) == "42",
               "RELEASE x inside a PRIVATE scope should immediately restore the saved outer binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "the caller binding should remain restored after the PRIVATE scope returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_release_local_restores_visible_outer_global() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_release_local_restore";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "release_local_restore.prg";
    write_text(
        main_path,
        "PUBLIC sub_x_after_release\n"
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 99\n"
        "RELEASE x\n"
        "sub_x_after_release = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE of LOCAL binding script should complete");

    const auto sub_x_after_release = state.globals.find("sub_x_after_release");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x_after_release != state.globals.end(), "sub_x_after_release should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");

    if (sub_x_after_release != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release->second) == "42",
               "RELEASE x inside a LOCAL scope should reveal the visible outer global binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "the outer global binding should remain intact after the LOCAL scope returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_macro_assignment_target_updates_private_binding_and_release_restores_outer_value() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_macro_assign_private_release";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "macro_assign_private_release.prg";
    write_text(
        main_path,
        "PUBLIC sub_x_after_macro_assign, sub_x_after_release\n"
        "x = 42\n"
        "cTarget = 'x'\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "&cTarget = 99\n"
        "sub_x_after_macro_assign = x\n"
        "RELEASE x\n"
        "sub_x_after_release = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro-expanded PRIVATE assignment target script should complete");

    const auto sub_x_after_macro_assign = state.globals.find("sub_x_after_macro_assign");
    const auto sub_x_after_release = state.globals.find("sub_x_after_release");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x_after_macro_assign != state.globals.end(), "sub_x_after_macro_assign should be captured");
    expect(sub_x_after_release != state.globals.end(), "sub_x_after_release should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");

    if (sub_x_after_macro_assign != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_macro_assign->second) == "99",
               "&cTarget = value should update the visible PRIVATE binding rather than resolve to the binding's current value");
    }
    if (sub_x_after_release != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release->second) == "42",
               "RELEASE after a macro-target PRIVATE assignment should restore the saved outer binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "the caller binding should remain intact after the PRIVATE scope returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_macro_assignment_target_preserves_public_binding_across_release_all() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_macro_assign_public_release_all";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "macro_assign_public_release_all.prg";
    write_text(
        main_path,
        "PUBLIC shared\n"
        "shared = 7\n"
        "cTarget = 'shared'\n"
        "&cTarget = 9\n"
        "RELEASE ALL\n"
        "nSharedAfterReleaseAll = shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro-expanded PUBLIC assignment target script should complete");

    const auto shared_after_release_all = state.globals.find("nsharedafterreleaseall");
    expect(shared_after_release_all != state.globals.end(), "nSharedAfterReleaseAll should be captured");

    if (shared_after_release_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(shared_after_release_all->second) == "9",
               "macro-expanded assignment should preserve PUBLIC binding identity so RELEASE ALL keeps the updated value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_store_command_assigns_multiple_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_store";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "store_test.prg";
    write_text(
        main_path,
        "STORE 7 TO a, b, c\n"
        "STORE 'hello' TO s1, s2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STORE script should complete");

    const auto a = state.globals.find("a");
    const auto b = state.globals.find("b");
    const auto c = state.globals.find("c");
    const auto s1 = state.globals.find("s1");
    const auto s2 = state.globals.find("s2");

    expect(a != state.globals.end(), "STORE should assign a");
    expect(b != state.globals.end(), "STORE should assign b");
    expect(c != state.globals.end(), "STORE should assign c");
    expect(s1 != state.globals.end(), "STORE should assign s1");
    expect(s2 != state.globals.end(), "STORE should assign s2");

    if (a != state.globals.end()) {
        expect(copperfin::runtime::format_value(a->second) == "7", "a should equal 7");
    }
    if (b != state.globals.end()) {
        expect(copperfin::runtime::format_value(b->second) == "7", "b should equal 7");
    }
    if (c != state.globals.end()) {
        expect(copperfin::runtime::format_value(c->second) == "7", "c should equal 7");
    }
    if (s1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(s1->second) == "hello", "s1 should equal 'hello'");
    }
    if (s2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2->second) == "hello", "s2 should equal 'hello'");
    }

    fs::remove_all(temp_root, ignored);
}

void test_release_vars_erases_named_globals() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_vars";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "x = 10\ny = 20\nRELEASE x\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE should not crash");
    expect(state.globals.find("x") == state.globals.end(), "x should be released");
    expect(state.globals.find("y") != state.globals.end(), "y should still exist");
    fs::remove_all(tmp, ign);
}

void test_release_all_clears_all_globals() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "a = 1\nb = 2\nRELEASE ALL\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL should complete");
    expect(state.globals.find("a") == state.globals.end(), "a should be released by RELEASE ALL");
    expect(state.globals.find("b") == state.globals.end(), "b should be released by RELEASE ALL");
    fs::remove_all(tmp, ign);
}

void test_release_all_clears_current_frame_locals_without_global_leak() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all_locals";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "PUBLIC after_release_type, after_reassign\n"
        "DO subproc\n"
        "outer_type = TYPE('x')\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 5\n"
        "RELEASE ALL\n"
        "PUBLIC after_release_type, after_reassign\n"
        "after_release_type = TYPE('x')\n"
        "x = 7\n"
        "after_reassign = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL should clear current-frame locals");
    const auto after_release_type = state.globals.find("after_release_type");
    const auto after_reassign = state.globals.find("after_reassign");
    const auto outer_type = state.globals.find("outer_type");
    expect(after_release_type != state.globals.end(), "released local TYPE() should be captured");
    expect(after_reassign != state.globals.end(), "reassigned local value should be captured");
    expect(outer_type != state.globals.end(), "post-return local TYPE() should be captured");
    if (after_release_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_type->second) == "U",
               "RELEASE ALL should clear the current frame's local variable binding");
    }
    if (after_reassign != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_reassign->second) == "7",
               "reassigning after RELEASE ALL should still work inside the local scope");
    }
    if (outer_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_type->second) == "U",
               "reassigning a released LOCAL should not leak a new global after the routine returns");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_local_shadow_preserves_outer_global() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all_local_shadow";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "PUBLIC sub_x_after_release_all\n"
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 99\n"
        "RELEASE ALL\n"
        "sub_x_after_release_all = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL with LOCAL shadowing should complete");

    const auto sub_x_after_release_all = state.globals.find("sub_x_after_release_all");
    const auto caller_x = state.globals.find("caller_x");
    expect(sub_x_after_release_all != state.globals.end(), "sub_x_after_release_all should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");
    if (sub_x_after_release_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release_all->second) == "42",
               "RELEASE ALL should clear the LOCAL shadow without erasing the outer global binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "RELEASE ALL should preserve the outer global after the LOCAL frame returns");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_private_shadow_restores_outer_global() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all_private_shadow";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "PUBLIC sub_x_after_release_all\n"
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "RELEASE ALL\n"
        "sub_x_after_release_all = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL with PRIVATE shadowing should complete");

    const auto sub_x_after_release_all = state.globals.find("sub_x_after_release_all");
    const auto caller_x = state.globals.find("caller_x");
    expect(sub_x_after_release_all != state.globals.end(), "sub_x_after_release_all should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");
    if (sub_x_after_release_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release_all->second) == "42",
               "RELEASE ALL should clear the PRIVATE shadow without erasing the outer global binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "RELEASE ALL should preserve the outer global after the PRIVATE frame returns");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_like_pattern() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_like";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "tmp_a = 1\ntmp_b = 2\nkeep_me = 3\nRELEASE ALL LIKE tmp_*\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL LIKE should complete");
    expect(state.globals.find("tmp_a") == state.globals.end(), "tmp_a should be released");
    expect(state.globals.find("tmp_b") == state.globals.end(), "tmp_b should be released");
    expect(state.globals.find("keep_me") != state.globals.end(), "keep_me should survive LIKE tmp_*");
    fs::remove_all(tmp, ign);
}

void test_release_all_like_pattern_reaches_arrays() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_like_arrays";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "DIMENSION tmp_arr[1], keep_arr[1]\n"
        "tmp_arr[1] = 'gone'\n"
        "keep_arr[1] = 'stay'\n"
        "RELEASE ALL LIKE tmp_*\n"
        "tmp_type = TYPE('tmp_arr')\n"
        "keep_val = keep_arr[1]\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL LIKE should reach arrays");
    const auto tmp_type = state.globals.find("tmp_type");
    const auto keep_val = state.globals.find("keep_val");
    expect(tmp_type != state.globals.end(), "released array TYPE() should be captured");
    expect(keep_val != state.globals.end(), "surviving array value should be captured");
    if (tmp_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(tmp_type->second) == "U", "RELEASE ALL LIKE tmp_* should release matching arrays");
    }
    if (keep_val != state.globals.end()) {
        expect(copperfin::runtime::format_value(keep_val->second) == "stay", "RELEASE ALL LIKE tmp_* should preserve non-matching arrays");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_except_pattern() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_except";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "keep_x = 1\ngone_y = 2\nRELEASE ALL EXCEPT keep_*\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL EXCEPT should complete");
    expect(state.globals.find("keep_x") != state.globals.end(), "keep_x should survive EXCEPT keep_*");
    expect(state.globals.find("gone_y") == state.globals.end(), "gone_y should be released");
    fs::remove_all(tmp, ign);
}

void test_release_all_except_pattern_reaches_arrays() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_except_arrays";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "DIMENSION keep_arr[1], gone_arr[1]\n"
        "keep_arr[1] = 'stay'\n"
        "gone_arr[1] = 'gone'\n"
        "RELEASE ALL EXCEPT keep_*\n"
        "keep_val = keep_arr[1]\n"
        "gone_type = TYPE('gone_arr')\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL EXCEPT should reach arrays");
    const auto keep_val = state.globals.find("keep_val");
    const auto gone_type = state.globals.find("gone_type");
    expect(keep_val != state.globals.end(), "surviving EXCEPT array value should be captured");
    expect(gone_type != state.globals.end(), "released EXCEPT array TYPE() should be captured");
    if (keep_val != state.globals.end()) {
        expect(copperfin::runtime::format_value(keep_val->second) == "stay", "RELEASE ALL EXCEPT keep_* should preserve matching arrays");
    }
    if (gone_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(gone_type->second) == "U", "RELEASE ALL EXCEPT keep_* should release non-matching arrays");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_preserves_public_bindings() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_public";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "PUBLIC pub_keep, pub_arr\n"
        "pub_keep = 7\n"
        "DIMENSION pub_arr[1]\n"
        "pub_arr[1] = 'A'\n"
        "drop_me = 1\n"
        "RELEASE ALL EXCEPT keep_*\n"
        "pub_after = pub_keep\n"
        "arr_after = pub_arr[1]\n"
        "drop_type = TYPE('drop_me')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL should preserve PUBLIC bindings");

    const auto pub_after = state.globals.find("pub_after");
    const auto arr_after = state.globals.find("arr_after");
    const auto drop_type = state.globals.find("drop_type");
    expect(pub_after != state.globals.end(), "PUBLIC scalar should be readable after RELEASE ALL EXCEPT");
    expect(arr_after != state.globals.end(), "PUBLIC array should be readable after RELEASE ALL EXCEPT");
    expect(drop_type != state.globals.end(), "released non-public variable type should be captured");
    if (pub_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_after->second) == "7",
               "RELEASE ALL EXCEPT should not erase a PUBLIC scalar that fails the EXCEPT pattern");
    }
    if (arr_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(arr_after->second) == "A",
               "RELEASE ALL EXCEPT should not erase a PUBLIC array that fails the EXCEPT pattern");
    }
    if (drop_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(drop_type->second) == "U",
               "RELEASE ALL EXCEPT should still erase matching non-public variables");
    }

    fs::remove_all(tmp, ign);
}

void test_clear_memory_erases_all_globals() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_clear_memory";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "p = 42\nq = 99\nCLEAR MEMORY\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLEAR MEMORY should complete");
    expect(state.globals.find("p") == state.globals.end(), "p should be cleared");
    expect(state.globals.find("q") == state.globals.end(), "q should be cleared");
    fs::remove_all(tmp, ign);
}

void test_clear_memory_prevents_private_bindings_from_restoring() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_clear_memory_private_restore";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "PUBLIC sub_type\n"
        "x = 42\n"
        "DO subproc\n"
        "caller_type = TYPE('x')\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "CLEAR MEMORY\n"
        "PUBLIC sub_type\n"
        "sub_type = TYPE('x')\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLEAR MEMORY with PRIVATE shadowing should complete");
    const auto sub_type = state.globals.find("sub_type");
    const auto caller_type = state.globals.find("caller_type");
    expect(sub_type != state.globals.end(), "sub_type should be captured after CLEAR MEMORY");
    expect(caller_type != state.globals.end(), "caller_type should be captured after PRIVATE frame returns");
    if (sub_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_type->second) == "U",
               "CLEAR MEMORY should remove the PRIVATE binding inside the current frame");
    }
    if (caller_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_type->second) == "U",
               "CLEAR MEMORY should prevent saved outer PRIVATE bindings from being restored later");
    }
    fs::remove_all(tmp, ign);
}

void test_clear_memory_clears_current_frame_locals_without_global_leak() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_clear_memory_locals";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "PUBLIC after_clear_type, after_reassign\n"
        "DO subproc\n"
        "outer_type = TYPE('x')\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 5\n"
        "CLEAR MEMORY\n"
        "PUBLIC after_clear_type, after_reassign\n"
        "after_clear_type = TYPE('x')\n"
        "x = 7\n"
        "after_reassign = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLEAR MEMORY should clear current-frame locals");
    const auto after_clear_type = state.globals.find("after_clear_type");
    const auto after_reassign = state.globals.find("after_reassign");
    const auto outer_type = state.globals.find("outer_type");
    expect(after_clear_type != state.globals.end(), "cleared local TYPE() should be captured");
    expect(after_reassign != state.globals.end(), "reassigned local after CLEAR MEMORY should be captured");
    expect(outer_type != state.globals.end(), "post-return local TYPE() after CLEAR MEMORY should be captured");
    if (after_clear_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_clear_type->second) == "U",
               "CLEAR MEMORY should clear the current frame's local variable binding");
    }
    if (after_reassign != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_reassign->second) == "7",
               "reassigning after CLEAR MEMORY should still work inside the local scope");
    }
    if (outer_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_type->second) == "U",
               "reassigning a cleared LOCAL should not leak a new global after the routine returns");
    }
    fs::remove_all(tmp, ign);
}

}  // namespace cf_test_prg_engine_control_flow
