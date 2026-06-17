#include "copperfin/studio/toolbox_creation.h"

#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::studio {

namespace {

[[nodiscard]] vfp::VisualObjectCreateResult failed_create_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .parent_name = {}
    };
}

[[nodiscard]] std::string trimmed_copy(std::string_view value) {
    std::size_t first = 0U;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])) != 0) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1U])) != 0) {
        --last;
    }

    return std::string(value.substr(first, last - first));
}

[[nodiscard]] std::string normalized_identity(std::string_view value) {
    std::string normalized = trimmed_copy(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return normalized;
}

[[nodiscard]] const vfp::DbfRecordValue* find_record_value(
    const vfp::DbfRecord& record,
    std::string_view field_name) {
    const std::string normalized_field_name = normalized_identity(field_name);
    const auto found = std::find_if(record.values.begin(), record.values.end(), [&](const vfp::DbfRecordValue& value) {
        return normalized_identity(value.field_name) == normalized_field_name;
    });
    return found == record.values.end() ? nullptr : &*found;
}

[[nodiscard]] bool table_has_identity(
    const vfp::DbfTable& table,
    std::string_view field_name,
    std::string_view candidate) {
    const std::string normalized_candidate = normalized_identity(candidate);
    if (normalized_candidate.empty()) {
        return false;
    }

    for (const auto& record : table.records) {
        const auto* value = find_record_value(record, field_name);
        if (value != nullptr && normalized_identity(value->display_value) == normalized_candidate) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool table_has_object_name(const vfp::DbfTable& table, std::string_view candidate) {
    return table_has_identity(table, "OBJNAME", candidate) || table_has_identity(table, "NAME", candidate);
}

[[nodiscard]] std::optional<StudioToolboxItemDescriptor> find_toolbox_item(std::string_view toolbox_item_id) {
    const auto& items = studio_toolbox_palette();
    const std::string normalized_item_id = normalized_identity(toolbox_item_id);
    const auto found = std::find_if(items.begin(), items.end(), [&](const StudioToolboxItemDescriptor& item) {
        return normalized_identity(item.id) == normalized_item_id;
    });
    if (found == items.end()) {
        return std::nullopt;
    }
    return *found;
}

[[nodiscard]] std::string generate_default_object_name(
    const StudioToolboxItemDescriptor& item,
    const vfp::DbfTable& table) {
    const std::string prefix = trimmed_copy(item.default_name_prefix);
    for (std::size_t ordinal = 1U; ordinal < std::numeric_limits<std::size_t>::max(); ++ordinal) {
        const std::string candidate = prefix + std::to_string(ordinal);
        if (!table_has_object_name(table, candidate)) {
            return candidate;
        }
    }
    return {};
}

[[nodiscard]] bool toolbox_item_supports_context(
    const StudioToolboxItemDescriptor& item,
    StudioToolboxContext context) {
    return std::find(item.contexts.begin(), item.contexts.end(), context) != item.contexts.end();
}

}  // namespace

vfp::VisualObjectCreateResult create_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request) {
    if (request.path.empty()) {
        return failed_create_result("No asset path was provided.");
    }

    const auto item = find_toolbox_item(request.toolbox_item_id);
    if (!item.has_value()) {
        return failed_create_result("The requested toolbox item was not found.");
    }
    if (request.toolbox_context_provided && !toolbox_item_supports_context(*item, request.toolbox_context)) {
        return failed_create_result("The requested toolbox item is not available in the requested designer context.");
    }

    const auto table_result = vfp::parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_create_result(table_result.error);
    }

    std::string object_name = trimmed_copy(request.object_name);
    if (object_name.empty()) {
        object_name = generate_default_object_name(*item, table_result.table);
    }
    if (object_name.empty()) {
        return failed_create_result("A unique object name could not be generated for the requested toolbox item.");
    }

    std::vector<vfp::VisualObjectPropertyChange> field_values{
        {.property_name = "OBJNAME", .property_value = object_name},
        {.property_name = "NAME", .property_value = object_name},
        {.property_name = "CLASS", .property_value = std::string(item->vfp_class)},
        {.property_name = "BASECLASS", .property_value = std::string(item->base_class)}
    };

    if (!trimmed_copy(request.unique_id).empty()) {
        field_values.push_back({.property_name = "UNIQUEID", .property_value = request.unique_id});
    }
    if (!trimmed_copy(request.parent_name).empty()) {
        field_values.push_back({.property_name = "PARENT", .property_value = request.parent_name});
    }

    field_values.insert(field_values.end(), request.field_values.begin(), request.field_values.end());

    return vfp::create_visual_object({
        .path = request.path,
        .field_values = std::move(field_values)
    });
}

}  // namespace copperfin::studio
