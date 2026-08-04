// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#ifndef COPPERFIN_VISUAL_ASSET_EDITOR_SUPPORT_H
#define COPPERFIN_VISUAL_ASSET_EDITOR_SUPPORT_H

#include "copperfin/vfp/visual_asset_editor.h"

#include "copperfin/localization/localization.h"
#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/sidecar_path.h"

#include <array>
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <optional>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>

namespace copperfin::vfp {

struct RawFieldDescriptor {
    std::string name;
    char type = '\0';
    std::uint32_t offset = 0;
    std::uint8_t length = 0;
};

struct VisualObjectGeometry {
    double hpos = 0.0;
    double vpos = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct VisualAssetUndoEntry {
    std::size_t record_index = 0;
    std::string property_name;
    std::string prior_value;
    bool prior_value_exists = false;
    std::string label;
    std::vector<VisualAssetUndoEntry> grouped_changes;
};

// Reserved property_name value marking an undo entry as a record deleted-flag
// change rather than a property-value change. '#' can never appear in a real
// VFP property name, so this cannot collide with a legitimate entry. The
// prior flag is carried in prior_value as "1"/"0" (prior_value_exists is
// always true for this entry kind).
inline const std::string kVisualAssetDeletedStateUndoPropertyName = "#RecordDeletedState";

struct VisualPropertyState {
    bool exists = false;
    bool direct_field = false;
    std::string property_name;
    std::string value;
    bool record_deleted = false;
};

struct VisualAssetRawRecordAppend {
    std::optional<std::size_t> source_record_index;
    std::vector<VisualObjectPropertyChange> field_values;
};

// ==== Shared low-level helpers (byte I/O, text/name normalization, localized text) ====
copperfin::localization::LocalizedCatalog visual_asset_editor_catalog();
std::string visual_asset_text(std::string_view key);
std::string visual_asset_text(
    std::string_view key,
    const copperfin::localization::PlaceholderMap& placeholders);
std::string visual_asset_property_non_negative_text(std::string property_name);
std::string visual_asset_rollback_failed_text(std::string error, std::string rollback_error);
std::string visual_asset_target_rollback_failed_text(std::string error, std::string rollback_error);
std::uint32_t read_le_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset);
std::uint32_t read_be_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset);
std::uint16_t read_be_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset);
void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
std::string trim_right(std::string text);
std::string trim_both(std::string text);
std::string read_ascii_name(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t length);
std::vector<std::uint8_t> read_binary_file(const std::string& path);
bool write_binary_file(const std::string& path, const std::vector<std::uint8_t>& bytes);
VisualAssetEditResult recover_visual_asset_table_transaction(const std::string& table_path);
VisualAssetEditResult write_visual_asset_table_transaction(
    const std::string& table_path,
    const std::vector<std::uint8_t>& table_bytes);
VisualAssetEditResult recover_visual_asset_file_transaction(
    const std::string& table_path,
    const std::string& memo_path);
VisualAssetEditResult write_visual_asset_file_transaction(
    const std::string& table_path,
    const std::vector<std::uint8_t>& table_bytes,
    const std::string& memo_path,
    const std::vector<std::uint8_t>& memo_bytes);
VisualAssetEditResult resolve_visual_asset_storage_memo_path(
    const std::string& table_path,
    std::string& memo_path);
VisualAssetEditResult recover_visual_asset_storage_transaction(const std::string& table_path);
SidecarPathResolution infer_memo_sidecar_path(const std::string& path);
std::string selected_memo_sidecar_path(const SidecarPathResolution& resolution);
std::string ambiguous_memo_sidecar_error(const SidecarPathResolution& resolution);
std::string normalize_visual_object_name(std::string value);
std::string format_visual_string_property_value(const std::string& value);
std::string normalize_visual_property_name(std::string value);
std::string lowercase_copy(std::string_view value);
bool contains_case_insensitive(std::string_view value, const std::string& lowered_needle);
std::string direct_field_descriptor_prefix(const std::string& normalized_property_name);
const DbfRecordValue* find_direct_visual_property_value(
    const std::vector<DbfRecordValue>& values,
    const std::string& property_name);
std::vector<RawFieldDescriptor>::const_iterator find_direct_visual_property_field(
    const std::vector<RawFieldDescriptor>& fields,
    const std::string& property_name);
bool starts_with_insensitive(const std::string& text, const std::string& prefix);

// ==== Method (event-code) blob parsing, serialization, and editing ====
std::vector<std::string> split_visual_lines(const std::string& text);
std::vector<VisualObjectMethodSnapshot> parse_visual_methods_blob(
    const std::string& text,
    std::uint32_t source_memo_block_number);
std::vector<std::string> split_replacement_source_lines(const std::string& source_text);
bool is_visual_method_end_line(const std::string& line);
bool parse_visual_method_declaration(const std::string& line, std::string& kind, std::string& method_name);
std::string serialize_visual_lines(const std::vector<std::string>& lines);
std::string update_visual_methods_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name,
    const std::string& requested_kind,
    const std::string& replacement_source);
std::string serialize_visual_methods(const std::vector<VisualObjectMethodSnapshot>& methods);
VisualAssetEditResult find_unique_visual_method_index(
    const std::vector<VisualObjectMethodSnapshot>& methods,
    const std::string& method_name,
    const std::string& missing_error,
    const std::string& ambiguous_error,
    std::size_t& method_index);
VisualAssetEditResult reorder_visual_methods_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name,
    const std::string& placement,
    const std::string& relative_method_name,
    std::string& updated_blob);
std::pair<bool, std::string> delete_visual_method_from_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name);
VisualAssetEditResult rename_visual_method_in_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name,
    const std::string& new_method_name,
    std::string& updated_blob);

// ==== Property-assignment blob parsing and generic property CRUD ====
VisualAssetEditResult find_unique_visual_property_assignment_index(
    const std::vector<VisualPropertyAssignment>& assignments,
    const std::string& property_name,
    const std::string& missing_error,
    const std::string& ambiguous_error,
    std::size_t& property_index);
VisualAssetEditResult reorder_visual_property_assignments(
    std::vector<VisualPropertyAssignment>& assignments,
    const std::string& requested_property_name,
    const std::string& placement,
    const std::string& relative_property_name);
std::optional<VisualPropertyState> read_current_visual_property_state(
    const std::string& path,
    std::size_t record_index,
    const std::string& property_name);
VisualAssetEditResult apply_visual_object_property_change(
    const VisualObjectEditRequest& request,
    bool record_undo_entry,
    bool remove_property_if_missing);
VisualAssetEditResult expand_report_section_top_batch_updates(
    const std::string& path,
    const std::vector<VisualObjectBatchEditItem>& objects,
    std::vector<VisualObjectBatchEditItem>& expanded_objects);
VisualAssetEditResult set_visual_object_text_property(
    const std::string& path,
    const std::vector<VisualObjectAlignmentTarget>& objects,
    const std::string& property_name,
    const std::string& property_label,
    const std::string& text);
VisualAssetEditResult set_visual_object_scalar_property(
    const std::string& path,
    const std::vector<VisualObjectAlignmentTarget>& objects,
    const std::string& property_name,
    const std::string& property_label,
    const std::string& property_value);
std::vector<VisualPropertyAssignment> parse_visual_property_blob(const std::string& text);
std::string serialize_visual_property_blob(const std::vector<VisualPropertyAssignment>& properties);
bool is_property_blob_asset_path(const std::string& path);
VisualAssetEditResult update_visual_object_property(const VisualObjectEditRequest& request);
VisualAssetEditResult clear_visual_object_property(const VisualObjectPropertyClearRequest& request);
VisualAssetEditResult clear_visual_object_properties(const VisualObjectPropertyClearBatchRequest& request);
VisualAssetEditResult copy_visual_object_property(const VisualObjectPropertyCopyRequest& request);
VisualAssetEditResult copy_visual_object_properties(const VisualObjectPropertyCopyBatchRequest& request);
VisualAssetEditResult move_visual_object_property(const VisualObjectPropertyMoveRequest& request);
VisualAssetEditResult move_visual_object_properties(const VisualObjectPropertyMoveBatchRequest& request);
VisualAssetEditResult rename_visual_object_property(const VisualObjectPropertyRenameRequest& request);
VisualAssetEditResult rename_visual_object_properties(const VisualObjectPropertyRenameBatchRequest& request);
VisualAssetEditResult reorder_visual_object_property(const VisualObjectPropertyReorderRequest& request);
VisualAssetEditResult reorder_visual_object_properties(const VisualObjectPropertyReorderBatchRequest& request);
VisualObjectPropertyQueryResult query_visual_object_property(const VisualObjectPropertyQueryRequest& request);
VisualObjectPropertyListResult list_visual_object_properties(const VisualObjectPropertyListRequest& request);
bool matches_property_filter(const VisualObjectPropertySnapshot& property, const std::string& lowered_search_text);
VisualObjectPropertyListFilterResult filter_visual_object_properties(
    const VisualObjectPropertyListFilterRequest& request);

// ==== DBF record lookup, identity/duplicate checks, snapshot + raw field I/O ====
const DbfRecordValue* find_record_value(const DbfRecord& record, const std::string& field_name);
std::optional<std::size_t> find_field_index(const DbfTable& table, const std::string& field_name);
std::vector<std::size_t> find_matching_record_indexes(
    const DbfTable& table,
    const std::string& field_name,
    const std::string& requested_value);
VisualObjectDuplicateResult failed_visual_object_duplicate_result(std::string error);
VisualObjectDuplicateBatchResult failed_visual_object_duplicate_batch_result(std::string error);
VisualObjectDuplicateResult reject_identity_collision(
    const DbfTable& table,
    const std::string& field_name,
    const std::string& requested_value);
VisualAssetEditResult reject_identity_collision_excluding_record(
    const DbfTable& table,
    const std::string& field_name,
    const std::string& requested_value,
    std::size_t excluded_record_index);
void replace_duplicate_field_value(
    const DbfTable& table,
    std::vector<std::string>& values,
    const std::string& field_name,
    const std::string& replacement_value);
std::string duplicate_field_value(
    const DbfTable& table,
    const std::vector<std::string>& values,
    const std::string& field_name);
const VisualObjectSubtreeDuplicateReplacement* find_subtree_duplicate_replacement(
    const std::vector<VisualObjectSubtreeDuplicateReplacement>& replacements,
    const std::string& source_unique_id);
std::string visual_object_record_name(const DbfRecord& record);
VisualObjectCreatedObject created_visual_object_from_record(const DbfRecord& record, std::size_t record_index);
std::optional<double> parse_visual_geometry_number(const std::string& text);
std::string format_visual_geometry_number(double value);
VisualAssetEditResult read_visual_object_geometry(
    const std::string& path,
    std::size_t record_index,
    const std::string& object_name,
    const std::string& unique_id,
    VisualObjectGeometry& geometry);
VisualAssetEditResult read_visual_object_geometry_coordinate(
    const std::string& path,
    const VisualObjectAlignmentTarget& object,
    const std::string& property_name,
    double& coordinate);
VisualObjectSnapshot build_visual_object_snapshot(const DbfRecord& record);
const DbfRecord* find_visual_object_record_by_record_index(
    const DbfTable& table,
    std::size_t record_index);
const DbfRecord* find_visual_object_record_by_name(
    const DbfTable& table,
    const std::string& object_name);
void enrich_visual_object_hierarchy_snapshot(
    VisualObjectSnapshot& snapshot,
    const DbfTable& table);
VisualObjectSnapshot build_visual_object_snapshot(const DbfRecord& record, const DbfTable& table);
VisualAssetEditResult resolve_visual_object_record_index(const VisualObjectEditRequest& request, std::size_t& record_index);
VisualAssetEditResult resolve_visual_object_record_index_from_records(
    const std::vector<DbfRecord>& records,
    std::size_t requested_record_index,
    const std::string& object_name,
    const std::string& unique_id,
    std::size_t& record_index);
VisualAssetEditResult apply_visual_object_reorder_to_records(
    std::vector<DbfRecord>& records,
    const VisualObjectReorderBatchItem& request);
std::vector<std::vector<std::string>> visual_record_values_for_write(
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<DbfRecord>& records);
VisualAssetEditResult append_visual_asset_records_preserving_raw(
    const std::string& path,
    const std::vector<VisualAssetRawRecordAppend>& appends);
VisualAssetEditResult reorder_visual_asset_records_preserving_raw(
    const std::string& path,
    const std::vector<std::size_t>& record_order);
std::optional<char> normalize_logical_value(std::string value);
VisualAssetEditResult replace_non_memo_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& new_value);
VisualAssetEditResult replace_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& new_value,
    bool raw_memo_value = false);
std::vector<RawFieldDescriptor> read_raw_field_descriptors(const std::vector<std::uint8_t>& table_bytes);
VisualAssetEditResult replace_memo_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& new_value,
    bool raw_value = false);

// ==== Undo-entry subsystem ====
std::filesystem::path visual_asset_undo_root_directory(const std::string& path);
std::filesystem::path visual_asset_undo_entries_directory(const std::string& path);
std::vector<std::filesystem::path> list_visual_asset_undo_entry_files(const std::string& path);
bool discard_visual_asset_undo_entries_after_depth(
    const std::string& path,
    std::size_t retained_depth,
    std::string& error);
bool write_visual_asset_undo_entry(const std::filesystem::path& path, const VisualAssetUndoEntry& entry);
std::optional<VisualAssetUndoEntry> read_visual_asset_undo_entry(const std::filesystem::path& path);
bool record_visual_asset_undo_entry(const std::string& path, const VisualAssetUndoEntry& entry, std::string& error);
VisualAssetUndoStatus query_visual_asset_undo_status_internal(const std::string& path);
VisualAssetUndoStatus query_visual_object_undo(const std::string& path);
VisualAssetEditResult undo_visual_object_property(const std::string& path);

// ==== Object listing, method high-level API, duplicate/create/group/layout, structural edits ====
VisualObjectListResult list_visual_objects(const std::string& path);
VisualObjectChildrenListResult list_visual_object_children(const VisualObjectChildrenListRequest& request);
VisualObjectDescendantsListResult list_visual_object_descendants(const VisualObjectDescendantsListRequest& request);
VisualObjectAncestorsListResult list_visual_object_ancestors(const VisualObjectAncestorsListRequest& request);
VisualObjectMethodListResult list_visual_object_methods(const VisualObjectMethodListRequest& request);
VisualObjectMethodQueryResult query_visual_object_method(const VisualObjectMethodQueryRequest& request);
VisualAssetEditResult update_visual_object_method(const VisualObjectMethodEditRequest& request);
VisualAssetEditResult delete_visual_object_method(const VisualObjectMethodDeleteRequest& request);
VisualAssetEditResult delete_visual_object_methods(const VisualObjectMethodDeleteBatchRequest& request);
VisualAssetEditResult rename_visual_object_method(const VisualObjectMethodRenameRequest& request);
VisualAssetEditResult rename_visual_object_methods(const VisualObjectMethodRenameBatchRequest& request);
VisualAssetEditResult copy_visual_object_method(const VisualObjectMethodCopyRequest& request);
VisualAssetEditResult copy_visual_object_methods(const VisualObjectMethodCopyBatchRequest& request);
VisualAssetEditResult move_visual_object_method(const VisualObjectMethodMoveRequest& request);
VisualAssetEditResult move_visual_object_methods(const VisualObjectMethodMoveBatchRequest& request);
VisualAssetEditResult reorder_visual_object_method(const VisualObjectMethodReorderRequest& request);
VisualAssetEditResult reorder_visual_object_methods(const VisualObjectMethodReorderBatchRequest& request);
VisualObjectDuplicateResult duplicate_visual_object(const VisualObjectDuplicateRequest& request);
VisualObjectDuplicateBatchResult duplicate_visual_objects(const VisualObjectDuplicateBatchRequest& request);
VisualObjectSubtreeDuplicateResult failed_visual_object_subtree_duplicate_result(std::string error);
VisualObjectSubtreeDuplicateResult empty_visual_object_subtree_duplicate_result();
VisualObjectSubtreeDuplicateResult duplicate_visual_object_subtree(const VisualObjectSubtreeDuplicateRequest& request);
VisualObjectCreateResult failed_visual_object_create_result(std::string error);
VisualObjectCreateBatchResult failed_visual_object_create_batch_result(std::string error);
VisualObjectCreateResult create_visual_object(const VisualObjectCreateRequest& request);
VisualObjectCreateBatchResult create_visual_objects(const VisualObjectCreateBatchRequest& request);
VisualAssetEditResult align_visual_objects(const VisualObjectAlignmentRequest& request);
VisualAssetEditResult resize_visual_objects(const VisualObjectResizeRequest& request);
VisualObjectGroupResult failed_visual_object_group_result(std::string error);
VisualObjectGroupResult group_visual_objects(const VisualObjectGroupRequest& request);
VisualObjectUngroupResult failed_visual_object_ungroup_result(std::string error);
VisualObjectUngroupResult ungroup_visual_object(const VisualObjectUngroupRequest& request);
VisualAssetEditResult distribute_visual_objects(const VisualObjectDistributeRequest& request);
VisualAssetEditResult snap_visual_objects_to_grid(const VisualObjectSnapToGridRequest& request);
VisualAssetEditResult nudge_visual_objects(const VisualObjectNudgeRequest& request);
VisualAssetEditResult reparent_visual_object(const VisualObjectReparentRequest& request);
VisualAssetEditResult reparent_visual_objects(const VisualObjectReparentBatchRequest& request);
VisualAssetEditResult rename_visual_object(const VisualObjectRenameRequest& request);
VisualAssetEditResult rename_visual_objects(const VisualObjectRenameBatchRequest& request);
VisualAssetEditResult reorder_visual_object(const VisualObjectReorderRequest& request);
VisualAssetEditResult reorder_visual_objects(const VisualObjectReorderBatchRequest& request);
VisualAssetEditResult set_visual_object_deleted_state(const VisualObjectDeletedStateRequest& request);
VisualAssetEditResult set_visual_object_deleted_states(const VisualObjectDeletedStateBatchRequest& request);
VisualAssetEditResult set_visual_object_subtree_deleted_state(const VisualObjectSubtreeDeletedStateRequest& request);
VisualAssetEditResult update_visual_object_properties(const VisualObjectMultiEditRequest& request);
VisualAssetEditResult update_visual_object_batch(const VisualObjectBatchEditRequest& request);

// ==== Per-property setters: behavior/state flags ====
VisualAssetEditResult set_visual_object_tab_order(const VisualObjectTabOrderRequest& request);
VisualAssetEditResult set_visual_object_tab_stop(const VisualObjectTabStopRequest& request);
VisualAssetEditResult set_visual_object_visibility(const VisualObjectVisibilityRequest& request);
VisualAssetEditResult set_visual_object_enabled(const VisualObjectEnabledRequest& request);
VisualAssetEditResult set_visual_object_read_only(const VisualObjectReadOnlyRequest& request);
VisualAssetEditResult set_visual_object_locked(const VisualObjectLockedRequest& request);
VisualAssetEditResult set_visual_object_caption(const VisualObjectCaptionRequest& request);
VisualAssetEditResult set_visual_object_tooltip_text(const VisualObjectToolTipTextRequest& request);
VisualAssetEditResult set_visual_object_status_bar_text(const VisualObjectStatusBarTextRequest& request);
VisualAssetEditResult set_visual_object_control_source(const VisualObjectControlSourceRequest& request);
VisualAssetEditResult set_visual_object_current_control(const VisualObjectCurrentControlRequest& request);
VisualAssetEditResult set_visual_object_closable(const VisualObjectClosableRequest& request);
VisualAssetEditResult set_visual_object_control_box(const VisualObjectControlBoxRequest& request);
VisualAssetEditResult set_visual_object_allow_output(const VisualObjectAllowOutputRequest& request);
VisualAssetEditResult set_visual_object_auto_center(const VisualObjectAutoCenterRequest& request);
VisualAssetEditResult set_visual_object_auto_size(const VisualObjectAutoSizeRequest& request);
VisualAssetEditResult set_visual_object_auto_release(const VisualObjectAutoReleaseRequest& request);
VisualAssetEditResult set_visual_object_auto_verb_menu(const VisualObjectAutoVerbMenuRequest& request);
VisualAssetEditResult set_visual_object_bind_controls(const VisualObjectBindControlsRequest& request);
VisualAssetEditResult set_visual_object_clip_controls(const VisualObjectClipControlsRequest& request);
VisualAssetEditResult set_visual_object_dockable(const VisualObjectDockableRequest& request);
VisualAssetEditResult set_visual_object_continuous_scroll(const VisualObjectContinuousScrollRequest& request);
VisualAssetEditResult set_visual_object_desktop(const VisualObjectDesktopRequest& request);
VisualAssetEditResult set_visual_object_key_preview(const VisualObjectKeyPreviewRequest& request);
VisualAssetEditResult set_visual_object_mac_desktop(const VisualObjectMacDesktopRequest& request);
VisualAssetEditResult set_visual_object_max_button(const VisualObjectMaxButtonRequest& request);
VisualAssetEditResult set_visual_object_max_height(const VisualObjectMaxHeightRequest& request);
VisualAssetEditResult set_visual_object_max_width(const VisualObjectMaxWidthRequest& request);
VisualAssetEditResult set_visual_object_max_left(const VisualObjectMaxLeftRequest& request);
VisualAssetEditResult set_visual_object_max_top(const VisualObjectMaxTopRequest& request);
VisualAssetEditResult set_visual_object_min_button(const VisualObjectMinButtonRequest& request);
VisualAssetEditResult set_visual_object_min_height(const VisualObjectMinHeightRequest& request);
VisualAssetEditResult set_visual_object_min_width(const VisualObjectMinWidthRequest& request);
VisualAssetEditResult set_visual_object_movable(const VisualObjectMovableRequest& request);
VisualAssetEditResult set_visual_object_half_height_caption(
    const VisualObjectHalfHeightCaptionRequest& request);
VisualAssetEditResult set_visual_object_mdi_form(const VisualObjectMdiFormRequest& request);
VisualAssetEditResult set_visual_object_whats_this_button(const VisualObjectWhatsThisButtonRequest& request);
VisualAssetEditResult set_visual_object_whats_this_help(const VisualObjectWhatsThisHelpRequest& request);
VisualAssetEditResult set_visual_object_whats_this_help_id(const VisualObjectWhatsThisHelpIdRequest& request);
VisualAssetEditResult set_visual_object_help_context_id(const VisualObjectHelpContextIdRequest& request);
VisualAssetEditResult set_visual_object_display_orientation(const VisualObjectDisplayOrientationRequest& request);
VisualAssetEditResult set_visual_object_tab_orientation(const VisualObjectTabOrientationRequest& request);
VisualAssetEditResult set_visual_object_list_item_id(const VisualObjectListItemIdRequest& request);
VisualAssetEditResult set_visual_object_lock_screen(const VisualObjectLockScreenRequest& request);
VisualAssetEditResult set_visual_object_hide_selection(const VisualObjectHideSelectionRequest& request);
VisualAssetEditResult set_visual_object_allow_cell_selection(const VisualObjectAllowCellSelectionRequest& request);
VisualAssetEditResult set_visual_object_delete_mark(const VisualObjectDeleteMarkRequest& request);
VisualAssetEditResult set_visual_object_record_mark(const VisualObjectRecordMarkRequest& request);
VisualAssetEditResult set_visual_object_split_bar(const VisualObjectSplitBarRequest& request);
VisualAssetEditResult set_visual_object_highlight_row(const VisualObjectHighlightRowRequest& request);
VisualAssetEditResult set_visual_object_panel_link(const VisualObjectPanelLinkRequest& request);
VisualAssetEditResult set_visual_object_allow_header_sizing(const VisualObjectAllowHeaderSizingRequest& request);
VisualAssetEditResult set_visual_object_allow_row_sizing(const VisualObjectAllowRowSizingRequest& request);
VisualAssetEditResult set_visual_object_resizable(const VisualObjectResizableRequest& request);
VisualAssetEditResult set_visual_object_sparse(const VisualObjectSparseRequest& request);
VisualAssetEditResult set_visual_object_add_line_feeds(const VisualObjectAddLineFeedsRequest& request);
VisualAssetEditResult set_visual_object_always_on_top(const VisualObjectAlwaysOnTopRequest& request);
VisualAssetEditResult set_visual_object_always_on_bottom(const VisualObjectAlwaysOnBottomRequest& request);
VisualAssetEditResult set_visual_object_style(const VisualObjectStyleRequest& request);

// ==== Per-property setters: colors, fonts, pictures, drawing/borders ====
VisualAssetEditResult set_visual_object_picture(const VisualObjectPictureRequest& request);
VisualAssetEditResult set_visual_object_down_picture(const VisualObjectDownPictureRequest& request);
VisualAssetEditResult set_visual_object_disabled_picture(const VisualObjectDisabledPictureRequest& request);
VisualAssetEditResult set_visual_object_ole_drag_picture(const VisualObjectOleDragPictureRequest& request);
VisualAssetEditResult set_visual_object_mouse_icon(const VisualObjectMouseIconRequest& request);
VisualAssetEditResult set_visual_object_drag_icon(const VisualObjectDragIconRequest& request);
VisualAssetEditResult set_visual_object_drag_mode(const VisualObjectDragModeRequest& request);
VisualAssetEditResult set_visual_object_ole_drag_mode(const VisualObjectOleDragModeRequest& request);
VisualAssetEditResult set_visual_object_ole_drop_mode(const VisualObjectOleDropModeRequest& request);
VisualAssetEditResult set_visual_object_ole_drop_effects(const VisualObjectOleDropEffectsRequest& request);
VisualAssetEditResult set_visual_object_ole_drop_text_insertion(
    const VisualObjectOleDropTextInsertionRequest& request);
VisualAssetEditResult set_visual_object_back_style(const VisualObjectBackStyleRequest& request);
VisualAssetEditResult set_visual_object_border_style(const VisualObjectBorderStyleRequest& request);
VisualAssetEditResult set_visual_object_border_width(const VisualObjectBorderWidthRequest& request);
VisualAssetEditResult set_visual_object_border_color(const VisualObjectBorderColorRequest& request);
VisualAssetEditResult set_visual_object_grid_line_color(const VisualObjectGridLineColorRequest& request);
VisualAssetEditResult set_visual_object_grid_line_width(const VisualObjectGridLineWidthRequest& request);
VisualAssetEditResult set_visual_object_grid_lines(const VisualObjectGridLinesRequest& request);
VisualAssetEditResult set_visual_object_highlight_row_line_width(
    const VisualObjectHighlightRowLineWidthRequest& request);
VisualAssetEditResult set_visual_object_highlight_style(const VisualObjectHighlightStyleRequest& request);
VisualAssetEditResult set_visual_object_header_height(const VisualObjectHeaderHeightRequest& request);
VisualAssetEditResult set_visual_object_row_height(const VisualObjectRowHeightRequest& request);
VisualAssetEditResult set_visual_object_special_effect(const VisualObjectSpecialEffectRequest& request);
VisualAssetEditResult set_visual_object_curvature(const VisualObjectCurvatureRequest& request);
VisualAssetEditResult set_visual_object_draw_mode(const VisualObjectDrawModeRequest& request);
VisualAssetEditResult set_visual_object_draw_style(const VisualObjectDrawStyleRequest& request);
VisualAssetEditResult set_visual_object_draw_width(const VisualObjectDrawWidthRequest& request);
VisualAssetEditResult set_visual_object_fill_color(const VisualObjectFillColorRequest& request);
VisualAssetEditResult set_visual_object_fill_style(const VisualObjectFillStyleRequest& request);
VisualAssetEditResult set_visual_object_buffer_mode(const VisualObjectBufferModeRequest& request);
VisualAssetEditResult set_visual_object_buffer_mode_override(
    const VisualObjectBufferModeOverrideRequest& request);
VisualAssetEditResult set_visual_object_scale_mode(const VisualObjectScaleModeRequest& request);
VisualAssetEditResult set_visual_object_scroll_bars(const VisualObjectScrollBarsRequest& request);
VisualAssetEditResult set_visual_object_window_state(const VisualObjectWindowStateRequest& request);
VisualAssetEditResult set_visual_object_show_window(const VisualObjectShowWindowRequest& request);
VisualAssetEditResult set_visual_object_title_bar(const VisualObjectTitleBarRequest& request);
VisualAssetEditResult set_visual_object_mouse_pointer(const VisualObjectMousePointerRequest& request);
VisualAssetEditResult set_visual_object_picture_margin(const VisualObjectPictureMarginRequest& request);
VisualAssetEditResult set_visual_object_picture_position(const VisualObjectPicturePositionRequest& request);
VisualAssetEditResult set_visual_object_picture_spacing(const VisualObjectPictureSpacingRequest& request);
VisualAssetEditResult set_visual_object_picture_selection_display(
    const VisualObjectPictureSelectionDisplayRequest& request);
VisualAssetEditResult set_visual_object_input_mask(const VisualObjectInputMaskRequest& request);
VisualAssetEditResult set_visual_object_dynamic_input_mask(const VisualObjectDynamicInputMaskRequest& request);
VisualAssetEditResult set_visual_object_dynamic_line_height(const VisualObjectDynamicLineHeightRequest& request);
VisualAssetEditResult set_visual_object_format(const VisualObjectFormatRequest& request);
VisualAssetEditResult set_visual_object_font_name(const VisualObjectFontNameRequest& request);
VisualAssetEditResult set_visual_object_font_size(const VisualObjectFontSizeRequest& request);
VisualAssetEditResult set_visual_object_font_bold(const VisualObjectFontBoldRequest& request);
VisualAssetEditResult set_visual_object_font_italic(const VisualObjectFontItalicRequest& request);
VisualAssetEditResult set_visual_object_font_underline(const VisualObjectFontUnderlineRequest& request);
VisualAssetEditResult set_visual_object_font_strikethru(const VisualObjectFontStrikethruRequest& request);
VisualAssetEditResult set_visual_object_font_outline(const VisualObjectFontOutlineRequest& request);
VisualAssetEditResult set_visual_object_font_shadow(const VisualObjectFontShadowRequest& request);
VisualAssetEditResult set_visual_object_dynamic_alignment(const VisualObjectDynamicAlignmentRequest& request);
VisualAssetEditResult set_visual_object_dynamic_current_control(
    const VisualObjectDynamicCurrentControlRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_name(const VisualObjectDynamicFontNameRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_size(const VisualObjectDynamicFontSizeRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_bold(const VisualObjectDynamicFontBoldRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_italic(const VisualObjectDynamicFontItalicRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_underline(const VisualObjectDynamicFontUnderlineRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_strikethru(const VisualObjectDynamicFontStrikethruRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_outline(const VisualObjectDynamicFontOutlineRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_shadow(const VisualObjectDynamicFontShadowRequest& request);
VisualAssetEditResult set_visual_object_selected_back_color(const VisualObjectSelectedBackColorRequest& request);
VisualAssetEditResult set_visual_object_selected_fore_color(const VisualObjectSelectedForeColorRequest& request);
VisualAssetEditResult set_visual_object_selected_item_back_color(
    const VisualObjectSelectedItemBackColorRequest& request);
VisualAssetEditResult set_visual_object_selected_item_fore_color(
    const VisualObjectSelectedItemForeColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_item_back_color(
    const VisualObjectDisabledItemBackColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_item_fore_color(
    const VisualObjectDisabledItemForeColorRequest& request);
VisualAssetEditResult set_visual_object_item_back_color(
    const VisualObjectItemBackColorRequest& request);
VisualAssetEditResult set_visual_object_item_fore_color(
    const VisualObjectItemForeColorRequest& request);
VisualAssetEditResult set_visual_object_highlight_back_color(
    const VisualObjectHighlightBackColorRequest& request);
VisualAssetEditResult set_visual_object_highlight_fore_color(
    const VisualObjectHighlightForeColorRequest& request);
VisualAssetEditResult set_visual_object_back_color(
    const VisualObjectBackColorRequest& request);
VisualAssetEditResult set_visual_object_fore_color(
    const VisualObjectForeColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_back_color(
    const VisualObjectDisabledBackColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_fore_color(
    const VisualObjectDisabledForeColorRequest& request);
VisualAssetEditResult set_visual_object_dynamic_back_color(
    const VisualObjectDynamicBackColorRequest& request);
VisualAssetEditResult set_visual_object_dynamic_fore_color(
    const VisualObjectDynamicForeColorRequest& request);

// ==== Per-property setters: data binding and grid/list configuration ====
VisualAssetEditResult set_visual_object_lock_columns(const VisualObjectLockColumnsRequest& request);
VisualAssetEditResult set_visual_object_lock_columns_left(const VisualObjectLockColumnsLeftRequest& request);
VisualAssetEditResult set_visual_object_record_source(const VisualObjectRecordSourceRequest& request);
VisualAssetEditResult set_visual_object_link_master(const VisualObjectLinkMasterRequest& request);
VisualAssetEditResult set_visual_object_initial_selected_alias(const VisualObjectInitialSelectedAliasRequest& request);
VisualAssetEditResult set_visual_object_default_file_path(const VisualObjectDefaultFilePathRequest& request);
VisualAssetEditResult set_visual_object_form_set_class(const VisualObjectFormSetClassRequest& request);
VisualAssetEditResult set_visual_object_record_source_type(const VisualObjectRecordSourceTypeRequest& request);
VisualAssetEditResult set_visual_object_partition(const VisualObjectPartitionRequest& request);
VisualAssetEditResult set_visual_object_column_order(const VisualObjectColumnOrderRequest& request);
VisualAssetEditResult set_visual_object_child_order(const VisualObjectChildOrderRequest& request);
VisualAssetEditResult set_visual_object_data_session(const VisualObjectDataSessionRequest& request);
VisualAssetEditResult set_visual_object_row_source(const VisualObjectRowSourceRequest& request);
VisualAssetEditResult set_visual_object_row_source_type(const VisualObjectRowSourceTypeRequest& request);
VisualAssetEditResult set_visual_object_bound_column(const VisualObjectBoundColumnRequest& request);
VisualAssetEditResult set_visual_object_button_count(const VisualObjectButtonCountRequest& request);
VisualAssetEditResult set_visual_object_column_count(const VisualObjectColumnCountRequest& request);
VisualAssetEditResult set_visual_object_column_widths(const VisualObjectColumnWidthsRequest& request);
VisualAssetEditResult set_visual_object_column_lines(const VisualObjectColumnLinesRequest& request);
VisualAssetEditResult set_visual_object_integral_height(const VisualObjectIntegralHeightRequest& request);
VisualAssetEditResult set_visual_object_incremental_search(const VisualObjectIncrementalSearchRequest& request);
VisualAssetEditResult set_visual_object_multi_select(const VisualObjectMultiSelectRequest& request);
VisualAssetEditResult set_visual_object_list_index(const VisualObjectListIndexRequest& request);
VisualAssetEditResult set_visual_object_left_column(const VisualObjectLeftColumnRequest& request);
VisualAssetEditResult set_visual_object_display_value(const VisualObjectDisplayValueRequest& request);

}  // namespace copperfin::vfp

#endif
