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

struct StudioOpenRequest {
    std::string path{};
    std::string symbol{};
    std::string property_name{};
    std::string property_value{};
    std::size_t line = 0;
    std::size_t column = 0;
    std::size_t record_index = 0;
    bool launched_from_visual_studio = false;
    bool read_only = false;
    bool load_full_table = false;
    bool apply_property_update = false;
    StudioUndoMode undo_mode = StudioUndoMode::unspecified;
    std::string undo_label{};
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
    std::string unique_id{};
    std::size_t unique_id_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t unique_id_memo_block_number = 0;
    std::string parent_name{};
    std::size_t parent_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t parent_name_memo_block_number = 0;
    std::string class_name{};
    std::size_t class_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t class_name_memo_block_number = 0;
    std::string baseclass_name{};
    std::size_t baseclass_name_field_index = StudioObjectMissingFieldIndex;
    std::uint32_t baseclass_name_memo_block_number = 0;
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
