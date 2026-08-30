    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_runtime_object_reference_member(
        const PrgValue &object_reference,
        const std::string &member_path,
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        auto object = resolve_ole_object(object_reference);
        if (!object.has_value())
        {
            return std::nullopt;
        }

        const auto resolved_path = resolve_runtime_object_member_path(*object, member_path);
        if (resolved_path.runtime_object == nullptr)
        {
            return std::nullopt;
        }

        return invoke_runtime_object_member(
            *resolved_path.runtime_object,
            resolved_path.remaining_member_path.empty()
                ? member_path
                : resolved_path.remaining_member_path,
            source_frame,
            arguments,
            argument_references);
    }

    bool PrgRuntimeSession::Impl::set_native_focus(
        RuntimeOleObjectState &runtime_object,
        const std::string &effective_member_path,
        const Frame &frame)
    {
        if (!is_native_focusable_runtime_object(runtime_object))
        {
            return false;
        }

        const PrgValue runtime_object_reference =
            make_string_value("object:" + runtime_object.prog_id + "#" + std::to_string(runtime_object.handle));
        std::optional<PrgValue> previous_active_control;
        bool focus_changed = true;
        bool suppress_focus_transition = false;
        if (const auto owner_form_reference = native_object_owner_form_reference(runtime_object);
            owner_form_reference.has_value())
        {
            if (auto owner_form = resolve_ole_object(*owner_form_reference);
                owner_form.has_value())
            {
                if (const auto current_active_control =
                        read_native_property_if_present(**owner_form, "activecontrol", frame);
                    current_active_control.has_value())
                {
                    previous_active_control = *current_active_control;
                }
                if (previous_active_control.has_value())
                {
                    if (auto previous_control = resolve_ole_object(*previous_active_control);
                        previous_control.has_value())
                    {
                        focus_changed = (*previous_control)->handle != runtime_object.handle;
                        if (focus_changed)
                        {
                            last_popped_frame_requested_nodefault = false;
                            bool valid_requested_nodefault = false;
                            const auto valid_result =
                                invoke_native_object_method_if_present(
                                    **previous_control,
                                    "valid",
                                    frame,
                                    {},
                                    {},
                                    &valid_requested_nodefault);
                            (void)consume_last_popped_frame_requested_nodefault();
                            const bool validation_rejected =
                                valid_result.has_value() &&
                                valid_result->kind != PrgValueKind::empty &&
                                !value_as_bool(*valid_result);
                            if (validation_rejected || valid_requested_nodefault)
                            {
                                suppress_focus_transition = true;
                            }
                        }
                        if (focus_changed && !suppress_focus_transition)
                        {
                            last_popped_frame_requested_nodefault = false;
                            bool lost_focus_requested_nodefault = false;
                            (void)invoke_native_object_method_if_present(
                                **previous_control,
                                "lostfocus",
                                frame,
                                {},
                                {},
                                &lost_focus_requested_nodefault);
                            (void)consume_last_popped_frame_requested_nodefault();
                            suppress_focus_transition = lost_focus_requested_nodefault;
                        }
                    }
                }
                if (!suppress_focus_transition)
                {
                    (void)write_native_property_if_present(
                        **owner_form,
                        "activecontrol",
                        runtime_object_reference,
                        frame);
                }
            }
        }
        else if (!suppress_focus_transition &&
                 normalize_identifier(trim_copy(runtime_object.base_class_name)) == "form")
        {
            (void)write_native_property_if_present(
                runtime_object,
                "activecontrol",
                runtime_object_reference,
                frame);
        }
        if (!suppress_focus_transition)
        {
            note_representative_active_form(runtime_object);
            if (focus_changed)
            {
                last_popped_frame_requested_nodefault = false;
                bool got_focus_requested_nodefault = false;
                (void)invoke_native_object_method_if_present(
                    runtime_object,
                    "gotfocus",
                    frame,
                    {},
                    {},
                    &got_focus_requested_nodefault);
                (void)consume_last_popped_frame_requested_nodefault();
            }
        }
        runtime_object.last_action = effective_member_path + "()";
        ++runtime_object.action_count;
        events.push_back({.category = "prg.object.setfocus",
                          .detail = runtime_object.prog_id + "." + effective_member_path,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        return true;
    }

    bool PrgRuntimeSession::Impl::move_native_focus_to_next_tab_stop(
        RuntimeOleObjectState &runtime_object,
        const Frame &frame)
    {
        RuntimeOleObjectState *owner_form = nullptr;
        if (const auto owner_form_reference = native_object_owner_form_reference(runtime_object);
            owner_form_reference.has_value())
        {
            if (auto resolved_form = resolve_ole_object(*owner_form_reference);
                resolved_form.has_value())
            {
                owner_form = *resolved_form;
            }
        }
        else if (normalize_identifier(trim_copy(runtime_object.base_class_name)) == "form")
        {
            owner_form = &runtime_object;
        }

        if (owner_form == nullptr)
        {
            return false;
        }

        struct TabStopCandidate
        {
            int handle = 0;
            std::vector<long long> tab_order;
        };
        struct PendingTabObject
        {
            int handle = 0;
            bool ancestor_visible = true;
            bool ancestor_enabled = true;
            std::vector<long long> tab_prefix;
        };
        const auto read_tab_index = [](const RuntimeOleObjectState &object)
        {
            if (const auto property = object.properties.find("tabindex");
                property != object.properties.end())
            {
                try
                {
                    return static_cast<long long>(std::llround(value_as_number(property->second)));
                }
                catch (...)
                {
                    return 0LL;
                }
            }
            return 0LL;
        };
        std::vector<TabStopCandidate> candidates;
        std::vector<PendingTabObject> pending_objects;
        std::set<int> seen_handles;
        for (const int child_handle : collect_native_owned_child_handles(*owner_form))
        {
            pending_objects.push_back({child_handle, true, true, {}});
        }

        while (!pending_objects.empty())
        {
            const PendingTabObject pending = pending_objects.back();
            pending_objects.pop_back();
            if (!seen_handles.insert(pending.handle).second)
            {
                continue;
            }

            const auto child_found = ole_objects.find(pending.handle);
            if (child_found == ole_objects.end())
            {
                continue;
            }

            const auto property_is_true = [&](const std::string &property_name)
            {
                const auto property = child_found->second.properties.find(property_name);
                return property == child_found->second.properties.end() ||
                    value_as_bool(property->second);
            };
            const bool visible = pending.ancestor_visible && property_is_true("visible");
            const bool enabled = pending.ancestor_enabled && property_is_true("enabled");
            const std::string normalized_base_class =
                normalize_identifier(trim_copy(child_found->second.base_class_name));
            if (is_native_focusable_runtime_object(child_found->second) &&
                normalized_base_class != "page" &&
                property_is_true("tabstop") && visible && enabled)
            {
                std::vector<long long> tab_order = pending.tab_prefix;
                tab_order.push_back(read_tab_index(child_found->second));
                candidates.push_back({pending.handle, std::move(tab_order)});
            }

            if (!visible || !enabled)
            {
                continue;
            }

            if (normalized_base_class == "container" || normalized_base_class == "page")
            {
                std::vector<long long> child_tab_prefix = pending.tab_prefix;
                child_tab_prefix.push_back(read_tab_index(child_found->second));
                for (const int nested_handle : collect_native_owned_child_handles(child_found->second))
                {
                    pending_objects.push_back({nested_handle, visible, enabled, child_tab_prefix});
                }
            }
            else if (normalized_base_class == "pageframe")
            {
                long long active_page = 0LL;
                if (const auto property = child_found->second.properties.find("activepage");
                    property != child_found->second.properties.end())
                {
                    try
                    {
                        active_page = std::llround(value_as_number(property->second));
                    }
                    catch (...)
                    {
                        active_page = 0LL;
                    }
                }

                if (active_page <= 0LL)
                {
                    continue;
                }

                const auto page_members = collect_native_pageframe_page_members(child_found->second);
                const auto page_index = static_cast<std::size_t>(active_page - 1LL);
                if (page_index >= page_members.size() || page_members[page_index].child_object == nullptr)
                {
                    continue;
                }

                std::vector<long long> page_tab_prefix = pending.tab_prefix;
                page_tab_prefix.push_back(read_tab_index(child_found->second));
                pending_objects.push_back({
                    page_members[page_index].child_object->handle,
                    visible,
                    enabled,
                    page_tab_prefix});
            }
            else if (normalized_base_class == "commandgroup" && property_is_true("tabstop"))
            {
                const std::vector<long long> group_tab_prefix{
                    pending.tab_prefix.begin(),
                    pending.tab_prefix.end()};
                const long long group_tab_index = read_tab_index(child_found->second);
                std::vector<long long> child_tab_prefix = group_tab_prefix;
                child_tab_prefix.push_back(group_tab_index);
                for (const int nested_handle : collect_native_owned_child_handles(child_found->second))
                {
                    pending_objects.push_back({nested_handle, visible, enabled, child_tab_prefix});
                }
            }
        }

        if (candidates.empty())
        {
            return false;
        }

        std::sort(
            candidates.begin(),
            candidates.end(),
            [](const TabStopCandidate &left, const TabStopCandidate &right)
            {
                if (left.tab_order != right.tab_order)
                {
                    return left.tab_order < right.tab_order;
                }
                return left.handle < right.handle;
            });

        std::size_t next_index = 0U;
        const auto current = std::find_if(
            candidates.begin(),
            candidates.end(),
            [&](const TabStopCandidate &candidate)
            {
                return candidate.handle == runtime_object.handle;
            });
        if (current != candidates.end())
        {
            next_index = static_cast<std::size_t>(std::distance(candidates.begin(), current) + 1U) %
                candidates.size();
        }

        auto next_object = ole_objects.find(candidates[next_index].handle);
        if (next_object == ole_objects.end())
        {
            return false;
        }

        bool requested_nodefault = false;
        if (const auto native_result = invoke_native_object_method_if_present(
                next_object->second,
                "setfocus",
                frame,
                {},
                {},
                &requested_nodefault);
            native_result.has_value())
        {
            (void)consume_last_popped_frame_requested_nodefault();
            return true;
        }

        return set_native_focus(next_object->second, "SetFocus", frame);
    }

    bool PrgRuntimeSession::Impl::move_native_optiongroup_selection(
        RuntimeOleObjectState &runtime_object,
        const Frame &frame,
        const int direction)
    {
        if (normalize_identifier(trim_copy(runtime_object.base_class_name)) != "optiongroup" ||
            options.keyboard_compatibility != RuntimeKeyboardCompatibility::windows ||
            (direction != -1 && direction != 1))
        {
            return false;
        }

        struct OptionCandidate
        {
            int handle = 0;
            long long option_number = 0;
            long long tab_index = 0;
            bool eligible = false;
        };
        std::vector<OptionCandidate> candidates;
        for (const int child_handle : collect_native_owned_child_handles(runtime_object))
        {
            const auto child_found = ole_objects.find(child_handle);
            if (child_found == ole_objects.end() ||
                normalize_identifier(trim_copy(child_found->second.base_class_name)) != "optionbutton")
            {
                continue;
            }

            const auto visible = read_native_property_if_present(child_found->second, "visible", frame);
            const auto enabled = read_native_property_if_present(child_found->second, "enabled", frame);
            const bool eligible = (!visible.has_value() || value_as_bool(*visible)) &&
                (!enabled.has_value() || value_as_bool(*enabled));

            long long tab_index = child_handle;
            if (const auto tab_index_value =
                    read_native_property_if_present(child_found->second, "tabindex", frame);
                tab_index_value.has_value())
            {
                try
                {
                    tab_index = std::llround(value_as_number(*tab_index_value));
                }
                catch (...)
                {
                    tab_index = child_handle;
                }
            }
            candidates.push_back({child_handle, 0, tab_index, eligible});
        }

        if (candidates.empty())
        {
            return false;
        }

        std::sort(
            candidates.begin(),
            candidates.end(),
            [](const OptionCandidate &left, const OptionCandidate &right)
            {
                return left.handle < right.handle;
            });
        for (std::size_t index = 0U; index < candidates.size(); ++index)
        {
            candidates[index].option_number = static_cast<long long>(index + 1U);
        }
        candidates.erase(
            std::remove_if(
                candidates.begin(),
                candidates.end(),
                [](const OptionCandidate &candidate)
                {
                    return !candidate.eligible;
                }),
            candidates.end());
        if (candidates.empty())
        {
            return false;
        }

        std::sort(
            candidates.begin(),
            candidates.end(),
            [](const OptionCandidate &left, const OptionCandidate &right)
            {
                if (left.tab_index != right.tab_index)
                {
                    return left.tab_index < right.tab_index;
                }
                return left.handle < right.handle;
            });

        std::optional<std::size_t> current_index;
        if (const auto group_value = read_native_property_if_present(runtime_object, "value", frame);
            group_value.has_value())
        {
            const long long selected_option = std::llround(value_as_number(*group_value));
            for (std::size_t index = 0U; index < candidates.size(); ++index)
            {
                if (candidates[index].option_number == selected_option)
                {
                    current_index = index;
                    break;
                }
            }
        }
        if (!current_index.has_value())
        {
            for (std::size_t index = 0U; index < candidates.size(); ++index)
            {
                const auto child_found = ole_objects.find(candidates[index].handle);
                if (child_found == ole_objects.end())
                {
                    continue;
                }
                const auto selected = read_native_property_if_present(child_found->second, "value", frame);
                if (selected.has_value() && value_as_bool(*selected))
                {
                    current_index = index;
                    break;
                }
            }
        }

        const std::size_t start_index = current_index.has_value()
            ? *current_index
            : (direction > 0 ? candidates.size() - 1U : 0U);
        const std::size_t next_index = direction > 0
            ? (start_index + 1U) % candidates.size()
            : (start_index + candidates.size() - 1U) % candidates.size();
        const long long selected_option = candidates[next_index].option_number;

        for (const OptionCandidate &candidate : candidates)
        {
            const auto child_found = ole_objects.find(candidate.handle);
            if (child_found == ole_objects.end())
            {
                continue;
            }
            (void)write_native_property_if_present(
                child_found->second,
                "value",
                make_boolean_value(candidate.handle == candidates[next_index].handle),
                frame);
        }
        if (!write_native_property_if_present(
                runtime_object,
                "value",
                make_number_value(static_cast<double>(selected_option)),
                frame))
        {
            return false;
        }

        bool ignored_nodefault = false;
        if (invoke_native_object_method_if_present(
                runtime_object,
                "interactivechange",
                frame,
                {},
                {},
                &ignored_nodefault,
                nullptr)
                .has_value())
        {
            events.push_back({.category = "prg.event.interactivechange",
                              .detail = runtime_object.prog_id,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        }
        runtime_object.last_action = "OptionGroup.Value";
        ++runtime_object.action_count;
        return true;
    }

    PrgValue PrgRuntimeSession::Impl::invoke_runtime_object_member(
        RuntimeOleObjectState &runtime_object,
        const std::string &effective_member_path,
        const Frame &frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        const auto make_runtime_object_reference = [](const RuntimeOleObjectState &object_state) -> PrgValue
        {
            return make_string_value("object:" + object_state.prog_id + "#" + std::to_string(object_state.handle));
        };
        RuntimeOleObjectState *target_object = &runtime_object;
        const std::string leaf = normalize_identifier(
            effective_member_path.substr(
                effective_member_path.rfind('.') == std::string::npos
                    ? 0U
                    : effective_member_path.rfind('.') + 1U));

        if (leaf == "addobject" && !target_object->source.empty() && arguments.size() >= 2U)
        {
            const std::string child_name_text = trim_copy(value_as_string(arguments[0]));
            const std::string child_name = normalize_identifier(child_name_text);
            const std::string child_class = trim_copy(value_as_string(arguments[1]));
            if (child_name.empty() || child_class.empty())
            {
                return make_boolean_value(false);
            }

            const std::string child_library =
                arguments.size() >= 3U ? trim_copy(value_as_string(arguments[2])) : std::string{};
            const bool explicit_native_prg_library =
                lowercase_copy(copperfin::platform::path_to_utf8_string(
                    copperfin::platform::path_from_utf8_string(child_library).extension())) == ".prg";
            const std::string implicit_child_program_path =
                frame.native_method_class_name.empty()
                    ? target_object->source
                    : frame.file_path;
            const std::size_t constructor_start_index = explicit_native_prg_library ? 3U : 2U;
            std::vector<PrgValue> child_constructor_arguments;
            std::vector<std::optional<std::string>> child_argument_references;
            child_constructor_arguments.reserve(arguments.size() > constructor_start_index ? arguments.size() - constructor_start_index : 0U);
            child_argument_references.reserve(argument_references.size() > constructor_start_index ? argument_references.size() - constructor_start_index : 0U);
            for (std::size_t index = constructor_start_index; index < arguments.size(); ++index)
            {
                child_constructor_arguments.push_back(arguments[index]);
                child_argument_references.push_back(
                    index < argument_references.size()
                        ? argument_references[index]
                        : std::optional<std::string>{});
            }

            const std::string primary_child_program_path =
                explicit_native_prg_library
                    ? resolve_native_prg_program_path(child_library, implicit_child_program_path)
                    : implicit_child_program_path;
            RuntimeOleObjectState *child_object = instantiate_native_class_object(
                frame,
                child_class,
                primary_child_program_path,
                "addobject",
                child_constructor_arguments,
                child_argument_references,
                make_runtime_object_reference(*target_object));
            if (child_object == nullptr && !explicit_native_prg_library)
            {
                const std::string owner_program_path = normalize_path(target_object->source);
                if (!owner_program_path.empty() &&
                    owner_program_path != normalize_path(primary_child_program_path))
                {
                    child_object = instantiate_native_class_object(
                        frame,
                        child_class,
                        owner_program_path,
                        "addobject",
                        child_constructor_arguments,
                        child_argument_references,
                        make_runtime_object_reference(*target_object));
                }
            }
            if (child_object == nullptr)
            {
                return make_boolean_value(false);
            }

            assign_native_runtime_object_name(*child_object, child_name_text);
            target_object->properties[child_name] = make_runtime_object_reference(*child_object);
            if (child_object->properties.contains("columnorder"))
            {
                (void)write_native_columnorder_property(
                    *child_object,
                    child_object->properties["columnorder"]);
            }
            if (is_native_column_runtime_object(*target_object) &&
                native_column_bound_value(*target_object))
            {
                sync_native_column_child_controlsources(*target_object);
            }
            (void)sync_native_owned_children_collection(*target_object);
            target_object->last_action = effective_member_path + "(" + child_name + "," + child_class + ")";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.addobject",
                              .detail = target_object->prog_id + "." + child_name + ":" + child_class,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(true);
        }
        if (leaf == "removeobject" && !target_object->source.empty() && !arguments.empty())
        {
            const std::string child_name = normalize_identifier(trim_copy(value_as_string(arguments[0])));
            if (child_name.empty())
            {
                return make_boolean_value(false);
            }

            const auto child_property = target_object->properties.find(child_name);
            if (child_property == target_object->properties.end())
            {
                return make_boolean_value(false);
            }

            const auto child_object = resolve_ole_object(child_property->second);
            if (!child_object.has_value())
            {
                return make_boolean_value(false);
            }
            if ((*child_object)->hidden_runtime_surface)
            {
                return make_boolean_value(false);
            }

            const auto child_parent = native_object_parent_reference(**child_object);
            int parent_handle = 0;
            std::string parent_prog_id;
            if (!child_parent.has_value() ||
                !parse_object_handle_reference(*child_parent, parent_handle, parent_prog_id) ||
                parent_handle != target_object->handle)
            {
                return make_boolean_value(false);
            }

            (*child_object)->properties.erase("parent");
            target_object->properties.erase(child_name);
            (void)sync_native_owned_children_collection(*target_object);
            target_object->last_action = effective_member_path + "(" + child_name + ")";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.removeobject",
                              .detail = target_object->prog_id + "." + child_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(true);
        }
        if (leaf == "setall" && !target_object->source.empty())
        {
            return apply_native_setall(
                *target_object,
                frame,
                effective_member_path,
                arguments);
        }
        if (leaf == "release" && !target_object->source.empty())
        {
            last_popped_frame_requested_nodefault = false;
            if (auto native_result = invoke_native_object_method_if_present(
                    *target_object,
                    leaf,
                    frame,
                    arguments,
                    argument_references);
                native_result.has_value())
            {
                if (consume_last_popped_frame_requested_nodefault())
                {
                    return *native_result;
                }
            }
            return release_native_object(*target_object, effective_member_path);
        }
        if (auto native_result = invoke_native_object_method_if_present(
                *target_object,
                leaf,
                frame,
                arguments,
                argument_references);
            native_result.has_value())
        {
            return *native_result;
        }
        const bool is_report_listener_object = std::any_of(
            target_object->class_hierarchy.begin(),
            target_object->class_hierarchy.end(),
            [](const std::string &class_name)
            {
                return normalize_identifier(trim_copy(class_name)) == "reportlistener";
            }) ||
            normalize_identifier(trim_copy(target_object->base_class_name)) == "reportlistener";
        if (is_report_listener_object && leaf == "createconfigtable")
        {
            std::string result_code = "invalid-path";
            bool created = false;
            if (!arguments.empty() && !trim_copy(value_as_string(arguments.front())).empty())
            {
                std::filesystem::path requested_path = copperfin::platform::path_from_utf8_string(
                    value_as_string(arguments.front()));
                if (requested_path.extension().empty())
                {
                    requested_path.replace_extension(".dbf");
                }
                if (requested_path.is_relative())
                {
                    requested_path = copperfin::platform::path_from_utf8_string(current_default_directory()) /
                        requested_path;
                }

                const bool overwrite = arguments.size() >= 2U && value_as_bool(arguments[1]);
                const auto existing_path = copperfin::vfp::resolve_unique_casefold_path(requested_path);
                std::filesystem::path memo_path = requested_path;
                memo_path.replace_extension(".fpt");
                std::filesystem::path index_path = requested_path;
                index_path.replace_extension(".cdx");
                const auto existing_memo_path = copperfin::vfp::resolve_unique_casefold_path(memo_path);
                const auto existing_index_path = copperfin::vfp::resolve_unique_casefold_path(index_path);
                if (existing_path.ambiguous || existing_memo_path.ambiguous || existing_index_path.ambiguous)
                {
                    result_code = "ambiguous-path";
                }
                else if (options.require_verified_file_byte_overrides)
                {
                    result_code = "verified-write-rejected";
                }
                else if ((existing_path.path.has_value() || existing_memo_path.path.has_value() ||
                          existing_index_path.path.has_value()) && !overwrite)
                {
                    result_code = "exists";
                }
                else
                {
                    const std::filesystem::path target_path = existing_path.path.value_or(requested_path);
                    const auto create_result = copperfin::vfp::create_dbf_table_file(
                        copperfin::platform::path_to_utf8_string(target_path),
                        {
                            {.name = "OBJTYPE", .type = 'I', .length = 4U},
                            {.name = "OBJCODE", .type = 'I', .length = 4U},
                            {.name = "OBJNAME", .type = 'V', .length = 60U},
                            {.name = "OBJVALUE", .type = 'V', .length = 60U},
                            {.name = "OBJINFO", .type = 'M', .length = 4U}
                        },
                        {});
                    created = create_result.ok;
                    result_code = created ? "created" : "write-failed";
                    if (created)
                    {
                        target_object->properties["configurationtable"] = make_string_value(
                            copperfin::platform::path_to_utf8_string(target_path.lexically_normal()));
                    }
                }
            }
            target_object->properties["haderror"] = make_boolean_value(!created);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.reportlistener.createconfigtable",
                              .detail = target_object->prog_id + ":" + result_code,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(created);
        }
        if (is_report_listener_object && leaf == "getconfigtable")
        {
            const std::filesystem::path working_directory =
                copperfin::platform::path_from_utf8_string(current_default_directory());
            const auto resolve_configuration_path = [&](const std::filesystem::path &candidate)
                -> std::optional<std::filesystem::path>
            {
                if (options.require_verified_file_byte_overrides)
                {
                    bool ambiguous = false;
                    return resolve_verified_file_byte_override_path(candidate, ambiguous, true);
                }
                const auto resolution = copperfin::vfp::resolve_unique_casefold_path(candidate);
                if (resolution.ambiguous || !resolution.path.has_value())
                {
                    return std::nullopt;
                }
                return resolution.path;
            };
            std::filesystem::path requested_path;
            if (const auto configured = target_object->properties.find("configurationtable");
                configured != target_object->properties.end() &&
                !trim_copy(value_as_string(configured->second)).empty())
            {
                requested_path = copperfin::platform::path_from_utf8_string(
                    value_as_string(configured->second));
                if (requested_path.is_relative())
                {
                    requested_path = working_directory / requested_path;
                }
            }
            else
            {
                const std::array<std::filesystem::path, 2U> candidates = {
                    working_directory / "OutputConfig.dbf",
                    working_directory / "_ReportOutputConfig.dbf"};
                for (const auto &candidate : candidates)
                {
                    if (const auto resolved = resolve_configuration_path(candidate);
                        resolved.has_value())
                    {
                        requested_path = *resolved;
                        break;
                    }
                }
            }

            std::optional<std::filesystem::path> resolved_path;
            if (!requested_path.empty())
            {
                resolved_path = resolve_configuration_path(requested_path);
            }
            const std::string config_path = resolved_path.has_value()
                ? copperfin::platform::path_to_utf8_string(resolved_path->lexically_normal())
                : std::string{};
            target_object->properties["configurationtable"] = make_string_value(config_path);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.reportlistener.getconfigtable",
                              .detail = target_object->prog_id + ":" + config_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_string_value(config_path);
        }
        if (is_report_listener_object && leaf == "verifyconfigtable")
        {
            const auto configured = target_object->properties.find("configurationtable");
            const std::string configured_path = configured == target_object->properties.end()
                ? std::string{}
                : trim_copy(value_as_string(configured->second));
            std::string result_code = "missing";
            bool valid = false;
            if (!configured_path.empty())
            {
                const auto requested_path = copperfin::platform::path_from_utf8_string(configured_path);
                std::optional<std::filesystem::path> resolved_path;
                if (options.require_verified_file_byte_overrides)
                {
                    bool ambiguous = false;
                    resolved_path = resolve_verified_file_byte_override_path(requested_path, ambiguous, true);
                    if (ambiguous)
                    {
                        resolved_path.reset();
                    }
                }
                else
                {
                    const auto resolution = copperfin::vfp::resolve_unique_casefold_path(requested_path);
                    if (!resolution.ambiguous && resolution.path.has_value())
                    {
                        resolved_path = resolution.path;
                    }
                }
                if (resolved_path.has_value())
                {
                    std::filesystem::path snapshot_root;
                    const auto table_path = options.require_verified_file_byte_overrides
                        ? materialize_verified_table_snapshot(*resolved_path, snapshot_root)
                        : resolved_path;
                    const auto table = table_path.has_value()
                        ? copperfin::vfp::parse_dbf_table_from_file(
                              copperfin::platform::path_to_utf8_string(*table_path),
                              std::numeric_limits<std::size_t>::max())
                        : copperfin::vfp::DbfTableParseResult{};
                    if (table.ok)
                    {
                        const auto has_exactly_one_field = [&](const std::string &name) {
                            const std::string normalized_name = normalize_identifier(name);
                            return std::count_if(
                                       table.table.fields.begin(),
                                       table.table.fields.end(),
                                       [&](const copperfin::vfp::DbfFieldDescriptor &field)
                                       {
                                           return normalize_identifier(field.name) == normalized_name;
                                       }) == 1;
                        };
                        const auto field_matches = [&](const std::string &name, const std::string &types) {
                            const std::string normalized_name = normalize_identifier(name);
                            const auto field = std::find_if(
                                table.table.fields.begin(),
                                table.table.fields.end(),
                                [&](const copperfin::vfp::DbfFieldDescriptor &candidate)
                                {
                                    return normalize_identifier(candidate.name) == normalized_name;
                                });
                            return field != table.table.fields.end() &&
                                types.find(field->type) != std::string::npos;
                        };
                        valid = has_exactly_one_field("OBJTYPE") &&
                            has_exactly_one_field("OBJCODE") &&
                            has_exactly_one_field("OBJNAME") &&
                            has_exactly_one_field("OBJVALUE") &&
                            has_exactly_one_field("OBJINFO") &&
                            field_matches("OBJTYPE", "IN") &&
                            field_matches("OBJCODE", "IN") &&
                            field_matches("OBJNAME", "CV") &&
                            field_matches("OBJVALUE", "CV") &&
                            field_matches("OBJINFO", "MCV");
                        result_code = valid ? "valid" : "unsupported";
                    }
                    else
                    {
                        result_code = "unsupported";
                    }
                    if (!snapshot_root.empty())
                    {
                        std::error_code snapshot_error;
                        std::filesystem::remove_all(snapshot_root, snapshot_error);
                    }
                }
            }
            target_object->properties["haderror"] = make_boolean_value(!valid);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.reportlistener.verifyconfigtable",
                              .detail = target_object->prog_id + ":" + result_code,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(valid);
        }
        std::function<bool(const std::vector<PrgValue>&)> before_list_control_move;
        if (leaf == "moveitem")
        {
            before_list_control_move = [&](const std::vector<PrgValue>& event_arguments)
            {
                bool requested_nodefault = false;
                bool returned_false = false;
                (void)invoke_native_object_method_if_present(
                    *target_object,
                    "onmoveitem",
                    frame,
                    event_arguments,
                    std::vector<std::optional<std::string>>(event_arguments.size()),
                    &requested_nodefault,
                    &returned_false);
                (void)consume_last_popped_frame_requested_nodefault();
                return !returned_false;
            };
        }
        const auto before_list_control_signature =
            native_list_control_selection_signature(*target_object);
        if (auto list_control_result = invoke_native_list_control_method(
                *target_object,
                leaf,
                arguments,
                before_list_control_move);
            list_control_result.has_value())
        {
            if (leaf == "additem" || leaf == "addlistitem" || leaf == "clear" ||
                leaf == "removeitem" || leaf == "removelistitem")
            {
                write_native_list_control_controlsource_target(*target_object, frame);
            }
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            if (leaf == "additem" || leaf == "addlistitem" || leaf == "clear" || leaf == "removeitem" ||
                leaf == "removelistitem")
            {
                events.push_back({.category = "prg.object." + leaf,
                                  .detail = target_object->prog_id + "." + effective_member_path,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            }
            invoke_native_list_control_programmatic_change_if_needed(
                *target_object,
                frame,
                before_list_control_signature);
            return *list_control_result;
        }
        if (leaf == "readexpression" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.empty())
            {
                return make_string_value("");
            }

            const auto expression_text =
                read_native_property_expression_if_present(*target_object, value_as_string(arguments.front()));
            target_object->last_action = effective_member_path + "(" + value_as_string(arguments.front()) + ")";
            ++target_object->action_count;
            return make_string_value(expression_text.value_or(std::string{}));
        }
        if (leaf == "readmethod" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.empty())
            {
                return make_string_value("");
            }

            const auto source_text =
                read_native_method_source_if_present(*target_object, value_as_string(arguments.front()));
            target_object->last_action = effective_member_path + "(" + value_as_string(arguments.front()) + ")";
            ++target_object->action_count;
            return make_string_value(source_text.value_or(std::string{}));
        }
        if (leaf == "writeexpression" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.size() < 2U)
            {
                return make_empty_value();
            }

            const std::string property_name = trim_copy(value_as_string(arguments[0]));
            const std::string expression_text = value_as_string(arguments[1]);
            const PrgValue assigned_value = evaluate_expression(expression_text, frame);
            if (!write_native_property_if_present(
                    *target_object,
                    property_name,
                    assigned_value,
                    frame,
                    expression_text))
            {
                return make_empty_value();
            }

            target_object->last_action = effective_member_path + "(" + property_name + ")";
            ++target_object->action_count;
            return make_empty_value();
        }
        if (leaf == "writemethod" &&
            (!target_object->class_hierarchy.empty() || !target_object->source.empty()))
        {
            if (arguments.size() < 2U)
            {
                return make_empty_value();
            }

            const std::string method_name = trim_copy(value_as_string(arguments[0]));
            const std::string method_source_text = value_as_string(arguments[1]);
            const bool create_if_missing =
                arguments.size() >= 3U &&
                value_as_bool(arguments[2]);
            if (!write_native_method_source_if_present(
                    *target_object,
                    method_name,
                    method_source_text,
                    create_if_missing))
            {
                return make_empty_value();
            }

            target_object->last_action = effective_member_path + "(" + method_name + ")";
            ++target_object->action_count;
            return make_empty_value();
        }
        if (leaf == "move" &&
            is_native_visual_runtime_object(*target_object))
        {
            if (arguments.empty())
            {
                return make_empty_value();
            }

            const bool left_written = write_native_property_if_present(
                *target_object,
                "left",
                make_number_value(value_as_number(arguments[0])),
                frame);
            if (!left_written)
            {
                return make_empty_value();
            }

            if (arguments.size() >= 2U)
            {
                (void)write_native_property_if_present(
                    *target_object,
                    "top",
                    make_number_value(value_as_number(arguments[1])),
                    frame);
            }
            if (arguments.size() >= 3U)
            {
                (void)write_native_property_if_present(
                    *target_object,
                    "width",
                    make_number_value(value_as_number(arguments[2])),
                    frame);
            }
            if (arguments.size() >= 4U)
            {
                (void)write_native_property_if_present(
                    *target_object,
                    "height",
                    make_number_value(value_as_number(arguments[3])),
                    frame);
            }

            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.move",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            last_popped_frame_requested_nodefault = false;
            bool moved_requested_nodefault = false;
            (void)invoke_native_object_method_if_present(
                *target_object,
                "moved",
                frame,
                {},
                {},
                &moved_requested_nodefault);
            (void)consume_last_popped_frame_requested_nodefault();
            (void)moved_requested_nodefault;
            return make_empty_value();
        }
        if (leaf == "refresh" && !target_object->class_hierarchy.empty())
        {
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.refresh",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            last_popped_frame_requested_nodefault = false;
            bool paint_requested_nodefault = false;
            (void)invoke_native_object_method_if_present(
                *target_object,
                "paint",
                frame,
                {},
                {},
                &paint_requested_nodefault);
            (void)consume_last_popped_frame_requested_nodefault();
            (void)paint_requested_nodefault;
            return make_empty_value();
        }
        if ((leaf == "show" || leaf == "hide") &&
            is_native_visual_runtime_object(*target_object))
        {
            const bool visible = leaf == "show";
            (void)write_native_property_if_present(
                *target_object,
                "visible",
                make_boolean_value(visible),
                frame);
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = visible ? "prg.object.show" : "prg.object.hide",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            last_popped_frame_requested_nodefault = false;
            bool handler_requested_nodefault = false;
            (void)invoke_native_object_method_if_present(
                *target_object,
                visible ? "activate" : "deactivate",
                frame,
                {},
                {},
                &handler_requested_nodefault);
            (void)consume_last_popped_frame_requested_nodefault();
            const bool suppress_default_activation = visible && handler_requested_nodefault;
            if (visible && !suppress_default_activation)
            {
                note_representative_active_form(*target_object);
            }
            return make_empty_value();
        }
        if (leaf == "setfocus" &&
            is_native_focusable_runtime_object(*target_object))
        {
            (void)set_native_focus(*target_object, effective_member_path, frame);
            return make_empty_value();
        }
        if (leaf == "resettodefault" && !target_object->class_hierarchy.empty())
        {
            if (arguments.empty())
            {
                return make_boolean_value(false);
            }

            const std::string property_name = trim_copy(value_as_string(arguments.front()));
            const std::string normalized_property_name = normalize_identifier(property_name);
            if (normalized_property_name.empty())
            {
                return make_boolean_value(false);
            }

            const auto default_value = target_object->default_properties.find(normalized_property_name);
            if (default_value == target_object->default_properties.end())
            {
                return make_boolean_value(false);
            }

            std::optional<std::string> default_expression_text;
            if (const auto default_texts =
                    native_default_property_expression_text_by_handle.find(target_object->handle);
                default_texts != native_default_property_expression_text_by_handle.end())
            {
                if (const auto expression_text = default_texts->second.find(normalized_property_name);
                    expression_text != default_texts->second.end())
                {
                    default_expression_text = expression_text->second;
                }
            }

            const bool restored = write_native_property_if_present(
                *target_object,
                property_name,
                default_value->second,
                frame,
                default_expression_text);
            if (!restored)
            {
                return make_boolean_value(false);
            }

            target_object->last_action = effective_member_path + "(" + property_name + ")";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.resettodefault",
                              .detail = target_object->prog_id + "." + normalized_property_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_boolean_value(true);
        }
        const auto before_requery_signature =
            native_list_control_selection_signature(*target_object);
        if (leaf == "requery" && requery_native_list_control(*target_object, frame))
        {
            if (!write_native_list_control_controlsource_target(*target_object, frame))
            {
                return make_boolean_value(false);
            }
            target_object->last_action = effective_member_path + "()";
            ++target_object->action_count;
            events.push_back({.category = "prg.object.requery",
                              .detail = target_object->prog_id + "." + effective_member_path,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            invoke_native_list_control_programmatic_change_if_needed(
                *target_object,
                frame,
                before_requery_signature);
            return make_empty_value();
        }
        if (is_native_olecontrol_host_object(*target_object) && leaf == "doverb")
        {
            RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(*target_object);
            if (object_surface == nullptr)
            {
                return make_boolean_value(false);
            }

            const PrgValue verb = arguments.empty()
                                      ? make_number_value(0.0)
                                      : canonicalize_native_olecontrol_doverb_argument(arguments.front());
            target_object->last_action = effective_member_path + "(" + format_value(verb) + ")";
            ++target_object->action_count;
            events.push_back({.category = "ole.invoke",
                              .detail = target_object->prog_id + "." + effective_member_path + ":" + format_value(verb),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            object_surface->last_action = "activate:" + format_value(verb);
            ++object_surface->action_count;
            return make_boolean_value(true);
        }
        if (is_native_olecontrol_host_object(*target_object) && leaf == "objectverbs")
        {
            return read_native_olecontrol_objectverb_by_index(*target_object, arguments).value_or(make_empty_value());
        }
        if (is_native_olecontrol_host_object(*target_object))
        {
            RuntimeOleObjectState *object_surface = ensure_native_olecontrol_object_surface(*target_object);
            if (object_surface != nullptr)
            {
                if (auto nested_native_result = invoke_native_object_method_if_present(
                        *object_surface,
                        leaf,
                        frame,
                        arguments,
                        argument_references);
                    nested_native_result.has_value())
                {
                    return *nested_native_result;
                }
                target_object = object_surface;
            }
        }

        target_object->last_action = effective_member_path + "()";
        ++target_object->action_count;
        events.push_back({.category = "ole.invoke",
                          .detail = target_object->prog_id + "." + effective_member_path,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        if (auto collection_result = invoke_native_collection_method(*target_object, leaf, arguments);
            collection_result.has_value())
        {
            return *collection_result;
        }
        if (normalize_identifier(target_object->prog_id) == "scripting.dictionary")
        {
            auto update_dictionary_count = [&]()
            {
                std::size_t entry_count = 0U;
                for (const auto &[property_name, property_value] : target_object->properties)
                {
                    if (property_name != "count" && property_name != "comparemode")
                    {
                        ++entry_count;
                    }
                }
                target_object->properties["count"] = make_number_value(static_cast<double>(entry_count));
            };
            const auto key_for_argument = [&](std::size_t index) -> std::string
            {
                return index < arguments.size()
                    ? normalize_identifier(trim_copy(value_as_string(arguments[index])))
                    : std::string{};
            };

            if (leaf == "add" && arguments.size() >= 2U)
            {
                const std::string key = key_for_argument(0U);
                if (!key.empty())
                {
                    target_object->properties[key] = arguments[1];
                    update_dictionary_count();
                }
                return make_boolean_value(true);
            }
            if (leaf == "exists" && !arguments.empty())
            {
                const std::string key = key_for_argument(0U);
                return make_boolean_value(!key.empty() && target_object->properties.contains(key));
            }
            if (leaf == "item" && !arguments.empty())
            {
                const std::string key = key_for_argument(0U);
                const auto found = target_object->properties.find(key);
                return found == target_object->properties.end() ? make_empty_value() : found->second;
            }
            if (leaf == "remove" && !arguments.empty())
            {
                const std::string key = key_for_argument(0U);
                if (!key.empty())
                {
                    target_object->properties.erase(key);
                    update_dictionary_count();
                }
                return make_boolean_value(true);
            }
            if (leaf == "removeall")
            {
                const auto comparemode = target_object->properties.find("comparemode");
                const PrgValue comparemode_value = comparemode == target_object->properties.end()
                    ? make_number_value(0.0)
                    : comparemode->second;
                target_object->properties.clear();
                target_object->properties["comparemode"] = comparemode_value;
                target_object->properties["count"] = make_number_value(0.0);
                return make_boolean_value(true);
            }

            const Statement *statement = current_statement();
            const std::string action_text = statement == nullptr
                ? target_object->prog_id + "." + effective_member_path + "()"
                : statement->text;
            record_ole_aerror_context(target_object->prog_id + "." + effective_member_path + "()",
                                      "Copperfin OLE",
                                      target_object->prog_id,
                                      action_text,
                                      1429);
            throw std::runtime_error(
                runtime_text(
                    "Runtime.Prg.Core.Error.OleMemberNotFoundForMethodInvocation",
                    {{"memberIdentifier", target_object->prog_id + "." + effective_member_path}}));
        }

        if (leaf == "add" || leaf == "create" || leaf == "open" || leaf == "item")
        {
            return make_string_value("object:" + target_object->prog_id + "." + effective_member_path + "#" + std::to_string(target_object->handle));
        }
        if (arguments.empty())
        {
            return make_string_value("ole:" + target_object->prog_id + "." + effective_member_path);
        }
        return arguments.front();
    }
