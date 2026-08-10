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

        long long allocate_async_task_handle()
        {
            // VFP numeric values represent every integer through 2^53 exactly.
            // Never wrap or recycle within a runtime session: stale handles must
            // remain unable to observe or cancel a later task.
            constexpr long long maximum_exact_prg_integer = 9007199254740991LL;
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            long long &next_handle =
                concurrency_state->next_async_task_handle_by_session[current_data_session];
            next_handle = std::max(1LL, next_handle);
            if (next_handle > maximum_exact_prg_integer)
            {
                throw std::runtime_error("Copperfin task handle space exhausted");
            }
            return next_handle++;
        }

        void register_async_task(const std::shared_ptr<AsyncTaskState> &task)
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            concurrency_state->async_tasks_by_session[current_data_session][task->handle] = task;
        }

        std::shared_ptr<AsyncTaskState> find_async_task(long long handle)
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

        void erase_async_task(long long handle)
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

        bool refresh_async_task_completion(const std::shared_ptr<AsyncTaskState> &task)
        {
            if (task == nullptr)
            {
                return false;
            }

            // Multiple spawned supervisors can poll the same handle. Publish
            // the future's result exactly once and establish a happens-before
            // edge before any caller reads the immutable completion record.
            std::lock_guard<std::mutex> completion_lock(task->completion_mutex);
            if (task->finished)
            {
                return true;
            }
            if (!task->future.valid() ||
                task->future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
            {
                return false;
            }
            task->result = task->future.get();
            task->finished = true;
            return true;
        }

        static const char *runtime_polyglot_status_name(
            RuntimePolyglotDispatchStatus status) noexcept
        {
            switch (status)
            {
            case RuntimePolyglotDispatchStatus::success:
                return "success";
            case RuntimePolyglotDispatchStatus::invalid_request:
                return "invalid-request";
            case RuntimePolyglotDispatchStatus::native_failed:
                return "native-failed";
            case RuntimePolyglotDispatchStatus::candidate_failed:
                return "candidate-failed";
            case RuntimePolyglotDispatchStatus::cancelled:
                return "cancelled";
            case RuntimePolyglotDispatchStatus::parity_failed:
                return "parity-failed";
            }
            return nullptr;
        }

        static const char *runtime_polyglot_authority_name(
            RuntimePolyglotDispatchAuthority authority) noexcept
        {
            switch (authority)
            {
            case RuntimePolyglotDispatchAuthority::none:
                return "none";
            case RuntimePolyglotDispatchAuthority::native:
                return "native";
            case RuntimePolyglotDispatchAuthority::candidate:
                return "candidate";
            }
            return nullptr;
        }

        static const char *runtime_polyglot_selection_name(
            RuntimePolyglotDispatchSelection selection) noexcept
        {
            switch (selection)
            {
            case RuntimePolyglotDispatchSelection::none:
                return "none";
            case RuntimePolyglotDispatchSelection::native:
                return "native";
            case RuntimePolyglotDispatchSelection::shadow:
                return "shadow";
            case RuntimePolyglotDispatchSelection::candidate:
                return "candidate";
            }
            return nullptr;
        }

        static bool canonical_polyglot_machine_code(const std::string &value)
        {
            if (value.empty() || value.size() > 160U ||
                value.front() < 'a' || value.front() > 'z')
            {
                return false;
            }
            return std::all_of(value.begin(), value.end(), [](unsigned char ch)
            {
                return (ch >= 'a' && ch <= 'z') ||
                       (ch >= '0' && ch <= '9') ||
                       ch == '.' || ch == '_' || ch == '-';
            });
        }

        static bool canonical_polyglot_capability_id(const std::string &value)
        {
            if (value.empty() || value.front() < 'a' || value.front() > 'z')
            {
                return false;
            }
            return std::all_of(value.begin() + 1, value.end(), [](unsigned char ch)
            {
                return (ch >= 'a' && ch <= 'z') ||
                       (ch >= '0' && ch <= '9') ||
                       ch == '.' || ch == '_' || ch == '-';
            });
        }

        static bool valid_runtime_polyglot_result(
            const RuntimePolyglotDispatchResult &result)
        {
            if (runtime_polyglot_status_name(result.status) == nullptr ||
                runtime_polyglot_authority_name(result.authority) == nullptr ||
                runtime_polyglot_selection_name(result.selection) == nullptr ||
                !canonical_polyglot_machine_code(result.error_code) ||
                result.native_invocation_count > 1U ||
                result.candidate_invocation_count > 1U)
            {
                return false;
            }

            if (result.selection == RuntimePolyglotDispatchSelection::none)
            {
                if (result.authority != RuntimePolyglotDispatchAuthority::none ||
                    result.native_invocation_count != 0U ||
                    result.candidate_invocation_count != 0U ||
                    result.native_fallback_executed)
                {
                    return false;
                }
            }
            else if (result.selection == RuntimePolyglotDispatchSelection::native)
            {
                if (result.authority != RuntimePolyglotDispatchAuthority::native ||
                    result.native_invocation_count != 1U ||
                    result.candidate_invocation_count != 0U ||
                    result.native_fallback_executed)
                {
                    return false;
                }
            }
            else if (result.selection == RuntimePolyglotDispatchSelection::shadow)
            {
                if (result.authority != RuntimePolyglotDispatchAuthority::native ||
                    result.native_invocation_count != 1U ||
                    result.candidate_invocation_count != 1U ||
                    result.native_fallback_executed)
                {
                    return false;
                }
            }
            else
            {
                const bool candidate_authoritative =
                    result.authority == RuntimePolyglotDispatchAuthority::candidate &&
                    result.native_invocation_count == 0U &&
                    !result.native_fallback_executed;
                const bool native_fallback_authoritative =
                    result.authority == RuntimePolyglotDispatchAuthority::native &&
                    result.native_invocation_count == 1U &&
                    result.native_fallback_executed;
                if (result.candidate_invocation_count != 1U ||
                    (!candidate_authoritative && !native_fallback_authoritative))
                {
                    return false;
                }
            }

            const bool native_primary =
                result.selection == RuntimePolyglotDispatchSelection::native;
            const bool shadow =
                result.selection == RuntimePolyglotDispatchSelection::shadow;
            const bool candidate_primary =
                result.selection == RuntimePolyglotDispatchSelection::candidate;
            const bool native_fallback = candidate_primary &&
                result.authority == RuntimePolyglotDispatchAuthority::native &&
                result.native_fallback_executed;
            switch (result.status)
            {
            case RuntimePolyglotDispatchStatus::success:
                if (result.authority == RuntimePolyglotDispatchAuthority::none)
                {
                    return false;
                }
                break;
            case RuntimePolyglotDispatchStatus::invalid_request:
                if (result.selection != RuntimePolyglotDispatchSelection::none)
                {
                    return false;
                }
                break;
            case RuntimePolyglotDispatchStatus::native_failed:
                if (!native_primary && !native_fallback)
                {
                    return false;
                }
                break;
            case RuntimePolyglotDispatchStatus::candidate_failed:
            case RuntimePolyglotDispatchStatus::cancelled:
                if (!candidate_primary ||
                    result.authority != RuntimePolyglotDispatchAuthority::candidate ||
                    result.native_fallback_executed)
                {
                    return false;
                }
                break;
            case RuntimePolyglotDispatchStatus::parity_failed:
                if (!shadow)
                {
                    return false;
                }
                break;
            default:
                return false;
            }
            if (!result.payload_json.empty())
            {
                const auto payload = copperfin::platform::select_json_value(result.payload_json);
                if (!payload.ok())
                {
                    return false;
                }
            }
            return true;
        }

        static std::string runtime_polyglot_dispatch_document(
            const std::string &capability_id,
            const std::string &status,
            const std::string &error_code,
            const RuntimePolyglotDispatchResult *result = nullptr)
        {
            const char *authority = result == nullptr
                                        ? "none"
                                        : runtime_polyglot_authority_name(result->authority);
            const char *selection = result == nullptr
                                        ? "none"
                                        : runtime_polyglot_selection_name(result->selection);
            const std::uint32_t native_count = result == nullptr
                                                   ? 0U
                                                   : result->native_invocation_count;
            const std::uint32_t candidate_count = result == nullptr
                                                      ? 0U
                                                      : result->candidate_invocation_count;
            const bool fallback_executed = result != nullptr &&
                                           result->native_fallback_executed;
            const std::string_view payload = result == nullptr
                                                 ? std::string_view{}
                                                 : std::string_view(result->payload_json);

            std::ostringstream document;
            document << "{\"schema_version\":1"
                     << ",\"status\":\""
                     << copperfin::platform::json_escape_string(status)
                     << "\",\"error_code\":\""
                     << copperfin::platform::json_escape_string(error_code)
                     << "\",\"capability_id\":\""
                     << copperfin::platform::json_escape_string(capability_id)
                     << "\",\"authority\":\"" << authority
                     << "\",\"route_selection\":\"" << selection
                     << "\",\"native_invocation_count\":" << native_count
                     << ",\"candidate_invocation_count\":" << candidate_count
                     << ",\"native_fallback_executed\":"
                     << (fallback_executed ? "true" : "false")
                     << ",\"payload\":"
                     << (payload.empty() ? "null" : payload)
                     << '}';
            return document.str();
        }

        std::optional<PrgValue> polyglot_dispatch_function(
            const std::string &function,
            const std::vector<PrgValue> &arguments)
        {
            if (function != "cfpolyglotdispatch")
            {
                return std::nullopt;
            }

            constexpr std::size_t maximum_result_document_bytes = 1024U * 1024U;
            auto fail = [&](const std::string &capability_id,
                            const std::string &status,
                            const std::string &error_code)
            {
                events.push_back({.category = "runtime.polyglot.dispatch",
                                  .detail = "capability=" + capability_id +
                                            " status=" + status +
                                            " reason=" + error_code,
                                  .location = current_statement() == nullptr
                                                  ? SourceLocation{}
                                                  : current_statement()->location});
                return make_string_value(runtime_polyglot_dispatch_document(
                    capability_id, status, error_code));
            };

            if (arguments.size() < 2U || arguments.size() > 3U ||
                arguments[0].kind != PrgValueKind::string ||
                arguments[1].kind != PrgValueKind::string)
            {
                return fail({}, "invalid-request", "polyglot.prg.invalid_arguments");
            }

            const std::string capability_id = arguments[0].string_value;
            if (!canonical_polyglot_capability_id(capability_id))
            {
                return fail({}, "invalid-request", "polyglot.prg.invalid_capability_id");
            }

            const auto input = copperfin::platform::select_json_value(
                arguments[1].string_value);
            if (!input.ok() || input.kind != copperfin::platform::JsonValueKind::object)
            {
                return fail(capability_id, "invalid-request", "polyglot.prg.invalid_arguments_json");
            }

            std::uint8_t selection_sample = 0U;
            if (arguments.size() == 3U)
            {
                long double sample = -1.0L;
                switch (arguments[2].kind)
                {
                case PrgValueKind::number:
                    sample = arguments[2].number_value;
                    break;
                case PrgValueKind::int64:
                    sample = static_cast<long double>(arguments[2].int64_value);
                    break;
                case PrgValueKind::uint64:
                    sample = static_cast<long double>(arguments[2].uint64_value);
                    break;
                default:
                    break;
                }
                if (!std::isfinite(sample) || std::trunc(sample) != sample ||
                    sample < 0.0L || sample > 99.0L)
                {
                    return fail(capability_id, "invalid-request", "polyglot.prg.invalid_selection_sample");
                }
                selection_sample = static_cast<std::uint8_t>(sample);
            }

            const SourceLocation location = current_statement() == nullptr
                                                ? SourceLocation{}
                                                : current_statement()->location;
            if (!ensure_non_blocking_critical_section_policy(
                    "CFPOLYGLOTDISPATCH", location))
            {
                return fail(capability_id, "invalid-request", "polyglot.prg.blocked_in_critical_section");
            }
            if (!options.polyglot_dispatch_callback)
            {
                return fail(capability_id, "unavailable", "polyglot.prg.dispatch_unavailable");
            }

            RuntimePolyglotDispatchResult result;
            try
            {
                const auto cancellation_token = task_cancel_requested;
                result = options.polyglot_dispatch_callback(
                    RuntimePolyglotDispatchRequest{
                        capability_id,
                        arguments[1].string_value,
                        selection_sample,
                        [cancellation_token]()
                        {
                            return cancellation_token != nullptr &&
                                   cancellation_token->load(std::memory_order_relaxed);
                        }});
            }
            catch (const std::exception &)
            {
                return fail(capability_id, "host-failed", "polyglot.prg.dispatch_exception");
            }
            catch (...)
            {
                return fail(capability_id, "host-failed", "polyglot.prg.dispatch_exception");
            }

            if (!valid_runtime_polyglot_result(result))
            {
                return fail(capability_id, "host-failed", "polyglot.prg.invalid_host_result");
            }
            const std::string status = runtime_polyglot_status_name(result.status);
            std::string document = runtime_polyglot_dispatch_document(
                capability_id, status, result.error_code, &result);
            if (document.size() > maximum_result_document_bytes)
            {
                return fail(capability_id, "host-failed", "polyglot.prg.result_too_large");
            }
            events.push_back({.category = "runtime.polyglot.dispatch",
                              .detail = "capability=" + capability_id +
                                        " status=" + status +
                                        " reason=" + result.error_code,
                              .location = location});
            return make_string_value(std::move(document));
        }

        std::optional<PrgValue> async_task_control_function(
            const std::string &function,
            const std::vector<PrgValue> &arguments)
        {
            if (const auto polyglot_result = polyglot_dispatch_function(function, arguments))
            {
                return polyglot_result;
            }
            if (function != "cftaskstatus" &&
                function != "cftaskcancel" &&
                function != "cftaskresult" &&
                function != "cftaskoutput")
            {
                return std::nullopt;
            }
            if (arguments.size() != 1U)
            {
                if (function == "cftaskstatus")
                {
                    return make_string_value("unknown");
                }
                if (function == "cftaskcancel")
                {
                    return make_boolean_value(false);
                }
                return make_empty_value();
            }

            const long long handle = std::llround(value_as_number(arguments.front()));
            const std::shared_ptr<AsyncTaskState> task = find_async_task(handle);
            if (task == nullptr)
            {
                if (function == "cftaskstatus")
                {
                    return make_string_value("unknown");
                }
                if (function == "cftaskcancel")
                {
                    return make_boolean_value(false);
                }
                return make_empty_value();
            }

            const bool finished = refresh_async_task_completion(task);
            if (function == "cftaskstatus")
            {
                if (finished)
                {
                    return make_string_value(debug_pause_reason_name(task->result.reason));
                }
                if (task->cancel_requested != nullptr &&
                    task->cancel_requested->load(std::memory_order_relaxed))
                {
                    return make_string_value("cancel-requested");
                }
                return make_string_value("running");
            }

            if (function == "cftaskcancel")
            {
                if (finished || task->cancel_requested == nullptr)
                {
                    return make_boolean_value(false);
                }
                task->cancel_requested->store(true, std::memory_order_relaxed);
                events.push_back({.category = "runtime.task.cancel_requested",
                                  .detail = "handle=" + std::to_string(handle),
                                  .location = current_statement() == nullptr
                                                  ? SourceLocation{}
                                                  : current_statement()->location});
                return make_boolean_value(true);
            }

            if (!finished)
            {
                return make_empty_value();
            }
            if (function == "cftaskresult")
            {
                return task->result.last_return_value.value_or(make_empty_value());
            }

            std::string output;
            for (const RuntimeEvent &event : task->result.events)
            {
                if (event.category != "runtime.print")
                {
                    continue;
                }
                if (!output.empty())
                {
                    output.push_back('\n');
                }
                output += event.detail;
            }
            return make_string_value(std::move(output));
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
