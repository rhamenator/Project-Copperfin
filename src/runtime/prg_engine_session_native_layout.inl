// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.
// Included inside PrgRuntimeSession::Impl by prg_engine_session.inl.

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
            (void)sync_native_owned_children_collection(parent_found->second);
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
                native_object_class_lineage_by_handle.erase(handle);
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
