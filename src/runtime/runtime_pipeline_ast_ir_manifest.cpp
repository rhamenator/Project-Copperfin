// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

const char* statement_kind_name(const StatementKind kind) {
    switch (kind) {
        case StatementKind::assignment:
            return "assignment";
        case StatementKind::expression:
            return "expression";
        case StatementKind::do_command:
            return "do_command";
        case StatementKind::do_while_statement:
            return "do_while_statement";
        case StatementKind::do_case_statement:
            return "do_case_statement";
        case StatementKind::case_statement:
            return "case_statement";
        case StatementKind::otherwise_statement:
            return "otherwise_statement";
        case StatementKind::calculate_command:
            return "calculate_command";
        case StatementKind::count_command:
            return "count_command";
        case StatementKind::sum_command:
            return "sum_command";
        case StatementKind::average_command:
            return "average_command";
        case StatementKind::text_command:
            return "text_command";
        case StatementKind::total_command:
            return "total_command";
        case StatementKind::do_form:
            return "do_form";
        case StatementKind::report_form:
            return "report_form";
        case StatementKind::label_form:
            return "label_form";
        case StatementKind::define_popup_command:
            return "define_popup_command";
        case StatementKind::define_bar_command:
            return "define_bar_command";
        case StatementKind::activate_surface:
            return "activate_surface";
        case StatementKind::release_surface:
            return "release_surface";
        case StatementKind::return_statement:
            return "return_statement";
        case StatementKind::nodefault_statement:
            return "nodefault_statement";
        case StatementKind::if_statement:
            return "if_statement";
        case StatementKind::else_statement:
            return "else_statement";
        case StatementKind::endif_statement:
            return "endif_statement";
        case StatementKind::for_statement:
            return "for_statement";
        case StatementKind::endfor_statement:
            return "endfor_statement";
        case StatementKind::loop_statement:
            return "loop_statement";
        case StatementKind::continue_command:
            return "continue_command";
        case StatementKind::exit_statement:
            return "exit_statement";
        case StatementKind::enddo_statement:
            return "enddo_statement";
        case StatementKind::endcase_statement:
            return "endcase_statement";
        case StatementKind::read_events:
            return "read_events";
        case StatementKind::clear_events:
            return "clear_events";
        case StatementKind::begin_transaction:
            return "begin_transaction";
        case StatementKind::end_transaction:
            return "end_transaction";
        case StatementKind::rollback_transaction:
            return "rollback_transaction";
        case StatementKind::undo_command:
            return "undo_command";
        case StatementKind::seek_command:
            return "seek_command";
        case StatementKind::locate_command:
            return "locate_command";
        case StatementKind::scan_statement:
            return "scan_statement";
        case StatementKind::endscan_statement:
            return "endscan_statement";
        case StatementKind::replace_command:
            return "replace_command";
        case StatementKind::append_blank_command:
            return "append_blank_command";
        case StatementKind::delete_command:
            return "delete_command";
        case StatementKind::recall_command:
            return "recall_command";
        case StatementKind::pack_command:
            return "pack_command";
        case StatementKind::zap_command:
            return "zap_command";
        case StatementKind::unlock_command:
            return "unlock_command";
        case StatementKind::delete_from_command:
            return "delete_from_command";
        case StatementKind::insert_into_command:
            return "insert_into_command";
        case StatementKind::go_command:
            return "go_command";
        case StatementKind::skip_command:
            return "skip_command";
        case StatementKind::browse_command:
            return "browse_command";
        case StatementKind::select_command:
            return "select_command";
        case StatementKind::use_command:
            return "use_command";
        case StatementKind::open_database:
            return "open_database";
        case StatementKind::set_order:
            return "set_order";
        case StatementKind::set_command:
            return "set_command";
        case StatementKind::set_procedure:
            return "set_procedure";
        case StatementKind::set_library:
            return "set_library";
        case StatementKind::set_datasession:
            return "set_datasession";
        case StatementKind::set_default:
            return "set_default";
        case StatementKind::set_memowidth:
            return "set_memowidth";
        case StatementKind::on_error:
            return "on_error";
        case StatementKind::on_shutdown:
            return "on_shutdown";
        case StatementKind::with_statement:
            return "with_statement";
        case StatementKind::endwith_statement:
            return "endwith_statement";
        case StatementKind::try_statement:
            return "try_statement";
        case StatementKind::catch_statement:
            return "catch_statement";
        case StatementKind::finally_statement:
            return "finally_statement";
        case StatementKind::endtry_statement:
            return "endtry_statement";
        case StatementKind::throw_statement:
            return "throw_statement";
        case StatementKind::public_declaration:
            return "public_declaration";
        case StatementKind::local_declaration:
            return "local_declaration";
        case StatementKind::private_declaration:
            return "private_declaration";
        case StatementKind::parameters_declaration:
            return "parameters_declaration";
        case StatementKind::lparameters_declaration:
            return "lparameters_declaration";
        case StatementKind::dimension_command:
            return "dimension_command";
        case StatementKind::store_command:
            return "store_command";
        case StatementKind::close_command:
            return "close_command";
        case StatementKind::erase_command:
            return "erase_command";
        case StatementKind::copy_file_command:
            return "copy_file_command";
        case StatementKind::rename_file_command:
            return "rename_file_command";
        case StatementKind::print_command:
            return "print_command";
        case StatementKind::create_cursor_command:
            return "create_cursor_command";
        case StatementKind::create_table_command:
            return "create_table_command";
        case StatementKind::alter_table_command:
            return "alter_table_command";
        case StatementKind::copy_to_command:
            return "copy_to_command";
        case StatementKind::append_from_command:
            return "append_from_command";
        case StatementKind::save_memvars_command:
            return "save_memvars_command";
        case StatementKind::restore_memvars_command:
            return "restore_memvars_command";
        case StatementKind::scatter_command:
            return "scatter_command";
        case StatementKind::gather_command:
            return "gather_command";
        case StatementKind::update_command:
            return "update_command";
        case StatementKind::retry_statement:
            return "retry_statement";
        case StatementKind::resume_statement:
            return "resume_statement";
        case StatementKind::declare_dll:
            return "declare_dll";
        case StatementKind::call_command:
            return "call_command";
        case StatementKind::for_each_statement:
            return "for_each_statement";
        case StatementKind::release_command:
            return "release_command";
        case StatementKind::clear_memory_command:
            return "clear_memory_command";
        case StatementKind::cancel_statement:
            return "cancel_statement";
        case StatementKind::quit_statement:
            return "quit_statement";
        case StatementKind::yield_statement:
            return "yield_statement";
        case StatementKind::doevents_command:
            return "doevents_command";
        case StatementKind::enter_critical_command:
            return "enter_critical_command";
        case StatementKind::exit_critical_command:
            return "exit_critical_command";
        case StatementKind::spawn_command:
            return "spawn_command";
        case StatementKind::await_command:
            return "await_command";
        case StatementKind::on_shutdown_statement:
            return "on_shutdown_statement";
        case StatementKind::edit_command:
            return "edit_command";
        case StatementKind::change_command:
            return "change_command";
        case StatementKind::input_command:
            return "input_command";
        case StatementKind::accept_command:
            return "accept_command";
        case StatementKind::getfile_command:
            return "getfile_command";
        case StatementKind::putfile_command:
            return "putfile_command";
        case StatementKind::getdir_command:
            return "getdir_command";
        case StatementKind::inputbox_command:
            return "inputbox_command";
        case StatementKind::wait_command:
            return "wait_command";
        case StatementKind::sleep_command:
            return "sleep_command";
        case StatementKind::keyboard_command:
            return "keyboard_command";
        case StatementKind::push_key_command:
            return "push_key_command";
        case StatementKind::pop_key_command:
            return "pop_key_command";
        case StatementKind::push_menu_command:
            return "push_menu_command";
        case StatementKind::pop_menu_command:
            return "pop_menu_command";
        case StatementKind::push_popup_command:
            return "push_popup_command";
        case StatementKind::pop_popup_command:
            return "pop_popup_command";
        case StatementKind::display_command:
            return "display_command";
        case StatementKind::list_command:
            return "list_command";
        case StatementKind::no_op:
            return "no_op";
    }
    return "no_op";
}

void append_ast_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements) {
    stream << "        {\n";
    stream << "          \"name\": \"" << json_escape(routine_name) << "\",\n";
    stream << "          \"statements\": [\n";
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto& statement = statements[index];
        stream << "            {\"line\": " << statement.location.line
               << ", \"text\": \"" << json_escape(statement.text) << "\""
               << ", \"identifier\": \"" << json_escape(statement.identifier) << "\""
               << ", \"expression\": \"" << json_escape(statement.expression) << "\"}";
        if (index + 1U != statements.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "          ]\n";
    stream << "        }";
}

std::string build_ast_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"schema_version\": 1,\n";
    stream << "  \"project_title\": \"" << json_escape(plan.project_title) << "\",\n";
    stream << "  \"output_kind\": \"" << json_escape(build_output_kind_name(plan.output_kind)) << "\",\n";
    stream << "  \"files\": [\n";

    bool first_file = true;
    for (const auto& asset : plan.assets) {
        if (!asset.copied ||
            lowercase_copy(trim_copy(copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(asset.source_path).extension()))) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        if (!first_file) {
            stream << ",\n";
        }
        first_file = false;
        stream << "    {\n";
        stream << "      \"relative_path\": \"" << json_escape(asset.relative_path) << "\",\n";
        stream << "      \"routines\": [\n";
        append_ast_routine_json(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            stream << ",\n";
            append_ast_routine_json(stream, routine_entry.first, routine_entry.second.statements);
        }
        stream << "\n";
        stream << "      ]\n";
        stream << "    }";
    }

    stream << "\n";
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

void append_ir_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements) {
    stream << "        {\n";
    stream << "          \"name\": \"" << json_escape(routine_name) << "\",\n";
    stream << "          \"instructions\": [\n";
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto& statement = statements[index];
        stream << "            {\"line\": " << statement.location.line
               << ", \"opcode\": \"" << json_escape(statement_kind_name(statement.kind)) << "\""
               << ", \"text\": \"" << json_escape(statement.text) << "\""
               << ", \"identifier\": \"" << json_escape(statement.identifier) << "\""
               << ", \"expression\": \"" << json_escape(statement.expression) << "\""
               << ", \"secondary_expression\": \"" << json_escape(statement.secondary_expression) << "\""
               << ", \"tertiary_expression\": \"" << json_escape(statement.tertiary_expression) << "\""
               << ", \"quaternary_expression\": \"" << json_escape(statement.quaternary_expression) << "\""
               << ", \"operand_count\": " << statement.names.size() << "}";
        if (index + 1U != statements.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "          ]\n";
    stream << "        }";
}

std::string build_ir_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"schema_version\": 1,\n";
    stream << "  \"project_title\": \"" << json_escape(plan.project_title) << "\",\n";
    stream << "  \"output_kind\": \"" << json_escape(build_output_kind_name(plan.output_kind)) << "\",\n";
    stream << "  \"files\": [\n";

    bool first_file = true;
    for (const auto& asset : plan.assets) {
        if (!asset.copied ||
            lowercase_copy(trim_copy(copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(asset.source_path).extension()))) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        if (!first_file) {
            stream << ",\n";
        }
        first_file = false;
        stream << "    {\n";
        stream << "      \"relative_path\": \"" << json_escape(asset.relative_path) << "\",\n";
        stream << "      \"routines\": [\n";
        append_ir_routine_json(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            stream << ",\n";
            append_ir_routine_json(stream, routine_entry.first, routine_entry.second.statements);
        }
        stream << "\n";
        stream << "      ]\n";
        stream << "    }";
    }

    stream << "\n";
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime
