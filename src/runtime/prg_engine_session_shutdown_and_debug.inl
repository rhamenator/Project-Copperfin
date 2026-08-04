// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

        std::map<int, RegisteredApiFunction> &current_registered_api_functions()
        {
            auto [iterator, _] = registered_api_functions_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        const std::map<int, RegisteredApiFunction> &current_registered_api_functions() const
        {
            const auto found = registered_api_functions_by_session.find(current_data_session);
            if (found != registered_api_functions_by_session.end())
            {
                return found->second;
            }

            static const std::map<int, RegisteredApiFunction> empty_registered_functions;
            return empty_registered_functions;
        }

        void release_declared_dll_functions() noexcept
        {
#if defined(_WIN32)
            for (auto &[_, declfn] : declared_dll_functions)
            {
                if (declfn.hmodule != nullptr)
                {
                    FreeLibrary(declfn.hmodule);
                }
                declfn.hmodule = nullptr;
                declfn.proc_address = nullptr;
            }
#endif
            declared_dll_functions.clear();
        }

        void cleanup_runtime_resources_for_shutdown()
        {
            // Release open work areas/cursors across all data sessions.
            for (auto &[_, session] : data_sessions)
            {
                session.cursors.clear();
                session.aliases.clear();
                session.table_locks.clear();
                session.record_locks.clear();
                session.databases.clear();
                session.current_database_path.clear();
                session.selected_work_area = 1;
                session.next_work_area = 1;
            }
            clear_all_shared_lock_ownership();

            // Release synthetic SQL/OLE/runtime interop state.
            sql_connections_by_session.clear();
            next_sql_handle_by_session.clear();
            registered_api_functions_by_session.clear();
            next_api_handle_by_session.clear();
            ole_objects.clear();

            // Ensure FOPEN handles are closed so files are not left locked.
            close_all_file_io_handles();

            release_declared_dll_functions();
            loaded_libraries.clear();
            procedure_program_paths.clear();
        }

        std::vector<int> collect_native_shutdown_roots() const
        {
            std::vector<int> roots;
            for (const auto &[handle, runtime_object] : ole_objects)
            {
                const auto parent_reference = native_object_parent_reference(runtime_object);
                int parent_handle = 0;
                std::string parent_prog_id;
                if (!parent_reference.has_value() ||
                    !parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id) ||
                    !ole_objects.contains(parent_handle))
                {
                    roots.push_back(handle);
                }
            }

            if (roots.empty())
            {
                for (const auto &[handle, _] : ole_objects)
                {
                    roots.push_back(handle);
                }
            }
            return roots;
        }

        std::vector<int> collect_native_shutdown_order()
        {
            struct PendingObject
            {
                int handle = 0;
                bool children_queued = false;
            };

            std::vector<int> order;
            std::vector<PendingObject> pending;
            std::set<int> scheduled_handles;
            const auto append_tree = [&](int root_handle) -> void
            {
                if (!scheduled_handles.insert(root_handle).second)
                {
                    return;
                }
                pending.push_back({.handle = root_handle, .children_queued = false});
                while (!pending.empty())
                {
                    const PendingObject current = pending.back();
                    pending.pop_back();
                    const auto found = ole_objects.find(current.handle);
                    if (found == ole_objects.end())
                    {
                        continue;
                    }

                    if (!current.children_queued)
                    {
                        pending.push_back({.handle = current.handle, .children_queued = true});
                        const std::vector<int> child_handles =
                            collect_native_owned_child_handles(found->second);
                        for (auto it = child_handles.rbegin(); it != child_handles.rend(); ++it)
                        {
                            if (scheduled_handles.insert(*it).second)
                            {
                                pending.push_back({.handle = *it, .children_queued = false});
                            }
                        }
                        continue;
                    }

                    order.push_back(current.handle);
                }
            };

            for (const int root_handle : collect_native_shutdown_roots())
            {
                append_tree(root_handle);
            }
            for (const auto &[handle, _] : ole_objects)
            {
                append_tree(handle);
            }
            return order;
        }

        std::vector<int> collect_native_shutdown_order_for_window(int target_handle)
        {
            std::set<int> target_handles;
            std::vector<int> pending{target_handle};
            while (!pending.empty())
            {
                const int handle = pending.back();
                pending.pop_back();
                if (!target_handles.insert(handle).second)
                {
                    continue;
                }

                const auto found = ole_objects.find(handle);
                if (found == ole_objects.end())
                {
                    continue;
                }
                const std::vector<int> child_handles =
                    collect_native_owned_child_handles(found->second);
                pending.insert(pending.end(), child_handles.begin(), child_handles.end());
            }

            std::vector<int> order;
            for (const int handle : collect_native_shutdown_order())
            {
                if (target_handles.contains(handle))
                {
                    order.push_back(handle);
                }
            }
            return order;
        }

        bool dispatch_query_unload_for_objects(
            const std::vector<int> &shutdown_order,
            const SourceLocation &location)
        {
            for (const int handle : shutdown_order)
            {
                auto found = ole_objects.find(handle);
                if (found == ole_objects.end())
                {
                    continue;
                }

                const std::string normalized_base_class =
                    normalize_identifier(trim_copy(found->second.base_class_name));
                if (normalized_base_class != "form" && normalized_base_class != "formset")
                {
                    continue;
                }

                std::string query_unload_program_path;
                std::string query_unload_method_name;
                if (find_native_object_method(
                        found->second,
                        "queryunload",
                        query_unload_program_path,
                        query_unload_method_name) == nullptr)
                {
                    continue;
                }
                if (!can_push_frame() || stack.empty())
                {
                    throw std::runtime_error(call_depth_limit_message());
                }

                events.push_back({.category = "prg.object.queryunload",
                                  .detail = query_unload_method_name,
                                  .location = location});
                last_popped_frame_requested_nodefault = false;
                bool query_unload_requested_nodefault = false;
                const auto query_unload_result = invoke_native_object_method_if_present(
                    found->second,
                    "queryunload",
                    stack.back(),
                    {},
                    {},
                    &query_unload_requested_nodefault);
                (void)consume_last_popped_frame_requested_nodefault();
                const bool query_unload_rejected =
                    query_unload_result.has_value() &&
                    query_unload_result->kind != PrgValueKind::empty &&
                    !value_as_bool(*query_unload_result);
                if (query_unload_rejected || query_unload_requested_nodefault)
                {
                    events.push_back({.category = "prg.object.queryunload_veto",
                                      .detail = found->second.prog_id,
                                      .location = location});
                    return false;
                }
            }
            return true;
        }

        bool dispatch_query_unload_for_quit(const SourceLocation &location)
        {
            return dispatch_query_unload_for_objects(collect_native_shutdown_order(), location);
        }

        void release_native_objects_for_shutdown()
        {
            for (const int handle : collect_native_shutdown_roots())
            {
                const auto found = ole_objects.find(handle);
                if (found != ole_objects.end())
                {
                    (void)release_native_object(found->second, "QUIT");
                }
            }
        }

        void close_runtime_scope(const std::string &scope, const SourceLocation &location)
        {
            DataSessionState &session = current_session_state();
            std::vector<int> areas;
            areas.reserve(session.cursors.size());
            for (const auto &[area, _cursor] : session.cursors)
            {
                areas.push_back(area);
            }
            for (const int area : areas)
            {
                close_cursor(std::to_string(area));
            }

            const auto [scope_name, scope_tail] = split_first_word(scope);
            const std::string close_scope = normalize_identifier(scope_name.empty() ? scope : scope_name);
            const bool close_all_databases =
                close_scope == "all" || normalize_identifier(scope_tail) == "all";
            if (close_scope == "database" || close_scope == "databases")
            {
                if (close_all_databases)
                {
                    session.databases.clear();
                    session.current_database_path.clear();
                }
                else if (!session.current_database_path.empty())
                {
                    const std::string closing_path = session.current_database_path;
                    std::erase_if(
                        session.databases,
                        [&](const auto &database)
                        {
                            return database_paths_equal(database.path, closing_path);
                        });
                    session.current_database_path.clear();
                }
                events.push_back({.category = "runtime.database.close",
                                  .detail = close_all_databases ? "ALL" : "CURRENT",
                                  .location = location});
            }
            else if (close_scope == "all")
            {
                for (auto &[_, candidate_session] : data_sessions)
                {
                    candidate_session.databases.clear();
                    candidate_session.current_database_path.clear();
                }
                events.push_back({.category = "runtime.database.close",
                                  .detail = "ALL",
                                  .location = location});
            }
            if (close_scope == "all" || close_scope == "databases" || close_scope == "database")
            {
                current_sql_connections().clear();
                current_registered_api_functions().clear();
                ole_objects.clear();
                close_all_file_io_handles();
            }

            events.push_back({.category = "runtime.close",
                              .detail = scope.empty() ? "ALL" : scope,
                              .location = location});
        }

        bool execute_inline_shutdown_clause(const SourceLocation &location)
        {
            const std::string trimmed = trim_copy(shutdown_handler);
            const std::string upper = uppercase_copy(trimmed);
            if (upper.empty())
            {
                return false;
            }
            if (upper == "CLEAR EVENTS")
            {
                waiting_for_events = false;
                restore_event_loop_after_dispatch = false;
                active_report_status = 0;
                report_interrupted = false;
                events.push_back({.category = "runtime.shutdown_handler",
                                  .detail = "CLEAR EVENTS",
                                  .location = location});
                return true;
            }
            if (upper == "CLOSE ALL" || upper == "CLOSE TABLES" || upper == "CLOSE DATABASE" ||
                upper == "CLOSE DATABASES" || upper == "CLOSE DATABASES ALL")
            {
                const std::size_t space_pos = upper.find(' ');
                const std::string scope = space_pos != std::string::npos ? trim_copy(upper.substr(space_pos + 1U)) : std::string{"ALL"};
                events.push_back({.category = "runtime.shutdown_handler",
                                  .detail = upper,
                                  .location = location});
                close_runtime_scope(scope, location);
                return true;
            }
            return false;
        }

        void abandon_expression_continuations()
        {
            for (Frame &active_frame : stack)
            {
                active_frame.expression_routine_return_pending = false;
                active_frame.expression_continuation.reset();
                active_frame.command_argument_continuation.reset();
                active_frame.loop_expression_continuation.reset();
                active_frame.scan_expression_continuation.reset();
            }
        }

        bool perform_quit(const SourceLocation &location)
        {
            if (!dispatch_query_unload_for_quit(location))
            {
                quit_pending_after_shutdown = false;
                pending_quit_location = {};
                events.push_back({.category = "runtime.quit_cancelled",
                                  .detail = "QueryUnload veto",
                                  .location = location});
                return false;
            }

            waiting_for_events = false;
            if (active_report_status != 0)
            {
                report_interrupted = true;
            }
            active_report_status = 0;
            restore_event_loop_after_dispatch = false;
            event_dispatch_return_depth.reset();
            handling_error = false;
            error_handler_return_depth.reset();
            error_metadata_stack.clear();
            handling_shutdown = false;
            shutdown_handler_return_depth.reset();
            quit_pending_after_shutdown = false;
            pending_quit_location = {};
            abandon_expression_continuations();

            release_native_objects_for_shutdown();
            cleanup_runtime_resources_for_shutdown();
            events.push_back({.category = "runtime.quit",
                              .detail = "QUIT",
                              .location = location});

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
            return true;
        }

        bool dispatch_shutdown_handler(const Frame &source_frame, const SourceLocation &location)
        {
            if (handling_shutdown || stack.empty())
            {
                return false;
            }

            std::string handler = trim_copy(shutdown_handler);
            if (handler.empty())
            {
                return false;
            }
            if (!starts_with_insensitive(handler, "DO "))
            {
                return false;
            }

            handler = trim_copy(handler.substr(3U));
            if (handler.empty())
            {
                return false;
            }

            std::string handler_arguments_clause;
            const std::size_t with_position = find_keyword_top_level(handler, "WITH");
            if (with_position != std::string::npos)
            {
                handler_arguments_clause = trim_copy(handler.substr(with_position + 4U));
                handler = trim_copy(handler.substr(0U, with_position));
            }
            if (handler.empty())
            {
                return false;
            }

            std::vector<PrgValue> handler_arguments;
            if (!handler_arguments_clause.empty())
            {
                for (const std::string &raw_argument : split_csv_like(handler_arguments_clause))
                {
                    const std::string argument_expression = trim_copy(raw_argument);
                    if (!argument_expression.empty())
                    {
                        handler_arguments.push_back(evaluate_expression(argument_expression, source_frame));
                    }
                }
            }

            for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
            {
                Program &program = load_program(iterator->file_path);
                const auto found = program.routines.find(normalize_identifier(handler));
                if (found == program.routines.end())
                {
                    continue;
                }
                if (!can_push_frame())
                {
                    return false;
                }

                handling_shutdown = true;
                shutdown_handler_return_depth = stack.size();
                quit_pending_after_shutdown = true;
                pending_quit_location = location;
                push_routine_frame(program.path, found->second, handler_arguments);
                events.push_back({.category = "runtime.shutdown_handler",
                                  .detail = handler_arguments.empty()
                                                ? found->second.name
                                                : found->second.name + " WITH " + std::to_string(handler_arguments.size()) + " argument(s)",
                                  .location = location});
                return true;
            }

            return false;
        }

        RuntimePauseState build_pause_state(DebugPauseReason reason, std::string message = {})
        {
            RuntimePauseState state;
            state.paused = reason != DebugPauseReason::completed;
            state.completed = reason == DebugPauseReason::completed;
            state.waiting_for_events = waiting_for_events;
            state.reason = reason;
            state.message = std::move(message);
            state.executed_statement_count = executed_statement_count;
            state.globals = globals;
            state.last_return_value = last_return_value;
            state.events = events;
            const DataSessionState &session = current_session_state();
            state.work_area.selected = session.selected_work_area;
            state.work_area.data_session = current_data_session;
            state.work_area.aliases = session.aliases;
            for (const auto &[_, cursor] : session.cursors)
            {
                state.cursors.push_back({.work_area = cursor.work_area,
                                         .alias = cursor.alias,
                                         .source_path = cursor.source_path,
                                         .source_kind = cursor.source_kind,
                                         .filter_expression = cursor.filter_expression,
                                         .remote = cursor.remote,
                                         .record_count = cursor.record_count,
                                         .recno = cursor.recno,
                                         .bof = cursor.bof,
                                         .eof = cursor.eof});
            }
            state.databases = session.databases;
            for (const auto &[_, connection] : current_sql_connections())
            {
                state.sql_connections.push_back(connection);
            }
            for (const auto &[_, object] : ole_objects)
            {
                if (!object.hidden_runtime_surface)
                {
                    state.ole_objects.push_back(object);
                }
            }

            if (reason == DebugPauseReason::error)
            {
                const auto error_event = std::find_if(events.rbegin(), events.rend(), [](const RuntimeEvent &event)
                                                      { return event.category == "runtime.error"; });
                if (error_event != events.rend())
                {
                    state.location = error_event->location;
                }
                else if (!last_fault_location.file_path.empty())
                {
                    state.location = last_fault_location;
                }
                if (!last_fault_statement.empty())
                {
                    state.statement_text = last_fault_statement;
                }
            }
            else if (const Statement *statement = current_statement())
            {
                state.location = statement->location;
                state.statement_text = statement->text;
            }
            state.statement_text = display_asset_paths_in_statement(std::move(state.statement_text));

            bool assigned_fault_frame_line = false;
            for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
            {
                RuntimeStackFrame frame;
                frame.file_path = iterator->file_path;
                frame.routine_name = iterator->routine_name;
                if (reason == DebugPauseReason::error &&
                    !assigned_fault_frame_line &&
                    !last_fault_location.file_path.empty())
                {
                    frame.line = last_fault_location.line;
                    assigned_fault_frame_line = true;
                }
                else if (iterator->routine != nullptr)
                {
                    const std::size_t statement_index =
                        iterator->expression_routine_return_pending && iterator->pc > 0U
                            ? iterator->pc - 1U
                            : iterator->pc;
                    if (statement_index < iterator->routine->statements.size())
                    {
                        frame.line = iterator->routine->statements[statement_index].location.line;
                    }
                }
                frame.locals = iterator->locals;
                state.call_stack.push_back(std::move(frame));
            }

            last_state = state;
            return state;
        }

        [[nodiscard]] bool can_push_frame() const
        {
            return stack.size() <= max_call_depth;
        }

        [[nodiscard]] std::string call_depth_limit_message() const
        {
            return runtime_text(
                "Runtime.Prg.Core.Error.GuardrailCallDepthExceeded",
                {{"limit", std::to_string(max_call_depth)}});
        }

        [[nodiscard]] std::string step_budget_limit_message() const
        {
            return runtime_text(
                "Runtime.Prg.Core.Error.GuardrailExecutedStatementsExceeded",
                {{"limit", std::to_string(max_executed_statements)}});
        }

        [[nodiscard]] std::string loop_iteration_limit_message() const
        {
            return runtime_text(
                "Runtime.Prg.Core.Error.GuardrailLoopIterationsExceeded",
                {{"limit", std::to_string(max_loop_iterations)}});
        }
