// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <map>
#include <string>
#include <vector>

namespace copperfin::runtime {

enum class StatementKind {
    assignment,
    expression,
    do_command,
    do_while_statement,
    do_case_statement,
    case_statement,
    otherwise_statement,
    calculate_command,
    count_command,
    sum_command,
    average_command,
    text_command,
    total_command,
    do_form,
    report_form,
    label_form,
    define_menu_command,
    define_popup_command,
    define_bar_command,
    on_bar_activate_popup_command,
    on_selection_bar_command,
    on_selection_bar_action_command,
    on_selection_popup_command,
    deactivate_surface,
    activate_surface,
    release_surface,
    return_statement,
    nodefault_statement,
    if_statement,
    else_statement,
    endif_statement,
    for_statement,
    endfor_statement,
    loop_statement,
    continue_command,
    exit_statement,
    enddo_statement,
    endcase_statement,
    read_events,
    clear_events,
    begin_transaction,
    end_transaction,
    rollback_transaction,
    undo_command,
    seek_command,
    locate_command,
    scan_statement,
    endscan_statement,
    replace_command,
    append_blank_command,
    delete_command,
    recall_command,
    pack_command,
    zap_command,
    unlock_command,
    delete_from_command,
    insert_into_command,
    go_command,
    skip_command,
    browse_command,
    select_command,
    use_command,
    open_database,
    set_order,
    set_command,
    set_procedure,
    set_library,
    set_datasession,
    set_default,
    set_memowidth,
    on_key_command,
    on_escape,
    on_error,
    on_shutdown,
    with_statement,
    endwith_statement,
    try_statement,
    catch_statement,
    finally_statement,
    endtry_statement,
    throw_statement,
    public_declaration,
    local_declaration,
    private_declaration,
    parameters_declaration,
    lparameters_declaration,
    dimension_command,
    store_command,
    close_command,
    erase_command,
    copy_file_command,
    rename_file_command,
    print_command,
    create_cursor_command,
    create_table_command,
    alter_table_command,
    copy_to_command,
    append_from_command,
    save_memvars_command,
    restore_memvars_command,
    scatter_command,
    gather_command,
    update_command,
    retry_statement,
    resume_statement,
    declare_dll,
    call_command,
    for_each_statement,
    release_command,
    clear_memory_command,
    cancel_statement,
    quit_statement,
    yield_statement,
    doevents_command,
    enter_critical_command,
    exit_critical_command,
    spawn_command,
    await_command,
    on_shutdown_statement,
    edit_command,
    change_command,
    input_command,
    accept_command,
    getfile_command,
    putfile_command,
    getdir_command,
    inputbox_command,
    wait_command,
    sleep_command,
    keyboard_command,
    push_key_command,
    pop_key_command,
    push_menu_command,
    pop_menu_command,
    push_popup_command,
    pop_popup_command,
    display_command,
    list_command,
    no_op
};

struct Statement {
    StatementKind kind = StatementKind::no_op;
    SourceLocation location{};
    std::string text;
    std::string identifier;
    std::string expression;
    std::string secondary_expression;
    std::string tertiary_expression;
    std::string quaternary_expression;
    std::vector<std::string> names;
};

enum class RoutineKind {
    main,
    procedure,
    function
};

struct Routine {
    std::string name;
    RoutineKind kind = RoutineKind::main;
    NativeMemberVisibility visibility = NativeMemberVisibility::public_member;
    SourceLocation declaration_location{};
    std::size_t body_end_line_exclusive = 0;
    std::vector<Statement> statements;
};

struct NativeChildObjectDeclaration {
    std::string name;
    std::string class_name;
    std::string source_path;
    SourceLocation declaration_location{};
    std::string text;
    std::vector<Statement> property_statements;
};

struct PrgClassDefinition {
    std::string name;
    std::string base_class_name;
    std::string base_class_source_path;
    SourceLocation declaration_location{};
    std::vector<Statement> property_statements;
    std::map<std::string, NativeMemberVisibility> member_visibility;
    std::vector<NativeChildObjectDeclaration> child_object_declarations;
    std::map<std::string, Routine> methods;
};

struct Program {
    std::string path;
    std::vector<std::string> source_lines;
    Routine main{};
    std::map<std::string, Routine> routines;
    std::map<std::string, PrgClassDefinition> classes;
};

Program parse_program(const std::string& path);
Program parse_program_source(
    const std::string& logical_path,
    const std::string& source_text,
    const std::map<std::string, std::string>& source_text_overrides = {},
    bool require_source_text_overrides = false);

}  // namespace copperfin::runtime
