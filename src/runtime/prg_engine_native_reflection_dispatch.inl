    std::optional<PrgValue> PrgRuntimeSession::Impl::read_native_property_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &property_name,
        const Frame &source_frame)
    {
        const std::string normalized_property_name = normalize_identifier(property_name);
        if (normalized_property_name.empty())
        {
            return std::nullopt;
        }

        const auto evaluate_integer_selector_expression = [&](const std::string& expression)
            -> std::optional<long long>
        {
            const std::string trimmed_expression = trim_copy(expression);
            if (const auto literal = copperfin::platform::try_parse_invariant_integer<long long>(trimmed_expression);
                literal.has_value())
            {
                return literal;
            }
            if (copperfin::platform::try_parse_invariant_double(trimmed_expression).has_value())
            {
                return std::nullopt;
            }
            const PrgValue evaluated = evaluate_expression(trimmed_expression, source_frame);
            return static_cast<long long>(std::llround(value_as_number(evaluated)));
        };

        if (normalized_property_name == "header" &&
            is_native_column_runtime_object(runtime_object))
        {
            (void)ensure_native_column_header_surface(runtime_object);
        }

        if (const auto visibility = runtime_object.member_visibility.find(normalized_property_name);
            visibility != runtime_object.member_visibility.end())
        {
            const auto owner = runtime_object.member_visibility_owner.find(normalized_property_name);
            if (!native_member_access_allowed(
                    runtime_object,
                    visibility->second,
                    owner == runtime_object.member_visibility_owner.end() ? std::string{} : owner->second,
                    source_frame))
            {
                raise_native_member_access_denied(runtime_object, property_name, false);
            }
        }

        auto resolve_list_member_cell = [&]() -> std::optional<NativeListControlCellReference>
        {
            const auto literal_cell =
                parse_native_list_control_list_member_cell(runtime_object, property_name);
            if (literal_cell.has_value())
            {
                return literal_cell;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("list");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const std::size_t comma = selector_text->find(',');
            const std::string row_expression = trim_copy(selector_text->substr(0U, comma));
            const std::string column_expression =
                comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
            if (row_expression.empty() || column_expression.empty())
            {
                return std::nullopt;
            }

            const auto requested_row = evaluate_integer_selector_expression(row_expression);
            const auto requested_column = evaluate_integer_selector_expression(column_expression);
            if (!requested_row.has_value() || !requested_column.has_value() ||
                *requested_row < 1LL || *requested_column < 1LL)
            {
                return std::nullopt;
            }
            return NativeListControlCellReference{
                .row_slot = static_cast<std::size_t>(*requested_row - 1LL),
                .column_slot = static_cast<std::size_t>(*requested_column - 1LL)};
        };

        auto resolve_listitem_member_cell = [&]() -> std::optional<NativeListControlItemCellReference>
        {
            const auto literal_cell =
                parse_native_list_control_listitem_member_cell(runtime_object, property_name);
            if (literal_cell.has_value())
            {
                return literal_cell;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("listitem");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const std::size_t comma = selector_text->find(',');
            const std::string item_id_expression = trim_copy(selector_text->substr(0U, comma));
            const std::string column_expression =
                comma == std::string::npos ? std::string("1") : trim_copy(selector_text->substr(comma + 1U));
            if (item_id_expression.empty() || column_expression.empty())
            {
                return std::nullopt;
            }

            const auto requested_item_id = evaluate_integer_selector_expression(item_id_expression);
            const auto requested_column = evaluate_integer_selector_expression(column_expression);
            if (!requested_item_id.has_value() || !requested_column.has_value() ||
                *requested_item_id < 1LL || *requested_column < 1LL)
            {
                return std::nullopt;
            }
            return NativeListControlItemCellReference{
                .item_id = *requested_item_id,
                .column_slot = static_cast<std::size_t>(*requested_column - 1LL)};
        };

        auto resolve_indextoitemid_member_slot = [&]() -> std::optional<std::size_t>
        {
            const auto literal_slot =
                parse_native_list_control_indextoitemid_member_slot(runtime_object, property_name);
            if (literal_slot.has_value())
            {
                return literal_slot;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("indextoitemid");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const auto requested_index = evaluate_integer_selector_expression(*selector_text);
            if (!requested_index.has_value() || *requested_index < 1LL)
            {
                return std::nullopt;
            }
            return static_cast<std::size_t>(*requested_index - 1LL);
        };

        auto resolve_itemidtoindex_member_item_id = [&]() -> std::optional<long long>
        {
            const auto literal_item_id =
                parse_native_list_control_itemidtoindex_member_item_id(runtime_object, property_name);
            if (literal_item_id.has_value())
            {
                return literal_item_id;
            }

            const auto extract_selector_text = [&](const std::string& base_name) -> std::optional<std::string>
            {
                const std::string trimmed_property_name = trim_copy(property_name);
                const auto extract_for_delimiters =
                    [&](char open_delimiter, char close_delimiter) -> std::optional<std::string>
                {
                    std::string prefix = base_name;
                    prefix.push_back(open_delimiter);
                    const std::string folded_property_name = lowercase_copy(trimmed_property_name);
                    if (!starts_with_insensitive(folded_property_name, prefix) ||
                        folded_property_name.empty() ||
                        folded_property_name.back() != close_delimiter)
                    {
                        return std::nullopt;
                    }

                    const std::size_t open = trimmed_property_name.find(open_delimiter);
                    const std::size_t close = trimmed_property_name.rfind(close_delimiter);
                    if (open == std::string::npos ||
                        close == std::string::npos ||
                        close <= open + 1U)
                    {
                        return std::nullopt;
                    }
                    return trimmed_property_name.substr(open + 1U, close - open - 1U);
                };

                if (const auto parenthesized = extract_for_delimiters('(', ')');
                    parenthesized.has_value())
                {
                    return parenthesized;
                }
                return extract_for_delimiters('[', ']');
            };

            const auto selector_text = extract_selector_text("itemidtoindex");
            if (!selector_text.has_value())
            {
                return std::nullopt;
            }

            const auto requested_item_id = evaluate_integer_selector_expression(*selector_text);
            if (!requested_item_id.has_value() || *requested_item_id < 1LL)
            {
                return std::nullopt;
            }
            return *requested_item_id;
        };

        const auto perform_property_read = [&]() -> std::optional<PrgValue>
        {
            if (auto access_result = invoke_native_object_method_body_if_present(
                    runtime_object,
                    normalized_property_name + "_access",
                    source_frame,
                    {},
                    {});
                access_result.has_value())
            {
                return *access_result;
            }
            if (auto metadata_value = read_native_identity_metadata(runtime_object, normalized_property_name);
                metadata_value.has_value())
            {
                return *metadata_value;
            }
            if (auto collection_value = read_native_collection_member(runtime_object, normalized_property_name);
                collection_value.has_value())
            {
                return *collection_value;
            }
            if (const auto list_cell = resolve_list_member_cell();
                list_cell.has_value())
            {
                return read_native_list_control_cell(
                    runtime_object,
                    list_cell->row_slot,
                    list_cell->column_slot);
            }
            if (const auto item_cell = resolve_listitem_member_cell();
                item_cell.has_value())
            {
                return read_native_list_control_item_cell(
                    runtime_object,
                    item_cell->item_id,
                    item_cell->column_slot);
            }
            if (const auto item_data_slot =
                    parse_native_list_control_itemdata_member_slot(runtime_object, property_name);
                item_data_slot.has_value())
            {
                return read_native_list_control_item_data(runtime_object, *item_data_slot);
            }
            if (const auto item_id_slot = resolve_indextoitemid_member_slot();
                item_id_slot.has_value())
            {
                return read_native_list_control_item_id_for_slot(
                    runtime_object,
                    *item_id_slot);
            }
            if (const auto item_index_id = resolve_itemidtoindex_member_item_id();
                item_index_id.has_value())
            {
                return read_native_list_control_index_for_item_id(
                    runtime_object,
                    *item_index_id);
            }
            if (is_native_listcount_member_name(runtime_object, normalized_property_name))
            {
                sync_native_list_control_count(runtime_object);
            }
            if (is_native_activepage_member_name(runtime_object, normalized_property_name))
            {
                normalize_native_pageframe_activepage_invariant(runtime_object);
            }
            if (is_native_listitemid_member_name(runtime_object, normalized_property_name))
            {
                sync_native_list_control_displayvalue_from_selection(runtime_object);
            }
            if (is_native_textbox_selection_member_name(runtime_object, normalized_property_name))
            {
                normalize_native_textbox_selection_invariant(runtime_object);
            }
            if (is_native_textbox_text_member_name(runtime_object, normalized_property_name))
            {
                normalize_native_textbox_text_invariant(runtime_object);
            }
            if (is_native_identity_member_name(runtime_object, normalized_property_name))
            {
                return make_empty_value();
            }
            const auto property = runtime_object.properties.find(normalized_property_name);
            if (property != runtime_object.properties.end())
            {
                return property->second;
            }
            if (is_native_olecontrol_host_object(runtime_object))
            {
                RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(runtime_object);
                if (object_surface != nullptr && object_surface->handle != runtime_object.handle)
                {
                    return read_native_property_if_present(*object_surface, property_name, source_frame);
                }
            }
            return std::nullopt;
        };

        std::vector<NativeEventBinding> bindings;
        bindings.reserve(native_event_bindings.size());
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            if (binding.source_handle == runtime_object.handle &&
                binding.event_name == normalized_property_name &&
                (binding.flags & 2) == 0)
            {
                bindings.push_back(binding);
            }
        }

        const std::string active_event_key =
            std::to_string(runtime_object.handle) + ":" + normalized_property_name;
        const bool already_active =
            active_native_event_keys.find(active_event_key) != active_native_event_keys.end();

        if (!bindings.empty() && !already_active)
        {
            ActiveNativeEventKeyGuard active_event_guard(active_native_event_keys, active_event_key);
            const auto invoke_delegates_for_phase = [&](bool after_source_member)
            {
                for (const NativeEventBinding &binding : bindings)
                {
                    const bool binding_after_source_member = (binding.flags & 1) == 0;
                    if (binding_after_source_member == after_source_member)
                    {
                        (void)invoke_native_event_delegate(
                            binding,
                            {.source_handle = runtime_object.handle,
                             .event_name = normalized_property_name,
                             .event_type = 2},
                            {},
                            {});
                    }
                }
            };

            invoke_delegates_for_phase(false);
            auto result = perform_property_read();
            invoke_delegates_for_phase(true);
            return result;
        }

        return perform_property_read();
    }

    std::optional<std::string> PrgRuntimeSession::Impl::read_native_property_expression_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &property_name)
    {
        const std::string normalized_property_name = normalize_identifier(property_name);
        if (normalized_property_name.empty())
        {
            return std::nullopt;
        }

        if (const auto object_texts = native_property_expression_text_by_handle.find(runtime_object.handle);
            object_texts != native_property_expression_text_by_handle.end())
        {
            if (const auto expression_text = object_texts->second.find(normalized_property_name);
                expression_text != object_texts->second.end())
            {
                return expression_text->second;
            }
        }

        if (const auto property = runtime_object.properties.find(normalized_property_name);
            property != runtime_object.properties.end())
        {
            return serialize_runtime_expression_text(property->second);
        }

        if (is_native_olecontrol_host_object(runtime_object))
        {
            RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(runtime_object);
            if (object_surface != nullptr && object_surface->handle != runtime_object.handle)
            {
                return read_native_property_expression_if_present(*object_surface, property_name);
            }
        }

        return std::nullopt;
    }

    std::optional<std::string> PrgRuntimeSession::Impl::read_native_method_source_if_present(
        const RuntimeOleObjectState &runtime_object,
        const std::string &method_name)
    {
        const std::string normalized_method_name = normalize_identifier(method_name);
        if (normalized_method_name.empty() || runtime_object.source.empty())
        {
            return std::nullopt;
        }

        std::string method_program_path;
        std::string qualified_method_name;
        const Routine *method =
            find_native_object_method(runtime_object,
                                      normalized_method_name,
                                      method_program_path,
                                      qualified_method_name);
        (void)qualified_method_name;
        if (method == nullptr)
        {
            return std::nullopt;
        }

        if (const auto override_text =
                native_method_source_text_by_key.find(
                    make_native_method_override_key(method_program_path, qualified_method_name));
            override_text != native_method_source_text_by_key.end())
        {
            return override_text->second;
        }

        Program &method_program = load_program(method_program_path);
        if (!method_program.source_lines.empty() &&
            method->declaration_location.line > 0 &&
            method->body_end_line_exclusive > method->declaration_location.line)
        {
            const std::size_t body_start_index = method->declaration_location.line;
            const std::size_t body_end_index =
                std::min(method->body_end_line_exclusive - 1U, method_program.source_lines.size());
            if (body_start_index < body_end_index)
            {
                std::string source_text;
                for (std::size_t line_index = body_start_index; line_index < body_end_index; ++line_index)
                {
                    if (!source_text.empty())
                    {
                        source_text += "\n";
                    }
                    source_text += method_program.source_lines[line_index];
                }
                return source_text;
            }
        }

        std::string source_text;
        for (const Statement &statement : method->statements)
        {
            if (!source_text.empty())
            {
                source_text += "\n";
            }
            source_text += statement.text;
        }
        return source_text;
    }

    bool PrgRuntimeSession::Impl::write_native_method_source_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &method_name,
        const std::string &method_source_text,
        bool create_if_missing)
    {
        const std::string normalized_method_name = normalize_identifier(method_name);
        if (normalized_method_name.empty() || runtime_object.source.empty())
        {
            return false;
        }

        const auto starting_class_lookup =
            [&]() -> std::optional<NativeClassLookup>
            {
                std::vector<NativeClassLookup> lineage =
                    resolved_native_object_class_lineage(runtime_object);
                return lineage.empty()
                    ? std::nullopt
                    : std::optional<NativeClassLookup>(lineage.back());
            }();
        if (!starting_class_lookup.has_value() ||
            starting_class_lookup->class_definition == nullptr)
        {
            return false;
        }

        const auto starting_method =
            starting_class_lookup->class_definition->methods.find(
                normalized_method_name);
        std::string qualified_method_name;
        const auto inherited_method_lookup =
            find_native_object_class_method_lookup(
                runtime_object,
                normalized_method_name,
                {},
                {},
                true,
                qualified_method_name);
        const bool create_on_starting_class =
            create_if_missing &&
            starting_method == starting_class_lookup->class_definition->methods.end();
        const auto &method_lookup =
            create_on_starting_class
                ? std::optional<NativeMethodLookup>{}
                : inherited_method_lookup;
        if (!create_on_starting_class &&
            (!method_lookup.has_value() || method_lookup->routine == nullptr))
        {
            return false;
        }

        const std::string temp_routine_name = "__CopperfinWriteMethodTemp";
        const bool parse_as_function =
            method_lookup.has_value() &&
            method_lookup->routine != nullptr &&
            method_lookup->routine->kind == RoutineKind::function;
        std::error_code ignored;
        std::filesystem::create_directories(runtime_temp_directory, ignored);
        const std::filesystem::path temp_path =
            runtime_temp_directory /
            ("writemethod_" + std::to_string(runtime_instance_id) + "_" +
             std::to_string(static_cast<unsigned long long>(executed_statement_count)) + ".prg");

        {
            std::ofstream output(temp_path, std::ios::binary | std::ios::trunc);
            if (!output)
            {
                return false;
            }

            if (parse_as_function)
            {
                output << "FUNCTION " << temp_routine_name << "\n";
                output << method_source_text << "\n";
                output << "ENDFUNC\n";
            }
            else
            {
                output << "PROCEDURE " << temp_routine_name << "\n";
                output << method_source_text << "\n";
                output << "ENDPROC\n";
            }
        }

        Program parsed_program = parse_program(copperfin::platform::path_to_utf8_string(temp_path));
        std::filesystem::remove(temp_path, ignored);

        const auto parsed_method =
            parsed_program.routines.find(normalize_identifier(temp_routine_name));
        if (parsed_method == parsed_program.routines.end())
        {
            return false;
        }

        Routine updated_routine = parsed_method->second;
        const PrgClassDefinition *target_class_definition = nullptr;
        const Program *target_program = nullptr;
        if (create_on_starting_class)
        {
            updated_routine.name = trim_copy(method_name);
            updated_routine.kind = RoutineKind::procedure;
            updated_routine.declaration_location = {};
            updated_routine.body_end_line_exclusive = 0;
            target_class_definition = starting_class_lookup->class_definition;
            target_program = starting_class_lookup->program;

            std::size_t line_number =
                target_class_definition->declaration_location.line;
            for (Statement &statement : updated_routine.statements)
            {
                statement.location.file_path = target_program->path;
                statement.location.line = line_number;
                ++line_number;
            }
        }
        else
        {
            updated_routine.name = method_lookup->routine->name;
            updated_routine.kind = method_lookup->routine->kind;
            updated_routine.declaration_location =
                method_lookup->routine->declaration_location;
            updated_routine.body_end_line_exclusive =
                updated_routine.declaration_location.line + updated_routine.statements.size() + 1U;
            target_class_definition = method_lookup->class_definition;
            target_program = method_lookup->program;

            std::size_t line_number = updated_routine.declaration_location.line;
            for (Statement &statement : updated_routine.statements)
            {
                statement.location.file_path =
                    updated_routine.declaration_location.file_path;
                statement.location.line = line_number;
                ++line_number;
            }
        }

        if (target_class_definition == nullptr || target_program == nullptr)
        {
            return false;
        }

        auto &mutable_methods =
            const_cast<PrgClassDefinition *>(target_class_definition)->methods;
        mutable_methods[normalized_method_name] = updated_routine;
        const std::string defining_class_name =
            target_class_definition->name.empty()
                ? runtime_object.prog_id
                : target_class_definition->name;
        qualified_method_name = defining_class_name + "." + updated_routine.name;
        native_method_source_text_by_key[make_native_method_override_key(
            target_program->path,
            qualified_method_name)] = method_source_text;
        if (!runtime_object_member_matches(
                runtime_object.methods,
                normalized_method_name))
        {
            runtime_object.methods.push_back(updated_routine.name);
        }
        return true;
    }
