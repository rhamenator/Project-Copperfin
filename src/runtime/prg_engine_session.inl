// prg_engine_session.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        std::string display_asset_paths_in_statement(std::string text) const
        {
            const std::string trimmed = trim_copy(text);
            if (!starts_with_insensitive(trimmed, "REPORT FORM ") &&
                !starts_with_insensitive(trimmed, "LABEL FORM "))
            {
                return text;
            }
            for (const auto &[source_path, display_path] : options.source_path_display_aliases)
            {
                std::size_t position = 0U;
                while (!source_path.empty() &&
                       (position = text.find(source_path, position)) != std::string::npos)
                {
                    text.replace(position, source_path.size(), display_path);
                    position += display_path.size();
                }
            }
            return text;
        }

        std::map<std::string, std::string>::const_iterator find_source_text_override(
            const std::string &path) const
        {
            if (const auto exact = options.source_text_overrides.find(path);
                exact != options.source_text_overrides.end())
            {
                return exact;
            }
            return std::find_if(
                options.source_text_overrides.begin(),
                options.source_text_overrides.end(),
                [&](const auto &candidate)
                {
                    return paths_equal_insensitive(candidate.first, path);
                });
        }

        Program &load_program(const std::string &path)
        {
            const std::string normalized = normalize_path(path);
            const auto existing = programs.find(normalized);
            if (existing != programs.end())
            {
                return existing->second;
            }
            const bool use_startup_source_text =
                options.startup_source_text.has_value() &&
                normalized == normalize_path(options.startup_path);
            const auto source_override = find_source_text_override(normalized);
            if (!use_startup_source_text &&
                source_override == options.source_text_overrides.end() &&
                options.require_source_text_overrides)
            {
                throw std::runtime_error("verified source text unavailable: " + normalized);
            }
            auto [inserted, _] = programs.emplace(
                normalized,
                use_startup_source_text
                    ? parse_program_source(
                          normalized,
                          *options.startup_source_text,
                          options.source_text_overrides,
                          options.require_source_text_overrides)
                    : (source_override != options.source_text_overrides.end()
                           ? parse_program_source(
                                 normalized,
                                 source_override->second,
                                 options.source_text_overrides,
                                 options.require_source_text_overrides)
                           : parse_program(normalized)));
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

        void push_method_frame(
            const std::string &path,
            const std::string &routine_name,
            const Routine &routine,
            const PrgValue &this_reference,
            const std::string &native_method_class_name = {},
            const std::string &native_method_name = {},
            const std::optional<PrgValue> &parent_reference = std::nullopt,
            const std::optional<PrgValue> &owner_form_reference = std::nullopt,
            const std::optional<PrgValue> &owner_formset_reference = std::nullopt,
            std::vector<PrgValue> call_arguments = {},
            std::vector<std::optional<std::string>> call_argument_references = {})
        {
            Frame frame;
            frame.file_path = normalize_path(path);
            frame.routine_name = routine_name;
            frame.routine = &routine;
            frame.call_arguments = std::move(call_arguments);
            frame.call_argument_references = std::move(call_argument_references);
            frame.native_method_class_name = normalize_identifier(native_method_class_name);
            frame.native_method_name = normalize_identifier(native_method_name);
            frame.locals["this"] = this_reference;
            frame.local_names.insert("this");
            if (parent_reference.has_value())
            {
                frame.locals["parent"] = *parent_reference;
                frame.local_names.insert("parent");
            }
            if (owner_form_reference.has_value())
            {
                frame.locals["thisform"] = *owner_form_reference;
                frame.local_names.insert("thisform");
            }
            if (owner_formset_reference.has_value())
            {
                frame.locals["thisformset"] = *owner_formset_reference;
                frame.local_names.insert("thisformset");
            }
            else if (owner_form_reference.has_value())
            {
                frame.locals["thisformset"] = *owner_form_reference;
                frame.local_names.insert("thisformset");
            }
            stack.push_back(std::move(frame));
        }

        struct NativeClassLookup
        {
            const Program *program = nullptr;
            const PrgClassDefinition *class_definition = nullptr;
        };

        struct NativeMethodLookup
        {
            const Program *program = nullptr;
            const PrgClassDefinition *class_definition = nullptr;
            const Routine *routine = nullptr;
        };

        struct RoutineLookup
        {
            const Program *program = nullptr;
            const Routine *routine = nullptr;
        };

        std::string resolve_procedure_program_path(
            const std::string &target_text,
            const std::string &fallback_path = {}) const
        {
            std::string resolved_target = unquote_string(trim_copy(target_text));
            if (resolved_target.empty())
            {
                return {};
            }

            std::filesystem::path program_path(resolved_target);
            if (program_path.extension().empty())
            {
                program_path += ".prg";
            }

            return resolve_native_prg_program_path(program_path.string(), fallback_path);
        }

        std::optional<RoutineLookup> find_loaded_procedure_routine_lookup(
            const std::string &identifier,
            const std::string &exclude_program_path = {})
        {
            const std::string normalized_identifier = normalize_identifier(identifier);
            const std::string normalized_exclude_program_path = normalize_path(exclude_program_path);
            for (const std::string &procedure_program_path : procedure_program_paths)
            {
                if (!normalized_exclude_program_path.empty() &&
                    procedure_program_path == normalized_exclude_program_path)
                {
                    continue;
                }

                Program &program = load_program(procedure_program_path);
                const auto found = program.routines.find(normalized_identifier);
                if (found != program.routines.end())
                {
                    return RoutineLookup{.program = &program, .routine = &found->second};
                }
            }

            return std::nullopt;
        }

        std::optional<RoutineLookup> find_unqualified_routine_lookup(
            const std::string &source_file_path,
            const std::string &identifier)
        {
            Program &program = load_program(source_file_path);
            const auto found = program.routines.find(normalize_identifier(identifier));
            if (found != program.routines.end())
            {
                return RoutineLookup{.program = &program, .routine = &found->second};
            }

            return find_loaded_procedure_routine_lookup(identifier, program.path);
        }

        std::string native_same_prg_base_class_name(const std::string &base_class_name) const
        {
            const std::string trimmed = trim_copy(base_class_name);
            if (trimmed.empty())
            {
                return {};
            }

            std::size_t end = 0U;
            while (end < trimmed.size())
            {
                const char ch = trimmed[end];
                if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_')
                {
                    ++end;
                    continue;
                }
                break;
            }

            return end == 0U ? std::string{} : trimmed.substr(0U, end);
        }

        bool is_supported_native_base_class_name(const std::string &class_name) const
        {
            const std::string normalized_class_name =
                normalize_identifier(native_same_prg_base_class_name(class_name));
            return normalized_class_name == "checkbox" ||
                   normalized_class_name == "column" ||
                   normalized_class_name == "combobox" ||
                   normalized_class_name == "commandbutton" ||
                   normalized_class_name == "commandgroup" ||
                   normalized_class_name == "container" ||
                   normalized_class_name == "custom" ||
                   normalized_class_name == "editbox" ||
                   normalized_class_name == "form" ||
                   normalized_class_name == "grid" ||
                   normalized_class_name == "image" ||
                   normalized_class_name == "label" ||
                   normalized_class_name == "line" ||
                   normalized_class_name == "listbox" ||
                   normalized_class_name == "object" ||
                   normalized_class_name == "olecontrol" ||
                   normalized_class_name == "optionbutton" ||
                   normalized_class_name == "optiongroup" ||
                   normalized_class_name == "page" ||
                   normalized_class_name == "pageframe" ||
                   normalized_class_name == "separator" ||
                   normalized_class_name == "shape" ||
                   normalized_class_name == "spinner" ||
                   normalized_class_name == "textbox" ||
                   normalized_class_name == "timer" ||
                   normalized_class_name == "toolbar";
        }

        bool is_native_olecontrol_host_object(const RuntimeOleObjectState &runtime_object) const
        {
            return normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
                   normalize_identifier(runtime_object.prog_id) == "olecontrol";
        }

        void append_builtin_native_olecontrol_methods(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return;
            }

            if (std::find(runtime_object.methods.begin(), runtime_object.methods.end(), "doverb") ==
                runtime_object.methods.end())
            {
                runtime_object.methods.push_back("doverb");
            }
            if (std::find(runtime_object.methods.begin(), runtime_object.methods.end(), "objectverbs") ==
                runtime_object.methods.end())
            {
                runtime_object.methods.push_back("objectverbs");
            }
        }

        void assign_native_runtime_object_name(RuntimeOleObjectState &runtime_object,
                                               const std::string &name)
        {
            const std::string trimmed_name = trim_copy(name);
            if (trimmed_name.empty())
            {
                return;
            }

            runtime_object.properties["name"] = make_string_value(trimmed_name);
            if (!runtime_object.default_properties.empty())
            {
                runtime_object.default_properties["name"] = make_string_value(trimmed_name);
            }
        }

        bool is_native_grid_runtime_object(const RuntimeOleObjectState &runtime_object) const
        {
            return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
        }

        bool is_native_column_runtime_object(const RuntimeOleObjectState &runtime_object) const
        {
            return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
        }

        bool is_native_page_runtime_object(const RuntimeOleObjectState &runtime_object) const
        {
            return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "page";
        }

        bool is_native_pageframe_runtime_object(const RuntimeOleObjectState &runtime_object) const
        {
            return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "pageframe";
        }

        bool native_column_bound_value(
            const RuntimeOleObjectState &runtime_object,
            bool default_value = true) const
        {
            const auto bound = runtime_object.properties.find("bound");
            return bound == runtime_object.properties.end()
                       ? default_value
                       : value_as_bool(bound->second);
        }

        RuntimeOleObjectState *native_parent_column_object(RuntimeOleObjectState &runtime_object)
        {
            const auto parent_reference = native_object_parent_reference(runtime_object);
            if (!parent_reference.has_value())
            {
                return nullptr;
            }

            int parent_handle = 0;
            std::string parent_prog_id;
            if (!parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id))
            {
                return nullptr;
            }

            const auto parent_found = ole_objects.find(parent_handle);
            if (parent_found == ole_objects.end() ||
                !is_native_column_runtime_object(parent_found->second))
            {
                return nullptr;
            }

            return &parent_found->second;
        }

        void sync_native_column_child_controlsources(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_column_runtime_object(runtime_object))
            {
                return;
            }

            const auto control_source = runtime_object.properties.find("controlsource");
            if (control_source == runtime_object.properties.end())
            {
                return;
            }

            for (const int child_handle : collect_native_owned_child_handles(runtime_object))
            {
                const auto child_found = ole_objects.find(child_handle);
                if (child_found == ole_objects.end() ||
                    child_found->second.hidden_runtime_surface ||
                    !is_native_controlsource_member_name(child_found->second, "controlsource"))
                {
                    continue;
                }

                child_found->second.properties["controlsource"] = control_source->second;
            }
        }

        bool write_native_column_bound_property(
            RuntimeOleObjectState &runtime_object,
            const PrgValue &assigned_value)
        {
            if (!is_native_column_runtime_object(runtime_object))
            {
                return false;
            }

            runtime_object.properties["bound"] = make_boolean_value(value_as_bool(assigned_value));
            if (value_as_bool(assigned_value))
            {
                sync_native_column_child_controlsources(runtime_object);
            }
            return true;
        }

        bool write_native_column_controlsource_property(
            RuntimeOleObjectState &runtime_object,
            const PrgValue &assigned_value)
        {
            if (!is_native_column_runtime_object(runtime_object))
            {
                return false;
            }

            runtime_object.properties["controlsource"] = assigned_value;
            sync_native_column_child_controlsources(runtime_object);
            return true;
        }

        bool native_child_controlsource_write_blocked_by_parent_column(
            RuntimeOleObjectState &runtime_object)
        {
            RuntimeOleObjectState *parent_column = native_parent_column_object(runtime_object);
            return parent_column != nullptr &&
                   native_column_bound_value(*parent_column);
        }

        bool write_native_list_control_controlsource_target(
            RuntimeOleObjectState& runtime_object,
            const Frame& frame)
        {
            const std::string normalized_base_class =
                normalize_identifier(trim_copy(runtime_object.base_class_name));
            if (normalized_base_class != "combobox" &&
                normalized_base_class != "listbox") {
                return true;
            }

            const auto controlsource = runtime_object.properties.find("controlsource");
            if (controlsource == runtime_object.properties.end()) {
                return true;
            }

            const std::string controlsource_text =
                trim_copy(value_as_string(controlsource->second));
            if (controlsource_text.empty()) {
                return true;
            }

            const auto value = runtime_object.properties.find("value");
            const PrgValue assigned_value =
                value == runtime_object.properties.end()
                    ? make_empty_value()
                    : value->second;

            auto resolve_target_field = [&](std::string& field_name) -> CursorState* {
                std::string designator;
                field_name = controlsource_text;
                if (const std::size_t separator = controlsource_text.find('.');
                    separator != std::string::npos) {
                    designator = trim_copy(controlsource_text.substr(0U, separator));
                    field_name = trim_copy(controlsource_text.substr(separator + 1U));
                    if (!designator.empty()) {
                        return resolve_cursor_target(designator);
                    }
                }

                CursorState* current_cursor = resolve_cursor_target({});
                if (current_cursor == nullptr || field_name.empty()) {
                    return nullptr;
                }

                const auto descriptors = cursor_field_descriptors(*current_cursor);
                const auto descriptor = std::find_if(
                    descriptors.begin(),
                    descriptors.end(),
                    [&](const vfp::DbfFieldDescriptor& candidate) {
                        return collapse_identifier(candidate.name) ==
                               collapse_identifier(field_name);
                    });
                return descriptor == descriptors.end() ? nullptr : current_cursor;
            };

            std::string field_name;
            if (CursorState* target_cursor = resolve_target_field(field_name);
                target_cursor != nullptr && !field_name.empty()) {
                if (target_cursor->remote) {
                    if (target_cursor->recno == 0U || target_cursor->eof ||
                        target_cursor->recno > target_cursor->remote_records.size()) {
                        return false;
                    }

                    vfp::DbfRecord& record =
                        target_cursor->remote_records[target_cursor->recno - 1U];
                    const std::string normalized_field = collapse_identifier(field_name);
                    auto field = std::find_if(
                        record.values.begin(),
                        record.values.end(),
                        [&](vfp::DbfRecordValue& candidate) {
                            return collapse_identifier(candidate.field_name) ==
                                   normalized_field;
                        });
                    if (field == record.values.end()) {
                        return false;
                    }

                    field->display_value =
                        serialize_prg_value_for_record_field(*field, assigned_value);
                } else {
                    if (target_cursor->source_path.empty() ||
                        target_cursor->recno == 0U ||
                        target_cursor->eof) {
                        return false;
                    }
                    if (!ensure_transaction_backup_for_table(target_cursor->source_path)) {
                        return false;
                    }

                    bool temporary_record_lock = false;
                    if (!acquire_record_lock(
                            *target_cursor,
                            target_cursor->recno,
                            "REPLACE",
                            false,
                            temporary_record_lock)) {
                        return false;
                    }

                    std::string serialized_value = value_as_string(assigned_value);
                    const auto descriptors = cursor_field_descriptors(*target_cursor);
                    const auto descriptor = std::find_if(
                        descriptors.begin(),
                        descriptors.end(),
                        [&](const vfp::DbfFieldDescriptor& candidate) {
                            return collapse_identifier(candidate.name) ==
                                   collapse_identifier(field_name);
                        });
                    if (descriptor != descriptors.end() && descriptor->type == 'C') {
                        const std::string trimmed = trim_copy(serialized_value);
                        serialized_value =
                            trimmed.size() > descriptor->length
                                ? trimmed.substr(0U, descriptor->length)
                                : trimmed;
                    }

                    const auto result = vfp::replace_record_field_value(
                        target_cursor->source_path,
                        target_cursor->recno - 1U,
                        field_name,
                        serialized_value);
                    if (temporary_record_lock) {
                        unlock_cursor_record_lock(*target_cursor, target_cursor->recno);
                    }
                    if (!result.ok) {
                        last_error_message = result.error;
                        return false;
                    }
                    target_cursor->record_count = result.record_count;
                }

                runtime_object.controlsource_value_kind_hint = assigned_value.kind;
                return true;
            }

            Frame& mutable_frame = const_cast<Frame&>(frame);
            assign_variable(mutable_frame, controlsource_text, assigned_value);
            runtime_object.controlsource_value_kind_hint = assigned_value.kind;
            return true;
        }

        int normalize_native_grid_columncount_value(const PrgValue &value) const
        {
            long long normalized_count = -1LL;
            try
            {
                normalized_count = std::llround(value_as_number(value));
            }
            catch (...)
            {
                normalized_count = -1LL;
            }

            if (normalized_count < -1LL)
            {
                normalized_count = -1LL;
            }
            if (normalized_count > 255LL)
            {
                normalized_count = 255LL;
            }
            return static_cast<int>(normalized_count);
        }

        int normalize_native_pageframe_pagecount_value(const PrgValue &value) const
        {
            long long normalized_count = 0LL;
            try
            {
                normalized_count = std::llround(value_as_number(value));
            }
            catch (...)
            {
                normalized_count = 0LL;
            }

            if (normalized_count < 0LL)
            {
                normalized_count = 0LL;
            }
            if (normalized_count > 99LL)
            {
                normalized_count = 99LL;
            }
            return static_cast<int>(normalized_count);
        }

        int normalize_native_column_order_value(const PrgValue &value, int fallback = 1) const
        {
            long long normalized_order = fallback;
            try
            {
                normalized_order = std::llround(value_as_number(value));
            }
            catch (...)
            {
                normalized_order = fallback;
            }

            if (normalized_order < 1LL)
            {
                normalized_order = 1LL;
            }
            if (normalized_order > 2147483647LL)
            {
                normalized_order = 2147483647LL;
            }
            return static_cast<int>(normalized_order);
        }

        int next_native_grid_column_order(const RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_column_runtime_object(runtime_object))
            {
                return 1;
            }

            const auto parent_reference = native_object_parent_reference(runtime_object);
            if (!parent_reference.has_value())
            {
                return 1;
            }

            int parent_handle = 0;
            std::string parent_prog_id;
            if (!parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id))
            {
                return 1;
            }

            const auto parent_found = ole_objects.find(parent_handle);
            if (parent_found == ole_objects.end() ||
                !is_native_grid_runtime_object(parent_found->second))
            {
                return 1;
            }

            int max_order = 0;
            for (const int child_handle : collect_native_owned_child_handles(parent_found->second))
            {
                const auto child_found = ole_objects.find(child_handle);
                if (child_found == ole_objects.end() ||
                    !is_native_column_runtime_object(child_found->second))
                {
                    continue;
                }

                const auto order = child_found->second.properties.find("columnorder");
                if (order == child_found->second.properties.end())
                {
                    max_order = std::max(max_order, 1);
                    continue;
                }

                max_order = std::max(
                    max_order,
                    normalize_native_column_order_value(order->second, max_order + 1));
            }

            return std::max(1, max_order + 1);
        }

        int next_native_tab_index(const RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_tabindex_runtime_object(runtime_object))
            {
                return 0;
            }

            const auto parent_reference = native_object_parent_reference(runtime_object);
            if (!parent_reference.has_value())
            {
                return 0;
            }

            int parent_handle = 0;
            std::string parent_prog_id;
            if (!parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id))
            {
                return 0;
            }

            const auto parent_found = ole_objects.find(parent_handle);
            if (parent_found == ole_objects.end())
            {
                return 0;
            }

            std::size_t sibling_slot = 0U;
            for (const int child_handle : collect_native_owned_child_handles(parent_found->second))
            {
                if (child_handle == runtime_object.handle ||
                    child_handle > runtime_object.handle)
                {
                    continue;
                }

                const auto child_found = ole_objects.find(child_handle);
                if (child_found == ole_objects.end() ||
                    !is_native_tabindex_runtime_object(child_found->second))
                {
                    continue;
                }

                ++sibling_slot;
            }

            return static_cast<int>(std::min<std::size_t>(
                sibling_slot,
                static_cast<std::size_t>(2147483647U)));
        }

        bool write_native_columnorder_property(
            RuntimeOleObjectState &runtime_object,
            const PrgValue &assigned_value)
        {
            if (!is_native_column_runtime_object(runtime_object))
            {
                return false;
            }

            const int target_order = normalize_native_column_order_value(assigned_value, 1);
            const auto existing_order = runtime_object.properties.find("columnorder");
            const int current_order =
                existing_order == runtime_object.properties.end()
                    ? target_order
                    : normalize_native_column_order_value(existing_order->second, target_order);

            const auto parent_reference = native_object_parent_reference(runtime_object);
            if (!parent_reference.has_value())
            {
                runtime_object.properties["columnorder"] = make_number_value(static_cast<double>(target_order));
                return true;
            }

            int parent_handle = 0;
            std::string parent_prog_id;
            if (!parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id))
            {
                runtime_object.properties["columnorder"] = make_number_value(static_cast<double>(target_order));
                return true;
            }

            const auto parent_found = ole_objects.find(parent_handle);
            if (parent_found == ole_objects.end() ||
                !is_native_grid_runtime_object(parent_found->second))
            {
                runtime_object.properties["columnorder"] = make_number_value(static_cast<double>(target_order));
                return true;
            }

            std::vector<RuntimeOleObjectState *> sibling_columns;
            for (const int child_handle : collect_native_owned_child_handles(parent_found->second))
            {
                const auto child_found = ole_objects.find(child_handle);
                if (child_found == ole_objects.end() ||
                    !is_native_column_runtime_object(child_found->second))
                {
                    continue;
                }

                sibling_columns.push_back(&child_found->second);
            }

            if (target_order != current_order)
            {
                for (RuntimeOleObjectState *sibling_column : sibling_columns)
                {
                    if (sibling_column == nullptr || sibling_column->handle == runtime_object.handle)
                    {
                        continue;
                    }

                    const auto sibling_order = sibling_column->properties.find("columnorder");
                    const int sibling_value =
                        sibling_order == sibling_column->properties.end()
                            ? 1
                            : normalize_native_column_order_value(sibling_order->second, 1);

                    if (target_order > current_order &&
                        sibling_value > current_order &&
                        sibling_value <= target_order)
                    {
                        sibling_column->properties["columnorder"] =
                            make_number_value(static_cast<double>(sibling_value - 1));
                    }
                    else if (target_order < current_order &&
                             sibling_value >= target_order &&
                             sibling_value < current_order)
                    {
                        sibling_column->properties["columnorder"] =
                            make_number_value(static_cast<double>(sibling_value + 1));
                    }
                }
            }

            runtime_object.properties["columnorder"] = make_number_value(static_cast<double>(target_order));
            return true;
        }

        struct NativeGridColumnMember
        {
            std::string property_name;
            PrgValue child_reference;
            RuntimeOleObjectState *child_object = nullptr;
            int column_order = 1;
        };

        struct NativePageFramePageMember
        {
            std::string property_name;
            PrgValue child_reference;
            RuntimeOleObjectState *child_object = nullptr;
            bool has_numeric_page_slot = false;
            int numeric_page_slot = 0;
        };

        std::vector<NativeGridColumnMember> collect_native_grid_column_members(
            RuntimeOleObjectState &runtime_object)
        {
            std::vector<NativeGridColumnMember> members;
            if (!is_native_grid_runtime_object(runtime_object))
            {
                return members;
            }

            for (const auto &[property_name, property_value] : runtime_object.properties)
            {
                if (property_name == "parent" ||
                    property_name == "objects" ||
                    property_name == "controls" ||
                    property_name == "columns")
                {
                    continue;
                }

                const auto child_object = resolve_ole_object(property_value);
                if (!child_object.has_value() ||
                    (*child_object)->hidden_runtime_surface ||
                    !is_native_column_runtime_object(**child_object))
                {
                    continue;
                }

                const auto child_parent = native_object_parent_reference(**child_object);
                int parent_handle = 0;
                std::string parent_prog_id;
                if (!child_parent.has_value() ||
                    !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                    parent_handle != runtime_object.handle)
                {
                    continue;
                }

                int column_order = 1;
                const auto order = (*child_object)->properties.find("columnorder");
                if (order != (*child_object)->properties.end())
                {
                    column_order = normalize_native_column_order_value(order->second, 1);
                }

                members.push_back({
                    .property_name = property_name,
                    .child_reference = make_string_value(
                        "object:" + (*child_object)->prog_id + "#" + std::to_string((*child_object)->handle)),
                    .child_object = *child_object,
                    .column_order = column_order});
            }

            std::sort(
                members.begin(),
                members.end(),
                [](const NativeGridColumnMember &left, const NativeGridColumnMember &right)
                {
                    if (left.column_order != right.column_order)
                    {
                        return left.column_order < right.column_order;
                    }
                    return left.property_name < right.property_name;
                });
            return members;
        }

        std::vector<NativePageFramePageMember> collect_native_pageframe_page_members(
            RuntimeOleObjectState &runtime_object)
        {
            std::vector<NativePageFramePageMember> members;
            if (!is_native_pageframe_runtime_object(runtime_object))
            {
                return members;
            }

            for (const auto &[property_name, property_value] : runtime_object.properties)
            {
                if (property_name == "parent" ||
                    property_name == "objects" ||
                    property_name == "controls" ||
                    property_name == "pages")
                {
                    continue;
                }

                const auto child_object = resolve_ole_object(property_value);
                if (!child_object.has_value() ||
                    (*child_object)->hidden_runtime_surface ||
                    !is_native_page_runtime_object(**child_object))
                {
                    continue;
                }

                const auto child_parent = native_object_parent_reference(**child_object);
                int parent_handle = 0;
                std::string parent_prog_id;
                if (!child_parent.has_value() ||
                    !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                    parent_handle != runtime_object.handle)
                {
                    continue;
                }

                NativePageFramePageMember member{
                    .property_name = property_name,
                    .child_reference = make_string_value(
                        "object:" + (*child_object)->prog_id + "#" + std::to_string((*child_object)->handle)),
                    .child_object = *child_object};
                if (starts_with_insensitive(property_name, "page"))
                {
                    const std::string suffix = property_name.substr(4U);
                    if (!suffix.empty() &&
                        std::all_of(suffix.begin(), suffix.end(), [](unsigned char ch)
                                    { return std::isdigit(ch) != 0; }))
                    {
                        try
                        {
                            member.numeric_page_slot = std::stoi(suffix);
                            member.has_numeric_page_slot = member.numeric_page_slot > 0;
                        }
                        catch (...)
                        {
                            member.has_numeric_page_slot = false;
                            member.numeric_page_slot = 0;
                        }
                    }
                }
                members.push_back(std::move(member));
            }

            std::sort(
                members.begin(),
                members.end(),
                [](const NativePageFramePageMember &left, const NativePageFramePageMember &right)
                {
                    const int left_handle = left.child_object != nullptr ? left.child_object->handle : 0;
                    const int right_handle = right.child_object != nullptr ? right.child_object->handle : 0;
                    if (left_handle != right_handle)
                    {
                        return left_handle < right_handle;
                    }
                    if (left.has_numeric_page_slot != right.has_numeric_page_slot)
                    {
                        return left.has_numeric_page_slot < right.has_numeric_page_slot;
                    }
                    if (left.has_numeric_page_slot &&
                        right.has_numeric_page_slot &&
                        left.numeric_page_slot != right.numeric_page_slot)
                    {
                        return left.numeric_page_slot < right.numeric_page_slot;
                    }
                    return left.property_name < right.property_name;
                });
            return members;
        }

        void erase_native_object_subtree(int root_handle)
        {
            struct PendingErase
            {
                int handle = 0;
                bool children_queued = false;
            };

            std::vector<int> erase_order;
            std::vector<PendingErase> pending;
            std::set<int> scheduled_handles;
            pending.push_back({.handle = root_handle, .children_queued = false});
            scheduled_handles.insert(root_handle);

            while (!pending.empty())
            {
                const PendingErase current = pending.back();
                pending.pop_back();

                const auto found = ole_objects.find(current.handle);
                if (found == ole_objects.end())
                {
                    continue;
                }

                if (!current.children_queued)
                {
                    pending.push_back({.handle = current.handle, .children_queued = true});
                    const std::vector<int> child_handles =
                        collect_native_owned_child_handles(found->second);
                    for (auto it = child_handles.rbegin(); it != child_handles.rend(); ++it)
                    {
                        if (scheduled_handles.insert(*it).second)
                        {
                            pending.push_back({.handle = *it, .children_queued = false});
                        }
                    }
                    continue;
                }

                erase_order.push_back(current.handle);
            }

            for (const int handle : erase_order)
            {
                auto found = ole_objects.find(handle);
                if (found == ole_objects.end())
                {
                    continue;
                }

                RuntimeOleObjectState &object_state = found->second;
                if (const auto parent_reference = native_object_parent_reference(object_state);
                    parent_reference.has_value())
                {
                    int parent_handle = 0;
                    std::string parent_prog_id;
                    if (parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id))
                    {
                        const auto parent_found = ole_objects.find(parent_handle);
                        if (parent_found != ole_objects.end())
                        {
                            const std::string released_reference =
                                value_as_string(make_string_value(
                                    "object:" + object_state.prog_id + "#" +
                                    std::to_string(object_state.handle)));
                            auto &parent_properties = parent_found->second.properties;
                            for (auto property_it = parent_properties.begin();
                                 property_it != parent_properties.end();)
                            {
                                if (value_as_string(property_it->second) == released_reference)
                                {
                                    property_it = parent_properties.erase(property_it);
                                }
                                else
                                {
                                    ++property_it;
                                }
                            }
                            (void)sync_native_owned_children_collection(parent_found->second);
                        }
                    }
                }

                native_event_bindings.erase(
                    std::remove_if(
                        native_event_bindings.begin(),
                        native_event_bindings.end(),
                        [&](const NativeEventBinding &binding)
                        {
                            return binding.source_handle == handle ||
                                   binding.target_handle == handle;
                        }),
                    native_event_bindings.end());
                window_message_bindings.erase(
                    std::remove_if(
                        window_message_bindings.begin(),
                        window_message_bindings.end(),
                        [handle](const WindowMessageBinding &binding)
                        {
                            return binding.target_handle == handle;
                        }),
                    window_message_bindings.end());
                native_property_expression_text_by_handle.erase(handle);
                native_default_property_expression_text_by_handle.erase(handle);
                ole_objects.erase(found);
            }
        }

        bool write_native_grid_columncount_property(
            RuntimeOleObjectState &runtime_object,
            const PrgValue &assigned_value,
            const Frame &source_frame)
        {
            if (!is_native_grid_runtime_object(runtime_object))
            {
                return false;
            }

            const int target_count = normalize_native_grid_columncount_value(assigned_value);
            if (target_count < 0)
            {
                runtime_object.properties["columncount"] = make_number_value(static_cast<double>(target_count));
                (void)sync_native_owned_children_collection(runtime_object);
                return true;
            }

            auto column_members = collect_native_grid_column_members(runtime_object);
            while (static_cast<int>(column_members.size()) > target_count)
            {
                NativeGridColumnMember removed_member = column_members.back();
                column_members.pop_back();
                if (removed_member.child_object != nullptr)
                {
                    removed_member.child_object->properties.erase("parent");
                }
                runtime_object.properties.erase(removed_member.property_name);
            }

            const std::string owner_program_path =
                runtime_object.source.empty()
                    ? normalize_path(source_frame.file_path)
                    : normalize_path(runtime_object.source);
            if (static_cast<int>(column_members.size()) < target_count &&
                owner_program_path.empty())
            {
                return false;
            }

            runtime_object.properties["columncount"] = make_number_value(static_cast<double>(target_count));

            while (static_cast<int>(column_members.size()) < target_count)
            {
                int next_suffix = 1;
                while (runtime_object.properties.contains("column" + std::to_string(next_suffix)))
                {
                    ++next_suffix;
                }

                const std::string child_name_text = "Column" + std::to_string(next_suffix);
                const std::string child_name = normalize_identifier(child_name_text);
                RuntimeOleObjectState *child_object = instantiate_native_class_object(
                    source_frame,
                    "Column",
                    owner_program_path,
                    "grid.columncount",
                    {},
                    {},
                    make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle)));
                if (child_object == nullptr)
                {
                    return false;
                }

                assign_native_runtime_object_name(*child_object, child_name_text);
                runtime_object.properties[child_name] =
                    make_string_value("object:" + child_object->prog_id + "#" + std::to_string(child_object->handle));
                if (child_object->properties.contains("columnorder"))
                {
                    (void)write_native_columnorder_property(
                        *child_object,
                        child_object->properties["columnorder"]);
                }
                column_members = collect_native_grid_column_members(runtime_object);
            }

            for (std::size_t index = 0U; index < column_members.size(); ++index)
            {
                if (column_members[index].child_object != nullptr)
                {
                    (void)write_native_columnorder_property(
                        *column_members[index].child_object,
                        make_number_value(static_cast<double>(index + 1U)));
                }
            }

            (void)sync_native_owned_children_collection(runtime_object);
            runtime_object.properties["columncount"] =
                make_number_value(static_cast<double>(target_count));
            return true;
        }

        bool write_native_pageframe_pagecount_property(
            RuntimeOleObjectState &runtime_object,
            const PrgValue &assigned_value,
            const Frame &source_frame)
        {
            if (!is_native_pageframe_runtime_object(runtime_object))
            {
                return false;
            }

            const int target_count = normalize_native_pageframe_pagecount_value(assigned_value);
            auto page_members = collect_native_pageframe_page_members(runtime_object);
            while (static_cast<int>(page_members.size()) > target_count)
            {
                NativePageFramePageMember removed_member = page_members.back();
                page_members.pop_back();
                if (removed_member.child_object != nullptr)
                {
                    erase_native_object_subtree(removed_member.child_object->handle);
                }
                runtime_object.properties.erase(removed_member.property_name);
            }

            const std::string owner_program_path =
                runtime_object.source.empty()
                    ? normalize_path(source_frame.file_path)
                    : normalize_path(runtime_object.source);
            if (static_cast<int>(page_members.size()) < target_count &&
                owner_program_path.empty())
            {
                return false;
            }

            while (static_cast<int>(page_members.size()) < target_count)
            {
                int next_suffix = 1;
                while (runtime_object.properties.contains("page" + std::to_string(next_suffix)))
                {
                    ++next_suffix;
                }

                const std::string child_name_text = "Page" + std::to_string(next_suffix);
                const std::string child_name = normalize_identifier(child_name_text);
                RuntimeOleObjectState *child_object = instantiate_native_class_object(
                    source_frame,
                    "Page",
                    owner_program_path,
                    "pageframe.pagecount",
                    {},
                    {},
                    make_string_value(
                        "object:" + runtime_object.prog_id + "#" +
                        std::to_string(runtime_object.handle)));
                if (child_object == nullptr)
                {
                    return false;
                }

                assign_native_runtime_object_name(*child_object, child_name_text);
                runtime_object.properties[child_name] =
                    make_string_value("object:" + child_object->prog_id + "#" +
                                      std::to_string(child_object->handle));
                page_members = collect_native_pageframe_page_members(runtime_object);
            }

            runtime_object.properties["pagecount"] =
                make_number_value(static_cast<double>(target_count));
            (void)sync_native_owned_children_collection(runtime_object);
            runtime_object.properties["pagecount"] =
                make_number_value(static_cast<double>(target_count));
            normalize_native_pageframe_activepage_invariant(runtime_object);
            return true;
        }

        void seed_native_visual_properties(RuntimeOleObjectState &runtime_object)
        {
            const std::string normalized_base_class =
                normalize_identifier(trim_copy(runtime_object.base_class_name));

            if (is_native_visual_runtime_object(runtime_object) &&
                !is_native_olecontrol_host_object(runtime_object))
            {
                if (!runtime_object.properties.contains("left"))
                {
                    runtime_object.properties["left"] = make_number_value(0.0);
                }
                if (!runtime_object.properties.contains("top"))
                {
                    runtime_object.properties["top"] = make_number_value(0.0);
                }
                if (!runtime_object.properties.contains("width"))
                {
                    runtime_object.properties["width"] = make_number_value(0.0);
                }
                if (!runtime_object.properties.contains("height"))
                {
                    runtime_object.properties["height"] = make_number_value(0.0);
                }
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("visible"))
            {
                runtime_object.properties["visible"] = make_boolean_value(true);
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("enabled"))
            {
                runtime_object.properties["enabled"] = make_boolean_value(true);
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("backcolor"))
            {
                // Headless contract: seed an explicit deterministic visual background
                // until per-class VFP defaults are modeled with stronger evidence.
                runtime_object.properties["backcolor"] = make_int64_value(16777215);
            }

            if (is_native_visual_runtime_object(runtime_object) &&
                !is_native_olecontrol_host_object(runtime_object) &&
                !runtime_object.properties.contains("forecolor"))
            {
                // Headless contract: seed an explicit deterministic visual foreground
                // until per-class VFP defaults are modeled with stronger evidence.
                runtime_object.properties["forecolor"] = make_int64_value(0);
            }

            if (is_native_tabindex_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("tabindex"))
            {
                runtime_object.properties["tabindex"] =
                    make_number_value(static_cast<double>(next_native_tab_index(runtime_object)));
            }

            if (is_native_tabstop_runtime_object(runtime_object) &&
                !runtime_object.properties.contains("tabstop"))
            {
                runtime_object.properties["tabstop"] = make_boolean_value(true);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("value"))
            {
                runtime_object.properties["value"] = make_string_value("");
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox" ||
                 normalized_base_class == "column" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "checkbox" ||
                 normalized_base_class == "spinner") &&
                !runtime_object.properties.contains("controlsource"))
            {
                runtime_object.properties["controlsource"] = make_string_value("");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("currentcontrol"))
            {
                runtime_object.properties["currentcontrol"] = make_string_value("Text1");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("dynamiccurrentcontrol"))
            {
                runtime_object.properties["dynamiccurrentcontrol"] = make_string_value("Text1");
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("columnorder"))
            {
                runtime_object.properties["columnorder"] =
                    make_number_value(static_cast<double>(next_native_grid_column_order(runtime_object)));
            }

            if (normalized_base_class == "column" &&
                !runtime_object.properties.contains("bound"))
            {
                runtime_object.properties["bound"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("columncount"))
            {
                runtime_object.properties["columncount"] = make_number_value(-1.0);
            }

            if (normalized_base_class == "pageframe" &&
                !runtime_object.properties.contains("pagecount"))
            {
                runtime_object.properties["pagecount"] = make_number_value(0.0);
            }

            if (normalized_base_class == "pageframe" &&
                !runtime_object.properties.contains("activepage"))
            {
                runtime_object.properties["activepage"] = make_number_value(0.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("recordsource"))
            {
                runtime_object.properties["recordsource"] = make_string_value("");
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("allowcellselection"))
            {
                runtime_object.properties["allowcellselection"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("allowaddnew"))
            {
                runtime_object.properties["allowaddnew"] = make_boolean_value(false);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("gridlines"))
            {
                runtime_object.properties["gridlines"] = make_number_value(3.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("highlight"))
            {
                runtime_object.properties["highlight"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("highlightrow"))
            {
                runtime_object.properties["highlightrow"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("deletemark"))
            {
                runtime_object.properties["deletemark"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("splitbar"))
            {
                runtime_object.properties["splitbar"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("leftcolumn"))
            {
                runtime_object.properties["leftcolumn"] = make_number_value(1.0);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("recordmark"))
            {
                runtime_object.properties["recordmark"] = make_boolean_value(true);
            }

            if (normalized_base_class == "grid" &&
                !runtime_object.properties.contains("recordsourcetype"))
            {
                runtime_object.properties["recordsourcetype"] = make_number_value(1.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("rowsource"))
            {
                runtime_object.properties["rowsource"] = make_string_value("");
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("rowsourcetype"))
            {
                runtime_object.properties["rowsourcetype"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("listindex"))
            {
                runtime_object.properties["listindex"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("displayvalue"))
            {
                runtime_object.properties["displayvalue"] = make_string_value("");
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("listcount"))
            {
                runtime_object.properties["listcount"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("sorted"))
            {
                runtime_object.properties["sorted"] = make_boolean_value(false);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("boundto"))
            {
                runtime_object.properties["boundto"] = make_boolean_value(false);
            }

            if (normalized_base_class == "listbox" &&
                !runtime_object.properties.contains("multiselect"))
            {
                runtime_object.properties["multiselect"] = make_boolean_value(false);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("newindex"))
            {
                runtime_object.properties["newindex"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("newitemid"))
            {
                runtime_object.properties["newitemid"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("listitemid"))
            {
                runtime_object.properties["listitemid"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("boundcolumn"))
            {
                runtime_object.properties["boundcolumn"] = make_number_value(1.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("columncount"))
            {
                runtime_object.properties["columncount"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "combobox" ||
                 normalized_base_class == "listbox") &&
                !runtime_object.properties.contains("columnwidths"))
            {
                runtime_object.properties["columnwidths"] = make_string_value("");
            }

            if (normalized_base_class == "combobox" &&
                !runtime_object.properties.contains("style"))
            {
                runtime_object.properties["style"] = make_number_value(0.0);
            }

            if ((normalized_base_class == "textbox" ||
                 normalized_base_class == "combobox" ||
                 normalized_base_class == "editbox" ||
                 normalized_base_class == "grid" ||
                 normalized_base_class == "column" ||
                 normalized_base_class == "checkbox" ||
                 normalized_base_class == "spinner") &&
                !runtime_object.properties.contains("readonly"))
            {
                runtime_object.properties["readonly"] = make_boolean_value(false);
            }

            normalize_native_combobox_readonly_invariant(runtime_object);
            normalize_native_list_control_sorted_invariant(runtime_object);
            normalize_native_listbox_multiselect_invariant(runtime_object);
            normalize_native_pageframe_activepage_invariant(runtime_object);

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("showwindow"))
            {
                runtime_object.properties["showwindow"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("windowtype"))
            {
                runtime_object.properties["windowtype"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("windowstate"))
            {
                runtime_object.properties["windowstate"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("borderstyle"))
            {
                runtime_object.properties["borderstyle"] = make_number_value(3.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("titlebar"))
            {
                runtime_object.properties["titlebar"] = make_number_value(1.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("desktop"))
            {
                runtime_object.properties["desktop"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("scrollbars"))
            {
                runtime_object.properties["scrollbars"] = make_number_value(0.0);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("alwaysontop"))
            {
                runtime_object.properties["alwaysontop"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("lockscreen"))
            {
                runtime_object.properties["lockscreen"] = make_boolean_value(false);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("controlbox"))
            {
                runtime_object.properties["controlbox"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("closable"))
            {
                runtime_object.properties["closable"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("minbutton"))
            {
                runtime_object.properties["minbutton"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("maxbutton"))
            {
                runtime_object.properties["maxbutton"] = make_boolean_value(true);
            }

            if (normalized_base_class == "form" &&
                !runtime_object.properties.contains("autocenter"))
            {
                runtime_object.properties["autocenter"] = make_boolean_value(false);
            }
        }

        void seed_native_olecontrol_timeout_policy_properties(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return;
            }

            if (!runtime_object.properties.contains("olerequestpendingtimeout"))
            {
                runtime_object.properties["olerequestpendingtimeout"] = make_int64_value(5000);
            }
            if (!runtime_object.properties.contains("oleserverbusytimeout"))
            {
                // VFP help documents the busy-timeout surface but not an explicit
                // default, so keep the native OleControl lane deterministic with a
                // representative 5-second retry window.
                runtime_object.properties["oleserverbusytimeout"] = make_int64_value(5000);
            }
            if (!runtime_object.properties.contains("oleserverbusyraiseerror"))
            {
                runtime_object.properties["oleserverbusyraiseerror"] = make_boolean_value(false);
            }
        }

        void seed_native_olecontrol_verb_inspection_properties(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return;
            }

            runtime_object.properties["objectverbscount"] = make_int64_value(2);
        }

        std::string representative_olecontrol_application_name(const std::string &oleclass) const
        {
            const std::string normalized = normalize_identifier(trim_copy(oleclass));
            if (normalized.rfind("excel.", 0U) == 0U)
            {
                return "Microsoft Excel";
            }
            if (normalized.rfind("word.", 0U) == 0U)
            {
                return "Microsoft Word";
            }
            return trim_copy(oleclass).empty() ? std::string("OLE Application") : trim_copy(oleclass);
        }

        std::string representative_olecontrol_application_progid(const std::string &oleclass) const
        {
            const std::string trimmed = trim_copy(oleclass);
            const std::size_t separator = trimmed.find('.');
            if (separator == std::string::npos || separator == 0U)
            {
                return trimmed.empty() ? std::string("Application") : trimmed + ".Application";
            }
            return trimmed.substr(0U, separator) + ".Application";
        }

        RuntimeOleObjectState *ensure_native_olecontrol_application_conflict_surface(
            RuntimeOleObjectState &runtime_object,
            RuntimeOleObjectState &object_surface)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return nullptr;
            }

            const auto existing_host_application = runtime_object.properties.find("application");
            if (existing_host_application != runtime_object.properties.end())
            {
                const auto nested = resolve_ole_object(existing_host_application->second);
                if (nested.has_value())
                {
                    return *nested;
                }
            }

            const auto oleclass = runtime_object.properties.find("oleclass");
            const std::string automation_prog_id =
                oleclass == runtime_object.properties.end() ? std::string{} : trim_copy(value_as_string(oleclass->second));

            const int host_application_handle = next_ole_handle++;
            RuntimeOleObjectState host_application_surface{
                .handle = host_application_handle,
                .prog_id = "Microsoft Visual FoxPro",
                .source = {},
                .last_action = "application",
                .action_count = 1};
            host_application_surface.properties["name"] = make_string_value("Microsoft Visual FoxPro");
            auto [host_application_it, _] = ole_objects.emplace(host_application_handle, std::move(host_application_surface));
            runtime_object.properties["application"] =
                make_string_value("object:" + host_application_it->second.prog_id + "#" +
                                  std::to_string(host_application_it->second.handle));

            if (!object_surface.properties.contains("application"))
            {
                const int object_application_handle = next_ole_handle++;
                RuntimeOleObjectState object_application_surface{
                    .handle = object_application_handle,
                    .prog_id = representative_olecontrol_application_progid(automation_prog_id),
                    .source = {},
                    .last_action = "application",
                    .action_count = 1};
                object_application_surface.properties["name"] =
                    make_string_value(representative_olecontrol_application_name(automation_prog_id));
                auto [object_application_it, __] = ole_objects.emplace(object_application_handle, std::move(object_application_surface));
                object_surface.properties["application"] =
                    make_string_value("object:" + object_application_it->second.prog_id + "#" +
                                      std::to_string(object_application_it->second.handle));
            }

            return &host_application_it->second;
        }

        RuntimeOleObjectState *ensure_native_olecontrol_object_surface(RuntimeOleObjectState &runtime_object)
        {
            if (!is_native_olecontrol_host_object(runtime_object))
            {
                return nullptr;
            }

            const auto existing_object = runtime_object.properties.find("object");
            if (existing_object != runtime_object.properties.end())
            {
                const auto nested = resolve_ole_object(existing_object->second);
                if (nested.has_value())
                {
                    return *nested;
                }
            }

            const auto oleclass = runtime_object.properties.find("oleclass");
            if (oleclass == runtime_object.properties.end())
            {
                return nullptr;
            }

            const std::string automation_prog_id = trim_copy(value_as_string(oleclass->second));
            if (automation_prog_id.empty())
            {
                return nullptr;
            }

            const int handle = next_ole_handle++;
            RuntimeOleObjectState object_surface{
                .handle = handle,
                .prog_id = automation_prog_id,
                .source = {},
                .last_action = "object",
                .action_count = 1};
            object_surface.properties["left"] =
                make_string_value("ole:" + automation_prog_id + ".left");
            object_surface.properties["visible"] =
                make_string_value("ole:" + automation_prog_id + ".visible");
            object_surface.methods.push_back("compose");
            auto [object_it, _] = ole_objects.emplace(handle, std::move(object_surface));
            runtime_object.properties["object"] =
                make_string_value("object:" + object_it->second.prog_id + "#" + std::to_string(object_it->second.handle));
            (void)ensure_native_olecontrol_application_conflict_surface(runtime_object, object_it->second);
            return &object_it->second;
        }

        RuntimeOleObjectState *sync_native_owned_children_collection(RuntimeOleObjectState &runtime_object)
        {
            std::vector<std::pair<std::string, PrgValue>> child_members;
            std::vector<std::pair<std::string, PrgValue>> column_members;
            std::vector<std::pair<std::string, PrgValue>> page_members;
            child_members.reserve(runtime_object.properties.size());
            for (const auto &[property_name, property_value] : runtime_object.properties)
            {
                if (property_name == "parent" ||
                    property_name == "objects" ||
                    property_name == "controls" ||
                    property_name == "columns" ||
                    property_name == "pages")
                {
                    continue;
                }
                if (property_name == "object" && is_native_olecontrol_host_object(runtime_object))
                {
                    continue;
                }

                const auto child_object = resolve_ole_object(property_value);
                if (!child_object.has_value() || (*child_object)->hidden_runtime_surface)
                {
                    continue;
                }

                const auto child_parent = native_object_parent_reference(**child_object);
                int parent_handle = 0;
                std::string parent_prog_id;
                if (!child_parent.has_value() ||
                    !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                    parent_handle != runtime_object.handle)
                {
                    continue;
                }

                child_members.emplace_back(
                    property_name,
                    make_string_value("object:" + (*child_object)->prog_id + "#" + std::to_string((*child_object)->handle)));
                if (is_native_grid_runtime_object(runtime_object) &&
                    is_native_column_runtime_object(**child_object))
                {
                    column_members.emplace_back(
                        property_name,
                        make_string_value("object:" + (*child_object)->prog_id + "#" + std::to_string((*child_object)->handle)));
                }
            }

            RuntimeOleObjectState *objects_collection = nullptr;
            const auto sync_collection_surface = [&](const std::string &property_name,
                                                    const std::vector<std::pair<std::string, PrgValue>> &members,
                                                    bool ensure_surface) -> RuntimeOleObjectState *
            {
                RuntimeOleObjectState *collection_object = nullptr;
                const auto collection_property = runtime_object.properties.find(property_name);
                if (collection_property != runtime_object.properties.end())
                {
                    const auto nested = resolve_ole_object(collection_property->second);
                    if (nested.has_value() &&
                        is_native_collection_object(**nested) &&
                        (*nested)->hidden_runtime_surface &&
                        (*nested)->read_only_collection_surface)
                    {
                        collection_object = *nested;
                    }
                }

                if (collection_object == nullptr && members.empty() && !ensure_surface)
                {
                    return nullptr;
                }

                if (collection_object == nullptr)
                {
                    const int handle = next_ole_handle++;
                    RuntimeOleObjectState collection_state{
                        .handle = handle,
                        .prog_id = "Collection",
                        .source = {},
                        .last_action = property_name,
                        .action_count = 1,
                        .hidden_runtime_surface = true,
                        .read_only_collection_surface = true};
                    collection_state.base_class_name = "Collection";
                    collection_state.class_hierarchy = {"COLLECTION", "OBJECT"};
                    collection_state.properties["parent"] =
                        make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle));
                    auto [collection_it, _] = ole_objects.emplace(handle, std::move(collection_state));
                    runtime_object.properties[property_name] =
                        make_string_value("object:" + collection_it->second.prog_id + "#" + std::to_string(collection_it->second.handle));
                    collection_object = &collection_it->second;
                }

                collection_object->properties["parent"] =
                    make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle));
                collection_object->collection_items.clear();
                collection_object->collection_item_keys.clear();
                collection_object->collection_items.reserve(members.size());
                collection_object->collection_item_keys.reserve(members.size());
                for (const auto &[child_name, child_reference] : members)
                {
                    collection_object->collection_items.push_back(child_reference);
                    collection_object->collection_item_keys.push_back(child_name);
                }
                (void)read_native_collection_member(*collection_object, "count");
                return collection_object;
            };

            if (!column_members.empty())
            {
                std::sort(
                    column_members.begin(),
                    column_members.end(),
                    [&](const auto &left, const auto &right)
                    {
                        const auto left_child = resolve_ole_object(left.second);
                        const auto right_child = resolve_ole_object(right.second);
                        const int left_order =
                            left_child.has_value() &&
                                    (*left_child)->properties.contains("columnorder")
                                ? normalize_native_column_order_value(
                                      (*left_child)->properties["columnorder"],
                                      1)
                                : 1;
                        const int right_order =
                            right_child.has_value() &&
                                    (*right_child)->properties.contains("columnorder")
                                ? normalize_native_column_order_value(
                                      (*right_child)->properties["columnorder"],
                                      1)
                                : 1;
                        if (left_order != right_order)
                        {
                            return left_order < right_order;
                        }
                        return left.first < right.first;
                    });
            }
            if (!child_members.empty())
            {
                std::sort(
                    child_members.begin(),
                    child_members.end(),
                    [&](const auto &left, const auto &right)
                    {
                        const auto left_child = resolve_ole_object(left.second);
                        const auto right_child = resolve_ole_object(right.second);
                        const int left_handle = left_child.has_value() ? (*left_child)->handle : 0;
                        const int right_handle = right_child.has_value() ? (*right_child)->handle : 0;
                        if (left_handle != right_handle)
                        {
                            return left_handle < right_handle;
                        }
                        return left.first < right.first;
                    });
            }
            if (is_native_pageframe_runtime_object(runtime_object))
            {
                for (const NativePageFramePageMember &page_member :
                     collect_native_pageframe_page_members(runtime_object))
                {
                    page_members.emplace_back(page_member.property_name, page_member.child_reference);
                }
            }

            objects_collection = sync_collection_surface("objects", child_members, false);
            RuntimeOleObjectState *controls_collection =
                sync_collection_surface("controls", child_members, false);
            RuntimeOleObjectState *columns_collection =
                is_native_grid_runtime_object(runtime_object)
                    ? sync_collection_surface("columns", column_members, true)
                    : nullptr;
            RuntimeOleObjectState *pages_collection =
                is_native_pageframe_runtime_object(runtime_object)
                    ? sync_collection_surface("pages", page_members, true)
                    : nullptr;
            if (controls_collection != nullptr)
            {
                runtime_object.properties["controlcount"] =
                    make_number_value(static_cast<double>(controls_collection->collection_items.size()));
            }
            else
            {
                runtime_object.properties.erase("controlcount");
            }
            if (columns_collection != nullptr &&
                runtime_object.properties.contains("columncount") &&
                normalize_native_grid_columncount_value(runtime_object.properties["columncount"]) >= 0)
            {
                runtime_object.properties["columncount"] =
                    make_number_value(static_cast<double>(columns_collection->collection_items.size()));
            }
            if (pages_collection != nullptr)
            {
                runtime_object.properties["pagecount"] =
                    make_number_value(static_cast<double>(pages_collection->collection_items.size()));
                normalize_native_pageframe_activepage_invariant(runtime_object);
            }
            return objects_collection;
        }

        std::string resolve_native_prg_program_path(
            const std::string &source_path,
            const std::string &fallback_path = {}) const
        {
            const std::string trimmed_source_path = trim_copy(source_path);
            if (trimmed_source_path.empty())
            {
                return normalize_path(fallback_path);
            }

            std::filesystem::path program_path(trimmed_source_path);
            if (program_path.is_absolute())
            {
                return program_path.lexically_normal().string();
            }

            const std::filesystem::path default_directory_candidate =
                (std::filesystem::path(current_default_directory()) / program_path).lexically_normal();
            std::error_code ignored;
            if (std::filesystem::exists(default_directory_candidate, ignored))
            {
                return default_directory_candidate.string();
            }

            const std::string normalized_fallback_path = normalize_path(fallback_path);
            if (!normalized_fallback_path.empty())
            {
                const std::filesystem::path fallback_directory_candidate =
                    (std::filesystem::path(normalized_fallback_path).parent_path() / program_path).lexically_normal();
                if (std::filesystem::exists(fallback_directory_candidate, ignored))
                {
                    return fallback_directory_candidate.string();
                }
            }

            return program_path.lexically_normal().string();
        }

        const PrgClassDefinition *find_native_same_prg_class(
            const Program &program,
            const std::string &class_name) const
        {
            const auto found = program.classes.find(normalize_identifier(class_name));
            return found == program.classes.end() ? nullptr : &found->second;
        }

        std::optional<NativeClassLookup> find_native_class_lookup(
            const Program &program,
            const std::string &class_name) const
        {
            const PrgClassDefinition *class_definition =
                find_native_same_prg_class(program, class_name);
            return class_definition == nullptr
                ? std::nullopt
                : std::optional<NativeClassLookup>({.program = &program, .class_definition = class_definition});
        }

        std::optional<NativeClassLookup> find_native_base_class_lookup(
            const Program &program,
            const PrgClassDefinition &class_definition)
        {
            const std::string base_class_name =
                native_same_prg_base_class_name(class_definition.base_class_name);
            if (base_class_name.empty())
            {
                return std::nullopt;
            }

            if (trim_copy(class_definition.base_class_source_path).empty())
            {
                return find_native_class_lookup(program, base_class_name);
            }

            const std::string resolved_program_path =
                resolve_native_prg_program_path(class_definition.base_class_source_path, program.path);
            if (resolved_program_path.empty())
            {
                return std::nullopt;
            }

            Program &base_program = load_program(resolved_program_path);
            return find_native_class_lookup(base_program, base_class_name);
        }

        std::vector<NativeClassLookup> collect_native_class_lineage(
            const Program &program,
            const std::string &class_name)
        {
            std::vector<NativeClassLookup> reverse_lineage;
            std::set<std::string> visited;
            std::optional<NativeClassLookup> current =
                find_native_class_lookup(program, class_name);
            while (current.has_value())
            {
                const std::string normalized_name =
                    normalize_identifier(current->class_definition->name);
                const std::string visit_key =
                    normalize_path(current->program->path) + ":" + normalized_name;
                if (!normalized_name.empty() && !visited.insert(visit_key).second)
                {
                    break;
                }

                reverse_lineage.push_back(*current);
                current = find_native_base_class_lookup(
                    *current->program,
                    *current->class_definition);
            }

            return std::vector<NativeClassLookup>(
                reverse_lineage.rbegin(),
                reverse_lineage.rend());
        }

        std::optional<NativeMethodLookup> find_native_class_method_lookup(
            const Program &program,
            const std::string &class_name,
            const std::string &member_name,
            bool include_starting_class,
            std::string &qualified_routine_name,
            std::string *defining_class_name = nullptr)
        {
            const std::string normalized_member_name = normalize_identifier(member_name);
            std::optional<NativeClassLookup> current_class =
                find_native_class_lookup(program, class_name);
            if (!current_class.has_value())
            {
                return std::nullopt;
            }
            if (!include_starting_class)
            {
                current_class = find_native_base_class_lookup(
                    *current_class->program,
                    *current_class->class_definition);
            }

            std::set<std::string> visited;
            while (current_class.has_value())
            {
                const std::string normalized_class_name =
                    normalize_identifier(current_class->class_definition->name);
                const std::string visit_key =
                    normalize_path(current_class->program->path) + ":" + normalized_class_name;
                if (!normalized_class_name.empty() &&
                    !visited.insert(visit_key).second)
                {
                    break;
                }

                const auto method_found =
                    current_class->class_definition->methods.find(normalized_member_name);
                if (method_found != current_class->class_definition->methods.end())
                {
                    const std::string resolved_class_name =
                        current_class->class_definition->name.empty()
                            ? class_name
                            : current_class->class_definition->name;
                    if (defining_class_name != nullptr)
                    {
                        *defining_class_name = resolved_class_name;
                    }
                    qualified_routine_name = resolved_class_name + "." + method_found->second.name;
                    return NativeMethodLookup{
                        .program = current_class->program,
                        .class_definition = current_class->class_definition,
                        .routine = &method_found->second};
                }

                current_class = find_native_base_class_lookup(
                    *current_class->program,
                    *current_class->class_definition);
            }

            return std::nullopt;
        }

        const Routine *find_native_object_method(
            const RuntimeOleObjectState &runtime_object,
            const std::string &member_name,
            std::string &program_path,
            std::string &qualified_routine_name)
        {
            if (runtime_object.source.empty())
            {
                return nullptr;
            }

            Program &program = load_program(runtime_object.source);
            program_path = program.path;
            const auto method_lookup =
                find_native_class_method_lookup(
                    program,
                    runtime_object.prog_id,
                    member_name,
                    true,
                    qualified_routine_name);
            if (!method_lookup.has_value())
            {
                return nullptr;
            }

            program_path = method_lookup->program->path;
            return method_lookup->routine;
        }

        std::optional<PrgValue> native_object_parent_reference(
            const RuntimeOleObjectState &runtime_object) const
        {
            const auto parent = runtime_object.properties.find("parent");
            if (parent == runtime_object.properties.end())
            {
                return std::nullopt;
            }

            int handle = 0;
            std::string prog_id;
            return parse_object_handle_reference(parent->second, handle, prog_id)
                ? std::optional<PrgValue>(parent->second)
                : std::nullopt;
        }

        std::optional<PrgValue> native_object_owner_form_reference(
            const RuntimeOleObjectState &runtime_object) const
        {
            const auto make_runtime_object_reference = [](const RuntimeOleObjectState &object_state) -> PrgValue
            {
                return make_string_value("object:" + object_state.prog_id + "#" + std::to_string(object_state.handle));
            };
            if (normalize_identifier(runtime_object.base_class_name) == "form")
            {
                return make_runtime_object_reference(runtime_object);
            }

            auto current_reference = native_object_parent_reference(runtime_object);
            std::optional<PrgValue> highest_owner_reference;
            std::set<int> visited_handles;
            while (current_reference.has_value())
            {
                int handle = 0;
                std::string prog_id;
                if (!parse_object_handle_reference(*current_reference, handle, prog_id))
                {
                    return std::nullopt;
                }
                if (!visited_handles.insert(handle).second)
                {
                    return current_reference;
                }

                highest_owner_reference = current_reference;
                const auto found = ole_objects.find(handle);
                if (found == ole_objects.end())
                {
                    return highest_owner_reference;
                }
                if (normalize_identifier(found->second.base_class_name) == "form")
                {
                    return current_reference;
                }

                const auto parent_reference = native_object_parent_reference(found->second);
                if (!parent_reference.has_value())
                {
                    return highest_owner_reference;
                }
                current_reference = parent_reference;
            }

            return highest_owner_reference;
        }

        std::optional<PrgValue> native_object_owner_formset_reference(
            const RuntimeOleObjectState &runtime_object) const
        {
            const auto make_runtime_object_reference = [](const RuntimeOleObjectState &object_state) -> PrgValue
            {
                return make_string_value("object:" + object_state.prog_id + "#" + std::to_string(object_state.handle));
            };
            if (normalize_identifier(runtime_object.base_class_name) == "formset")
            {
                return make_runtime_object_reference(runtime_object);
            }

            auto current_reference = native_object_parent_reference(runtime_object);
            std::set<int> visited_handles;
            while (current_reference.has_value())
            {
                int handle = 0;
                std::string prog_id;
                if (!parse_object_handle_reference(*current_reference, handle, prog_id))
                {
                    return std::nullopt;
                }
                if (!visited_handles.insert(handle).second)
                {
                    return current_reference;
                }

                const auto found = ole_objects.find(handle);
                if (found == ole_objects.end())
                {
                    return std::nullopt;
                }
                if (normalize_identifier(found->second.base_class_name) == "formset")
                {
                    return current_reference;
                }

                current_reference = native_object_parent_reference(found->second);
            }

            return native_object_owner_form_reference(runtime_object);
        }

        struct ResolvedRuntimeObjectMemberPath
        {
            RuntimeOleObjectState *runtime_object = nullptr;
            std::string remaining_member_path;
        };

        ResolvedRuntimeObjectMemberPath resolve_runtime_object_member_path(
            RuntimeOleObjectState *runtime_object,
            const std::string &member_path)
        {
            RuntimeOleObjectState *current_object = runtime_object;
            if (current_object == nullptr)
            {
                return {};
            }

            std::vector<std::string> segments;
            std::size_t start = 0U;
            while (start <= member_path.size())
            {
                const std::size_t separator = member_path.find('.', start);
                std::string segment = separator == std::string::npos
                                          ? member_path.substr(start)
                                          : member_path.substr(start, separator - start);
                segment = trim_copy(segment);
                if (!segment.empty())
                {
                    segments.push_back(segment);
                }
                if (separator == std::string::npos)
                {
                    break;
                }
                start = separator + 1U;
            }

            if (segments.empty())
            {
                return {.runtime_object = current_object, .remaining_member_path = member_path};
            }

            std::size_t consumed_segments = 0U;
            for (std::size_t index = 0U; index + 1U < segments.size(); ++index)
            {
                const std::string property_name = normalize_identifier(segments[index]);
                const auto property = current_object->properties.find(property_name);
                if (property == current_object->properties.end())
                {
                    break;
                }

                const auto nested_object = resolve_ole_object(property->second);
                if (!nested_object.has_value())
                {
                    break;
                }

                current_object = *nested_object;
                consumed_segments = index + 1U;
            }

            std::string remaining_member_path;
            for (std::size_t index = consumed_segments; index < segments.size(); ++index)
            {
                if (!remaining_member_path.empty())
                {
                    remaining_member_path += '.';
                }
                remaining_member_path += segments[index];
            }

            return {.runtime_object = current_object, .remaining_member_path = remaining_member_path};
        }

        ResolvedRuntimeObjectMemberPath resolve_runtime_object_member_path(
            const Frame &frame,
            const std::string &base_name,
            const std::string &member_path)
        {
            const PrgValue object_value = lookup_variable(frame, base_name);
            auto object = resolve_ole_object(object_value);
            return resolve_runtime_object_member_path(
                object.has_value() ? *object : nullptr,
                member_path);
        }

        std::vector<int> collect_native_owned_child_handles(const RuntimeOleObjectState &runtime_object)
        {
            std::vector<int> child_handles;
            for (const auto &[property_name, property_value] : runtime_object.properties)
            {
                if (property_name == "object" && is_native_olecontrol_host_object(runtime_object))
                {
                    const auto child_object = resolve_ole_object(property_value);
                    if (child_object.has_value())
                    {
                        child_handles.push_back((*child_object)->handle);
                    }
                    continue;
                }

                if (property_name == "parent")
                {
                    continue;
                }

                const auto child_object = resolve_ole_object(property_value);
                if (!child_object.has_value())
                {
                    continue;
                }

                const auto child_parent = native_object_parent_reference(**child_object);
                int parent_handle = 0;
                std::string parent_prog_id;
                if (!child_parent.has_value() ||
                    !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                    parent_handle != runtime_object.handle)
                {
                    continue;
                }

                child_handles.push_back((*child_object)->handle);
            }

            return child_handles;
        }

        std::vector<int> collect_native_setall_child_handles(const RuntimeOleObjectState &runtime_object)
        {
            std::vector<int> child_handles;
            for (const auto &[property_name, property_value] : runtime_object.properties)
            {
                if (property_name == "parent")
                {
                    continue;
                }

                const auto child_object = resolve_ole_object(property_value);
                if (!child_object.has_value())
                {
                    continue;
                }
                if ((*child_object)->hidden_runtime_surface)
                {
                    continue;
                }

                const auto child_parent = native_object_parent_reference(**child_object);
                int parent_handle = 0;
                std::string parent_prog_id;
                if (!child_parent.has_value() ||
                    !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                    parent_handle != runtime_object.handle)
                {
                    continue;
                }

                child_handles.push_back((*child_object)->handle);
            }

            return child_handles;
        }

        bool native_setall_candidate_has_writable_property(
            RuntimeOleObjectState &runtime_object,
            const std::string &normalized_property_name)
        {
            if (normalized_property_name.empty())
            {
                return false;
            }

            if (is_native_identity_member_name(runtime_object, normalized_property_name) ||
                is_native_controlcount_member_name(runtime_object, normalized_property_name) ||
                is_native_pagecount_member_name(runtime_object, normalized_property_name) ||
                is_native_child_collection_member_name(runtime_object, normalized_property_name) ||
                is_native_name_member_name(runtime_object, normalized_property_name) ||
                is_native_splitbar_member_name(runtime_object, normalized_property_name) ||
                is_native_leftcolumn_member_name(runtime_object, normalized_property_name) ||
                is_native_olecontrol_creation_time_member_name(runtime_object, normalized_property_name) ||
                is_native_olecontrol_object_member_name(runtime_object, normalized_property_name) ||
                is_native_olecontrol_inspection_member_name(runtime_object, normalized_property_name) ||
                is_native_olecontrol_conflict_member_name(runtime_object, normalized_property_name) ||
                is_native_child_parent_member_name(runtime_object, normalized_property_name) ||
                is_native_form_desktop_member_name(runtime_object, normalized_property_name) ||
                is_native_form_scrollbars_member_name(runtime_object, normalized_property_name) ||
                is_native_collection_readonly_member_name(runtime_object, normalized_property_name))
            {
                return false;
            }

            if (runtime_object.properties.contains(normalized_property_name) ||
                runtime_object_has_assigner_property(runtime_object, normalized_property_name))
            {
                return true;
            }

            if (runtime_object_has_accessor_property(runtime_object, normalized_property_name))
            {
                return runtime_object_has_assigner_property(runtime_object, normalized_property_name);
            }

            if (runtime_object_member_matches(runtime_object.methods, normalized_property_name) ||
                runtime_object_member_matches(runtime_object.events, normalized_property_name))
            {
                return false;
            }

            if (is_native_olecontrol_host_object(runtime_object))
            {
                RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(runtime_object);
                if (object_surface != nullptr && object_surface->handle != runtime_object.handle)
                {
                    return native_setall_candidate_has_writable_property(
                        *object_surface,
                        normalized_property_name);
                }
            }

            return false;
        }

        PrgValue apply_native_setall(
            RuntimeOleObjectState &runtime_object,
            const Frame &source_frame,
            const std::string &effective_member_path,
            const std::vector<PrgValue> &arguments)
        {
            if (arguments.size() < 2U)
            {
                return make_boolean_value(false);
            }

            const std::string property_name = normalize_identifier(trim_copy(value_as_string(arguments[0])));
            if (property_name.empty())
            {
                return make_boolean_value(false);
            }

            const std::string class_filter =
                arguments.size() >= 3U
                    ? normalize_identifier(trim_copy(value_as_string(arguments[2])))
                    : std::string{};

            std::size_t updated_count = 0U;
            std::vector<int> pending = collect_native_setall_child_handles(runtime_object);
            std::set<int> visited_handles;
            while (!pending.empty())
            {
                const int handle = pending.back();
                pending.pop_back();
                if (!visited_handles.insert(handle).second)
                {
                    continue;
                }

                const auto found = ole_objects.find(handle);
                if (found == ole_objects.end())
                {
                    continue;
                }

                RuntimeOleObjectState &child_object = found->second;
                std::vector<int> nested_children = collect_native_setall_child_handles(child_object);
                pending.insert(pending.end(), nested_children.begin(), nested_children.end());

                if (!class_filter.empty() &&
                    normalize_identifier(trim_copy(child_object.prog_id)) != class_filter)
                {
                    continue;
                }

                if (!native_setall_candidate_has_writable_property(child_object, property_name))
                {
                    continue;
                }

                if (write_native_property_if_present(
                        child_object,
                        property_name,
                        arguments[1],
                        source_frame))
                {
                    ++updated_count;
                }
            }

            runtime_object.last_action = effective_member_path + "(" + property_name + ")";
            ++runtime_object.action_count;
            events.push_back({.category = "prg.object.setall",
                              .detail = runtime_object.prog_id + "." + property_name + ":" +
                                            std::to_string(updated_count),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_number_value(static_cast<double>(updated_count));
        }

        RuntimeOleObjectState *instantiate_native_class_object(
            const Frame &frame,
            const std::string &prog_id,
            const std::string &program_path,
            const std::string &source_tag,
            const std::vector<PrgValue> &constructor_arguments = {},
            const std::vector<std::optional<std::string>> &constructor_argument_references = {},
            const std::optional<PrgValue> &parent_reference = std::nullopt)
        {
            Program &program = load_program(program_path);
            const auto class_found = program.classes.find(normalize_identifier(prog_id));
            if (class_found == program.classes.end())
            {
                if (!is_supported_native_base_class_name(prog_id))
                {
                    return nullptr;
                }

                const int handle = next_ole_handle++;
                RuntimeOleObjectState object_state{
                    .handle = handle,
                    .prog_id = native_same_prg_base_class_name(prog_id),
                    .source = {},
                    .last_action = source_tag,
                    .action_count = 1};
                if (parent_reference.has_value())
                {
                    object_state.properties["parent"] = *parent_reference;
                }
                object_state.base_class_name = native_same_prg_base_class_name(prog_id);
                assign_native_runtime_object_name(
                    object_state,
                    native_same_prg_base_class_name(prog_id));
                if (normalize_identifier(object_state.base_class_name) == "olecontrol" &&
                    !constructor_arguments.empty())
                {
                    const std::string oleclass = trim_copy(value_as_string(constructor_arguments.front()));
                    if (!oleclass.empty())
                    {
                        object_state.properties["oleclass"] = make_string_value(oleclass);
                        object_state.properties["documentfile"] = make_string_value("");
                        object_state.properties["oletypeallowed"] = make_int64_value(-2);
                        object_state.properties["autoactivate"] = make_int64_value(2);
                        object_state.properties["autoverbmenu"] = make_boolean_value(true);
                        seed_native_olecontrol_timeout_policy_properties(object_state);
                        seed_native_olecontrol_verb_inspection_properties(object_state);
                    }
                }
                const std::string class_token = uppercase_copy(trim_copy(object_state.prog_id));
                if (!class_token.empty())
                {
                    object_state.class_hierarchy.push_back(class_token);
                }
                if (object_state.class_hierarchy.empty() || object_state.class_hierarchy.back() != "OBJECT")
                {
                    object_state.class_hierarchy.push_back("OBJECT");
                }
                assign_native_window_metadata(object_state);
                seed_native_visual_properties(object_state);
                append_builtin_native_olecontrol_methods(object_state);
                object_state.default_properties = object_state.properties;
                object_state.default_properties.erase("parent");

                auto [object_it, _] = ole_objects.emplace(handle, std::move(object_state));
                (void)ensure_native_olecontrol_object_surface(object_it->second);
                (void)sync_native_owned_children_collection(object_it->second);
                return &object_it->second;
            }

            if (native_class_instantiation_depth >= max_call_depth)
            {
                throw std::runtime_error(call_depth_limit_message());
            }

            struct NativeClassInstantiationGuard
            {
                std::size_t &depth;

                explicit NativeClassInstantiationGuard(std::size_t &current_depth)
                    : depth(current_depth)
                {
                    ++depth;
                }

                ~NativeClassInstantiationGuard()
                {
                    --depth;
                }
            } guard(native_class_instantiation_depth);

            const PrgClassDefinition &class_definition = class_found->second;
            std::vector<NativeClassLookup> class_lineage =
                collect_native_class_lineage(
                    program,
                    class_definition.name.empty() ? prog_id : class_definition.name);
            if (class_lineage.empty())
            {
                class_lineage.push_back({.program = &program, .class_definition = &class_definition});
            }
            const int handle = next_ole_handle++;
            RuntimeOleObjectState object_state{
                .handle = handle,
                .prog_id = class_definition.name.empty() ? prog_id : class_definition.name,
                .source = program.path,
                .last_action = source_tag,
                .action_count = 1};
            if (parent_reference.has_value())
            {
                object_state.properties["parent"] = *parent_reference;
            }

            object_state.base_class_name =
                native_same_prg_base_class_name(class_definition.base_class_name);
            assign_native_runtime_object_name(
                object_state,
                class_definition.name.empty() ? prog_id : class_definition.name);
            if (!trim_copy(class_definition.base_class_source_path).empty())
            {
                object_state.class_library =
                    resolve_native_prg_program_path(class_definition.base_class_source_path, program.path);
            }

            object_state.class_hierarchy.reserve(class_lineage.size() + 2U);
            for (auto lineage_it = class_lineage.rbegin(); lineage_it != class_lineage.rend(); ++lineage_it)
            {
                const std::string class_name = uppercase_copy(trim_copy(
                    lineage_it->class_definition->name.empty() ? prog_id : lineage_it->class_definition->name));
                if (!class_name.empty() &&
                    (object_state.class_hierarchy.empty() || object_state.class_hierarchy.back() != class_name))
                {
                    object_state.class_hierarchy.push_back(class_name);
                }
            }
            if (!class_lineage.empty())
            {
                const std::string base_class_token = uppercase_copy(trim_copy(
                    native_same_prg_base_class_name(class_lineage.front().class_definition->base_class_name)));
                if (!base_class_token.empty() &&
                    (object_state.class_hierarchy.empty() || object_state.class_hierarchy.back() != base_class_token))
                {
                    object_state.class_hierarchy.push_back(base_class_token);
                }
            }
            if (object_state.class_hierarchy.empty() || object_state.class_hierarchy.back() != "OBJECT")
            {
                object_state.class_hierarchy.push_back("OBJECT");
            }

            assign_native_window_metadata(object_state);

            std::map<std::string, std::string> effective_methods;
            for (const NativeClassLookup &lineage_class : class_lineage)
            {
                for (const auto &[normalized_method_name, method] : lineage_class.class_definition->methods)
                {
                    effective_methods[normalized_method_name] = method.name;
                }
            }
            object_state.methods.reserve(effective_methods.size());
            for (const auto &[_, method_name] : effective_methods)
            {
                object_state.methods.push_back(method_name);
            }
            append_builtin_native_olecontrol_methods(object_state);

            auto [object_it, _] = ole_objects.emplace(handle, std::move(object_state));
            RuntimeOleObjectState *runtime_object = &object_it->second;
            const auto make_runtime_object_reference = [](const RuntimeOleObjectState &object_state) -> PrgValue
            {
                return make_string_value("object:" + object_state.prog_id + "#" + std::to_string(object_state.handle));
            };
            const auto resolve_controlsource_value = [&](const std::string& controlsource_text)
                -> std::optional<PrgValue>
            {
                const PrgValue variable_value = lookup_variable(frame, controlsource_text);
                if (variable_value.kind != PrgValueKind::empty)
                {
                    return variable_value;
                }

                const auto field_value =
                    resolve_field_value(controlsource_text, resolve_cursor_target({}));
                if (!field_value.has_value() ||
                    field_value->kind == PrgValueKind::empty)
                {
                    return std::nullopt;
                }

                return *field_value;
            };
            for (const NativeClassLookup &lineage_class : class_lineage)
            {
                for (const Statement &property_statement : lineage_class.class_definition->property_statements)
                {
                    if (property_statement.kind != StatementKind::assignment)
                    {
                        continue;
                    }

                    const std::string property_name = normalize_identifier(property_statement.identifier);
                    if (property_name.empty())
                    {
                        continue;
                    }

                    runtime_object->properties[property_name] =
                        evaluate_expression(property_statement.expression, frame);
                    native_property_expression_text_by_handle[runtime_object->handle][property_name] =
                        trim_copy(property_statement.expression);
                }
            }
            seed_native_olecontrol_timeout_policy_properties(*runtime_object);
            seed_native_olecontrol_verb_inspection_properties(*runtime_object);
            seed_native_visual_properties(*runtime_object);
            refresh_native_list_control_controlsource_value_kind_hint(
                *runtime_object,
                resolve_controlsource_value);
            if (RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(*runtime_object);
                object_surface != nullptr)
            {
                (void)ensure_native_olecontrol_application_conflict_surface(*runtime_object, *object_surface);
            }
            (void)read_native_collection_member(*runtime_object, "count");
            runtime_object->default_properties = runtime_object->properties;
            runtime_object->default_properties.erase("parent");
            native_default_property_expression_text_by_handle[runtime_object->handle] =
                native_property_expression_text_by_handle[runtime_object->handle];
            const PrgValue runtime_object_reference = make_runtime_object_reference(*runtime_object);
            for (const NativeClassLookup &lineage_class : class_lineage)
            {
                for (const NativeChildObjectDeclaration &child_declaration : lineage_class.class_definition->child_object_declarations)
                {
                    const std::string child_name_text = trim_copy(child_declaration.name);
                    const std::string child_name = normalize_identifier(child_name_text);
                    if (child_name.empty() || child_declaration.class_name.empty())
                    {
                        continue;
                    }

                    const std::string primary_child_program_path =
                        resolve_native_prg_program_path(child_declaration.source_path, lineage_class.program->path);
                    RuntimeOleObjectState *child_object = instantiate_native_class_object(
                        frame,
                        child_declaration.class_name,
                        primary_child_program_path,
                        "classbody.addobject",
                        {},
                        {},
                        runtime_object_reference);
                    if (child_object == nullptr && trim_copy(child_declaration.source_path).empty())
                    {
                        const std::string owner_program_path = normalize_path(runtime_object->source);
                        if (!owner_program_path.empty() &&
                            owner_program_path != normalize_path(primary_child_program_path))
                        {
                            child_object = instantiate_native_class_object(
                                frame,
                                child_declaration.class_name,
                                owner_program_path,
                                "classbody.addobject",
                                {},
                                {},
                                runtime_object_reference);
                        }
                    }
                    if (child_object == nullptr)
                    {
                        continue;
                    }
                    for (const Statement& property_statement : child_declaration.property_statements)
                    {
                        if (property_statement.kind != StatementKind::assignment)
                        {
                            continue;
                        }

                        const std::string property_name = normalize_identifier(property_statement.identifier);
                        if (property_name.empty())
                        {
                            continue;
                        }

                        child_object->properties[property_name] =
                            evaluate_expression(property_statement.expression, frame);
                        native_property_expression_text_by_handle[child_object->handle][property_name] =
                            trim_copy(property_statement.expression);
                    }
                    refresh_native_list_control_controlsource_value_kind_hint(
                        *child_object,
                        resolve_controlsource_value);
                    assign_native_runtime_object_name(*child_object, child_name_text);

                    runtime_object->properties[child_name] = make_runtime_object_reference(*child_object);
                    if (child_object->properties.contains("columnorder"))
                    {
                        (void)write_native_columnorder_property(
                            *child_object,
                            child_object->properties["columnorder"]);
                    }
                    runtime_object->last_action = "addobject(" + child_name + "," + child_declaration.class_name + ")";
                    ++runtime_object->action_count;
                    events.push_back({.category = "prg.object.addobject",
                                      .detail = runtime_object->prog_id + "." + child_name + ":" + child_declaration.class_name,
                                      .location = current_statement() == nullptr ? child_declaration.declaration_location : current_statement()->location});
                }
            }
            if (is_native_column_runtime_object(*runtime_object) &&
                native_column_bound_value(*runtime_object))
            {
                sync_native_column_child_controlsources(*runtime_object);
            }
            if (runtime_object->properties.contains("columncount") &&
                is_native_grid_runtime_object(*runtime_object) &&
                normalize_native_grid_columncount_value(runtime_object->properties["columncount"]) >= 0)
            {
                (void)write_native_grid_columncount_property(
                    *runtime_object,
                    runtime_object->properties["columncount"],
                    frame);
            }
            (void)sync_native_owned_children_collection(*runtime_object);

            std::string init_program_path;
            std::string init_method_name;
            if (const Routine *init_method = find_native_object_method(
                    *runtime_object,
                    "init",
                    init_program_path,
                    init_method_name);
                init_method != nullptr)
            {
                if (!can_push_frame())
                {
                    throw std::runtime_error(call_depth_limit_message());
                }

                events.push_back({.category = "prg.object.init",
                                  .detail = init_method_name,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                const std::size_t return_depth = stack.size();
                std::vector<PrgValue> effective_constructor_arguments = constructor_arguments;
                if (effective_constructor_arguments.size() < constructor_argument_references.size())
                {
                    effective_constructor_arguments.resize(constructor_argument_references.size());
                }
                for (std::size_t index = 0U; index < constructor_argument_references.size(); ++index)
                {
                    if (constructor_argument_references[index].has_value())
                    {
                        effective_constructor_arguments[index] =
                            lookup_variable(frame, *constructor_argument_references[index]);
                    }
                }
                push_method_frame(init_program_path,
                                  init_method_name,
                                  *init_method,
                                  runtime_object_reference,
                                  init_method_name.substr(0U, init_method_name.rfind('.')),
                                  "init",
                                  parent_reference,
                                  native_object_owner_form_reference(*runtime_object),
                                  native_object_owner_formset_reference(*runtime_object),
                                  effective_constructor_arguments,
                                  constructor_argument_references);
                (void)run_expression_invoked_routine_until_return(return_depth);
            }

            return runtime_object;
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
            const bool compatibility_matches_current_fault =
                last_error_compatibility.preserve_fault_context ||
                (last_fault_location.file_path == statement.location.file_path &&
                 last_fault_location.line == statement.location.line &&
                 last_fault_statement == statement.text);
            const bool preserve_fault_context = last_error_compatibility.preserve_fault_context;
            if (!preserve_fault_context)
            {
                last_fault_location = statement.location;
                last_fault_statement = statement.text;
                last_error_work_area = current_selected_work_area();
                last_error_procedure = frame.routine_name;
            }
            last_error_code =
                compatibility_matches_current_fault && last_error_compatibility.explicit_error_code.has_value()
                    ? *last_error_compatibility.explicit_error_code
                    : classify_runtime_error_code(last_error_message);
            const bool preserve_compatibility =
                last_error_code == 1526 ||
                last_error_code == 1429 ||
                (compatibility_matches_current_fault && last_error_compatibility.thrown_user_value.has_value()) ||
                (compatibility_matches_current_fault && last_error_compatibility.explicit_error_code.has_value()) ||
                last_error_compatibility.preserve_fault_context;
            if (!preserve_compatibility)
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
            snapshot.data_session = current_data_session;
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

        [[nodiscard]] bool has_active_exception_context() const
        {
            if (!error_metadata_stack.empty() || handling_error)
            {
                return true;
            }

            for (const Frame &frame : stack)
            {
                for (const TryState &try_state : frame.tries)
                {
                    if (try_state.handling_error)
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        void remember_active_exception_reference(const PrgValue &reference)
        {
            last_error_compatibility.active_exception_reference = reference;
            if (!error_metadata_stack.empty())
            {
                error_metadata_stack.back().compatibility.active_exception_reference = reference;
            }
        }

        PrgValue materialize_catch_exception_object()
        {
            const AErrorCompatibilitySnapshot &compatibility = last_error_compatibility;
            if (compatibility.preserve_fault_context &&
                compatibility.active_exception_reference.has_value())
            {
                remember_active_exception_reference(*compatibility.active_exception_reference);
                return *compatibility.active_exception_reference;
            }
            std::string detail = last_error_message;
            if (!compatibility.sql_detail.empty())
            {
                detail = compatibility.sql_detail;
            }
            else if (!compatibility.ole_detail.empty())
            {
                detail = compatibility.ole_detail;
            }

            const int handle = next_ole_handle++;
            RuntimeOleObjectState object_state{
                .handle = handle,
                .prog_id = "Exception",
                .source = {},
                .last_action = "catch",
                .action_count = 1};
            object_state.base_class_name = "Exception";
            object_state.class_hierarchy = {"EXCEPTION", "OBJECT"};
            object_state.properties["message"] = make_string_value(last_error_message);
            object_state.properties["errorno"] = make_number_value(static_cast<double>(last_error_code));
            object_state.properties["lineno"] = make_number_value(static_cast<double>(last_fault_location.line));
            object_state.properties["procedure"] = make_string_value(last_error_procedure);
            object_state.properties["details"] = make_string_value(detail);
            object_state.properties["linecontents"] = make_string_value(last_fault_statement);
            object_state.properties["stacklevel"] = make_number_value(static_cast<double>(stack.size()));
            object_state.properties["uservalue"] = compatibility.thrown_user_value.value_or(make_empty_value());

            auto [inserted, _] = ole_objects.emplace(handle, std::move(object_state));
            const PrgValue reference =
                make_string_value("object:" + inserted->second.prog_id + "#" + std::to_string(inserted->second.handle));
            remember_active_exception_reference(reference);
            return reference;
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

            const std::string requested_name = candidate.filename().string();
            const std::string expected_name = lowercase_copy(requested_name);
            const std::string expected_stem = candidate.stem().string();
            std::optional<std::filesystem::path> exact_stem_match;
            std::optional<std::filesystem::path> folded_match;
            bool folded_match_ambiguous = false;
            std::filesystem::directory_iterator iterator(parent, ignored);
            const std::filesystem::directory_iterator end;
            for (; iterator != end && !ignored; iterator.increment(ignored))
            {
                const std::filesystem::path entry_path = iterator->path();
                const std::string entry_name = entry_path.filename().string();
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
                if (entry_path.stem().string() == expected_stem)
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
                std::filesystem::path(current_default_directory())};
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
                    std::filesystem::path directory(entry);
                    if (directory.is_relative())
                    {
                        directory = std::filesystem::path(current_default_directory()) / directory;
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
            const std::string normalized = path.lexically_normal().string();
            if (const auto exact = options.verified_file_byte_overrides.find(normalized);
                exact != options.verified_file_byte_overrides.end())
            {
                return exact;
            }
#if defined(_WIN32)
            return std::find_if(
                options.verified_file_byte_overrides.begin(),
                options.verified_file_byte_overrides.end(),
                [&](const auto &candidate)
                {
                    return paths_equal_insensitive(candidate.first, normalized);
                });
#else
            return options.verified_file_byte_overrides.end();
#endif
        }

        bool database_paths_equal(const std::string &left, const std::string &right) const
        {
#if defined(_WIN32)
            return paths_equal_insensitive(left, right);
#else
            return std::filesystem::path(left).lexically_normal() ==
                std::filesystem::path(right).lexically_normal();
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
            const auto verified = find_verified_file_byte_override(path);
            if (verified != options.verified_file_byte_overrides.end())
            {
                bytes = verified->second;
                if (bytes.empty())
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Database.Error.ComponentMalformed",
                        {{"path", path.string()}});
                    return false;
                }
                return true;
            }
            if (options.require_verified_file_byte_overrides)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.VerifiedBytesUnavailable",
                    {{"path", path.string()}});
                return false;
            }

            std::ifstream input(path, std::ios::binary);
            if (!input.good())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentReadFailed",
                    {{"path", path.string()}});
                return false;
            }
            std::ostringstream stream;
            stream << input.rdbuf();
            bytes = stream.str();
            if (bytes.empty())
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentMalformed",
                    {{"path", path.string()}});
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
                    find_open_database_by_path(session, database_path->string()))
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
                    find_open_database_by_path(candidate_session, database_path->string());
                if (existing != nullptr && (existing->exclusive || exclusive))
                {
                    last_error_message = runtime_text(
                        "Runtime.Prg.Database.Error.ExclusiveConflict",
                        {{"path", database_path->string()}});
                    return false;
                }
            }

            const auto dct_path =
                resolve_existing_database_component(std::filesystem::path(*database_path).replace_extension(".dct"));
            const auto dcx_path =
                resolve_existing_database_component(std::filesystem::path(*database_path).replace_extension(".dcx"));
            if (!dct_path.has_value() || !dcx_path.has_value())
            {
                const std::filesystem::path missing_path = !dct_path.has_value()
                    ? std::filesystem::path(*database_path).replace_extension(".dct")
                    : std::filesystem::path(*database_path).replace_extension(".dcx");
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.CompanionMissing",
                    {{"path", missing_path.string()}});
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
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ContainerMalformed",
                    {{"path", database_path->string()}, {"errorMessage", header.error}});
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
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentMalformed",
                    {{"path", dct_path->string()}});
                return false;
            }
            const std::vector<std::uint8_t> dcx_binary(dcx_bytes.begin(), dcx_bytes.end());
            const auto index_probe = vfp::parse_index_probe(
                dcx_binary,
                static_cast<std::uint64_t>(dcx_binary.size()),
                vfp::IndexKind::dcx);
            if (!index_probe.ok)
            {
                last_error_message = runtime_text(
                    "Runtime.Prg.Database.Error.ComponentMalformed",
                    {{"path", dcx_path->string()}});
                return false;
            }

            RuntimeDatabaseState database{
                .path = database_path->lexically_normal().string(),
                .name = database_path->stem().string(),
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

            try
            {
                const long long parsed = std::stoll(trimmed);
                return {.display_value = std::to_string(std::max<long long>(0LL, parsed)),
                        .retry_budget = static_cast<std::size_t>(std::max<long long>(0LL, parsed))};
            }
            catch (...)
            {
                return {.display_value = uppercase_copy(trimmed), .retry_budget = 0U};
            }
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
                        last_error_message = command_undo_backup_message(key);
                        return false;
                    }
                    entry.backup_path = backup_path.string();
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
                        last_error_message = transaction_backup_message(key);
                        return false;
                    }
                    entry.backup_path = backup_path.string();
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

                const auto table_result = vfp::parse_dbf_table_from_file(cursor.source_path, std::max<std::size_t>(cursor.record_count, 1U));
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
