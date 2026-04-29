// prg_engine_sql.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        struct SyntheticSqlCatalogColumn
        {
            std::string name;
            char field_type = 'C';
            std::size_t length = 0U;
            std::size_t decimal_count = 0U;
            std::string native_type;
        };

        struct SyntheticSqlCatalogTable
        {
            std::string catalog;
            std::string schema;
            std::string name;
            std::string table_type;
            std::string remarks;
            std::vector<SyntheticSqlCatalogColumn> columns;
        };

        struct SyntheticSqlDatabase
        {
            std::string name;
            std::string remarks;
        };

        std::vector<SyntheticSqlDatabase> synthetic_sql_databases() const
        {
            return {
                SyntheticSqlDatabase{.name = "NORTHWIND", .remarks = "Synthetic primary catalog"},
                SyntheticSqlDatabase{.name = "ARCHIVE", .remarks = "Synthetic archive catalog"}};
        }

        std::vector<SyntheticSqlCatalogTable> synthetic_sql_catalog() const
        {
            return {
                SyntheticSqlCatalogTable{
                    .catalog = "NORTHWIND",
                    .schema = "dbo",
                    .name = "CUSTOMERS",
                    .table_type = "TABLE",
                    .remarks = "Synthetic customer table",
                    .columns = {
                        SyntheticSqlCatalogColumn{.name = "ID", .field_type = 'N', .length = 18U, .decimal_count = 0U, .native_type = "INTEGER"},
                        SyntheticSqlCatalogColumn{.name = "NAME", .field_type = 'C', .length = 32U, .decimal_count = 0U, .native_type = "VARCHAR"},
                        SyntheticSqlCatalogColumn{.name = "AMOUNT", .field_type = 'N', .length = 18U, .decimal_count = 0U, .native_type = "DECIMAL"}}},
                SyntheticSqlCatalogTable{
                    .catalog = "NORTHWIND",
                    .schema = "dbo",
                    .name = "ORDERS",
                    .table_type = "TABLE",
                    .remarks = "Synthetic orders table",
                    .columns = {
                        SyntheticSqlCatalogColumn{.name = "ORDER_ID", .field_type = 'N', .length = 18U, .decimal_count = 0U, .native_type = "INTEGER"},
                        SyntheticSqlCatalogColumn{.name = "CUSTOMER_ID", .field_type = 'N', .length = 18U, .decimal_count = 0U, .native_type = "INTEGER"},
                        SyntheticSqlCatalogColumn{.name = "ORDER_DATE", .field_type = 'D', .length = 8U, .decimal_count = 0U, .native_type = "DATE"}}},
                SyntheticSqlCatalogTable{
                    .catalog = "NORTHWIND",
                    .schema = "dbo",
                    .name = "CUSTOMER_SUMMARY",
                    .table_type = "VIEW",
                    .remarks = "Synthetic customer summary view",
                    .columns = {
                        SyntheticSqlCatalogColumn{.name = "NAME", .field_type = 'C', .length = 32U, .decimal_count = 0U, .native_type = "VARCHAR"},
                        SyntheticSqlCatalogColumn{.name = "TOTAL_ORDERS", .field_type = 'N', .length = 18U, .decimal_count = 0U, .native_type = "INTEGER"}}},
                SyntheticSqlCatalogTable{
                    .catalog = "NORTHWIND",
                    .schema = "sys",
                    .name = "SYSRELATIONS",
                    .table_type = "SYSTEM TABLE",
                    .remarks = "Synthetic system metadata table",
                    .columns = {
                        SyntheticSqlCatalogColumn{.name = "PARENT", .field_type = 'C', .length = 32U, .decimal_count = 0U, .native_type = "VARCHAR"},
                        SyntheticSqlCatalogColumn{.name = "CHILD", .field_type = 'C', .length = 32U, .decimal_count = 0U, .native_type = "VARCHAR"}}}};
        }

        std::vector<std::string> parse_sql_table_type_filter(const std::string &table_types) const
        {
            std::vector<std::string> results;
            std::string current;
            bool in_quote = false;
            for (char ch : table_types)
            {
                if (ch == '\'')
                {
                    in_quote = !in_quote;
                    continue;
                }
                if (!in_quote && ch == ',')
                {
                    const std::string trimmed = uppercase_copy(trim_copy(current));
                    if (!trimmed.empty())
                    {
                        results.push_back(trimmed);
                    }
                    current.clear();
                    continue;
                }
                current.push_back(ch);
            }

            const std::string trimmed = uppercase_copy(trim_copy(current));
            if (!trimmed.empty())
            {
                results.push_back(trimmed);
            }
            return results;
        }

        bool synthetic_sql_table_type_matches(
            const std::vector<std::string> &requested_types,
            const std::string &candidate_type) const
        {
            if (requested_types.empty())
            {
                return true;
            }

            const std::string normalized_candidate = uppercase_copy(trim_copy(candidate_type));
            return std::find(requested_types.begin(), requested_types.end(), normalized_candidate) != requested_types.end();
        }

        std::vector<vfp::DbfRecord> make_sqltables_records(const std::vector<std::string> &requested_types) const
        {
            std::vector<vfp::DbfRecord> records;
            std::size_t recno = 1U;
            for (const auto &table : synthetic_sql_catalog())
            {
                if (!synthetic_sql_table_type_matches(requested_types, table.table_type))
                {
                    continue;
                }

                records.push_back(vfp::DbfRecord{
                    .record_index = recno - 1U,
                    .deleted = false,
                    .values = {
                        vfp::DbfRecordValue{.field_name = "TABLE_CAT", .field_type = 'C', .display_value = table.catalog},
                        vfp::DbfRecordValue{.field_name = "TABLE_SCHEM", .field_type = 'C', .display_value = table.schema},
                        vfp::DbfRecordValue{.field_name = "TABLE_NAME", .field_type = 'C', .display_value = table.name},
                        vfp::DbfRecordValue{.field_name = "TABLE_TYPE", .field_type = 'C', .display_value = table.table_type},
                        vfp::DbfRecordValue{.field_name = "REMARKS", .field_type = 'C', .display_value = table.remarks}}});
                ++recno;
            }
            return records;
        }

        std::vector<vfp::DbfRecord> make_sqldatabases_records() const
        {
            std::vector<vfp::DbfRecord> records;
            std::size_t recno = 1U;
            for (const auto &database : synthetic_sql_databases())
            {
                records.push_back(vfp::DbfRecord{
                    .record_index = recno - 1U,
                    .deleted = false,
                    .values = {
                        vfp::DbfRecordValue{.field_name = "DATABASE_NAME", .field_type = 'C', .display_value = database.name},
                        vfp::DbfRecordValue{.field_name = "REMARKS", .field_type = 'C', .display_value = database.remarks}}});
                ++recno;
            }
            return records;
        }

        std::vector<vfp::DbfRecord> make_sqlcolumns_records(
            const std::string &table_name,
            bool native_format,
            bool &matched_any) const
        {
            std::vector<vfp::DbfRecord> records;
            matched_any = false;
            const std::string effective_pattern = trim_copy(table_name).empty() ? "*" : trim_copy(table_name);
            std::size_t recno = 1U;

            for (const auto &table : synthetic_sql_catalog())
            {
                if (!wildcard_match_insensitive(effective_pattern, table.name))
                {
                    continue;
                }

                matched_any = true;
                for (std::size_t index = 0U; index < table.columns.size(); ++index)
                {
                    const auto &column = table.columns[index];
                    if (native_format)
                    {
                        records.push_back(vfp::DbfRecord{
                            .record_index = recno - 1U,
                            .deleted = false,
                            .values = {
                                vfp::DbfRecordValue{.field_name = "TABLE_CAT", .field_type = 'C', .display_value = table.catalog},
                                vfp::DbfRecordValue{.field_name = "TABLE_SCHEM", .field_type = 'C', .display_value = table.schema},
                                vfp::DbfRecordValue{.field_name = "TABLE_NAME", .field_type = 'C', .display_value = table.name},
                                vfp::DbfRecordValue{.field_name = "COLUMN_NAME", .field_type = 'C', .display_value = column.name},
                                vfp::DbfRecordValue{.field_name = "TYPE_NAME", .field_type = 'C', .display_value = column.native_type},
                                vfp::DbfRecordValue{.field_name = "COLUMN_SIZE", .field_type = 'N', .display_value = std::to_string(column.length)},
                                vfp::DbfRecordValue{.field_name = "DECIMAL_DIGITS", .field_type = 'N', .display_value = std::to_string(column.decimal_count)},
                                vfp::DbfRecordValue{.field_name = "ORDINAL_POSITION", .field_type = 'N', .display_value = std::to_string(index + 1U)}}});
                    }
                    else
                    {
                        records.push_back(vfp::DbfRecord{
                            .record_index = recno - 1U,
                            .deleted = false,
                            .values = {
                                vfp::DbfRecordValue{.field_name = "FIELD_NAME", .field_type = 'C', .display_value = column.name},
                                vfp::DbfRecordValue{.field_name = "FIELD_TYPE", .field_type = 'C', .display_value = std::string(1U, column.field_type)},
                                vfp::DbfRecordValue{.field_name = "FIELD_LEN", .field_type = 'N', .display_value = std::to_string(column.length)},
                                vfp::DbfRecordValue{.field_name = "FIELD_DEC", .field_type = 'N', .display_value = std::to_string(column.decimal_count)}}});
                    }
                    ++recno;
                }
            }

            return records;
        }

        bool open_remote_cursor(
            const std::string &requested_alias,
            const std::string &in_expression,
            int sql_handle,
            const std::string &source_kind,
            std::vector<vfp::DbfRecord> remote_records,
            std::vector<vfp::DbfFieldDescriptor> remote_fields = {})
        {
            std::string alias = trim_copy(requested_alias);
            if (alias.empty())
            {
                alias = "sqlresult";
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

            if (!can_open_table_cursor({}, alias, true, true, target_area))
            {
                return false;
            }

            DataSessionState &session = current_session_state();
            session.aliases[target_area] = alias;
            CursorState cursor;
            cursor.work_area = target_area;
            cursor.alias = alias;
            cursor.dbf_identity = alias;
            cursor.source_kind = source_kind;
            cursor.remote = true;
            cursor.exclusive = false;
            cursor.field_count = remote_fields.empty()
                                     ? (remote_records.empty() ? 0U : remote_records.front().values.size())
                                     : remote_fields.size();
            cursor.record_count = remote_records.size();
            cursor.recno = remote_records.empty() ? 0U : 1U;
            cursor.bof = remote_records.empty();
            cursor.eof = remote_records.empty();
            cursor.remote_records = std::move(remote_records);
            cursor.remote_fields = std::move(remote_fields);
            session.cursors[target_area] = std::move(cursor);
            if (sql_handle > 0)
            {
                auto &connections = current_sql_connections();
                const auto found = connections.find(sql_handle);
                if (found != connections.end())
                {
                    found->second.last_cursor_alias = alias;
                    found->second.last_result_count = session.cursors[target_area].record_count;
                }
            }
            return true;
        }

        std::vector<vfp::DbfFieldDescriptor> sqltables_field_descriptors() const
        {
            return {
                vfp::DbfFieldDescriptor{.name = "TABLE_CAT", .type = 'C', .length = 32U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "TABLE_SCHEM", .type = 'C', .length = 32U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "TABLE_NAME", .type = 'C', .length = 64U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "TABLE_TYPE", .type = 'C', .length = 32U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "REMARKS", .type = 'C', .length = 64U, .decimal_count = 0U}};
        }

        std::vector<vfp::DbfFieldDescriptor> sqldatabases_field_descriptors() const
        {
            return {
                vfp::DbfFieldDescriptor{.name = "DATABASE_NAME", .type = 'C', .length = 64U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "REMARKS", .type = 'C', .length = 64U, .decimal_count = 0U}};
        }

        std::vector<vfp::DbfFieldDescriptor> sqlcolumns_field_descriptors(bool native_format) const
        {
            if (native_format)
            {
                return {
                    vfp::DbfFieldDescriptor{.name = "TABLE_CAT", .type = 'C', .length = 32U, .decimal_count = 0U},
                    vfp::DbfFieldDescriptor{.name = "TABLE_SCHEM", .type = 'C', .length = 32U, .decimal_count = 0U},
                    vfp::DbfFieldDescriptor{.name = "TABLE_NAME", .type = 'C', .length = 64U, .decimal_count = 0U},
                    vfp::DbfFieldDescriptor{.name = "COLUMN_NAME", .type = 'C', .length = 64U, .decimal_count = 0U},
                    vfp::DbfFieldDescriptor{.name = "TYPE_NAME", .type = 'C', .length = 32U, .decimal_count = 0U},
                    vfp::DbfFieldDescriptor{.name = "COLUMN_SIZE", .type = 'N', .length = 18U, .decimal_count = 0U},
                    vfp::DbfFieldDescriptor{.name = "DECIMAL_DIGITS", .type = 'N', .length = 18U, .decimal_count = 0U},
                    vfp::DbfFieldDescriptor{.name = "ORDINAL_POSITION", .type = 'N', .length = 18U, .decimal_count = 0U}};
            }

            return {
                vfp::DbfFieldDescriptor{.name = "FIELD_NAME", .type = 'C', .length = 64U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "FIELD_TYPE", .type = 'C', .length = 1U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "FIELD_LEN", .type = 'N', .length = 18U, .decimal_count = 0U},
                vfp::DbfFieldDescriptor{.name = "FIELD_DEC", .type = 'N', .length = 18U, .decimal_count = 0U}};
        }

        bool open_table_cursor(
            const std::string &raw_path,
            const std::string &requested_alias,
            const std::string &in_expression,
            bool allow_again,
            bool remote,
            int sql_handle,
            const std::string &sql_command,
            std::size_t synthetic_record_count,
            const std::map<std::string, CursorState::FieldRule> &field_rules = {},
            std::optional<bool> exclusive_override = std::nullopt)
        {
            (void)sql_command;
            std::string alias = requested_alias;
            std::string resolved_path = raw_path;
            std::string dbf_identity;
            std::size_t field_count = 0;
            std::size_t record_count = synthetic_record_count;
            const bool exclusive_open = remote ? false : exclusive_override.value_or(is_set_enabled_or_default("exclusive", true));

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
                cursor.exclusive = exclusive_open;
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
            cursor.exclusive = exclusive_open;
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
                                                          .last_sql_action = "connect",
                                                          .transaction_dirty = false,
                                                          .cancel_requested = false,
                                                          .properties = {
                                                              {"provider", provider_hint},
                                                              {"target", target},
                                                              {"querytimeout", "0"},
                                                              {"connecttimeout", "0"},
                                                              {"lastsqlaction", "connect"},
                                                              {"transactiondirty", "false"},
                                                              {"cancelrequested", "false"}}});
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
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLPREPARE", trim_copy(command));
                return -1;
            }
            found->second.prepared_command = command;
            found->second.cancel_requested = false;
            found->second.last_sql_action = "prepare";
            found->second.properties["preparedcommand"] = command;
            found->second.properties["cancelrequested"] = "false";
            found->second.properties["lastsqlaction"] = "prepare";
            events.push_back({.category = "sql.prepare",
                              .detail = "handle " + std::to_string(handle) + ": " + command,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int sql_cancel(int handle)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLCANCEL", std::string{});
                return -1;
            }

            found->second.cancel_requested = true;
            found->second.last_sql_action = "cancel";
            found->second.properties["cancelrequested"] = "true";
            found->second.properties["lastsqlaction"] = "cancel";
            events.push_back({.category = "sql.cancel",
                              .detail = "handle " + std::to_string(handle),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int sql_commit(int handle)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLCOMMIT", std::string{});
                return -1;
            }

            found->second.transaction_dirty = false;
            found->second.cancel_requested = false;
            found->second.last_sql_action = "commit";
            found->second.properties["transactiondirty"] = "false";
            found->second.properties["cancelrequested"] = "false";
            found->second.properties["lastsqlaction"] = "commit";
            events.push_back({.category = "sql.commit",
                              .detail = "handle " + std::to_string(handle),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int sql_rollback(int handle)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLROLLBACK", std::string{});
                return -1;
            }

            found->second.transaction_dirty = false;
            found->second.cancel_requested = false;
            found->second.last_sql_action = "rollback";
            found->second.properties["transactiondirty"] = "false";
            found->second.properties["cancelrequested"] = "false";
            found->second.properties["lastsqlaction"] = "rollback";
            events.push_back({.category = "sql.rollback",
                              .detail = "handle " + std::to_string(handle),
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
            if (normalized_name == "lastsqlaction")
            {
                return make_string_value(found->second.last_sql_action);
            }
            if (normalized_name == "rowcount" || normalized_name == "lastresultcount")
            {
                return make_number_value(static_cast<double>(found->second.last_result_count));
            }
            if (normalized_name == "transactiondirty")
            {
                return make_boolean_value(found->second.transaction_dirty);
            }
            if (normalized_name == "cancelrequested")
            {
                return make_boolean_value(found->second.cancel_requested);
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
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLSETPROP", trim_copy(property_name));
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
            else if (normalized_name == "transactiondirty")
            {
                found->second.transaction_dirty = value_as_bool(value);
            }
            else if (normalized_name == "cancelrequested")
            {
                found->second.cancel_requested = value_as_bool(value);
            }
            else if (normalized_name == "lastsqlaction")
            {
                found->second.last_sql_action = property_value;
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
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLEXEC", trim_copy(command));
                return -1;
            }

            RuntimeSqlConnectionState &connection = found->second;
            connection.last_cursor_alias.clear();
            connection.last_result_count = 0U;
            connection.cancel_requested = false;

            const std::string effective_command = trim_copy(command).empty() ? connection.prepared_command : trim_copy(command);
            if (effective_command.empty())
            {
                last_error_message = "SQLEXEC requires a command or a prepared SQL statement";
                last_error_code = classify_runtime_error_code(last_error_message);
                record_sql_aerror_context(std::to_string(handle),
                                          "HY000",
                                          -1,
                                          connection.provider.empty() ? std::string("SQLEXEC") : connection.provider,
                                          connection.target);
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
                connection.transaction_dirty = true;
            }
            connection.last_sql_action = "exec";
            connection.properties["lastsqlaction"] = "exec";
            connection.properties["cancelrequested"] = "false";
            connection.properties["transactiondirty"] = connection.transaction_dirty ? "true" : "false";
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

        int sql_tables(int handle, const std::string &table_types, const std::string &cursor_alias)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLTABLES", trim_copy(table_types));
                return -1;
            }

            RuntimeSqlConnectionState &connection = found->second;
            connection.cancel_requested = false;
            const std::vector<std::string> requested_types = parse_sql_table_type_filter(table_types);
            std::vector<vfp::DbfRecord> records = make_sqltables_records(requested_types);
            if (!open_remote_cursor(
                    cursor_alias,
                    resolve_sql_cursor_auto_target(),
                    handle,
                    "sql-tables-cursor",
                    std::move(records),
                    sqltables_field_descriptors()))
            {
                return -1;
            }

            connection.last_sql_action = "tables";
            connection.properties["lastsqlaction"] = "tables";
            connection.properties["cancelrequested"] = "false";
            events.push_back({.category = "sql.tables",
                              .detail = "handle " + std::to_string(handle) + ": " + trim_copy(table_types),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            events.push_back({.category = "sql.cursor",
                              .detail = connection.last_cursor_alias + " (" + std::to_string(connection.last_result_count) + " rows)",
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int sql_databases(int handle, const std::string &cursor_alias)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLDATABASES", std::string{});
                return -1;
            }

            RuntimeSqlConnectionState &connection = found->second;
            connection.cancel_requested = false;
            std::vector<vfp::DbfRecord> records = make_sqldatabases_records();
            if (!open_remote_cursor(
                    cursor_alias,
                    resolve_sql_cursor_auto_target(),
                    handle,
                    "sql-databases-cursor",
                    std::move(records),
                    sqldatabases_field_descriptors()))
            {
                return -1;
            }

            connection.last_sql_action = "databases";
            connection.properties["lastsqlaction"] = "databases";
            connection.properties["cancelrequested"] = "false";
            events.push_back({.category = "sql.databases",
                              .detail = "handle " + std::to_string(handle),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            events.push_back({.category = "sql.cursor",
                              .detail = connection.last_cursor_alias + " (" + std::to_string(connection.last_result_count) + " rows)",
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int sql_columns(int handle, const std::string &table_name, const std::string &format, const std::string &cursor_alias)
        {
            auto &connections = current_sql_connections();
            const auto found = connections.find(handle);
            if (found == connections.end())
            {
                last_error_message = "SQL handle not found: " + std::to_string(handle);
                last_error_code = classify_runtime_error_code(last_error_message);
                record_sql_aerror_context(std::to_string(handle), "HY000", -1, "SQLCOLUMNS", trim_copy(table_name));
                return -1;
            }

            RuntimeSqlConnectionState &connection = found->second;
            connection.cancel_requested = false;
            const std::string normalized_format = uppercase_copy(trim_copy(format.empty() ? "FOXPRO" : format));
            const bool native_format = normalized_format == "NATIVE";

            bool matched_any = false;
            std::vector<vfp::DbfRecord> records = make_sqlcolumns_records(table_name, native_format, matched_any);
            if (!matched_any && !native_format)
            {
                return 0;
            }

            if (!open_remote_cursor(
                    cursor_alias,
                    resolve_sql_cursor_auto_target(),
                    handle,
                    "sql-columns-cursor",
                    std::move(records),
                    sqlcolumns_field_descriptors(native_format)))
            {
                return -1;
            }

            connection.last_sql_action = "columns";
            connection.properties["lastsqlaction"] = "columns";
            connection.properties["cancelrequested"] = "false";
            events.push_back({.category = "sql.columns",
                              .detail = "handle " + std::to_string(handle) + ": " + trim_copy(table_name) + " [" + normalized_format + "]",
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            events.push_back({.category = "sql.cursor",
                              .detail = connection.last_cursor_alias + " (" + std::to_string(connection.last_result_count) + " rows)",
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return 1;
        }

        int register_ole_object(const std::string &prog_id, const std::string &source)
        {
            const int handle = next_ole_handle++;
            RuntimeOleObjectState object_state{
                .handle = handle,
                .prog_id = prog_id,
                .source = source,
                .last_action = source,
                .action_count = 1};

            const std::string normalized_prog_id = normalize_identifier(prog_id);
            if (normalized_prog_id == "scripting.dictionary")
            {
                object_state.methods = {
                    "add",
                    "exists",
                    "item",
                    "remove",
                    "removeall",
                    "keys",
                    "items"};
                object_state.properties["count"] = make_number_value(0.0);
                object_state.properties["comparemode"] = make_number_value(0.0);
            }

            ole_objects.emplace(handle, std::move(object_state));
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
