#pragma once

#include "copperfin/studio/designer_context.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_table.h"

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::studio {

enum class StudioAssetKind {
    unknown,
    project,
    form,
    class_library,
    report,
    label,
    menu,
    index,
    table,
    database_container,
    program,
    header
};

enum class StudioUndoMode {
    unspecified,
    edit,
    command
};

inline constexpr std::size_t StudioObjectMissingFieldIndex = static_cast<std::size_t>(-1);
inline constexpr std::size_t StudioObjectMissingLineIndex = static_cast<std::size_t>(-1);
inline constexpr std::size_t StudioObjectMissingRecordIndex = static_cast<std::size_t>(-1);

struct StudioObjectSelector {
    std::size_t record_index = 0;
    std::string object_name{};
    std::string unique_id{};
};

struct StudioFieldValueAssignment {
    std::string property_name{};
    std::string property_value{};
};

struct StudioOpenRequest {
    std::string path{};
    std::string symbol{};
    std::string property_name{};
    std::string property_value{};
    std::string new_property_name{};
    std::string object_name{};
    std::string unique_id{};
    std::string new_object_name{};
    std::string new_name{};
    std::string new_unique_id{};
    std::string parent_name{};
    std::string parent_unique_id{};
    std::string placement{};
    std::string target_object_name{};
    std::string target_unique_id{};
    std::string alignment_mode{};
    std::string resize_mode{};
    std::string distribution_mode{};
    std::string snap_mode{};
    std::string nudge_mode{};
    std::string caption{};
    std::string tooltip_text{};
    std::string status_bar_text{};
    std::string control_source{};
    std::string current_control{};
    std::string input_mask{};
    std::string format{};
    std::string row_source{};
    std::string display_value{};
    std::string dynamic_back_color{};
    std::string dynamic_fore_color{};
    std::string anchor_object_name{};
    std::string anchor_unique_id{};
    double grid_width = 0.0;
    double grid_height = 0.0;
    double delta_hpos = 0.0;
    double delta_vpos = 0.0;
    int starting_tab_index = 0;
    int row_source_type = 0;
    int bound_column = 0;
    int column_count = 0;
    int style = 0;
    int list_index = 0;
    int left_column = 0;
    int selected_back_color = 0;
    int selected_fore_color = 0;
    int selected_item_back_color = 0;
    int selected_item_fore_color = 0;
    int disabled_item_back_color = 0;
    int disabled_item_fore_color = 0;
    int item_back_color = 0;
    int item_fore_color = 0;
    int highlight_back_color = 0;
    int highlight_fore_color = 0;
    int back_color = 0;
    int fore_color = 0;
    int disabled_back_color = 0;
    int disabled_fore_color = 0;
    std::size_t line = 0;
    std::size_t column = 0;
    std::size_t record_index = 0;
    bool launched_from_visual_studio = false;
    bool read_only = false;
    bool load_full_table = false;
    bool apply_property_update = false;
    bool clear_property = false;
    bool rename_property = false;
    bool delete_object = false;
    bool restore_object = false;
    bool duplicate_object = false;
    bool rename_object = false;
    bool reparent_object = false;
    bool reorder_object = false;
    bool group_object = false;
    bool align_object = false;
    bool resize_object = false;
    bool distribute_object = false;
    bool snap_object = false;
    bool nudge_object = false;
    bool tab_order_object = false;
    bool tab_stop_object = false;
    bool visibility_object = false;
    bool enabled_object = false;
    bool read_only_object = false;
    bool locked_object = false;
    bool caption_object = false;
    bool tooltip_text_object = false;
    bool status_bar_text_object = false;
    bool control_source_object = false;
    bool current_control_object = false;
    bool input_mask_object = false;
    bool format_object = false;
    bool row_source_object = false;
    bool row_source_type_object = false;
    bool bound_column_object = false;
    bool column_count_object = false;
    bool style_object = false;
    bool list_index_object = false;
    bool left_column_object = false;
    bool display_value_object = false;
    bool selected_back_color_object = false;
    bool selected_fore_color_object = false;
    bool selected_item_back_color_object = false;
    bool selected_item_fore_color_object = false;
    bool disabled_item_back_color_object = false;
    bool disabled_item_fore_color_object = false;
    bool item_back_color_object = false;
    bool item_fore_color_object = false;
    bool highlight_back_color_object = false;
    bool highlight_fore_color_object = false;
    bool back_color_object = false;
    bool fore_color_object = false;
    bool disabled_back_color_object = false;
    bool disabled_fore_color_object = false;
    bool dynamic_back_color_object = false;
    bool dynamic_fore_color_object = false;
    bool closable_object = false;
    bool control_box_object = false;
    bool allow_output_object = false;
    bool ungroup_object = false;
    bool tab_stop = false;
    bool visible = false;
    bool enabled = false;
    bool object_read_only = false;
    bool locked = false;
    bool clear_parent = false;
    bool selection_record_available = false;
    bool starting_tab_index_available = false;
    bool tab_stop_available = false;
    bool visible_available = false;
    bool enabled_available = false;
    bool object_read_only_available = false;
    bool locked_available = false;
    bool caption_available = false;
    bool tooltip_text_available = false;
    bool status_bar_text_available = false;
    bool control_source_available = false;
    bool current_control_available = false;
    bool input_mask_available = false;
    bool format_available = false;
    bool row_source_available = false;
    bool row_source_type_available = false;
    bool bound_column_available = false;
    bool column_count_available = false;
    bool style_available = false;
    bool list_index_available = false;
    bool left_column_available = false;
    bool display_value_available = false;
    bool selected_back_color_available = false;
    bool selected_fore_color_available = false;
    bool selected_item_back_color_available = false;
    bool selected_item_fore_color_available = false;
    bool disabled_item_back_color_available = false;
    bool disabled_item_fore_color_available = false;
    bool item_back_color_available = false;
    bool item_fore_color_available = false;
    bool highlight_back_color_available = false;
    bool highlight_fore_color_available = false;
    bool back_color_available = false;
    bool fore_color_available = false;
    bool disabled_back_color_available = false;
    bool disabled_fore_color_available = false;
    bool dynamic_back_color_available = false;
    bool dynamic_fore_color_available = false;
    bool closable_available = false;
    bool control_box_available = false;
    bool allow_output_available = false;
    bool closable = false;
    bool control_box = false;
    bool allow_output = false;
    StudioUndoMode undo_mode = StudioUndoMode::unspecified;
    std::string undo_label{};
    std::vector<StudioFieldValueAssignment> field_values{};
    std::vector<StudioObjectSelector> group_objects{};
    std::vector<StudioObjectSelector> align_objects{};
    std::vector<StudioObjectSelector> resize_objects{};
    std::vector<StudioObjectSelector> distribute_objects{};
    std::vector<StudioObjectSelector> snap_objects{};
    std::vector<StudioObjectSelector> nudge_objects{};
    std::vector<StudioObjectSelector> tab_order_objects{};
    std::vector<StudioObjectSelector> tab_stop_objects{};
    std::vector<StudioObjectSelector> visibility_objects{};
    std::vector<StudioObjectSelector> enabled_objects{};
    std::vector<StudioObjectSelector> read_only_objects{};
    std::vector<StudioObjectSelector> locked_objects{};
    std::vector<StudioObjectSelector> caption_objects{};
    std::vector<StudioObjectSelector> tooltip_text_objects{};
    std::vector<StudioObjectSelector> status_bar_text_objects{};
    std::vector<StudioObjectSelector> control_source_objects{};
    std::vector<StudioObjectSelector> current_control_objects{};
    std::vector<StudioObjectSelector> input_mask_objects{};
    std::vector<StudioObjectSelector> format_objects{};
    std::vector<StudioObjectSelector> row_source_objects{};
    std::vector<StudioObjectSelector> row_source_type_objects{};
    std::vector<StudioObjectSelector> bound_column_objects{};
    std::vector<StudioObjectSelector> column_count_objects{};
    std::vector<StudioObjectSelector> style_objects{};
    std::vector<StudioObjectSelector> list_index_objects{};
    std::vector<StudioObjectSelector> left_column_objects{};
    std::vector<StudioObjectSelector> display_value_objects{};
    std::vector<StudioObjectSelector> selected_back_color_objects{};
    std::vector<StudioObjectSelector> selected_fore_color_objects{};
    std::vector<StudioObjectSelector> selected_item_back_color_objects{};
    std::vector<StudioObjectSelector> selected_item_fore_color_objects{};
    std::vector<StudioObjectSelector> disabled_item_back_color_objects{};
    std::vector<StudioObjectSelector> disabled_item_fore_color_objects{};
    std::vector<StudioObjectSelector> item_back_color_objects{};
    std::vector<StudioObjectSelector> item_fore_color_objects{};
    std::vector<StudioObjectSelector> highlight_back_color_objects{};
    std::vector<StudioObjectSelector> highlight_fore_color_objects{};
    std::vector<StudioObjectSelector> back_color_objects{};
    std::vector<StudioObjectSelector> fore_color_objects{};
    std::vector<StudioObjectSelector> disabled_back_color_objects{};
    std::vector<StudioObjectSelector> disabled_fore_color_objects{};
    std::vector<StudioObjectSelector> dynamic_back_color_objects{};
    std::vector<StudioObjectSelector> dynamic_fore_color_objects{};
    std::vector<StudioObjectSelector> closable_objects{};
    std::vector<StudioObjectSelector> control_box_objects{};
    std::vector<StudioObjectSelector> allow_output_objects{};
    std::vector<StudioEditorSelectionContext> designer_selection_contexts{};
};

struct StudioDocumentModel {
    std::string path;
    std::string display_name;
    std::string sidecar_path;
    std::string selection_symbol;
    StudioAssetKind kind = StudioAssetKind::unknown;
    bool has_sidecar = false;
    bool read_only = false;
    bool launched_from_visual_studio = false;
    bool selection_record_available = false;
    std::size_t selection_line = 0;
    std::size_t selection_column = 0;
    std::size_t selection_record_index = 0;
    vfp::AssetInspectionResult inspection{};
    std::vector<runtime::PrgStaticDiagnostic> static_diagnostics;
    bool table_preview_available = false;
    vfp::DbfTable table_preview{};
    std::vector<StudioDesignerContextResult> designer_contexts{};
};

struct StudioPropertySnapshot {
    std::string name{};
    std::size_t field_index = 0;
    char type = '\0';
    bool is_null = false;
    bool derived_from_property_blob = false;
    std::size_t source_line_index = StudioObjectMissingLineIndex;
    std::uint32_t memo_block_number = 0;
    std::string value{};
};

struct StudioObjectSnapshot {
    std::size_t record_index = 0;
    bool deleted = false;
    int objtype_code = 0;
    std::size_t objtype_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t objtype_memo_block_number = 0;
    int objcode_code = 0;
    std::size_t objcode_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t objcode_memo_block_number = 0;
    std::string platform{};
    std::size_t platform_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t platform_memo_block_number = 0;
    std::string object_name{};
    std::size_t object_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t object_name_memo_block_number = 0;
    std::string object_path{};
    std::size_t object_depth = 0;
    std::size_t sibling_index = 0;
    std::size_t sibling_count = 0;
    std::string unique_id{};
    std::size_t unique_id_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t unique_id_memo_block_number = 0;
    std::string parent_name{};
    std::size_t parent_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t parent_name_memo_block_number = 0;
    std::size_t parent_record_index = StudioObjectMissingRecordIndex;
    std::vector<std::size_t> ancestor_record_indexes{};
    std::string class_name{};
    std::size_t class_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t class_name_memo_block_number = 0;
    std::string baseclass_name{};
    std::size_t baseclass_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t baseclass_name_memo_block_number = 0;
    std::size_t child_count = 0;
    std::vector<std::size_t> child_record_indexes{};
    std::string menu_prompt{};
    std::size_t menu_prompt_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t menu_prompt_memo_block_number = 0;
    std::string menu_level_name{};
    std::size_t menu_level_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t menu_level_name_memo_block_number = 0;
    std::string menu_command{};
    std::size_t menu_command_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t menu_command_memo_block_number = 0;
    std::string menu_message{};
    std::size_t menu_message_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t menu_message_memo_block_number = 0;
    std::string title{};
    std::size_t title_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t title_memo_block_number = 0;
    std::string subtitle{};
    std::size_t subtitle_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t subtitle_memo_block_number = 0;
    std::vector<StudioPropertySnapshot> properties{};
};

struct StudioOpenResult {
    bool ok = false;
    StudioDocumentModel document{};
    std::string error{};
};

[[nodiscard]] StudioAssetKind studio_asset_kind_from_vfp_family(vfp::AssetFamily family);
[[nodiscard]] const char* studio_asset_kind_name(StudioAssetKind kind);
[[nodiscard]] std::string infer_sidecar_path(const std::string& path, StudioAssetKind kind);
[[nodiscard]] std::vector<StudioObjectSnapshot> build_object_snapshot(const StudioDocumentModel& document);
StudioOpenResult open_document(const StudioOpenRequest& request);

}  // namespace copperfin::studio
