// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

namespace cf_studio_host_main_detail {
std::string toolbox_parse_selection_batch_dispatch_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.ToolboxParse.Error.SelectionBatchDispatchItemRequiresToolboxItem",
        {{"toolboxItemOption", "--toolbox-item"}});
}

ToolboxInvocationAdmissionParseResult parse_toolbox_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxInvocationAdmissionParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-invocation-admission") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-invocation-admission") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-invocation-admission", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxDispatchParseResult parse_toolbox_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxDispatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-dispatch") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-dispatch") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-dispatch", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxExecuteParseResult parse_toolbox_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxExecuteParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-execute") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-execute") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.admit_palette_invocation = admitted;
        } else if (argument == "--admit-toolbox-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-toolbox-execution"));
                continue;
            }
            result.admit_execution = admitted;
        } else if (argument == "--toolbox-launch-command") {
            result.launch_command = require_value(argument);
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-execute", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.launch_command.empty()) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxLaunchCommand"));
    }
    return result;
}

ToolboxInvocationAdmissionCatalogParseResult parse_toolbox_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxInvocationAdmissionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--toolbox-invocation-admission-catalog") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-invocation-admission-catalog") {
            continue;
        }
        if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.request.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-invocation-admission-catalog", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxContext"));
    }
    return result;
}

SelectionToolboxInvocationAdmissionCatalogParseResult
parse_selection_toolbox_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxInvocationAdmissionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--selection-toolbox-invocation-admission-catalog") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-toolbox-invocation-admission-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.request.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-invocation-admission-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

ToolboxDispatchCatalogParseResult parse_toolbox_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-dispatch-catalog") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-dispatch-catalog") {
            continue;
        }
        if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.request.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-dispatch-catalog", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxContext"));
    }
    return result;
}

ToolboxDispatchExecutionCatalogParseResult parse_toolbox_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    ToolboxDispatchExecutionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-dispatch-execution-catalog") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--toolbox-dispatch-execution-catalog") {
            continue;
        }
        if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_toolbox_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.request.admit_palette_invocation = admitted;
        } else if (argument == "--admit-toolbox-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-toolbox-execution"));
                continue;
            }
            result.request.admit_execution = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "toolbox-dispatch-execution-catalog", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoToolboxContext"));
    }
    return result;
}

SelectionToolboxDispatchExecutionCatalogParseResult
parse_selection_toolbox_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxDispatchExecutionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-dispatch-execution-catalog")
        != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-toolbox-dispatch-execution-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.request.admit_palette_invocation = admitted;
        } else if (argument == "--admit-toolbox-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-toolbox-execution"));
                continue;
            }
            result.request.admit_execution = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-dispatch-execution-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

SelectionToolboxDispatchCatalogParseResult parse_selection_toolbox_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    SelectionToolboxDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--selection-toolbox-dispatch-catalog") != args.end();
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
                fail(toolbox_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--selection-toolbox-dispatch-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(toolbox_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.request.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(toolbox_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-palette-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(toolbox_parse_boolean_value_required(catalog, "--admit-palette-invocation"));
                continue;
            }
            result.request.admit_palette_invocation = admitted;
        } else {
            fail(toolbox_parse_unknown_option(catalog, "selection-toolbox-dispatch-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(toolbox_parse_message(catalog, "StudioHost.ToolboxParse.Error.NoSelectionContext"));
    }
    return result;
}

void print_json_toolbox_invocation_admission_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxInvocationAdmission\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    std::vector<std::string> admission_ready_item_ids;
    admission_ready_item_ids.reserve(plan.items.size());
    for (const auto& item : plan.items) {
        admission_ready_item_ids.push_back(std::string(item.id));
    }
    const std::vector<std::string> admission_blocked_item_ids;
    const std::vector<std::string> admission_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << plan.item_count << ",\n";
    std::cout << "    \"admissionReadyItemIds\": ";
    print_json_string_array(admission_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedItemIds\": ";
    print_json_string_array(admission_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < plan.items.size(); ++index) {
        print_json_toolbox_item_descriptor(plan.items[index], "      ");
        if ((index + 1U) != plan.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"paletteInvocationAdmitted\": "
              << (plan.palette_invocation_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_dispatch_result(const copperfin::studio::StudioToolboxDispatchResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxDispatch\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.plan;
    std::vector<std::string> dispatch_ready_item_ids;
    dispatch_ready_item_ids.reserve(plan.items.size());
    for (const auto& item : plan.items) {
        dispatch_ready_item_ids.push_back(std::string(item.id));
    }
    const std::vector<std::string> dispatch_blocked_item_ids;
    const std::vector<std::string> dispatch_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << plan.item_count << ",\n";
    std::cout << "    \"dispatchReadyItemIds\": ";
    print_json_string_array(dispatch_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedItemIds\": ";
    print_json_string_array(dispatch_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < plan.items.size(); ++index) {
        print_json_toolbox_item_descriptor(plan.items[index], "      ");
        if ((index + 1U) != plan.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"dispatchArguments\": [";
    for (std::size_t index = 0U; index < plan.dispatch_arguments.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_string(plan.dispatch_arguments[index]);
    }
    std::cout << "],\n";
    std::cout << "    \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (plan.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"executed\": " << (plan.executed ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (plan.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_execution_result(
    const copperfin::studio::StudioToolboxDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxExecution\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << ",\n";
        std::cout << "  \"executionAdmitted\": " << (result.execution_admitted ? "true" : "false") << ",\n";
        std::cout << "  \"executed\": " << (result.executed ? "true" : "false") << ",\n";
        std::cout << "  \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
        std::cout << "  \"launchCommand\": ";
        print_json_string(launch_command);
        std::cout << ",\n";
        std::cout << "  \"executedCommand\": ";
        print_json_string(executed_command);
        std::cout << ",\n";
        std::cout << "  \"observedExitCode\": " << result.observation.exit_code << "\n";
        std::cout << "}\n";
        return;
    }

    const auto& plan = result.dispatch_plan;
    std::vector<std::string> execution_ready_item_ids;
    execution_ready_item_ids.reserve(plan.items.size());
    for (const auto& item : plan.items) {
        execution_ready_item_ids.push_back(std::string(item.id));
    }
    const std::vector<std::string> execution_blocked_item_ids;
    const std::vector<std::string> execution_blocked_errors;

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(plan.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(plan.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(plan.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(plan.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << plan.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(plan.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(plan.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << plan.item_count << ",\n";
    std::cout << "    \"executionReadyItemIds\": ";
    print_json_string_array(execution_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedItemIds\": ";
    print_json_string_array(execution_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedErrors\": ";
    print_json_string_array(execution_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < plan.items.size(); ++index) {
        print_json_toolbox_item_descriptor(plan.items[index], "      ");
        if ((index + 1U) != plan.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"launchCommand\": ";
    print_json_string(launch_command);
    std::cout << ",\n";
    std::cout << "    \"executedCommand\": ";
    print_json_string(executed_command);
    std::cout << ",\n";
    std::cout << "    \"observedExitCode\": " << result.observation.exit_code << ",\n";
    std::cout << "    \"executionAdmitted\": " << (result.execution_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchAdmitted\": " << (plan.dispatch_admitted ? "true" : "false") << ",\n";
    std::cout << "    \"executed\": " << (result.executed ? "true" : "false") << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxInvocationAdmissionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> admission_ready_item_ids;
    std::vector<std::string> admission_blocked_item_ids;
    std::vector<std::string> admission_blocked_errors;
    for (const auto& item : result.items) {
        if (result.invocation_admission.ok) {
            admission_ready_item_ids.push_back(std::string(item.id));
        } else {
            admission_blocked_item_ids.push_back(std::string(item.id));
            admission_blocked_errors.push_back(result.invocation_admission.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"selectionContext\": ";
    print_json_string(copperfin::studio::studio_editor_selection_context_name(result.selection_context));
    std::cout << ",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(result.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(result.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"admissionCount\": " << result.admission_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"admissionReadyItemIds\": ";
    print_json_string_array(admission_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedItemIds\": ";
    print_json_string_array(admission_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"admissionBlockedErrors\": ";
    print_json_string_array(admission_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < result.items.size(); ++index) {
        print_json_toolbox_item_descriptor(result.items[index], "      ");
        if ((index + 1U) != result.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"invocationAdmissionOk\": "
              << (result.invocation_admission.ok ? "true" : "false") << ",\n";
    std::cout << "    \"paletteInvocationAdmitted\": "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << ",\n";
    std::cout << "    \"invocationAdmissionError\": ";
    print_json_string(result.invocation_admission.error);
    std::cout << "\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxDispatchCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxDispatchCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> dispatch_ready_item_ids;
    std::vector<std::string> dispatch_blocked_item_ids;
    std::vector<std::string> dispatch_blocked_errors;
    for (const auto& item : result.items) {
        if (result.dispatch.ok) {
            dispatch_ready_item_ids.push_back(std::string(item.id));
        } else {
            dispatch_blocked_item_ids.push_back(std::string(item.id));
            dispatch_blocked_errors.push_back(result.dispatch.error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(result.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(result.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"dispatchCount\": " << result.dispatch_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchReadyItemIds\": ";
    print_json_string_array(dispatch_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedItemIds\": ";
    print_json_string_array(dispatch_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"dispatchBlockedErrors\": ";
    print_json_string_array(dispatch_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < result.items.size(); ++index) {
        print_json_toolbox_item_descriptor(result.items[index], "      ");
        if ((index + 1U) != result.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"invocationAdmissionOk\": "
              << (result.invocation_admission.ok ? "true" : "false") << ",\n";
    std::cout << "    \"paletteInvocationAdmitted\": "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << ",\n";
    std::cout << "    \"dispatchOk\": " << (result.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchError\": ";
    print_json_string(result.dispatch.error);
    std::cout << ",\n";
    std::cout << "    \"dispatchArguments\": [";
    if (result.dispatch.ok) {
        for (std::size_t index = 0U; index < result.dispatch.plan.dispatch_arguments.size(); ++index) {
            if (index != 0U) {
                std::cout << ", ";
            }
            print_json_string(result.dispatch.plan.dispatch_arguments[index]);
        }
    }
    std::cout << "]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_json_toolbox_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioToolboxDispatchExecutionCatalogEntry& entry,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"item\": ";
    print_json_toolbox_item_descriptor(entry.item, indent + "  ");
    std::cout << ",\n";
    std::cout << indent << "  \"executionAdmitted\": "
              << (entry.execution_admitted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"executionReady\": " << (entry.execution_ready ? "true" : "false")
              << ",\n";
    std::cout << indent << "  \"executionError\": ";
    print_json_string(entry.execution_error);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioToolboxDispatchExecutionCatalogResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxDispatchExecutionCatalog\": ";
    if (!result.ok) {
        std::cout << "null,\n";
        std::cout << "  \"error\": ";
        print_json_string(result.error);
        std::cout << "\n";
        std::cout << "}\n";
        return;
    }

    std::vector<std::string> execution_ready_item_ids;
    std::vector<std::string> execution_blocked_item_ids;
    std::vector<std::string> execution_blocked_errors;
    for (const auto& entry : result.entries) {
        if (entry.execution_ready) {
            execution_ready_item_ids.push_back(std::string(entry.item.id));
        } else {
            execution_blocked_item_ids.push_back(std::string(entry.item.id));
            execution_blocked_errors.push_back(entry.execution_error);
        }
    }

    std::cout << "{\n";
    std::cout << "    \"ok\": true,\n";
    std::cout << "    \"error\": \"\",\n";
    std::cout << "    \"toolboxContext\": ";
    print_json_string(copperfin::studio::studio_toolbox_context_name(result.toolbox_context));
    std::cout << ",\n";
    std::cout << "    \"commandToken\": ";
    print_json_string(result.command_token);
    std::cout << ",\n";
    std::cout << "    \"assetPath\": ";
    print_json_string(result.asset_path);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
    std::cout << ",\n";
    std::cout << "    \"itemCount\": " << result.item_count << ",\n";
    std::cout << "    \"executionReadyCount\": " << result.execution_ready_count << ",\n";
    std::cout << "    \"errorCount\": " << result.error_count << ",\n";
    std::cout << "    \"dryRun\": " << (result.dry_run ? "true" : "false") << ",\n";
    std::cout << "    \"mutatesAsset\": " << (result.mutates_asset ? "true" : "false") << ",\n";
    std::cout << "    \"executionReadyItemIds\": ";
    print_json_string_array(execution_ready_item_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedItemIds\": ";
    print_json_string_array(execution_blocked_item_ids);
    std::cout << ",\n";
    std::cout << "    \"executionBlockedErrors\": ";
    print_json_string_array(execution_blocked_errors);
    std::cout << ",\n";
    std::cout << "    \"items\": [\n";
    for (std::size_t index = 0U; index < result.items.size(); ++index) {
        print_json_toolbox_item_descriptor(result.items[index], "      ");
        if ((index + 1U) != result.items.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"entries\": [\n";
    for (std::size_t index = 0U; index < result.entries.size(); ++index) {
        print_json_toolbox_dispatch_execution_catalog_entry(result.entries[index], "      ");
        if ((index + 1U) != result.entries.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"invocationAdmissionOk\": "
              << (result.invocation_admission.ok ? "true" : "false") << ",\n";
    std::cout << "    \"paletteInvocationAdmitted\": "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << ",\n";
    std::cout << "    \"dispatchOk\": " << (result.dispatch.ok ? "true" : "false") << ",\n";
    std::cout << "    \"dispatchError\": ";
    print_json_string(result.dispatch.error);
    std::cout << ",\n";
    std::cout << "    \"dispatchArguments\": [";
    if (result.dispatch.ok) {
        for (std::size_t index = 0U; index < result.dispatch.plan.dispatch_arguments.size(); ++index) {
            if (index != 0U) {
                std::cout << ", ";
            }
            print_json_string(result.dispatch.plan.dispatch_arguments[index]);
        }
    }
    std::cout << "]\n";
    std::cout << "  },\n";
    std::cout << "  \"error\": \"\"\n";
    std::cout << "}\n";
}

void print_text_toolbox_invocation_admission_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(plan.toolbox_context) << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "item_count: " << plan.item_count << "\n";
    for (const auto& item : plan.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    std::cout << "palette_invocation_admitted: "
              << (plan.palette_invocation_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(result.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "command_token: " << result.command_token << "\n";
    std::cout << "asset_path: " << result.asset_path << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "admission_count: " << result.admission_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& item : result.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    std::cout << "invocation_admission_ok: " << (result.invocation_admission.ok ? "true" : "false") << "\n";
    std::cout << "palette_invocation_admitted: "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << "\n";
    if (!result.invocation_admission.error.empty()) {
        std::cout << "invocation_admission_error: " << result.invocation_admission.error << "\n";
    }
}

void print_text_toolbox_dispatch_result(const copperfin::studio::StudioToolboxDispatchResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    const auto& plan = result.plan;
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(plan.toolbox_context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "item_count: " << plan.item_count << "\n";
    for (const auto& item : plan.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    for (const auto& argument : plan.dispatch_arguments) {
        std::cout << "dispatch_argument: " << argument << "\n";
    }
    std::cout << "dispatch_admitted: " << (plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (plan.dry_run ? "true" : "false") << "\n";
    std::cout << "executed: " << (plan.executed ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (plan.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_execution_result(
    const copperfin::studio::StudioToolboxDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    std::cout << "execution_admitted: " << (result.execution_admitted ? "true" : "false") << "\n";
    std::cout << "executed: " << (result.executed ? "true" : "false") << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "launch_command: " << launch_command << "\n";
    std::cout << "executed_command: " << executed_command << "\n";
    std::cout << "observed_exit_code: " << result.observation.exit_code << "\n";
    if (!result.ok) {
        return;
    }
    const auto& plan = result.dispatch_plan;
    std::cout << "selection_context: "
              << copperfin::studio::studio_editor_selection_context_name(plan.selection_context) << "\n";
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(plan.toolbox_context) << "\n";
    std::cout << "command_token: " << plan.command_token << "\n";
    std::cout << "asset_path: " << plan.asset_path << "\n";
    std::cout << "record_index: " << plan.record_index << "\n";
    std::cout << "object_name: " << plan.object_name << "\n";
    std::cout << "unique_id: " << plan.unique_id << "\n";
    std::cout << "item_count: " << plan.item_count << "\n";
    for (const auto& item : plan.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    std::cout << "dispatch_admitted: " << (plan.dispatch_admitted ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
}

void print_text_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxDispatchCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "command_token: " << result.command_token << "\n";
    std::cout << "asset_path: " << result.asset_path << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "dispatch_count: " << result.dispatch_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& item : result.items) {
        std::cout << "item: " << item.id << " " << item.title << "\n";
    }
    std::cout << "invocation_admission_ok: " << (result.invocation_admission.ok ? "true" : "false") << "\n";
    std::cout << "palette_invocation_admitted: "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << "\n";
    std::cout << "dispatch_ok: " << (result.dispatch.ok ? "true" : "false") << "\n";
    if (!result.dispatch.error.empty()) {
        std::cout << "dispatch_error: " << result.dispatch.error << "\n";
    }
    if (result.dispatch.ok) {
        for (const auto& argument : result.dispatch.plan.dispatch_arguments) {
            std::cout << "dispatch_argument: " << argument << "\n";
        }
    }
}

void print_text_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioToolboxDispatchExecutionCatalogResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << studio_error_prefix() << result.error << "\n";
    }
    if (!result.ok) {
        return;
    }
    std::cout << "toolbox_context: " << copperfin::studio::studio_toolbox_context_name(result.toolbox_context)
              << "\n";
    std::cout << "command_token: " << result.command_token << "\n";
    std::cout << "asset_path: " << result.asset_path << "\n";
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "item_count: " << result.item_count << "\n";
    std::cout << "execution_ready_count: " << result.execution_ready_count << "\n";
    std::cout << "error_count: " << result.error_count << "\n";
    std::cout << "dry_run: " << (result.dry_run ? "true" : "false") << "\n";
    std::cout << "mutates_asset: " << (result.mutates_asset ? "true" : "false") << "\n";
    for (const auto& entry : result.entries) {
        std::cout << "entry_item: " << entry.item.id << " " << entry.item.title << "\n";
        std::cout << "entry_execution_ready: " << (entry.execution_ready ? "true" : "false") << "\n";
        if (!entry.execution_error.empty()) {
            std::cout << "entry_execution_error: " << entry.execution_error << "\n";
        }
    }
    std::cout << "invocation_admission_ok: " << (result.invocation_admission.ok ? "true" : "false") << "\n";
    std::cout << "palette_invocation_admitted: "
              << (result.invocation_admission.ok && result.invocation_admission.plan.palette_invocation_admitted
                      ? "true"
                      : "false")
              << "\n";
    std::cout << "dispatch_ok: " << (result.dispatch.ok ? "true" : "false") << "\n";
    if (!result.dispatch.error.empty()) {
        std::cout << "dispatch_error: " << result.dispatch.error << "\n";
    }
}

std::optional<int> try_handle_toolbox_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_invocation_admission_parse = parse_toolbox_invocation_admission_arguments(catalog, args);
    if (!(toolbox_invocation_admission_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_invocation_admission_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxInvocationAdmissionResult{
                .ok = false,
                .error = toolbox_invocation_admission_parse.error,
                .plan = {}
            };
            if (toolbox_invocation_admission_parse.output_json) {
                print_json_toolbox_invocation_admission_result(result);
            } else {
                print_text_toolbox_invocation_admission_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_invocation_admission_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioToolboxInvocationAdmissionResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (toolbox_invocation_admission_parse.output_json) {
                print_json_toolbox_invocation_admission_result(result);
            } else {
                print_text_toolbox_invocation_admission_result(result);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation = toolbox_invocation_admission_parse.admit_palette_invocation
        });
        if (toolbox_invocation_admission_parse.output_json) {
            print_json_toolbox_invocation_admission_result(result);
        } else {
            print_text_toolbox_invocation_admission_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_invocation_admission_catalog_parse =
        parse_toolbox_invocation_admission_catalog_arguments(catalog, args);
    if (!(toolbox_invocation_admission_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_invocation_admission_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxInvocationAdmissionCatalogResult{
                .ok = false,
                .error = toolbox_invocation_admission_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .command_token = {},
                .asset_path = {},
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .item_count = 0U,
                .items = {},
                .invocation_admission = {},
                .admission_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_invocation_admission_catalog_parse.output_json) {
                print_json_toolbox_invocation_admission_catalog_result(result);
            } else {
                print_text_toolbox_invocation_admission_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_invocation_admission_catalog(
            toolbox_invocation_admission_catalog_parse.request);
        if (toolbox_invocation_admission_catalog_parse.output_json) {
            print_json_toolbox_invocation_admission_catalog_result(result);
        } else {
            print_text_toolbox_invocation_admission_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_invocation_admission_catalog_parse =
        parse_selection_toolbox_invocation_admission_catalog_arguments(catalog, args);
    if (!(selection_toolbox_invocation_admission_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_invocation_admission_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxInvocationAdmissionCatalogResult{
                .ok = false,
                .error = selection_toolbox_invocation_admission_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .command_token = {},
                .asset_path = {},
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .item_count = 0U,
                .items = {},
                .launch_plan = {},
                .invocation_admission = {},
                .admission_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_invocation_admission_catalog_parse.output_json) {
                print_json_selection_toolbox_invocation_admission_catalog_result(result);
            } else {
                print_text_selection_toolbox_invocation_admission_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_invocation_admission_catalog_for_selection(
            selection_toolbox_invocation_admission_catalog_parse.request);
        if (selection_toolbox_invocation_admission_catalog_parse.output_json) {
            print_json_selection_toolbox_invocation_admission_catalog_result(result);
        } else {
            print_text_selection_toolbox_invocation_admission_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_dispatch_parse = parse_toolbox_dispatch_arguments(catalog, args);
    if (!(toolbox_dispatch_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_dispatch_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchResult{
                .ok = false,
                .error = toolbox_dispatch_parse.error,
                .plan = {}
            };
            if (toolbox_dispatch_parse.output_json) {
                print_json_toolbox_dispatch_result(result);
            } else {
                print_text_toolbox_dispatch_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_dispatch_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchResult{
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            };
            if (toolbox_dispatch_parse.output_json) {
                print_json_toolbox_dispatch_result(result);
            } else {
                print_text_toolbox_dispatch_result(result);
            }
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation = toolbox_dispatch_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchResult{
                .ok = false,
                .error = admission_result.error,
                .plan = {}
            };
            if (toolbox_dispatch_parse.output_json) {
                print_json_toolbox_dispatch_result(result);
            } else {
                print_text_toolbox_dispatch_result(result);
            }
            return 4;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (toolbox_dispatch_parse.output_json) {
            print_json_toolbox_dispatch_result(result);
        } else {
            print_text_toolbox_dispatch_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_execute_parse = parse_toolbox_execute_arguments(catalog, args);
    if (!(toolbox_execute_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_execute_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchExecutionResult{
                .ok = false,
                .error = toolbox_execute_parse.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = toolbox_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_execute_parse.output_json) {
                print_json_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
            } else {
                print_text_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
                print_usage(catalog);
            }
            return 2;
        }

        const auto launch_result = copperfin::studio::plan_studio_toolbox_palette_launch(
            toolbox_execute_parse.request);
        if (!launch_result.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchExecutionResult{
                .ok = false,
                .error = launch_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = toolbox_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_execute_parse.output_json) {
                print_json_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
            } else {
                print_text_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
            }
            return 4;
        }

        const auto admission_result = copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_palette_invocation = toolbox_execute_parse.admit_palette_invocation
        });
        if (!admission_result.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchExecutionResult{
                .ok = false,
                .error = admission_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = toolbox_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_execute_parse.output_json) {
                print_json_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
            } else {
                print_text_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
            }
            return 4;
        }

        const auto dispatch_result = copperfin::studio::plan_studio_toolbox_dispatch({
            .admission_plan = admission_result.plan
        });
        if (!dispatch_result.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchExecutionResult{
                .ok = false,
                .error = dispatch_result.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = toolbox_execute_parse.admit_execution,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_execute_parse.output_json) {
                print_json_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
            } else {
                print_text_toolbox_execution_result(result, toolbox_execute_parse.launch_command, {});
            }
            return 4;
        }

        const std::string executed_command = build_shell_command(
            toolbox_execute_parse.launch_command,
            dispatch_result.plan.dispatch_arguments);
        const auto result = copperfin::studio::execute_studio_toolbox_dispatch({
            .dispatch_plan = dispatch_result.plan,
            .admit_execution = toolbox_execute_parse.admit_execution,
            .executor = [&](const copperfin::studio::StudioToolboxDispatchPlan& plan) {
                const int exit_code = execute_launch_command(
                    toolbox_execute_parse.launch_command,
                    plan.dispatch_arguments);
                return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                    .launched = true,
                    .exit_code = exit_code,
                    .output = {},
                    .error = exit_code == 0
                        ? std::string{}
                        : catalog.translate("StudioHost.ToolboxExecution.Error.LaunchCommandNonZeroExit"),
                    .mutates_asset = false
                };
            }
        });
        if (toolbox_execute_parse.output_json) {
            print_json_toolbox_execution_result(
                result,
                toolbox_execute_parse.launch_command,
                executed_command);
        } else {
            print_text_toolbox_execution_result(
                result,
                toolbox_execute_parse.launch_command,
                executed_command);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_dispatch_catalog_parse = parse_toolbox_dispatch_catalog_arguments(catalog, args);
    if (!(toolbox_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchCatalogResult{
                .ok = false,
                .error = toolbox_dispatch_catalog_parse.error,
                .toolbox_context = {},
                .command_token = {},
                .asset_path = {},
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .item_count = 0U,
                .items = {},
                .invocation_admission = {},
                .dispatch = {},
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (toolbox_dispatch_catalog_parse.output_json) {
                print_json_toolbox_dispatch_catalog_result(result);
            } else {
                print_text_toolbox_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_dispatch_catalog(
            toolbox_dispatch_catalog_parse.request);
        if (toolbox_dispatch_catalog_parse.output_json) {
            print_json_toolbox_dispatch_catalog_result(result);
        } else {
            print_text_toolbox_dispatch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_toolbox_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto toolbox_dispatch_execution_catalog_parse =
        parse_toolbox_dispatch_execution_catalog_arguments(catalog, args);
    if (!(toolbox_dispatch_execution_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!toolbox_dispatch_execution_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioToolboxDispatchExecutionCatalogResult{
                .ok = false,
                .error = toolbox_dispatch_execution_catalog_parse.error,
                .toolbox_context = {},
                .command_token = {},
                .asset_path = {},
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .item_count = 0U,
                .items = {},
                .invocation_admission = {},
                .dispatch = {},
                .execution_ready_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (toolbox_dispatch_execution_catalog_parse.output_json) {
                print_json_toolbox_dispatch_execution_catalog_result(result);
            } else {
                print_text_toolbox_dispatch_execution_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_dispatch_execution_catalog(
            toolbox_dispatch_execution_catalog_parse.request);
        if (toolbox_dispatch_execution_catalog_parse.output_json) {
            print_json_toolbox_dispatch_execution_catalog_result(result);
        } else {
            print_text_toolbox_dispatch_execution_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_dispatch_execution_catalog_parse =
        parse_selection_toolbox_dispatch_execution_catalog_arguments(catalog, args);
    if (!(selection_toolbox_dispatch_execution_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_dispatch_execution_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxDispatchExecutionCatalogResult{
                .ok = false,
                .error = selection_toolbox_dispatch_execution_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .command_token = {},
                .asset_path = {},
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .item_count = 0U,
                .items = {},
                .launch_plan = {},
                .invocation_admission = {},
                .dispatch = {},
                .execution_ready_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false,
                .entries = {}
            };
            if (selection_toolbox_dispatch_execution_catalog_parse.output_json) {
                print_json_selection_toolbox_dispatch_execution_catalog_result(result);
            } else {
                print_text_selection_toolbox_dispatch_execution_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_dispatch_execution_catalog_for_selection(
            selection_toolbox_dispatch_execution_catalog_parse.request);
        if (selection_toolbox_dispatch_execution_catalog_parse.output_json) {
            print_json_selection_toolbox_dispatch_execution_catalog_result(result);
        } else {
            print_text_selection_toolbox_dispatch_execution_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

std::optional<int> try_handle_selection_toolbox_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto selection_toolbox_dispatch_catalog_parse = parse_selection_toolbox_dispatch_catalog_arguments(catalog, args);
    if (!(selection_toolbox_dispatch_catalog_parse.requested)) {
        return std::nullopt;
    }

        if (!selection_toolbox_dispatch_catalog_parse.ok) {
            const auto result = copperfin::studio::StudioSelectionToolboxDispatchCatalogResult{
                .ok = false,
                .error = selection_toolbox_dispatch_catalog_parse.error,
                .selection_context = {},
                .toolbox_context = {},
                .command_token = {},
                .asset_path = {},
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .item_count = 0U,
                .items = {},
                .launch_plan = {},
                .invocation_admission = {},
                .dispatch = {},
                .dispatch_count = 0U,
                .error_count = 0U,
                .dry_run = true,
                .mutates_asset = false
            };
            if (selection_toolbox_dispatch_catalog_parse.output_json) {
                print_json_selection_toolbox_dispatch_catalog_result(result);
            } else {
                print_text_selection_toolbox_dispatch_catalog_result(result);
                print_usage(catalog);
            }
            return 2;
        }

        const auto result = copperfin::studio::plan_studio_toolbox_dispatch_catalog_for_selection(
            selection_toolbox_dispatch_catalog_parse.request);
        if (selection_toolbox_dispatch_catalog_parse.output_json) {
            print_json_selection_toolbox_dispatch_catalog_result(result);
        } else {
            print_text_selection_toolbox_dispatch_catalog_result(result);
        }
        return result.ok ? 0 : 4;
    }

}  // namespace cf_studio_host_main_detail
