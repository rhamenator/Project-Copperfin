// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.
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

        void perform_quit(const SourceLocation &location)
        {
            waiting_for_events = false;
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
