// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

        std::vector<std::filesystem::path> transaction_companion_paths(const std::string &table_path) const
        {
            std::vector<std::filesystem::path> paths;
            const std::filesystem::path source = copperfin::platform::path_from_utf8_string(
                normalize_path(table_path)).lexically_normal();
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

            const std::string source_stem = copperfin::platform::path_to_utf8_string(source.stem());
            push_if_unique(source.parent_path() / (source_stem + ".fpt"));
            push_if_unique(source.parent_path() / (source_stem + ".cdx"));
            return paths;
        }

        bool replay_transaction_journal_state(const TransactionJournalState &state)
        {
            bool ok = true;
            for (const auto &[key, entry] : state.tracked_files)
            {
                bool entry_ok = true;
                const std::filesystem::path original = copperfin::platform::path_from_utf8_string(entry.original_path);
                if (entry.existed_at_start)
                {
                    if (entry.backup_path.empty())
                    {
                        ok = false;
                        continue;
                    }

                    const std::filesystem::path backup = copperfin::platform::path_from_utf8_string(entry.backup_path);
                    std::error_code backup_exists_error;
                    if (!std::filesystem::exists(backup, backup_exists_error) || backup_exists_error)
                    {
                        ok = false;
                        continue;
                    }

                    std::error_code copy_error;
                    if (!original.parent_path().empty())
                    {
                        std::filesystem::create_directories(original.parent_path(), copy_error);
                    }
                    if (!copy_error)
                    {
                        std::filesystem::copy_file(
                            backup, original, std::filesystem::copy_options::overwrite_existing, copy_error);
                    }
                    if (copy_error)
                    {
                        ok = false;
                        entry_ok = false;
                    }
                }
                else
                {
                    std::error_code exists_error;
                    const bool original_exists = std::filesystem::exists(original, exists_error);
                    if (exists_error)
                    {
                        ok = false;
                        continue;
                    }
                    if (original_exists)
                    {
                        std::error_code remove_error;
                        if (!std::filesystem::remove(original, remove_error) || remove_error)
                        {
                            ok = false;
                            entry_ok = false;
                        }
                    }
                }

                (void)entry_ok;
                if (entry.acquired_exclusive_lock)
                {
                    // #6451: release the resource lock this transaction has
                    // held (blocking any other runtime instance's writes to
                    // it) since its first write, whether or not this
                    // specific companion file's replay succeeded.
                    release_transaction_resource_lock(key);
                }
            }

            // Preserve the journal and backups when replay fails.  A caller
            // must not publish its matching verified-byte admission snapshot
            // until this physical restoration succeeds, and a retained
            // journal remains the recovery authority for a later attempt.
            if (ok)
            {
                std::error_code ignored;
                std::filesystem::remove_all(state.root_path, ignored);
            }
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

            bool saw_version = false;
            bool saw_level = false;
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
                if (tokens[0] == "VERSION")
                {
                    if (saw_version || tokens.size() != 2U || tokens[1] != "1")
                    {
                        return false;
                    }
                    saw_version = true;
                    continue;
                }
                if (tokens[0] == "LEVEL")
                {
                    if (saw_level || tokens.size() != 2U)
                    {
                        return false;
                    }
                    const auto parsed_level = copperfin::platform::try_parse_invariant_integer<int>(tokens[1]);
                    if (!parsed_level.has_value() || *parsed_level < 0)
                    {
                        return false;
                    }
                    state.level = *parsed_level;
                    saw_level = true;
                    continue;
                }
                if (tokens[0] == "FILE")
                {
                    if (tokens.size() != 4U || (tokens[2] != "0" && tokens[2] != "1"))
                    {
                        return false;
                    }
                    TransactionJournalFileEntry entry;
                    entry.original_path = tokens[1];
                    entry.existed_at_start = tokens[2] == "1";
                    entry.backup_path = tokens[3];
                    state.tracked_files[normalize_path(entry.original_path)] = std::move(entry);
                }
            }

            return saw_version && saw_level;
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
                                      .detail = copperfin::platform::path_to_utf8_string(journal_path),
                                      .location = {}});
                }
            }
        }

        TransactionJournalState &current_transaction_journal()
        {
            auto [iterator, _] = transaction_journal_by_session.try_emplace(current_data_session);
            return iterator->second;
        }

        std::string transaction_journal_initialize_message() const
        {
            return runtime_text("Runtime.Prg.Transaction.Error.JournalInitializeFailed");
        }

        std::string transaction_journal_persist_state_message() const
        {
            return runtime_text("Runtime.Prg.Transaction.Error.JournalStatePersistFailed");
        }

        std::string transaction_backup_message(const std::string &path) const
        {
            return runtime_text("Runtime.Prg.Transaction.Error.BackupCreateFailed", {{"path", path}});
        }

        std::string transaction_backup_journal_persist_message() const
        {
            return runtime_text("Runtime.Prg.Transaction.Error.BackupJournalPersistFailed");
        }

        std::string transaction_journal_replay_message() const
        {
            return runtime_text("Runtime.Prg.Transaction.Error.JournalReplayFailed");
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
            static std::atomic<unsigned long long> transaction_nonce_counter{0ULL};
            const auto now_ticks = static_cast<unsigned long long>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count());
            const unsigned long long nonce_counter = transaction_nonce_counter.fetch_add(1ULL, std::memory_order_relaxed);
            const std::string nonce = std::to_string(now_ticks) +
                                      "_" + std::to_string(process_id) +
                                      "_" + std::to_string(static_cast<unsigned long long>(current_data_session)) +
                                      "_" + std::to_string(nonce_counter);
            journal.root_path = transaction_journal_root_directory() / ("txn_" + nonce);
            journal.journal_path = journal.root_path / "journal.log";
            journal.level = current_transaction_level();
            if (!write_transaction_journal_file(journal))
            {
                last_error_message = transaction_journal_initialize_message();
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
                last_error_message = transaction_journal_persist_state_message();
                return false;
            }
            return true;
        }

        // #6451: acquires an exclusive, resource-scoped lock in the same
        // shared table_lock_owner_by_resource/record_lock_owner_by_resource
        // maps FLOCK()/RLOCK()/the implicit per-write locks already use, so
        // it correctly conflicts with (and is conflicted by) any of them
        // from any other runtime instance -- including a SPAWN child with no
        // transaction of its own, since REPLACE/APPEND/etc. always take an
        // implicit record or table lock for the physical write regardless of
        // transaction state. Returns false (with a runtime.lock_timeout
        // event, matching FLOCK()'s own contended-timeout behavior) if
        // another runtime instance already holds a conflicting lock.
        // `newly_acquired` is false when this runtime instance already owned
        // the resource lock (e.g. an explicit prior FLOCK()), so the caller
        // does not release a lock it did not itself acquire.
        bool acquire_transaction_resource_lock(const std::string &resource_key, bool &newly_acquired)
        {
            const ReprocessPolicy policy = current_reprocess_policy();
            const std::string owner_key = current_lock_owner_key();
            const SourceLocation location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location;
            newly_acquired = false;

            for (std::size_t attempt = 0U;; ++attempt)
            {
                {
                    std::lock_guard<std::mutex> lock(concurrency_state->mutex);
                    const auto table_owner_found = concurrency_state->table_lock_owner_by_resource.find(resource_key);
                    bool other_record_lock_present = false;
                    const auto shared_record_found = concurrency_state->record_lock_owner_by_resource.find(resource_key);
                    if (shared_record_found != concurrency_state->record_lock_owner_by_resource.end())
                    {
                        for (const auto &[_, record_owner] : shared_record_found->second)
                        {
                            if (record_owner != owner_key)
                            {
                                other_record_lock_present = true;
                                break;
                            }
                        }
                    }
                    const bool table_conflict = table_owner_found != concurrency_state->table_lock_owner_by_resource.end() &&
                                                table_owner_found->second != owner_key;
                    if (!table_conflict && !other_record_lock_present)
                    {
                        newly_acquired = table_owner_found == concurrency_state->table_lock_owner_by_resource.end();
                        concurrency_state->table_lock_owner_by_resource[resource_key] = owner_key;
                        return true;
                    }
                }

                if (attempt >= policy.retry_budget)
                {
                    events.push_back({.category = "runtime.lock_timeout",
                                      .detail = "BEGIN TRANSACTION timeout reprocess=" + policy.display_value,
                                      .location = location});
                    return false;
                }

                if (!pause_for_lock_retry("BEGIN TRANSACTION reprocess=" + policy.display_value, location, attempt + 1U))
                {
                    return false;
                }
            }
        }

        void release_transaction_resource_lock(const std::string &resource_key)
        {
            std::lock_guard<std::mutex> lock(concurrency_state->mutex);
            const auto found = concurrency_state->table_lock_owner_by_resource.find(resource_key);
            if (found != concurrency_state->table_lock_owner_by_resource.end() &&
                found->second == current_lock_owner_key())
            {
                concurrency_state->table_lock_owner_by_resource.erase(found);
            }
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
            snapshot_verified_file_byte_overrides_for_table(journal, table_path);
            const std::string primary_resource_key = normalize_path(table_path);
            std::error_code ignored;
            for (const auto &path : transaction_companion_paths(table_path))
            {
                const std::string key = normalize_path(copperfin::platform::path_to_utf8_string(path));
                if (journal.tracked_files.contains(key))
                {
                    continue;
                }

                // #6451: this is this transaction's first touch of this
                // exact resource; acquire the resource lock now so that no
                // other runtime instance can write to it (and silently be
                // clobbered, or clobber this transaction's own change) for
                // as long as this transaction's backup over it stays
                // outstanding. Locks are keyed by the primary table path
                // only (matching cursor_lock_resource_key()), not per
                // companion file, so only that one key is meaningful here.
                bool acquired_exclusive_lock = false;
                if (key == primary_resource_key)
                {
                    if (!acquire_transaction_resource_lock(key, acquired_exclusive_lock))
                    {
                        last_error_message = runtime_text(
                            "Runtime.Prg.Transaction.Error.BackupLockTimeout", {{"path", key}});
                        return false;
                    }
                }

                TransactionJournalFileEntry entry;
                entry.original_path = key;
                entry.existed_at_start = std::filesystem::exists(path, ignored);
                entry.acquired_exclusive_lock = acquired_exclusive_lock;
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
                        last_error_message = transaction_backup_message(key);
                        if (acquired_exclusive_lock)
                        {
                            release_transaction_resource_lock(key);
                        }
                        return false;
                    }
                    entry.backup_path = copperfin::platform::path_to_utf8_string(backup_path);
                }

                journal.tracked_files.emplace(key, std::move(entry));
                if (!write_transaction_journal_file(journal))
                {
                    last_error_message = transaction_backup_journal_persist_message();
                    if (acquired_exclusive_lock)
                    {
                        release_transaction_resource_lock(key);
                    }
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

                // A transaction replay restores the journaled admission view;
                // cursor-private overlays must not reapply the rolled-back
                // commit over that restored view.
                cursor.verified_committed_records.clear();

                const auto table_result = parse_cursor_table(cursor, std::max<std::size_t>(cursor.record_count, 1U));
                if (!table_result.ok)
                {
                    closed_areas.push_back(area);
                    continue;
                }

                cursor.record_count = table_result.table.header.record_count;
                cursor.local_fields = visible_cursor_fields(table_result.table.fields);
                cursor.field_count = cursor.local_fields.size();
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
                if (const auto cursor_found = session.cursors.find(area); cursor_found != session.cursors.end())
                {
                    release_shared_lock_ownership_for_cursor(cursor_found->second, session, current_data_session);
                    unregister_open_cursor_alias(session, area, cursor_found->second.alias);
                }
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
                last_error_message = transaction_journal_replay_message();
                return false;
            }

            // Publish the owned admission only after the matching physical
            // DBF/FPT rollback succeeds, before the cursor reparse below.
            restore_verified_file_byte_overrides(found->second);
            transaction_journal_by_session.erase(found);
            refresh_local_cursors_after_transaction_replay();
            return true;
        }

        // #6263: an uncommitted BEGIN TRANSACTION must not leave its backed-up
        // rows live on disk when the session ends without END TRANSACTION or
        // ROLLBACK, across every data session, not just the currently
        // selected one. Cursor refresh is skipped here (unlike
        // rollback_active_transaction_journal()): every cursor is discarded
        // by this same shutdown pass right after, so there is nothing left
        // to reparse against the restored bytes. A failed replay leaves the
        // physical journal/backups in place -- replay_transaction_journal_state's
        // own contract -- so a future session that happens to scan this temp
        // directory can still recover it via replay_pending_transaction_journals().
        void rollback_all_pending_transaction_journals()
        {
            for (auto it = transaction_journal_by_session.begin(); it != transaction_journal_by_session.end();)
            {
                TransactionJournalState state = std::move(it->second);
                it = transaction_journal_by_session.erase(it);
                if (replay_transaction_journal_state(state))
                {
                    restore_verified_file_byte_overrides(state);
                }
            }
        }

        void commit_active_transaction_journal()
        {
            auto found = transaction_journal_by_session.find(current_data_session);
            if (found == transaction_journal_by_session.end())
            {
                return;
            }

            // #6451: a commit accepts the current on-disk state; release
            // the resource lock this transaction has held (serializing out
            // any other runtime instance's writes) since its first write.
            for (const auto &[key, entry] : found->second.tracked_files)
            {
                if (entry.acquired_exclusive_lock)
                {
                    release_transaction_resource_lock(key);
                }
            }

            std::error_code ignored;
            std::filesystem::remove_all(found->second.root_path, ignored);
            transaction_journal_by_session.erase(found);
        }
