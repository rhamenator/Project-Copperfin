// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/studio/report_layout.h"

#include "copperfin/localization/localization.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "copperfin/vfp/report_layout_records.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace copperfin::studio {

namespace {

using vfp::DbfRecord;

localization::LocalizedCatalog report_layout_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(
            localization::resolve_catalog_root(),
            localization::default_locale)};
    const std::filesystem::path locale_root = localization::resolve_catalog_root();
    const std::string locale = localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string report_layout_text(
    const localization::LocalizedCatalog& catalog,
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
    return catalog.translate(key, placeholders);
}

std::string trim_copy(std::string text) {
    const auto first = std::find_if(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    text.erase(text.begin(), first);

    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }

    return text;
}

bool equals_ignore_case(std::string_view left, std::string_view right) {
    return left.size() == right.size() &&
           std::equal(left.begin(), left.end(), right.begin(), [](unsigned char lhs, unsigned char rhs) {
               return std::toupper(lhs) == std::toupper(rhs);
           });
}

bool physical_field_name_matches(std::string_view physical_name, std::string_view logical_name) {
    if (equals_ignore_case(physical_name, logical_name)) {
        return true;
    }

    constexpr std::size_t vfp_free_table_field_name_max_bytes = 10U;
    return logical_name.size() > vfp_free_table_field_name_max_bytes &&
           physical_name.size() == vfp_free_table_field_name_max_bytes &&
           equals_ignore_case(
               physical_name,
               logical_name.substr(0U, vfp_free_table_field_name_max_bytes));
}

const vfp::DbfRecordValue* find_value(const DbfRecord& record, std::string_view field_name) {
    for (const auto& value : record.values) {
        if (physical_field_name_matches(value.field_name, field_name)) {
            return &value;
        }
    }
    return nullptr;
}

bool looks_like_unresolved_memo(const std::string& value) {
    return value.rfind("<memo block ", 0) == 0;
}

struct FieldSelection {
    std::string value{};
    std::size_t field_index = StudioReportMissingFieldIndex;
    std::uint32_t memo_block_number = 0;
};

std::optional<std::size_t> find_field_index(const DbfRecord& record, std::string_view field_name) {
    for (std::size_t index = 0U; index < record.values.size(); ++index) {
        if (physical_field_name_matches(record.values[index].field_name, field_name)) {
            return index;
        }
    }
    return std::nullopt;
}

std::size_t field_index_or_missing(const DbfRecord& record, std::string_view field_name) {
    return find_field_index(record, field_name).value_or(StudioReportMissingFieldIndex);
}

std::uint32_t memo_block_number_or_zero(const DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    return value == nullptr ? 0U : value->memo_block_number;
}

std::string value_or_empty(const DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    if (value == nullptr || looks_like_unresolved_memo(value->display_value)) {
        return {};
    }
    return value->display_value;
}

std::optional<int> parse_scaled_int(const DbfRecord& record, std::string_view field_name) {
    const std::string raw = trim_copy(value_or_empty(record, field_name));
    if (raw.empty()) {
        return std::nullopt;
    }

    const auto dot = raw.find('.');
    const std::string integer_portion = dot == std::string::npos ? raw : raw.substr(0U, dot);
    if (integer_portion.empty()) {
        return std::nullopt;
    }

    int value = 0;
    const auto [ptr, ec] = std::from_chars(integer_portion.data(), integer_portion.data() + integer_portion.size(), value);
    if (ec != std::errc() || ptr != (integer_portion.data() + integer_portion.size())) {
        return std::nullopt;
    }

    return vfp::truncate_report_layout_geometry(static_cast<double>(value));
}

int parse_scaled_int_or_default(const DbfRecord& record, std::string_view field_name, int fallback = 0) {
    const auto parsed = parse_scaled_int(record, field_name);
    return parsed.value_or(fallback);
}

std::optional<int> parse_named_value_int(const StudioNamedValue& named_value) {
    const std::string raw = trim_copy(named_value.value);
    if (raw.empty()) {
        return std::nullopt;
    }

    const auto dot = raw.find('.');
    const std::string integer_portion = dot == std::string::npos ? raw : raw.substr(0U, dot);
    if (integer_portion.empty()) {
        return std::nullopt;
    }

    int parsed_value = 0;
    const auto [ptr, ec] = std::from_chars(
        integer_portion.data(),
        integer_portion.data() + integer_portion.size(),
        parsed_value);
    if (ec != std::errc() || ptr != integer_portion.data() + integer_portion.size()) {
        return std::nullopt;
    }

    return parsed_value;
}

std::string band_kind_name(int objcode) {
    switch (objcode) {
        case 0:
            return "title";
        case 1:
            return "page_header";
        case 2:
            return "column_header";
        case 3:
            return "group_header";
        case 4:
            return "detail";
        case 5:
            return "group_footer";
        case 6:
            return "column_footer";
        case 7:
            return "page_footer";
        case 8:
            return "summary";
        case 9:
            return "detail_header";
        case 10:
            return "detail_footer";
        default:
            return "other";
    }
}

std::string band_title(int objcode, const localization::LocalizedCatalog& catalog) {
    switch (objcode) {
        case 0:
            return catalog.translate("Studio.ReportLayout.Section.Title");
        case 1:
            return catalog.translate("Studio.ReportLayout.Section.PageHeader");
        case 2:
            return catalog.translate("Studio.ReportLayout.Section.ColumnHeader");
        case 3:
            return catalog.translate("Studio.ReportLayout.Section.GroupHeader");
        case 4:
            return catalog.translate("Studio.ReportLayout.Section.Detail");
        case 5:
            return catalog.translate("Studio.ReportLayout.Section.GroupFooter");
        case 6:
            return catalog.translate("Studio.ReportLayout.Section.ColumnFooter");
        case 7:
            return catalog.translate("Studio.ReportLayout.Section.PageFooter");
        case 8:
            return catalog.translate("Studio.ReportLayout.Section.Summary");
        case 9:
            return catalog.translate("Studio.ReportLayout.Section.DetailHeader");
        case 10:
            return catalog.translate("Studio.ReportLayout.Section.DetailFooter");
        default:
            return catalog.translate("Studio.ReportLayout.Section.OtherBand");
    }
}

std::string object_kind_name(int objtype) {
    switch (objtype) {
        case 5:
            return "label";
        case 6:
            return "line";
        case 7:
            return "rectangle";
        case 8:
            return "field";
        case 9:
            return "band";
        case 10:
            return "group";
        case 17:
            return "picture";
        case 18:
            return "variable";
        default:
            return "object";
    }
}

std::string first_non_empty(const DbfRecord& record, std::initializer_list<std::string_view> field_names) {
    for (const auto field_name : field_names) {
        const std::string value = trim_copy(value_or_empty(record, field_name));
        if (!value.empty()) {
            return value;
        }
    }
    return {};
}

FieldSelection first_non_empty_selection(const DbfRecord& record, std::initializer_list<std::string_view> field_names) {
    for (const auto field_name : field_names) {
        const std::string value = trim_copy(value_or_empty(record, field_name));
        if (!value.empty()) {
            return {
                .value = value,
                .field_index = field_index_or_missing(record, field_name),
                .memo_block_number = memo_block_number_or_zero(record, field_name)
            };
        }
    }
    return {};
}

StudioLayoutObjectSnapshot build_layout_object(
    const DbfRecord& record,
    const localization::LocalizedCatalog& catalog) {
    StudioLayoutObjectSnapshot object;
    object.record_index = record.record_index;
    object.deleted = record.deleted;
    object.objtype_code = parse_scaled_int_or_default(record, "OBJTYPE");
    object.objcode_code = parse_scaled_int_or_default(record, "OBJCODE");
    object.object_kind = object_kind_name(object.objtype_code);
    object.objtype_field_index = field_index_or_missing(record, "OBJTYPE");
    object.objtype_memo_block_number = memo_block_number_or_zero(record, "OBJTYPE");
    object.object_kind_field_index = object.objtype_field_index;
    object.object_kind_memo_block_number = object.objtype_memo_block_number;
    object.objcode_field_index = field_index_or_missing(record, "OBJCODE");
    object.objcode_memo_block_number = memo_block_number_or_zero(record, "OBJCODE");
    const FieldSelection title = first_non_empty_selection(record, {"NAME", "EXPR", "UNIQUEID"});
    object.title = title.value;
    object.title_field_index = title.field_index;
    object.title_memo_block_number = title.memo_block_number;
    object.expression = first_non_empty(record, {"EXPR"});
    object.expression_field_index = field_index_or_missing(record, "EXPR");
    object.expression_memo_block_number = memo_block_number_or_zero(record, "EXPR");
    if (object.objtype_code == 5 || object.objtype_code == 8 || object.objtype_code == 17) {
        object.picture = value_or_empty(record, "PICTURE");
        object.picture_field_index = field_index_or_missing(record, "PICTURE");
        object.picture_memo_block_number = memo_block_number_or_zero(record, "PICTURE");
    }
    object.left = parse_scaled_int_or_default(record, "HPOS");
    object.left_field_index = field_index_or_missing(record, "HPOS");
    object.left_memo_block_number = memo_block_number_or_zero(record, "HPOS");
    object.top = parse_scaled_int_or_default(record, "VPOS");
    object.top_field_index = field_index_or_missing(record, "VPOS");
    object.top_memo_block_number = memo_block_number_or_zero(record, "VPOS");
    object.width = std::max(0, parse_scaled_int_or_default(record, "WIDTH"));
    object.width_field_index = field_index_or_missing(record, "WIDTH");
    object.width_memo_block_number = memo_block_number_or_zero(record, "WIDTH");
    object.right = object.left + object.width;
    object.height = std::max(0, parse_scaled_int_or_default(record, "HEIGHT"));
    object.height_field_index = field_index_or_missing(record, "HEIGHT");
    object.height_memo_block_number = memo_block_number_or_zero(record, "HEIGHT");
    object.bottom = object.top + object.height;

    if (object.title.empty()) {
        object.title = report_layout_text(
            catalog,
            "Studio.ReportLayout.Fallback.RecordTitle",
            {{"recordIndex", std::to_string(record.record_index)}});
    }

    const auto add_highlight = [&](std::string_view name) {
        const std::string value = first_non_empty(record, {name});
        if (!value.empty()) {
            object.highlights.push_back({
                .name = std::string(name),
                .record_index = record.record_index,
                .field_index = field_index_or_missing(record, name),
                .memo_block_number = memo_block_number_or_zero(record, name),
                .value = value
            });
        }
    };

    if (object.objtype_code == 5 || object.objtype_code == 8 || object.objtype_code == 17) {
        add_highlight("PICTURE");
    }
    if (object.objtype_code == 8) {
        add_highlight("RULERLINES");
        add_highlight("OFFSET");
        add_highlight("FILLCHAR");
        add_highlight("TOTALTYPE");
        add_highlight("RESETTOTAL");
    }
    if (object.objtype_code == 5) {
        add_highlight("SPACING");
    }
    if (object.objtype_code == 17) {
        add_highlight("GENERAL");
        add_highlight("OFFSET");
    }
    if (object.objtype_code == 5 || object.objtype_code == 8) {
        add_highlight("PENRED");
        add_highlight("PENGREEN");
        add_highlight("PENBLUE");
    }
    if (object.objtype_code == 8) {
        add_highlight("FILLRED");
        add_highlight("FILLGREEN");
        add_highlight("FILLBLUE");
    }
    if (object.objtype_code == 6 || object.objtype_code == 7) {
        add_highlight("PENSIZE");
        add_highlight("PENPAT");
    }
    if (object.objtype_code == 7) {
        add_highlight("FILLPAT");
    }
    if (object.objtype_code == 1 || object.objtype_code == 5 || object.objtype_code == 8 ||
        object.objtype_code == 17) {
        add_highlight("DOUBLE");
    }
    if (object.objtype_code == 1 || object.objtype_code == 5 || object.objtype_code == 8) {
        add_highlight("RESOID");
    }
    if (object.objtype_code == 5 || object.objtype_code == 6 || object.objtype_code == 7 ||
        object.objtype_code == 8 || object.objtype_code == 10 || object.objtype_code == 17) {
        add_highlight("TAG2");
    }
    add_highlight("EXPR");
    add_highlight("SUPEXPR");
    add_highlight("SUPGROUP");
    add_highlight("SUPALWAYS");
    add_highlight("SUPVALCHNG");
    add_highlight("SUPRPCOL");
    add_highlight("SUPOVFLOW");
    add_highlight("BOTTOM");
    add_highlight("TOP");
    add_highlight("FONTFACE");
    add_highlight("FONTSTYLE");
    add_highlight("FONTSIZE");
    add_highlight("MODE");

    return object;
}

bool is_report_family(const StudioDocumentModel& document) {
    return document.kind == StudioAssetKind::report || document.kind == StudioAssetKind::label;
}

bool is_layout_object_type(int objtype) {
    return objtype == 5 || objtype == 6 || objtype == 7 || objtype == 8 || objtype == 17 || objtype == 18;
}

bool is_report_root_record(const DbfRecord& record) {
    return vfp::is_report_settings_root_record(parse_scaled_int_or_default(record, "OBJTYPE"));
}

FieldSelection document_title_source(const std::vector<DbfRecord>& records) {
    const DbfRecord* deleted_root = nullptr;
    for (const auto& record : records) {
        if (!is_report_root_record(record)) {
            continue;
        }
        if (record.deleted) {
            if (deleted_root == nullptr) {
                deleted_root = &record;
            }
            continue;
        }

        const std::size_t field_index = field_index_or_missing(record, "NAME");
        if (field_index != StudioReportMissingFieldIndex) {
            return {
                .value = trim_copy(value_or_empty(record, "NAME")),
                .field_index = field_index,
                .memo_block_number = memo_block_number_or_zero(record, "NAME")
            };
        }
        return {};
    }

    if (deleted_root != nullptr) {
        const std::size_t field_index = field_index_or_missing(*deleted_root, "NAME");
        if (field_index != StudioReportMissingFieldIndex) {
            return {
                .value = trim_copy(value_or_empty(*deleted_root, "NAME")),
                .field_index = field_index,
                .memo_block_number = memo_block_number_or_zero(*deleted_root, "NAME")
            };
        }
    }
    return {};
}

bool is_band_record(const DbfRecord& record) {
    return parse_scaled_int_or_default(record, "OBJTYPE") == 9;
}

void append_report_settings(
    const DbfRecord& record,
    std::vector<StudioNamedValue>& settings,
    std::string_view memo_sidecar_path = {}) {
    const auto append_name_value_pairs = [&](std::string_view field_name, bool replace_existing) {
        const std::string blob = value_or_empty(record, field_name);
        const std::size_t field_index = field_index_or_missing(record, field_name);
        const std::uint32_t memo_block_number = memo_block_number_or_zero(record, field_name);
        std::size_t start = 0U;
        std::size_t line_index = 0U;
        while (start <= blob.size()) {
            const std::size_t end = blob.find_first_of("\r\n", start);
            std::string line = end == std::string::npos ? blob.substr(start) : blob.substr(start, end - start);

            const auto equals = line.find('=');
            if (equals != std::string::npos) {
                const std::string name = trim_copy(line.substr(0U, equals));
                const std::string value = trim_copy(line.substr(equals + 1U));
                if (!name.empty()) {
                    StudioNamedValue setting{
                        .name = name,
                        .record_index = record.record_index,
                        .field_index = field_index,
                        .source_line_index = line_index,
                        .memo_block_number = memo_block_number,
                        .value = value
                    };
                    if (replace_existing) {
                        const auto existing = std::find_if(
                            settings.begin(),
                            settings.end(),
                            [&](const StudioNamedValue& candidate) {
                                return equals_ignore_case(candidate.name, name);
                            });
                        if (existing != settings.end()) {
                            *existing = std::move(setting);
                            auto duplicate = std::find_if(
                                std::next(existing),
                                settings.end(),
                                [&](const StudioNamedValue& candidate) {
                                    return equals_ignore_case(candidate.name, name);
                                });
                            while (duplicate != settings.end()) {
                                duplicate = settings.erase(duplicate);
                                duplicate = std::find_if(
                                    duplicate,
                                    settings.end(),
                                    [&](const StudioNamedValue& candidate) {
                                        return equals_ignore_case(candidate.name, name);
                                    });
                            }
                        } else {
                            settings.push_back(std::move(setting));
                        }
                    } else {
                        settings.push_back(std::move(setting));
                    }
                }
            }

            if (end == std::string::npos) {
                break;
            }
            start = end + 1U;
            if (start < blob.size() &&
                ((blob[end] == '\r' && blob[start] == '\n') || (blob[end] == '\n' && blob[start] == '\r'))) {
                ++start;
            }
            ++line_index;
        }
    };

    append_name_value_pairs("EXPR", false);

    const auto append_direct_setting = [&](std::string_view field_name) {
        std::string value = trim_copy(value_or_empty(record, field_name));
        if (equals_ignore_case(field_name, "ORDER")) {
            const auto* order_field = find_value(record, field_name);
            if (order_field != nullptr && order_field->memo_block_number > 0U && !memo_sidecar_path.empty()) {
                const auto raw_value = vfp::read_memo_block_raw(
                    std::string(memo_sidecar_path),
                    order_field->memo_block_number);
                if (!raw_value.empty()) {
                    value = vfp::encode_report_order_value(std::string_view(
                        reinterpret_cast<const char*>(raw_value.data()),
                        raw_value.size()));
                }
            } else if (!value.empty() && value != "?") {
                value = vfp::encode_report_order_value(value);
            }
        }
        if (!value.empty() && value != "?") {
            settings.push_back({
                .name = std::string(field_name),
                .record_index = record.record_index,
                .field_index = field_index_or_missing(record, field_name),
                .memo_block_number = memo_block_number_or_zero(record, field_name),
                .value = value
            });
        }
    };

    append_direct_setting("ORIENTATION");
    append_direct_setting("PAPERSIZE");
    append_direct_setting("PAPERLENGTH");
    append_direct_setting("PAPERWIDTH");
    append_direct_setting("TOPMARGIN");
    append_direct_setting("BOTMARGIN");
    append_direct_setting("LEFTMARGIN");
    append_direct_setting("RIGHTMARGIN");
    append_direct_setting("GRIDV");
    append_direct_setting("GRIDH");
    append_direct_setting("COLOR");
    append_direct_setting("COPIES");
    append_direct_setting("COLS");
    append_direct_setting("COLWIDTH");
    append_direct_setting("COLSPACING");
    append_direct_setting("GRID");
    append_direct_setting("RULER");
    append_direct_setting("TAG");
    append_direct_setting("DOUBLE");
    append_direct_setting("RESOID");
    append_direct_setting("RULERLINES");
    append_direct_setting("ADDALIAS");
    append_direct_setting("CURPOS");
    append_direct_setting("UNIQUE");
    append_direct_setting("ORDER");
    append_name_value_pairs("PICTURE", true);
}

std::string make_section_id(std::size_t record_index, int objcode) {
    return band_kind_name(objcode) + "_" + std::to_string(record_index);
}

std::size_t find_section_index(
    const std::vector<StudioReportSectionSnapshot>& sections,
    int top,
    int height) {
    std::size_t best_index = sections.size();
    for (std::size_t index = 0; index < sections.size(); ++index) {
        const auto& section = sections[index];
        const int section_bottom = section.top + std::max(section.height, 1);
        const int object_bottom = top + std::max(height, 1);
        const bool begins_inside = top >= section.top && top < section_bottom;
        const bool overlaps = object_bottom > section.top && top < section_bottom;
        if (begins_inside || overlaps) {
            if (best_index >= sections.size()) {
                best_index = index;
                continue;
            }

            const auto& best_section = sections[best_index];
            const int best_bottom = best_section.top + std::max(best_section.height, 1);
            const bool best_begins_inside = top >= best_section.top && top < best_bottom;
            if (begins_inside != best_begins_inside) {
                if (begins_inside) {
                    best_index = index;
                }
                continue;
            }

            if (section.top > best_section.top ||
                (section.top == best_section.top && section_bottom > best_bottom) ||
                (section.top == best_section.top && section_bottom == best_bottom &&
                 section.record_index > best_section.record_index)) {
                best_index = index;
            }
        }
    }
    return best_index;
}

void expand_preview_bounds(StudioReportLayoutSnapshot& snapshot, int left, int top, int right, int bottom) {
    if (!snapshot.preview_bounds_available) {
        snapshot.preview_bounds_available = true;
        snapshot.preview_bounds_left = left;
        snapshot.preview_bounds_top = top;
        snapshot.preview_bounds_right = right;
        snapshot.preview_bounds_bottom = bottom;
        return;
    }

    snapshot.preview_bounds_left = std::min(snapshot.preview_bounds_left, left);
    snapshot.preview_bounds_top = std::min(snapshot.preview_bounds_top, top);
    snapshot.preview_bounds_right = std::max(snapshot.preview_bounds_right, right);
    snapshot.preview_bounds_bottom = std::max(snapshot.preview_bounds_bottom, bottom);
}

void expand_deleted_preview_bounds(StudioReportLayoutSnapshot& snapshot, int left, int top, int right, int bottom) {
    if (!snapshot.deleted_preview_bounds_available) {
        snapshot.deleted_preview_bounds_available = true;
        snapshot.deleted_preview_bounds_left = left;
        snapshot.deleted_preview_bounds_top = top;
        snapshot.deleted_preview_bounds_right = right;
        snapshot.deleted_preview_bounds_bottom = bottom;
        return;
    }

    snapshot.deleted_preview_bounds_left = std::min(snapshot.deleted_preview_bounds_left, left);
    snapshot.deleted_preview_bounds_top = std::min(snapshot.deleted_preview_bounds_top, top);
    snapshot.deleted_preview_bounds_right = std::max(snapshot.deleted_preview_bounds_right, right);
    snapshot.deleted_preview_bounds_bottom = std::max(snapshot.deleted_preview_bounds_bottom, bottom);
}

void finalize_preview_bounds(StudioReportLayoutSnapshot& snapshot) {
    snapshot.preview_bounds_width = std::max(0, snapshot.preview_bounds_right - snapshot.preview_bounds_left);
    snapshot.preview_bounds_height = std::max(0, snapshot.preview_bounds_bottom - snapshot.preview_bounds_top);
    snapshot.deleted_preview_bounds_width =
        std::max(0, snapshot.deleted_preview_bounds_right - snapshot.deleted_preview_bounds_left);
    snapshot.deleted_preview_bounds_height =
        std::max(0, snapshot.deleted_preview_bounds_bottom - snapshot.deleted_preview_bounds_top);
}

void finalize_page_setup_summary(StudioReportLayoutSnapshot& snapshot) {
    const auto& summary_settings = snapshot.settings.empty() ? snapshot.deleted_settings : snapshot.settings;

    const auto apply_setting = [&](std::string_view setting_name, auto assign_setting) {
        const auto setting = std::find_if(
            summary_settings.begin(),
            summary_settings.end(),
            [&](const StudioNamedValue& named_value) {
                return equals_ignore_case(named_value.name, setting_name);
            });
        if (setting == summary_settings.end()) {
            return;
        }

        const auto parsed_value = parse_named_value_int(*setting);
        if (!parsed_value.has_value()) {
            return;
        }

        assign_setting(*parsed_value);
    };

    const auto apply_string_setting = [&](std::string_view setting_name, auto assign_setting) {
        const auto setting = std::find_if(
            summary_settings.begin(),
            summary_settings.end(),
            [&](const StudioNamedValue& named_value) {
                return equals_ignore_case(named_value.name, setting_name);
            });
        if (setting == summary_settings.end()) {
            return;
        }

        const std::string value = trim_copy(setting->value);
        if (value.empty()) {
            return;
        }

        assign_setting(value);
    };

    apply_setting("ORIENTATION", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.orientation_available = true;
        snapshot.orientation_code = value;
    });
    apply_setting("PAPERSIZE", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.paper_size_available = true;
        snapshot.paper_size_code = value;
    });
    apply_setting("PAPERLENGTH", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.paper_length_available = true;
        snapshot.paper_length = value;
    });
    apply_setting("PAPERWIDTH", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.paper_width_available = true;
        snapshot.paper_width = value;
    });
    apply_setting("TOPMARGIN", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.top_margin_available = true;
        snapshot.top_margin = value;
    });
    apply_setting("BOTMARGIN", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.bottom_margin_available = true;
        snapshot.bottom_margin = value;
    });
    apply_setting("LEFTMARGIN", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.left_margin_available = true;
        snapshot.left_margin = value;
    });
    apply_setting("RIGHTMARGIN", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.right_margin_available = true;
        snapshot.right_margin = value;
    });
    apply_setting("GRIDV", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.grid_vertical_available = true;
        snapshot.grid_vertical = value;
    });
    apply_setting("GRIDH", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.grid_horizontal_available = true;
        snapshot.grid_horizontal = value;
    });
    apply_setting("COLOR", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.color_available = true;
        snapshot.color = value;
    });
    apply_setting("COPIES", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.copies_available = true;
        snapshot.copies = value;
    });
    apply_string_setting("DRIVER", [&](const std::string& value) {
        snapshot.page_setup_available = true;
        snapshot.driver_available = true;
        snapshot.driver = value;
    });
    apply_string_setting("DEVICE", [&](const std::string& value) {
        snapshot.page_setup_available = true;
        snapshot.device_available = true;
        snapshot.device = value;
    });
    apply_string_setting("OUTPUT", [&](const std::string& value) {
        snapshot.page_setup_available = true;
        snapshot.output_available = true;
        snapshot.output = value;
    });
    apply_setting("DEFAULTSOURCE", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.default_source_available = true;
        snapshot.default_source = value;
    });
    apply_setting("PRINTQUALITY", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.print_quality_available = true;
        snapshot.print_quality = value;
    });
    apply_setting("YRESOLUTION", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.y_resolution_available = true;
        snapshot.y_resolution = value;
    });
    apply_setting("TTOPTION", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.true_type_option_available = true;
        snapshot.true_type_option = value;
    });
    apply_setting("ASCII", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.ascii_available = true;
        snapshot.ascii = value;
    });
    apply_setting("COLLATE", [&](int value) {
        snapshot.page_setup_available = true;
        snapshot.collate_available = true;
        snapshot.collate = value;
    });
    apply_setting("COLS", [&](int value) {
        snapshot.column_setup_available = true;
        snapshot.column_count_available = true;
        snapshot.column_count = value;
    });
    apply_setting("COLWIDTH", [&](int value) {
        snapshot.column_setup_available = true;
        snapshot.column_width_available = true;
        snapshot.column_width = value;
    });
    apply_setting("COLSPACING", [&](int value) {
        snapshot.column_setup_available = true;
        snapshot.column_spacing_available = true;
        snapshot.column_spacing = value;
    });
    apply_string_setting("TAG", [&](const std::string& value) {
        snapshot.sort_expression_available = true;
        snapshot.sort_expression = value;
    });
}

void increment_kind_count(std::vector<StudioReportKindCount>& counts, const std::string& kind) {
    const auto existing = std::find_if(
        counts.begin(),
        counts.end(),
        [&](const StudioReportKindCount& count) {
            return count.kind == kind;
        });
    if (existing != counts.end()) {
        ++existing->count;
        return;
    }
    counts.push_back({.kind = kind, .count = 1U});
}

void sort_kind_counts(std::vector<StudioReportKindCount>& counts) {
    std::sort(counts.begin(), counts.end(), [](const auto& left, const auto& right) {
        return left.kind < right.kind;
    });
}

void finalize_object_kind_counts(StudioReportLayoutSnapshot& snapshot) {
    for (const auto& section : snapshot.sections) {
        for (const auto& object : section.objects) {
            increment_kind_count(snapshot.object_kind_counts, object.object_kind);
        }
    }
    for (const auto& section : snapshot.deleted_sections) {
        for (const auto& object : section.objects) {
            increment_kind_count(snapshot.object_kind_counts, object.object_kind);
        }
    }
    for (const auto& object : snapshot.unplaced_objects) {
        increment_kind_count(snapshot.object_kind_counts, object.object_kind);
        increment_kind_count(snapshot.unplaced_object_kind_counts, object.object_kind);
    }
    for (const auto& object : snapshot.deleted_objects) {
        increment_kind_count(snapshot.deleted_object_kind_counts, object.object_kind);
    }

    sort_kind_counts(snapshot.object_kind_counts);
    sort_kind_counts(snapshot.unplaced_object_kind_counts);
    sort_kind_counts(snapshot.deleted_object_kind_counts);
}

void finalize_section_kind_counts(StudioReportLayoutSnapshot& snapshot) {
    for (const auto& section : snapshot.sections) {
        increment_kind_count(snapshot.section_kind_counts, section.band_kind);
        snapshot.section_height_total += section.height;
    }
    for (const auto& section : snapshot.deleted_sections) {
        increment_kind_count(snapshot.deleted_section_kind_counts, section.band_kind);
        snapshot.deleted_section_height_total += section.height;
    }

    sort_kind_counts(snapshot.section_kind_counts);
    sort_kind_counts(snapshot.deleted_section_kind_counts);
}

void finalize_groupings(StudioReportLayoutSnapshot& snapshot) {
    struct SectionRef {
        const StudioReportSectionSnapshot* section = nullptr;
    };

    std::vector<SectionRef> ordered_sections;
    ordered_sections.reserve(snapshot.sections.size() + snapshot.deleted_sections.size());
    for (const auto& section : snapshot.sections) {
        ordered_sections.push_back({.section = &section});
    }
    for (const auto& section : snapshot.deleted_sections) {
        ordered_sections.push_back({.section = &section});
    }

    std::sort(ordered_sections.begin(), ordered_sections.end(), [](const SectionRef& left, const SectionRef& right) {
        return left.section->record_index < right.section->record_index;
    });

    std::vector<std::size_t> open_group_stack;
    for (const auto& section_ref : ordered_sections) {
        const auto& section = *section_ref.section;
        if (section.band_kind == "group_header") {
            snapshot.groupings.push_back({
                .grouping_index = snapshot.groupings.size(),
                .nesting_depth = open_group_stack.size(),
                .expression = section.expression,
                .expression_field_index = section.expression_field_index,
                .expression_memo_block_number = section.expression_memo_block_number,
                .header_section_id = section.id,
                .header_record_index = section.record_index,
                .header_deleted = section.deleted
            });
            open_group_stack.push_back(snapshot.groupings.size() - 1U);
            continue;
        }

        if (section.band_kind != "group_footer") {
            continue;
        }

        if (open_group_stack.empty()) {
            snapshot.groupings.push_back({
                .grouping_index = snapshot.groupings.size(),
                .nesting_depth = 0U,
                .expression = section.expression,
                .expression_field_index = section.expression_field_index,
                .expression_memo_block_number = section.expression_memo_block_number,
                .footer_section_id = section.id,
                .footer_record_index = section.record_index,
                .footer_deleted = section.deleted
            });
            continue;
        }

        const std::size_t grouping_index = open_group_stack.back();
        open_group_stack.pop_back();
        auto& grouping = snapshot.groupings[grouping_index];
        if (grouping.expression.empty()) {
            grouping.expression = section.expression;
            grouping.expression_field_index = section.expression_field_index;
            grouping.expression_memo_block_number = section.expression_memo_block_number;
        }
        grouping.footer_section_id = section.id;
        grouping.footer_record_index = section.record_index;
        grouping.footer_deleted = section.deleted;
    }
}

void finalize_section_grouping_context(StudioReportLayoutSnapshot& snapshot) {
    const auto annotate_section = [](
        std::vector<StudioReportSectionSnapshot>& sections,
        std::size_t record_index,
        const StudioReportGroupingSnapshot& grouping,
        std::string role,
        std::string partner_section_id,
        std::size_t partner_record_index,
        bool partner_deleted) {
        const auto section = std::find_if(
            sections.begin(),
            sections.end(),
            [&](const StudioReportSectionSnapshot& candidate) {
                return candidate.record_index == record_index;
            });
        if (section == sections.end()) {
            return;
        }

        section->grouping_context_available = true;
        section->grouping_index = grouping.grouping_index;
        section->grouping_nesting_depth = grouping.nesting_depth;
        section->grouping_role = std::move(role);
        section->grouping_expression = grouping.expression;
        section->grouping_expression_field_index = grouping.expression_field_index;
        section->grouping_expression_memo_block_number = grouping.expression_memo_block_number;
        section->grouping_partner_section_id = std::move(partner_section_id);
        section->grouping_partner_record_index = partner_record_index;
        section->grouping_partner_deleted = partner_deleted;
    };

    for (const auto& grouping : snapshot.groupings) {
        if (grouping.header_record_index != StudioReportMissingRecordIndex) {
            auto& target_sections = grouping.header_deleted ? snapshot.deleted_sections : snapshot.sections;
            annotate_section(
                target_sections,
                grouping.header_record_index,
                grouping,
                "header",
                grouping.footer_section_id,
                grouping.footer_record_index,
                grouping.footer_deleted);
        }

        if (grouping.footer_record_index != StudioReportMissingRecordIndex) {
            auto& target_sections = grouping.footer_deleted ? snapshot.deleted_sections : snapshot.sections;
            annotate_section(
                target_sections,
                grouping.footer_record_index,
                grouping,
                "footer",
                grouping.header_section_id,
                grouping.header_record_index,
                grouping.header_deleted);
        }
    }
}

void assign_deleted_object_section_order(std::vector<StudioLayoutObjectSnapshot*>& objects) {
    std::sort(objects.begin(), objects.end(), [](const StudioLayoutObjectSnapshot* left,
                                                 const StudioLayoutObjectSnapshot* right) {
        if (left->top != right->top) {
            return left->top < right->top;
        }
        if (left->left != right->left) {
            return left->left < right->left;
        }
        return left->record_index < right->record_index;
    });

    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        objects[object_index]->section_object_index = object_index;
        objects[object_index]->section_object_count = objects.size();
    }
}

StudioReportSectionSnapshot build_report_section(
    const DbfRecord& record,
    const localization::LocalizedCatalog& catalog) {
    const int objcode = parse_scaled_int_or_default(record, "OBJCODE");
    const std::size_t objcode_field_index = field_index_or_missing(record, "OBJCODE");
    const std::uint32_t objcode_memo_block_number = memo_block_number_or_zero(record, "OBJCODE");
    const FieldSelection unique_id = first_non_empty_selection(record, {"UNIQUEID"});
    const FieldSelection expression = first_non_empty_selection(record, {"EXPR"});
    const FieldSelection page_break = first_non_empty_selection(record, {"PAGEBREAK"});
    const FieldSelection column_break = first_non_empty_selection(record, {"COLBREAK"});
    const FieldSelection reset_page = first_non_empty_selection(record, {"RESETPAGE"});
    const FieldSelection eject_before = first_non_empty_selection(record, {"EJECTBEFOR"});
    const FieldSelection eject_after = first_non_empty_selection(record, {"EJECTAFTER"});
    const FieldSelection plain = first_non_empty_selection(record, {"PLAIN"});
    const FieldSelection on_entry_expression = first_non_empty_selection(record, {"TAG"});
    const FieldSelection on_exit_expression = first_non_empty_selection(record, {"TAG2"});
    const int top = parse_scaled_int_or_default(record, "VPOS");
    const int height = std::max(0, parse_scaled_int_or_default(record, "HEIGHT"));
    return {
        .id = unique_id.value.empty() ? make_section_id(record.record_index, objcode) : unique_id.value,
        .id_field_index = unique_id.value.empty() ? StudioReportMissingFieldIndex : unique_id.field_index,
        .id_memo_block_number = unique_id.value.empty() ? 0U : unique_id.memo_block_number,
        .title = band_title(objcode, catalog),
        .title_field_index = objcode_field_index,
        .title_memo_block_number = objcode_memo_block_number,
        .band_kind = band_kind_name(objcode),
        .band_kind_field_index = objcode_field_index,
        .band_kind_memo_block_number = objcode_memo_block_number,
        .expression = expression.value,
        .expression_field_index = expression.field_index,
        .expression_memo_block_number = expression.memo_block_number,
        .record_index = record.record_index,
        .deleted = record.deleted,
        .objcode_code = objcode,
        .objcode_field_index = objcode_field_index,
        .objcode_memo_block_number = objcode_memo_block_number,
        .top = top,
        .top_field_index = field_index_or_missing(record, "VPOS"),
        .top_memo_block_number = memo_block_number_or_zero(record, "VPOS"),
        .height = height,
        .height_field_index = field_index_or_missing(record, "HEIGHT"),
        .height_memo_block_number = memo_block_number_or_zero(record, "HEIGHT"),
        .bottom = top + height,
        .page_break = page_break.value,
        .page_break_field_index = page_break.field_index,
        .page_break_memo_block_number = page_break.memo_block_number,
        .column_break = column_break.value,
        .column_break_field_index = column_break.field_index,
        .column_break_memo_block_number = column_break.memo_block_number,
        .reset_page = reset_page.value,
        .reset_page_field_index = reset_page.field_index,
        .reset_page_memo_block_number = reset_page.memo_block_number,
        .eject_before = eject_before.value,
        .eject_before_field_index = eject_before.field_index,
        .eject_before_memo_block_number = eject_before.memo_block_number,
        .eject_after = eject_after.value,
        .eject_after_field_index = eject_after.field_index,
        .eject_after_memo_block_number = eject_after.memo_block_number,
        .plain = plain.value,
        .plain_field_index = plain.field_index,
        .plain_memo_block_number = plain.memo_block_number,
        .on_entry_expression = on_entry_expression.value,
        .on_entry_expression_field_index = on_entry_expression.field_index,
        .on_entry_expression_memo_block_number = on_entry_expression.memo_block_number,
        .on_exit_expression = on_exit_expression.value,
        .on_exit_expression_field_index = on_exit_expression.field_index,
        .on_exit_expression_memo_block_number = on_exit_expression.memo_block_number
    };
}

}  // namespace

StudioReportLayoutSnapshot build_report_layout(
    const StudioDocumentModel& document,
    const localization::LocalizedCatalog& catalog) {
    StudioReportLayoutSnapshot snapshot;
    if (!is_report_family(document) || !document.table_preview_available) {
        return snapshot;
    }

    snapshot.available = true;
    snapshot.is_label = document.kind == StudioAssetKind::label;
    snapshot.document_title = document.display_name;
    const FieldSelection title_source = document_title_source(document.table_preview.records);
    snapshot.document_title_field_index = title_source.field_index;
    snapshot.document_title_memo_block_number = title_source.memo_block_number;

    for (const auto& record : document.table_preview.records) {
        if (record.deleted) {
            if (is_report_root_record(record)) {
                append_report_settings(record, snapshot.deleted_settings, document.sidecar_path);
            }
            if (is_band_record(record)) {
                StudioReportSectionSnapshot section = build_report_section(record, catalog);
                expand_deleted_preview_bounds(snapshot, 0, section.top, 0, section.bottom);
                snapshot.deleted_sections.push_back(std::move(section));
            }
            const int objtype = parse_scaled_int_or_default(record, "OBJTYPE");
            if (is_layout_object_type(objtype)) {
                StudioLayoutObjectSnapshot object = build_layout_object(record, catalog);
                expand_deleted_preview_bounds(snapshot, object.left, object.top, object.right, object.bottom);
                snapshot.deleted_objects.push_back(std::move(object));
            }
            continue;
        }

        if (is_report_root_record(record)) {
            append_report_settings(record, snapshot.settings, document.sidecar_path);
            continue;
        }

        if (is_band_record(record)) {
            snapshot.sections.push_back(build_report_section(record, catalog));
        }
    }

    std::sort(snapshot.sections.begin(), snapshot.sections.end(), [](const StudioReportSectionSnapshot& left,
                                                                     const StudioReportSectionSnapshot& right) {
        if (left.top != right.top) {
            return left.top < right.top;
        }
        return left.record_index < right.record_index;
    });
    for (std::size_t section_index = 0; section_index < snapshot.sections.size(); ++section_index) {
        snapshot.sections[section_index].section_index = section_index;
        snapshot.sections[section_index].section_count = snapshot.sections.size();
        expand_preview_bounds(
            snapshot,
            0,
            snapshot.sections[section_index].top,
            0,
            snapshot.sections[section_index].bottom);
    }

    for (const auto& record : document.table_preview.records) {
        if (record.deleted) {
            continue;
        }

        const int objtype = parse_scaled_int_or_default(record, "OBJTYPE");
        if (!is_layout_object_type(objtype)) {
            continue;
        }

        StudioLayoutObjectSnapshot object = build_layout_object(record, catalog);
        const std::size_t section_index = find_section_index(snapshot.sections, object.top, object.height);
        if (section_index < snapshot.sections.size()) {
            object.containing_section_id = snapshot.sections[section_index].id;
            object.containing_section_record_index = snapshot.sections[section_index].record_index;
            object.section_relative_top = object.top - snapshot.sections[section_index].top;
            object.section_relative_bottom = object.bottom - snapshot.sections[section_index].top;
            expand_preview_bounds(snapshot, object.left, object.top, object.right, object.bottom);
            ++snapshot.live_object_count;
            ++snapshot.placed_object_count;
            snapshot.sections[section_index].objects.push_back(std::move(object));
        } else {
            const std::size_t deleted_section_index =
                find_section_index(snapshot.deleted_sections, object.top, object.height);
            if (deleted_section_index < snapshot.deleted_sections.size()) {
                object.containing_section_id = snapshot.deleted_sections[deleted_section_index].id;
                object.containing_section_record_index = snapshot.deleted_sections[deleted_section_index].record_index;
                object.section_relative_top = object.top - snapshot.deleted_sections[deleted_section_index].top;
                object.section_relative_bottom = object.bottom - snapshot.deleted_sections[deleted_section_index].top;
                expand_deleted_preview_bounds(snapshot, object.left, object.top, object.right, object.bottom);
                expand_preview_bounds(snapshot, object.left, object.top, object.right, object.bottom);
                ++snapshot.live_object_count;
                ++snapshot.placed_object_count;
                snapshot.deleted_sections[deleted_section_index].objects.push_back(std::move(object));
            } else {
                expand_preview_bounds(snapshot, object.left, object.top, object.right, object.bottom);
                ++snapshot.live_object_count;
                snapshot.unplaced_objects.push_back(std::move(object));
            }
        }
    }

    const auto finalize_section_object_order = [](std::vector<StudioReportSectionSnapshot>& sections) {
        for (auto& section : sections) {
            std::sort(section.objects.begin(), section.objects.end(), [](const StudioLayoutObjectSnapshot& left,
                                                                        const StudioLayoutObjectSnapshot& right) {
                if (left.top != right.top) {
                    return left.top < right.top;
                }
                if (left.left != right.left) {
                    return left.left < right.left;
                }
                return left.record_index < right.record_index;
            });
            for (std::size_t object_index = 0; object_index < section.objects.size(); ++object_index) {
                section.objects[object_index].section_object_index = object_index;
                section.objects[object_index].section_object_count = section.objects.size();
            }
        }
    };
    finalize_section_object_order(snapshot.sections);
    finalize_section_object_order(snapshot.deleted_sections);

    std::sort(snapshot.unplaced_objects.begin(), snapshot.unplaced_objects.end(), [](const StudioLayoutObjectSnapshot& left,
                                                                                    const StudioLayoutObjectSnapshot& right) {
        if (left.top != right.top) {
            return left.top < right.top;
        }
        if (left.left != right.left) {
            return left.left < right.left;
        }
        return left.record_index < right.record_index;
    });

    std::vector<std::vector<StudioLayoutObjectSnapshot*>> live_section_deleted_objects(snapshot.sections.size());
    std::vector<std::vector<StudioLayoutObjectSnapshot*>> deleted_section_deleted_objects(
        snapshot.deleted_sections.size());

    for (auto& object : snapshot.deleted_objects) {
        const std::size_t live_section_index = find_section_index(snapshot.sections, object.top, object.height);
        const std::size_t deleted_section_index =
            find_section_index(snapshot.deleted_sections, object.top, object.height);
        const bool inside_live_section = live_section_index < snapshot.sections.size();
        const bool inside_deleted_section = deleted_section_index < snapshot.deleted_sections.size();
        if (inside_live_section) {
            object.containing_section_id = snapshot.sections[live_section_index].id;
            object.containing_section_record_index = snapshot.sections[live_section_index].record_index;
            object.section_relative_top = object.top - snapshot.sections[live_section_index].top;
            object.section_relative_bottom = object.bottom - snapshot.sections[live_section_index].top;
            ++snapshot.deleted_placed_object_count;
            ++snapshot.sections[live_section_index].deleted_object_count;
            live_section_deleted_objects[live_section_index].push_back(&object);
        } else if (inside_deleted_section) {
            object.containing_section_id = snapshot.deleted_sections[deleted_section_index].id;
            object.containing_section_record_index = snapshot.deleted_sections[deleted_section_index].record_index;
            object.section_relative_top = object.top - snapshot.deleted_sections[deleted_section_index].top;
            object.section_relative_bottom = object.bottom - snapshot.deleted_sections[deleted_section_index].top;
            ++snapshot.deleted_placed_object_count;
            ++snapshot.deleted_sections[deleted_section_index].deleted_object_count;
            deleted_section_deleted_objects[deleted_section_index].push_back(&object);
        } else {
            ++snapshot.deleted_unplaced_object_count;
        }
    }

    for (auto& section_objects : live_section_deleted_objects) {
        assign_deleted_object_section_order(section_objects);
    }
    for (auto& section_objects : deleted_section_deleted_objects) {
        assign_deleted_object_section_order(section_objects);
    }

    finalize_preview_bounds(snapshot);
    finalize_page_setup_summary(snapshot);
    finalize_object_kind_counts(snapshot);
    finalize_section_kind_counts(snapshot);
    finalize_groupings(snapshot);
    finalize_section_grouping_context(snapshot);

    return snapshot;
}

StudioReportLayoutSnapshot build_report_layout(const StudioDocumentModel& document) {
    return build_report_layout(document, report_layout_catalog());
}

}  // namespace copperfin::studio
