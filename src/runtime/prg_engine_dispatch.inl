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

            auto append_memory_metadata = [&](Frame &current_frame, std::string &detail)
            {
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

            auto assign_runtime_target_value = [&](const std::string &raw_identifier, const PrgValue &assignment_value) -> ExecutionOutcome
            {
                std::string assignment_identifier = apply_with_context(raw_identifier, frame);
                if (!assignment_identifier.empty() && assignment_identifier.front() == '&')
                {
                    const PrgValue expanded_identifier = evaluate_expression(assignment_identifier, frame);
                    const std::string expanded_text = trim_copy(value_as_string(expanded_identifier));
                    if (!expanded_text.empty())
                    {
                        assignment_identifier = expanded_text;
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
                    const PrgValue object_value = lookup_variable(frame, object_part);
                    auto object = resolve_ole_object(object_value);
                    if (!object.has_value())
                    {
                        last_error_message = "OLE object not found for property assignment: " + assignment_identifier;
                        record_ole_aerror_context(assignment_identifier,
                                                  "Copperfin OLE",
                                                  object_part,
                                                  statement.text,
                                                  1429);
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
            };
            auto resolve_command_array_name = [&](const std::string &raw_name, const std::string &command_name) -> std::optional<std::string>
            {
                const std::string candidate = trim_copy(raw_name);
                if (candidate.empty())
                {
                    last_error_message = command_name + ": array name required";
                    return std::nullopt;
                }
                if (is_bare_identifier_text(candidate))
                {
                    return candidate;
                }

                const PrgValue evaluated = evaluate_expression(candidate, frame);
                const std::string evaluated_name = trim_copy(value_as_string(evaluated));
                if (is_bare_identifier_text(evaluated_name))
                {
                    return evaluated_name;
                }

                last_error_message = command_name + ": invalid array name";
                return std::nullopt;
            };
            auto parse_command_object_target_path = [&](const std::string &raw_name, const std::string &command_name)
                -> std::optional<std::vector<std::string>>
            {
                std::string candidate = trim_copy(raw_name);
                if (candidate.empty())
                {
                    last_error_message = command_name + ": object target required";
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
                        const std::string expanded_base = trim_copy(value_as_string(lookup_variable(frame, macro_expression)));
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
                    last_error_message = command_name + ": invalid object target";
                    return std::nullopt;
                }

                std::vector<std::string> segments;
                std::size_t start = 0U;
                while (start <= candidate.size())
                {
                    const std::size_t dot = candidate.find('.', start);
                    const std::string segment = trim_copy(candidate.substr(start, dot == std::string::npos ? std::string::npos : dot - start));
                    if (!is_bare_identifier_text(segment))
                    {
                        last_error_message = command_name + ": invalid object target";
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
                    last_error_message = command_name + ": invalid object target";
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
                    last_error_message = "Object target assignment failed";
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
                        last_error_message = "SCATTER NAME: unable to create object";
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
                            last_error_message = "SCATTER NAME: unable to create object";
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
                    last_error_message = "Object target assignment failed";
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

            switch (statement.kind)
            {
            case StatementKind::assignment:
            {
                const PrgValue assignment_value = evaluate_expression(statement.expression, frame);
                return assign_runtime_target_value(statement.identifier, assignment_value);
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

                    std::filesystem::path target_path(target);
                    if (target_path.extension().empty())
                    {
                        target_path += ".prg";
                    }
                    if (target_path.is_relative())
                    {
                        target_path = std::filesystem::path(current_default_directory()) / target_path;
                    }
                    if (std::filesystem::exists(target_path))
                    {
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
                    if (!ensure_exclusive_table_maintenance(*cursor, "PACK MEMO"))
                    {
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
                    last_error_message = "UNLOCK target work area not found";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                if (!trim_copy(statement.identifier).empty())
                {
                    const std::size_t recno = static_cast<std::size_t>(
                        std::max<double>(0.0, std::llround(value_as_number(evaluate_expression(statement.identifier, frame)))));
                    if (recno == 0U || recno > cursor->record_count)
                    {
                        last_error_message = "UNLOCK RECORD target record not found";
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
            case StatementKind::browse_command:
            {
                CursorState *cursor = resolve_cursor_target_expression(statement.secondary_expression, frame);
                if (cursor == nullptr)
                {
                    last_error_message = "BROWSE target work area not found";
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
                const auto strip_set_to_value = [](const std::string &raw_value) -> std::string
                {
                    std::string candidate = trim_copy(raw_value);
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
                const auto evaluate_set_string_value = [&](const std::string &raw_value, const std::string &default_value) -> std::string
                {
                    const std::string candidate = strip_set_to_value(raw_value);
                    if (candidate.empty())
                    {
                        return default_value;
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
                        normalized_name == "strictdate" || normalized_name == "optimize" ||
                        normalized_name == "talk" || normalized_name == "safety" || normalized_name == "escape" ||
                        normalized_name == "century" || normalized_name == "seconds" || normalized_name == "exclusive" ||
                        normalized_name == "multilocks" || normalized_name == "null" || normalized_name == "ansi")
                    {
                        current_set_state()[normalized_name] = normalize_boolean_set_value(option_value.empty() ? "on" : option_value);
                    }
                    else if (normalized_name == "reprocess")
                    {
                        std::string reprocess_value = evaluate_set_string_value(option_value, "0");
                        current_set_state()[normalized_name] = uppercase_copy(reprocess_value.empty() ? std::string{"0"} : reprocess_value);
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
                        std::string fields_value = strip_set_to_value(option_value);
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
                    const std::string normalized = normalize_memory_variable_identifier(name);
                    if (normalized.empty())
                    {
                        continue;
                    }
                    public_names.insert(normalized);
                    globals.try_emplace(normalized, make_empty_value());
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
                for (std::size_t index = 0U; index < statement.names.size(); ++index)
                {
                    const auto [parameter_name, default_expression] = split_parameter_default(statement.names[index]);
                    const std::string normalized = normalize_memory_variable_identifier(parameter_name);
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
                        frame.locals[normalized] = default_expression.empty()
                                                       ? make_empty_value()
                                                       : evaluate_expression(default_expression, frame);
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
                    ExecutionOutcome outcome = assign_runtime_target_value(trim_copy(name), result);
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
                std::string alias = statement.identifier.empty()
                                        ? "CURSOR1"
                                        : normalize_identifier(unquote_identifier(trim_copy(statement.identifier)));
                if (alias.empty())
                {
                    last_error_message = "CREATE CURSOR requires a non-empty alias";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const auto declarations = parse_table_field_declarations(statement.expression);
                const std::vector<vfp::DbfFieldDescriptor> fields = table_field_descriptors(declarations);
                if (fields.empty())
                {
                    last_error_message = "CREATE CURSOR requires at least one supported field declaration";
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

                if (!ensure_transaction_backup_for_table(table_path.string()))
                {
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

                const auto field_rules = field_rules_from_declarations(declarations);
                if (!open_table_cursor(table_path.string(), alias, {}, true, false, 0, {}, 0U, field_rules))
                {
                    std::filesystem::remove(table_path, ignored);
                    std::filesystem::remove(table_path.replace_extension(".fpt"), ignored);
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
                events.push_back({.category = "runtime.create_cursor",
                                  .detail = alias + " -> " + table_path.string(),
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

                if (!ensure_transaction_backup_for_table(table_path.string()))
                {
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

                if (!ensure_transaction_backup_for_table(table_path.string()))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

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
                        default:
                            escaped.push_back(ch);
                            break;
                        }
                    }
                    return escaped;
                };

                std::string destination = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                if (destination.empty())
                {
                    last_error_message = "SAVE TO: filename required";
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

                if (!destination_path.parent_path().empty())
                {
                    std::error_code ignored;
                    fs::create_directories(destination_path.parent_path(), ignored);
                }

                std::ofstream output(destination_path, std::ios::binary);
                if (!output.good())
                {
                    last_error_message = "SAVE TO: unable to open output file";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                std::size_t saved_count = 0U;
                for (const auto &[name, value] : visible_variables)
                {
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

                    char type_code = 'C';
                    std::string serialized_value;
                    switch (value.kind)
                    {
                    case PrgValueKind::boolean:
                        type_code = 'L';
                        serialized_value = value.boolean_value ? "true" : "false";
                        break;
                    case PrgValueKind::number:
                    case PrgValueKind::int64:
                    case PrgValueKind::uint64:
                        type_code = 'N';
                        serialized_value = value_as_string(value);
                        break;
                    case PrgValueKind::string:
                    {
                        int year = 0;
                        int month = 0;
                        int day = 0;
                        serialized_value = value.string_value;
                        type_code = parse_runtime_date_string(serialized_value, year, month, day) ? 'D' : 'C';
                        break;
                    }
                    case PrgValueKind::empty:
                        type_code = 'E';
                        serialized_value.clear();
                        break;
                    }

                    output << name << "=" << type_code << ":" << escape_memvar_value(serialized_value) << "\n";
                    ++saved_count;
                }

                output.close();
                if (!output.good())
                {
                    last_error_message = "SAVE TO: unable to write output file";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                events.push_back({.category = "runtime.save_memory",
                                  .detail = destination_path.string() + " (" + std::to_string(saved_count) + " variables)",
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
                        default:
                            unescaped.push_back(next);
                            break;
                        }
                    }
                    return unescaped;
                };

                std::string source = unquote_string(trim_copy(
                    value_as_string(evaluate_expression(statement.expression, frame))));
                if (source.empty())
                {
                    last_error_message = "RESTORE FROM: filename required";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                fs::path source_path(source);
                if (source_path.extension().empty())
                {
                    source_path += ".mem";
                }
                if (source_path.is_relative())
                {
                    source_path = fs::path(current_default_directory()) / source_path;
                }
                source_path = source_path.lexically_normal();

                std::ifstream input(source_path, std::ios::binary);
                if (!input.good())
                {
                    last_error_message = "RESTORE FROM: unable to open source file";
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }

                const bool additive = normalize_identifier(statement.identifier) == "additive";
                if (!additive)
                {
                    globals.clear();
                }

                std::size_t restored_count = 0U;
                std::string line;
                while (std::getline(input, line))
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
                    const char type_code = raw_type.empty()
                                               ? 'C'
                                               : static_cast<char>(std::toupper(static_cast<unsigned char>(raw_type.front())));
                    const std::string raw_value = unescape_memvar_value(line.substr(colon_position + 1U));

                    PrgValue restored_value;
                    if (type_code == 'L')
                    {
                        const std::string normalized_bool = normalize_identifier(raw_value);
                        const bool bool_value = normalized_bool == "true" ||
                                                normalized_bool == ".t." ||
                                                normalized_bool == "t" ||
                                                normalized_bool == "1" ||
                                                normalized_bool == "y" ||
                                                normalized_bool == "yes";
                        restored_value = make_boolean_value(bool_value);
                    }
                    else if (type_code == 'N')
                    {
                        const std::string numeric_text = trim_copy(raw_value);
                        char *number_end = nullptr;
                        const double parsed = std::strtod(numeric_text.c_str(), &number_end);
                        restored_value = (number_end != numeric_text.c_str() && number_end != nullptr && *number_end == '\0')
                                             ? make_number_value(parsed)
                                             : make_number_value(0.0);
                    }
                    else if (type_code == 'D')
                    {
                        restored_value = make_string_value(trim_copy(raw_value));
                    }
                    else if (type_code == 'E')
                    {
                        restored_value = make_empty_value();
                    }
                    else
                    {
                        restored_value = make_string_value(raw_value);
                    }

                    globals[name] = restored_value;
                    ++restored_count;
                }

                events.push_back({.category = "runtime.restore_memory",
                                  .detail = source_path.string() + " (" + std::to_string(restored_count) + " variables)",
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
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    const std::string array_name = *resolved_array_name;
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
                const bool copy_as_json = copy_type == "json";
                const bool copy_as_sdf = copy_type == "sdf";
                const bool copy_as_dif = copy_type == "dif";
                const bool copy_as_sylk = copy_type == "sylk";
                const bool copy_as_tab = copy_type == "tab";
                const bool copy_as_xls = copy_type == "xls";
                const bool copy_as_delimited = copy_type == "csv" || copy_type == "delimited" || copy_as_tab;
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
                        last_error_message = "COPY TO TYPE JSON: unable to open output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_json_records(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = "COPY TO TYPE JSON: unable to write output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = dest_path.string(),
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
                        last_error_message = "COPY TO TYPE DIF: unable to open output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_dif_table(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = "COPY TO TYPE DIF: unable to write output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = dest_path.string(),
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
                        last_error_message = "COPY TO TYPE SYLK: unable to open output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_sylk_table(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = "COPY TO TYPE SYLK: unable to write output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    events.push_back({.category = "runtime.copy_to",
                                      .detail = dest_path.string(),
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
                        last_error_message = "COPY TO TYPE XLS: unable to open output file";
                        last_fault_location = statement.location;
                        last_fault_statement = statement.text;
                        return {.ok = false, .message = last_error_message};
                    }
                    output << serialize_spreadsheetml_workbook(out_fields, out_rows);
                    output.close();
                    if (!output.good())
                    {
                        last_error_message = "COPY TO TYPE XLS: unable to write output file";
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
                    const auto resolved_array_name = resolve_command_array_name(statement.expression, "APPEND FROM ARRAY");
                    if (!resolved_array_name.has_value())
                    {
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
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
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
                const bool append_from_json = append_type == "json";
                const bool append_from_sdf = append_type == "sdf";
                const bool append_from_dif = append_type == "dif";
                const bool append_from_sylk = append_type == "sylk";
                const bool append_from_tab = append_type == "tab";
                const bool append_from_xls = append_type == "xls";
                const bool append_from_delimited = append_type == "csv" || append_type == "delimited" || append_from_tab;
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
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
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

                if (append_from_json)
                {
                    std::ifstream input(src_path, std::ios::binary);
                    if (!input.good())
                    {
                        last_error_message = "APPEND FROM TYPE JSON: unable to open source file";
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
                        last_error_message = "APPEND FROM TYPE JSON: " + dest_result.error;
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
                        last_error_message = "APPEND FROM TYPE JSON: no fields match the FIELDS clause";
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

                    const std::vector<std::map<std::string, std::string>> json_rows = parse_json_record_objects(buffer.str());
                    std::size_t appended_count = 0U;
                    for (const auto &row : json_rows)
                    {
                        const auto blank_result = vfp::append_blank_record_to_file(cursor->source_path);
                        if (!blank_result.ok)
                        {
                            last_error_message = "APPEND FROM TYPE JSON: " + blank_result.error;
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
                                last_error_message = "APPEND FROM TYPE JSON: " + rep_result.error;
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE JSON)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_dif)
                {
                    std::ifstream input(src_path, std::ios::binary);
                    if (!input.good())
                    {
                        last_error_message = "APPEND FROM TYPE DIF: unable to open source file";
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
                        last_error_message = "APPEND FROM TYPE DIF: " + dest_result.error;
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
                        last_error_message = "APPEND FROM TYPE DIF: no fields match the FIELDS clause";
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

                    std::vector<std::vector<std::string>> dif_rows = parse_dif_table(buffer.str(), target_fields.size());
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
                            last_error_message = "APPEND FROM TYPE DIF: " + blank_result.error;
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
                                last_error_message = "APPEND FROM TYPE DIF: " + rep_result.error;
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE DIF)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_sylk)
                {
                    std::ifstream input(src_path, std::ios::binary);
                    if (!input.good())
                    {
                        last_error_message = "APPEND FROM TYPE SYLK: unable to open source file";
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
                        last_error_message = "APPEND FROM TYPE SYLK: " + dest_result.error;
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
                        last_error_message = "APPEND FROM TYPE SYLK: no fields match the FIELDS clause";
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

                    std::vector<std::vector<std::string>> sylk_rows = parse_sylk_table(buffer.str(), target_fields.size());
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
                            last_error_message = "APPEND FROM TYPE SYLK: " + blank_result.error;
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
                                last_error_message = "APPEND FROM TYPE SYLK: " + rep_result.error;
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE SYLK)",
                                      .location = statement.location});
                    return {};
                }

                if (append_from_xls)
                {
                    std::ifstream input(src_path, std::ios::binary);
                    if (!input.good())
                    {
                        last_error_message = "APPEND FROM TYPE XLS: unable to open source file";
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
                        last_error_message = "APPEND FROM TYPE XLS: " + dest_result.error;
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
                        last_error_message = "APPEND FROM TYPE XLS: no fields match the FIELDS clause";
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

                    std::vector<std::vector<std::string>> workbook_rows = parse_spreadsheetml_workbook(buffer.str());
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
                            last_error_message = "APPEND FROM TYPE XLS: " + blank_result.error;
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
                                last_error_message = "APPEND FROM TYPE XLS: " + rep_result.error;
                                last_fault_location = statement.location;
                                last_fault_statement = statement.text;
                                return {.ok = false, .message = last_error_message};
                            }
                            cursor->record_count = rep_result.record_count;
                        }
                        ++appended_count;
                    }

                    events.push_back({.category = "runtime.append_from",
                                      .detail = src_raw + " (" + std::to_string(appended_count) + " records, TYPE XLS)",
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
                    if (!ensure_transaction_backup_for_table(cursor->source_path))
                    {
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
                if (!ensure_transaction_backup_for_table(cursor->source_path))
                {
                    last_fault_location = statement.location;
                    last_fault_statement = statement.text;
                    return {.ok = false, .message = last_error_message};
                }
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
                std::vector<std::string> scattered_field_names;
                std::string array_name;
                std::size_t matched_field_count = 0U;
                for (const auto &field : rec->values)
                {
                    if (!field_matches_filter(field.field_name, field_filter))
                    {
                        continue;
                    }
                    const char field_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field.field_type)));
                    const bool is_memo_field = field_type == 'M' || field_type == 'G' || field_type == 'W';
                    if (!include_memo && is_memo_field)
                    {
                        continue;
                    }

                    ++matched_field_count;
                    const PrgValue val = blank ? blank_value_for_field(field) : record_value_to_prg_value(field);
                    if (use_memvar)
                    {
                        assign_variable(frame, "m." + field.field_name, val);
                    }
                    else if (use_name_object)
                    {
                        scattered_field_names.push_back(normalize_identifier(field.field_name));
                        scattered_values.push_back(val);
                    }
                    else
                    {
                        scattered_field_names.push_back(field.field_name);
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
                    std::string detail = "memvar skipped";
                    if (use_name_object)
                    {
                        detail = trim_copy(statement.expression) + " skipped";
                    }
                    else if (!use_memvar)
                    {
                        const auto resolved_array_name = resolve_command_array_name(statement.expression, "GATHER FROM");
                        detail = (resolved_array_name.has_value() ? *resolved_array_name : statement.expression) + " skipped";
                    }
                    events.push_back({.category = "runtime.gather",
                                      .detail = detail,
                                      .location = statement.location});
                    return {};
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
                        last_error_message = "GATHER NAME: object variable not found";
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
                std::size_t array_index = 1U;
                for (const auto &field : rec->values)
                {
                    if (!field_matches_filter(field.field_name, field_filter))
                    {
                        continue;
                    }
                    PrgValue val = make_empty_value();
                    if (use_memvar)
                    {
                        val = lookup_variable(frame, "m." + field.field_name);
                    }
                    else if (use_name_object)
                    {
                        const std::string field_name = normalize_identifier(field.field_name);
                        const auto property = source_object->properties.find(field_name);
                        if (property == source_object->properties.end())
                        {
                            continue;
                        }
                        val = property->second;
                    }
                    else if (use_name_value_pairs)
                    {
                        const auto pair = name_value_pairs.find(normalize_identifier(field.field_name));
                        if (pair == name_value_pairs.end())
                        {
                            continue;
                        }
                        val = pair->second;
                    }
                    else if (source_array != nullptr && source_array->columns > 1U)
                    {
                        val = array_value(array_name, 1U, array_index++);
                    }
                    else
                    {
                        val = array_value(array_name, array_index++);
                    }
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
                            it->display_value = serialize_prg_value_for_record_field(*it, val);
                        }
                    }
                    else
                    {
                        const auto rep_result = vfp::replace_record_field_value(
                            cursor->source_path,
                            cursor->recno - 1U,
                            field.field_name,
                            serialize_prg_value_for_record_field(field, val));
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
                    error_metadata_stack.pop_back();
                }
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
                    detail += "prompt=" + statement.expression;
                }
                if (!statement.identifier.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.identifier;
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
                    detail += "prompt=" + statement.expression;
                }
                if (!statement.identifier.empty())
                {
                    if (!detail.empty()) detail += " ";
                    detail += "target=" + statement.identifier;
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
                    const std::string resolved_prompt = resolve_runtime_expression_text(statement.expression, frame);
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
                    std::string resolved_target = apply_with_context(statement.names.front(), frame);
                    if (!resolved_target.empty() && resolved_target.front() == '&')
                    {
                        const PrgValue expanded_identifier = evaluate_expression(resolved_target, frame);
                        const std::string expanded_text = trim_copy(value_as_string(expanded_identifier));
                        if (!expanded_text.empty())
                        {
                            resolved_target = expanded_text;
                        }
                    }
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
                    }
                }
                else if (normalize_identifier(statement.identifier) == "structure")
                {
                    append_cursor_structure_metadata(resolve_cursor_target(std::to_string(current_selected_work_area())), detail);
                }
                else if (normalize_identifier(statement.identifier) == "status")
                {
                    append_session_status_metadata(frame, detail);
                }
                else if (normalize_identifier(statement.identifier) == "memory")
                {
                    append_memory_metadata(frame, detail);
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
                    }
                }
                else if (normalize_identifier(statement.identifier) == "structure")
                {
                    append_cursor_structure_metadata(resolve_cursor_target(std::to_string(current_selected_work_area())), detail);
                }
                else if (normalize_identifier(statement.identifier) == "status")
                {
                    append_session_status_metadata(frame, detail);
                }
                else if (normalize_identifier(statement.identifier) == "memory")
                {
                    append_memory_metadata(frame, detail);
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
                        for (auto iterator = globals.begin(); iterator != globals.end();)
                        {
                            if (public_names.contains(iterator->first))
                            {
                                ++iterator;
                            }
                            else
                            {
                                iterator = globals.erase(iterator);
                            }
                        }
                        for (auto iterator = arrays.begin(); iterator != arrays.end();)
                        {
                            if (public_names.contains(iterator->first))
                            {
                                ++iterator;
                            }
                            else
                            {
                                iterator = arrays.erase(iterator);
                            }
                        }
                    }
                    else
                    {
                        std::vector<std::string> to_erase;
                        for (const auto &[name, val] : globals)
                        {
                            (void)val;
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
                        public_names.erase(name);
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
                public_names.clear();
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
