// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "prg_engine_runtime_surface_functions.h"

#include "copperfin/platform/environment.h"
#include "copperfin/platform/path.h"
#include "prg_engine_file_io_functions.h"
#include "prg_engine_helpers.h"
#include "prg_engine_locale_code_page.h"
#include "localized_text.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <iconv.h>
#include <langinfo.h>
#endif

namespace copperfin::runtime {

namespace {

#include "prg_engine_runtime_surface_platform_helpers.inl"
#include "prg_engine_runtime_surface_list_state_helpers.inl"

#include "prg_engine_runtime_surface_list_selector_helpers.inl"
#include "prg_engine_runtime_surface_reflection_helpers.inl"

#include "prg_engine_runtime_surface_cursor_helpers.inl"

}  // namespace

bool native_list_control_boundto_enabled(const RuntimeOleObjectState& runtime_object) {
    return native_list_control_boundto_enabled_impl(runtime_object);
}

bool is_native_visual_picture_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_picture_runtime_object(runtime_object);
}

bool is_native_visual_drag_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_drag_runtime_object(runtime_object);
}

bool is_native_visual_button_state_picture_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_button_state_picture_runtime_object(runtime_object);
}

bool is_native_visual_autosize_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_autosize_runtime_object(runtime_object);
}

bool is_native_visual_drawmode_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_drawmode_runtime_object(runtime_object);
}

bool is_native_visual_backstyle_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_backstyle_runtime_object(runtime_object);
}

bool is_native_visual_specialeffect_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_specialeffect_runtime_object(runtime_object);
}

bool is_native_visual_fillstyle_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_fillstyle_runtime_object(runtime_object);
}

bool is_native_visual_fillcolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_fillcolor_runtime_object(runtime_object);
}

bool is_native_visual_borderwidth_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_borderwidth_runtime_object(runtime_object);
}

bool is_native_visual_bordercolor_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_bordercolor_runtime_object(runtime_object);
}

bool is_native_visual_borderstyle_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_visual_borderstyle_runtime_object(runtime_object);
}

bool is_native_form_drawwidth_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return native_form_drawwidth_runtime_object(runtime_object);
}

void update_native_list_control_boundto_index_value_mode(
    RuntimeOleObjectState& runtime_object,
    bool was_boundto) {
    update_native_list_control_boundto_index_value_mode_impl(runtime_object, was_boundto);
}

#include "prg_engine_runtime_surface_list_api.inl"

#include "prg_engine_runtime_surface_reflection_api.inl"
#include "prg_engine_runtime_surface_object_methods.inl"

std::optional<PrgValue> evaluate_runtime_surface_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::vector<std::string>& raw_arguments,
    const std::string& default_directory,
    const std::string& frame_file_path,
    const std::string& last_error_message,
    int last_error_code,
    const std::string& last_error_procedure,
    std::size_t last_error_line,
    const std::string& error_handler,
    const std::string& shutdown_handler,
    const std::function<int(const std::string&)>& aerror_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback,
    const std::function<std::string(const std::string&)>& set_callback,
    const std::function<std::optional<RuntimeSurfaceCursorSnapshot>(const std::string&)>& snapshot_cursor_callback,
    const std::function<std::optional<std::size_t>(const RuntimeSurfaceCursorSnapshot&, const std::string&)>& load_cursor_snapshot_callback,
    bool require_verified_file_byte_overrides,
    const std::function<std::optional<std::string>(const std::filesystem::path&)>& read_verified_file_callback,
    const std::function<RuntimeOleObjectState*(const PrgValue&)>& resolve_object_callback,
    const std::function<std::optional<PrgValue>(const PrgValue&, const std::string&)>& read_native_member_callback,
    const std::function<bool(const PrgValue&, const std::string&, const PrgValue&)>& write_native_member_callback,
    const std::function<std::optional<std::int64_t>(std::int64_t)>& whandle_from_hwnd_callback,
    const std::function<std::optional<std::int64_t>(std::int64_t)>& hwnd_from_whandle_callback,
    const std::function<void(const std::string&, std::vector<PrgValue>)>& assign_array_callback,
    const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_prompt_callback,
    const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_count_callback,
    const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_position_callback,
    const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_skip_callback,
    const std::function<std::optional<PrgValue>(const std::vector<PrgValue>&)>& popup_bar_mark_callback,
    const std::function<void(const std::string&, const std::string&)>& record_event_callback) {
    auto record_runtime_warning = [&](const std::string& detail) {
        if (record_event_callback) {
            record_event_callback("runtime.warning", detail);
        }
    };

    if (const auto file_io_result = evaluate_file_io_function(
            function,
            arguments,
            default_directory,
            require_verified_file_byte_overrides,
            read_verified_file_callback,
            [&](const std::filesystem::path& path)
            {
                record_runtime_warning(runtime_text(
                    "Runtime.Prg.RuntimeSurface.Warning.FileToStrVerifiedBytesUnavailable",
                    {{"path", copperfin::platform::path_to_utf8_string(path)}}));
            })) {
        return file_io_result;
    }

    if (function == "dbc") {
        return make_string_value(set_callback("DATABASE"));
    }
    if (function == "dbused") {
        if (arguments.empty()) {
            return make_boolean_value(false);
        }
        const std::string designator = value_as_string(arguments[0]);
        return make_boolean_value(
            set_callback("__dbused__\x1f" + designator) == "1");
    }

    auto safe_int_argument = [&](std::size_t index, int default_value) {
        if (index >= arguments.size()) {
            return default_value;
        }
        const PrgValue& value = arguments[index];
        switch (value.kind) {
            case PrgValueKind::boolean:
                return value.boolean_value ? 1 : 0;
            case PrgValueKind::number:
                return static_cast<int>(std::llround(value.number_value));
            case PrgValueKind::currency:
                return static_cast<int>(std::llround(value_as_number(value)));
            case PrgValueKind::int64:
                return static_cast<int>(value.int64_value);
            case PrgValueKind::uint64:
                return static_cast<int>(value.uint64_value);
            case PrgValueKind::string:
                try {
                    return value.string_value.empty() ? default_value : static_cast<int>(std::llround(std::stod(value.string_value)));
                } catch (...) {
                    return default_value;
                }
            case PrgValueKind::empty:
                return default_value;
        }
        return default_value;
    };
    auto safe_int64_argument = [&](std::size_t index, std::int64_t default_value) {
        if (index >= arguments.size()) {
            return default_value;
        }
        const PrgValue& value = arguments[index];
        switch (value.kind) {
            case PrgValueKind::boolean:
                return value.boolean_value ? static_cast<std::int64_t>(1) : static_cast<std::int64_t>(0);
            case PrgValueKind::number:
                return static_cast<std::int64_t>(std::llround(value.number_value));
            case PrgValueKind::currency:
                return static_cast<std::int64_t>(std::llround(value_as_number(value)));
            case PrgValueKind::int64:
                return value.int64_value;
            case PrgValueKind::uint64:
                return static_cast<std::int64_t>(value.uint64_value);
            case PrgValueKind::string:
                try {
                    return static_cast<std::int64_t>(std::stoll(trim_copy(value.string_value)));
                } catch (...) {
                    return default_value;
                }
            case PrgValueKind::empty:
                return default_value;
        }
        return default_value;
    };

    const auto is_native_olecontrol_host_runtime_object = [](const RuntimeOleObjectState& runtime_object) {
        return normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
               normalize_identifier(runtime_object.prog_id) == "olecontrol";
    };
    const auto reflectable_member_exists_locally = [](const RuntimeOleObjectState& runtime_object,
                                                      const std::string& member_name) {
        return get_native_identity_reflection_metadata(runtime_object, member_name).has_value() ||
               (normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column" &&
                member_name == "header") ||
               native_controlcount_member_name_matches(runtime_object, member_name) ||
               native_pagecount_member_name_matches(runtime_object, member_name) ||
               native_activepage_member_name_matches(runtime_object, member_name) ||
               native_form_mdiform_member_name_matches(runtime_object, member_name) ||
               native_listcount_member_name_matches(runtime_object, member_name) ||
               native_listitem_member_name_matches(runtime_object, member_name) ||
               native_itemdata_member_name_matches(runtime_object, member_name) ||
               native_topitemid_member_name_matches(runtime_object, member_name) ||
               native_topindex_member_name_matches(runtime_object, member_name) ||
               native_sorted_member_name_matches(runtime_object, member_name) ||
               native_moverbars_member_name_matches(runtime_object, member_name) ||
               native_autohidescrollbar_member_name_matches(runtime_object, member_name) ||
               native_firstelement_member_name_matches(runtime_object, member_name) ||
               native_numberofelements_member_name_matches(runtime_object, member_name) ||
               native_displaycount_member_name_matches(runtime_object, member_name) ||
               native_nulldisplay_member_name_matches(runtime_object, member_name) ||
               native_columnlines_member_name_matches(runtime_object, member_name) ||
               native_itemtips_member_name_matches(runtime_object, member_name) ||
               native_incrementalsearch_member_name_matches(runtime_object, member_name) ||
               native_integralheight_member_name_matches(runtime_object, member_name) ||
               native_newindex_member_name_matches(runtime_object, member_name) ||
               native_newitemid_member_name_matches(runtime_object, member_name) ||
               native_listitemid_member_name_matches(runtime_object, member_name) ||
               native_visual_geometry_member_name_matches(runtime_object, member_name) ||
               native_controltiptext_member_name_matches(runtime_object, member_name) ||
               native_header_column_tooltiptext_member_name_matches(runtime_object, member_name) ||
               native_header_column_statusbartext_member_name_matches(runtime_object, member_name) ||
               native_header_column_mouseicon_member_name_matches(runtime_object, member_name) ||
               native_visual_helpcontextid_member_name_matches(runtime_object, member_name) ||
               native_visual_whatsthishelpid_member_name_matches(runtime_object, member_name) ||
               native_visual_tag_member_name_matches(runtime_object, member_name) ||
               native_visual_caption_member_name_matches(runtime_object, member_name) ||
               native_commandbutton_picture_layout_member_name_matches(runtime_object, member_name) ||
               native_visual_picture_member_name_matches(runtime_object, member_name) ||
               native_visual_dragmode_member_name_matches(runtime_object, member_name) ||
               native_visual_anchor_member_name_matches(runtime_object, member_name) ||
               native_visual_righttoleft_member_name_matches(runtime_object, member_name) ||
               native_visual_wordwrap_member_name_matches(runtime_object, member_name) ||
               native_visual_dragicon_member_name_matches(runtime_object, member_name) ||
               native_visual_downpicture_member_name_matches(runtime_object, member_name) ||
               native_visual_disabledpicture_member_name_matches(runtime_object, member_name) ||
               native_visual_autosize_member_name_matches(runtime_object, member_name) ||
               native_visual_backstyle_member_name_matches(runtime_object, member_name) ||
               native_visual_specialeffect_member_name_matches(runtime_object, member_name) ||
               native_form_drawstyle_member_name_matches(runtime_object, member_name) ||
               native_visual_fillstyle_member_name_matches(runtime_object, member_name) ||
               native_visual_fillcolor_member_name_matches(runtime_object, member_name) ||
               native_visual_borderwidth_member_name_matches(runtime_object, member_name) ||
               native_visual_bordercolor_member_name_matches(runtime_object, member_name) ||
               native_visual_borderstyle_member_name_matches(runtime_object, member_name) ||
               native_form_drawwidth_member_name_matches(runtime_object, member_name) ||
               native_visual_alignment_member_name_matches(runtime_object, member_name) ||
               native_grid_rowheight_member_name_matches(runtime_object, member_name) ||
               native_grid_linkmaster_member_name_matches(runtime_object, member_name) ||
               native_grid_childorder_member_name_matches(runtime_object, member_name) ||
               native_grid_relation_relationalexpr_member_name_matches(runtime_object, member_name) ||
               native_relation_onetomany_member_name_matches(runtime_object, member_name) ||
               native_grid_headerheight_member_name_matches(runtime_object, member_name) ||
               native_grid_allowheadersizing_member_name_matches(runtime_object, member_name) ||
               native_grid_allowrowsizing_member_name_matches(runtime_object, member_name) ||
               native_grid_allowautocolumnfit_member_name_matches(runtime_object, member_name) ||
               native_grid_gridlinecolor_member_name_matches(runtime_object, member_name) ||
               native_grid_gridlinewidth_member_name_matches(runtime_object, member_name) ||
               native_grid_highlightstyle_member_name_matches(runtime_object, member_name) ||
               native_grid_highlightrowlinewidth_member_name_matches(runtime_object, member_name) ||
               native_grid_view_member_name_matches(runtime_object, member_name) ||
               native_grid_activecolumn_member_name_matches(runtime_object, member_name) ||
               native_grid_activerow_member_name_matches(runtime_object, member_name) ||
               native_grid_relativecolumn_member_name_matches(runtime_object, member_name) ||
               native_grid_relativerow_member_name_matches(runtime_object, member_name) ||
               native_editbox_scrollbars_member_name_matches(runtime_object, member_name) ||
               native_textbox_inputmask_member_name_matches(runtime_object, member_name) ||
               native_textbox_dynamicinputmask_member_name_matches(runtime_object, member_name) ||
               native_column_dynamicalignment_member_name_matches(runtime_object, member_name) ||
               native_column_sparse_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontname_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontbold_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontitalic_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontstrikethru_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontunderline_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontsize_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontshadow_member_name_matches(runtime_object, member_name) ||
               native_visual_dynamicfontoutline_member_name_matches(runtime_object, member_name) ||
               native_column_dynamicbackcolor_member_name_matches(runtime_object, member_name) ||
               native_column_dynamicforecolor_member_name_matches(runtime_object, member_name) ||
               native_textbox_format_member_name_matches(runtime_object, member_name) ||
               native_textbox_passwordchar_member_name_matches(runtime_object, member_name) ||
               native_textbox_maxlength_member_name_matches(runtime_object, member_name) ||
               native_textbox_specialeffect_member_name_matches(runtime_object, member_name) ||
               native_textbox_borderstyle_member_name_matches(runtime_object, member_name) ||
               native_textbox_hideselection_member_name_matches(runtime_object, member_name) ||
               native_textbox_autocomplete_member_name_matches(runtime_object, member_name) ||
               native_textbox_enablehyperlinks_member_name_matches(runtime_object, member_name) ||
               native_textbox_tooltiptext_member_name_matches(runtime_object, member_name) ||
               native_textbox_margin_member_name_matches(runtime_object, member_name) ||
               native_textbox_mouseicon_member_name_matches(runtime_object, member_name) ||
               native_textbox_disabledbackcolor_member_name_matches(runtime_object, member_name) ||
               native_textbox_disabledforecolor_member_name_matches(runtime_object, member_name) ||
               native_list_control_disableditembackcolor_member_name_matches(runtime_object, member_name) ||
               native_list_control_disableditemforecolor_member_name_matches(runtime_object, member_name) ||
               native_list_control_itembackcolor_member_name_matches(runtime_object, member_name) ||
               native_list_control_itemforecolor_member_name_matches(runtime_object, member_name) ||
               native_list_control_selecteditembackcolor_member_name_matches(runtime_object, member_name) ||
               native_list_control_selecteditemforecolor_member_name_matches(runtime_object, member_name) ||
               native_textbox_statusbartext_member_name_matches(runtime_object, member_name) ||
               native_textbox_strictdateentry_member_name_matches(runtime_object, member_name) ||
               native_textbox_themes_member_name_matches(runtime_object, member_name) ||
               native_textbox_selectedbackcolor_member_name_matches(runtime_object, member_name) ||
               native_textbox_selectedforecolor_member_name_matches(runtime_object, member_name) ||
               native_textbox_dateformat_member_name_matches(runtime_object, member_name) ||
               native_textbox_century_member_name_matches(runtime_object, member_name) ||
               native_textbox_datemark_member_name_matches(runtime_object, member_name) ||
               native_textbox_hours_member_name_matches(runtime_object, member_name) ||
               native_textbox_seconds_member_name_matches(runtime_object, member_name) ||
               native_textbox_selection_member_name_matches(runtime_object, member_name) ||
               native_textbox_text_member_name_matches(runtime_object, member_name) ||
               native_selectonentry_member_name_matches(runtime_object, member_name) ||
               native_resizable_member_name_matches(runtime_object, member_name) ||
               is_native_collection_member_name(runtime_object, member_name) ||
               runtime_object.properties.contains(member_name) ||
               object_has_accessor_property(runtime_object, member_name) ||
               is_builtin_native_runtime_method_name(runtime_object, member_name) ||
               object_has_member(runtime_object.methods, member_name) ||
               object_has_member(runtime_object.events, member_name);
    };
    const auto reflectable_member_readonly_locally = [](const RuntimeOleObjectState& runtime_object,
                                                        const std::string& member_name) {
        return get_native_identity_reflection_metadata(runtime_object, member_name).has_value() ||
               native_controlcount_member_name_matches(runtime_object, member_name) ||
               native_listcount_member_name_matches(runtime_object, member_name) ||
               (native_topitemid_member_name_matches(runtime_object, member_name) &&
                normalize_identifier(trim_copy(runtime_object.base_class_name)) == "combobox") ||
               (native_topindex_member_name_matches(runtime_object, member_name) &&
                normalize_identifier(trim_copy(runtime_object.base_class_name)) == "combobox") ||
               native_integralheight_member_name_matches(runtime_object, member_name) ||
               native_textbox_text_member_name_matches(runtime_object, member_name) ||
               native_newindex_member_name_matches(runtime_object, member_name) ||
               native_newitemid_member_name_matches(runtime_object, member_name) ||
               native_child_collection_member_name_matches(runtime_object, member_name) ||
               is_native_splitbar_member_name(runtime_object, member_name) ||
               is_native_leftcolumn_member_name(runtime_object, member_name) ||
               is_native_grid_childorder_member_name(runtime_object, member_name) ||
               is_native_relation_onetomany_member_name(runtime_object, member_name) ||
               is_native_grid_activecolumn_member_name(runtime_object, member_name) ||
               is_native_grid_activerow_member_name(runtime_object, member_name) ||
               is_native_grid_relativecolumn_member_name(runtime_object, member_name) ||
               is_native_grid_relativerow_member_name(runtime_object, member_name) ||
               is_native_form_desktop_member_name(runtime_object, member_name) ||
               is_native_form_scrollbars_member_name(runtime_object, member_name) ||
               is_native_movable_member_name(runtime_object, member_name) ||
               is_native_olecontrol_creation_time_member_name(runtime_object, member_name) ||
               is_native_olecontrol_object_member_name(runtime_object, member_name) ||
               is_native_olecontrol_inspection_member_name(runtime_object, member_name) ||
               is_native_olecontrol_conflict_member_name(runtime_object, member_name) ||
               native_name_member_name_matches(runtime_object, member_name) ||
               native_child_parent_member_name_matches(runtime_object, member_name) ||
               is_native_collection_readonly_member_name(runtime_object, member_name) ||
               (is_scripting_dictionary_object(runtime_object) && member_name == "count") ||
               (object_has_accessor_property(runtime_object, member_name) &&
                !object_has_assigner_property(runtime_object, member_name));
    };
    const auto resolve_direct_olecontrol_reflection_surface =
        [&](RuntimeOleObjectState& runtime_object) -> RuntimeOleObjectState*
    {
        if (!resolve_object_callback || !is_native_olecontrol_host_runtime_object(runtime_object)) {
            return nullptr;
        }
        const auto object_surface = runtime_object.properties.find("object");
        if (object_surface == runtime_object.properties.end()) {
            return nullptr;
        }
        RuntimeOleObjectState* nested_object = resolve_object_callback(object_surface->second);
        if (nested_object == nullptr || nested_object == &runtime_object) {
            return nullptr;
        }
        return nested_object;
    };
    const auto merge_member_tokens = [](const std::vector<std::string>& primary,
                                        const std::vector<std::string>& secondary) {
        std::vector<std::string> merged = primary;
        merged.reserve(primary.size() + secondary.size());
        merged.insert(merged.end(), secondary.begin(), secondary.end());
        std::sort(merged.begin(), merged.end(), [](const std::string& left, const std::string& right) {
            const std::string normalized_left = lowercase_copy(left);
            const std::string normalized_right = lowercase_copy(right);
            if (normalized_left == normalized_right) {
                return left < right;
            }
            return normalized_left < normalized_right;
        });
        merged.erase(std::unique(merged.begin(), merged.end(), [](const std::string& left, const std::string& right) {
            return lowercase_copy(left) == lowercase_copy(right);
        }), merged.end());
        return merged;
    };

#include "prg_engine_runtime_surface_dispatch_object.inl"
#include "prg_engine_runtime_surface_dispatch_general.inl"
    return std::nullopt;
}

}  // namespace copperfin::runtime
