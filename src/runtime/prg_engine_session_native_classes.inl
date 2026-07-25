// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

        std::string resolve_native_prg_program_path(
            const std::string &source_path,
            const std::string &fallback_path = {}) const
        {
            const std::string trimmed_source_path = trim_copy(source_path);
            if (trimmed_source_path.empty())
            {
                return normalize_path(fallback_path);
            }

            std::filesystem::path program_path = copperfin::platform::path_from_utf8_string(trimmed_source_path);
            if (program_path.is_absolute())
            {
                return copperfin::platform::path_to_utf8_string(program_path.lexically_normal());
            }

            const std::filesystem::path default_directory_candidate =
                (copperfin::platform::path_from_utf8_string(current_default_directory()) / program_path).lexically_normal();
            std::error_code ignored;
            if (std::filesystem::exists(default_directory_candidate, ignored))
            {
                return copperfin::platform::path_to_utf8_string(default_directory_candidate);
            }

            const std::string normalized_fallback_path = normalize_path(fallback_path);
            if (!normalized_fallback_path.empty())
            {
                const std::filesystem::path fallback_directory_candidate =
                    (copperfin::platform::path_from_utf8_string(normalized_fallback_path).parent_path() /
                     program_path).lexically_normal();
                if (std::filesystem::exists(fallback_directory_candidate, ignored))
                {
                    return copperfin::platform::path_to_utf8_string(fallback_directory_candidate);
                }
            }

            return copperfin::platform::path_to_utf8_string(program_path.lexically_normal());
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
                if (auto same_program = find_native_class_lookup(program, base_class_name);
                    same_program.has_value())
                {
                    return same_program;
                }
                if (is_supported_native_base_class_name(base_class_name))
                {
                    return std::nullopt;
                }
                return find_loaded_procedure_class_lookup(base_class_name, program.path);
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

        std::vector<NativeClassLookup> resolved_native_object_class_lineage(
            const RuntimeOleObjectState &runtime_object)
        {
            std::vector<NativeClassLookup> resolved;
            if (const auto saved = native_object_class_lineage_by_handle.find(runtime_object.handle);
                saved != native_object_class_lineage_by_handle.end())
            {
                resolved.reserve(saved->second.size());
                for (const NativeClassIdentity &identity : saved->second)
                {
                    Program &program = load_program(identity.program_path);
                    const auto class_found = program.classes.find(normalize_identifier(identity.class_name));
                    if (class_found != program.classes.end())
                    {
                        resolved.push_back(
                            {.program = &program, .class_definition = &class_found->second});
                    }
                }
            }

            if (!resolved.empty() || runtime_object.source.empty())
            {
                return resolved;
            }

            Program &program = load_program(runtime_object.source);
            return collect_native_class_lineage(program, runtime_object.prog_id);
        }

        std::optional<NativeMethodLookup> find_native_object_class_method_lookup(
            const RuntimeOleObjectState &runtime_object,
            const std::string &member_name,
            const std::string &starting_program_path,
            const std::string &starting_class_name,
            bool include_starting_class,
            std::string &qualified_routine_name,
            std::string *defining_class_name = nullptr)
        {
            const std::string normalized_member_name = normalize_identifier(member_name);
            const std::string normalized_starting_path = normalize_path(starting_program_path);
            const std::string normalized_starting_class = normalize_identifier(starting_class_name);
            std::vector<NativeClassLookup> lineage =
                resolved_native_object_class_lineage(runtime_object);
            if (lineage.empty())
            {
                return std::nullopt;
            }

            std::size_t search_count = lineage.size();
            if (!normalized_starting_class.empty())
            {
                search_count = 0U;
                for (std::size_t index = lineage.size(); index > 0U; --index)
                {
                    const NativeClassLookup &candidate = lineage[index - 1U];
                    if (normalize_identifier(candidate.class_definition->name) != normalized_starting_class ||
                        (!normalized_starting_path.empty() &&
                         normalize_path(candidate.program->path) != normalized_starting_path))
                    {
                        continue;
                    }
                    search_count = include_starting_class ? index : index - 1U;
                    break;
                }
            }
            else if (!include_starting_class)
            {
                --search_count;
            }

            while (search_count > 0U)
            {
                const NativeClassLookup &candidate = lineage[--search_count];
                const auto method_found =
                    candidate.class_definition->methods.find(normalized_member_name);
                if (method_found == candidate.class_definition->methods.end())
                {
                    continue;
                }

                const std::string resolved_class_name =
                    candidate.class_definition->name.empty()
                        ? runtime_object.prog_id
                        : candidate.class_definition->name;
                if (defining_class_name != nullptr)
                {
                    *defining_class_name = resolved_class_name;
                }
                qualified_routine_name = resolved_class_name + "." + method_found->second.name;
                return NativeMethodLookup{
                    .program = candidate.program,
                    .class_definition = candidate.class_definition,
                    .routine = &method_found->second};
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

            const auto method_lookup =
                find_native_object_class_method_lookup(
                    runtime_object,
                    member_name,
                    {},
                    {},
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
                auto property = current_object->properties.find(property_name);
                if (property == current_object->properties.end() &&
                    property_name == "header" &&
                    is_native_column_runtime_object(*current_object))
                {
                    (void)ensure_native_column_header_surface(*current_object);
                    property = current_object->properties.find(property_name);
                }
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
