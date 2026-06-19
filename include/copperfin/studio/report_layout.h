#pragma once

#include "copperfin/studio/document_model.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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
    std::size_t record_index = 0;
    bool deleted = false;
    std::size_t section_index = StudioReportMissingRecordIndex;
    std::size_t section_count = 0;
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
    std::vector<StudioLayoutObjectSnapshot> objects{};
};

struct StudioReportLayoutSnapshot {
    bool available = false;
    bool is_label = false;
    std::string document_title{};
    std::size_t document_title_field_index = StudioReportMissingFieldIndex;
    std::uint32_t document_title_memo_block_number = 0;
    std::vector<StudioNamedValue> settings{};
    std::vector<StudioNamedValue> deleted_settings{};
    std::vector<StudioReportSectionSnapshot> sections{};
    std::vector<StudioReportSectionSnapshot> deleted_sections{};
    std::vector<StudioLayoutObjectSnapshot> unplaced_objects{};
    std::vector<StudioLayoutObjectSnapshot> deleted_objects{};
};

[[nodiscard]] StudioReportLayoutSnapshot build_report_layout(const StudioDocumentModel& document);

}  // namespace copperfin::studio
