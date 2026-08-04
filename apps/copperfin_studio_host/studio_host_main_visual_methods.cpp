// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string visual_method_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.VisualMethodParse.Error.MissingValue",
        {{"option", option}});
}

std::string visual_method_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return catalog.translate(
        "StudioHost.VisualMethodParse.Error.NonNegativeInteger",
        {{"option", option}});
}

std::string visual_method_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument) {
    return catalog.translate(
        "StudioHost.VisualMethodParse.Error.UnknownOption",
        {
            {"commandName", command_name},
            {"argument", argument}
        });
}

std::string visual_method_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

VisualMethodListParseResult parse_visual_method_list_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodListParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-list") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-list") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-list", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    return result;
}

VisualMethodQueryParseResult parse_visual_method_query_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodQueryParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-query") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-query") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--method-name") {
            result.request.method_name = require_value(argument);
            result.method_name_provided = !result.request.method_name.empty();
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-query", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodName"));
    }
    return result;
}

VisualMethodUpdateParseResult parse_visual_method_update_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodUpdateParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-update") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-update") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--method-name") {
            result.request.method_name = require_value(argument);
            result.method_name_provided = !result.request.method_name.empty();
        } else if (argument == "--method-kind") {
            result.request.method_kind = require_value(argument);
            result.method_kind_provided = !result.request.method_kind.empty();
        } else if (argument == "--method-source") {
            result.request.source_text = require_value(argument);
            result.method_source_provided = !result.request.source_text.empty();
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-update", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodName"));
    }
    if (result.ok && !result.method_kind_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodKind"));
    }
    if (result.ok && !result.method_source_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodSource"));
    }
    return result;
}

VisualMethodDeleteParseResult parse_visual_method_delete_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodDeleteParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-delete") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-delete") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--method-name") {
            result.request.method_name = require_value(argument);
            result.method_name_provided = !result.request.method_name.empty();
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-delete", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodName"));
    }
    return result;
}

VisualMethodDeleteBatchParseResult parse_visual_method_delete_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodDeleteBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-delete-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_method = [&]() -> copperfin::vfp::VisualObjectMethodDeleteBatchItem* {
        if (result.request.methods.empty()) {
            fail(catalog.translate(
                "StudioHost.VisualMethodParse.Error.DeleteBatchItemRequiresMethodName",
                {{"methodNameOption", "--method-name"}}));
            return nullptr;
        }
        return &result.request.methods.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-delete-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--method-name") {
            result.request.methods.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .method_name = require_value(argument)
            });
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            if (auto* method = current_method()) {
                method->record_index = record_index;
            }
        } else if (argument == "--object-name") {
            if (auto* method = current_method()) {
                method->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            if (auto* method = current_method()) {
                method->unique_id = require_value(argument);
            }
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-delete-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.methods.empty()) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodDeletes"));
    }
    return result;
}

VisualMethodRenameParseResult parse_visual_method_rename_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodRenameParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-rename") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-rename") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--method-name") {
            result.request.method_name = require_value(argument);
            result.method_name_provided = !result.request.method_name.empty();
        } else if (argument == "--new-method-name") {
            result.request.new_method_name = require_value(argument);
            result.new_method_name_provided = !result.request.new_method_name.empty();
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-rename", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodName"));
    }
    if (result.ok && !result.new_method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoTargetMethodName"));
    }
    return result;
}

VisualMethodRenameBatchParseResult parse_visual_method_rename_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodRenameBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-rename-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_method = [&]() -> copperfin::vfp::VisualObjectMethodRenameBatchItem* {
        if (result.request.methods.empty()) {
            fail(catalog.translate(
                "StudioHost.VisualMethodParse.Error.RenameBatchItemRequiresMethodName",
                {{"methodNameOption", "--method-name"}}));
            return nullptr;
        }
        return &result.request.methods.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-rename-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--method-name") {
            result.request.methods.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .method_name = require_value(argument),
                .new_method_name = {}
            });
        } else if (argument == "--new-method-name") {
            if (auto* method = current_method()) {
                method->new_method_name = require_value(argument);
            }
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            if (auto* method = current_method()) {
                method->record_index = record_index;
            }
        } else if (argument == "--object-name") {
            if (auto* method = current_method()) {
                method->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            if (auto* method = current_method()) {
                method->unique_id = require_value(argument);
            }
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-rename-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.methods.empty()) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodRenames"));
    }
    if (result.ok) {
        for (const auto& method : result.request.methods) {
            if (method.new_method_name.empty()) {
                fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoTargetMethodName"));
                break;
            }
        }
    }
    return result;
}

VisualMethodCopyParseResult parse_visual_method_copy_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodCopyParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-copy") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-copy") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            result.request.source_record_index = record_index;
        } else if (argument == "--source-object-name") {
            result.request.source_object_name = require_value(argument);
        } else if (argument == "--source-unique-id") {
            result.request.source_unique_id = require_value(argument);
        } else if (argument == "--method-name") {
            result.request.source_method_name = require_value(argument);
            result.method_name_provided = !result.request.source_method_name.empty();
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            result.request.target_record_index = record_index;
        } else if (argument == "--target-object-name") {
            result.request.target_object_name = require_value(argument);
        } else if (argument == "--target-unique-id") {
            result.request.target_unique_id = require_value(argument);
        } else if (argument == "--target-method-name") {
            result.request.target_method_name = require_value(argument);
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(catalog.translate(
                    "StudioHost.VisualMethodParse.Error.BooleanValue",
                    {
                        {"option", "--replace-existing"},
                        {"trueToken", "true"},
                        {"falseToken", "false"}
                    }));
                continue;
            }
            result.request.replace_existing = replace_existing;
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-copy", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodName"));
    }
    return result;
}

VisualMethodCopyBatchParseResult parse_visual_method_copy_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodCopyBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-copy-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_method = [&]() -> copperfin::vfp::VisualObjectMethodCopyBatchItem* {
        if (result.request.methods.empty()) {
            fail(catalog.translate(
                "StudioHost.VisualMethodParse.Error.CopyBatchItemRequiresMethodName",
                {{"methodNameOption", "--method-name"}}));
            return nullptr;
        }
        return &result.request.methods.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-copy-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--method-name") {
            result.request.methods.push_back({
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = {},
                .source_method_name = require_value(argument),
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_method_name = {},
                .replace_existing = false
            });
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            if (auto* method = current_method()) {
                method->source_record_index = record_index;
            }
        } else if (argument == "--source-object-name") {
            if (auto* method = current_method()) {
                method->source_object_name = require_value(argument);
            }
        } else if (argument == "--source-unique-id") {
            if (auto* method = current_method()) {
                method->source_unique_id = require_value(argument);
            }
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            if (auto* method = current_method()) {
                method->target_record_index = record_index;
            }
        } else if (argument == "--target-object-name") {
            if (auto* method = current_method()) {
                method->target_object_name = require_value(argument);
            }
        } else if (argument == "--target-unique-id") {
            if (auto* method = current_method()) {
                method->target_unique_id = require_value(argument);
            }
        } else if (argument == "--target-method-name") {
            if (auto* method = current_method()) {
                method->target_method_name = require_value(argument);
            }
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(catalog.translate(
                    "StudioHost.VisualMethodParse.Error.BooleanValue",
                    {
                        {"option", "--replace-existing"},
                        {"trueToken", "true"},
                        {"falseToken", "false"}
                    }));
                continue;
            }
            if (auto* method = current_method()) {
                method->replace_existing = replace_existing;
            }
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-copy-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.methods.empty()) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodCopies"));
    }
    return result;
}

VisualMethodMoveBatchParseResult parse_visual_method_move_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodMoveBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-move-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_method = [&]() -> copperfin::vfp::VisualObjectMethodMoveBatchItem* {
        if (result.request.methods.empty()) {
            fail(catalog.translate(
                "StudioHost.VisualMethodParse.Error.MoveBatchItemRequiresMethodName",
                {{"methodNameOption", "--method-name"}}));
            return nullptr;
        }
        return &result.request.methods.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-move-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--method-name") {
            result.request.methods.push_back({
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = {},
                .source_method_name = require_value(argument),
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_method_name = {},
                .replace_existing = false
            });
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            if (auto* method = current_method()) {
                method->source_record_index = record_index;
            }
        } else if (argument == "--source-object-name") {
            if (auto* method = current_method()) {
                method->source_object_name = require_value(argument);
            }
        } else if (argument == "--source-unique-id") {
            if (auto* method = current_method()) {
                method->source_unique_id = require_value(argument);
            }
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            if (auto* method = current_method()) {
                method->target_record_index = record_index;
            }
        } else if (argument == "--target-object-name") {
            if (auto* method = current_method()) {
                method->target_object_name = require_value(argument);
            }
        } else if (argument == "--target-unique-id") {
            if (auto* method = current_method()) {
                method->target_unique_id = require_value(argument);
            }
        } else if (argument == "--target-method-name") {
            if (auto* method = current_method()) {
                method->target_method_name = require_value(argument);
            }
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(catalog.translate(
                    "StudioHost.VisualMethodParse.Error.BooleanValue",
                    {
                        {"option", "--replace-existing"},
                        {"trueToken", "true"},
                        {"falseToken", "false"}
                    }));
                continue;
            }
            if (auto* method = current_method()) {
                method->replace_existing = replace_existing;
            }
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-move-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.methods.empty()) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodMoves"));
    }
    return result;
}

VisualMethodMoveParseResult parse_visual_method_move_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodMoveParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-move") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-move") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--source-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--source-record"));
                continue;
            }
            result.request.source_record_index = record_index;
        } else if (argument == "--source-object-name") {
            result.request.source_object_name = require_value(argument);
        } else if (argument == "--source-unique-id") {
            result.request.source_unique_id = require_value(argument);
        } else if (argument == "--method-name") {
            result.request.source_method_name = require_value(argument);
            result.method_name_provided = !result.request.source_method_name.empty();
        } else if (argument == "--target-record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--target-record"));
                continue;
            }
            result.request.target_record_index = record_index;
        } else if (argument == "--target-object-name") {
            result.request.target_object_name = require_value(argument);
        } else if (argument == "--target-unique-id") {
            result.request.target_unique_id = require_value(argument);
        } else if (argument == "--target-method-name") {
            result.request.target_method_name = require_value(argument);
        } else if (argument == "--replace-existing") {
            const std::string token = require_value(argument);
            bool replace_existing = false;
            if (!parse_bool_token(token, replace_existing)) {
                fail(catalog.translate(
                    "StudioHost.VisualMethodParse.Error.BooleanValue",
                    {
                        {"option", "--replace-existing"},
                        {"trueToken", "true"},
                        {"falseToken", "false"}
                    }));
                continue;
            }
            result.request.replace_existing = replace_existing;
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-move", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodName"));
    }
    return result;
}

VisualMethodReorderParseResult parse_visual_method_reorder_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodReorderParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-reorder") != args.end();
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
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-reorder") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--method-name") {
            result.request.method_name = require_value(argument);
            result.method_name_provided = !result.request.method_name.empty();
        } else if (argument == "--placement") {
            result.request.placement = require_value(argument);
            result.placement_provided = !result.request.placement.empty();
        } else if (argument == "--relative-method-name") {
            result.request.relative_method_name = require_value(argument);
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-reorder", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && !result.method_name_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodName"));
    }
    if (result.ok && !result.placement_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodPlacement"));
    }
    return result;
}

VisualMethodReorderBatchParseResult parse_visual_method_reorder_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    VisualMethodReorderBatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--visual-method-reorder-batch") != args.end();
    if (!result.requested) {
        return result;
    }

    auto fail = [&](std::string error) {
        result.ok = false;
        result.error = std::move(error);
    };

    auto current_method = [&]() -> copperfin::vfp::VisualObjectMethodReorderBatchItem* {
        if (result.request.methods.empty()) {
            fail(catalog.translate(
                "StudioHost.VisualMethodParse.Error.ReorderBatchItemRequiresMethodName",
                {{"methodNameOption", "--method-name"}}));
            return nullptr;
        }
        return &result.request.methods.back();
    };

    for (std::size_t index = 0U; index < args.size() && result.ok; ++index) {
        const std::string& argument = args[index];
        auto require_value = [&](const std::string& option) -> std::string {
            if ((index + 1U) >= args.size() || args[index + 1U].rfind("--", 0U) == 0U) {
                fail(visual_method_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--visual-method-reorder-batch") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
            result.path_provided = !result.request.path.empty();
        } else if (argument == "--method-name") {
            result.request.methods.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .method_name = require_value(argument),
                .placement = {},
                .relative_method_name = {}
            });
        } else if (argument == "--placement") {
            if (auto* method = current_method()) {
                method->placement = require_value(argument);
            }
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(visual_method_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            if (auto* method = current_method()) {
                method->record_index = record_index;
            }
        } else if (argument == "--object-name") {
            if (auto* method = current_method()) {
                method->object_name = require_value(argument);
            }
        } else if (argument == "--unique-id") {
            if (auto* method = current_method()) {
                method->unique_id = require_value(argument);
            }
        } else if (argument == "--relative-method-name") {
            if (auto* method = current_method()) {
                method->relative_method_name = require_value(argument);
            }
        } else {
            fail(visual_method_parse_unknown_option(catalog, "visual-method-reorder-batch", argument));
        }
    }

    if (result.ok && !result.path_provided) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoAssetPath"));
    }
    if (result.ok && result.request.methods.empty()) {
        fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodReorders"));
    }
    if (result.ok) {
        for (const auto& method : result.request.methods) {
            if (method.placement.empty()) {
                fail(visual_method_parse_message(catalog, "StudioHost.VisualMethodParse.Error.NoMethodPlacement"));
                break;
            }
        }
    }
    return result;
}

void print_json_visual_method_snapshot(const copperfin::vfp::VisualObjectMethodSnapshot& method,
                                       const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"methodName\": ";
    print_json_string(method.method_name);
    std::cout << ",\n";
    std::cout << indent << "  \"kind\": ";
    print_json_string(method.kind);
    std::cout << ",\n";
    std::cout << indent << "  \"sourceText\": ";
    print_json_string(method.source_text);
    std::cout << ",\n";
    std::cout << indent << "  \"sourceLineIndex\": ";
    if (method.source_line_index == static_cast<std::size_t>(-1)) {
        std::cout << "null";
    } else {
        std::cout << method.source_line_index;
    }
    std::cout << ",\n";
    std::cout << indent << "  \"sourceMemoBlockNumber\": " << method.source_memo_block_number << "\n";
    std::cout << indent << "}";
}

void print_json_visual_method_list_result(
    const copperfin::vfp::VisualObjectMethodListResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualMethodList\": ";
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
    std::cout << "    \"methodCount\": " << result.methods.size() << ",\n";
    std::cout << "    \"dryRun\": true,\n";
    std::cout << "    \"mutatesAsset\": false,\n";
    std::cout << "    \"methods\": [\n";
    for (std::size_t index = 0U; index < result.methods.size(); ++index) {
        print_json_visual_method_snapshot(result.methods[index], "      ");
        if ((index + 1U) != result.methods.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_method_query_result(
    const copperfin::vfp::VisualObjectMethodQueryResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"visualMethodQuery\": ";
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
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"recordDeleted\": " << (result.record_deleted ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": true,\n";
    std::cout << "    \"mutatesAsset\": false,\n";
    std::cout << "    \"method\": ";
    if (result.exists) {
        print_json_visual_method_snapshot(result.method, "    ");
        std::cout << "\n";
    } else {
        std::cout << "null\n";
    }
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_visual_method_update_result(
    const copperfin::vfp::VisualAssetEditResult& result,
    const copperfin::vfp::VisualAssetUndoStatus& undo_status,
    const std::string& result_name,
    const std::optional<bool> launched_from_visual_studio) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    if (launched_from_visual_studio.has_value()) {
        std::cout << "  \"launchedFromVisualStudio\": "
                  << (*launched_from_visual_studio ? "true" : "false") << ",\n";
    }
    std::cout << "  ";
    print_json_string(result_name);
    std::cout << ": ";
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
    std::cout << "    \"affectedObjectCount\": " << result.affected_object_count << ",\n";
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

void print_text_visual_method_list_result(
    const copperfin::vfp::VisualObjectMethodListResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "record_deleted: " << (result.record_deleted ? "true" : "false") << "\n";
    std::cout << "method_count: " << result.methods.size() << "\n";
    std::cout << "dry_run: true\n";
    std::cout << "mutates_asset: false\n";
    for (const auto& method : result.methods) {
        std::cout << "method: " << method.method_name << " " << method.kind
                  << " " << method.source_line_index << "\n";
    }
}

void print_text_visual_method_query_result(
    const copperfin::vfp::VisualObjectMethodQueryResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "exists: " << (result.exists ? "true" : "false") << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "record_deleted: " << (result.record_deleted ? "true" : "false") << "\n";
    std::cout << "dry_run: true\n";
    std::cout << "mutates_asset: false\n";
    if (result.exists) {
        std::cout << "method: " << result.method.method_name << " "
                  << result.method.kind << " " << result.method.source_line_index << "\n";
    }
}

void print_text_visual_method_update_result(
    const copperfin::vfp::VisualAssetEditResult& result,
    const copperfin::vfp::VisualAssetUndoStatus& undo_status) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "affected_object_count: " << result.affected_object_count << "\n";
    std::cout << "dry_run: false\n";
    std::cout << "mutates_asset: true\n";
    std::cout << "undo_available: " << (undo_status.available ? "true" : "false") << "\n";
    std::cout << "undo_label: " << undo_status.label << "\n";
}

std::optional<int> try_handle_visual_method_reorder_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_reorder_batch_parse = parse_visual_method_reorder_batch_arguments(catalog, args);
    if (!(visual_method_reorder_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_reorder_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_reorder_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_reorder_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodReorderBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::reorder_visual_object_methods(
            visual_method_reorder_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_reorder_batch_parse.request.path);
        if (visual_method_reorder_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodReorderBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_reorder(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_reorder_parse = parse_visual_method_reorder_arguments(catalog, args);
    if (!(visual_method_reorder_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_reorder_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_reorder_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_reorder_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodReorder");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::reorder_visual_object_method(
            visual_method_reorder_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_reorder_parse.request.path);
        if (visual_method_reorder_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodReorder");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_delete_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_delete_batch_parse = parse_visual_method_delete_batch_arguments(catalog, args);
    if (!(visual_method_delete_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_delete_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_delete_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_delete_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodDeleteBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::delete_visual_object_methods(
            visual_method_delete_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_delete_batch_parse.request.path);
        if (visual_method_delete_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodDeleteBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_rename_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_rename_batch_parse = parse_visual_method_rename_batch_arguments(catalog, args);
    if (!(visual_method_rename_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_rename_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_rename_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_rename_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodRenameBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::rename_visual_object_methods(
            visual_method_rename_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_rename_batch_parse.request.path);
        if (visual_method_rename_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodRenameBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_copy_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_copy_batch_parse = parse_visual_method_copy_batch_arguments(catalog, args);
    if (!(visual_method_copy_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_copy_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_copy_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_copy_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodCopyBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::copy_visual_object_methods(
            visual_method_copy_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_copy_batch_parse.request.path);
        if (visual_method_copy_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodCopyBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_move_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_move_batch_parse = parse_visual_method_move_batch_arguments(catalog, args);
    if (!(visual_method_move_batch_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_move_batch_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_move_batch_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_move_batch_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodMoveBatch");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::move_visual_object_methods(
            visual_method_move_batch_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_move_batch_parse.request.path);
        if (visual_method_move_batch_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodMoveBatch");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_move(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_move_parse = parse_visual_method_move_arguments(catalog, args);
    if (!(visual_method_move_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_move_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_move_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_move_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodMove");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::move_visual_object_method(
            visual_method_move_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_move_parse.request.path);
        if (visual_method_move_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodMove");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_copy(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_copy_parse = parse_visual_method_copy_arguments(catalog, args);
    if (!(visual_method_copy_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_copy_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_copy_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_copy_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodCopy");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::copy_visual_object_method(
            visual_method_copy_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_copy_parse.request.path);
        if (visual_method_copy_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodCopy");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_rename(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_rename_parse = parse_visual_method_rename_arguments(catalog, args);
    if (!(visual_method_rename_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_rename_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_rename_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_rename_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodRename");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::rename_visual_object_method(
            visual_method_rename_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_rename_parse.request.path);
        if (visual_method_rename_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodRename");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_delete(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_delete_parse = parse_visual_method_delete_arguments(catalog, args);
    if (!(visual_method_delete_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_delete_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_delete_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_delete_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status, "visualMethodDelete");
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::delete_visual_object_method(
            visual_method_delete_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_delete_parse.request.path);
        if (visual_method_delete_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status, "visualMethodDelete");
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_update(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_update_parse = parse_visual_method_update_arguments(catalog, args);
    if (!(visual_method_update_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_update_parse.ok) {
            const auto result = copperfin::vfp::VisualAssetEditResult{
                .ok = false,
                .error = visual_method_update_parse.error,
                .affected_object_count = 0U
            };
            const auto undo_status = copperfin::vfp::VisualAssetUndoStatus{};
            if (visual_method_update_parse.output_json) {
                print_json_visual_method_update_result(result, undo_status);
            } else {
                print_text_visual_method_update_result(result, undo_status);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::update_visual_object_method(
            visual_method_update_parse.request);
        const auto undo_status = copperfin::vfp::query_visual_object_undo(
            visual_method_update_parse.request.path);
        if (visual_method_update_parse.output_json) {
            print_json_visual_method_update_result(result, undo_status);
        } else {
            print_text_visual_method_update_result(result, undo_status);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_query(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_query_parse = parse_visual_method_query_arguments(catalog, args);
    if (!(visual_method_query_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_query_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectMethodQueryResult{
                .ok = false,
                .error = visual_method_query_parse.error,
                .exists = false,
                .record_index = 0U,
                .record_deleted = false,
                .method = {}
            };
            if (visual_method_query_parse.output_json) {
                print_json_visual_method_query_result(result);
            } else {
                print_text_visual_method_query_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::query_visual_object_method(
            visual_method_query_parse.request);
        if (visual_method_query_parse.output_json) {
            print_json_visual_method_query_result(result);
        } else {
            print_text_visual_method_query_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_visual_method_list(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto visual_method_list_parse = parse_visual_method_list_arguments(catalog, args);
    if (!(visual_method_list_parse.requested)) {
        return std::nullopt;
    }

        if (!visual_method_list_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectMethodListResult{
                .ok = false,
                .error = visual_method_list_parse.error,
                .record_index = 0U,
                .record_deleted = false,
                .methods = {}
            };
            if (visual_method_list_parse.output_json) {
                print_json_visual_method_list_result(result);
            } else {
                print_text_visual_method_list_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::vfp::list_visual_object_methods(
            visual_method_list_parse.request);
        if (visual_method_list_parse.output_json) {
            print_json_visual_method_list_result(result);
        } else {
            print_text_visual_method_list_result(result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
