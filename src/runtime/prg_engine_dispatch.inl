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
        if (frame.routine == nullptr || frame.pc >= frame.routine->statements.size())
        {
            pop_frame();
            return {};
        }

        const Statement statement = frame.routine->statements[frame.pc];
        ++frame.pc;
        ++executed_statement_count;

        events.push_back({.category = "execute",
                          .detail = statement.text,
                          .location = statement.location});

        try
        {
            switch (statement.kind)
            {
            case StatementKind::assignment:
            {
                std::string assignment_identifier = apply_with_context(statement.identifier, frame);
                if (!assignment_identifier.empty() && assignment_identifier.front() == '&')
                {
                    const PrgValue expanded_identifier = evaluate_expression(assignment_identifier, frame);
                    const std::string expanded_text = trim_copy(value_as_string(expanded_identifier));
                    if (!expanded_text.empty())
                    {
                        assignment_identifier = expanded_text;
                    }
                }
                const PrgValue assignment_value = evaluate_expression(statement.expression, frame);
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
                    const PrgValue object_value = lookup_variable(frame, object_part);
                    auto object = resolve_ole_object(object_value);
                    if (!object.has_value())
                    {
                        last_error_message = "OLE object not found for property assignment: " + assignment_identifier;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    RuntimeOleObjectState *runtime_object = *object;
                    runtime_object->last_action = assignment_identifier.substr(separator + 1U) + " = " + value_as_string(assignment_value);
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
            }
            case StatementKind::expression:
                if (!statement.expression.empty())
                {
                    if (starts_with_insensitive(statement.expression, "WAIT WINDOW "))
                    {
                        events.push_back({.category = "ui.wait_window",
                                          .detail = value_as_string(evaluate_expression(statement.expression.substr(12U), frame)),
                                          .location = statement.location});
                    }
                    else
                    {
                        (void)evaluate_expression(statement.expression, frame);
                    }
                }
                return {};
            case StatementKind::do_command:
            {
                std::string target = trim_copy(statement.identifier);
                if (!target.empty() && target.front() == '&')
                {
                    const std::string expanded_target = trim_copy(value_as_string(evaluate_expression(target, frame)));
                    if (!expanded_target.empty())
                    {
                        target = expanded_target;
                    }
                }
                std::vector<PrgValue> call_arguments;
                std::vector<std::optional<std::string>> call_argument_references;
                if (!trim_copy(statement.expression).empty())
                {
                    for (const std::string &raw_argument : split_csv_like(statement.expression))
                    {
                        const std::string argument_expression = trim_copy(raw_argument);
                        if (!argument_expression.empty())
                        {
                            if (argument_expression.front() == '@')
                            {
                                const std::string reference_name = trim_copy(argument_expression.substr(1U));
                                if (is_bare_identifier_text(reference_name))
                                {
                                    call_arguments.push_back(lookup_variable(frame, reference_name));
                                    call_argument_references.push_back(reference_name);
                                    continue;
                                }
                            }
                            call_arguments.push_back(evaluate_expression(argument_expression, frame));
                            call_argument_references.push_back(std::nullopt);
                        }
                    }
                }
                Program &program = load_program(frame.file_path);
                if (const auto routine = program.routines.find(normalize_identifier(target)); routine != program.routines.end())
                {
                    if (!can_push_frame())
                    {
                        last_error_message = call_depth_limit_message();
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    push_routine_frame(
                        program.path,
                        routine->second,
                        std::move(call_arguments),
                        std::move(call_argument_references));
                    return {};
                }

                std::filesystem::path target_path(target);
                if (target_path.extension().empty())
                {
                    target_path += ".prg";
                }
                if (target_path.is_relative())
                {
                    target_path = std::filesystem::path(current_default_directory()) / target_path;
                }
                if (!std::filesystem::exists(target_path))
                {
                    last_error_message = "Unable to resolve DO target: " + target;
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
                    target_path.string(),
                    std::move(call_arguments),
                    std::move(call_argument_references));
                return {};
            }
                case StatementKind::call_command:
                {
                    std::string target = trim_copy(statement.identifier);
                    if (!target.empty() && target.front() == '&')
                    {
                        const std::string expanded_target = trim_copy(value_as_string(evaluate_expression(target, frame)));
                        if (!expanded_target.empty())
                        {
                            target = expanded_target;
                        }
                    }
                    std::vector<PrgValue> call_arguments;
                    std::vector<std::optional<std::string>> call_argument_references;
                    // CALL does not take arguments in classic VFP, but Copperfin may extend for macro compatibility
                    // For now, mimic DO semantics for macro/compat surface
                    if (!trim_copy(statement.expression).empty())
                    {
                        for (const std::string &raw_argument : split_csv_like(statement.expression))
                        {
                            const std::string argument_expression = trim_copy(raw_argument);
                            if (!argument_expression.empty())
                            {
                                if (argument_expression.front() == '@')
                                {
                                    const std::string reference_name = trim_copy(argument_expression.substr(1U));
                                    if (is_bare_identifier_text(reference_name))
                                    {
                                        call_arguments.push_back(lookup_variable(frame, reference_name));
                                        call_argument_references.push_back(reference_name);
                                        continue;
                                    }
                                }
                                call_arguments.push_back(evaluate_expression(argument_expression, frame));
                                call_argument_references.push_back(std::nullopt);
                            }
                        }
                    }
                    Program &program = load_program(frame.file_path);
                    if (const auto routine = program.routines.find(normalize_identifier(target)); routine != program.routines.end())
                    {
                        if (!can_push_frame())
                        {
                            last_error_message = call_depth_limit_message();
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        push_routine_frame(
                            program.path,
                            routine->second,
                            std::move(call_arguments),
                            std::move(call_argument_references));
                        return {};
                    }
                    last_error_message = "Unable to resolve CALL target: " + target;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
            case StatementKind::do_form:
            {
                const std::filesystem::path form_path = resolve_asset_path(statement.identifier, ".scx");
                events.push_back({.category = "form.open",
                                  .detail = form_path.lexically_normal().string(),
                                  .location = statement.location});
                if (std::filesystem::exists(form_path))
                {
                    if (const auto bootstrap_path = materialize_xasset_bootstrap(form_path.string(), true))
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
                events.push_back({.category = "runtime.calculate",
                                  .detail = statement.expression,
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
                events.push_back({.category = category,
                                  .detail = statement.text,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::text_command:
            {
                if (statement.identifier.empty())
                {
                    last_error_message = "TEXT requires TO <variable> in the current runtime slice";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::string text_value = statement.expression;
                if (normalize_identifier(statement.tertiary_expression) == "textmerge")
                {
                    std::string merged_text;
                    merged_text.reserve(text_value.size());

                    std::size_t cursor = 0U;
                    while (cursor < text_value.size())
                    {
                        const std::size_t start = text_value.find("<<", cursor);
                        if (start == std::string::npos)
                        {
                            merged_text.append(text_value.substr(cursor));
                            break;
                        }

                        merged_text.append(text_value.substr(cursor, start - cursor));
                        const std::size_t end = text_value.find(">>", start + 2U);
                        if (end == std::string::npos)
                        {
                            merged_text.append(text_value.substr(start));
                            break;
                        }

                        const std::string merge_expression = trim_copy(text_value.substr(start + 2U, end - start - 2U));
                        if (!merge_expression.empty())
                        {
                            merged_text.append(value_as_string(evaluate_expression(merge_expression, frame)));
                        }

                        cursor = end + 2U;
                    }

                    text_value = std::move(merged_text);
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
                events.push_back({.category = "runtime.total",
                                  .detail = statement.text,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::report_form:
                return open_report_surface(statement, frame, ".frx", "report");
            case StatementKind::label_form:
                return open_report_surface(statement, frame, ".lbx", "label");
            case StatementKind::activate_surface:
                waiting_for_events = true;
                events.push_back({.category = statement.identifier + ".activate",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {.ok = true, .waiting_for_events = true, .frame_returned = false, .message = {}};
            case StatementKind::release_surface:
                waiting_for_events = false;
                events.push_back({.category = statement.identifier + ".release",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            case StatementKind::return_statement:
                pop_frame();
                return {.ok = true, .waiting_for_events = false, .frame_returned = true, .message = {}};
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

                CaseState &active_case = frame.cases.back();
                if (active_case.matched)
                {
                    const std::size_t next_pc = active_case.endcase_statement_index + 1U;
                    frame.cases.pop_back();
                    frame.pc = next_pc;
                    return {};
                }

                if (value_as_bool(evaluate_expression(statement.expression, frame)))
                {
                    active_case.matched = true;
                    return {};
                }

                if (const auto destination = find_next_case_clause(frame, frame.pc - 1U))
                {
                    frame.pc = *destination;
                }
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
                if (!value_as_bool(evaluate_expression(statement.expression, frame)))
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
                }
                else
                {
                    frame.evaluate_conditional_else = false;
                }
                return {};
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

                    frame.evaluate_conditional_else = false;
                    if (value_as_bool(evaluate_expression(statement.expression, frame)))
                    {
                        return {};
                    }
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
                const double start_value = value_as_number(evaluate_expression(statement.expression, frame));
                const double end_value = value_as_number(evaluate_expression(statement.secondary_expression, frame));
                const double step_value = statement.tertiary_expression.empty()
                                              ? 1.0
                                              : value_as_number(evaluate_expression(statement.tertiary_expression, frame));
                assign_variable(frame, statement.identifier, make_number_value(start_value));
                const bool should_enter = step_value >= 0.0 ? start_value <= end_value : start_value >= end_value;
                if (!should_enter)
                {
                    if (const auto destination = find_matching_endfor(frame, frame.pc - 1U))
                    {
                        frame.pc = *destination + 1U;
                    }
                    return {};
                }
                const auto existing = std::find_if(frame.loops.rbegin(), frame.loops.rend(), [&](const LoopState &loop)
                                                   { return loop.for_statement_index == (frame.pc - 1U); });
                if (existing != frame.loops.rend())
                {
                    return {};
                }
                frame.loops.push_back({.for_statement_index = frame.pc - 1U,
                                       .endfor_statement_index = find_matching_endfor(frame, frame.pc - 1U).value_or(frame.pc - 1U),
                                       .variable_name = normalize_identifier(statement.identifier),
                                       .end_value = end_value,
                                       .step_value = step_value,
                                       .iteration_count = 0});
                return {};
            }
            case StatementKind::do_while_statement:
            {
                const bool should_continue = value_as_bool(evaluate_expression(statement.expression, frame));
                const auto existing = std::find_if(frame.whiles.rbegin(), frame.whiles.rend(), [&](const WhileState &loop)
                                                   { return loop.do_while_statement_index == (frame.pc - 1U); });
                if (should_continue)
                {
                    if (existing == frame.whiles.rend())
                    {
                        frame.whiles.push_back({.do_while_statement_index = frame.pc - 1U,
                                                .enddo_statement_index = find_matching_enddo(frame, frame.pc - 1U).value_or(frame.pc - 1U),
                                                .iteration_count = 0});
                    }
                }
                else
                {
                    if (existing != frame.whiles.rend())
                    {
                        frame.whiles.erase(std::next(existing).base());
                    }
                    if (const auto destination = find_matching_enddo(frame, frame.pc - 1U))
                    {
                        frame.pc = *destination + 1U;
                    }
                }
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
                    frame.pc = active_loop->start_statement_index;
                    return {};
                }
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
                    frame.loops.pop_back();
                    frame.pc = active_loop->end_statement_index + 1U;
                    return {};
                case ActiveLoopKind::scan_loop:
                    frame.scans.pop_back();
                    frame.pc = active_loop->end_statement_index + 1U;
                    return {};
                case ActiveLoopKind::while_loop:
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
                const PrgValue target = evaluate_expression(statement.expression, frame);
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
                    last_error_message = "TRY block is missing ENDTRY";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                std::string catch_variable;
                if (targets.catch_statement_index.has_value())
                {
                    catch_variable = normalize_identifier(frame.routine->statements[*targets.catch_statement_index].identifier);
                }
                frame.tries.push_back({.try_statement_index = frame.pc - 1U,
                                       .catch_statement_index = targets.catch_statement_index,
                                       .finally_statement_index = targets.finally_statement_index,
                                       .endtry_statement_index = *targets.endtry_statement_index,
                                       .catch_variable = catch_variable,
                                       .handling_error = false,
                                       .entered_catch = false,
                                       .entered_finally = false});
                return {};
            }
            case StatementKind::catch_statement:
                if (!frame.tries.empty() && frame.tries.back().catch_statement_index == (frame.pc - 1U))
                {
                    const TryState active_try = frame.tries.back();
                    if (!active_try.handling_error)
                    {
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
                    frame.tries.pop_back();
                }
                return {};
            case StatementKind::read_events:
                waiting_for_events = true;
                events.push_back({.category = "runtime.event_loop",
                                  .detail = "READ EVENTS entered",
                                  .location = statement.location});
                return {.ok = true, .waiting_for_events = true, .frame_returned = false, .message = {}};
            case StatementKind::clear_events:
                waiting_for_events = false;
                restore_event_loop_after_dispatch = false;
                events.push_back({.category = "runtime.event_loop",
                                  .detail = "CLEAR EVENTS",
                                  .location = statement.location});
                return {};
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
                    last_error_message = "SEEK target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::string search_key = value_as_string(evaluate_expression(statement.expression, frame));
                std::string used_order_name;
                std::string used_order_normalization_hint;
                std::string used_order_collation_hint;
                bool used_order_descending = false;
                const bool found = execute_seek(
                    *cursor,
                    search_key,
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
                    last_error_message = "LOCATE target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

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
                    last_error_message = "SCAN target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::size_t start_recno = cursor->recno == 0U ? 1U : cursor->recno;
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
                    last_error_message = "REPLACE target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::vector<ReplaceAssignment> assignments = parse_replace_assignments(statement.expression);
                if (assignments.empty())
                {
                    last_error_message = "REPLACE requires at least one FIELD WITH expression assignment";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!replace_records(*cursor, assignments, frame, statement.tertiary_expression, statement.quaternary_expression))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string replace_detail = statement.expression;
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
                    last_error_message = "UPDATE target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::vector<ReplaceAssignment> assignments = parse_update_set_assignments(statement.expression);
                if (assignments.empty())
                {
                    last_error_message = "UPDATE requires SET field = expression assignments";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const std::string for_expression = trim_copy(statement.tertiary_expression).empty()
                                                       ? ".T."
                                                       : statement.tertiary_expression;
                if (!replace_records(*cursor, assignments, frame, for_expression, statement.quaternary_expression))
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
                    last_error_message = "APPEND BLANK target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!append_blank_record(*cursor))
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
                    last_error_message = "DELETE target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!set_deleted_flag(*cursor, frame, statement.expression, statement.tertiary_expression, true))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string delete_detail = statement.expression.empty() ? cursor->alias : statement.expression;
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
                    last_error_message = "DELETE FROM target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const std::string where_expression = trim_copy(statement.expression).empty()
                                                         ? ".T."
                                                         : statement.expression;
                if (!set_deleted_flag(*cursor, frame, where_expression, statement.tertiary_expression, true))
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
                    last_error_message = "RECALL target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!set_deleted_flag(*cursor, frame, statement.expression, statement.tertiary_expression, false))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string recall_detail = statement.expression.empty() ? cursor->alias : statement.expression;
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
                    last_error_message = "INSERT INTO target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (trim_copy(statement.secondary_expression).empty())
                {
                    last_error_message = "INSERT INTO requires a VALUES clause";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!insert_record_values(*cursor, frame, statement.expression, statement.secondary_expression))
                {
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
                    last_error_message = "PACK target work area not found";
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
                    const auto pack_result = vfp::pack_dbf_memo_file(cursor->source_path);
                    if (!pack_result.ok)
                    {
                        last_error_message = pack_result.error;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    cursor->record_count = pack_result.record_count;
                    move_cursor_to(*cursor, static_cast<long long>(std::min(cursor->recno, cursor->record_count)));
                }
                else
                {
                    if (!pack_cursor(*cursor))
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
                    last_error_message = "ZAP target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!zap_cursor(*cursor))
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
            case StatementKind::go_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = "GO target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::string destination = uppercase_copy(trim_copy(statement.expression));
                if (destination == "TOP")
                {
                    if (!seek_visible_record(*cursor, frame, 1, 1, {}, {}, false) && cursor->record_count == 0U)
                    {
                        move_cursor_to(*cursor, 0);
                    }
                }
                else if (destination == "BOTTOM")
                {
                    if (!seek_visible_record(*cursor, frame, static_cast<long long>(cursor->record_count), -1, {}, {}, false) && cursor->record_count == 0U)
                    {
                        move_cursor_to(*cursor, 0);
                    }
                }
                else
                {
                    const long long requested = std::llround(value_as_number(evaluate_expression(statement.expression, frame)));
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
                    last_error_message = "SKIP target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const long long delta = std::llround(value_as_number(evaluate_expression(statement.expression, frame)));
                if (!move_by_visible_records(*cursor, frame, delta))
                {
                    cursor->found = false;
                }
                events.push_back({.category = "runtime.skip",
                                  .detail = statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_order:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = "SET ORDER target work area not found";
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
                        last_error_message = "SELECT target work area not found: " + selection;
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
                    close_cursor(std::to_string(current_selected_work_area()));
                    events.push_back({.category = "runtime.use.close",
                                      .detail = "current work area",
                                      .location = statement.location});
                    return {};
                }
                if (statement.expression.empty())
                {
                    close_cursor(evaluate_cursor_designator_expression(statement.secondary_expression, frame));
                    events.push_back({.category = "runtime.use.close",
                                      .detail = trim_copy(statement.secondary_expression),
                                      .location = statement.location});
                    return {};
                }

                const std::string target = value_as_string(evaluate_expression(statement.expression, frame));
                std::string alias = statement.identifier.empty()
                                        ? std::filesystem::path(unquote_string(target)).stem().string()
                                        : value_as_string(evaluate_expression(statement.identifier, frame));
                if (alias.empty() && !statement.identifier.empty())
                {
                    alias = unquote_string(statement.identifier);
                }
                const bool allow_again = normalize_identifier(statement.tertiary_expression) == "again";
                std::string in_target = statement.secondary_expression;
                if (allow_again && trim_copy(in_target).empty())
                {
                    in_target = "0";
                }
                if (!open_table_cursor(target, alias, in_target, allow_again, false, 0, {}, 0U))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.use.open",
                                  .detail = alias.empty() ? target : alias + " <- " + target,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_command:
            {
                const auto [option_name, option_value] = split_first_word(statement.expression);
                const std::string normalized_name = normalize_identifier(option_name);
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
                        if (evaluated.kind == PrgValueKind::number || evaluated.kind == PrgValueKind::int64 || evaluated.kind == PrgValueKind::uint64)
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

                if (normalized_name == "filter")
                {
                    std::string filter_clause = trim_copy(option_value);
                    if (starts_with_insensitive(filter_clause, "TO "))
                    {
                        filter_clause = trim_copy(filter_clause.substr(3U));
                    }

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
                        last_error_message = "SET FILTER requires a selected work area";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    if (normalize_identifier(filter_clause) == "off")
                    {
                        filter_clause.clear();
                    }

                    cursor->filter_expression = filter_clause;
                    if (!cursor->filter_expression.empty() && cursor->record_count > 0U &&
                        !current_record_matches_visibility(*cursor, frame, {}))
                    {
                        (void)seek_visible_record(*cursor, frame, static_cast<long long>(cursor->recno), 1, {}, {}, false);
                    }

                    events.push_back({.category = "runtime.filter",
                                      .detail = cursor->filter_expression.empty() ? "OFF" : cursor->filter_expression,
                                      .location = statement.location});
                    return {};
                }
                if (!normalized_name.empty())
                {
                    if (normalized_name == "exact" || normalized_name == "deleted" || normalized_name == "near" ||
                        normalized_name == "strictdate" || normalized_name == "optimize")
                    {
                        current_set_state()[normalized_name] = normalize_boolean_set_value(option_value.empty() ? "on" : option_value);
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
                const std::string library_name = normalize_identifier(value_as_string(evaluate_expression(statement.expression, frame)));
                if (!library_name.empty())
                {
                    loaded_libraries.insert(library_name);
                }
                events.push_back({.category = "runtime.library",
                                  .detail = statement.expression,
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
                const std::string dll_path_raw = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                const std::string ret_type = uppercase_copy(trim_copy(statement.secondary_expression));
                const std::string param_types_str = trim_copy(statement.tertiary_expression);
                const std::string alias_raw = trim_copy(statement.quaternary_expression);
                const std::string alias = alias_raw.empty() ? fn_name : alias_raw;
                const std::string alias_key = normalize_identifier(alias);

                if (fn_name.empty() || dll_path_raw.empty())
                {
                    last_error_message = "DECLARE: missing function name or DLL path.";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                // Resolve path relative to current default directory
                std::filesystem::path dll_fspath(dll_path_raw);
                if (dll_fspath.is_relative())
                {
                    dll_fspath = std::filesystem::path(current_default_directory()) / dll_fspath;
                }
                const std::wstring dll_wpath = dll_fspath.wstring();

                DeclaredDllFunction declfn;
                declfn.alias = alias;
                declfn.function_name = fn_name;
                declfn.dll_path = dll_fspath.string();
                declfn.return_type = ret_type;
                declfn.param_types = param_types_str;

                // Attempt native LoadLibrary (.dll or .fll both treated the same)
                HMODULE hmod = LoadLibraryW(dll_wpath.c_str());
                if (!hmod)
                {
                    // Check if it's a .NET assembly by inspecting the PE CLR header
                    // PE Optional Header offset 0x168 (for PE32) or 0x178 (PE32+) contains CLR directory
                    bool is_dotnet_assembly = false;
                    {
                        HANDLE hfile = CreateFileW(dll_wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                        if (hfile != INVALID_HANDLE_VALUE)
                        {
                            // Read IMAGE_DOS_HEADER
                            IMAGE_DOS_HEADER dos_header{};
                            DWORD bytes_read = 0;
                            if (ReadFile(hfile, &dos_header, sizeof(dos_header), &bytes_read, nullptr) &&
                                bytes_read == sizeof(dos_header) && dos_header.e_magic == IMAGE_DOS_SIGNATURE)
                            {
                                // Seek to PE header
                                SetFilePointer(hfile, static_cast<LONG>(dos_header.e_lfanew), nullptr, FILE_BEGIN);
                                IMAGE_NT_HEADERS nt_headers{};
                                if (ReadFile(hfile, &nt_headers, sizeof(nt_headers), &bytes_read, nullptr) &&
                                    bytes_read >= sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) &&
                                    nt_headers.Signature == IMAGE_NT_SIGNATURE)
                                {
                                    // Check CLR data directory (index 14 = IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
                                    const DWORD magic = nt_headers.OptionalHeader.Magic;
                                    if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC || magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
                                    {
                                        const auto &clr_dir = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
                                        if (clr_dir.VirtualAddress != 0 && clr_dir.Size != 0)
                                        {
                                            is_dotnet_assembly = true;
                                        }
                                    }
                                }
                            }
                            CloseHandle(hfile);
                        }
                    }

                    if (!is_dotnet_assembly)
                    {
                        const DWORD err = GetLastError();
                        char msg_buf[256]{};
                        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                                       err, 0, msg_buf, sizeof(msg_buf) - 1U, nullptr);
                        last_error_message = "DECLARE: cannot load '" + declfn.dll_path + "': " + std::string(msg_buf);
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        if (dispatch_error_handler())
                            return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                        return {.ok = false, .message = last_error_message};
                    }

                    // .NET assembly — mark for CLR invocation
                    declfn.is_dotnet = true;
                    // Parse Type.Method from function_name (convention: "Namespace.Type.Method" or "Type.Method")
                    const std::string full_name = fn_name;
                    const auto last_dot = full_name.rfind('.');
                    if (last_dot != std::string::npos && last_dot > 0U)
                    {
                        declfn.dotnet_type_name = full_name.substr(0U, last_dot);
                        declfn.dotnet_method_name = full_name.substr(last_dot + 1U);
                    }
                    else
                    {
                        declfn.dotnet_type_name = std::string{};
                        declfn.dotnet_method_name = full_name;
                    }
                }
                else
                {
                    declfn.hmodule = hmod;
                    declfn.proc_address = GetProcAddress(hmod, fn_name.c_str());
                    if (!declfn.proc_address)
                    {
                        // Try decorated name with leading underscore (cdecl x86)
                        declfn.proc_address = GetProcAddress(hmod, ("_" + fn_name).c_str());
                    }
                    if (!declfn.proc_address)
                    {
                        last_error_message = "DECLARE: function '" + fn_name + "' not found in '" + declfn.dll_path + "'.";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        FreeLibrary(hmod);
                        if (dispatch_error_handler())
                            return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                        return {.ok = false, .message = last_error_message};
                    }
                }

                declared_dll_functions[alias_key] = std::move(declfn);
                events.push_back({.category = "runtime.declare_dll",
                                  .detail = alias + " IN " + dll_path_raw,
                                  .location = statement.location});
                return {};
#else
                last_error_message = "DECLARE DLL is only supported on Windows.";
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                if (dispatch_error_handler())
                    return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                return {.ok = false, .message = last_error_message};
#endif
            }
            case StatementKind::set_datasession:
            {
                const int session_id = static_cast<int>(std::llround(value_as_number(evaluate_expression(statement.expression, frame))));
                current_data_session = std::max(1, session_id);
                (void)current_session_state();
                events.push_back({.category = "runtime.datasession",
                                  .detail = "SET DATASESSION TO " + std::to_string(current_data_session),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::set_default:
            {
                const std::string evaluated = value_as_string(evaluate_expression(statement.expression, frame));
                if (!evaluated.empty())
                {
                    current_default_directory() = normalize_path(evaluated);
                }
                return {};
            }
            case StatementKind::set_memowidth:
            {
                const double width_value = value_as_number(evaluate_expression(statement.expression, frame));
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
            case StatementKind::public_declaration:
                for (const auto &name : statement.names)
                {
                    globals.try_emplace(normalize_memory_variable_identifier(name), make_empty_value());
                }
                return {};
            case StatementKind::local_declaration:
                for (const auto &name : statement.names)
                {
                    const std::string normalized = normalize_memory_variable_identifier(name);
                    frame.local_names.insert(normalized);
                    frame.locals.try_emplace(normalized, make_empty_value());
                }
                return {};
            case StatementKind::private_declaration:
                for (const auto &name : statement.names)
                {
                    const std::string normalized = normalize_memory_variable_identifier(name);
                    const auto existing = globals.find(normalized);
                    if (existing != globals.end())
                    {
                        frame.private_saved_values.try_emplace(normalized, existing->second);
                        existing->second = make_empty_value();
                    }
                    else
                    {
                        frame.private_saved_values.try_emplace(normalized, std::nullopt);
                        globals[normalized] = make_empty_value();
                    }
                }
                return {};
            case StatementKind::parameters_declaration:
            case StatementKind::lparameters_declaration:
            {
                for (std::size_t index = 0U; index < statement.names.size(); ++index)
                {
                    const std::string normalized = normalize_memory_variable_identifier(statement.names[index]);
                    if (normalized.empty())
                    {
                        continue;
                    }
                    frame.local_names.insert(normalized);
                    if (index < frame.call_arguments.size())
                    {
                        frame.locals[normalized] = frame.call_arguments[index];
                    }
                    else
                    {
                        frame.locals[normalized] = make_empty_value();
                    }
                    if (index < frame.call_argument_references.size() && frame.call_argument_references[index].has_value())
                    {
                        frame.parameter_reference_bindings[normalized] = *frame.call_argument_references[index];
                    }
                }
                return {};
            }
            case StatementKind::dimension_command:
            {
                for (const auto &name : statement.names)
                {
                    if (!declare_array(name, frame))
                    {
                        last_error_message = "DIMENSION/DECLARE requires array dimensions";
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
                const PrgValue result = evaluate_expression(statement.expression, frame);
                for (const auto &name : statement.names)
                {
                    assign_variable(frame, trim_copy(name), result);
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
                const std::string raw_path = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                std::filesystem::path fpath(raw_path);
                if (fpath.is_relative())
                {
                    fpath = std::filesystem::path(current_default_directory()) / fpath;
                }
                std::error_code ec;
                std::filesystem::remove(fpath, ec);
                if (ec)
                {
                    last_error_message = "ERASE failed: " + ec.message() + " (" + fpath.string() + ")";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.erase",
                                  .detail = fpath.string(),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::copy_file_command:
            {
                // COPY FILE <src> TO <dest>
                const std::string src_raw = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                const std::string dst_raw = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.secondary_expression, frame))));
                auto make_abs = [&](const std::string &raw)
                {
                    std::filesystem::path p(raw);
                    if (p.is_relative())
                    {
                        p = std::filesystem::path(current_default_directory()) / p;
                    }
                    return p;
                };
                const std::filesystem::path src = make_abs(src_raw);
                const std::filesystem::path dst = make_abs(dst_raw);
                std::error_code ec;
                std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
                if (ec)
                {
                    last_error_message = "COPY FILE failed: " + ec.message();
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.copy_file",
                                  .detail = src.string() + " -> " + dst.string(),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::rename_file_command:
            {
                // RENAME <old> TO <new>
                const std::string old_raw = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                const std::string new_raw = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.secondary_expression, frame))));
                auto make_abs = [&](const std::string &raw)
                {
                    std::filesystem::path p(raw);
                    if (p.is_relative())
                    {
                        p = std::filesystem::path(current_default_directory()) / p;
                    }
                    return p;
                };
                const std::filesystem::path old_path = make_abs(old_raw);
                const std::filesystem::path new_path = make_abs(new_raw);
                std::error_code ec;
                std::filesystem::rename(old_path, new_path, ec);
                if (ec)
                {
                    last_error_message = "RENAME failed: " + ec.message();
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.rename",
                                  .detail = old_path.string() + " -> " + new_path.string(),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::print_command:
            {
                // ? or ?? expression — evaluate and emit as output event
                const PrgValue result = evaluate_expression(statement.expression, frame);
                const std::string text_value = value_as_string(result);
                events.push_back({.category = "runtime.print",
                                  .detail = text_value,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::create_cursor_command:
            {
                // CREATE CURSOR <alias> (<field_list>) — stub: open as an empty in-memory cursor
                const std::string alias = statement.identifier.empty()
                                              ? "CURSOR1"
                                              : normalize_identifier(value_as_string(evaluate_expression(statement.identifier, frame)));
                if (alias.empty())
                {
                    last_error_message = "CREATE CURSOR requires a non-empty alias";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!open_table_cursor({}, alias, {}, true, true, 0, {}, 0U))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.create_cursor",
                                  .detail = alias,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::create_table_command:
            {
                std::string target = trim_copy(statement.identifier);
                if (target.empty())
                {
                    last_error_message = "CREATE TABLE requires a target table name";
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

                std::filesystem::path table_path(target);
                if (table_path.extension().empty())
                {
                    table_path += ".dbf";
                }
                if (table_path.is_relative())
                {
                    table_path = std::filesystem::path(current_default_directory()) / table_path;
                }
                table_path = table_path.lexically_normal();

                const auto declarations = parse_table_field_declarations(statement.expression);
                const std::vector<vfp::DbfFieldDescriptor> fields = table_field_descriptors(declarations);
                if (fields.empty())
                {
                    last_error_message = "CREATE TABLE requires at least one supported field declaration";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto create_result = vfp::create_dbf_table_file(table_path.string(), fields, {});
                if (!create_result.ok)
                {
                    last_error_message = create_result.error;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const std::string alias = normalize_identifier(table_path.stem().string());
                const auto field_rules = field_rules_from_declarations(declarations);
                if (!open_table_cursor(table_path.string(), alias, {}, true, false, 0, {}, 0U, field_rules))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.create_table",
                                  .detail = table_path.string(),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::alter_table_command:
            {
                const std::string action = normalize_identifier(statement.secondary_expression);
                if (action != "add" && action != "drop" && action != "alter")
                {
                    last_error_message = "ALTER TABLE currently supports ADD COLUMN, DROP COLUMN, and ALTER COLUMN only";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::string target = trim_copy(statement.identifier);
                if (target.empty())
                {
                    last_error_message = "ALTER TABLE requires a target table name";
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

                std::filesystem::path table_path(target);
                if (table_path.extension().empty())
                {
                    table_path += ".dbf";
                }
                if (table_path.is_relative())
                {
                    table_path = std::filesystem::path(current_default_directory()) / table_path;
                }
                table_path = table_path.lexically_normal();

                vfp::DbfWriteResult add_result;
                std::optional<TableFieldDeclaration> declaration;
                std::string affected_field = trim_copy(statement.expression);
                if (action == "add" || action == "alter")
                {
                    declaration = parse_table_field_declaration(statement.expression);
                    if (!declaration.has_value())
                    {
                        last_error_message = action == "add"
                                                 ? "ALTER TABLE ADD COLUMN requires a supported field declaration"
                                                 : "ALTER TABLE ALTER COLUMN requires a supported field declaration";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    affected_field = declaration->descriptor.name;
                    add_result = action == "add"
                                     ? vfp::add_dbf_table_field(table_path.string(), declaration->descriptor)
                                     : vfp::alter_dbf_table_field(table_path.string(), declaration->descriptor);
                }
                else
                {
                    affected_field = unquote_identifier(affected_field);
                    add_result = vfp::drop_dbf_table_field(table_path.string(), affected_field);
                }
                if (!add_result.ok)
                {
                    last_error_message = add_result.error;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                for (auto &[_, cursor] : current_session_state().cursors)
                {
                    if (!cursor.remote && normalize_path(cursor.source_path) == normalize_path(table_path.string()))
                    {
                        if (action == "add")
                        {
                            cursor.field_count += 1U;
                        }
                        else if (action == "drop" && cursor.field_count > 0U)
                        {
                            cursor.field_count -= 1U;
                        }
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

                events.push_back({.category = "runtime.alter_table",
                                  .detail = table_path.string() + " " + uppercase_copy(action) + " " + affected_field,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::copy_to_command:
            {
                // COPY TO ARRAY <array> [FIELDS <list>] [FOR <expr>]
                if (statement.identifier == "array")
                {
                    const std::string array_name = trim_copy(statement.expression);
                    if (array_name.empty())
                    {
                        last_error_message = "COPY TO ARRAY: array name required";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    if (cursor == nullptr)
                    {
                        last_error_message = "COPY TO ARRAY: no current work area";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    // Determine field set (optionally filtered)
                    const std::vector<std::string> field_filter = parse_field_filter_clause(statement.tertiary_expression);
                    const std::string for_expr = statement.quaternary_expression;
                    // Gather field descriptors for column order from current record or cursor schema
                    std::vector<std::string> col_names;
                    const auto sample_rec = current_record(*cursor);
                    if (sample_rec.has_value())
                    {
                        for (const auto &rv : sample_rec->values)
                        {
                            if (field_matches_filter(rv.field_name, field_filter))
                            {
                                col_names.push_back(rv.field_name);
                            }
                        }
                    }
                    else if (cursor->source_path.empty())
                    {
                        // Remote cursor — use remote_records schema if available
                        if (!cursor->remote_records.empty())
                        {
                            for (const auto &rv : cursor->remote_records.front().values)
                            {
                                if (field_matches_filter(rv.field_name, field_filter))
                                {
                                    col_names.push_back(rv.field_name);
                                }
                            }
                        }
                    }
                    const std::size_t num_cols = col_names.empty() ? 1U : col_names.size();
                    std::vector<PrgValue> flat_values;
                    const CursorPositionSnapshot saved = capture_cursor_snapshot(*cursor);
                    for (std::size_t recno = 1U; recno <= cursor->record_count; ++recno)
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
                const bool is_structure = (statement.identifier == "structure");
                const std::string dest_raw = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                const std::string copy_type = normalize_identifier(unquote_string(trim_copy(statement.secondary_expression)));
                const bool copy_as_sdf = copy_type == "sdf";
                const bool copy_as_delimited = copy_type == "csv" || copy_type == "delimited";
                const std::string with_clause = statement.names.empty() ? std::string{} : statement.names.front();

                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr || cursor->source_path.empty())
                {
                    // Remote-only or no open table — emit event stub
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = dest_raw,
                                      .location = statement.location});
                    return {};
                }

                namespace fs = std::filesystem;
                fs::path dest_path(dest_raw);
                if (dest_path.extension().empty())
                {
                    if (copy_as_sdf || copy_type == "delimited")
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

                // Load source table schema + records up to the cursor record count
                const auto table_result = vfp::parse_dbf_table_from_file(
                    cursor->source_path, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                if (!table_result.ok)
                {
                    last_error_message = "COPY TO: " + table_result.error;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                // Build field filter from FIELDS clause (comma-separated names)
                const std::string fields_clause = statement.tertiary_expression;
                const std::vector<std::string> field_filter = parse_field_filter_clause(fields_clause);

                // Filter descriptors by FIELDS clause
                std::vector<vfp::DbfFieldDescriptor> out_fields;
                for (const auto &f : table_result.table.fields)
                {
                    if (field_matches_filter(f.name, field_filter))
                    {
                        out_fields.push_back(f);
                    }
                }
                if (out_fields.empty())
                {
                    last_error_message = "COPY TO: no fields match the FIELDS clause";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                // Collect qualifying rows (skip for COPY STRUCTURE TO)
                const std::string for_expr = statement.quaternary_expression;
                std::vector<std::vector<std::string>> out_rows;
                if (!is_structure)
                {
                    const CursorPositionSnapshot saved = capture_cursor_snapshot(*cursor);
                    for (std::size_t recno = 1U; recno <= cursor->record_count; ++recno)
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
                        last_error_message = "COPY TO TYPE SDF: unable to open output file";
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
                        last_error_message = "COPY TO TYPE SDF: unable to write output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = dest_path.string(),
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
                        last_error_message = "COPY TO TYPE DELIMITED: unable to open output file";
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
                        last_error_message = "COPY TO TYPE DELIMITED: unable to write output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = dest_path.string(),
                                      .location = statement.location});
                    return {};
                }

                const auto write_result = vfp::create_dbf_table_file(
                    dest_path.string(), out_fields, out_rows);
                if (!write_result.ok)
                {
                    last_error_message = "COPY TO: " + write_result.error;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.copy_to",
                                  .detail = dest_path.string(),
                                  .location = statement.location});
                return {};
            }
            case StatementKind::append_from_command:
            {
                // APPEND FROM ARRAY <array> [FIELDS <list>]
                if (statement.identifier == "array")
                {
                    const std::string array_name = trim_copy(statement.expression);
                    if (array_name.empty())
                    {
                        last_error_message = "APPEND FROM ARRAY: array name required";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                    if (cursor == nullptr || cursor->source_path.empty())
                    {
                        events.push_back({.category = "runtime.append_from_array",
                                          .detail = array_name,
                                          .location = statement.location});
                        return {};
                    }
                    // Determine dest fields order (filtered by FIELDS clause)
                    const std::vector<std::string> field_filter = parse_field_filter_clause(statement.tertiary_expression);
                    const auto dest_result = vfp::parse_dbf_table_from_file(
                        cursor->source_path, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = "APPEND FROM ARRAY: " + dest_result.error;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    std::vector<vfp::DbfFieldDescriptor> target_fields;
                    for (const auto &f : dest_result.table.fields)
                    {
                        if (field_matches_filter(f.name, field_filter))
                        {
                            target_fields.push_back(f);
                        }
                    }
                    if (target_fields.empty())
                    {
                        last_error_message = "APPEND FROM ARRAY: no fields match the FIELDS clause";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    const std::size_t num_rows = array_length(array_name, 1);
                    const std::size_t num_cols = std::max<std::size_t>(1U, array_length(array_name, 2));
                    std::size_t appended_count = 0U;
                    for (std::size_t row = 1U; row <= num_rows; ++row)
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = "APPEND FROM ARRAY: " + blank_result.error;
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = blank_result.record_count;
                        cursor->eof = false;
                        cursor->recno = blank_result.record_count;
                        const std::size_t usable_cols = std::min(target_fields.size(), num_cols);
                        for (std::size_t col = 1U; col <= usable_cols; ++col)
                        {
                            const PrgValue val = array_value(array_name, row, col);
                            const auto rep_result = vfp::replace_record_field_value(
                                cursor->source_path,
                                cursor->recno - 1U,
                                target_fields[col - 1U].name,
                                value_as_string(val));
                            if (!rep_result.ok)
                            {
                                continue;
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }
                    events.push_back({.category = "runtime.append_from_array",
                                      .detail = array_name + " (" + std::to_string(appended_count) + " records)",
                                      .location = statement.location});
                    return {};
                }

                // APPEND FROM <src> [TYPE <type>] [FIELDS <list>] [FOR <expr>]
                // First pass: copy non-deleted records from source DBF into current local cursor.
                // Field matching is by name; extra fields in source that do not exist in destination are silently skipped.
                const std::string src_raw = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                const std::string append_type = normalize_identifier(unquote_string(trim_copy(statement.secondary_expression)));
                const bool append_from_sdf = append_type == "sdf";
                const bool append_from_delimited = append_type == "csv" || append_type == "delimited";
                const std::string with_clause = statement.names.empty() ? std::string{} : statement.names.front();

                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr || cursor->source_path.empty())
                {
                    // Remote-only or no open table — emit event stub
                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw,
                                      .location = statement.location});
                    return {};
                }

                namespace fs = std::filesystem;
                fs::path src_path(src_raw);
                if (src_path.extension().empty())
                {
                    if (append_from_sdf || append_type == "delimited")
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
                    src_path = fs::path(current_default_directory()) / src_path;
                }
                src_path = src_path.lexically_normal();

                const std::string fields_clause = statement.tertiary_expression;
                const std::vector<std::string> field_filter = parse_field_filter_clause(fields_clause);

                if (append_from_sdf)
                {
                    std::ifstream input(src_path, std::ios::binary);
                    if (!input.good())
                    {
                        last_error_message = "APPEND FROM TYPE SDF: unable to open source file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    std::ostringstream buffer;
                    buffer << input.rdbuf();

                    const auto dest_result = vfp::parse_dbf_table_from_file(
                        cursor->source_path, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = "APPEND FROM TYPE SDF: " + dest_result.error;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields;
                    for (const auto &field : dest_result.table.fields)
                    {
                        if (field_matches_filter(field.name, field_filter))
                        {
                            target_fields.push_back(field);
                        }
                    }
                    if (target_fields.empty())
                    {
                        last_error_message = "APPEND FROM TYPE SDF: no fields match the FIELDS clause";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::size_t appended_count = 0U;
                    for (const std::string &line : split_sdf_lines(buffer.str()))
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = "APPEND FROM TYPE SDF: " + blank_result.error;
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
                                last_error_message = "APPEND FROM TYPE SDF: " + rep_result.error;
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE SDF)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_delimited)
                {
                    std::ifstream input(src_path, std::ios::binary);
                    if (!input.good())
                    {
                        last_error_message = "APPEND FROM TYPE DELIMITED: unable to open source file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    std::ostringstream buffer;
                    buffer << input.rdbuf();

                    const auto dest_result = vfp::parse_dbf_table_from_file(
                        cursor->source_path, std::max<std::size_t>(cursor->record_count + 1U, 1U));
                    if (!dest_result.ok)
                    {
                        last_error_message = "APPEND FROM TYPE DELIMITED: " + dest_result.error;
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    std::vector<vfp::DbfFieldDescriptor> target_fields;
                    for (const auto &field : dest_result.table.fields)
                    {
                        if (field_matches_filter(field.name, field_filter))
                        {
                            target_fields.push_back(field);
                        }
                    }
                    if (target_fields.empty())
                    {
                        last_error_message = "APPEND FROM TYPE DELIMITED: no fields match the FIELDS clause";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }

                    const DelimitedTextOptions delimited_options = parse_delimited_text_options(append_type, with_clause);
                    std::size_t appended_count = 0U;
                    bool first_delimited_line = true;
                    for (const std::string &line : split_text_lines(buffer.str()))
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
                            last_error_message = "APPEND FROM TYPE DELIMITED: " + blank_result.error;
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
                                last_error_message = "APPEND FROM TYPE DELIMITED: " + rep_result.error;
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE DELIMITED)",
                                      .location = statement.location});
                    return {};
                }

                // Parse all records from the source file
                const auto src_result = vfp::parse_dbf_table_from_file(
                    src_path.string(), std::numeric_limits<std::size_t>::max());
                if (!src_result.ok)
                {
                    last_error_message = "APPEND FROM: " + src_result.error;
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                // Append each qualifying source record into the destination cursor
                std::size_t appended_count = 0U;
                for (const auto &src_rec : src_result.table.records)
                {
                    if (src_rec.deleted)
                    {
                        continue;
                    }
                    // Append a blank record and then replace matching fields by name
                    const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                    if (!blank_result.ok)
                    {
                        last_error_message = "APPEND FROM: " + blank_result.error;
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
                        const auto rep_result = vfp::replace_record_field_value(
                            cursor->source_path,
                            cursor->recno - 1U,
                            src_field.field_name,
                            src_field.display_value);
                        if (!rep_result.ok)
                        {
                            // Field may not exist in destination — skip silently
                            continue;
                        }
                        cursor->record_count = rep_result.record_count;
                    }
                    ++appended_count;
                }

                events.push_back({.category = "runtime.append_from",
                                  .detail = src_raw + " (" + std::to_string(appended_count) + " records)",
                                  .location = statement.location});
                return {};
            }
            case StatementKind::scatter_command:
            {
                // SCATTER [FIELDS <list>] TO <var>|MEMVAR [BLANK]
                const bool use_memvar = (statement.identifier == "memvar");
                const bool blank = !statement.tertiary_expression.empty();
                const std::vector<std::string> field_filter = parse_field_filter_clause(statement.secondary_expression);
                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr)
                {
                    last_error_message = "SCATTER: no current work area";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const auto rec = current_record(*cursor);
                if (!rec.has_value())
                {
                    last_error_message = "SCATTER: no current record";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::vector<PrgValue> scattered_values;
                for (const auto &field : rec->values)
                {
                    if (!field_matches_filter(field.field_name, field_filter))
                    {
                        continue;
                    }
                    const PrgValue val = blank ? blank_value_for_field(field) : record_value_to_prg_value(field);
                    if (use_memvar)
                    {
                        assign_variable(frame, "m." + field.field_name, val);
                    }
                    else
                    {
                        scattered_values.push_back(val);
                    }
                }

                if (!use_memvar)
                {
                    const std::string array_name = trim_copy(statement.expression);
                    if (array_name.empty())
                    {
                        last_error_message = "SCATTER TO requires an array name when MEMVAR is not used";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    assign_array(array_name, std::move(scattered_values));
                }

                if (!field_filter.empty())
                {
                    const std::size_t selected_count = use_memvar ? 0U : array_length(statement.expression, 0);
                    if (!use_memvar && selected_count == 0U)
                    {
                        last_error_message = "SCATTER: no fields match the FIELDS clause";
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
                                  .detail = use_memvar ? "memvar" : statement.expression,
                                  .location = statement.location});
                return {};
            }
            case StatementKind::gather_command:
            {
                // GATHER FROM <var>|MEMVAR [FIELDS <list>] [FOR <expr>]
                const bool use_memvar = (statement.identifier == "memvar");
                CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
                if (cursor == nullptr)
                {
                    last_error_message = "GATHER: no current work area";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                const auto rec = current_record(*cursor);
                if (!rec.has_value())
                {
                    last_error_message = "GATHER: no current record";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                if (!trim_copy(statement.quaternary_expression).empty() &&
                    !value_as_bool(evaluate_expression(statement.quaternary_expression, frame, cursor)))
                {
                    events.push_back({.category = "runtime.gather",
                                      .detail = use_memvar ? "memvar skipped" : statement.expression + " skipped",
                                      .location = statement.location});
                    return {};
                }

                const std::vector<std::string> field_filter = parse_field_filter_clause(statement.secondary_expression);
                std::size_t array_index = 1U;
                for (const auto &field : rec->values)
                {
                    if (!field_matches_filter(field.field_name, field_filter))
                    {
                        continue;
                    }
                    const PrgValue val = use_memvar
                                             ? lookup_variable(frame, "m." + field.field_name)
                                             : array_value(statement.expression, array_index++);
                    if (cursor->remote)
                    {
                        auto it = std::find_if(
                            cursor->remote_records[cursor->recno - 1U].values.begin(),
                            cursor->remote_records[cursor->recno - 1U].values.end(),
                            [&](const vfp::DbfRecordValue &rv)
                            {
                                return collapse_identifier(rv.field_name) == collapse_identifier(field.field_name);
                            });
                        if (it != cursor->remote_records[cursor->recno - 1U].values.end())
                        {
                            it->display_value = value_as_string(val);
                        }
                    }
                    else
                    {
                        const auto rep_result = vfp::replace_record_field_value(
                            cursor->source_path,
                            cursor->recno - 1U,
                            field.field_name,
                            value_as_string(val));
                        if (!rep_result.ok)
                        {
                            last_error_message = rep_result.error;
                            last_fault_location = statement.location;
                            last_fault_statement = statement.text;
                            return {.ok = false, .message = last_error_message};
                        }
                        cursor->record_count = rep_result.record_count;
                    }
                }
                events.push_back({.category = "runtime.gather",
                                  .detail = use_memvar ? "memvar" : statement.expression,
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
                fault_pc_valid = false;
                const std::size_t resume_pc = fault_statement_index + 1U;
                while (!stack.empty())
                {
                    if (stack.back().file_path == fault_frame_file_path &&
                        stack.back().routine_name == fault_frame_routine_name)
                    {
                        const Routine *r = stack.back().routine;
                        stack.back().pc = (r && resume_pc < r->statements.size()) ? resume_pc : (r ? r->statements.size() : 0U);
                        return {.ok = true, .waiting_for_events = false, .frame_returned = false, .message = {}};
                    }
                    restore_private_declarations(stack.back());
                    stack.pop_back();
                }
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
                std::vector<PrgValue> elements;
                const std::string coll_norm = normalize_memory_variable_identifier(collection_expr);
                if (const RuntimeArray *arr = find_array(coll_norm); arr != nullptr)
                {
                    elements = arr->values;
                }
                else
                {
                    // Evaluate as expression; treat result as single element
                    const PrgValue result = evaluate_expression(collection_expr, frame);
                    elements.push_back(result);
                }

                if (elements.empty())
                {
                    // Skip the loop body entirely
                    if (const auto dest = find_matching_endfor(frame, frame.pc - 1U))
                    {
                        frame.pc = *dest + 1U;
                    }
                    return {};
                }

                // Assign first element and enter loop
                assign_variable(frame, var_name, elements[0]);
                frame.loops.push_back({
                    .for_statement_index = frame.pc - 1U,
                    .endfor_statement_index = find_matching_endfor(frame, frame.pc - 1U).value_or(frame.pc - 1U),
                    .variable_name = var_name,
                    .is_for_each = true,
                    .each_values = std::move(elements),
                    .each_index = 0U
                });
                return {};
            }
            case StatementKind::release_command:
            {
                if (statement.identifier == "all")
                {
                    // RELEASE ALL [LIKE <pattern> | EXCEPT <pattern>]
                    const std::string mode = statement.expression; // "like", "except", or ""
                    const std::string pattern = statement.secondary_expression;
                    if (mode.empty())
                    {
                        // RELEASE ALL — erase all globals that are not PUBLIC-pinned arrays
                        globals.clear();
                        arrays.clear();
                    }
                    else
                    {
                        std::vector<std::string> to_erase;
                        for (const auto &[name, val] : globals)
                        {
                            const bool matches = wildcard_match_insensitive(pattern, name);
                            if ((mode == "like" && matches) || (mode == "except" && !matches))
                            {
                                to_erase.push_back(name);
                            }
                        }
                        for (const auto &name : to_erase)
                        {
                            globals.erase(name);
                            arrays.erase(name);
                        }
                    }
                }
                else
                {
                    // RELEASE <varlist>
                    for (const auto &raw : statement.names)
                    {
                        const std::string name = normalize_memory_variable_identifier(trim_copy(raw));
                        globals.erase(name);
                        arrays.erase(name);
                        // Also remove from current frame locals if declared LOCAL
                        frame.locals.erase(name);
                        frame.local_names.erase(name);
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
                if (statement.identifier == "all")
                {
                    // Also close all open work areas
                    for (auto &[session_id, session] : data_sessions)
                    {
                        session.cursors.clear();
                        session.aliases.clear();
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
        catch (const std::bad_alloc &)
        {
            last_error_message = "Runtime resource fault: out of memory. Execution paused safely.";
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (const std::filesystem::filesystem_error &error)
        {
            last_error_message = std::string("Runtime resource fault: filesystem failure: ") + error.what();
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (const std::system_error &error)
        {
            last_error_message = std::string("Runtime resource fault: system error: ") + error.what();
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (const std::exception &error)
        {
            last_error_message = std::string("Runtime fault: ") + error.what();
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }
        catch (...)
        {
            last_error_message = "Runtime fault: unknown exception";
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            events.push_back({.category = "runtime.error",
                              .detail = last_error_message,
                              .location = statement.location});
            return {.ok = false, .message = last_error_message};
        }

        return {};
    }

