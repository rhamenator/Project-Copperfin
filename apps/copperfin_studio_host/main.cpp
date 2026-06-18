#include "copperfin/studio/document_model.h"
#include "copperfin/platform/database_model.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/project_workspace.h"
#include "copperfin/studio/product_subsystems.h"
#include "copperfin/studio/report_layout.h"
#include "copperfin/studio/toolbox_creation.h"
#include "copperfin/studio/vs_launch_contract.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

void print_usage() {
    std::cout << "Usage: copperfin_studio_host --path <asset> [--from-vs] [--read-only] [--json] [--selection-context <token>] [--delete-object|--restore-object|--duplicate-object|--rename-object|--reparent-object|--reorder-object|--group-object|--align-object|--resize-object|--distribute-object|--snap-object|--nudge-object|--tab-order-object|--tab-stop-object|--visibility-object|--enabled-object|--read-only-object|--locked-object|--caption-object|--picture-object|--down-picture-object|--disabled-picture-object|--ole-drag-picture-object|--tooltip-text-object|--status-bar-text-object|--control-source-object|--input-mask-object|--format-object|--row-source-object|--row-source-type-object|--bound-column-object|--column-count-object|--style-object|--list-index-object|--left-column-object|--auto-center-object|--auto-size-object|--auto-release-object|--continuous-scroll-object|--dockable-object|--clip-controls-object|--sparse-object|--lock-screen-object|--allow-cell-selection-object|--delete-mark-object|--record-mark-object|--split-bar-object|--highlight-row-object|--panel-link-object|--allow-header-sizing-object|--allow-row-sizing-object|--resizable-object|--add-line-feeds-object|--always-on-top-object|--always-on-bottom-object|--ungroup-object] [--set-property|--clear-property|--rename-property --record <n> --object-name <name> --unique-id <id> --property-name <name> --property-value <value> --new-property-name <name>] [--new-object-name <name>] [--new-name <name>] [--new-unique-id <id>] [--parent-name <name>] [--parent-unique-id <id>] [--clear-parent] [--placement <front|back|before|after>] [--target-object-name <name>] [--target-unique-id <id>] [--group-child-object-name <name>] [--group-child-unique-id <id>] [--field-value <name=value>] [--alignment-mode <mode>] [--resize-mode <width|height|size>] [--distribution-mode <horizontal|vertical>] [--snap-mode <horizontal|vertical|both>] [--nudge-mode <horizontal|vertical|both>] [--grid-width <n>] [--grid-height <n>] [--delta-hpos <n>] [--delta-vpos <n>] [--starting-tab-index <n>] [--tab-stop <true|false>] [--visible <true|false>] [--enabled <true|false>] [--object-read-only <true|false>] [--locked <true|false>] [--caption <value>] [--picture <value>] [--down-picture <value>] [--disabled-picture <value>] [--ole-drag-picture <value>] [--tooltip-text <value>] [--status-bar-text <value>] [--control-source <value>] [--input-mask <value>] [--format <value>] [--row-source <value>] [--row-source-type <n>] [--bound-column <n>] [--column-count <n>] [--style <n>] [--list-index <n>] [--left-column <n>] [--auto-center <true|false>] [--auto-size <true|false>] [--auto-release <true|false>] [--continuous-scroll <true|false>] [--dockable <true|false>] [--clip-controls <true|false>] [--sparse <true|false>] [--lock-screen <true|false>] [--allow-cell-selection <true|false>] [--delete-mark <true|false>] [--record-mark <true|false>] [--split-bar <true|false>] [--highlight-row <true|false>] [--panel-link <true|false>] [--allow-header-sizing <true|false>] [--allow-row-sizing <true|false>] [--resizable <true|false>] [--add-line-feeds <true|false>] [--always-on-top <true|false>] [--always-on-bottom <true|false>] [--anchor-object-name <name>] [--anchor-unique-id <id>] [--align-target-object-name <name>] [--align-target-unique-id <id>] [--resize-target-object-name <name>] [--resize-target-unique-id <id>] [--distribute-target-object-name <name>] [--distribute-target-unique-id <id>] [--snap-target-object-name <name>] [--snap-target-unique-id <id>] [--nudge-target-object-name <name>] [--nudge-target-unique-id <id>] [--tab-order-target-object-name <name>] [--tab-order-target-unique-id <id>] [--tab-stop-target-object-name <name>] [--tab-stop-target-unique-id <id>] [--visibility-target-object-name <name>] [--visibility-target-unique-id <id>] [--enabled-target-object-name <name>] [--enabled-target-unique-id <id>] [--read-only-target-object-name <name>] [--read-only-target-unique-id <id>] [--locked-target-object-name <name>] [--locked-target-unique-id <id>] [--caption-target-object-name <name>] [--caption-target-unique-id <id>] [--picture-target-object-name <name>] [--picture-target-unique-id <id>] [--down-picture-target-object-name <name>] [--down-picture-target-unique-id <id>] [--disabled-picture-target-object-name <name>] [--disabled-picture-target-unique-id <id>] [--ole-drag-picture-target-object-name <name>] [--ole-drag-picture-target-unique-id <id>] [--tooltip-text-target-object-name <name>] [--tooltip-text-target-unique-id <id>] [--status-bar-text-target-object-name <name>] [--status-bar-text-target-unique-id <id>] [--control-source-target-object-name <name>] [--control-source-target-unique-id <id>] [--input-mask-target-object-name <name>] [--input-mask-target-unique-id <id>] [--format-target-object-name <name>] [--format-target-unique-id <id>] [--row-source-target-object-name <name>] [--row-source-target-unique-id <id>] [--row-source-type-target-object-name <name>] [--row-source-type-target-unique-id <id>] [--bound-column-target-object-name <name>] [--bound-column-target-unique-id <id>] [--column-count-target-object-name <name>] [--column-count-target-unique-id <id>] [--style-target-object-name <name>] [--style-target-unique-id <id>] [--list-index-target-object-name <name>] [--list-index-target-unique-id <id>] [--left-column-target-object-name <name>] [--left-column-target-unique-id <id>] [--auto-center-target-object-name <name>] [--auto-center-target-unique-id <id>] [--auto-size-target-object-name <name>] [--auto-size-target-unique-id <id>] [--auto-release-target-object-name <name>] [--auto-release-target-unique-id <id>] [--continuous-scroll-target-object-name <name>] [--continuous-scroll-target-unique-id <id>] [--dockable-target-object-name <name>] [--dockable-target-unique-id <id>] [--clip-controls-target-object-name <name>] [--clip-controls-target-unique-id <id>] [--sparse-target-object-name <name>] [--sparse-target-unique-id <id>] [--lock-screen-target-object-name <name>] [--lock-screen-target-unique-id <id>] [--allow-cell-selection-target-object-name <name>] [--allow-cell-selection-target-unique-id <id>] [--delete-mark-target-object-name <name>] [--delete-mark-target-unique-id <id>] [--record-mark-target-object-name <name>] [--record-mark-target-unique-id <id>] [--split-bar-target-object-name <name>] [--split-bar-target-unique-id <id>] [--highlight-row-target-object-name <name>] [--highlight-row-target-unique-id <id>] [--panel-link-target-object-name <name>] [--panel-link-target-unique-id <id>] [--allow-header-sizing-target-object-name <name>] [--allow-header-sizing-target-unique-id <id>] [--allow-row-sizing-target-object-name <name>] [--allow-row-sizing-target-unique-id <id>] [--resizable-target-object-name <name>] [--resizable-target-unique-id <id>] [--add-line-feeds-target-object-name <name>] [--add-line-feeds-target-unique-id <id>] [--always-on-top-target-object-name <name>] [--always-on-top-target-unique-id <id>] [--always-on-bottom-target-object-name <name>] [--always-on-bottom-target-unique-id <id>] [--line <n>] [--column <n>] [--symbol <name>]\n";
    std::cout << "   or: copperfin_studio_host --path <asset> --toolbox-create <id> [--toolbox-context <token>] [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] [--json]\n";
    std::cout << "Display-value object: --display-value-object --display-value <value> [--display-value-target-object-name <name>] [--display-value-target-unique-id <id>]\n";
    std::cout << "Selected-back-color object: --selected-back-color-object --selected-back-color <n> [--selected-back-color-target-object-name <name>] [--selected-back-color-target-unique-id <id>]\n";
    std::cout << "Selected-fore-color object: --selected-fore-color-object --selected-fore-color <n> [--selected-fore-color-target-object-name <name>] [--selected-fore-color-target-unique-id <id>]\n";
    std::cout << "Selected-item-back-color object: --selected-item-back-color-object --selected-item-back-color <n> [--selected-item-back-color-target-object-name <name>] [--selected-item-back-color-target-unique-id <id>]\n";
    std::cout << "Selected-item-fore-color object: --selected-item-fore-color-object --selected-item-fore-color <n> [--selected-item-fore-color-target-object-name <name>] [--selected-item-fore-color-target-unique-id <id>]\n";
    std::cout << "Disabled-item-back-color object: --disabled-item-back-color-object --disabled-item-back-color <n> [--disabled-item-back-color-target-object-name <name>] [--disabled-item-back-color-target-unique-id <id>]\n";
    std::cout << "Disabled-item-fore-color object: --disabled-item-fore-color-object --disabled-item-fore-color <n> [--disabled-item-fore-color-target-object-name <name>] [--disabled-item-fore-color-target-unique-id <id>]\n";
    std::cout << "Item-back-color object: --item-back-color-object --item-back-color <n> [--item-back-color-target-object-name <name>] [--item-back-color-target-unique-id <id>]\n";
    std::cout << "Item-fore-color object: --item-fore-color-object --item-fore-color <n> [--item-fore-color-target-object-name <name>] [--item-fore-color-target-unique-id <id>]\n";
    std::cout << "Highlight-back-color object: --highlight-back-color-object --highlight-back-color <n> [--highlight-back-color-target-object-name <name>] [--highlight-back-color-target-unique-id <id>]\n";
    std::cout << "Highlight-fore-color object: --highlight-fore-color-object --highlight-fore-color <n> [--highlight-fore-color-target-object-name <name>] [--highlight-fore-color-target-unique-id <id>]\n";
    std::cout << "Back-color object: --back-color-object --back-color <n> [--back-color-target-object-name <name>] [--back-color-target-unique-id <id>]\n";
    std::cout << "Fore-color object: --fore-color-object --fore-color <n> [--fore-color-target-object-name <name>] [--fore-color-target-unique-id <id>]\n";
    std::cout << "Disabled-back-color object: --disabled-back-color-object --disabled-back-color <n> [--disabled-back-color-target-object-name <name>] [--disabled-back-color-target-unique-id <id>]\n";
    std::cout << "Disabled-fore-color object: --disabled-fore-color-object --disabled-fore-color <n> [--disabled-fore-color-target-object-name <name>] [--disabled-fore-color-target-unique-id <id>]\n";
    std::cout << "Dynamic-back-color object: --dynamic-back-color-object --dynamic-back-color <expr> [--dynamic-back-color-target-object-name <name>] [--dynamic-back-color-target-unique-id <id>]\n";
    std::cout << "Dynamic-fore-color object: --dynamic-fore-color-object --dynamic-fore-color <expr> [--dynamic-fore-color-target-object-name <name>] [--dynamic-fore-color-target-unique-id <id>]\n";
    std::cout << "   or: copperfin_studio_host --list-subsystems [--json]\n";
    std::cout << "   or: copperfin_studio_host <asset>\n";
    std::cout << "Selection context tokens: visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, data_environment\n";
}

std::string json_escape(const std::string& value) {
    std::ostringstream stream;
    for (unsigned char ch : value) {
        switch (ch) {
            case '\"':
                stream << "\\\"";
                break;
            case '\\':
                stream << "\\\\";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            default:
                if (ch < 0x20U) {
                    stream << "\\u"
                           << std::hex
                           << std::setw(4)
                           << std::setfill('0')
                           << static_cast<unsigned int>(ch)
                           << std::dec
                           << std::setfill(' ');
                } else {
                    stream << static_cast<char>(ch);
                }
                break;
        }
    }
    return stream.str();
}

void print_json_string(const std::string& value) {
    std::cout << "\"" << json_escape(value) << "\"";
}

void print_json_string_view(std::string_view value) {
    print_json_string(std::string(value));
}

struct ToolboxCreateParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateRequest request;
};

bool parse_toolbox_context_token(
    const std::string& token,
    copperfin::studio::StudioToolboxContext& context) {
    for (const auto candidate : {
             copperfin::studio::StudioToolboxContext::form,
             copperfin::studio::StudioToolboxContext::class_designer,
             copperfin::studio::StudioToolboxContext::container,
             copperfin::studio::StudioToolboxContext::report
         }) {
        if (token == copperfin::studio::studio_toolbox_context_name(candidate)) {
            context = candidate;
            return true;
        }
    }
    return false;
}

ToolboxCreateParseResult parse_toolbox_create_arguments(const std::vector<std::string>& args) {
    ToolboxCreateParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--toolbox-create") != args.end();
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
            if ((index + 1U) >= args.size()) {
                fail("Missing value for " + option + ".");
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--path") {
            result.request.path = require_value(argument);
        } else if (argument == "--toolbox-create") {
            result.request.toolbox_item_id = require_value(argument);
        } else if (argument == "--toolbox-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioToolboxContext parsed_context{};
            if (!parse_toolbox_context_token(token, parsed_context)) {
                fail("Unknown toolbox context token: " + token);
                continue;
            }
            result.request.toolbox_context_provided = true;
            result.request.toolbox_context = parsed_context;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--parent-name") {
            result.request.parent_name = require_value(argument);
        } else if (argument == "--field-value") {
            const std::string assignment = require_value(argument);
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                fail("Toolbox field values must use name=value syntax.");
                continue;
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
        } else {
            fail("Unknown toolbox-create option: " + argument);
        }
    }

    if (result.ok && result.request.path.empty()) {
        fail("No asset path was provided.");
    }
    if (result.ok && result.request.toolbox_item_id.empty()) {
        fail("No toolbox item id was provided.");
    }
    return result;
}

void print_json_toolbox_create_result(const copperfin::vfp::VisualObjectCreateResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": " << (result.ok ? "\"ok\"" : "\"error\"") << ",\n";
    std::cout << "  \"toolboxCreate\": {\n";
    std::cout << "    \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    std::cout << "    \"error\": ";
    print_json_string(result.error);
    std::cout << ",\n";
    std::cout << "    \"recordIndex\": " << result.record_index << ",\n";
    std::cout << "    \"objectName\": ";
    print_json_string(result.object_name);
    std::cout << ",\n";
    std::cout << "    \"uniqueId\": ";
    print_json_string(result.unique_id);
    std::cout << ",\n";
    std::cout << "    \"parentName\": ";
    print_json_string(result.parent_name);
    std::cout << "\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

void print_text_toolbox_create_result(const copperfin::vfp::VisualObjectCreateResult& result) {
    std::cout << "status: " << (result.ok ? "ok" : "error") << "\n";
    if (!result.error.empty()) {
        std::cout << "error: " << result.error << "\n";
    }
    std::cout << "record_index: " << result.record_index << "\n";
    std::cout << "object_name: " << result.object_name << "\n";
    std::cout << "unique_id: " << result.unique_id << "\n";
    std::cout << "parent_name: " << result.parent_name << "\n";
}

void print_json_line_index_or_null(std::size_t line_index) {
    if (line_index == copperfin::studio::StudioObjectMissingLineIndex) {
        std::cout << "null";
    } else {
        std::cout << line_index;
    }
}

void print_json_editor_contexts(const std::vector<copperfin::studio::StudioEditorSelectionContext>& contexts) {
    std::cout << "[";
    for (std::size_t index = 0; index < contexts.size(); ++index) {
        print_json_string(copperfin::studio::studio_editor_selection_context_name(contexts[index]));
        if ((index + 1U) != contexts.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

void print_json_toolbox_contexts(const std::vector<copperfin::studio::StudioToolboxContext>& contexts) {
    std::cout << "[";
    for (std::size_t index = 0; index < contexts.size(); ++index) {
        print_json_string(copperfin::studio::studio_toolbox_context_name(contexts[index]));
        if ((index + 1U) != contexts.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

void print_json_designer_contexts(const std::vector<copperfin::studio::StudioDesignerContextResult>& contexts) {
    std::cout << "[\n";
    for (std::size_t context_index = 0; context_index < contexts.size(); ++context_index) {
        const auto& context = contexts[context_index];
        std::cout << "      {\n";
        std::cout << "        \"selectionContext\": ";
        print_json_string(copperfin::studio::studio_editor_selection_context_name(context.selection_context));
        std::cout << ",\n";
        std::cout << "        \"editorActionCount\": " << context.editor_action_count << ",\n";
        std::cout << "        \"builderCount\": " << context.builder_count << ",\n";
        std::cout << "        \"toolboxItemCount\": " << context.toolbox_item_count << ",\n";
        std::cout << "        \"editorActions\": [\n";
        for (std::size_t action_index = 0; action_index < context.editor_actions.size(); ++action_index) {
            const auto& action = context.editor_actions[action_index];
            std::cout << "          {\n";
            std::cout << "            \"id\": ";
            print_json_string_view(action.id);
            std::cout << ",\n";
            std::cout << "            \"label\": ";
            print_json_string_view(action.label);
            std::cout << ",\n";
            std::cout << "            \"kind\": ";
            print_json_string(copperfin::studio::studio_editor_action_kind_name(action.kind));
            std::cout << ",\n";
            std::cout << "            \"contexts\": ";
            print_json_editor_contexts(action.contexts);
            std::cout << ",\n";
            std::cout << "            \"commandToken\": ";
            print_json_string_view(action.command_token);
            std::cout << ",\n";
            std::cout << "            \"targetSurface\": ";
            print_json_string_view(action.target_surface);
            std::cout << ",\n";
            std::cout << "            \"description\": ";
            print_json_string_view(action.description);
            std::cout << "\n";
            std::cout << "          }";
            if ((action_index + 1U) != context.editor_actions.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "        ],\n";
        std::cout << "        \"builders\": [\n";
        for (std::size_t builder_index = 0; builder_index < context.builders.size(); ++builder_index) {
            const auto& builder = context.builders[builder_index];
            std::cout << "          {\n";
            std::cout << "            \"id\": ";
            print_json_string_view(builder.id);
            std::cout << ",\n";
            std::cout << "            \"title\": ";
            print_json_string_view(builder.title);
            std::cout << ",\n";
            std::cout << "            \"kind\": ";
            print_json_string(copperfin::studio::studio_builder_kind_name(builder.kind));
            std::cout << ",\n";
            std::cout << "            \"context\": ";
            print_json_string(copperfin::studio::studio_builder_context_name(builder.context));
            std::cout << ",\n";
            std::cout << "            \"vfp9Equivalent\": ";
            print_json_string_view(builder.vfp9_equivalent);
            std::cout << ",\n";
            std::cout << "            \"copperfinComponent\": ";
            print_json_string_view(builder.copperfin_component);
            std::cout << ",\n";
            std::cout << "            \"entryPoint\": ";
            print_json_string_view(builder.entry_point);
            std::cout << ",\n";
            std::cout << "            \"description\": ";
            print_json_string_view(builder.description);
            std::cout << "\n";
            std::cout << "          }";
            if ((builder_index + 1U) != context.builders.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "        ],\n";
        std::cout << "        \"toolboxItems\": [\n";
        for (std::size_t toolbox_index = 0; toolbox_index < context.toolbox_items.size(); ++toolbox_index) {
            const auto& toolbox_item = context.toolbox_items[toolbox_index];
            std::cout << "          {\n";
            std::cout << "            \"id\": ";
            print_json_string_view(toolbox_item.id);
            std::cout << ",\n";
            std::cout << "            \"title\": ";
            print_json_string_view(toolbox_item.title);
            std::cout << ",\n";
            std::cout << "            \"category\": ";
            print_json_string_view(toolbox_item.category);
            std::cout << ",\n";
            std::cout << "            \"vfpClass\": ";
            print_json_string_view(toolbox_item.vfp_class);
            std::cout << ",\n";
            std::cout << "            \"baseClass\": ";
            print_json_string_view(toolbox_item.base_class);
            std::cout << ",\n";
            std::cout << "            \"defaultNamePrefix\": ";
            print_json_string_view(toolbox_item.default_name_prefix);
            std::cout << ",\n";
            std::cout << "            \"contexts\": ";
            print_json_toolbox_contexts(toolbox_item.contexts);
            std::cout << ",\n";
            std::cout << "            \"container\": " << (toolbox_item.container ? "true" : "false") << ",\n";
            std::cout << "            \"description\": ";
            print_json_string_view(toolbox_item.description);
            std::cout << "\n";
            std::cout << "          }";
            if ((toolbox_index + 1U) != context.toolbox_items.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "        ]\n";
        std::cout << "      }";
        if ((context_index + 1U) != contexts.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]";
}

void print_json_object_properties(
    const std::vector<copperfin::studio::StudioPropertySnapshot>& properties,
    const std::string& indent) {
    std::cout << "[\n";
    for (std::size_t property_index = 0; property_index < properties.size(); ++property_index) {
        const auto& property = properties[property_index];
        std::cout << indent << "  {\"name\": ";
        print_json_string(property.name);
        std::cout << ", \"type\": ";
        print_json_string(std::string(1U, property.type));
        std::cout << ", \"isNull\": " << (property.is_null ? "true" : "false") << ", \"value\": ";
        print_json_string(property.value);
        std::cout << ", \"fieldIndex\": " << property.field_index;
        std::cout << ", \"memoBlockNumber\": " << property.memo_block_number;
        std::cout << ", \"derivedFromPropertyBlob\": " << (property.derived_from_property_blob ? "true" : "false");
        std::cout << ", \"sourceLineIndex\": ";
        print_json_line_index_or_null(property.source_line_index);
        std::cout << "}";
        if ((property_index + 1U) != properties.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

void print_json_record_index_or_null(std::size_t record_index) {
    if (record_index == copperfin::studio::StudioObjectMissingRecordIndex) {
        std::cout << "null";
    } else {
        std::cout << record_index;
    }
}

void print_json_record_index_array(const std::vector<std::size_t>& record_indexes) {
    std::cout << "[";
    for (std::size_t index = 0; index < record_indexes.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        std::cout << record_indexes[index];
    }
    std::cout << "]";
}

void print_json_object_summary(const copperfin::studio::StudioObjectSnapshot& object, const std::string& indent) {
    std::cout << "{\n";
    std::cout << indent << "  \"recordIndex\": " << object.record_index << ",\n";
    std::cout << indent << "  \"deleted\": " << (object.deleted ? "true" : "false") << ",\n";
    std::cout << indent << "  \"title\": ";
    print_json_string(object.title);
    std::cout << ",\n";
    std::cout << indent << "  \"subtitle\": ";
    print_json_string(object.subtitle);
    std::cout << ",\n";
    std::cout << indent << "  \"objectTypeCode\": " << object.objtype_code << ",\n";
    std::cout << indent << "  \"objectCode\": " << object.objcode_code << ",\n";
    std::cout << indent << "  \"platform\": ";
    print_json_string(object.platform);
    std::cout << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(object.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"objectPath\": ";
    print_json_string(object.object_path);
    std::cout << ",\n";
    std::cout << indent << "  \"objectDepth\": " << object.object_depth << ",\n";
    std::cout << indent << "  \"siblingIndex\": " << object.sibling_index << ",\n";
    std::cout << indent << "  \"siblingCount\": " << object.sibling_count << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(object.unique_id);
    std::cout << ",\n";
    std::cout << indent << "  \"parentName\": ";
    print_json_string(object.parent_name);
    std::cout << ",\n";
    std::cout << indent << "  \"parentRecordIndex\": ";
    print_json_record_index_or_null(object.parent_record_index);
    std::cout << ",\n";
    std::cout << indent << "  \"ancestorRecordIndexes\": ";
    print_json_record_index_array(object.ancestor_record_indexes);
    std::cout << ",\n";
    std::cout << indent << "  \"className\": ";
    print_json_string(object.class_name);
    std::cout << ",\n";
    std::cout << indent << "  \"baseclassName\": ";
    print_json_string(object.baseclass_name);
    std::cout << ",\n";
    std::cout << indent << "  \"childCount\": " << object.child_count << ",\n";
    std::cout << indent << "  \"childRecordIndexes\": ";
    print_json_record_index_array(object.child_record_indexes);
    std::cout << ",\n";
    std::cout << indent << "  \"propertyCount\": " << object.properties.size() << ",\n";
    std::cout << indent << "  \"properties\": ";
    print_json_object_properties(object.properties, indent + "  ");
    std::cout << "\n";
    std::cout << indent << "}";
}

const copperfin::studio::StudioObjectSnapshot* find_selected_object(
    const std::vector<copperfin::studio::StudioObjectSnapshot>& objects,
    std::size_t record_index) {
    const auto selected = std::find_if(objects.begin(), objects.end(), [&](const auto& object) {
        return object.record_index == record_index;
    });
    return selected == objects.end() ? nullptr : &*selected;
}

void print_json_document(const copperfin::studio::StudioDocumentModel& document) {
    const auto objects = copperfin::studio::build_object_snapshot(document);
    const auto deleted_object_count = static_cast<std::size_t>(std::count_if(
        objects.begin(),
        objects.end(),
        [](const copperfin::studio::StudioObjectSnapshot& object) {
            return object.deleted;
        }));
    const auto root_object_count = static_cast<std::size_t>(std::count_if(
        objects.begin(),
        objects.end(),
        [](const copperfin::studio::StudioObjectSnapshot& object) {
            return object.parent_record_index == copperfin::studio::StudioObjectMissingRecordIndex;
        }));
    std::vector<std::size_t> root_record_indexes;
    root_record_indexes.reserve(root_object_count);
    std::vector<std::size_t> leaf_record_indexes;
    leaf_record_indexes.reserve(objects.size());
    for (const auto& object : objects) {
        if (object.parent_record_index == copperfin::studio::StudioObjectMissingRecordIndex) {
            root_record_indexes.push_back(object.record_index);
        }
        if (object.child_record_indexes.empty()) {
            leaf_record_indexes.push_back(object.record_index);
        }
    }
    const auto max_object_depth = objects.empty()
        ? 0U
        : std::max_element(
              objects.begin(),
              objects.end(),
              [](const copperfin::studio::StudioObjectSnapshot& left,
                 const copperfin::studio::StudioObjectSnapshot& right) {
                  return left.object_depth < right.object_depth;
              })
              ->object_depth;
    const auto report_layout = copperfin::studio::build_report_layout(document);
    const auto project_workspace = copperfin::studio::build_project_workspace(document);
    const auto security_profile = copperfin::security::default_native_security_profile();
    const auto database_profile = copperfin::platform::default_database_federation_profile();
    const auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    const auto command_undo_status = copperfin::vfp::query_visual_object_undo(document.path);
    const auto* selected_object = document.selection_record_available
        ? find_selected_object(objects, document.selection_record_index)
        : nullptr;

    std::cout << "{\n";
    std::cout << "  \"status\": \"ok\",\n";
    std::cout << "  \"document\": {\n";
    std::cout << "    \"path\": ";
    print_json_string(document.path);
    std::cout << ",\n";
    std::cout << "    \"displayName\": ";
    print_json_string(document.display_name);
    std::cout << ",\n";
    std::cout << "    \"kind\": ";
    print_json_string(copperfin::studio::studio_asset_kind_name(document.kind));
    std::cout << ",\n";
    std::cout << "    \"readOnly\": " << (document.read_only ? "true" : "false") << ",\n";
    std::cout << "    \"launchedFromVisualStudio\": "
              << (document.launched_from_visual_studio ? "true" : "false") << ",\n";
    std::cout << "    \"launchSelection\": {\n";
    std::cout << "      \"symbol\": ";
    print_json_string(document.selection_symbol);
    std::cout << ",\n";
    std::cout << "      \"line\": " << document.selection_line << ",\n";
    std::cout << "      \"column\": " << document.selection_column << ",\n";
    std::cout << "      \"recordAvailable\": " << (document.selection_record_available ? "true" : "false") << ",\n";
    std::cout << "      \"recordIndex\": " << document.selection_record_index << "\n";
    std::cout << "    },\n";
    std::cout << "    \"selectedObjectAvailable\": " << (selected_object != nullptr ? "true" : "false") << ",\n";
    std::cout << "    \"selectedObject\": ";
    if (selected_object != nullptr) {
        print_json_object_summary(*selected_object, "    ");
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "    \"hasSidecar\": " << (document.has_sidecar ? "true" : "false") << ",\n";
    std::cout << "    \"sidecarPath\": ";
    print_json_string(document.sidecar_path);
    std::cout << ",\n";
    std::cout << "    \"assetFamily\": ";
    print_json_string(copperfin::vfp::asset_family_name(document.inspection.family));
    std::cout << ",\n";
    std::cout << "    \"indexCount\": " << document.inspection.indexes.size() << ",\n";
    std::cout << "    \"headerVersionDescription\": ";
    if (document.inspection.header_available) {
        print_json_string(document.inspection.header.version_description());
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "    \"fieldCount\": " << document.table_preview.fields.size() << ",\n";
    std::cout << "    \"recordCount\": " << document.table_preview.records.size() << ",\n";
    std::cout << "    \"objectCount\": " << objects.size() << ",\n";
    std::cout << "    \"deletedObjectCount\": " << deleted_object_count << ",\n";
    std::cout << "    \"rootObjectCount\": " << root_object_count << ",\n";
    std::cout << "    \"rootRecordIndexes\": ";
    print_json_record_index_array(root_record_indexes);
    std::cout << ",\n";
    std::cout << "    \"leafObjectCount\": " << leaf_record_indexes.size() << ",\n";
    std::cout << "    \"leafRecordIndexes\": ";
    print_json_record_index_array(leaf_record_indexes);
    std::cout << ",\n";
    std::cout << "    \"maxObjectDepth\": " << max_object_depth << ",\n";
    std::cout << "    \"commandUndoAvailable\": " << (command_undo_status.available ? "true" : "false") << ",\n";
    std::cout << "    \"commandUndoLabel\": ";
    print_json_string(command_undo_status.label);
    std::cout << ",\n";
    std::cout << "    \"designerContexts\": ";
    print_json_designer_contexts(document.designer_contexts);
    std::cout << ",\n";
    std::cout << "    \"fields\": [\n";
    for (std::size_t index = 0; index < document.table_preview.fields.size(); ++index) {
        const auto& field = document.table_preview.fields[index];
        std::cout << "      {\"name\": ";
        print_json_string(field.name);
        std::cout << ", \"type\": ";
        print_json_string(std::string(1U, field.type));
        std::cout << ", \"length\": " << static_cast<unsigned int>(field.length);
        std::cout << ", \"decimalCount\": " << static_cast<unsigned int>(field.decimal_count) << "}";
        if ((index + 1U) != document.table_preview.fields.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ],\n";
    std::cout << "    \"reportLayout\": ";
    if (!report_layout.available) {
        std::cout << "null,\n";
    } else {
        std::cout << "{\n";
        std::cout << "      \"isLabel\": " << (report_layout.is_label ? "true" : "false") << ",\n";
        std::cout << "      \"documentTitle\": ";
        print_json_string(report_layout.document_title);
        std::cout << ",\n";
        std::cout << "      \"settings\": [\n";
        for (std::size_t index = 0; index < report_layout.settings.size(); ++index) {
            const auto& setting = report_layout.settings[index];
            std::cout << "        {\"name\": ";
            print_json_string(setting.name);
            std::cout << ", \"value\": ";
            print_json_string(setting.value);
            std::cout << "}";
            if ((index + 1U) != report_layout.settings.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"sections\": [\n";
        for (std::size_t section_index = 0; section_index < report_layout.sections.size(); ++section_index) {
            const auto& section = report_layout.sections[section_index];
            std::cout << "        {\n";
            std::cout << "          \"id\": ";
            print_json_string(section.id);
            std::cout << ",\n";
            std::cout << "          \"title\": ";
            print_json_string(section.title);
            std::cout << ",\n";
            std::cout << "          \"bandKind\": ";
            print_json_string(section.band_kind);
            std::cout << ",\n";
            std::cout << "          \"recordIndex\": " << section.record_index << ",\n";
            std::cout << "          \"top\": " << section.top << ",\n";
            std::cout << "          \"height\": " << section.height << ",\n";
            std::cout << "          \"objects\": [\n";
            for (std::size_t object_index = 0; object_index < section.objects.size(); ++object_index) {
                const auto& object = section.objects[object_index];
                std::cout << "            {\n";
                std::cout << "              \"recordIndex\": " << object.record_index << ",\n";
                std::cout << "              \"objectKind\": ";
                print_json_string(object.object_kind);
                std::cout << ",\n";
                std::cout << "              \"title\": ";
                print_json_string(object.title);
                std::cout << ",\n";
                std::cout << "              \"expression\": ";
                print_json_string(object.expression);
                std::cout << ",\n";
                std::cout << "              \"left\": " << object.left << ",\n";
                std::cout << "              \"top\": " << object.top << ",\n";
                std::cout << "              \"width\": " << object.width << ",\n";
                std::cout << "              \"height\": " << object.height << ",\n";
                std::cout << "              \"highlights\": [\n";
                for (std::size_t highlight_index = 0; highlight_index < object.highlights.size(); ++highlight_index) {
                    const auto& highlight = object.highlights[highlight_index];
                    std::cout << "                {\"name\": ";
                    print_json_string(highlight.name);
                    std::cout << ", \"value\": ";
                    print_json_string(highlight.value);
                    std::cout << "}";
                    if ((highlight_index + 1U) != object.highlights.size()) {
                        std::cout << ",";
                    }
                    std::cout << "\n";
                }
                std::cout << "              ]\n";
                std::cout << "            }";
                if ((object_index + 1U) != section.objects.size()) {
                    std::cout << ",";
                }
                std::cout << "\n";
            }
            std::cout << "          ]\n";
            std::cout << "        }";
            if ((section_index + 1U) != report_layout.sections.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"unplacedObjects\": [\n";
        for (std::size_t object_index = 0; object_index < report_layout.unplaced_objects.size(); ++object_index) {
            const auto& object = report_layout.unplaced_objects[object_index];
            std::cout << "        {\n";
            std::cout << "          \"recordIndex\": " << object.record_index << ",\n";
            std::cout << "          \"objectKind\": ";
            print_json_string(object.object_kind);
            std::cout << ",\n";
            std::cout << "          \"title\": ";
            print_json_string(object.title);
            std::cout << ",\n";
            std::cout << "          \"expression\": ";
            print_json_string(object.expression);
            std::cout << ",\n";
            std::cout << "          \"left\": " << object.left << ",\n";
            std::cout << "          \"top\": " << object.top << ",\n";
            std::cout << "          \"width\": " << object.width << ",\n";
            std::cout << "          \"height\": " << object.height << "\n";
            std::cout << "        }";
            if ((object_index + 1U) != report_layout.unplaced_objects.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ]\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"projectWorkspace\": ";
    if (!project_workspace.available) {
        std::cout << "null,\n";
    } else {
        std::cout << "{\n";
        std::cout << "      \"projectTitle\": ";
        print_json_string(project_workspace.project_title);
        std::cout << ",\n";
        std::cout << "      \"projectKey\": ";
        print_json_string(project_workspace.project_key);
        std::cout << ",\n";
        std::cout << "      \"homeDirectory\": ";
        print_json_string(project_workspace.home_directory);
        std::cout << ",\n";
        std::cout << "      \"outputPath\": ";
        print_json_string(project_workspace.output_path);
        std::cout << ",\n";
        std::cout << "      \"groups\": [\n";
        for (std::size_t group_index = 0; group_index < project_workspace.groups.size(); ++group_index) {
            const auto& group = project_workspace.groups[group_index];
            std::cout << "        {\n";
            std::cout << "          \"id\": ";
            print_json_string(group.id);
            std::cout << ",\n";
            std::cout << "          \"title\": ";
            print_json_string(group.title);
            std::cout << ",\n";
            std::cout << "          \"itemCount\": " << group.item_count << ",\n";
            std::cout << "          \"excludedCount\": " << group.excluded_count << ",\n";
            std::cout << "          \"recordIndexes\": [";
            for (std::size_t record_index = 0; record_index < group.record_indexes.size(); ++record_index) {
                std::cout << group.record_indexes[record_index];
                if ((record_index + 1U) != group.record_indexes.size()) {
                    std::cout << ", ";
                }
            }
            std::cout << "]\n";
            std::cout << "        }";
            if ((group_index + 1U) != project_workspace.groups.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"entries\": [\n";
        for (std::size_t entry_index = 0; entry_index < project_workspace.entries.size(); ++entry_index) {
            const auto& entry = project_workspace.entries[entry_index];
            std::cout << "        {\n";
            std::cout << "          \"recordIndex\": " << entry.record_index << ",\n";
            std::cout << "          \"name\": ";
            print_json_string(entry.name);
            std::cout << ",\n";
            std::cout << "          \"relativePath\": ";
            print_json_string(entry.relative_path);
            std::cout << ",\n";
            std::cout << "          \"typeCode\": ";
            print_json_string(entry.type_code);
            std::cout << ",\n";
            std::cout << "          \"typeTitle\": ";
            print_json_string(entry.type_title);
            std::cout << ",\n";
            std::cout << "          \"groupId\": ";
            print_json_string(entry.group_id);
            std::cout << ",\n";
            std::cout << "          \"groupTitle\": ";
            print_json_string(entry.group_title);
            std::cout << ",\n";
            std::cout << "          \"key\": ";
            print_json_string(entry.key);
            std::cout << ",\n";
            std::cout << "          \"comments\": ";
            print_json_string(entry.comments);
            std::cout << ",\n";
            std::cout << "          \"excluded\": " << (entry.excluded ? "true" : "false") << ",\n";
            std::cout << "          \"mainProgram\": " << (entry.main_program ? "true" : "false") << ",\n";
            std::cout << "          \"local\": " << (entry.local ? "true" : "false") << "\n";
            std::cout << "        }";
            if ((entry_index + 1U) != project_workspace.entries.size()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "      ],\n";
        std::cout << "      \"buildPlan\": {\n";
        std::cout << "        \"available\": " << (project_workspace.build_plan.available ? "true" : "false") << ",\n";
        std::cout << "        \"canBuild\": " << (project_workspace.build_plan.can_build ? "true" : "false") << ",\n";
        std::cout << "        \"projectTitle\": ";
        print_json_string(project_workspace.build_plan.project_title);
        std::cout << ",\n";
        std::cout << "        \"projectKey\": ";
        print_json_string(project_workspace.build_plan.project_key);
        std::cout << ",\n";
        std::cout << "        \"homeDirectory\": ";
        print_json_string(project_workspace.build_plan.home_directory);
        std::cout << ",\n";
        std::cout << "        \"outputPath\": ";
        print_json_string(project_workspace.build_plan.output_path);
        std::cout << ",\n";
        std::cout << "        \"buildTarget\": ";
        print_json_string(project_workspace.build_plan.build_target);
        std::cout << ",\n";
        std::cout << "        \"startupItem\": ";
        print_json_string(project_workspace.build_plan.startup_item);
        std::cout << ",\n";
        std::cout << "        \"startupRecordIndex\": " << project_workspace.build_plan.startup_record_index << ",\n";
        std::cout << "        \"totalItems\": " << project_workspace.build_plan.total_items << ",\n";
        std::cout << "        \"excludedItems\": " << project_workspace.build_plan.excluded_items << ",\n";
        std::cout << "        \"debugEnabled\": " << (project_workspace.build_plan.debug_enabled ? "true" : "false") << ",\n";
        std::cout << "        \"encryptEnabled\": " << (project_workspace.build_plan.encrypt_enabled ? "true" : "false") << ",\n";
        std::cout << "        \"saveCode\": " << (project_workspace.build_plan.save_code ? "true" : "false") << ",\n";
        std::cout << "        \"noLogo\": " << (project_workspace.build_plan.no_logo ? "true" : "false") << "\n";
        std::cout << "      }\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"securityProfile\": {\n";
    std::cout << "      \"available\": " << (security_profile.available ? "true" : "false") << ",\n";
    std::cout << "      \"optional\": " << (security_profile.optional ? "true" : "false") << ",\n";
    std::cout << "      \"mode\": ";
    print_json_string(security_profile.mode);
    std::cout << ",\n";
    std::cout << "      \"packagePolicy\": ";
    print_json_string(security_profile.package_policy);
    std::cout << ",\n";
    std::cout << "      \"managedInteropPolicy\": ";
    print_json_string(security_profile.managed_interop_policy);
    std::cout << ",\n";
    std::cout << "      \"roles\": [\n";
    for (std::size_t role_index = 0; role_index < security_profile.roles.size(); ++role_index) {
        const auto& role = security_profile.roles[role_index];
        std::cout << "        {\n";
        std::cout << "          \"id\": ";
        print_json_string(role.id);
        std::cout << ",\n";
        std::cout << "          \"title\": ";
        print_json_string(role.title);
        std::cout << ",\n";
        std::cout << "          \"description\": ";
        print_json_string(role.description);
        std::cout << ",\n";
        std::cout << "          \"defaultAssignment\": " << (role.default_assignment ? "true" : "false") << ",\n";
        std::cout << "          \"permissionIds\": [";
        for (std::size_t permission_index = 0; permission_index < role.permission_ids.size(); ++permission_index) {
            print_json_string(role.permission_ids[permission_index]);
            if ((permission_index + 1U) != role.permission_ids.size()) {
                std::cout << ", ";
            }
        }
        std::cout << "]\n";
        std::cout << "        }";
        if ((role_index + 1U) != security_profile.roles.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ],\n";
    std::cout << "      \"identityProviders\": [\n";
    for (std::size_t provider_index = 0; provider_index < security_profile.identity_providers.size(); ++provider_index) {
        const auto& provider = security_profile.identity_providers[provider_index];
        std::cout << "        {\n";
        std::cout << "          \"id\": ";
        print_json_string(provider.id);
        std::cout << ",\n";
        std::cout << "          \"title\": ";
        print_json_string(provider.title);
        std::cout << ",\n";
        std::cout << "          \"kind\": ";
        print_json_string(provider.kind);
        std::cout << ",\n";
        std::cout << "          \"description\": ";
        print_json_string(provider.description);
        std::cout << ",\n";
        std::cout << "          \"enabledByDefault\": " << (provider.enabled_by_default ? "true" : "false") << "\n";
        std::cout << "        }";
        if ((provider_index + 1U) != security_profile.identity_providers.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ],\n";
    std::cout << "      \"features\": [\n";
    for (std::size_t feature_index = 0; feature_index < security_profile.features.size(); ++feature_index) {
        const auto& feature = security_profile.features[feature_index];
        std::cout << "        {\n";
        std::cout << "          \"id\": ";
        print_json_string(feature.id);
        std::cout << ",\n";
        std::cout << "          \"title\": ";
        print_json_string(feature.title);
        std::cout << ",\n";
        std::cout << "          \"description\": ";
        print_json_string(feature.description);
        std::cout << ",\n";
        std::cout << "          \"enabledByDefault\": " << (feature.enabled_by_default ? "true" : "false") << ",\n";
        std::cout << "          \"optional\": " << (feature.optional ? "true" : "false") << "\n";
        std::cout << "        }";
        if ((feature_index + 1U) != security_profile.features.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ],\n";
    std::cout << "      \"auditEvents\": [";
    for (std::size_t audit_index = 0; audit_index < security_profile.audit_events.size(); ++audit_index) {
        print_json_string(security_profile.audit_events[audit_index]);
        if ((audit_index + 1U) != security_profile.audit_events.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "],\n";
    std::cout << "      \"hardeningProfiles\": [";
    for (std::size_t hardening_index = 0; hardening_index < security_profile.hardening_profiles.size(); ++hardening_index) {
        print_json_string(security_profile.hardening_profiles[hardening_index]);
        if ((hardening_index + 1U) != security_profile.hardening_profiles.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "    },\n";
    std::cout << "    \"extensibilityProfile\": {\n";
    std::cout << "      \"available\": " << (extensibility_profile.available ? "true" : "false") << ",\n";
    std::cout << "      \"languages\": [\n";
    for (std::size_t language_index = 0; language_index < extensibility_profile.languages.size(); ++language_index) {
        const auto& language = extensibility_profile.languages[language_index];
        std::cout << "        {\n";
        std::cout << "          \"id\": ";
        print_json_string(language.id);
        std::cout << ",\n";
        std::cout << "          \"title\": ";
        print_json_string(language.title);
        std::cout << ",\n";
        std::cout << "          \"integrationMode\": ";
        print_json_string(language.integration_mode);
        std::cout << ",\n";
        std::cout << "          \"trustBoundary\": ";
        print_json_string(language.trust_boundary);
        std::cout << ",\n";
        std::cout << "          \"outputStory\": ";
        print_json_string(language.output_story);
        std::cout << ",\n";
        std::cout << "          \"enabledByDefault\": " << (language.enabled_by_default ? "true" : "false") << "\n";
        std::cout << "        }";
        if ((language_index + 1U) != extensibility_profile.languages.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ],\n";
    std::cout << "      \"aiFeatures\": [\n";
    for (std::size_t feature_index = 0; feature_index < extensibility_profile.ai_features.size(); ++feature_index) {
        const auto& feature = extensibility_profile.ai_features[feature_index];
        std::cout << "        {\n";
        std::cout << "          \"id\": ";
        print_json_string(feature.id);
        std::cout << ",\n";
        std::cout << "          \"title\": ";
        print_json_string(feature.title);
        std::cout << ",\n";
        std::cout << "          \"description\": ";
        print_json_string(feature.description);
        std::cout << ",\n";
        std::cout << "          \"trustBoundary\": ";
        print_json_string(feature.trust_boundary);
        std::cout << ",\n";
        std::cout << "          \"enabledByDefault\": " << (feature.enabled_by_default ? "true" : "false") << "\n";
        std::cout << "        }";
        if ((feature_index + 1U) != extensibility_profile.ai_features.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ],\n";
    std::cout << "      \"dotNetOutput\": {\n";
    std::cout << "        \"available\": " << (extensibility_profile.dotnet_output.available ? "true" : "false") << ",\n";
    std::cout << "        \"nativeHostExecutables\": " << (extensibility_profile.dotnet_output.native_host_executables ? "true" : "false") << ",\n";
    std::cout << "        \"managedWrappers\": " << (extensibility_profile.dotnet_output.managed_wrappers ? "true" : "false") << ",\n";
    std::cout << "        \"nugetSdk\": " << (extensibility_profile.dotnet_output.nuget_sdk ? "true" : "false") << ",\n";
    std::cout << "        \"primaryStory\": ";
    print_json_string(extensibility_profile.dotnet_output.primary_story);
    std::cout << "\n";
    std::cout << "      },\n";
    std::cout << "      \"guardrails\": [";
    for (std::size_t guardrail_index = 0; guardrail_index < extensibility_profile.guardrails.size(); ++guardrail_index) {
        print_json_string(extensibility_profile.guardrails[guardrail_index]);
        if ((guardrail_index + 1U) != extensibility_profile.guardrails.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "    },\n";
    std::cout << "    \"databaseProfile\": {\n";
    std::cout << "      \"available\": " << (database_profile.available ? "true" : "false") << ",\n";
    std::cout << "      \"connectors\": [\n";
    for (std::size_t connector_index = 0; connector_index < database_profile.connectors.size(); ++connector_index) {
        const auto& connector = database_profile.connectors[connector_index];
        std::cout << "        {\n";
        std::cout << "          \"id\": ";
        print_json_string(connector.id);
        std::cout << ",\n";
        std::cout << "          \"title\": ";
        print_json_string(connector.title);
        std::cout << ",\n";
        std::cout << "          \"family\": ";
        print_json_string(connector.family);
        std::cout << ",\n";
        std::cout << "          \"accessMode\": ";
        print_json_string(connector.access_mode);
        std::cout << ",\n";
        std::cout << "          \"schemaShape\": ";
        print_json_string(connector.schema_shape);
        std::cout << ",\n";
        std::cout << "          \"translationStory\": ";
        print_json_string(connector.translation_story);
        std::cout << ",\n";
        std::cout << "          \"xbaseCommandsFirstClass\": " << (connector.xbase_commands_first_class ? "true" : "false") << ",\n";
        std::cout << "          \"foxSqlTranslationDirect\": " << (connector.fox_sql_translation_direct ? "true" : "false") << ",\n";
        std::cout << "          \"aiQueryPlanningOptional\": " << (connector.ai_query_planning_optional ? "true" : "false") << "\n";
        std::cout << "        }";
        if ((connector_index + 1U) != database_profile.connectors.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ],\n";
    std::cout << "      \"queryPaths\": [\n";
    for (std::size_t path_index = 0; path_index < database_profile.query_paths.size(); ++path_index) {
        const auto& path = database_profile.query_paths[path_index];
        std::cout << "        {\n";
        std::cout << "          \"id\": ";
        print_json_string(path.id);
        std::cout << ",\n";
        std::cout << "          \"title\": ";
        print_json_string(path.title);
        std::cout << ",\n";
        std::cout << "          \"sourceShape\": ";
        print_json_string(path.source_shape);
        std::cout << ",\n";
        std::cout << "          \"targetShape\": ";
        print_json_string(path.target_shape);
        std::cout << ",\n";
        std::cout << "          \"complexity\": ";
        print_json_string(path.complexity);
        std::cout << ",\n";
        std::cout << "          \"strategy\": ";
        print_json_string(path.strategy);
        std::cout << ",\n";
        std::cout << "          \"deterministicFirst\": " << (path.deterministic_first ? "true" : "false") << ",\n";
        std::cout << "          \"aiOptional\": " << (path.ai_optional ? "true" : "false") << "\n";
        std::cout << "        }";
        if ((path_index + 1U) != database_profile.query_paths.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "      ],\n";
    std::cout << "      \"guardrails\": [";
    for (std::size_t guardrail_index = 0; guardrail_index < database_profile.guardrails.size(); ++guardrail_index) {
        print_json_string(database_profile.guardrails[guardrail_index]);
        if ((guardrail_index + 1U) != database_profile.guardrails.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]\n";
    std::cout << "    },\n";
    std::cout << "    \"objects\": [\n";
    for (std::size_t index = 0; index < objects.size(); ++index) {
        const auto& object = objects[index];
        std::cout << "      {\n";
        std::cout << "        \"recordIndex\": " << object.record_index << ",\n";
        std::cout << "        \"deleted\": " << (object.deleted ? "true" : "false") << ",\n";
        std::cout << "        \"title\": ";
        print_json_string(object.title);
        std::cout << ",\n";
        std::cout << "        \"subtitle\": ";
        print_json_string(object.subtitle);
        std::cout << ",\n";
        std::cout << "        \"objectTypeCode\": " << object.objtype_code << ",\n";
        std::cout << "        \"objectCode\": " << object.objcode_code << ",\n";
        std::cout << "        \"platform\": ";
        print_json_string(object.platform);
        std::cout << ",\n";
        std::cout << "        \"objectName\": ";
        print_json_string(object.object_name);
        std::cout << ",\n";
        std::cout << "        \"objectPath\": ";
        print_json_string(object.object_path);
        std::cout << ",\n";
        std::cout << "        \"objectDepth\": " << object.object_depth << ",\n";
        std::cout << "        \"siblingIndex\": " << object.sibling_index << ",\n";
        std::cout << "        \"siblingCount\": " << object.sibling_count << ",\n";
        std::cout << "        \"uniqueId\": ";
        print_json_string(object.unique_id);
        std::cout << ",\n";
        std::cout << "        \"parentName\": ";
        print_json_string(object.parent_name);
        std::cout << ",\n";
        std::cout << "        \"parentRecordIndex\": ";
        print_json_record_index_or_null(object.parent_record_index);
        std::cout << ",\n";
        std::cout << "        \"ancestorRecordIndexes\": ";
        print_json_record_index_array(object.ancestor_record_indexes);
        std::cout << ",\n";
        std::cout << "        \"className\": ";
        print_json_string(object.class_name);
        std::cout << ",\n";
        std::cout << "        \"baseclassName\": ";
        print_json_string(object.baseclass_name);
        std::cout << ",\n";
        std::cout << "        \"childCount\": " << object.child_count << ",\n";
        std::cout << "        \"childRecordIndexes\": ";
        print_json_record_index_array(object.child_record_indexes);
        std::cout << ",\n";
        std::cout << "        \"propertyCount\": " << object.properties.size() << ",\n";
        std::cout << "        \"properties\": ";
        print_json_object_properties(object.properties, "        ");
        std::cout << "\n";
        std::cout << "      }";
        if ((index + 1U) != objects.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "    ]\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

void print_document(const copperfin::studio::StudioDocumentModel& document) {
    const auto report_layout = copperfin::studio::build_report_layout(document);
    const auto project_workspace = copperfin::studio::build_project_workspace(document);
    const auto security_profile = copperfin::security::default_native_security_profile();
    const auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    std::cout << "status: ok\n";
    std::cout << "document.path: " << document.path << "\n";
    std::cout << "document.display_name: " << document.display_name << "\n";
    std::cout << "document.kind: " << copperfin::studio::studio_asset_kind_name(document.kind) << "\n";
    std::cout << "document.read_only: " << (document.read_only ? "true" : "false") << "\n";
    std::cout << "document.launched_from_visual_studio: "
              << (document.launched_from_visual_studio ? "true" : "false") << "\n";
    std::cout << "document.selection_symbol: " << document.selection_symbol << "\n";
    std::cout << "document.selection_line: " << document.selection_line << "\n";
    std::cout << "document.selection_column: " << document.selection_column << "\n";
    std::cout << "document.selection_record_available: "
              << (document.selection_record_available ? "true" : "false") << "\n";
    std::cout << "document.selection_record_index: " << document.selection_record_index << "\n";
    std::cout << "document.has_sidecar: " << (document.has_sidecar ? "true" : "false") << "\n";
    if (!document.sidecar_path.empty()) {
        std::cout << "document.sidecar_path: " << document.sidecar_path << "\n";
    }
    std::cout << "inspection.asset_family: "
              << copperfin::vfp::asset_family_name(document.inspection.family) << "\n";
    std::cout << "inspection.index_count: " << document.inspection.indexes.size() << "\n";
    if (document.inspection.header_available) {
        std::cout << "inspection.header.version_description: "
                  << document.inspection.header.version_description() << "\n";
    }

    if (!document.table_preview_available) {
        return;
    }

    std::cout << "preview.field_count: " << document.table_preview.fields.size() << "\n";
    std::cout << "preview.record_count: " << document.table_preview.records.size() << "\n";
    if (report_layout.available) {
        std::cout << "preview.report_layout.section_count: " << report_layout.sections.size() << "\n";
        for (const auto& section : report_layout.sections) {
            std::cout << "section[" << section.record_index << "]: " << section.title
                      << " objects=" << section.objects.size()
                      << " top=" << section.top
                      << " height=" << section.height << "\n";
        }
    }

    if (project_workspace.available) {
        std::cout << "preview.project_workspace.group_count: " << project_workspace.groups.size() << "\n";
        std::cout << "preview.project_workspace.entry_count: " << project_workspace.entries.size() << "\n";
        std::cout << "preview.project_workspace.output_path: " << project_workspace.output_path << "\n";
        std::cout << "preview.project_workspace.startup_item: " << project_workspace.build_plan.startup_item << "\n";
        for (const auto& group : project_workspace.groups) {
            std::cout << "group[" << group.id << "]: " << group.title
                      << " items=" << group.item_count
                      << " excluded=" << group.excluded_count << "\n";
        }
    }

    std::cout << "preview.security.mode: " << security_profile.mode << "\n";
    std::cout << "preview.security.role_count: " << security_profile.roles.size() << "\n";
    std::cout << "preview.extensibility.language_count: " << extensibility_profile.languages.size() << "\n";
    std::cout << "preview.extensibility.dotnet_story: " << extensibility_profile.dotnet_output.primary_story << "\n";

    if (!document.table_preview.fields.empty()) {
        std::cout << "preview.fields:";
        for (const auto& field : document.table_preview.fields) {
            std::cout << " " << field.name << "(" << field.type << "," << static_cast<unsigned int>(field.length) << ")";
        }
        std::cout << "\n";
    }

    for (const auto& record : document.table_preview.records) {
        std::cout << "record[" << record.record_index << "]";
        if (record.deleted) {
            std::cout << " deleted";
        }
        std::cout << "\n";

        for (const auto& value : record.values) {
            if (value.display_value.empty()) {
                continue;
            }
            std::cout << "  " << value.field_name << ": " << value.display_value << "\n";
        }
    }
}

void print_json_subsystems() {
    const auto& subsystems = copperfin::studio::product_subsystems();
    std::cout << "{\n";
    std::cout << "  \"status\": \"ok\",\n";
    std::cout << "  \"subsystems\": [\n";
    for (std::size_t index = 0; index < subsystems.size(); ++index) {
        const auto& subsystem = subsystems[index];
        std::cout << "    {\n";
        std::cout << "      \"id\": ";
        print_json_string(std::string(subsystem.id));
        std::cout << ",\n";
        std::cout << "      \"title\": ";
        print_json_string(std::string(subsystem.title));
        std::cout << ",\n";
        std::cout << "      \"vfp9Equivalent\": ";
        print_json_string(std::string(subsystem.vfp9_equivalent));
        std::cout << ",\n";
        std::cout << "      \"copperfinComponent\": ";
        print_json_string(std::string(subsystem.copperfin_component));
        std::cout << ",\n";
        std::cout << "      \"hostKind\": ";
        print_json_string(copperfin::studio::product_host_kind_name(subsystem.host_kind));
        std::cout << ",\n";
        std::cout << "      \"currentStatus\": ";
        print_json_string(std::string(subsystem.current_status));
        std::cout << ",\n";
        std::cout << "      \"parityScope\": ";
        print_json_string(std::string(subsystem.parity_scope));
        std::cout << ",\n";
        std::cout << "      \"modernEditorDirection\": ";
        print_json_string(std::string(subsystem.modern_editor_direction));
        std::cout << "\n";
        std::cout << "    }";
        if ((index + 1U) != subsystems.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    std::cout << "  ]\n";
    std::cout << "}\n";
}

void print_subsystems() {
    const auto& subsystems = copperfin::studio::product_subsystems();
    std::cout << "status: ok\n";
    std::cout << "subsystem_count: " << subsystems.size() << "\n";
    for (const auto& subsystem : subsystems) {
        std::cout << "subsystem.id: " << subsystem.id << "\n";
        std::cout << "  title: " << subsystem.title << "\n";
        std::cout << "  vfp9_equivalent: " << subsystem.vfp9_equivalent << "\n";
        std::cout << "  copperfin_component: " << subsystem.copperfin_component << "\n";
        std::cout << "  host_kind: " << copperfin::studio::product_host_kind_name(subsystem.host_kind) << "\n";
        std::cout << "  current_status: " << subsystem.current_status << "\n";
        std::cout << "  parity_scope: " << subsystem.parity_scope << "\n";
        std::cout << "  modern_editor_direction: " << subsystem.modern_editor_direction << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied) {
        std::cerr << "warning: " << hardening.message << "\n";
    }

    std::vector<std::string> args;
    args.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0U);
    for (int index = 1; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }

    const bool list_subsystems = std::find(args.begin(), args.end(), "--list-subsystems") != args.end();
    if (list_subsystems) {
        const bool output_json = std::find(args.begin(), args.end(), "--json") != args.end();
        if (output_json) {
            print_json_subsystems();
        } else {
            print_subsystems();
        }
        return 0;
    }

    const auto toolbox_create_parse = parse_toolbox_create_arguments(args);
    if (toolbox_create_parse.requested) {
        if (!toolbox_create_parse.ok) {
            const auto result = copperfin::vfp::VisualObjectCreateResult{
                .ok = false,
                .error = toolbox_create_parse.error,
                .record_index = 0U,
                .object_name = {},
                .unique_id = {},
                .parent_name = {}
            };
            if (toolbox_create_parse.output_json) {
                print_json_toolbox_create_result(result);
            } else {
                print_text_toolbox_create_result(result);
                print_usage();
            }
            return 2;
        }

        const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item(
            toolbox_create_parse.request);
        if (toolbox_create_parse.output_json) {
            print_json_toolbox_create_result(create_result);
        } else {
            print_text_toolbox_create_result(create_result);
        }
        return create_result.ok ? 0 : 4;
    }

    const auto parse_result = copperfin::studio::parse_launch_arguments(args);
    if (!parse_result.ok) {
        std::cout << "status: error\n";
        std::cout << "error: " << parse_result.error << "\n";
        print_usage();
        return 2;
    }

    if (parse_result.show_help) {
        print_usage();
        return 0;
    }

    if (parse_result.request.undo_mode == copperfin::studio::StudioUndoMode::command) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(parse_result.request.path);
        if (!undo_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << undo_result.error << "\n";
            return 5;
        }
    }

    if (parse_result.request.apply_property_update) {
        const auto update_result = copperfin::vfp::update_visual_object_property({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .property_name = parse_result.request.property_name,
            .property_value = parse_result.request.property_value
        });

        if (!update_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << update_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.clear_property) {
        const auto clear_result = copperfin::vfp::clear_visual_object_property({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .property_name = parse_result.request.property_name
        });

        if (!clear_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << clear_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.rename_property) {
        const auto rename_result = copperfin::vfp::rename_visual_object_property({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .property_name = parse_result.request.property_name,
            .new_property_name = parse_result.request.new_property_name
        });

        if (!rename_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << rename_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.delete_object) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .deleted = true
        });

        if (!delete_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << delete_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.restore_object) {
        const auto restore_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .deleted = false
        });

        if (!restore_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << restore_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.duplicate_object) {
        const auto duplicate_result = copperfin::vfp::duplicate_visual_object({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .new_object_name = parse_result.request.new_object_name,
            .new_name = parse_result.request.new_name,
            .new_unique_id = parse_result.request.new_unique_id
        });

        if (!duplicate_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << duplicate_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.rename_object) {
        const auto rename_result = copperfin::vfp::rename_visual_object({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .update_object_name = !parse_result.request.new_object_name.empty(),
            .new_object_name = parse_result.request.new_object_name,
            .update_name = !parse_result.request.new_name.empty(),
            .new_name = parse_result.request.new_name,
            .update_unique_id = !parse_result.request.new_unique_id.empty(),
            .new_unique_id = parse_result.request.new_unique_id
        });

        if (!rename_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << rename_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.reparent_object) {
        const auto reparent_result = copperfin::vfp::reparent_visual_object({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .parent_object_name = parse_result.request.parent_name,
            .parent_unique_id = parse_result.request.parent_unique_id,
            .clear_parent = parse_result.request.clear_parent
        });

        if (!reparent_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << reparent_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.reorder_object) {
        const auto reorder_result = copperfin::vfp::reorder_visual_object({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .placement = parse_result.request.placement,
            .target_object_name = parse_result.request.target_object_name,
            .target_unique_id = parse_result.request.target_unique_id
        });

        if (!reorder_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << reorder_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.group_object) {
        std::vector<copperfin::vfp::VisualObjectPropertyChange> container_field_values;
        container_field_values.reserve(parse_result.request.field_values.size());
        for (const auto& field_value : parse_result.request.field_values) {
            container_field_values.push_back({
                .property_name = field_value.property_name,
                .property_value = field_value.property_value
            });
        }

        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> group_objects;
        group_objects.reserve(parse_result.request.group_objects.size());
        for (const auto& group_object : parse_result.request.group_objects) {
            group_objects.push_back({
                .record_index = group_object.record_index,
                .object_name = group_object.object_name,
                .unique_id = group_object.unique_id
            });
        }

        const auto group_result = copperfin::vfp::group_visual_objects({
            .path = parse_result.request.path,
            .container_field_values = container_field_values,
            .objects = group_objects
        });

        if (!group_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << group_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.align_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> align_objects;
        align_objects.reserve(parse_result.request.align_objects.size());
        for (const auto& align_object : parse_result.request.align_objects) {
            align_objects.push_back({
                .record_index = align_object.record_index,
                .object_name = align_object.object_name,
                .unique_id = align_object.unique_id
            });
        }

        const auto align_result = copperfin::vfp::align_visual_objects({
            .path = parse_result.request.path,
            .anchor_record_index = 0U,
            .anchor_object_name = parse_result.request.anchor_object_name,
            .anchor_unique_id = parse_result.request.anchor_unique_id,
            .objects = align_objects,
            .mode = parse_result.request.alignment_mode
        });

        if (!align_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << align_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.resize_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> resize_objects;
        resize_objects.reserve(parse_result.request.resize_objects.size());
        for (const auto& resize_object : parse_result.request.resize_objects) {
            resize_objects.push_back({
                .record_index = resize_object.record_index,
                .object_name = resize_object.object_name,
                .unique_id = resize_object.unique_id
            });
        }

        const auto resize_result = copperfin::vfp::resize_visual_objects({
            .path = parse_result.request.path,
            .anchor_record_index = 0U,
            .anchor_object_name = parse_result.request.anchor_object_name,
            .anchor_unique_id = parse_result.request.anchor_unique_id,
            .objects = resize_objects,
            .mode = parse_result.request.resize_mode
        });

        if (!resize_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << resize_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.distribute_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> distribute_objects;
        distribute_objects.reserve(parse_result.request.distribute_objects.size());
        for (const auto& distribute_object : parse_result.request.distribute_objects) {
            distribute_objects.push_back({
                .record_index = distribute_object.record_index,
                .object_name = distribute_object.object_name,
                .unique_id = distribute_object.unique_id
            });
        }

        const auto distribute_result = copperfin::vfp::distribute_visual_objects({
            .path = parse_result.request.path,
            .objects = distribute_objects,
            .mode = parse_result.request.distribution_mode
        });

        if (!distribute_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << distribute_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.snap_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> snap_objects;
        snap_objects.reserve(parse_result.request.snap_objects.size());
        for (const auto& snap_object : parse_result.request.snap_objects) {
            snap_objects.push_back({
                .record_index = snap_object.record_index,
                .object_name = snap_object.object_name,
                .unique_id = snap_object.unique_id
            });
        }

        const auto snap_result = copperfin::vfp::snap_visual_objects_to_grid({
            .path = parse_result.request.path,
            .objects = snap_objects,
            .mode = parse_result.request.snap_mode,
            .grid_width = parse_result.request.grid_width,
            .grid_height = parse_result.request.grid_height
        });

        if (!snap_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << snap_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.nudge_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> nudge_objects;
        nudge_objects.reserve(parse_result.request.nudge_objects.size());
        for (const auto& nudge_object : parse_result.request.nudge_objects) {
            nudge_objects.push_back({
                .record_index = nudge_object.record_index,
                .object_name = nudge_object.object_name,
                .unique_id = nudge_object.unique_id
            });
        }

        const auto nudge_result = copperfin::vfp::nudge_visual_objects({
            .path = parse_result.request.path,
            .objects = nudge_objects,
            .mode = parse_result.request.nudge_mode,
            .delta_hpos = parse_result.request.delta_hpos,
            .delta_vpos = parse_result.request.delta_vpos
        });

        if (!nudge_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << nudge_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.tab_order_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> tab_order_objects;
        tab_order_objects.reserve(parse_result.request.tab_order_objects.size());
        for (const auto& tab_order_object : parse_result.request.tab_order_objects) {
            tab_order_objects.push_back({
                .record_index = tab_order_object.record_index,
                .object_name = tab_order_object.object_name,
                .unique_id = tab_order_object.unique_id
            });
        }

        const auto tab_order_result = copperfin::vfp::set_visual_object_tab_order({
            .path = parse_result.request.path,
            .objects = tab_order_objects,
            .starting_tab_index = parse_result.request.starting_tab_index
        });

        if (!tab_order_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << tab_order_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.tab_stop_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> tab_stop_objects;
        tab_stop_objects.reserve(parse_result.request.tab_stop_objects.size());
        for (const auto& tab_stop_object : parse_result.request.tab_stop_objects) {
            tab_stop_objects.push_back({
                .record_index = tab_stop_object.record_index,
                .object_name = tab_stop_object.object_name,
                .unique_id = tab_stop_object.unique_id
            });
        }

        const auto tab_stop_result = copperfin::vfp::set_visual_object_tab_stop({
            .path = parse_result.request.path,
            .objects = tab_stop_objects,
            .tab_stop = parse_result.request.tab_stop
        });

        if (!tab_stop_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << tab_stop_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.visibility_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> visibility_objects;
        visibility_objects.reserve(parse_result.request.visibility_objects.size());
        for (const auto& visibility_object : parse_result.request.visibility_objects) {
            visibility_objects.push_back({
                .record_index = visibility_object.record_index,
                .object_name = visibility_object.object_name,
                .unique_id = visibility_object.unique_id
            });
        }

        const auto visibility_result = copperfin::vfp::set_visual_object_visibility({
            .path = parse_result.request.path,
            .objects = visibility_objects,
            .visible = parse_result.request.visible
        });

        if (!visibility_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << visibility_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.enabled_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> enabled_objects;
        enabled_objects.reserve(parse_result.request.enabled_objects.size());
        for (const auto& enabled_object : parse_result.request.enabled_objects) {
            enabled_objects.push_back({
                .record_index = enabled_object.record_index,
                .object_name = enabled_object.object_name,
                .unique_id = enabled_object.unique_id
            });
        }

        const auto enabled_result = copperfin::vfp::set_visual_object_enabled({
            .path = parse_result.request.path,
            .objects = enabled_objects,
            .enabled = parse_result.request.enabled
        });

        if (!enabled_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << enabled_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.read_only_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> read_only_objects;
        read_only_objects.reserve(parse_result.request.read_only_objects.size());
        for (const auto& read_only_object : parse_result.request.read_only_objects) {
            read_only_objects.push_back({
                .record_index = read_only_object.record_index,
                .object_name = read_only_object.object_name,
                .unique_id = read_only_object.unique_id
            });
        }

        const auto read_only_result = copperfin::vfp::set_visual_object_read_only({
            .path = parse_result.request.path,
            .objects = read_only_objects,
            .read_only = parse_result.request.object_read_only
        });

        if (!read_only_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << read_only_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.locked_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> locked_objects;
        locked_objects.reserve(parse_result.request.locked_objects.size());
        for (const auto& locked_object : parse_result.request.locked_objects) {
            locked_objects.push_back({
                .record_index = locked_object.record_index,
                .object_name = locked_object.object_name,
                .unique_id = locked_object.unique_id
            });
        }

        const auto locked_result = copperfin::vfp::set_visual_object_locked({
            .path = parse_result.request.path,
            .objects = locked_objects,
            .locked = parse_result.request.locked
        });

        if (!locked_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << locked_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.caption_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> caption_objects;
        caption_objects.reserve(parse_result.request.caption_objects.size());
        for (const auto& caption_object : parse_result.request.caption_objects) {
            caption_objects.push_back({
                .record_index = caption_object.record_index,
                .object_name = caption_object.object_name,
                .unique_id = caption_object.unique_id
            });
        }

        const auto caption_result = copperfin::vfp::set_visual_object_caption({
            .path = parse_result.request.path,
            .objects = caption_objects,
            .caption = parse_result.request.caption
        });

        if (!caption_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << caption_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.picture_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> picture_objects;
        picture_objects.reserve(parse_result.request.picture_objects.size());
        for (const auto& picture_object : parse_result.request.picture_objects) {
            picture_objects.push_back({
                .record_index = picture_object.record_index,
                .object_name = picture_object.object_name,
                .unique_id = picture_object.unique_id
            });
        }

        const auto picture_result = copperfin::vfp::set_visual_object_picture({
            .path = parse_result.request.path,
            .objects = picture_objects,
            .picture = parse_result.request.picture
        });

        if (!picture_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << picture_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.down_picture_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> down_picture_objects;
        down_picture_objects.reserve(parse_result.request.down_picture_objects.size());
        for (const auto& down_picture_object : parse_result.request.down_picture_objects) {
            down_picture_objects.push_back({
                .record_index = down_picture_object.record_index,
                .object_name = down_picture_object.object_name,
                .unique_id = down_picture_object.unique_id
            });
        }

        const auto down_picture_result = copperfin::vfp::set_visual_object_down_picture({
            .path = parse_result.request.path,
            .objects = down_picture_objects,
            .down_picture = parse_result.request.down_picture
        });

        if (!down_picture_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << down_picture_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.disabled_picture_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> disabled_picture_objects;
        disabled_picture_objects.reserve(parse_result.request.disabled_picture_objects.size());
        for (const auto& disabled_picture_object : parse_result.request.disabled_picture_objects) {
            disabled_picture_objects.push_back({
                .record_index = disabled_picture_object.record_index,
                .object_name = disabled_picture_object.object_name,
                .unique_id = disabled_picture_object.unique_id
            });
        }

        const auto disabled_picture_result = copperfin::vfp::set_visual_object_disabled_picture({
            .path = parse_result.request.path,
            .objects = disabled_picture_objects,
            .disabled_picture = parse_result.request.disabled_picture
        });

        if (!disabled_picture_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << disabled_picture_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.ole_drag_picture_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> ole_drag_picture_objects;
        ole_drag_picture_objects.reserve(parse_result.request.ole_drag_picture_objects.size());
        for (const auto& ole_drag_picture_object : parse_result.request.ole_drag_picture_objects) {
            ole_drag_picture_objects.push_back({
                .record_index = ole_drag_picture_object.record_index,
                .object_name = ole_drag_picture_object.object_name,
                .unique_id = ole_drag_picture_object.unique_id
            });
        }

        const auto ole_drag_picture_result = copperfin::vfp::set_visual_object_ole_drag_picture({
            .path = parse_result.request.path,
            .objects = ole_drag_picture_objects,
            .ole_drag_picture = parse_result.request.ole_drag_picture
        });

        if (!ole_drag_picture_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << ole_drag_picture_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.tooltip_text_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> tooltip_text_objects;
        tooltip_text_objects.reserve(parse_result.request.tooltip_text_objects.size());
        for (const auto& tooltip_text_object : parse_result.request.tooltip_text_objects) {
            tooltip_text_objects.push_back({
                .record_index = tooltip_text_object.record_index,
                .object_name = tooltip_text_object.object_name,
                .unique_id = tooltip_text_object.unique_id
            });
        }

        const auto tooltip_text_result = copperfin::vfp::set_visual_object_tooltip_text({
            .path = parse_result.request.path,
            .objects = tooltip_text_objects,
            .tooltip_text = parse_result.request.tooltip_text
        });

        if (!tooltip_text_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << tooltip_text_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.status_bar_text_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> status_bar_text_objects;
        status_bar_text_objects.reserve(parse_result.request.status_bar_text_objects.size());
        for (const auto& status_bar_text_object : parse_result.request.status_bar_text_objects) {
            status_bar_text_objects.push_back({
                .record_index = status_bar_text_object.record_index,
                .object_name = status_bar_text_object.object_name,
                .unique_id = status_bar_text_object.unique_id
            });
        }

        const auto status_bar_text_result = copperfin::vfp::set_visual_object_status_bar_text({
            .path = parse_result.request.path,
            .objects = status_bar_text_objects,
            .status_bar_text = parse_result.request.status_bar_text
        });

        if (!status_bar_text_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << status_bar_text_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.control_source_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> control_source_objects;
        control_source_objects.reserve(parse_result.request.control_source_objects.size());
        for (const auto& control_source_object : parse_result.request.control_source_objects) {
            control_source_objects.push_back({
                .record_index = control_source_object.record_index,
                .object_name = control_source_object.object_name,
                .unique_id = control_source_object.unique_id
            });
        }

        const auto control_source_result = copperfin::vfp::set_visual_object_control_source({
            .path = parse_result.request.path,
            .objects = control_source_objects,
            .control_source = parse_result.request.control_source
        });

        if (!control_source_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << control_source_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.current_control_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> current_control_objects;
        current_control_objects.reserve(parse_result.request.current_control_objects.size());
        for (const auto& current_control_object : parse_result.request.current_control_objects) {
            current_control_objects.push_back({
                .record_index = current_control_object.record_index,
                .object_name = current_control_object.object_name,
                .unique_id = current_control_object.unique_id
            });
        }

        const auto current_control_result = copperfin::vfp::set_visual_object_current_control({
            .path = parse_result.request.path,
            .objects = current_control_objects,
            .current_control = parse_result.request.current_control
        });

        if (!current_control_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << current_control_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.input_mask_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> input_mask_objects;
        input_mask_objects.reserve(parse_result.request.input_mask_objects.size());
        for (const auto& input_mask_object : parse_result.request.input_mask_objects) {
            input_mask_objects.push_back({
                .record_index = input_mask_object.record_index,
                .object_name = input_mask_object.object_name,
                .unique_id = input_mask_object.unique_id
            });
        }

        const auto input_mask_result = copperfin::vfp::set_visual_object_input_mask({
            .path = parse_result.request.path,
            .objects = input_mask_objects,
            .input_mask = parse_result.request.input_mask
        });

        if (!input_mask_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << input_mask_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.format_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> format_objects;
        format_objects.reserve(parse_result.request.format_objects.size());
        for (const auto& format_object : parse_result.request.format_objects) {
            format_objects.push_back({
                .record_index = format_object.record_index,
                .object_name = format_object.object_name,
                .unique_id = format_object.unique_id
            });
        }

        const auto format_result = copperfin::vfp::set_visual_object_format({
            .path = parse_result.request.path,
            .objects = format_objects,
            .format = parse_result.request.format
        });

        if (!format_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << format_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.row_source_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> row_source_objects;
        row_source_objects.reserve(parse_result.request.row_source_objects.size());
        for (const auto& row_source_object : parse_result.request.row_source_objects) {
            row_source_objects.push_back({
                .record_index = row_source_object.record_index,
                .object_name = row_source_object.object_name,
                .unique_id = row_source_object.unique_id
            });
        }

        const auto row_source_result = copperfin::vfp::set_visual_object_row_source({
            .path = parse_result.request.path,
            .objects = row_source_objects,
            .row_source = parse_result.request.row_source
        });

        if (!row_source_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << row_source_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.row_source_type_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> row_source_type_objects;
        row_source_type_objects.reserve(parse_result.request.row_source_type_objects.size());
        for (const auto& row_source_type_object : parse_result.request.row_source_type_objects) {
            row_source_type_objects.push_back({
                .record_index = row_source_type_object.record_index,
                .object_name = row_source_type_object.object_name,
                .unique_id = row_source_type_object.unique_id
            });
        }

        const auto row_source_type_result = copperfin::vfp::set_visual_object_row_source_type({
            .path = parse_result.request.path,
            .objects = row_source_type_objects,
            .row_source_type = parse_result.request.row_source_type
        });

        if (!row_source_type_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << row_source_type_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.bound_column_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> bound_column_objects;
        bound_column_objects.reserve(parse_result.request.bound_column_objects.size());
        for (const auto& bound_column_object : parse_result.request.bound_column_objects) {
            bound_column_objects.push_back({
                .record_index = bound_column_object.record_index,
                .object_name = bound_column_object.object_name,
                .unique_id = bound_column_object.unique_id
            });
        }

        const auto bound_column_result = copperfin::vfp::set_visual_object_bound_column({
            .path = parse_result.request.path,
            .objects = bound_column_objects,
            .bound_column = parse_result.request.bound_column
        });

        if (!bound_column_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << bound_column_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.column_count_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> column_count_objects;
        column_count_objects.reserve(parse_result.request.column_count_objects.size());
        for (const auto& column_count_object : parse_result.request.column_count_objects) {
            column_count_objects.push_back({
                .record_index = column_count_object.record_index,
                .object_name = column_count_object.object_name,
                .unique_id = column_count_object.unique_id
            });
        }

        const auto column_count_result = copperfin::vfp::set_visual_object_column_count({
            .path = parse_result.request.path,
            .objects = column_count_objects,
            .column_count = parse_result.request.column_count
        });

        if (!column_count_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << column_count_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.style_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> style_objects;
        style_objects.reserve(parse_result.request.style_objects.size());
        for (const auto& style_object : parse_result.request.style_objects) {
            style_objects.push_back({
                .record_index = style_object.record_index,
                .object_name = style_object.object_name,
                .unique_id = style_object.unique_id
            });
        }

        const auto style_result = copperfin::vfp::set_visual_object_style({
            .path = parse_result.request.path,
            .objects = style_objects,
            .style = parse_result.request.style
        });

        if (!style_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << style_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.list_index_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> list_index_objects;
        list_index_objects.reserve(parse_result.request.list_index_objects.size());
        for (const auto& list_index_object : parse_result.request.list_index_objects) {
            list_index_objects.push_back({
                .record_index = list_index_object.record_index,
                .object_name = list_index_object.object_name,
                .unique_id = list_index_object.unique_id
            });
        }

        const auto list_index_result = copperfin::vfp::set_visual_object_list_index({
            .path = parse_result.request.path,
            .objects = list_index_objects,
            .list_index = parse_result.request.list_index
        });

        if (!list_index_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << list_index_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.left_column_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> left_column_objects;
        left_column_objects.reserve(parse_result.request.left_column_objects.size());
        for (const auto& left_column_object : parse_result.request.left_column_objects) {
            left_column_objects.push_back({
                .record_index = left_column_object.record_index,
                .object_name = left_column_object.object_name,
                .unique_id = left_column_object.unique_id
            });
        }

        const auto left_column_result = copperfin::vfp::set_visual_object_left_column({
            .path = parse_result.request.path,
            .objects = left_column_objects,
            .left_column = parse_result.request.left_column
        });

        if (!left_column_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << left_column_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.display_value_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> display_value_objects;
        display_value_objects.reserve(parse_result.request.display_value_objects.size());
        for (const auto& display_value_object : parse_result.request.display_value_objects) {
            display_value_objects.push_back({
                .record_index = display_value_object.record_index,
                .object_name = display_value_object.object_name,
                .unique_id = display_value_object.unique_id
            });
        }

        const auto display_value_result = copperfin::vfp::set_visual_object_display_value({
            .path = parse_result.request.path,
            .objects = display_value_objects,
            .display_value = parse_result.request.display_value
        });

        if (!display_value_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << display_value_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.selected_back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> selected_back_color_objects;
        selected_back_color_objects.reserve(parse_result.request.selected_back_color_objects.size());
        for (const auto& selected_back_color_object : parse_result.request.selected_back_color_objects) {
            selected_back_color_objects.push_back({
                .record_index = selected_back_color_object.record_index,
                .object_name = selected_back_color_object.object_name,
                .unique_id = selected_back_color_object.unique_id
            });
        }

        const auto selected_back_color_result = copperfin::vfp::set_visual_object_selected_back_color({
            .path = parse_result.request.path,
            .objects = selected_back_color_objects,
            .selected_back_color = parse_result.request.selected_back_color
        });

        if (!selected_back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << selected_back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.selected_fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> selected_fore_color_objects;
        selected_fore_color_objects.reserve(parse_result.request.selected_fore_color_objects.size());
        for (const auto& selected_fore_color_object : parse_result.request.selected_fore_color_objects) {
            selected_fore_color_objects.push_back({
                .record_index = selected_fore_color_object.record_index,
                .object_name = selected_fore_color_object.object_name,
                .unique_id = selected_fore_color_object.unique_id
            });
        }

        const auto selected_fore_color_result = copperfin::vfp::set_visual_object_selected_fore_color({
            .path = parse_result.request.path,
            .objects = selected_fore_color_objects,
            .selected_fore_color = parse_result.request.selected_fore_color
        });

        if (!selected_fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << selected_fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.selected_item_back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> selected_item_back_color_objects;
        selected_item_back_color_objects.reserve(parse_result.request.selected_item_back_color_objects.size());
        for (const auto& selected_item_back_color_object : parse_result.request.selected_item_back_color_objects) {
            selected_item_back_color_objects.push_back({
                .record_index = selected_item_back_color_object.record_index,
                .object_name = selected_item_back_color_object.object_name,
                .unique_id = selected_item_back_color_object.unique_id
            });
        }

        const auto selected_item_back_color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
            .path = parse_result.request.path,
            .objects = selected_item_back_color_objects,
            .selected_item_back_color = parse_result.request.selected_item_back_color
        });

        if (!selected_item_back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << selected_item_back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.selected_item_fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> selected_item_fore_color_objects;
        selected_item_fore_color_objects.reserve(parse_result.request.selected_item_fore_color_objects.size());
        for (const auto& selected_item_fore_color_object : parse_result.request.selected_item_fore_color_objects) {
            selected_item_fore_color_objects.push_back({
                .record_index = selected_item_fore_color_object.record_index,
                .object_name = selected_item_fore_color_object.object_name,
                .unique_id = selected_item_fore_color_object.unique_id
            });
        }

        const auto selected_item_fore_color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
            .path = parse_result.request.path,
            .objects = selected_item_fore_color_objects,
            .selected_item_fore_color = parse_result.request.selected_item_fore_color
        });

        if (!selected_item_fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << selected_item_fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.disabled_item_back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> disabled_item_back_color_objects;
        disabled_item_back_color_objects.reserve(parse_result.request.disabled_item_back_color_objects.size());
        for (const auto& disabled_item_back_color_object : parse_result.request.disabled_item_back_color_objects) {
            disabled_item_back_color_objects.push_back({
                .record_index = disabled_item_back_color_object.record_index,
                .object_name = disabled_item_back_color_object.object_name,
                .unique_id = disabled_item_back_color_object.unique_id
            });
        }

        const auto disabled_item_back_color_result = copperfin::vfp::set_visual_object_disabled_item_back_color({
            .path = parse_result.request.path,
            .objects = disabled_item_back_color_objects,
            .disabled_item_back_color = parse_result.request.disabled_item_back_color
        });

        if (!disabled_item_back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << disabled_item_back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.disabled_item_fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> disabled_item_fore_color_objects;
        disabled_item_fore_color_objects.reserve(parse_result.request.disabled_item_fore_color_objects.size());
        for (const auto& disabled_item_fore_color_object : parse_result.request.disabled_item_fore_color_objects) {
            disabled_item_fore_color_objects.push_back({
                .record_index = disabled_item_fore_color_object.record_index,
                .object_name = disabled_item_fore_color_object.object_name,
                .unique_id = disabled_item_fore_color_object.unique_id
            });
        }

        const auto disabled_item_fore_color_result = copperfin::vfp::set_visual_object_disabled_item_fore_color({
            .path = parse_result.request.path,
            .objects = disabled_item_fore_color_objects,
            .disabled_item_fore_color = parse_result.request.disabled_item_fore_color
        });

        if (!disabled_item_fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << disabled_item_fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.item_back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> item_back_color_objects;
        item_back_color_objects.reserve(parse_result.request.item_back_color_objects.size());
        for (const auto& item_back_color_object : parse_result.request.item_back_color_objects) {
            item_back_color_objects.push_back({
                .record_index = item_back_color_object.record_index,
                .object_name = item_back_color_object.object_name,
                .unique_id = item_back_color_object.unique_id
            });
        }

        const auto item_back_color_result = copperfin::vfp::set_visual_object_item_back_color({
            .path = parse_result.request.path,
            .objects = item_back_color_objects,
            .item_back_color = parse_result.request.item_back_color
        });

        if (!item_back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << item_back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.item_fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> item_fore_color_objects;
        item_fore_color_objects.reserve(parse_result.request.item_fore_color_objects.size());
        for (const auto& item_fore_color_object : parse_result.request.item_fore_color_objects) {
            item_fore_color_objects.push_back({
                .record_index = item_fore_color_object.record_index,
                .object_name = item_fore_color_object.object_name,
                .unique_id = item_fore_color_object.unique_id
            });
        }

        const auto item_fore_color_result = copperfin::vfp::set_visual_object_item_fore_color({
            .path = parse_result.request.path,
            .objects = item_fore_color_objects,
            .item_fore_color = parse_result.request.item_fore_color
        });

        if (!item_fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << item_fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.highlight_back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> highlight_back_color_objects;
        highlight_back_color_objects.reserve(parse_result.request.highlight_back_color_objects.size());
        for (const auto& highlight_back_color_object : parse_result.request.highlight_back_color_objects) {
            highlight_back_color_objects.push_back({
                .record_index = highlight_back_color_object.record_index,
                .object_name = highlight_back_color_object.object_name,
                .unique_id = highlight_back_color_object.unique_id
            });
        }

        const auto highlight_back_color_result = copperfin::vfp::set_visual_object_highlight_back_color({
            .path = parse_result.request.path,
            .objects = highlight_back_color_objects,
            .highlight_back_color = parse_result.request.highlight_back_color
        });

        if (!highlight_back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << highlight_back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.highlight_fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> highlight_fore_color_objects;
        highlight_fore_color_objects.reserve(parse_result.request.highlight_fore_color_objects.size());
        for (const auto& highlight_fore_color_object : parse_result.request.highlight_fore_color_objects) {
            highlight_fore_color_objects.push_back({
                .record_index = highlight_fore_color_object.record_index,
                .object_name = highlight_fore_color_object.object_name,
                .unique_id = highlight_fore_color_object.unique_id
            });
        }

        const auto highlight_fore_color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
            .path = parse_result.request.path,
            .objects = highlight_fore_color_objects,
            .highlight_fore_color = parse_result.request.highlight_fore_color
        });

        if (!highlight_fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << highlight_fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> back_color_objects;
        back_color_objects.reserve(parse_result.request.back_color_objects.size());
        for (const auto& back_color_object : parse_result.request.back_color_objects) {
            back_color_objects.push_back({
                .record_index = back_color_object.record_index,
                .object_name = back_color_object.object_name,
                .unique_id = back_color_object.unique_id
            });
        }

        const auto back_color_result = copperfin::vfp::set_visual_object_back_color({
            .path = parse_result.request.path,
            .objects = back_color_objects,
            .back_color = parse_result.request.back_color
        });

        if (!back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> fore_color_objects;
        fore_color_objects.reserve(parse_result.request.fore_color_objects.size());
        for (const auto& fore_color_object : parse_result.request.fore_color_objects) {
            fore_color_objects.push_back({
                .record_index = fore_color_object.record_index,
                .object_name = fore_color_object.object_name,
                .unique_id = fore_color_object.unique_id
            });
        }

        const auto fore_color_result = copperfin::vfp::set_visual_object_fore_color({
            .path = parse_result.request.path,
            .objects = fore_color_objects,
            .fore_color = parse_result.request.fore_color
        });

        if (!fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.disabled_back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> disabled_back_color_objects;
        disabled_back_color_objects.reserve(parse_result.request.disabled_back_color_objects.size());
        for (const auto& disabled_back_color_object : parse_result.request.disabled_back_color_objects) {
            disabled_back_color_objects.push_back({
                .record_index = disabled_back_color_object.record_index,
                .object_name = disabled_back_color_object.object_name,
                .unique_id = disabled_back_color_object.unique_id
            });
        }

        const auto disabled_back_color_result = copperfin::vfp::set_visual_object_disabled_back_color({
            .path = parse_result.request.path,
            .objects = disabled_back_color_objects,
            .disabled_back_color = parse_result.request.disabled_back_color
        });

        if (!disabled_back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << disabled_back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.disabled_fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> disabled_fore_color_objects;
        disabled_fore_color_objects.reserve(parse_result.request.disabled_fore_color_objects.size());
        for (const auto& disabled_fore_color_object : parse_result.request.disabled_fore_color_objects) {
            disabled_fore_color_objects.push_back({
                .record_index = disabled_fore_color_object.record_index,
                .object_name = disabled_fore_color_object.object_name,
                .unique_id = disabled_fore_color_object.unique_id
            });
        }

        const auto disabled_fore_color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
            .path = parse_result.request.path,
            .objects = disabled_fore_color_objects,
            .disabled_fore_color = parse_result.request.disabled_fore_color
        });

        if (!disabled_fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << disabled_fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.dynamic_back_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_back_color_objects;
        dynamic_back_color_objects.reserve(parse_result.request.dynamic_back_color_objects.size());
        for (const auto& dynamic_back_color_object : parse_result.request.dynamic_back_color_objects) {
            dynamic_back_color_objects.push_back({
                .record_index = dynamic_back_color_object.record_index,
                .object_name = dynamic_back_color_object.object_name,
                .unique_id = dynamic_back_color_object.unique_id
            });
        }

        const auto dynamic_back_color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
            .path = parse_result.request.path,
            .objects = dynamic_back_color_objects,
            .dynamic_back_color = parse_result.request.dynamic_back_color
        });

        if (!dynamic_back_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << dynamic_back_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.dynamic_fore_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_fore_color_objects;
        dynamic_fore_color_objects.reserve(parse_result.request.dynamic_fore_color_objects.size());
        for (const auto& dynamic_fore_color_object : parse_result.request.dynamic_fore_color_objects) {
            dynamic_fore_color_objects.push_back({
                .record_index = dynamic_fore_color_object.record_index,
                .object_name = dynamic_fore_color_object.object_name,
                .unique_id = dynamic_fore_color_object.unique_id
            });
        }

        const auto dynamic_fore_color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
            .path = parse_result.request.path,
            .objects = dynamic_fore_color_objects,
            .dynamic_fore_color = parse_result.request.dynamic_fore_color
        });

        if (!dynamic_fore_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << dynamic_fore_color_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.closable_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> closable_objects;
        closable_objects.reserve(parse_result.request.closable_objects.size());
        for (const auto& closable_object : parse_result.request.closable_objects) {
            closable_objects.push_back({
                .record_index = closable_object.record_index,
                .object_name = closable_object.object_name,
                .unique_id = closable_object.unique_id
            });
        }

        const auto closable_result = copperfin::vfp::set_visual_object_closable({
            .path = parse_result.request.path,
            .objects = closable_objects,
            .closable = parse_result.request.closable
        });

        if (!closable_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << closable_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.control_box_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> control_box_objects;
        control_box_objects.reserve(parse_result.request.control_box_objects.size());
        for (const auto& control_box_object : parse_result.request.control_box_objects) {
            control_box_objects.push_back({
                .record_index = control_box_object.record_index,
                .object_name = control_box_object.object_name,
                .unique_id = control_box_object.unique_id
            });
        }

        const auto control_box_result = copperfin::vfp::set_visual_object_control_box({
            .path = parse_result.request.path,
            .objects = control_box_objects,
            .control_box = parse_result.request.control_box
        });

        if (!control_box_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << control_box_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.allow_output_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> allow_output_objects;
        allow_output_objects.reserve(parse_result.request.allow_output_objects.size());
        for (const auto& allow_output_object : parse_result.request.allow_output_objects) {
            allow_output_objects.push_back({
                .record_index = allow_output_object.record_index,
                .object_name = allow_output_object.object_name,
                .unique_id = allow_output_object.unique_id
            });
        }

        const auto allow_output_result = copperfin::vfp::set_visual_object_allow_output({
            .path = parse_result.request.path,
            .objects = allow_output_objects,
            .allow_output = parse_result.request.allow_output
        });

        if (!allow_output_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << allow_output_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.auto_center_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> auto_center_objects;
        auto_center_objects.reserve(parse_result.request.auto_center_objects.size());
        for (const auto& auto_center_object : parse_result.request.auto_center_objects) {
            auto_center_objects.push_back({
                .record_index = auto_center_object.record_index,
                .object_name = auto_center_object.object_name,
                .unique_id = auto_center_object.unique_id
            });
        }

        const auto auto_center_result = copperfin::vfp::set_visual_object_auto_center({
            .path = parse_result.request.path,
            .objects = auto_center_objects,
            .auto_center = parse_result.request.auto_center
        });

        if (!auto_center_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << auto_center_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.auto_size_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> auto_size_objects;
        auto_size_objects.reserve(parse_result.request.auto_size_objects.size());
        for (const auto& auto_size_object : parse_result.request.auto_size_objects) {
            auto_size_objects.push_back({
                .record_index = auto_size_object.record_index,
                .object_name = auto_size_object.object_name,
                .unique_id = auto_size_object.unique_id
            });
        }

        const auto auto_size_result = copperfin::vfp::set_visual_object_auto_size({
            .path = parse_result.request.path,
            .objects = auto_size_objects,
            .auto_size = parse_result.request.auto_size
        });

        if (!auto_size_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << auto_size_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.auto_release_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> auto_release_objects;
        auto_release_objects.reserve(parse_result.request.auto_release_objects.size());
        for (const auto& auto_release_object : parse_result.request.auto_release_objects) {
            auto_release_objects.push_back({
                .record_index = auto_release_object.record_index,
                .object_name = auto_release_object.object_name,
                .unique_id = auto_release_object.unique_id
            });
        }

        const auto auto_release_result = copperfin::vfp::set_visual_object_auto_release({
            .path = parse_result.request.path,
            .objects = auto_release_objects,
            .auto_release = parse_result.request.auto_release
        });

        if (!auto_release_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << auto_release_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.continuous_scroll_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> continuous_scroll_objects;
        continuous_scroll_objects.reserve(parse_result.request.continuous_scroll_objects.size());
        for (const auto& continuous_scroll_object : parse_result.request.continuous_scroll_objects) {
            continuous_scroll_objects.push_back({
                .record_index = continuous_scroll_object.record_index,
                .object_name = continuous_scroll_object.object_name,
                .unique_id = continuous_scroll_object.unique_id
            });
        }

        const auto continuous_scroll_result = copperfin::vfp::set_visual_object_continuous_scroll({
            .path = parse_result.request.path,
            .objects = continuous_scroll_objects,
            .continuous_scroll = parse_result.request.continuous_scroll
        });

        if (!continuous_scroll_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << continuous_scroll_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.dockable_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dockable_objects;
        dockable_objects.reserve(parse_result.request.dockable_objects.size());
        for (const auto& dockable_object : parse_result.request.dockable_objects) {
            dockable_objects.push_back({
                .record_index = dockable_object.record_index,
                .object_name = dockable_object.object_name,
                .unique_id = dockable_object.unique_id
            });
        }

        const auto dockable_result = copperfin::vfp::set_visual_object_dockable({
            .path = parse_result.request.path,
            .objects = dockable_objects,
            .dockable = parse_result.request.dockable
        });

        if (!dockable_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << dockable_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.clip_controls_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> clip_controls_objects;
        clip_controls_objects.reserve(parse_result.request.clip_controls_objects.size());
        for (const auto& clip_controls_object : parse_result.request.clip_controls_objects) {
            clip_controls_objects.push_back({
                .record_index = clip_controls_object.record_index,
                .object_name = clip_controls_object.object_name,
                .unique_id = clip_controls_object.unique_id
            });
        }

        const auto clip_controls_result = copperfin::vfp::set_visual_object_clip_controls({
            .path = parse_result.request.path,
            .objects = clip_controls_objects,
            .clip_controls = parse_result.request.clip_controls
        });

        if (!clip_controls_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << clip_controls_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.sparse_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> sparse_objects;
        sparse_objects.reserve(parse_result.request.sparse_objects.size());
        for (const auto& sparse_object : parse_result.request.sparse_objects) {
            sparse_objects.push_back({
                .record_index = sparse_object.record_index,
                .object_name = sparse_object.object_name,
                .unique_id = sparse_object.unique_id
            });
        }

        const auto sparse_result = copperfin::vfp::set_visual_object_sparse({
            .path = parse_result.request.path,
            .objects = sparse_objects,
            .sparse = parse_result.request.sparse
        });

        if (!sparse_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << sparse_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.lock_screen_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> lock_screen_objects;
        lock_screen_objects.reserve(parse_result.request.lock_screen_objects.size());
        for (const auto& lock_screen_object : parse_result.request.lock_screen_objects) {
            lock_screen_objects.push_back({
                .record_index = lock_screen_object.record_index,
                .object_name = lock_screen_object.object_name,
                .unique_id = lock_screen_object.unique_id
            });
        }

        const auto lock_screen_result = copperfin::vfp::set_visual_object_lock_screen({
            .path = parse_result.request.path,
            .objects = lock_screen_objects,
            .lock_screen = parse_result.request.lock_screen
        });

        if (!lock_screen_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << lock_screen_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.allow_cell_selection_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> allow_cell_selection_objects;
        allow_cell_selection_objects.reserve(parse_result.request.allow_cell_selection_objects.size());
        for (const auto& allow_cell_selection_object : parse_result.request.allow_cell_selection_objects) {
            allow_cell_selection_objects.push_back({
                .record_index = allow_cell_selection_object.record_index,
                .object_name = allow_cell_selection_object.object_name,
                .unique_id = allow_cell_selection_object.unique_id
            });
        }

        const auto allow_cell_selection_result = copperfin::vfp::set_visual_object_allow_cell_selection({
            .path = parse_result.request.path,
            .objects = allow_cell_selection_objects,
            .allow_cell_selection = parse_result.request.allow_cell_selection
        });

        if (!allow_cell_selection_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << allow_cell_selection_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.delete_mark_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> delete_mark_objects;
        delete_mark_objects.reserve(parse_result.request.delete_mark_objects.size());
        for (const auto& delete_mark_object : parse_result.request.delete_mark_objects) {
            delete_mark_objects.push_back({
                .record_index = delete_mark_object.record_index,
                .object_name = delete_mark_object.object_name,
                .unique_id = delete_mark_object.unique_id
            });
        }

        const auto delete_mark_result = copperfin::vfp::set_visual_object_delete_mark({
            .path = parse_result.request.path,
            .objects = delete_mark_objects,
            .delete_mark = parse_result.request.delete_mark
        });

        if (!delete_mark_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << delete_mark_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.record_mark_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> record_mark_objects;
        record_mark_objects.reserve(parse_result.request.record_mark_objects.size());
        for (const auto& record_mark_object : parse_result.request.record_mark_objects) {
            record_mark_objects.push_back({
                .record_index = record_mark_object.record_index,
                .object_name = record_mark_object.object_name,
                .unique_id = record_mark_object.unique_id
            });
        }

        const auto record_mark_result = copperfin::vfp::set_visual_object_record_mark({
            .path = parse_result.request.path,
            .objects = record_mark_objects,
            .record_mark = parse_result.request.record_mark
        });

        if (!record_mark_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << record_mark_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.split_bar_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> split_bar_objects;
        split_bar_objects.reserve(parse_result.request.split_bar_objects.size());
        for (const auto& split_bar_object : parse_result.request.split_bar_objects) {
            split_bar_objects.push_back({
                .record_index = split_bar_object.record_index,
                .object_name = split_bar_object.object_name,
                .unique_id = split_bar_object.unique_id
            });
        }

        const auto split_bar_result = copperfin::vfp::set_visual_object_split_bar({
            .path = parse_result.request.path,
            .objects = split_bar_objects,
            .split_bar = parse_result.request.split_bar
        });

        if (!split_bar_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << split_bar_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.highlight_row_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> highlight_row_objects;
        highlight_row_objects.reserve(parse_result.request.highlight_row_objects.size());
        for (const auto& highlight_row_object : parse_result.request.highlight_row_objects) {
            highlight_row_objects.push_back({
                .record_index = highlight_row_object.record_index,
                .object_name = highlight_row_object.object_name,
                .unique_id = highlight_row_object.unique_id
            });
        }

        const auto highlight_row_result = copperfin::vfp::set_visual_object_highlight_row({
            .path = parse_result.request.path,
            .objects = highlight_row_objects,
            .highlight_row = parse_result.request.highlight_row
        });

        if (!highlight_row_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << highlight_row_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.panel_link_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> panel_link_objects;
        panel_link_objects.reserve(parse_result.request.panel_link_objects.size());
        for (const auto& panel_link_object : parse_result.request.panel_link_objects) {
            panel_link_objects.push_back({
                .record_index = panel_link_object.record_index,
                .object_name = panel_link_object.object_name,
                .unique_id = panel_link_object.unique_id
            });
        }

        const auto panel_link_result = copperfin::vfp::set_visual_object_panel_link({
            .path = parse_result.request.path,
            .objects = panel_link_objects,
            .panel_link = parse_result.request.panel_link
        });

        if (!panel_link_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << panel_link_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.allow_header_sizing_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> allow_header_sizing_objects;
        allow_header_sizing_objects.reserve(parse_result.request.allow_header_sizing_objects.size());
        for (const auto& allow_header_sizing_object : parse_result.request.allow_header_sizing_objects) {
            allow_header_sizing_objects.push_back({
                .record_index = allow_header_sizing_object.record_index,
                .object_name = allow_header_sizing_object.object_name,
                .unique_id = allow_header_sizing_object.unique_id
            });
        }

        const auto allow_header_sizing_result = copperfin::vfp::set_visual_object_allow_header_sizing({
            .path = parse_result.request.path,
            .objects = allow_header_sizing_objects,
            .allow_header_sizing = parse_result.request.allow_header_sizing
        });

        if (!allow_header_sizing_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << allow_header_sizing_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.allow_row_sizing_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> allow_row_sizing_objects;
        allow_row_sizing_objects.reserve(parse_result.request.allow_row_sizing_objects.size());
        for (const auto& allow_row_sizing_object : parse_result.request.allow_row_sizing_objects) {
            allow_row_sizing_objects.push_back({
                .record_index = allow_row_sizing_object.record_index,
                .object_name = allow_row_sizing_object.object_name,
                .unique_id = allow_row_sizing_object.unique_id
            });
        }

        const auto allow_row_sizing_result = copperfin::vfp::set_visual_object_allow_row_sizing({
            .path = parse_result.request.path,
            .objects = allow_row_sizing_objects,
            .allow_row_sizing = parse_result.request.allow_row_sizing
        });

        if (!allow_row_sizing_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << allow_row_sizing_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.resizable_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> resizable_objects;
        resizable_objects.reserve(parse_result.request.resizable_objects.size());
        for (const auto& resizable_object : parse_result.request.resizable_objects) {
            resizable_objects.push_back({
                .record_index = resizable_object.record_index,
                .object_name = resizable_object.object_name,
                .unique_id = resizable_object.unique_id
            });
        }

        const auto resizable_result = copperfin::vfp::set_visual_object_resizable({
            .path = parse_result.request.path,
            .objects = resizable_objects,
            .resizable = parse_result.request.resizable
        });

        if (!resizable_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << resizable_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.add_line_feeds_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> add_line_feeds_objects;
        add_line_feeds_objects.reserve(parse_result.request.add_line_feeds_objects.size());
        for (const auto& add_line_feeds_object : parse_result.request.add_line_feeds_objects) {
            add_line_feeds_objects.push_back({
                .record_index = add_line_feeds_object.record_index,
                .object_name = add_line_feeds_object.object_name,
                .unique_id = add_line_feeds_object.unique_id
            });
        }

        const auto add_line_feeds_result = copperfin::vfp::set_visual_object_add_line_feeds({
            .path = parse_result.request.path,
            .objects = add_line_feeds_objects,
            .add_line_feeds = parse_result.request.add_line_feeds
        });

        if (!add_line_feeds_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << add_line_feeds_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.always_on_top_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> always_on_top_objects;
        always_on_top_objects.reserve(parse_result.request.always_on_top_objects.size());
        for (const auto& always_on_top_object : parse_result.request.always_on_top_objects) {
            always_on_top_objects.push_back({
                .record_index = always_on_top_object.record_index,
                .object_name = always_on_top_object.object_name,
                .unique_id = always_on_top_object.unique_id
            });
        }

        const auto always_on_top_result = copperfin::vfp::set_visual_object_always_on_top({
            .path = parse_result.request.path,
            .objects = always_on_top_objects,
            .always_on_top = parse_result.request.always_on_top
        });

        if (!always_on_top_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << always_on_top_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.always_on_bottom_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> always_on_bottom_objects;
        always_on_bottom_objects.reserve(parse_result.request.always_on_bottom_objects.size());
        for (const auto& always_on_bottom_object : parse_result.request.always_on_bottom_objects) {
            always_on_bottom_objects.push_back({
                .record_index = always_on_bottom_object.record_index,
                .object_name = always_on_bottom_object.object_name,
                .unique_id = always_on_bottom_object.unique_id
            });
        }

        const auto always_on_bottom_result = copperfin::vfp::set_visual_object_always_on_bottom({
            .path = parse_result.request.path,
            .objects = always_on_bottom_objects,
            .always_on_bottom = parse_result.request.always_on_bottom
        });

        if (!always_on_bottom_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << always_on_bottom_result.error << "\n";
            return 4;
        }
    }

    if (parse_result.request.ungroup_object) {
        const auto ungroup_result = copperfin::vfp::ungroup_visual_object({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id
        });

        if (!ungroup_result.ok) {
            std::cout << "status: error\n";
            std::cout << "error: " << ungroup_result.error << "\n";
            return 4;
        }
    }

    const auto open_result = copperfin::studio::open_document(parse_result.request);
    if (!open_result.ok) {
        std::cout << "status: error\n";
        std::cout << "error: " << open_result.error << "\n";
        return 3;
    }

    if (parse_result.output_json) {
        print_json_document(open_result.document);
        return 0;
    }

    print_document(open_result.document);
    return 0;
}
