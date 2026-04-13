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
    activate_surface,
    release_surface,
    return_statement,
    if_statement,
    else_statement,
    endif_statement,
    for_statement,
    endfor_statement,
    loop_statement,
    exit_statement,
    enddo_statement,
    endcase_statement,
    read_events,
    clear_events,
    seek_command,
    locate_command,
    scan_statement,
    endscan_statement,
    replace_command,
    append_blank_command,
    delete_command,
    recall_command,
    go_command,
    skip_command,
    select_command,
    use_command,
    set_order,
    set_command,
    set_library,
    set_datasession,
    set_default,
    on_error,
    with_statement,
    endwith_statement,
    try_statement,
    catch_statement,
    finally_statement,
    endtry_statement,
    public_declaration,
    local_declaration,
    private_declaration,
    parameters_declaration,
    lparameters_declaration,
    store_command,
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

struct Routine {
    std::string name;
    std::vector<Statement> statements;
};

struct Program {
    std::string path;
    Routine main{};
    std::map<std::string, Routine> routines;
};

Program parse_program(const std::string& path);

}  // namespace copperfin::runtime
