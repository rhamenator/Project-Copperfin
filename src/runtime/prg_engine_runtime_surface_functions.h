// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

struct RuntimeSurfaceCursorField {
    std::string name;
    char type = 'C';
    std::size_t width = 0;
    std::size_t decimals = 0;
};

struct RuntimeSurfaceCursorRow {
    std::vector<std::string> values;
};

struct RuntimeSurfaceCursorSnapshot {
    std::string alias;
    std::optional<int> code_page;
    std::vector<RuntimeSurfaceCursorField> fields;
    std::vector<RuntimeSurfaceCursorRow> rows;
};

struct NativeListControlCellReference {
    std::size_t row_slot = 0U;
    std::size_t column_slot = 0U;
};

struct NativeListControlItemCellReference {
    long long item_id = 0LL;
    std::size_t column_slot = 0U;
};

using RuntimeControlSourceValueResolver =
    std::function<std::optional<PrgValue>(const std::string&)>;

bool is_native_identity_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_creation_time_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_object_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_inspection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_conflict_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_child_parent_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_controlcount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_pagecount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_activepage_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_alwaysontop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_showwindow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_windowstate_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_desktop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_scrollbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_lockscreen_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_enabled_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_visible_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_controltiptext_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_tag_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_caption_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_alignment_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_editbox_scrollbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_inputmask_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_format_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_passwordchar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_maxlength_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_specialeffect_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_borderstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_hideselection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_autocomplete_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_enablehyperlinks_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_tooltiptext_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_margin_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_mouseicon_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_disabledbackcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_disabledforecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_statusbartext_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_strictdateentry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_themes_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_selectedbackcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_selectedforecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_dateformat_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_century_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_datemark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_hours_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_seconds_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_selection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_textbox_text_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_geometry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_tabindex_runtime_object(const RuntimeOleObjectState& runtime_object);
bool is_native_tabindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_tabstop_runtime_object(const RuntimeOleObjectState& runtime_object);
bool is_native_tabstop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_allowaddnew_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_grid_rowheight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_grid_headerheight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_grid_allowheadersizing_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_grid_allowrowsizing_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_deletemark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_splitbar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_leftcolumn_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_columnorder_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_columncount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_listcount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_sorted_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_multiselect_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_moverbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_autohidescrollbar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_firstelement_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_numberofelements_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_displaycount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_nulldisplay_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_columnlines_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_itemtips_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_incrementalsearch_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_integralheight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_boundto_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_newindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_newitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_listitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_itemdata_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_topitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_topindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_column_bound_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_child_collection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_control_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool native_listbox_moverbars_row_source_supported(const RuntimeOleObjectState& runtime_object);
bool native_combobox_readonly_assignment_blocked(const RuntimeOleObjectState& runtime_object, const PrgValue& assigned_value);
void normalize_native_pageframe_activepage_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_combobox_readonly_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_list_control_sorted_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_listbox_multiselect_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_listbox_moverbars_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_listbox_autohidescrollbar_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_list_control_array_range_invariants(RuntimeOleObjectState& runtime_object);
void normalize_native_combobox_displaycount_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_list_control_nulldisplay_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_list_control_columnlines_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_list_control_itemtips_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_list_control_incrementalsearch_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_visual_alignment_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_grid_rowheight_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_grid_headerheight_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_grid_allowheadersizing_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_grid_allowrowsizing_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_editbox_scrollbars_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_inputmask_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_format_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_passwordchar_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_maxlength_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_specialeffect_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_borderstyle_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_hideselection_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_autocomplete_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_enablehyperlinks_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_tooltiptext_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_margin_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_mouseicon_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_disabledbackcolor_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_disabledforecolor_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_statusbartext_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_strictdateentry_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_themes_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_selectedbackcolor_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_selectedforecolor_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_dateformat_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_century_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_datemark_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_hours_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_seconds_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_selection_invariant(RuntimeOleObjectState& runtime_object);
void normalize_native_textbox_text_invariant(RuntimeOleObjectState& runtime_object);
bool write_native_textbox_selection_property(RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name, const PrgValue& assigned_value);
bool is_native_string_control_value_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_selectonentry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_resizable_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_controlsource_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_name_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_collection_object(const RuntimeOleObjectState& runtime_object);
bool is_native_collection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_collection_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
std::optional<PrgValue> read_native_collection_member(RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
std::optional<PrgValue> invoke_native_collection_method(RuntimeOleObjectState& runtime_object,
                                                        const std::string& normalized_method_name,
                                                        const std::vector<PrgValue>& arguments);
std::optional<PrgValue> invoke_native_list_control_method(RuntimeOleObjectState& runtime_object,
                                                          const std::string& normalized_method_name,
                                                          const std::vector<PrgValue>& arguments,
                                                          const std::function<bool(const std::vector<PrgValue>&)>& before_move = {});
std::optional<NativeListControlCellReference> parse_native_list_control_list_member_cell(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name);
std::optional<NativeListControlItemCellReference> parse_native_list_control_listitem_member_cell(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name);
std::optional<std::size_t> parse_native_list_control_itemdata_member_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name);
std::optional<std::size_t> parse_native_list_control_indextoitemid_member_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name);
std::optional<long long> parse_native_list_control_itemidtoindex_member_item_id(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name);
std::optional<PrgValue> read_native_list_control_cell(RuntimeOleObjectState& runtime_object,
                                                      std::size_t row_slot,
                                                      std::size_t column_slot);
std::optional<PrgValue> read_native_list_control_item_cell(
    RuntimeOleObjectState& runtime_object,
    long long item_id,
    std::size_t column_slot);
std::optional<PrgValue> read_native_list_control_item_id_for_slot(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot);
std::optional<PrgValue> read_native_list_control_item_data(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot);
void sync_native_list_control_top_item_id(RuntimeOleObjectState& runtime_object);
std::optional<PrgValue> read_native_list_control_index_for_item_id(
    RuntimeOleObjectState& runtime_object,
    long long item_id);
bool write_native_list_control_cell(RuntimeOleObjectState& runtime_object,
                                    std::size_t row_slot,
                                    std::size_t column_slot,
                                    const PrgValue& assigned_value);
bool write_native_list_control_item_cell(RuntimeOleObjectState& runtime_object,
                                         long long item_id,
                                         std::size_t column_slot,
                                         const PrgValue& assigned_value);
bool write_native_list_control_item_data(RuntimeOleObjectState& runtime_object,
                                         std::size_t row_slot,
                                         const PrgValue& assigned_value);
bool write_native_list_control_top_item_id(RuntimeOleObjectState& runtime_object,
                                           const PrgValue& assigned_value);
bool write_native_list_control_top_index(RuntimeOleObjectState& runtime_object,
                                         const PrgValue& assigned_value);
std::optional<std::size_t> parse_native_list_control_selected_member_slot(const RuntimeOleObjectState& runtime_object,
                                                                          const std::string& member_name);
std::optional<long long> parse_native_list_control_selectedid_member_item_id(const RuntimeOleObjectState& runtime_object,
                                                                             const std::string& member_name);
bool write_native_list_control_item_id(RuntimeOleObjectState& runtime_object, const PrgValue& assigned_value);
bool write_native_list_control_selected_slot(RuntimeOleObjectState& runtime_object,
                                             std::size_t slot,
                                             const PrgValue& assigned_value);
bool write_native_list_control_selected_item_id(RuntimeOleObjectState& runtime_object,
                                                long long requested_item_id,
                                                const PrgValue& assigned_value);
bool write_native_list_control_value(RuntimeOleObjectState& runtime_object,
                                     const PrgValue& assigned_value);
void refresh_native_list_control_controlsource_value_kind_hint(
    RuntimeOleObjectState& runtime_object,
    const RuntimeControlSourceValueResolver& resolver);
void sync_native_list_control_count(RuntimeOleObjectState& runtime_object);
void sync_native_list_control_displayvalue_from_selection(RuntimeOleObjectState& runtime_object);
std::optional<std::string> native_list_control_selection_signature(RuntimeOleObjectState& runtime_object);
std::optional<PrgValue> read_native_identity_metadata(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);

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
    const std::function<void(const std::string&, const std::string&)>& record_event_callback);

}  // namespace copperfin::runtime
