// prg_engine_session.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        Program &load_program(const std::string &path)
        {
            const std::string normalized = normalize_path(path);
            const auto existing = programs.find(normalized);
            if (existing != programs.end())
            {
                return existing->second;
            }
            auto [inserted, _] = programs.emplace(normalized, parse_program(normalized));
            return inserted->second;
        }

        void push_main_frame(
            const std::string &path,
            std::vector<PrgValue> call_arguments = {},
            std::vector<std::optional<std::string>> call_argument_references = {})
        {
            Program &program = load_program(path);
            Frame frame;
            frame.file_path = program.path;
            frame.routine_name = "main";
            frame.routine = &program.main;
            frame.call_arguments = std::move(call_arguments);
            frame.call_argument_references = std::move(call_argument_references);
            stack.push_back(std::move(frame));
        }

        void push_routine_frame(
            const std::string &path,
            const Routine &routine,
            std::vector<PrgValue> call_arguments = {},
            std::vector<std::optional<std::string>> call_argument_references = {})
        {
            Frame frame;
            frame.file_path = normalize_path(path);
            frame.routine_name = routine.name;
            frame.routine = &routine;
            frame.call_arguments = std::move(call_arguments);
            frame.call_argument_references = std::move(call_argument_references);
            stack.push_back(std::move(frame));
        }

        const Statement *current_statement() const
        {
            if (stack.empty())
            {
                return nullptr;
            }
            const Frame &frame = stack.back();
            if (frame.routine == nullptr || frame.pc >= frame.routine->statements.size())
            {
                return nullptr;
            }
            return &frame.routine->statements[frame.pc];
        }

        void capture_last_error_context(const Frame &frame, const Statement &statement)
        {
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            last_error_code = classify_runtime_error_code(last_error_message);
            last_error_work_area = current_selected_work_area();
            last_error_procedure = frame.routine_name;
            if (last_error_code != 1526 && last_error_code != 1429)
            {
                last_error_compatibility = {};
            }
        }

        [[nodiscard]] FaultMetadataSnapshot snapshot_current_error_metadata() const
        {
            FaultMetadataSnapshot snapshot;
            snapshot.message = last_error_message;
            snapshot.location = last_fault_location;
            snapshot.statement = last_fault_statement;
            snapshot.code = last_error_code;
            snapshot.work_area = last_error_work_area;
            snapshot.procedure = last_error_procedure;
            snapshot.compatibility = last_error_compatibility;
            snapshot.session_state_snapshot = current_session_state();
            return snapshot;
        }

        [[nodiscard]] const FaultMetadataSnapshot *active_error_metadata() const
        {
            if (error_metadata_stack.empty())
            {
                return nullptr;
            }

            return &error_metadata_stack.back();
        }

        [[nodiscard]] const std::string &current_error_message() const
        {
            const FaultMetadataSnapshot *snapshot = active_error_metadata();
            return snapshot == nullptr ? last_error_message : snapshot->message;
        }

        [[nodiscard]] int current_error_code() const
        {
            const FaultMetadataSnapshot *snapshot = active_error_metadata();
            return snapshot == nullptr ? last_error_code : snapshot->code;
        }

        [[nodiscard]] int current_error_work_area() const
        {
            const FaultMetadataSnapshot *snapshot = active_error_metadata();
            return snapshot == nullptr ? last_error_work_area : snapshot->work_area;
        }

        [[nodiscard]] const std::string &current_error_procedure() const
        {
            const FaultMetadataSnapshot *snapshot = active_error_metadata();
            return snapshot == nullptr ? last_error_procedure : snapshot->procedure;
        }

        [[nodiscard]] const SourceLocation &current_fault_location() const
        {
            const FaultMetadataSnapshot *snapshot = active_error_metadata();
            return snapshot == nullptr ? last_fault_location : snapshot->location;
        }

        [[nodiscard]] const std::string &current_fault_statement() const
        {
            const FaultMetadataSnapshot *snapshot = active_error_metadata();
            return snapshot == nullptr ? last_fault_statement : snapshot->statement;
        }

        [[nodiscard]] const AErrorCompatibilitySnapshot &current_error_compatibility() const
        {
            const FaultMetadataSnapshot *snapshot = active_error_metadata();
            return snapshot == nullptr ? last_error_compatibility : snapshot->compatibility;
        }

        void record_sql_aerror_context(const std::string &detail,
                                       const std::string &state,
                                       int native_code,
                                       const std::string &context,
                                       const std::string &payload)
        {
            last_error_compatibility = {};
            last_error_compatibility.sql_detail = detail;
            last_error_compatibility.sql_state = state;
            last_error_compatibility.sql_native_code = native_code;
            last_error_compatibility.has_sql_native_code = true;
            last_error_compatibility.sql_context = context;
            last_error_compatibility.sql_payload = payload;
        }

        void record_ole_aerror_context(const std::string &detail,
                                       const std::string &app,
                                       const std::string &source,
                                       const std::string &action,
                                       int native_code)
        {
            last_error_compatibility = {};
            last_error_compatibility.ole_detail = detail;
            last_error_compatibility.ole_app = app;
            last_error_compatibility.ole_source = source;
            last_error_compatibility.ole_action = action;
            last_error_compatibility.ole_native_code = native_code;
            last_error_compatibility.has_ole_native_code = true;
        }

        DataSessionState &current_session_state()
        {
            auto [iterator, _] = data_sessions.try_emplace(current_data_session);
            iterator->second.selected_work_area = std::max(1, iterator->second.selected_work_area);
            iterator->second.next_work_area = std::max(1, iterator->second.next_work_area);
            return iterator->second;
        }

        const DataSessionState &current_session_state() const
        {
            const auto found = data_sessions.find(current_data_session);
            if (found != data_sessions.end())
            {
                return found->second;
            }
            static const DataSessionState empty_session{};
            return empty_session;
        }

        int current_selected_work_area() const
        {
            return current_session_state().selected_work_area;
        }

        std::string &current_default_directory()
        {
            auto [iterator, _] = default_directory_by_session.try_emplace(current_data_session, startup_default_directory);
            return iterator->second;
        }

        const std::string &current_default_directory() const
        {
            const auto found = default_directory_by_session.find(current_data_session);
            if (found != default_directory_by_session.end())
            {
                return found->second;
            }

            return startup_default_directory;
        }

        std::map<int, RuntimeSqlConnectionState> &current_sql_connections()
        {
            auto [iterator, _] = sql_connections_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        const std::map<int, RuntimeSqlConnectionState> &current_sql_connections() const
        {
            const auto found = sql_connections_by_session.find(current_data_session);
            if (found != sql_connections_by_session.end())
            {
                return found->second;
            }

            static const std::map<int, RuntimeSqlConnectionState> empty_connections;
            return empty_connections;
        }

        int &current_sql_handle_counter()
        {
            auto [iterator, _] = next_sql_handle_by_session.try_emplace(current_data_session, 1);
            iterator->second = std::max(1, iterator->second);
            return iterator->second;
        }

        int &current_api_handle_counter()
        {
            auto [iterator, _] = next_api_handle_by_session.try_emplace(current_data_session, 1);
            iterator->second = std::max(1, iterator->second);
            return iterator->second;
        }

        int &current_transaction_level()
        {
            auto [iterator, _] = transaction_level_by_session.try_emplace(current_data_session, 0);
            iterator->second = std::max(0, iterator->second);
            return iterator->second;
        }

        int current_transaction_level() const
        {
            const auto found = transaction_level_by_session.find(current_data_session);
            if (found != transaction_level_by_session.end())
            {
                return std::max(0, found->second);
            }

            return 0;
        }

        std::filesystem::path transaction_journal_root_directory() const
        {
            return runtime_temp_directory / "transactions";
        }

        bool write_transaction_journal_file(const TransactionJournalState &state) const
        {
            std::error_code directory_error;
            std::filesystem::create_directories(state.root_path, directory_error);
            if (directory_error)
            {
                return false;
            }

            std::ofstream output(state.journal_path, std::ios::binary | std::ios::trunc);
            if (!output)
            {
                return false;
            }

            output << "VERSION\t1\n";
            output << "LEVEL\t" << state.level << "\n";
            for (const auto &[_, entry] : state.tracked_files)
            {
                output << "FILE\t"
                       << entry.original_path << "\t"
                       << (entry.existed_at_start ? "1" : "0") << "\t"
                       << entry.backup_path << "\n";
            }

            output.flush();
            return output.good();
        }

        std::vector<std::filesystem::path> transaction_companion_paths(const std::string &table_path) const
        {
            std::vector<std::filesystem::path> paths;
            const std::filesystem::path source = std::filesystem::path(normalize_path(table_path)).lexically_normal();
            if (source.empty())
            {
                return paths;
            }

            paths.push_back(source);

            const auto push_if_unique = [&paths](const std::filesystem::path &candidate)
            {
                const std::filesystem::path normalized = candidate.lexically_normal();
                if (std::find(paths.begin(), paths.end(), normalized) == paths.end())
                {
                    paths.push_back(normalized);
                }
            };

            push_if_unique(source.parent_path() / (source.stem().string() + ".fpt"));
            push_if_unique(source.parent_path() / (source.stem().string() + ".cdx"));
            return paths;
        }

        bool replay_transaction_journal_state(const TransactionJournalState &state)
        {
            bool ok = true;
            std::error_code ignored;
            for (const auto &[_, entry] : state.tracked_files)
            {
                const std::filesystem::path original(entry.original_path);
                if (entry.existed_at_start)
                {
                    if (!entry.backup_path.empty())
                    {
                        const std::filesystem::path backup(entry.backup_path);
                        if (std::filesystem::exists(backup, ignored))
                        {
                            std::error_code copy_error;
                            std::filesystem::create_directories(original.parent_path(), copy_error);
                            copy_error.clear();
                            std::filesystem::copy_file(backup, original, std::filesystem::copy_options::overwrite_existing, copy_error);
                            if (copy_error)
                            {
                                ok = false;
                            }
                        }
                    }
                }
                else if (std::filesystem::exists(original, ignored))
                {
                    std::filesystem::remove(original, ignored);
                }
            }

            std::filesystem::remove_all(state.root_path, ignored);
            return ok;
        }

        bool load_transaction_journal_from_file(
            const std::filesystem::path &journal_path,
            TransactionJournalState &state)
        {
            std::ifstream input(journal_path, std::ios::binary);
            if (!input)
            {
                return false;
            }

            state = TransactionJournalState{};
            state.root_path = journal_path.parent_path();
            state.journal_path = journal_path;

            std::string line;
            while (std::getline(input, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                std::vector<std::string> tokens;
                std::size_t token_start = 0;
                while (token_start <= line.size())
                {
                    const std::size_t separator = line.find('\t', token_start);
                    if (separator == std::string::npos)
                    {
                        tokens.push_back(line.substr(token_start));
                        break;
                    }
                    tokens.push_back(line.substr(token_start, separator - token_start));
                    token_start = separator + 1;
                }
                if (tokens.empty())
                {
                    continue;
                }
                if (tokens[0] == "LEVEL" && tokens.size() >= 2U)
                {
                    try
                    {
                        state.level = std::max(0, std::stoi(tokens[1]));
                    }
                    catch (...)
                    {
                        state.level = 0;
                    }
                    continue;
                }
                if (tokens[0] == "FILE" && tokens.size() >= 4U)
                {
                    TransactionJournalFileEntry entry;
                    entry.original_path = tokens[1];
                    entry.existed_at_start = tokens[2] == "1";
                    entry.backup_path = tokens[3];
                    state.tracked_files[normalize_path(entry.original_path)] = std::move(entry);
                }
            }

            return true;
        }

        void replay_pending_transaction_journals()
        {
            const std::filesystem::path root = transaction_journal_root_directory();
            std::error_code ignored;
            if (!std::filesystem::exists(root, ignored))
            {
                return;
            }

            for (const auto &entry : std::filesystem::directory_iterator(root, ignored))
            {
                if (ignored)
                {
                    break;
                }
                if (!entry.is_directory())
                {
                    continue;
                }

                const std::filesystem::path journal_path = entry.path() / "journal.log";
                if (!std::filesystem::exists(journal_path, ignored))
                {
                    continue;
                }

                TransactionJournalState state;
                if (!load_transaction_journal_from_file(journal_path, state))
                {
                    std::filesystem::remove_all(entry.path(), ignored);
                    continue;
                }

                if (replay_transaction_journal_state(state))
                {
                    events.push_back({.category = "runtime.transaction.replay",
                                      .detail = journal_path.string(),
                                      .location = {}});
                }
            }
        }

        TransactionJournalState &current_transaction_journal()
        {
            auto [iterator, _] = transaction_journal_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        bool begin_transaction_journal_if_needed()
        {
            if (current_transaction_level() <= 0)
            {
                return true;
            }

            TransactionJournalState &journal = current_transaction_journal();
            if (!journal.journal_path.empty())
            {
                return true;
            }

            const unsigned long long process_id =
#if defined(_WIN32)
                static_cast<unsigned long long>(::_getpid());
#else
                static_cast<unsigned long long>(::getpid());
#endif
            const std::string nonce = std::to_string(static_cast<unsigned long long>(std::time(nullptr))) +
                                      "_" + std::to_string(process_id) +
                                      "_" + std::to_string(static_cast<unsigned long long>(current_data_session));
            journal.root_path = transaction_journal_root_directory() / ("txn_" + nonce);
            journal.journal_path = journal.root_path / "journal.log";
            journal.level = current_transaction_level();
            if (!write_transaction_journal_file(journal))
            {
                last_error_message = "Unable to initialize transaction journal";
                return false;
            }
            return true;
        }

        bool sync_transaction_journal_level()
        {
            auto found = transaction_journal_by_session.find(current_data_session);
            if (found == transaction_journal_by_session.end())
            {
                return true;
            }

            found->second.level = current_transaction_level();
            if (found->second.journal_path.empty())
            {
                return true;
            }

            if (!write_transaction_journal_file(found->second))
            {
                last_error_message = "Unable to persist transaction journal state";
                return false;
            }
            return true;
        }

        bool ensure_transaction_backup_for_table(const std::string &table_path)
        {
            if (current_transaction_level() <= 0)
            {
                return true;
            }
            if (!begin_transaction_journal_if_needed())
            {
                return false;
            }

            TransactionJournalState &journal = current_transaction_journal();
            std::error_code ignored;
            for (const auto &path : transaction_companion_paths(table_path))
            {
                const std::string key = normalize_path(path.string());
                if (journal.tracked_files.contains(key))
                {
                    continue;
                }

                TransactionJournalFileEntry entry;
                entry.original_path = key;
                entry.existed_at_start = std::filesystem::exists(path, ignored);
                if (entry.existed_at_start)
                {
                    const std::filesystem::path backup_path = journal.root_path /
                                                              ("backup_" + std::to_string(journal.tracked_files.size()) +
                                                               path.extension().string());
                    std::error_code copy_error;
                    std::filesystem::create_directories(backup_path.parent_path(), copy_error);
                    copy_error.clear();
                    std::filesystem::copy_file(path, backup_path, std::filesystem::copy_options::overwrite_existing, copy_error);
                    if (copy_error)
                    {
                        last_error_message = "Unable to create transaction backup for: " + key;
                        return false;
                    }
                    entry.backup_path = backup_path.string();
                }

                journal.tracked_files.emplace(key, std::move(entry));
                if (!write_transaction_journal_file(journal))
                {
                    last_error_message = "Unable to persist transaction backup journal";
                    return false;
                }
            }

            return true;
        }

        void refresh_local_cursors_after_transaction_replay()
        {
            DataSessionState &session = current_session_state();
            std::vector<int> closed_areas;
            for (auto &[area, cursor] : session.cursors)
            {
                if (cursor.remote || cursor.source_path.empty())
                {
                    continue;
                }

                std::error_code ignored;
                if (!std::filesystem::exists(cursor.source_path, ignored))
                {
                    closed_areas.push_back(area);
                    continue;
                }

                const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, std::max<std::size_t>(cursor.record_count, 1U));
                if (!table_result.ok)
                {
                    closed_areas.push_back(area);
                    continue;
                }

                cursor.record_count = table_result.table.header.record_count;
                cursor.field_count = table_result.table.fields.size();
                cursor.record_length = table_result.table.header.record_length;
                std::set<std::string> visible_fields;
                for (const auto &field : table_result.table.fields)
                {
                    visible_fields.insert(collapse_identifier(field.name));
                }
                for (auto it = cursor.field_rules.begin(); it != cursor.field_rules.end();)
                {
                    if (!visible_fields.contains(it->first))
                    {
                        it = cursor.field_rules.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
                if (cursor.record_count == 0U)
                {
                    move_cursor_to(cursor, 0);
                }
                else
                {
                    move_cursor_to(cursor, static_cast<long long>(std::min<std::size_t>(cursor.recno == 0U ? 1U : cursor.recno, cursor.record_count)));
                }
            }

            for (const int area : closed_areas)
            {
                session.aliases.erase(area);
                session.table_locks.erase(area);
                session.record_locks.erase(area);
                session.cursors.erase(area);
                session.next_work_area = std::min(session.next_work_area, area);
            }
        }

        bool rollback_active_transaction_journal()
        {
            auto found = transaction_journal_by_session.find(current_data_session);
            if (found == transaction_journal_by_session.end())
            {
                return true;
            }

            if (!replay_transaction_journal_state(found->second))
            {
                last_error_message = "Failed to replay transaction journal";
                return false;
            }

            transaction_journal_by_session.erase(found);
            refresh_local_cursors_after_transaction_replay();
            return true;
        }

        void commit_active_transaction_journal()
        {
            auto found = transaction_journal_by_session.find(current_data_session);
            if (found == transaction_journal_by_session.end())
            {
                return;
            }

            std::error_code ignored;
            std::filesystem::remove_all(found->second.root_path, ignored);
            transaction_journal_by_session.erase(found);
        }

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

        void cleanup_runtime_resources_for_shutdown()
        {
            // Release open work areas/cursors across all data sessions.
            for (auto &[_, session] : data_sessions)
            {
                session.cursors.clear();
                session.aliases.clear();
                session.table_locks.clear();
                session.record_locks.clear();
                session.selected_work_area = 1;
                session.next_work_area = 1;
            }

            // Release synthetic SQL/OLE/runtime interop state.
            sql_connections_by_session.clear();
            next_sql_handle_by_session.clear();
            registered_api_functions_by_session.clear();
            next_api_handle_by_session.clear();
            ole_objects.clear();

            // Ensure FOPEN handles are closed so files are not left locked.
            close_all_file_io_handles();

#if defined(_WIN32)
            // Release any DLL handles loaded through DECLARE ... IN.
            std::set<HMODULE> released_modules;
            for (auto &[_, declfn] : declared_dll_functions)
            {
                if (declfn.hmodule != nullptr && !released_modules.contains(declfn.hmodule))
                {
                    FreeLibrary(declfn.hmodule);
                    released_modules.insert(declfn.hmodule);
                }
                declfn.hmodule = nullptr;
                declfn.proc_address = nullptr;
            }
#endif
            declared_dll_functions.clear();
            loaded_libraries.clear();
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

            const auto [scope_name, _scope_tail] = split_first_word(scope);
            const std::string close_scope = normalize_identifier(scope_name.empty() ? scope : scope_name);
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
            for (const auto &[_, connection] : current_sql_connections())
            {
                state.sql_connections.push_back(connection);
            }
            for (const auto &[_, object] : ole_objects)
            {
                state.ole_objects.push_back(object);
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
                else if (iterator->routine != nullptr && iterator->pc < iterator->routine->statements.size())
                {
                    frame.line = iterator->routine->statements[iterator->pc].location.line;
                }
                frame.locals = iterator->locals;
                state.call_stack.push_back(std::move(frame));
            }

            last_state = state;
            return state;
        }

        [[nodiscard]] bool can_push_frame() const
        {
            return stack.size() < max_call_depth;
        }

        [[nodiscard]] std::string call_depth_limit_message() const
        {
            return "Runtime guardrail: maximum call depth (" + std::to_string(max_call_depth) + ") exceeded.";
        }

        [[nodiscard]] std::string step_budget_limit_message() const
        {
            return "Runtime guardrail: maximum executed statements (" + std::to_string(max_executed_statements) + ") exceeded.";
        }

        [[nodiscard]] std::string loop_iteration_limit_message() const
        {
            return "Runtime guardrail: maximum loop iterations (" + std::to_string(max_loop_iterations) + ") exceeded.";
        }
