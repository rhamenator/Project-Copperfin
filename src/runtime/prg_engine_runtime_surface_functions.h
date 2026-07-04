// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/runtime/prg_engine.h"

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

bool is_native_identity_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_creation_time_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_object_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_inspection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_olecontrol_conflict_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_child_parent_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_controlcount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_alwaysontop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_showwindow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_windowstate_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_desktop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_scrollbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_form_lockscreen_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_enabled_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_visible_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_visual_geometry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_allowaddnew_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_deletemark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_splitbar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_leftcolumn_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_columnorder_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_control_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool native_combobox_readonly_assignment_blocked(const RuntimeOleObjectState& runtime_object, const PrgValue& assigned_value);
void normalize_native_combobox_readonly_invariant(RuntimeOleObjectState& runtime_object);
bool is_native_string_control_value_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_name_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_collection_object(const RuntimeOleObjectState& runtime_object);
bool is_native_collection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
bool is_native_collection_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
std::optional<PrgValue> read_native_collection_member(RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name);
std::optional<PrgValue> invoke_native_collection_method(RuntimeOleObjectState& runtime_object,
                                                        const std::string& normalized_method_name,
                                                        const std::vector<PrgValue>& arguments);
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
    const std::function<RuntimeOleObjectState*(const PrgValue&)>& resolve_object_callback,
    const std::function<std::optional<PrgValue>(const PrgValue&, const std::string&)>& read_native_member_callback,
    const std::function<bool(const PrgValue&, const std::string&, const PrgValue&)>& write_native_member_callback,
    const std::function<std::optional<std::int64_t>(std::int64_t)>& whandle_from_hwnd_callback,
    const std::function<std::optional<std::int64_t>(std::int64_t)>& hwnd_from_whandle_callback,
    const std::function<void(const std::string&, std::vector<PrgValue>)>& assign_array_callback,
    const std::function<void(const std::string&, const std::string&)>& record_event_callback);

}  // namespace copperfin::runtime
