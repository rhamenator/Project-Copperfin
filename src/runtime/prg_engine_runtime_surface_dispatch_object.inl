// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

    if (function == "compobj" && arguments.size() >= 2U) {
        int handle_left = 0;
        int handle_right = 0;
        std::string prog_id_left;
        std::string prog_id_right;
        if (!parse_object_handle_reference(arguments[0], handle_left, prog_id_left) ||
            !parse_object_handle_reference(arguments[1], handle_right, prog_id_right)) {
            return make_boolean_value(false);
        }
        if (handle_left != handle_right) {
            return make_boolean_value(false);
        }
        // VFP: same handle means same object; use callback for pointer-level confirm when available
        if (resolve_object_callback) {
            RuntimeOleObjectState* left = resolve_object_callback(arguments[0]);
            RuntimeOleObjectState* right = resolve_object_callback(arguments[1]);
            return make_boolean_value(left != nullptr && right != nullptr && left == right);
        }
        return make_boolean_value(true);
    }

    if (function == "amembers" && arguments.size() >= 2U && !raw_arguments.empty()) {
        const std::string array_name = trim_copy(raw_arguments[0]);
        try {
            const int flags = safe_int_argument(2U, 0);
            std::optional<NativeMemberVisibility> visibility_filter;
            if (arguments.size() >= 4U) {
                const std::string raw_visibility_filter = uppercase_copy(trim_copy(value_as_string(arguments[3])));
                if (raw_visibility_filter == "P") {
                    visibility_filter = NativeMemberVisibility::protected_member;
                } else if (raw_visibility_filter == "H") {
                    visibility_filter = NativeMemberVisibility::hidden_member;
                } else if (raw_visibility_filter == "G") {
                    visibility_filter = NativeMemberVisibility::public_member;
                }
            }
            if (!resolve_object_callback || !assign_array_callback) {
                record_runtime_warning(runtime_text(
                    "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
                    {
                        {"capability", "object/array"},
                        {"function", "AMEMBERS()"}
                    }));
                if (assign_array_callback && !array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[1]);
            if (runtime_object == nullptr || array_name.empty()) {
                if (!array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            const std::optional<NativeMemberVisibility> native_visibility_filter =
                trim_copy(runtime_object->source).empty() ? std::nullopt : visibility_filter;
            std::vector<PrgValue> member_names;
            std::vector<std::string> member_tokens =
                collect_object_member_names(*runtime_object, flags, native_visibility_filter);
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr) {
                member_tokens = merge_member_tokens(
                    member_tokens,
                    collect_object_member_names(*object_surface, flags));
            }
            member_names.reserve(member_tokens.size());
            for (const std::string& member_name : member_tokens) {
                member_names.push_back(make_string_value(member_name));
            }
            assign_array_callback(array_name, member_names);
            return make_number_value(static_cast<double>(member_names.size()));
        } catch (...) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.AmembersFallback",
                {{"function", "AMEMBERS()"}}));
            if (assign_array_callback && !array_name.empty()) {
                assign_array_callback(array_name, {});
            }
            return make_number_value(0.0);
        }
    }

    if (function == "aclass" && arguments.size() >= 2U && !raw_arguments.empty()) {
        const std::string array_name = trim_copy(raw_arguments[0]);
        try {
            if (!resolve_object_callback || !assign_array_callback) {
                record_runtime_warning(runtime_text(
                    "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
                    {
                        {"capability", "object/array"},
                        {"function", "ACLASS()"}
                    }));
                if (assign_array_callback && !array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[1]);
            if (runtime_object == nullptr || array_name.empty()) {
                if (!array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            std::vector<PrgValue> class_chain;
            if (!runtime_object->class_hierarchy.empty()) {
                class_chain.reserve(runtime_object->class_hierarchy.size());
                for (const std::string& class_name : runtime_object->class_hierarchy) {
                    class_chain.push_back(make_string_value(class_name));
                }
            } else {
                const std::string class_name = class_token_from_prog_id(runtime_object->prog_id);
                class_chain.push_back(make_string_value(class_name));
                class_chain.push_back(make_string_value("OBJECT"));
            }
            assign_array_callback(array_name, class_chain);
            return make_number_value(static_cast<double>(class_chain.size()));
        } catch (...) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.AclassFallback",
                {{"function", "ACLASS()"}}));
            if (assign_array_callback && !array_name.empty()) {
                assign_array_callback(array_name, {});
            }
            return make_number_value(0.0);
        }
    }

    if (function == "pemstatus" && arguments.size() >= 3U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubObjectResolution",
                {{"function", "PEMSTATUS()"}}));
            return make_boolean_value(false);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        const int attribute = safe_int_argument(2U, 0);
        if (runtime_object == nullptr || member_name.empty()) {
            return make_boolean_value(false);
        }
        if (attribute == 1) {
            bool exists = reflectable_member_exists_locally(*runtime_object, member_name);
            if (!exists) {
                if (RuntimeOleObjectState* object_surface =
                        resolve_direct_olecontrol_reflection_surface(*runtime_object);
                    object_surface != nullptr) {
                    exists = reflectable_member_exists_locally(*object_surface, member_name);
                }
            }
            return make_boolean_value(exists);
        }
        if (attribute == 3) {
            const auto visibility = runtime_object->member_visibility.find(member_name);
            return make_boolean_value(
                visibility != runtime_object->member_visibility.end() &&
                visibility->second != NativeMemberVisibility::public_member);
        }
        if (attribute == 5) {
            bool readonly = reflectable_member_readonly_locally(*runtime_object, member_name);
            if (!readonly && !reflectable_member_exists_locally(*runtime_object, member_name)) {
                if (RuntimeOleObjectState* object_surface =
                        resolve_direct_olecontrol_reflection_surface(*runtime_object);
                    object_surface != nullptr) {
                    readonly = reflectable_member_readonly_locally(*object_surface, member_name);
                }
            }
            return make_boolean_value(readonly);
        }
        return make_boolean_value(false);
    }

    if ((function == "addproperty" || function == "addprop") && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", "ADDPROPERTY()"}}));
            return make_boolean_value(true);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        if (runtime_object == nullptr) {
            return make_boolean_value(false);
        }
        const std::string property_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (property_name.empty()) {
            return make_boolean_value(false);
        }
        if (parse_native_list_control_list_member_cell(*runtime_object, property_name).has_value() ||
            parse_native_list_control_listitem_member_cell(*runtime_object, property_name).has_value() ||
            parse_native_list_control_itemdata_member_slot(*runtime_object, property_name).has_value() ||
            parse_native_list_control_indextoitemid_member_slot(*runtime_object, property_name).has_value() ||
            parse_native_list_control_itemidtoindex_member_item_id(*runtime_object, property_name).has_value()) {
            return make_boolean_value(false);
        }
        if (is_native_identity_member_name(*runtime_object, property_name) ||
            is_native_controlcount_member_name(*runtime_object, property_name) ||
            is_native_pagecount_member_name(*runtime_object, property_name) ||
            is_native_activepage_member_name(*runtime_object, property_name) ||
            is_native_child_collection_member_name(*runtime_object, property_name) ||
            is_native_name_member_name(*runtime_object, property_name) ||
            is_native_form_alwaysontop_member_name(*runtime_object, property_name) ||
            is_native_form_keypreview_member_name(*runtime_object, property_name) ||
            is_native_form_showwindow_member_name(*runtime_object, property_name) ||
            is_native_form_windowtype_member_name(*runtime_object, property_name) ||
            is_native_form_windowstate_member_name(*runtime_object, property_name) ||
            is_native_form_scalemode_member_name(*runtime_object, property_name) ||
            is_native_form_drawstyle_member_name(*runtime_object, property_name) ||
            is_native_visual_fillstyle_member_name(*runtime_object, property_name) ||
            is_native_visual_fillcolor_member_name(*runtime_object, property_name) ||
            is_native_visual_borderwidth_member_name(*runtime_object, property_name) ||
            is_native_visual_bordercolor_member_name(*runtime_object, property_name) ||
            is_native_visual_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_form_drawwidth_member_name(*runtime_object, property_name) ||
            is_native_form_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_form_titlebar_member_name(*runtime_object, property_name) ||
            is_native_form_desktop_member_name(*runtime_object, property_name) ||
            is_native_form_scrollbars_member_name(*runtime_object, property_name) ||
            is_native_form_lockscreen_member_name(*runtime_object, property_name) ||
            is_native_form_controlbox_member_name(*runtime_object, property_name) ||
            is_native_form_closable_member_name(*runtime_object, property_name) ||
            is_native_form_minbutton_member_name(*runtime_object, property_name) ||
            is_native_form_maxbutton_member_name(*runtime_object, property_name) ||
            is_native_form_autocenter_member_name(*runtime_object, property_name) ||
            is_native_visual_enabled_member_name(*runtime_object, property_name) ||
            is_native_visual_fontname_member_name(*runtime_object, property_name) ||
            is_native_visual_fontcharset_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontname_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontsize_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontshadow_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontoutline_member_name(*runtime_object, property_name) ||
            is_native_visual_fontsize_member_name(*runtime_object, property_name) ||
            is_native_visual_fontbold_member_name(*runtime_object, property_name) ||
            is_native_visual_fontitalic_member_name(*runtime_object, property_name) ||
            is_native_visual_fontunderline_member_name(*runtime_object, property_name) ||
            is_native_visual_fontstrikethru_member_name(*runtime_object, property_name) ||
            is_native_visual_fontoutline_member_name(*runtime_object, property_name) ||
            is_native_visual_fontshadow_member_name(*runtime_object, property_name) ||
            is_native_visual_visible_member_name(*runtime_object, property_name) ||
            is_native_controltiptext_member_name(*runtime_object, property_name) ||
            is_native_visual_helpcontextid_member_name(*runtime_object, property_name) ||
            is_native_visual_whatsthishelpid_member_name(*runtime_object, property_name) ||
            is_native_visual_tag_member_name(*runtime_object, property_name) ||
            is_native_visual_caption_member_name(*runtime_object, property_name) ||
            is_native_commandbutton_default_cancel_member_name(*runtime_object, property_name) ||
            is_native_commandbutton_style_member_name(*runtime_object, property_name) ||
            is_native_commandbutton_picture_layout_member_name(*runtime_object, property_name) ||
            is_native_visual_picture_member_name(*runtime_object, property_name) ||
            is_native_visual_dragmode_member_name(*runtime_object, property_name) ||
            is_native_visual_anchor_member_name(*runtime_object, property_name) ||
            is_native_visual_dragicon_member_name(*runtime_object, property_name) ||
            is_native_visual_downpicture_member_name(*runtime_object, property_name) ||
            is_native_visual_disabledpicture_member_name(*runtime_object, property_name) ||
            is_native_visual_autosize_member_name(*runtime_object, property_name) ||
            is_native_visual_drawmode_member_name(*runtime_object, property_name) ||
            is_native_visual_backstyle_member_name(*runtime_object, property_name) ||
            is_native_visual_specialeffect_member_name(*runtime_object, property_name) ||
            is_native_visual_alignment_member_name(*runtime_object, property_name) ||
            is_native_editbox_scrollbars_member_name(*runtime_object, property_name) ||
            is_native_textbox_inputmask_member_name(*runtime_object, property_name) ||
            is_native_textbox_dynamicinputmask_member_name(*runtime_object, property_name) ||
            is_native_column_dynamicalignment_member_name(*runtime_object, property_name) ||
            is_native_column_sparse_member_name(*runtime_object, property_name) ||
            is_native_textbox_format_member_name(*runtime_object, property_name) ||
            is_native_textbox_passwordchar_member_name(*runtime_object, property_name) ||
            is_native_textbox_maxlength_member_name(*runtime_object, property_name) ||
            is_native_textbox_specialeffect_member_name(*runtime_object, property_name) ||
            is_native_textbox_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_textbox_hideselection_member_name(*runtime_object, property_name) ||
            is_native_textbox_autocomplete_member_name(*runtime_object, property_name) ||
            is_native_textbox_enablehyperlinks_member_name(*runtime_object, property_name) ||
            is_native_textbox_tooltiptext_member_name(*runtime_object, property_name) ||
            is_native_textbox_margin_member_name(*runtime_object, property_name) ||
            is_native_textbox_mouseicon_member_name(*runtime_object, property_name) ||
            is_native_textbox_disabledbackcolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_disabledforecolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_disableditembackcolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_disableditemforecolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_itembackcolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_itemforecolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_selecteditembackcolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_selecteditemforecolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_statusbartext_member_name(*runtime_object, property_name) ||
            is_native_textbox_strictdateentry_member_name(*runtime_object, property_name) ||
            is_native_textbox_themes_member_name(*runtime_object, property_name) ||
            is_native_textbox_selectedbackcolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_selectedforecolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_dateformat_member_name(*runtime_object, property_name) ||
            is_native_textbox_century_member_name(*runtime_object, property_name) ||
            is_native_textbox_datemark_member_name(*runtime_object, property_name) ||
            is_native_textbox_hours_member_name(*runtime_object, property_name) ||
            is_native_textbox_seconds_member_name(*runtime_object, property_name) ||
            is_native_textbox_selection_member_name(*runtime_object, property_name) ||
            is_native_textbox_text_member_name(*runtime_object, property_name) ||
            is_native_visual_backcolor_member_name(*runtime_object, property_name) ||
            is_native_visual_forecolor_member_name(*runtime_object, property_name) ||
            is_native_tabindex_member_name(*runtime_object, property_name) ||
            is_native_tabstop_member_name(*runtime_object, property_name) ||
            is_native_visual_geometry_member_name(*runtime_object, property_name) ||
            is_native_combobox_style_member_name(*runtime_object, property_name) ||
            is_native_control_readonly_member_name(*runtime_object, property_name) ||
            is_native_string_control_value_member_name(*runtime_object, property_name) ||
            is_native_selectonentry_member_name(*runtime_object, property_name) ||
            is_native_resizable_member_name(*runtime_object, property_name) ||
            is_native_movable_member_name(*runtime_object, property_name) ||
            is_native_visual_anchor_member_name(*runtime_object, property_name) ||
            is_native_controlsource_member_name(*runtime_object, property_name) ||
            is_native_allowaddnew_member_name(*runtime_object, property_name) ||
            is_native_grid_rowheight_member_name(*runtime_object, property_name) ||
            is_native_grid_headerheight_member_name(*runtime_object, property_name) ||
            is_native_grid_allowheadersizing_member_name(*runtime_object, property_name) ||
            is_native_grid_allowrowsizing_member_name(*runtime_object, property_name) ||
            is_native_grid_allowautocolumnfit_member_name(*runtime_object, property_name) ||
            is_native_grid_gridlinecolor_member_name(*runtime_object, property_name) ||
            is_native_grid_gridlinewidth_member_name(*runtime_object, property_name) ||
            is_native_grid_highlightstyle_member_name(*runtime_object, property_name) ||
            is_native_grid_highlightrowlinewidth_member_name(*runtime_object, property_name) ||
            is_native_grid_activecolumn_member_name(*runtime_object, property_name) ||
            is_native_grid_activerow_member_name(*runtime_object, property_name) ||
            is_native_grid_relativecolumn_member_name(*runtime_object, property_name) ||
            is_native_grid_relativerow_member_name(*runtime_object, property_name) ||
            is_native_allowcellselection_member_name(*runtime_object, property_name) ||
            is_native_gridlines_member_name(*runtime_object, property_name) ||
            is_native_highlight_member_name(*runtime_object, property_name) ||
            is_native_highlightrow_member_name(*runtime_object, property_name) ||
            is_native_deletemark_member_name(*runtime_object, property_name) ||
            is_native_splitbar_member_name(*runtime_object, property_name) ||
            is_native_currentcontrol_member_name(*runtime_object, property_name) ||
            is_native_dynamiccurrentcontrol_member_name(*runtime_object, property_name) ||
            is_native_columnorder_member_name(*runtime_object, property_name) ||
            is_native_recordsource_member_name(*runtime_object, property_name) ||
            is_native_leftcolumn_member_name(*runtime_object, property_name) ||
            is_native_recordmark_member_name(*runtime_object, property_name) ||
            is_native_recordsourcetype_member_name(*runtime_object, property_name) ||
            is_native_rowsource_member_name(*runtime_object, property_name) ||
            is_native_rowsourcetype_member_name(*runtime_object, property_name) ||
            is_native_listindex_member_name(*runtime_object, property_name) ||
            is_native_displayvalue_member_name(*runtime_object, property_name) ||
            is_native_listcount_member_name(*runtime_object, property_name) ||
            is_native_sorted_member_name(*runtime_object, property_name) ||
            is_native_multiselect_member_name(*runtime_object, property_name) ||
            is_native_moverbars_member_name(*runtime_object, property_name) ||
            is_native_autohidescrollbar_member_name(*runtime_object, property_name) ||
            is_native_firstelement_member_name(*runtime_object, property_name) ||
            is_native_numberofelements_member_name(*runtime_object, property_name) ||
            is_native_displaycount_member_name(*runtime_object, property_name) ||
            is_native_nulldisplay_member_name(*runtime_object, property_name) ||
            is_native_columnlines_member_name(*runtime_object, property_name) ||
            is_native_itemtips_member_name(*runtime_object, property_name) ||
            is_native_incrementalsearch_member_name(*runtime_object, property_name) ||
            is_native_integralheight_member_name(*runtime_object, property_name) ||
            is_native_boundto_member_name(*runtime_object, property_name) ||
            is_native_newindex_member_name(*runtime_object, property_name) ||
            is_native_newitemid_member_name(*runtime_object, property_name) ||
            is_native_listitemid_member_name(*runtime_object, property_name) ||
            native_listitem_member_name_matches(*runtime_object, property_name) ||
            native_itemdata_member_name_matches(*runtime_object, property_name) ||
            is_native_topitemid_member_name(*runtime_object, property_name) ||
            is_native_topindex_member_name(*runtime_object, property_name) ||
            is_native_boundcolumn_member_name(*runtime_object, property_name) ||
            is_native_columncount_member_name(*runtime_object, property_name) ||
            is_native_column_bound_member_name(*runtime_object, property_name) ||
            is_native_columnwidths_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_creation_time_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_object_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_inspection_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_conflict_member_name(*runtime_object, property_name) ||
            native_child_parent_member_name_matches(*runtime_object, property_name) ||
            is_native_collection_member_name(*runtime_object, property_name)) {
            return make_boolean_value(false);
        }
        if (runtime_object->default_properties.contains(property_name)) {
            return make_boolean_value(false);
        }
        if (!reflectable_member_exists_locally(*runtime_object, property_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr &&
                reflectable_member_exists_locally(*object_surface, property_name)) {
                return make_boolean_value(false);
            }
        }
        const PrgValue initial_value = arguments.size() >= 3U ? arguments[2] : make_empty_value();
        runtime_object->properties[property_name] = initial_value;
        return make_boolean_value(true);
    }

    if (function == "getpem" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", "GETPEM()"}}));
            return make_empty_value();
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (runtime_object == nullptr || member_name.empty()) {
            return make_empty_value();
        }
        if (is_native_textbox_selection_member_name(*runtime_object, member_name)) {
            normalize_native_textbox_selection_invariant(*runtime_object);
        }
        if (const auto list_cell = parse_native_list_control_list_member_cell(*runtime_object, member_name);
            list_cell.has_value()) {
            return *read_native_list_control_cell(
                *runtime_object,
                list_cell->row_slot,
                list_cell->column_slot);
        }
        if (const auto item_cell =
                parse_native_list_control_listitem_member_cell(*runtime_object, member_name);
            item_cell.has_value()) {
            return *read_native_list_control_item_cell(
                *runtime_object,
                item_cell->item_id,
                item_cell->column_slot);
        }
        if (const auto item_data_slot =
                parse_native_list_control_itemdata_member_slot(*runtime_object, member_name);
            item_data_slot.has_value()) {
            return *read_native_list_control_item_data(*runtime_object, *item_data_slot);
        }
        if (is_native_topitemid_member_name(*runtime_object, member_name)) {
            sync_native_list_control_top_item_id(*runtime_object);
            return runtime_object->properties[member_name];
        }
        if (is_native_topindex_member_name(*runtime_object, member_name)) {
            sync_native_list_control_top_item_id(*runtime_object);
            return runtime_object->properties[member_name];
        }
        if (const auto item_id_slot =
                parse_native_list_control_indextoitemid_member_slot(*runtime_object, member_name);
            item_id_slot.has_value()) {
            return *read_native_list_control_item_id_for_slot(*runtime_object, *item_id_slot);
        }
        if (const auto item_index_id =
                parse_native_list_control_itemidtoindex_member_item_id(*runtime_object, member_name);
            item_index_id.has_value()) {
            return *read_native_list_control_index_for_item_id(*runtime_object, *item_index_id);
        }
        if (is_native_listcount_member_name(*runtime_object, member_name)) {
            sync_native_list_control_count(*runtime_object);
        }
        if (is_native_activepage_member_name(*runtime_object, member_name)) {
            normalize_native_pageframe_activepage_invariant(*runtime_object);
        }
        if (is_native_listitemid_member_name(*runtime_object, member_name)) {
            sync_native_list_control_displayvalue_from_selection(*runtime_object);
            const auto property = runtime_object->properties.find(member_name);
            if (property != runtime_object->properties.end()) {
                return property->second;
            }
        }
        if (const auto metadata_value = get_native_identity_reflection_metadata(*runtime_object, member_name);
            metadata_value.has_value()) {
            return *metadata_value;
        }
        if (const auto collection_value = read_native_collection_member(*runtime_object, member_name);
            collection_value.has_value()) {
            return *collection_value;
        }
        if (runtime_object->source.empty()) {
            const auto prop_it = runtime_object->properties.find(member_name);
            if (prop_it != runtime_object->properties.end()) {
                return prop_it->second;
            }
        }
        if (!runtime_object->source.empty()) {
            if (read_native_member_callback) {
                const auto member_value = read_native_member_callback(arguments[0], member_name);
                if (member_value.has_value()) {
                    return *member_value;
                }
            }
            if (object_has_accessor_property(*runtime_object, member_name)) {
                return make_boolean_value(true);
            }
        } else if (object_has_accessor_property(*runtime_object, member_name)) {
            return make_boolean_value(true);
        }
        if (is_native_collection_member_name(*runtime_object, member_name)) {
            return make_boolean_value(true);
        }
        if (object_has_member(runtime_object->methods, member_name) ||
            object_has_member(runtime_object->events, member_name)) {
            return make_boolean_value(true);
        }
        if (is_builtin_native_runtime_method_name(*runtime_object, member_name)) {
            return make_boolean_value(true);
        }
        if (!reflectable_member_exists_locally(*runtime_object, member_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr) {
                if (const auto metadata_value =
                        get_native_identity_reflection_metadata(*object_surface, member_name);
                    metadata_value.has_value()) {
                    return *metadata_value;
                }
                if (const auto collection_value =
                        read_native_collection_member(*object_surface, member_name);
                    collection_value.has_value()) {
                    return *collection_value;
                }
                const auto property = object_surface->properties.find(member_name);
                if (property != object_surface->properties.end()) {
                    return property->second;
                }
                if (object_has_accessor_property(*object_surface, member_name) ||
                    is_native_collection_member_name(*object_surface, member_name) ||
                    is_builtin_native_runtime_method_name(*object_surface, member_name) ||
                    object_has_member(object_surface->methods, member_name) ||
                    object_has_member(object_surface->events, member_name)) {
                    return make_boolean_value(true);
                }
            }
        }
        return make_empty_value();
    }
    if ((function == "putpem" || function == "setpem") && arguments.size() >= 3U) {
        const std::string function_display_name = function == "putpem" ? "PUTPEM()" : "SETPEM()";
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", function_display_name}}));
            return make_boolean_value(false);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (runtime_object == nullptr || member_name.empty()) {
            return make_boolean_value(false);
        }
        if (is_scripting_dictionary_object(*runtime_object) && member_name == "count") {
            return make_boolean_value(false);
        }
        if (const auto list_cell = parse_native_list_control_list_member_cell(*runtime_object, member_name);
            list_cell.has_value()) {
            return make_boolean_value(
                write_native_list_control_cell(
                    *runtime_object,
                    list_cell->row_slot,
                    list_cell->column_slot,
                    arguments[2]));
        }
        if (const auto item_cell =
                parse_native_list_control_listitem_member_cell(*runtime_object, member_name);
            item_cell.has_value()) {
            return make_boolean_value(
                write_native_list_control_item_cell(
                    *runtime_object,
                    item_cell->item_id,
                    item_cell->column_slot,
                    arguments[2]));
        }
        if (const auto item_data_slot =
                parse_native_list_control_itemdata_member_slot(*runtime_object, member_name);
            item_data_slot.has_value()) {
            return make_boolean_value(
                write_native_list_control_item_data(
                    *runtime_object,
                    *item_data_slot,
                    arguments[2]));
        }
        if (is_native_topitemid_member_name(*runtime_object, member_name)) {
            return make_boolean_value(
                write_native_list_control_top_item_id(*runtime_object, arguments[2]));
        }
        if (is_native_topindex_member_name(*runtime_object, member_name)) {
            return make_boolean_value(
                write_native_list_control_top_index(*runtime_object, arguments[2]));
        }
        if (is_native_textbox_selection_member_name(*runtime_object, member_name)) {
            return make_boolean_value(write_native_textbox_selection_property(
                *runtime_object,
                member_name,
                arguments[2]));
        }
        if (is_native_textbox_text_member_name(*runtime_object, member_name)) {
            return make_boolean_value(false);
        }
        if (native_child_parent_member_name_matches(*runtime_object, member_name) ||
            is_native_controlcount_member_name(*runtime_object, member_name) ||
            is_native_listcount_member_name(*runtime_object, member_name) ||
            is_native_newindex_member_name(*runtime_object, member_name) ||
            is_native_newitemid_member_name(*runtime_object, member_name) ||
            is_native_child_collection_member_name(*runtime_object, member_name) ||
            is_native_name_member_name(*runtime_object, member_name) ||
            is_native_splitbar_member_name(*runtime_object, member_name) ||
            is_native_leftcolumn_member_name(*runtime_object, member_name) ||
            is_native_grid_activecolumn_member_name(*runtime_object, member_name) ||
            is_native_grid_activerow_member_name(*runtime_object, member_name) ||
            is_native_grid_relativecolumn_member_name(*runtime_object, member_name) ||
            is_native_grid_relativerow_member_name(*runtime_object, member_name) ||
            is_native_form_desktop_member_name(*runtime_object, member_name) ||
            is_native_form_scrollbars_member_name(*runtime_object, member_name) ||
            is_native_movable_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_creation_time_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_object_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_inspection_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_conflict_member_name(*runtime_object, member_name) ||
            is_native_integralheight_member_name(*runtime_object, member_name) ||
            is_native_collection_readonly_member_name(*runtime_object, member_name)) {
            return make_boolean_value(false);
        }
        if (parse_native_list_control_selected_member_slot(*runtime_object, member_name).has_value()) {
            return make_boolean_value(
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]));
        }
        if (parse_native_list_control_selectedid_member_item_id(*runtime_object, member_name).has_value()) {
            return make_boolean_value(
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]));
        }
        if (is_native_listitemid_member_name(*runtime_object, member_name)) {
            return make_boolean_value(
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]));
        }
        if (is_native_activepage_member_name(*runtime_object, member_name)) {
            runtime_object->properties[member_name] = arguments[2];
            normalize_native_pageframe_activepage_invariant(*runtime_object);
            return make_boolean_value(true);
        }
        if (member_name == "value" &&
            (normalize_identifier(runtime_object->base_class_name) == "combobox" ||
             normalize_identifier(runtime_object->base_class_name) == "listbox")) {
            return make_boolean_value(
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]));
        }
        if (member_name == "boundto") {
            const bool was_boundto = native_list_control_boundto_enabled(*runtime_object);
            const bool wrote =
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]);
            if (wrote) {
                update_native_list_control_boundto_index_value_mode(*runtime_object, was_boundto);
            }
            return make_boolean_value(wrote);
        }
        if (is_native_pagecount_member_name(*runtime_object, member_name)) {
            return make_boolean_value(
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]));
        }
        if (!reflectable_member_exists_locally(*runtime_object, member_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr) {
                return make_boolean_value(
                    write_native_member_callback &&
                    write_native_member_callback(arguments[0], member_name, arguments[2]));
            }
        }
        if (!runtime_object->source.empty() &&
            (object_has_assigner_property(*runtime_object, member_name) ||
             runtime_object->properties.contains(member_name) ||
             object_has_member(runtime_object->methods, member_name) ||
             object_has_member(runtime_object->events, member_name))) {
            return make_boolean_value(
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]));
        }
        if (object_has_assigner_property(*runtime_object, member_name)) {
            return make_boolean_value(false);
        }
        if (object_has_member(runtime_object->methods, member_name) ||
            object_has_member(runtime_object->events, member_name)) {
            if (member_name == "columnorder" ||
                member_name == "columncount" ||
                member_name == "pagecount" ||
                is_native_column_bound_member_name(*runtime_object, member_name) ||
                is_native_controlsource_member_name(*runtime_object, member_name)) {
                return make_boolean_value(
                    write_native_member_callback &&
                    write_native_member_callback(arguments[0], member_name, arguments[2]));
            }
            if (member_name == "readonly" &&
                native_combobox_readonly_assignment_blocked(*runtime_object, arguments[2])) {
                return make_boolean_value(false);
            }
            if (is_native_moverbars_member_name(*runtime_object, member_name) &&
                !native_list_control_rowsourcetype_supports_additem(*runtime_object)) {
                return make_boolean_value(false);
            }
            runtime_object->properties[member_name] = arguments[2];
            if (member_name == "keypreview") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontcharset") {
                normalize_native_visual_fontcharset_invariant(*runtime_object);
            }
            if (member_name == "mousepointer") {
                normalize_native_visual_mousepointer_invariant(*runtime_object);
            }
            if (member_name == "dragmode") {
                normalize_native_visual_dragmode_invariant(*runtime_object);
            }
            if (member_name == "anchor") {
                normalize_native_visual_anchor_invariant(*runtime_object);
            }
            if (member_name == "fontsize") {
                const double font_size = value_as_number(runtime_object->properties[member_name]);
                runtime_object->properties[member_name] = make_number_value(
                    std::isfinite(font_size) && font_size >= 0.0 ? font_size : 0.0);
            }
            if (member_name == "fontbold") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontitalic") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontunderline") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontstrikethru") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontoutline") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontshadow") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "alignment") {
                normalize_native_visual_alignment_invariant(*runtime_object);
            }
            if (member_name == "scalemode") {
                normalize_native_form_scalemode_invariant(*runtime_object);
            }
            if (member_name == "drawstyle") {
                normalize_native_form_drawstyle_invariant(*runtime_object);
            }
            if (member_name == "fillstyle") {
                normalize_native_visual_fillstyle_invariant(*runtime_object);
            }
            if (member_name == "fillcolor") {
                normalize_native_visual_fillcolor_invariant(*runtime_object);
            }
            if (member_name == "borderwidth") {
                normalize_native_visual_borderwidth_invariant(*runtime_object);
            }
            if (member_name == "bordercolor") {
                normalize_native_visual_bordercolor_invariant(*runtime_object);
            }
            if (member_name == "borderstyle") {
                normalize_native_visual_borderstyle_invariant(*runtime_object);
            }
            if (member_name == "drawwidth") {
                normalize_native_form_drawwidth_invariant(*runtime_object);
            }
            if (member_name == "drawmode") {
                normalize_native_visual_drawmode_invariant(*runtime_object);
            }
            if (member_name == "backstyle") {
                normalize_native_visual_backstyle_invariant(*runtime_object);
            }
            if (member_name == "rowheight") {
                normalize_native_grid_rowheight_invariant(*runtime_object);
            }
            if (member_name == "headerheight") {
                normalize_native_grid_headerheight_invariant(*runtime_object);
            }
            if (member_name == "allowheadersizing") {
                normalize_native_grid_allowheadersizing_invariant(*runtime_object);
            }
            if (member_name == "allowrowsizing") {
                normalize_native_grid_allowrowsizing_invariant(*runtime_object);
            }
            if (member_name == "allowautocolumnfit") {
                normalize_native_grid_allowautocolumnfit_invariant(*runtime_object);
            }
            if (member_name == "gridlinecolor") {
                normalize_native_grid_gridlinecolor_invariant(*runtime_object);
            }
            if (member_name == "gridlinewidth") {
                normalize_native_grid_gridlinewidth_invariant(*runtime_object);
            }
            if (member_name == "highlightstyle") {
                normalize_native_grid_highlightstyle_invariant(*runtime_object);
            }
            if (member_name == "highlightrowlinewidth") {
                normalize_native_grid_highlightrowlinewidth_invariant(*runtime_object);
            }
            if (member_name == "scrollbars") {
                normalize_native_editbox_scrollbars_invariant(*runtime_object);
            }
            if (member_name == "inputmask") {
                normalize_native_textbox_inputmask_invariant(*runtime_object);
            }
            if (member_name == "dynamicinputmask") {
                normalize_native_textbox_dynamicinputmask_invariant(*runtime_object);
            }
            if (member_name == "dynamicalignment") {
                normalize_native_column_dynamicalignment_invariant(*runtime_object);
            }
            if (member_name == "sparse") {
                normalize_native_column_sparse_invariant(*runtime_object);
            }
            if (member_name == "format") {
                normalize_native_textbox_format_invariant(*runtime_object);
            }
            if (member_name == "passwordchar") {
                normalize_native_textbox_passwordchar_invariant(*runtime_object);
            }
            if (member_name == "maxlength") {
                normalize_native_textbox_maxlength_invariant(*runtime_object);
            }
            if (member_name == "specialeffect") {
                normalize_native_textbox_specialeffect_invariant(*runtime_object);
            }
            if (member_name == "specialeffect") {
                normalize_native_visual_specialeffect_invariant(*runtime_object);
            }
            if (member_name == "borderstyle") {
                normalize_native_textbox_borderstyle_invariant(*runtime_object);
            }
            if (member_name == "hideselection") {
                normalize_native_textbox_hideselection_invariant(*runtime_object);
            }
            if (member_name == "autocomplete") {
                normalize_native_textbox_autocomplete_invariant(*runtime_object);
            }
            if (member_name == "enablehyperlinks") {
                normalize_native_textbox_enablehyperlinks_invariant(*runtime_object);
            }
            if (member_name == "tooltiptext") {
                normalize_native_textbox_tooltiptext_invariant(*runtime_object);
            }
            if (member_name == "margin") {
                normalize_native_textbox_margin_invariant(*runtime_object);
            }
            if (member_name == "mouseicon") {
                normalize_native_textbox_mouseicon_invariant(*runtime_object);
            }
            if (member_name == "disabledbackcolor") {
                normalize_native_textbox_disabledbackcolor_invariant(*runtime_object);
            }
            if (member_name == "disabledforecolor") {
                normalize_native_textbox_disabledforecolor_invariant(*runtime_object);
            }
            if (member_name == "disableditembackcolor") {
                normalize_native_list_control_disableditembackcolor_invariant(*runtime_object);
            }
            if (member_name == "disableditemforecolor") {
                normalize_native_list_control_disableditemforecolor_invariant(*runtime_object);
            }
            if (member_name == "itembackcolor") {
                normalize_native_list_control_itembackcolor_invariant(*runtime_object);
            }
            if (member_name == "itemforecolor") {
                normalize_native_list_control_itemforecolor_invariant(*runtime_object);
            }
            if (member_name == "selecteditembackcolor") {
                normalize_native_list_control_selecteditembackcolor_invariant(*runtime_object);
            }
            if (member_name == "selecteditemforecolor") {
                normalize_native_list_control_selecteditemforecolor_invariant(*runtime_object);
            }
            if (member_name == "statusbartext") {
                normalize_native_textbox_statusbartext_invariant(*runtime_object);
            }
            if (member_name == "helpcontextid") {
                normalize_native_visual_helpcontextid_invariant(*runtime_object);
            }
            if (member_name == "whatsthishelpid") {
                normalize_native_visual_whatsthishelpid_invariant(*runtime_object);
            }
            if (member_name == "default" || member_name == "cancel") {
                normalize_native_commandbutton_default_cancel_invariant(*runtime_object);
            }
            if (member_name == "style") {
                normalize_native_commandbutton_style_invariant(*runtime_object);
            }
            if (member_name == "picturemargin" ||
                member_name == "pictureposition" ||
                member_name == "picturespacing") {
                normalize_native_commandbutton_picture_layout_invariant(*runtime_object);
            }
            if (member_name == "strictdateentry") {
                normalize_native_textbox_strictdateentry_invariant(*runtime_object);
            }
            if (member_name == "themes") {
                normalize_native_textbox_themes_invariant(*runtime_object);
            }
            if (member_name == "selectedbackcolor") {
                normalize_native_textbox_selectedbackcolor_invariant(*runtime_object);
            }
            if (member_name == "selectedforecolor") {
                normalize_native_textbox_selectedforecolor_invariant(*runtime_object);
            }
            if (member_name == "dateformat") {
                normalize_native_textbox_dateformat_invariant(*runtime_object);
            }
            if (member_name == "century") {
                normalize_native_textbox_century_invariant(*runtime_object);
            }
            if (member_name == "datemark") {
                normalize_native_textbox_datemark_invariant(*runtime_object);
            }
            if (member_name == "hours") {
                normalize_native_textbox_hours_invariant(*runtime_object);
            }
            if (member_name == "seconds") {
                normalize_native_textbox_seconds_invariant(*runtime_object);
            }
            if (member_name == "style" || member_name == "readonly") {
                normalize_native_combobox_readonly_invariant(*runtime_object);
            }
            if (is_native_activepage_member_name(*runtime_object, member_name)) {
                normalize_native_pageframe_activepage_invariant(*runtime_object);
            }
            if (member_name == "multiselect") {
                normalize_native_listbox_multiselect_invariant(*runtime_object);
            }
            if (member_name == "sorted" ||
                member_name == "rowsourcetype") {
                normalize_native_list_control_sorted_invariant(*runtime_object);
            }
            if (member_name == "moverbars" ||
                member_name == "rowsourcetype") {
                normalize_native_listbox_moverbars_invariant(*runtime_object);
            }
            if (member_name == "autohidescrollbar") {
                normalize_native_listbox_autohidescrollbar_invariant(*runtime_object);
            }
            if (member_name == "firstelement" ||
                member_name == "numberofelements") {
                normalize_native_list_control_array_range_invariants(*runtime_object);
            }
            if (member_name == "displaycount") {
                normalize_native_combobox_displaycount_invariant(*runtime_object);
            }
            if (member_name == "nulldisplay") {
                normalize_native_list_control_nulldisplay_invariant(*runtime_object);
            }
            if (member_name == "columnlines") {
                normalize_native_list_control_columnlines_invariant(*runtime_object);
            }
            if (member_name == "columnwidths") {
                normalize_native_list_control_columnwidths_invariant(*runtime_object);
            }
            if (member_name == "itemtips") {
                normalize_native_list_control_itemtips_invariant(*runtime_object);
            }
            if (member_name == "incrementalsearch") {
                normalize_native_list_control_incrementalsearch_invariant(*runtime_object);
            }
            if (member_name == "selectonentry" &&
                runtime_object->properties.contains("selectonentry")) {
                runtime_object->properties["selectonentry"] =
                    make_boolean_value(value_as_bool(runtime_object->properties["selectonentry"]));
            }
            if (member_name == "resizable" &&
                is_native_resizable_member_name(*runtime_object, member_name)) {
                runtime_object->properties["resizable"] =
                    make_boolean_value(value_as_bool(runtime_object->properties["resizable"]));
            }
            return make_boolean_value(true);
        }
        if (runtime_object->properties.contains(member_name)) {
            if (member_name == "columnorder" ||
                member_name == "columncount" ||
                member_name == "pagecount" ||
                is_native_column_bound_member_name(*runtime_object, member_name) ||
                is_native_controlsource_member_name(*runtime_object, member_name)) {
                return make_boolean_value(
                    write_native_member_callback &&
                    write_native_member_callback(arguments[0], member_name, arguments[2]));
            }
            if (member_name == "readonly" &&
                native_combobox_readonly_assignment_blocked(*runtime_object, arguments[2])) {
                return make_boolean_value(false);
            }
            if (is_native_moverbars_member_name(*runtime_object, member_name) &&
                !native_list_control_rowsourcetype_supports_additem(*runtime_object)) {
                return make_boolean_value(false);
            }
            runtime_object->properties[member_name] = arguments[2];
            if (member_name == "keypreview") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontcharset") {
                normalize_native_visual_fontcharset_invariant(*runtime_object);
            }
            if (member_name == "mousepointer") {
                normalize_native_visual_mousepointer_invariant(*runtime_object);
            }
            if (member_name == "dragmode") {
                normalize_native_visual_dragmode_invariant(*runtime_object);
            }
            if (member_name == "anchor") {
                normalize_native_visual_anchor_invariant(*runtime_object);
            }
            if (member_name == "fontsize") {
                const double font_size = value_as_number(runtime_object->properties[member_name]);
                runtime_object->properties[member_name] = make_number_value(
                    std::isfinite(font_size) && font_size >= 0.0 ? font_size : 0.0);
            }
            if (member_name == "fontbold") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontitalic") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontunderline") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontstrikethru") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontoutline") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "fontshadow") {
                runtime_object->properties[member_name] = make_boolean_value(
                    value_as_bool(runtime_object->properties[member_name]));
            }
            if (member_name == "alignment") {
                normalize_native_visual_alignment_invariant(*runtime_object);
            }
            if (member_name == "scalemode") {
                normalize_native_form_scalemode_invariant(*runtime_object);
            }
            if (member_name == "drawstyle") {
                normalize_native_form_drawstyle_invariant(*runtime_object);
            }
            if (member_name == "fillstyle") {
                normalize_native_visual_fillstyle_invariant(*runtime_object);
            }
            if (member_name == "fillcolor") {
                normalize_native_visual_fillcolor_invariant(*runtime_object);
            }
            if (member_name == "borderwidth") {
                normalize_native_visual_borderwidth_invariant(*runtime_object);
            }
            if (member_name == "bordercolor") {
                normalize_native_visual_bordercolor_invariant(*runtime_object);
            }
            if (member_name == "borderstyle") {
                normalize_native_visual_borderstyle_invariant(*runtime_object);
            }
            if (member_name == "drawwidth") {
                normalize_native_form_drawwidth_invariant(*runtime_object);
            }
            if (member_name == "drawmode") {
                normalize_native_visual_drawmode_invariant(*runtime_object);
            }
            if (member_name == "backstyle") {
                normalize_native_visual_backstyle_invariant(*runtime_object);
            }
            if (member_name == "rowheight") {
                normalize_native_grid_rowheight_invariant(*runtime_object);
            }
            if (member_name == "headerheight") {
                normalize_native_grid_headerheight_invariant(*runtime_object);
            }
            if (member_name == "allowheadersizing") {
                normalize_native_grid_allowheadersizing_invariant(*runtime_object);
            }
            if (member_name == "allowrowsizing") {
                normalize_native_grid_allowrowsizing_invariant(*runtime_object);
            }
            if (member_name == "allowautocolumnfit") {
                normalize_native_grid_allowautocolumnfit_invariant(*runtime_object);
            }
            if (member_name == "gridlinecolor") {
                normalize_native_grid_gridlinecolor_invariant(*runtime_object);
            }
            if (member_name == "gridlinewidth") {
                normalize_native_grid_gridlinewidth_invariant(*runtime_object);
            }
            if (member_name == "highlightstyle") {
                normalize_native_grid_highlightstyle_invariant(*runtime_object);
            }
            if (member_name == "highlightrowlinewidth") {
                normalize_native_grid_highlightrowlinewidth_invariant(*runtime_object);
            }
            if (member_name == "scrollbars") {
                normalize_native_editbox_scrollbars_invariant(*runtime_object);
            }
            if (member_name == "inputmask") {
                normalize_native_textbox_inputmask_invariant(*runtime_object);
            }
            if (member_name == "dynamicinputmask") {
                normalize_native_textbox_dynamicinputmask_invariant(*runtime_object);
            }
            if (member_name == "dynamicalignment") {
                normalize_native_column_dynamicalignment_invariant(*runtime_object);
            }
            if (member_name == "sparse") {
                normalize_native_column_sparse_invariant(*runtime_object);
            }
            if (member_name == "format") {
                normalize_native_textbox_format_invariant(*runtime_object);
            }
            if (member_name == "passwordchar") {
                normalize_native_textbox_passwordchar_invariant(*runtime_object);
            }
            if (member_name == "maxlength") {
                normalize_native_textbox_maxlength_invariant(*runtime_object);
            }
            if (member_name == "specialeffect") {
                normalize_native_textbox_specialeffect_invariant(*runtime_object);
            }
            if (member_name == "specialeffect") {
                normalize_native_visual_specialeffect_invariant(*runtime_object);
            }
            if (member_name == "borderstyle") {
                normalize_native_textbox_borderstyle_invariant(*runtime_object);
            }
            if (member_name == "hideselection") {
                normalize_native_textbox_hideselection_invariant(*runtime_object);
            }
            if (member_name == "autocomplete") {
                normalize_native_textbox_autocomplete_invariant(*runtime_object);
            }
            if (member_name == "enablehyperlinks") {
                normalize_native_textbox_enablehyperlinks_invariant(*runtime_object);
            }
            if (member_name == "tooltiptext") {
                normalize_native_textbox_tooltiptext_invariant(*runtime_object);
            }
            if (member_name == "margin") {
                normalize_native_textbox_margin_invariant(*runtime_object);
            }
            if (member_name == "mouseicon") {
                normalize_native_textbox_mouseicon_invariant(*runtime_object);
            }
            if (member_name == "disabledbackcolor") {
                normalize_native_textbox_disabledbackcolor_invariant(*runtime_object);
            }
            if (member_name == "disabledforecolor") {
                normalize_native_textbox_disabledforecolor_invariant(*runtime_object);
            }
            if (member_name == "disableditembackcolor") {
                normalize_native_list_control_disableditembackcolor_invariant(*runtime_object);
            }
            if (member_name == "disableditemforecolor") {
                normalize_native_list_control_disableditemforecolor_invariant(*runtime_object);
            }
            if (member_name == "itembackcolor") {
                normalize_native_list_control_itembackcolor_invariant(*runtime_object);
            }
            if (member_name == "itemforecolor") {
                normalize_native_list_control_itemforecolor_invariant(*runtime_object);
            }
            if (member_name == "selecteditembackcolor") {
                normalize_native_list_control_selecteditembackcolor_invariant(*runtime_object);
            }
            if (member_name == "selecteditemforecolor") {
                normalize_native_list_control_selecteditemforecolor_invariant(*runtime_object);
            }
            if (member_name == "statusbartext") {
                normalize_native_textbox_statusbartext_invariant(*runtime_object);
            }
            if (member_name == "helpcontextid") {
                normalize_native_visual_helpcontextid_invariant(*runtime_object);
            }
            if (member_name == "whatsthishelpid") {
                normalize_native_visual_whatsthishelpid_invariant(*runtime_object);
            }
            if (member_name == "default" || member_name == "cancel") {
                normalize_native_commandbutton_default_cancel_invariant(*runtime_object);
            }
            if (member_name == "style") {
                normalize_native_commandbutton_style_invariant(*runtime_object);
            }
            if (member_name == "picturemargin" ||
                member_name == "pictureposition" ||
                member_name == "picturespacing") {
                normalize_native_commandbutton_picture_layout_invariant(*runtime_object);
            }
            if (member_name == "strictdateentry") {
                normalize_native_textbox_strictdateentry_invariant(*runtime_object);
            }
            if (member_name == "themes") {
                normalize_native_textbox_themes_invariant(*runtime_object);
            }
            if (member_name == "selectedbackcolor") {
                normalize_native_textbox_selectedbackcolor_invariant(*runtime_object);
            }
            if (member_name == "selectedforecolor") {
                normalize_native_textbox_selectedforecolor_invariant(*runtime_object);
            }
            if (member_name == "dateformat") {
                normalize_native_textbox_dateformat_invariant(*runtime_object);
            }
            if (member_name == "century") {
                normalize_native_textbox_century_invariant(*runtime_object);
            }
            if (member_name == "datemark") {
                normalize_native_textbox_datemark_invariant(*runtime_object);
            }
            if (member_name == "hours") {
                normalize_native_textbox_hours_invariant(*runtime_object);
            }
            if (member_name == "seconds") {
                normalize_native_textbox_seconds_invariant(*runtime_object);
            }
            if (member_name == "style" || member_name == "readonly") {
                normalize_native_combobox_readonly_invariant(*runtime_object);
            }
            if (is_native_activepage_member_name(*runtime_object, member_name)) {
                normalize_native_pageframe_activepage_invariant(*runtime_object);
            }
            if (member_name == "multiselect") {
                normalize_native_listbox_multiselect_invariant(*runtime_object);
            }
            if (member_name == "sorted" ||
                member_name == "rowsourcetype") {
                normalize_native_list_control_sorted_invariant(*runtime_object);
            }
            if (member_name == "moverbars" ||
                member_name == "rowsourcetype") {
                normalize_native_listbox_moverbars_invariant(*runtime_object);
            }
            if (member_name == "autohidescrollbar") {
                normalize_native_listbox_autohidescrollbar_invariant(*runtime_object);
            }
            if (member_name == "firstelement" ||
                member_name == "numberofelements") {
                normalize_native_list_control_array_range_invariants(*runtime_object);
            }
            if (member_name == "displaycount") {
                normalize_native_combobox_displaycount_invariant(*runtime_object);
            }
            if (member_name == "nulldisplay") {
                normalize_native_list_control_nulldisplay_invariant(*runtime_object);
            }
            if (member_name == "columnlines") {
                normalize_native_list_control_columnlines_invariant(*runtime_object);
            }
            if (member_name == "columnwidths") {
                normalize_native_list_control_columnwidths_invariant(*runtime_object);
            }
            if (member_name == "itemtips") {
                normalize_native_list_control_itemtips_invariant(*runtime_object);
            }
            if (member_name == "incrementalsearch") {
                normalize_native_list_control_incrementalsearch_invariant(*runtime_object);
            }
            if (member_name == "selectonentry" &&
                runtime_object->properties.contains("selectonentry")) {
                runtime_object->properties["selectonentry"] =
                    make_boolean_value(value_as_bool(runtime_object->properties["selectonentry"]));
            }
            if (member_name == "resizable" &&
                is_native_resizable_member_name(*runtime_object, member_name)) {
                runtime_object->properties["resizable"] =
                    make_boolean_value(value_as_bool(runtime_object->properties["resizable"]));
            }
            if (member_name == "boundcolumn" ||
                member_name == "boundto") {
                sync_native_list_control_displayvalue_from_selection(*runtime_object);
            }
            return make_boolean_value(true);
        }
        return make_boolean_value(false);
    }
    if (function == "removeproperty" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", "REMOVEPROPERTY()"}}));
            return make_boolean_value(true);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        if (runtime_object == nullptr) {
            return make_boolean_value(false);
        }
        const std::string property_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (property_name.empty()) {
            return make_boolean_value(false);
        }
        if (parse_native_list_control_list_member_cell(*runtime_object, property_name).has_value() ||
            parse_native_list_control_listitem_member_cell(*runtime_object, property_name).has_value() ||
            parse_native_list_control_itemdata_member_slot(*runtime_object, property_name).has_value() ||
            parse_native_list_control_indextoitemid_member_slot(*runtime_object, property_name).has_value() ||
            parse_native_list_control_itemidtoindex_member_item_id(*runtime_object, property_name).has_value()) {
            return make_boolean_value(false);
        }
        if (is_native_identity_member_name(*runtime_object, property_name) ||
            is_native_controlcount_member_name(*runtime_object, property_name) ||
            is_native_pagecount_member_name(*runtime_object, property_name) ||
            is_native_activepage_member_name(*runtime_object, property_name) ||
            is_native_child_collection_member_name(*runtime_object, property_name) ||
            is_native_name_member_name(*runtime_object, property_name) ||
            is_native_form_alwaysontop_member_name(*runtime_object, property_name) ||
            is_native_form_keypreview_member_name(*runtime_object, property_name) ||
            is_native_form_showwindow_member_name(*runtime_object, property_name) ||
            is_native_form_windowtype_member_name(*runtime_object, property_name) ||
            is_native_form_windowstate_member_name(*runtime_object, property_name) ||
            is_native_form_scalemode_member_name(*runtime_object, property_name) ||
            is_native_form_drawstyle_member_name(*runtime_object, property_name) ||
            is_native_visual_fillstyle_member_name(*runtime_object, property_name) ||
            is_native_visual_fillcolor_member_name(*runtime_object, property_name) ||
            is_native_visual_borderwidth_member_name(*runtime_object, property_name) ||
            is_native_visual_bordercolor_member_name(*runtime_object, property_name) ||
            is_native_visual_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_form_drawwidth_member_name(*runtime_object, property_name) ||
            is_native_form_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_form_titlebar_member_name(*runtime_object, property_name) ||
            is_native_form_desktop_member_name(*runtime_object, property_name) ||
            is_native_form_scrollbars_member_name(*runtime_object, property_name) ||
            is_native_form_lockscreen_member_name(*runtime_object, property_name) ||
            is_native_form_controlbox_member_name(*runtime_object, property_name) ||
            is_native_form_closable_member_name(*runtime_object, property_name) ||
            is_native_form_minbutton_member_name(*runtime_object, property_name) ||
            is_native_form_maxbutton_member_name(*runtime_object, property_name) ||
            is_native_form_autocenter_member_name(*runtime_object, property_name) ||
            is_native_visual_enabled_member_name(*runtime_object, property_name) ||
            is_native_visual_fontname_member_name(*runtime_object, property_name) ||
            is_native_visual_fontcharset_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontname_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontsize_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontshadow_member_name(*runtime_object, property_name) ||
            is_native_visual_dynamicfontoutline_member_name(*runtime_object, property_name) ||
            is_native_visual_fontsize_member_name(*runtime_object, property_name) ||
            is_native_visual_fontbold_member_name(*runtime_object, property_name) ||
            is_native_visual_fontitalic_member_name(*runtime_object, property_name) ||
            is_native_visual_fontunderline_member_name(*runtime_object, property_name) ||
            is_native_visual_fontstrikethru_member_name(*runtime_object, property_name) ||
            is_native_visual_fontoutline_member_name(*runtime_object, property_name) ||
            is_native_visual_fontshadow_member_name(*runtime_object, property_name) ||
            is_native_visual_visible_member_name(*runtime_object, property_name) ||
            is_native_controltiptext_member_name(*runtime_object, property_name) ||
            is_native_visual_helpcontextid_member_name(*runtime_object, property_name) ||
            is_native_visual_whatsthishelpid_member_name(*runtime_object, property_name) ||
            is_native_visual_tag_member_name(*runtime_object, property_name) ||
            is_native_visual_caption_member_name(*runtime_object, property_name) ||
            is_native_commandbutton_default_cancel_member_name(*runtime_object, property_name) ||
            is_native_commandbutton_style_member_name(*runtime_object, property_name) ||
            is_native_commandbutton_picture_layout_member_name(*runtime_object, property_name) ||
            is_native_visual_picture_member_name(*runtime_object, property_name) ||
            is_native_visual_dragmode_member_name(*runtime_object, property_name) ||
            is_native_visual_anchor_member_name(*runtime_object, property_name) ||
            is_native_visual_dragicon_member_name(*runtime_object, property_name) ||
            is_native_visual_downpicture_member_name(*runtime_object, property_name) ||
            is_native_visual_disabledpicture_member_name(*runtime_object, property_name) ||
            is_native_visual_autosize_member_name(*runtime_object, property_name) ||
            is_native_visual_drawmode_member_name(*runtime_object, property_name) ||
            is_native_visual_backstyle_member_name(*runtime_object, property_name) ||
            is_native_visual_specialeffect_member_name(*runtime_object, property_name) ||
            is_native_visual_alignment_member_name(*runtime_object, property_name) ||
            is_native_editbox_scrollbars_member_name(*runtime_object, property_name) ||
            is_native_textbox_inputmask_member_name(*runtime_object, property_name) ||
            is_native_textbox_dynamicinputmask_member_name(*runtime_object, property_name) ||
            is_native_column_dynamicalignment_member_name(*runtime_object, property_name) ||
            is_native_column_sparse_member_name(*runtime_object, property_name) ||
            is_native_textbox_format_member_name(*runtime_object, property_name) ||
            is_native_textbox_passwordchar_member_name(*runtime_object, property_name) ||
            is_native_textbox_maxlength_member_name(*runtime_object, property_name) ||
            is_native_textbox_specialeffect_member_name(*runtime_object, property_name) ||
            is_native_textbox_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_textbox_hideselection_member_name(*runtime_object, property_name) ||
            is_native_textbox_autocomplete_member_name(*runtime_object, property_name) ||
            is_native_textbox_enablehyperlinks_member_name(*runtime_object, property_name) ||
            is_native_textbox_tooltiptext_member_name(*runtime_object, property_name) ||
            is_native_textbox_margin_member_name(*runtime_object, property_name) ||
            is_native_textbox_mouseicon_member_name(*runtime_object, property_name) ||
            is_native_textbox_disabledbackcolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_disabledforecolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_disableditembackcolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_disableditemforecolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_itembackcolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_itemforecolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_selecteditembackcolor_member_name(*runtime_object, property_name) ||
            is_native_list_control_selecteditemforecolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_statusbartext_member_name(*runtime_object, property_name) ||
            is_native_textbox_strictdateentry_member_name(*runtime_object, property_name) ||
            is_native_textbox_themes_member_name(*runtime_object, property_name) ||
            is_native_textbox_selectedbackcolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_selectedforecolor_member_name(*runtime_object, property_name) ||
            is_native_textbox_dateformat_member_name(*runtime_object, property_name) ||
            is_native_textbox_century_member_name(*runtime_object, property_name) ||
            is_native_textbox_datemark_member_name(*runtime_object, property_name) ||
            is_native_textbox_hours_member_name(*runtime_object, property_name) ||
            is_native_textbox_seconds_member_name(*runtime_object, property_name) ||
            is_native_textbox_selection_member_name(*runtime_object, property_name) ||
            is_native_textbox_text_member_name(*runtime_object, property_name) ||
            is_native_visual_backcolor_member_name(*runtime_object, property_name) ||
            is_native_visual_forecolor_member_name(*runtime_object, property_name) ||
            is_native_tabindex_member_name(*runtime_object, property_name) ||
            is_native_tabstop_member_name(*runtime_object, property_name) ||
            is_native_visual_geometry_member_name(*runtime_object, property_name) ||
            is_native_combobox_style_member_name(*runtime_object, property_name) ||
            is_native_control_readonly_member_name(*runtime_object, property_name) ||
            is_native_string_control_value_member_name(*runtime_object, property_name) ||
            is_native_selectonentry_member_name(*runtime_object, property_name) ||
            is_native_resizable_member_name(*runtime_object, property_name) ||
            is_native_movable_member_name(*runtime_object, property_name) ||
            is_native_visual_anchor_member_name(*runtime_object, property_name) ||
            is_native_controlsource_member_name(*runtime_object, property_name) ||
            is_native_allowaddnew_member_name(*runtime_object, property_name) ||
            is_native_grid_rowheight_member_name(*runtime_object, property_name) ||
            is_native_grid_headerheight_member_name(*runtime_object, property_name) ||
            is_native_grid_allowheadersizing_member_name(*runtime_object, property_name) ||
            is_native_grid_allowrowsizing_member_name(*runtime_object, property_name) ||
            is_native_grid_allowautocolumnfit_member_name(*runtime_object, property_name) ||
            is_native_grid_gridlinecolor_member_name(*runtime_object, property_name) ||
            is_native_grid_gridlinewidth_member_name(*runtime_object, property_name) ||
            is_native_grid_highlightstyle_member_name(*runtime_object, property_name) ||
            is_native_grid_highlightrowlinewidth_member_name(*runtime_object, property_name) ||
            is_native_grid_activecolumn_member_name(*runtime_object, property_name) ||
            is_native_grid_activerow_member_name(*runtime_object, property_name) ||
            is_native_grid_relativecolumn_member_name(*runtime_object, property_name) ||
            is_native_grid_relativerow_member_name(*runtime_object, property_name) ||
            is_native_allowcellselection_member_name(*runtime_object, property_name) ||
            is_native_gridlines_member_name(*runtime_object, property_name) ||
            is_native_highlight_member_name(*runtime_object, property_name) ||
            is_native_highlightrow_member_name(*runtime_object, property_name) ||
            is_native_deletemark_member_name(*runtime_object, property_name) ||
            is_native_splitbar_member_name(*runtime_object, property_name) ||
            is_native_currentcontrol_member_name(*runtime_object, property_name) ||
            is_native_dynamiccurrentcontrol_member_name(*runtime_object, property_name) ||
            is_native_columnorder_member_name(*runtime_object, property_name) ||
            is_native_recordsource_member_name(*runtime_object, property_name) ||
            is_native_leftcolumn_member_name(*runtime_object, property_name) ||
            is_native_recordmark_member_name(*runtime_object, property_name) ||
            is_native_recordsourcetype_member_name(*runtime_object, property_name) ||
            is_native_rowsource_member_name(*runtime_object, property_name) ||
            is_native_rowsourcetype_member_name(*runtime_object, property_name) ||
            is_native_listindex_member_name(*runtime_object, property_name) ||
            is_native_displayvalue_member_name(*runtime_object, property_name) ||
            is_native_listcount_member_name(*runtime_object, property_name) ||
            is_native_sorted_member_name(*runtime_object, property_name) ||
            is_native_multiselect_member_name(*runtime_object, property_name) ||
            is_native_moverbars_member_name(*runtime_object, property_name) ||
            is_native_autohidescrollbar_member_name(*runtime_object, property_name) ||
            is_native_firstelement_member_name(*runtime_object, property_name) ||
            is_native_numberofelements_member_name(*runtime_object, property_name) ||
            is_native_displaycount_member_name(*runtime_object, property_name) ||
            is_native_nulldisplay_member_name(*runtime_object, property_name) ||
            is_native_columnlines_member_name(*runtime_object, property_name) ||
            is_native_itemtips_member_name(*runtime_object, property_name) ||
            is_native_incrementalsearch_member_name(*runtime_object, property_name) ||
            is_native_integralheight_member_name(*runtime_object, property_name) ||
            is_native_boundto_member_name(*runtime_object, property_name) ||
            is_native_newindex_member_name(*runtime_object, property_name) ||
            is_native_newitemid_member_name(*runtime_object, property_name) ||
            is_native_listitemid_member_name(*runtime_object, property_name) ||
            native_listitem_member_name_matches(*runtime_object, property_name) ||
            native_itemdata_member_name_matches(*runtime_object, property_name) ||
            is_native_topitemid_member_name(*runtime_object, property_name) ||
            is_native_topindex_member_name(*runtime_object, property_name) ||
            is_native_boundcolumn_member_name(*runtime_object, property_name) ||
            is_native_columncount_member_name(*runtime_object, property_name) ||
            is_native_column_bound_member_name(*runtime_object, property_name) ||
            is_native_columnwidths_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_creation_time_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_object_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_inspection_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_conflict_member_name(*runtime_object, property_name) ||
            native_child_parent_member_name_matches(*runtime_object, property_name) ||
            is_native_collection_member_name(*runtime_object, property_name)) {
            return make_boolean_value(false);
        }
        if (runtime_object->default_properties.contains(property_name)) {
            return make_boolean_value(false);
        }
        if (!runtime_object->properties.contains(property_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr &&
                reflectable_member_exists_locally(*object_surface, property_name)) {
                return make_boolean_value(false);
            }
        }
        const std::size_t removed = runtime_object->properties.erase(property_name);
        return make_boolean_value(removed != 0U);
    }
