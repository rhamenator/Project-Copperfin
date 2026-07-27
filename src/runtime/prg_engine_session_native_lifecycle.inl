// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

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

            const bool is_report_listener_object = std::any_of(
                object_state.class_hierarchy.begin(),
                object_state.class_hierarchy.end(),
                [](const std::string &class_name)
                {
                    return normalize_identifier(trim_copy(class_name)) == "reportlistener";
                }) ||
                normalize_identifier(trim_copy(object_state.base_class_name)) == "reportlistener";
            if (is_report_listener_object && !object_state.properties.contains("haderror"))
            {
                object_state.properties["haderror"] = make_boolean_value(false);
            }

            assign_native_window_metadata(object_state);

            std::map<std::string, std::string> effective_methods;
            for (const NativeClassLookup &lineage_class : class_lineage)
            {
                const std::string owner_class_name = normalize_identifier(
                    lineage_class.class_definition->name.empty()
                        ? prog_id
                        : lineage_class.class_definition->name);
                for (const auto &[member_name, visibility] : lineage_class.class_definition->member_visibility)
                {
                    object_state.member_visibility[member_name] = visibility;
                    object_state.member_visibility_owner[member_name] = owner_class_name;
                }
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
            auto &saved_lineage = native_object_class_lineage_by_handle[handle];
            saved_lineage.reserve(class_lineage.size());
            for (const NativeClassLookup &lineage_class : class_lineage)
            {
                saved_lineage.push_back(
                    {.program_path = lineage_class.program->path,
                     .class_name = lineage_class.class_definition->name});
            }
            const std::size_t construction_event_start = events.size();
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
                    if (property_statement.kind == StatementKind::dimension_command)
                    {
                        for (const std::string &declaration : property_statement.names)
                        {
                            std::string property_name;
                            std::size_t rows = 0U;
                            std::size_t columns = 1U;
                            if (!parse_array_reference(declaration, frame, property_name, rows, columns) ||
                                !is_bare_identifier_text(property_name))
                            {
                                continue;
                            }
                            property_name = normalize_identifier(property_name);
                            RuntimeArray array;
                            array.rows = rows;
                            array.columns = columns;
                            array.values.resize(rows * columns);
                            native_object_arrays[runtime_object->handle][property_name] = std::move(array);
                            runtime_object->properties[property_name] = make_empty_value();
                        }
                        continue;
                    }
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
            normalize_native_commandbutton_default_cancel_invariant(*runtime_object);
            normalize_native_commandbutton_style_invariant(*runtime_object);
            normalize_native_commandbutton_picture_layout_invariant(*runtime_object);
            normalize_native_grid_rowheight_invariant(*runtime_object);
            normalize_native_grid_headerheight_invariant(*runtime_object);
            normalize_native_grid_allowheadersizing_invariant(*runtime_object);
            normalize_native_grid_allowrowsizing_invariant(*runtime_object);
            normalize_native_grid_allowautocolumnfit_invariant(*runtime_object);
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

            const std::string normalized_base_class_name =
                normalize_identifier(runtime_object->base_class_name);
            if (normalized_base_class_name == "form" || normalized_base_class_name == "formset")
            {
                std::string load_program_path;
                std::string load_method_name;
                if (const Routine *load_method = find_native_object_method(
                        *runtime_object,
                        "load",
                        load_program_path,
                        load_method_name);
                    load_method != nullptr)
                {
                    if (!can_push_frame())
                    {
                        throw std::runtime_error(call_depth_limit_message());
                    }

                    events.push_back({.category = "prg.object.load",
                                      .detail = load_method_name,
                                      .location = current_statement() == nullptr ? SourceLocation{} : current_statement()->location});
                    const std::size_t return_depth = stack.size();
                    push_method_frame(load_program_path,
                                      load_method_name,
                                      *load_method,
                                      runtime_object_reference,
                                      load_method_name.substr(0U, load_method_name.rfind('.')),
                                      "load",
                                      parent_reference,
                                      native_object_owner_form_reference(*runtime_object),
                                      native_object_owner_formset_reference(*runtime_object),
                                      {},
                                      {});
                    const PrgValue load_result = run_expression_invoked_routine_until_return(return_depth);
                    if (load_result.kind == PrgValueKind::boolean && !load_result.boolean_value)
                    {
                        discard_native_object_tree_without_destroy(*runtime_object);
                        return nullptr;
                    }
                }
            }
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
                    normalize_native_commandbutton_default_cancel_invariant(*child_object);
                    normalize_native_commandbutton_style_invariant(*child_object);
                    normalize_native_commandbutton_picture_layout_invariant(*child_object);
                    normalize_native_grid_rowheight_invariant(*child_object);
                    normalize_native_grid_headerheight_invariant(*child_object);
                    normalize_native_grid_allowheadersizing_invariant(*child_object);
                    normalize_native_grid_allowrowsizing_invariant(*child_object);
                    normalize_native_grid_allowautocolumnfit_invariant(*child_object);
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

                const std::size_t init_event_start = events.size();
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
                const PrgValue init_result = run_expression_invoked_routine_until_return(return_depth);
                const bool init_returned_explicitly = consume_last_popped_frame_returned();
                if (init_returned_explicitly &&
                    init_result.kind == PrgValueKind::boolean && !init_result.boolean_value)
                {
                    events.erase(
                        std::remove_if(
                            events.begin() + static_cast<std::ptrdiff_t>(construction_event_start),
                            events.begin() + static_cast<std::ptrdiff_t>(init_event_start),
                            [](const RuntimeEvent &event)
                            {
                                return event.category == "prg.object.addobject";
                            }),
                        events.begin() + static_cast<std::ptrdiff_t>(init_event_start));
                    discard_native_object_tree_without_destroy(*runtime_object);
                    return nullptr;
                }
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
            if (frame.expression_routine_return_pending &&
                frame.routine != nullptr &&
                frame.pc > 0U &&
                frame.pc <= frame.routine->statements.size())
            {
                return &frame.routine->statements[frame.pc - 1U];
            }
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
            object_state.properties["messagetext"] = make_string_value(last_error_message);
            object_state.properties["errorno"] = make_number_value(static_cast<double>(last_error_code));
            object_state.properties["helpcontext"] = make_number_value(0.0);
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
