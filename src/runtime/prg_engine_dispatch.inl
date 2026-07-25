// prg_engine_dispatch.inl
// PrgRuntimeSession::Impl::execute_current_statement implementation.
// This file is #included directly into prg_engine.cpp inside namespace copperfin::runtime.
// It must not be compiled separately.

    ExecutionOutcome PrgRuntimeSession::Impl::execute_current_statement()
    {
        if (stack.empty())
        {
            return {};
        }

        Frame &frame = stack.back();
        const bool resuming_expression = frame.expression_routine_return_pending;
        if (!resuming_expression &&
            (frame.routine == nullptr || frame.pc >= frame.routine->statements.size()))
        {
            pop_frame();
            return {};
        }

        const std::size_t statement_index =
            resuming_expression && frame.pc > 0U ? frame.pc - 1U : frame.pc;
        if (frame.routine == nullptr || statement_index >= frame.routine->statements.size())
        {
            pop_frame();
            return {};
        }
        const Statement statement = frame.routine->statements[statement_index];
        if (!resuming_expression)
        {
            ++frame.pc;
            ++executed_statement_count;

            events.push_back({.category = "execute",
                              .detail = display_asset_paths_in_statement(statement.text),
                              .location = statement.location});
        }
        const auto abandon_resumed_expression = [&]()
        {
            if (resuming_expression)
            {
                frame.expression_routine_return_pending = false;
                frame.expression_continuation.reset();
                frame.command_target_continuation.reset();
                frame.command_array_name_continuation.reset();
                frame.command_argument_continuation.reset();
                frame.text_merge_continuation.reset();
                frame.parameter_default_continuation.reset();
                frame.use_command_continuation.reset();
                frame.copy_file_continuation.reset();
                frame.rename_file_continuation.reset();
            }
        };
        std::optional<PrgValue> resumed_assignment_value;
        std::optional<Statement> resumed_assignment_statement;
        std::optional<PrgValue> resumed_store_value;
        std::optional<Statement> resumed_store_statement;
        std::optional<PrgValue> resumed_sleep_value;
        std::optional<PrgValue> resumed_seek_value;
        std::optional<PrgValue> resumed_skip_value;
        std::optional<PrgValue> resumed_go_value;
        std::optional<PrgValue> resumed_unlock_record_value;
        std::optional<PrgValue> resumed_use_target_value;
        std::optional<PrgValue> resumed_use_alias_value;
        std::optional<PrgValue> resumed_open_database_target_value;
        std::optional<PrgValue> resumed_await_handle_value;
        std::optional<PrgValue> resumed_erase_path_value;
        std::optional<PrgValue> resumed_with_target_value;
        std::optional<PrgValue> resumed_throw_value;
        std::optional<PrgValue> resumed_textmerge_value;
        std::optional<PrgValue> resumed_parameter_default_value;
        std::optional<PrgValue> resumed_copy_source_value;
        std::optional<PrgValue> resumed_copy_destination_value;
        std::optional<PrgValue> resumed_copy_to_destination_value;
        std::optional<PrgValue> resumed_append_from_source_value;
        std::optional<PrgValue> resumed_save_memvars_path_value;
        std::optional<PrgValue> resumed_restore_memvars_path_value;
        std::optional<PrgValue> resumed_set_default_path_value;
        std::optional<PrgValue> resumed_set_datasession_value;
        std::optional<PrgValue> resumed_set_memowidth_value;
        std::optional<PrgValue> resumed_set_library_value;
        std::optional<PrgValue> resumed_declare_dll_path_value;
        std::optional<PrgValue> resumed_gather_for_value;
        std::optional<PrgValue> resumed_set_procedure_target_value;
        std::optional<PrgValue> resumed_rename_source_value;
        std::optional<PrgValue> resumed_rename_destination_value;
        std::optional<PrgValue> resumed_do_argument_value;
        std::optional<PrgValue> resumed_spawn_argument_value;
        std::optional<PrgValue> resumed_call_argument_value;
        std::optional<PrgValue> resumed_expression_value;
        std::optional<Statement> resumed_expression_statement;
        std::optional<PrgValue> resumed_command_target_value;
        std::optional<PrgValue> resumed_command_array_name_value;
        bool resumed_conditional_expression = false;
        bool resumed_case_expression = false;
        bool resumed_loop_expression = false;
        bool resumed_scan_expression = false;

        const auto emit_print_event = [&](const PrgValue &value, const SourceLocation &location)
        {
            const auto display_set_value = [this](const std::string &option_name)
            {
                const std::string normalized_name = normalize_identifier(trim_copy(option_name));
                const auto found = current_set_state().find(normalized_name);
                if (found != current_set_state().end())
                {
                    return found->second;
                }
                if (normalized_name == "decimals")
                {
                    return std::string{"2"};
                }
                if (normalized_name == "point")
                {
                    return std::string{"."};
                }
                if (normalized_name == "separator")
                {
                    return std::string{","};
                }
                return std::string{"OFF"};
            };
            events.push_back({.category = "runtime.print",
                              .detail = format_value_for_display(value, display_set_value),
                              .location = location});
        };

        const auto emit_wait_window_event = [&](const PrgValue &value, const SourceLocation &location)
        {
            events.push_back({.category = "ui.wait_window",
                              .detail = value_as_string(value),
                              .location = location});
        };

        const auto route_false_conditional = [&]()
        {
            if (const auto destination = find_matching_branch(frame, frame.pc - 1U, true))
            {
                const Statement &destination_statement = frame.routine->statements[*destination];
                if (destination_statement.kind == StatementKind::else_statement &&
                    !trim_copy(destination_statement.expression).empty())
                {
                    frame.evaluate_conditional_else = true;
                    frame.pc = *destination;
                }
                else
                {
                    frame.evaluate_conditional_else = false;
                    frame.pc = *destination + 1U;
                }
            }
        };
        const auto apply_conditional_predicate = [&](StatementKind kind, bool predicate_value)
        {
            if (kind == StatementKind::if_statement)
            {
                if (!predicate_value)
                {
                    route_false_conditional();
                }
                else
                {
                    frame.evaluate_conditional_else = false;
                }
                return;
            }

            frame.evaluate_conditional_else = false;
            if (!predicate_value)
            {
                route_false_conditional();
            }
        };
        const auto apply_case_predicate = [&](bool predicate_value)
        {
            if (frame.cases.empty())
            {
                return;
            }

            CaseState &active_case = frame.cases.back();
            if (active_case.matched)
            {
                const std::size_t next_pc = active_case.endcase_statement_index + 1U;
                frame.cases.pop_back();
                frame.pc = next_pc;
                return;
            }

            if (predicate_value)
            {
                active_case.matched = true;
                return;
            }

            if (const auto destination = find_next_case_clause(frame, frame.pc - 1U))
            {
                frame.pc = *destination;
            }
        };

        const auto make_loop_stage_statement = [&](const Statement &original,
                                                   const std::string &expression,
                                                   LoopExpressionStage stage)
        {
            Statement staged = original;
            staged.expression = expression;
            staged.secondary_expression.clear();
            staged.tertiary_expression.clear();
            staged.text = original.text + " [loop-expression-stage=" +
                          std::to_string(static_cast<int>(stage)) + "]";
            return staged;
        };

        const auto initialize_for_loop = [&](Frame &target,
                                             const Statement &loop_statement,
                                             double start_value,
                                             double end_value,
                                             double step_value)
        {
            assign_variable(target, loop_statement.identifier, make_number_value(start_value));
            const bool should_enter = step_value >= 0.0 ? start_value <= end_value : start_value >= end_value;
            if (!should_enter)
            {
                if (const auto destination = find_matching_endfor(target, target.pc - 1U))
                {
                    target.pc = *destination + 1U;
                }
                return;
            }
            const auto existing = std::find_if(target.loops.rbegin(), target.loops.rend(), [&](const LoopState &loop)
                                               { return loop.for_statement_index == (target.pc - 1U); });
            if (existing != target.loops.rend())
            {
                return;
            }
            target.loops.push_back({.for_statement_index = target.pc - 1U,
                                    .endfor_statement_index = find_matching_endfor(target, target.pc - 1U).value_or(target.pc - 1U),
                                    .case_stack_depth_at_entry = target.cases.size(),
                                    .with_stack_depth_at_entry = target.withs.size(),
                                    .variable_name = normalize_identifier(loop_statement.identifier),
                                    .end_value = end_value,
                                    .step_value = step_value,
                                    .iteration_count = 0});
        };

        const auto initialize_do_while = [&](Frame &target, const Statement &, bool should_continue)
        {
            const auto existing = std::find_if(target.whiles.rbegin(), target.whiles.rend(), [&](const WhileState &loop)
                                                { return loop.do_while_statement_index == (target.pc - 1U); });
            if (should_continue)
            {
                if (existing == target.whiles.rend())
                {
                    target.whiles.push_back({.do_while_statement_index = target.pc - 1U,
                                             .enddo_statement_index = find_matching_enddo(target, target.pc - 1U).value_or(target.pc - 1U),
                                             .case_stack_depth_at_entry = target.cases.size(),
                                             .with_stack_depth_at_entry = target.withs.size(),
                                             .iteration_count = 0});
                }
            }
            else
            {
                if (existing != target.whiles.rend())
                {
                    target.whiles.erase(std::next(existing).base());
                }
                if (const auto destination = find_matching_enddo(target, target.pc - 1U))
                {
                    target.pc = *destination + 1U;
                }
            }
        };

        const auto initialize_for_each = [&](Frame &target, const Statement &loop_statement, PrgValue result)
        {
            const std::string var_name = normalize_memory_variable_identifier(loop_statement.identifier);
            std::vector<PrgValue> elements;
            if (const auto runtime_object = resolve_ole_object(result);
                runtime_object.has_value() && is_native_collection_object(**runtime_object))
            {
                elements = (*runtime_object)->collection_items;
            }
            else
            {
                elements.push_back(std::move(result));
            }

            if (elements.empty())
            {
                if (const auto destination = find_matching_endfor(target, target.pc - 1U))
                {
                    target.pc = *destination + 1U;
                }
                return;
            }

            assign_variable(target, var_name, elements[0]);
            target.loops.push_back({
                .for_statement_index = target.pc - 1U,
                .endfor_statement_index = find_matching_endfor(target, target.pc - 1U).value_or(target.pc - 1U),
                .case_stack_depth_at_entry = target.cases.size(),
                .with_stack_depth_at_entry = target.withs.size(),
                .variable_name = var_name,
                .is_for_each = true,
                .each_values = std::move(elements),
                .each_index = 0U});
        };

        const auto finish_loop_expression = [&](Frame &target, const Statement &, PrgValue value)
        {
            if (!target.loop_expression_continuation.has_value())
            {
                return true;
            }

            LoopExpressionContinuation &continuation = *target.loop_expression_continuation;
            while (true)
            {
                switch (continuation.stage)
                {
                case LoopExpressionStage::do_while_predicate:
                    initialize_do_while(target, continuation.statement, value_as_bool(value));
                    target.loop_expression_continuation.reset();
                    return true;
                case LoopExpressionStage::for_each_collection:
                    initialize_for_each(target, continuation.statement, std::move(value));
                    target.loop_expression_continuation.reset();
                    return true;
                case LoopExpressionStage::for_start:
                    continuation.start_value = value_as_number(value);
                    continuation.stage = LoopExpressionStage::for_end;
                    {
                        const auto end_value = evaluate_resumable_expression(
                            target,
                            make_loop_stage_statement(
                                continuation.statement,
                                continuation.statement.secondary_expression,
                                LoopExpressionStage::for_end));
                        if (!end_value.has_value())
                        {
                            return false;
                        }
                        value = *end_value;
                    }
                    if (target.expression_routine_return_pending)
                    {
                        return false;
                    }
                    continue;
                case LoopExpressionStage::for_end:
                    continuation.end_value = value_as_number(value);
                    continuation.stage = LoopExpressionStage::for_step;
                    if (continuation.statement.tertiary_expression.empty())
                    {
                        value = make_number_value(1.0);
                    }
                    else
                    {
                        const auto step_value = evaluate_resumable_expression(
                            target,
                            make_loop_stage_statement(
                                continuation.statement,
                                continuation.statement.tertiary_expression,
                                LoopExpressionStage::for_step));
                        if (!step_value.has_value())
                        {
                            return false;
                        }
                        value = *step_value;
                    }
                    continue;
                case LoopExpressionStage::for_step:
                    initialize_for_loop(
                        target,
                        continuation.statement,
                        continuation.start_value,
                        continuation.end_value,
                        value_as_number(value));
                    target.loop_expression_continuation.reset();
                    return true;
                }
            }
        };

        try
        {
            if (resuming_expression)
            {
                frame.expression_routine_return_pending = false;
                if (frame.expression_continuation.has_value())
                {
                    ExpressionContinuation &continuation =
                        *frame.expression_continuation;
                    if (continuation.awaiting_routine.has_value())
                    {
                        continuation.routine_results[*continuation.awaiting_routine] =
                            last_return_value.value_or(make_empty_value());
                        continuation.awaiting_routine.reset();
                    }

                    const Statement continued_statement = continuation.statement;
                    const auto expression_value =
                        evaluate_resumable_expression(frame, continued_statement);
                    if (!expression_value.has_value())
                    {
                        return {};
                    }
                    if (continued_statement.kind == StatementKind::assignment)
                    {
                        resumed_assignment_value = *expression_value;
                        resumed_assignment_statement = continued_statement;
                    }
                    else if (continued_statement.kind == StatementKind::if_statement ||
                             continued_statement.kind == StatementKind::else_statement)
                    {
                        apply_conditional_predicate(
                            continued_statement.kind,
                            value_as_bool(*expression_value));
                        resumed_conditional_expression = true;
                    }
                    else if (continued_statement.kind == StatementKind::case_statement)
                    {
                        apply_case_predicate(value_as_bool(*expression_value));
                        resumed_case_expression = true;
                    }
                    else if (continued_statement.kind == StatementKind::for_statement ||
                             continued_statement.kind == StatementKind::do_while_statement ||
                             continued_statement.kind == StatementKind::for_each_statement)
                    {
                        resumed_loop_expression = true;
                        if (!finish_loop_expression(frame, continued_statement, *expression_value))
                        {
                            return {};
                        }
                    }
                    else if (continued_statement.kind == StatementKind::scan_statement)
                    {
                        resumed_scan_expression = true;
                        const auto scan_outcome = continue_scan_expression_search(frame, *expression_value);
                        if (!scan_outcome.ok)
                        {
                            return scan_outcome;
                        }
                    }
                    else if (continued_statement.kind == StatementKind::expression ||
                             continued_statement.kind == StatementKind::print_command)
                    {
                        resumed_expression_value = *expression_value;
                        resumed_expression_statement = continued_statement;
                    }
                    else if (continued_statement.kind == StatementKind::wait_command)
                    {
                        resumed_expression_value = *expression_value;
                        resumed_expression_statement = continued_statement;
                    }
                    else if (continued_statement.kind == StatementKind::store_command)
                    {
                        resumed_store_value = *expression_value;
                        resumed_store_statement = continued_statement;
                    }
                    else if (continued_statement.kind == StatementKind::sleep_command)
                    {
                        resumed_sleep_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::seek_command)
                    {
                        resumed_seek_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::skip_command)
                    {
                        resumed_skip_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::go_command)
                    {
                        resumed_go_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::unlock_command &&
                             !trim_copy(continued_statement.identifier).empty())
                    {
                        resumed_unlock_record_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::use_command &&
                             frame.use_command_continuation.has_value() &&
                             frame.use_command_continuation->pending_alias)
                    {
                        resumed_use_alias_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::use_command &&
                             !continued_statement.expression.empty())
                    {
                        resumed_use_target_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::open_database)
                    {
                        resumed_open_database_target_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::await_command)
                    {
                        resumed_await_handle_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::erase_command)
                    {
                        resumed_erase_path_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::copy_file_command &&
                             frame.copy_file_continuation.has_value() &&
                             frame.copy_file_continuation->pending_destination)
                    {
                        resumed_copy_destination_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::copy_file_command)
                    {
                        resumed_copy_source_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::rename_file_command &&
                             frame.rename_file_continuation.has_value() &&
                             frame.rename_file_continuation->pending_destination)
                    {
                        resumed_rename_destination_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::rename_file_command)
                    {
                        resumed_rename_source_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::copy_to_command &&
                             continued_statement.identifier != "array")
                    {
                        resumed_copy_to_destination_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::append_from_command &&
                             continued_statement.identifier != "array")
                    {
                        resumed_append_from_source_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::save_memvars_command)
                    {
                        resumed_save_memvars_path_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::restore_memvars_command)
                    {
                        resumed_restore_memvars_path_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::set_default)
                    {
                        resumed_set_default_path_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::set_datasession)
                    {
                        resumed_set_datasession_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::set_memowidth)
                    {
                        resumed_set_memowidth_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::set_library)
                    {
                        resumed_set_library_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::declare_dll)
                    {
                        resumed_declare_dll_path_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::set_procedure &&
                             !trim_copy(continued_statement.expression).empty() &&
                             trim_copy(continued_statement.expression).front() == '&')
                    {
                        resumed_set_procedure_target_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::with_statement)
                    {
                        resumed_with_target_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::throw_statement)
                    {
                        resumed_throw_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::text_command &&
                             frame.text_merge_continuation.has_value())
                    {
                        resumed_textmerge_value = *expression_value;
                    }
                    else if ((continued_statement.kind == StatementKind::parameters_declaration ||
                              continued_statement.kind == StatementKind::lparameters_declaration) &&
                             frame.parameter_default_continuation.has_value())
                    {
                        resumed_parameter_default_value = *expression_value;
                    }
                    else if ((continued_statement.kind == StatementKind::do_command ||
                              continued_statement.kind == StatementKind::spawn_command ||
                              continued_statement.kind == StatementKind::call_command) &&
                             frame.command_target_continuation.has_value())
                    {
                        resumed_command_target_value = *expression_value;
                    }
                    else if (frame.command_array_name_continuation.has_value())
                    {
                        resumed_command_array_name_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::gather_command)
                    {
                        resumed_gather_for_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::do_command &&
                             frame.command_argument_continuation.has_value())
                    {
                        resumed_do_argument_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::spawn_command &&
                             frame.command_argument_continuation.has_value())
                    {
                        resumed_spawn_argument_value = *expression_value;
                    }
                    else if (continued_statement.kind == StatementKind::call_command &&
                             frame.command_argument_continuation.has_value())
                    {
                        resumed_call_argument_value = *expression_value;
                    }
                    else
                    {
                        last_return_value = *expression_value;
                    }
                }
                if (!resumed_assignment_value.has_value() && !resumed_store_value.has_value() &&
                    !resumed_sleep_value.has_value() &&
                    !resumed_seek_value.has_value() &&
                    !resumed_skip_value.has_value() &&
                    !resumed_go_value.has_value() &&
                    !resumed_unlock_record_value.has_value() &&
                    !resumed_use_target_value.has_value() &&
                    !resumed_use_alias_value.has_value() &&
                    !resumed_open_database_target_value.has_value() &&
                    !resumed_await_handle_value.has_value() &&
                    !resumed_erase_path_value.has_value() &&
                    !resumed_with_target_value.has_value() &&
                    !resumed_throw_value.has_value() &&
                    !resumed_textmerge_value.has_value() &&
                    !resumed_parameter_default_value.has_value() &&
                    !resumed_copy_source_value.has_value() &&
                    !resumed_copy_destination_value.has_value() &&
                    !resumed_copy_to_destination_value.has_value() &&
                    !resumed_append_from_source_value.has_value() &&
                    !resumed_save_memvars_path_value.has_value() &&
                    !resumed_restore_memvars_path_value.has_value() &&
                    !resumed_set_default_path_value.has_value() &&
                    !resumed_set_datasession_value.has_value() &&
                    !resumed_set_memowidth_value.has_value() &&
                    !resumed_set_library_value.has_value() &&
                    !resumed_declare_dll_path_value.has_value() &&
                    !resumed_gather_for_value.has_value() &&
                    !resumed_set_procedure_target_value.has_value() &&
                    !resumed_command_target_value.has_value() &&
                    !resumed_command_array_name_value.has_value() &&
                    !resumed_rename_source_value.has_value() &&
                    !resumed_rename_destination_value.has_value() &&
                    !resumed_do_argument_value.has_value() &&
                    !resumed_spawn_argument_value.has_value() &&
                    !resumed_call_argument_value.has_value() &&
                    !resumed_expression_value.has_value() &&
                    !resumed_conditional_expression && !resumed_case_expression &&
                    !resumed_loop_expression && !resumed_scan_expression)
                {
                    frame.return_pending = true;
                    if (const auto outcome = continue_pending_return(frame); outcome.has_value())
                    {
                        return *outcome;
                    }
                    return {};
                }
                if (resumed_expression_value.has_value() && resumed_expression_statement.has_value())
                {
                    const Statement &continued_statement = *resumed_expression_statement;
                    if (continued_statement.kind == StatementKind::print_command)
                    {
                        emit_print_event(*resumed_expression_value, continued_statement.location);
                        return {};
                    }
                    if (continued_statement.kind == StatementKind::expression &&
                        continued_statement.identifier == "wait_window")
                    {
                        emit_wait_window_event(*resumed_expression_value, continued_statement.location);
                        return {};
                    }
                    if (continued_statement.kind != StatementKind::wait_command)
                    {
                        return {};
                    }
                }
                if (resumed_conditional_expression)
                {
                    return {};
                }
                if (resumed_case_expression)
                {
                    return {};
                }
                if (resumed_loop_expression)
                {
                    return {};
                }
                if (resumed_scan_expression)
                {
                    return {};
                }
            }

            auto format_stack_event_detail = [](std::size_t depth, const std::string &target, bool empty_pop)
            {
                std::string detail = "depth=" + std::to_string(depth);
                const std::string trimmed_target = trim_copy(target);
                if (!trimmed_target.empty())
                {
                    detail += "; target=" + trimmed_target;
                }
                if (empty_pop)
                {
                    detail += "; empty=true";
                }
                return detail;
            };

            auto assign_dialog_target_value = [&](const Statement &dialog_statement, Frame &dialog_frame, std::string &detail)
            {
                if (dialog_statement.names.empty())
                {
                    return;
                }
                const std::string target_name = trim_copy(dialog_statement.names.front());
                if (target_name.empty())
                {
                    return;
                }

                assign_variable(dialog_frame, target_name, make_string_value(""));
                if (!detail.empty())
                {
                    detail += " ";
                }
                detail += "result='";
                detail += "'";
            };

            auto append_cursor_view_metadata = [&](CursorState *cursor, const std::string &override_field_list_text, std::string &detail)
            {
                if (cursor == nullptr)
                {
                    return;
                }

                const std::vector<std::string> visible_fields = effective_visible_field_names(*cursor, override_field_list_text);
                std::string field_detail;
                for (std::size_t index = 0U; index < visible_fields.size(); ++index)
                {
                    if (index > 0U)
                    {
                        field_detail += ",";
                    }
                    field_detail += visible_fields[index];
                }

                if (!detail.empty())
                {
                    detail += " ";
                }
                detail += cursor->alias.empty()
                    ? ("workarea=" + std::to_string(cursor->work_area))
                    : (cursor->alias + "@" + std::to_string(cursor->work_area));
                detail += " recno=" + std::to_string(cursor->recno);
                detail += " records=" + std::to_string(cursor->record_count);
                detail += " fields=" + (field_detail.empty() ? std::string{"ALL"} : field_detail);
                detail += " filter=" + (cursor->filter_expression.empty() ? std::string{"<none>"} : cursor->filter_expression);
            };

            auto append_runtime_cursor_target_detail = [&](const std::string &raw_target, Frame &current_frame, std::string &detail)
            {
                const std::string trimmed_target = trim_copy(raw_target);
                if (trimmed_target.empty())
                {
                    return;
                }

                if (!detail.empty())
                {
                    detail += " ";
                }
                detail += "target=" + trimmed_target;
                const std::string resolved_target = evaluate_cursor_designator_expression(trimmed_target, current_frame);
                if (!resolved_target.empty() && resolved_target != trimmed_target)
                {
                    detail += " target_resolved=" + resolved_target;
                }
            };

            auto append_aggregate_scope_metadata = [&](const AggregateScopeClause &scope, std::string &detail)
            {
                std::string scope_detail = "ALL";
                switch (scope.kind)
                {
                case AggregateScopeKind::all_records:
                    scope_detail = "ALL";
                    break;
                case AggregateScopeKind::rest_records:
                    scope_detail = "REST";
                    break;
                case AggregateScopeKind::next_records:
                    scope_detail = "NEXT";
                    break;
                case AggregateScopeKind::record:
                    scope_detail = "RECORD";
                    break;
                }

                if (!scope.raw_value.empty())
                {
                    scope_detail += " " + trim_copy(scope.raw_value);
                }

                if (!detail.empty())
                {
                    detail += " ";
                }
                detail += "scope=" + scope_detail;
            };

            auto append_cursor_structure_metadata = [&](CursorState *cursor, std::string &detail)
            {
                if (cursor == nullptr)
                {
                    return;
                }

                const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);
                std::string field_detail;
                for (std::size_t index = 0U; index < fields.size(); ++index)
                {
                    if (index > 0U)
                    {
                        field_detail += ",";
                    }
                    field_detail += fields[index].name;
                }

                if (!detail.empty())
                {
                    detail += " ";
                }
                detail += cursor->alias.empty()
                    ? ("workarea=" + std::to_string(cursor->work_area))
                    : (cursor->alias + "@" + std::to_string(cursor->work_area));
                detail += " field_count=" + std::to_string(fields.size());
                detail += " schema_fields=" + (field_detail.empty() ? std::string{"<none>"} : field_detail);
                if (!cursor->source_path.empty())
                {
                    detail += " source=" + cursor->source_path;
                }
            };

            auto append_session_status_metadata = [&](Frame &current_frame, std::string &detail)
            {
                if (!detail.empty())
                {
                    detail += " ";
                }
                detail += "datasession=" + std::to_string(current_data_session);
                detail += " open_cursors=" + std::to_string(current_session_state().cursors.size());
                detail += " globals=" + std::to_string(globals.size());
                detail += " locals=" + std::to_string(current_frame.locals.size());
                detail += " selected_workarea=" + std::to_string(current_selected_work_area());
                append_cursor_view_metadata(resolve_cursor_target(std::to_string(current_selected_work_area())), {}, detail);
            };

            auto resolve_runtime_expression_text = [&](const std::string &raw_expression, Frame &current_frame) -> std::string
            {
                const std::string trimmed = trim_copy(raw_expression);
                if (trimmed.empty())
                {
                    return {};
                }

                const PrgValue evaluated = evaluate_expression(trimmed, current_frame);
                if (evaluated.kind != PrgValueKind::empty)
                {
                    return value_as_string(evaluated);
                }

                return unquote_string(trimmed);
            };

            auto resolve_runtime_target_identifier = [&](const std::string &raw_identifier, Frame &current_frame) -> std::string
            {
                std::string resolved_identifier = apply_with_context(raw_identifier, current_frame);
                if (!resolved_identifier.empty() && resolved_identifier.front() == '&')
                {
                    std::size_t macro_end = 1U;
                    while (macro_end < resolved_identifier.size())
                    {
                        const char ch = resolved_identifier[macro_end];
                        if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_')
                        {
                            ++macro_end;
                            continue;
                        }
                        break;
                    }

                    const std::string macro_name = resolved_identifier.substr(1U, macro_end - 1U);
                    if (!macro_name.empty())
                    {
                        std::string expanded_text = macro_name;
                        constexpr std::size_t max_macro_target_depth = 16U;
                        std::vector<std::string> visited_identifiers;
                        visited_identifiers.reserve(8U);
                        for (std::size_t depth = 0U; depth < max_macro_target_depth; ++depth)
                        {
                            if (!is_bare_identifier_text(expanded_text))
                            {
                                break;
                            }

                            const std::string normalized = normalize_memory_variable_identifier(expanded_text);
                            if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                            {
                                break;
                            }
                            visited_identifiers.push_back(normalized);

                            std::string next;
                            if (const auto local = current_frame.locals.find(normalized); local != current_frame.locals.end())
                            {
                                next = trim_copy(value_as_string(local->second));
                            }
                            else if (const auto global = globals.find(normalized); global != globals.end())
                            {
                                next = trim_copy(value_as_string(global->second));
                            }

                            if (next.empty() || next == expanded_text)
                            {
                                break;
                            }

                            const bool next_is_bare_identifier =
                                is_bare_identifier_text(next) &&
                                !next.empty() &&
                                (std::isalpha(static_cast<unsigned char>(next.front())) != 0 || next.front() == '_');
                            const bool next_is_structured_target =
                                next.find('.') != std::string::npos ||
                                next.find('[') != std::string::npos ||
                                next.find('(') != std::string::npos;
                            if (!next_is_bare_identifier && !next_is_structured_target)
                            {
                                break;
                            }

                            expanded_text = next;
                            if (!next_is_bare_identifier)
                            {
                                break;
                            }
                        }

                        resolved_identifier = expanded_text + resolved_identifier.substr(macro_end);
                    }
                }
                return resolved_identifier;
            };

            auto execute_with_command_undo = [&](const std::string &table_path, const std::string &command_label, auto &&operation) -> bool
            {
                if (!ensure_command_undo_backup_for_table(table_path))
                {
                    return false;
                }

                current_command_undo_journal().command_label = uppercase_copy(trim_copy(command_label));

                if (!operation())
                {
                    rollback_active_command_undo_journal();
                    return false;
                }

                commit_active_command_undo_journal();
                return true;
            };

            auto memory_value_type_code = [&](const std::string &name, const PrgValue &value) -> std::string
            {
                if (find_array(name) != nullptr)
                {
                    return "A";
                }
                if (resolve_ole_object(value).has_value())
                {
                    return "O";
                }
                switch (value.kind)
                {
                case PrgValueKind::boolean:
                    return "L";
                case PrgValueKind::number:
                case PrgValueKind::int64:
                case PrgValueKind::uint64:
                    return "N";
                case PrgValueKind::currency:
                    return "Y";
                case PrgValueKind::string:
                    return "C";
                case PrgValueKind::empty:
                    return "U";
                }
                return "U";
            };

            auto memory_value_preview = [&](const std::string &name, const PrgValue &value) -> std::string
            {
                if (const RuntimeArray *array = find_array(name); array != nullptr)
                {
                    return std::to_string(array->rows) + "x" + std::to_string(array->columns);
                }
                if (const auto object = resolve_ole_object(value); object.has_value())
                {
                    return "<object:" + (*object)->prog_id + " props=" + std::to_string((*object)->properties.size()) + ">";
                }
                return format_value(value);
            };

            auto memory_scope_for_name = [&](const std::string &name, const Frame &current_frame) -> std::string
            {
                if (current_frame.local_names.contains(name) || current_frame.locals.contains(name))
                {
                    return "local";
                }
                if (current_frame.private_saved_values.contains(name))
                {
                    return "private";
                }
                if (public_names.contains(name))
                {
                    return "public";
                }
                return "global";
            };

            auto append_memory_metadata = [&](Frame &current_frame, std::string &detail,
                                               const std::string &filter_pattern = std::string{},
                                               bool filter_is_except = false)
            {
                auto memory_name_wildcard_match = [](const std::string &pattern, const std::string &text) -> bool
                {
                    const std::string p = lowercase_copy(pattern);
                    const std::string t = lowercase_copy(text);
                    std::size_t pi = 0U;
                    std::size_t ti = 0U;
                    std::size_t star = std::string::npos;
                    std::size_t star_t = 0U;
                    while (ti < t.size())
                    {
                        if (pi < p.size() && (p[pi] == '?' || p[pi] == t[ti]))
                        {
                            ++pi; ++ti;
                        }
                        else if (pi < p.size() && p[pi] == '*')
                        {
                            star = pi++;
                            star_t = ti;
                        }
                        else if (star != std::string::npos)
                        {
                            pi = star + 1U;
                            ti = ++star_t;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    while (pi < p.size() && p[pi] == '*') { ++pi; }
                    return pi == p.size();
                };
                auto name_passes_filter = [&](const std::string &name) -> bool
                {
                    if (filter_pattern.empty())
                    {
                        return true;
                    }
                    const bool matches = memory_name_wildcard_match(filter_pattern, name);
                    return filter_is_except ? !matches : matches;
                };
                struct MemoryEntry
                {
                    std::string name;
                    std::string scope;
                    std::string type;
                    std::string preview;
                };

                std::vector<MemoryEntry> entries;
                std::vector<MemoryEntry> shadowed_entries;
                entries.reserve(globals.size() + current_frame.locals.size());

                for (const auto &[name, value] : globals)
                {
                    if (!name_passes_filter(name))
                    {
                        continue;
                    }
                    const MemoryEntry global_entry{
                        .name = name,
                        .scope = current_frame.private_saved_values.contains(name) ? "private" : (public_names.contains(name) ? "public" : "global"),
                        .type = memory_value_type_code(name, value),
                        .preview = memory_value_preview(name, value)};
                    if (current_frame.local_names.contains(name) || current_frame.locals.contains(name))
                    {
                        shadowed_entries.push_back(global_entry);
                    }
                    else
                    {
                        entries.push_back(global_entry);
                    }
                }

                for (const auto &[name, value] : current_frame.locals)
                {
                    if (!name_passes_filter(name))
                    {
                        continue;
                    }
                    entries.push_back({.name = name,
                                       .scope = "local",
                                       .type = memory_value_type_code(name, value),
                                       .preview = memory_value_preview(name, value)});
                }

                std::sort(entries.begin(), entries.end(), [](const MemoryEntry &left, const MemoryEntry &right) {
                    return left.name < right.name;
                });
                std::sort(shadowed_entries.begin(), shadowed_entries.end(), [](const MemoryEntry &left, const MemoryEntry &right) {
                    return left.name < right.name;
                });

                std::size_t public_count = 0U;
                std::size_t private_count = 0U;
                std::size_t local_count = 0U;
                std::size_t global_count = 0U;
                std::string memvar_detail;
                for (std::size_t index = 0U; index < entries.size(); ++index)
                {
                    const auto &entry = entries[index];
                    if (entry.scope == "public")
                    {
                        ++public_count;
                    }
                    else if (entry.scope == "private")
                    {
                        ++private_count;
                    }
                    else if (entry.scope == "local")
                    {
                        ++local_count;
                    }
                    else
                    {
                        ++global_count;
                    }

                    if (index > 0U)
                    {
                        memvar_detail += ",";
                    }
                    memvar_detail += entry.name + "{" + entry.scope + ":" + entry.type + "=" + entry.preview + "}";
                }

                std::string shadowed_detail;
                for (std::size_t index = 0U; index < shadowed_entries.size(); ++index)
                {
                    if (index > 0U)
                    {
                        shadowed_detail += ",";
                    }
                    const auto &entry = shadowed_entries[index];
                    shadowed_detail += entry.name + "{" + entry.scope + ":" + entry.type + "=" + entry.preview + "}";
                }

                std::vector<std::string> array_entries;
                array_entries.reserve(arrays.size());
                for (const auto &[name, array] : arrays)
                {
                    if (!name_passes_filter(name))
                    {
                        continue;
                    }
                    array_entries.push_back(name + "{" + memory_scope_for_name(name, current_frame) + ":A=" +
                                            std::to_string(array.rows) + "x" + std::to_string(array.columns) + "}");
                }
                std::sort(array_entries.begin(), array_entries.end());

                std::string array_detail;
                for (std::size_t index = 0U; index < array_entries.size(); ++index)
                {
                    if (index > 0U)
                    {
                        array_detail += ",";
                    }
                    array_detail += array_entries[index];
                }

                if (!detail.empty())
                {
                    detail += " ";
                }
                detail += "datasession=" + std::to_string(current_data_session);
                detail += " memvar_count=" + std::to_string(entries.size());
                detail += " public_count=" + std::to_string(public_count);
                detail += " private_count=" + std::to_string(private_count);
                detail += " local_count=" + std::to_string(local_count);
                detail += " global_count=" + std::to_string(global_count);
                detail += " array_count=" + std::to_string(array_entries.size());
                detail += " memvars=" + (memvar_detail.empty() ? std::string{"<none>"} : memvar_detail);
                detail += " shadowed=" + (shadowed_detail.empty() ? std::string{"<none>"} : shadowed_detail);
                detail += " arrays=" + (array_detail.empty() ? std::string{"<none>"} : array_detail);
            };

            auto assign_runtime_target_value = [&](const std::string &raw_identifier,
                                                  const PrgValue &assignment_value,
                                                  std::optional<std::string> assignment_expression_text = std::nullopt) -> ExecutionOutcome
            {
                std::string assignment_identifier = resolve_runtime_target_identifier(raw_identifier, frame);
                if (assignment_identifier.find('.') != std::string::npos &&
                    (assignment_identifier.find('(') != std::string::npos ||
                     assignment_identifier.find('[') != std::string::npos))
                {
                    const auto separator = assignment_identifier.find('.');
                    const std::string object_part = assignment_identifier.substr(0U, separator);
                    if (normalize_identifier(object_part) != "m")
                    {
                        const std::string member_path = assignment_identifier.substr(separator + 1U);
                        const std::string normalized_member_path = normalize_identifier(member_path);
                        if (starts_with_insensitive(normalized_member_path, "selected(") ||
                            starts_with_insensitive(normalized_member_path, "selected[") ||
                            starts_with_insensitive(normalized_member_path, "selectedid(") ||
                            starts_with_insensitive(normalized_member_path, "selectedid[") ||
                            starts_with_insensitive(normalized_member_path, "list(") ||
                            starts_with_insensitive(normalized_member_path, "list[") ||
                            starts_with_insensitive(normalized_member_path, "listitem(") ||
                            starts_with_insensitive(normalized_member_path, "listitem[") ||
                            starts_with_insensitive(normalized_member_path, "itemdata(") ||
                            starts_with_insensitive(normalized_member_path, "itemdata["))
                        {
                            const PrgValue object_value = lookup_variable(frame, object_part);
                            auto object = resolve_ole_object(object_value);
                            if (object.has_value())
                            {
                                const auto resolved_path = resolve_runtime_object_member_path(frame, object_part, member_path);
                                RuntimeOleObjectState *runtime_object =
                                    resolved_path.runtime_object == nullptr ? *object : resolved_path.runtime_object;
                                const std::string effective_member_path =
                                    resolved_path.remaining_member_path.empty()
                                        ? member_path
                                        : resolved_path.remaining_member_path;
                                if (write_native_property_if_present(
                                        *runtime_object,
                                        effective_member_path,
                                        assignment_value,
                                        frame,
                                        assignment_expression_text))
                                {
                                    runtime_object->last_action = effective_member_path + " = " + value_as_string(assignment_value);
                                    ++runtime_object->action_count;
                                    events.push_back({.category = "ole.set",
                                                      .detail = runtime_object->prog_id + "." + runtime_object->last_action,
                                                      .location = statement.location});
                                    return {};
                                }
                            }
                        }
                    }
                }
                if (assignment_expression_text.has_value() &&
                    is_memory_variable_reference_text(assignment_identifier))
                {
                    const std::string source_identifier = trim_copy(*assignment_expression_text);
                    if (is_memory_variable_reference_text(source_identifier) &&
                        assign_array_copy(assignment_identifier, source_identifier, frame))
                    {
                        return {};
                    }
                }
                if (assign_array_element(assignment_identifier, frame, assignment_value))
                {
                    return {};
                }
                if (assignment_identifier.find('.') != std::string::npos)
                {
                    const auto separator = assignment_identifier.find('.');
                    const std::string object_part = assignment_identifier.substr(0U, separator);
                    // VFP uses m. as a memory-variable namespace prefix (not an OLE object).
                    if (normalize_identifier(object_part) == "m")
                    {
                        assign_variable(frame, assignment_identifier, assignment_value);
                        return {};
                    }
                    const std::string member_path = assignment_identifier.substr(separator + 1U);
                    const std::string normalized_object_part = normalize_identifier(object_part);
                    if (normalized_object_part == "_screen" || normalized_object_part == "_vfp")
                    {
                        const std::string normalized_member_path = normalize_identifier(member_path);
                        if (normalized_member_path == "caption")
                        {
                            representative_application_caption = value_as_string(assignment_value);
                            events.push_back({.category = "ole.set",
                                              .detail = object_part + ".Caption = " + representative_application_caption,
                                              .location = statement.location});
                            return {};
                        }
                        if (normalized_member_path == "windowstate")
                        {
                            representative_application_window_state =
                                static_cast<int>(std::llround(value_as_number(assignment_value)));
                            events.push_back({.category = "ole.set",
                                              .detail = object_part + ".WindowState = " +
                                                            std::to_string(representative_application_window_state),
                                              .location = statement.location});
                            return {};
                        }
                    }
                    const PrgValue object_value = lookup_variable(frame, object_part);
                    auto object = resolve_ole_object(object_value);
                    if (!object.has_value())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.OleObjectNotFoundForPropertyAssignment",
                            {{"targetIdentifier", assignment_identifier}});
                        record_ole_aerror_context(assignment_identifier,
                                                  "Copperfin OLE",
                                                  object_part,
                                                  statement.text,
                                                  1429);
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto resolved_path = resolve_runtime_object_member_path(frame, object_part, member_path);
                    RuntimeOleObjectState *runtime_object =
                        resolved_path.runtime_object == nullptr ? *object : resolved_path.runtime_object;
                    const std::string effective_member_path =
                        resolved_path.remaining_member_path.empty()
                            ? member_path
                            : resolved_path.remaining_member_path;
                    const std::string property_name = normalize_identifier(effective_member_path);
                    if (!property_name.empty())
                    {
                        if (write_native_property_if_present(
                                *runtime_object,
                                property_name,
                                assignment_value,
                                frame,
                                assignment_expression_text))
                        {
                            runtime_object->last_action = effective_member_path + " = " + value_as_string(assignment_value);
                            ++runtime_object->action_count;
                            events.push_back({.category = "ole.set",
                                              .detail = runtime_object->prog_id + "." + runtime_object->last_action,
                                              .location = statement.location});
                            return {};
                        }
                    }
                    runtime_object->last_action = effective_member_path + " = " + value_as_string(assignment_value);
                    ++runtime_object->action_count;
                    events.push_back({.category = "ole.set",
                                      .detail = runtime_object->prog_id + "." + runtime_object->last_action,
                                      .location = statement.location});
                }
                else
                {
                    assign_variable(frame, assignment_identifier, assignment_value);
                }
                return {};
            };
            if (resumed_assignment_value.has_value() && resumed_assignment_statement.has_value())
            {
                return assign_runtime_target_value(
                    resumed_assignment_statement->identifier,
                    *resumed_assignment_value,
                    trim_copy(resumed_assignment_statement->expression));
            }
            if (resumed_store_value.has_value() && resumed_store_statement.has_value())
            {
                for (const auto &name : resumed_store_statement->names)
                {
                    ExecutionOutcome outcome = assign_runtime_target_value(
                        trim_copy(name),
                        *resumed_store_value);
                    if (!outcome.ok)
                    {
                        return outcome;
                    }
                }
                return {};
            }
            auto resolve_command_array_name = [&](const std::string &raw_name, const std::string &command_name) -> std::optional<std::string>
            {
                const std::string candidate = trim_copy(raw_name);
                if (candidate.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.ArrayNameRequired",
                        {{"command", command_name}});
                    return std::nullopt;
                }
                if (is_bare_identifier_text(candidate))
                {
                    return candidate;
                }

                if (!frame.command_array_name_continuation.has_value())
                {
                    Statement array_name_statement = statement;
                    array_name_statement.expression = candidate;
                    frame.command_array_name_continuation = CommandArrayNameContinuation{
                        .statement = std::move(array_name_statement)};
                }
                const auto evaluated = resumed_command_array_name_value.has_value()
                                           ? resumed_command_array_name_value
                                           : evaluate_resumable_expression(
                                                 frame,
                                                 frame.command_array_name_continuation->statement);
                if (!evaluated.has_value())
                {
                    return std::nullopt;
                }
                const std::string evaluated_name = trim_copy(value_as_string(*evaluated));
                resumed_command_array_name_value.reset();
                frame.command_array_name_continuation.reset();
                if (is_bare_identifier_text(evaluated_name))
                {
                    return evaluated_name;
                }

                last_error_message = runtime_text(
                    "Runtime.Prg.Dispatch.Error.InvalidArrayName",
                    {{"command", command_name}});
                return std::nullopt;
            };
            auto parse_command_object_target_path = [&](const std::string &raw_name, const std::string &command_name)
                -> std::optional<std::vector<std::string>>
            {
                auto expand_object_target_identifier_chain = [&](std::string text)
                {
                    text = trim_copy(std::move(text));
                    if (text.empty())
                    {
                        return text;
                    }

                    constexpr std::size_t max_macro_object_target_depth = 16U;
                    std::vector<std::string> visited_identifiers;
                    visited_identifiers.reserve(8U);
                    for (std::size_t depth = 0U; depth < max_macro_object_target_depth; ++depth)
                    {
                        if (!is_bare_identifier_text(text))
                        {
                            break;
                        }

                        const std::string normalized_identifier = normalize_memory_variable_identifier(text);
                        if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized_identifier) != visited_identifiers.end())
                        {
                            break;
                        }
                        visited_identifiers.push_back(normalized_identifier);

                        const PrgValue expanded_value = lookup_variable(frame, text);
                        if (expanded_value.kind != PrgValueKind::string)
                        {
                            break;
                        }

                        const std::string next = trim_copy(value_as_string(expanded_value));
                        if (next.empty() || next == text || !is_bare_identifier_text(next))
                        {
                            break;
                        }

                        text = next;
                    }

                    return text;
                };

                std::string candidate = trim_copy(raw_name);
                if (candidate.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.ObjectTargetRequired",
                        {{"command", command_name}});
                    return std::nullopt;
                }
                if (!candidate.empty() && candidate.front() == '&')
                {
                    std::string macro_expression = trim_copy(candidate.substr(1U));
                    std::string dot_suffix;
                    const std::size_t dot = macro_expression.find('.');
                    if (dot != std::string::npos)
                    {
                        dot_suffix = trim_copy(macro_expression.substr(dot + 1U));
                        macro_expression = trim_copy(macro_expression.substr(0U, dot));
                    }

                    if (is_bare_identifier_text(macro_expression))
                    {
                        const std::string expanded_base = expand_object_target_identifier_chain(macro_expression);
                        candidate = trim_copy(expanded_base + (dot_suffix.empty() ? std::string{} : "." + dot_suffix));
                    }
                    else
                    {
                        const PrgValue evaluated = evaluate_expression(candidate, frame);
                        candidate = trim_copy(value_as_string(evaluated));
                    }
                }
                if (candidate.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.InvalidObjectTarget",
                        {{"command", command_name}});
                    return std::nullopt;
                }

                std::vector<std::string> segments;
                std::size_t start = 0U;
                while (start <= candidate.size())
                {
                    const std::size_t dot = candidate.find('.', start);
                    std::string segment = trim_copy(candidate.substr(start, dot == std::string::npos ? std::string::npos : dot - start));
                    if (!segment.empty() && segment.front() == '&')
                    {
                        const std::string expanded = trim_copy(value_as_string(evaluate_expression(segment, frame)));
                        if (expanded.empty())
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.InvalidObjectTarget",
                                {{"command", command_name}});
                            return std::nullopt;
                        }
                        if (expanded.find('.') != std::string::npos)
                        {
                            std::size_t nested_start = 0U;
                            while (nested_start <= expanded.size())
                            {
                                const std::size_t nested_dot = expanded.find('.', nested_start);
                                const std::string nested_segment = trim_copy(expanded.substr(
                                    nested_start,
                                    nested_dot == std::string::npos ? std::string::npos : nested_dot - nested_start));
                                const std::string expanded_nested_segment =
                                    expand_object_target_identifier_chain(nested_segment);
                                if (!is_bare_identifier_text(expanded_nested_segment))
                                {
                                    last_error_message = runtime_text(
                                        "Runtime.Prg.Dispatch.Error.InvalidObjectTarget",
                                        {{"command", command_name}});
                                    return std::nullopt;
                                }
                                segments.push_back(expanded_nested_segment);
                                if (nested_dot == std::string::npos)
                                {
                                    break;
                                }
                                nested_start = nested_dot + 1U;
                            }
                            if (dot == std::string::npos)
                            {
                                break;
                            }
                            start = dot + 1U;
                            continue;
                        }
                        segment = expand_object_target_identifier_chain(expanded);
                    }
                    if (!is_bare_identifier_text(segment))
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.InvalidObjectTarget",
                            {{"command", command_name}});
                        return std::nullopt;
                    }
                    segments.push_back(segment);
                    if (dot == std::string::npos)
                    {
                        break;
                    }
                    start = dot + 1U;
                }
                if (segments.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.InvalidObjectTarget",
                        {{"command", command_name}});
                    return std::nullopt;
                }
                return segments;
            };
            auto resolve_existing_object_target = [&](const std::vector<std::string> &segments) -> RuntimeOleObjectState *
            {
                if (segments.empty())
                {
                    return nullptr;
                }
                const PrgValue object_value = lookup_variable(frame, segments.front());
                const auto resolved_object = resolve_ole_object(object_value);
                if (!resolved_object.has_value())
                {
                    return nullptr;
                }

                RuntimeOleObjectState *current_object = *resolved_object;
                for (std::size_t index = 1U; index < segments.size(); ++index)
                {
                    const auto property = current_object->properties.find(normalize_identifier(segments[index]));
                    if (property == current_object->properties.end())
                    {
                        return nullptr;
                    }
                    const auto nested_object = resolve_ole_object(property->second);
                    if (!nested_object.has_value())
                    {
                        return nullptr;
                    }
                    current_object = *nested_object;
                }
                return current_object;
            };
            auto make_runtime_object_reference = [&](RuntimeOleObjectState *object_state) -> PrgValue
            {
                return make_string_value("object:" + object_state->prog_id + "#" + std::to_string(object_state->handle));
            };
            auto create_empty_runtime_object = [&](const std::string &source_tag) -> RuntimeOleObjectState *
            {
                const int handle = register_ole_object("Empty", source_tag);
                const auto object_it = ole_objects.find(handle);
                if (object_it == ole_objects.end())
                {
                    return nullptr;
                }
                return &object_it->second;
            };
            auto ensure_object_parent_path = [&](const std::vector<std::string> &segments, const std::string &source_tag) -> RuntimeOleObjectState *
            {
                if (segments.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed");
                    return nullptr;
                }

                RuntimeOleObjectState *current_object = nullptr;
                const PrgValue root_value = lookup_variable(frame, segments.front());
                const auto resolved_root = resolve_ole_object(root_value);
                if (resolved_root.has_value())
                {
                    current_object = *resolved_root;
                }
                else
                {
                    current_object = create_empty_runtime_object(source_tag);
                    if (current_object == nullptr)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject");
                        return nullptr;
                    }
                    assign_variable(frame, segments.front(), make_runtime_object_reference(current_object));
                }

                for (std::size_t index = 1U; index < segments.size(); ++index)
                {
                    const std::string property_name = normalize_identifier(segments[index]);
                    RuntimeOleObjectState *next_object = nullptr;
                    const auto existing_property = current_object->properties.find(property_name);
                    if (existing_property != current_object->properties.end())
                    {
                        const auto resolved_child = resolve_ole_object(existing_property->second);
                        if (resolved_child.has_value())
                        {
                            next_object = *resolved_child;
                        }
                    }
                    if (next_object == nullptr)
                    {
                        next_object = create_empty_runtime_object(source_tag + " nested");
                        if (next_object == nullptr)
                        {
                            last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject");
                            return nullptr;
                        }
                        current_object->properties[property_name] = make_runtime_object_reference(next_object);
                    }
                    current_object = next_object;
                }

                return current_object;
            };
            auto assign_object_target_reference = [&](const std::vector<std::string> &segments, const PrgValue &object_reference, const std::string &source_tag) -> bool
            {
                if (segments.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed");
                    return false;
                }
                if (segments.size() == 1U)
                {
                    assign_variable(frame, segments.front(), object_reference);
                    return true;
                }

                std::vector<std::string> parent_segments(segments.begin(), segments.end() - 1U);
                RuntimeOleObjectState *parent_object = ensure_object_parent_path(parent_segments, source_tag);
                if (parent_object == nullptr)
                {
                    return false;
                }
                parent_object->properties[normalize_identifier(segments.back())] = object_reference;
                return true;
            };
            auto ensure_object_target = [&](const std::vector<std::string> &segments, const std::string &source_tag) -> RuntimeOleObjectState *
            {
                return ensure_object_parent_path(segments, source_tag);
            };
            auto try_invoke_bare_native_member_expression = [&](const std::string &expression) -> bool
            {
                const std::string trimmed_expression = trim_copy(expression);
                if (trimmed_expression.empty() || trimmed_expression.find('.') == std::string::npos)
                {
                    return false;
                }

                if (!std::all_of(trimmed_expression.begin(), trimmed_expression.end(), [](unsigned char ch)
                    {
                        return std::isalnum(ch) != 0 || ch == '_' || ch == '.';
                    }))
                {
                    return false;
                }

                const std::size_t separator = trimmed_expression.find('.');
                if (separator == std::string::npos || separator == 0U || separator + 1U >= trimmed_expression.size())
                {
                    return false;
                }

                const std::string base_name = trim_copy(trimmed_expression.substr(0U, separator));
                const std::string member_path = trim_copy(trimmed_expression.substr(separator + 1U));
                if (!is_bare_identifier_text(base_name) || member_path.empty())
                {
                    return false;
                }

                const auto resolved_path = resolve_runtime_object_member_path(frame, base_name, member_path);
                if (resolved_path.runtime_object == nullptr)
                {
                    return false;
                }

                const std::string effective_member_path =
                    resolved_path.remaining_member_path.empty()
                        ? member_path
                        : resolved_path.remaining_member_path;
                const std::size_t leaf_separator = effective_member_path.rfind('.');
                const std::string leaf_member_name =
                    leaf_separator == std::string::npos
                        ? effective_member_path
                        : effective_member_path.substr(leaf_separator + 1U);
                const std::string normalized_leaf = normalize_identifier(leaf_member_name);
                if (normalized_leaf.empty())
                {
                    return false;
                }

                if (!runtime_object_member_matches(resolved_path.runtime_object->methods, normalized_leaf) &&
                    !is_builtin_native_noarg_method_name(*resolved_path.runtime_object, normalized_leaf))
                {
                    return false;
                }

                Statement member_invocation = statement;
                member_invocation.expression = trimmed_expression + "()";
                (void)evaluate_resumable_expression(frame, member_invocation);
                return true;
            };

            const auto evaluate_command_target = [&](const Statement &command_statement)
                -> std::optional<std::string>
            {
                std::string target = trim_copy(command_statement.identifier);
                if (target.empty() || target.front() != '&')
                {
                    return target;
                }

                if (!frame.command_target_continuation.has_value())
                {
                    Statement target_statement = command_statement;
                    target_statement.expression = target;
                    frame.command_target_continuation = CommandTargetContinuation{
                        .statement = std::move(target_statement)};
                }

                const auto target_value = resumed_command_target_value.has_value()
                                              ? resumed_command_target_value
                                              : evaluate_resumable_expression(
                                                    frame,
                                                    frame.command_target_continuation->statement);
                if (!target_value.has_value())
                {
                    return std::nullopt;
                }

                const std::string expanded_target = trim_copy(value_as_string(*target_value));
                resumed_command_target_value.reset();
                frame.command_target_continuation.reset();
                if (!expanded_target.empty())
                {
                    target = expanded_target;
                }
                return target;
            };

            switch (statement.kind)
            {
            case StatementKind::assignment:
            {
                const auto assignment_value = evaluate_resumable_expression(frame, statement);
                if (!assignment_value.has_value())
                {
                    return {};
                }
                return assign_runtime_target_value(
                    statement.identifier,
                    *assignment_value,
                    trim_copy(statement.expression));
            }
            case StatementKind::expression:
                if (!statement.expression.empty())
                {
                    if (starts_with_insensitive(statement.expression, "WAIT WINDOW "))
                    {
                        Statement wait_window_statement = statement;
                        wait_window_statement.identifier = "wait_window";
                        wait_window_statement.expression = trim_copy(statement.expression.substr(12U));
                        const auto value = evaluate_resumable_expression(frame, wait_window_statement);
                        if (!value.has_value())
                        {
                            return {};
                        }
                        emit_wait_window_event(*value, statement.location);
                    }
                    else
                    {
                        if (!try_invoke_bare_native_member_expression(statement.expression))
                        {
                            const auto value = evaluate_resumable_expression(frame, statement);
                            if (!value.has_value())
                            {
                                return {};
                            }
                        }
                    }
                }
                return {};
            case StatementKind::do_command:
            {
                if (!frame.command_argument_continuation.has_value())
                {
                    const auto target_value = evaluate_command_target(statement);
                    if (!target_value.has_value())
                    {
                        return {};
                    }
                    std::string target = *target_value;
                    frame.command_argument_continuation = CommandArgumentContinuation{
                        .statement = statement,
                        .target = std::move(target),
                        .argument_expressions = split_csv_like(statement.expression),
                        .next_argument_index = 0U,
                        .values = {},
                        .references = {}};
                }

                CommandArgumentContinuation &argument_continuation =
                    *frame.command_argument_continuation;
                while (argument_continuation.next_argument_index <
                       argument_continuation.argument_expressions.size())
                {
                    const std::string argument_expression = trim_copy(
                        argument_continuation.argument_expressions[argument_continuation.next_argument_index]);
                    if (argument_expression.empty())
                    {
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    if (resumed_do_argument_value.has_value())
                    {
                        argument_continuation.values.push_back(*resumed_do_argument_value);
                        argument_continuation.references.push_back(std::nullopt);
                        resumed_do_argument_value.reset();
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    if (argument_expression.front() == '@')
                    {
                        const std::string reference_name = trim_copy(argument_expression.substr(1U));
                        if (is_memory_variable_reference_text(reference_name))
                        {
                            argument_continuation.values.push_back(lookup_variable(frame, reference_name));
                            argument_continuation.references.push_back(reference_name);
                            ++argument_continuation.next_argument_index;
                            continue;
                        }
                    }
                    if (is_memory_variable_reference_text(argument_expression))
                    {
                        argument_continuation.values.push_back(lookup_variable(frame, argument_expression));
                        argument_continuation.references.push_back(argument_expression);
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    Statement argument_statement = argument_continuation.statement;
                    argument_statement.expression = argument_expression;
                    const auto argument_value = evaluate_resumable_expression(frame, argument_statement);
                    if (!argument_value.has_value())
                    {
                        return {};
                    }
                    argument_continuation.values.push_back(*argument_value);
                    argument_continuation.references.push_back(std::nullopt);
                    ++argument_continuation.next_argument_index;
                }

                std::string target = std::move(argument_continuation.target);
                std::vector<PrgValue> call_arguments = std::move(argument_continuation.values);
                std::vector<std::optional<std::string>> call_argument_references =
                    std::move(argument_continuation.references);
                frame.command_argument_continuation.reset();
                frame.command_target_continuation.reset();
                frame.command_array_name_continuation.reset();
                frame.text_merge_continuation.reset();
                frame.parameter_default_continuation.reset();
                frame.use_command_continuation.reset();
                frame.copy_file_continuation.reset();
                frame.rename_file_continuation.reset();
                Program &program = load_program(frame.file_path);
                if (const auto routine = find_unqualified_routine_lookup(program.path, target); routine.has_value())
                {
                    if (!can_push_frame())
                    {
                        last_error_message = call_depth_limit_message();
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    push_routine_frame(
                        routine->program->path,
                        *routine->routine,
                        std::move(call_arguments),
                        std::move(call_argument_references));
                    return {};
                }

                std::filesystem::path target_path = copperfin::platform::path_from_utf8_string(target);
                if (target_path.extension().empty())
                {
                    target_path += ".prg";
                }
                if (target_path.is_relative())
                {
                    target_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                        target_path;
                }
                std::error_code target_exists_error;
                if (!std::filesystem::exists(target_path, target_exists_error) || target_exists_error)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed",
                        {
                            {"command", "DO"},
                            {"target", target}
                        });
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!can_push_frame())
                {
                    last_error_message = call_depth_limit_message();
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                push_main_frame(
                    copperfin::platform::path_to_utf8_string(target_path),
                    std::move(call_arguments),
                    std::move(call_argument_references));
                return {};
            }
            case StatementKind::enter_critical_command:
            {
                const std::string section_name = trim_copy(statement.identifier);
                if (!enter_critical_section(section_name, statement.location))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::string detail = "section=" + normalize_identifier(section_name.empty() ? std::string{"default"} : section_name);
                detail += " depth=" + std::to_string(critical_section_depth_by_name[normalize_identifier(section_name.empty() ? std::string{"default"} : section_name)]);
                events.push_back({.category = "runtime.critical.enter",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::exit_critical_command:
            {
                const std::string section_name = trim_copy(statement.identifier);
                if (!exit_critical_section(section_name, statement.location))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::string detail = "section=" + normalize_identifier(section_name.empty() ? std::string{"default"} : section_name);
                events.push_back({.category = "runtime.critical.exit",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::yield_statement:
            {
                if (!trim_copy(statement.expression).empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandDoesNotTakeArguments",
                        {{"command", "YIELD"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::this_thread::yield();
                events.push_back({.category = "runtime.yield",
                                  .detail = "operation=YIELD cooperative",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::spawn_command:
            {
                if (!frame.command_argument_continuation.has_value())
                {
                    const auto target_value = evaluate_command_target(statement);
                    if (!target_value.has_value())
                    {
                        return {};
                    }
                    std::string target = *target_value;
                    if (target.empty())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.SpawnRequiresTarget",
                            {{"command", "SPAWN"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    frame.command_argument_continuation = CommandArgumentContinuation{
                        .statement = statement,
                        .target = std::move(target),
                        .argument_expressions = split_csv_like(statement.expression),
                        .next_argument_index = 0U,
                        .values = {},
                        .references = {}};
                }

                CommandArgumentContinuation &argument_continuation =
                    *frame.command_argument_continuation;
                while (argument_continuation.next_argument_index <
                       argument_continuation.argument_expressions.size())
                {
                    const std::string argument_expression = trim_copy(
                        argument_continuation.argument_expressions[argument_continuation.next_argument_index]);
                    if (argument_expression.empty())
                    {
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    if (resumed_spawn_argument_value.has_value())
                    {
                        argument_continuation.values.push_back(*resumed_spawn_argument_value);
                        argument_continuation.references.push_back(std::nullopt);
                        resumed_spawn_argument_value.reset();
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    if (argument_expression.front() == '@')
                    {
                        const std::string reference_name = trim_copy(argument_expression.substr(1U));
                        if (is_memory_variable_reference_text(reference_name))
                        {
                            argument_continuation.values.push_back(lookup_variable(frame, reference_name));
                            argument_continuation.references.push_back(reference_name);
                            ++argument_continuation.next_argument_index;
                            continue;
                        }
                    }

                    Statement argument_statement = argument_continuation.statement;
                    argument_statement.expression = argument_expression;
                    const auto argument_value = evaluate_resumable_expression(frame, argument_statement);
                    if (!argument_value.has_value())
                    {
                        return {};
                    }
                    argument_continuation.values.push_back(*argument_value);
                    argument_continuation.references.push_back(std::nullopt);
                    ++argument_continuation.next_argument_index;
                }

                std::string target = std::move(argument_continuation.target);
                std::vector<PrgValue> call_arguments = std::move(argument_continuation.values);
                std::vector<std::optional<std::string>> call_argument_references =
                    std::move(argument_continuation.references);
                frame.command_argument_continuation.reset();
                frame.command_target_continuation.reset();
                frame.command_array_name_continuation.reset();
                frame.text_merge_continuation.reset();
                frame.parameter_default_continuation.reset();
                frame.use_command_continuation.reset();
                frame.copy_file_continuation.reset();
                frame.rename_file_continuation.reset();

                Program &program = load_program(frame.file_path);
                std::shared_ptr<Impl> child = std::make_shared<Impl>(*this);
                child->stack.clear();
                child->breakpoints.clear();
                child->resume_skip_breakpoint_location.reset();
                child->events.clear();
                child->last_error_message.clear();
                child->last_fault_location = {};
                child->last_fault_statement.clear();
                child->last_error_code = 0;
                child->last_error_work_area = 0;
                child->last_error_procedure.clear();
                child->last_error_compatibility = {};
                child->error_metadata_stack.clear();
                child->handling_error = false;
                child->handling_shutdown = false;
                child->entry_pause_pending = false;
                child->waiting_for_events = false;
                child->quit_pending_after_shutdown = false;
                child->fault_frame_file_path.clear();
                child->fault_frame_routine_name.clear();
                child->fault_statement_index = 0U;
                child->fault_pc_valid = false;
                child->event_dispatch_return_depth.reset();
                child->restore_event_loop_after_dispatch = false;
                child->task_cancel_requested = std::make_shared<std::atomic<bool>>(false);
                child->critical_section_stack.clear();
                child->critical_section_depth_by_name.clear();
                child->critical_section_mutexes_by_name.clear();
                child->transaction_level_by_session.clear();
                child->transaction_journal_by_session.clear();
                child->command_undo_journal_by_session.clear();
                child->command_undo_stack_by_session.clear();
                static std::atomic<std::uint64_t> spawned_runtime_instance_counter{1000000ULL};
                child->runtime_instance_id = spawned_runtime_instance_counter.fetch_add(1ULL, std::memory_order_relaxed);
                child->current_data_session = current_data_session;
                std::string task_source_path = program.path;
                if (const auto routine = find_unqualified_routine_lookup(program.path, target); routine.has_value())
                {
                    if (!can_push_frame())
                    {
                        last_error_message = call_depth_limit_message();
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    child->push_routine_frame(
                        routine->program->path,
                        *routine->routine,
                        std::move(call_arguments),
                        std::move(call_argument_references));
                }
                else
                {
                    std::filesystem::path target_path = copperfin::platform::path_from_utf8_string(target);
                    if (target_path.extension().empty())
                    {
                        target_path += ".prg";
                    }
                    if (target_path.is_relative())
                    {
                        target_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                            target_path;
                    }
                    std::error_code target_exists_error;
                    if (!std::filesystem::exists(target_path, target_exists_error) || target_exists_error)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed",
                            {
                                {"command", "SPAWN"},
                                {"target", target}
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    task_source_path = copperfin::platform::path_to_utf8_string(target_path);
                    child->push_main_frame(
                        copperfin::platform::path_to_utf8_string(target_path),
                        std::move(call_arguments),
                        std::move(call_argument_references));
                }

                const int handle = allocate_async_task_handle();
                auto task = std::make_shared<AsyncTaskState>();
                task->handle = handle;
                task->routine_name = target;
                task->source_path = task_source_path;
                task->cancel_requested = child->task_cancel_requested;
                task->future = std::async(std::launch::async, [child]() mutable
                {
                    return child->run(DebugResumeAction::continue_run);
                }).share();
                register_async_task(task);

                std::string detail = "handle=" + std::to_string(handle) + " target=" + target;
                if (!statement.expression.empty())
                {
                    detail += " args=" + std::to_string(split_csv_like(statement.expression).size());
                }
                if (!statement.names.empty() && !statement.names.front().empty())
                {
                    ExecutionOutcome outcome = assign_runtime_target_value(statement.names.front(), make_number_value(static_cast<double>(handle)));
                    if (!outcome.ok)
                    {
                        return outcome;
                    }
                    detail += " assigned=" + statement.names.front();
                }
                events.push_back({.category = "runtime.task.spawn",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::await_command:
            {
                const std::string handle_text = trim_copy(statement.expression);
                if (handle_text.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.AwaitRequiresTaskHandle",
                        {{"command", "AWAIT"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!ensure_non_blocking_critical_section_policy("AWAIT", statement.location))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto handle_value = resumed_await_handle_value.has_value()
                                              ? resumed_await_handle_value
                                              : evaluate_resumable_expression(frame, statement);
                if (!handle_value.has_value())
                {
                    return {};
                }
                const int handle = static_cast<int>(std::llround(value_as_number(*handle_value)));
                const std::shared_ptr<AsyncTaskState> task = find_async_task(handle);
                if (task == nullptr)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.UnknownTaskHandle",
                        {{"handle", std::to_string(handle)}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                task->future.wait();
                task->result = task->future.get();
                task->finished = true;

                if (!task->result.events.empty())
                {
                    events.insert(events.end(), task->result.events.begin(), task->result.events.end());
                }

                std::string detail = "handle=" + std::to_string(handle) +
                                     " state=" + debug_pause_reason_name(task->result.reason) +
                                     " message=" + task->result.message;
                if (!statement.names.empty() && !statement.names.front().empty())
                {
                    const bool completed = task->result.reason == DebugPauseReason::completed;
                    ExecutionOutcome outcome = assign_runtime_target_value(
                        statement.names.front(),
                        make_boolean_value(completed));
                    if (!outcome.ok)
                    {
                        return outcome;
                    }
                    detail += " assigned=" + statement.names.front();
                }

                if (task->result.reason != DebugPauseReason::completed && !task->result.message.empty())
                {
                    detail += " error=" + task->result.message;
                }

                events.push_back({.category = "runtime.task.await",
                                  .detail = detail,
                                  .location = statement.location});
                erase_async_task(handle);
                return {};
            }
            case StatementKind::call_command:
            {
                if (!frame.command_argument_continuation.has_value())
                {
                    const auto target_value = evaluate_command_target(statement);
                    if (!target_value.has_value())
                    {
                        return {};
                    }
                    std::string target = *target_value;
                    frame.command_argument_continuation = CommandArgumentContinuation{
                        .statement = statement,
                        .target = std::move(target),
                        .argument_expressions = split_csv_like(statement.expression),
                        .next_argument_index = 0U,
                        .values = {},
                        .references = {}};
                }

                CommandArgumentContinuation &argument_continuation =
                    *frame.command_argument_continuation;
                while (argument_continuation.next_argument_index <
                       argument_continuation.argument_expressions.size())
                {
                    const std::string argument_expression = trim_copy(
                        argument_continuation.argument_expressions[argument_continuation.next_argument_index]);
                    if (argument_expression.empty())
                    {
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    if (resumed_call_argument_value.has_value())
                    {
                        argument_continuation.values.push_back(*resumed_call_argument_value);
                        argument_continuation.references.push_back(std::nullopt);
                        resumed_call_argument_value.reset();
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    if (argument_expression.front() == '@')
                    {
                        const std::string reference_name = trim_copy(argument_expression.substr(1U));
                        if (is_memory_variable_reference_text(reference_name))
                        {
                            argument_continuation.values.push_back(lookup_variable(frame, reference_name));
                            argument_continuation.references.push_back(reference_name);
                            ++argument_continuation.next_argument_index;
                            continue;
                        }
                    }
                    if (is_memory_variable_reference_text(argument_expression))
                    {
                        argument_continuation.values.push_back(lookup_variable(frame, argument_expression));
                        argument_continuation.references.push_back(argument_expression);
                        ++argument_continuation.next_argument_index;
                        continue;
                    }

                    Statement argument_statement = argument_continuation.statement;
                    argument_statement.expression = argument_expression;
                    const auto argument_value = evaluate_resumable_expression(frame, argument_statement);
                    if (!argument_value.has_value())
                    {
                        return {};
                    }
                    argument_continuation.values.push_back(*argument_value);
                    argument_continuation.references.push_back(std::nullopt);
                    ++argument_continuation.next_argument_index;
                }

                std::string target = std::move(argument_continuation.target);
                std::vector<PrgValue> call_arguments = std::move(argument_continuation.values);
                std::vector<std::optional<std::string>> call_argument_references =
                    std::move(argument_continuation.references);
                frame.command_argument_continuation.reset();
                frame.command_target_continuation.reset();
                frame.command_array_name_continuation.reset();
                frame.text_merge_continuation.reset();
                frame.parameter_default_continuation.reset();
                frame.use_command_continuation.reset();
                frame.copy_file_continuation.reset();
                frame.rename_file_continuation.reset();

                Program &program = load_program(frame.file_path);
                    if (const auto routine = find_unqualified_routine_lookup(program.path, target); routine.has_value())
                    {
                        if (!can_push_frame())
                        {
                            last_error_message = call_depth_limit_message();
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        push_routine_frame(
                            routine->program->path,
                            *routine->routine,
                            std::move(call_arguments),
                            std::move(call_argument_references));
                        return {};
                    }

                    std::filesystem::path target_path = copperfin::platform::path_from_utf8_string(target);
                    if (target_path.extension().empty())
                    {
                        target_path += ".prg";
                    }
                    if (target_path.is_relative())
                    {
                        target_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                            target_path;
                    }
                    std::error_code target_exists_error;
                    if (std::filesystem::exists(target_path, target_exists_error) && !target_exists_error)
                    {
                        if (!can_push_frame())
                        {
                            last_error_message = call_depth_limit_message();
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        push_main_frame(
                            copperfin::platform::path_to_utf8_string(target_path),
                            std::move(call_arguments),
                            std::move(call_argument_references));
                        return {};
                    }

                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed",
                        {
                            {"command", "CALL"},
                            {"target", target}
                        });
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
            case StatementKind::do_form:
            {
                const std::filesystem::path form_path = resolve_asset_path(statement.identifier, ".scx");
                events.push_back({.category = "form.open",
                                  .detail = copperfin::platform::path_to_utf8_string(form_path.lexically_normal()),
                                  .location = statement.location});
                std::error_code form_exists_error;
                if (std::filesystem::exists(form_path, form_exists_error) && !form_exists_error)
                {
                    if (const auto bootstrap_path = materialize_xasset_bootstrap(
                            copperfin::platform::path_to_utf8_string(form_path), true))
                    {
                        if (!can_push_frame())
                        {
                            last_error_message = call_depth_limit_message();
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        push_main_frame(*bootstrap_path);
                    }
                    else if (!last_error_message.empty())
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                return {};
            }
            case StatementKind::calculate_command:
            {
                std::string error_message;
                if (!execute_calculate_command(statement, frame, error_message))
                {
                    last_error_message = error_message;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                CursorState *cursor = resolve_cursor_target_expression(statement.tertiary_expression, frame);
                if (cursor == nullptr)
                {
                    cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                }

                std::string detail = "assignments=" + trim_copy(statement.expression);
                append_cursor_view_metadata(cursor, {}, detail);
                append_runtime_cursor_target_detail(statement.tertiary_expression, frame, detail);
                if (!statement.secondary_expression.empty())
                {
                    detail += " for=" + trim_copy(statement.secondary_expression);
                }
                if (!statement.quaternary_expression.empty())
                {
                    detail += " while=" + trim_copy(statement.quaternary_expression);
                }

                events.push_back({.category = "runtime.calculate",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::count_command:
            case StatementKind::sum_command:
            case StatementKind::average_command:
            {
                std::string function = "count";
                std::string category = "runtime.count";
                if (statement.kind == StatementKind::sum_command)
                {
                    function = "sum";
                    category = "runtime.sum";
                }
                else if (statement.kind == StatementKind::average_command)
                {
                    function = "average";
                    category = "runtime.average";
                }

                std::string error_message;
                if (!execute_command_aggregate(statement, frame, function, error_message))
                {
                    last_error_message = error_message;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                CursorState *cursor = resolve_cursor_target_expression(statement.quaternary_expression, frame);
                if (cursor == nullptr)
                {
                    cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                }
                std::string expression_text;
                const AggregateScopeClause scope = parse_aggregate_scope_clause(statement.expression, expression_text);
                std::string detail = "function=" + function;
                append_cursor_view_metadata(cursor, {}, detail);
                append_aggregate_scope_metadata(scope, detail);
                if (!expression_text.empty())
                {
                    detail += " expr=" + trim_copy(expression_text);
                }
                if (!statement.secondary_expression.empty())
                {
                    detail += " for=" + trim_copy(statement.secondary_expression);
                }
                if (!statement.tertiary_expression.empty())
                {
                    detail += " while=" + trim_copy(statement.tertiary_expression);
                }
                if (!statement.identifier.empty())
                {
                    detail += " into=" + trim_copy(statement.identifier);
                    const std::string resolved_target = resolve_runtime_target_identifier(statement.identifier, frame);
                    if (!resolved_target.empty() && resolved_target != trim_copy(statement.identifier))
                    {
                        detail += " into_resolved=" + resolved_target;
                    }
                }
                append_runtime_cursor_target_detail(statement.quaternary_expression, frame, detail);

                events.push_back({.category = category,
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::text_command:
            {
                if (statement.identifier.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.TextRequiresToVariableInCurrentRuntimeSlice");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::string text_value = statement.expression;
                if (normalize_identifier(statement.tertiary_expression) == "textmerge" || is_set_enabled("textmerge"))
                {
                    const auto [left_delimiter, right_delimiter] = current_textmerge_delimiters();
                    if (!frame.text_merge_continuation.has_value() ||
                        frame.text_merge_continuation->statement.text != statement.text ||
                        frame.text_merge_continuation->source_text != statement.expression ||
                        frame.text_merge_continuation->left_delimiter != left_delimiter ||
                        frame.text_merge_continuation->right_delimiter != right_delimiter)
                    {
                        frame.text_merge_continuation = TextMergeContinuation{
                            .statement = statement,
                            .source_text = statement.expression,
                            .left_delimiter = left_delimiter,
                            .right_delimiter = right_delimiter,
                            .merged_text = {}};
                    }

                    TextMergeContinuation &continuation = *frame.text_merge_continuation;
                    continuation.merged_text.reserve(continuation.source_text.size());
                    while (continuation.cursor < continuation.source_text.size())
                    {
                        if (continuation.pending_expression)
                        {
                            if (!resumed_textmerge_value.has_value())
                            {
                                return {};
                            }
                            continuation.merged_text.append(value_as_string(*resumed_textmerge_value));
                            continuation.cursor = continuation.pending_expression_end;
                            continuation.pending_expression = false;
                            resumed_textmerge_value.reset();
                            continue;
                        }

                        const std::size_t start = continuation.source_text.find(
                            continuation.left_delimiter,
                            continuation.cursor);
                        if (start == std::string::npos)
                        {
                            continuation.merged_text.append(continuation.source_text.substr(continuation.cursor));
                            continuation.cursor = continuation.source_text.size();
                            break;
                        }

                        continuation.merged_text.append(
                            continuation.source_text.substr(continuation.cursor, start - continuation.cursor));
                        const std::size_t end = continuation.source_text.find(
                            continuation.right_delimiter,
                            start + continuation.left_delimiter.size());
                        if (end == std::string::npos)
                        {
                            continuation.merged_text.append(continuation.source_text.substr(start));
                            continuation.cursor = continuation.source_text.size();
                            break;
                        }

                        const std::string merge_expression = trim_copy(continuation.source_text.substr(
                            start + continuation.left_delimiter.size(),
                            end - start - continuation.left_delimiter.size()));
                        continuation.pending_expression_end = end + continuation.right_delimiter.size();
                        continuation.cursor = start;
                        if (merge_expression.empty())
                        {
                            continuation.cursor = continuation.pending_expression_end;
                            continue;
                        }

                        Statement merge_statement = continuation.statement;
                        merge_statement.expression = merge_expression;
                        merge_statement.text = continuation.statement.text + " [textmerge-expression]";
                        continuation.pending_expression = true;
                        const auto merged_value = evaluate_resumable_expression(frame, merge_statement);
                        if (!merged_value.has_value())
                        {
                            return {};
                        }
                        continuation.merged_text.append(value_as_string(*merged_value));
                        continuation.cursor = continuation.pending_expression_end;
                        continuation.pending_expression = false;
                    }

                    text_value = std::move(continuation.merged_text);
                    frame.text_merge_continuation.reset();
                }
                else
                {
                    frame.text_merge_continuation.reset();
                }

                if (normalize_identifier(statement.secondary_expression) == "additive")
                {
                    text_value = value_as_string(lookup_variable(frame, statement.identifier)) + text_value;
                }

                assign_variable(frame, statement.identifier, make_string_value(std::move(text_value)));
                events.push_back({.category = "runtime.text",
                                  .detail = statement.identifier,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::total_command:
            {
                std::string error_message;
                if (!execute_total_command(statement, frame, error_message))
                {
                    last_error_message = error_message;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string detail = trim_copy(statement.text);
                std::string ignored_error;
                if (const auto parsed = parse_total_command_plan(statement.expression, ignored_error))
                {
                    CursorState *cursor = resolve_cursor_target_expression(parsed->in_expression, frame);
                    if (cursor == nullptr)
                    {
                        cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    }

                    detail.clear();
                    detail = "on=" + trim_copy(parsed->on_field_name);
                    if (!parsed->field_names.empty())
                    {
                        detail += " totals=";
                        for (std::size_t index = 0U; index < parsed->field_names.size(); ++index)
                        {
                            if (index > 0U)
                            {
                                detail += ",";
                            }
                            detail += trim_copy(parsed->field_names[index]);
                        }
                    }
                    detail += " output=" + trim_copy(parsed->target_expression);
                    append_cursor_view_metadata(cursor, {}, detail);
                    append_aggregate_scope_metadata(parsed->scope, detail);
                    if (!parsed->for_expression.empty())
                    {
                        detail += " for=" + trim_copy(parsed->for_expression);
                    }
                    if (!parsed->while_expression.empty())
                    {
                        detail += " while=" + trim_copy(parsed->while_expression);
                    }
                    append_runtime_cursor_target_detail(parsed->in_expression, frame, detail);
                }

                events.push_back({.category = "runtime.total",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::report_form:
                return open_report_surface(statement, frame, ".frx", "report");
            case StatementKind::label_form:
                return open_report_surface(statement, frame, ".lbx", "label");
            case StatementKind::define_popup_command:
            {
                const std::string popup_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.identifier)));
                if (!popup_name.empty())
                {
                    current_session_state().popup_bar_prompts[popup_name].clear();
                    current_session_state().popup_bar_skip_states[popup_name].clear();
                    current_session_state().popup_bar_mark_states[popup_name].clear();
                    current_session_state().popup_bar_selection_handlers[popup_name].clear();
                    current_session_state().popup_bar_selection_actions[popup_name].clear();
                    current_session_state().popup_bar_action_routines[popup_name].clear();
                    current_session_state().popup_bar_activation_targets[popup_name].clear();
                    current_session_state().popup_selection_handlers.erase(popup_name);
                }
                return {};
            }
            case StatementKind::define_bar_command:
            {
                const auto bar_number = try_parse_numeric_index_value(statement.secondary_expression);
                const std::string popup_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.identifier)));
                if (!bar_number.has_value() || popup_name.empty())
                {
                    return {};
                }

                const PrgValue prompt_value = evaluate_expression(statement.expression, frame);
                current_session_state().popup_bar_prompts[popup_name][
                    static_cast<long long>(std::llround(*bar_number))] = value_as_string(prompt_value);
                return {};
            }
            case StatementKind::on_bar_activate_popup_command:
            {
                const auto bar_number = try_parse_numeric_index_value(statement.secondary_expression);
                const std::string popup_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.identifier)));
                const std::string submenu_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.expression)));
                if (!bar_number.has_value() || *bar_number < 1.0 ||
                    popup_name.empty() || submenu_name.empty())
                {
                    return {};
                }

                current_session_state().popup_bar_activation_targets[popup_name][
                    static_cast<long long>(std::llround(*bar_number))] = submenu_name;
                return {};
            }
            case StatementKind::on_selection_bar_command:
            {
                const auto bar_number = try_parse_numeric_index_value(statement.secondary_expression);
                const std::string popup_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.identifier)));
                const std::string handler_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.expression)));
                if (!bar_number.has_value() || *bar_number < 1.0 ||
                    popup_name.empty() || handler_name.empty())
                {
                    return {};
                }

                current_session_state().popup_bar_selection_handlers[popup_name][
                    static_cast<long long>(std::llround(*bar_number))] = handler_name;
                return {};
            }
            case StatementKind::on_selection_bar_action_command:
            {
                const auto bar_number = try_parse_numeric_index_value(statement.secondary_expression);
                const std::string popup_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.identifier)));
                const std::string action_text = trim_copy(statement.expression);
                if (!bar_number.has_value() || *bar_number < 1.0 ||
                    popup_name.empty() || action_text.empty() ||
                    action_text.find('&') != std::string::npos)
                {
                    return {};
                }

                current_session_state().popup_bar_selection_actions[popup_name][
                    static_cast<long long>(std::llround(*bar_number))] = action_text;
                current_session_state().popup_bar_action_routines[popup_name].erase(
                    static_cast<long long>(std::llround(*bar_number)));
                return {};
            }
            case StatementKind::on_selection_popup_command:
            {
                const std::string popup_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.identifier)));
                if (popup_name.empty())
                {
                    return {};
                }

                const std::string handler_name = normalize_identifier(
                    unquote_identifier(trim_copy(statement.expression)));
                if (handler_name.empty())
                {
                    current_session_state().popup_selection_handlers.erase(popup_name);
                }
                else
                {
                    current_session_state().popup_selection_handlers[popup_name] = handler_name;
                }
                return {};
            }
            case StatementKind::activate_surface:
                waiting_for_events = true;
                events.push_back({.category = statement.identifier + ".activate",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {.ok = true, .waiting_for_events = true, .frame_returned = false, .message = {}};
            case StatementKind::deactivate_surface:
                waiting_for_events = false;
                events.push_back({.category = statement.identifier + ".deactivate",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            case StatementKind::release_surface:
                if (statement.identifier == "popup")
                {
                    const std::string popup_name = normalize_identifier(
                        unquote_identifier(trim_copy(statement.expression)));
                    current_session_state().popup_bar_prompts.erase(popup_name);
                    current_session_state().popup_bar_skip_states.erase(popup_name);
                    current_session_state().popup_bar_mark_states.erase(popup_name);
                    current_session_state().popup_bar_selection_handlers.erase(popup_name);
                    current_session_state().popup_bar_selection_actions.erase(popup_name);
                    current_session_state().popup_bar_action_routines.erase(popup_name);
                    current_session_state().popup_bar_activation_targets.erase(popup_name);
                    current_session_state().popup_selection_handlers.erase(popup_name);
                }
                waiting_for_events = false;
                events.push_back({.category = statement.identifier + ".release",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            case StatementKind::push_key_command:
            {
                DataSessionState &session_state = current_session_state();
                std::string marker = trim_copy(statement.expression);
                if (marker.empty())
                {
                    marker = trim_copy(statement.identifier);
                }
                session_state.key_stack.push_back(marker);
                events.push_back({.category = "runtime.push_key",
                                  .detail = format_stack_event_detail(session_state.key_stack.size(), marker, false),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::pop_key_command:
            {
                DataSessionState &session_state = current_session_state();
                const std::string requested_marker = trim_copy(statement.expression);
                std::string marker = requested_marker;
                const bool empty = session_state.key_stack.empty();
                if (!empty)
                {
                    marker = session_state.key_stack.back();
                    session_state.key_stack.pop_back();
                }
                events.push_back({.category = "runtime.pop_key",
                                  .detail = format_stack_event_detail(session_state.key_stack.size(), marker, empty),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::push_menu_command:
            {
                DataSessionState &session_state = current_session_state();
                std::string marker = trim_copy(statement.expression);
                if (marker.empty())
                {
                    marker = trim_copy(statement.identifier);
                }
                session_state.menu_stack.push_back(marker);
                events.push_back({.category = "runtime.push_menu",
                                  .detail = format_stack_event_detail(session_state.menu_stack.size(), marker, false),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::pop_menu_command:
            {
                DataSessionState &session_state = current_session_state();
                const std::string requested_marker = trim_copy(statement.expression);
                std::string marker = requested_marker;
                const bool empty = session_state.menu_stack.empty();
                if (!empty)
                {
                    marker = session_state.menu_stack.back();
                    session_state.menu_stack.pop_back();
                }
                events.push_back({.category = "runtime.pop_menu",
                                  .detail = format_stack_event_detail(session_state.menu_stack.size(), marker, empty),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::push_popup_command:
            {
                DataSessionState &session_state = current_session_state();
                std::string marker = trim_copy(statement.expression);
                if (marker.empty())
                {
                    marker = trim_copy(statement.identifier);
                }
                session_state.popup_stack.push_back(marker);
                events.push_back({.category = "runtime.push_popup",
                                  .detail = format_stack_event_detail(session_state.popup_stack.size(), marker, false),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::pop_popup_command:
            {
                DataSessionState &session_state = current_session_state();
                const std::string requested_marker = trim_copy(statement.expression);
                std::string marker = requested_marker;
                const bool empty = session_state.popup_stack.empty();
                if (!empty)
                {
                    marker = session_state.popup_stack.back();
                    session_state.popup_stack.pop_back();
                }
                events.push_back({.category = "runtime.pop_popup",
                                  .detail = format_stack_event_detail(session_state.popup_stack.size(), marker, empty),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::return_statement:
                if (trim_copy(statement.expression).empty())
                {
                    last_return_value = make_empty_value();
                }
                else
                {
                    const auto return_value = evaluate_resumable_expression(frame, statement);
                    if (!return_value.has_value())
                    {
                        return {};
                    }
                    last_return_value = *return_value;
                }
                frame.return_pending = true;
                if (const auto outcome = continue_pending_return(frame); outcome.has_value())
                {
                    return *outcome;
                }
                return {};
            case StatementKind::nodefault_statement:
                frame.requested_nodefault = true;
                return {};
            case StatementKind::do_case_statement:
                frame.cases.push_back({.do_case_statement_index = frame.pc - 1U,
                                       .endcase_statement_index = find_matching_endcase(frame, frame.pc - 1U).value_or(frame.pc - 1U),
                                       .matched = false});
                return {};
            case StatementKind::case_statement:
            {
                if (frame.cases.empty())
                {
                    return {};
                }

                if (frame.cases.back().matched)
                {
                    const std::size_t next_pc = frame.cases.back().endcase_statement_index + 1U;
                    frame.cases.pop_back();
                    frame.pc = next_pc;
                    return {};
                }

                const auto predicate_value = evaluate_resumable_expression(frame, statement);
                if (!predicate_value.has_value())
                {
                    return {};
                }
                apply_case_predicate(value_as_bool(*predicate_value));
                return {};
            }
            case StatementKind::otherwise_statement:
                if (frame.cases.empty())
                {
                    return {};
                }
                if (frame.cases.back().matched)
                {
                    const std::size_t next_pc = frame.cases.back().endcase_statement_index + 1U;
                    frame.cases.pop_back();
                    frame.pc = next_pc;
                    return {};
                }
                frame.cases.back().matched = true;
                return {};
            case StatementKind::if_statement:
            {
                const auto predicate_value = evaluate_resumable_expression(frame, statement);
                if (!predicate_value.has_value())
                {
                    return {};
                }
                apply_conditional_predicate(statement.kind, value_as_bool(*predicate_value));
                return {};
            }
            case StatementKind::else_statement:
                if (!trim_copy(statement.expression).empty())
                {
                    if (!frame.evaluate_conditional_else)
                    {
                        if (const auto destination = find_matching_branch(frame, frame.pc - 1U, false))
                        {
                            frame.pc = *destination + 1U;
                        }
                        return {};
                    }

                    const auto predicate_value = evaluate_resumable_expression(frame, statement);
                    if (!predicate_value.has_value())
                    {
                        return {};
                    }
                    if (value_as_bool(*predicate_value))
                    {
                        frame.evaluate_conditional_else = false;
                        return {};
                    }
                    apply_conditional_predicate(statement.kind, false);
                    return {};
                }

                frame.evaluate_conditional_else = false;
                if (const auto destination = find_matching_branch(frame, frame.pc - 1U, false))
                {
                    frame.pc = *destination + 1U;
                }
                return {};
            case StatementKind::endif_statement:
                frame.evaluate_conditional_else = false;
                return {};
            case StatementKind::for_statement:
            {
                frame.loop_expression_continuation = LoopExpressionContinuation{
                    .statement = statement,
                    .stage = LoopExpressionStage::for_start};
                const auto start_value = evaluate_resumable_expression(
                    frame,
                    make_loop_stage_statement(frame.loop_expression_continuation->statement,
                                              statement.expression,
                                              LoopExpressionStage::for_start));
                if (!start_value.has_value())
                {
                    return {};
                }
                finish_loop_expression(frame, statement, *start_value);
                return {};
            }
            case StatementKind::do_while_statement:
            {
                frame.loop_expression_continuation = LoopExpressionContinuation{
                    .statement = statement,
                    .stage = LoopExpressionStage::do_while_predicate};
                const auto predicate_value = evaluate_resumable_expression(
                    frame,
                    make_loop_stage_statement(frame.loop_expression_continuation->statement,
                                              statement.expression,
                                              LoopExpressionStage::do_while_predicate));
                if (!predicate_value.has_value())
                {
                    return {};
                }
                finish_loop_expression(frame, statement, *predicate_value);
                return {};
            }
            case StatementKind::endfor_statement:
                return continue_for_loop(frame, statement, false);
            case StatementKind::loop_statement:
            {
                const auto active_loop = find_innermost_active_loop(frame);
                if (!active_loop.has_value())
                {
                    return {};
                }

                switch (active_loop->kind)
                {
                case ActiveLoopKind::for_loop:
                    return continue_for_loop(frame, statement, true);
                case ActiveLoopKind::scan_loop:
                    return continue_scan_loop(frame, statement, true);
                case ActiveLoopKind::while_loop:
                    if (!frame.whiles.empty())
                    {
                        unwind_case_contexts(frame, frame.whiles.back().case_stack_depth_at_entry);
                        unwind_with_bindings(frame, frame.whiles.back().with_stack_depth_at_entry);
                    }
                    frame.pc = active_loop->start_statement_index;
                    return {};
                }
                return {};
            }
            case StatementKind::continue_command:
            {
                CursorState *cursor = resolve_cursor_target_expression({}, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                        {{"command", "CONTINUE"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!cursor->locate_active)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ContinueRequiresActiveLocate");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::size_t start_recno = cursor->eof ? (cursor->record_count + 1U) : (cursor->recno + 1U);
                if (!locate_next_matching_record(
                        *cursor,
                        cursor->active_locate_for_expression,
                        cursor->active_locate_while_expression,
                        frame,
                        start_recno))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::string locate_detail = cursor->active_locate_for_expression.empty()
                    ? std::string{"ALL"}
                    : cursor->active_locate_for_expression;
                events.push_back({.category = "runtime.locate",
                                  .detail = "CONTINUE " + locate_detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::exit_statement:
            {
                const auto active_loop = find_innermost_active_loop(frame);
                if (!active_loop.has_value())
                {
                    return {};
                }

                switch (active_loop->kind)
                {
                case ActiveLoopKind::for_loop:
                    if (!frame.loops.empty())
                    {
                        unwind_case_contexts(frame, frame.loops.back().case_stack_depth_at_entry);
                        unwind_with_bindings(frame, frame.loops.back().with_stack_depth_at_entry);
                    }
                    frame.loops.pop_back();
                    frame.pc = active_loop->end_statement_index + 1U;
                    return {};
                case ActiveLoopKind::scan_loop:
                    if (!frame.scans.empty())
                    {
                        unwind_case_contexts(frame, frame.scans.back().case_stack_depth_at_entry);
                        unwind_with_bindings(frame, frame.scans.back().with_stack_depth_at_entry);
                    }
                    frame.scans.pop_back();
                    frame.pc = active_loop->end_statement_index + 1U;
                    return {};
                case ActiveLoopKind::while_loop:
                    if (!frame.whiles.empty())
                    {
                        unwind_case_contexts(frame, frame.whiles.back().case_stack_depth_at_entry);
                        unwind_with_bindings(frame, frame.whiles.back().with_stack_depth_at_entry);
                    }
                    frame.whiles.pop_back();
                    frame.pc = active_loop->end_statement_index + 1U;
                    return {};
                }
                return {};
            }
            case StatementKind::enddo_statement:
                if (!frame.whiles.empty())
                {
                    ++frame.whiles.back().iteration_count;
                    if (frame.whiles.back().iteration_count > max_loop_iterations)
                    {
                        last_error_message = loop_iteration_limit_message();
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    frame.pc = frame.whiles.back().do_while_statement_index;
                }
                return {};
            case StatementKind::endcase_statement:
                if (!frame.cases.empty())
                {
                    frame.cases.pop_back();
                }
                return {};
            case StatementKind::with_statement:
            {
                const auto target_value = resumed_with_target_value.has_value()
                                              ? resumed_with_target_value
                                              : evaluate_resumable_expression(frame, statement);
                if (!target_value.has_value())
                {
                    return {};
                }
                const PrgValue target = *target_value;
                const std::string binding_name =
                    "__with_" + std::to_string(frame.withs.size() + 1U) + "_" + std::to_string(frame.pc - 1U);
                frame.local_names.insert(binding_name);
                frame.locals[binding_name] = target;
                frame.withs.push_back({.target = target,
                                       .binding_name = binding_name});
                events.push_back({.category = "runtime.with",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::endwith_statement:
                if (!frame.withs.empty())
                {
                    frame.locals.erase(frame.withs.back().binding_name);
                    frame.local_names.erase(frame.withs.back().binding_name);
                    frame.withs.pop_back();
                }
                return {};
            case StatementKind::try_statement:
            {
                const TryClauseTargets targets = find_try_clause_targets(frame, frame.pc - 1U);
                if (!targets.endtry_statement_index.has_value())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.TryBlockMissingEndtry");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                frame.tries.push_back({.try_statement_index = frame.pc - 1U,
                                       .with_stack_depth_at_try_entry = frame.withs.size(),
                                       .case_stack_depth_at_try_entry = frame.cases.size(),
                                       .catch_statement_indices = targets.catch_statement_indices,
                                       .finally_statement_index = targets.finally_statement_index,
                                       .endtry_statement_index = *targets.endtry_statement_index,
                                       .handling_error = false,
                                       .entered_catch = false,
                                       .entered_finally = false,
                                       .propagate_after_finally = false,
                                       .return_after_finally = false});
                return {};
            }
            case StatementKind::catch_statement:
                if (!frame.tries.empty() &&
                    std::find(frame.tries.back().catch_statement_indices.begin(),
                              frame.tries.back().catch_statement_indices.end(),
                              frame.pc - 1U) != frame.tries.back().catch_statement_indices.end())
                {
                    const TryState active_try = frame.tries.back();
                    if (active_try.finally_statement_index.has_value())
                    {
                        frame.tries.back().entered_finally = true;
                        frame.pc = *active_try.finally_statement_index + 1U;
                    }
                    else
                    {
                        frame.tries.pop_back();
                        frame.pc = active_try.endtry_statement_index + 1U;
                    }
                }
                return {};
            case StatementKind::finally_statement:
                if (!frame.tries.empty() && frame.tries.back().finally_statement_index == (frame.pc - 1U))
                {
                    frame.tries.back().entered_finally = true;
                }
                return {};
            case StatementKind::endtry_statement:
                if (!frame.tries.empty() && frame.tries.back().endtry_statement_index == (frame.pc - 1U))
                {
                    const bool propagate_after_finally = frame.tries.back().propagate_after_finally;
                    const bool return_after_finally = frame.tries.back().return_after_finally;
                    frame.tries.pop_back();
                    if (frame.return_pending && return_after_finally)
                    {
                        if (const auto outcome = continue_pending_return(frame); outcome.has_value())
                        {
                            return *outcome;
                        }
                        return {};
                    }
                    if (propagate_after_finally)
                    {
                        return {.ok = false, .message = last_error_message};
                    }
                }
                return {};
            case StatementKind::throw_statement:
            {
                if (statement.expression.empty())
                {
                    if (has_active_exception_context())
                    {
                        last_error_message = current_error_message();
                        last_error_code = current_error_code();
                        last_error_work_area = current_error_work_area();
                        last_error_procedure = current_error_procedure();
                        last_fault_location = current_fault_location();
                        last_fault_statement = current_fault_statement();
                        last_error_compatibility = current_error_compatibility();
                        last_error_compatibility.explicit_error_code = last_error_code;
                        last_error_compatibility.preserve_fault_context = true;
                    }
                    else
                    {
                        last_error_message = runtime_text("Runtime.Prg.Core.Error.UserThrown");
                        last_error_code = 2071;
                        last_error_work_area = current_selected_work_area();
                        last_error_procedure = frame.routine_name;
                        last_error_compatibility = {};
                        last_error_compatibility.explicit_error_code = 2071;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                    }
                    return {.ok = false, .message = last_error_message};
                }

                const auto thrown_value_result = resumed_throw_value.has_value()
                                                     ? resumed_throw_value
                                                     : evaluate_resumable_expression(frame, statement);
                if (!thrown_value_result.has_value())
                {
                    return {};
                }
                const PrgValue thrown_value = *thrown_value_result;
                last_error_message = runtime_text("Runtime.Prg.Core.Error.UserThrown");
                last_error_code = 2071;
                last_error_work_area = current_selected_work_area();
                last_error_procedure = frame.routine_name;
                last_error_compatibility = {};
                last_error_compatibility.thrown_user_value = thrown_value;
                last_error_compatibility.explicit_error_code = 2071;
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }
            case StatementKind::read_events:
                waiting_for_events = true;
                events.push_back({.category = "runtime.event_loop",
                                  .detail = "READ EVENTS entered",
                                  .location = statement.location});
                return {.ok = true, .waiting_for_events = true, .frame_returned = false, .message = {}};
            case StatementKind::clear_events:
            {
                waiting_for_events = false;
                restore_event_loop_after_dispatch = false;
                events.push_back({.category = "runtime.event_loop",
                                  .detail = "CLEAR EVENTS",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::begin_transaction:
            {
                int &level = current_transaction_level();
                const bool opening_root_transaction = level == 0;
                ++level;
                if (opening_root_transaction)
                {
                    if (!begin_transaction_journal_if_needed() || !sync_transaction_journal_level())
                    {
                        --level;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                else if (!sync_transaction_journal_level())
                {
                    --level;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.transaction.begin",
                                  .detail = std::to_string(level),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::end_transaction:
            {
                int &level = current_transaction_level();
                if (level <= 0)
                {
                    last_error_message = runtime_text("Runtime.Prg.Transaction.Error.NoActiveTransaction");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (level > 0)
                {
                    --level;
                }
                if (level == 0)
                {
                    commit_active_transaction_journal();
                }
                else if (!sync_transaction_journal_level())
                {
                    ++level;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.transaction.end",
                                  .detail = std::to_string(level),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::rollback_transaction:
            {
                int &level = current_transaction_level();
                if (level <= 0)
                {
                    last_error_message = runtime_text("Runtime.Prg.Transaction.Error.NoActiveTransaction");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (level > 0 && !rollback_active_transaction_journal())
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                level = 0;
                events.push_back({.category = "runtime.transaction.rollback",
                                  .detail = "0",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::undo_command:
            {
                const bool is_all = normalize_identifier(statement.secondary_expression) == "all";
                if (is_all ? !undo_all_command_journals() : !undo_latest_command_journal())
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.command_undo",
                                  .detail = is_all ? "ALL" : "LATEST",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::doevents_command:
                // DOEVENTS: Pump pending event queue without blocking indefinitely.
                // In a GUI app, this allows UI responsiveness during long operations.
                // For now, we emit an event and yield to allow background processing.
                events.push_back({.category = "runtime.event_loop",
                                  .detail = "DOEVENTS",
                                  .location = statement.location});
                // Cooperative yield: brief pause to allow OS event processing
                // (In a real GUI framework, this would dispatch queued events)
                return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
            case StatementKind::seek_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                        {{"command", "SEEK"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto search_key_value = resumed_seek_value.has_value()
                                                  ? resumed_seek_value
                                                  : evaluate_resumable_expression(frame, statement);
                if (!search_key_value.has_value())
                {
                    return {};
                }
                const std::string search_key = value_as_string(*search_key_value);
                std::string used_order_name;
                std::string used_order_normalization_hint;
                std::string used_order_collation_hint;
                bool used_order_descending = false;
                const bool found = execute_seek(
                    *cursor,
                    search_key,
                    frame,
                    true,
                    false,
                    statement.tertiary_expression,
                    parse_order_direction_override(statement.quaternary_expression),
                    nullptr,
                    &used_order_name,
                    &used_order_normalization_hint,
                    &used_order_collation_hint,
                    &used_order_descending);
                events.push_back({.category = "runtime.seek",
                                  .detail = format_order_metadata_detail(
                                                used_order_name.empty() ? std::string{"<default>"} : used_order_name,
                                                used_order_normalization_hint,
                                                used_order_collation_hint,
                                                used_order_descending) +
                                            ": " + search_key + (found ? " -> found" : " -> not found"),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::locate_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                        {{"command", "LOCATE"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                cursor->active_locate_for_expression = statement.expression;
                cursor->active_locate_while_expression = statement.tertiary_expression;
                cursor->locate_active = true;
                if (!locate_next_matching_record(*cursor, statement.expression, statement.tertiary_expression, frame, 1U))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.locate",
                                  .detail = statement.expression.empty() ? "ALL" : statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::scan_statement:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                        {{"command", "SCAN"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::size_t start_recno = cursor->recno == 0U ? 1U : cursor->recno;
                if (scan_expression_requires_continuation(
                        frame,
                        statement.expression,
                        statement.tertiary_expression,
                        cursor->filter_expression))
                {
                    return begin_scan_expression_search(
                        frame,
                        statement,
                        ScanSearchKind::enter_scan,
                        cursor->work_area,
                        start_recno,
                        frame.pc - 1U,
                        find_matching_endscan(frame, frame.pc - 1U).value_or(frame.pc - 1U),
                        0U,
                        false);
                }
                if (!locate_next_matching_record(*cursor, statement.expression, statement.tertiary_expression, frame, start_recno))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!cursor->found)
                {
                    if (const auto destination = find_matching_endscan(frame, frame.pc - 1U))
                    {
                        frame.pc = *destination + 1U;
                    }
                    return {};
                }

                frame.scans.push_back({.scan_statement_index = frame.pc - 1U,
                                       .endscan_statement_index = find_matching_endscan(frame, frame.pc - 1U).value_or(frame.pc - 1U),
                                       .case_stack_depth_at_entry = frame.cases.size(),
                                       .with_stack_depth_at_entry = frame.withs.size(),
                                       .work_area = cursor->work_area,
                                       .for_expression = statement.expression,
                                       .while_expression = statement.tertiary_expression,
                                       .iteration_count = 0});
                events.push_back({.category = "runtime.scan",
                                  .detail = statement.expression.empty() ? "ALL" : statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::endscan_statement:
                return continue_scan_loop(frame, statement, false);
            case StatementKind::replace_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "REPLACE"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::vector<ReplaceAssignment> assignments = parse_replace_assignments(statement.expression);
                if (assignments.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ReplaceRequiresFieldWithExpressionAssignment");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::optional<AggregateScopeClause> replace_scope;
                if (!statement.identifier.empty())
                {
                    AggregateScopeClause parsed_scope;
                    if (!statement.names.empty())
                    {
                        parsed_scope.raw_value = statement.names.front();
                    }
                    if (statement.identifier == "rest")
                    {
                        parsed_scope.kind = AggregateScopeKind::rest_records;
                    }
                    else if (statement.identifier == "next")
                    {
                        parsed_scope.kind = AggregateScopeKind::next_records;
                    }
                    else if (statement.identifier == "record")
                    {
                        parsed_scope.kind = AggregateScopeKind::record;
                    }
                    replace_scope = std::move(parsed_scope);
                }
                if (!execute_with_command_undo(cursor->source_path, "REPLACE", [&]
                    {
                        return replace_records(
                            *cursor,
                            assignments,
                            frame,
                            replace_scope,
                            statement.tertiary_expression,
                            statement.quaternary_expression);
                    }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string replace_detail = statement.expression;
                if (!statement.identifier.empty())
                {
                    replace_detail += " " + uppercase_copy(statement.identifier);
                    if (!statement.names.empty())
                    {
                        replace_detail += " " + statement.names.front();
                    }
                }
                if (!trim_copy(statement.tertiary_expression).empty())
                {
                    replace_detail += " FOR " + statement.tertiary_expression;
                }
                if (!trim_copy(statement.quaternary_expression).empty())
                {
                    replace_detail += " WHILE " + statement.quaternary_expression;
                }
                events.push_back({.category = "runtime.replace",
                                  .detail = replace_detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::update_command:
            {
                const std::string target_expression = trim_copy(statement.secondary_expression).empty()
                                                          ? trim_copy(statement.identifier)
                                                          : trim_copy(statement.secondary_expression);
                CursorState *cursor = resolve_cursor_target_expression(target_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "UPDATE"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::vector<ReplaceAssignment> assignments = parse_update_set_assignments(statement.expression);
                if (assignments.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.UpdateRequiresSetFieldExpressionAssignments");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const std::string for_expression = trim_copy(statement.tertiary_expression).empty()
                                                       ? ".T."
                                                       : statement.tertiary_expression;
                if (!execute_with_command_undo(cursor->source_path, "UPDATE", [&]
                    {
                        return replace_records(
                            *cursor,
                            assignments,
                            frame,
                            AggregateScopeClause{},
                            for_expression,
                            statement.quaternary_expression);
                    }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.update",
                                  .detail = statement.text,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::append_blank_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "APPEND BLANK"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!execute_with_command_undo(cursor->source_path, "APPEND BLANK", [&]
                    {
                        return append_blank_record(*cursor);
                    }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.append_blank",
                                  .detail = cursor->alias,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::delete_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "DELETE"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::optional<AggregateScopeClause> delete_scope;
                if (!statement.identifier.empty())
                {
                    AggregateScopeClause parsed_scope;
                    if (!statement.names.empty())
                    {
                        parsed_scope.raw_value = statement.names.front();
                    }
                    if (statement.identifier == "rest")
                    {
                        parsed_scope.kind = AggregateScopeKind::rest_records;
                    }
                    else if (statement.identifier == "next")
                    {
                        parsed_scope.kind = AggregateScopeKind::next_records;
                    }
                    else if (statement.identifier == "record")
                    {
                        parsed_scope.kind = AggregateScopeKind::record;
                    }
                    delete_scope = std::move(parsed_scope);
                }
                if (!execute_with_command_undo(cursor->source_path, "DELETE", [&]
                    {
                        return set_deleted_flag(
                            *cursor, frame, delete_scope, statement.expression, statement.tertiary_expression, true);
                    }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string delete_detail = statement.expression.empty() ? cursor->alias : statement.expression;
                if (delete_scope.has_value())
                {
                    delete_detail = uppercase_copy(statement.identifier);
                    if (!delete_scope->raw_value.empty())
                    {
                        delete_detail += " " + delete_scope->raw_value;
                    }
                }
                if (!trim_copy(statement.tertiary_expression).empty())
                {
                    delete_detail += " WHILE " + statement.tertiary_expression;
                }
                events.push_back({.category = "runtime.delete",
                                  .detail = delete_detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::delete_from_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.identifier, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "DELETE FROM"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const std::string where_expression = trim_copy(statement.expression).empty()
                                                         ? ".T."
                                                         : statement.expression;
                if (!execute_with_command_undo(cursor->source_path, "DELETE FROM", [&]
                    {
                        return set_deleted_flag(
                            *cursor, frame, std::nullopt, where_expression, statement.tertiary_expression, true);
                    }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string delete_detail = cursor->alias;
                if (!trim_copy(statement.expression).empty())
                {
                    delete_detail += " WHERE " + statement.expression;
                }
                events.push_back({.category = "runtime.delete_from",
                                  .detail = delete_detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::recall_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "RECALL"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::optional<AggregateScopeClause> recall_scope;
                if (!statement.identifier.empty())
                {
                    AggregateScopeClause parsed_scope;
                    if (!statement.names.empty())
                    {
                        parsed_scope.raw_value = statement.names.front();
                    }
                    if (statement.identifier == "rest")
                    {
                        parsed_scope.kind = AggregateScopeKind::rest_records;
                    }
                    else if (statement.identifier == "next")
                    {
                        parsed_scope.kind = AggregateScopeKind::next_records;
                    }
                    else if (statement.identifier == "record")
                    {
                        parsed_scope.kind = AggregateScopeKind::record;
                    }
                    recall_scope = std::move(parsed_scope);
                }
                if (!execute_with_command_undo(cursor->source_path, "RECALL", [&]
                    {
                        return set_deleted_flag(
                            *cursor, frame, recall_scope, statement.expression, statement.tertiary_expression, false);
                    }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string recall_detail = statement.expression.empty() ? cursor->alias : statement.expression;
                if (recall_scope.has_value())
                {
                    recall_detail = uppercase_copy(statement.identifier);
                    if (!recall_scope->raw_value.empty())
                    {
                        recall_detail += " " + recall_scope->raw_value;
                    }
                }
                if (!trim_copy(statement.tertiary_expression).empty())
                {
                    recall_detail += " WHILE " + statement.tertiary_expression;
                }
                events.push_back({.category = "runtime.recall",
                                  .detail = recall_detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::insert_into_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.identifier, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "INSERT INTO"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (trim_copy(statement.secondary_expression).empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.InsertIntoRequiresValuesClause");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const bool inserts_query_rows =
                    normalize_identifier(statement.tertiary_expression) == "select";
                std::vector<std::vector<PrgValue>> query_rows;
                if (inserts_query_rows &&
                    !materialize_select_query_rows(statement.secondary_expression, frame, query_rows))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const CursorPositionSnapshot original_position = capture_cursor_snapshot(*cursor);
                const std::size_t original_record_count = cursor->record_count;
                const std::vector<vfp::DbfRecord> original_remote_records =
                    cursor->remote ? cursor->remote_records : std::vector<vfp::DbfRecord>{};
                const bool inserted = execute_with_command_undo(cursor->source_path, "INSERT INTO", [&]
                    {
                        if (!inserts_query_rows)
                        {
                            return insert_record_values(
                                *cursor,
                                frame,
                                statement.expression,
                                statement.secondary_expression);
                        }

                        for (const auto &row : query_rows)
                        {
                            if (!insert_record_values(
                                    *cursor,
                                    frame,
                                    statement.expression,
                                    serialize_insert_row_expression_list(row)))
                            {
                                return false;
                            }
                        }
                        return true;
                    });
                if (!inserted)
                {
                    if (cursor->remote)
                    {
                        cursor->remote_records = original_remote_records;
                    }
                    cursor->record_count = original_record_count;
                    restore_cursor_snapshot(*cursor, original_position);
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.insert_into",
                                  .detail = cursor->alias,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::pack_command:
            {
                const std::string pack_options = uppercase_copy(trim_copy(statement.expression));
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "PACK"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (pack_options.find("MEMO") != std::string::npos && pack_options.find("DBF") == std::string::npos)
                {
                    if (cursor->remote)
                    {
                        events.push_back({.category = "runtime.pack",
                                          .detail = cursor->alias + " MEMO",
                                          .location = statement.location});
                        return {};
                    }
                    if (!ensure_exclusive_table_maintenance(*cursor, "PACK MEMO"))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (!execute_with_command_undo(cursor->source_path, "PACK MEMO", [&]
                        {
                            if (!ensure_transaction_backup_for_table(cursor->source_path))
                            {
                                return false;
                            }
                            const auto pack_result = vfp::pack_dbf_memo_file(cursor->source_path);
                            if (!pack_result.ok)
                            {
                                last_error_message = pack_result.error;
                                return false;
                            }
                            cursor->record_count = pack_result.record_count;
                            move_cursor_to(*cursor, static_cast<long long>(std::min(cursor->recno, cursor->record_count)));
                            return true;
                        }))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                else
                {
                    const bool pack_ok = cursor->remote
                        ? pack_cursor(*cursor)
                        : execute_with_command_undo(cursor->source_path, "PACK", [&] { return pack_cursor(*cursor); });
                    if (!pack_ok)
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }

                events.push_back({.category = "runtime.pack",
                                  .detail = pack_options.find("MEMO") != std::string::npos ? cursor->alias + " MEMO" : cursor->alias,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::zap_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "ZAP"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const bool zap_ok = cursor->remote
                    ? zap_cursor(*cursor)
                    : execute_with_command_undo(cursor->source_path, "ZAP", [&] { return zap_cursor(*cursor); });
                if (!zap_ok)
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.zap",
                                  .detail = cursor->alias,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::unlock_command:
            {
                const std::string unlock_scope = normalize_identifier(statement.expression);
                if (unlock_scope == "all")
                {
                    unlock_cursor_locks(nullptr, true);
                    events.push_back({.category = "runtime.unlock",
                                      .detail = "ALL",
                                      .location = statement.location});
                    return {};
                }

                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "UNLOCK"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!trim_copy(statement.identifier).empty())
                {
                    const auto record_value = resumed_unlock_record_value.has_value()
                                                  ? resumed_unlock_record_value
                                                  : evaluate_resumable_expression(
                                                        frame,
                                                        [&]()
                                                        {
                                                            Statement record_statement = statement;
                                                            record_statement.expression = statement.identifier;
                                                            return record_statement;
                                                        }());
                    if (!record_value.has_value())
                    {
                        return {};
                    }
                    const std::size_t recno = static_cast<std::size_t>(
                        std::max<double>(0.0, std::llround(value_as_number(*record_value))));
                    if (recno == 0U || recno > cursor->record_count)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.UnlockRecordTargetRecordNotFound");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    unlock_cursor_record_lock(*cursor, recno);
                    events.push_back({.category = "runtime.unlock",
                                      .detail = (cursor->alias.empty() ? std::to_string(cursor->work_area) : cursor->alias) +
                                                " RECORD " + std::to_string(recno),
                                      .location = statement.location});
                    return {};
                }

                unlock_cursor_locks(cursor, false);
                events.push_back({.category = "runtime.unlock",
                                  .detail = cursor->alias.empty() ? std::to_string(cursor->work_area) : cursor->alias,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::go_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "GO"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::string destination = uppercase_copy(trim_copy(statement.expression));
                if (destination == "TOP")
                {
                    (void)seek_visible_record(*cursor, frame, 1, 1, {}, {}, false, true);
                }
                else if (destination == "BOTTOM")
                {
                    if (!seek_visible_record(*cursor, frame, static_cast<long long>(cursor->record_count), -1, {}, {}, false, true) &&
                        cursor->record_count > 0U)
                    {
                        // VFP keeps physical EOF while reporting both boundary flags when the filtered set is empty.
                        cursor->recno = cursor->record_count + 1U;
                        cursor->bof = true;
                        cursor->eof = true;
                    }
                }
                else
                {
                    const auto requested_value = resumed_go_value.has_value()
                                                     ? resumed_go_value
                                                     : evaluate_resumable_expression(frame, statement);
                    if (!requested_value.has_value())
                    {
                        return {};
                    }
                    const long long requested = std::llround(value_as_number(*requested_value));
                    move_cursor_to(*cursor, requested);
                }

                events.push_back({.category = "runtime.go",
                                  .detail = destination.empty() ? statement.expression : destination,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::skip_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "SKIP"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto delta_value = resumed_skip_value.has_value()
                                             ? resumed_skip_value
                                             : evaluate_resumable_expression(frame, statement);
                if (!delta_value.has_value())
                {
                    return {};
                }
                const long long delta = std::llround(value_as_number(*delta_value));
                if (!move_by_visible_records(*cursor, frame, delta))
                {
                    cursor->found = false;
                }
                events.push_back({.category = "runtime.skip",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::browse_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "BROWSE"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::vector<std::string> visible_fields = effective_visible_field_names(*cursor, statement.tertiary_expression);
                std::string field_detail;
                for (std::size_t index = 0U; index < visible_fields.size(); ++index)
                {
                    if (index > 0U)
                    {
                        field_detail += ",";
                    }
                    field_detail += visible_fields[index];
                }

                std::string detail = cursor->alias.empty()
                    ? ("workarea=" + std::to_string(cursor->work_area))
                    : (cursor->alias + "@" + std::to_string(cursor->work_area));
                detail += " recno=" + std::to_string(cursor->recno);
                detail += " records=" + std::to_string(cursor->record_count);
                detail += " fields=" + (field_detail.empty() ? std::string{"ALL"} : field_detail);
                detail += " filter=" + (cursor->filter_expression.empty() ? std::string{"<none>"} : cursor->filter_expression);
                if (!statement.quaternary_expression.empty())
                {
                    detail += " for=" + trim_copy(statement.quaternary_expression);
                }
                if (!statement.expression.empty())
                {
                    detail += " clause=" + trim_copy(statement.expression);
                }

                events.push_back({.category = "runtime.browse",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_order:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                     {{"command", "SET ORDER"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!activate_order(*cursor, statement.expression, parse_order_direction_override(statement.tertiary_expression)))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.order",
                                  .detail = format_order_metadata_detail(
                                      cursor->active_order_name,
                                      cursor->active_order_normalization_hint,
                                      cursor->active_order_collation_hint,
                                      cursor->active_order_descending),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::select_command:
            {
                std::string selection = evaluate_cursor_designator_expression(statement.expression, frame);
                if (selection.empty())
                {
                    selection = trim_copy(statement.expression);
                }
                if (selection.empty())
                {
                    return {};
                }

                int target_area = 0;
                const bool numeric_selection = std::all_of(selection.begin(), selection.end(), [](unsigned char ch)
                                                           { return std::isdigit(ch) != 0; });
                if (numeric_selection)
                {
                    target_area = std::stoi(selection);
                }
                else
                {
                    const CursorState *existing = find_cursor_by_alias(selection);
                    if (existing == nullptr)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                         {{"command", "SELECT"}}) +
                                            ": " + selection;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    target_area = existing->work_area;
                }

                const int selected = select_work_area(target_area);
                events.push_back({.category = "runtime.select",
                                  .detail = "work area " + std::to_string(selected),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::use_command:
            {
                if (statement.expression.empty() && statement.secondary_expression.empty())
                {
                    frame.use_command_continuation.reset();
                    close_cursor(std::to_string(current_selected_work_area()));
                    events.push_back({.category = "runtime.use.close",
                                      .detail = "current work area",
                                      .location = statement.location});
                    return {};
                }
                if (statement.expression.empty())
                {
                    frame.use_command_continuation.reset();
                    close_cursor(evaluate_cursor_designator_expression(statement.secondary_expression, frame));
                    events.push_back({.category = "runtime.use.close",
                                      .detail = trim_copy(statement.secondary_expression),
                                      .location = statement.location});
                    return {};
                }

                if (!frame.use_command_continuation.has_value() ||
                    frame.use_command_continuation->statement.text != statement.text)
                {
                    frame.use_command_continuation = UseCommandContinuation{
                        .statement = statement,
                        .target_value = std::nullopt};
                }
                UseCommandContinuation &continuation = *frame.use_command_continuation;
                if (!continuation.target_value.has_value())
                {
                    const auto target_value = resumed_use_target_value.has_value()
                                                  ? resumed_use_target_value
                                                  : evaluate_resumable_expression(frame, statement);
                    if (!target_value.has_value())
                    {
                        return {};
                    }
                    continuation.target_value = *target_value;
                    resumed_use_target_value.reset();
                }
                const std::string target = value_as_string(*continuation.target_value);
                std::string alias;
                if (statement.identifier.empty())
                {
                    alias = copperfin::platform::path_to_utf8_string(
                        copperfin::platform::path_from_utf8_string(unquote_string(target)).stem());
                }
                else
                {
                    Statement alias_statement = continuation.statement;
                    alias_statement.expression = statement.identifier;
                    alias_statement.text = continuation.statement.text + " [use-alias]";
                    continuation.pending_alias = true;
                    const auto alias_value = resumed_use_alias_value.has_value()
                                                 ? resumed_use_alias_value
                                                 : evaluate_resumable_expression(frame, alias_statement);
                    if (!alias_value.has_value())
                    {
                        return {};
                    }
                    alias = value_as_string(*alias_value);
                    continuation.pending_alias = false;
                    resumed_use_alias_value.reset();
                }
                if (alias.empty() && !statement.identifier.empty())
                {
                    alias = unquote_string(statement.identifier);
                }
                const bool allow_again = normalize_identifier(statement.tertiary_expression) == "again";
                std::optional<bool> exclusive_override;
                const std::string open_mode = normalize_identifier(statement.quaternary_expression);
                if (open_mode == "exclusive")
                {
                    exclusive_override = true;
                }
                else if (open_mode == "shared")
                {
                    exclusive_override = false;
                }
                std::string in_target = statement.secondary_expression;
                if (allow_again && trim_copy(in_target).empty())
                {
                    in_target = "0";
                }
                if (!open_table_cursor(target, alias, in_target, allow_again, false, 0, {}, 0U, {}, exclusive_override))
                {
                    frame.use_command_continuation.reset();
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.use.open",
                                  .detail = alias.empty() ? target : alias + " <- " + target,
                                  .location = statement.location});
                frame.use_command_continuation.reset();
                return {};
            }
            case StatementKind::open_database:
            {
                if (normalize_identifier(statement.quaternary_expression) == "validate")
                {
                    last_error_message = runtime_text("Runtime.Prg.Database.Error.ValidateUnsupported");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto target_value = resumed_open_database_target_value.has_value()
                                              ? resumed_open_database_target_value
                                              : evaluate_resumable_expression(frame, statement);
                if (!target_value.has_value())
                {
                    return {};
                }
                std::string target = value_as_string(*target_value);
                if (target.empty())
                {
                    target = unquote_string(trim_copy(statement.expression));
                }
                std::optional<bool> exclusive_override;
                const std::string mode = normalize_identifier(statement.secondary_expression);
                if (mode == "exclusive")
                {
                    exclusive_override = true;
                }
                else if (mode == "shared")
                {
                    exclusive_override = false;
                }
                const bool read_only = normalize_identifier(statement.tertiary_expression) == "noupdate";
                if (!open_database(target, exclusive_override, read_only))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.database.open",
                                  .detail = current_database_path(),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_command:
            {
                const auto [option_name, option_value] = split_first_word(statement.expression);
                std::string normalized_name = normalize_identifier(option_name);
                if (normalized_name == "udfp")
                {
                    normalized_name = "udfparms";
                }
                const auto strip_set_to_value = [](const std::string &raw_value) -> std::string
                {
                    std::string candidate = trim_copy(raw_value);
                    if (normalize_identifier(candidate) == "to")
                    {
                        return {};
                    }
                    if (starts_with_insensitive(candidate, "TO "))
                    {
                        candidate = trim_copy(candidate.substr(3U));
                    }
                    return candidate;
                };
                const auto should_evaluate_set_value = [&](const std::string &candidate) -> bool
                {
                    if (candidate.empty())
                    {
                        return false;
                    }
                    if (candidate.front() == '&')
                    {
                        return true;
                    }
                    if (candidate.size() >= 2U &&
                        candidate.front() == '(' && candidate.back() == ')')
                    {
                        return true;
                    }
                    if (candidate.size() >= 2U &&
                        ((candidate.front() == '\'' && candidate.back() == '\'') ||
                         (candidate.front() == '"' && candidate.back() == '"')))
                    {
                        return true;
                    }
                    if (is_bare_identifier_text(candidate))
                    {
                        return lookup_variable(frame, candidate).kind != PrgValueKind::empty;
                    }
                    return false;
                };
                const auto expand_set_text_macro = [&](const std::string &raw_value) -> std::optional<std::string>
                {
                    std::string candidate = strip_set_to_value(raw_value);
                    if (candidate.empty() || candidate.front() != '&')
                    {
                        return std::nullopt;
                    }

                    std::string expanded = candidate;
                    for (int depth = 0; depth < 4 && !expanded.empty() && expanded.front() == '&'; ++depth)
                    {
                        const std::string referent = trim_copy(expanded.substr(1U));
                        if (referent.empty())
                        {
                            break;
                        }
                        const PrgValue referent_value = evaluate_expression(referent, frame);
                        const std::string referent_text = trim_copy(value_as_string(referent_value));
                        if (referent_text.empty() || referent_text == expanded)
                        {
                            break;
                        }
                        expanded = referent_text;
                    }

                    constexpr std::size_t max_identifier_hops = 16U;
                    std::vector<std::string> visited_identifiers;
                    visited_identifiers.reserve(8U);
                    while (is_bare_identifier_text(expanded) && visited_identifiers.size() < max_identifier_hops)
                    {
                        const std::string normalized = normalize_memory_variable_identifier(expanded);
                        if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                        {
                            break;
                        }
                        visited_identifiers.push_back(normalized);

                        const PrgValue referent_value = evaluate_expression(expanded, frame);
                        const std::string referent_text = trim_copy(value_as_string(referent_value));
                        if (referent_text.empty() || referent_text == expanded)
                        {
                            break;
                        }
                        expanded = referent_text;
                    }

                    return expanded == candidate ? std::optional<std::string>{std::string{}} : std::optional<std::string>{expanded};
                };
                const auto evaluate_set_string_value = [&](const std::string &raw_value, const std::string &default_value) -> std::string
                {
                    const std::string candidate = strip_set_to_value(raw_value);
                    if (candidate.empty())
                    {
                        return default_value;
                    }
                    if (const auto expanded_macro = expand_set_text_macro(candidate); expanded_macro.has_value())
                    {
                        return *expanded_macro;
                    }
                    if (should_evaluate_set_value(candidate))
                    {
                        return value_as_string(evaluate_expression(candidate, frame));
                    }
                    return unquote_string(candidate);
                };
                const auto evaluate_set_integer_value = [&](const std::string &raw_value, int default_value, int min_value, int max_value) -> int
                {
                    const std::string candidate = strip_set_to_value(raw_value);
                    if (candidate.empty())
                    {
                        return default_value;
                    }
                    int parsed_value = default_value;
                    try
                    {
                        parsed_value = static_cast<int>(std::llround(value_as_number(
                            should_evaluate_set_value(candidate) ? evaluate_expression(candidate, frame)
                                                                 : make_string_value(unquote_string(candidate)))));
                    }
                    catch (...)
                    {
                        parsed_value = default_value;
                    }
                    return std::clamp(parsed_value, min_value, max_value);
                };
                const auto normalize_boolean_set_value = [&](const std::string &raw_value) -> std::string
                {
                    auto map_boolean_token = [](const std::string &raw_token) -> std::optional<std::string>
                    {
                        const std::string normalized_token = normalize_identifier(raw_token);
                        if (normalized_token.empty() || normalized_token == "on" || normalized_token == "true" || normalized_token == "1" ||
                            normalized_token == ".t." || normalized_token == "yes" || normalized_token == "y")
                        {
                            return std::string{"on"};
                        }
                        if (normalized_token == "off" || normalized_token == "false" || normalized_token == "0" ||
                            normalized_token == ".f." || normalized_token == "no" || normalized_token == "n")
                        {
                            return std::string{"off"};
                        }
                        return std::nullopt;
                    };

                    std::string candidate = trim_copy(raw_value);
                    if (starts_with_insensitive(candidate, "TO "))
                    {
                        candidate = trim_copy(candidate.substr(3U));
                    }

                    if (const auto mapped = map_boolean_token(candidate))
                    {
                        return *mapped;
                    }

                    if (!candidate.empty())
                    {
                        const PrgValue evaluated = evaluate_expression(candidate, frame);
                        if (evaluated.kind == PrgValueKind::boolean)
                        {
                            return value_as_bool(evaluated) ? "on" : "off";
                        }
                        if (evaluated.kind == PrgValueKind::number || evaluated.kind == PrgValueKind::int64 ||
                            evaluated.kind == PrgValueKind::uint64 || evaluated.kind == PrgValueKind::currency)
                        {
                            return std::abs(value_as_number(evaluated)) > 0.000001 ? "on" : "off";
                        }
                        if (const auto mapped = map_boolean_token(value_as_string(evaluated)))
                        {
                            return *mapped;
                        }
                    }

                    return raw_value;
                };

                if (starts_with_insensitive(statement.expression, "SKIP OF BAR "))
                {
                    const std::string body = trim_copy(statement.expression.substr(12U));
                    const auto [bar_text, bar_tail] = split_first_word(body);
                    const std::size_t of_position = find_keyword_top_level(bar_tail, "OF");
                    if (of_position != std::string::npos)
                    {
                        const auto [popup_text, skip_expression] = split_first_word(
                            trim_copy(bar_tail.substr(of_position + 2U)));
                        const auto bar_number = try_parse_numeric_index_value(bar_text);
                        const std::string popup_name = normalize_identifier(unquote_identifier(popup_text));
                        if (bar_number.has_value() && !popup_name.empty())
                        {
                            const bool disabled = skip_expression.empty()
                                ? false
                                : value_as_bool(evaluate_expression(skip_expression, frame));
                            current_session_state().popup_bar_skip_states[popup_name][
                                static_cast<long long>(std::llround(*bar_number))] = disabled;
                            events.push_back({.category = "runtime.set_skip",
                                              .detail = "popup=" + popup_name +
                                                        " bar=" + std::to_string(static_cast<long long>(std::llround(*bar_number))) +
                                                        " disabled=" + (disabled ? "true" : "false"),
                                              .location = statement.location});
                            return {};
                        }
                    }
                }

                if (starts_with_insensitive(statement.expression, "MARK OF BAR "))
                {
                    const std::string body = trim_copy(statement.expression.substr(12U));
                    const auto [bar_text, bar_tail] = split_first_word(body);
                    const std::size_t of_position = find_keyword_top_level(bar_tail, "OF");
                    if (of_position != std::string::npos)
                    {
                        const auto [popup_text, mark_tail] = split_first_word(
                            trim_copy(bar_tail.substr(of_position + 2U)));
                        const std::size_t to_position = find_keyword_top_level(mark_tail, "TO");
                        const auto bar_number = try_parse_numeric_index_value(bar_text);
                        const std::string popup_name = normalize_identifier(unquote_identifier(popup_text));
                        if (to_position != std::string::npos && bar_number.has_value() && !popup_name.empty())
                        {
                            const bool marked = value_as_bool(evaluate_expression(
                                trim_copy(mark_tail.substr(to_position + 2U)), frame));
                            current_session_state().popup_bar_mark_states[popup_name][
                                static_cast<long long>(std::llround(*bar_number))] = marked;
                            events.push_back({.category = "runtime.set_mark",
                                              .detail = "popup=" + popup_name +
                                                        " bar=" + std::to_string(static_cast<long long>(std::llround(*bar_number))) +
                                                        " marked=" + (marked ? "true" : "false"),
                                              .location = statement.location});
                            return {};
                        }
                    }
                }

                if (normalized_name == "database")
                {
                    const std::string target = evaluate_set_string_value(option_value, {});
                    if (!set_current_database(target))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.database.current",
                                      .detail = current_database_path(),
                                      .location = statement.location});
                    return {};
                }

                if (normalized_name == "filter")
                {
                    std::string filter_clause = strip_set_to_value(option_value);

                    std::string filter_target;
                    const std::size_t in_position = find_keyword_top_level(filter_clause, "IN");
                    if (in_position != std::string::npos)
                    {
                        filter_target = trim_copy(filter_clause.substr(in_position + 2U));
                        filter_clause = trim_copy(filter_clause.substr(0U, in_position));
                    }

                    const std::string resolved_filter_target = evaluate_cursor_designator_expression(filter_target, frame);
                    CursorState *cursor = resolve_cursor_target(resolved_filter_target);
                    if (cursor == nullptr)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.SetFilterRequiresSelectedWorkArea");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    if (normalize_identifier(filter_clause) == "off")
                    {
                        filter_clause.clear();
                    }
                    else if (const auto expanded_macro = expand_set_text_macro(filter_clause); expanded_macro.has_value())
                    {
                        filter_clause = *expanded_macro;
                    }
                    else if (should_evaluate_set_value(filter_clause))
                    {
                        filter_clause = value_as_string(evaluate_expression(filter_clause, frame));
                    }
                    else
                    {
                        filter_clause = unquote_string(filter_clause);
                    }

                    cursor->filter_expression = filter_clause;

                    events.push_back({.category = "runtime.filter",
                                      .detail = cursor->filter_expression.empty() ? "OFF" : cursor->filter_expression,
                                      .location = statement.location});
                    return {};
                }
                if (normalized_name == "textmerge")
                {
                    auto clamp_textmerge_delimiter = [](std::string delimiter) -> std::string
                    {
                        if (delimiter.size() > 2U)
                        {
                            delimiter.resize(2U);
                        }
                        return delimiter;
                    };

                    std::string textmerge_value = trim_copy(option_value);
                    if (starts_with_insensitive(textmerge_value, "DELIMITERS"))
                    {
                        textmerge_value = strip_set_to_value(textmerge_value.substr(10U));

                        std::string left_delimiter = "<<";
                        std::string right_delimiter = ">>";
                        if (!textmerge_value.empty())
                        {
                            const std::vector<std::string> delimiter_parts = split_csv_like(textmerge_value);
                            if (!delimiter_parts.empty())
                            {
                                left_delimiter = clamp_textmerge_delimiter(
                                    evaluate_set_string_value(delimiter_parts[0], "<<"));
                                if (delimiter_parts.size() >= 2U)
                                {
                                    right_delimiter = clamp_textmerge_delimiter(
                                        evaluate_set_string_value(delimiter_parts[1], left_delimiter));
                                }
                                else
                                {
                                    right_delimiter = left_delimiter;
                                }

                                if (left_delimiter.empty())
                                {
                                    left_delimiter = "<<";
                                }
                                if (right_delimiter.empty())
                                {
                                    right_delimiter = left_delimiter;
                                }
                            }
                        }

                        current_set_state()["textmerge_left_delimiter"] = left_delimiter;
                        current_set_state()["textmerge_right_delimiter"] = right_delimiter;
                        events.push_back({.category = "runtime.set",
                                          .detail = statement.expression,
                                          .location = statement.location});
                        return {};
                    }

                    bool recognized_textmerge_state = false;
                    bool saw_unrecognized_textmerge_token = false;
                    std::istringstream textmerge_stream(textmerge_value);
                    std::string textmerge_token;
                    while (textmerge_stream >> textmerge_token)
                    {
                        const std::string normalized_textmerge_token = normalize_identifier(textmerge_token);
                        if (normalized_textmerge_token == "on" || normalized_textmerge_token == "off")
                        {
                            current_set_state()["textmerge"] = normalized_textmerge_token;
                            recognized_textmerge_state = true;
                            continue;
                        }
                        if (normalized_textmerge_token == "show" || normalized_textmerge_token == "noshow")
                        {
                            current_set_state()["textmerge_show"] = normalized_textmerge_token;
                            recognized_textmerge_state = true;
                            continue;
                        }

                        saw_unrecognized_textmerge_token = true;
                        break;
                    }

                    if (recognized_textmerge_state && !saw_unrecognized_textmerge_token)
                    {
                        if (current_set_state().find("textmerge") == current_set_state().end())
                        {
                            current_set_state()["textmerge"] = "off";
                        }
                        events.push_back({.category = "runtime.set",
                                          .detail = statement.expression,
                                          .location = statement.location});
                        return {};
                    }
                }
                if (!normalized_name.empty())
                {
                    if (normalized_name == "udfparms")
                    {
                        const std::string udfparms_value = normalize_identifier(
                            evaluate_set_string_value(option_value, "VALUE"));
                        udfparms_mode =
                            udfparms_value == "reference" || udfparms_value == "refe"
                                ? std::string{"REFERENCE"}
                                : std::string{"VALUE"};
                    }
                    else if (normalized_name == "exact" || normalized_name == "deleted" || normalized_name == "near" ||
                        normalized_name == "strictdate" || normalized_name == "optimize" ||
                        normalized_name == "talk" || normalized_name == "safety" || normalized_name == "escape" ||
                        normalized_name == "century" || normalized_name == "seconds" || normalized_name == "exclusive" ||
                        normalized_name == "multilocks" || normalized_name == "null" || normalized_name == "ansi")
                    {
                        current_set_state()[normalized_name] = normalize_boolean_set_value(option_value.empty() ? "on" : option_value);
                    }
                    else if (normalized_name == "reprocess")
                    {
                        std::string reprocess_value = trim_copy(evaluate_set_string_value(option_value, "AUTOMATIC"));
                        const std::string normalized_reprocess = normalize_identifier(reprocess_value);
                        if (reprocess_value.empty() ||
                            normalized_reprocess == "automatic" ||
                            normalized_reprocess == "auto" ||
                            normalized_reprocess == "on" ||
                            normalized_reprocess == "true" ||
                            normalized_reprocess == "yes")
                        {
                            current_set_state()[normalized_name] = "AUTOMATIC";
                        }
                        else
                        {
                            current_set_state()[normalized_name] = uppercase_copy(reprocess_value);
                        }
                    }
                    else if (normalized_name == "hours")
                    {
                        std::string hours_value = evaluate_set_string_value(option_value, "24");
                        current_set_state()[normalized_name] = normalize_identifier(hours_value) == "12" ? std::string{"12"} : std::string{"24"};
                    }
                    else if (normalized_name == "fdow" || normalized_name == "fweek")
                    {
                        const int max_value = normalized_name == "fdow" ? 7 : 3;
                        const int parsed_value = evaluate_set_integer_value(option_value, 1, 1, max_value);
                        current_set_state()[normalized_name] = std::to_string(parsed_value);
                    }
                    else if (normalized_name == "decimals")
                    {
                        current_set_state()[normalized_name] = std::to_string(evaluate_set_integer_value(option_value, 2, 0, 18));
                    }
                    else if (normalized_name == "epoch")
                    {
                        current_set_state()[normalized_name] = std::to_string(evaluate_set_integer_value(option_value, 1950, 1, 9999));
                    }
                    else if (normalized_name == "date")
                    {
                        const std::string date_value = evaluate_set_string_value(option_value, "MDY");
                        current_set_state()[normalized_name] = uppercase_copy(date_value.empty() ? std::string{"MDY"} : date_value);
                    }
                    else if (normalized_name == "mark")
                    {
                        std::string mark_value = evaluate_set_string_value(option_value, "/");
                        current_set_state()[normalized_name] = mark_value.empty() ? std::string{"/"} : mark_value;
                    }
                    else if (normalized_name == "point" || normalized_name == "separator" || normalized_name == "currency")
                    {
                        const std::string fallback = normalized_name == "point" ? std::string{"."} : (normalized_name == "separator" ? std::string{","} : std::string{"$"});
                        std::string symbol_value = evaluate_set_string_value(option_value, fallback);
                        current_set_state()[normalized_name] = symbol_value.empty()
                                                                  ? fallback
                                                                  : symbol_value;
                    }
                    else if (normalized_name == "path" || normalized_name == "collate")
                    {
                        std::string string_value = evaluate_set_string_value(option_value, normalized_name == "collate" ? "MACHINE" : "");
                        current_set_state()[normalized_name] = normalized_name == "collate" ? uppercase_copy(string_value) : string_value;
                    }
                    else if (normalized_name == "fields")
                    {
                        std::string fields_value;
                        if (const auto expanded_macro = expand_set_text_macro(option_value); expanded_macro.has_value() && !expanded_macro->empty())
                        {
                            fields_value = *expanded_macro;
                        }
                        else
                        {
                            fields_value = evaluate_set_string_value(option_value, "ALL");
                        }
                        const std::string normalized_fields_value = normalize_identifier(fields_value);
                        if (normalized_fields_value == "off")
                        {
                            current_set_state()["fields_enabled"] = "off";
                        }
                        else if (normalized_fields_value == "on")
                        {
                            current_set_state()["fields_enabled"] = "on";
                        }
                        else
                        {
                            current_set_state()["fields"] = uppercase_copy(fields_value.empty() ? std::string{"ALL"} : fields_value);
                            current_set_state()["fields_enabled"] = "on";
                        }
                    }
                    else
                    {
                        current_set_state()[normalized_name] = option_value.empty() ? "on" : option_value;
                    }
                }
                else
                {
                    current_set_state()[normalize_identifier(statement.expression)] = "true";
                }
                events.push_back({.category = "runtime.set",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_library:
            {
                const auto library_value = resumed_set_library_value.has_value()
                                                ? resumed_set_library_value
                                                : evaluate_resumable_expression(frame, statement);
                if (!library_value.has_value())
                {
                    return {};
                }
                const std::string library_name = normalize_identifier(value_as_string(*library_value));
                resumed_set_library_value.reset();
                if (!library_name.empty())
                {
                    loaded_libraries.insert(library_name);
                }
                events.push_back({.category = "runtime.library",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_procedure:
            {
                std::string target = trim_copy(statement.expression);
                if (!target.empty() && target.front() == '&')
                {
                    const auto target_value = resumed_set_procedure_target_value.has_value()
                                                  ? resumed_set_procedure_target_value
                                                  : evaluate_resumable_expression(frame, statement);
                    if (!target_value.has_value())
                    {
                        return {};
                    }
                    const std::string expanded_target = trim_copy(value_as_string(*target_value));
                    resumed_set_procedure_target_value.reset();
                    if (!expanded_target.empty())
                    {
                        target = expanded_target;
                    }
                }

                if (target.empty())
                {
                    procedure_program_paths.clear();
                    events.push_back({.category = "runtime.procedure",
                                      .detail = "clear",
                                      .location = statement.location});
                    return {};
                }

                const std::string resolved_program_path =
                    normalize_path(resolve_procedure_program_path(target, frame.file_path));
                std::error_code exists_error;
                if (resolved_program_path.empty() ||
                    !std::filesystem::exists(resolved_program_path, exists_error))
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed",
                        {
                            {"command", "SET PROCEDURE TO"},
                            {"target", unquote_string(trim_copy(target))}
                        });
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const bool additive = normalize_identifier(statement.secondary_expression) == "additive";
                if (!additive)
                {
                    procedure_program_paths.clear();
                }

                load_program(resolved_program_path);
                if (std::find(procedure_program_paths.begin(),
                              procedure_program_paths.end(),
                              resolved_program_path) == procedure_program_paths.end())
                {
                    procedure_program_paths.push_back(resolved_program_path);
                }

                events.push_back({.category = "runtime.procedure",
                                  .detail = additive ? "add:" + resolved_program_path : "set:" + resolved_program_path,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::declare_dll:
            {
#if defined(_WIN32)
                // statement.identifier       = function name (export)
                // statement.expression       = dll_path
                // statement.secondary_expression = return type
                // statement.tertiary_expression  = param types
                // statement.quaternary_expression = alias (optional)
                const std::string fn_name = trim_copy(statement.identifier);
                const std::string library_expression = trim_copy(statement.expression);
                const bool is_win32api_designator = normalize_identifier(library_expression) == "win32api";
                const auto dll_path_value = is_win32api_designator
                                                ? std::optional<PrgValue>{make_string_value("WIN32API")}
                                                : (resumed_declare_dll_path_value.has_value()
                                                       ? resumed_declare_dll_path_value
                                                       : evaluate_resumable_expression(frame, statement));
                if (!dll_path_value.has_value())
                {
                    return {};
                }
                const std::string dll_path_raw = is_win32api_designator
                                                     ? std::string("WIN32API")
                                                     : unquote_string(trim_copy(value_as_string(*dll_path_value)));
                const std::string ret_type = uppercase_copy(trim_copy(statement.secondary_expression));
                const std::string param_types_str = trim_copy(statement.tertiary_expression);
                const std::string alias_raw = trim_copy(statement.quaternary_expression);
                const std::string alias = alias_raw.empty() ? fn_name : alias_raw;
                const std::string alias_key = normalize_identifier(alias);

                if (fn_name.empty() || dll_path_raw.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.DeclareMissingFunctionNameOrDllPath");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (declared_dll_parameter_list_contains_type(param_types_str, "short"))
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.DeclareUnsupportedParameterType",
                        {{"parameterType", "SHORT"}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    if (dispatch_error_handler())
                        return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                    return {.ok = false, .message = last_error_message};
                }

                // Preserve parentless names for the Windows loader search policy. VFP-style
                // explicit relative paths still resolve from the current default directory.
                std::filesystem::path dll_fspath;
                if (!is_win32api_designator)
                {
                    dll_fspath = copperfin::platform::path_from_utf8_string(dll_path_raw);
                    if (dll_fspath.is_relative() && dll_fspath.has_parent_path())
                    {
                        dll_fspath = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                            dll_fspath;
                    }
                    dll_fspath.make_preferred();
                }
                const std::wstring dll_wpath = dll_fspath.wstring();

                DeclaredDllFunction declfn;
                declfn.alias = alias;
                declfn.function_name = fn_name;
                declfn.dll_path = is_win32api_designator
                    ? dll_path_raw
                    : copperfin::platform::path_to_utf8_string(dll_fspath);
                declfn.return_type = ret_type;
                declfn.param_types = param_types_str;
                declfn.resolved_function_name = fn_name;

                const auto resolve_native_export = [&](HMODULE module, bool allow_ansi_fallback) -> FARPROC
                {
                    const std::array<std::string, 2U> export_names{fn_name, fn_name + "A"};
                    const std::size_t export_name_count = allow_ansi_fallback ? export_names.size() : 1U;
                    for (std::size_t export_index = 0U; export_index < export_name_count; ++export_index)
                    {
                        const std::string &export_name = export_names[export_index];
                        FARPROC procedure = GetProcAddress(module, export_name.c_str());
                        if (procedure != nullptr)
                        {
                            declfn.native_cdecl = false;
                            declfn.resolved_function_name = export_name;
                            return procedure;
                        }

                        procedure = GetProcAddress(module, ("_" + export_name).c_str());
                        if (procedure != nullptr)
                        {
#if !defined(_WIN64)
                            declfn.native_cdecl = true;
#endif
                            declfn.resolved_function_name = export_name;
                            return procedure;
                        }
#if !defined(_WIN64)
                        const std::string stack_suffix = "@" + std::to_string(
                            declared_dll_x86_stdcall_stack_bytes(param_types_str));
                        procedure = GetProcAddress(
                            module,
                            ("_" + export_name + stack_suffix).c_str());
                        if (procedure == nullptr)
                        {
                            procedure = GetProcAddress(
                                module,
                                (export_name + stack_suffix).c_str());
                        }
                        if (procedure != nullptr)
                        {
                            declfn.native_cdecl = false;
                            declfn.resolved_function_name = export_name;
                            return procedure;
                        }
#endif
                    }
                    return nullptr;
                };

                const auto loaded_module_path = [](HMODULE module, const std::wstring &fallback_name)
                {
                    std::wstring module_path(32768U, L'\0');
                    const DWORD length = GetModuleFileNameW(
                        module,
                        module_path.data(),
                        static_cast<DWORD>(module_path.size()));
                    if (length > 0U && length < module_path.size())
                    {
                        module_path.resize(length);
                        return module_path;
                    }
                    return fallback_name;
                };

                const auto retain_resolved_module_identity = [&](const std::filesystem::path &module_path)
                {
                    const std::u8string utf8_path = module_path.u8string();
                    declfn.loaded_module_path.assign(
                        reinterpret_cast<const char *>(utf8_path.data()),
                        utf8_path.size());
                };

                const auto retain_loaded_module_identity = [&](HMODULE module, const std::wstring &fallback_name)
                {
                    retain_resolved_module_identity(loaded_module_path(module, fallback_name));
                };

                HMODULE hmod = nullptr;
                if (is_win32api_designator)
                {
                    constexpr std::array<const wchar_t *, 5U> win32api_modules{
                        L"Kernel32.dll",
                        L"Gdi32.dll",
                        L"User32.dll",
                        L"Mpr.dll",
                        L"Advapi32.dll",
                    };
                    std::wstring system_directory_buffer(32768U, L'\0');
                    const UINT system_directory_length = GetSystemDirectoryW(
                        system_directory_buffer.data(),
                        static_cast<UINT>(system_directory_buffer.size()));
                    if (system_directory_length > 0U &&
                        system_directory_length < system_directory_buffer.size())
                    {
                        system_directory_buffer.resize(system_directory_length);
                        const std::filesystem::path system_directory(system_directory_buffer);
                        for (const wchar_t *module_name : win32api_modules)
                        {
                            const std::filesystem::path module_path = system_directory / module_name;
                            HMODULE candidate = LoadLibraryW(module_path.c_str());
                            if (candidate == nullptr)
                            {
                                continue;
                            }
                            FARPROC procedure = resolve_native_export(candidate, true);
                            if (procedure != nullptr)
                            {
                                hmod = candidate;
                                declfn.hmodule = candidate;
                                declfn.proc_address = procedure;
                                retain_loaded_module_identity(candidate, module_path.wstring());
                                break;
                            }
                            FreeLibrary(candidate);
                        }
                    }

                    if (hmod == nullptr)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.DeclareFunctionNotFoundInDll",
                            {
                                {"functionName", fn_name},
                                {"path", declfn.dll_path},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        if (dispatch_error_handler())
                            return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                        return {.ok = false, .message = last_error_message};
                    }

                }
                else
                {
                    hmod = LoadLibraryW(dll_wpath.c_str());
                    const DWORD load_error = hmod == nullptr ? GetLastError() : ERROR_SUCCESS;

                    std::filesystem::path inspection_path;
                    if (hmod != nullptr)
                    {
                        inspection_path = loaded_module_path(hmod, dll_wpath);
                    }
                    else if (dll_fspath.is_absolute() || dll_fspath.has_parent_path())
                    {
                        inspection_path = dll_fspath;
                    }
                    else
                    {
                        std::wstring search_result(32768U, L'\0');
                        const DWORD search_length = SearchPathW(
                            nullptr,
                            dll_wpath.c_str(),
                            nullptr,
                            static_cast<DWORD>(search_result.size()),
                            search_result.data(),
                            nullptr);
                        if (search_length > 0U && search_length < search_result.size())
                        {
                            search_result.resize(search_length);
                            inspection_path = search_result;
                        }
                    }
                    const bool is_dotnet_assembly =
                        !inspection_path.empty() &&
                        inspect_portable_executable(inspection_path) == PortableExecutableKind::managed;
                    FARPROC native_procedure = hmod == nullptr ? nullptr : resolve_native_export(hmod, true);

                    if (native_procedure != nullptr)
                    {
                        declfn.hmodule = hmod;
                        declfn.proc_address = native_procedure;
                        retain_loaded_module_identity(hmod, dll_wpath);
                    }
                    else if (is_dotnet_assembly)
                    {
                        if (hmod != nullptr)
                        {
                            FreeLibrary(hmod);
                            hmod = nullptr;
                        }
                        declfn.is_dotnet = true;
                        retain_resolved_module_identity(inspection_path);
                        const auto last_dot = fn_name.rfind('.');
                        if (last_dot != std::string::npos && last_dot > 0U)
                        {
                            declfn.dotnet_type_name = fn_name.substr(0U, last_dot);
                            declfn.dotnet_method_name = fn_name.substr(last_dot + 1U);
                        }
                        else
                        {
                            declfn.dotnet_type_name = std::string{};
                            declfn.dotnet_method_name = fn_name;
                        }
                    }
                    else if (hmod == nullptr)
                    {
                        char msg_buf[256]{};
                        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                                       load_error, 0, msg_buf, sizeof(msg_buf) - 1U, nullptr);
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.DeclareCannotLoadDll",
                            {
                                {"path", declfn.dll_path},
                                {"errorMessage", std::string(msg_buf)},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        if (dispatch_error_handler())
                            return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                        return {.ok = false, .message = last_error_message};
                    }
                    else
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.DeclareFunctionNotFoundInDll",
                            {
                                {"functionName", fn_name},
                                {"path", declfn.dll_path},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        FreeLibrary(hmod);
                        if (dispatch_error_handler())
                            return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                        return {.ok = false, .message = last_error_message};
                    }
                }

                if (const auto existing = declared_dll_functions.find(alias_key);
                    existing != declared_dll_functions.end() && existing->second.hmodule != nullptr)
                {
                    FreeLibrary(existing->second.hmodule);
                }
                declared_dll_functions.insert_or_assign(alias_key, std::move(declfn));
                events.push_back({.category = "runtime.declare_dll",
                                  .detail = alias + " IN " + dll_path_raw,
                                  .location = statement.location});
                return {};
#else
                last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.DeclareDllOnlySupportedOnWindows");
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                if (dispatch_error_handler())
                    return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                return {.ok = false, .message = last_error_message};
#endif
            }
            case StatementKind::set_datasession:
            {
                const auto session_value = resumed_set_datasession_value.has_value()
                                                ? resumed_set_datasession_value
                                                : evaluate_resumable_expression(frame, statement);
                if (!session_value.has_value())
                {
                    return {};
                }
                const int session_id = static_cast<int>(std::llround(value_as_number(*session_value)));
                resumed_set_datasession_value.reset();
                current_data_session = std::max(1, session_id);
                (void)current_session_state();
                events.push_back({.category = "runtime.datasession",
                                  .detail = "SET DATASESSION TO " + std::to_string(current_data_session),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_default:
            {
                const auto path_value = resumed_set_default_path_value.has_value()
                                            ? resumed_set_default_path_value
                                            : evaluate_resumable_expression(frame, statement);
                if (!path_value.has_value())
                {
                    return {};
                }
                const std::string evaluated = value_as_string(*path_value);
                resumed_set_default_path_value.reset();
                if (!evaluated.empty())
                {
                    current_default_directory() = normalize_path(evaluated);
                }
                return {};
            }
            case StatementKind::set_memowidth:
            {
                const auto width_expression_value = resumed_set_memowidth_value.has_value()
                                                        ? resumed_set_memowidth_value
                                                        : evaluate_resumable_expression(frame, statement);
                if (!width_expression_value.has_value())
                {
                    return {};
                }
                const double width_value = value_as_number(*width_expression_value);
                resumed_set_memowidth_value.reset();
                const std::size_t new_width = static_cast<std::size_t>(std::max(1.0, std::min(32767.0, width_value)));
                memowidth_by_session[current_data_session] = new_width;
                return {};
            }
            case StatementKind::on_error:
                error_handler = statement.expression;
                return {};
            case StatementKind::on_shutdown:
                shutdown_handler = statement.expression;
                return {};
            case StatementKind::on_shutdown_statement:
                shutdown_handler = statement.expression;
                return {};
            case StatementKind::public_declaration:
                if (statement.identifier == "array")
                {
                    std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>> declarations;
                    declarations.reserve(statement.names.size());
                    for (const auto &declaration : statement.names)
                    {
                        std::string array_name;
                        std::size_t rows = 0U;
                        std::size_t columns = 1U;
                        if (!parse_array_reference(declaration, frame, array_name, rows, columns))
                        {
                            last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions");
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        const std::string normalized = normalize_memory_variable_identifier(array_name);
                        if (public_declaration_conflicts(normalized, true))
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.IllegalVariableRedefinition",
                                {{"variableName", normalized}});
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        declarations.push_back({normalized, {rows, columns}});
                    }
                    for (const auto &[normalized, dimensions] : declarations)
                    {
                        public_names.insert(normalized);
                        arrays[normalized] = RuntimeArray{
                            .rows = dimensions.first,
                            .columns = dimensions.second,
                            .values = std::vector<PrgValue>(dimensions.first * dimensions.second, make_boolean_value(false))};
                    }
                    return {};
                }
                for (const auto &name : statement.names)
                {
                    const std::string normalized = normalize_memory_variable_identifier(name);
                    if (!normalized.empty() && public_declaration_conflicts(normalized, false))
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.IllegalVariableRedefinition",
                            {{"variableName", normalized}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                for (const auto &name : statement.names)
                {
                    const std::string normalized = normalize_memory_variable_identifier(name);
                    if (normalized.empty())
                    {
                        continue;
                    }
                    public_names.insert(normalized);
                    globals.try_emplace(normalized, make_boolean_value(false));
                }
                return {};
            case StatementKind::local_declaration:
                if (statement.identifier == "array")
                {
                    for (const auto &declaration : statement.names)
                    {
                        std::string array_name;
                        std::size_t rows = 0U;
                        std::size_t columns = 1U;
                        if (!parse_array_reference(declaration, frame, array_name, rows, columns))
                        {
                            last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions");
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        frame.local_arrays[normalize_memory_variable_identifier(array_name)] = RuntimeArray{
                            .rows = rows,
                            .columns = columns,
                            .values = std::vector<PrgValue>(rows * columns, make_boolean_value(false))};
                    }
                    return {};
                }
                for (const auto &name : statement.names)
                {
                    const std::string normalized = normalize_memory_variable_identifier(name);
                    frame.local_names.insert(normalized);
                    frame.locals.try_emplace(normalized, make_boolean_value(false));
                }
                return {};
            case StatementKind::private_declaration:
            {
                if (statement.identifier == "array")
                {
                    for (const auto &declaration : statement.names)
                    {
                        std::string array_name;
                        std::size_t rows = 0U;
                        std::size_t columns = 1U;
                        if (!parse_array_reference(declaration, frame, array_name, rows, columns))
                        {
                            last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions");
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        const std::string normalized = normalize_memory_variable_identifier(array_name);
                        const auto existing_array = arrays.find(normalized);
                        frame.private_saved_arrays.try_emplace(
                            normalized,
                            existing_array == arrays.end()
                                ? std::optional<RuntimeArray>{std::nullopt}
                                : std::optional<RuntimeArray>{existing_array->second});
                        if (existing_array != arrays.end())
                        {
                            arrays.erase(existing_array);
                        }
                        arrays[normalized] = RuntimeArray{
                            .rows = rows,
                            .columns = columns,
                            .values = std::vector<PrgValue>(rows * columns, make_boolean_value(false))};
                    }
                    return {};
                }
                const auto privatize_name = [&](const std::string &raw_name, bool create_if_missing)
                {
                    const std::string normalized = normalize_memory_variable_identifier(raw_name);
                    if (normalized.empty())
                    {
                        return;
                    }
                    const auto existing = globals.find(normalized);
                    const auto existing_array = arrays.find(normalized);
                    if (!create_if_missing && existing == globals.end() && existing_array == arrays.end())
                    {
                        return;
                    }
                    if (existing != globals.end())
                    {
                        frame.private_saved_values.try_emplace(normalized, existing->second);
                        existing->second = make_empty_value();
                    }
                    else
                    {
                        frame.private_saved_values.try_emplace(normalized, std::nullopt);
                        if (create_if_missing)
                        {
                            globals[normalized] = make_empty_value();
                        }
                    }
                    if (existing_array != arrays.end())
                    {
                        frame.private_saved_arrays.try_emplace(normalized, existing_array->second);
                        arrays.erase(existing_array);
                    }
                    else
                    {
                        frame.private_saved_arrays.try_emplace(normalized, std::nullopt);
                    }
                };

                if (statement.identifier == "all")
                {
                    const std::string mode = statement.expression;
                    const std::string pattern = statement.secondary_expression;
                    std::set<std::string> candidate_names;
                    for (const auto &[name, _] : globals)
                    {
                        candidate_names.insert(name);
                    }
                    for (const auto &[name, _] : arrays)
                    {
                        candidate_names.insert(name);
                    }
                    for (const std::string &name : candidate_names)
                    {
                        const bool matches = wildcard_match_insensitive(pattern, name);
                        if (mode.empty() || (mode == "like" && matches) || (mode == "except" && !matches))
                        {
                            privatize_name(name, false);
                        }
                    }
                }
                else
                {
                    for (const auto &name : statement.names)
                    {
                        privatize_name(name, true);
                    }
                }
                return {};
            }
            case StatementKind::parameters_declaration:
            case StatementKind::lparameters_declaration:
            {
                const auto split_parameter_default = [](const std::string &raw_declaration)
                {
                    std::string parameter_name = trim_copy(raw_declaration);
                    std::string default_expression;
                    char quote_delimiter = '\0';
                    std::size_t paren_depth = 0U;
                    std::size_t bracket_depth = 0U;
                    std::size_t brace_depth = 0U;
                    for (std::size_t index = 0U; index < raw_declaration.size(); ++index)
                    {
                        const char ch = raw_declaration[index];
                        if (quote_delimiter != '\0')
                        {
                            if (ch == quote_delimiter)
                            {
                                if ((index + 1U) < raw_declaration.size() && raw_declaration[index + 1U] == quote_delimiter)
                                {
                                    ++index;
                                    continue;
                                }
                                quote_delimiter = '\0';
                            }
                            continue;
                        }
                        if (ch == '\'' || ch == '"')
                        {
                            quote_delimiter = ch;
                            continue;
                        }
                        if (ch == '(')
                        {
                            ++paren_depth;
                            continue;
                        }
                        if (ch == ')' && paren_depth > 0U)
                        {
                            --paren_depth;
                            continue;
                        }
                        if (ch == '[')
                        {
                            ++bracket_depth;
                            continue;
                        }
                        if (ch == ']' && bracket_depth > 0U)
                        {
                            --bracket_depth;
                            continue;
                        }
                        if (ch == '{')
                        {
                            ++brace_depth;
                            continue;
                        }
                        if (ch == '}' && brace_depth > 0U)
                        {
                            --brace_depth;
                            continue;
                        }
                        if (ch == '=' && paren_depth == 0U && bracket_depth == 0U && brace_depth == 0U)
                        {
                            parameter_name = trim_copy(raw_declaration.substr(0U, index));
                            default_expression = trim_copy(raw_declaration.substr(index + 1U));
                            break;
                        }
                    }
                    return std::pair<std::string, std::string>{parameter_name, default_expression};
                };
                if (!frame.parameter_default_continuation.has_value() ||
                    frame.parameter_default_continuation->statement.text != statement.text)
                {
                    frame.parameter_default_continuation = ParameterDefaultContinuation{.statement = statement};
                }

                ParameterDefaultContinuation &continuation = *frame.parameter_default_continuation;
                while (continuation.next_parameter_index < statement.names.size())
                {
                    const std::size_t index = continuation.next_parameter_index;
                    const auto [parameter_name, default_expression] = split_parameter_default(statement.names[index]);
                    const std::string normalized = normalize_memory_variable_identifier(parameter_name);
                    if (normalized.empty())
                    {
                        ++continuation.next_parameter_index;
                        continue;
                    }

                    frame.local_names.insert(normalized);
                    if (index < frame.call_arguments.size())
                    {
                        frame.locals[normalized] = frame.call_arguments[index];
                    }
                    else if (default_expression.empty())
                    {
                        frame.locals[normalized] = make_boolean_value(false);
                    }
                    else
                    {
                        Statement default_statement = continuation.statement;
                        default_statement.expression = default_expression;
                        default_statement.text = continuation.statement.text + " [parameter-default]";
                        continuation.pending_default = true;
                        const auto default_value = resumed_parameter_default_value.has_value()
                                                       ? resumed_parameter_default_value
                                                       : evaluate_resumable_expression(frame, default_statement);
                        if (!default_value.has_value())
                        {
                            return {};
                        }
                        frame.locals[normalized] = *default_value;
                        continuation.pending_default = false;
                        resumed_parameter_default_value.reset();
                    }

                    if (index < frame.call_argument_references.size() && frame.call_argument_references[index].has_value())
                    {
                        const std::string &reference_name = *frame.call_argument_references[index];
                        Frame *caller = stack.size() >= 2U ? &stack[stack.size() - 2U] : nullptr;
                        if (caller != nullptr && is_array_copy_reference(reference_name))
                        {
                            const RuntimeArray *source_array = find_array(array_copy_source_name(reference_name), *caller);
                            if (source_array != nullptr)
                            {
                                frame.locals.erase(normalized);
                                frame.local_arrays[normalized] = *source_array;
                            }
                        }
                        else if (caller != nullptr && find_array(reference_name, *caller) != nullptr)
                        {
                            frame.array_reference_bindings[normalized] = canonical_array_name(reference_name, *caller);
                        }
                        else
                        {
                            frame.parameter_reference_bindings[normalized] = reference_name;
                        }
                    }
                    ++continuation.next_parameter_index;
                }
                frame.parameter_default_continuation.reset();
                return {};
            }
            case StatementKind::dimension_command:
            {
                for (const auto &name : statement.names)
                {
                    if (!declare_array(name, frame))
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                events.push_back({.category = "runtime.dimension",
                                  .detail = std::to_string(statement.names.size()) + " array(s)",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::store_command:
            {
                const auto result = evaluate_resumable_expression(frame, statement);
                if (!result.has_value())
                {
                    return {};
                }
                for (const auto &name : statement.names)
                {
                    ExecutionOutcome outcome = assign_runtime_target_value(trim_copy(name), *result);
                    if (!outcome.ok)
                    {
                        return outcome;
                    }
                }
                return {};
            }
            case StatementKind::close_command:
            {
                close_runtime_scope(statement.expression.empty() ? std::string{"ALL"} : statement.expression,
                                    statement.location);
                return {};
            }
            case StatementKind::erase_command:
            {
                // ERASE <file> / DELETE FILE <file>
                const auto path_value = resumed_erase_path_value.has_value()
                                            ? resumed_erase_path_value
                                            : evaluate_resumable_expression(frame, statement);
                if (!path_value.has_value())
                {
                    return {};
                }
                const std::string raw_path = unquote_string(trim_copy(value_as_string(*path_value)));
                std::filesystem::path fpath = copperfin::platform::path_from_utf8_string(raw_path);
                if (fpath.is_relative())
                {
                    fpath = copperfin::platform::path_from_utf8_string(current_default_directory()) / fpath;
                }
                std::error_code ec;
                std::filesystem::remove(fpath, ec);
                if (ec)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.EraseFailed",
                        {{"errorMessage", ec.message()},
                         {"path", copperfin::platform::path_to_utf8_string(fpath)}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.erase",
                                  .detail = copperfin::platform::path_to_utf8_string(fpath),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::copy_file_command:
            {
                // COPY FILE <src> TO <dest>
                if (!frame.copy_file_continuation.has_value() ||
                    frame.copy_file_continuation->statement.text != statement.text)
                {
                    frame.copy_file_continuation = CopyFileContinuation{
                        .statement = statement,
                        .source_value = std::nullopt,
                        .pending_destination = false};
                }
                CopyFileContinuation &continuation = *frame.copy_file_continuation;
                if (!continuation.source_value.has_value())
                {
                    const auto source_value = resumed_copy_source_value.has_value()
                                                  ? resumed_copy_source_value
                                                  : evaluate_resumable_expression(frame, statement);
                    if (!source_value.has_value())
                    {
                        return {};
                    }
                    continuation.source_value = *source_value;
                    resumed_copy_source_value.reset();
                }

                Statement destination_statement = continuation.statement;
                destination_statement.expression = statement.secondary_expression;
                destination_statement.text = statement.text + " [copy-destination]";
                continuation.pending_destination = true;
                const auto destination_value = resumed_copy_destination_value.has_value()
                                                   ? resumed_copy_destination_value
                                                   : evaluate_resumable_expression(frame, destination_statement);
                if (!destination_value.has_value())
                {
                    return {};
                }
                const std::string src_raw = unquote_string(trim_copy(
                    value_as_string(*continuation.source_value)));
                const std::string dst_raw = unquote_string(trim_copy(
                    value_as_string(*destination_value)));
                frame.copy_file_continuation.reset();
                resumed_copy_destination_value.reset();
                auto make_abs = [&](const std::string &raw)
                {
                    std::filesystem::path p = copperfin::platform::path_from_utf8_string(raw);
                    if (p.is_relative())
                    {
                        p = copperfin::platform::path_from_utf8_string(current_default_directory()) / p;
                    }
                    return p;
                };
                const std::filesystem::path src = make_abs(src_raw);
                const std::filesystem::path dst = make_abs(dst_raw);
                std::error_code ec;
                std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
                if (ec)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CopyFileFailed",
                        {{"errorMessage", ec.message()}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.copy_file",
                                  .detail = copperfin::platform::path_to_utf8_string(src) + " -> " +
                                      copperfin::platform::path_to_utf8_string(dst),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::rename_file_command:
            {
                // RENAME <old> TO <new>
                if (!frame.rename_file_continuation.has_value() ||
                    frame.rename_file_continuation->statement.text != statement.text)
                {
                    frame.rename_file_continuation = RenameFileContinuation{
                        .statement = statement,
                        .source_value = std::nullopt,
                        .pending_destination = false};
                }
                RenameFileContinuation &continuation = *frame.rename_file_continuation;
                if (!continuation.source_value.has_value())
                {
                    const auto source_value = resumed_rename_source_value.has_value()
                                                  ? resumed_rename_source_value
                                                  : evaluate_resumable_expression(frame, statement);
                    if (!source_value.has_value())
                    {
                        return {};
                    }
                    continuation.source_value = *source_value;
                    resumed_rename_source_value.reset();
                }

                Statement destination_statement = continuation.statement;
                destination_statement.expression = statement.secondary_expression;
                destination_statement.text = statement.text + " [rename-destination]";
                continuation.pending_destination = true;
                const auto destination_value = resumed_rename_destination_value.has_value()
                                                   ? resumed_rename_destination_value
                                                   : evaluate_resumable_expression(frame, destination_statement);
                if (!destination_value.has_value())
                {
                    return {};
                }
                const std::string old_raw = unquote_string(trim_copy(
                    value_as_string(*continuation.source_value)));
                const std::string new_raw = unquote_string(trim_copy(
                    value_as_string(*destination_value)));
                frame.rename_file_continuation.reset();
                resumed_rename_destination_value.reset();
                auto make_abs = [&](const std::string &raw)
                {
                    std::filesystem::path p = copperfin::platform::path_from_utf8_string(raw);
                    if (p.is_relative())
                    {
                        p = copperfin::platform::path_from_utf8_string(current_default_directory()) / p;
                    }
                    return p;
                };
                const std::filesystem::path old_path = make_abs(old_raw);
                const std::filesystem::path new_path = make_abs(new_raw);
                if (old_path.lexically_normal() != new_path.lexically_normal())
                {
                    std::error_code exists_error;
                    if (std::filesystem::exists(new_path, exists_error))
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.RenameFileTargetExists",
                            {{"path", copperfin::platform::path_to_utf8_string(new_path)}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (exists_error)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.RenameFileFailed",
                            {{"errorMessage", exists_error.message()}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                std::error_code ec;
                std::filesystem::rename(old_path, new_path, ec);
                if (ec)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.RenameFileFailed",
                        {{"errorMessage", ec.message()}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.rename",
                                  .detail = copperfin::platform::path_to_utf8_string(old_path) + " -> " +
                                      copperfin::platform::path_to_utf8_string(new_path),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::print_command:
            {
                // ? or ?? expression — evaluate and emit as output event
                const auto result = evaluate_resumable_expression(frame, statement);
                if (!result.has_value())
                {
                    return {};
                }
                emit_print_event(*result, statement.location);
                return {};
            }
            case StatementKind::create_cursor_command:
            {
                const std::string alias_expression = statement.secondary_expression.empty()
                                                         ? statement.identifier
                                                         : statement.secondary_expression;
                std::string alias = alias_expression.empty()
                                        ? "CURSOR1"
                                        : normalize_identifier(unquote_identifier(trim_copy(alias_expression)));
                if (alias.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CreateCursorRequiresNonEmptyAlias");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto declarations = parse_table_field_declarations(statement.expression);
                const std::vector<vfp::DbfFieldDescriptor> fields = table_field_descriptors(declarations);
                if (fields.empty())
                {
                    last_error_message =
                        runtime_text("Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::error_code ignored;
                const std::filesystem::path cursor_root = runtime_temp_directory / "cursors";
                std::filesystem::create_directories(cursor_root, ignored);

                std::filesystem::path table_path;
                for (std::size_t attempt = 0U;; ++attempt)
                {
                    const std::string suffix = attempt == 0U ? std::string{} : "_" + std::to_string(attempt + 1U);
                    table_path = cursor_root /
                                 (alias + "_ds" + std::to_string(current_data_session) + suffix + ".dbf");
                    if (!std::filesystem::exists(table_path, ignored))
                    {
                        break;
                    }
                }

                if (!ensure_transaction_backup_for_table(copperfin::platform::path_to_utf8_string(table_path)))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto create_result = vfp::create_dbf_table_file(
                    copperfin::platform::path_to_utf8_string(table_path), fields, {});
                if (!create_result.ok)
                {
                    last_error_message = create_result.error;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto field_rules = field_rules_from_declarations(declarations);
                if (!open_table_cursor(copperfin::platform::path_to_utf8_string(table_path), alias, {}, true, false, 0, {}, 0U, field_rules))
                {
                    std::filesystem::remove(table_path, ignored);
                    std::filesystem::remove(table_path.replace_extension(".fpt"), ignored);
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.create_cursor",
                                  .detail = alias + " -> " + copperfin::platform::path_to_utf8_string(table_path),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::create_table_command:
            {
                std::string target = trim_copy(statement.identifier);
                if (target.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CreateTableRequiresTargetTableName");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if ((target.size() >= 2U && target.front() == '\'' && target.back() == '\'') ||
                    (target.size() >= 2U && target.front() == '"' && target.back() == '"'))
                {
                    target = value_as_string(evaluate_expression(target, frame));
                }
                else
                {
                    target = unquote_string(target);
                }

                std::filesystem::path table_path = copperfin::platform::path_from_utf8_string(target);
                if (table_path.extension().empty())
                {
                    table_path += ".dbf";
                }
                if (table_path.is_relative())
                {
                    table_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                        table_path;
                }
                table_path = table_path.lexically_normal();

                std::vector<TableFieldDeclaration> declarations;
                if (statement.tertiary_expression == "array")
                {
                    const auto resolved_array_name = resolve_command_array_name(
                        statement.secondary_expression, "CREATE TABLE FROM ARRAY");
                    if (!resolved_array_name.has_value() && frame.command_array_name_continuation.has_value())
                    {
                        return {};
                    }
                    const RuntimeArray *metadata_array = resolved_array_name.has_value()
                        ? find_array(*resolved_array_name)
                        : nullptr;
                    if (metadata_array == nullptr || metadata_array->columns < 4U || metadata_array->rows == 0U)
                    {
                        if (resolved_array_name.has_value())
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.CreateTableRequiresSupportedFieldDeclaration");
                        }
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto read_dimension = [&](const PrgValue &value, std::uint8_t &result) {
                        const double number = value_as_number(value);
                        if (!std::isfinite(number) || number < 0.0 || number > 255.0 ||
                            std::llround(number) != number)
                        {
                            return false;
                        }
                        result = static_cast<std::uint8_t>(std::llround(number));
                        return true;
                    };
                    for (std::size_t row = 1U; row <= metadata_array->rows; ++row)
                    {
                        const std::string name = trim_copy(value_as_string(
                            array_value(*resolved_array_name, row, 1U)));
                        const std::string type = uppercase_copy(trim_copy(value_as_string(
                            array_value(*resolved_array_name, row, 2U))));
                        std::uint8_t length = 0U;
                        std::uint8_t decimals = 0U;
                        if (name.empty() || type.empty() ||
                            !read_dimension(array_value(*resolved_array_name, row, 3U), length) ||
                            !read_dimension(array_value(*resolved_array_name, row, 4U), decimals))
                        {
                            declarations.clear();
                            break;
                        }

                        const std::string declaration_text = name + " " + type + "(" +
                            std::to_string(length) + "," + std::to_string(decimals) + ")";
                        const auto declaration = parse_table_field_declaration(declaration_text);
                        if (!declaration.has_value())
                        {
                            declarations.clear();
                            break;
                        }
                        declarations.push_back(*declaration);
                    }
                }
                else
                {
                    declarations = parse_table_field_declarations(statement.expression);
                }
                const std::vector<vfp::DbfFieldDescriptor> fields = table_field_descriptors(declarations);
                if (fields.empty())
                {
                    last_error_message =
                        runtime_text("Runtime.Prg.Dispatch.Error.CreateTableRequiresSupportedFieldDeclaration");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!execute_with_command_undo(copperfin::platform::path_to_utf8_string(table_path), "CREATE TABLE", [&]
                    {
                        if (!ensure_transaction_backup_for_table(copperfin::platform::path_to_utf8_string(table_path)))
                        {
                            return false;
                        }

                        const auto create_result = vfp::create_dbf_table_file(
                            copperfin::platform::path_to_utf8_string(table_path), fields, {});
                        if (!create_result.ok)
                        {
                            last_error_message = create_result.error;
                            return false;
                        }

                        const std::string alias = normalize_identifier(
                            copperfin::platform::path_to_utf8_string(table_path.stem()));
                        const auto field_rules = field_rules_from_declarations(declarations);
                        if (!open_table_cursor(copperfin::platform::path_to_utf8_string(table_path), alias, {}, true, false, 0, {}, 0U, field_rules))
                        {
                            return false;
                        }
                        return true;
                    }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.create_table",
                                  .detail = copperfin::platform::path_to_utf8_string(table_path),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::alter_table_command:
            {
                const std::string action = normalize_identifier(statement.secondary_expression);
                if (action != "add" && action != "drop" && action != "alter")
                {
                    last_error_message =
                        runtime_text("Runtime.Prg.Dispatch.Error.AlterTableSupportsAddDropAlterColumnOnly");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string target = trim_copy(statement.identifier);
                if (target.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.AlterTableRequiresTargetTableName");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if ((target.size() >= 2U && target.front() == '\'' && target.back() == '\'') ||
                    (target.size() >= 2U && target.front() == '"' && target.back() == '"'))
                {
                    target = value_as_string(evaluate_expression(target, frame));
                }
                else
                {
                    target = unquote_string(target);
                }

                std::filesystem::path table_path = copperfin::platform::path_from_utf8_string(target);
                if (table_path.extension().empty())
                {
                    table_path += ".dbf";
                }
                if (table_path.is_relative())
                {
                    table_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                        table_path;
                }
                table_path = table_path.lexically_normal();
                std::string affected_field = trim_copy(statement.expression);

                // Verified package DBFs are immutable admissions until the runtime has
                // an exact-object mutation/publication path. Do not reopen and mutate
                // the logical path under strict verification.
                if (options.require_verified_file_byte_overrides)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.VerifiedAlterTableMutationUnsupported",
                        { {"path", copperfin::platform::path_to_utf8_string(table_path)} });
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!execute_with_command_undo(copperfin::platform::path_to_utf8_string(table_path), "ALTER TABLE", [&]
                {
                    if (!ensure_transaction_backup_for_table(copperfin::platform::path_to_utf8_string(table_path)))
                    {
                        return false;
                    }

                    vfp::DbfWriteResult add_result;
                    std::optional<TableFieldDeclaration> declaration;
                    if (action == "add" || action == "alter")
                    {
                        declaration = parse_table_field_declaration(statement.expression);
                        if (!declaration.has_value())
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
                                {{"command", action == "add"
                                                 ? "ALTER TABLE ADD COLUMN"
                                                 : "ALTER TABLE ALTER COLUMN"}});
                            return false;
                        }
                        affected_field = declaration->descriptor.name;
                        add_result = action == "add"
                                         ? vfp::add_dbf_table_field(copperfin::platform::path_to_utf8_string(table_path), declaration->descriptor)
                                         : vfp::alter_dbf_table_field(copperfin::platform::path_to_utf8_string(table_path), declaration->descriptor);
                    }
                    else
                    {
                        affected_field = unquote_identifier(affected_field);
                        add_result = vfp::drop_dbf_table_field(
                            copperfin::platform::path_to_utf8_string(table_path), affected_field);
                    }
                    if (!add_result.ok)
                    {
                        last_error_message = add_result.error;
                        return false;
                    }

                    if (action == "add" && declaration.has_value() && declaration->has_default)
                    {
                        const vfp::DbfRecordValue synthetic_field{
                            .field_name = declaration->descriptor.name,
                            .field_type = declaration->descriptor.type,
                            .is_null = false,
                            .display_value = {}};
                        for (std::size_t record_index = 0U; record_index < add_result.record_count; ++record_index)
                        {
                            const PrgValue default_value = evaluate_expression(declaration->default_expression, frame);
                            const auto replace_result = vfp::replace_record_field_value(
                                copperfin::platform::path_to_utf8_string(table_path),
                                record_index,
                                declaration->descriptor.name,
                                serialize_prg_value_for_record_field(synthetic_field, default_value));
                            if (!replace_result.ok)
                            {
                                last_error_message = replace_result.error;
                                return false;
                            }
                        }
                    }

                    const auto schema_result = vfp::parse_dbf_table_from_file(
                        copperfin::platform::path_to_utf8_string(table_path), 0U);
                    if (!schema_result.ok)
                    {
                        last_error_message = schema_result.error;
                        return false;
                    }

                    for (auto &[_, cursor] : current_session_state().cursors)
                    {
                        if (!cursor.remote && normalize_path(cursor.source_path) ==
                            normalize_path(copperfin::platform::path_to_utf8_string(table_path)))
                        {
                            cursor.field_count = schema_result.table.fields.size();
                            cursor.record_length = schema_result.table.header.record_length;
                            cursor.local_fields = schema_result.table.fields;
                            cursor.record_count = add_result.record_count;
                            const std::string normalized_field = collapse_identifier(affected_field);
                            if (action == "drop")
                            {
                                cursor.field_rules.erase(normalized_field);
                            }
                            else if (declaration.has_value())
                            {
                                if (!declaration->nullable || declaration->has_default)
                                {
                                    cursor.field_rules[normalized_field] = CursorState::FieldRule{
                                        .nullable = declaration->nullable,
                                        .has_default = declaration->has_default,
                                        .default_expression = declaration->default_expression};
                                }
                                else
                                {
                                    cursor.field_rules.erase(normalized_field);
                                }
                            }
                        }
                    }

                    return true;
                }))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.alter_table",
                                  .detail = copperfin::platform::path_to_utf8_string(table_path) + " " +
                                      uppercase_copy(action) + " " + affected_field,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::save_memvars_command:
            {
                namespace fs = std::filesystem;
                const auto escape_memvar_value = [](const std::string &raw) {
                    std::string escaped;
                    escaped.reserve(raw.size());
                    for (const char ch : raw)
                    {
                        switch (ch)
                        {
                        case '\\':
                            escaped += "\\\\";
                            break;
                        case '\n':
                            escaped += "\\n";
                            break;
                        case '\r':
                            escaped += "\\r";
                            break;
                        case '\t':
                            escaped += "\\t";
                            break;
                        case '=':
                            escaped += "\\=";
                            break;
                        case ':':
                            escaped += "\\:";
                            break;
                        case ';':
                            escaped += "\\;";
                            break;
                        case '|':
                            escaped += "\\|";
                            break;
                        case ',':
                            escaped += "\\,";
                            break;
                        default:
                            escaped.push_back(ch);
                            break;
                        }
                    }
                    return escaped;
                };
                const auto serialize_memvar_value = [&](const PrgValue &value) {
                    std::pair<char, std::string> serialized{'C', std::string{}};
                    switch (value.kind)
                    {
                    case PrgValueKind::boolean:
                        serialized.first = 'L';
                        serialized.second = value.boolean_value ? "true" : "false";
                        break;
                    case PrgValueKind::number:
                    case PrgValueKind::int64:
                    case PrgValueKind::uint64:
                        serialized.first = 'N';
                        serialized.second = value_as_string(value);
                        break;
                    case PrgValueKind::currency:
                        serialized.first = 'Y';
                        serialized.second = value_as_string(value);
                        break;
                    case PrgValueKind::string:
                    {
                        int year = 0;
                        int month = 0;
                        int day = 0;
                        serialized.first = parse_runtime_date_string(value.string_value, year, month, day) ? 'D' : 'C';
                        serialized.second = value.string_value;
                        break;
                    }
                    case PrgValueKind::empty:
                        serialized.first = 'E';
                        serialized.second.clear();
                        break;
                    }
                    return serialized;
                };

                const auto destination_value = resumed_save_memvars_path_value.has_value()
                                                   ? resumed_save_memvars_path_value
                                                   : evaluate_resumable_expression(frame, statement);
                if (!destination_value.has_value())
                {
                    return {};
                }
                std::string destination = unquote_string(trim_copy(
                    value_as_string(*destination_value)));
                resumed_save_memvars_path_value.reset();
                if (destination.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.SaveToFilenameRequired");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                fs::path destination_path(destination);
                if (destination_path.extension().empty())
                {
                    destination_path += ".mem";
                }
                if (destination_path.is_relative())
                {
                    destination_path = fs::path(current_default_directory()) / destination_path;
                }
                destination_path = destination_path.lexically_normal();

                std::string filter_mode;
                std::string filter_pattern;
                if (starts_with_insensitive(statement.identifier, "LIKE:"))
                {
                    filter_mode = "like";
                    filter_pattern = trim_copy(statement.identifier.substr(5U));
                }
                else if (starts_with_insensitive(statement.identifier, "EXCEPT:"))
                {
                    filter_mode = "except";
                    filter_pattern = trim_copy(statement.identifier.substr(7U));
                }

                std::map<std::string, PrgValue> visible_variables = globals;
                for (const auto &[name, value] : frame.locals)
                {
                    visible_variables[name] = value;
                }

                const auto visible_binding_is_public = [&](const std::string &name) -> bool
                {
                    if (frame.locals.contains(name) || frame.local_names.contains(name))
                    {
                        return false;
                    }
                    if (frame.private_saved_values.contains(name))
                    {
                        return false;
                    }
                    return public_names.contains(name);
                };

                if (!destination_path.parent_path().empty())
                {
                    std::error_code ignored;
                    fs::create_directories(destination_path.parent_path(), ignored);
                }

                std::ofstream output(destination_path, std::ios::binary);
                if (!output.good())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.SaveToOpenFailed");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::size_t saved_count = 0U;
                std::set<std::string> saved_names;
                for (const auto &[name, value] : visible_variables)
                {
                    if (arrays.contains(name))
                    {
                        continue;
                    }

                    bool include_variable = true;
                    if (!filter_mode.empty())
                    {
                        const bool matches = wildcard_match_insensitive(filter_pattern, name);
                        include_variable = filter_mode == "like" ? matches : !matches;
                    }
                    if (!include_variable)
                    {
                        continue;
                    }

                    const auto [type_code, serialized_value] = serialize_memvar_value(value);
                    std::string raw_type(1U, type_code);
                    if (visible_binding_is_public(name))
                    {
                        raw_type += ",PUBLIC";
                    }

                    output << name << "=" << raw_type << ":" << escape_memvar_value(serialized_value) << "\n";
                    ++saved_count;
                    saved_names.insert(name);
                }

                for (const auto &[name, array] : arrays)
                {
                    if (saved_names.contains(name))
                    {
                        continue;
                    }

                    bool include_variable = true;
                    if (!filter_mode.empty())
                    {
                        const bool matches = wildcard_match_insensitive(filter_pattern, name);
                        include_variable = filter_mode == "like" ? matches : !matches;
                    }
                    if (!include_variable)
                    {
                        continue;
                    }

                    std::ostringstream array_payload;
                    array_payload << array.rows << "," << array.columns;
                    for (const auto &element : array.values)
                    {
                        const auto [element_type, element_value] = serialize_memvar_value(element);
                        array_payload << "|" << element_type << ":" << escape_memvar_value(element_value);
                    }

                    std::string raw_type = "A";
                    if (visible_binding_is_public(name))
                    {
                        raw_type += ",PUBLIC";
                    }
                    output << name << "=" << raw_type << ":" << array_payload.str() << "\n";
                    ++saved_count;
                }

                output.close();
                if (!output.good())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.SaveToWriteFailed");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.save_memory",
                                  .detail = copperfin::platform::path_to_utf8_string(destination_path) +
                                      " (" + std::to_string(saved_count) + " variables)",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::restore_memvars_command:
            {
                namespace fs = std::filesystem;
                const auto unescape_memvar_value = [](const std::string &encoded) {
                    std::string unescaped;
                    unescaped.reserve(encoded.size());
                    for (std::size_t index = 0U; index < encoded.size(); ++index)
                    {
                        const char ch = encoded[index];
                        if (ch != '\\')
                        {
                            unescaped.push_back(ch);
                            continue;
                        }

                        if (index + 1U >= encoded.size())
                        {
                            unescaped.push_back('\\');
                            continue;
                        }

                        const char next = encoded[++index];
                        switch (next)
                        {
                        case '\\':
                            unescaped.push_back('\\');
                            break;
                        case 'n':
                            unescaped.push_back('\n');
                            break;
                        case 'r':
                            unescaped.push_back('\r');
                            break;
                        case 't':
                            unescaped.push_back('\t');
                            break;
                        case '=':
                            unescaped.push_back('=');
                            break;
                        case ':':
                            unescaped.push_back(':');
                            break;
                        case ';':
                            unescaped.push_back(';');
                            break;
                        case '|':
                            unescaped.push_back('|');
                            break;
                        case ',':
                            unescaped.push_back(',');
                            break;
                        default:
                            unescaped.push_back(next);
                            break;
                        }
                    }
                    return unescaped;
                };
                const auto split_escaped_memvar_list = [](const std::string &encoded, char delimiter) {
                    std::vector<std::string> parts;
                    std::string current;
                    current.reserve(encoded.size());
                    bool escaped = false;
                    for (const char ch : encoded)
                    {
                        if (escaped)
                        {
                            current.push_back('\\');
                            current.push_back(ch);
                            escaped = false;
                            continue;
                        }
                        if (ch == '\\')
                        {
                            escaped = true;
                            continue;
                        }
                        if (ch == delimiter)
                        {
                            parts.push_back(current);
                            current.clear();
                            continue;
                        }
                        current.push_back(ch);
                    }
                    if (escaped)
                    {
                        current.push_back('\\');
                    }
                    parts.push_back(current);
                    return parts;
                };

                const auto source_value = resumed_restore_memvars_path_value.has_value()
                                              ? resumed_restore_memvars_path_value
                                              : evaluate_resumable_expression(frame, statement);
                if (!source_value.has_value())
                {
                    return {};
                }
                std::string source = unquote_string(trim_copy(
                    value_as_string(*source_value)));
                resumed_restore_memvars_path_value.reset();
                if (source.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.RestoreFromFilenameRequired");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                fs::path source_path = copperfin::platform::path_from_utf8_string(source);
                if (source_path.extension().empty())
                {
                    source_path += ".mem";
                }
                if (source_path.is_relative())
                {
                    source_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                        source_path;
                }
                source_path = source_path.lexically_normal();

                std::ifstream file_input;
                std::istringstream verified_input;
                std::istream *input = nullptr;
                if (options.require_verified_file_byte_overrides)
                {
                    const auto verified = find_verified_file_byte_override(source_path);
                    if (verified == options.verified_file_byte_overrides.end() || verified->second.empty())
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    verified_input.str(verified->second);
                    input = &verified_input;
                }
                else
                {
                    file_input.open(source_path, std::ios::binary);
                    if (!file_input.good())
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    input = &file_input;
                }

                if (input == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const bool additive = normalize_identifier(statement.identifier) == "additive";
                if (!additive)
                {
                    globals.clear();
                    arrays.clear();
                    public_names.clear();
                    for (auto &active_frame : stack)
                    {
                        active_frame.private_saved_values.clear();
                        active_frame.private_saved_arrays.clear();
                        active_frame.locals.clear();
                    }
                }

                std::size_t restored_count = 0U;
                std::string line;
                while (std::getline(*input, line))
                {
                    if (!line.empty() && line.back() == '\r')
                    {
                        line.pop_back();
                    }
                    if (line.empty())
                    {
                        continue;
                    }

                    const std::size_t equals_position = line.find('=');
                    if (equals_position == std::string::npos)
                    {
                        continue;
                    }
                    const std::size_t colon_position = line.find(':', equals_position + 1U);
                    if (colon_position == std::string::npos)
                    {
                        continue;
                    }

                    const std::string name = normalize_memory_variable_identifier(line.substr(0U, equals_position));
                    if (name.empty())
                    {
                        continue;
                    }

                    const std::string raw_type = trim_copy(line.substr(equals_position + 1U, colon_position - equals_position - 1U));
                    const std::vector<std::string> type_tokens = split_csv_like(raw_type);
                    const std::string type_name = type_tokens.empty() ? std::string{} : trim_copy(type_tokens.front());
                    const char type_code = type_name.empty()
                                               ? 'C'
                                               : static_cast<char>(std::toupper(static_cast<unsigned char>(type_name.front())));
                    bool restore_public = false;
                    for (std::size_t token_index = 1U; token_index < type_tokens.size(); ++token_index)
                    {
                        if (normalize_identifier(type_tokens[token_index]) == "public")
                        {
                            restore_public = true;
                        }
                    }
                    const std::string raw_value = unescape_memvar_value(line.substr(colon_position + 1U));

                    const auto parse_memvar_value = [&](char value_type_code, const std::string &value_text) {
                        PrgValue restored_value;
                        if (value_type_code == 'L')
                        {
                            const std::string normalized_bool = normalize_identifier(value_text);
                            const bool bool_value = normalized_bool == "true" ||
                                                    normalized_bool == ".t." ||
                                                    normalized_bool == "t" ||
                                                    normalized_bool == "1" ||
                                                    normalized_bool == "y" ||
                                                    normalized_bool == "yes";
                            restored_value = make_boolean_value(bool_value);
                        }
                        else if (value_type_code == 'N')
                        {
                            const std::string numeric_text = trim_copy(value_text);
                            char *number_end = nullptr;
                            const double parsed = std::strtod(numeric_text.c_str(), &number_end);
                            restored_value = (number_end != numeric_text.c_str() && number_end != nullptr && *number_end == '\0')
                                                 ? make_number_value(parsed)
                                                 : make_number_value(0.0);
                        }
                        else if (value_type_code == 'Y')
                        {
                            try
                            {
                                restored_value = make_currency_value(
                                    static_cast<std::int64_t>(std::llround(std::stod(trim_copy(value_text)) * 10000.0)));
                            }
                            catch (...)
                            {
                                restored_value = make_currency_value(0);
                            }
                        }
                        else if (value_type_code == 'D')
                        {
                            restored_value = make_string_value(trim_copy(value_text));
                        }
                        else if (value_type_code == 'E')
                        {
                            restored_value = make_empty_value();
                        }
                        else
                        {
                            restored_value = make_string_value(value_text);
                        }
                        return restored_value;
                    };

                    if (type_code == 'A')
                    {
                        const std::string encoded_array_value = line.substr(colon_position + 1U);
                        const std::vector<std::string> array_tokens = split_escaped_memvar_list(encoded_array_value, '|');
                        if (array_tokens.empty())
                        {
                            continue;
                        }
                        const std::vector<std::string> size_tokens = split_csv_like(array_tokens.front());
                        const std::size_t rows = size_tokens.empty() ? 0U : static_cast<std::size_t>(std::max(0, std::stoi(trim_copy(size_tokens[0]))));
                        const std::size_t columns = size_tokens.size() >= 2U ? static_cast<std::size_t>(std::max(1, std::stoi(trim_copy(size_tokens[1])))) : 1U;
                        std::vector<PrgValue> values;
                        values.reserve(array_tokens.size() > 0U ? array_tokens.size() - 1U : 0U);
                        for (std::size_t element_index = 1U; element_index < array_tokens.size(); ++element_index)
                        {
                            const std::string encoded_element = array_tokens[element_index];
                            const std::size_t element_colon = encoded_element.find(':');
                            const char element_type = element_colon == std::string::npos || encoded_element.empty()
                                                          ? 'C'
                                                          : static_cast<char>(std::toupper(static_cast<unsigned char>(encoded_element.front())));
                            const std::string element_text = element_colon == std::string::npos ? std::string{} : unescape_memvar_value(encoded_element.substr(element_colon + 1U));
                            values.push_back(parse_memvar_value(element_type, element_text));
                        }
                        values.resize(rows * columns);
                        assign_array(name, std::move(values), columns);
                    }
                    else
                    {
                        const PrgValue restored_value = parse_memvar_value(type_code, raw_value);
                        if (frame.local_names.contains(name) || frame.locals.contains(name))
                        {
                            frame.locals[name] = restored_value;
                        }
                        else
                        {
                            globals[name] = restored_value;
                        }
                    }

                    if (restore_public)
                    {
                        public_names.insert(name);
                    }
                    ++restored_count;
                }

                events.push_back({.category = "runtime.restore_memory",
                                  .detail = copperfin::platform::path_to_utf8_string(source_path) +
                                      " (" + std::to_string(restored_count) + " variables)",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::copy_to_command:
            {
                // COPY TO ARRAY <array> [FIELDS <list>] [FOR <expr>]
                if (statement.identifier == "array")
                {
                    const auto resolved_array_name = resolve_command_array_name(statement.expression, "COPY TO ARRAY");
                    if (!resolved_array_name.has_value())
                    {
                        if (frame.command_array_name_continuation.has_value())
                        {
                            return {};
                        }
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    const std::string array_name = *resolved_array_name;
                    CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    if (cursor == nullptr)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CopyToArrayNoCurrentWorkArea");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    // Determine field set (optionally filtered)
                    const std::vector<std::string> field_filter = parse_field_filter_clause(statement.tertiary_expression);
                    const std::string for_expr = statement.quaternary_expression;
                    // Gather field descriptors for column order from cursor schema
                    const std::vector<vfp::DbfFieldDescriptor> source_fields = cursor_field_descriptors(*cursor);
                    const std::vector<vfp::DbfFieldDescriptor> selected_fields =
                        filter_field_descriptors(source_fields, field_filter, true);
                    std::vector<std::string> col_names;
                    col_names.reserve(selected_fields.size());
                    for (const auto &field : selected_fields)
                    {
                        col_names.push_back(field.name);
                    }
                    const std::size_t num_cols = col_names.empty() ? 1U : col_names.size();
                    std::vector<PrgValue> flat_values;
                    const CursorPositionSnapshot saved = capture_cursor_snapshot(*cursor);
                    for (const std::size_t recno : record_iteration_order(*cursor))
                    {
                        move_cursor_to(*cursor, static_cast<long long>(recno));
                        if (!current_record_matches_visibility(*cursor, frame, for_expr))
                        {
                            continue;
                        }
                        const auto rec = current_record(*cursor);
                        if (!rec.has_value())
                        {
                            continue;
                        }
                        for (const auto &col : col_names)
                        {
                            const auto it = std::find_if(
                                rec->values.begin(), rec->values.end(),
                                [&](const vfp::DbfRecordValue &rv)
                                {
                                    return collapse_identifier(rv.field_name) == collapse_identifier(col);
                                });
                            flat_values.push_back(
                                it != rec->values.end()
                                    ? record_value_to_prg_value(*it)
                                    : make_empty_value());
                        }
                    }
                    restore_cursor_snapshot(*cursor, saved);
                    assign_array(array_name, std::move(flat_values), num_cols);
                    events.push_back({.category = "runtime.copy_to_array",
                                      .detail = array_name,
                                      .location = statement.location});
                    return {};
                }

                // COPY TO <dest> [TYPE <type>] [FIELDS <list>] [FOR <expr>]
                // COPY STRUCTURE TO <dest> — copies schema only (no rows)
                // COPY STRUCTURE EXTENDED TO <dest> — emits VFP structure metadata rows
                const bool is_structure_extended = (statement.identifier == "structure_extended");
                const bool is_structure = (statement.identifier == "structure") || is_structure_extended;
                const auto destination_value = resumed_copy_to_destination_value.has_value()
                                                   ? resumed_copy_to_destination_value
                                                   : evaluate_resumable_expression(frame, statement);
                if (!destination_value.has_value())
                {
                    return {};
                }
                const std::string dest_raw = unquote_string(trim_copy(
                    value_as_string(*destination_value)));
                resumed_copy_to_destination_value.reset();
                const std::string copy_type = normalize_identifier(unquote_string(trim_copy(statement.secondary_expression)));
                const bool copy_as_json = copy_type == "json";
                const bool copy_as_sdf = copy_type == "sdf";
                const bool copy_as_dif = copy_type == "dif";
                const bool copy_as_sylk = copy_type == "sylk";
                const bool copy_as_tab = copy_type == "tab";
                const bool copy_as_xls = copy_type == "xls";
                const bool copy_as_delimited = copy_type == "csv" || copy_type == "delimited" || copy_as_tab;
                const std::string with_clause = statement.names.empty() ? std::string{} : statement.names.front();

                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CopyToNoCurrentWorkArea");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                namespace fs = std::filesystem;
                fs::path dest_path(dest_raw);
                if (dest_path.extension().empty())
                {
                    if (copy_as_xls)
                    {
                        dest_path += ".xls";
                    }
                    else if (copy_as_json)
                    {
                        dest_path += ".json";
                    }
                    else if (copy_as_sylk)
                    {
                        dest_path += ".slk";
                    }
                    else if (copy_as_dif)
                    {
                        dest_path += ".dif";
                    }
                    else if (copy_as_sdf || copy_type == "delimited" || copy_as_tab)
                    {
                        dest_path += ".txt";
                    }
                    else if (copy_type == "csv")
                    {
                        dest_path += ".csv";
                    }
                    else
                    {
                        dest_path += ".dbf";
                    }
                }
                if (dest_path.is_relative())
                {
                    dest_path = fs::path(current_default_directory()) / dest_path;
                }
                dest_path = dest_path.lexically_normal();

                std::vector<vfp::DbfFieldDescriptor> source_fields = cursor_field_descriptors(*cursor);
                if (source_fields.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CopyToSourceCursorSchemaUnavailable");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                // Build field filter from FIELDS clause (comma-separated names)
                const std::string fields_clause = statement.tertiary_expression;
                const std::vector<std::string> field_filter = parse_field_filter_clause(fields_clause);

                // Filter descriptors by FIELDS clause
                std::vector<vfp::DbfFieldDescriptor> out_fields =
                    filter_field_descriptors(source_fields, field_filter, true);
                if (out_fields.empty())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CopyToNoFieldsMatchFieldsClause");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (is_structure_extended)
                {
                    const std::vector<vfp::DbfFieldDescriptor> structure_fields{
                        {.name = "FIELD_NAME", .type = 'C', .length = 128U},
                        {.name = "FIELD_TYPE", .type = 'C', .length = 1U},
                        {.name = "FIELD_LEN", .type = 'N', .length = 3U},
                        {.name = "FIELD_DEC", .type = 'N', .length = 3U},
                        {.name = "FIELD_NULL", .type = 'L', .length = 1U},
                        {.name = "FIELD_NOCP", .type = 'L', .length = 1U},
                        {.name = "FIELD_DEFA", .type = 'M', .length = 4U},
                        {.name = "FIELD_RULE", .type = 'M', .length = 4U},
                        {.name = "FIELD_ERR", .type = 'M', .length = 4U},
                        {.name = "TABLE_RULE", .type = 'M', .length = 4U},
                        {.name = "TABLE_ERR", .type = 'M', .length = 4U},
                        {.name = "TABLE_NAME", .type = 'C', .length = 128U},
                        {.name = "INS_TRIG", .type = 'M', .length = 4U},
                        {.name = "UPD_TRIG", .type = 'M', .length = 4U},
                        {.name = "DEL_TRIG", .type = 'M', .length = 4U},
                        {.name = "TABLE_CMT", .type = 'M', .length = 4U}};
                    std::vector<std::vector<std::string>> structure_rows;
                    structure_rows.reserve(out_fields.size());
                    for (const auto &field : out_fields)
                    {
                        structure_rows.push_back({
                            field.name.substr(0U, std::min<std::size_t>(field.name.size(), 128U)),
                            std::string(1U, field.type),
                            std::to_string(field.length),
                            std::to_string(field.decimal_count),
                            "F",
                            "F",
                            {},
                            {},
                            {},
                            {},
                            {},
                            {},
                            {},
                            {},
                            {},
                            {}});
                    }

                    const auto write_result = vfp::create_dbf_table_file(
                        copperfin::platform::path_to_utf8_string(dest_path), structure_fields, structure_rows);
                    if (!write_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToWriteFailed",
                            {{"errorMessage", write_result.error}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    events.push_back({.category = "runtime.copy_to",
                                      .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                      .location = statement.location});
                    return {};
                }

                // Collect qualifying rows (skip for COPY STRUCTURE TO)
                const std::string for_expr = statement.quaternary_expression;
                std::vector<std::vector<std::string>> out_rows;
                if (!is_structure)
                {
                    const CursorPositionSnapshot saved = capture_cursor_snapshot(*cursor);
                    for (const std::size_t recno : record_iteration_order(*cursor))
                    {
                        move_cursor_to(*cursor, static_cast<long long>(recno));
                        if (!current_record_matches_visibility(*cursor, frame, for_expr))
                        {
                            continue;
                        }
                        const auto rec = current_record(*cursor);
                        if (!rec.has_value())
                        {
                            continue;
                        }
                        std::vector<std::string> row;
                        row.reserve(out_fields.size());
                        for (const auto &desc : out_fields)
                        {
                            const auto it = std::find_if(
                                rec->values.begin(), rec->values.end(),
                                [&](const vfp::DbfRecordValue &rv)
                                {
                                    return collapse_identifier(rv.field_name) == collapse_identifier(desc.name);
                                });
                            row.push_back(it != rec->values.end() ? it->display_value : std::string{});
                        }
                        out_rows.push_back(std::move(row));
                    }
                    restore_cursor_snapshot(*cursor, saved);
                }

                if (copy_as_sdf)
                {
                    if (!dest_path.parent_path().empty())
                    {
                        std::error_code ignored;
                        fs::create_directories(dest_path.parent_path(), ignored);
                    }
                    std::ofstream output(dest_path, std::ios::binary);
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
                            {{"type", "SDF"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    for (const auto &row : out_rows)
                    {
                        for (std::size_t index = 0U; index < out_fields.size(); ++index)
                        {
                            output << format_sdf_field_value(out_fields[index], index < row.size() ? row[index] : std::string{});
                        }
                        output << "\r\n";
                    }
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
                            {{"type", "SDF"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                      .location = statement.location});
                    return {};
                }

                if (copy_as_json)
                {
                    if (!dest_path.parent_path().empty())
                    {
                        std::error_code ignored;
                        fs::create_directories(dest_path.parent_path(), ignored);
                    }
                    std::ofstream output(dest_path, std::ios::binary);
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
                            {{"type", "JSON"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_json_records(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
                            {{"type", "JSON"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                      .location = statement.location});
                    return {};
                }

                if (copy_as_dif)
                {
                    if (!dest_path.parent_path().empty())
                    {
                        std::error_code ignored;
                        fs::create_directories(dest_path.parent_path(), ignored);
                    }
                    std::ofstream output(dest_path, std::ios::binary);
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
                            {{"type", "DIF"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_dif_table(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
                            {{"type", "DIF"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                      .location = statement.location});
                    return {};
                }

                if (copy_as_sylk)
                {
                    if (!dest_path.parent_path().empty())
                    {
                        std::error_code ignored;
                        fs::create_directories(dest_path.parent_path(), ignored);
                    }
                    std::ofstream output(dest_path, std::ios::binary);
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
                            {{"type", "SYLK"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_sylk_table(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
                            {{"type", "SYLK"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                      .location = statement.location});
                    return {};
                }

                if (copy_as_xls)
                {
                    if (!dest_path.parent_path().empty())
                    {
                        std::error_code ignored;
                        fs::create_directories(dest_path.parent_path(), ignored);
                    }
                    std::ofstream output(dest_path, std::ios::binary);
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
                            {{"type", "XLS"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_spreadsheetml_workbook(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
                            {{"type", "XLS"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                      .location = statement.location});
                    return {};
                }

                if (copy_as_delimited)
                {
                    if (!dest_path.parent_path().empty())
                    {
                        std::error_code ignored;
                        fs::create_directories(dest_path.parent_path(), ignored);
                    }
                    std::ofstream output(dest_path, std::ios::binary);
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
                            {{"type", "DELIMITED"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    const DelimitedTextOptions delimited_options = parse_delimited_text_options(copy_type, with_clause);
                    if (copy_type == "csv")
                    {
                        for (std::size_t index = 0U; index < out_fields.size(); ++index)
                        {
                            if (index != 0U)
                            {
                                output << delimited_options.delimiter;
                            }
                            output << out_fields[index].name;
                        }
                        output << "\r\n";
                    }
                    for (const auto &row : out_rows)
                    {
                        for (std::size_t index = 0U; index < out_fields.size(); ++index)
                        {
                            if (index != 0U)
                            {
                                output << delimited_options.delimiter;
                            }
                            output << format_delimited_field_value(
                                out_fields[index],
                                index < row.size() ? row[index] : std::string{},
                                delimited_options);
                        }
                        output << "\r\n";
                    }
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
                            {{"type", "DELIMITED"}});
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                      .location = statement.location});
                    return {};
                }

                // Cursor schemas can carry logical names longer than a free DBF's
                // 10-byte physical-name limit (notably ODBC metadata cursors).
                // Preserve already-valid names, then shorten and disambiguate only
                // the physical descriptors passed to the DBF writer.
                std::set<std::string> assigned_physical_names;
                for (const auto &field : out_fields)
                {
                    if (field.name.size() <= 10U)
                    {
                        assigned_physical_names.insert(collapse_identifier(field.name));
                    }
                }

                std::vector<vfp::DbfFieldDescriptor> physical_out_fields = out_fields;
                for (auto &field : physical_out_fields)
                {
                    if (field.name.size() <= 10U)
                    {
                        continue;
                    }

                    const std::string logical_name = field.name;
                    std::string candidate = logical_name.substr(0U, 10U);
                    for (std::size_t suffix_number = 2U;
                         assigned_physical_names.contains(collapse_identifier(candidate));
                         ++suffix_number)
                    {
                        const std::string suffix = "_" + std::to_string(suffix_number);
                        candidate = logical_name.substr(0U, 10U - std::min<std::size_t>(10U, suffix.size())) + suffix;
                    }
                    field.name = candidate;
                    assigned_physical_names.insert(collapse_identifier(candidate));
                }

                const auto write_result = vfp::create_dbf_table_file(
                    copperfin::platform::path_to_utf8_string(dest_path), physical_out_fields, out_rows);
                if (!write_result.ok)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.CopyToWriteFailed",
                        {{"errorMessage", write_result.error}});
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.copy_to",
                                  .detail = copperfin::platform::path_to_utf8_string(dest_path),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::append_from_command:
            {
                // APPEND FROM ARRAY <array> [FIELDS <list>]
                if (statement.identifier == "array")
                {
                    const auto resolved_array_name = resolve_command_array_name(statement.expression, "APPEND FROM ARRAY");
                    if (!resolved_array_name.has_value())
                    {
                        if (frame.command_array_name_continuation.has_value())
                        {
                            return {};
                        }
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    const std::string array_name = *resolved_array_name;
                    CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    if (cursor == nullptr || cursor->source_path.empty())
                    {
                        events.push_back({.category = "runtime.append_from_array",
                                          .detail = array_name,
                                          .location = statement.location});
                        return {};
                    }

                    std::size_t appended_count = 0U;
                    const CursorPositionSnapshot original_position = capture_cursor_snapshot(*cursor);
                    const std::size_t original_record_count = cursor->record_count;
                    const bool appended = execute_with_command_undo(cursor->source_path, "APPEND FROM ARRAY", [&]
                    {
                        // Determine dest fields order (filtered by FIELDS clause)
                        const std::vector<std::string> field_filter =
                            parse_field_filter_clause(statement.tertiary_expression);
                        const auto dest_result = parse_table_path(
                            cursor->source_path, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                        if (!dest_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromArrayFailed",
                                {
                                    {"errorMessage", dest_result.error},
                                });
                            return false;
                        }
                        std::vector<vfp::DbfFieldDescriptor> target_fields =
                            filter_field_descriptors(dest_result.table.fields, field_filter, true);
                        if (target_fields.empty())
                        {
                            last_error_message =
                                runtime_text("Runtime.Prg.Dispatch.Error.AppendFromArrayNoFieldsMatchFieldsClause");
                            return false;
                        }
                        if (!ensure_transaction_backup_for_table(cursor->source_path))
                        {
                            return false;
                        }
                        const std::size_t num_rows = array_length(array_name, 1);
                        const std::size_t num_cols = std::max<std::size_t>(1U, array_length(array_name, 2));
                        appended_count = 0U;
                        for (std::size_t row = 1U; row <= num_rows; ++row)
                        {
                            const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                            if (!blank_result.ok)
                            {
                                last_error_message = runtime_text(
                                    "Runtime.Prg.Dispatch.Error.AppendFromArrayFailed",
                                    {
                                        {"errorMessage", blank_result.error},
                                    });
                                return false;
                            }
                            cursor->record_count = blank_result.record_count;
                            cursor->eof = false;
                            cursor->recno = blank_result.record_count;
                            const std::size_t usable_cols = std::min(target_fields.size(), num_cols);
                            for (std::size_t col = 1U; col <= usable_cols; ++col)
                            {
                                const PrgValue val = array_value(array_name, row, col);
                                const vfp::DbfRecordValue synthetic_field{
                                    .field_name = target_fields[col - 1U].name,
                                    .field_type = target_fields[col - 1U].type,
                                    .is_null = false,
                                    .display_value = {}};
                                const auto rep_result = vfp::replace_record_field_value(
                                    cursor->source_path,
                                    cursor->recno - 1U,
                                    target_fields[col - 1U].name,
                                    serialize_prg_value_for_record_field(synthetic_field, val));
                                if (!rep_result.ok)
                                {
                                    last_error_message = runtime_text(
                                        "Runtime.Prg.Dispatch.Error.AppendFromArrayFailed",
                                        {
                                            {"errorMessage", rep_result.error},
                                        });
                                    return false;
                                }
                                cursor->record_count = rep_result.record_count;
                            }
                            ++appended_count;
                        }
                        return true;
                    });
                    if (!appended)
                    {
                        cursor->record_count = original_record_count;
                        restore_cursor_snapshot(*cursor, original_position);
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    events.push_back({.category = "runtime.append_from_array",
                                      .detail = array_name + " (" + std::to_string(appended_count) + " records)",
                                      .location = statement.location});
                    return {};
                }

                // APPEND FROM <src> [TYPE <type>] [FIELDS <list>] [FOR <expr>]
                // First pass: copy non-deleted records from source DBF into current local cursor.
                // Field matching is by name; extra fields in source that do not exist in destination are silently skipped.
                const auto source_value = resumed_append_from_source_value.has_value()
                                              ? resumed_append_from_source_value
                                              : evaluate_resumable_expression(frame, statement);
                if (!source_value.has_value())
                {
                    return {};
                }
                const std::string src_raw = unquote_string(trim_copy(
                    value_as_string(*source_value)));
                resumed_append_from_source_value.reset();
                const std::string append_type = normalize_identifier(unquote_string(trim_copy(statement.secondary_expression)));
                const bool append_from_json = append_type == "json";
                const bool append_from_sdf = append_type == "sdf";
                const bool append_from_dif = append_type == "dif";
                const bool append_from_sylk = append_type == "sylk";
                const bool append_from_tab = append_type == "tab";
                const bool append_from_xls = append_type == "xls";
                const bool append_from_delimited = append_type == "csv" || append_type == "delimited" || append_from_tab;
                const std::string with_clause = statement.names.empty() ? std::string{} : statement.names.front();

                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.AppendFromNoCurrentWorkArea");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                namespace fs = std::filesystem;
                fs::path src_path = copperfin::platform::path_from_utf8_string(src_raw);
                if (src_path.extension().empty())
                {
                    if (append_from_xls)
                    {
                        src_path += ".xls";
                    }
                    else if (append_from_json)
                    {
                        src_path += ".json";
                    }
                    else if (append_from_sylk)
                    {
                        src_path += ".slk";
                    }
                    else if (append_from_dif)
                    {
                        src_path += ".dif";
                    }
                    else if (append_from_sdf || append_type == "delimited" || append_from_tab)
                    {
                        src_path += ".txt";
                    }
                    else if (append_type == "csv")
                    {
                        src_path += ".csv";
                    }
                    else
                    {
                        src_path += ".dbf";
                    }
                }
                if (src_path.is_relative())
                {
                    src_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                        src_path;
                }
                src_path = src_path.lexically_normal();

                const auto read_append_source_bytes = [&](const fs::path &path,
                                                          const std::string &type,
                                                          std::string &bytes)
                {
                    if (options.require_verified_file_byte_overrides)
                    {
                        const auto verified = find_verified_file_byte_override(path);
                        if (verified == options.verified_file_byte_overrides.end() || verified->second.empty())
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed",
                                {{"type", type}});
                            return false;
                        }
                        bytes = verified->second;
                        return true;
                    }

                    std::ifstream input(path, std::ios::binary);
                    if (!input.good())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed",
                            {{"type", type}});
                        return false;
                    }
                    std::ostringstream buffer;
                    buffer << input.rdbuf();
                    bytes = buffer.str();
                    return true;
                };

                const std::string fields_clause = statement.tertiary_expression;
                const std::vector<std::string> field_filter = parse_field_filter_clause(fields_clause);

                if (cursor->remote && cursor->source_path.empty())
                {
                    if (append_from_sdf || append_from_dif || append_from_sylk ||
                        append_from_tab || append_from_xls)
                    {
                        last_error_message =
                            runtime_text("Runtime.Prg.Dispatch.Error.AppendFromSelectedSqlResultCursorUnsupportedSourceType");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    if (append_from_json)
                    {
                        std::string json_bytes;
                        if (!read_append_source_bytes(src_path, "JSON", json_bytes))
                        {
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }

                        std::vector<vfp::DbfFieldDescriptor> target_fields = cursor_field_descriptors(*cursor);
                        const std::string for_expr = statement.quaternary_expression;
                        std::vector<vfp::DbfFieldDescriptor> filtered_target_fields =
                            filter_field_descriptors(target_fields, field_filter, true);
                        if (filtered_target_fields.empty())
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                                {
                                    {"type", "JSON"},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }

                        const std::vector<std::map<std::string, std::string>> json_rows =
                            parse_json_record_objects(json_bytes);
                        std::size_t appended_count = 0U;
                        for (const auto &row : json_rows)
                        {
                            vfp::DbfRecord appended_record;
                            appended_record.record_index = cursor->remote_records.size();
                            appended_record.deleted = false;
                            appended_record.values.reserve(target_fields.size());

                            for (const auto &target_field : target_fields)
                            {
                                vfp::DbfRecordValue value{
                                    .field_name = target_field.name,
                                    .field_type = target_field.type,
                                    .is_null = false,
                                    .display_value = {}};

                                if (field_matches_filter(target_field.name, field_filter))
                                {
                                    const auto found = row.find(collapse_identifier(target_field.name));
                                    if (found != row.end())
                                    {
                                        value.display_value = found->second;
                                    }
                                }

                                appended_record.values.push_back(std::move(value));
                            }

                            cursor->remote_records.push_back(std::move(appended_record));
                            cursor->record_count = cursor->remote_records.size();
                            cursor->recno = cursor->record_count;
                            cursor->eof = false;
                            cursor->bof = cursor->record_count == 0U;

                            if (!trim_copy(for_expr).empty() && !current_record_matches_visibility(*cursor, frame, for_expr))
                            {
                                cursor->remote_records.pop_back();
                                cursor->record_count = cursor->remote_records.size();
                                continue;
                            }

                            ++appended_count;
                        }

                        if (cursor->remote_fields.empty())
                        {
                            cursor->remote_fields = target_fields;
                        }
                        cursor->record_count = cursor->remote_records.size();
                        cursor->eof = cursor->record_count == 0U;
                        cursor->bof = cursor->record_count == 0U;
                        cursor->found = false;
                        if (appended_count > 0U)
                        {
                            cursor->recno = cursor->record_count;
                            cursor->eof = false;
                            cursor->bof = false;
                        }

                        events.push_back({.category = "runtime.append_from",
                                          .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE JSON)",
                                          .location = statement.location});
                        return {};
                    }

                    if (append_from_delimited)
                    {
                        std::string csv_bytes;
                        if (!read_append_source_bytes(src_path, "DELIMITED", csv_bytes))
                        {
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }

                        std::vector<vfp::DbfFieldDescriptor> target_fields = cursor_field_descriptors(*cursor);
                        const std::string for_expr = statement.quaternary_expression;
                        std::vector<vfp::DbfFieldDescriptor> filtered_target_fields =
                            filter_field_descriptors(target_fields, field_filter, true);
                        if (filtered_target_fields.empty())
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                                {
                                    {"type", "DELIMITED"},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }

                        const DelimitedTextOptions delimited_options =
                            parse_delimited_text_options(append_type, with_clause);
                        std::size_t appended_count = 0U;
                        bool first_line = true;
                        for (const std::string &line : split_text_lines(csv_bytes))
                        {
                            if (line.empty())
                            {
                                continue;
                            }
                            const std::vector<std::string> values = parse_delimited_text_line(line, delimited_options);
                            if (append_type == "csv" && first_line && values.size() >= filtered_target_fields.size())
                            {
                                bool matches_header = true;
                                for (std::size_t idx = 0U; idx < filtered_target_fields.size(); ++idx)
                                {
                                    if (collapse_identifier(values[idx]) != collapse_identifier(filtered_target_fields[idx].name))
                                    {
                                        matches_header = false;
                                        break;
                                    }
                                }
                                if (matches_header)
                                {
                                    first_line = false;
                                    continue;
                                }
                            }
                            first_line = false;

                            vfp::DbfRecord appended_record;
                            appended_record.record_index = cursor->remote_records.size();
                            appended_record.deleted = false;
                            appended_record.values.reserve(target_fields.size());

                            std::vector<std::pair<std::string, std::string>> imported_values;
                            imported_values.reserve(std::min(filtered_target_fields.size(), values.size()));
                            for (std::size_t index = 0U;
                                 index < filtered_target_fields.size() && index < values.size();
                                 ++index)
                            {
                                imported_values.emplace_back(
                                    collapse_identifier(filtered_target_fields[index].name),
                                    values[index]);
                            }

                            for (const auto &target_field : target_fields)
                            {
                                vfp::DbfRecordValue value{
                                    .field_name = target_field.name,
                                    .field_type = target_field.type,
                                    .is_null = false,
                                    .display_value = {}};

                                const auto imported_value = std::find_if(
                                    imported_values.begin(),
                                    imported_values.end(),
                                    [&](const auto &candidate)
                                    {
                                        return candidate.first == collapse_identifier(target_field.name);
                                    });
                                if (imported_value != imported_values.end())
                                {
                                    value.display_value = imported_value->second;
                                }

                                appended_record.values.push_back(std::move(value));
                            }

                            cursor->remote_records.push_back(std::move(appended_record));
                            cursor->record_count = cursor->remote_records.size();
                            cursor->recno = cursor->record_count;
                            cursor->eof = false;
                            cursor->bof = cursor->record_count == 0U;

                            if (!trim_copy(for_expr).empty() && !current_record_matches_visibility(*cursor, frame, for_expr))
                            {
                                cursor->remote_records.pop_back();
                                cursor->record_count = cursor->remote_records.size();
                                continue;
                            }

                            ++appended_count;
                        }

                        if (cursor->remote_fields.empty())
                        {
                            cursor->remote_fields = target_fields;
                        }
                        cursor->record_count = cursor->remote_records.size();
                        cursor->eof = cursor->record_count == 0U;
                        cursor->bof = cursor->record_count == 0U;
                        cursor->found = false;
                        if (appended_count > 0U)
                        {
                            cursor->recno = cursor->record_count;
                            cursor->eof = false;
                            cursor->bof = false;
                        }

                        events.push_back({.category = "runtime.append_from",
                                          .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE DELIMITED)",
                                          .location = statement.location});
                        return {};
                    }

                    const auto source_result = parse_table_path(
                        copperfin::platform::path_to_utf8_string(src_path), 1000000U);
                    if (!source_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromFailed",
                            {
                                {"errorMessage", source_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields = cursor_field_descriptors(*cursor);
                    const std::string for_expr = statement.quaternary_expression;
                    std::vector<vfp::DbfFieldDescriptor> filtered_target_fields;
                    filtered_target_fields.reserve(target_fields.size());
                    for (const auto &field : target_fields)
                    {
                        if (field_matches_filter(field.name, field_filter))
                        {
                            filtered_target_fields.push_back(field);
                        }
                    }
                    if (filtered_target_fields.empty())
                    {
                        last_error_message =
                            runtime_text("Runtime.Prg.Dispatch.Error.AppendFromNoFieldsMatchFieldsClause");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::size_t appended_count = 0U;
                    for (const auto &source_record : source_result.table.records)
                    {
                        if (source_record.deleted)
                        {
                            continue;
                        }

                        vfp::DbfRecord appended_record;
                        appended_record.record_index = cursor->remote_records.size();
                        appended_record.deleted = false;
                        appended_record.values.reserve(target_fields.size());

                        for (const auto &target_field : target_fields)
                        {
                            vfp::DbfRecordValue value{
                                .field_name = target_field.name,
                                .field_type = target_field.type,
                                .is_null = false,
                                .display_value = {}};

                            const bool included_by_filter = field_matches_filter(target_field.name, field_filter);
                            if (included_by_filter)
                            {
                                const auto source_value = std::find_if(
                                    source_record.values.begin(),
                                    source_record.values.end(),
                                    [&](const vfp::DbfRecordValue &candidate)
                                    {
                                        return collapse_identifier(candidate.field_name) == collapse_identifier(target_field.name);
                                    });
                                if (source_value != source_record.values.end())
                                {
                                    value.is_null = source_value->is_null;
                                    value.display_value = source_value->display_value;
                                }
                            }

                            appended_record.values.push_back(std::move(value));
                        }

                        cursor->remote_records.push_back(std::move(appended_record));
                        cursor->record_count = cursor->remote_records.size();
                        cursor->recno = cursor->record_count;
                        cursor->eof = false;
                        cursor->bof = cursor->record_count == 0U;

                        if (!trim_copy(for_expr).empty() && !current_record_matches_visibility(*cursor, frame, for_expr))
                        {
                            cursor->remote_records.pop_back();
                            cursor->record_count = cursor->remote_records.size();
                            continue;
                        }

                        ++appended_count;
                    }

                    if (cursor->remote_fields.empty())
                    {
                        cursor->remote_fields = target_fields;
                    }
                    cursor->record_count = cursor->remote_records.size();
                    cursor->eof = cursor->record_count == 0U;
                    cursor->bof = cursor->record_count == 0U;
                    cursor->found = false;
                    if (appended_count > 0U)
                    {
                        cursor->recno = cursor->record_count;
                        cursor->eof = false;
                        cursor->bof = false;
                    }

                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records)",
                                      .location = statement.location});
                    return {};
                }

                if (!ensure_command_undo_backup_for_table(cursor->source_path))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                current_command_undo_journal().command_label = "APPEND FROM";
                const CursorPositionSnapshot append_from_original_position = capture_cursor_snapshot(*cursor);
                const std::size_t append_from_original_record_count = cursor->record_count;
                struct AppendFromCommandUndoGuard
                {
                    PrgRuntimeSession::Impl &runtime;
                    CursorState &cursor;
                    CursorPositionSnapshot original_position;
                    std::size_t original_record_count = 0U;
                    bool committed = false;
                    ~AppendFromCommandUndoGuard()
                    {
                        if (committed)
                        {
                            runtime.commit_active_command_undo_journal();
                        }
                        else
                        {
                            runtime.rollback_active_command_undo_journal();
                            cursor.record_count = original_record_count;
                            runtime.restore_cursor_snapshot(cursor, original_position);
                        }
                    }
                } append_from_command_undo_guard{
                    *this,
                    *cursor,
                    append_from_original_position,
                    append_from_original_record_count};

                if (append_from_sdf)
                {
                    std::string buffer;
                    if (!read_append_source_bytes(src_path, "SDF", buffer))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto dest_result = parse_cursor_table(
                        *cursor, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                            {
                                {"type", "SDF"},
                                {"errorMessage", dest_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields =
                        filter_field_descriptors(dest_result.table.fields, field_filter, true);
                    if (target_fields.empty())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                            {
                                {"type", "SDF"},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::size_t appended_count = 0U;
                    for (const std::string &line : split_sdf_lines(buffer))
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                {
                                    {"type", "SDF"},
                                    {"errorMessage", blank_result.error},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = blank_result.record_count;
                        cursor->eof = false;
                        cursor->recno = blank_result.record_count;

                        std::size_t offset = 0U;
                        for (const auto &field : target_fields)
                        {
                            const std::string raw_value = offset < line.size()
                                                              ? line.substr(offset, std::min<std::size_t>(field.length, line.size() - offset))
                                                              : std::string{};
                            offset += field.length;
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                field.name,
                                raw_value);
                            if (!rep_result.ok)
                            {
                                last_error_message = runtime_text(
                                    "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                    {
                                        {"type", "SDF"},
                                        {"errorMessage", rep_result.error},
                                    });
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    append_from_command_undo_guard.committed = true;
                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE SDF)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_json)
                {
                    std::string buffer;
                    if (!read_append_source_bytes(src_path, "JSON", buffer))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto dest_result = parse_cursor_table(
                        *cursor, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                            {
                                {"type", "JSON"},
                                {"errorMessage", dest_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields =
                        filter_field_descriptors(dest_result.table.fields, field_filter, true);
                    if (target_fields.empty())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                            {
                                {"type", "JSON"},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const std::vector<std::map<std::string, std::string>> json_rows = parse_json_record_objects(buffer);
                    std::size_t appended_count = 0U;
                    for (const auto &row : json_rows)
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                {
                                    {"type", "JSON"},
                                    {"errorMessage", blank_result.error},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = blank_result.record_count;
                        cursor->eof = false;
                        cursor->recno = blank_result.record_count;

                        for (const auto &field : target_fields)
                        {
                            const auto found = row.find(collapse_identifier(field.name));
                            if (found == row.end())
                            {
                                continue;
                            }
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                field.name,
                                found->second);
                            if (!rep_result.ok)
                            {
                                last_error_message = runtime_text(
                                    "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                    {
                                        {"type", "JSON"},
                                        {"errorMessage", rep_result.error},
                                    });
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    append_from_command_undo_guard.committed = true;
                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE JSON)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_dif)
                {
                    std::string buffer;
                    if (!read_append_source_bytes(src_path, "DIF", buffer))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto dest_result = parse_cursor_table(
                        *cursor, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                            {
                                {"type", "DIF"},
                                {"errorMessage", dest_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields =
                        filter_field_descriptors(dest_result.table.fields, field_filter, true);
                    if (target_fields.empty())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                            {
                                {"type", "DIF"},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<std::vector<std::string>> dif_rows = parse_dif_table(buffer, target_fields.size());
                    if (!dif_rows.empty() && dif_rows.front().size() >= target_fields.size())
                    {
                        bool matches_header = true;
                        for (std::size_t index = 0U; index < target_fields.size(); ++index)
                        {
                            if (collapse_identifier(dif_rows.front()[index]) != collapse_identifier(target_fields[index].name))
                            {
                                matches_header = false;
                                break;
                            }
                        }
                        if (matches_header)
                        {
                            dif_rows.erase(dif_rows.begin());
                        }
                    }

                    std::size_t appended_count = 0U;
                    for (const auto &row : dif_rows)
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                {
                                    {"type", "DIF"},
                                    {"errorMessage", blank_result.error},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = blank_result.record_count;
                        cursor->eof = false;
                        cursor->recno = blank_result.record_count;

                        for (std::size_t index = 0U; index < target_fields.size() && index < row.size(); ++index)
                        {
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                target_fields[index].name,
                                row[index]);
                            if (!rep_result.ok)
                            {
                                last_error_message = runtime_text(
                                    "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                    {
                                        {"type", "DIF"},
                                        {"errorMessage", rep_result.error},
                                    });
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    append_from_command_undo_guard.committed = true;
                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE DIF)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_sylk)
                {
                    std::string buffer;
                    if (!read_append_source_bytes(src_path, "SYLK", buffer))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto dest_result = parse_cursor_table(
                        *cursor, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                            {
                                {"type", "SYLK"},
                                {"errorMessage", dest_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields =
                        filter_field_descriptors(dest_result.table.fields, field_filter, true);
                    if (target_fields.empty())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                            {
                                {"type", "SYLK"},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<std::vector<std::string>> sylk_rows = parse_sylk_table(buffer, target_fields.size());
                    if (!sylk_rows.empty() && sylk_rows.front().size() >= target_fields.size())
                    {
                        bool matches_header = true;
                        for (std::size_t index = 0U; index < target_fields.size(); ++index)
                        {
                            if (collapse_identifier(sylk_rows.front()[index]) != collapse_identifier(target_fields[index].name))
                            {
                                matches_header = false;
                                break;
                            }
                        }
                        if (matches_header)
                        {
                            sylk_rows.erase(sylk_rows.begin());
                        }
                    }

                    std::size_t appended_count = 0U;
                    for (const auto &row : sylk_rows)
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                {
                                    {"type", "SYLK"},
                                    {"errorMessage", blank_result.error},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = blank_result.record_count;
                        cursor->eof = false;
                        cursor->recno = blank_result.record_count;

                        for (std::size_t index = 0U; index < target_fields.size() && index < row.size(); ++index)
                        {
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                target_fields[index].name,
                                row[index]);
                            if (!rep_result.ok)
                            {
                                last_error_message = runtime_text(
                                    "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                    {
                                        {"type", "SYLK"},
                                        {"errorMessage", rep_result.error},
                                    });
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    append_from_command_undo_guard.committed = true;
                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE SYLK)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_xls)
                {
                    std::string buffer;
                    if (!read_append_source_bytes(src_path, "XLS", buffer))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto dest_result = parse_cursor_table(
                        *cursor, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                            {
                                {"type", "XLS"},
                                {"errorMessage", dest_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields =
                        filter_field_descriptors(dest_result.table.fields, field_filter, true);
                    if (target_fields.empty())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                            {
                                {"type", "XLS"},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<std::vector<std::string>> workbook_rows = parse_spreadsheetml_workbook(buffer);
                    if (!workbook_rows.empty() && workbook_rows.front().size() >= target_fields.size())
                    {
                        bool matches_header = true;
                        for (std::size_t index = 0U; index < target_fields.size(); ++index)
                        {
                            if (collapse_identifier(workbook_rows.front()[index]) != collapse_identifier(target_fields[index].name))
                            {
                                matches_header = false;
                                break;
                            }
                        }
                        if (matches_header)
                        {
                            workbook_rows.erase(workbook_rows.begin());
                        }
                    }

                    std::size_t appended_count = 0U;
                    for (const auto &row : workbook_rows)
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                {
                                    {"type", "XLS"},
                                    {"errorMessage", blank_result.error},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = blank_result.record_count;
                        cursor->eof = false;
                        cursor->recno = blank_result.record_count;

                        for (std::size_t index = 0U; index < target_fields.size() && index < row.size(); ++index)
                        {
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                target_fields[index].name,
                                row[index]);
                            if (!rep_result.ok)
                            {
                                last_error_message = runtime_text(
                                    "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                    {
                                        {"type", "XLS"},
                                        {"errorMessage", rep_result.error},
                                    });
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    append_from_command_undo_guard.committed = true;
                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE XLS)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_delimited)
                {
                    std::string buffer;
                    if (!read_append_source_bytes(src_path, "DELIMITED", buffer))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const auto dest_result = parse_cursor_table(
                        *cursor, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                            {
                                {"type", "DELIMITED"},
                                {"errorMessage", dest_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields =
                        filter_field_descriptors(dest_result.table.fields, field_filter, true);
                    if (target_fields.empty())
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
                            {
                                {"type", "DELIMITED"},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const DelimitedTextOptions delimited_options = parse_delimited_text_options(append_type, with_clause);
                    std::size_t appended_count = 0U;
                    bool first_delimited_line = true;
                    for (const std::string &line : split_text_lines(buffer))
                    {
                        if (line.empty())
                        {
                            continue;
                        }
                        const std::vector<std::string> values = parse_delimited_text_line(line, delimited_options);
                        if (append_type == "csv" && first_delimited_line && values.size() >= target_fields.size())
                        {
                            bool matches_header = true;
                            for (std::size_t index = 0U; index < target_fields.size(); ++index)
                            {
                                if (collapse_identifier(values[index]) != collapse_identifier(target_fields[index].name))
                                {
                                    matches_header = false;
                                    break;
                                }
                            }
                            if (matches_header)
                            {
                                first_delimited_line = false;
                                continue;
                            }
                        }
                        first_delimited_line = false;
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                {
                                    {"type", "DELIMITED"},
                                    {"errorMessage", blank_result.error},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = blank_result.record_count;
                        cursor->eof = false;
                        cursor->recno = blank_result.record_count;

                        for (std::size_t index = 0U; index < target_fields.size() && index < values.size(); ++index)
                        {
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                target_fields[index].name,
                                values[index]);
                            if (!rep_result.ok)
                            {
                                last_error_message = runtime_text(
                                    "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
                                    {
                                        {"type", "DELIMITED"},
                                        {"errorMessage", rep_result.error},
                                    });
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    append_from_command_undo_guard.committed = true;
                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE DELIMITED)",
                                      .location = statement.location});
                    return {};
                }

                // Parse all records from the source file
                const auto src_result = parse_table_path(
                    copperfin::platform::path_to_utf8_string(src_path),
                    std::numeric_limits<std::size_t>::max());
                if (!src_result.ok)
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Dispatch.Error.AppendFromFailed",
                        {
                            {"errorMessage", src_result.error},
                        });
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                CursorState *open_source_cursor = nullptr;
                const std::string normalized_source_path = normalize_path(
                    copperfin::platform::path_to_utf8_string(src_path));
                for (auto &[_, candidate] : current_session_state().cursors)
                {
                    if (&candidate != cursor && !candidate.remote &&
                        normalize_path(candidate.source_path) == normalized_source_path)
                    {
                        open_source_cursor = &candidate;
                        break;
                    }
                }

                // Append each qualifying source record into the destination cursor
                if (!ensure_transaction_backup_for_table(cursor->source_path))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::size_t appended_count = 0U;
                const std::vector<vfp::DbfFieldDescriptor> destination_fields =
                    cursor_field_descriptors(*cursor);
                for (std::size_t source_record_index = 0U;
                     source_record_index < src_result.table.records.size();
                     ++source_record_index)
                {
                    const vfp::DbfRecord &src_rec = src_result.table.records[source_record_index];
                    if (src_rec.deleted)
                    {
                        continue;
                    }
                    if (open_source_cursor != nullptr &&
                        !filter_expression_matches_record(
                            *open_source_cursor,
                            frame,
                            src_rec,
                            source_record_index + 1U))
                    {
                        continue;
                    }
                    // Append a blank record and then replace matching fields by name
                    const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                    if (!blank_result.ok)
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Dispatch.Error.AppendFromFailed",
                            {
                                {"errorMessage", blank_result.error},
                            });
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    cursor->record_count = blank_result.record_count;
                    cursor->eof = false;
                    cursor->recno = blank_result.record_count;

                    for (const auto &src_field : src_rec.values)
                    {
                        if (!field_matches_filter(src_field.field_name, field_filter))
                        {
                            continue;
                        }
                        const std::string normalized_source_field =
                            collapse_identifier(src_field.field_name);
                        const bool destination_has_field = std::any_of(
                            destination_fields.begin(),
                            destination_fields.end(),
                            [&](const vfp::DbfFieldDescriptor &destination_field)
                            {
                                return collapse_identifier(destination_field.name) ==
                                       normalized_source_field;
                            });
                        if (!destination_has_field)
                        {
                            continue;
                        }
                        const auto rep_result = vfp::replace_record_field_value(
                            cursor->source_path,
                            cursor->recno - 1U,
                            src_field.field_name,
                            src_field.display_value);
                        if (!rep_result.ok)
                        {
                            last_error_message = runtime_text(
                                "Runtime.Prg.Dispatch.Error.AppendFromFailed",
                                {
                                    {"errorMessage", rep_result.error},
                                });
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = rep_result.record_count;
                    }
                    ++appended_count;
                }

                append_from_command_undo_guard.committed = true;
                events.push_back({.category = "runtime.append_from",
                                  .detail = src_raw + " (" + std::to_string(appended_count) + " records)",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::scatter_command:
            {
                // SCATTER [FIELDS <list>] TO <array>|MEMVAR|NAME <object> [BLANK] [MEMO] [ADDITIVE]
                const bool use_memvar = (statement.identifier == "memvar");
                const bool use_name_object = (statement.identifier == "name");
                const bool blank = !statement.tertiary_expression.empty();
                const bool include_memo = !statement.quaternary_expression.empty();
                const bool additive = std::any_of(statement.names.begin(), statement.names.end(),
                                                  [](const std::string &name)
                                                  {
                                                      return normalize_identifier(name) == "additive";
                                                  });
                const std::vector<std::string> field_filter = parse_field_filter_clause(statement.secondary_expression);
                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ScatterNoCurrentWorkArea");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const auto rec = current_record(*cursor);
                if (!rec.has_value() && !blank)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.ScatterNoCurrentRecord");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                // For BLANK mode on an empty table, build synthetic field list from schema.
                std::vector<vfp::DbfRecordValue> blank_field_list;
                if (!rec.has_value() && blank)
                {
                    if (cursor->remote)
                    {
                        for (const auto &fd : cursor->remote_fields)
                        {
                            vfp::DbfRecordValue rv;
                            rv.field_name = fd.name;
                            rv.field_type = fd.type;
                            blank_field_list.push_back(rv);
                        }
                    }
                    else if (!cursor->source_path.empty())
                    {
                        const auto schema = parse_cursor_table(*cursor, 0U);
                        if (schema.ok)
                        {
                            for (const auto &fd : schema.table.fields)
                            {
                                vfp::DbfRecordValue rv;
                                rv.field_name = fd.name;
                                rv.field_type = fd.type;
                                blank_field_list.push_back(rv);
                            }
                        }
                    }
                }
                const std::vector<vfp::DbfRecordValue> &field_source =
                    rec.has_value() ? rec->values : blank_field_list;
                const std::vector<vfp::DbfFieldDescriptor> selected_fields =
                    filter_field_descriptors(cursor_field_descriptors(*cursor), field_filter, true);

                std::vector<PrgValue> scattered_values;
                std::vector<std::string> scattered_field_names;
                std::string array_name;
                std::size_t matched_field_count = 0U;
                for (const auto &selected_field : selected_fields)
                {
                    const auto field = std::find_if(
                        field_source.begin(),
                        field_source.end(),
                        [&](const vfp::DbfRecordValue &candidate)
                        {
                            return collapse_identifier(candidate.field_name) ==
                                   collapse_identifier(selected_field.name);
                        });

                    vfp::DbfRecordValue resolved_field;
                    if (field != field_source.end())
                    {
                        resolved_field = *field;
                    }
                    else
                    {
                        resolved_field.field_name = selected_field.name;
                        resolved_field.field_type = selected_field.type;
                    }

                    const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(resolved_field.field_type)));
                    const bool is_memo_field = field_type == 'M' || field_type == 'G' || field_type == 'W';
                    if (!include_memo && is_memo_field)
                    {
                        continue;
                    }

                    ++matched_field_count;
                    const PrgValue val = blank ? blank_value_for_field(resolved_field)
                                               : record_value_to_prg_value(resolved_field);
                    if (use_memvar)
                    {
                        assign_variable(frame, "m." + resolved_field.field_name, val);
                    }
                    else if (use_name_object)
                    {
                        scattered_field_names.push_back(normalize_identifier(resolved_field.field_name));
                        scattered_values.push_back(val);
                    }
                    else
                    {
                        scattered_field_names.push_back(resolved_field.field_name);
                        scattered_values.push_back(val);
                    }
                }

                if (use_name_object)
                {
                    const auto object_target_path = parse_command_object_target_path(statement.expression, "SCATTER NAME");
                    if (!object_target_path.has_value())
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    RuntimeOleObjectState *target_object = nullptr;
                    if (additive)
                    {
                        target_object = resolve_existing_object_target(*object_target_path);
                    }
                    if (target_object == nullptr)
                    {
                        if (additive)
                        {
                            target_object = ensure_object_target(
                                *object_target_path,
                                "scatter name additive");
                        }
                        else
                        {
                            target_object = create_empty_runtime_object("scatter name");
                            if (target_object != nullptr &&
                                !assign_object_target_reference(
                                    *object_target_path,
                                    make_runtime_object_reference(target_object),
                                    "scatter name"))
                            {
                                target_object = nullptr;
                            }
                        }
                        if (target_object == nullptr)
                        {
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                    }
                    for (std::size_t index = 0U; index < scattered_values.size() && index < scattered_field_names.size(); ++index)
                    {
                        target_object->properties[normalize_identifier(scattered_field_names[index])] = scattered_values[index];
                    }
                }
                else if (!use_memvar)
                {
                    const auto resolved_array_name = resolve_command_array_name(statement.expression, "SCATTER TO");
                    if (!resolved_array_name.has_value())
                    {
                        if (frame.command_array_name_continuation.has_value())
                        {
                            return {};
                        }
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    array_name = *resolved_array_name;
                    RuntimeArray *existing_array = find_array(array_name);
                    if (existing_array != nullptr && existing_array->columns > 1U)
                    {
                        if (existing_array->columns == 2U)
                        {
                            const std::size_t required_rows = std::max(existing_array->rows, scattered_values.size());
                            if (required_rows > existing_array->rows)
                            {
                                resize_array(array_name, required_rows, 2U);
                                existing_array = find_array(array_name);
                            }
                            if (existing_array != nullptr)
                            {
                                for (std::size_t index = 0U; index < scattered_values.size() && index < existing_array->rows; ++index)
                                {
                                    existing_array->values[(index * 2U)] = make_string_value(scattered_field_names[index]);
                                    existing_array->values[(index * 2U) + 1U] = scattered_values[index];
                                }
                            }
                        }
                        else
                        {
                            const std::size_t required_columns = std::max(existing_array->columns, scattered_values.size());
                            if (existing_array->rows == 0U || required_columns != existing_array->columns)
                            {
                                resize_array(array_name,
                                             std::max<std::size_t>(1U, existing_array->rows),
                                             required_columns);
                                existing_array = find_array(array_name);
                            }
                            if (existing_array != nullptr)
                            {
                                for (std::size_t index = 0U; index < scattered_values.size() && index < existing_array->columns; ++index)
                                {
                                    existing_array->values[index] = scattered_values[index];
                                }
                            }
                        }
                    }
                    else
                    {
                        assign_array(array_name, std::move(scattered_values));
                    }
                }

                if (!field_filter.empty())
                {
                    if (matched_field_count == 0U)
                    {
                        last_error_message =
                            runtime_text("Runtime.Prg.Dispatch.Error.ScatterNoFieldsMatchFieldsClause");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                else
                {
                    (void)field_filter;
                }
                events.push_back({.category = "runtime.scatter",
                                  .detail = use_memvar ? "memvar" : (use_name_object ? trim_copy(statement.expression) : array_name),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::gather_command:
            {
                // GATHER FROM <array>|MEMVAR|NAME <object> [FIELDS <list>] [FOR <expr>]
                const bool use_memvar = (statement.identifier == "memvar");
                const bool use_name_object = (statement.identifier == "name");
                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr)
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.GatherNoCurrentWorkArea");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const auto rec = current_record(*cursor);
                if (!rec.has_value())
                {
                    last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.GatherNoCurrentRecord");
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!trim_copy(statement.quaternary_expression).empty())
                {
                    Statement predicate_statement = statement;
                    predicate_statement.expression = statement.quaternary_expression;
                    const auto predicate_value = resumed_gather_for_value.has_value()
                                                     ? resumed_gather_for_value
                                                     : evaluate_resumable_expression(frame, predicate_statement, cursor);
                    if (!predicate_value.has_value())
                    {
                        return {};
                    }
                    if (!value_as_bool(*predicate_value))
                    {
                        std::string detail = "memvar skipped";
                        if (use_name_object)
                        {
                            detail = trim_copy(statement.expression) + " skipped";
                        }
                        else if (!use_memvar)
                        {
                            const auto resolved_array_name = resolve_command_array_name(statement.expression, "GATHER FROM");
                            if (!resolved_array_name.has_value() && frame.command_array_name_continuation.has_value())
                            {
                                return {};
                            }
                            detail = (resolved_array_name.has_value() ? *resolved_array_name : statement.expression) + " skipped";
                        }
                        events.push_back({.category = "runtime.gather",
                                          .detail = detail,
                                          .location = statement.location});
                        return {};
                    }
                }
                if (!cursor->remote && !ensure_transaction_backup_for_table(cursor->source_path))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::vector<std::string> field_filter = parse_field_filter_clause(statement.secondary_expression);
                std::string array_name;
                RuntimeArray *source_array = nullptr;
                RuntimeOleObjectState *source_object = nullptr;
                if (use_name_object)
                {
                    const auto object_target_path = parse_command_object_target_path(statement.expression, "GATHER NAME");
                    if (!object_target_path.has_value())
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    source_object = resolve_existing_object_target(*object_target_path);
                    if (source_object == nullptr)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                }
                else if (!use_memvar)
                {
                    const auto resolved_array_name = resolve_command_array_name(statement.expression, "GATHER FROM");
                    if (!resolved_array_name.has_value())
                    {
                        if (frame.command_array_name_continuation.has_value())
                        {
                            return {};
                        }
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    array_name = *resolved_array_name;
                    source_array = find_array(array_name);
                }
                std::map<std::string, PrgValue> name_value_pairs;
                bool use_name_value_pairs = false;
                if (source_array != nullptr && source_array->columns == 2U)
                {
                    for (std::size_t row = 1U; row <= source_array->rows; ++row)
                    {
                        const PrgValue field_name_value = array_value(array_name, row, 1U);
                        if (field_name_value.kind != PrgValueKind::string)
                        {
                            continue;
                        }
                        const std::string normalized_field_name = normalize_identifier(value_as_string(field_name_value));
                        if (normalized_field_name.empty())
                        {
                            continue;
                        }
                        name_value_pairs[normalized_field_name] = array_value(array_name, row, 2U);
                    }
                    use_name_value_pairs = !name_value_pairs.empty();
                }
                bool temporary_record_lock = false;
                if (!cursor->remote && !acquire_record_lock(*cursor, cursor->recno, "GATHER", false, temporary_record_lock))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto perform_gather = [&]() -> bool
                {
                    const std::vector<vfp::DbfFieldDescriptor> selected_fields =
                        filter_field_descriptors(cursor_field_descriptors(*cursor), field_filter, true);
                    std::size_t array_index = 1U;
                    for (const auto &selected_field : selected_fields)
                    {
                        const auto field = std::find_if(
                            rec->values.begin(),
                            rec->values.end(),
                            [&](const vfp::DbfRecordValue &candidate)
                            {
                                return collapse_identifier(candidate.field_name) ==
                                       collapse_identifier(selected_field.name);
                            });
                        if (field == rec->values.end())
                        {
                            continue;
                        }
                        const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field->field_type)));
                        const bool is_memo_field = field_type == 'M' || field_type == 'G' || field_type == 'W';
                        PrgValue val = make_empty_value();
                        if (use_memvar)
                        {
                            if (is_memo_field)
                            {
                                continue;
                            }
                            const PrgValue *memvar = find_variable(frame, "m." + field->field_name);
                            if (memvar == nullptr)
                            {
                                continue;
                            }
                            val = *memvar;
                        }
                        else if (use_name_object)
                        {
                            const std::string field_name = normalize_identifier(field->field_name);
                            const auto property = source_object->properties.find(field_name);
                            if (property == source_object->properties.end())
                            {
                                continue;
                            }
                            val = property->second;
                        }
                        else if (use_name_value_pairs)
                        {
                            const auto pair = name_value_pairs.find(normalize_identifier(field->field_name));
                            if (pair == name_value_pairs.end())
                            {
                                continue;
                            }
                            val = pair->second;
                        }
                        else
                        {
                            if (is_memo_field)
                            {
                                continue;
                            }

                            if (source_array != nullptr && source_array->columns > 1U)
                            {
                                val = array_value(array_name, 1U, array_index++);
                            }
                            else
                            {
                                val = array_value(array_name, array_index++);
                            }
                        }
                        if (cursor->remote)
                        {
                            auto it = std::find_if(
                                cursor->remote_records[cursor->recno - 1U].values.begin(),
                                cursor->remote_records[cursor->recno - 1U].values.end(),
                                [&](const vfp::DbfRecordValue &rv)
                                {
                                    return collapse_identifier(rv.field_name) == collapse_identifier(field->field_name);
                                });
                            if (it != cursor->remote_records[cursor->recno - 1U].values.end())
                            {
                                it->display_value = serialize_prg_value_for_record_field(*it, val);
                            }
                        }
                        else
                        {
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                field->field_name,
                                serialize_prg_value_for_record_field(*field, val));
                            if (!rep_result.ok)
                            {
                                last_error_message = rep_result.error;
                                return false;
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                    }
                    return true;
                };

                const bool gather_ok = cursor->remote
                    ? perform_gather()
                    : execute_with_command_undo(cursor->source_path, "GATHER", perform_gather);

                if (!cursor->remote && temporary_record_lock)
                {
                    unlock_cursor_record_lock(*cursor, cursor->recno);
                }

                if (!gather_ok)
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.gather",
                                  .detail = use_memvar ? "memvar" : (use_name_object ? trim_copy(statement.expression) : array_name),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::retry_statement:
            {
                // RETRY: re-execute the faulting statement in the faulting frame
                if (!fault_pc_valid)
                {
                    return {}; // If no fault PC saved, do nothing
                }
                handling_error = false;
                error_handler_return_depth.reset();
                if (!error_metadata_stack.empty())
                {
                    current_data_session = std::max(1, error_metadata_stack.back().data_session);
                    if (error_metadata_stack.back().session_state_snapshot.has_value())
                    {
                        current_session_state() = *error_metadata_stack.back().session_state_snapshot;
                    }
                    error_metadata_stack.pop_back();
                }
                fault_pc_valid = false;
                // Unwind to the fault frame
                while (!stack.empty())
                {
                    if (stack.back().file_path == fault_frame_file_path &&
                        stack.back().routine_name == fault_frame_routine_name)
                    {
                        stack.back().pc = fault_statement_index;
                        return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                    }
                    restore_private_declarations(stack.back());
                    stack.pop_back();
                }
                return {};
            }
            case StatementKind::resume_statement:
            {
                // RESUME [NEXT]: continue after the faulting statement
                if (!fault_pc_valid)
                {
                    return {};
                }
                handling_error = false;
                error_handler_return_depth.reset();
                if (!error_metadata_stack.empty())
                {
                    current_data_session = std::max(1, error_metadata_stack.back().data_session);
                    if (error_metadata_stack.back().session_state_snapshot.has_value())
                    {
                        current_session_state() = *error_metadata_stack.back().session_state_snapshot;
                    }
                    error_metadata_stack.pop_back();
                }
                fault_pc_valid = false;
                while (!stack.empty())
                {
                    if (stack.back().file_path == fault_frame_file_path &&
                        stack.back().routine_name == fault_frame_routine_name)
                    {
                        const Routine *r = stack.back().routine;
                        std::size_t resume_pc = fault_statement_index + 1U;
                        if (r != nullptr &&
                            fault_statement_index < r->statements.size() &&
                            r->statements[fault_statement_index].kind == StatementKind::case_statement &&
                            !stack.back().cases.empty())
                        {
                            resume_pc = stack.back().cases.back().endcase_statement_index + 1U;
                            stack.back().cases.pop_back();
                        }

                        // A predicate can suspend into one or more user-routine
                        // frames before the fault reaches RESUME. Abandon the
                        // caller's CASE continuation as one unit; otherwise the
                        // faulting routine can return a stale value and the
                        // caller may incorrectly execute the CASE branch.
                        for (std::size_t index = stack.size(); index > 0U; --index)
                        {
                            Frame &candidate = stack[index - 1U];
                            if (candidate.scan_expression_continuation.has_value())
                            {
                                const ScanExpressionContinuation &scan_continuation =
                                    *candidate.scan_expression_continuation;
                                const int scan_work_area = scan_continuation.work_area;
                                const std::size_t scan_resume_pc =
                                    scan_continuation.endscan_statement_index + 1U;
                                if (CursorState *scan_cursor = find_cursor_by_area(scan_work_area);
                                    scan_cursor != nullptr)
                                {
                                    move_cursor_to(
                                        *scan_cursor,
                                        static_cast<long long>(scan_cursor->record_count + 1U));
                                    scan_cursor->found = false;
                                }
                                if (scan_continuation.kind == ScanSearchKind::continue_scan)
                                {
                                    candidate.scans.erase(
                                        std::remove_if(
                                            candidate.scans.begin(),
                                            candidate.scans.end(),
                                            [&](const ScanState &state)
                                            {
                                                return state.scan_statement_index ==
                                                       scan_continuation.scan_statement_index;
                                            }),
                                        candidate.scans.end());
                                }
                                candidate.scan_expression_continuation.reset();
                                candidate.expression_routine_return_pending = false;
                                candidate.expression_continuation.reset();
                                candidate.command_target_continuation.reset();
                                candidate.command_array_name_continuation.reset();
                                candidate.command_argument_continuation.reset();
                    candidate.text_merge_continuation.reset();
                    candidate.parameter_default_continuation.reset();
                    candidate.use_command_continuation.reset();
                    candidate.copy_file_continuation.reset();
                    candidate.rename_file_continuation.reset();
                                if (index == stack.size())
                                {
                                    resume_pc = scan_resume_pc;
                                }
                                else
                                {
                                    candidate.pc = scan_resume_pc;
                                }
                                last_return_value = make_empty_value();
                                for (std::size_t nested_index = index; nested_index < stack.size(); ++nested_index)
                                {
                                    stack[nested_index].expression_routine_return_pending = false;
                                    stack[nested_index].expression_continuation.reset();
                    stack[nested_index].text_merge_continuation.reset();
                    stack[nested_index].parameter_default_continuation.reset();
                    stack[nested_index].use_command_continuation.reset();
                    stack[nested_index].copy_file_continuation.reset();
                    stack[nested_index].rename_file_continuation.reset();
                                    stack[nested_index].loop_expression_continuation.reset();
                                    stack[nested_index].scan_expression_continuation.reset();
                                }
                                break;
                            }
                            if (candidate.loop_expression_continuation.has_value())
                            {
                                const Statement &loop_statement = candidate.loop_expression_continuation->statement;
                                const std::size_t loop_statement_index = candidate.pc > 0U ? candidate.pc - 1U : 0U;
                                std::size_t loop_resume_pc = candidate.pc;
                                if (loop_statement.kind == StatementKind::do_while_statement)
                                {
                                    if (const auto destination = find_matching_enddo(candidate, loop_statement_index))
                                    {
                                        loop_resume_pc = *destination + 1U;
                                    }
                                    candidate.whiles.erase(
                                        std::remove_if(
                                            candidate.whiles.begin(),
                                            candidate.whiles.end(),
                                            [&](const WhileState &state)
                                            {
                                                return state.do_while_statement_index == loop_statement_index;
                                            }),
                                        candidate.whiles.end());
                                }
                                else if (loop_statement.kind == StatementKind::for_statement ||
                                         loop_statement.kind == StatementKind::for_each_statement)
                                {
                                    if (const auto destination = find_matching_endfor(candidate, loop_statement_index))
                                    {
                                        loop_resume_pc = *destination + 1U;
                                    }
                                    candidate.loops.erase(
                                        std::remove_if(
                                            candidate.loops.begin(),
                                            candidate.loops.end(),
                                            [&](const LoopState &state)
                                            {
                                                return state.for_statement_index == loop_statement_index;
                                            }),
                                        candidate.loops.end());
                                }
                                candidate.loop_expression_continuation.reset();
                                candidate.expression_routine_return_pending = false;
                                candidate.expression_continuation.reset();
                                candidate.command_target_continuation.reset();
                                candidate.command_array_name_continuation.reset();
                                candidate.command_argument_continuation.reset();
                                candidate.text_merge_continuation.reset();
                                candidate.parameter_default_continuation.reset();
                                candidate.use_command_continuation.reset();
                                candidate.copy_file_continuation.reset();
                                candidate.rename_file_continuation.reset();
                                if (index == stack.size())
                                {
                                    resume_pc = loop_resume_pc;
                                }
                                else
                                {
                                    candidate.pc = loop_resume_pc;
                                }
                                last_return_value = make_empty_value();
                                for (std::size_t nested_index = index; nested_index < stack.size(); ++nested_index)
                                {
                                    stack[nested_index].expression_routine_return_pending = false;
                                    stack[nested_index].expression_continuation.reset();
                                    stack[nested_index].text_merge_continuation.reset();
                                    stack[nested_index].parameter_default_continuation.reset();
                                    stack[nested_index].use_command_continuation.reset();
                                    stack[nested_index].copy_file_continuation.reset();
                                    stack[nested_index].rename_file_continuation.reset();
                                    stack[nested_index].loop_expression_continuation.reset();
                                    stack[nested_index].scan_expression_continuation.reset();
                                }
                                break;
                            }
                            if (!candidate.expression_routine_return_pending ||
                                !candidate.expression_continuation.has_value() ||
                                candidate.expression_continuation->statement.kind != StatementKind::case_statement)
                            {
                                continue;
                            }

                            candidate.expression_routine_return_pending = false;
                            candidate.expression_continuation.reset();
                            candidate.command_target_continuation.reset();
                            candidate.command_array_name_continuation.reset();
                            candidate.command_argument_continuation.reset();
                            candidate.text_merge_continuation.reset();
                            candidate.parameter_default_continuation.reset();
                            candidate.use_command_continuation.reset();
                            candidate.copy_file_continuation.reset();
                            candidate.rename_file_continuation.reset();
                            if (!candidate.cases.empty())
                            {
                                candidate.pc = candidate.cases.back().endcase_statement_index + 1U;
                                candidate.cases.pop_back();
                            }
                            last_return_value = make_empty_value();
                            for (std::size_t nested_index = index; nested_index < stack.size(); ++nested_index)
                            {
                                stack[nested_index].expression_routine_return_pending = false;
                                stack[nested_index].expression_continuation.reset();
                                stack[nested_index].text_merge_continuation.reset();
                                stack[nested_index].parameter_default_continuation.reset();
                                stack[nested_index].use_command_continuation.reset();
                                stack[nested_index].copy_file_continuation.reset();
                                stack[nested_index].rename_file_continuation.reset();
                                stack[nested_index].loop_expression_continuation.reset();
                                stack[nested_index].scan_expression_continuation.reset();
                            }
                            break;
                        }
                        stack.back().pc = (r && resume_pc < r->statements.size()) ? resume_pc : (r ? r->statements.size() : 0U);
                        return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                    }
                    restore_private_declarations(stack.back());
                    stack.pop_back();
                }
                return {};
            }
            case StatementKind::edit_command:
            {
                std::string detail;
                append_cursor_view_metadata(resolve_cursor_target(std::to_string(current_selected_work_area())), {}, detail);
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "memo=" + statement.expression;
                }
                events.push_back({.category = "runtime.edit",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::change_command:
            {
                std::string detail;
                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor != nullptr)
                {
                    append_cursor_view_metadata(cursor, statement.expression, detail);
                }
                else if (!statement.expression.empty())
                {
                    detail += "fields=" + statement.expression;
                }
                events.push_back({.category = "runtime.change",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::input_command:
            {
                std::string detail;
                append_cursor_view_metadata(resolve_cursor_target(std::to_string(current_selected_work_area())), {}, detail);
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    const std::string resolved_prompt = resolve_runtime_expression_text(statement.expression, frame);
                    detail += "prompt=" + resolved_prompt;
                    if (trim_copy(statement.expression) != resolved_prompt)
                    {
                        detail += " prompt_expr=" + trim_copy(statement.expression);
                    }
                }
                if (!statement.identifier.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.identifier;
                    const std::string resolved_target = resolve_runtime_target_identifier(statement.identifier, frame);
                    if (!resolved_target.empty() && resolved_target != statement.identifier)
                    {
                        detail += " target_resolved=" + resolved_target;
                    }
                    ExecutionOutcome outcome = assign_runtime_target_value(statement.identifier, make_string_value(""));
                    if (!outcome.ok)
                    {
                        return outcome;
                    }
                    detail += " result=''";
                }
                events.push_back({.category = "runtime.input",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::accept_command:
            {
                std::string detail;
                append_cursor_view_metadata(resolve_cursor_target(std::to_string(current_selected_work_area())), {}, detail);
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    const std::string resolved_prompt = resolve_runtime_expression_text(statement.expression, frame);
                    detail += "prompt=" + resolved_prompt;
                    if (trim_copy(statement.expression) != resolved_prompt)
                    {
                        detail += " prompt_expr=" + trim_copy(statement.expression);
                    }
                }
                if (!statement.identifier.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.identifier;
                    const std::string resolved_target = resolve_runtime_target_identifier(statement.identifier, frame);
                    if (!resolved_target.empty() && resolved_target != statement.identifier)
                    {
                        detail += " target_resolved=" + resolved_target;
                    }
                    ExecutionOutcome outcome = assign_runtime_target_value(statement.identifier, make_string_value(""));
                    if (!outcome.ok)
                    {
                        return outcome;
                    }
                    detail += " result=''";
                }
                events.push_back({.category = "runtime.accept",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::getfile_command:
            {
                std::string detail;
                if (!statement.secondary_expression.empty())
                {
                    detail += "prompt=" + statement.secondary_expression;
                }
                if (!statement.tertiary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "title=" + statement.tertiary_expression;
                }
                if (!statement.quaternary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "default=" + statement.quaternary_expression;
                }
                if (!statement.identifier.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "filter=" + statement.identifier;
                }
                if (!statement.names.empty() && !statement.names.front().empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.names.front();
                }
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "clause=" + statement.expression;
                }
                assign_dialog_target_value(statement, frame, detail);
                events.push_back({.category = "runtime.getfile",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::putfile_command:
            {
                std::string detail;
                if (!statement.secondary_expression.empty())
                {
                    detail += "prompt=" + statement.secondary_expression;
                }
                if (!statement.tertiary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "title=" + statement.tertiary_expression;
                }
                if (!statement.quaternary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "default=" + statement.quaternary_expression;
                }
                if (!statement.identifier.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "filter=" + statement.identifier;
                }
                if (!statement.names.empty() && !statement.names.front().empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.names.front();
                }
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "clause=" + statement.expression;
                }
                assign_dialog_target_value(statement, frame, detail);
                events.push_back({.category = "runtime.putfile",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::getdir_command:
            {
                std::string detail;
                if (!statement.secondary_expression.empty())
                {
                    detail += "prompt=" + statement.secondary_expression;
                }
                if (!statement.tertiary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "title=" + statement.tertiary_expression;
                }
                if (!statement.quaternary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "default=" + statement.quaternary_expression;
                }
                if (!statement.names.empty() && !statement.names.front().empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.names.front();
                }
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "clause=" + statement.expression;
                }
                assign_dialog_target_value(statement, frame, detail);
                events.push_back({.category = "runtime.getdir",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::inputbox_command:
            {
                std::string detail;
                if (!statement.secondary_expression.empty())
                {
                    detail += "prompt=" + statement.secondary_expression;
                }
                if (!statement.tertiary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "title=" + statement.tertiary_expression;
                }
                if (!statement.quaternary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "default=" + statement.quaternary_expression;
                }
                if (!statement.names.empty() && !statement.names.front().empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.names.front();
                }
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "clause=" + statement.expression;
                }
                assign_dialog_target_value(statement, frame, detail);
                events.push_back({.category = "runtime.inputbox",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::wait_command:
            {
                std::string detail;
                if (!statement.identifier.empty())
                {
                    detail += "mode=" + statement.identifier;
                }
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    std::optional<PrgValue> prompt_value;
                    if (resumed_expression_value.has_value())
                    {
                        prompt_value = *resumed_expression_value;
                    }
                    else
                    {
                        prompt_value = evaluate_resumable_expression(frame, statement);
                        if (!prompt_value.has_value())
                        {
                            return {};
                        }
                    }
                    const std::string resolved_prompt = value_as_string(*prompt_value);
                    detail += "prompt=" + resolved_prompt;
                    if (trim_copy(statement.expression) != resolved_prompt)
                    {
                        detail += " prompt_expr=" + trim_copy(statement.expression);
                    }
                }
                if (!statement.secondary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    const std::string resolved_timeout = resolve_runtime_expression_text(statement.secondary_expression, frame);
                    detail += "timeout=" + resolved_timeout;
                    if (trim_copy(statement.secondary_expression) != resolved_timeout)
                    {
                        detail += " timeout_expr=" + trim_copy(statement.secondary_expression);
                    }
                }
                if (!statement.tertiary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "flag=" + statement.tertiary_expression;
                }
                if (!statement.quaternary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "flag=" + statement.quaternary_expression;
                }
                if (!statement.names.empty() && !statement.names.front().empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.names.front();
                    const std::string resolved_target = resolve_runtime_target_identifier(statement.names.front(), frame);
                    if (resolved_target != statement.names.front())
                    {
                        detail += " target_resolved=" + resolved_target;
                    }
                    ExecutionOutcome outcome = assign_runtime_target_value(statement.names.front(), make_string_value(""));
                    if (!outcome.ok)
                    {
                        return outcome;
                    }
                    detail += " result=''";
                }
                events.push_back({.category = "runtime.wait",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::sleep_command:
            {
                const auto cancel_sleep = [&]() -> ExecutionOutcome
                {
                    (void)handle_async_runtime_cancellation(
                        statement.location,
                        statement.text,
                        runtime_text("Runtime.Prg.Dispatch.Error.SleepCancelled"));
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                };
                if (task_cancel_requested != nullptr && task_cancel_requested->load(std::memory_order_relaxed))
                {
                    return cancel_sleep();
                }

                std::size_t sleep_duration_ms = scheduler_yield_sleep_ms;
                if (!trim_copy(statement.expression).empty())
                {
                    const auto delay_value = resumed_sleep_value.has_value()
                                                 ? resumed_sleep_value
                                                 : evaluate_resumable_expression(frame, statement);
                    if (!delay_value.has_value())
                    {
                        return {};
                    }
                    const double evaluated_delay = value_as_number(*delay_value);
                    if (!std::isfinite(evaluated_delay) || evaluated_delay < 0.0)
                    {
                        last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.SleepInvalidDuration");
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    sleep_duration_ms = static_cast<std::size_t>(std::llround(evaluated_delay));
                }

                if (sleep_duration_ms != 0U &&
                    !ensure_non_blocking_critical_section_policy("SLEEP", statement.location,
                                                                 "duration=" + std::to_string(sleep_duration_ms) + "ms"))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (sleep_duration_ms == 0U)
                {
                    std::this_thread::yield();
                }
                else
                {
                    for (std::size_t elapsed = 0U; elapsed < sleep_duration_ms; ++elapsed)
                    {
                        if (task_cancel_requested != nullptr && task_cancel_requested->load(std::memory_order_relaxed))
                        {
                            return cancel_sleep();
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(1U));
                    }
                }

                std::string detail = "duration=" + std::to_string(sleep_duration_ms) + "ms";
                if (!trim_copy(statement.expression).empty())
                {
                    detail += " expression=" + trim_copy(statement.expression);
                }
                events.push_back({.category = "runtime.sleep",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::keyboard_command:
            {
                std::string detail;
                if (!statement.expression.empty())
                {
                    const std::string resolved_keys = resolve_runtime_expression_text(statement.expression, frame);
                    detail += "keys=" + resolved_keys;
                    if (trim_copy(statement.expression) != resolved_keys)
                    {
                        detail += " keys_expr=" + trim_copy(statement.expression);
                    }
                }
                if (!statement.secondary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "flag=" + statement.secondary_expression;
                }
                if (!statement.tertiary_expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "flag=" + statement.tertiary_expression;
                }
                events.push_back({.category = "runtime.keyboard",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::display_command:
            {
                std::string detail;
                if (!statement.identifier.empty())
                {
                    detail += "mode=" + statement.identifier;
                }
                if (normalize_identifier(statement.identifier) == "records")
                {
                    CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                    if (cursor == nullptr)
                    {
                        cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    }
                    if (cursor != nullptr)
                    {
                        const std::vector<std::string> visible_fields = effective_visible_field_names(*cursor, statement.tertiary_expression);
                        std::string field_detail;
                        for (std::size_t index = 0U; index < visible_fields.size(); ++index)
                        {
                            if (index > 0U)
                            {
                                field_detail += ",";
                            }
                            field_detail += visible_fields[index];
                        }

                        if (!detail.empty()) detail += " ";
                        detail += cursor->alias.empty()
                            ? ("workarea=" + std::to_string(cursor->work_area))
                            : (cursor->alias + "@" + std::to_string(cursor->work_area));
                        detail += " recno=" + std::to_string(cursor->recno);
                        detail += " records=" + std::to_string(cursor->record_count);
                        detail += " fields=" + (field_detail.empty() ? std::string{"ALL"} : field_detail);
                        detail += " filter=" + (cursor->filter_expression.empty() ? std::string{"<none>"} : cursor->filter_expression);
                        if (!statement.quaternary_expression.empty())
                        {
                            detail += " for=" + trim_copy(statement.quaternary_expression);
                        }
                        if (!statement.names.empty() && !trim_copy(statement.names.front()).empty())
                        {
                            detail += " while=" + trim_copy(statement.names.front());
                        }
                    }
                    const std::string raw_target = trim_copy(statement.secondary_expression);
                    if (!raw_target.empty())
                    {
                        if (!detail.empty()) detail += " ";
                        detail += "target=" + raw_target;
                        const std::string resolved_target = evaluate_cursor_designator_expression(raw_target, frame);
                        if (!resolved_target.empty() && resolved_target != raw_target)
                        {
                            detail += " target_resolved=" + resolved_target;
                        }
                    }
                }
                else if (normalize_identifier(statement.identifier) == "structure")
                {
                    CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                    if (cursor == nullptr)
                    {
                        cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    }
                    append_cursor_structure_metadata(cursor, detail);
                    append_runtime_cursor_target_detail(statement.secondary_expression, frame, detail);
                }
                else if (normalize_identifier(statement.identifier) == "status")
                {
                    append_session_status_metadata(frame, detail);
                }
                else if (normalize_identifier(statement.identifier) == "memory")
                {
                    std::string mem_pattern;
                    bool mem_is_except = false;
                    const std::string mem_clause = trim_copy(statement.expression);
                    if (starts_with_insensitive(mem_clause, "LIKE "))
                    {
                        mem_pattern = trim_copy(mem_clause.substr(5U));
                    }
                    else if (starts_with_insensitive(mem_clause, "EXCEPT "))
                    {
                        mem_pattern = trim_copy(mem_clause.substr(7U));
                        mem_is_except = true;
                    }
                    append_memory_metadata(frame, detail, mem_pattern, mem_is_except);
                }
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "clause=" + statement.expression;
                }
                events.push_back({.category = "runtime.display",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::list_command:
            {
                std::string detail;
                if (!statement.identifier.empty())
                {
                    detail += "mode=" + statement.identifier;
                }
                if (normalize_identifier(statement.identifier) == "records")
                {
                    CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                    if (cursor == nullptr)
                    {
                        cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    }
                    if (cursor != nullptr)
                    {
                        const std::vector<std::string> visible_fields = effective_visible_field_names(*cursor, statement.tertiary_expression);
                        std::string field_detail;
                        for (std::size_t index = 0U; index < visible_fields.size(); ++index)
                        {
                            if (index > 0U)
                            {
                                field_detail += ",";
                            }
                            field_detail += visible_fields[index];
                        }

                        if (!detail.empty()) detail += " ";
                        detail += cursor->alias.empty()
                            ? ("workarea=" + std::to_string(cursor->work_area))
                            : (cursor->alias + "@" + std::to_string(cursor->work_area));
                        detail += " recno=" + std::to_string(cursor->recno);
                        detail += " records=" + std::to_string(cursor->record_count);
                        detail += " fields=" + (field_detail.empty() ? std::string{"ALL"} : field_detail);
                        detail += " filter=" + (cursor->filter_expression.empty() ? std::string{"<none>"} : cursor->filter_expression);
                        if (!statement.quaternary_expression.empty())
                        {
                            detail += " for=" + trim_copy(statement.quaternary_expression);
                        }
                        if (!statement.names.empty() && !trim_copy(statement.names.front()).empty())
                        {
                            detail += " while=" + trim_copy(statement.names.front());
                        }
                    }
                    const std::string raw_target = trim_copy(statement.secondary_expression);
                    if (!raw_target.empty())
                    {
                        if (!detail.empty()) detail += " ";
                        detail += "target=" + raw_target;
                        const std::string resolved_target = evaluate_cursor_designator_expression(raw_target, frame);
                        if (!resolved_target.empty() && resolved_target != raw_target)
                        {
                            detail += " target_resolved=" + resolved_target;
                        }
                    }
                }
                else if (normalize_identifier(statement.identifier) == "structure")
                {
                    CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                    if (cursor == nullptr)
                    {
                        cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    }
                    append_cursor_structure_metadata(cursor, detail);
                    append_runtime_cursor_target_detail(statement.secondary_expression, frame, detail);
                }
                else if (normalize_identifier(statement.identifier) == "status")
                {
                    append_session_status_metadata(frame, detail);
                }
                else if (normalize_identifier(statement.identifier) == "memory")
                {
                    std::string mem_pattern;
                    bool mem_is_except = false;
                    const std::string mem_clause = trim_copy(statement.expression);
                    if (starts_with_insensitive(mem_clause, "LIKE "))
                    {
                        mem_pattern = trim_copy(mem_clause.substr(5U));
                    }
                    else if (starts_with_insensitive(mem_clause, "EXCEPT "))
                    {
                        mem_pattern = trim_copy(mem_clause.substr(7U));
                        mem_is_except = true;
                    }
                    append_memory_metadata(frame, detail, mem_pattern, mem_is_except);
                }
                if (!statement.expression.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "clause=" + statement.expression;
                }
                events.push_back({.category = "runtime.list",
                                  .detail = detail,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::no_op:
                return {};
            case StatementKind::for_each_statement:
            {
                // FOR EACH <element> IN <collection>
                // Collection may be a declared array or any expression.
                const std::string var_name = normalize_memory_variable_identifier(statement.identifier);
                const std::string collection_expr = statement.expression;

                // Build the element snapshot at loop entry (only once per loop push)
                const auto existing_loop = std::find_if(frame.loops.rbegin(), frame.loops.rend(),
                    [&](const LoopState &l) { return l.for_statement_index == (frame.pc - 1U); });
                if (existing_loop != frame.loops.rend())
                {
                    // Already entered; the ENDFOR/LOOP continuation handles iteration
                    return {};
                }

                // Snapshot collection elements
                const std::string coll_norm = normalize_memory_variable_identifier(collection_expr);
                if (const RuntimeArray *arr = find_array(coll_norm); arr != nullptr)
                {
                    std::vector<PrgValue> elements = arr->values;
                    if (elements.empty())
                    {
                        if (const auto dest = find_matching_endfor(frame, frame.pc - 1U))
                        {
                            frame.pc = *dest + 1U;
                        }
                        return {};
                    }
                    assign_variable(frame, var_name, elements[0]);
                    frame.loops.push_back({
                        .for_statement_index = frame.pc - 1U,
                        .endfor_statement_index = find_matching_endfor(frame, frame.pc - 1U).value_or(frame.pc - 1U),
                        .case_stack_depth_at_entry = frame.cases.size(),
                        .with_stack_depth_at_entry = frame.withs.size(),
                        .variable_name = var_name,
                        .is_for_each = true,
                        .each_values = std::move(elements),
                        .each_index = 0U});
                }
                else
                {
                    // Evaluate as expression; native Collection objects iterate
                    // their contained items, and everything else is a single element.
                    frame.loop_expression_continuation = LoopExpressionContinuation{
                        .statement = statement,
                        .stage = LoopExpressionStage::for_each_collection};
                    const auto result = evaluate_resumable_expression(
                        frame,
                        make_loop_stage_statement(frame.loop_expression_continuation->statement,
                                                  collection_expr,
                                                  LoopExpressionStage::for_each_collection));
                    if (!result.has_value())
                    {
                        return {};
                    }
                    finish_loop_expression(frame, statement, *result);
                }
                return {};
            }
            case StatementKind::release_command:
            {
                if (statement.identifier == "all")
                {
                    // RELEASE ALL [LIKE <pattern> | EXCEPT <pattern>]
                    const std::string mode = statement.expression; // "like", "except", or ""
                    const std::string pattern = statement.secondary_expression;
                    std::vector<std::string> candidate_names;
                    candidate_names.reserve(
                        globals.size() + arrays.size() + frame.locals.size() + frame.local_arrays.size() + frame.local_names.size() +
                        frame.private_saved_values.size() + frame.private_saved_arrays.size());
                    for (const auto &[name, _] : globals)
                    {
                        candidate_names.push_back(name);
                    }
                    for (const auto &[name, _] : arrays)
                    {
                        if (std::find(candidate_names.begin(), candidate_names.end(), name) == candidate_names.end())
                        {
                            candidate_names.push_back(name);
                        }
                    }
                    for (const auto &[name, _] : frame.locals)
                    {
                        if (std::find(candidate_names.begin(), candidate_names.end(), name) == candidate_names.end())
                        {
                            candidate_names.push_back(name);
                        }
                    }
                    for (const auto &[name, _] : frame.local_arrays)
                    {
                        if (std::find(candidate_names.begin(), candidate_names.end(), name) == candidate_names.end())
                        {
                            candidate_names.push_back(name);
                        }
                    }
                    for (const auto &name : frame.local_names)
                    {
                        if (std::find(candidate_names.begin(), candidate_names.end(), name) == candidate_names.end())
                        {
                            candidate_names.push_back(name);
                        }
                    }
                    for (const auto &[name, _] : frame.private_saved_values)
                    {
                        if (std::find(candidate_names.begin(), candidate_names.end(), name) == candidate_names.end())
                        {
                            candidate_names.push_back(name);
                        }
                    }
                    for (const auto &[name, _] : frame.private_saved_arrays)
                    {
                        if (std::find(candidate_names.begin(), candidate_names.end(), name) == candidate_names.end())
                        {
                            candidate_names.push_back(name);
                        }
                    }
                    if (mode.empty())
                    {
                        std::vector<std::string> to_release;
                        for (const auto &name : candidate_names)
                        {
                            if (public_names.contains(name))
                            {
                                continue;
                            }
                            to_release.push_back(name);
                        }
                        for (const auto &name : to_release)
                        {
                            release_memory_binding(frame, name, false);
                        }
                    }
                    else
                    {
                        std::vector<std::string> to_erase;
                        for (const auto &name : candidate_names)
                        {
                            if (public_names.contains(name))
                            {
                                continue;
                            }
                            const bool matches = wildcard_match_insensitive(pattern, name);
                            if ((mode == "like" && matches) || (mode == "except" && !matches))
                            {
                                to_erase.push_back(name);
                            }
                        }
                        for (const auto &name : to_erase)
                        {
                            release_memory_binding(frame, name, false);
                        }
                    }
                }
                else
                {
                    // RELEASE <varlist>
                    std::set<int> released_special_handles;
                    for (const auto &raw : statement.names)
                    {
                        const std::string normalized_name =
                            normalize_memory_variable_identifier(trim_copy(raw));
                        if (normalized_name == "thisform" || normalized_name == "thisformset")
                        {
                            const auto local = frame.locals.find(normalized_name);
                            if (local != frame.locals.end())
                            {
                                if (auto runtime_object = resolve_ole_object(local->second);
                                    runtime_object.has_value() &&
                                    released_special_handles.insert((*runtime_object)->handle).second)
                                {
                                    (void)release_native_object(**runtime_object, normalized_name);
                                    continue;
                                }
                            }
                        }
                        if (release_object_memory_binding(frame, raw, released_special_handles))
                        {
                            continue;
                        }
                        release_memory_binding(frame, raw, true);
                    }
                }
                events.push_back({.category = "runtime.release",
                                  .detail = statement.identifier,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::clear_memory_command:
            {
                // CLEAR MEMORY — release all public/global variables and arrays
                // CLEAR ALL — same plus closes all tables and releases procedures
                globals.clear();
                arrays.clear();
                public_names.clear();
                for (auto &active_frame : stack)
                {
                    active_frame.private_saved_values.clear();
                    active_frame.private_saved_arrays.clear();
                    active_frame.locals.clear();
                    active_frame.local_arrays.clear();
                }
                if (statement.identifier == "all")
                {
                    // Also close all open work areas
                    for (auto &[session_id, session] : data_sessions)
                    {
                        session.cursors.clear();
                        session.aliases.clear();
                        session.table_locks.clear();
                        session.record_locks.clear();
                    }
                }
                events.push_back({.category = "runtime.clear_memory",
                                  .detail = statement.identifier,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::cancel_statement:
            {
                // CANCEL — abort execution and return to top level
                events.push_back({.category = "runtime.cancel",
                                  .detail = "CANCEL",
                                  .location = statement.location});
                cancel_all_async_tasks();
                abandon_expression_continuations();
                int &level = current_transaction_level();
                if (level > 0)
                {
                    if (!rollback_active_transaction_journal())
                    {
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    level = 0;
                    events.push_back({.category = "runtime.transaction.rollback",
                                      .detail = "0",
                                      .location = statement.location});
                }
                // Unwind entire call stack
                while (stack.size() > 1U)
                {
                    restore_private_declarations(stack.back());
                    stack.pop_back();
                }
                if (!stack.empty())
                {
                    Frame &top = stack.back();
                    if (top.routine != nullptr)
                    {
                        top.pc = top.routine->statements.size();
                    }
                }
                return {};
            }
            case StatementKind::quit_statement:
            {
                // QUIT — ask host via callback whether the user confirms quitting.
                // In VFP this could produce "Cannot quit Visual FoxPro" in certain
                // contexts. Here we give the host a chance to show a confirmation
                // dialog; if the user declines, QUIT is silently cancelled.
                if (options.quit_confirm_callback && !options.quit_confirm_callback())
                {
                    // User chose not to quit — cancel and continue after QUIT statement
                    events.push_back({.category = "runtime.quit_cancelled",
                                      .detail = "QUIT cancelled by user",
                                      .location = statement.location});
                    return {};
                }

                // First-pass ON SHUTDOWN compatibility: support a shutdown routine
                // before the final QUIT cleanup/unwind executes.
                if (!handling_shutdown)
                {
                    if (execute_inline_shutdown_clause(statement.location))
                    {
                    }
                    else if (dispatch_shutdown_handler(frame, statement.location))
                    {
                        return {};
                    }
                }

                // Confirmed (or no callback) — unwind entire call stack.
                // QUIT represents application shutdown intent; make event-loop
                // cleanup implicit so callers do not need explicit CLEAR EVENTS.
                perform_quit(statement.location);
                return {};
            }
        }
        }
        catch (const PrgCompatibilityError &error)
        {
            abandon_resumed_expression();
            last_error_message = error.what();
            last_error_code = error.error_code();
            last_error_compatibility = {};
            last_error_compatibility.explicit_error_code = last_error_code;
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (const PrgPropagatedRuntimeError &error)
        {
            abandon_resumed_expression();
            if (last_error_message.empty())
            {
                last_error_message = error.what();
            }
            return {.ok = false, .message = last_error_message};
        }
        catch (const std::bad_alloc &)
        {
            abandon_resumed_expression();
            clear_caught_exception_identity(last_error_compatibility);
            last_error_message = runtime_text("Runtime.Prg.Core.Error.ResourceOutOfMemory");
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (const std::filesystem::filesystem_error &error)
        {
            abandon_resumed_expression();
            clear_caught_exception_identity(last_error_compatibility);
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.ResourceFilesystemFailure",
                {
                    {"detail", error.what()},
                });
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (const std::system_error &error)
        {
            abandon_resumed_expression();
            clear_caught_exception_identity(last_error_compatibility);
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.ResourceSystemError",
                {
                    {"detail", error.what()},
                });
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (const std::exception &error)
        {
            abandon_resumed_expression();
            clear_caught_exception_identity(last_error_compatibility);
            last_error_message = runtime_text(
                "Runtime.Prg.Core.Error.RuntimeFault",
                {
                    {"detail", error.what()},
                });
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (...)
        {
            abandon_resumed_expression();
            clear_caught_exception_identity(last_error_compatibility);
            last_error_message = runtime_text("Runtime.Prg.Core.Error.UnknownRuntimeFault");
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }

        return {};
    }
