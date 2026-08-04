// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
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
            std::error_code ignored;
            for (const auto &[_, entry] : state.tracked_files)
            {
                const std::filesystem::path original = copperfin::platform::path_from_utf8_string(entry.original_path);
                if (entry.existed_at_start)
                {
                    if (!entry.backup_path.empty())
                    {
                        const std::filesystem::path backup = copperfin::platform::path_from_utf8_string(entry.backup_path);
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
                        last_error_message = transaction_backup_message(key);
                        return false;
                    }
                    entry.backup_path = copperfin::platform::path_to_utf8_string(backup_path);
                }

                journal.tracked_files.emplace(key, std::move(entry));
                if (!write_transaction_journal_file(journal))
                {
                    last_error_message = transaction_backup_journal_persist_message();
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

                const auto table_result = parse_cursor_table(cursor, std::max<std::size_t>(cursor.record_count, 1U));
                if (!table_result.ok)
                {
                    closed_areas.push_back(area);
                    continue;
                }

                cursor.record_count = table_result.table.header.record_count;
                cursor.field_count = table_result.table.fields.size();
                cursor.record_length = table_result.table.header.record_length;
                cursor.local_fields = table_result.table.fields;
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
