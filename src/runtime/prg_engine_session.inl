// prg_engine_session.inl
// PrgRuntimeSession::Impl method group. Included inside Impl struct in prg_engine.cpp.
// This file must not be compiled separately.

        Program &load_program(const std::string &path)
        {
            const std::string normalized = normalize_path(path);
            const auto existing = programs.find(normalized);
            if (existing != programs.end())
            {
                return existing->second;
            }
            auto [inserted, _] = programs.emplace(normalized, parse_program(normalized));
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
            stack.push_back(std::move(frame));
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

        const PrgClassDefinition *find_native_same_prg_class(
            const Program &program,
            const std::string &class_name) const
        {
            const auto found = program.classes.find(normalize_identifier(class_name));
            return found == program.classes.end() ? nullptr : &found->second;
        }

        const PrgClassDefinition *find_native_same_prg_base_class(
            const Program &program,
            const PrgClassDefinition &class_definition) const
        {
            const std::string base_class_name =
                native_same_prg_base_class_name(class_definition.base_class_name);
            return base_class_name.empty()
                ? nullptr
                : find_native_same_prg_class(program, base_class_name);
        }

        std::vector<const PrgClassDefinition *> collect_native_same_prg_class_lineage(
            const Program &program,
            const std::string &class_name) const
        {
            std::vector<const PrgClassDefinition *> reverse_lineage;
            std::set<std::string> visited;
            const PrgClassDefinition *current = find_native_same_prg_class(program, class_name);
            while (current != nullptr)
            {
                const std::string normalized_name = normalize_identifier(current->name);
                if (!normalized_name.empty() && !visited.insert(normalized_name).second)
                {
                    break;
                }

                reverse_lineage.push_back(current);
                current = find_native_same_prg_base_class(program, *current);
            }

            return std::vector<const PrgClassDefinition *>(
                reverse_lineage.rbegin(),
                reverse_lineage.rend());
        }

        const Routine *find_native_same_prg_method(
            const Program &program,
            const std::string &class_name,
            const std::string &member_name,
            bool include_starting_class,
            std::string &qualified_routine_name,
            std::string *defining_class_name = nullptr) const
        {
            const std::string normalized_member_name = normalize_identifier(member_name);
            const PrgClassDefinition *current_class =
                find_native_same_prg_class(program, class_name);
            if (current_class == nullptr)
            {
                return nullptr;
            }
            if (!include_starting_class)
            {
                current_class = find_native_same_prg_base_class(program, *current_class);
            }

            std::set<std::string> visited;
            while (current_class != nullptr)
            {
                const std::string normalized_class_name = normalize_identifier(current_class->name);
                if (!normalized_class_name.empty() &&
                    !visited.insert(normalized_class_name).second)
                {
                    break;
                }

                const auto method_found = current_class->methods.find(normalized_member_name);
                if (method_found != current_class->methods.end())
                {
                    const std::string resolved_class_name =
                        current_class->name.empty() ? class_name : current_class->name;
                    if (defining_class_name != nullptr)
                    {
                        *defining_class_name = resolved_class_name;
                    }
                    qualified_routine_name = resolved_class_name + "." + method_found->second.name;
                    return &method_found->second;
                }

                current_class = find_native_same_prg_base_class(program, *current_class);
            }

            return nullptr;
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
            return find_native_same_prg_method(
                program,
                runtime_object.prog_id,
                member_name,
                true,
                qualified_routine_name);
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
            auto current_reference = native_object_parent_reference(runtime_object);
            if (!current_reference.has_value())
            {
                return std::nullopt;
            }

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
                    return current_reference;
                }

                const auto parent_reference = native_object_parent_reference(found->second);
                if (!parent_reference.has_value())
                {
                    return current_reference;
                }
                current_reference = parent_reference;
            }

            return std::nullopt;
        }

        struct ResolvedRuntimeObjectMemberPath
        {
            RuntimeOleObjectState *runtime_object = nullptr;
            std::string remaining_member_path;
        };

        ResolvedRuntimeObjectMemberPath resolve_runtime_object_member_path(
            const Frame &frame,
            const std::string &base_name,
            const std::string &member_path)
        {
            const PrgValue object_value = lookup_variable(frame, base_name);
            auto object = resolve_ole_object(object_value);
            if (!object.has_value())
            {
                return {};
            }

            RuntimeOleObjectState *current_object = *object;
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
            const auto current_matches_parent_of_child = [&](const RuntimeOleObjectState &child_object) -> bool
            {
                const auto parent_reference = native_object_parent_reference(child_object);
                if (!parent_reference.has_value())
                {
                    return false;
                }

                int parent_handle = 0;
                std::string parent_prog_id;
                return parse_object_handle_reference(*parent_reference, parent_handle, parent_prog_id) &&
                       parent_handle == current_object->handle;
            };

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

                RuntimeOleObjectState *next_object = *nested_object;
                const bool parent_step = property_name == "parent";
                if (!parent_step && !current_matches_parent_of_child(*next_object))
                {
                    break;
                }

                current_object = next_object;
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
                return nullptr;
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
            std::vector<const PrgClassDefinition *> class_lineage =
                collect_native_same_prg_class_lineage(
                    program,
                    class_definition.name.empty() ? prog_id : class_definition.name);
            if (class_lineage.empty())
            {
                class_lineage.push_back(&class_definition);
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

            std::map<std::string, std::string> effective_methods;
            for (const PrgClassDefinition *lineage_class : class_lineage)
            {
                for (const auto &[normalized_method_name, method] : lineage_class->methods)
                {
                    effective_methods[normalized_method_name] = method.name;
                }
            }
            object_state.methods.reserve(effective_methods.size());
            for (const auto &[_, method_name] : effective_methods)
            {
                object_state.methods.push_back(method_name);
            }

            auto [object_it, _] = ole_objects.emplace(handle, std::move(object_state));
            RuntimeOleObjectState *runtime_object = &object_it->second;
            for (const PrgClassDefinition *lineage_class : class_lineage)
            {
                for (const Statement &property_statement : lineage_class->property_statements)
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
                }
            }

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
                const PrgValue this_reference =
                    make_string_value("object:" + runtime_object->prog_id + "#" + std::to_string(runtime_object->handle));
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
                                  this_reference,
                                  init_method_name.substr(0U, init_method_name.rfind('.')),
                                  "init",
                                  parent_reference,
                                  native_object_owner_form_reference(*runtime_object),
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
            last_fault_location = statement.location;
            last_fault_statement = statement.text;
            last_error_code = classify_runtime_error_code(last_error_message);
            last_error_work_area = current_selected_work_area();
            last_error_procedure = frame.routine_name;
            if (last_error_code != 1526 && last_error_code != 1429)
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

        void cleanup_runtime_resources_for_shutdown()
        {
            // Release open work areas/cursors across all data sessions.
            for (auto &[_, session] : data_sessions)
            {
                session.cursors.clear();
                session.aliases.clear();
                session.table_locks.clear();
                session.record_locks.clear();
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

#if defined(_WIN32)
            // Release any DLL handles loaded through DECLARE ... IN.
            std::set<HMODULE> released_modules;
            for (auto &[_, declfn] : declared_dll_functions)
            {
                if (declfn.hmodule != nullptr && !released_modules.contains(declfn.hmodule))
                {
                    FreeLibrary(declfn.hmodule);
                    released_modules.insert(declfn.hmodule);
                }
                declfn.hmodule = nullptr;
                declfn.proc_address = nullptr;
            }
#endif
            declared_dll_functions.clear();
            loaded_libraries.clear();
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

            const auto [scope_name, _scope_tail] = split_first_word(scope);
            const std::string close_scope = normalize_identifier(scope_name.empty() ? scope : scope_name);
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
            for (const auto &[_, connection] : current_sql_connections())
            {
                state.sql_connections.push_back(connection);
            }
            for (const auto &[_, object] : ole_objects)
            {
                state.ole_objects.push_back(object);
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
