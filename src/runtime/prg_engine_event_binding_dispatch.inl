    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_native_event_delegate(
        const NativeEventBinding &binding,
        const CurrentNativeEventContext &event_context,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        struct CurrentEventGuard
        {
            std::vector<CurrentNativeEventContext> &stack;

            ~CurrentEventGuard()
            {
                if (!stack.empty())
                {
                    stack.pop_back();
                }
            }
        };

        active_native_event_contexts.push_back(event_context);
        CurrentEventGuard guard{active_native_event_contexts};

        if (binding.target_is_routine)
        {
            Program &program = load_program(binding.target_program_path);
            const auto found = program.routines.find(normalize_identifier(binding.delegate_name));
            if (found == program.routines.end())
            {
                return std::nullopt;
            }
            if (!can_push_frame())
            {
                throw std::runtime_error(call_depth_limit_message());
            }

            events.push_back({.category = "prg.event.delegate",
                              .detail = binding.event_name + " -> " + found->second.name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            const std::size_t return_depth = stack.size();
            push_routine_frame(program.path, found->second, arguments, argument_references);
            return run_expression_invoked_routine_until_return(return_depth);
        }

        const auto target_found = ole_objects.find(binding.target_handle);
        if (target_found == ole_objects.end())
        {
            return std::nullopt;
        }

        RuntimeOleObjectState &target_object = target_found->second;
        std::string method_program_path;
        std::string method_name;
        if (const Routine *method = find_native_object_method(
                target_object,
                binding.delegate_name,
                method_program_path,
                method_name);
            method != nullptr)
        {
            if (!can_push_frame())
            {
                throw std::runtime_error(call_depth_limit_message());
            }

            target_object.last_action = "bindevent:" + binding.delegate_name;
            ++target_object.action_count;
            events.push_back({.category = "prg.event.delegate",
                              .detail = binding.event_name + " -> " + method_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            const std::size_t return_depth = stack.size();
            const PrgValue this_reference =
                make_string_value("object:" + target_object.prog_id + "#" + std::to_string(target_object.handle));
            push_method_frame(method_program_path,
                              method_name,
                              *method,
                              this_reference,
                              method_name.substr(0U, method_name.rfind('.')),
                              normalize_identifier(binding.delegate_name),
                              native_object_parent_reference(target_object),
                              native_object_owner_form_reference(target_object),
                              native_object_owner_formset_reference(target_object),
                              arguments,
                              argument_references);
            return run_expression_invoked_routine_until_return(return_depth);
        }

        return std::nullopt;
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_window_message_delegate(
        const WindowMessageBinding &binding,
        const CurrentWindowMessageContext &message_context,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        struct CurrentMessageGuard
        {
            std::vector<CurrentWindowMessageContext> &stack;

            ~CurrentMessageGuard()
            {
                if (!stack.empty())
                {
                    stack.pop_back();
                }
            }
        };

        active_window_message_contexts.push_back(message_context);
        CurrentMessageGuard guard{active_window_message_contexts};

        const auto target_found = ole_objects.find(binding.target_handle);
        if (target_found == ole_objects.end())
        {
            return std::nullopt;
        }

        RuntimeOleObjectState &target_object = target_found->second;
        std::string method_program_path;
        std::string method_name;
        if (const Routine *method = find_native_object_method(
                target_object,
                binding.delegate_name,
                method_program_path,
                method_name);
            method != nullptr)
        {
            if (!can_push_frame())
            {
                throw std::runtime_error(call_depth_limit_message());
            }

            target_object.last_action = "bindevent:" + binding.delegate_name;
            ++target_object.action_count;
            events.push_back({.category = "prg.event.delegate",
                              .detail = std::to_string(message_context.window_handle) + ":" +
                                            std::to_string(message_context.message) + " -> " + method_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            const std::size_t return_depth = stack.size();
            const PrgValue this_reference =
                make_string_value("object:" + target_object.prog_id + "#" + std::to_string(target_object.handle));
            push_method_frame(method_program_path,
                              method_name,
                              *method,
                              this_reference,
                              method_name.substr(0U, method_name.rfind('.')),
                              normalize_identifier(binding.delegate_name),
                              native_object_parent_reference(target_object),
                              native_object_owner_form_reference(target_object),
                              native_object_owner_formset_reference(target_object),
                              arguments,
                              argument_references);
            return run_expression_invoked_routine_until_return(return_depth);
        }

        return std::nullopt;
    }

    std::optional<PrgValue> PrgRuntimeSession::Impl::invoke_native_object_method_if_present(
        RuntimeOleObjectState &runtime_object,
        const std::string &identifier,
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references,
        bool *requested_nodefault,
        bool *returned_false)
    {
        if (returned_false != nullptr)
        {
            *returned_false = false;
        }
        const std::string normalized_identifier = normalize_identifier(identifier);
        std::vector<NativeEventBinding> bindings;
        bindings.reserve(native_event_bindings.size());
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            if (binding.source_handle == runtime_object.handle &&
                binding.event_name == normalized_identifier &&
                (binding.flags & 2) == 0)
            {
                bindings.push_back(binding);
            }
        }

        auto invoke_delegates_for_phase = [&](bool after_source_method)
        {
            for (const NativeEventBinding &binding : bindings)
            {
                const bool binding_after_source_method = (binding.flags & 1) == 0;
                if (binding_after_source_method == after_source_method)
                {
                    const auto delegate_result = invoke_native_event_delegate(
                        binding,
                        {.source_handle = runtime_object.handle,
                         .event_name = normalized_identifier,
                         .event_type = 2},
                        arguments,
                        argument_references);
                    if (returned_false != nullptr &&
                        delegate_result.has_value() &&
                        delegate_result->kind != PrgValueKind::empty &&
                        !value_as_bool(*delegate_result))
                    {
                        *returned_false = true;
                    }
                }
            }
        };

        const std::string active_event_key =
            std::to_string(runtime_object.handle) + ":" + normalized_identifier;
        const bool already_active =
            active_native_event_keys.find(active_event_key) != active_native_event_keys.end();

        if (!bindings.empty() && !already_active)
        {
            ActiveNativeEventKeyGuard active_event_guard(active_native_event_keys, active_event_key);
            invoke_delegates_for_phase(false);
            auto result = invoke_native_object_method_body_if_present(
                runtime_object,
                normalized_identifier,
                source_frame,
                arguments,
                argument_references,
                requested_nodefault);
            if (returned_false != nullptr &&
                result.has_value() &&
                result->kind != PrgValueKind::empty &&
                !value_as_bool(*result))
            {
                *returned_false = true;
            }
            invoke_delegates_for_phase(true);
            return result;
        }

        auto result = invoke_native_object_method_body_if_present(
            runtime_object,
            normalized_identifier,
            source_frame,
            arguments,
            argument_references,
            requested_nodefault);
        if (returned_false != nullptr &&
            result.has_value() &&
            result->kind != PrgValueKind::empty &&
            !value_as_bool(*result))
        {
            *returned_false = true;
        }
        return result;
    }

    void PrgRuntimeSession::Impl::invoke_native_list_control_programmatic_change_if_needed(
        RuntimeOleObjectState &runtime_object,
        const Frame &source_frame,
        const std::optional<std::string> &before_signature)
    {
        if (!before_signature.has_value()) {
            return;
        }

        const auto after_signature = native_list_control_selection_signature(runtime_object);
        if (!after_signature.has_value() || *after_signature == *before_signature) {
            return;
        }

        bool requested_nodefault = false;
        (void)invoke_native_object_method_if_present(
            runtime_object,
            "programmaticchange",
            source_frame,
            {},
            {},
            &requested_nodefault);
        (void)consume_last_popped_frame_requested_nodefault();
    }

    PrgValue PrgRuntimeSession::Impl::bind_native_event(
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        (void)argument_references;
        if (arguments.size() < 3U)
        {
            return make_number_value(0.0);
        }

        int object_handle_probe = 0;
        std::string object_prog_id_probe;
        const bool source_is_object =
            parse_object_handle_reference(arguments[0], object_handle_probe, object_prog_id_probe);
        const bool looks_like_window_message_binding =
            arguments.size() >= 4U &&
            !source_is_object &&
            arguments[1].kind != PrgValueKind::string;
        if (looks_like_window_message_binding)
        {
            const int window_handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const int message = static_cast<int>(std::llround(value_as_number(arguments[1])));
            if (message == 0)
            {
                return make_number_value(0.0);
            }

            auto target_object = resolve_ole_object(arguments[2]);
            const std::string delegate_name = trim_copy(value_as_string(arguments[3]));
            if (!target_object.has_value() || (*target_object)->source.empty() || delegate_name.empty())
            {
                return make_number_value(0.0);
            }

            std::string target_program_path;
            std::string target_method_name;
            if (const Routine *target_method = find_native_object_method(
                    **target_object,
                    delegate_name,
                    target_program_path,
                    target_method_name);
                target_method == nullptr)
            {
                return make_number_value(0.0);
            }

            WindowMessageBinding binding;
            binding.window_handle = window_handle;
            binding.message = message;
            binding.target_handle = (*target_object)->handle;
            binding.delegate_name = delegate_name;
            binding.ordinal = next_native_event_binding_ordinal++;

            const auto duplicate = std::find_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&](const WindowMessageBinding &existing)
                {
                    return existing.window_handle == binding.window_handle &&
                           existing.message == binding.message &&
                           existing.target_handle == binding.target_handle &&
                           normalize_identifier(existing.delegate_name) == normalize_identifier(binding.delegate_name);
                });

            if (duplicate == window_message_bindings.end())
            {
                window_message_bindings.push_back(binding);
            }

            const auto binding_count = static_cast<double>(std::count_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&](const WindowMessageBinding &existing)
                {
                    return existing.window_handle == binding.window_handle &&
                           existing.message == binding.message;
                }));

            events.push_back({.category = "prg.event.bind",
                              .detail = std::to_string(window_handle) + ":" + std::to_string(message) +
                                            " -> " + binding.delegate_name,
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            return make_number_value(binding_count);
        }

        auto source_object = resolve_ole_object(arguments[0]);
        const bool source_is_native_list_control =
            source_object.has_value() &&
            (normalize_identifier(trim_copy((*source_object)->base_class_name)) == "combobox" ||
             normalize_identifier(trim_copy((*source_object)->base_class_name)) == "listbox");
        if (!source_object.has_value() ||
            ((*source_object)->source.empty() && !source_is_native_list_control))
        {
            return make_number_value(0.0);
        }

        const std::string event_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (event_name.empty())
        {
            return make_number_value(0.0);
        }

        NativeEventBinding binding;
        binding.source_handle = (*source_object)->handle;
        binding.event_name = event_name;
        binding.ordinal = next_native_event_binding_ordinal++;

        auto target_object = arguments.size() >= 4U ? resolve_ole_object(arguments[2]) : std::optional<RuntimeOleObjectState *>{};
        if (arguments.size() >= 4U && target_object.has_value())
        {
            const std::string delegate_name = trim_copy(value_as_string(arguments[3]));
            if (!target_object.has_value() || (*target_object)->source.empty() || delegate_name.empty())
            {
                return make_number_value(0.0);
            }

            std::string target_program_path;
            std::string target_method_name;
            if (const Routine *target_method = find_native_object_method(
                    **target_object,
                    delegate_name,
                    target_program_path,
                    target_method_name);
                target_method == nullptr)
            {
                return make_number_value(0.0);
            }

            binding.target_is_routine = false;
            binding.target_handle = (*target_object)->handle;
            binding.delegate_name = delegate_name;
            binding.flags = arguments.size() >= 5U
                                ? static_cast<int>(std::llround(value_as_number(arguments[4]))) & 3
                                : 0;
        }
        else
        {
            const std::string routine_name = trim_copy(value_as_string(arguments[2]));
            if (routine_name.empty())
            {
                return make_number_value(0.0);
            }

            const auto found = find_unqualified_routine_lookup(source_frame.file_path, routine_name);
            if (!found.has_value())
            {
                return make_number_value(0.0);
            }

            binding.target_is_routine = true;
            binding.target_program_path = found->program->path;
            binding.delegate_name = found->routine->name;
            binding.flags = arguments.size() >= 4U
                                ? static_cast<int>(std::llround(value_as_number(arguments[3]))) & 3
                                : 0;
        }

        const auto duplicate = std::find_if(
            native_event_bindings.begin(),
            native_event_bindings.end(),
            [&](const NativeEventBinding &existing)
            {
                return existing.source_handle == binding.source_handle &&
                       existing.event_name == binding.event_name &&
                       existing.target_is_routine == binding.target_is_routine &&
                       existing.target_program_path == binding.target_program_path &&
                       existing.target_handle == binding.target_handle &&
                       normalize_identifier(existing.delegate_name) == normalize_identifier(binding.delegate_name);
            });

        if (duplicate == native_event_bindings.end())
        {
            native_event_bindings.push_back(binding);
        }
        else
        {
            duplicate->flags = binding.flags;
        }

        const auto binding_count = static_cast<double>(std::count_if(
            native_event_bindings.begin(),
            native_event_bindings.end(),
            [&](const NativeEventBinding &existing)
            {
                return existing.source_handle == binding.source_handle &&
                       existing.event_name == binding.event_name;
            }));

        events.push_back({.category = "prg.event.bind",
                          .detail = (*source_object)->prog_id + "." + event_name + " -> " + binding.delegate_name,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        return make_number_value(binding_count);
    }

    void PrgRuntimeSession::Impl::retire_com_eventhandler_binding(std::size_t ordinal)
    {
        const auto found = std::find_if(
            com_eventhandler_bindings.begin(),
            com_eventhandler_bindings.end(),
            [ordinal](const ComEventHandlerBinding &binding)
            {
                return binding.ordinal == ordinal;
            });
        if (found == com_eventhandler_bindings.end())
        {
            return;
        }

        // The host contract requires disconnect to quiesce its callback before
        // return. Do it before dropping the runtime binding so a callback can
        // never retain an established subscription after explicit unbind,
        // release, or a contained delivery fault.
        if (found->disconnect_subscription)
        {
            try
            {
                found->disconnect_subscription();
            }
            catch (...)
            {
                // A host teardown failure must not preserve the runtime
                // binding. The weak queue sink prevents later callbacks from
                // acquiring mutable runtime state.
            }
        }
        com_eventhandler_bindings.erase(found);
    }

    void PrgRuntimeSession::Impl::retire_com_eventhandler_bindings_for_handles(const std::set<int> &handles)
    {
        std::vector<std::size_t> retired_ordinals;
        for (const ComEventHandlerBinding &binding : com_eventhandler_bindings)
        {
            if (handles.contains(binding.source_handle) || handles.contains(binding.handler_handle))
            {
                retired_ordinals.push_back(binding.ordinal);
            }
        }
        for (const std::size_t ordinal : retired_ordinals)
        {
            retire_com_eventhandler_binding(ordinal);
        }
    }

    bool PrgRuntimeSession::Impl::drain_external_event_tokens()
    {
        if (external_event_tokens == nullptr)
        {
            return false;
        }

        bool delivered = false;
        for (const std::string &token : external_event_tokens->drain())
        {
            constexpr std::string_view prefix = "com-eventhandler:";
            if (!token.starts_with(prefix))
            {
                continue;
            }
            const std::size_t separator = token.find(':', prefix.size());
            if (separator == std::string::npos || separator == prefix.size() ||
                separator + 1U >= token.size())
            {
                continue;
            }

            std::size_t ordinal = 0U;
            const char *ordinal_begin = token.data() + prefix.size();
            const char *ordinal_end = token.data() + separator;
            if (const auto [parsed_end, error] = std::from_chars(ordinal_begin, ordinal_end, ordinal);
                error != std::errc{} || parsed_end != ordinal_end || ordinal == 0U)
            {
                continue;
            }
            const std::string method = normalize_identifier(token.substr(separator + 1U));
            if (method.empty())
            {
                continue;
            }

            const auto binding = std::find_if(
                com_eventhandler_bindings.begin(),
                com_eventhandler_bindings.end(),
                [ordinal](const ComEventHandlerBinding &candidate)
                {
                    return candidate.ordinal == ordinal;
                });
            if (binding == com_eventhandler_bindings.end() ||
                !ole_objects.contains(binding->source_handle) ||
                !ole_objects.contains(binding->handler_handle) ||
                std::find(binding->required_handler_methods.begin(),
                          binding->required_handler_methods.end(),
                          method) == binding->required_handler_methods.end())
            {
                continue;
            }

            RuntimeOleObjectState &handler_object = ole_objects.at(binding->handler_handle);
            std::string method_program_path;
            std::string resolved_method;
            if (find_native_object_method(handler_object, method, method_program_path, resolved_method) == nullptr)
            {
                continue;
            }

            const NativeEventBinding dispatch_binding{
                .source_handle = binding->source_handle,
                .event_name = method,
                .target_is_routine = false,
                .target_program_path = {},
                .target_handle = binding->handler_handle,
                .delegate_name = method,
                .flags = 0,
                .ordinal = binding->ordinal};
            const std::string source_identity = binding->source_identity;
            const bool was_waiting_for_events = waiting_for_events;
            waiting_for_events = false;
            try
            {
                (void)invoke_native_event_delegate(
                    dispatch_binding,
                    {.source_handle = binding->source_handle, .event_name = method, .event_type = 0},
                    {},
                    {});
                events.push_back({.category = "prg.eventhandler.dispatch",
                                  .detail = source_identity + " -> " + method,
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                delivered = true;
            }
            catch (...)
            {
                retire_com_eventhandler_binding(ordinal);
                waiting_for_events = was_waiting_for_events && !stack.empty();
                throw;
            }
            waiting_for_events = was_waiting_for_events && !stack.empty();
        }
        return delivered;
    }

    PrgValue PrgRuntimeSession::Impl::eventhandler_com_event(const std::vector<PrgValue> &arguments)
    {
        // LLR-VFP-COM-002 / HZ-runtime-crash-01: absence of an explicit,
        // host-owned local admission capability is a side-effect-free failure.
        if (arguments.size() < 2U)
        {
            return make_boolean_value(false);
        }

        auto source_object = resolve_ole_object(arguments[0]);
        auto handler_object = resolve_ole_object(arguments[1]);
        if (!source_object.has_value() || !handler_object.has_value())
        {
            return make_boolean_value(false);
        }
        const bool unbind = arguments.size() >= 3U && value_as_bool(arguments[2]);
        if (unbind)
        {
            // Unbinding must remain available after a host revokes admission
            // (for example during fault rollback). The source/handler pair is
            // the runtime-owned binding key; no refreshed host metadata is
            // needed to remove stale state.
            std::vector<std::size_t> retired_ordinals;
            for (const ComEventHandlerBinding &binding : com_eventhandler_bindings)
            {
                if (binding.source_handle == (*source_object)->handle &&
                    binding.handler_handle == (*handler_object)->handle)
                {
                    retired_ordinals.push_back(binding.ordinal);
                }
            }
            for (const std::size_t ordinal : retired_ordinals)
            {
                retire_com_eventhandler_binding(ordinal);
            }
            return make_boolean_value(!retired_ordinals.empty());
        }
        if (!options.com_event_source_admission_callback)
        {
            return make_boolean_value(false);
        }
        if (normalize_identifier((*source_object)->last_action) == "createobjectex")
        {
            return make_boolean_value(false);
        }

        const auto admission = options.com_event_source_admission_callback(**source_object);
        if (!admission.has_value() || trim_copy(admission->source_identity).empty() ||
            trim_copy(admission->handler_interface_id).empty())
        {
            return make_boolean_value(false);
        }

        std::vector<std::string> required_methods;
        for (const std::string &method : admission->required_handler_methods)
        {
            const std::string normalized = normalize_identifier(trim_copy(method));
            if (normalized.empty() ||
                std::find(required_methods.begin(), required_methods.end(), normalized) != required_methods.end())
            {
                return make_boolean_value(false);
            }
            std::string program_path;
            std::string resolved_method;
            if (find_native_object_method(**handler_object, normalized, program_path, resolved_method) == nullptr)
            {
                return make_boolean_value(false);
            }
            required_methods.push_back(normalized);
        }
        if (required_methods.empty())
        {
            return make_boolean_value(false);
        }

        const auto matches = [&](const ComEventHandlerBinding &binding)
        {
            return binding.source_handle == (*source_object)->handle &&
                   binding.handler_handle == (*handler_object)->handle &&
                   binding.source_identity == admission->source_identity &&
                   binding.handler_interface_id == admission->handler_interface_id;
        };
        if (std::find_if(com_eventhandler_bindings.begin(), com_eventhandler_bindings.end(), matches) ==
            com_eventhandler_bindings.end())
        {
            ComEventHandlerBinding binding{
                .source_handle = (*source_object)->handle,
                .handler_handle = (*handler_object)->handle,
                .source_identity = admission->source_identity,
                .handler_interface_id = admission->handler_interface_id,
                .required_handler_methods = std::move(required_methods),
                .disconnect_subscription = {},
                .ordinal = next_native_event_binding_ordinal++};
            if (admission->subscribe_local_event_source)
            {
                const std::weak_ptr<detail::ExternalEventTokenQueue> queue = external_event_tokens;
                const std::size_t ordinal = binding.ordinal;
                const std::vector<std::string> allowed_methods = binding.required_handler_methods;
                try
                {
                    binding.disconnect_subscription = admission->subscribe_local_event_source(
                        [queue, ordinal, allowed_methods](std::string method) -> bool
                        {
                            const std::string normalized = normalize_identifier(trim_copy(method));
                            if (std::find(allowed_methods.begin(), allowed_methods.end(), normalized) ==
                                allowed_methods.end())
                            {
                                return false;
                            }
                            const std::shared_ptr<detail::ExternalEventTokenQueue> locked_queue = queue.lock();
                            return locked_queue != nullptr &&
                                locked_queue->try_push(
                                    "com-eventhandler:" + std::to_string(ordinal) + ":" + normalized);
                        });
                }
                catch (...)
                {
                    return make_boolean_value(false);
                }
                if (!binding.disconnect_subscription)
                {
                    return make_boolean_value(false);
                }
            }
            com_eventhandler_bindings.push_back(std::move(binding));
        }
        return make_boolean_value(true);
    }

    PrgValue PrgRuntimeSession::Impl::raise_native_event(
        const Frame &source_frame,
        const std::vector<PrgValue> &arguments,
        const std::vector<std::optional<std::string>> &argument_references)
    {
        if (arguments.size() < 2U)
        {
            return make_boolean_value(false);
        }

        auto source_object = resolve_ole_object(arguments[0]);
        if (!source_object.has_value() || (*source_object)->source.empty())
        {
            return make_boolean_value(false);
        }

        const std::string event_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (event_name.empty())
        {
            return make_boolean_value(false);
        }

        std::vector<PrgValue> event_arguments;
        std::vector<std::optional<std::string>> event_argument_references;
        if (arguments.size() > 2U)
        {
            event_arguments.assign(arguments.begin() + 2U, arguments.end());
        }
        if (argument_references.size() > 2U)
        {
            event_argument_references.assign(argument_references.begin() + 2U, argument_references.end());
        }

        std::vector<NativeEventBinding> bindings;
        bindings.reserve(native_event_bindings.size());
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            if (binding.source_handle == (*source_object)->handle &&
                binding.event_name == event_name)
            {
                bindings.push_back(binding);
            }
        }

        const std::string active_event_key =
            std::to_string((*source_object)->handle) + ":" + event_name;
        if (active_native_event_keys.find(active_event_key) != active_native_event_keys.end())
        {
            return make_boolean_value(true);
        }

        ActiveNativeEventKeyGuard active_event_guard(active_native_event_keys, active_event_key);
        if (!active_event_guard.engaged)
        {
            return make_boolean_value(true);
        }
        const auto invoke_delegates_for_phase = [&](bool after_source_method)
        {
            for (const NativeEventBinding &binding : bindings)
            {
                const bool binding_after_source_method = (binding.flags & 1) == 0;
                if (binding_after_source_method == after_source_method)
                {
                    (void)invoke_native_event_delegate(
                        binding,
                        {.source_handle = (*source_object)->handle,
                         .event_name = event_name,
                         .event_type = 1},
                        event_arguments,
                        event_argument_references);
                }
            }
        };

        invoke_delegates_for_phase(false);
        (void)invoke_native_object_method_body_if_present(
            **source_object,
            event_name,
            source_frame,
            event_arguments,
            event_argument_references);
        invoke_delegates_for_phase(true);

        events.push_back({.category = "prg.event.raise",
                          .detail = (*source_object)->prog_id + "." + event_name,
                          .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        return make_boolean_value(true);
    }

    PrgValue PrgRuntimeSession::Impl::unbind_native_events(
        const std::vector<PrgValue> &arguments)
    {
        if (arguments.empty())
        {
            return make_number_value(0.0);
        }

        int object_handle_probe = 0;
        std::string object_prog_id_probe;
        if (!parse_object_handle_reference(arguments[0], object_handle_probe, object_prog_id_probe))
        {
            const int window_handle = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const bool has_message_filter = arguments.size() >= 2U;
            const int message = has_message_filter
                                    ? static_cast<int>(std::llround(value_as_number(arguments[1])))
                                    : 0;
            const std::size_t before_count = window_message_bindings.size();
            const auto erase_from = std::remove_if(
                window_message_bindings.begin(),
                window_message_bindings.end(),
                [&](const WindowMessageBinding &binding)
                {
                    return binding.window_handle == window_handle &&
                           (!has_message_filter || binding.message == message);
                });
            window_message_bindings.erase(erase_from, window_message_bindings.end());

            const std::size_t removed_count = before_count - window_message_bindings.size();
            if (removed_count != 0U)
            {
                events.push_back({.category = "prg.event.unbind",
                                  .detail = std::to_string(window_handle) + ":" +
                                                (has_message_filter ? std::to_string(message) : std::string("*")),
                                  .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
            }
            return make_number_value(static_cast<double>(removed_count));
        }

        const std::size_t before_count = native_event_bindings.size();
        const auto erase_from = std::remove_if(
            native_event_bindings.begin(),
            native_event_bindings.end(),
            [&](const NativeEventBinding &binding)
            {
                if (arguments.size() == 1U)
                {
                    int object_handle = 0;
                    std::string object_prog_id;
                    return parse_object_handle_reference(arguments[0], object_handle, object_prog_id) &&
                           (binding.source_handle == object_handle ||
                            (!binding.target_is_routine && binding.target_handle == object_handle));
                }

                int source_handle = 0;
                std::string source_prog_id;
                if (!parse_object_handle_reference(arguments[0], source_handle, source_prog_id) ||
                    binding.source_handle != source_handle)
                {
                    return false;
                }

                const std::string event_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
                if (event_name.empty() || binding.event_name != event_name)
                {
                    return false;
                }

                if (arguments.size() == 3U && binding.target_is_routine)
                {
                    return normalize_identifier(binding.delegate_name) ==
                           normalize_identifier(trim_copy(value_as_string(arguments[2])));
                }

                if (arguments.size() >= 4U && !binding.target_is_routine)
                {
                    int target_handle = 0;
                    std::string target_prog_id;
                    return parse_object_handle_reference(arguments[2], target_handle, target_prog_id) &&
                           binding.target_handle == target_handle &&
                           normalize_identifier(binding.delegate_name) ==
                               normalize_identifier(trim_copy(value_as_string(arguments[3])));
                }

                return arguments.size() == 2U;
            });
        native_event_bindings.erase(erase_from, native_event_bindings.end());

        const std::size_t removed_count = before_count - native_event_bindings.size();
        if (removed_count != 0U)
        {
            events.push_back({.category = "prg.event.unbind",
                              .detail = std::to_string(removed_count),
                              .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
        }
        return make_number_value(static_cast<double>(removed_count));
    }

    PrgValue PrgRuntimeSession::Impl::inspect_native_events(
        const std::vector<PrgValue> &arguments,
        const std::vector<std::string> &raw_arguments)
    {
        if (raw_arguments.empty() || arguments.size() < 2U)
        {
            return make_number_value(0.0);
        }

        const auto resolve_array_argument_name = [&](std::size_t index)
        {
            std::string candidate = index < raw_arguments.size() ? trim_copy(raw_arguments[index]) : std::string{};
            if (!is_bare_identifier_text(candidate) &&
                index < arguments.size() &&
                arguments[index].kind == PrgValueKind::string)
            {
                const std::string evaluated_name = trim_copy(value_as_string(arguments[index]));
                if (is_bare_identifier_text(evaluated_name))
                {
                    candidate = evaluated_name;
                }
            }
            if (is_bare_identifier_text(candidate) && !stack.empty())
            {
                Frame &frame = stack.back();
                constexpr std::size_t max_array_name_depth = 16U;
                std::vector<std::string> visited_identifiers;
                visited_identifiers.reserve(8U);
                for (std::size_t depth = 0U; depth < max_array_name_depth; ++depth)
                {
                    const std::string normalized = normalize_memory_variable_identifier(candidate);
                    if (std::find(visited_identifiers.begin(), visited_identifiers.end(), normalized) != visited_identifiers.end())
                    {
                        break;
                    }
                    visited_identifiers.push_back(normalized);

                    const PrgValue indirect_value = lookup_variable(frame, candidate);
                    if (indirect_value.kind != PrgValueKind::string)
                    {
                        break;
                    }

                    const std::string next = trim_copy(value_as_string(indirect_value));
                    if (next.empty() || next == candidate || !is_bare_identifier_text(next))
                    {
                        break;
                    }

                    candidate = next;
                }
            }
            return candidate;
        };

        const std::string array_name = resolve_array_argument_name(0U);
        if (array_name.empty())
        {
            return make_number_value(0.0);
        }

        const auto second_argument_is_zero_probe = [&]() -> bool
        {
            if (arguments.size() < 2U)
            {
                return false;
            }
            int object_handle = 0;
            std::string object_prog_id;
            if (parse_object_handle_reference(arguments[1], object_handle, object_prog_id))
            {
                return false;
            }

            switch (arguments[1].kind)
            {
            case PrgValueKind::number:
                return std::abs(arguments[1].number_value) < 0.000001;
            case PrgValueKind::currency:
                return arguments[1].currency_value == 0;
            case PrgValueKind::int64:
                return arguments[1].int64_value == 0;
            case PrgValueKind::uint64:
                return arguments[1].uint64_value == 0U;
            case PrgValueKind::boolean:
                return !arguments[1].boolean_value;
            case PrgValueKind::string:
                return trim_copy(arguments[1].string_value) == "0";
            case PrgValueKind::empty:
                return false;
            }
            return false;
        };

        if (second_argument_is_zero_probe())
        {
            if (active_native_event_contexts.empty())
            {
                if (active_window_message_contexts.empty())
                {
                    return make_number_value(0.0);
                }

                const CurrentWindowMessageContext &message_context = active_window_message_contexts.back();
                assign_array(
                    array_name,
                    {make_int64_value(static_cast<std::int64_t>(message_context.window_handle)),
                     make_string_value(std::to_string(message_context.message)),
                     make_number_value(0.0)},
                    1U);
                return make_number_value(3.0);
            }

            const CurrentNativeEventContext &event_context = active_native_event_contexts.back();
            const auto source_found = ole_objects.find(event_context.source_handle);
            if (source_found == ole_objects.end())
            {
                return make_number_value(0.0);
            }

            assign_array(
                array_name,
                {make_string_value("object:" + source_found->second.prog_id + "#" + std::to_string(source_found->second.handle)),
                 make_string_value(event_context.event_name),
                 make_number_value(static_cast<double>(event_context.event_type))},
                1U);
            return make_number_value(3.0);
        }

        const auto second_argument_is_one_probe = [&]() -> bool
        {
            if (arguments.size() < 2U)
            {
                return false;
            }
            int object_handle = 0;
            std::string object_prog_id;
            if (parse_object_handle_reference(arguments[1], object_handle, object_prog_id))
            {
                return false;
            }

            switch (arguments[1].kind)
            {
            case PrgValueKind::number:
                return std::abs(arguments[1].number_value - 1.0) < 0.000001;
            case PrgValueKind::currency:
                return arguments[1].currency_value == 10000;
            case PrgValueKind::int64:
                return arguments[1].int64_value == 1;
            case PrgValueKind::uint64:
                return arguments[1].uint64_value == 1U;
            case PrgValueKind::boolean:
                return arguments[1].boolean_value;
            case PrgValueKind::string:
                return trim_copy(arguments[1].string_value) == "1";
            case PrgValueKind::empty:
                return false;
            }
            return false;
        };

        if (second_argument_is_one_probe())
        {
            std::vector<PrgValue> values;
            values.reserve(window_message_bindings.size() * 4U);
            for (const WindowMessageBinding &binding : window_message_bindings)
            {
                const auto target = ole_objects.find(binding.target_handle);
                values.push_back(make_int64_value(static_cast<std::int64_t>(binding.window_handle)));
                values.push_back(make_int64_value(static_cast<std::int64_t>(binding.message)));
                values.push_back(
                    target == ole_objects.end()
                        ? make_empty_value()
                        : make_string_value("object:" + target->second.prog_id + "#" + std::to_string(target->second.handle)));
                values.push_back(make_string_value(binding.delegate_name));
            }

            if (values.empty())
            {
                return make_number_value(0.0);
            }

            assign_array(array_name, std::move(values), 4U);
            return make_number_value(static_cast<double>(array_length(array_name, 1)));
        }

        int object_handle = 0;
        std::string object_prog_id;
        if (!parse_object_handle_reference(arguments[1], object_handle, object_prog_id))
        {
            return make_number_value(0.0);
        }

        std::vector<PrgValue> values;
        values.reserve(native_event_bindings.size() * 5U);
        for (const NativeEventBinding &binding : native_event_bindings)
        {
            const bool object_is_source = binding.source_handle == object_handle;
            const bool object_is_handler = !binding.target_is_routine && binding.target_handle == object_handle;
            if (!object_is_source && !object_is_handler)
            {
                continue;
            }

            values.push_back(make_boolean_value(object_is_handler));
            if (object_is_handler)
            {
                const auto source = ole_objects.find(binding.source_handle);
                values.push_back(
                    source == ole_objects.end()
                        ? make_empty_value()
                        : make_string_value("object:" + source->second.prog_id + "#" + std::to_string(source->second.handle)));
            }
            else if (binding.target_is_routine)
            {
                values.push_back(make_empty_value());
            }
            else
            {
                const auto target = ole_objects.find(binding.target_handle);
                values.push_back(
                    target == ole_objects.end()
                        ? make_empty_value()
                        : make_string_value("object:" + target->second.prog_id + "#" + std::to_string(target->second.handle)));
            }
            values.push_back(make_string_value(binding.event_name));
            values.push_back(make_string_value(binding.delegate_name));
            values.push_back(make_number_value(static_cast<double>(binding.flags)));
        }

        if (values.empty())
        {
            return make_number_value(0.0);
        }

        assign_array(array_name, std::move(values), 5U);
        return make_number_value(static_cast<double>(array_length(array_name, 1)));
    }
