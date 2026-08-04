// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

        static std::string make_lock_owner_key(std::uint64_t runtime_id, int data_session)
        {
            return std::to_string(runtime_id) + ":" + std::to_string(std::max(1, data_session));
        }

        [[nodiscard]] std::string current_lock_owner_key() const
        {
            return make_lock_owner_key(runtime_instance_id, current_data_session);
        }

        [[nodiscard]] std::string cursor_lock_resource_key(const CursorState &cursor) const
        {
            if (!cursor.source_path.empty())
            {
                return normalize_path(cursor.source_path);
            }

            return "runtime:" + std::to_string(runtime_instance_id) +
                   ":session:" + std::to_string(std::max(1, current_data_session)) +
                   ":area:" + std::to_string(cursor.work_area);
        }

        struct ReprocessPolicy
        {
            std::string display_value = "AUTOMATIC";
            std::size_t retry_budget = 8U;
        };

        [[nodiscard]] ReprocessPolicy current_reprocess_policy() const
        {
            const auto found = current_set_state().find("reprocess");
            if (found == current_set_state().end())
            {
                return {};
            }

            const std::string trimmed = trim_copy(found->second);
            if (trimmed.empty())
            {
                return {};
            }

            const std::string normalized = normalize_identifier(trimmed);
            if (normalized == "automatic" || normalized == "auto" ||
                normalized == "on" || normalized == "true" || normalized == "yes")
            {
                return {.display_value = "AUTOMATIC", .retry_budget = 8U};
            }

            const auto parsed = copperfin::platform::try_parse_invariant_integer<long long>(trimmed);
            if (!parsed.has_value() || *parsed < 0LL)
            {
                return {.display_value = uppercase_copy(trimmed), .retry_budget = 0U};
            }
            return {.display_value = std::to_string(*parsed),
                    .retry_budget = static_cast<std::size_t>(*parsed)};
        }

        void release_shared_lock_ownership_for_cursor(const CursorState &cursor,
                                                      const DataSessionState &session,
                                                      int data_session)
        {
            const std::string owner_key = make_lock_owner_key(runtime_instance_id, data_session);
            const std::string resource_key = cursor_lock_resource_key(cursor);
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);

            if (session.table_locks.contains(cursor.work_area))
            {
                const auto table_found = concurrency_state->table_lock_owner_by_resource.find(resource_key);
                if (table_found != concurrency_state->table_lock_owner_by_resource.end() &&
                    table_found->second == owner_key)
                {
                    concurrency_state->table_lock_owner_by_resource.erase(table_found);
                }
            }

            const auto record_found = session.record_locks.find(cursor.work_area);
            if (record_found == session.record_locks.end())
            {
                return;
            }

            auto shared_record_found = concurrency_state->record_lock_owner_by_resource.find(resource_key);
            if (shared_record_found == concurrency_state->record_lock_owner_by_resource.end())
            {
                return;
            }

            for (const std::size_t recno : record_found->second)
            {
                const auto owner_found = shared_record_found->second.find(recno);
                if (owner_found != shared_record_found->second.end() && owner_found->second == owner_key)
                {
                    shared_record_found->second.erase(owner_found);
                }
            }

            if (shared_record_found->second.empty())
            {
                concurrency_state->record_lock_owner_by_resource.erase(shared_record_found);
            }
        }

        void release_shared_record_lock_ownership(const CursorState &cursor,
                                                  std::size_t recno,
                                                  int data_session)
        {
            const std::string owner_key = make_lock_owner_key(runtime_instance_id, data_session);
            const std::string resource_key = cursor_lock_resource_key(cursor);
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);

            auto shared_record_found = concurrency_state->record_lock_owner_by_resource.find(resource_key);
            if (shared_record_found == concurrency_state->record_lock_owner_by_resource.end())
            {
                return;
            }

            const auto owner_found = shared_record_found->second.find(recno);
            if (owner_found != shared_record_found->second.end() && owner_found->second == owner_key)
            {
                shared_record_found->second.erase(owner_found);
                if (shared_record_found->second.empty())
                {
                    concurrency_state->record_lock_owner_by_resource.erase(shared_record_found);
                }
            }
        }
        void release_shared_table_lock_ownership(const CursorState &cursor, int data_session)
        {
            const std::string owner_key = make_lock_owner_key(runtime_instance_id, data_session);
            const std::string resource_key = cursor_lock_resource_key(cursor);
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);

            const auto table_found = concurrency_state->table_lock_owner_by_resource.find(resource_key);
            if (table_found != concurrency_state->table_lock_owner_by_resource.end() &&
                table_found->second == owner_key)
            {
                concurrency_state->table_lock_owner_by_resource.erase(table_found);
            }
        }

        void clear_all_shared_lock_ownership()
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            concurrency_state->table_lock_owner_by_resource.clear();
            concurrency_state->record_lock_owner_by_resource.clear();
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
            output.imbue(std::locale::classic());

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

        std::filesystem::path command_undo_journal_root_directory() const
        {
            return runtime_temp_directory / "command_undo";
        }

        TransactionJournalState &current_command_undo_journal()
        {
            auto [iterator, _] = command_undo_journal_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        std::vector<TransactionJournalState> &current_command_undo_stack()
        {
            auto [iterator, _] = command_undo_stack_by_session.try_emplace(current_data_session, std::vector<TransactionJournalState>{});
            return iterator->second;
        }

        [[nodiscard]] bool can_undo_command() const
        {
            const auto found = command_undo_stack_by_session.find(current_data_session);
            return found != command_undo_stack_by_session.end() && !found->second.empty();
        }

        [[nodiscard]] std::string command_undo_label() const
        {
            const auto found = command_undo_stack_by_session.find(current_data_session);
            if (found == command_undo_stack_by_session.end() || found->second.empty())
            {
                return {};
            }
            return found->second.back().command_label;
        }

        std::string command_undo_backup_message(const std::string &path) const
        {
            return runtime_text("Runtime.Prg.CommandUndo.Error.BackupCreateFailed", {{"path", path}});
        }

        std::string command_undo_journal_initialize_message() const
        {
            return runtime_text("Runtime.Prg.CommandUndo.Error.JournalInitializeFailed");
        }

        std::string command_undo_journal_persist_message() const
        {
            return runtime_text("Runtime.Prg.CommandUndo.Error.JournalPersistFailed");
        }

        std::string command_undo_journal_replay_message() const
        {
            return runtime_text("Runtime.Prg.CommandUndo.Error.JournalReplayFailed");
        }

        std::string command_undo_empty_message() const
        {
            return runtime_text("Runtime.Prg.CommandUndo.Error.NoCommand");
        }

        bool begin_command_undo_journal_if_needed()
        {
            TransactionJournalState &journal = current_command_undo_journal();
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
            static std::atomic<unsigned long long> command_undo_nonce_counter{0ULL};
            const auto now_ticks = static_cast<unsigned long long>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count());
            const unsigned long long nonce_counter = command_undo_nonce_counter.fetch_add(1ULL, std::memory_order_relaxed);
            const std::string nonce = std::to_string(now_ticks) +
                                      "_" + std::to_string(process_id) +
                                      "_" + std::to_string(static_cast<unsigned long long>(current_data_session)) +
                                      "_" + std::to_string(nonce_counter);
            journal = {};
            journal.root_path = command_undo_journal_root_directory() / ("undo_" + nonce);
            journal.journal_path = journal.root_path / "journal.log";
            journal.level = 0;
            if (!write_transaction_journal_file(journal))
            {
                last_error_message = command_undo_journal_initialize_message();
                return false;
            }
            return true;
        }

        bool ensure_command_undo_backup_for_table(const std::string &table_path)
        {
            if (!begin_command_undo_journal_if_needed())
            {
                return false;
            }

            TransactionJournalState &journal = current_command_undo_journal();
            std::error_code ignored;
            for (const auto &path : transaction_companion_paths(table_path))
            {
                const std::string key = normalize_path(copperfin::platform::path_to_utf8_string(path));
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
                                                               copperfin::platform::path_to_utf8_string(path.extension()));
                    std::error_code copy_error;
                    std::filesystem::create_directories(backup_path.parent_path(), copy_error);
                    copy_error.clear();
                    std::filesystem::copy_file(path, backup_path, std::filesystem::copy_options::overwrite_existing, copy_error);
                    if (copy_error)
                    {
                        last_error_message = command_undo_backup_message(key);
                        return false;
                    }
                    entry.backup_path = copperfin::platform::path_to_utf8_string(backup_path);
                }

                journal.tracked_files.emplace(key, std::move(entry));
                if (!write_transaction_journal_file(journal))
                {
                    last_error_message = command_undo_journal_persist_message();
                    return false;
                }
            }

            return true;
        }

        void rollback_active_command_undo_journal()
        {
            auto found = command_undo_journal_by_session.find(current_data_session);
            if (found == command_undo_journal_by_session.end())
            {
                return;
            }

            TransactionJournalState state = std::move(found->second);
            command_undo_journal_by_session.erase(found);
            if (!state.tracked_files.empty() && !replay_transaction_journal_state(state))
            {
                last_error_message = command_undo_journal_replay_message();
                return;
            }
            std::error_code ignored;
            if (!state.tracked_files.empty())
            {
                refresh_local_cursors_after_transaction_replay();
            }
            std::filesystem::remove_all(state.root_path, ignored);
        }

        void commit_active_command_undo_journal()
        {
            auto found = command_undo_journal_by_session.find(current_data_session);
            if (found == command_undo_journal_by_session.end())
            {
                return;
            }

            if (found->second.tracked_files.empty())
            {
                std::error_code ignored;
                std::filesystem::remove_all(found->second.root_path, ignored);
            }
            else
            {
                current_command_undo_stack().push_back(std::move(found->second));
            }
            command_undo_journal_by_session.erase(found);
        }

        bool undo_latest_command_journal()
        {
            auto found = command_undo_stack_by_session.find(current_data_session);
            if (found == command_undo_stack_by_session.end() || found->second.empty())
            {
                last_error_message = command_undo_empty_message();
                return false;
            }

            TransactionJournalState state = std::move(found->second.back());
            found->second.pop_back();
            if (found->second.empty())
            {
                command_undo_stack_by_session.erase(found);
            }

            if (!replay_transaction_journal_state(state))
            {
                last_error_message = command_undo_journal_replay_message();
                return false;
            }
            refresh_local_cursors_after_transaction_replay();
            std::error_code ignored;
            std::filesystem::remove_all(state.root_path, ignored);
            return true;
        }

        bool undo_all_command_journals()
        {
            auto found = command_undo_stack_by_session.find(current_data_session);
            if (found == command_undo_stack_by_session.end() || found->second.empty())
            {
                last_error_message = command_undo_empty_message();
                return false;
            }

            while (!found->second.empty())
            {
                TransactionJournalState state = std::move(found->second.back());
                found->second.pop_back();
                if (!replay_transaction_journal_state(state))
                {
                    last_error_message = command_undo_journal_replay_message();
                    return false;
                }
                refresh_local_cursors_after_transaction_replay();
                std::error_code ignored;
                std::filesystem::remove_all(state.root_path, ignored);
            }
            command_undo_stack_by_session.erase(found);
            return true;
        }

        int allocate_async_task_handle()
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            int &handle = concurrency_state->next_async_task_handle_by_session[current_data_session];
            handle = std::max(1, handle);
            return handle++;
        }

        void register_async_task(const std::shared_ptr<AsyncTaskState> &task)
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            concurrency_state->async_tasks_by_session[current_data_session][task->handle] = task;
        }

        std::shared_ptr<AsyncTaskState> find_async_task(int handle)
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            const auto session_found = concurrency_state->async_tasks_by_session.find(current_data_session);
            if (session_found == concurrency_state->async_tasks_by_session.end())
            {
                return nullptr;
            }

            const auto task_found = session_found->second.find(handle);
            if (task_found == session_found->second.end())
            {
                return nullptr;
            }

            return task_found->second;
        }

        void erase_async_task(int handle)
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            const auto session_found = concurrency_state->async_tasks_by_session.find(current_data_session);
            if (session_found == concurrency_state->async_tasks_by_session.end())
            {
                return;
            }

            session_found->second.erase(handle);
            if (session_found->second.empty())
            {
                concurrency_state->async_tasks_by_session.erase(session_found);
            }
        }

        void cancel_all_async_tasks()
        {
            std::vector<std::shared_ptr<std::atomic<bool>>> cancellation_tokens;
            {
                std::lock_guard<std::mutex> lock(concurrency_state->mutex);
                const auto session_found = concurrency_state->async_tasks_by_session.find(current_data_session);
                if (session_found == concurrency_state->async_tasks_by_session.end())
                {
                    return;
                }

                for (const auto &[_, task] : session_found->second)
                {
                    if (task != nullptr && task->cancel_requested != nullptr)
                    {
                        cancellation_tokens.push_back(task->cancel_requested);
                    }
                }
            }

            for (const auto &token : cancellation_tokens)
            {
                token->store(true, std::memory_order_relaxed);
            }
        }

        std::shared_ptr<std::recursive_mutex> critical_section_mutex(const std::string &name)
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            auto &critical_section = concurrency_state->critical_sections[name];
            if (critical_section == nullptr)
            {
                critical_section = std::make_shared<std::recursive_mutex>();
            }
            return critical_section;
        }

        std::string critical_section_blocking_operation_message(const std::string &operation,
                                                                const std::string &section_name) const
        {
            return runtime_text("Runtime.Prg.CriticalSection.Error.BlockingOperation",
                                {{"operation", operation}, {"section", section_name}});
        }

        std::string critical_section_enter_order_message(const std::string &held_section,
                                                         const std::string &requested_section) const
        {
            return runtime_text("Runtime.Prg.CriticalSection.Error.EnterAscendingOrder",
                                {{"heldSection", held_section}, {"requestedSection", requested_section}});
        }

        std::string critical_section_unknown_message(const std::string &section_name) const
        {
            return runtime_text("Runtime.Prg.CriticalSection.Error.UnknownSection", {{"section", section_name}});
        }

        std::string critical_section_exit_lifo_message(const std::string &held_section,
                                                       const std::string &requested_section) const
        {
            return runtime_text("Runtime.Prg.CriticalSection.Error.ExitLifoOrder",
                                {{"heldSection", held_section}, {"requestedSection", requested_section}});
        }

        std::string critical_section_mutex_missing_message(const std::string &section_name) const
        {
            return runtime_text("Runtime.Prg.CriticalSection.Error.MutexNotFound", {{"section", section_name}});
        }

        // Engine critical-section policy (see docs/25-engine-concurrency-policy.md):
        // - critical-section names are normalized, with an implicit "default" section when omitted
        // - nested acquires across different section names must follow ascending normalized-name order
        // - while any critical section is held, runtime surfaces must not block on external progress,
        //   time, or lock contention; new wait/retry paths must call this helper before blocking
        // These rules keep Copperfin's in-memory coordination deterministic and prevent deadlock-prone
        // interactions between spawned workers, waits, and lock-retry backoff behavior.
        bool ensure_non_blocking_critical_section_policy(const std::string &operation,
                                                         const SourceLocation &location,
                                                         const std::string &detail = {})
        {
            if (critical_section_stack.empty())
            {
                return true;
            }

            const std::string section_name = critical_section_stack.back();
            last_error_message = critical_section_blocking_operation_message(operation, section_name);
            std::string event_detail = "operation=" + operation + " section=" + section_name;
            if (!detail.empty())
            {
                event_detail += " detail=" + detail;
            }
            events.push_back({.category = "runtime.critical.blocking_violation",
                              .detail = event_detail,
                              .location = location});
            return false;
        }

        bool enter_critical_section(const std::string &name, const SourceLocation &location)
        {
            const std::string section_name = normalize_identifier(name.empty() ? std::string{"default"} : name);
            if (!critical_section_stack.empty())
            {
                const std::string &held_section = critical_section_stack.back();
                if (section_name != held_section && section_name < held_section)
                {
                    last_error_message = critical_section_enter_order_message(held_section, section_name);
                    events.push_back({.category = "runtime.critical.order_violation",
                                      .detail = "held=" + held_section + " requested=" + section_name,
                                      .location = location});
                    return false;
                }
            }

            auto mutex = critical_section_mutex(section_name);
            mutex->lock();
            critical_section_mutexes_by_name[section_name] = std::move(mutex);
            critical_section_stack.push_back(section_name);
            ++critical_section_depth_by_name[section_name];
            return true;
        }

        bool exit_critical_section(const std::string &name,
                                   const SourceLocation &location)
        {
            const std::string section_name = normalize_identifier(name.empty() ? std::string{"default"} : name);
            if (critical_section_stack.empty())
            {
                last_error_message = critical_section_unknown_message(section_name);
                return false;
            }

            if (section_name != critical_section_stack.back())
            {
                const std::string held_section = critical_section_stack.back();
                last_error_message = critical_section_exit_lifo_message(held_section, section_name);
                events.push_back({.category = "runtime.critical.order_violation",
                                  .detail = "held=" + held_section + " requested=" + section_name,
                                  .location = location});
                return false;
            }

            auto depth_found = critical_section_depth_by_name.find(section_name);
            if (depth_found == critical_section_depth_by_name.end() || depth_found->second == 0U)
            {
                last_error_message = critical_section_unknown_message(section_name);
                return false;
            }

            auto mutex_found = critical_section_mutexes_by_name.find(section_name);
            if (mutex_found == critical_section_mutexes_by_name.end() || mutex_found->second == nullptr)
            {
                last_error_message = critical_section_mutex_missing_message(section_name);
                return false;
            }

            mutex_found->second->unlock();
            if (--depth_found->second == 0U)
            {
                critical_section_depth_by_name.erase(depth_found);
                critical_section_mutexes_by_name.erase(mutex_found);
            }

            critical_section_stack.pop_back();
            return true;
        }

        void release_all_critical_sections()
        {
            while (!critical_section_stack.empty())
            {
                const std::string section_name = critical_section_stack.back();
                auto depth_found = critical_section_depth_by_name.find(section_name);
                auto mutex_found = critical_section_mutexes_by_name.find(section_name);
                if (depth_found == critical_section_depth_by_name.end() ||
                    mutex_found == critical_section_mutexes_by_name.end() ||
                    mutex_found->second == nullptr)
                {
                    critical_section_stack.pop_back();
                    continue;
                }

                mutex_found->second->unlock();
                critical_section_stack.pop_back();
                if (--depth_found->second == 0U)
                {
                    critical_section_depth_by_name.erase(depth_found);
                    critical_section_mutexes_by_name.erase(mutex_found);
                }
            }
        }
