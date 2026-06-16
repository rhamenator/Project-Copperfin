#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

struct VisualPropertyAssignment {
    std::string name;
    std::string value;
    std::size_t source_line_index = static_cast<std::size_t>(-1);
};

struct VisualObjectEditRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
    std::string property_value;
};

struct VisualObjectPropertyChange {
    std::string property_name;
    std::string property_value;
};

struct VisualObjectMultiEditRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::vector<VisualObjectPropertyChange> properties;
};

struct VisualObjectBatchEditItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::vector<VisualObjectPropertyChange> properties;
};

struct VisualObjectBatchEditRequest {
    std::string path;
    std::vector<VisualObjectBatchEditItem> objects;
};

struct VisualObjectPropertyQueryRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
};

struct VisualObjectPropertyQueryResult {
    bool ok = false;
    std::string error;
    bool exists = false;
    bool direct_field = false;
    std::size_t record_index = 0;
    bool record_deleted = false;
    std::string property_name;
    std::string value;
};

struct VisualObjectPropertyListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectPropertyClearRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
};

struct VisualObjectPropertyClearBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
};

struct VisualObjectPropertyClearBatchRequest {
    std::string path;
    std::vector<VisualObjectPropertyClearBatchItem> properties;
};

struct VisualObjectPropertyCopyRequest {
    std::string path;
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_property_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_property_name;
    bool replace_existing = false;
};

struct VisualObjectPropertyCopyBatchItem {
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_property_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_property_name;
    bool replace_existing = false;
};

struct VisualObjectPropertyCopyBatchRequest {
    std::string path;
    std::vector<VisualObjectPropertyCopyBatchItem> properties;
};

struct VisualObjectPropertyMoveRequest {
    std::string path;
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_property_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_property_name;
    bool replace_existing = false;
};

struct VisualObjectPropertyMoveBatchItem {
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_property_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_property_name;
    bool replace_existing = false;
};

struct VisualObjectPropertyMoveBatchRequest {
    std::string path;
    std::vector<VisualObjectPropertyMoveBatchItem> properties;
};

struct VisualObjectPropertyRenameRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
    std::string new_property_name;
};

struct VisualObjectPropertyRenameBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
    std::string new_property_name;
};

struct VisualObjectPropertyRenameBatchRequest {
    std::string path;
    std::vector<VisualObjectPropertyRenameBatchItem> properties;
};

struct VisualObjectPropertyReorderRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
    std::string placement;
    std::string relative_property_name;
};

struct VisualObjectPropertyReorderBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string property_name;
    std::string placement;
    std::string relative_property_name;
};

struct VisualObjectPropertyReorderBatchRequest {
    std::string path;
    std::vector<VisualObjectPropertyReorderBatchItem> properties;
};

struct VisualObjectPropertySnapshot {
    std::string property_name;
    std::string value;
    bool direct_field = false;
    char field_type = '\0';
    std::size_t source_line_index = static_cast<std::size_t>(-1);
};

struct VisualObjectPropertyListResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
    bool record_deleted = false;
    std::vector<VisualObjectPropertySnapshot> properties;
};

struct VisualObjectDeletedStateRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool deleted = false;
};

struct VisualObjectDeletedStateBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool deleted = false;
};

struct VisualObjectDeletedStateBatchRequest {
    std::string path;
    std::vector<VisualObjectDeletedStateBatchItem> objects;
};

struct VisualObjectSubtreeDeletedStateRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool deleted = false;
};

struct VisualObjectSnapshot {
    std::size_t record_index = 0;
    bool deleted = false;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::string class_name;
    std::string baseclass_name;
    std::string caption;
};

struct VisualObjectListResult {
    bool ok = false;
    std::string error;
    std::vector<VisualObjectSnapshot> objects;
};

struct VisualObjectChildrenListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectChildrenListResult {
    bool ok = false;
    std::string error;
    std::size_t parent_record_index = 0;
    std::string parent_name;
    std::vector<VisualObjectSnapshot> children;
};

struct VisualObjectDescendantSnapshot {
    VisualObjectSnapshot object;
    std::size_t depth = 0;
};

struct VisualObjectDescendantsListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectDescendantsListResult {
    bool ok = false;
    std::string error;
    std::size_t parent_record_index = 0;
    std::string parent_name;
    std::vector<VisualObjectDescendantSnapshot> descendants;
};

struct VisualObjectAncestorSnapshot {
    VisualObjectSnapshot object;
    std::size_t depth = 0;
};

struct VisualObjectAncestorsListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectAncestorsListResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
    std::vector<VisualObjectAncestorSnapshot> ancestors;
};

struct VisualObjectMethodListRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectMethodSnapshot {
    std::string method_name;
    std::string kind;
    std::string source_text;
    std::size_t source_line_index = static_cast<std::size_t>(-1);
    std::uint32_t source_memo_block_number = 0;
};

struct VisualObjectMethodListResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
    bool record_deleted = false;
    std::vector<VisualObjectMethodSnapshot> methods;
};

struct VisualObjectMethodQueryRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
};

struct VisualObjectMethodQueryResult {
    bool ok = false;
    std::string error;
    bool exists = false;
    std::size_t record_index = 0;
    bool record_deleted = false;
    VisualObjectMethodSnapshot method;
};

struct VisualObjectMethodEditRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
    std::string method_kind = "procedure";
    std::string source_text;
};

struct VisualObjectMethodDeleteRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
};

struct VisualObjectMethodDeleteBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
};

struct VisualObjectMethodDeleteBatchRequest {
    std::string path;
    std::vector<VisualObjectMethodDeleteBatchItem> methods;
};

struct VisualObjectMethodRenameRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
    std::string new_method_name;
};

struct VisualObjectMethodRenameBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
    std::string new_method_name;
};

struct VisualObjectMethodRenameBatchRequest {
    std::string path;
    std::vector<VisualObjectMethodRenameBatchItem> methods;
};

struct VisualObjectMethodCopyRequest {
    std::string path;
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_method_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_method_name;
    bool replace_existing = false;
};

struct VisualObjectMethodCopyBatchItem {
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_method_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_method_name;
    bool replace_existing = false;
};

struct VisualObjectMethodCopyBatchRequest {
    std::string path;
    std::vector<VisualObjectMethodCopyBatchItem> methods;
};

struct VisualObjectMethodMoveRequest {
    std::string path;
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_method_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_method_name;
    bool replace_existing = false;
};

struct VisualObjectMethodMoveBatchItem {
    std::size_t source_record_index = 0;
    std::string source_object_name;
    std::string source_unique_id;
    std::string source_method_name;
    std::size_t target_record_index = 0;
    std::string target_object_name;
    std::string target_unique_id;
    std::string target_method_name;
    bool replace_existing = false;
};

struct VisualObjectMethodMoveBatchRequest {
    std::string path;
    std::vector<VisualObjectMethodMoveBatchItem> methods;
};

struct VisualObjectMethodReorderRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
    std::string placement;
    std::string relative_method_name;
};

struct VisualObjectMethodReorderBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string method_name;
    std::string placement;
    std::string relative_method_name;
};

struct VisualObjectMethodReorderBatchRequest {
    std::string path;
    std::vector<VisualObjectMethodReorderBatchItem> methods;
};

struct VisualObjectDuplicateRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string new_object_name;
    std::string new_name;
    std::string new_unique_id;
};

struct VisualObjectDuplicateBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string new_object_name;
    std::string new_name;
    std::string new_unique_id;
};

struct VisualObjectDuplicateBatchRequest {
    std::string path;
    std::vector<VisualObjectDuplicateBatchItem> objects;
};

struct VisualObjectDuplicateResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
};

struct VisualObjectDuplicateBatchResult {
    bool ok = false;
    std::string error;
    std::vector<std::size_t> record_indexes;
};

struct VisualObjectSubtreeDuplicateReplacement {
    std::string source_unique_id;
    std::string new_object_name;
    std::string new_name;
    std::string new_unique_id;
};

struct VisualObjectSubtreeDuplicateRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::vector<VisualObjectSubtreeDuplicateReplacement> replacements;
};

struct VisualObjectSubtreeDuplicateResult {
    bool ok = false;
    std::string error;
    std::size_t root_record_index = 0;
    std::size_t copied_count = 0;
};

struct VisualObjectCreateRequest {
    std::string path;
    std::vector<VisualObjectPropertyChange> field_values;
};

struct VisualObjectCreateBatchItem {
    std::vector<VisualObjectPropertyChange> field_values;
};

struct VisualObjectCreateBatchRequest {
    std::string path;
    std::vector<VisualObjectCreateBatchItem> objects;
};

struct VisualObjectCreateResult {
    bool ok = false;
    std::string error;
    std::size_t record_index = 0;
};

struct VisualObjectCreateBatchResult {
    bool ok = false;
    std::string error;
    std::vector<std::size_t> record_indexes;
};

struct VisualObjectAlignmentTarget {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectAlignmentRequest {
    std::string path;
    std::size_t anchor_record_index = 0;
    std::string anchor_object_name;
    std::string anchor_unique_id;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string mode;
};

struct VisualObjectResizeRequest {
    std::string path;
    std::size_t anchor_record_index = 0;
    std::string anchor_object_name;
    std::string anchor_unique_id;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string mode;
};

struct VisualObjectGroupRequest {
    std::string path;
    std::vector<VisualObjectPropertyChange> container_field_values;
    std::vector<VisualObjectAlignmentTarget> objects;
};

struct VisualObjectGroupResult {
    bool ok = false;
    std::string error;
    std::size_t container_record_index = 0;
};

struct VisualObjectUngroupRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct VisualObjectUngroupResult {
    bool ok = false;
    std::string error;
    std::size_t container_record_index = 0;
    std::size_t child_count = 0;
};

struct VisualObjectDistributeRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string mode;
};

struct VisualObjectSnapToGridRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string mode;
    double grid_width = 0.0;
    double grid_height = 0.0;
};

struct VisualObjectNudgeRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string mode;
    double delta_hpos = 0.0;
    double delta_vpos = 0.0;
};

struct VisualObjectTabOrderRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int starting_tab_index = 0;
};

struct VisualObjectTabStopRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool tab_stop = false;
};

struct VisualObjectVisibilityRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool visible = false;
};

struct VisualObjectEnabledRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool enabled = false;
};

struct VisualObjectReadOnlyRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool read_only = false;
};

struct VisualObjectLockedRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool locked = false;
};

struct VisualObjectCaptionRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string caption;
};

struct VisualObjectToolTipTextRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string tooltip_text;
};

struct VisualObjectStatusBarTextRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string status_bar_text;
};

struct VisualObjectControlSourceRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string control_source;
};

struct VisualObjectCurrentControlRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string current_control;
};

struct VisualObjectSparseRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool sparse = false;
};

struct VisualObjectAddLineFeedsRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool add_line_feeds = false;
};

struct VisualObjectAlwaysOnTopRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool always_on_top = false;
};

struct VisualObjectAlwaysOnBottomRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool always_on_bottom = false;
};

struct VisualObjectPictureRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string picture;
};

struct VisualObjectDownPictureRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string down_picture;
};

struct VisualObjectDisabledPictureRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string disabled_picture;
};

struct VisualObjectOleDragPictureRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string ole_drag_picture;
};

struct VisualObjectBackStyleRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int back_style = 0;
};

struct VisualObjectBorderStyleRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int border_style = 0;
};

struct VisualObjectBorderWidthRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int border_width = 0;
};

struct VisualObjectBorderColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int border_color = 0;
};

struct VisualObjectInputMaskRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string input_mask;
};

struct VisualObjectDynamicInputMaskRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_input_mask;
};

struct VisualObjectDynamicLineHeightRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_line_height;
};

struct VisualObjectFormatRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string format;
};

struct VisualObjectFontNameRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string font_name;
};

struct VisualObjectFontSizeRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    double font_size = 0.0;
};

struct VisualObjectFontBoldRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool font_bold = false;
};

struct VisualObjectFontItalicRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool font_italic = false;
};

struct VisualObjectFontUnderlineRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool font_underline = false;
};

struct VisualObjectFontStrikethruRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool font_strikethru = false;
};

struct VisualObjectFontOutlineRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool font_outline = false;
};

struct VisualObjectFontShadowRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool font_shadow = false;
};

struct VisualObjectDynamicAlignmentRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_alignment;
};

struct VisualObjectDynamicCurrentControlRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_current_control;
};

struct VisualObjectDynamicFontNameRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_name;
};

struct VisualObjectDynamicFontSizeRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_size;
};

struct VisualObjectDynamicFontBoldRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_bold;
};

struct VisualObjectDynamicFontItalicRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_italic;
};

struct VisualObjectDynamicFontUnderlineRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_underline;
};

struct VisualObjectDynamicFontStrikethruRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_strikethru;
};

struct VisualObjectDynamicFontOutlineRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_outline;
};

struct VisualObjectDynamicFontShadowRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_font_shadow;
};

struct VisualObjectRowSourceRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string row_source;
};

struct VisualObjectRowSourceTypeRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int row_source_type = 0;
};

struct VisualObjectBoundColumnRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int bound_column = 0;
};

struct VisualObjectColumnCountRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int column_count = 0;
};

struct VisualObjectColumnWidthsRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string column_widths;
};

struct VisualObjectColumnLinesRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool column_lines = false;
};

struct VisualObjectIntegralHeightRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool integral_height = false;
};

struct VisualObjectIncrementalSearchRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool incremental_search = false;
};

struct VisualObjectMultiSelectRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    bool multi_select = false;
};

struct VisualObjectStyleRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int style = 0;
};

struct VisualObjectListIndexRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int list_index = 0;
};

struct VisualObjectDisplayValueRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string display_value;
};

struct VisualObjectSelectedItemBackColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int selected_item_back_color = 0;
};

struct VisualObjectSelectedItemForeColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int selected_item_fore_color = 0;
};

struct VisualObjectDisabledItemBackColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int disabled_item_back_color = 0;
};

struct VisualObjectDisabledItemForeColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int disabled_item_fore_color = 0;
};

struct VisualObjectItemBackColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int item_back_color = 0;
};

struct VisualObjectItemForeColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int item_fore_color = 0;
};

struct VisualObjectHighlightBackColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int highlight_back_color = 0;
};

struct VisualObjectHighlightForeColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int highlight_fore_color = 0;
};

struct VisualObjectBackColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int back_color = 0;
};

struct VisualObjectForeColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int fore_color = 0;
};

struct VisualObjectDisabledBackColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int disabled_back_color = 0;
};

struct VisualObjectDisabledForeColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    int disabled_fore_color = 0;
};

struct VisualObjectDynamicBackColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_back_color;
};

struct VisualObjectDynamicForeColorRequest {
    std::string path;
    std::vector<VisualObjectAlignmentTarget> objects;
    std::string dynamic_fore_color;
};

struct VisualObjectReparentRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string parent_object_name;
    std::string parent_unique_id;
    bool clear_parent = false;
};

struct VisualObjectReparentBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string parent_object_name;
    std::string parent_unique_id;
    bool clear_parent = false;
};

struct VisualObjectReparentBatchRequest {
    std::string path;
    std::vector<VisualObjectReparentBatchItem> objects;
};

struct VisualObjectRenameRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool update_object_name = false;
    std::string new_object_name;
    bool update_name = false;
    std::string new_name;
    bool update_unique_id = false;
    std::string new_unique_id;
};

struct VisualObjectRenameBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool update_object_name = false;
    std::string new_object_name;
    bool update_name = false;
    std::string new_name;
    bool update_unique_id = false;
    std::string new_unique_id;
};

struct VisualObjectRenameBatchRequest {
    std::string path;
    std::vector<VisualObjectRenameBatchItem> objects;
};

struct VisualObjectReorderRequest {
    std::string path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string placement;
    std::string target_object_name;
    std::string target_unique_id;
};

struct VisualObjectReorderBatchItem {
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string placement;
    std::string target_object_name;
    std::string target_unique_id;
};

struct VisualObjectReorderBatchRequest {
    std::string path;
    std::vector<VisualObjectReorderBatchItem> objects;
};

struct VisualAssetEditResult {
    bool ok = false;
    std::string error;
};

struct VisualAssetUndoStatus {
    bool available = false;
    std::string label;
};

[[nodiscard]] std::vector<VisualPropertyAssignment> parse_visual_property_blob(const std::string& text);
[[nodiscard]] std::string serialize_visual_property_blob(const std::vector<VisualPropertyAssignment>& properties);
[[nodiscard]] bool is_property_blob_asset_path(const std::string& path);
[[nodiscard]] VisualObjectPropertyQueryResult query_visual_object_property(const VisualObjectPropertyQueryRequest& request);
[[nodiscard]] VisualObjectPropertyListResult list_visual_object_properties(const VisualObjectPropertyListRequest& request);
[[nodiscard]] VisualObjectListResult list_visual_objects(const std::string& path);
[[nodiscard]] VisualObjectChildrenListResult list_visual_object_children(const VisualObjectChildrenListRequest& request);
[[nodiscard]] VisualObjectDescendantsListResult list_visual_object_descendants(const VisualObjectDescendantsListRequest& request);
[[nodiscard]] VisualObjectAncestorsListResult list_visual_object_ancestors(const VisualObjectAncestorsListRequest& request);
[[nodiscard]] VisualObjectMethodListResult list_visual_object_methods(const VisualObjectMethodListRequest& request);
[[nodiscard]] VisualObjectMethodQueryResult query_visual_object_method(const VisualObjectMethodQueryRequest& request);
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
VisualObjectSubtreeDuplicateResult duplicate_visual_object_subtree(const VisualObjectSubtreeDuplicateRequest& request);
VisualObjectCreateResult create_visual_object(const VisualObjectCreateRequest& request);
VisualObjectCreateBatchResult create_visual_objects(const VisualObjectCreateBatchRequest& request);
VisualAssetEditResult align_visual_objects(const VisualObjectAlignmentRequest& request);
VisualAssetEditResult resize_visual_objects(const VisualObjectResizeRequest& request);
VisualObjectGroupResult group_visual_objects(const VisualObjectGroupRequest& request);
VisualObjectUngroupResult ungroup_visual_object(const VisualObjectUngroupRequest& request);
VisualAssetEditResult distribute_visual_objects(const VisualObjectDistributeRequest& request);
VisualAssetEditResult snap_visual_objects_to_grid(const VisualObjectSnapToGridRequest& request);
VisualAssetEditResult nudge_visual_objects(const VisualObjectNudgeRequest& request);
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
VisualAssetEditResult set_visual_object_sparse(const VisualObjectSparseRequest& request);
VisualAssetEditResult set_visual_object_add_line_feeds(const VisualObjectAddLineFeedsRequest& request);
VisualAssetEditResult set_visual_object_always_on_top(const VisualObjectAlwaysOnTopRequest& request);
VisualAssetEditResult set_visual_object_always_on_bottom(const VisualObjectAlwaysOnBottomRequest& request);
VisualAssetEditResult set_visual_object_picture(const VisualObjectPictureRequest& request);
VisualAssetEditResult set_visual_object_down_picture(const VisualObjectDownPictureRequest& request);
VisualAssetEditResult set_visual_object_disabled_picture(const VisualObjectDisabledPictureRequest& request);
VisualAssetEditResult set_visual_object_ole_drag_picture(const VisualObjectOleDragPictureRequest& request);
VisualAssetEditResult set_visual_object_back_style(const VisualObjectBackStyleRequest& request);
VisualAssetEditResult set_visual_object_border_style(const VisualObjectBorderStyleRequest& request);
VisualAssetEditResult set_visual_object_border_width(const VisualObjectBorderWidthRequest& request);
VisualAssetEditResult set_visual_object_border_color(const VisualObjectBorderColorRequest& request);
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
VisualAssetEditResult set_visual_object_dynamic_current_control(const VisualObjectDynamicCurrentControlRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_name(const VisualObjectDynamicFontNameRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_size(const VisualObjectDynamicFontSizeRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_bold(const VisualObjectDynamicFontBoldRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_italic(const VisualObjectDynamicFontItalicRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_underline(const VisualObjectDynamicFontUnderlineRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_strikethru(const VisualObjectDynamicFontStrikethruRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_outline(const VisualObjectDynamicFontOutlineRequest& request);
VisualAssetEditResult set_visual_object_dynamic_font_shadow(const VisualObjectDynamicFontShadowRequest& request);
VisualAssetEditResult set_visual_object_row_source(const VisualObjectRowSourceRequest& request);
VisualAssetEditResult set_visual_object_row_source_type(const VisualObjectRowSourceTypeRequest& request);
VisualAssetEditResult set_visual_object_bound_column(const VisualObjectBoundColumnRequest& request);
VisualAssetEditResult set_visual_object_column_count(const VisualObjectColumnCountRequest& request);
VisualAssetEditResult set_visual_object_column_widths(const VisualObjectColumnWidthsRequest& request);
VisualAssetEditResult set_visual_object_column_lines(const VisualObjectColumnLinesRequest& request);
VisualAssetEditResult set_visual_object_integral_height(const VisualObjectIntegralHeightRequest& request);
VisualAssetEditResult set_visual_object_incremental_search(const VisualObjectIncrementalSearchRequest& request);
VisualAssetEditResult set_visual_object_multi_select(const VisualObjectMultiSelectRequest& request);
VisualAssetEditResult set_visual_object_style(const VisualObjectStyleRequest& request);
VisualAssetEditResult set_visual_object_list_index(const VisualObjectListIndexRequest& request);
VisualAssetEditResult set_visual_object_display_value(const VisualObjectDisplayValueRequest& request);
VisualAssetEditResult set_visual_object_selected_item_back_color(const VisualObjectSelectedItemBackColorRequest& request);
VisualAssetEditResult set_visual_object_selected_item_fore_color(const VisualObjectSelectedItemForeColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_item_back_color(const VisualObjectDisabledItemBackColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_item_fore_color(const VisualObjectDisabledItemForeColorRequest& request);
VisualAssetEditResult set_visual_object_item_back_color(const VisualObjectItemBackColorRequest& request);
VisualAssetEditResult set_visual_object_item_fore_color(const VisualObjectItemForeColorRequest& request);
VisualAssetEditResult set_visual_object_highlight_back_color(const VisualObjectHighlightBackColorRequest& request);
VisualAssetEditResult set_visual_object_highlight_fore_color(const VisualObjectHighlightForeColorRequest& request);
VisualAssetEditResult set_visual_object_back_color(const VisualObjectBackColorRequest& request);
VisualAssetEditResult set_visual_object_fore_color(const VisualObjectForeColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_back_color(const VisualObjectDisabledBackColorRequest& request);
VisualAssetEditResult set_visual_object_disabled_fore_color(const VisualObjectDisabledForeColorRequest& request);
VisualAssetEditResult set_visual_object_dynamic_back_color(const VisualObjectDynamicBackColorRequest& request);
VisualAssetEditResult set_visual_object_dynamic_fore_color(const VisualObjectDynamicForeColorRequest& request);
VisualAssetEditResult reparent_visual_object(const VisualObjectReparentRequest& request);
VisualAssetEditResult reparent_visual_objects(const VisualObjectReparentBatchRequest& request);
VisualAssetEditResult rename_visual_object(const VisualObjectRenameRequest& request);
VisualAssetEditResult rename_visual_objects(const VisualObjectRenameBatchRequest& request);
VisualAssetEditResult reorder_visual_object(const VisualObjectReorderRequest& request);
VisualAssetEditResult reorder_visual_objects(const VisualObjectReorderBatchRequest& request);
VisualAssetEditResult set_visual_object_deleted_state(const VisualObjectDeletedStateRequest& request);
VisualAssetEditResult set_visual_object_deleted_states(const VisualObjectDeletedStateBatchRequest& request);
VisualAssetEditResult set_visual_object_subtree_deleted_state(const VisualObjectSubtreeDeletedStateRequest& request);
VisualAssetEditResult update_visual_object_property(const VisualObjectEditRequest& request);
VisualAssetEditResult update_visual_object_properties(const VisualObjectMultiEditRequest& request);
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
VisualAssetEditResult update_visual_object_batch(const VisualObjectBatchEditRequest& request);
[[nodiscard]] VisualAssetUndoStatus query_visual_object_undo(const std::string& path);
VisualAssetEditResult undo_visual_object_property(const std::string& path);

}  // namespace copperfin::vfp
