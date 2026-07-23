// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/studio/document_model.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::localization {
struct LocalizedCatalog;
}

namespace copperfin::studio {

inline constexpr std::size_t StudioReportMissingFieldIndex = static_cast<std::size_t>(-1);
inline constexpr std::size_t StudioReportMissingLineIndex = static_cast<std::size_t>(-1);
inline constexpr std::size_t StudioReportMissingRecordIndex = static_cast<std::size_t>(-1);

struct StudioNamedValue {
    std::string name{};
    std::size_t record_index = 0;
    std::size_t field_index = StudioReportMissingFieldIndex;
    std::size_t source_line_index = StudioReportMissingLineIndex;
    std::uint32_t memo_block_number = 0;
    std::string value{};
};

struct StudioLayoutObjectSnapshot {
    std::size_t record_index = 0;
    bool deleted = false;
    int objtype_code = 0;
    int objcode_code = 0;
    std::string containing_section_id{};
    std::size_t containing_section_record_index = StudioReportMissingRecordIndex;
    int section_relative_top = 0;
    int section_relative_bottom = 0;
    std::size_t section_object_index = StudioReportMissingRecordIndex;
    std::size_t section_object_count = 0;
    std::string object_kind{};
    std::size_t object_kind_field_index = StudioReportMissingFieldIndex;
    std::uint32_t object_kind_memo_block_number = 0;
    std::size_t objtype_field_index = StudioReportMissingFieldIndex;
    std::uint32_t objtype_memo_block_number = 0;
    std::size_t objcode_field_index = StudioReportMissingFieldIndex;
    std::uint32_t objcode_memo_block_number = 0;
    std::string title{};
    std::size_t title_field_index = StudioReportMissingFieldIndex;
    std::uint32_t title_memo_block_number = 0;
    std::string expression{};
    std::size_t expression_field_index = StudioReportMissingFieldIndex;
    std::uint32_t expression_memo_block_number = 0;
    std::string picture{};
    std::size_t picture_field_index = StudioReportMissingFieldIndex;
    std::uint32_t picture_memo_block_number = 0;
    int left = 0;
    std::size_t left_field_index = StudioReportMissingFieldIndex;
    std::uint32_t left_memo_block_number = 0;
    int top = 0;
    std::size_t top_field_index = StudioReportMissingFieldIndex;
    std::uint32_t top_memo_block_number = 0;
    int width = 0;
    std::size_t width_field_index = StudioReportMissingFieldIndex;
    std::uint32_t width_memo_block_number = 0;
    int right = 0;
    int height = 0;
    std::size_t height_field_index = StudioReportMissingFieldIndex;
    std::uint32_t height_memo_block_number = 0;
    int bottom = 0;
    std::vector<StudioNamedValue> highlights{};
};

struct StudioReportSectionSnapshot {
    std::string id{};
    std::size_t id_field_index = StudioReportMissingFieldIndex;
    std::uint32_t id_memo_block_number = 0;
    std::string title{};
    std::size_t title_field_index = StudioReportMissingFieldIndex;
    std::uint32_t title_memo_block_number = 0;
    std::string band_kind{};
    std::size_t band_kind_field_index = StudioReportMissingFieldIndex;
    std::uint32_t band_kind_memo_block_number = 0;
    std::string expression{};
    std::size_t expression_field_index = StudioReportMissingFieldIndex;
    std::uint32_t expression_memo_block_number = 0;
    std::size_t record_index = 0;
    bool deleted = false;
    std::size_t section_index = StudioReportMissingRecordIndex;
    std::size_t section_count = 0;
    std::size_t deleted_object_count = 0;
    bool grouping_context_available = false;
    std::size_t grouping_index = StudioReportMissingRecordIndex;
    std::size_t grouping_nesting_depth = StudioReportMissingRecordIndex;
    std::string grouping_role{};
    std::string grouping_expression{};
    std::size_t grouping_expression_field_index = StudioReportMissingFieldIndex;
    std::uint32_t grouping_expression_memo_block_number = 0;
    std::string grouping_partner_section_id{};
    std::size_t grouping_partner_record_index = StudioReportMissingRecordIndex;
    bool grouping_partner_deleted = false;
    int objcode_code = 0;
    std::size_t objcode_field_index = StudioReportMissingFieldIndex;
    std::uint32_t objcode_memo_block_number = 0;
    int top = 0;
    std::size_t top_field_index = StudioReportMissingFieldIndex;
    std::uint32_t top_memo_block_number = 0;
    int height = 0;
    std::size_t height_field_index = StudioReportMissingFieldIndex;
    std::uint32_t height_memo_block_number = 0;
    int bottom = 0;
    std::string page_break{};
    std::size_t page_break_field_index = StudioReportMissingFieldIndex;
    std::uint32_t page_break_memo_block_number = 0;
    std::string column_break{};
    std::size_t column_break_field_index = StudioReportMissingFieldIndex;
    std::uint32_t column_break_memo_block_number = 0;
    std::string reset_page{};
    std::size_t reset_page_field_index = StudioReportMissingFieldIndex;
    std::uint32_t reset_page_memo_block_number = 0;
    std::vector<StudioLayoutObjectSnapshot> objects{};
};

struct StudioReportKindCount {
    std::string kind{};
    std::size_t count = 0;
};

struct StudioReportGroupingSnapshot {
    std::size_t grouping_index = 0;
    std::size_t nesting_depth = 0;
    std::string expression{};
    std::size_t expression_field_index = StudioReportMissingFieldIndex;
    std::uint32_t expression_memo_block_number = 0;
    std::string header_section_id{};
    std::size_t header_record_index = StudioReportMissingRecordIndex;
    bool header_deleted = false;
    std::string footer_section_id{};
    std::size_t footer_record_index = StudioReportMissingRecordIndex;
    bool footer_deleted = false;
};

struct StudioReportLayoutSnapshot {
    bool available = false;
    bool is_label = false;
    std::string document_title{};
    std::size_t document_title_field_index = StudioReportMissingFieldIndex;
    std::uint32_t document_title_memo_block_number = 0;
    bool preview_bounds_available = false;
    int preview_bounds_left = 0;
    int preview_bounds_top = 0;
    int preview_bounds_right = 0;
    int preview_bounds_bottom = 0;
    int preview_bounds_width = 0;
    int preview_bounds_height = 0;
    bool deleted_preview_bounds_available = false;
    int deleted_preview_bounds_left = 0;
    int deleted_preview_bounds_top = 0;
    int deleted_preview_bounds_right = 0;
    int deleted_preview_bounds_bottom = 0;
    int deleted_preview_bounds_width = 0;
    int deleted_preview_bounds_height = 0;
    bool page_setup_available = false;
    bool orientation_available = false;
    int orientation_code = 0;
    bool paper_size_available = false;
    int paper_size_code = 0;
    bool paper_length_available = false;
    int paper_length = 0;
    bool paper_width_available = false;
    int paper_width = 0;
    bool top_margin_available = false;
    int top_margin = 0;
    bool bottom_margin_available = false;
    int bottom_margin = 0;
    bool left_margin_available = false;
    int left_margin = 0;
    bool right_margin_available = false;
    int right_margin = 0;
    bool grid_vertical_available = false;
    int grid_vertical = 0;
    bool grid_horizontal_available = false;
    int grid_horizontal = 0;
    bool color_available = false;
    int color = 0;
    bool copies_available = false;
    int copies = 0;
    bool driver_available = false;
    std::string driver{};
    bool device_available = false;
    std::string device{};
    bool output_available = false;
    std::string output{};
    bool default_source_available = false;
    int default_source = 0;
    bool print_quality_available = false;
    int print_quality = 0;
    bool y_resolution_available = false;
    int y_resolution = 0;
    bool true_type_option_available = false;
    int true_type_option = 0;
    bool ascii_available = false;
    int ascii = 0;
    bool collate_available = false;
    int collate = 0;
    bool column_setup_available = false;
    bool column_count_available = false;
    int column_count = 0;
    bool column_width_available = false;
    int column_width = 0;
    bool column_spacing_available = false;
    int column_spacing = 0;
    bool sort_expression_available = false;
    std::string sort_expression{};
    std::size_t live_object_count = 0;
    std::size_t placed_object_count = 0;
    std::size_t deleted_placed_object_count = 0;
    std::size_t deleted_unplaced_object_count = 0;
    int section_height_total = 0;
    int deleted_section_height_total = 0;
    std::vector<StudioReportKindCount> object_kind_counts{};
    std::vector<StudioReportKindCount> unplaced_object_kind_counts{};
    std::vector<StudioReportKindCount> deleted_object_kind_counts{};
    std::vector<StudioReportKindCount> section_kind_counts{};
    std::vector<StudioReportKindCount> deleted_section_kind_counts{};
    std::vector<StudioReportGroupingSnapshot> groupings{};
    std::vector<StudioNamedValue> settings{};
    std::vector<StudioNamedValue> deleted_settings{};
    std::vector<StudioReportSectionSnapshot> sections{};
    std::vector<StudioReportSectionSnapshot> deleted_sections{};
    std::vector<StudioLayoutObjectSnapshot> unplaced_objects{};
    std::vector<StudioLayoutObjectSnapshot> deleted_objects{};
};

[[nodiscard]] StudioReportLayoutSnapshot build_report_layout(const StudioDocumentModel& document);
[[nodiscard]] StudioReportLayoutSnapshot build_report_layout(
    const StudioDocumentModel& document,
    const localization::LocalizedCatalog& catalog);

}  // namespace copperfin::studio
