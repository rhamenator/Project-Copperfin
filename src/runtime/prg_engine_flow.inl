// prg_engine_flow.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

#if defined(COPPERFIN_PRG_ENGINE_IMPL_CONTEXT)

        void restore_private_declarations(Frame &frame)
        {
            for (const auto &[name, saved] : frame.private_saved_values)
            {
                if (saved.has_value())
                {
                    globals[name] = *saved;
                }
                else
                {
                    globals.erase(name);
                }
            }
            for (const auto &[name, saved] : frame.private_saved_arrays)
            {
                if (saved.has_value())
                {
                    arrays[name] = *saved;
                }
                else
                {
                    arrays.erase(name);
                }
            }
        }

        void unwind_with_bindings(Frame &frame, std::size_t target_depth)
        {
            while (frame.withs.size() > target_depth)
            {
                frame.locals.erase(frame.withs.back().binding_name);
                frame.local_names.erase(frame.withs.back().binding_name);
                frame.withs.pop_back();
            }
        }

        void unwind_case_contexts(Frame &frame, std::size_t target_depth)
        {
            if (frame.cases.size() > target_depth)
            {
                frame.cases.resize(target_depth);
            }
        }

        std::optional<ExecutionOutcome> continue_pending_return(Frame &frame)
        {
            while (!frame.tries.empty())
            {
                TryState &active_try = frame.tries.back();
                if (active_try.entered_finally || !active_try.finally_statement_index.has_value())
                {
                    frame.tries.pop_back();
                    continue;
                }

                unwind_with_bindings(frame, active_try.with_stack_depth_at_try_entry);
                unwind_case_contexts(frame, active_try.case_stack_depth_at_try_entry);
                active_try.handling_error = false;
                active_try.entered_catch = false;
                active_try.entered_finally = true;
                active_try.propagate_after_finally = false;
                active_try.return_after_finally = true;
                frame.pc = *active_try.finally_statement_index + 1U;
                return std::nullopt;
            }

            pop_frame();
            return ExecutionOutcome{.ok = true,
                                    .waiting_for_events = false,
                                    .frame_returned = true,
                                    .message = {}};
        }

        void release_memory_binding(Frame &frame, const std::string &raw_name, bool clear_public_binding)
        {
            const std::string name = normalize_memory_variable_identifier(trim_copy(raw_name));
            if (name.empty())
            {
                return;
            }

            if (frame.local_arrays.erase(name) != 0U)
            {
                return;
            }

            if (const auto private_saved = frame.private_saved_values.find(name);
                private_saved != frame.private_saved_values.end())
            {
                if (private_saved->second.has_value())
                {
                    globals[name] = *private_saved->second;
                }
                else
                {
                    globals.erase(name);
                }
                frame.private_saved_values.erase(private_saved);
            }
            else if (frame.local_names.contains(name))
            {
                frame.locals.erase(name);
                return;
            }
            else
            {
                globals.erase(name);
                if (clear_public_binding)
                {
                    public_names.erase(name);
                }
            }

            if (const auto private_array = frame.private_saved_arrays.find(name);
                private_array != frame.private_saved_arrays.end())
            {
                if (private_array->second.has_value())
                {
                    arrays[name] = *private_array->second;
                }
                else
                {
                    arrays.erase(name);
                }
                frame.private_saved_arrays.erase(private_array);
            }
            else
            {
                arrays.erase(name);
            }
            frame.locals.erase(name);
        }

        bool value_references_object_handle(const PrgValue &value, int handle) const
        {
            int value_handle = 0;
            std::string value_prog_id;
            return parse_object_handle_reference(value, value_handle, value_prog_id) && value_handle == handle;
        }

        bool has_live_variable_reference_to_object(int handle) const
        {
            const auto is_context_alias = [](const std::string &name)
            {
                const std::string normalized = normalize_memory_variable_identifier(name);
                return normalized == "this" ||
                       normalized == "parent" ||
                       normalized == "thisform" ||
                       normalized == "thisformset";
            };
            const auto map_has_reference = [this, handle, &is_context_alias](const auto &values, bool skip_context_aliases) {
                for (const auto &[name, value] : values)
                {
                    if (skip_context_aliases && is_context_alias(name))
                    {
                        continue;
                    }
                    if (value_references_object_handle(value, handle))
                    {
                        return true;
                    }
                }
                return false;
            };
            const auto optional_map_has_reference = [this, handle](const auto &values) {
                for (const auto &[name, value] : values)
                {
                    if (value.has_value() && value_references_object_handle(*value, handle))
                    {
                        return true;
                    }
                }
                return false;
            };
            const auto array_map_has_reference = [this, handle](const auto &value_arrays) {
                for (const auto &[name, array] : value_arrays)
                {
                    for (const auto &value : array.values)
                    {
                        if (value_references_object_handle(value, handle))
                        {
                            return true;
                        }
                    }
                }
                return false;
            };
            const auto native_collection_has_reference = [this, handle]()
            {
                for (const auto &[object_handle, object] : ole_objects)
                {
                    if (!is_native_collection_object(object))
                    {
                        continue;
                    }
                    for (const auto &value : object.collection_items)
                    {
                        if (value_references_object_handle(value, handle))
                        {
                            return true;
                        }
                    }
                }
                return false;
            };

            if (map_has_reference(globals, false) || array_map_has_reference(arrays) || native_collection_has_reference())
            {
                return true;
            }
            for (const auto &active_frame : stack)
            {
                if (map_has_reference(active_frame.locals, true) ||
                    optional_map_has_reference(active_frame.private_saved_values) ||
                    array_map_has_reference(active_frame.local_arrays))
                {
                    return true;
                }
                for (const auto &[name, saved_array] : active_frame.private_saved_arrays)
                {
                    if (!saved_array.has_value())
                    {
                        continue;
                    }
                    for (const auto &value : saved_array->values)
                    {
                        if (value_references_object_handle(value, handle))
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        bool release_object_memory_binding(
            Frame &frame,
            const std::string &raw_name,
            std::set<int> &released_handles)
        {
            const PrgValue *value = find_variable(frame, raw_name);
            if (value == nullptr)
            {
                return false;
            }

            int handle = 0;
            std::string prog_id;
            if (!parse_object_handle_reference(*value, handle, prog_id))
            {
                return false;
            }

            release_memory_binding(frame, raw_name, true);
            if (has_live_variable_reference_to_object(handle))
            {
                return true;
            }

            const auto object = ole_objects.find(handle);
            if (object != ole_objects.end() && released_handles.insert(handle).second)
            {
                (void)release_native_object(object->second, normalize_memory_variable_identifier(trim_copy(raw_name)));
            }
            return true;
        }

        void release_frame_object_bindings(Frame &frame)
        {
            const auto is_context_alias = [](const std::string &name)
            {
                const std::string normalized = normalize_memory_variable_identifier(name);
                return normalized == "this" ||
                       normalized == "parent" ||
                       normalized == "thisform" ||
                       normalized == "thisformset";
            };
            const auto object_handle_for_value = [this](const PrgValue &value) -> std::optional<int>
            {
                int handle = 0;
                std::string prog_id;
                return parse_object_handle_reference(value, handle, prog_id)
                    ? std::optional<int>(handle)
                    : std::nullopt;
            };

            std::set<int> contextual_handles;
            for (const auto &[name, value] : frame.locals)
            {
                if (!is_context_alias(name))
                {
                    continue;
                }
                if (const auto handle = object_handle_for_value(value); handle.has_value())
                {
                    contextual_handles.insert(*handle);
                }
            }

            std::set<int> returned_handles;
            if (frame.return_pending && last_return_value.has_value())
            {
                if (const auto handle = object_handle_for_value(*last_return_value); handle.has_value())
                {
                    returned_handles.insert(*handle);
                }
            }

            std::vector<std::string> local_object_names;
            for (const auto &[name, value] : frame.locals)
            {
                if (is_context_alias(name))
                {
                    continue;
                }
                const auto handle = object_handle_for_value(value);
                if (!handle.has_value() || contextual_handles.contains(*handle) || returned_handles.contains(*handle))
                {
                    continue;
                }
                local_object_names.push_back(name);
            }

            std::vector<std::pair<std::string, std::vector<int>>> local_array_objects;
            for (const auto &[name, array] : frame.local_arrays)
            {
                std::vector<int> handles;
                for (const PrgValue &value : array.values)
                {
                    if (const auto handle = object_handle_for_value(value);
                        handle.has_value() && !contextual_handles.contains(*handle) && !returned_handles.contains(*handle))
                    {
                        handles.push_back(*handle);
                    }
                }
                if (!handles.empty())
                {
                    local_array_objects.emplace_back(name, std::move(handles));
                }
            }

            std::set<int> released_handles;
            for (const std::string &name : local_object_names)
            {
                (void)release_object_memory_binding(frame, name, released_handles);
            }
            for (const auto &[name, handles] : local_array_objects)
            {
                release_memory_binding(frame, name, true);
                for (const int handle : handles)
                {
                    if (has_live_variable_reference_to_object(handle))
                    {
                        continue;
                    }
                    const auto object = ole_objects.find(handle);
                    if (object != ole_objects.end() && released_handles.insert(handle).second)
                    {
                        (void)release_native_object(object->second, name);
                    }
                }
            }
        }

        void sync_byref_arguments(Frame &frame)
        {
            if (frame.parameter_reference_bindings.empty())
            {
                return;
            }

            Frame *caller = stack.size() >= 2U ? &stack[stack.size() - 2U] : nullptr;
            for (const auto &[parameter_name, reference_name] : frame.parameter_reference_bindings)
            {
                const auto local = frame.locals.find(parameter_name);
                if (local == frame.locals.end())
                {
                    continue;
                }
                if (caller != nullptr)
                {
                    assign_variable(*caller, reference_name, local->second);
                }
                else
                {
                    globals[normalize_memory_variable_identifier(reference_name)] = local->second;
                }
            }
        }

        void pop_frame()
        {
            if (!stack.empty())
            {
                const bool requested_nodefault = stack.back().requested_nodefault;
                const bool returned_explicitly = stack.back().return_pending;
                const std::optional<PrgValue> saved_return_value = last_return_value;
                sync_byref_arguments(stack.back());
                release_frame_object_bindings(stack.back());
                restore_private_declarations(stack.back());
                stack.pop_back();
                last_popped_frame_requested_nodefault = requested_nodefault;
                last_popped_frame_returned = returned_explicitly;
                last_return_value = saved_return_value;
            }
        }

        bool breakpoint_matches(const SourceLocation &location) const
        {
            return std::any_of(breakpoints.begin(), breakpoints.end(), [&](const RuntimeBreakpoint &breakpoint)
                               { return paths_equal_for_platform(breakpoint.file_path, location.file_path) && breakpoint.line == location.line; });
        }

        std::optional<std::size_t> find_matching_branch(const Frame &frame, std::size_t pc, bool seek_else) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::if_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endif_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
                else if (seek_else && kind == StatementKind::else_statement && depth == 0)
                {
                    return index;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_endfor(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::for_statement || kind == StatementKind::for_each_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endfor_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_enddo(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_while_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::enddo_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_endcase(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_case_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endcase_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        std::optional<std::size_t> find_matching_endwith(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::with_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endwith_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
            }
            return std::nullopt;
        }

        struct TryClauseTargets
        {
            std::vector<std::size_t> catch_statement_indices;
            std::optional<std::size_t> finally_statement_index;
            std::optional<std::size_t> endtry_statement_index;
        };

        TryClauseTargets find_try_clause_targets(const Frame &frame, std::size_t pc) const
        {
            TryClauseTargets targets;
            if (frame.routine == nullptr)
            {
                return targets;
            }

            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::try_statement)
                {
                    ++depth;
                    continue;
                }
                if (kind == StatementKind::endtry_statement)
                {
                    if (depth == 0)
                    {
                        targets.endtry_statement_index = index;
                        return targets;
                    }
                    --depth;
                    continue;
                }
                if (depth != 0)
                {
                    continue;
                }
                if (kind == StatementKind::catch_statement)
                {
                    targets.catch_statement_indices.push_back(index);
                }
                else if (kind == StatementKind::finally_statement && !targets.finally_statement_index.has_value())
                {
                    targets.finally_statement_index = index;
                }
            }

            return targets;
        }

        std::string apply_with_context(std::string text, const Frame &frame) const
        {
            if (frame.withs.empty())
            {
                return text;
            }

            const std::string binding_name = frame.withs.back().binding_name;
            if (binding_name.empty())
            {
                return text;
            }

            std::string rewritten;
            rewritten.reserve(text.size() + binding_name.size() * 2U);
            const auto starts_reserved_dotted_token = [&](std::size_t offset)
            {
                static constexpr std::array<std::string_view, 6U> tokens{
                    ".T.", ".F.", ".NULL.", ".NOT.", ".AND.", ".OR."
                };
                return std::any_of(tokens.begin(), tokens.end(), [&](std::string_view token)
                {
                    if (offset + token.size() > text.size())
                    {
                        return false;
                    }
                    for (std::size_t token_index = 0U; token_index < token.size(); ++token_index)
                    {
                        if (std::tolower(static_cast<unsigned char>(text[offset + token_index])) !=
                            std::tolower(static_cast<unsigned char>(token[token_index])))
                        {
                            return false;
                        }
                    }
                    return true;
                });
            };
            bool in_string = false;
            for (std::size_t index = 0U; index < text.size(); ++index)
            {
                const char ch = text[index];
                if (ch == '\'')
                {
                    in_string = !in_string;
                    rewritten.push_back(ch);
                    continue;
                }
                if (in_string || ch != '.' || (index + 1U) >= text.size())
                {
                    rewritten.push_back(ch);
                    continue;
                }
                if (starts_reserved_dotted_token(index))
                {
                    rewritten.push_back(ch);
                    continue;
                }

                const unsigned char next = static_cast<unsigned char>(text[index + 1U]);
                if (std::isalpha(next) == 0 && next != '_')
                {
                    rewritten.push_back(ch);
                    continue;
                }

                char previous_nonspace = '\0';
                for (std::size_t lookback = index; lookback > 0U; --lookback)
                {
                    const char candidate = text[lookback - 1U];
                    if (std::isspace(static_cast<unsigned char>(candidate)) != 0)
                    {
                        continue;
                    }
                    previous_nonspace = candidate;
                    break;
                }

                if (std::isalnum(static_cast<unsigned char>(previous_nonspace)) != 0 ||
                    previous_nonspace == '_' ||
                    previous_nonspace == '.')
                {
                    rewritten.push_back(ch);
                    continue;
                }

                rewritten.append(binding_name);
                rewritten.push_back(ch);
            }

            return rewritten;
        }

        bool dispatch_fault_finally(Frame &frame, TryState &active_try, const Statement &statement)
        {
            if (!active_try.finally_statement_index.has_value() || active_try.entered_finally)
            {
                return false;
            }

            frame.loop_expression_continuation.reset();
            frame.scan_expression_continuation.reset();
            unwind_with_bindings(frame, active_try.with_stack_depth_at_try_entry);
            unwind_case_contexts(frame, active_try.case_stack_depth_at_try_entry);
            active_try.handling_error = true;
            active_try.entered_catch = false;
            active_try.entered_finally = true;
            active_try.propagate_after_finally = true;
            active_try.return_after_finally = false;
            last_error_compatibility.explicit_error_code = last_error_code;
            last_error_compatibility.preserve_fault_context = true;
            frame.pc = *active_try.finally_statement_index + 1U;

            events.push_back({.category = "runtime.try_handler",
                              .detail = statement.text,
                              .location = statement.location});
            return true;
        }

        bool dispatch_try_handler(Frame &frame, const Statement &statement)
        {
            for (std::size_t try_index = frame.tries.size(); try_index > 0U; --try_index)
            {
                TryState &active_try = frame.tries[try_index - 1U];
                if (active_try.entered_finally)
                {
                    frame.tries.erase(frame.tries.begin() + static_cast<std::ptrdiff_t>(try_index - 1U));
                    continue;
                }
                if (active_try.handling_error)
                {
                    if (dispatch_fault_finally(frame, active_try, statement))
                    {
                        return true;
                    }
                    frame.tries.erase(frame.tries.begin() + static_cast<std::ptrdiff_t>(try_index - 1U));
                    continue;
                }

                std::optional<PrgValue> caught_exception_reference;
                const auto ensure_caught_exception_reference = [&]() -> const PrgValue &
                {
                    if (!caught_exception_reference.has_value())
                    {
                        caught_exception_reference = materialize_catch_exception_object();
                    }
                    return *caught_exception_reference;
                };

                for (const std::size_t catch_statement_index : active_try.catch_statement_indices)
                {
                    if (frame.routine == nullptr || catch_statement_index >= frame.routine->statements.size())
                    {
                        continue;
                    }

                    const Statement &catch_statement = frame.routine->statements[catch_statement_index];
                    const std::string catch_variable = trim_copy(catch_statement.identifier);
                    if (!catch_variable.empty())
                    {
                        assign_variable(frame, catch_variable, ensure_caught_exception_reference());
                    }

                    bool predicate_matches = true;
                    if (!catch_statement.secondary_expression.empty())
                    {
                        predicate_matches = value_as_bool(
                            evaluate_expression(catch_statement.secondary_expression, frame));
                    }

                    if (!predicate_matches)
                    {
                        if (!catch_variable.empty())
                        {
                            assign_variable(frame, catch_variable, make_empty_value());
                        }
                        continue;
                    }

                    unwind_with_bindings(frame, active_try.with_stack_depth_at_try_entry);
                    unwind_case_contexts(frame, active_try.case_stack_depth_at_try_entry);
                    frame.loop_expression_continuation.reset();
                    frame.scan_expression_continuation.reset();
                    active_try.handling_error = true;
                    active_try.entered_catch = true;
                    active_try.entered_finally = false;
                    active_try.propagate_after_finally = false;
                    active_try.return_after_finally = false;
                    frame.pc = catch_statement_index + 1U;

                    events.push_back({.category = "runtime.try_handler",
                                      .detail = statement.text,
                                      .location = statement.location});
                    return true;
                }

                if (dispatch_fault_finally(frame, active_try, statement))
                {
                    return true;
                }

                frame.tries.erase(frame.tries.begin() + static_cast<std::ptrdiff_t>(try_index - 1U));
            }

            return false;
        }

        std::optional<std::size_t> find_next_case_clause(const Frame &frame, std::size_t pc) const
        {
            if (frame.routine == nullptr)
            {
                return std::nullopt;
            }
            int depth = 0;
            for (std::size_t index = pc + 1U; index < frame.routine->statements.size(); ++index)
            {
                const auto kind = frame.routine->statements[index].kind;
                if (kind == StatementKind::do_case_statement)
                {
                    ++depth;
                }
                else if (kind == StatementKind::endcase_statement)
                {
                    if (depth == 0)
                    {
                        return index;
                    }
                    --depth;
                }
                else if (depth == 0 && (kind == StatementKind::case_statement || kind == StatementKind::otherwise_statement))
                {
                    return index;
                }
            }
            return std::nullopt;
        }

        enum class ActiveLoopKind
        {
            for_loop,
            scan_loop,
            while_loop
        };

        struct ActiveLoop
        {
            ActiveLoopKind kind = ActiveLoopKind::for_loop;
            std::size_t start_statement_index = 0;
            std::size_t end_statement_index = 0;
        };

        std::optional<ActiveLoop> find_innermost_active_loop(const Frame &frame) const
        {
            std::optional<ActiveLoop> active;
            const auto consider = [&](ActiveLoop candidate)
            {
                if (!active.has_value() || candidate.start_statement_index > active->start_statement_index)
                {
                    active = candidate;
                }
            };

            if (!frame.loops.empty())
            {
                const LoopState &loop = frame.loops.back();
                consider({.kind = ActiveLoopKind::for_loop,
                          .start_statement_index = loop.for_statement_index,
                          .end_statement_index = loop.endfor_statement_index});
            }
            if (!frame.scans.empty())
            {
                const ScanState &scan = frame.scans.back();
                consider({.kind = ActiveLoopKind::scan_loop,
                          .start_statement_index = scan.scan_statement_index,
                          .end_statement_index = scan.endscan_statement_index});
            }
            if (!frame.whiles.empty())
            {
                const WhileState &loop = frame.whiles.back();
                consider({.kind = ActiveLoopKind::while_loop,
                          .start_statement_index = loop.do_while_statement_index,
                          .end_statement_index = loop.enddo_statement_index});
            }

            return active;
        }

        ExecutionOutcome continue_for_loop(Frame &frame, const Statement &, bool jump_after_completion)
        {
            if (frame.loops.empty())
            {
                return {};
            }

            LoopState &loop = frame.loops.back();
            if (jump_after_completion)
            {
                unwind_case_contexts(frame, loop.case_stack_depth_at_entry);
                unwind_with_bindings(frame, loop.with_stack_depth_at_entry);
            }

            // FOR EACH loops use a separate continuation path
            if (loop.is_for_each)
            {
                ++loop.each_index;
                ++loop.iteration_count;
                if (loop.iteration_count > max_loop_iterations)
                {
                    last_error_message = loop_iteration_limit_message();
                    return {.ok = false, .message = last_error_message};
                }
                if (loop.each_index < loop.each_values.size())
                {
                    assign_variable(frame, loop.variable_name, loop.each_values[loop.each_index]);
                    frame.pc = loop.for_statement_index + 1U;
                }
                else
                {
                    const std::size_t completion_pc = loop.endfor_statement_index + 1U;
                    frame.loops.pop_back();
                    if (jump_after_completion)
                    {
                        frame.pc = completion_pc;
                    }
                }
                return {};
            }

            ++loop.iteration_count;
            if (loop.iteration_count > max_loop_iterations)
            {
                last_error_message = loop_iteration_limit_message();
                return {.ok = false, .message = last_error_message};
            }
            const double next_value = value_as_number(lookup_variable(frame, loop.variable_name)) + loop.step_value;
            assign_variable(frame, loop.variable_name, make_number_value(next_value));
            const bool should_continue = loop.step_value >= 0.0
                                             ? next_value <= loop.end_value
                                             : next_value >= loop.end_value;
            if (should_continue)
            {
                frame.pc = loop.for_statement_index + 1U;
            }
            else
            {
                const std::size_t completion_pc = loop.endfor_statement_index + 1U;
                frame.loops.pop_back();
                if (jump_after_completion)
                {
                    frame.pc = completion_pc;
                }
            }
            return {};
        }

        bool scan_expression_contains_user_routine(
            const Frame &frame,
            const std::string &expression)
        {
            bool in_single_quoted_string = false;
            bool in_double_quoted_string = false;
            for (std::size_t index = 0U; index < expression.size(); ++index)
            {
                const char current = expression[index];
                if (current == '\'' && !in_double_quoted_string)
                {
                    if (in_single_quoted_string && index + 1U < expression.size() && expression[index + 1U] == '\'')
                    {
                        ++index;
                        continue;
                    }
                    in_single_quoted_string = !in_single_quoted_string;
                    continue;
                }
                if (current == '"' && !in_single_quoted_string)
                {
                    if (in_double_quoted_string && index + 1U < expression.size() && expression[index + 1U] == '"')
                    {
                        ++index;
                        continue;
                    }
                    in_double_quoted_string = !in_double_quoted_string;
                    continue;
                }
                if (in_single_quoted_string || in_double_quoted_string ||
                    (std::isalpha(static_cast<unsigned char>(current)) == 0 && current != '_'))
                {
                    continue;
                }

                const std::size_t identifier_start = index;
                ++index;
                while (index < expression.size())
                {
                    const char character = expression[index];
                    if (std::isalnum(static_cast<unsigned char>(character)) == 0 && character != '_')
                    {
                        break;
                    }
                    ++index;
                }
                const std::string identifier = expression.substr(identifier_start, index - identifier_start);
                std::size_t lookahead = index;
                while (lookahead < expression.size() &&
                       std::isspace(static_cast<unsigned char>(expression[lookahead])) != 0)
                {
                    ++lookahead;
                }
                const bool is_member_call = identifier_start > 0U && expression[identifier_start - 1U] == '.';
                if (lookahead < expression.size() && expression[lookahead] == '(' && !is_member_call &&
                    find_unqualified_routine_lookup(frame.file_path, identifier).has_value())
                {
                    return true;
                }
                if (index == identifier_start)
                {
                    continue;
                }
                --index;
            }
            return false;
        }

        bool scan_expression_requires_continuation(
            const Frame &frame,
            const std::string &for_expression,
            const std::string &while_expression,
            const std::string &filter_expression)
        {
            return scan_expression_contains_user_routine(frame, for_expression) ||
                   scan_expression_contains_user_routine(frame, while_expression) ||
                   scan_expression_contains_user_routine(frame, filter_expression);
        }

        ExecutionOutcome continue_scan_loop(Frame &frame, const Statement &statement, bool jump_after_completion)
        {
            if (frame.scans.empty())
            {
                return {};
            }

            ScanState &scan = frame.scans.back();
            if (jump_after_completion)
            {
                unwind_case_contexts(frame, scan.case_stack_depth_at_entry);
                unwind_with_bindings(frame, scan.with_stack_depth_at_entry);
            }
            ++scan.iteration_count;
            if (scan.iteration_count > max_loop_iterations)
            {
                last_error_message = loop_iteration_limit_message();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }
            CursorState *cursor = find_cursor_by_area(scan.work_area);
            if (cursor == nullptr)
            {
                frame.scans.pop_back();
                return {};
            }

            if (scan_expression_requires_continuation(
                    frame,
                    scan.for_expression,
                    scan.while_expression,
                    cursor->filter_expression))
            {
                const Statement scan_statement = frame.routine != nullptr &&
                        scan.scan_statement_index < frame.routine->statements.size()
                    ? frame.routine->statements[scan.scan_statement_index]
                    : Statement{};
                return begin_scan_expression_search(
                    frame,
                    scan_statement,
                    ScanSearchKind::continue_scan,
                    scan.work_area,
                    cursor->recno + 1U,
                    scan.scan_statement_index,
                    scan.endscan_statement_index,
                    scan.iteration_count,
                    jump_after_completion);
            }

            if (!locate_next_matching_record(*cursor, scan.for_expression, scan.while_expression, frame, cursor->recno + 1U))
            {
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            if (cursor->found)
            {
                frame.pc = scan.scan_statement_index + 1U;
            }
            else
            {
                frame.scans.pop_back();
                if (jump_after_completion)
                {
                    frame.pc = scan.endscan_statement_index + 1U;
                }
            }
            return {};
        }

        Statement make_scan_expression_statement(
            const Statement &original,
            const std::string &expression,
            ScanExpressionStage stage) const
        {
            Statement staged = original;
            staged.expression = expression;
            staged.secondary_expression.clear();
            staged.tertiary_expression.clear();
            staged.text = original.text + " [scan-expression-stage=" +
                          std::to_string(static_cast<int>(stage)) + "]";
            return staged;
        }

        ExecutionOutcome complete_scan_expression_search(Frame &frame, bool found)
        {
            if (!frame.scan_expression_continuation.has_value())
            {
                return {};
            }

            const ScanExpressionContinuation continuation = *frame.scan_expression_continuation;
            CursorState *cursor = find_cursor_by_area(continuation.work_area);
            if (cursor == nullptr)
            {
                frame.scan_expression_continuation.reset();
                last_error_message = runtime_text("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound",
                                                  {{"command", "SCAN"}});
                return {.ok = false, .message = last_error_message};
            }

            cursor->found = found;
            if (!found)
            {
                move_cursor_to(*cursor, static_cast<long long>(cursor->record_count + 1U));
                cursor->found = false;
            }
            frame.scan_expression_continuation.reset();
            if (continuation.kind == ScanSearchKind::enter_scan)
            {
                if (!found)
                {
                    frame.pc = continuation.endscan_statement_index + 1U;
                    return {};
                }

                frame.scans.push_back({.scan_statement_index = continuation.scan_statement_index,
                                       .endscan_statement_index = continuation.endscan_statement_index,
                                       .case_stack_depth_at_entry = frame.cases.size(),
                                       .with_stack_depth_at_entry = frame.withs.size(),
                                       .work_area = continuation.work_area,
                                       .for_expression = continuation.statement.expression,
                                       .while_expression = continuation.statement.tertiary_expression,
                                       .iteration_count = 0});
                events.push_back({.category = "runtime.scan",
                                  .detail = continuation.statement.expression.empty()
                                      ? "ALL"
                                      : continuation.statement.expression,
                                  .location = continuation.statement.location});
                return {};
            }

            if (found)
            {
                frame.pc = continuation.scan_statement_index + 1U;
                return {};
            }

            if (!frame.scans.empty())
            {
                frame.scans.pop_back();
            }
            if (continuation.jump_after_completion)
            {
                frame.pc = continuation.endscan_statement_index + 1U;
            }
            return {};
        }

        ExecutionOutcome continue_scan_expression_search(
            Frame &frame,
            std::optional<PrgValue> completed_value = std::nullopt)
        {
            while (frame.scan_expression_continuation.has_value())
            {
                ScanExpressionContinuation &continuation = *frame.scan_expression_continuation;
                CursorState *cursor = find_cursor_by_area(continuation.work_area);
                if (cursor == nullptr)
                {
                    return complete_scan_expression_search(frame, false);
                }

                if (completed_value.has_value())
                {
                    const bool predicate_value = value_as_bool(*completed_value);
                    completed_value.reset();
                    if (continuation.stage == ScanExpressionStage::while_predicate)
                    {
                        if (!predicate_value)
                        {
                            return complete_scan_expression_search(frame, false);
                        }
                        continuation.stage = ScanExpressionStage::cursor_filter;
                    }
                    else if (continuation.stage == ScanExpressionStage::cursor_filter)
                    {
                        if (!predicate_value)
                        {
                            ++continuation.candidate_recno;
                            continuation.stage = ScanExpressionStage::while_predicate;
                        }
                        else
                        {
                            continuation.stage = ScanExpressionStage::for_predicate;
                        }
                    }
                    else
                    {
                        if (!predicate_value)
                        {
                            ++continuation.candidate_recno;
                            continuation.stage = ScanExpressionStage::while_predicate;
                        }
                        else
                        {
                            return complete_scan_expression_search(frame, true);
                        }
                    }
                }

                if (continuation.candidate_recno < 1U ||
                    continuation.candidate_recno > cursor->record_count)
                {
                    return complete_scan_expression_search(frame, false);
                }

                move_cursor_to(*cursor, static_cast<long long>(continuation.candidate_recno));
                const auto record = current_record(*cursor);
                if (!record.has_value() || (is_set_enabled("deleted") && record->deleted))
                {
                    ++continuation.candidate_recno;
                    continuation.stage = ScanExpressionStage::while_predicate;
                    continue;
                }

                if (continuation.stage == ScanExpressionStage::while_predicate)
                {
                    if (continuation.statement.tertiary_expression.empty())
                    {
                        continuation.stage = ScanExpressionStage::cursor_filter;
                        continue;
                    }
                    const auto value = evaluate_resumable_expression(
                        frame,
                        make_scan_expression_statement(
                            continuation.statement,
                            continuation.statement.tertiary_expression,
                            ScanExpressionStage::while_predicate),
                        cursor);
                    if (!value.has_value())
                    {
                        return {};
                    }
                    completed_value = *value;
                    continue;
                }

                if (continuation.stage == ScanExpressionStage::cursor_filter)
                {
                    if (cursor->filter_expression.empty())
                    {
                        continuation.stage = ScanExpressionStage::for_predicate;
                        continue;
                    }
                    const auto value = evaluate_resumable_expression(
                        frame,
                        make_scan_expression_statement(
                            continuation.statement,
                            cursor->filter_expression,
                            ScanExpressionStage::cursor_filter),
                        cursor);
                    if (!value.has_value())
                    {
                        return {};
                    }
                    completed_value = *value;
                    continue;
                }

                if (continuation.statement.expression.empty())
                {
                    return complete_scan_expression_search(frame, true);
                }
                const auto value = evaluate_resumable_expression(
                    frame,
                    make_scan_expression_statement(
                        continuation.statement,
                        continuation.statement.expression,
                        ScanExpressionStage::for_predicate),
                    cursor);
                if (!value.has_value())
                {
                    return {};
                }
                completed_value = *value;
            }
            return {};
        }

        ExecutionOutcome begin_scan_expression_search(
            Frame &frame,
            const Statement &statement,
            ScanSearchKind kind,
            int work_area,
            std::size_t start_recno,
            std::size_t scan_statement_index,
            std::size_t endscan_statement_index,
            std::size_t iteration_count,
            bool jump_after_completion)
        {
            frame.scan_expression_continuation = ScanExpressionContinuation{
                .statement = statement,
                .stage = ScanExpressionStage::while_predicate,
                .kind = kind,
                .work_area = work_area,
                .candidate_recno = start_recno,
                .scan_statement_index = scan_statement_index,
                .endscan_statement_index = endscan_statement_index,
                .iteration_count = iteration_count,
                .jump_after_completion = jump_after_completion};
            events.push_back({.category = "runtime.rushmore",
                              .detail = (statement.expression.empty() ? std::string{"ALL"} : statement.expression) +
                                        " -> linear_scan (resumable scan filter)",
                              .location = statement.location});
            return continue_scan_expression_search(frame);
        }

        std::filesystem::path resolve_asset_path(const std::string &raw_path, const char *extension) const
        {
            std::filesystem::path asset_path = copperfin::platform::path_from_utf8_string(
                unquote_string(take_first_token(raw_path)));
            if (asset_path.extension().empty())
            {
                asset_path += extension;
            }
            if (asset_path.is_relative())
            {
                asset_path = copperfin::platform::path_from_utf8_string(current_default_directory()) / asset_path;
            }
            return asset_path.lexically_normal();
        }

        std::filesystem::path resolve_report_output_path(const std::string &to_clause, const Frame &frame)
        {
            std::string target = trim_copy(to_clause);
            if (target.empty())
            {
                return {};
            }

            if (starts_with_insensitive(target, "FILE") &&
                (target.size() == 4U || std::isspace(static_cast<unsigned char>(target[4])) != 0))
            {
                target = trim_copy(target.substr(4U));
            }
            if (target.empty())
            {
                return {};
            }

            std::filesystem::path output_path;
            if (target.size() >= 2U && target.front() == '\'' && target.back() == '\'')
            {
                output_path = copperfin::platform::path_from_utf8_string(unquote_string(target));
            }
            else
            {
                output_path = copperfin::platform::path_from_utf8_string(
                    value_as_string(evaluate_expression(target, frame)));
            }

            if (output_path.is_relative())
            {
                output_path = copperfin::platform::path_from_utf8_string(current_default_directory()) / output_path;
            }
            return output_path.lexically_normal();
        }

        std::string report_output_path_required_message() const
        {
            return runtime_text("Runtime.Prg.ReportOutput.Error.PathRequired");
        }

        std::string report_output_open_message(const std::filesystem::path &path) const
        {
            return runtime_text("Runtime.Prg.ReportOutput.Error.OpenFailed", {{"path", copperfin::platform::path_to_utf8_string(path)}});
        }

        std::string report_output_write_message(const std::filesystem::path &path) const
        {
            return runtime_text("Runtime.Prg.ReportOutput.Error.WriteFailed", {{"path", copperfin::platform::path_to_utf8_string(path)}});
        }

        std::string report_asset_resolve_message(const std::filesystem::path &path) const
        {
            return runtime_text("Runtime.Prg.ReportAsset.Error.ResolveFailed", {{"path", copperfin::platform::path_to_utf8_string(path)}});
        }

        std::vector<std::string> render_report_output_rows(
            CursorState &cursor,
            const Frame &frame,
            const studio::StudioReportLayoutSnapshot &layout,
            const std::vector<vfp::DbfFieldDescriptor> &fields,
            const std::string &for_expression,
            const std::string &while_expression)
        {
            std::vector<std::string> rows;
            const CursorPositionSnapshot saved = capture_cursor_snapshot(cursor);
            for (const std::size_t recno : record_iteration_order(cursor))
            {
                move_cursor_to(cursor, static_cast<long long>(recno));
                if (!while_expression.empty() &&
                    !evaluate_visibility_expression(while_expression, frame, &cursor))
                {
                    break;
                }
                if (!current_record_matches_visibility(cursor, frame, for_expression))
                {
                    continue;
                }

                const auto record = current_record(cursor);
                if (!record.has_value())
                {
                    continue;
                }

                std::string row = "row[" + std::to_string(cursor.recno) + "]=";
                for (std::size_t index = 0U; index < fields.size(); ++index)
                {
                    if (index > 0U)
                    {
                        row += "|";
                    }
                    row += fields[index].name;
                    row += "=";
                    row += record_field_value(*record, fields[index].name).value_or(std::string{});
                }

                std::vector<std::string> object_expression_values;
                for (const auto &section : layout.sections)
                {
                    for (const auto &object : section.objects)
                    {
                        const std::string expression = trim_copy(object.expression);
                        if (object.deleted || expression.empty())
                        {
                            continue;
                        }

                        object_expression_values.push_back(
                            std::to_string(object.record_index) + ":" +
                            format_value(evaluate_expression(expression, frame)));
                    }
                }
                if (!object_expression_values.empty())
                {
                    row += "|object_exprs=";
                    for (std::size_t index = 0U; index < object_expression_values.size(); ++index)
                    {
                        if (index > 0U)
                        {
                            row += ";";
                        }
                        row += object_expression_values[index];
                    }
                }
                rows.push_back(std::move(row));
            }
            restore_cursor_snapshot(cursor, saved);
            return rows;
        }

        ExecutionOutcome open_report_surface(const Statement &statement, const Frame &frame, const char *extension, const char *category_prefix)
        {
            report_interrupted = false;
            active_report_status = 0;
            const std::filesystem::path asset_path = resolve_asset_path(statement.identifier, extension);
            const std::string normalized_asset_path = copperfin::platform::path_to_utf8_string(asset_path.lexically_normal());
            const auto display_alias = options.source_path_display_aliases.find(normalized_asset_path);
            const std::string display_asset_path = display_alias == options.source_path_display_aliases.end()
                ? copperfin::platform::path_to_utf8_string(asset_path)
                : display_alias->second;
            std::error_code asset_exists_error;
            const auto admitted_asset = find_verified_file_byte_override(asset_path);
            const bool has_admitted_asset = options.require_verified_file_byte_overrides &&
                admitted_asset != options.verified_file_byte_overrides.end() &&
                !admitted_asset->second.empty();
            if ((!std::filesystem::exists(asset_path, asset_exists_error) || asset_exists_error) &&
                !has_admitted_asset && !options.require_verified_file_byte_overrides)
            {
                last_error_message = report_asset_resolve_message(asset_path);
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            std::filesystem::path snapshot_root;
            const auto snapshot_path = materialize_verified_xasset_snapshot(asset_path, snapshot_root);
            if (!snapshot_path.has_value())
            {
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            studio::StudioOpenRequest request;
            request.path = copperfin::platform::path_to_utf8_string(*snapshot_path);
            request.read_only = true;
            request.load_full_table = true;
            const auto open_result = studio::open_document(request);
            std::error_code ignored;
            if (!snapshot_root.empty())
            {
                std::filesystem::remove_all(snapshot_root, ignored);
            }
            if (!open_result.ok)
            {
                last_error_message = open_result.error;
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            const auto layout = studio::build_report_layout(open_result.document);
            const bool preview_mode =
                normalize_identifier(statement.secondary_expression) == "preview" ||
                trim_copy(statement.tertiary_expression).empty();
            if (preview_mode)
            {
                active_report_status = 1;
                waiting_for_events = true;
                events.push_back({.category = std::string(category_prefix) + ".preview",
                                  .detail = display_asset_path,
                                  .location = statement.location});
                if (layout.available)
                {
                    events.push_back({.category = std::string(category_prefix) + ".preview.layout",
                                      .detail = std::to_string(layout.sections.size()) + " sections",
                                      .location = statement.location});
                }
                return {.ok = true, .waiting_for_events = true, .frame_returned = false, .message = {}};
            }

            active_report_status = 2;
            struct ReportStatusResetGuard
            {
                Impl &runtime;
                ~ReportStatusResetGuard() { runtime.active_report_status = 0; }
            } report_status_reset{*this};

            const std::filesystem::path output_path = resolve_report_output_path(statement.tertiary_expression, frame);
            if (output_path.empty())
            {
                last_error_message = report_output_path_required_message();
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            if (!output_path.parent_path().empty())
            {
                std::error_code ignored;
                std::filesystem::create_directories(output_path.parent_path(), ignored);
            }

            std::ofstream output(output_path, std::ios::binary);
            if (!output.good())
            {
                last_error_message = report_output_open_message(output_path);
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            output << "Copperfin " << category_prefix << " render\n";
            output << "source=" << copperfin::platform::path_to_utf8_string(asset_path) << "\n";
            if (layout.available)
            {
                output << "sections=" << layout.sections.size() << "\n";
                for (std::size_t index = 0U; index < layout.sections.size(); ++index)
                {
                    const auto &section = layout.sections[index];
                    output << "section[" << index << "]=" << section.band_kind
                           << " title=" << section.title
                           << " objects=" << section.objects.size() << "\n";
                }
            }

            CursorState *cursor = resolve_cursor_target(std::to_string(current_selected_work_area()));
            std::size_t rendered_row_count = 0U;
            if (cursor != nullptr)
            {
                const std::vector<vfp::DbfFieldDescriptor> fields = cursor_field_descriptors(*cursor);
                const std::string while_expression =
                    statement.names.empty() ? std::string{} : trim_copy(statement.names.front());
                output << "cursor=" << (cursor->alias.empty() ? std::to_string(cursor->work_area) : cursor->alias) << "\n";
                if (!fields.empty())
                {
                    output << "fields=";
                    for (std::size_t index = 0U; index < fields.size(); ++index)
                    {
                        if (index > 0U)
                        {
                            output << ",";
                        }
                        output << fields[index].name;
                    }
                    output << "\n";
                }
                if (!trim_copy(cursor->filter_expression).empty())
                {
                    output << "set_filter=" << trim_copy(cursor->filter_expression) << "\n";
                }
                if (!trim_copy(statement.quaternary_expression).empty())
                {
                    output << "for=" << trim_copy(statement.quaternary_expression) << "\n";
                }
                if (!while_expression.empty())
                {
                    output << "while=" << while_expression << "\n";
                }

                const std::vector<std::string> rendered_rows = render_report_output_rows(
                    *cursor,
                    frame,
                    layout,
                    fields,
                    trim_copy(statement.quaternary_expression),
                    while_expression);
                rendered_row_count = rendered_rows.size();
                output << "rows=" << rendered_row_count << "\n";
                for (const auto &row : rendered_rows)
                {
                    output << row << "\n";
                }
            }
            output.close();
            if (!output.good())
            {
                last_error_message = report_output_write_message(output_path);
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                return {.ok = false, .message = last_error_message};
            }

            events.push_back({.category = std::string(category_prefix) + ".render",
                              .detail = copperfin::platform::path_to_utf8_string(output_path) +
                                  " rows=" + std::to_string(rendered_row_count),
                              .location = statement.location});
            return {};
        }

#endif // defined(COPPERFIN_PRG_ENGINE_IMPL_CONTEXT)
