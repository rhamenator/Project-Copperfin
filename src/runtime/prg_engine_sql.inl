// prg_engine_sql.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        bool open_table_cursor(
            const std::string &raw_path,
            const std::string &requested_alias,
            const std::string &in_expression,
            bool allow_again,
            bool remote,
            int sql_handle,
            const std::string &sql_command,
            std::size_t synthetic_record_count,
            const std::map<std::string, CursorState::FieldRule> &field_rules = {})
        {
            (void)sql_command;
            std::string alias = requested_alias;
            std::string resolved_path = raw_path;
            std::string dbf_identity;
            std::size_t field_count = 0;
            std::size_t record_count = synthetic_record_count;

            if (!remote)
            {
                std::filesystem::path table_path(unquote_string(trim_copy(raw_path)));
                if (table_path.extension().empty())
                {
                    table_path += ".dbf";
                }
                if (table_path.is_relative())
                {
                    table_path = std::filesystem::path(current_default_directory()) / table_path;
                }
                table_path = table_path.lexically_normal();
                if (!std::filesystem::exists(table_path))
                {
                    last_error_message = "Unable to resolve USE target: " + table_path.string();
                    return false;
                }

                const auto table_result = vfp::parse_dbf_table_from_file(table_path.string(), 1U);
                if (!table_result.ok)
                {
                    last_error_message = table_result.error;
                    return false;
                }

                resolved_path = table_path.string();
                dbf_identity = resolved_path;
                field_count = table_result.table.fields.size();
                record_count = table_result.table.header.record_count;
                std::size_t record_length = table_result.table.header.record_length;
                std::vector<CursorState::OrderState> orders = load_cursor_orders(resolved_path);
                if (alias.empty())
                {
                    alias = table_path.stem().string();
                }

                const std::optional<int> requested_target_area = resolve_use_target_work_area(in_expression, stack.back());
                if (!requested_target_area.has_value())
                {
                    return false;
                }
                int target_area = *requested_target_area;
                const bool preserve_selected_work_area =
                    !trim_copy(in_expression).empty() &&
                    target_area > 0 &&
                    target_area != current_selected_work_area();
                target_area = preserve_selected_work_area
                                  ? reserve_work_area(target_area)
                                  : select_work_area(target_area);

                if (!can_open_table_cursor(resolved_path, alias, false, allow_again, target_area))
                {
                    return false;
                }

                DataSessionState &session = current_session_state();
                session.aliases[target_area] = alias;
                CursorState cursor;
                cursor.work_area = target_area;
                cursor.alias = alias;
                cursor.source_path = resolved_path;
                cursor.dbf_identity = dbf_identity;
                cursor.source_kind = "table";
                cursor.field_count = field_count;
                cursor.record_count = record_count;
                cursor.record_length = record_length;
                cursor.recno = record_count == 0U ? 0U : 1U;
                cursor.bof = record_count == 0U;
                cursor.eof = record_count == 0U;
                cursor.orders = std::move(orders);
                cursor.field_rules = field_rules;
                session.cursors[target_area] = std::move(cursor);
                return true;
            }
            else if (alias.empty())
            {
                alias = "sqlresult" + std::to_string(next_available_work_area());
            }
            if (remote)
            {
                dbf_identity = alias;
                field_count = synthetic_record_count == 0U ? 0U : 3U;
            }

            const std::optional<int> requested_target_area = resolve_use_target_work_area(in_expression, stack.back());
            if (!requested_target_area.has_value())
            {
                return false;
            }
            int target_area = *requested_target_area;
            const bool preserve_selected_work_area =
                !trim_copy(in_expression).empty() &&
                target_area > 0 &&
                target_area != current_selected_work_area();
            target_area = preserve_selected_work_area
                              ? reserve_work_area(target_area)
                              : select_work_area(target_area);

            if (!can_open_table_cursor(resolved_path, alias, remote, allow_again, target_area))
            {
                return false;
            }

            DataSessionState &session = current_session_state();
            std::vector<vfp::DbfRecord> remote_records;
            session.aliases[target_area] = alias;
            if (remote)
            {
                remote_records.reserve(record_count);
                for (std::size_t recno = 1U; recno <= record_count; ++recno)
                {
                    remote_records.push_back(make_synthetic_sql_record(recno));
                }
            }
            CursorState cursor;
            cursor.work_area = target_area;
            cursor.alias = alias;
            cursor.source_path = resolved_path;
            cursor.dbf_identity = dbf_identity;
            cursor.source_kind = remote ? "sql-cursor" : "table";
            cursor.remote = remote;
            cursor.field_count = field_count;
            cursor.record_count = record_count;
            cursor.recno = record_count == 0U ? 0U : 1U;
            cursor.bof = record_count == 0U;
            cursor.eof = record_count == 0U;
            cursor.remote_records = std::move(remote_records);
            cursor.field_rules = field_rules;
            session.cursors[target_area] = std::move(cursor);
            if (remote && sql_handle > 0)
            {
                auto &connections = current_sql_connections();
                auto found = connections.find(sql_handle);
                if (found != connections.end())
                {
                    found->second.last_cursor_alias = alias;
                    found->second.last_result_count = record_count;
                }
            }
            return true;
        }

        int sql_connect(const std::string &target, const std::string &provider)
        {
            std::string provider_hint = provider;
            const std::string normalized_target = lowercase_copy(target);
            if (normalized_target.find("provider=") != std::string::npos)
            {
                provider_hint = "oledb";
            }
            else if (normalized_target.find("driver=") != std::string::npos || normalized_target.find("dsn=") != std::string::npos)
            {
                provider_hint = "odbc";
            }

            int &next_handle = current_sql_handle_counter();
            const int handle = next_handle++;
            current_sql_connections().emplace(handle, RuntimeSqlConnectionState{
                                                          .handle = handle,
                                                          .target = target,
                                                          .provider = provider_hint,
                                                          .last_cursor_alias = {},
                                                          .last_result_count = 0U,
                                                          .prepared_command = {},
                                                          .properties = {
                                                              {"provider", provider_hint},
                                                              {"target", target},
                                                              {"querytimeout", "0"},
                                                              {"connecttimeout", "0"}}});
            return handle;
        }

        bool sql_disconnect(int handle)
        {
            return current_sql_connections().erase(handle) > 0;
        }

        int sql_row_count(int handle) const
        {
            const auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                return -1;
            }
            return static_cast<int>(found->second.last_result_count);
        }

        int sql_prepare(int handle, const std::string &command)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }
            found->second.prepared_command = command;
            found->second.properties["preparedcommand"] = command;
            events.push_back({.category = "sql.prepare",
                              .detail = "handle " + std::to_string(handle) + ": " + command,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        PrgValue sql_get_prop(int handle, const std::string &property_name) const
        {
            const auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                return make_number_value(-1.0);
            }

            const std::string normalized_name = normalize_identifier(property_name);
            if (normalized_name == "provider")
            {
                return make_string_value(found->second.provider);
            }
            if (normalized_name == "target" || normalized_name == "connectstring")
            {
                return make_string_value(found->second.target);
            }
            if (normalized_name == "preparedcommand")
            {
                return make_string_value(found->second.prepared_command);
            }
            if (normalized_name == "rowcount" || normalized_name == "lastresultcount")
            {
                return make_number_value(static_cast<double>(found->second.last_result_count));
            }

            const auto property = found->second.properties.find(normalized_name);
            if (property == found->second.properties.end())
            {
                return make_empty_value();
            }

            const std::string raw_value = property->second;
            if (!raw_value.empty() && std::all_of(raw_value.begin(), raw_value.end(), [](unsigned char ch)
                                                  { return std::isdigit(ch) != 0 || ch == '-' || ch == '.'; }))
            {
                return make_number_value(std::stod(raw_value));
            }
            return make_string_value(raw_value);
        }

        int sql_set_prop(int handle, const std::string &property_name, const PrgValue &value)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }

            const std::string normalized_name = normalize_identifier(property_name);
            const std::string property_value = value.kind == PrgValueKind::number
                                                   ? std::to_string(static_cast<long long>(std::llround(value.number_value)))
                                                   : value_as_string(value);

            if (normalized_name == "preparedcommand")
            {
                found->second.prepared_command = property_value;
            }
            found->second.properties[normalized_name] = property_value;
            events.push_back({.category = "sql.setprop",
                              .detail = "handle " + std::to_string(handle) + ": " + normalized_name + "=" + property_value,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int sql_exec(int handle, const std::string &command, const std::string &cursor_alias)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }

            RuntimeSqlConnectionState &connection = found->second;
            connection.last_cursor_alias.clear();
            connection.last_result_count = 0U;

            const std::string effective_command = trim_copy(command).empty() ? connection.prepared_command : trim_copy(command);
            if (effective_command.empty())
            {
                last_error_message = "SQLEXEC requires a command or a prepared SQL statement";
                last_error_code = classify_runtime_error_code(last_error_message);
                return -1;
            }

            std::size_t result_count = 0;
            const std::string normalized_command = lowercase_copy(effective_command);
            if (normalized_command.rfind("select", 0) == 0)
            {
                result_count = 3U;
                const std::string alias = trim_copy(cursor_alias).empty()
                                              ? "sqlresult" + std::to_string(handle)
                                              : trim_copy(cursor_alias);
                if (!open_table_cursor({}, alias, resolve_sql_cursor_auto_target(), true, true, handle, effective_command, result_count))
                {
                    return -1;
                }
            }
            else if (
                normalized_command.rfind("insert", 0) == 0 ||
                normalized_command.rfind("update", 0) == 0 ||
                normalized_command.rfind("delete", 0) == 0)
            {
                result_count = normalized_command.find("where 1=0") == std::string::npos ? 1U : 0U;
                connection.last_result_count = result_count;
            }
            events.push_back({.category = "sql.exec",
                              .detail = "handle " + std::to_string(handle) + ": " + effective_command,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            if (result_count > 0U)
            {
                if (normalized_command.rfind("select", 0) == 0)
                {
                    events.push_back({.category = "sql.cursor",
                                      .detail = connection.last_cursor_alias + " (" + std::to_string(result_count) + " rows)",
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                }
                else
                {
                    events.push_back({.category = "sql.rows",
                                      .detail = "handle " + std::to_string(handle) + ": " + std::to_string(connection.last_result_count) + " affected",
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                }
            }
            else if (
                normalized_command.rfind("insert", 0) == 0 ||
                normalized_command.rfind("update", 0) == 0 ||
                normalized_command.rfind("delete", 0) == 0)
            {
                events.push_back({.category = "sql.rows",
                                  .detail = "handle " + std::to_string(handle) + ": 0 affected",
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            }
            return 1;
        }

        int register_ole_object(const std::string &prog_id, const std::string &source)
        {
            const int handle = next_ole_handle++;
            ole_objects.emplace(handle, RuntimeOleObjectState{
                                            .handle = handle,
                                            .prog_id = prog_id,
                                            .source = source,
                                            .last_action = source,
                                            .action_count = 1});
            return handle;
        }

        std::optional<RuntimeOleObjectState *> resolve_ole_object(const PrgValue &value)
        {
            int handle = 0;
            std::string prog_id;
            if (!parse_object_handle_reference(value, handle, prog_id))
            {
                return std::nullopt;
            }

            auto found = ole_objects.find(handle);
            if (found == ole_objects.end())
            {
                return std::nullopt;
            }
            return &found->second;
        }

