#pragma once

#include "copperfin/studio/document_model.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

inline constexpr std::size_t StudioReportMissingFieldIndex = static_cast<std::size_t>(-1);

struct StudioNamedValue {
    std::string name{};
    std::size_t record_index = 0;
    std::size_t field_index = StudioReportMissingFieldIndex;
    std::string value{};
};

struct StudioLayoutObjectSnapshot {
    std::size_t record_index = 0;
    int objtype_code = 0;
    int objcode_code = 0;
    std::string object_kind{};
    std::size_t objtype_field_index = StudioReportMissingFieldIndex;
    std::size_t objcode_field_index = StudioReportMissingFieldIndex;
    std::string title{};
    std::string expression{};
    std::size_t expression_field_index = StudioReportMissingFieldIndex;
    int left = 0;
    std::size_t left_field_index = StudioReportMissingFieldIndex;
    int top = 0;
    std::size_t top_field_index = StudioReportMissingFieldIndex;
    int width = 0;
    std::size_t width_field_index = StudioReportMissingFieldIndex;
    int height = 0;
    std::size_t height_field_index = StudioReportMissingFieldIndex;
    std::vector<StudioNamedValue> highlights{};
};

struct StudioReportSectionSnapshot {
    std::string id{};
    std::string title{};
    std::string band_kind{};
    std::size_t record_index = 0;
    int objcode_code = 0;
    std::size_t objcode_field_index = StudioReportMissingFieldIndex;
    int top = 0;
    std::size_t top_field_index = StudioReportMissingFieldIndex;
    int height = 0;
    std::size_t height_field_index = StudioReportMissingFieldIndex;
    std::vector<StudioLayoutObjectSnapshot> objects{};
};

struct StudioReportLayoutSnapshot {
    bool available = false;
    bool is_label = false;
    std::string document_title{};
    std::vector<StudioNamedValue> settings{};
    std::vector<StudioReportSectionSnapshot> sections{};
    std::vector<StudioLayoutObjectSnapshot> unplaced_objects{};
};

[[nodiscard]] StudioReportLayoutSnapshot build_report_layout(const StudioDocumentModel& document);

}  // namespace copperfin::studio
