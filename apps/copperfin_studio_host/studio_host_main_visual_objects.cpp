// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string visual_object_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.VisualObjectParse.Error.MissingValue",
        {{"option", option}});
}

std::string visual_object_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.VisualObjectParse.Error.NonNegativeInteger",
        {{"option", option}});
}

std::string visual_object_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.VisualObjectParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string visual_object_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

std::string visual_object_parse_subtree_replacement_requires_source(
    const copperfin::localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.VisualObjectParse.Error.SubtreeReplacementRequiresSourceUniqueId",
        {{"replacementSourceUniqueIdOption", "--replacement-source-unique-id"}});
}

VisualObjectListParseResult parse_visual_object_list_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectListParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-list") != args.end();
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
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-list") {
            continue;
        }
        if (argument == "--path") {
            result.path = require_value(argument);
            result.path_provided = !result.path.empty();
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-list", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    return result;
}

VisualObjectChildrenParseResult parse_visual_object_children_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectChildrenParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-children") != args.end();
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
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-children") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-children", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    return result;
}

VisualObjectDescendantsParseResult parse_visual_object_descendants_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectDescendantsParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-descendants") != args.end();
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
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-descendants") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-descendants", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    return result;
}

VisualObjectAncestorsParseResult parse_visual_object_ancestors_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectAncestorsParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-ancestors") != args.end();
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
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-ancestors") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-ancestors", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    return result;
}

VisualObjectReparentBatchParseResult parse_visual_object_reparent_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectReparentBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-reparent-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_object = [&]() -> copperfin::vfp::VisualObjectReparentBatchItem* {
        if (result.request.objects.empty()) {
            fail(visual_object_parse_message(
                catalog,
                "StudioHost.VisualObjectParse.Error.ReparentBatchItemRequiresSelectedObject"));
            return nullptr;
        }
        return &result.request.objects.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-reparent-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--selected-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--selected-record"));
                continue;
            }
            result.request.objects.push_back({
                .record_index = record_index,
                .object_name = {},
                .unique_id = {},
                .parent_object_name = {},
                .parent_unique_id = {},
                .clear_parent = false
            });
        } else if (argument == "--selected-object-name") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = require_value(argument),
                .unique_id = {},
                .parent_object_name = {},
                .parent_unique_id = {},
                .clear_parent = false
            });
        } else if (argument == "--selected-unique-id") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = require_value(argument),
                .parent_object_name = {},
                .parent_unique_id = {},
                .clear_parent = false
            });
        } else if (argument == "--parent-name") {
            if (auto* object = current_object()) {
                object->parent_object_name = require_value(argument);
            }
        } else if (argument == "--parent-unique-id") {
            if (auto* object = current_object()) {
                object->parent_unique_id = require_value(argument);
            }
        } else if (argument == "--clear-parent") {
            if (auto* object = current_object()) {
                object->clear_parent = true;
            }
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-reparent-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.objects.empty()) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoReparentOperations"));
    }
    return result;
}

VisualObjectDuplicateBatchParseResult parse_visual_object_duplicate_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectDuplicateBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-duplicate-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_object = [&]() -> copperfin::vfp::VisualObjectDuplicateBatchItem* {
        if (result.request.objects.empty()) {
            fail(visual_object_parse_message(
                catalog,
                "StudioHost.VisualObjectParse.Error.DuplicateBatchItemRequiresSelectedObject"));
            return nullptr;
        }
        return &result.request.objects.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-duplicate-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--selected-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--selected-record"));
                continue;
            }
            result.request.objects.push_back({
                .record_index = record_index,
                .object_name = {},
                .unique_id = {},
                .new_object_name = {},
                .new_name = {},
                .new_unique_id = {}
            });
        } else if (argument == "--selected-object-name") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = require_value(argument),
                .unique_id = {},
                .new_object_name = {},
                .new_name = {},
                .new_unique_id = {}
            });
        } else if (argument == "--selected-unique-id") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = require_value(argument),
                .new_object_name = {},
                .new_name = {},
                .new_unique_id = {}
            });
        } else if (argument == "--new-object-name") {
            if (auto* object = current_object()) {
                object->new_object_name = require_value(argument);
            }
        } else if (argument == "--new-name") {
            if (auto* object = current_object()) {
                object->new_name = require_value(argument);
            }
        } else if (argument == "--new-unique-id") {
            if (auto* object = current_object()) {
                object->new_unique_id = require_value(argument);
            }
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-duplicate-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.objects.empty()) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoDuplicateOperations"));
    }
    return result;
}

VisualObjectDuplicateSubtreeParseResult parse_visual_object_duplicate_subtree_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectDuplicateSubtreeParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-duplicate-subtree") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_replacement = [&]() -> copperfin::vfp::VisualObjectSubtreeDuplicateReplacement* {
        if (result.request.replacements.empty()) {
            fail(visual_object_parse_subtree_replacement_requires_source(catalog));
            return nullptr;
        }
        return &result.request.replacements.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-duplicate-subtree") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
            result.root_selector_provided = true;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
            result.root_selector_provided = !result.request.object_name.empty();
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
            result.root_selector_provided = !result.request.unique_id.empty();
        } else if (argument == "--replacement-source-unique-id") {
            result.request.replacements.push_back({
                .source_unique_id = require_value(argument),
                .new_object_name = {},
                .new_name = {},
                .new_unique_id = {}
            });
        } else if (argument == "--new-object-name") {
            if (auto* replacement = current_replacement()) {
                replacement->new_object_name = require_value(argument);
            }
        } else if (argument == "--new-name") {
            if (auto* replacement = current_replacement()) {
                replacement->new_name = require_value(argument);
            }
        } else if (argument == "--new-unique-id") {
            if (auto* replacement = current_replacement()) {
                replacement->new_unique_id = require_value(argument);
            }
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-duplicate-subtree", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.root_selector_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoRootObjectSelector"));
    }
    if (result.ok && result.request.replacements.empty()) {
        fail(visual_object_parse_message(
            catalog,
            "StudioHost.VisualObjectParse.Error.NoSubtreeReplacementIdentities"));
    }
    return result;
}

VisualObjectRenameBatchParseResult parse_visual_object_rename_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectRenameBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-rename-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_object = [&]() -> copperfin::vfp::VisualObjectRenameBatchItem* {
        if (result.request.objects.empty()) {
            fail(visual_object_parse_message(
                catalog,
                "StudioHost.VisualObjectParse.Error.RenameBatchItemRequiresSelectedObject"));
            return nullptr;
        }
        return &result.request.objects.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-rename-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--selected-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--selected-record"));
                continue;
            }
            result.request.objects.push_back({
                .record_index = record_index,
                .object_name = {},
                .unique_id = {},
                .update_object_name = false,
                .new_object_name = {},
                .update_name = false,
                .new_name = {},
                .update_unique_id = false,
                .new_unique_id = {}
            });
        } else if (argument == "--selected-object-name") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = require_value(argument),
                .unique_id = {},
                .update_object_name = false,
                .new_object_name = {},
                .update_name = false,
                .new_name = {},
                .update_unique_id = false,
                .new_unique_id = {}
            });
        } else if (argument == "--selected-unique-id") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = require_value(argument),
                .update_object_name = false,
                .new_object_name = {},
                .update_name = false,
                .new_name = {},
                .update_unique_id = false,
                .new_unique_id = {}
            });
        } else if (argument == "--new-object-name") {
            if (auto* object = current_object()) {
                object->update_object_name = true;
                object->new_object_name = require_value(argument);
            }
        } else if (argument == "--new-name") {
            if (auto* object = current_object()) {
                object->update_name = true;
                object->new_name = require_value(argument);
            }
        } else if (argument == "--new-unique-id") {
            if (auto* object = current_object()) {
                object->update_unique_id = true;
                object->new_unique_id = require_value(argument);
            }
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-rename-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.objects.empty()) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoRenameOperations"));
    }
    return result;
}

VisualObjectReorderBatchParseResult parse_visual_object_reorder_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectReorderBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-reorder-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_object = [&]() -> copperfin::vfp::VisualObjectReorderBatchItem* {
        if (result.request.objects.empty()) {
            fail(visual_object_parse_message(
                catalog,
                "StudioHost.VisualObjectParse.Error.ReorderBatchItemRequiresSelectedObject"));
            return nullptr;
        }
        return &result.request.objects.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-object-reorder-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--selected-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--selected-record"));
                continue;
            }
            result.request.objects.push_back({
                .record_index = record_index,
                .object_name = {},
                .unique_id = {},
                .placement = {},
                .target_object_name = {},
                .target_unique_id = {}
            });
        } else if (argument == "--selected-object-name") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = require_value(argument),
                .unique_id = {},
                .placement = {},
                .target_object_name = {},
                .target_unique_id = {}
            });
        } else if (argument == "--selected-unique-id") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = require_value(argument),
                .placement = {},
                .target_object_name = {},
                .target_unique_id = {}
            });
        } else if (argument == "--placement") {
            if (auto* object = current_object()) {
                object->placement = require_value(argument);
            }
        } else if (argument == "--target-object-name") {
            if (auto* object = current_object()) {
                object->target_object_name = require_value(argument);
            }
        } else if (argument == "--target-unique-id") {
            if (auto* object = current_object()) {
                object->target_unique_id = require_value(argument);
            }
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-reorder-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.objects.empty()) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoReorderOperations"));
    }
    if (result.ok) {
        for (const auto& object : result.request.objects) {
            if (object.placement.empty()) {
                fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoObjectPlacement"));
                break;
            }
        }
    }
    return result;
}

VisualObjectUpdateBatchParseResult parse_visual_object_update_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualObjectUpdateBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.launched_from_visual_studio =
        std::find(args.begin(), args.end(), "--from-vs") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-object-update-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_object = [&]() -> copperfin::vfp::VisualObjectBatchEditItem* {
        if (result.request.objects.empty()) {
            fail(visual_object_parse_message(
                catalog,
                "StudioHost.VisualObjectParse.Error.UpdateBatchPropertyOptionsRequireSelectedObject"));
            return nullptr;
        }
        return &result.request.objects.back();
    };

    auto current_property = [&]() -> copperfin::vfp::VisualObjectPropertyChange* {
        auto* object = current_object();
        if (object == nullptr) {
            return nullptr;
        }
        if (object->properties.empty()) {
            fail(catalog.translate(
                "StudioHost.VisualObjectParse.Error.UpdateBatchPropertyValuesRequirePropertyName",
                {{"propertyNameOption", "--property-name"}}));
            return nullptr;
        }
        return &object->properties.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_object_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--from-vs" ||
            argument == "--visual-object-update-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--selected-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_object_parse_non_negative_integer(catalog, "--selected-record"));
                continue;
            }
            result.request.objects.push_back({
                .record_index = record_index,
                .object_name = {},
                .unique_id = {},
                .properties = {}
            });
        } else if (argument == "--selected-object-name") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = require_value(argument),
                .unique_id = {},
                .properties = {}
            });
        } else if (argument == "--selected-unique-id") {
            result.request.objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = require_value(argument),
                .properties = {}
            });
        } else if (argument == "--property-name") {
            if (auto* object = current_object()) {
                object->properties.push_back({
                    .property_name = require_value(argument),
                    .property_value = {}
                });
            }
        } else if (argument == "--property-value") {
            if (auto* property = current_property()) {
                property->property_value = require_value(argument);
            }
        } else {
            fail(visual_object_parse_unknown_option(catalog, "visual-object-update-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.objects.empty()) {
        fail(visual_object_parse_message(catalog, "StudioHost.VisualObjectParse.Error.NoUpdateOperations"));
    }
    return result;
}

void print_json_visual_object_snapshot(
    const copperfin::vfp::VisualObjectSnapshot& object,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"recordIndex\": " << object.record_index << ",\n";
    std::cout << indent << "  \"deleted\": " << (object.deleted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(object.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"objectPath\": ";
    print_json_string(object.object_path);
    std::cout << ",\n";
    std::cout << indent << "  \"objectDepth\": " << object.object_depth << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(object.unique_id);
    std::cout << ",\n";
    std::cout << indent << "  \"parentName\": ";
    print_json_string(object.parent_name);
    std::cout << ",\n";
    std::cout << indent << "  \"parentRecordIndex\": ";
    if (object.parent_record_available) {
        std::cout << object.parent_record_index;
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << indent << "  \"ancestorRecordIndexes\": ";
    print_json_record_index_array(object.ancestor_record_indexes);
    std::cout << ",\n";
    std::cout << indent << "  \"siblingIndex\": " << object.sibling_index << ",\n";
    std::cout << indent << "  \"siblingCount\": " << object.sibling_count << ",\n";
    std::cout << indent << "  \"childCount\": " << object.child_count << ",\n";
    std::cout << indent << "  \"propertyCount\": " << object.property_count << ",\n";
    std::cout << indent << "  \"methodCount\": " << object.method_count << ",\n";
    std::cout << indent << "  \"className\": ";
    print_json_string(object.class_name);
    std::cout << ",\n";
    std::cout << indent << "  \"baseclassName\": ";
    print_json_string(object.baseclass_name);
    std::cout << ",\n";
    std::cout << indent << "  \"caption\": ";
    print_json_string(object.caption);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_visual_object_list_result(
    const copperfin::vfp::VisualObjectListResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualObjectList\": ";
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
    std::cout << "    \"objectCount\": " << result.objects.size() << ",\n";
    std::cout << "    \"dryRun\": true,\n";
    std::cout << "    \"mutatesAsset\": false,\n";
    std::cout << "    \"objects\": [\n";
    for (std::size_t index = 0U; index < result.objects.size(); ++index) {
        print_json_visual_object_snapshot(result.objects[index], "      ");
        if ((index + 1U) != result.objects.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_object_children_result(
    const copperfin::vfp::VisualObjectChildrenListResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualObjectChildren\": ";
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
    std::cout << "    \"parentRecordIndex\": " << result.parent_record_index << ",\n";
    std::cout << "    \"parentName\": ";
    print_json_string(result.parent_name);
    std::cout << ",\n";
    std::cout << "    \"childCount\": " << result.children.size() << ",\n";
    std::cout << "    \"dryRun\": true,\n";
    std::cout << "    \"mutatesAsset\": false,\n";
    std::cout << "    \"children\": [\n";
    for (std::size_t index = 0U; index < result.children.size(); ++index) {
        print_json_visual_object_snapshot(result.children[index], "      ");
        if ((index + 1U) != result.children.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_object_descendants_result(
    const copperfin::vfp::VisualObjectDescendantsListResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualObjectDescendants\": ";
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
    std::cout << "    \"parentRecordIndex\": " << result.parent_record_index << ",\n";
    std::cout << "    \"parentName\": ";
    print_json_string(result.parent_name);
    std::cout << ",\n";
    std::cout << "    \"descendantCount\": " << result.descendants.size() << ",\n";
    std::cout << "    \"dryRun\": true,\n";
    std::cout << "    \"mutatesAsset\": false,\n";
    std::cout << "    \"descendants\": [\n";
    for (std::size_t index = 0U; index < result.descendants.size(); ++index) {
        const auto& descendant = result.descendants[index];
        std::cout << "      {\n";
        std::cout << "        \"depth\": " << descendant.depth << ",\n";
        std::cout << "        \"object\": ";
        print_json_visual_object_snapshot(descendant.object, "        ");
        std::cout << "\n";
        std::cout << "      }";
        if ((index + 1U) != result.descendants.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_object_ancestors_result(
    const copperfin::vfp::VisualObjectAncestorsListResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualObjectAncestors\": ";
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
    std::cout << "    \"ancestorCount\": " << result.ancestors.size() << ",\n";
    std::cout << "    \"dryRun\": true,\n";
    std::cout << "    \"mutatesAsset\": false,\n";
    std::cout << "    \"ancestors\": [\n";
    for (std::size_t index = 0U; index < result.ancestors.size(); ++index) {
        const auto& ancestor = result.ancestors[index];
        std::cout << "      {\n";
        std::cout << "        \"depth\": " << ancestor.depth << ",\n";
        std::cout << "        \"object\": ";
        print_json_visual_object_snapshot(ancestor.object, "        ");
        std::cout << "\n";
        std::cout << "      }";
        if ((index + 1U) != result.ancestors.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_object_subtree_duplicate_result(
    const copperfin::vfp::VisualObjectSubtreeDuplicateResult& result,
    const copperfin::vfp::VisualAssetUndoStatus& undo_status) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualObjectDuplicateSubtree\": ";
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
    std::cout << "    \"rootRecordIndex\": " << result.root_record_index << ",\n";
    std::cout << "    \"copiedCount\": " << result.copied_count << ",\n";
    std::cout << "    \"affectedObjectCount\": " << result.copied_count << ",\n";
    std::cout << "    \"rootObjectName\": ";
    print_json_string(result.root_object_name);
    std::cout << ",\n";
    std::cout << "    \"rootUniqueId\": ";
    print_json_string(result.root_unique_id);
    std::cout << ",\n";
    std::cout << "    \"rootParentName\": ";
    print_json_string(result.root_parent_name);
    std::cout << ",\n";
    std::cout << "    \"dryRun\": false,\n";
    std::cout << "    \"mutatesAsset\": true,\n";
    std::cout << "    \"undoAvailable\": " << (undo_status.available ? "true" : "false") << ",\n";
    std::cout << "    \"undoLabel\": ";
    print_json_string(undo_status.label);
    std::cout << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_text_visual_object_list_result(
    const copperfin::vfp::VisualObjectListResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "object_count: " << result.objects.size() << "\n";
    std::cout << "dry_run: true\n";
    std::cout << "mutates_asset: false\n";
    for (const auto& object : result.objects) {
        std::cout << "object: " << object.record_index << " " << object.object_name << "\n";
    }
}

void print_text_visual_object_children_result(
    const copperfin::vfp::VisualObjectChildrenListResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "parent_record_index: " << result.parent_record_index << "\n";
    std::cout << "parent_name: " << result.parent_name << "\n";
    std::cout << "child_count: " << result.children.size() << "\n";
    std::cout << "dry_run: true\n";
    std::cout << "mutates_asset: false\n";
    for (const auto& child : result.children) {
        std::cout << "child: " << child.record_index << " " << child.object_name << "\n";
    }
}

void print_text_visual_object_descendants_result(
    const copperfin::vfp::VisualObjectDescendantsListResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "parent_record_index: " << result.parent_record_index << "\n";
    std::cout << "parent_name: " << result.parent_name << "\n";
    std::cout << "descendant_count: " << result.descendants.size() << "\n";
    std::cout << "dry_run: true\n";
    std::cout << "mutates_asset: false\n";
    for (const auto& descendant : result.descendants) {
        std::cout << "descendant: " << descendant.depth << " "
                  << descendant.object.record_index << " " << descendant.object.object_name << "\n";
    }
}

void print_text_visual_object_ancestors_result(
    const copperfin::vfp::VisualObjectAncestorsListResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "ancestor_count: " << result.ancestors.size() << "\n";
    std::cout << "dry_run: true\n";
    std::cout << "mutates_asset: false\n";
    for (const auto& ancestor : result.ancestors) {
        std::cout << "ancestor: " << ancestor.depth << " "
                  << ancestor.object.record_index << " " << ancestor.object.object_name << "\n";
    }
}

std::optional<int> try_handle_visual_object_duplicate_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_duplicate_batch_parse = parse_visual_object_duplicate_batch_arguments(catalog, args);
    if (!(visual_object_duplicate_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_duplicate_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_object_duplicate_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_object_duplicate_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualObjectDuplicateBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto duplicate_result = copperfin::vfp::duplicate_visual_objects(
            visual_object_duplicate_batch_parse.request);
        const auto result = copperfin::vfp::VisualAssetEditResult{
            .ok = duplicate_result.ok,
            .error = duplicate_result.error,
            .affected_object_count = duplicate_result.duplicated_objects.size()
        };
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_object_duplicate_batch_parse.request.path);
        if (visual_object_duplicate_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualObjectDuplicateBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_duplicate_subtree(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_duplicate_subtree_parse = parse_visual_object_duplicate_subtree_arguments(catalog, args);
    if (!(visual_object_duplicate_subtree_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_duplicate_subtree_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectSubtreeDuplicateResult{
                .ok = false,
                .error = visual_object_duplicate_subtree_parse.error,
                .root_record_index = 0U,
                .copied_count = 0U,
                .root_object_name = {},
                .root_unique_id = {},
                .root_parent_name = {}
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_object_duplicate_subtree_parse.output_json) {
                print_json_visual_object_subtree_duplicate_result(result, undo_status);
            } else {
                std::cout << "status: error\n";
                std::cout << studio_error_prefix() << result.error << "\n";
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::duplicate_visual_object_subtree(
            visual_object_duplicate_subtree_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_object_duplicate_subtree_parse.request.path);
        if (visual_object_duplicate_subtree_parse.output_json) {
            print_json_visual_object_subtree_duplicate_result(result, undo_status);
        } else if (!result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << result.error << "\n";
        } else {
            std::cout << "status: ok\n";
            std::cout << "copied-count: " << result.copied_count << "\n";
            std::cout << "root-record-index: " << result.root_record_index << "\n";
            std::cout << "root-object-name: " << result.root_object_name << "\n";
            std::cout << "root-unique-id: " << result.root_unique_id << "\n";
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_rename_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_rename_batch_parse = parse_visual_object_rename_batch_arguments(catalog, args);
    if (!(visual_object_rename_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_rename_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_object_rename_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_object_rename_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualObjectRenameBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::rename_visual_objects(
            visual_object_rename_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_object_rename_batch_parse.request.path);
        if (visual_object_rename_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualObjectRenameBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_reorder_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_reorder_batch_parse = parse_visual_object_reorder_batch_arguments(catalog, args);
    if (!(visual_object_reorder_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_reorder_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_object_reorder_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_object_reorder_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualObjectReorderBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::reorder_visual_objects(
            visual_object_reorder_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_object_reorder_batch_parse.request.path);
        if (visual_object_reorder_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualObjectReorderBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_reparent_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_reparent_batch_parse = parse_visual_object_reparent_batch_arguments(catalog, args);
    if (!(visual_object_reparent_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_reparent_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_object_reparent_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_object_reparent_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualObjectReparentBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::reparent_visual_objects(
            visual_object_reparent_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_object_reparent_batch_parse.request.path);
        if (visual_object_reparent_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualObjectReparentBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_update_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_update_batch_parse = parse_visual_object_update_batch_arguments(catalog, args);
    if (!(visual_object_update_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_update_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_object_update_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_object_update_batch_parse.output_json) {
                print_json_visual_method_update_result(
                    result,
                    undo_status,
                    "visualObjectUpdateBatch",
                    visual_object_update_batch_parse.launched_from_visual_studio);
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::update_visual_object_batch(
            visual_object_update_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_object_update_batch_parse.request.path);
        if (visual_object_update_batch_parse.output_json) {
            print_json_visual_method_update_result(
                result,
                undo_status,
                "visualObjectUpdateBatch",
                visual_object_update_batch_parse.launched_from_visual_studio);
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_ancestors(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_ancestors_parse = parse_visual_object_ancestors_arguments(catalog, args);
    if (!(visual_object_ancestors_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_ancestors_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectAncestorsListResult{
                .ok = false,
                .error = visual_object_ancestors_parse.error,
                .record_index = 0U,
                .ancestors = {}
            };
            if (visual_object_ancestors_parse.output_json) {
                print_json_visual_object_ancestors_result(result);
            } else {
                print_text_visual_object_ancestors_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::list_visual_object_ancestors(
            visual_object_ancestors_parse.request);
        if (visual_object_ancestors_parse.output_json) {
            print_json_visual_object_ancestors_result(result);
        } else {
            print_text_visual_object_ancestors_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_descendants(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_descendants_parse = parse_visual_object_descendants_arguments(catalog, args);
    if (!(visual_object_descendants_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_descendants_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectDescendantsListResult{
                .ok = false,
                .error = visual_object_descendants_parse.error,
                .parent_record_index = 0U,
                .parent_name = {},
                .descendants = {}
            };
            if (visual_object_descendants_parse.output_json) {
                print_json_visual_object_descendants_result(result);
            } else {
                print_text_visual_object_descendants_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::list_visual_object_descendants(
            visual_object_descendants_parse.request);
        if (visual_object_descendants_parse.output_json) {
            print_json_visual_object_descendants_result(result);
        } else {
            print_text_visual_object_descendants_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_children(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_children_parse = parse_visual_object_children_arguments(catalog, args);
    if (!(visual_object_children_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_children_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectChildrenListResult{
                .ok = false,
                .error = visual_object_children_parse.error,
                .parent_record_index = 0U,
                .parent_name = {},
                .children = {}
            };
            if (visual_object_children_parse.output_json) {
                print_json_visual_object_children_result(result);
            } else {
                print_text_visual_object_children_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::list_visual_object_children(
            visual_object_children_parse.request);
        if (visual_object_children_parse.output_json) {
            print_json_visual_object_children_result(result);
        } else {
            print_text_visual_object_children_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_object_list(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_object_list_parse = parse_visual_object_list_arguments(catalog, args);
    if (!(visual_object_list_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_object_list_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectListResult{
                .ok = false,
                .error = visual_object_list_parse.error,
                .objects = {}
            };
            if (visual_object_list_parse.output_json) {
                print_json_visual_object_list_result(result);
            } else {
                print_text_visual_object_list_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::list_visual_objects(visual_object_list_parse.path);
        if (visual_object_list_parse.output_json) {
            print_json_visual_object_list_result(result);
        } else {
            print_text_visual_object_list_result(result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
