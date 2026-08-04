// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string visual_property_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.VisualPropertyParse.Error.MissingValue",
        {{"option", option}});
}

std::string visual_property_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.VisualPropertyParse.Error.NonNegativeInteger",
        {{"option", option}});
}

std::string visual_property_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.VisualPropertyParse.Error.BooleanValueRequired",
        {
            {"option", option},
            {"trueValue", "true"},
            {"falseValue", "false"}
        });
}

std::string visual_property_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.VisualPropertyParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string visual_property_parse_batch_item_requires_property_name(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(
        key,
        {{"propertyNameOption", "--property-name"}});
}

std::string visual_property_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

VisualPropertyFilterParseResult parse_visual_property_filter_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyFilterParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-filter") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-filter") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--property-filter-text") {
            result.request.search_text = require_value(argument);
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-filter", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    return result;
}

VisualPropertyQueryParseResult parse_visual_property_query_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyQueryParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-query") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-query") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--property-name") {
            result.request.property_name = require_value(argument);
            result.property_name_provided = !result.request.property_name.empty();
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-query", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.property_name_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyName"));
    }
    return result;
}

VisualPropertyUpdateBatchParseResult parse_visual_property_update_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyUpdateBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--visual-property-update-batch") != args.end() ||
        std::find(args.begin(), args.end(), "--visual-object-property-update-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_property = [&]() -> copperfin::vfp::VisualObjectPropertyChange* {
        if (result.request.properties.empty()) {
            fail(visual_property_parse_batch_item_requires_property_name(
                catalog,
                "StudioHost.VisualPropertyParse.Error.UpdateBatchItemRequiresPropertyName"));
            return nullptr;
        }
        return &result.request.properties.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" ||
            argument == "--visual-property-update-batch" ||
            argument == "--visual-object-property-update-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--property-name") {
            result.request.properties.push_back({
                .property_name = require_value(argument),
                .property_value = {}
            });
        } else if (argument == "--property-value") {
            if (auto* property = current_property()) {
                property->property_value = require_value(argument);
            }
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-update-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.properties.empty()) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyChanges"));
    }
    return result;
}

VisualPropertyClearParseResult parse_visual_property_clear_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyClearParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-clear") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-clear") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--property-name") {
            result.request.property_name = require_value(argument);
            result.property_name_provided = !result.request.property_name.empty();
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-clear", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.property_name_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyName"));
    }
    return result;
}

VisualPropertyClearBatchParseResult parse_visual_property_clear_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyClearBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-clear-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_property = [&]() -> copperfin::vfp::VisualObjectPropertyClearBatchItem* {
        if (result.request.properties.empty()) {
            fail(visual_property_parse_batch_item_requires_property_name(
                catalog,
                "StudioHost.VisualPropertyParse.Error.ClearBatchItemRequiresPropertyName"));
            return nullptr;
        }
        return &result.request.properties.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-clear-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--property-name") {
            result.request.properties.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .property_name = require_value(argument)
            });
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            if (auto* property = current_property()) {
                property->record_index = record_index;
            }
        } else if (argument == "--object-name") {
            if (auto* property = current_property()) {
                property->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            if (auto* property = current_property()) {
                property->unique_id = require_value(argument);
            }
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-clear-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.properties.empty()) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyClears"));
    }
    return result;
}

VisualPropertyCopyParseResult parse_visual_property_copy_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyCopyParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-copy") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-copy") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            result.request.source_record_index = record_index;
        } else if (argument == "--source-object-name") {
            result.request.source_object_name = require_value(argument);
        } else if (argument == "--source-unique-id") {
            result.request.source_unique_id = require_value(argument);
        } else if (argument == "--property-name") {
            result.request.source_property_name = require_value(argument);
            result.property_name_provided = !result.request.source_property_name.empty();
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            result.request.target_record_index = record_index;
        } else if (argument == "--target-object-name") {
            result.request.target_object_name = require_value(argument);
        } else if (argument == "--target-unique-id") {
            result.request.target_unique_id = require_value(argument);
        } else if (argument == "--target-property-name") {
            result.request.target_property_name = require_value(argument);
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(visual_property_parse_boolean_value_required(catalog, "--replace-existing"));
                continue;
            }
            result.request.replace_existing = replace_existing;
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-copy", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.property_name_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyName"));
    }
    return result;
}

VisualPropertyCopyBatchParseResult parse_visual_property_copy_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyCopyBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-copy-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_property = [&]() -> copperfin::vfp::VisualObjectPropertyCopyBatchItem* {
        if (result.request.properties.empty()) {
            fail(visual_property_parse_batch_item_requires_property_name(
                catalog,
                "StudioHost.VisualPropertyParse.Error.CopyBatchItemRequiresPropertyName"));
            return nullptr;
        }
        return &result.request.properties.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-copy-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--property-name") {
            result.request.properties.push_back({
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = {},
                .source_property_name = require_value(argument),
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_property_name = {},
                .replace_existing = false
            });
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            if (auto* property = current_property()) {
                property->source_record_index = record_index;
            }
        } else if (argument == "--source-object-name") {
            if (auto* property = current_property()) {
                property->source_object_name = require_value(argument);
            }
        } else if (argument == "--source-unique-id") {
            if (auto* property = current_property()) {
                property->source_unique_id = require_value(argument);
            }
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            if (auto* property = current_property()) {
                property->target_record_index = record_index;
            }
        } else if (argument == "--target-object-name") {
            if (auto* property = current_property()) {
                property->target_object_name = require_value(argument);
            }
        } else if (argument == "--target-unique-id") {
            if (auto* property = current_property()) {
                property->target_unique_id = require_value(argument);
            }
        } else if (argument == "--target-property-name") {
            if (auto* property = current_property()) {
                property->target_property_name = require_value(argument);
            }
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(visual_property_parse_boolean_value_required(catalog, "--replace-existing"));
                continue;
            }
            if (auto* property = current_property()) {
                property->replace_existing = replace_existing;
            }
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-copy-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.properties.empty()) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyCopies"));
    }
    return result;
}

VisualPropertyMoveParseResult parse_visual_property_move_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyMoveParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-move") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-move") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            result.request.source_record_index = record_index;
        } else if (argument == "--source-object-name") {
            result.request.source_object_name = require_value(argument);
        } else if (argument == "--source-unique-id") {
            result.request.source_unique_id = require_value(argument);
        } else if (argument == "--property-name") {
            result.request.source_property_name = require_value(argument);
            result.property_name_provided = !result.request.source_property_name.empty();
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            result.request.target_record_index = record_index;
        } else if (argument == "--target-object-name") {
            result.request.target_object_name = require_value(argument);
        } else if (argument == "--target-unique-id") {
            result.request.target_unique_id = require_value(argument);
        } else if (argument == "--target-property-name") {
            result.request.target_property_name = require_value(argument);
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(visual_property_parse_boolean_value_required(catalog, "--replace-existing"));
                continue;
            }
            result.request.replace_existing = replace_existing;
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-move", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.property_name_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyName"));
    }
    return result;
}

VisualPropertyMoveBatchParseResult parse_visual_property_move_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyMoveBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-move-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_property = [&]() -> copperfin::vfp::VisualObjectPropertyMoveBatchItem* {
        if (result.request.properties.empty()) {
            fail(visual_property_parse_batch_item_requires_property_name(
                catalog,
                "StudioHost.VisualPropertyParse.Error.MoveBatchItemRequiresPropertyName"));
            return nullptr;
        }
        return &result.request.properties.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-move-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--property-name") {
            result.request.properties.push_back({
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = {},
                .source_property_name = require_value(argument),
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_property_name = {},
                .replace_existing = false
            });
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            if (auto* property = current_property()) {
                property->source_record_index = record_index;
            }
        } else if (argument == "--source-object-name") {
            if (auto* property = current_property()) {
                property->source_object_name = require_value(argument);
            }
        } else if (argument == "--source-unique-id") {
            if (auto* property = current_property()) {
                property->source_unique_id = require_value(argument);
            }
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            if (auto* property = current_property()) {
                property->target_record_index = record_index;
            }
        } else if (argument == "--target-object-name") {
            if (auto* property = current_property()) {
                property->target_object_name = require_value(argument);
            }
        } else if (argument == "--target-unique-id") {
            if (auto* property = current_property()) {
                property->target_unique_id = require_value(argument);
            }
        } else if (argument == "--target-property-name") {
            if (auto* property = current_property()) {
                property->target_property_name = require_value(argument);
            }
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(visual_property_parse_boolean_value_required(catalog, "--replace-existing"));
                continue;
            }
            if (auto* property = current_property()) {
                property->replace_existing = replace_existing;
            }
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-move-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.properties.empty()) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyMoves"));
    }
    return result;
}

VisualPropertyRenameParseResult parse_visual_property_rename_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyRenameParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-rename") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-rename") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--property-name") {
            result.request.property_name = require_value(argument);
            result.property_name_provided = !result.request.property_name.empty();
        } else if (argument == "--new-property-name") {
            result.request.new_property_name = require_value(argument);
            result.new_property_name_provided = !result.request.new_property_name.empty();
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-rename", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.property_name_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyName"));
    }
    if (result.ok && !result.new_property_name_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoTargetPropertyName"));
    }
    return result;
}

VisualPropertyRenameBatchParseResult parse_visual_property_rename_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyRenameBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-rename-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_property = [&]() -> copperfin::vfp::VisualObjectPropertyRenameBatchItem* {
        if (result.request.properties.empty()) {
            fail(visual_property_parse_batch_item_requires_property_name(
                catalog,
                "StudioHost.VisualPropertyParse.Error.RenameBatchItemRequiresPropertyName"));
            return nullptr;
        }
        return &result.request.properties.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-rename-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--property-name") {
            result.request.properties.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .property_name = require_value(argument),
                .new_property_name = {}
            });
        } else if (argument == "--new-property-name") {
            if (auto* property = current_property()) {
                property->new_property_name = require_value(argument);
            }
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            if (auto* property = current_property()) {
                property->record_index = record_index;
            }
        } else if (argument == "--object-name") {
            if (auto* property = current_property()) {
                property->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            if (auto* property = current_property()) {
                property->unique_id = require_value(argument);
            }
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-rename-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.properties.empty()) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyRenames"));
    }
    return result;
}

VisualPropertyReorderParseResult parse_visual_property_reorder_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyReorderParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-reorder") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-reorder") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--property-name") {
            result.request.property_name = require_value(argument);
            result.property_name_provided = !result.request.property_name.empty();
        } else if (argument == "--placement") {
            result.request.placement = require_value(argument);
            result.placement_provided = !result.request.placement.empty();
        } else if (argument == "--relative-property-name") {
            result.request.relative_property_name = require_value(argument);
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-reorder", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.property_name_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyName"));
    }
    if (result.ok && !result.placement_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyPlacement"));
    }
    return result;
}

VisualPropertyReorderBatchParseResult parse_visual_property_reorder_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyReorderBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-reorder-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_property = [&]() -> copperfin::vfp::VisualObjectPropertyReorderBatchItem* {
        if (result.request.properties.empty()) {
            fail(visual_property_parse_batch_item_requires_property_name(
                catalog,
                "StudioHost.VisualPropertyParse.Error.ReorderBatchItemRequiresPropertyName"));
            return nullptr;
        }
        return &result.request.properties.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-reorder-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--property-name") {
            result.request.properties.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .property_name = require_value(argument),
                .placement = {},
                .relative_property_name = {}
            });
        } else if (argument == "--placement") {
            if (auto* property = current_property()) {
                property->placement = require_value(argument);
            }
        } else if (argument == "--relative-property-name") {
            if (auto* property = current_property()) {
                property->relative_property_name = require_value(argument);
            }
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            if (auto* property = current_property()) {
                property->record_index = record_index;
            }
        } else if (argument == "--object-name") {
            if (auto* property = current_property()) {
                property->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            if (auto* property = current_property()) {
                property->unique_id = require_value(argument);
            }
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-reorder-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.properties.empty()) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoPropertyReorders"));
    }
    return result;
}

VisualPropertyListParseResult parse_visual_property_list_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualPropertyListParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-property-list") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_property_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-property-list") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_property_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(visual_property_parse_unknown_option(catalog, "visual-property-list", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_property_parse_message(catalog, "StudioHost.VisualPropertyParse.Error.NoAssetPath"));
    }
    return result;
}

void print_json_visual_property_snapshot(
    const copperfin::vfp::VisualObjectPropertySnapshot& property,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"propertyName\": ";
    print_json_string(property.property_name);
    std::cout << ",\n";
    std::cout << indent << "  \"value\": ";
    print_json_string(property.value);
    std::cout << ",\n";
    std::cout << indent << "  \"directField\": " << (property.direct_field ? "true" : "false") << ",\n";
    std::cout << indent << "  \"fieldType\": ";
    if (property.field_type == '\0') {
        std::cout << "null";
    } else {
        print_json_string(std::string(1U, property.field_type));
    }
    std::cout << ",\n";
    std::cout << indent << "  \"sourceLineIndex\": ";
    if (property.source_line_index == static_cast<std::size_t>(-1)) {
        std::cout << "null\n";
    } else {
        std::cout << property.source_line_index << "\n";
    }
    std::cout << indent << "}";
}

void print_json_visual_property_query_result(
    const copperfin::vfp::VisualObjectPropertyQueryResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualPropertyQuery\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"exists\": " << (result.exists ? "true" : "false") << ",\n";
    std::cout << "    \"directField\": " << (result.direct_field ? "true" : "false") << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"recordDeleted\": " << (result.record_deleted ? "true" : "false") << ",\n";
    std::cout << "    \"propertyName\": ";
    print_json_string(result.property_name);
    std::cout << ",\n";
    std::cout << "    \"value\": ";
    print_json_string(result.value);
    std::cout << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_property_list_result(
    const copperfin::vfp::VisualObjectPropertyListResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualPropertyList\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"recordDeleted\": " << (result.record_deleted ? "true" : "false") << ",\n";
    std::cout << "    \"propertyCount\": " << result.properties.size() << ",\n";
    std::cout << "    \"dryRun\": true,\n";
    std::cout << "    \"mutatesAsset\": false,\n";
    std::cout << "    \"properties\": [\n";
    for (std::size_t index = 0U; index < result.properties.size(); ++index) {
        print_json_visual_property_snapshot(result.properties[index], "      ");
        if ((index + 1U) != result.properties.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_property_filter_result(
    const copperfin::vfp::VisualObjectPropertyListFilterResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualPropertyFilter\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"recordDeleted\": " << (result.record_deleted ? "true" : "false") << ",\n";
    std::cout << "    \"searchText\": ";
    print_json_string(result.search_text);
    std::cout << ",\n";
    std::cout << "    \"propertyCount\": " << result.property_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"properties\": [\n";
    for (std::size_t index = 0U; index < result.properties.size(); ++index) {
        print_json_visual_property_snapshot(result.properties[index], "      ");
        if ((index + 1U) != result.properties.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_text_visual_property_filter_result(
    const copperfin::vfp::VisualObjectPropertyListFilterResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "record_deleted: " << (result.record_deleted ? "true" : "false") << "\n";
    std::cout << "search_text: " << result.search_text << "\n";
    std::cout << "property_count: " << result.property_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& property : result.properties) {
        std::cout << "property: " << property.property_name << " " << property.value << "\n";
    }
}

void print_text_visual_property_query_result(
    const copperfin::vfp::VisualObjectPropertyQueryResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "exists: " << (result.exists ? "true" : "false") << "\n";
    std::cout << "direct_field: " << (result.direct_field ? "true" : "false") << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "record_deleted: " << (result.record_deleted ? "true" : "false") << "\n";
    std::cout << "property_name: " << result.property_name << "\n";
    std::cout << "value: " << result.value << "\n";
}

void print_text_visual_property_list_result(
    const copperfin::vfp::VisualObjectPropertyListResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "record_deleted: " << (result.record_deleted ? "true" : "false") << "\n";
    std::cout << "property_count: " << result.properties.size() << "\n";
    std::cout << "dry_run: true\n";
    std::cout << "mutates_asset: false\n";
    for (const auto& property : result.properties) {
        std::cout << "property: " << property.property_name << " " << property.value << "\n";
    }
}

std::optional<int> try_handle_visual_property_list(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_list_parse = parse_visual_property_list_arguments(catalog, args);
    if (!(visual_property_list_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_list_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectPropertyListResult{
                .ok = false,
                .error = visual_property_list_parse.error,
                .record_index = 0U,
                .record_deleted = false,
                .properties = {}
            };
            if (visual_property_list_parse.output_json) {
                print_json_visual_property_list_result(result);
            } else {
                print_text_visual_property_list_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::list_visual_object_properties(
            visual_property_list_parse.request);
        if (visual_property_list_parse.output_json) {
            print_json_visual_property_list_result(result);
        } else {
            print_text_visual_property_list_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_query(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_query_parse = parse_visual_property_query_arguments(catalog, args);
    if (!(visual_property_query_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_query_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectPropertyQueryResult{
                .ok = false,
                .error = visual_property_query_parse.error,
                .exists = false,
                .direct_field = false,
                .record_index = 0U,
                .record_deleted = false,
                .property_name = visual_property_query_parse.request.property_name,
                .value = {}
            };
            if (visual_property_query_parse.output_json) {
                print_json_visual_property_query_result(result);
            } else {
                print_text_visual_property_query_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::query_visual_object_property(
            visual_property_query_parse.request);
        if (visual_property_query_parse.output_json) {
            print_json_visual_property_query_result(result);
        } else {
            print_text_visual_property_query_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_update_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_update_batch_parse = parse_visual_property_update_batch_arguments(catalog, args);
    if (!(visual_property_update_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_update_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_update_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_update_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyUpdateBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::update_visual_object_properties(
            visual_property_update_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_update_batch_parse.request.path);
        if (visual_property_update_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyUpdateBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_clear(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_clear_parse = parse_visual_property_clear_arguments(catalog, args);
    if (!(visual_property_clear_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_clear_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_clear_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_clear_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyClear");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::clear_visual_object_property(
            visual_property_clear_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_clear_parse.request.path);
        if (visual_property_clear_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyClear");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_clear_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_clear_batch_parse = parse_visual_property_clear_batch_arguments(catalog, args);
    if (!(visual_property_clear_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_clear_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_clear_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_clear_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyClearBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::clear_visual_object_properties(
            visual_property_clear_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_clear_batch_parse.request.path);
        if (visual_property_clear_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyClearBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_copy(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_copy_parse = parse_visual_property_copy_arguments(catalog, args);
    if (!(visual_property_copy_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_copy_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_copy_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_copy_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyCopy");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::copy_visual_object_property(
            visual_property_copy_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_copy_parse.request.path);
        if (visual_property_copy_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyCopy");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_copy_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_copy_batch_parse = parse_visual_property_copy_batch_arguments(catalog, args);
    if (!(visual_property_copy_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_copy_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_copy_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_copy_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyCopyBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::copy_visual_object_properties(
            visual_property_copy_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_copy_batch_parse.request.path);
        if (visual_property_copy_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyCopyBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_move(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_move_parse = parse_visual_property_move_arguments(catalog, args);
    if (!(visual_property_move_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_move_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_move_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_move_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyMove");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::move_visual_object_property(
            visual_property_move_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_move_parse.request.path);
        if (visual_property_move_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyMove");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_move_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_move_batch_parse = parse_visual_property_move_batch_arguments(catalog, args);
    if (!(visual_property_move_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_move_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_move_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_move_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyMoveBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::move_visual_object_properties(
            visual_property_move_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_move_batch_parse.request.path);
        if (visual_property_move_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyMoveBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_rename(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_rename_parse = parse_visual_property_rename_arguments(catalog, args);
    if (!(visual_property_rename_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_rename_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_rename_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_rename_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyRename");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::rename_visual_object_property(
            visual_property_rename_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_rename_parse.request.path);
        if (visual_property_rename_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyRename");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_rename_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_rename_batch_parse = parse_visual_property_rename_batch_arguments(catalog, args);
    if (!(visual_property_rename_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_rename_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_rename_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_rename_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyRenameBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::rename_visual_object_properties(
            visual_property_rename_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_rename_batch_parse.request.path);
        if (visual_property_rename_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyRenameBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_reorder(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_reorder_parse = parse_visual_property_reorder_arguments(catalog, args);
    if (!(visual_property_reorder_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_reorder_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_reorder_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_reorder_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyReorder");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::reorder_visual_object_property(
            visual_property_reorder_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_reorder_parse.request.path);
        if (visual_property_reorder_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualPropertyReorder");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_reorder_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_reorder_batch_parse = parse_visual_property_reorder_batch_arguments(catalog, args);
    if (!(visual_property_reorder_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_reorder_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_property_reorder_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_property_reorder_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualPropertyReorderBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::reorder_visual_object_properties(
            visual_property_reorder_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_property_reorder_batch_parse.request.path);
        auto output_result = result;
        if (output_result.ok) {
            output_result.affected_object_count = visual_property_reorder_batch_parse.request.properties.size();
        }
        if (visual_property_reorder_batch_parse.output_json) {
            print_json_visual_method_update_result(output_result, undo_status, "visualPropertyReorderBatch");
        } else {
            print_text_visual_method_update_result(output_result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_property_filter(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_property_filter_parse = parse_visual_property_filter_arguments(catalog, args);
    if (!(visual_property_filter_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_property_filter_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectPropertyListFilterResult{
                .ok = false,
                .error = visual_property_filter_parse.error,
                .record_index = 0U,
                .record_deleted = false,
                .search_text = visual_property_filter_parse.request.search_text,
                .property_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .properties = {}
            };
            if (visual_property_filter_parse.output_json) {
                print_json_visual_property_filter_result(result);
            } else {
                print_text_visual_property_filter_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::filter_visual_object_properties(
            visual_property_filter_parse.request);
        if (visual_property_filter_parse.output_json) {
            print_json_visual_property_filter_result(result);
        } else {
            print_text_visual_property_filter_result(result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
