// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

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

        std::string portable_database_path_text(std::string value) const
        {
#if !defined(_WIN32)
            std::replace(value.begin(), value.end(), '\\', '/');
#endif
            return value;
        }

        std::optional<std::filesystem::path> resolve_existing_database_component(
            const std::filesystem::path &candidate) const
        {
            std::error_code ignored;
            const bool candidate_exists =
                std::filesystem::is_regular_file(candidate, ignored) && !ignored;
            ignored.clear();

            const std::filesystem::path parent =
                candidate.parent_path().empty() ? std::filesystem::path{"."} : candidate.parent_path();
            if (!std::filesystem::is_directory(parent, ignored) || ignored)
            {
                return candidate_exists
                    ? std::optional<std::filesystem::path>(candidate.lexically_normal())
                    : std::nullopt;
            }

            const std::string requested_name = copperfin::platform::path_to_utf8_string(candidate.filename());
            const std::string expected_name = lowercase_copy(requested_name);
            const std::string expected_stem = copperfin::platform::path_to_utf8_string(candidate.stem());
            std::optional<std::filesystem::path> exact_stem_match;
            std::optional<std::filesystem::path> folded_match;
            bool folded_match_ambiguous = false;
            std::filesystem::directory_iterator iterator(parent, ignored);
            const std::filesystem::directory_iterator end;
            for (; iterator != end && !ignored; iterator.increment(ignored))
            {
                const std::filesystem::path entry_path = iterator->path();
                const std::string entry_name = copperfin::platform::path_to_utf8_string(entry_path.filename());
                if (lowercase_copy(entry_name) != expected_name)
                {
                    continue;
                }
                std::error_code type_error;
                if (!iterator->is_regular_file(type_error) || type_error)
                {
                    continue;
                }
                if (entry_name == requested_name)
                {
                    return entry_path.lexically_normal();
                }
                if (copperfin::platform::path_to_utf8_string(entry_path.stem()) == expected_stem)
                {
                    if (exact_stem_match.has_value())
                    {
                        return std::nullopt;
                    }
                    exact_stem_match = entry_path.lexically_normal();
                    continue;
                }
                folded_match_ambiguous = folded_match.has_value();
                folded_match = entry_path.lexically_normal();
            }
            if (ignored)
            {
                return candidate_exists
                    ? std::optional<std::filesystem::path>(candidate.lexically_normal())
                    : std::nullopt;
            }
            if (exact_stem_match.has_value())
            {
                return exact_stem_match;
            }
            return folded_match_ambiguous ? std::nullopt : folded_match;
        }

        std::vector<std::filesystem::path> database_search_directories() const
        {
            std::vector<std::filesystem::path> directories{
                copperfin::platform::path_from_utf8_string(current_default_directory())};
            const auto found_path = current_set_state().find("path");
            if (found_path == current_set_state().end())
            {
                return directories;
            }

            std::string remaining = found_path->second;
            std::replace(remaining.begin(), remaining.end(), ';', ',');
            while (!remaining.empty())
            {
                const std::size_t comma = remaining.find(',');
                std::string entry = trim_copy(
                    comma == std::string::npos ? remaining : remaining.substr(0U, comma));
                entry = portable_database_path_text(unquote_string(entry));
                if (!entry.empty())
                {
                    std::filesystem::path directory = copperfin::platform::path_from_utf8_string(entry);
                    if (directory.is_relative())
                    {
                        directory = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                            directory;
                    }
                    directories.push_back(directory.lexically_normal());
                }
                if (comma == std::string::npos)
                {
                    break;
                }
                remaining = remaining.substr(comma + 1U);
            }
            return directories;
        }

        std::optional<std::filesystem::path> resolve_database_path(const std::string &raw_path) const
        {
            std::string path_text = portable_database_path_text(unquote_string(trim_copy(raw_path)));
            if (path_text.empty() || path_text == "?")
            {
                return std::nullopt;
            }

            std::filesystem::path requested(path_text);
            if (requested.extension().empty())
            {
                requested += ".dbc";
            }
            if (requested.is_absolute())
            {
                return resolve_existing_database_component(requested.lexically_normal());
            }

            for (const auto &directory : database_search_directories())
            {
                if (const auto resolved = resolve_existing_database_component(
                        (directory / requested).lexically_normal()))
                {
                    return resolved;
                }
            }
            return std::nullopt;
        }

        std::map<std::string, std::string>::const_iterator find_verified_file_byte_override(
            const std::filesystem::path &path) const
        {
            const std::string normalized = copperfin::platform::path_to_utf8_string(path.lexically_normal());
            if (const auto exact = options.verified_file_byte_overrides.find(normalized);
                exact != options.verified_file_byte_overrides.end())
            {
                return exact;
            }
            auto matching = options.verified_file_byte_overrides.end();
            for (auto candidate = options.verified_file_byte_overrides.begin();
                 candidate != options.verified_file_byte_overrides.end();
                 ++candidate)
            {
                if (!paths_equal_insensitive(candidate->first, normalized))
                {
                    continue;
                }
                if (matching != options.verified_file_byte_overrides.end())
                {
                    return options.verified_file_byte_overrides.end();
                }
                matching = candidate;
            }
            return matching;
        }

        std::map<std::string, std::string>::const_iterator find_verified_database_file_byte_override(
            const std::filesystem::path &path) const
        {
            const std::string normalized = copperfin::platform::path_to_utf8_string(path.lexically_normal());
            if (const auto exact = options.verified_file_byte_overrides.find(normalized);
                exact != options.verified_file_byte_overrides.end())
            {
                return exact;
            }
#if defined(_WIN32)
            auto matching = options.verified_file_byte_overrides.end();
            for (auto candidate = options.verified_file_byte_overrides.begin();
                 candidate != options.verified_file_byte_overrides.end();
                 ++candidate)
            {
                if (!paths_equal_insensitive(candidate->first, normalized))
                {
                    continue;
                }
                if (matching != options.verified_file_byte_overrides.end())
                {
                    return options.verified_file_byte_overrides.end();
                }
                matching = candidate;
            }
            return matching;
#else
            return options.verified_file_byte_overrides.end();
#endif
        }

        bool database_paths_equal(const std::string &left, const std::string &right) const
        {
#if defined(_WIN32)
            return paths_equal_insensitive(left, right);
#else
            return copperfin::platform::path_from_utf8_string(left).lexically_normal() ==
                copperfin::platform::path_from_utf8_string(right).lexically_normal();
#endif
        }

        RuntimeDatabaseState *find_open_database_by_path(
            DataSessionState &session,
            const std::string &path)
        {
            const auto found = std::find_if(
                session.databases.begin(),
                session.databases.end(),
                [&](const auto &database)
                {
                    return database_paths_equal(database.path, path);
                });
            return found == session.databases.end() ? nullptr : &(*found);
        }

        const RuntimeDatabaseState *find_open_database_by_path(
            const DataSessionState &session,
            const std::string &path) const
        {
            const auto found = std::find_if(
                session.databases.begin(),
                session.databases.end(),
                [&](const auto &database)
                {
                    return database_paths_equal(database.path, path);
                });
            return found == session.databases.end() ? nullptr : &(*found);
        }

        bool read_database_component_bytes(
            const std::filesystem::path &path,
            std::string &bytes)
        {
            const auto verified = find_verified_database_file_byte_override(path);
            if (verified != options.verified_file_byte_overrides.end())
            {
                bytes = verified->second;
                if (bytes.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Database.Error.ComponentMalformed",
                        {{"path", copperfin::platform::path_to_utf8_string(path)}});
                    return false;
                }
                return true;
            }
            if (options.require_verified_file_byte_overrides)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.VerifiedBytesUnavailable",
                    {{"path", copperfin::platform::path_to_utf8_string(path)}});
                return false;
            }

            std::ifstream input(path, std::ios::binary);
            if (!input.good())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentReadFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(path)}});
                return false;
            }
            std::ostringstream stream;
            stream << input.rdbuf();
            bytes = stream.str();
            if (bytes.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentMalformed",
                    {{"path", copperfin::platform::path_to_utf8_string(path)}});
                return false;
            }
            return true;
        }

        RuntimeDatabaseState *find_open_database(
            DataSessionState &session,
            const std::string &designator)
        {
            const std::string designator_text =
                portable_database_path_text(unquote_string(trim_copy(designator)));
            const std::string normalized_designator = normalize_identifier(designator_text);
            if (normalized_designator.empty())
            {
                return nullptr;
            }
            if (RuntimeDatabaseState *exact_path =
                    find_open_database_by_path(session, designator_text))
            {
                return exact_path;
            }
            const std::string designator_stem =
                normalize_identifier(portable_path_stem(normalized_designator));
            RuntimeDatabaseState *match = nullptr;
            for (auto &database : session.databases)
            {
                if (normalize_identifier(database.name) != normalized_designator &&
                    normalize_identifier(database.name) != designator_stem &&
                    normalize_identifier(portable_path_stem(database.path)) != designator_stem)
                {
                    continue;
                }
                if (match != nullptr)
                {
                    return nullptr;
                }
                match = &database;
            }
            return match;
        }

        const RuntimeDatabaseState *find_open_database(
            const DataSessionState &session,
            const std::string &designator) const
        {
            const std::string designator_text =
                portable_database_path_text(unquote_string(trim_copy(designator)));
            const std::string normalized_designator = normalize_identifier(designator_text);
            if (normalized_designator.empty())
            {
                return nullptr;
            }
            if (const RuntimeDatabaseState *exact_path =
                    find_open_database_by_path(session, designator_text))
            {
                return exact_path;
            }
            const std::string designator_stem =
                normalize_identifier(portable_path_stem(normalized_designator));
            const RuntimeDatabaseState *match = nullptr;
            for (const auto &database : session.databases)
            {
                if (normalize_identifier(database.name) != normalized_designator &&
                    normalize_identifier(database.name) != designator_stem &&
                    normalize_identifier(portable_path_stem(database.path)) != designator_stem)
                {
                    continue;
                }
                if (match != nullptr)
                {
                    return nullptr;
                }
                match = &database;
            }
            return match;
        }

        bool database_is_open(const std::string &designator, int data_session = 0) const
        {
            const int effective_session = data_session > 0 ? data_session : current_data_session;
            const auto found_session = data_sessions.find(effective_session);
            if (found_session == data_sessions.end())
            {
                return false;
            }
            if (trim_copy(designator).empty())
            {
                return !found_session->second.current_database_path.empty();
            }
            return find_open_database(found_session->second, designator) != nullptr;
        }

        std::string current_database_path() const
        {
            return current_session_state().current_database_path;
        }

        bool set_current_database(const std::string &designator)
        {
            DataSessionState &session = current_session_state();
            if (trim_copy(designator).empty())
            {
                session.current_database_path.clear();
                for (auto &database : session.databases)
                {
                    database.current = false;
                }
                return true;
            }

            RuntimeDatabaseState *database = find_open_database(session, designator);
            if (database == nullptr)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.NotOpen",
                    {{"database", unquote_string(trim_copy(designator))}});
                return false;
            }
            session.current_database_path = database->path;
            for (auto &candidate : session.databases)
            {
                candidate.current = database_paths_equal(candidate.path, database->path);
            }
            return true;
        }

        bool open_database(
            const std::string &raw_path,
            std::optional<bool> exclusive_override,
            bool read_only)
        {
            const auto database_path = resolve_database_path(raw_path);
            if (!database_path.has_value())
            {
                last_error_message = runtime_text(
                    trim_copy(raw_path).empty() || trim_copy(raw_path) == "?"
                        ? "Runtime.Prg.Database.Error.PathRequired"
                        : "Runtime.Prg.Database.Error.NotFound",
                    {{"path", unquote_string(trim_copy(raw_path))}});
                return false;
            }

            DataSessionState &session = current_session_state();
            if (RuntimeDatabaseState *existing =
                    find_open_database_by_path(
                        session, copperfin::platform::path_to_utf8_string(*database_path)))
            {
                return set_current_database(existing->path);
            }

            const bool exclusive =
                exclusive_override.value_or(is_set_enabled_or_default("exclusive", true));
            for (const auto &[session_id, candidate_session] : data_sessions)
            {
                if (session_id == current_data_session)
                {
                    continue;
                }
                const RuntimeDatabaseState *existing =
                    find_open_database_by_path(
                        candidate_session, copperfin::platform::path_to_utf8_string(*database_path));
                if (existing != nullptr && (existing->exclusive || exclusive))
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Database.Error.ExclusiveConflict",
                        {{"path", copperfin::platform::path_to_utf8_string(*database_path)}});
                    return false;
                }
            }

            std::filesystem::path dct_candidate = *database_path;
            dct_candidate.replace_extension(".dct");
            std::filesystem::path dcx_candidate = *database_path;
            dcx_candidate.replace_extension(".dcx");
            const auto dct_path = resolve_existing_database_component(dct_candidate);
            const auto dcx_path = resolve_existing_database_component(dcx_candidate);
            if (!dct_path.has_value() || !dcx_path.has_value())
            {
                const std::filesystem::path missing_path = !dct_path.has_value()
                    ? dct_candidate
                    : dcx_candidate;
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.CompanionMissing",
                    {{"path", copperfin::platform::path_to_utf8_string(missing_path)}});
                return false;
            }

            std::string dbc_bytes;
            std::string dct_bytes;
            std::string dcx_bytes;
            if (!read_database_component_bytes(*database_path, dbc_bytes) ||
                !read_database_component_bytes(*dct_path, dct_bytes) ||
                !read_database_component_bytes(*dcx_path, dcx_bytes))
            {
                return false;
            }
            const std::vector<std::uint8_t> dbc_binary(dbc_bytes.begin(), dbc_bytes.end());
            const auto header = vfp::parse_dbf_header(dbc_binary);
            if (!header.ok)
            {
                const std::string database_display_path =
                    copperfin::platform::path_to_utf8_string(*database_path);
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ContainerMalformed",
                    {{"path", database_display_path},
                     {"errorMessage", header.error}});
                return false;
            }
            const auto read_be_u16 = [](const std::string &bytes, std::size_t offset)
            {
                return static_cast<std::uint16_t>(
                    (static_cast<std::uint16_t>(static_cast<unsigned char>(bytes[offset])) << 8U) |
                    static_cast<std::uint16_t>(static_cast<unsigned char>(bytes[offset + 1U])));
            };
            if (dct_bytes.size() < 512U || read_be_u16(dct_bytes, 6U) == 0U)
            {
                const std::string dct_display_path = copperfin::platform::path_to_utf8_string(*dct_path);
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentMalformed",
                    {{"path", dct_display_path}});
                return false;
            }
            const std::vector<std::uint8_t> dcx_binary(dcx_bytes.begin(), dcx_bytes.end());
            const auto index_probe = vfp::parse_index_probe(
                dcx_binary,
                static_cast<std::uint64_t>(dcx_binary.size()),
                vfp::IndexKind::dcx);
            if (!index_probe.ok)
            {
                const std::string dcx_display_path = copperfin::platform::path_to_utf8_string(*dcx_path);
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentMalformed",
                    {{"path", dcx_display_path}});
                return false;
            }

            RuntimeDatabaseState database{
                .path = copperfin::platform::path_to_utf8_string(database_path->lexically_normal()),
                .name = copperfin::platform::path_to_utf8_string(database_path->stem()),
                .exclusive = exclusive,
                .read_only = read_only,
                .current = true};
            for (auto &candidate : session.databases)
            {
                candidate.current = false;
            }
            session.current_database_path = database.path;
            session.databases.push_back(std::move(database));
            return true;
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
