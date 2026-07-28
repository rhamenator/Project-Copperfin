// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

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
            const std::string &path,
            const bool fail_on_ambiguity = false,
            bool *ambiguous = nullptr) const
        {
            if (ambiguous != nullptr)
            {
                *ambiguous = false;
            }
            if (const auto exact = options.source_text_overrides.find(path);
                exact != options.source_text_overrides.end())
            {
                return exact;
            }
            auto folded_match = options.source_text_overrides.end();
            for (auto candidate = options.source_text_overrides.begin();
                 candidate != options.source_text_overrides.end();
                 ++candidate)
            {
                if (!paths_equal_insensitive(candidate->first, path))
                {
                    continue;
                }
                if (folded_match != options.source_text_overrides.end())
                {
                    if (fail_on_ambiguity)
                    {
                        if (ambiguous != nullptr)
                        {
                            *ambiguous = true;
                        }
                        return options.source_text_overrides.end();
                    }
                    return folded_match;
                }
                folded_match = candidate;
            }
            return folded_match;
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
            bool source_override_ambiguous = false;
            const auto source_override = find_source_text_override(
                normalized,
                options.require_source_text_overrides,
                &source_override_ambiguous);
            if (!use_startup_source_text &&
                (source_override_ambiguous ||
                 source_override == options.source_text_overrides.end()) &&
                options.require_source_text_overrides)
            {
                throw std::runtime_error(runtime_text(
                    "Runtime.Prg.Parser.Error.VerifiedSourceUnavailable",
                    {{"path", normalized}}));
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

            std::filesystem::path program_path = copperfin::platform::path_from_utf8_string(resolved_target);
            if (program_path.extension().empty())
            {
                program_path += ".prg";
            }

            return resolve_native_prg_program_path(
                copperfin::platform::path_to_utf8_string(program_path), fallback_path);
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

        std::optional<RoutineLookup> find_event_handler_routine_lookup(
            const std::string &identifier)
        {
            for (auto iterator = stack.rbegin(); iterator != stack.rend(); ++iterator)
            {
                Program &program = load_program(iterator->file_path);
                const auto found = program.routines.find(normalize_identifier(identifier));
                if (found != program.routines.end())
                {
                    return RoutineLookup{.program = &program, .routine = &found->second};
                }
            }

            return find_loaded_procedure_routine_lookup(identifier);
        }

        std::optional<NativeClassLookup> find_loaded_procedure_class_lookup(
            const std::string &class_name,
            const std::string &exclude_program_path = {})
        {
            const std::string normalized_class_name = normalize_identifier(class_name);
            const std::string normalized_exclude_program_path = normalize_path(exclude_program_path);
            for (const std::string &procedure_program_path : procedure_program_paths)
            {
                if (!normalized_exclude_program_path.empty() &&
                    procedure_program_path == normalized_exclude_program_path)
                {
                    continue;
                }

                Program &program = load_program(procedure_program_path);
                const auto found = program.classes.find(normalized_class_name);
                if (found != program.classes.end())
                {
                    return NativeClassLookup{.program = &program, .class_definition = &found->second};
                }
            }

            return std::nullopt;
        }

        std::optional<NativeClassLookup> find_unqualified_native_class_lookup(
            const std::string &source_file_path,
            const std::string &class_name)
        {
            Program &program = load_program(source_file_path);
            if (const auto found = program.classes.find(normalize_identifier(class_name));
                found != program.classes.end())
            {
                return NativeClassLookup{.program = &program, .class_definition = &found->second};
            }

            return find_loaded_procedure_class_lookup(class_name, program.path);
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
                   normalized_class_name == "header" ||
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
